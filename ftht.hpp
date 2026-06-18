// ftht.hpp - Fuck This Hyper Text
// Single-header immediate-mode HTML UI for tiny devices and desktop tools.
//
// Usage: #define FTHT_IMPLEMENTATION in exactly one .cpp before including.
//
// Windows: cl main.cpp /std:c++17
//          (MSVC auto-links ws2_32.lib)
// Linux:   g++ main.cpp -o app -DFTHT_IMPLEMENTATION -std=c++17
// ESP32 Arduino: include after WiFi is connected, then call ftht::create_server().

#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <cstdarg>
#include <ctime>
#include <string>
#include <string_view>
#include <vector>

#ifndef FTHT_VERSION_MAJOR
#define FTHT_VERSION_MAJOR 0
#endif
#ifndef FTHT_VERSION_MINOR
#define FTHT_VERSION_MINOR 1
#endif
#ifndef FTHT_VERSION_PATCH
#define FTHT_VERSION_PATCH 0
#endif

namespace ftht {

enum class Align { Start, Center, End };

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct Style {
    Color background    = {0.055f, 0.055f, 0.059f, 1.0f};
    Color panel         = {0.090f, 0.094f, 0.102f, 1.0f};
    Color text          = {0.910f, 0.910f, 0.910f, 1.0f};
    Color text_dim      = {0.663f, 0.678f, 0.702f, 1.0f};
    Color border        = {0.169f, 0.176f, 0.192f, 1.0f};
    Color button        = {0.090f, 0.094f, 0.102f, 1.0f};
    Color button_hover  = {0.137f, 0.149f, 0.169f, 1.0f};
    Color button_active = {0.176f, 0.192f, 0.220f, 1.0f};
    Color input_bg      = {0.071f, 0.075f, 0.082f, 1.0f};
    Color input_focus   = {0.310f, 0.420f, 0.780f, 1.0f};
    Color accent        = {0.310f, 0.420f, 0.780f, 1.0f};
    Color warning       = {0.894f, 0.486f, 0.353f, 1.0f};
    Color success       = {0.420f, 0.741f, 0.522f, 1.0f};
    float window_padding = 20.0f;
    float item_spacing   = 10.0f;
    float item_height    = 36.0f;
    float rounding       = 8.0f;
    float border_width   = 1.0f;
    float font_size      = 16.0f;
};

enum class ColorRole {
    Background,
    Panel,
    Text,
    TextDim,
    Border,
    Button,
    ButtonHover,
    ButtonActive,
    InputBg,
    InputFocus,
    Accent,
    Warning,
    Success,
};

enum class LoginMode {
    None,
    DrawPenis,
    Form,
};

enum class ToastType {
    Info,
    Success,
    Warning,
    Error,
};

struct Toast {
    std::string title;
    std::string message;
    int duration_ms = 4000;
    bool dismissible = false;
    ToastType type = ToastType::Info;
};

Style default_dark_style();
Style one_dark_style();
void         set_style(const Style& s);
const Style& get_style();
Color        color_from_hex(const char* hex);
Color        color_from_hex(std::string_view hex);

struct Config {
    const char* title = "FTHT App";
    const char* host = "0.0.0.0";
    int         port = 8080;
    int         max_request_bytes = 32 * 1024;
    int         read_timeout_ms = 1500;
    int         client_poll_ms = 0;
    bool        auto_submit = true;
    bool        print_url = true;
    bool        debug_output = false;
    bool        client_debug_output = false;
    bool        dark_mode = false;
    bool        respect_browser_dark_mode = false;
    const char* extra_head_html = nullptr;
    LoginMode   login_mode = LoginMode::None;
    const char* login_username = "admin";
    const char* login_password = nullptr;
    const char* login_salt = "ftht-login";
    const char* login_db_path = nullptr;
    uint64_t    login_password_hash = 0;
    int         login_session_seconds = 10 * 60;
    bool        login_allow_registration = false;
};

bool create_server(const Config& cfg = {});
bool pump(int timeout_ms = -1);
void begin();
void end();
void shutdown();
uint64_t make_login_password_hash(const char* password, const char* salt = "ftht-login");

const Config& config();
const char* url();
const char* path();
const char* method();
bool is_post();
const char* param(const char* name);
void set_status(int code, const char* text);

void html(const char* markup);
void text(const char* label);
void text_wrapped(const char* text);
void separator();
void spacing(float px = 8.0f);
void toast(const char* message);
void toast(const Toast& toast);
void toast_success(const char* message);
void toast_warning(const char* message);
void toast_error(const char* message);

enum class InputFlags : unsigned {
    Default = 0,
    Password = 1 << 0,
    ReadOnly = 1 << 1,
    CharsDecimal = 1 << 2,
    CharsHexadecimal = 1 << 3,
    CharsUppercase = 1 << 4,
    CharsNoBlank = 1 << 5,
};
inline InputFlags operator|(InputFlags a, InputFlags b) {
    return static_cast<InputFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline bool operator&(InputFlags a, InputFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

enum class TextAreaFlags : unsigned {
    Default = 0,
    ReadOnly = 1 << 0,
    WordWrap = 1 << 1,
    AutoScrollBottom = 1 << 2,
};
inline TextAreaFlags operator|(TextAreaFlags a, TextAreaFlags b) {
    return static_cast<TextAreaFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline bool operator&(TextAreaFlags a, TextAreaFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

enum class LogViewFlags : unsigned {
    Default = 0,
    WordWrap = 1 << 0,
    AutoScrollBottom = 1 << 1,
};
inline LogViewFlags operator|(LogViewFlags a, LogViewFlags b) {
    return static_cast<LogViewFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline bool operator&(LogViewFlags a, LogViewFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

bool input(const char* label, char* buffer, int buffer_size,
           InputFlags flags = InputFlags::Default, bool* enter_pressed = nullptr);
bool text_area(const char* label, char* buffer, int buffer_size, int rows = 5);
bool text_area_ex(const char* label, char* buffer, int buffer_size, int rows = 5,
                  TextAreaFlags flags = TextAreaFlags::Default);
void log_view(const char* label, const char* text, int rows = 8,
              LogViewFlags flags = LogViewFlags::AutoScrollBottom);

bool checkbox(const char* label, bool* value);
bool slider_float(const char* label, float* value, float min_v, float max_v);
bool button(const char* label);
bool button(const char* label, ColorRole role);
bool button(const char* label, Color color);
bool dropdown(const char* label, const char* const* items, int count, int* selected, int popup_rows = 8);
bool listbox(const char* label, const char* const* items, int count, int* selected, int visible_rows = 6);
bool radio_group(const char* label, const char* const* items, int count, int* selected, int columns = 1);
bool collapsing_header(const char* label, bool* open = nullptr);
bool tabs(const char* const* labels, int count, int* selected);

void row(int cols, std::function<void()> fn);
void row(std::initializer_list<float> weights, std::function<void()> fn);
void scroll_area(const char* label, float height, std::function<void()> fn);
void set_next_width(float px);
void set_next_fill();
void set_next_percent(float pct);
void set_next_limits(float min_px, float max_px);
void set_next_align(Align align);
void open_modal(const char* label);
bool modal(const char* label, std::function<void()> fn);
void close_modal();
void begin_disabled();
void end_disabled();
void tooltip(const char* text);
void request_focus(const char* label);
float calc_text_width(const char* text);
float calc_text_height(const char* text, float wrap_width);

} // namespace ftht

#ifdef FTHT_IMPLEMENTATION

#if defined(ARDUINO) && defined(ESP32)
#include <Arduino.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#elif defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

namespace ftht {
namespace internal {

struct Param {
    std::string key;
    std::string value;
};

struct ActiveToast {
    uint64_t id = 0;
    Toast toast;
    long long created_ms = 0;
};

struct NextLayoutState {
    bool has_width = false;
    bool fill = false;
    bool has_percent = false;
    bool has_limits = false;
    bool has_align = false;
    float width = 0.0f;
    float percent = 1.0f;
    float min_width = 0.0f;
    float max_width = 0.0f;
    Align align = Align::Start;
};

#if defined(ARDUINO) && defined(ESP32)
struct ServerHandle {
    WiFiServer* server = nullptr;
    WiFiClient client;
};
#elif defined(_WIN32)
using Socket = SOCKET;
static constexpr Socket invalid_socket = INVALID_SOCKET;
struct ServerHandle {
    Socket server = invalid_socket;
    Socket client = invalid_socket;
    bool wsa_started = false;
};
#else
using Socket = int;
static constexpr Socket invalid_socket = -1;
struct ServerHandle {
    Socket server = invalid_socket;
    Socket client = invalid_socket;
};
#endif

struct Context {
    Config cfg;
    Style style = one_dark_style();
    ServerHandle net;
    std::string current_url;
    std::string req_method;
    std::string req_path;
    std::string req_query;
    std::string req_body;
    std::string req_headers;
    std::string req_cookie;
    std::vector<Param> params;
    std::string out;
    std::string extra_response_headers;
    std::string login_session_token;
    std::time_t login_session_until = 0;
    std::vector<ActiveToast> toasts;
    uint64_t next_toast_id = 1;
    std::string modal_label;
    std::string last_widget_id;
    int status_code = 200;
    std::string status_text = "OK";
    int disabled_depth = 0;
    bool request_active = false;
    bool response_open = false;
    bool modal_rendered = false;
    bool client_update = false;
    NextLayoutState next;
};

static Context& ctx() {
    static Context c;
    return c;
}

static void send_response(const std::string& body);
static void append_css();
static void append_ink_filters();
static void append_toast_stack();
static void append_toast_script();

static void debugf(const char* fmt, ...) {
    Context& c = ctx();
    if (!c.cfg.debug_output) return;
#if defined(ARDUINO) && defined(ESP32)
    char buf[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Serial.print("[ftht] ");
    Serial.println(buf);
#else
    std::fprintf(stderr, "[ftht] ");
    va_list args;
    va_start(args, fmt);
    std::vfprintf(stderr, fmt, args);
    va_end(args);
    std::fprintf(stderr, "\n");
    std::fflush(stderr);
#endif
}

#if !defined(ARDUINO) || !defined(ESP32)
static int last_net_error() {
#if defined(_WIN32)
    return WSAGetLastError();
#else
    return errno;
#endif
}
#endif

static std::string visible_label(const char* label) {
    if (!label) return {};
    const char* p = std::strstr(label, "##");
    return p ? std::string(label, p) : std::string(label);
}

static uint32_t hash_label(const char* text) {
    const unsigned char* s = reinterpret_cast<const unsigned char*>(text ? text : "");
    uint32_t h = 2166136261u;
    while (*s) {
        h ^= (uint32_t)*s++;
        h *= 16777619u;
    }
    return h;
}

static std::string widget_id(const char* label, const char* salt = "") {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "w_%08x", hash_label((std::string(label ? label : "") + salt).c_str()));
    return std::string(buf);
}

static void append(std::string& s, const char* text) {
    if (text) s += text;
}

static std::string escape_html(std::string_view in) {
    std::string out;
    out.reserve(in.size());
    for (char ch : in) {
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

static int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static std::string url_decode(std::string_view in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '+') {
            out += ' ';
        } else if (c == '%' && i + 2 < in.size()) {
            int hi = hex_digit(in[i + 1]);
            int lo = hex_digit(in[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out += (char)((hi << 4) | lo);
                i += 2;
            } else {
                out += c;
            }
        } else {
            out += c;
        }
    }
    return out;
}

static bool contains_header_token(std::string_view headers, const char* token) {
    std::string haystack(headers);
    std::string needle(token ? token : "");
    for (char& ch : haystack) ch = (char)std::tolower((unsigned char)ch);
    for (char& ch : needle) ch = (char)std::tolower((unsigned char)ch);
    return !needle.empty() && haystack.find(needle) != std::string::npos;
}

static std::string trim_copy(std::string_view in) {
    while (!in.empty() && std::isspace((unsigned char)in.front())) in.remove_prefix(1);
    while (!in.empty() && std::isspace((unsigned char)in.back())) in.remove_suffix(1);
    return std::string(in);
}

static std::string lower_copy(std::string_view in) {
    std::string out(in);
    for (char& ch : out) ch = (char)std::tolower((unsigned char)ch);
    return out;
}

static std::string header_value(std::string_view headers, const char* name) {
    std::string wanted = lower_copy(name ? name : "");
    if (wanted.empty()) return {};
    size_t pos = 0;
    while (pos < headers.size()) {
        size_t end = headers.find("\r\n", pos);
        if (end == std::string_view::npos) end = headers.size();
        std::string_view line = headers.substr(pos, end - pos);
        size_t colon = line.find(':');
        if (colon != std::string_view::npos) {
            std::string key = lower_copy(line.substr(0, colon));
            if (key == wanted) return trim_copy(line.substr(colon + 1));
        }
        if (end == headers.size()) break;
        pos = end + 2;
    }
    return {};
}

static std::string cookie_value(const std::string& cookie, const char* name) {
    std::string wanted = name ? name : "";
    size_t pos = 0;
    while (pos <= cookie.size()) {
        size_t semi = cookie.find(';', pos);
        if (semi == std::string::npos) semi = cookie.size();
        std::string part = trim_copy(std::string_view(cookie).substr(pos, semi - pos));
        size_t eq = part.find('=');
        if (eq != std::string::npos && part.substr(0, eq) == wanted) return part.substr(eq + 1);
        if (semi == cookie.size()) break;
        pos = semi + 1;
    }
    return {};
}

static void add_param(std::string_view key, std::string_view value) {
    Param p;
    p.key = url_decode(key);
    p.value = url_decode(value);
    for (Param& existing : ctx().params) {
        if (existing.key == p.key) {
            existing.value = p.value;
            return;
        }
    }
    ctx().params.push_back(p);
}

static void parse_params(std::string_view data) {
    size_t pos = 0;
    while (pos <= data.size()) {
        size_t amp = data.find('&', pos);
        if (amp == std::string_view::npos) amp = data.size();
        std::string_view pair = data.substr(pos, amp - pos);
        if (!pair.empty()) {
            size_t eq = pair.find('=');
            if (eq == std::string_view::npos) {
                add_param(pair, "");
            } else {
                add_param(pair.substr(0, eq), pair.substr(eq + 1));
            }
        }
        if (amp == data.size()) break;
        pos = amp + 1;
    }
}

static std::string color_css(Color c) {
    int r = std::max(0, std::min(255, (int)(c.r * 255.0f + 0.5f)));
    int g = std::max(0, std::min(255, (int)(c.g * 255.0f + 0.5f)));
    int b = std::max(0, std::min(255, (int)(c.b * 255.0f + 0.5f)));
    char buf[32];
    std::snprintf(buf, sizeof(buf), "#%02x%02x%02x", r, g, b);
    return std::string(buf);
}

static uint64_t hash_bytes64(std::string_view value) {
    const unsigned char* data = reinterpret_cast<const unsigned char*>(value.data());
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < value.size(); ++i) {
        h ^= (uint64_t)data[i];
        h *= 1099511628211ull;
    }
    return h;
}

static uint64_t salted_password_hash(std::string_view password, std::string_view salt) {
    std::string material(salt);
    material += ":";
    material += password;
    return hash_bytes64(material);
}

static long long now_ms() {
    return (long long)std::time(nullptr) * 1000ll;
}

static std::string hex64(uint64_t value) {
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%016llx", (unsigned long long)value);
    return std::string(buf);
}

static uint64_t parse_hex64(std::string_view value) {
    uint64_t out = 0;
    for (char c : value) {
        int d = hex_digit(c);
        if (d < 0) break;
        out = (out << 4) | (uint64_t)d;
    }
    return out;
}

static std::string sql_quote(std::string_view in) {
    std::string out = "'";
    for (char ch : in) {
        if (ch == '\'') out += "''";
        else out += ch;
    }
    out += "'";
    return out;
}

static FILE* open_file(const char* path, const char* mode) {
    if (!path || !mode) return nullptr;
#if defined(_WIN32)
    FILE* f = nullptr;
    return fopen_s(&f, path, mode) == 0 ? f : nullptr;
#else
    return std::fopen(path, mode);
#endif
}

static bool read_sql_quoted(const std::string& text, size_t& pos, std::string& out) {
    out.clear();
    if (pos >= text.size() || text[pos] != '\'') return false;
    ++pos;
    while (pos < text.size()) {
        char ch = text[pos++];
        if (ch == '\'') {
            if (pos < text.size() && text[pos] == '\'') {
                out += '\'';
                ++pos;
                continue;
            }
            return true;
        }
        out += ch;
    }
    return false;
}

struct LoginCredential {
    std::string username;
    std::string salt;
    uint64_t hash = 0;
    bool ok = false;
};

static const char* toast_type_class(ToastType type) {
    switch (type) {
        case ToastType::Success: return "success";
        case ToastType::Warning: return "warning";
        case ToastType::Error: return "error";
        case ToastType::Info:
        default: return "info";
    }
}

static const char* toast_default_title(ToastType type) {
    switch (type) {
        case ToastType::Success: return "Success";
        case ToastType::Warning: return "Warning";
        case ToastType::Error: return "Error";
        case ToastType::Info:
        default: return "";
    }
}

static void prune_toasts() {
    Context& c = ctx();
    long long now = now_ms();
    c.toasts.erase(std::remove_if(c.toasts.begin(), c.toasts.end(), [&](const ActiveToast& item) {
        int duration = std::max(1000, item.toast.duration_ms);
        return now - item.created_ms > (long long)duration + 8000ll;
    }), c.toasts.end());
    while (c.toasts.size() > 32) c.toasts.erase(c.toasts.begin());
}

static void queue_toast(const Toast& value) {
    Context& c = ctx();
    Toast copy = value;
    if (copy.title.empty() && copy.message.empty()) return;
    if (copy.duration_ms <= 0) copy.duration_ms = 4000;
    if (copy.duration_ms < 1000) copy.duration_ms = 1000;
    ActiveToast item;
    item.id = c.next_toast_id++;
    item.toast = copy;
    item.created_ms = now_ms();
    c.toasts.push_back(item);
    prune_toasts();
    debugf("toast queued id=%llu type=%s title='%s' message_bytes=%d",
           (unsigned long long)item.id, toast_type_class(item.toast.type),
           item.toast.title.c_str(), (int)item.toast.message.size());
}

static void queue_toast(ToastType type, const char* title, const char* message,
                        int duration_ms = 4000, bool dismissible = false) {
    Toast t;
    t.type = type;
    t.title = title ? title : "";
    t.message = message ? message : "";
    t.duration_ms = duration_ms;
    t.dismissible = dismissible;
    queue_toast(t);
}

static const char* random_draw_success_message() {
    static const char* messages[] = {
        "Humanity confirmed. Shape classified as penis. A penis is the only accepted password.",
        "Authentication successful. Password confirmed to be a penis.",
        "Access granted. Required penis detected.",
        "Human verified. Shape quality acceptable.",
        "Login successful. The penis remains the correct password.",
        "Authentication complete. The system continues to be disappointed in humanity.",
        "Login successful. Researchers remain uncertain why users keep drawing that. So they just set it as the only password.",
    };
    int count = (int)(sizeof(messages) / sizeof(messages[0]));
    return messages[std::rand() % count];
}

static std::string generated_salt() {
    Context& c = ctx();
    std::string seed = std::to_string((long long)std::time(nullptr));
    seed += ":";
    seed += std::to_string((unsigned long long)std::rand());
    seed += ":";
    seed += c.cfg.title ? c.cfg.title : "FTHT App";
    seed += ":";
    seed += c.current_url;
    return hex64(hash_bytes64(seed));
}

static bool load_login_db(std::vector<LoginCredential>& creds) {
    Context& c = ctx();
    if (!c.cfg.login_db_path || !*c.cfg.login_db_path) return false;
    FILE* f = open_file(c.cfg.login_db_path, "rb");
    if (!f) return false;
    std::string text;
    char buf[512];
    size_t n = 0;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) text.append(buf, n);
    std::fclose(f);

    const char* prefix = "INSERT INTO users VALUES(";
    size_t search = 0;
    while (true) {
        size_t pos = text.find(prefix, search);
        if (pos == std::string::npos) break;
        search = pos + std::strlen(prefix);
        pos = search;
        std::string user, salt, hash_text;
        if (!read_sql_quoted(text, pos, user)) continue;
        if (pos >= text.size() || text[pos++] != ',') continue;
        if (!read_sql_quoted(text, pos, salt)) continue;
        if (pos >= text.size() || text[pos++] != ',') continue;
        if (!read_sql_quoted(text, pos, hash_text)) continue;

        LoginCredential cred;
        cred.username = user;
        cred.salt = salt;
        cred.hash = parse_hex64(hash_text);
        cred.ok = !cred.username.empty() && !cred.salt.empty() && cred.hash != 0;
        if (cred.ok) creds.push_back(cred);
    }
    return !creds.empty();
}

static bool save_login_db(const std::vector<LoginCredential>& creds) {
    Context& c = ctx();
    if (!c.cfg.login_db_path || !*c.cfg.login_db_path || creds.empty()) return false;
    FILE* f = open_file(c.cfg.login_db_path, "wb");
    if (!f) return false;
    std::string text;
    text += "-- FTHT LiteSQL auth store\n";
    text += "CREATE TABLE users(username TEXT, salt TEXT, password_hash TEXT);\n";
    for (const LoginCredential& cred : creds) {
        if (!cred.ok) continue;
        text += "INSERT INTO users VALUES(";
        text += sql_quote(cred.username);
        text += ",";
        text += sql_quote(cred.salt);
        text += ",";
        text += sql_quote(hex64(cred.hash));
        text += ");\n";
    }
    std::fwrite(text.data(), 1, text.size(), f);
    std::fclose(f);
    return true;
}

static std::vector<LoginCredential> configured_login_credentials() {
    Context& c = ctx();
    std::vector<LoginCredential> creds;
    if (load_login_db(creds)) return creds;

    LoginCredential cred;

    cred.username = c.cfg.login_username && *c.cfg.login_username ? c.cfg.login_username : "admin";
    cred.salt = c.cfg.login_salt && *c.cfg.login_salt ? c.cfg.login_salt : generated_salt();
    if (c.cfg.login_password_hash != 0) {
        cred.hash = c.cfg.login_password_hash;
    } else if (c.cfg.login_password) {
        cred.hash = salted_password_hash(c.cfg.login_password, cred.salt);
    }
    cred.ok = !cred.username.empty() && !cred.salt.empty() && cred.hash != 0;
    if (cred.ok) {
        creds.push_back(cred);
        save_login_db(creds);
    }
    return creds;
}

static bool find_login_credential(const char* username, LoginCredential& out) {
    if (!username || !*username) return false;
    std::vector<LoginCredential> creds = configured_login_credentials();
    for (const LoginCredential& cred : creds) {
        if (cred.ok && cred.username == username) {
            out = cred;
            return true;
        }
    }
    return false;
}

static bool login_username_exists(const std::vector<LoginCredential>& creds, const char* username) {
    if (!username || !*username) return false;
    for (const LoginCredential& cred : creds) {
        if (cred.ok && cred.username == username) return true;
    }
    return false;
}

static bool prompt_accept_registration(const char* username) {
    std::printf("\n[ftht] Registration request for username '%s'. Accept? [y/N]: ", username ? username : "");
    std::fflush(stdout);
    char answer[32] = {};
    if (!std::fgets(answer, sizeof(answer), stdin)) return false;
    return answer[0] == 'y' || answer[0] == 'Y';
}

static bool register_login_user(const char* username, const char* password, std::string& message) {
    Context& c = ctx();
    if (!c.cfg.login_allow_registration) {
        message = "Registration is not enabled on this server.";
        queue_toast(ToastType::Warning, "Registration Unavailable", message.c_str());
        return false;
    }
    if (!c.cfg.login_db_path || !*c.cfg.login_db_path) {
        message = "Registration requires login_db_path so accepted accounts can be saved.";
        queue_toast(ToastType::Error, "Registration Failed", message.c_str(), 5000, true);
        return false;
    }
    if (!username || !*username || !password || !*password) {
        message = "Choose a username and password.";
        queue_toast(ToastType::Warning, "Validation Error", message.c_str());
        return false;
    }
    std::vector<LoginCredential> creds = configured_login_credentials();
    if (login_username_exists(creds, username)) {
        message = "That username already exists.";
        queue_toast(ToastType::Warning, "Validation Error", message.c_str());
        return false;
    }
    if (!prompt_accept_registration(username)) {
        message = "Registration was rejected by the server operator.";
        queue_toast(ToastType::Warning, "Registration Rejected", message.c_str(), 5000, true);
        return false;
    }

    LoginCredential cred;
    cred.username = username;
    cred.salt = generated_salt();
    cred.hash = salted_password_hash(password, cred.salt);
    cred.ok = !cred.username.empty() && !cred.salt.empty() && cred.hash != 0;
    creds.push_back(cred);
    if (!save_login_db(creds)) {
        message = "Could not save the accepted registration.";
        queue_toast(ToastType::Error, "Save Failed", message.c_str(), 5000, true);
        return false;
    }
    message = "Registration accepted.";
    queue_toast(ToastType::Success, "Registration Saved", "Accepted account saved to the login store.");
    return true;
}

static std::string new_session_token() {
    Context& c = ctx();
    std::string seed = std::to_string((long long)std::time(nullptr));
    seed += ":";
    seed += std::to_string((unsigned long long)std::rand());
    seed += ":";
    seed += c.req_cookie;
    seed += ":";
    seed += c.current_url;
    return hex64(hash_bytes64(seed));
}

static void start_login_session() {
    Context& c = ctx();
    c.login_session_token = new_session_token();
    c.login_session_until = std::time(nullptr) + std::max(60, c.cfg.login_session_seconds);
    c.extra_response_headers += "Set-Cookie: ftht_session=" + c.login_session_token + "; Path=/; HttpOnly; SameSite=Lax\r\n";
}

static bool has_login_session() {
    Context& c = ctx();
    if (c.cfg.login_mode == LoginMode::None) return true;
    std::string token = cookie_value(c.req_cookie, "ftht_session");
    std::time_t now = std::time(nullptr);
    if (!token.empty() && token == c.login_session_token && c.login_session_until > now) {
        c.login_session_until = now + std::max(60, c.cfg.login_session_seconds);
        c.extra_response_headers += "Set-Cookie: ftht_session=" + c.login_session_token + "; Path=/; HttpOnly; SameSite=Lax\r\n";
        return true;
    }
    return false;
}

static void append_auth_page_start(const char* title) {
    Context& c = ctx();
    c.status_code = 200;
    c.status_text = "OK";
    c.out.clear();
    c.out += "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">";
    c.out += "<title>" + escape_html(title ? title : "Login") + "</title>";
    append_css();
    c.out += "</head><body>";
    append_ink_filters();
    c.out += "<main><div class=\"ft-bleed\"></div><form method=\"post\" action=\"/\">";
    c.out += "<h1 class=\"ft-title\">" + escape_html(title ? title : "Login") + "</h1>";
}

static void append_auth_page_end() {
    Context& c = ctx();
    append_toast_stack();
    c.out += "</form></main>";
    append_toast_script();
    c.out += "</body></html>";
}

static void send_auth_page(const std::string& body) {
    send_response(body);
}

static void send_redirect_home() {
    Context& c = ctx();
    c.status_code = 303;
    c.status_text = "See Other";
    c.extra_response_headers += "Location: /\r\n";
    send_response("<!doctype html><html><body>Logged in.</body></html>");
}

struct DrawShape {
    char type = 0;
    float x = 0;
    float y = 0;
    float w = 0;
    float h = 0;
};

static std::vector<DrawShape> parse_draw_shapes(const char* text) {
    std::vector<DrawShape> shapes;
    if (!text) return shapes;
    const char* p = text;
    while (*p) {
        DrawShape s;
        while (std::isspace((unsigned char)*p) || *p == ';') ++p;
        if (!*p) break;
        s.type = *p++;
        if (*p++ != ',') break;
        char* end = nullptr;
        s.x = std::strtof(p, &end);
        if (end == p || *end++ != ',') break;
        p = end;
        s.y = std::strtof(p, &end);
        if (end == p || *end++ != ',') break;
        p = end;
        s.w = std::strtof(p, &end);
        if (end == p || *end++ != ',') break;
        p = end;
        s.h = std::strtof(p, &end);
        if (end == p) break;
        p = end;
        if ((s.type == 'e' || s.type == 'r') && s.w > 0 && s.h > 0) shapes.push_back(s);
    }
    return shapes;
}

static float shape_cx(const DrawShape& s) { return s.x + s.w * 0.5f; }
static float shape_cy(const DrawShape& s) { return s.y + s.h * 0.5f; }
static float shape_major(const DrawShape& s) { return std::max(s.w, s.h); }
static float shape_minor(const DrawShape& s) { return std::max(1.0f, std::min(s.w, s.h)); }

static bool is_ball_shape(const DrawShape& s) {
    if (s.type != 'e') return false;
    float major = shape_major(s);
    float minor = shape_minor(s);
    return major >= 28.0f && major <= 130.0f && major / minor <= 1.65f;
}

static bool is_shaft_shape(const DrawShape& s) {
    float major = shape_major(s);
    float minor = shape_minor(s);
    return major >= 110.0f && major / minor >= 2.2f;
}

static bool drawing_is_penis(const char* text) {
    std::vector<DrawShape> shapes = parse_draw_shapes(text);
    for (size_t i = 0; i < shapes.size(); ++i) {
        if (!is_ball_shape(shapes[i])) continue;
        float cx1 = shapes[i].x + shapes[i].w * 0.5f;
        float cy1 = shapes[i].y + shapes[i].h * 0.5f;
        for (size_t j = i + 1; j < shapes.size(); ++j) {
            if (!is_ball_shape(shapes[j])) continue;
            float cx2 = shapes[j].x + shapes[j].w * 0.5f;
            float cy2 = shapes[j].y + shapes[j].h * 0.5f;
            float dx = cx1 > cx2 ? cx1 - cx2 : cx2 - cx1;
            float dy = cy1 > cy2 ? cy1 - cy2 : cy2 - cy1;
            float y_tol = std::max(24.0f, std::min(shapes[i].h, shapes[j].h) * 0.65f);
            float x_tol = std::max(24.0f, std::min(shapes[i].w, shapes[j].w) * 0.65f);
            float min_dx = std::min(shapes[i].w, shapes[j].w) * 0.45f;
            float max_dx = std::max(95.0f, (shapes[i].w + shapes[j].w) * 1.15f);
            float min_dy = std::min(shapes[i].h, shapes[j].h) * 0.45f;
            float max_dy = std::max(95.0f, (shapes[i].h + shapes[j].h) * 1.15f);
            bool horizontal_pair = dy <= y_tol && dx >= min_dx && dx <= max_dx;
            bool vertical_pair = dx <= x_tol && dy >= min_dy && dy <= max_dy;
            if (!horizontal_pair && !vertical_pair) continue;

            float pair_cx = (cx1 + cx2) * 0.5f;
            float pair_cy = (cy1 + cy2) * 0.5f;
            float pair_width = dx + (shapes[i].w + shapes[j].w) * 0.5f;
            float pair_height = std::max(shapes[i].h, shapes[j].h);
            if (vertical_pair) {
                pair_width = std::max(shapes[i].w, shapes[j].w);
                pair_height = dy + (shapes[i].h + shapes[j].h) * 0.5f;
            }
            float pair_left = pair_cx - pair_width * 0.5f;
            float pair_right = pair_cx + pair_width * 0.5f;
            float pair_top = pair_cy - pair_height * 0.5f;
            float pair_bottom = pair_cy + pair_height * 0.5f;

            for (size_t k = 0; k < shapes.size(); ++k) {
                if (k == i || k == j || !is_shaft_shape(shapes[k])) continue;
                const DrawShape& shaft = shapes[k];
                float scx = shape_cx(shaft);
                float scy = shape_cy(shaft);
                bool vertical = shaft.h >= shaft.w;
                if (horizontal_pair && vertical) {
                    float center_tol = std::max(34.0f, pair_width * 0.45f);
                    bool x_aligned = std::abs(scx - pair_cx) <= center_tol;
                    bool end_near_pair =
                        std::abs((shaft.y + shaft.h) - pair_top) <= std::max(44.0f, pair_height * 0.75f) ||
                        std::abs(shaft.y - pair_bottom) <= std::max(44.0f, pair_height * 0.75f);
                    bool reaches_pair_row = shaft.y <= pair_bottom && (shaft.y + shaft.h) >= pair_top;
                    if (x_aligned && (end_near_pair || reaches_pair_row)) return true;
                } else if (vertical_pair && !vertical) {
                    float center_tol = std::max(34.0f, pair_height * 0.85f);
                    bool y_aligned = std::abs(scy - pair_cy) <= center_tol;
                    bool end_near_pair =
                        std::abs((shaft.x + shaft.w) - pair_left) <= std::max(44.0f, pair_width * 0.75f) ||
                        std::abs(shaft.x - pair_right) <= std::max(44.0f, pair_width * 0.75f);
                    bool reaches_pair_col = shaft.x <= pair_right && (shaft.x + shaft.w) >= pair_left;
                    if (y_aligned && (end_near_pair || reaches_pair_col)) return true;
                }
            }
        }
    }
    return false;
}

static const char* builtin_favicon_svg() {
    return
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 4961 4961\">"
        "<rect x=\"553\" y=\"553\" width=\"3855\" height=\"3855\" fill=\"#000\"/>"
        "<path d=\"M2480 553 4408 4408H553L2480 553Z\" fill=\"#fff\"/>"
        "<path d=\"M553 553 4408 4408\" fill=\"none\" stroke=\"#ff8200\" stroke-width=\"160\" stroke-linecap=\"round\"/>"
        "<rect x=\"553\" y=\"553\" width=\"3855\" height=\"3855\" fill=\"none\" stroke=\"#ff8200\" stroke-width=\"120\"/>"
        "</svg>";
}

static const char* find_param(const char* name) {
    if (!name) return nullptr;
    for (const Param& p : ctx().params) {
        if (p.key == name) return p.value.c_str();
    }
    return nullptr;
}

static void copy_to_buffer(char* buffer, int buffer_size, const std::string& value) {
    if (!buffer || buffer_size <= 0) return;
    int n = (int)std::min<size_t>((size_t)buffer_size - 1, value.size());
    if (n > 0) std::memcpy(buffer, value.data(), (size_t)n);
    buffer[n] = '\0';
}

static std::string disabled_attr() {
    return ctx().disabled_depth > 0 ? " disabled" : "";
}

static std::string auto_submit_attr() {
    return ctx().cfg.auto_submit ? " data-ftht-auto=\"1\"" : "";
}

static std::string next_style() {
    Context& c = ctx();
    std::string s;
    if (c.next.has_width) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "width:%.1fpx;", c.next.width);
        s += buf;
    }
    if (c.next.fill) s += "width:100%;";
    if (c.next.has_percent) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "width:%.4f%%;", c.next.percent * 100.0f);
        s += buf;
    }
    if (c.next.has_limits) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "min-width:%.1fpx;max-width:%.1fpx;", c.next.min_width, c.next.max_width);
        s += buf;
    }
    if (c.next.has_align) {
        if (c.next.align == Align::Center) s += "margin-left:auto;margin-right:auto;";
        if (c.next.align == Align::End) s += "margin-left:auto;";
    }
    c.next = NextLayoutState();
    if (s.empty()) return "";
    return " style=\"" + s + "\"";
}

static bool string_to_bool(const char* value) {
    if (!value) return false;
    return std::strcmp(value, "1") == 0 || std::strcmp(value, "true") == 0 ||
           std::strcmp(value, "on") == 0 || std::strcmp(value, "yes") == 0;
}

static bool action_matches(const std::string& id) {
    const char* action = find_param("_ftht_action");
    return action && id == action && ctx().disabled_depth == 0;
}

#if defined(ARDUINO) && defined(ESP32)
static bool read_http_request(std::string& raw, int max_bytes) {
    Context& c = ctx();
    raw.clear();
    unsigned long start = millis();
    while (c.net.client.connected() && (millis() - start) < 3000) {
        while (c.net.client.available()) {
            char ch = (char)c.net.client.read();
            raw += ch;
            if ((int)raw.size() >= max_bytes) return false;
            if (raw.find("\r\n\r\n") != std::string::npos) return true;
        }
        delay(1);
    }
    return raw.find("\r\n\r\n") != std::string::npos;
}

static void send_raw_response(const char* content_type, const std::string& body) {
    Context& c = ctx();
    std::string head = "HTTP/1.1 " + std::to_string(c.status_code) + " " + c.status_text + "\r\n";
    head += "Content-Type: ";
    head += content_type ? content_type : "application/octet-stream";
    head += "\r\n";
    head += c.extra_response_headers;
    head += "Connection: close\r\n";
    head += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    c.net.client.print(head.c_str());
    c.net.client.write((const uint8_t*)body.data(), body.size());
    c.net.client.stop();
    c.extra_response_headers.clear();
}

static void send_response(const std::string& body) {
    send_raw_response("text/html; charset=utf-8", body);
}
#else
static void close_socket(Socket& s) {
    if (s == invalid_socket) return;
#if defined(_WIN32)
    closesocket(s);
#else
    close(s);
#endif
    s = invalid_socket;
}

static bool wait_for_socket_read(Socket s, int timeout_ms) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(s, &readfds);
    timeval tv;
    timeval* tvp = nullptr;
    if (timeout_ms >= 0) {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        tvp = &tv;
    }
    int ready = select((int)(s + 1), &readfds, nullptr, nullptr, tvp);
    return ready > 0 && FD_ISSET(s, &readfds);
}

static bool send_all(Socket s, const char* data, int len) {
    int sent = 0;
    while (sent < len) {
#if defined(_WIN32)
        int n = send(s, data + sent, len - sent, 0);
#else
        int n = (int)send(s, data + sent, (size_t)(len - sent), 0);
#endif
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

static bool read_http_request(std::string& raw, int max_bytes) {
    Context& c = ctx();
    raw.clear();
    char buf[1024];
    while ((int)raw.size() < max_bytes) {
        if (!wait_for_socket_read(c.net.client, c.cfg.read_timeout_ms)) {
            debugf("client read timeout after %d ms; closing idle/partial connection", c.cfg.read_timeout_ms);
            return false;
        }
#if defined(_WIN32)
        int n = recv(c.net.client, buf, (int)sizeof(buf), 0);
#else
        int n = (int)recv(c.net.client, buf, sizeof(buf), 0);
#endif
        if (n <= 0) {
            debugf("recv failed or client closed before headers; n=%d err=%d", n, last_net_error());
            return false;
        }
        raw.append(buf, buf + n);
        debugf("read %d bytes from client (total=%d)", n, (int)raw.size());
        size_t header_end = raw.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            int content_length = 0;
            std::string headers = raw.substr(0, header_end + 4);
            size_t p = headers.find("Content-Length:");
            if (p == std::string::npos) p = headers.find("content-length:");
            if (p != std::string::npos) {
                p += 15;
                while (p < headers.size() && std::isspace((unsigned char)headers[p])) ++p;
                content_length = std::atoi(headers.c_str() + p);
            }
            size_t have_body = raw.size() - (header_end + 4);
            while ((int)have_body < content_length && (int)raw.size() < max_bytes) {
                if (!wait_for_socket_read(c.net.client, c.cfg.read_timeout_ms)) {
                    debugf("client body read timeout after %d ms (%d/%d bytes)", c.cfg.read_timeout_ms, (int)have_body, content_length);
                    return false;
                }
#if defined(_WIN32)
                n = recv(c.net.client, buf, (int)sizeof(buf), 0);
#else
                n = (int)recv(c.net.client, buf, sizeof(buf), 0);
#endif
                if (n <= 0) {
                    debugf("recv failed while reading body; n=%d err=%d", n, last_net_error());
                    return false;
                }
                raw.append(buf, buf + n);
                have_body += (size_t)n;
                debugf("read body chunk %d bytes (%d/%d)", n, (int)have_body, content_length);
            }
            return true;
        }
    }
    debugf("request exceeded max_request_bytes=%d", max_bytes);
    return false;
}

static void send_raw_response(const char* content_type, const std::string& body) {
    Context& c = ctx();
    std::string head = "HTTP/1.1 " + std::to_string(c.status_code) + " " + c.status_text + "\r\n";
    head += "Content-Type: ";
    head += content_type ? content_type : "application/octet-stream";
    head += "\r\n";
    head += c.extra_response_headers;
    head += "Connection: close\r\n";
    head += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    debugf("sending response %d %s (%d body bytes)", c.status_code, c.status_text.c_str(), (int)body.size());
    if (!send_all(c.net.client, head.c_str(), (int)head.size())) {
        debugf("failed sending response headers; err=%d", last_net_error());
    } else if (!send_all(c.net.client, body.data(), (int)body.size())) {
        debugf("failed sending response body; err=%d", last_net_error());
    }
    close_socket(c.net.client);
    c.extra_response_headers.clear();
}

static void send_response(const std::string& body) {
    send_raw_response("text/html; charset=utf-8", body);
}
#endif

static void render_form_login_page(const char* message) {
    Context& c = ctx();
    append_auth_page_start("Login");
    if (message && *message) {
        c.out += "<p class=\"ft-wrap\">" + escape_html(message) + "</p>";
    }
    c.out += "<label class=\"ft-field\"><span class=\"ft-label\">Username</span>";
    c.out += "<input name=\"_ftht_login_user\" autocomplete=\"username\"></label>";
    c.out += "<label class=\"ft-field\"><span class=\"ft-label\">Password</span>";
    c.out += "<input type=\"password\" name=\"_ftht_login_password\" autocomplete=\"current-password\"></label>";
    c.out += "<button type=\"submit\" class=\"accent\" name=\"_ftht_login\" value=\"form\">Login</button>";
    if (c.cfg.login_allow_registration) {
        c.out += "<hr>";
        c.out += "<p class=\"ft-wrap\">Registration requests pause here until the server terminal accepts them.</p>";
        c.out += "<label class=\"ft-field\"><span class=\"ft-label\">New username</span>";
        c.out += "<input name=\"_ftht_register_user\" autocomplete=\"username\"></label>";
        c.out += "<label class=\"ft-field\"><span class=\"ft-label\">New password</span>";
        c.out += "<input type=\"password\" name=\"_ftht_register_password\" autocomplete=\"new-password\"></label>";
        c.out += "<button type=\"submit\" name=\"_ftht_login\" value=\"register\">Request registration</button>";
    }
    append_auth_page_end();
    send_auth_page(c.out);
}

static void render_draw_login_page(const char* message) {
    Context& c = ctx();
    append_auth_page_start("Draw Login");
    if (message && *message) c.out += "<p class=\"ft-wrap\">" + escape_html(message) + "</p>";
    c.out += R"FTHT(
<input type="hidden" name="_ftht_shapes" id="ftht_shapes">
<div class="ft-row" style="grid-template-columns:1fr 1fr 1fr 1fr">
  <button type="button" class="sel" id="ft_shape_ellipse">Ellipse</button>
  <button type="button" id="ft_shape_rect">Rectangle</button>
  <button type="button" id="ft_shape_undo">Undo</button>
  <button type="button" id="ft_shape_clear">Clear</button>
</div>
<canvas id="ft_draw_canvas" width="760" height="360" style="width:100%;height:auto;touch-action:none;background:var(--paper2);border:2px solid var(--border);box-shadow:3px 3px 0 var(--border)"></canvas>
<button type="submit" class="accent" name="_ftht_login" value="draw">Login</button>
<script>
(() => {
  const canvas = document.getElementById("ft_draw_canvas");
  const hidden = document.getElementById("ftht_shapes");
  const ctx = canvas.getContext("2d");
  let tool = "e";
  let drawing = null;
  const shapes = [];
  const buttons = {
    e: document.getElementById("ft_shape_ellipse"),
    r: document.getElementById("ft_shape_rect")
  };
  function setTool(next) {
    tool = next;
    buttons.e.classList.toggle("sel", tool === "e");
    buttons.r.classList.toggle("sel", tool === "r");
  }
  function point(ev) {
    const rect = canvas.getBoundingClientRect();
    return {
      x: (ev.clientX - rect.left) * canvas.width / rect.width,
      y: (ev.clientY - rect.top) * canvas.height / rect.height
    };
  }
  function norm(s) {
    const x = Math.min(s.x, s.x + s.w);
    const y = Math.min(s.y, s.y + s.h);
    return { type: s.type, x, y, w: Math.abs(s.w), h: Math.abs(s.h) };
  }
  function drawShape(s) {
    s = norm(s);
    ctx.beginPath();
    if (s.type === "e") {
      ctx.ellipse(s.x + s.w / 2, s.y + s.h / 2, Math.max(1, s.w / 2), Math.max(1, s.h / 2), 0, 0, Math.PI * 2);
    } else {
      ctx.rect(s.x, s.y, s.w, s.h);
    }
    ctx.stroke();
  }
  function sync() {
    hidden.value = shapes.map(s => {
      s = norm(s);
      return [s.type, s.x.toFixed(1), s.y.toFixed(1), s.w.toFixed(1), s.h.toFixed(1)].join(",");
    }).join(";");
  }
  function render() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.lineWidth = 4;
    ctx.strokeStyle = getComputedStyle(document.documentElement).getPropertyValue("--ink") || "#050505";
    shapes.forEach(drawShape);
    if (drawing) {
      ctx.setLineDash([10, 6]);
      drawShape(drawing);
      ctx.setLineDash([]);
    }
    sync();
  }
  buttons.e.addEventListener("click", () => setTool("e"));
  buttons.r.addEventListener("click", () => setTool("r"));
  document.getElementById("ft_shape_undo").addEventListener("click", () => { shapes.pop(); render(); });
  document.getElementById("ft_shape_clear").addEventListener("click", () => { shapes.length = 0; render(); });
  canvas.addEventListener("pointerdown", ev => {
    canvas.setPointerCapture(ev.pointerId);
    const p = point(ev);
    drawing = { type: tool, x: p.x, y: p.y, w: 0, h: 0 };
    render();
  });
  canvas.addEventListener("pointermove", ev => {
    if (!drawing) return;
    const p = point(ev);
    drawing.w = p.x - drawing.x;
    drawing.h = p.y - drawing.y;
    render();
  });
  canvas.addEventListener("pointerup", () => {
    if (!drawing) return;
    const s = norm(drawing);
    drawing = null;
    if (s.w >= 8 && s.h >= 8) shapes.push(s);
    render();
  });
  render();
})();
</script>)FTHT";
    append_auth_page_end();
    send_auth_page(c.out);
}

static bool serve_auth_if_needed() {
    Context& c = ctx();
    if (c.cfg.login_mode == LoginMode::None) return false;
    if (c.req_path == "/ftht/logout") {
        c.login_session_token.clear();
        c.login_session_until = 0;
        c.extra_response_headers += "Set-Cookie: ftht_session=; Path=/; Max-Age=0; HttpOnly; SameSite=Lax\r\n";
        queue_toast(ToastType::Info, "Logged Out", "Session cleared.");
        send_redirect_home();
        return true;
    }
    if (has_login_session()) return false;

    const char* login_action = find_param("_ftht_login");
    if (c.req_method == "POST" && login_action) {
        if (c.cfg.login_mode == LoginMode::DrawPenis && std::strcmp(login_action, "draw") == 0) {
            if (drawing_is_penis(find_param("_ftht_shapes"))) {
                start_login_session();
                queue_toast(ToastType::Success, "Authentication Successful", random_draw_success_message(), 6000, true);
                send_redirect_home();
            } else {
                queue_toast(ToastType::Warning, "Authentication Failed", "Shape was not classified as penis. A penis is the only accepted password.", 5000, true);
                render_draw_login_page("Not quite. The recognizer wants two nearby ellipses and one long ellipse or rectangle.");
            }
            return true;
        }

        if (c.cfg.login_mode == LoginMode::Form && std::strcmp(login_action, "form") == 0) {
            LoginCredential cred;
            const char* user = find_param("_ftht_login_user");
            const char* password = find_param("_ftht_login_password");
            bool ok = find_login_credential(user, cred) && password &&
                      salted_password_hash(password, cred.salt) == cred.hash;
            if (ok) {
                start_login_session();
                queue_toast(ToastType::Success, "Authentication Successful", "Signed in.");
                send_redirect_home();
            } else if (configured_login_credentials().empty()) {
                queue_toast(ToastType::Error, "Login Not Configured", "Set login_password or login_password_hash before using form login.", 5000, true);
                render_form_login_page("Set login_password or login_password_hash before using form login.");
            } else {
                queue_toast(ToastType::Error, "Authentication Failed", "Invalid username or password.", 5000, true);
                render_form_login_page("Invalid username or password.");
            }
            return true;
        }

        if (c.cfg.login_mode == LoginMode::Form && std::strcmp(login_action, "register") == 0) {
            const char* user = find_param("_ftht_register_user");
            const char* password = find_param("_ftht_register_password");
            std::string message;
            if (register_login_user(user, password, message)) {
                start_login_session();
                queue_toast(ToastType::Success, "Authentication Successful", "Registered and signed in.");
                send_redirect_home();
            } else {
                render_form_login_page(message.c_str());
            }
            return true;
        }
    }

    if (c.cfg.login_mode == LoginMode::DrawPenis) {
        render_draw_login_page("Complete the shape challenge to continue.");
    } else {
        std::vector<LoginCredential> creds = configured_login_credentials();
        render_form_login_page(!creds.empty() ? "Sign in to continue." : "Set login_password or login_password_hash before using form login.");
    }
    return true;
}

static bool serve_builtin_asset_if_needed() {
    Context& c = ctx();
    if (c.req_path == "/favicon.svg" || c.req_path == "/favicon.ico") {
        c.status_code = 200;
        c.status_text = "OK";
        debugf("serving builtin favicon");
        send_raw_response("image/svg+xml; charset=utf-8", std::string(builtin_favicon_svg()));
        return true;
    }
    return false;
}

static bool parse_request(const std::string& raw) {
    Context& c = ctx();
    c.req_method.clear();
    c.req_path.clear();
    c.req_query.clear();
    c.req_body.clear();
    c.req_headers.clear();
    c.req_cookie.clear();
    c.params.clear();
    c.extra_response_headers.clear();
    c.status_code = 200;
    c.status_text = "OK";
    c.client_update = false;

    size_t line_end = raw.find("\r\n");
    if (line_end == std::string::npos) {
        debugf("request parse failed: no request line");
        return false;
    }
    std::string first = raw.substr(0, line_end);
    size_t a = first.find(' ');
    size_t b = first.find(' ', a == std::string::npos ? 0 : a + 1);
    if (a == std::string::npos || b == std::string::npos) {
        debugf("request parse failed: malformed request line '%s'", first.c_str());
        return false;
    }
    c.req_method = first.substr(0, a);
    std::string target = first.substr(a + 1, b - a - 1);
    size_t q = target.find('?');
    c.req_path = q == std::string::npos ? target : target.substr(0, q);
    c.req_query = q == std::string::npos ? "" : target.substr(q + 1);

    size_t header_end = raw.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        c.req_headers = raw.substr(0, header_end + 2);
        std::string_view headers(c.req_headers.data(), c.req_headers.size());
        c.client_update = contains_header_token(headers, "x-ftht-client: fetch");
        c.req_cookie = header_value(headers, "cookie");
    }
    if (header_end != std::string::npos && header_end + 4 < raw.size()) {
        c.req_body = raw.substr(header_end + 4);
    }
    parse_params(c.req_query);
    if (!c.req_body.empty()) parse_params(c.req_body);
    debugf("request parsed: method=%s path=%s query_bytes=%d body_bytes=%d params=%d",
           c.req_method.c_str(), c.req_path.c_str(), (int)c.req_query.size(),
           (int)c.req_body.size(), (int)c.params.size());
    return true;
}

static bool accept_next_request(int timeout_ms) {
    Context& c = ctx();
#if defined(ARDUINO) && defined(ESP32)
    if (!c.net.server) return false;
    unsigned long start = millis();
    do {
        c.net.client = c.net.server->available();
        if (c.net.client) {
            std::string raw;
            if (!read_http_request(raw, c.cfg.max_request_bytes)) return false;
            if (!parse_request(raw)) return false;
            if (serve_builtin_asset_if_needed()) continue;
            if (serve_auth_if_needed()) continue;
            return true;
        }
        if (timeout_ms < 0) delay(1);
    } while (timeout_ms < 0 || (int)(millis() - start) < timeout_ms);
    return false;
#else
    if (c.net.server == invalid_socket) {
        debugf("pump called before create_server succeeded");
        return false;
    }

    for (;;) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(c.net.server, &readfds);
        timeval tv;
        timeval* tvp = nullptr;
        if (timeout_ms >= 0) {
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            tvp = &tv;
        }
        debugf("waiting for client on port %d timeout_ms=%d", c.cfg.port, timeout_ms);
        int ready = select((int)(c.net.server + 1), &readfds, nullptr, nullptr, tvp);
        if (ready <= 0) {
            if (ready < 0) debugf("select failed while waiting for client; err=%d", last_net_error());
            return false;
        }

        c.net.client = accept(c.net.server, nullptr, nullptr);
        if (c.net.client == invalid_socket) {
            debugf("accept failed; err=%d", last_net_error());
            if (timeout_ms >= 0) return false;
            continue;
        }
        debugf("client accepted");

        std::string raw;
        if (!read_http_request(raw, c.cfg.max_request_bytes)) {
            debugf("dropping invalid/empty client connection");
            close_socket(c.net.client);
            if (timeout_ms >= 0) return false;
            continue;
        }
        if (!parse_request(raw)) {
            debugf("dropping unparseable request (%d bytes)", (int)raw.size());
            close_socket(c.net.client);
            if (timeout_ms >= 0) return false;
            continue;
        }
        if (serve_builtin_asset_if_needed()) {
            if (timeout_ms >= 0) return false;
            continue;
        }
        if (serve_auth_if_needed()) {
            if (timeout_ms >= 0) return false;
            continue;
        }
        return true;
    }
#endif
}

static void append_css() {
    Context& c = ctx();
    const Style& s = c.style;
    c.out += "<style>";
    c.out += ":root{color-scheme:light dark;--desk:#d9d6c7;--paper:#fffefa;--paper2:#f2f0e6;--ink:#050505;--muted:#333;--faint:#777;--border:#050505;";
    c.out += "--panel:" + color_css(s.panel) + ";--text:" + color_css(s.text) + ";--dim:" + color_css(s.text_dim);
    c.out += ";--button:#fffefa;--hover:#efeee5;--active:#e4e1d3;--input:#fffefa;--focus:" + color_css(s.input_focus);
    c.out += ";--accent:" + color_css(s.accent) + ";--warning:" + color_css(s.warning) + ";--success:" + color_css(s.success) + ";--error:#d94b4b;}";
    if (c.cfg.dark_mode) {
        c.out += ":root{--desk:#060607;--paper:#101011;--paper2:#181819;--ink:#f5f3e8;--muted:#c6c2b5;--faint:#898579;--border:#f5f3e8;--button:#151516;--hover:#202023;--active:#2b2b2f;--input:#0b0b0c;}";
    }
    if (c.cfg.respect_browser_dark_mode) {
        c.out += "@media(prefers-color-scheme:dark){:root{--desk:#060607;--paper:#101011;--paper2:#181819;--ink:#f5f3e8;--muted:#c6c2b5;--faint:#898579;--border:#f5f3e8;--button:#151516;--hover:#202023;--active:#2b2b2f;--input:#0b0b0c;}}";
    }
    c.out += "*{box-sizing:border-box}html{background:var(--desk)}body{margin:0;min-height:100vh;background:var(--desk);color:var(--ink);font:15px/1.35 ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;letter-spacing:0;}";
    c.out += "main{max-width:1120px;margin:22px auto;padding:20px;background:var(--paper);color:var(--ink);border:3px solid var(--border);border-radius:3px;box-shadow:8px 10px 0 rgba(0,0,0,.18);position:relative;overflow:hidden;transition:opacity .12s ease,transform .12s ease;}";
    c.out += "main.ft-updating{opacity:.76;transform:translateY(1px)}form{display:flex;flex-direction:column;gap:10px;position:relative}.ft-title{font-size:30px;font-weight:900;margin:0 0 4px;text-transform:uppercase;text-shadow:.7px 0 0 var(--ink),-.35px .4px 0 rgba(0,0,0,.20)}";
    c.out += ".ft-text{margin:0;font-weight:700}.ft-wrap{margin:0;color:var(--muted)}hr{border:0;border-top:3px solid var(--border);width:100%;margin:6px 0;filter:url(#ftht-rough-ink)}";
    c.out += ".ft-field{display:flex;flex-direction:column;gap:6px}.ft-label{font-size:12px;font-weight:900;color:var(--muted);text-transform:uppercase;letter-spacing:.04em}";
    c.out += "input,textarea,select{width:100%;min-height:38px;border:2px solid var(--border);border-radius:2px;background:var(--input);color:var(--ink);padding:8px 10px;font:inherit;box-shadow:inset .8px 0 0 rgba(0,0,0,.22),inset -.6px .5px 0 rgba(0,0,0,.16)}";
    c.out += "textarea{resize:vertical}input:focus,textarea:focus,select:focus{outline:3px solid var(--focus);outline-offset:2px}";
    c.out += ".ft-label{display:flex;justify-content:space-between;gap:10px;align-items:baseline}.ft-range-value{font-weight:900;color:var(--ink)}";
    c.out += "input[type=range]{appearance:none;-webkit-appearance:none;min-height:28px;border:0;background:transparent;padding:8px 0;box-shadow:none;filter:none;cursor:pointer}";
    c.out += "input[type=range]::-webkit-slider-runnable-track{height:8px;border:2px solid var(--border);background:linear-gradient(90deg,var(--accent) 0 var(--ft-range-pct,50%),var(--paper2) var(--ft-range-pct,50%) 100%);box-shadow:2px 2px 0 var(--border)}";
    c.out += "input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;border:3px solid var(--border);border-radius:50%;background:var(--paper);margin-top:-9px;box-shadow:2px 2px 0 var(--border)}";
    c.out += "input[type=range]::-moz-range-track{height:8px;border:2px solid var(--border);background:var(--paper2);box-shadow:2px 2px 0 var(--border)}input[type=range]::-moz-range-progress{height:8px;background:var(--accent)}input[type=range]::-moz-range-thumb{width:18px;height:18px;border:3px solid var(--border);border-radius:50%;background:var(--paper);box-shadow:2px 2px 0 var(--border)}";
    c.out += "button{min-height:38px;border:2px solid var(--border);border-radius:2px;background:var(--button);color:var(--ink);padding:8px 12px;font:900 14px/1.2 ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;text-transform:uppercase;cursor:pointer;filter:url(#ftht-rough-ink-light);box-shadow:3px 3px 0 var(--border);transition:background .10s ease,transform .08s ease,box-shadow .08s ease}";
    c.out += "button:hover{background:var(--hover)}button:active{background:var(--active);transform:translate(2px,2px);box-shadow:1px 1px 0 var(--border)}button:disabled,input:disabled,textarea:disabled,select:disabled{opacity:.50;cursor:not-allowed}";
    c.out += ".ft-row{display:grid;gap:10px;align-items:start}.ft-check{display:flex;align-items:center;gap:10px}.ft-check input{width:20px;min-height:20px;accent-color:var(--ink)}.ft-radio{display:grid;gap:8px}.ft-radio label{display:flex;gap:8px;align-items:center}.ft-radio input{width:18px;min-height:18px;accent-color:var(--ink)}";
    c.out += ".ft-log{white-space:pre-wrap;background:var(--paper2);border:2px solid var(--border);border-radius:2px;margin:0;padding:10px;overflow:auto;color:var(--ink);box-shadow:inset 1px 0 0 rgba(0,0,0,.18)}";
    c.out += ".ft-tabs{display:flex;gap:8px;flex-wrap:wrap}.ft-tabs .sel,button.sel{background:var(--ink);border-color:var(--ink);color:var(--paper);box-shadow:3px 3px 0 var(--faint)}.ft-scroll{overflow:auto;border:2px solid var(--border);border-radius:2px;padding:10px;background:var(--paper2)}";
    c.out += "details{border:2px solid var(--border);border-radius:2px;padding:10px;background:var(--paper2)}summary{cursor:pointer;font-weight:900}.ft-modal{position:fixed;inset:0;background:rgba(0,0,0,.58);display:grid;place-items:center;padding:20px;z-index:10}.ft-modal>section{width:min(560px,100%);background:var(--paper);color:var(--ink);border:3px solid var(--border);border-radius:2px;padding:16px;display:flex;flex-direction:column;gap:10px;box-shadow:8px 10px 0 rgba(0,0,0,.28);filter:url(#ftht-rough-ink-light)}";
    c.out += ".accent{background:var(--accent);border-color:var(--border);color:#fff}.warning{background:var(--warning);border-color:var(--border);color:#0b0807}.success{background:var(--success);border-color:var(--border);color:#071007}";
    c.out += ".ft-toast-stack{position:fixed;right:18px;bottom:18px;z-index:60;display:flex;flex-direction:column;align-items:flex-end;gap:10px;pointer-events:none;width:min(420px,calc(100vw - 24px))}";
    c.out += ".ft-toast{pointer-events:auto;width:100%;display:grid;grid-template-columns:1fr auto;gap:6px 10px;background:var(--paper);color:var(--ink);border:2px solid var(--border);border-left-width:8px;border-radius:2px;padding:10px 10px 10px 12px;box-shadow:5px 6px 0 rgba(0,0,0,.22);filter:url(#ftht-rough-ink-light);opacity:0;transform:translateY(8px);animation:ft-toast-in .18s ease forwards}";
    c.out += ".ft-toast-info{border-left-color:var(--accent)}.ft-toast-success{border-left-color:var(--success)}.ft-toast-warning{border-left-color:var(--warning)}.ft-toast-error{border-left-color:var(--error)}";
    c.out += ".ft-toast-title{font-weight:900;text-transform:uppercase;font-size:12px;color:var(--muted);letter-spacing:.04em}.ft-toast-body{grid-column:1;margin:0;font-weight:800}.ft-toast-close{grid-column:2;grid-row:1/span 2;align-self:start;min-height:24px;min-width:24px;padding:0 6px;box-shadow:2px 2px 0 var(--border);font-size:16px;line-height:1}.ft-toast-out{animation:ft-toast-out .20s ease forwards}";
    c.out += "@keyframes ft-toast-in{to{opacity:1;transform:translateY(0)}}@keyframes ft-toast-out{to{opacity:0;transform:translateY(8px)}}";
    c.out += ".ft-bleed{pointer-events:none;position:absolute;inset:0;border:0 solid transparent;opacity:0}";
    c.out += "@media(prefers-color-scheme:dark){.accent{color:#fff}.warning,.success{color:#050505}}@media(max-width:720px){main{margin:0;padding:14px;border-width:2px;box-shadow:none}.ft-row{grid-template-columns:1fr!important}.ft-toast-stack{left:10px;right:10px;bottom:10px;width:auto;align-items:stretch}.ft-toast{width:100%}}";
    c.out += "</style>";
}

static void append_ink_filters() {
    Context& c = ctx();
    c.out += "<svg width=\"0\" height=\"0\" aria-hidden=\"true\" focusable=\"false\" style=\"position:absolute;left:-9999px;top:-9999px\">";
    c.out += "<defs>";
    c.out += "<filter id=\"ftht-rough-ink\" x=\"-12%\" y=\"-12%\" width=\"124%\" height=\"124%\">";
    c.out += "<feMorphology in=\"SourceAlpha\" operator=\"dilate\" radius=\"0.75\" result=\"fat\"/>";
    c.out += "<feGaussianBlur in=\"fat\" stdDeviation=\"0.50\" result=\"blur\"/>";
    c.out += "<feTurbulence type=\"fractalNoise\" baseFrequency=\"0.92\" numOctaves=\"2\" seed=\"7\" result=\"noise\"/>";
    c.out += "<feDisplacementMap in=\"blur\" in2=\"noise\" scale=\"1.65\" xChannelSelector=\"R\" yChannelSelector=\"G\" result=\"disp\"/>";
    c.out += "<feColorMatrix in=\"disp\" type=\"matrix\" values=\"1 0 0 0 0  0 1 0 0 0  0 0 1 0 0  0 0 0 28 -9\" result=\"thresh\"/>";
    c.out += "<feComposite in=\"SourceGraphic\" in2=\"thresh\" operator=\"over\"/>";
    c.out += "</filter>";
    c.out += "<filter id=\"ftht-rough-ink-light\" x=\"-8%\" y=\"-8%\" width=\"116%\" height=\"116%\">";
    c.out += "<feMorphology in=\"SourceAlpha\" operator=\"dilate\" radius=\"0.45\" result=\"fat\"/>";
    c.out += "<feGaussianBlur in=\"fat\" stdDeviation=\"0.28\" result=\"blur\"/>";
    c.out += "<feTurbulence type=\"fractalNoise\" baseFrequency=\"0.8\" numOctaves=\"2\" seed=\"11\" result=\"noise\"/>";
    c.out += "<feDisplacementMap in=\"blur\" in2=\"noise\" scale=\"0.95\" xChannelSelector=\"R\" yChannelSelector=\"G\" result=\"disp\"/>";
    c.out += "<feColorMatrix in=\"disp\" type=\"matrix\" values=\"1 0 0 0 0  0 1 0 0 0  0 0 1 0 0  0 0 0 24 -7\" result=\"thresh\"/>";
    c.out += "<feComposite in=\"SourceGraphic\" in2=\"thresh\" operator=\"over\"/>";
    c.out += "</filter>";
    c.out += "</defs></svg>";
}

static void append_toast_stack() {
    Context& c = ctx();
    prune_toasts();
    if (c.toasts.empty()) return;
    c.out += "<div class=\"ft-toast-stack\" aria-live=\"polite\" aria-atomic=\"false\">";
    for (const ActiveToast& item : c.toasts) {
        const Toast& t = item.toast;
        std::string title = t.title;
        if (title.empty()) title = toast_default_title(t.type);
        c.out += "<section class=\"ft-toast ft-toast-";
        c.out += toast_type_class(t.type);
        c.out += "\" data-ft-toast-id=\"";
        c.out += std::to_string((unsigned long long)item.id);
        c.out += "\" data-ft-toast-duration=\"";
        c.out += std::to_string(std::max(1000, t.duration_ms));
        c.out += "\" role=\"";
        c.out += t.type == ToastType::Error ? "alert" : "status";
        c.out += "\">";
        c.out += "<div>";
        if (!title.empty()) {
            c.out += "<div class=\"ft-toast-title\">";
            c.out += escape_html(title);
            c.out += "</div>";
        }
        if (!t.message.empty()) {
            c.out += "<p class=\"ft-toast-body\">";
            c.out += escape_html(t.message);
            c.out += "</p>";
        }
        c.out += "</div>";
        if (t.dismissible) {
            c.out += "<button type=\"button\" class=\"ft-toast-close\" aria-label=\"Dismiss toast\">&times;</button>";
        }
        c.out += "</section>";
    }
    c.out += "</div>";
}

static void append_toast_script() {
    Context& c = ctx();
    c.out += R"(<script>
(() => {
  if (window.__fthtToastClient) {
    if (window.__fthtInitToasts) window.__fthtInitToasts(document);
    return;
  }
  window.__fthtToastClient = true;
  const dismissed = window.__fthtDismissedToasts || (window.__fthtDismissedToasts = new Set());
  function ensureStack() {
    let stack = null;
    for (const child of document.body.children) {
      if (child.classList && child.classList.contains("ft-toast-stack")) {
        stack = child;
        break;
      }
    }
    if (!stack) {
      stack = document.createElement("div");
      stack.className = "ft-toast-stack";
      stack.setAttribute("aria-live", "polite");
      stack.setAttribute("aria-atomic", "false");
      document.body.appendChild(stack);
    }
    return stack;
  }
  function dismissToast(el) {
    if (!el || el.classList.contains("ft-toast-out")) return;
    const id = el.getAttribute("data-ft-toast-id");
    if (id) dismissed.add(id);
    el.classList.add("ft-toast-out");
    setTimeout(() => el.remove(), 240);
  }
  window.__fthtInitToasts = function(root) {
    root = root || document;
    const sources = Array.from(root.querySelectorAll(".ft-toast-stack"));
    if (!sources.length) return;
    const stack = ensureStack();
    for (const source of sources) {
      if (source === stack) continue;
      const items = Array.from(source.querySelectorAll(".ft-toast"));
      for (const item of items) {
        const id = item.getAttribute("data-ft-toast-id") || "";
        if (id && dismissed.has(id)) {
          item.remove();
          continue;
        }
        if (id && stack.querySelector(`[data-ft-toast-id="${id}"]`)) {
          item.remove();
          continue;
        }
        const close = item.querySelector(".ft-toast-close");
        if (close) close.addEventListener("click", () => dismissToast(item));
        const duration = Math.max(1000, Number(item.getAttribute("data-ft-toast-duration") || 4000));
        stack.appendChild(item);
        setTimeout(() => dismissToast(item), duration);
      }
      if (source !== stack) source.remove();
    }
  };
  window.__fthtInitToasts(document);
})();
</script>)";
}

static void append_client_script() {
    Context& c = ctx();
    c.out += R"(<script>
(() => {
  if (window.__fthtClient) return;
  window.__fthtClient = true;
  const clientDebug = )";
    c.out += c.cfg.client_debug_output ? "true" : "false";
    c.out += R"(;
  const log = (...a) => { if (clientDebug) console.debug("[ftht]", ...a); };
  const pollMs = )";
    c.out += std::to_string(std::max(0, c.cfg.client_poll_ms));
    c.out += R"(;
  let timer = 0;
  let inflight = null;
  let lastButton = null;

  function main() { return document.querySelector("main"); }
  function form() { return document.querySelector("main form"); }
  function activeSnapshot() {
    const el = document.activeElement;
    if (!el || !el.name) return null;
    let selection = null;
    try {
      selection = {
        start: typeof el.selectionStart === "number" ? el.selectionStart : null,
        end: typeof el.selectionEnd === "number" ? el.selectionEnd : null
      };
    } catch (_) {}
    return { name: el.name, value: el.value, selection };
  }
  function cssName(name) {
    if (window.CSS && CSS.escape) return CSS.escape(name);
    return String(name).replace(/\\/g, "\\\\").replace(/"/g, "\\\"");
  }
  function restoreActive(snapshot) {
    if (!snapshot) return;
    const el = document.querySelector(`[name="${cssName(snapshot.name)}"]`);
    if (!el) return;
    el.focus({ preventScroll: true });
    if (snapshot.selection && snapshot.selection.start !== null && "setSelectionRange" in el) {
      try { el.setSelectionRange(snapshot.selection.start, snapshot.selection.end); } catch (_) {}
    }
  }
  function formatRangeValue(el) {
    const n = Number(el.value || 0);
    if (!Number.isFinite(n)) return el.value || "";
    return Math.abs(n) >= 10 ? n.toFixed(1).replace(/\.0$/, "") : n.toFixed(2).replace(/0$/, "").replace(/\.$/, "");
  }
  function updateRange(el) {
    const min = Number(el.min || 0);
    const max = Number(el.max || 100);
    const val = Number(el.value || 0);
    const pct = max > min ? Math.max(0, Math.min(100, ((val - min) / (max - min)) * 100)) : 0;
    el.style.setProperty("--ft-range-pct", pct + "%");
    const field = el.closest(".ft-field");
    const out = field && field.querySelector(".ft-range-value");
    if (out) out.textContent = formatRangeValue(el);
  }
  function initRanges(root = document) {
    root.querySelectorAll('input[type="range"]').forEach(updateRange);
  }
  async function send(reason, submitter) {
    const f = form();
    const shell = main();
    if (!f || !shell) return;
    const startedAt = performance.now();
    clearTimeout(timer);
    const snapshot = activeSnapshot();
    const data = reason === "poll" ? new FormData() : new FormData(f);
    if (reason === "poll") data.set("_ftht_poll", "1");
    if (submitter && submitter.name) data.set(submitter.name, submitter.value || "");
    if (inflight) inflight.abort();
    inflight = new AbortController();
    shell.classList.add("ft-updating");
    log("send", reason, Array.from(data.keys()));
    try {
      const res = await fetch(f.action || location.href, {
        method: "POST",
        body: new URLSearchParams(data),
        headers: { "Content-Type": "application/x-www-form-urlencoded;charset=UTF-8", "X-FTHT-Client": "fetch" },
        signal: inflight.signal
      });
      const fetchedAt = performance.now();
      const html = await res.text();
      const doc = new DOMParser().parseFromString(html, "text/html");
      const next = doc.querySelector("main");
      if (!next) throw new Error("response did not contain <main>");
      shell.innerHTML = next.innerHTML;
      initRanges(shell);
      if (window.__fthtInitToasts) window.__fthtInitToasts(shell);
      restoreActive(snapshot);
      const doneAt = performance.now();
      log("updated", res.status, html.length, "fetch_ms", (fetchedAt - startedAt).toFixed(1), "swap_ms", (doneAt - fetchedAt).toFixed(1), "total_ms", (doneAt - startedAt).toFixed(1));
    } catch (err) {
      const msg = String((err && err.message) || err || "");
      if (err.name === "AbortError" || reason === "poll" || msg.includes("Failed to fetch")) {
        log("update skipped", reason, msg);
      } else {
        log("update failed; falling back to normal submit", msg);
        f.submit();
      }
    } finally {
      const m = main();
      if (m) m.classList.remove("ft-updating");
    }
  }
  function schedule(reason, el) {
    if (!el || !el.matches("[data-ftht-auto]")) return;
    const tag = el.tagName;
    const type = (el.type || "").toLowerCase();
    if (type === "range" && reason === "input") {
      updateRange(el);
      return;
    }
    const fast = tag === "SELECT" || type === "checkbox" || type === "radio" || type === "range";
    clearTimeout(timer);
    timer = setTimeout(() => send(reason, null), fast ? 20 : 220);
  }
  document.addEventListener("submit", ev => {
    const f = form();
    if (!f || ev.target !== f) return;
    ev.preventDefault();
    send("submit", ev.submitter || lastButton);
    lastButton = null;
  });
  document.addEventListener("click", ev => {
    const b = ev.target.closest("button");
    if (b) lastButton = b;
  });
  document.addEventListener("change", ev => schedule("change", ev.target));
  document.addEventListener("input", ev => schedule("input", ev.target));
  initRanges();
  if (pollMs > 0) {
    setInterval(() => {
      const active = document.activeElement;
      if (active && /^(INPUT|TEXTAREA|SELECT)$/.test(active.tagName)) return;
      send("poll", null);
    }, pollMs);
  }
  log("client ready");
})();
</script>)";
}

static void begin_field(const char* label, const std::string& id) {
    Context& c = ctx();
    c.last_widget_id = id;
    c.out += "<label class=\"ft-field\"" + next_style() + "><span class=\"ft-label\">";
    c.out += escape_html(visible_label(label));
    c.out += "</span>";
}

} // namespace internal

Style default_dark_style() {
    return Style();
}

Style one_dark_style() {
    Style s;
    s.background    = {0.110f, 0.122f, 0.153f, 1.0f};
    s.panel         = {0.133f, 0.145f, 0.176f, 1.0f};
    s.text          = {0.835f, 0.859f, 0.914f, 1.0f};
    s.text_dim      = {0.604f, 0.647f, 0.718f, 1.0f};
    s.border        = {0.247f, 0.271f, 0.329f, 1.0f};
    s.button        = {0.173f, 0.192f, 0.235f, 1.0f};
    s.button_hover  = {0.220f, 0.243f, 0.294f, 1.0f};
    s.button_active = {0.282f, 0.314f, 0.376f, 1.0f};
    s.input_bg      = {0.095f, 0.106f, 0.133f, 1.0f};
    s.input_focus   = {0.380f, 0.569f, 0.937f, 1.0f};
    s.accent        = {0.380f, 0.569f, 0.937f, 1.0f};
    s.warning       = {0.878f, 0.537f, 0.333f, 1.0f};
    s.success       = {0.596f, 0.761f, 0.463f, 1.0f};
    return s;
}

void set_style(const Style& s) {
    internal::ctx().style = s;
}

const Style& get_style() {
    return internal::ctx().style;
}

Color color_from_hex(const char* hex) {
    return color_from_hex(std::string_view(hex ? hex : ""));
}

Color color_from_hex(std::string_view hex) {
    if (!hex.empty() && hex[0] == '#') hex.remove_prefix(1);
    unsigned value = 0;
    for (char c : hex) {
        int d = internal::hex_digit(c);
        if (d < 0) break;
        value = (value << 4) | (unsigned)d;
    }
    if (hex.size() == 6) {
        return {((value >> 16) & 255) / 255.0f, ((value >> 8) & 255) / 255.0f, (value & 255) / 255.0f, 1.0f};
    }
    if (hex.size() >= 8) {
        return {((value >> 24) & 255) / 255.0f, ((value >> 16) & 255) / 255.0f, ((value >> 8) & 255) / 255.0f, (value & 255) / 255.0f};
    }
    return {1, 1, 1, 1};
}

bool create_server(const Config& cfg) {
    using namespace internal;
    Context& c = ctx();
    c.cfg = cfg;
    c.current_url = std::string("http://127.0.0.1:") + std::to_string(cfg.port) + "/";
    std::srand((unsigned)std::time(nullptr));
    c.login_session_token.clear();
    c.login_session_until = 0;
    c.toasts.clear();
    c.next_toast_id = 1;
    debugf("create_server title='%s' host='%s' port=%d max_request_bytes=%d read_timeout_ms=%d client_poll_ms=%d auto_submit=%d debug=%d client_debug=%d dark_mode=%d browser_dark=%d login_mode=%d login_idle_seconds=%d",
           cfg.title ? cfg.title : "", cfg.host ? cfg.host : "", cfg.port, cfg.max_request_bytes,
           cfg.read_timeout_ms, cfg.client_poll_ms, cfg.auto_submit ? 1 : 0,
           cfg.debug_output ? 1 : 0, cfg.client_debug_output ? 1 : 0, cfg.dark_mode ? 1 : 0,
           cfg.respect_browser_dark_mode ? 1 : 0, (int)cfg.login_mode, cfg.login_session_seconds);
#if defined(ARDUINO) && defined(ESP32)
    if (c.net.server) delete c.net.server;
    c.net.server = new WiFiServer((uint16_t)cfg.port);
    c.net.server->begin();
    debugf("ESP32 WiFiServer started on port %d", cfg.port);
    if (cfg.print_url) {
        Serial.print("FTHT listening on port ");
        Serial.println(cfg.port);
    }
    return true;
#else
#if defined(_WIN32)
    if (!c.net.wsa_started) {
        WSADATA wsa;
        int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa);
        if (wsa_result != 0) {
            debugf("WSAStartup failed err=%d", wsa_result);
            return false;
        }
        c.net.wsa_started = true;
        debugf("WSAStartup succeeded");
    }
#endif
    c.net.server = socket(AF_INET, SOCK_STREAM, 0);
    if (c.net.server == invalid_socket) {
        debugf("socket(AF_INET, SOCK_STREAM) failed err=%d", last_net_error());
        return false;
    }
    debugf("server socket created");
    int yes = 1;
    setsockopt(c.net.server, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)cfg.port);
    if (!cfg.host || std::strcmp(cfg.host, "0.0.0.0") == 0) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        if (inet_pton(AF_INET, cfg.host, &addr.sin_addr) != 1) {
            debugf("inet_pton failed for host '%s'", cfg.host);
            close_socket(c.net.server);
            return false;
        }
    }
    if (bind(c.net.server, (sockaddr*)&addr, sizeof(addr)) != 0) {
        debugf("bind failed host='%s' port=%d err=%d", cfg.host ? cfg.host : "", cfg.port, last_net_error());
        close_socket(c.net.server);
        return false;
    }
    debugf("bind succeeded host='%s' port=%d", cfg.host ? cfg.host : "", cfg.port);
    if (listen(c.net.server, 8) != 0) {
        debugf("listen failed err=%d", last_net_error());
        close_socket(c.net.server);
        return false;
    }
    debugf("listen succeeded backlog=8");
    if (cfg.print_url) std::printf("FTHT listening at %s\n", c.current_url.c_str());
    return true;
#endif
}

bool pump(int timeout_ms) {
    using namespace internal;
    Context& c = ctx();
    c.request_active = false;
    c.response_open = false;
    c.status_code = 200;
    c.status_text = "OK";
    bool ok = accept_next_request(timeout_ms);
    c.request_active = ok;
    if (!ok && timeout_ms >= 0) debugf("pump timeout/no request");
    if (ok) debugf("pump has request: %s %s", c.req_method.c_str(), c.req_path.c_str());
    return ok;
}

void begin() {
    using namespace internal;
    Context& c = ctx();
    debugf("begin render for %s %s", c.req_method.c_str(), c.req_path.c_str());
    c.out.clear();
    c.response_open = true;
    c.modal_rendered = false;
    if (c.client_update) {
        c.out += "<main><div class=\"ft-bleed\"></div><form method=\"post\" action=\"";
        c.out += escape_html(c.req_path.empty() ? "/" : c.req_path);
        c.out += "\"><h1 class=\"ft-title\">";
        c.out += escape_html(c.cfg.title ? c.cfg.title : "FTHT App");
        c.out += "</h1>";
        return;
    }
    c.out += "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">";
    c.out += "<title>" + escape_html(c.cfg.title ? c.cfg.title : "FTHT App") + "</title>";
    c.out += "<link rel=\"icon\" href=\"/favicon.svg\" type=\"image/svg+xml\">";
    append_css();
    append(c.out, c.cfg.extra_head_html);
    c.out += "</head><body>";
    append_ink_filters();
    c.out += "<main><div class=\"ft-bleed\"></div><form method=\"post\" action=\"";
    c.out += escape_html(c.req_path.empty() ? "/" : c.req_path);
    c.out += "\">";
    c.out += "<h1 class=\"ft-title\">";
    c.out += escape_html(c.cfg.title ? c.cfg.title : "FTHT App");
    c.out += "</h1>";
}

void end() {
    using namespace internal;
    Context& c = ctx();
    if (!c.modal_label.empty() && !c.modal_rendered) {
        c.out += "<div class=\"ft-modal\"><section>";
        c.out += "<p class=\"ft-text\">";
        c.out += escape_html(c.modal_label);
        c.out += "</p><button name=\"_ftht_action\" value=\"_ftht_close_modal\">Close</button></section></div>";
    }
    append_toast_stack();
    c.out += "</form></main>";
    if (!c.client_update) {
        append_toast_script();
        append_client_script();
        c.out += "</body></html>";
    }
    debugf("end render html_bytes=%d status=%d", (int)c.out.size(), c.status_code);
    send_response(c.out);
    c.response_open = false;
    c.request_active = false;
}

void shutdown() {
    using namespace internal;
    Context& c = ctx();
    debugf("shutdown");
#if defined(ARDUINO) && defined(ESP32)
    if (c.net.server) {
        delete c.net.server;
        c.net.server = nullptr;
    }
#else
    close_socket(c.net.client);
    close_socket(c.net.server);
#if defined(_WIN32)
    if (c.net.wsa_started) {
        WSACleanup();
        c.net.wsa_started = false;
    }
#endif
#endif
}

const Config& config() { return internal::ctx().cfg; }
const char* url() { return internal::ctx().current_url.c_str(); }
const char* path() { return internal::ctx().req_path.c_str(); }
const char* method() { return internal::ctx().req_method.c_str(); }
bool is_post() { return internal::ctx().req_method == "POST"; }
const char* param(const char* name) { return internal::find_param(name); }
uint64_t make_login_password_hash(const char* password, const char* salt) {
    return internal::salted_password_hash(password ? password : "", salt ? salt : "");
}

void set_status(int code, const char* text) {
    internal::ctx().status_code = code;
    internal::ctx().status_text = text ? text : "";
}

void html(const char* markup) {
    internal::append(internal::ctx().out, markup);
}

void text(const char* label) {
    internal::ctx().out += "<p class=\"ft-text\"" + internal::next_style() + ">" + internal::escape_html(label ? label : "") + "</p>";
}

void text_wrapped(const char* value) {
    internal::ctx().out += "<p class=\"ft-wrap\"" + internal::next_style() + ">" + internal::escape_html(value ? value : "") + "</p>";
}

void separator() {
    internal::ctx().out += "<hr>";
}

void spacing(float px) {
    char buf[80];
    std::snprintf(buf, sizeof(buf), "<div style=\"height:%.1fpx\"></div>", px);
    internal::ctx().out += buf;
}

void toast(const char* message) {
    Toast t;
    t.message = message ? message : "";
    internal::queue_toast(t);
}

void toast(const Toast& value) {
    internal::queue_toast(value);
}

void toast_success(const char* message) {
    Toast t;
    t.type = ToastType::Success;
    t.message = message ? message : "";
    internal::queue_toast(t);
}

void toast_warning(const char* message) {
    Toast t;
    t.type = ToastType::Warning;
    t.message = message ? message : "";
    internal::queue_toast(t);
}

void toast_error(const char* message) {
    Toast t;
    t.type = ToastType::Error;
    t.message = message ? message : "";
    internal::queue_toast(t);
}

bool input(const char* label, char* buffer, int buffer_size, InputFlags flags, bool* enter_pressed) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && !(flags & InputFlags::ReadOnly) && ctx().disabled_depth == 0) {
        std::string value = posted;
        if (flags & InputFlags::CharsUppercase) {
            for (char& ch : value) ch = (char)std::toupper((unsigned char)ch);
        }
        if (buffer && std::strcmp(buffer, value.c_str()) != 0) {
            copy_to_buffer(buffer, buffer_size, value);
            changed = true;
            debugf("input changed id=%s label='%s' bytes=%d", id.c_str(), visible_label(label).c_str(), (int)value.size());
        }
    }
    if (enter_pressed) *enter_pressed = changed;
    begin_field(label, id);
    ctx().out += "<input name=\"" + id + "\" value=\"" + escape_html(buffer ? buffer : "") + "\"";
    ctx().out += (flags & InputFlags::Password) ? " type=\"password\"" : " type=\"text\"";
    if (flags & InputFlags::ReadOnly) ctx().out += " readonly";
    if (flags & InputFlags::CharsDecimal) ctx().out += " inputmode=\"decimal\" pattern=\"[0-9.\\-]*\"";
    if (flags & InputFlags::CharsHexadecimal) ctx().out += " pattern=\"[0-9a-fA-F]*\"";
    if (flags & InputFlags::CharsNoBlank) ctx().out += " pattern=\"\\S*\"";
    ctx().out += disabled_attr() + auto_submit_attr() + "></label>";
    return changed;
}

bool text_area(const char* label, char* buffer, int buffer_size, int rows) {
    return text_area_ex(label, buffer, buffer_size, rows, TextAreaFlags::Default);
}

bool text_area_ex(const char* label, char* buffer, int buffer_size, int rows, TextAreaFlags flags) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && !(flags & TextAreaFlags::ReadOnly) && ctx().disabled_depth == 0) {
        if (buffer && std::strcmp(buffer, posted) != 0) {
            copy_to_buffer(buffer, buffer_size, posted);
            changed = true;
            debugf("text_area changed id=%s label='%s' bytes=%d", id.c_str(), visible_label(label).c_str(), (int)std::strlen(posted));
        }
    }
    begin_field(label, id);
    char row_buf[32];
    std::snprintf(row_buf, sizeof(row_buf), " rows=\"%d\"", rows);
    ctx().out += "<textarea name=\"" + id + "\"" + row_buf;
    if (flags & TextAreaFlags::ReadOnly) ctx().out += " readonly";
    ctx().out += disabled_attr() + auto_submit_attr() + ">";
    ctx().out += escape_html(buffer ? buffer : "");
    ctx().out += "</textarea></label>";
    return changed;
}

void log_view(const char* label, const char* value, int rows, LogViewFlags) {
    using namespace internal;
    std::string id = widget_id(label);
    begin_field(label, id);
    char style[96];
    std::snprintf(style, sizeof(style), " style=\"max-height:%.1fpx\"", rows * 24.0f + 24.0f);
    ctx().out += "<pre class=\"ft-log\"";
    ctx().out += style;
    ctx().out += ">";
    ctx().out += escape_html(value ? value : "");
    ctx().out += "</pre></label>";
}

bool checkbox(const char* label, bool* value) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && value && ctx().disabled_depth == 0) {
        bool next = string_to_bool(posted);
        changed = (*value != next);
        *value = next;
        if (changed) debugf("checkbox changed id=%s label='%s' value=%d", id.c_str(), visible_label(label).c_str(), *value ? 1 : 0);
    }
    ctx().last_widget_id = id;
    ctx().out += "<label class=\"ft-check\"" + next_style() + ">";
    ctx().out += "<input type=\"hidden\" name=\"" + id + "\" value=\"0\">";
    ctx().out += "<input type=\"checkbox\" name=\"" + id + "\" value=\"1\"";
    if (value && *value) ctx().out += " checked";
    ctx().out += disabled_attr();
    ctx().out += auto_submit_attr();
    ctx().out += "><span>" + escape_html(visible_label(label)) + "</span></label>";
    return changed;
}

bool slider_float(const char* label, float* value, float min_v, float max_v) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && value && ctx().disabled_depth == 0) {
        float next = (float)std::atof(posted);
        next = std::max(min_v, std::min(max_v, next));
        changed = (*value != next);
        *value = next;
        if (changed) debugf("slider changed id=%s label='%s' value=%.6g", id.c_str(), visible_label(label).c_str(), *value);
    }
    ctx().last_widget_id = id;
    float current = value ? *value : min_v;
    float pct = max_v > min_v ? ((current - min_v) / (max_v - min_v)) * 100.0f : 0.0f;
    pct = std::max(0.0f, std::min(100.0f, pct));
    char value_buf[32];
    std::snprintf(value_buf, sizeof(value_buf), "%.2f", current);
    char* dot = std::strchr(value_buf, '.');
    if (dot) {
        char* end = value_buf + std::strlen(value_buf) - 1;
        while (end > dot && *end == '0') *end-- = '\0';
        if (end == dot) *end = '\0';
    }
    ctx().out += "<label class=\"ft-field\"" + next_style() + "><span class=\"ft-label\"><span>";
    ctx().out += escape_html(visible_label(label));
    ctx().out += "</span><output class=\"ft-range-value\">";
    ctx().out += escape_html(value_buf);
    ctx().out += "</output></span>";
    char buf[256];
    std::snprintf(buf, sizeof(buf),
                  "<input type=\"range\" name=\"%s\" min=\"%.6g\" max=\"%.6g\" step=\"any\" value=\"%.6g\" data-ftht-range=\"1\" style=\"--ft-range-pct:%.3f%%\"",
                  id.c_str(), min_v, max_v, current, pct);
    ctx().out += buf;
    ctx().out += disabled_attr() + auto_submit_attr() + "></label>";
    return changed;
}

bool button(const char* label) {
    return button(label, ColorRole::Button);
}

bool button(const char* label, ColorRole role) {
    using namespace internal;
    std::string id = widget_id(label);
    ctx().last_widget_id = id;
    std::string klass;
    if (role == ColorRole::Accent) klass = " class=\"accent\"";
    if (role == ColorRole::Warning) klass = " class=\"warning\"";
    if (role == ColorRole::Success) klass = " class=\"success\"";
    ctx().out += "<button type=\"submit\" name=\"_ftht_action\" value=\"" + id + "\"" + klass + next_style() + disabled_attr() + ">";
    ctx().out += escape_html(visible_label(label));
    ctx().out += "</button>";
    if (id == "_ftht_close_modal") close_modal();
    bool clicked = action_matches(id);
    if (clicked) debugf("button clicked id=%s label='%s'", id.c_str(), visible_label(label).c_str());
    return clicked;
}

bool button(const char* label, Color color) {
    using namespace internal;
    std::string id = widget_id(label);
    ctx().last_widget_id = id;
    std::string style = next_style();
    std::string custom = "background:" + color_css(color) + ";border-color:" + color_css(color) + ";";
    if (style.empty()) {
        style = " style=\"" + custom + "\"";
    } else {
        size_t end_quote = style.rfind('"');
        style.insert(end_quote == std::string::npos ? style.size() : end_quote, custom);
    }
    ctx().out += "<button type=\"submit\" name=\"_ftht_action\" value=\"" + id + "\"" + style + disabled_attr() + ">";
    ctx().out += escape_html(visible_label(label));
    ctx().out += "</button>";
    bool clicked = action_matches(id);
    if (clicked) debugf("button clicked id=%s label='%s' custom_color=1", id.c_str(), visible_label(label).c_str());
    return clicked;
}

bool dropdown(const char* label, const char* const* items, int count, int* selected, int) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && selected && ctx().disabled_depth == 0) {
        int next = std::atoi(posted);
        if (next >= 0 && next < count) {
            changed = (*selected != next);
            *selected = next;
            if (changed) debugf("dropdown changed id=%s label='%s' selected=%d", id.c_str(), visible_label(label).c_str(), *selected);
        }
    }
    begin_field(label, id);
    ctx().out += "<select name=\"" + id + "\"" + disabled_attr() + auto_submit_attr() + ">";
    for (int i = 0; i < count; ++i) {
        ctx().out += "<option value=\"" + std::to_string(i) + "\"";
        if (selected && *selected == i) ctx().out += " selected";
        ctx().out += ">" + escape_html(items && items[i] ? items[i] : "") + "</option>";
    }
    ctx().out += "</select></label>";
    return changed;
}

bool listbox(const char* label, const char* const* items, int count, int* selected, int visible_rows) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && selected && ctx().disabled_depth == 0) {
        int next = std::atoi(posted);
        if (next >= 0 && next < count) {
            changed = (*selected != next);
            *selected = next;
            if (changed) debugf("listbox changed id=%s label='%s' selected=%d", id.c_str(), visible_label(label).c_str(), *selected);
        }
    }
    begin_field(label, id);
    ctx().out += "<select name=\"" + id + "\" size=\"" + std::to_string(std::max(1, visible_rows)) + "\"" + disabled_attr() + auto_submit_attr() + ">";
    for (int i = 0; i < count; ++i) {
        ctx().out += "<option value=\"" + std::to_string(i) + "\"";
        if (selected && *selected == i) ctx().out += " selected";
        ctx().out += ">" + escape_html(items && items[i] ? items[i] : "") + "</option>";
    }
    ctx().out += "</select></label>";
    return changed;
}

bool radio_group(const char* label, const char* const* items, int count, int* selected, int columns) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && selected && ctx().disabled_depth == 0) {
        int next = std::atoi(posted);
        if (next >= 0 && next < count) {
            changed = (*selected != next);
            *selected = next;
            if (changed) debugf("radio_group changed id=%s label='%s' selected=%d", id.c_str(), visible_label(label).c_str(), *selected);
        }
    }
    begin_field(label, id);
    ctx().out += "<div class=\"ft-radio\" style=\"grid-template-columns:repeat(" + std::to_string(std::max(1, columns)) + ",minmax(0,1fr))\">";
    for (int i = 0; i < count; ++i) {
        ctx().out += "<label><input type=\"radio\" name=\"" + id + "\" value=\"" + std::to_string(i) + "\"";
        if (selected && *selected == i) ctx().out += " checked";
        ctx().out += disabled_attr() + auto_submit_attr() + "><span>" + escape_html(items && items[i] ? items[i] : "") + "</span></label>";
    }
    ctx().out += "</div></label>";
    return changed;
}

bool collapsing_header(const char* label, bool* open) {
    using namespace internal;
    bool is_open = !open || *open;
    ctx().out += "<p class=\"ft-text\" style=\"font-weight:700;margin-top:6px\">" + escape_html(visible_label(label)) + "</p>";
    return is_open;
}

bool tabs(const char* const* labels, int count, int* selected) {
    using namespace internal;
    std::string seed = "tabs";
    for (int i = 0; i < count; ++i) {
        seed += "##";
        seed += labels && labels[i] ? labels[i] : "";
    }
    std::string id = widget_id(seed.c_str());
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && selected && ctx().disabled_depth == 0) {
        int next = std::atoi(posted);
        if (next >= 0 && next < count) {
            changed = (*selected != next);
            *selected = next;
            if (changed) debugf("tabs changed id=%s selected=%d", id.c_str(), *selected);
        }
    }
    ctx().out += "<div class=\"ft-tabs\"" + next_style() + ">";
    for (int i = 0; i < count; ++i) {
        ctx().out += "<button type=\"submit\" name=\"" + id + "\" value=\"" + std::to_string(i) + "\"";
        if (selected && *selected == i) ctx().out += " class=\"sel\"";
        ctx().out += disabled_attr() + ">" + escape_html(labels && labels[i] ? labels[i] : "") + "</button>";
    }
    ctx().out += "</div>";
    return changed;
}

void row(int cols, std::function<void()> fn) {
    using namespace internal;
    ctx().out += "<div class=\"ft-row\" style=\"grid-template-columns:repeat(" + std::to_string(std::max(1, cols)) + ",minmax(0,1fr))\">";
    if (fn) fn();
    ctx().out += "</div>";
}

void row(std::initializer_list<float> weights, std::function<void()> fn) {
    using namespace internal;
    ctx().out += "<div class=\"ft-row\" style=\"grid-template-columns:";
    for (float w : weights) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), " minmax(0,%.3gfr)", std::max(0.01f, w));
        ctx().out += buf;
    }
    ctx().out += "\">";
    if (fn) fn();
    ctx().out += "</div>";
}

void scroll_area(const char* label, float height, std::function<void()> fn) {
    using namespace internal;
    ctx().out += "<section class=\"ft-scroll\" style=\"max-height:" + std::to_string((int)height) + "px\">";
    ctx().out += "<p class=\"ft-label\">" + escape_html(visible_label(label)) + "</p>";
    if (fn) fn();
    ctx().out += "</section>";
}

void set_next_width(float px) { internal::ctx().next.has_width = true; internal::ctx().next.width = px; }
void set_next_fill() { internal::ctx().next.fill = true; }
void set_next_percent(float pct) { internal::ctx().next.has_percent = true; internal::ctx().next.percent = pct; }
void set_next_limits(float min_px, float max_px) {
    internal::ctx().next.has_limits = true;
    internal::ctx().next.min_width = min_px;
    internal::ctx().next.max_width = max_px;
}
void set_next_align(Align align) { internal::ctx().next.has_align = true; internal::ctx().next.align = align; }

void open_modal(const char* label) {
    internal::ctx().modal_label = internal::visible_label(label);
    internal::debugf("modal opened label='%s'", internal::ctx().modal_label.c_str());
}

bool modal(const char* label, std::function<void()> fn) {
    using namespace internal;
    if (action_matches("_ftht_close_modal")) close_modal();
    if (ctx().modal_label != visible_label(label)) return false;
    ctx().modal_rendered = true;
    ctx().out += "<div class=\"ft-modal\"><section>";
    if (fn) fn();
    ctx().out += "</section></div>";
    return true;
}

void close_modal() {
    if (!internal::ctx().modal_label.empty()) {
        internal::debugf("modal closed label='%s'", internal::ctx().modal_label.c_str());
    }
    internal::ctx().modal_label.clear();
}

void begin_disabled() {
    internal::ctx().disabled_depth++;
}

void end_disabled() {
    if (internal::ctx().disabled_depth > 0) internal::ctx().disabled_depth--;
}

void tooltip(const char* text) {
    using namespace internal;
    if (!text || ctx().last_widget_id.empty()) return;
    ctx().out += "<small class=\"ft-wrap\">" + escape_html(text) + "</small>";
}

void request_focus(const char*) {}

float calc_text_width(const char* value) {
    return value ? (float)std::strlen(value) * get_style().font_size * 0.55f : 0.0f;
}

float calc_text_height(const char* value, float wrap_width) {
    if (!value) return get_style().font_size * 1.45f;
    float line_w = std::max(1.0f, wrap_width);
    float chars_per_line = std::max(1.0f, line_w / (get_style().font_size * 0.55f));
    int lines = 1;
    int col = 0;
    for (const char* p = value; *p; ++p) {
        if (*p == '\n' || ++col >= (int)chars_per_line) {
            lines++;
            col = 0;
        }
    }
    return lines * get_style().font_size * 1.45f;
}

} // namespace ftht

#endif // FTHT_IMPLEMENTATION
