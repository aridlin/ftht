#define FTHT_IMPLEMENTATION
#include "../ftht.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

struct Options {
    std::string title = "Compile Overwatch";
    std::string host = "0.0.0.0";
    int port = 8090;
    int poll_ms = 1000;
    int max_output_kb = 512;
    double estimate_minutes = 0.0;
    bool word_wrap = false;

    std::string command;
    std::string log_path;
    std::string output_command;
    std::string process_pattern;
    std::string probe_command;
};

struct SharedState {
    std::mutex lock;
    std::string command_output;
    std::string live_output;
    std::string process_output;
    std::string probe_output;
    std::string status_note = "idle";
    int exit_code = -1;
    std::atomic<bool> command_started{false};
    std::atomic<bool> command_done{false};
    std::atomic<bool> process_running{false};
    std::atomic<bool> stop_monitor{false};
    Clock::time_point started_at = Clock::now();
};

static std::string join_args(int argc, char** argv, int first) {
    std::string out;
    for (int i = first; i < argc; ++i) {
        if (!out.empty()) out += " ";
        out += argv[i];
    }
    return out;
}

static bool parse_int(const char* text, int* out) {
    if (!text || !*text || !out) return false;
    char* end = nullptr;
    long value = std::strtol(text, &end, 10);
    if (!end || *end != '\0') return false;
    *out = (int)value;
    return true;
}

static bool parse_double(const char* text, double* out) {
    if (!text || !*text || !out) return false;
    char* end = nullptr;
    double value = std::strtod(text, &end);
    if (!end || *end != '\0') return false;
    *out = value;
    return true;
}

static std::string expand_path(std::string path) {
    if (path.size() >= 2 && path[0] == '~' && (path[1] == '/' || path[1] == '\\')) {
#if defined(_WIN32)
        const char* home = std::getenv("USERPROFILE");
#else
        const char* home = std::getenv("HOME");
#endif
        if (home && *home) path = std::string(home) + path.substr(1);
    }
    return path;
}

static std::string shell_quote_posix(const std::string& value) {
    std::string out = "'";
    for (char ch : value) {
        if (ch == '\'') out += "'\\''";
        else out += ch;
    }
    out += "'";
    return out;
}

static std::string html_escape(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (char ch : value) {
        switch (ch) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += ch; break;
        }
    }
    return out;
}

static std::string to_lower(std::string value) {
    for (char& ch : value) ch = (char)std::tolower((unsigned char)ch);
    return value;
}

static std::string ltrim_copy(const std::string& value) {
    size_t i = 0;
    while (i < value.size() && std::isspace((unsigned char)value[i])) ++i;
    return value.substr(i);
}

static bool starts_with(const std::string& value, const char* prefix) {
    size_t n = std::strlen(prefix);
    return value.size() >= n && value.compare(0, n, prefix) == 0;
}

static bool token_looks_like_path(const std::string& token) {
    std::string lower = to_lower(token);
    const char* exts[] = {
        ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx",
        ".s", ".S", ".o", ".a", ".so", ".ld", ".mk", "makefile"
    };
    bool has_ext = false;
    for (const char* ext : exts) {
        if (lower.find(to_lower(ext)) != std::string::npos) {
            has_ext = true;
            break;
        }
    }
    return has_ext && (token.find('/') != std::string::npos ||
                       token.find('\\') != std::string::npos ||
                       token.find(':') != std::string::npos ||
                       token.find('.') != std::string::npos);
}

static const char* token_class(const std::string& token) {
    std::string lower = to_lower(token);
    if (lower.find("error:") != std::string::npos ||
        lower == "error" ||
        lower.find("fatal") != std::string::npos) {
        return "ow-tok-error";
    }
    if (lower.find("warning:") != std::string::npos || lower == "warning") {
        return "ow-tok-warning";
    }
    if (lower.find("note:") != std::string::npos || lower == "note") {
        return "ow-tok-note";
    }
    if (token == "CC" || token == "LD" || token == "AR" || token == "AS" ||
        token == "HOSTCC" || token == "HOSTLD" || token == "CALL" ||
        token == "CHK" || token == "GEN" || token == "MAKE" || token == ">>>") {
        return "ow-tok-phase";
    }
    if (starts_with(token, "$") || starts_with(token, ">>>")) {
        return "ow-tok-command";
    }
    if (token_looks_like_path(token)) {
        return "ow-tok-path";
    }
    return nullptr;
}

static std::string highlight_tokens(const std::string& line) {
    std::string out;
    for (size_t i = 0; i < line.size();) {
        if (std::isspace((unsigned char)line[i])) {
            size_t j = i + 1;
            while (j < line.size() && std::isspace((unsigned char)line[j])) ++j;
            out += html_escape(line.substr(i, j - i));
            i = j;
            continue;
        }

        size_t j = i + 1;
        while (j < line.size() && !std::isspace((unsigned char)line[j])) ++j;
        std::string token = line.substr(i, j - i);
        const char* klass = token_class(token);
        if (klass) {
            out += "<span class=\"";
            out += klass;
            out += "\">";
            out += html_escape(token);
            out += "</span>";
        } else {
            out += html_escape(token);
        }
        i = j;
    }
    return out;
}

static std::string highlight_compiler_output(const std::string& text) {
    std::string out;
    size_t start = 0;
    while (start <= text.size()) {
        size_t end = text.find('\n', start);
        std::string line = end == std::string::npos ? text.substr(start) : text.substr(start, end - start);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::string clean_line;
        clean_line.reserve(line.size());
        for (size_t i = 0; i < line.size();) {
            if ((unsigned char)line[i] == 0x1b && i + 1 < line.size() && line[i + 1] == '[') {
                i += 2;
                while (i < line.size() && !std::isalpha((unsigned char)line[i])) ++i;
                if (i < line.size()) ++i;
                continue;
            }
            clean_line += line[i++];
        }
        line = clean_line;

        std::string lower = to_lower(line);
        std::string trimmed = ltrim_copy(line);
        std::string klass = "ow-line";
        if (lower.find("error:") != std::string::npos ||
            lower.find("fatal error") != std::string::npos ||
            lower.find("undefined reference") != std::string::npos ||
            lower.find("no such file") != std::string::npos) {
            klass += " ow-line-error";
        } else if (lower.find("warning:") != std::string::npos) {
            klass += " ow-line-warning";
        } else if (lower.find("note:") != std::string::npos) {
            klass += " ow-line-note";
        } else if (starts_with(trimmed, "$") || starts_with(trimmed, ">>>")) {
            klass += " ow-line-command";
        } else if (starts_with(trimmed, "CC ") || starts_with(trimmed, "LD ") ||
                   starts_with(trimmed, "AR ") || starts_with(trimmed, "AS ") ||
                   starts_with(trimmed, "HOSTCC ") || starts_with(trimmed, "MAKE ")) {
            klass += " ow-line-phase";
        }

        out += "<span class=\"";
        out += klass;
        out += "\">";
        out += highlight_tokens(line);
        out += "</span>";
        if (end == std::string::npos) break;
        out += "\n";
        start = end + 1;
    }
    return out;
}

static void compiler_output_view(const char* label, const std::string& value,
                                 int rows, bool wrap, bool auto_scroll) {
    int height = rows * 24 + 24;
    std::string html =
        "<label class=\"ft-field\"><span class=\"ft-label\">" +
        html_escape(label ? label : "") +
        "</span><pre id=\"ow-output\" class=\"ow-code";
    if (wrap) html += " ow-wrap";
    html += "\"";
    if (auto_scroll) html += " data-autoscroll=\"1\"";
    html += " style=\"max-height:" + std::to_string(height) + "px\">";
    html += highlight_compiler_output(value);
    html += "</pre></label>";
    ftht::html(html.c_str());
}

static std::string format_duration(long long seconds) {
    if (seconds < 0) seconds = 0;
    long long h = seconds / 3600;
    long long m = (seconds % 3600) / 60;
    long long s = seconds % 60;
    char buf[64];
    if (h > 0) std::snprintf(buf, sizeof(buf), "%lldh %02lldm %02llds", h, m, s);
    else std::snprintf(buf, sizeof(buf), "%lldm %02llds", m, s);
    return buf;
}

static int count_lines(const std::string& text) {
    if (text.empty()) return 0;
    return (int)std::count(text.begin(), text.end(), '\n') + 1;
}

static std::string read_tail(const std::string& raw_path, size_t max_bytes) {
    std::string path = expand_path(raw_path);
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) return "Unable to open log: " + path;

    std::ifstream::pos_type end = in.tellg();
    if (end < 0) return "Unable to read log: " + path;

    size_t size = (size_t)end;
    size_t start = size > max_bytes ? size - max_bytes : 0;
    in.seekg((std::ifstream::pos_type)start, std::ios::beg);

    std::string out;
    out.resize(size - start);
    if (!out.empty()) in.read(&out[0], (std::streamsize)out.size());
    if (start > 0) out.insert(0, "[... earlier output trimmed ...]\n");
    return out;
}

static void trim_front(std::string& text, size_t max_bytes) {
    if (text.size() <= max_bytes) return;
    size_t remove = text.size() - max_bytes;
    size_t nl = text.find('\n', remove);
    if (nl != std::string::npos && nl + 1 < text.size()) remove = nl + 1;
    text.erase(0, remove);
    text.insert(0, "[... earlier output trimmed ...]\n");
}

static void append_output(SharedState& state, const std::string& line, size_t max_bytes) {
    std::lock_guard<std::mutex> guard(state.lock);
    state.command_output += line;
    trim_front(state.command_output, max_bytes);
}

static FILE* open_pipe(const std::string& command) {
#if defined(_WIN32)
    return _popen(command.c_str(), "r");
#else
    return popen(("exec 3>&-; " + command).c_str(), "r");
#endif
}

static int close_pipe(FILE* pipe) {
#if defined(_WIN32)
    return _pclose(pipe);
#else
    return pclose(pipe);
#endif
}

static std::string run_capture(const std::string& command, size_t max_bytes = 128 * 1024) {
    FILE* pipe = open_pipe(command);
    if (!pipe) return "Unable to run: " + command + "\n";

    std::string out;
    char buf[512];
    while (std::fgets(buf, sizeof(buf), pipe)) {
        out += buf;
        trim_front(out, max_bytes);
    }
    int code = close_pipe(pipe);
    if (code != 0 && out.empty()) {
        out = "Command exited with status " + std::to_string(code) + ": " + command + "\n";
    }
    return out;
}

static void command_worker(Options opts, SharedState& state) {
    const size_t max_bytes = (size_t)std::max(32, opts.max_output_kb) * 1024;
    state.started_at = Clock::now();
    state.command_started = true;
    state.command_done = false;
    {
        std::lock_guard<std::mutex> guard(state.lock);
        state.status_note = "running wrapped command";
        state.command_output = "$ " + opts.command + "\n";
    }

    FILE* pipe = open_pipe(opts.command + " 2>&1");
    if (!pipe) {
        append_output(state, "Unable to start command.\n", max_bytes);
        std::lock_guard<std::mutex> guard(state.lock);
        state.exit_code = -1;
        state.status_note = "failed to start";
        state.command_done = true;
        return;
    }

    char buf[1024];
    while (std::fgets(buf, sizeof(buf), pipe)) {
        append_output(state, buf, max_bytes);
    }

    int code = close_pipe(pipe);
    {
        std::lock_guard<std::mutex> guard(state.lock);
        state.exit_code = code;
        state.status_note = code == 0 ? "command completed" : "command exited non-zero";
        state.command_output += "\n[overwatch] command exit status: " + std::to_string(code) + "\n";
        trim_front(state.command_output, max_bytes);
    }
    state.command_done = true;
}

static void monitor_worker(Options opts, SharedState& state) {
    while (!state.stop_monitor) {
        std::string process_text;
        bool running = false;

        if (!opts.process_pattern.empty()) {
#if defined(_WIN32)
            std::string command = "tasklist";
            process_text = run_capture(command, 64 * 1024);
            running = process_text.find(opts.process_pattern) != std::string::npos;
#else
            std::string command =
                "pgrep -af -- " + shell_quote_posix(opts.process_pattern) +
                " | grep -Fv ftht_compile_overwatch | grep -Fv 'pgrep -af --'";
            process_text = run_capture(command, 64 * 1024);
            running = !process_text.empty() &&
                      process_text.find("Command exited with status") == std::string::npos;
#endif
        }

        std::string live_text;
        if (!opts.output_command.empty()) {
            live_text = run_capture(opts.output_command, 256 * 1024);
        }

        std::string probe_text;
        if (!opts.probe_command.empty()) {
            probe_text = run_capture(opts.probe_command, 128 * 1024);
        }

        {
            std::lock_guard<std::mutex> guard(state.lock);
            if (!opts.output_command.empty()) state.live_output = live_text;
            if (!opts.process_pattern.empty()) state.process_output = process_text;
            if (!opts.probe_command.empty()) state.probe_output = probe_text;
        }
        if (!opts.process_pattern.empty()) state.process_running = running;

        for (int i = 0; i < 20 && !state.stop_monitor; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

static void metric_card(const char* id, const char* title, const std::string& value,
                        const std::string& note, const char* klass = "") {
    std::string html =
        "<section id=\"" + html_escape(id ? id : "") + "\" class=\"ow-metric " +
        std::string(klass ? klass : "") + "\">"
        "<div>" + html_escape(title ? title : "") + "</div>"
        "<b>" + html_escape(value) + "</b>"
        "<small>" + html_escape(note) + "</small>"
        "</section>";
    ftht::html(html.c_str());
}

static void usage() {
    std::puts(
        "compile_overwatch [options] [-- command]\n"
        "\n"
        "Options:\n"
        "  --host ADDR              bind address, default 0.0.0.0\n"
        "  --port N                 HTTP port, default 8090\n"
        "  --title TEXT             panel title\n"
        "  --poll-ms N              browser refresh interval, default 1000\n"
        "  --estimate-minutes N     enable an estimate timer\n"
        "  --max-output-kb N        output buffer/tail size, default 512\n"
        "  --log FILE               passively tail a log file\n"
        "  --output-command COMMAND show this periodic command in the output pane\n"
        "  --process PATTERN        watch an existing process with pgrep -af\n"
        "  --probe COMMAND          periodically run a read-only status command\n"
        "  --wrap                   wrap long output lines\n"
        "  --cmd COMMAND            run and capture a command\n"
        "  --help                   show this text\n"
        "\n"
        "Examples:\n"
        "  compile_overwatch -- make -j8\n"
        "  compile_overwatch --log build.log --process 'make -j8'\n");
}

static Options parse_options(int argc, char** argv) {
    Options opts;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto need_value = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "%s needs a value\n", name);
                std::exit(2);
            }
            return argv[++i];
        };

        if (arg == "--") {
            opts.command = join_args(argc, argv, i + 1);
            break;
        } else if (arg == "--help" || arg == "-h") {
            usage();
            std::exit(0);
        } else if (arg == "--host") {
            opts.host = need_value("--host");
        } else if (arg == "--port") {
            if (!parse_int(need_value("--port"), &opts.port)) std::exit(2);
        } else if (arg == "--title") {
            opts.title = need_value("--title");
        } else if (arg == "--poll-ms") {
            if (!parse_int(need_value("--poll-ms"), &opts.poll_ms)) std::exit(2);
        } else if (arg == "--estimate-minutes") {
            if (!parse_double(need_value("--estimate-minutes"), &opts.estimate_minutes)) std::exit(2);
        } else if (arg == "--max-output-kb") {
            if (!parse_int(need_value("--max-output-kb"), &opts.max_output_kb)) std::exit(2);
        } else if (arg == "--log") {
            opts.log_path = need_value("--log");
        } else if (arg == "--output-command") {
            opts.output_command = need_value("--output-command");
        } else if (arg == "--process") {
            opts.process_pattern = need_value("--process");
        } else if (arg == "--probe") {
            opts.probe_command = need_value("--probe");
        } else if (arg == "--cmd") {
            opts.command = need_value("--cmd");
        } else if (arg == "--wrap") {
            opts.word_wrap = true;
        } else {
            std::fprintf(stderr, "Unknown argument: %s\n", arg.c_str());
            std::exit(2);
        }
    }
    return opts;
}

} // namespace

int main(int argc, char** argv) {
    Options opts = parse_options(argc, argv);

    ftht::Config cfg;
    cfg.title = opts.title.c_str();
    cfg.host = opts.host.c_str();
    cfg.port = opts.port;
    cfg.client_poll_ms = 0;
    cfg.auto_submit = false;
    cfg.dark_mode = true;
    std::string extra_head =
        "<style>"
        ".ow-grid{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:10px}"
        ".ow-metric{border:2px solid var(--border);background:var(--paper2);padding:10px;min-height:82px}"
        ".ow-metric div{font-size:12px;font-weight:900;text-transform:uppercase;color:var(--muted)}"
        ".ow-metric b{display:block;font-size:24px;line-height:1.05;margin-top:6px;overflow-wrap:anywhere}"
        ".ow-metric small{display:block;color:var(--muted);margin-top:8px}"
        ".ow-ok b{color:#68d391}.ow-warn b{color:#f6ad55}.ow-bad b{color:#fc8181}"
        ".ow-progress{height:13px;border:2px solid var(--border);background:var(--paper2);box-shadow:2px 2px 0 var(--border);overflow:hidden}"
        ".ow-progress>i{display:block;height:100%;background:var(--accent);width:0%}"
        ".ow-code{white-space:pre;overflow:auto;background:#0b0c0f;border:2px solid var(--border);border-radius:2px;margin:0;padding:10px;color:#d6deeb;box-shadow:inset 1px 0 0 rgba(0,0,0,.18);line-height:1.35;tab-size:4}"
        ".ow-code.ow-wrap{white-space:pre-wrap;overflow-wrap:anywhere}"
        ".ow-line{display:block;min-height:1.35em}"
        ".ow-line-error{background:rgba(252,129,129,.14);color:#ffd1d1}"
        ".ow-line-warning{background:rgba(246,173,85,.13);color:#ffe4ba}"
        ".ow-line-note{color:#b8d8ff}"
        ".ow-line-command{color:#c3e88d}"
        ".ow-line-phase{color:#82aaff}"
        ".ow-tok-error{color:#ff6b7a;font-weight:900}.ow-tok-warning{color:#ffc66d;font-weight:900}"
        ".ow-tok-note{color:#89ddff;font-weight:900}.ow-tok-phase{color:#82aaff;font-weight:900}"
        ".ow-tok-command{color:#c3e88d;font-weight:900}.ow-tok-path{color:#c792ea}"
        ".ow-toolbar{display:flex;gap:14px;align-items:center;flex-wrap:wrap;border:2px solid var(--border);background:var(--paper2);padding:8px 10px}"
        ".ow-toolbar label{display:flex;gap:8px;align-items:center;font-weight:900}.ow-toolbar input{width:18px;min-height:18px;accent-color:var(--accent)}"
        ".ow-note{font-size:12px;color:var(--muted);margin:0}"
        "@media(max-width:860px){.ow-grid{grid-template-columns:repeat(2,minmax(0,1fr))}}"
        "</style>";
    extra_head += "<script>document.addEventListener('DOMContentLoaded',()=>{const pollMs=";
    extra_head += std::to_string(std::max(250, opts.poll_ms));
    extra_head +=
        ";function replaceOuter(id,doc){const cur=document.getElementById(id);const next=doc.getElementById(id);if(cur&&next&&cur.outerHTML!==next.outerHTML)cur.outerHTML=next.outerHTML}"
        "async function tick(){try{const out=document.getElementById('ow-output');if(!out)return;const follow=document.getElementById('ow-follow');const wasBottom=(out.scrollHeight-out.scrollTop-out.clientHeight)<32;const oldTop=out.scrollTop;const res=await fetch(location.pathname+'?ow_fragment=1&ow_t='+Date.now(),{cache:'no-store'});const text=await res.text();const doc=new DOMParser().parseFromString(text,'text/html');['ow-state','ow-elapsed','ow-estimate','ow-size','ow-progress'].forEach(id=>replaceOuter(id,doc));const next=doc.getElementById('ow-output');if(!next)return;const html=next.innerHTML;if(html!==out.innerHTML){out.innerHTML=html;if(follow&&follow.checked&&wasBottom)out.scrollTop=out.scrollHeight;else out.scrollTop=oldTop}}catch(e){}}"
        "setTimeout(()=>{const out=document.getElementById('ow-output');if(out)out.scrollTop=out.scrollHeight},0);setInterval(tick,pollMs)});</script>";
    cfg.extra_head_html = extra_head.c_str();

    if (!ftht::create_server(cfg)) return 1;

    SharedState state;
    if (!opts.command.empty()) {
        std::thread(command_worker, opts, std::ref(state)).detach();
    }
    std::thread monitor;
    if (!opts.output_command.empty() || !opts.process_pattern.empty() || !opts.probe_command.empty()) {
        monitor = std::thread(monitor_worker, opts, std::ref(state));
    }

    bool show_estimate = opts.estimate_minutes > 0.0;
    float estimate_minutes = (float)std::max(0.0, opts.estimate_minutes);

    const Clock::time_point panel_started = Clock::now();
    while (ftht::pump()) {
        Clock::time_point started = state.command_started ? state.started_at : panel_started;
        long long elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - started).count();

        std::string command_output;
        std::string live_output;
        std::string process_output;
        std::string probe_output;
        std::string status_note;
        int exit_code = -1;
        {
            std::lock_guard<std::mutex> guard(state.lock);
            command_output = state.command_output;
            live_output = state.live_output;
            process_output = state.process_output;
            probe_output = state.probe_output;
            status_note = state.status_note;
            exit_code = state.exit_code;
        }

        std::string log_output;
        if (!opts.log_path.empty()) {
            log_output = read_tail(opts.log_path, (size_t)std::max(32, opts.max_output_kb) * 1024);
        }

        bool command_mode = !opts.command.empty();
        bool running = command_mode ? !state.command_done.load() :
                       (!opts.process_pattern.empty() ? state.process_running.load() : true);
        std::string state_text = running ? "running" : "stopped";
        const char* state_class = running ? "ow-ok" : "ow-warn";
        if (command_mode && state.command_done && exit_code != 0) {
            state_text = "failed";
            state_class = "ow-bad";
        } else if (command_mode && state.command_done) {
            state_text = "done";
            state_class = "ow-ok";
        }

        double estimate_s = std::max(0.0f, estimate_minutes) * 60.0;
        double progress = show_estimate && estimate_s > 0.0 ? std::min(1.0, elapsed_s / estimate_s) : 0.0;
        long long remaining_s = show_estimate && estimate_s > 0.0 ?
            (long long)std::max(0.0, estimate_s - elapsed_s) : 0;

        std::string visible_output = command_mode ? command_output :
                                     (!opts.output_command.empty() ? live_output : log_output);
        if (visible_output.empty()) {
            visible_output = "Waiting for output...\n";
        }

        int output_lines = count_lines(visible_output);
        int output_kb = (int)((visible_output.size() + 1023) / 1024);

        ftht::begin();

        ftht::html("<div class=\"ow-grid\">");
        metric_card("ow-state", "State", state_text, status_note, state_class);
        metric_card("ow-elapsed", "Elapsed", format_duration(elapsed_s), command_mode ? "command timer" : "panel timer");
        metric_card("ow-estimate", "Estimate", show_estimate ? format_duration((long long)estimate_s) : "off",
                    show_estimate ? ("remaining " + format_duration(remaining_s)) : "optional timer");
        metric_card("ow-size", "Output", std::to_string(output_lines) + " lines",
                    std::to_string(output_kb) + " KB visible");
        ftht::html("</div>");

        if (show_estimate && estimate_s > 0.0) {
            char progress_html[256];
            std::snprintf(progress_html, sizeof(progress_html),
                          "<div id=\"ow-progress\" class=\"ow-progress\" title=\"%.0f%%\"><i style=\"width:%.1f%%\"></i></div>",
                          progress * 100.0, progress * 100.0);
            ftht::html(progress_html);
        }

        ftht::html(
            "<div class=\"ow-toolbar\">"
            "<label><input id=\"ow-follow\" type=\"checkbox\" checked>Follow latest output</label>"
            "<span class=\"ow-note\">No word wrap. Use horizontal scroll for long compiler lines.</span>"
            "</div>");

        compiler_output_view("Compiler output", visible_output, 30, opts.word_wrap, false);

        std::string probe = "Process watch:\n";
        probe += opts.process_pattern.empty() ? "(not configured)\n" : process_output;
        probe += "\nProbe command:\n";
        probe += opts.probe_command.empty() ? "(not configured)\n" : probe_output;
        ftht::html("<details><summary>Probe / Status</summary>");
        ftht::log_view("Watch probe", probe.c_str(), 18,
                       ftht::LogViewFlags::WordWrap | ftht::LogViewFlags::AutoScrollBottom);
        ftht::html("</details>");

        std::ostringstream info;
        info << "Bind: " << opts.host << ":" << opts.port << "\n";
        info << "Poll: " << opts.poll_ms << " ms\n";
        info << "Command: " << (opts.command.empty() ? "(none)" : opts.command) << "\n";
        info << "Log: " << (opts.log_path.empty() ? "(none)" : opts.log_path) << "\n";
        info << "Output command: " << (opts.output_command.empty() ? "(none)" : opts.output_command) << "\n";
        info << "Process pattern: " << (opts.process_pattern.empty() ? "(none)" : opts.process_pattern) << "\n";
        info << "Probe: " << (opts.probe_command.empty() ? "(none)" : opts.probe_command) << "\n\n";
        info << "Future compile example:\n";
        info << "  compile_overwatch --estimate-minutes 20 -- make -j8\n\n";
        info << "Passive watch example:\n";
        info << "  compile_overwatch --log build.log --process 'make -j8'\n";
        ftht::html("<details><summary>Panel Configuration</summary>");
        ftht::log_view("Panel configuration", info.str().c_str(), 14, ftht::LogViewFlags::WordWrap);
        ftht::html("</details>");

        ftht::end();
    }

    state.stop_monitor = true;
    if (monitor.joinable()) monitor.join();
    ftht::shutdown();
    return 0;
}
