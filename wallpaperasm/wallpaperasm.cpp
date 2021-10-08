// wallpaperasm.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "wallpaperasm.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND hWinWorkerW;
HWND hWinMain;
DWORD idTimerRedraw;
HBITMAP hBitmapCache;
HDC hDCCache;
WCHAR szFileName[MAX_PATH];
// bitmap
LPVOID lpDIBBits;
DWORD dwDIBHeight;
DWORD dwDIBWidth;
DWORD dwDIBBitCount;
WCHAR szTestBuffer[1024];

// 此代码模块中包含的函数的前向声明:
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

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WALLPAPERASM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WALLPAPERASM));

    MSG msg;

    // 主消息循环:
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
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WALLPAPERASM));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WALLPAPERASM);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

BOOL CALLBACK SearchWorkerW(HWND hwnd, LPARAM lParam)
{
    if (!FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL))
    {
        return TRUE;
    }
    hWinWorkerW = FindWindowEx(NULL, hwnd, L"WorkerW", NULL);
    return FALSE;
}

HBITMAP GetWallpaperhBmp(LPWSTR lpszFileName) {
    WORD wFileType;
    DWORD dwBytesRead;
    DWORD dwFileSizeLow;
    DWORD dwFileSizeHigh;
    HANDLE hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(hWinMain, L"Failed to open bmp file!", NULL, MB_OK | MB_ICONEXCLAMATION);
        return NULL;
    }
    if (!ReadFile(hFile, &wFileType, 2, &dwBytesRead, 0))
    {
        MessageBox(hWinMain, L"Not a bmp file!", NULL, MB_OK | MB_ICONEXCLAMATION);
        return NULL;
    }
    if (wFileType != 0x4d42) 
    {
        MessageBox(hWinMain, L"Not a bmp file!", NULL, MB_OK | MB_ICONEXCLAMATION);
        return NULL;
    }
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    dwFileSizeLow = GetFileSize(hFile, &dwFileSizeHigh);
    HANDLE hHeap = HeapCreate(NULL, dwFileSizeLow, NULL);
    if (hHeap == NULL) 
    {
        MessageBox(hWinMain, L"Failed to allocate memory!", NULL, MB_OK | MB_ICONEXCLAMATION);
        return NULL;
    }
    LPVOID lpReadFileBuffer = HeapAlloc(hHeap, NULL, dwFileSizeLow);
    ReadFile(hFile, lpReadFileBuffer, dwFileSizeLow, &dwBytesRead, 0);
    CloseHandle(hFile);

    BITMAPFILEHEADER* pstBMFH = (BITMAPFILEHEADER*)lpReadFileBuffer;
    dwOffsetBits = pstBMFH->bfOffBits;
    BITMAPINFO* pstBMI = (BITMAPINFO*)lpReadFileBuffer + pstBMFH->bfSize;
    BITMAPINFOHEADER* pstBMH = &(pstBMI)->bmiHeader;
    if (pstBMH->biSize == sizeof BITMAPCOREHEADER)
    {
        dwDIBHeight = pstBMH->biWidth;
        dwDIBWidth = pstBMH->biHeight;
    }
    else
    {
        dwDIBHeight = ((BITMAPCOREHEADER*)pstBMH)->bcHeight;
        dwDIBWidth = ((BITMAPCOREHEADER*)pstBMH)->bcWidth;
    }

    HDC hDCWorkerW = GetDC(hWinWorkerW);
    HDC hDCCompat = CreateCompatibleDC(hDCWorkerW);
    HBITMAP hBmpCompat = CreateCompatibleBitmap(hDCWorkerW, dwDIBWidth, dwDIBHeight);
    SelectObject(hDCCompat, hBmpCompat);
    ReleaseDC(hWinWorkerW, hDCWorkerW);
    SetDIBitsToDevice(hDCCompat, 0, 0, dwDIBWidth, dwDIBHeight, 0, 0, 0, dwDIBHeight, pstBMI, )
}
//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PDWORD_PTR dwMsgResult = NULL;
    OPENFILENAME stOFN;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_BROWSE:
                if (!idTimerRedraw)
                {
                    KillTimer(NULL, idTimerRedraw);
                    idTimerRedraw = 0;
                    DeleteObject(hBitmapCache);
                    DeleteDC(hDCCache);
                }
                RtlZeroMemory(&stOFN, sizeof stOFN);
                stOFN.lStructSize = sizeof stOFN;
                stOFN.hwndOwner = hWinMain;
                stOFN.lpstrFilter = L"Bitmap\0*.bmp\0";
                stOFN.lpstrFile = (LPWSTR)&szFileName;
                stOFN.nMaxFile = MAX_PATH;
                stOFN.Flags = OFN_FILEMUSTEXIST or OFN_PATHMUSTEXIST;
                if (GetOpenFileName(&stOFN))
                {
                    return 0;
                }
                MessageBox(hWinMain, (LPWSTR)&szFileName, NULL, MB_OK);
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
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CREATE:
        hWinMain = hWnd;
        SendMessageTimeout(FindWindow(L"Progman", NULL), WM_SPAWN_WORKERW, NULL, NULL, SMTO_NORMAL, 1000, dwMsgResult);
        EnumWindows(SearchWorkerW, NULL);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
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
