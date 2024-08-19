#include "ShakerOfWindows.h"
#include <strsafe.h>
#include <cwchar>

bool ShakerOfWindows::HandleWindow(HWND hwnd) {
    if (shake_data.count(hwnd) == 0) {
        shake_data.emplace(hwnd, WindowShakeData(hwnd, max_offset));
    }

    return shake_data.at(hwnd).Work();
}

ShakerOfWindows::ShakerOfWindows(uint8_t max_offset) : win_que_ndx(0), max_offset(max_offset), collect_windows(true) {
    for (int x = 0; x < 1000; ++x) {
        win_que.push_back(NULL);
    }
}

void ShakerOfWindows::PauseNewWindowCollection() {
    collect_windows = false;
}

void ShakerOfWindows::UnpauseNewWindowCollection() {
    collect_windows = true;
}

std::optional<std::vector<WCHAR>> ShakerOfWindows::ShakeWindows() {
    /*
    auto win_handles = std::vector<HWND>();

    BOOL_THROW(EnumWindows(_ShakeWindows_EnumWindowCb, (LPARAM)&win_handles));

    for (auto some_hwnd : win_handles) {
        HandleWindow(some_hwnd);
    }
    */
    HWND cur_hwnd = GetForegroundWindow();

    bool hwnd_exists = false;

    for (int x = 0; x < win_que.size(); ++x) {
        auto hwnd = win_que.at(x);

        if (hwnd == cur_hwnd) {
            hwnd_exists = true;
        }

        if (!HandleWindow(hwnd)) {
            win_que[x] = NULL;
        }
    }

    if (cur_hwnd == GetDesktopWindow()) {
        return {};
    }

    if (cur_hwnd == GetShellWindow()) {
        return {};
    }

    if (hwnd_exists == false && collect_windows) {
        std::vector<WCHAR> title(255);
        size_t title_size = 0;
        GetWindowText(cur_hwnd, title.data(), (int)title.size());

        StringCchLengthW(title.data(), 255, &title_size);

        bool is_notepad = wcsstr(title.data(), L"Notepad") != NULL;

        if (title_size > 0 && !is_notepad) {
            win_que_ndx = (win_que_ndx + 1) % win_que.size();
            win_que[win_que_ndx] = cur_hwnd;
            return title;
        }
    }

    return {};
}

