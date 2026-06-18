#define FTHT_IMPLEMENTATION
#include "../ftht.hpp"

#include <cstdio>
#include <cstring>

static void append_line(char* buffer, int cap, const char* line) {
    if (!buffer || cap <= 0 || !line) return;
    int len = (int)std::strlen(buffer);
    if (len > 0 && len < cap - 1) {
        buffer[len++] = '\n';
        buffer[len] = '\0';
    }
    if (len >= cap - 1) return;
    int room = cap - len - 1;
    int add = (int)std::strlen(line);
    if (add > room) add = room;
    if (add > 0) {
        std::memcpy(buffer + len, line, (size_t)add);
        buffer[len + add] = '\0';
    }
}

int main() {
    ftht::Config cfg;
    cfg.title = "FTHT Showcase";
    cfg.port = 8080;

    if (!ftht::create_server(cfg)) return 1;

    char username[64] = "operator";
    char port[16] = "8080";
    char notes[1024] =
        "This page is generated from immediate-mode C++ calls.\n"
        "Change a field and the browser posts back to the tiny embedded server.";
    char logs[2048] =
        "[boot] server started\n"
        "[boot] waiting for browser requests";

    bool safe_mode = false;
    float blend = 0.45f;
    int env_sel = 1;
    int tab_sel = 0;

    static const char* tabs[] = { "Controls", "Logs" };
    static const char* envs[] = { "Local", "Staging", "Production" };

    while (ftht::pump()) {
        ftht::begin();

        ftht::text_wrapped(
            "FTHT keeps a tiny immediate-mode loop, but renders HTML and serves it over HTTP. "
            "It is intended for small control panels on PCs, ESP32 boards, and similar devices.");
        ftht::separator();

        ftht::tabs(tabs, 2, &tab_sel);
        ftht::separator();

        if (tab_sel == 0) {
            ftht::row({2.0f, 1.0f}, [&]() {
                if (ftht::input("Username", username, sizeof(username))) {
                    append_line(logs, sizeof(logs), "[form] username changed");
                }
                ftht::input("Port", port, sizeof(port), ftht::InputFlags::CharsDecimal);
            });

            ftht::row({1.0f, 1.0f, 1.0f}, [&]() {
                ftht::dropdown("Environment", envs, 3, &env_sel, 3);
                ftht::checkbox("Safe mode", &safe_mode);
                ftht::slider_float("Blend", &blend, 0.0f, 1.0f);
            });

            ftht::text_area_ex("Notes", notes, sizeof(notes), 7, ftht::TextAreaFlags::WordWrap);

            ftht::row(3, [&]() {
                if (ftht::button("Ping", ftht::ColorRole::Accent)) {
                    append_line(logs, sizeof(logs), "[action] ping clicked");
                    ftht::toast_success("Ping sent");
                }
                if (ftht::button("Warn", ftht::ColorRole::Warning)) {
                    append_line(logs, sizeof(logs), "[action] warning clicked");
                    ftht::toast_warning("Warning toast queued");
                }
                if (ftht::button("Open modal")) {
                    ftht::open_modal("Confirm action");
                    ftht::toast("Modal opened");
                }
            });
        } else {
            ftht::log_view("Output", logs, 12,
                           ftht::LogViewFlags::WordWrap | ftht::LogViewFlags::AutoScrollBottom);
            if (ftht::button("Append sample")) {
                append_line(logs, sizeof(logs), "[log] sample line appended");
            }
        }

        if (ftht::modal("Confirm action", [&]() {
                ftht::text_wrapped("This modal is still just immediate-mode C++ rendered into HTML.");
                ftht::row(2, [&]() {
                    if (ftht::button("Cancel")) ftht::close_modal();
                    if (ftht::button("Confirm", ftht::ColorRole::Success)) {
                        append_line(logs, sizeof(logs), "[modal] action confirmed");
                        ftht::toast_success("Action confirmed");
                        ftht::close_modal();
                    }
                });
            })) {
        }

        ftht::end();
    }

    ftht::shutdown();
    return 0;
}
