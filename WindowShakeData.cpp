#include "WindowShakeData.h"
#include "AppExceptions.h"

#include <windows.h>
#include <random>

LastPosition::LastPosition() : x(0), y(0), known(0) {
}

LastPosition::LastPosition(int x, int y) : x(x), y(y), known(1) {

}

int LastPosition::GetX() {
    return x;
}

int LastPosition::GetY() {
    return y;
}

bool LastPosition::IsUnknown() {
    return !known;
}

LastPosition LastPosition::Unknown() {
    return LastPosition();
}

LastPosition LastPosition::Known(int x, int y) {
    return LastPosition(x, y);
}

void WindowShakeData::FillMoves() {
    std::random_device hw_random;;
    std::mt19937 pesudo_random(hw_random());

    if (moves.size() != 0) {
        throw ProcessFailure("not implemented working from existing move list");
    }

    int max_move_width = max_offset * 2 + 1;

    size_t map_byte_size = max_move_width * max_move_width;

    auto map = std::unique_ptr<uint8_t[]>(new uint8_t[map_byte_size]);

    memset(map.get(), 0, map_byte_size);

    //int cur_x = max_offset;
    //int cur_y = max_offset;

    int last_pos_x = last_pos.GetX();
    int last_pos_y = last_pos.GetY();

    int cur_x = (last_pos_x - origin.GetX()) + max_offset;
    int cur_y = (last_pos_y - origin.GetY()) + max_offset;

    map[cur_y * max_move_width + cur_x] = 1;

    int failure_count = 0;

    const int max_move_inc = 6;
    const int short_list_size = (max_move_inc * 2 + 1) * (max_move_inc * 2 + 1) - 1;
    Move short_list[short_list_size];
    uint8_t short_count;

    int off_x = 0;
    int off_y = 0;

    while (moves.size() < 60 * 5) {
        short_count = 0;

        for (int oy = -max_move_inc; oy < (max_move_inc + 1); ++oy) {
            for (int ox = -max_move_inc; ox < (max_move_inc + 1); ++ox) {
                if (ox == 0 && oy == 0) {
                    continue;
                }

                int this_x = cur_x + ox;
                int this_y = cur_y + oy;

                if (this_x < 0 || this_x >= max_move_width) {
                    continue;
                }

                if (this_y < 0 || this_y >= max_move_width) {
                    continue;
                }

                int ondx = this_y * max_move_width + this_x;

                if (map[ondx] == 0) {
                    if (short_count >= short_list_size) {
                        throw ProcessFailure("short list overrun");
                    }
                    short_list[short_count].ox = ox;
                    short_list[short_count].oy = oy;
                    ++short_count;
                }
            }
        }

        if (short_count == 0) {
            break;
        }

        int new_move_ndx = pesudo_random() % short_count;
        int move_x = short_list[new_move_ndx].ox;
        int move_y = short_list[new_move_ndx].oy;
        int new_x = move_x + cur_x;
        int new_y = move_y + cur_y;
        int new_ndx = new_y * max_move_width + new_x;
        cur_x = new_x;
        cur_y = new_y;
        map[new_ndx] = 1;
        off_x += move_x;
        off_y += move_y;

        moves.push(Move{
            .ox = off_x + last_pos_x,
            .oy = off_y + last_pos_y,
            });
    }
}

WindowShakeData::WindowShakeData(HWND hwnd, uint8_t max_offset) : hwnd(hwnd), max_offset(max_offset) {
}

bool WindowShakeData::Work() {
    RECT rect;
    GetWindowRect(hwnd, &rect);

    if (last_pos.IsUnknown()) {
        last_pos = LastPosition(rect.left, rect.top);
        origin = LastPosition(rect.left, rect.top);
    }

    float lp_delta_x = (float)last_pos.GetX() - (float)rect.left;
    float lp_delta_y = (float)last_pos.GetY() - (float)rect.top;
    float lp_delta = std::sqrtf(lp_delta_x * lp_delta_x + lp_delta_y * lp_delta_y);

    if (lp_delta > 128.0f) {
        last_pos = LastPosition(rect.left, rect.top);
        origin = LastPosition(rect.left, rect.top);
        while (moves.size() > 0) {
            moves.pop();
        }
    }

    if (moves.size() == 0) {
        FillMoves();
    }

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    const Move move = moves.front();
    moves.pop();

    const int new_x = move.ox; // +rect.left;
    const int new_y = move.oy; // +rect.top;

    last_pos = LastPosition(new_x, new_y);

    /*
    *  I don't think I can wiggle enough to keep it from being predicted
    *  or easily cracked. I don't think I can get a nice 64x64 wiggle space
    *  without making things unreadable and such and so forth if this below
    *  even worked.
    *
    HDC hdc;
    if ((hdc = GetDC(hwnd)) != NULL) {
        XFORM xform;
        memset(&xform, 0, sizeof(xform));
        xform.eDx = 0;
        xform.eDy = 0;
        SetWorldTransform(hdc, &xform);
    }
    */

    return SetWindowPos(
        hwnd, NULL,
        new_x, new_y,
        width, height,
        SWP_NOSIZE |
        SWP_NOSENDCHANGING |
        SWP_NOREDRAW |
        SWP_NOZORDER |
        SWP_NOOWNERZORDER |
        SWP_NOACTIVATE |
        SWP_ASYNCWINDOWPOS
    ) == TRUE;
}
