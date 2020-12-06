#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif
#include "resource.h"

#define CROSS 1
#define NOUGHT 2
#define EMPTY 0
#define GAME_OVER 5
#define OUTSIDE -2;

#include <iostream>
#include <tchar.h>
#include <windows.h>
#include "windowsx.h"

/// Globals
// game board
int board[9] = { EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY };
int turn = CROSS;
// drawing globals
HBRUSH p1brush, p2brush, bBrush;
HICON p1icon, p2icon;
int p1Score = 0, p2Score = 0;
auto p1Color = RGB(0xFF, 0, 0), p2Color = RGB(0, 0x84, 0xFF), boardColor = RGB(0xFF, 0xFF, 0xFF);
int cellSize = 100;

// program
HINSTANCE hInst;
/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    hInst = hThisInstance; // store instance;
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("TicTacToe"),       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void RestartGame(){
    turn = CROSS;
    for (auto i = 0; i < 9; i++) board[i] = EMPTY;
}

void Force_WM_PAINT(const HWND& hwnd){
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
}

void LoadResources(const HWND& hwnd){
    p1brush = CreateSolidBrush(p1Color);
    p2brush = CreateSolidBrush(p2Color);
    bBrush = CreateSolidBrush(boardColor);
    p1icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CROSS));
    p2icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NOUGHT));
    Force_WM_PAINT(hwnd);
}

int GetClientWidth(const HWND& hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    return clientRect.right - clientRect.left;
}

int GetClientHeight(const HWND& hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    return clientRect.bottom - clientRect.top;
}

void CreateBoard(const HWND& hwnd, RECT* board){
    int width = GetClientWidth(hwnd);
    int height = GetClientHeight(hwnd);

    board->left = (width - (cellSize * 3)) * 0.5;
    board->top = (height - (cellSize * 3)) * 0.5;
    board->right = board->left + cellSize * 3;
    board->bottom = board->top + cellSize * 3;
}

void DrawPlayerInfo(const HWND& hwnd, const HDC& hdc){
    int width = GetClientWidth(hwnd);
    //p1
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, p1Color);
    TextOut(hdc, 20, 20, "Player 1", 8);
    DrawIcon(hdc, 40, 40, p1icon);
    //p2
    SetTextColor(hdc, p2Color);
    TextOut(hdc, width - (8 * 9), 20, "Player 2", 8);
    DrawIcon(hdc, width - (8 * 9), 40, p2icon);
}

void DrawLine(const HDC& hdc, const int& xOrigin, const int& yOrigin, const int& xEnd, const int& yEnd)
{
	MoveToEx(hdc, xOrigin, yOrigin, NULL);
	LineTo(hdc, xEnd, yEnd);
}

void DrawBoard(const HWND& hwnd, const HDC& hdc){
    RECT board;
    CreateBoard(hwnd, &board);
    FillRect(hdc, &board, bBrush);
    //draw separating lines
    for (int i = 0; i < 4; ++i)
    {
        DrawLine(hdc, board.left + cellSize * i, board.top, board.left + cellSize * i, board.bottom);
        DrawLine(hdc, board.left, board.top + cellSize * i, board.right, board.top + cellSize * i);
    }
}

void DrawPlayerTurn(HWND hwnd, HDC hdc){
    SetBkMode(hdc, TRANSPARENT);
    RECT client;
    GetClientRect(hwnd, &client);
    auto color = turn == GAME_OVER ? RGB(0xFF,0xFF,0xFF) : turn == CROSS ? p1Color : p2Color;
    auto text =  turn == GAME_OVER ? "game over" : turn == CROSS ? "player 1" : "player 2";
    SetTextColor(hdc, color);

    DrawText(hdc, text, 9, &client, DT_CENTER);
}


void Draw(const HWND& hwnd){
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    DrawBoard(hwnd, hdc);
    //Texts
    DrawPlayerInfo(hwnd, hdc);
    DrawPlayerTurn(hwnd, hdc);
    EndPaint(hwnd, &ps);
}

int GetCellValue(const HWND& hwnd, const int& x, const int &y){
    RECT board;
    CreateBoard(hwnd, &board);
    if (PtInRect(&board, {x, y})){
        int column = (x - board.left) / cellSize;
        int row = (y - board.top) / cellSize;
        return column + row * 3;
    }
    return OUTSIDE;
}

void ProccessClick(const HWND& hwnd, const LPARAM& lParam){
    if (turn == GAME_OVER){
        return;
    }
    //windowsx.h
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    int cell = GetCellValue(hwnd, x, y);
    std::cout << cell << std::endl;
}


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:
            LoadResources(hwnd);
            break;
        case WM_PAINT:
            Draw(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        case WM_LBUTTONDBLCLK:
            ProccessClick(hwnd, lParam);
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
