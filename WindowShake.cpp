// WindowShake.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowShake.h"

#include "AppExceptions.h"
#include "SmartHandleClass.h"

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

   SetTimer(hWnd, 100, 250, NULL);

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

class WindowShakeData {
private:
    //WindowShakeData& operator=(WindowShakeData& other) = delete;
    //WindowShakeData(const WindowShakeData& other) = delete;
    //WindowShakeData& operator=(WindowShakeData&& other) = default;

    HWND hwnd;
    std::queue<Move> moves;
    uint8_t max_offset;

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

        int cur_x = max_offset;
        int cur_y = max_offset;

        map[cur_y * max_move_width + cur_x] = 1;

        int failure_count = 0;

        Move short_list[8];
        uint8_t short_count;

        while (moves.size() < 64) {
            short_count = 0;

            for (int oy = -1; oy < 2; ++oy) {
                for (int ox = -1; ox < 2; ++ox) {
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
            moves.push(Move {
                .ox = move_x,
                .oy = move_y,
            });
        }
    }

public:
    //WindowShakeData() : hwnd(NULL), max_offset(16) {
    //}

    WindowShakeData(HWND hwnd, uint8_t max_offset) : hwnd(hwnd), max_offset(max_offset) {
    }

    void Work() {
        if (moves.size() == 0) {
            FillMoves();
        }

        RECT rect;

        GetWindowRect(hwnd, &rect);

        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        const Move move = moves.front();
        moves.pop();

        const int new_x = move.ox + rect.left;
        const int new_y = move.oy + rect.top;

        SetWindowPos(
            hwnd, NULL, new_x, new_y, width, height, 
            SWP_NOSIZE | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS
        );

        //MoveWindow(hwnd, new_x, new_y, width, height, FALSE);
    }
};

class ShakerOfWindows {
private:
    std::unordered_map<HWND, WindowShakeData> shake_data;

    void HandleWindow(HWND hwnd) {
        if (shake_data.count(hwnd) == 0) {
            shake_data.emplace(hwnd, WindowShakeData(hwnd, 16));
        }

        shake_data.at(hwnd).Work();
    }

public:
    ShakerOfWindows() {
    }

    void ShakeWindows() {
        auto win_handles = std::vector<HWND>();

        BOOL_THROW(EnumWindows(_ShakeWindows_EnumWindowCb, (LPARAM)&win_handles));

        for (auto some_hwnd : win_handles) {
            HandleWindow(some_hwnd);
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
