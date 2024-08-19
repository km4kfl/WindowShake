#pragma once
#include "WindowShakeData.h"
#include <windows.h>
#include <vector>
#include <unordered_map>
#include <optional>

class ShakerOfWindows {
private:
    std::unordered_map<HWND, WindowShakeData> shake_data;
    std::vector<HWND> win_que;
    uint8_t max_offset;

    bool collect_windows;

    int win_que_ndx;

    bool HandleWindow(HWND hwnd);

public:
    ShakerOfWindows(uint8_t max_offset);
    void PauseNewWindowCollection();
    void UnpauseNewWindowCollection();
    std::optional<std::vector<WCHAR>> ShakeWindows();
};
