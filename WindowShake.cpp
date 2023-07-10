// WindowShake.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowShake.h"

#include "AppExceptions.h"
#include "SmartHandleClass.h"

#include <strsafe.h>
#include <queue>
#include <vector>
#include <unordered_map>
#include <random>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSHAKE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSHAKE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSHAKE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSHAKE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   SetTimer(hWnd, 100, 1000 / 60, NULL);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

static BOOL _ShakeWindows_EnumWindowCb(HWND hwnd, LPARAM lparam) {
    auto win_handles = (std::vector<HWND>*)lparam;
    win_handles->push_back(hwnd);
    return TRUE;
}

struct Move {
    int ox;
    int oy;
};

class LastPosition {
    bool known;
    int x;
    int y;
public:

    LastPosition() : x(0), y(0), known(0) {
    }

    LastPosition(int x, int y) : x(x), y(y), known(1) {

    }

    int GetX() {
        return x;
    }

    int GetY() {
        return y;
    }

    bool IsUnknown() {
        return !known;
    }

    static LastPosition Unknown() {
        return LastPosition();
    }

    static LastPosition Known(int x, int y) {
        return LastPosition(x, y);
    }
};

class WindowShakeData {
private:
    //WindowShakeData& operator=(WindowShakeData& other) = delete;
    //WindowShakeData(const WindowShakeData& other) = delete;
    //WindowShakeData& operator=(WindowShakeData&& other) = default;

    HWND hwnd;
    std::queue<Move> moves;
    uint8_t max_offset;
    LastPosition last_pos;
    LastPosition origin;

    void FillMoves() {
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

        const int max_move_inc = 2;
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

            moves.push(Move {
                .ox = off_x + last_pos_x,
                .oy = off_y + last_pos_y,
            });
        }
    }

public:
    //WindowShakeData() : hwnd(NULL), max_offset(16) {
    //}

    WindowShakeData(HWND hwnd, uint8_t max_offset) : hwnd(hwnd), max_offset(max_offset) {
    }

    bool Work() {
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
};

class ShakerOfWindows {
private:
    std::unordered_map<HWND, WindowShakeData> shake_data;
    std::vector<HWND> win_que;

    int win_que_ndx;

    bool HandleWindow(HWND hwnd) {
        if (shake_data.count(hwnd) == 0) {
            shake_data.emplace(hwnd, WindowShakeData(hwnd, 8));
        }

        return shake_data.at(hwnd).Work();
    }

public:
    ShakerOfWindows() : win_que_ndx(0) {
        for (int x = 0; x < 5; ++x) {
            win_que.push_back(NULL);
        }
    }

    void ShakeWindows() {
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
            return;
        }

        if (cur_hwnd == GetShellWindow()) {
            return;
        }

        if (hwnd_exists == false) {
            WCHAR title[255];
            size_t title_size = 0;
            memset(&title, 0, sizeof(title));
            GetWindowText(cur_hwnd, &title[0], sizeof(title));
            StringCchLengthW(&title[0], 255, &title_size);
            
            if (title_size > 0) {
                win_que_ndx = (win_que_ndx + 1) % win_que.size();
                win_que[win_que_ndx] = cur_hwnd;
            }
        }
    }
};


static ShakerOfWindows _g_shaker;

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
        switch (wParam) {
            case 100:
            {
                _g_shaker.ShakeWindows();
                break;
            }
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
