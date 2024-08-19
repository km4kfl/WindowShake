#pragma once
#include <Windows.h>
#include <queue>

struct Move {
    int ox;
    int oy;
};

class LastPosition {
    bool known;
    int x;
    int y;
public:
    LastPosition();
    LastPosition(int x, int y);
    int GetX();
    int GetY();
    bool IsUnknown();
    static LastPosition Unknown();
    static LastPosition Known(int x, int y);
};

class WindowShakeData {
private:
    HWND hwnd;
    std::queue<Move> moves;
    uint8_t max_offset;
    LastPosition last_pos;
    LastPosition origin;

    void FillMoves();
public:
    WindowShakeData(HWND hwnd, uint8_t max_offset);
    bool Work();
};