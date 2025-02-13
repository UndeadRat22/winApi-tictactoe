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
#define TIE 10
#define OUTSIDE -2

#include <iostream>
#include <tchar.h>
#include <windows.h>
#include "windowsx.h"



/// Globals
//for dll
typedef int (__cdecl *MYPROC)(int*, int*);

// game board
int boardValues[9] = { EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY };
int turn = CROSS;
// drawing globals
HBRUSH p1brush, p2brush, bBrush;
HICON p1icon, p2icon;
int p1Score = 0, p2Score = 0;
auto p1Color = RGB(0xFF, 0, 0), p2Color = RGB(0, 0x84, 0xFF), boardColor = RGB(0xFF, 0xFF, 0xFF);
int cellSize = 100;

// program
HINSTANCE hInst;
HINSTANCE hInstLib;
/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");
MYPROC externVictorFunction;

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    hInstLib = LoadLibrary("boardUtils\\bin\\Debug\\boardUtils.dll");
    if (hInstLib == NULL){
        std::cerr << "No boardUtils library found" << std::endl;
        exit(-1);
    }

    externVictorFunction = (MYPROC) GetProcAddress(hInstLib, "GetVictor");
    if (externVictorFunction == NULL){
        std::cerr << "No GetVictor function found" << std::endl;
        exit(-1);
    }

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
           600,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           LoadMenu(NULL, MAKEINTRESOURCE(IDC_TICTACTOE)),
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
    for (auto i = 0; i < 9; i++) boardValues[i] = EMPTY;
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

void DrawPlayerTurn(const HWND &hwnd, const HDC& hdc){
    SetBkMode(hdc, TRANSPARENT);
    RECT client;
    GetClientRect(hwnd, &client);
    client.bottom = 17;
    client.left = 40;
    client.right = client.right - 40;
    FillRect(hdc, &client, (HBRUSH)COLOR_BACKGROUND);
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

int GetCellIndex(const HWND& hwnd, const int& x, const int &y){
    RECT board;
    CreateBoard(hwnd, &board);
    if (PtInRect(&board, {x, y})){
        int column = (x - board.left) / cellSize;
        int row = (y - board.top) / cellSize;
        return column + row * 3;
    }
    return OUTSIDE;
}

RECT GetCellRect(const HWND& hWnd, const int& index)
{
	RECT rcBoard;
	CreateBoard(hWnd, &rcBoard);
    int row = index / 3;
    int col = index % 3;
    RECT rect;
    rect.left = rcBoard.left + col * cellSize +1;
    rect.top = rcBoard.top + row * cellSize + 1;

    rect.right = rect.left + cellSize - 1;
    rect.bottom = rect.top + cellSize - 1;

    return rect;
}

void DrawIconCentered(const HDC& hdc, const RECT& rect, const HICON& hIcon)
{
	const int ICON_WIDTH = GetSystemMetrics(SM_CXICON);
	const int ICON_HEIGHT = GetSystemMetrics(SM_CYICON);
    int left = rect.left + ((rect.right - rect.left) - ICON_WIDTH) / 2;
    int top = rect.top + ((rect.bottom - rect.top) - ICON_HEIGHT) / 2;
    DrawIcon(hdc, left, top, hIcon);
}

int GetVictor(int winningCells[3])
{
    return externVictorFunction(boardValues, winningCells);
}

void AdvanceTurn(const HWND &hwnd, const HDC& hdc){
    turn = turn == CROSS ? NOUGHT : CROSS;
    int cells[3];
    int victor = GetVictor(cells);
    if (victor != EMPTY) {
        turn = GAME_OVER;
    }
    DrawPlayerTurn(hwnd, hdc);
    if (turn != GAME_OVER){
        return;
    }
    //process GAME_OVER state:
    //draw line if victor exists
    if (victor != TIE){
        RECT startRect = GetCellRect(hwnd, cells[0]);
        RECT endRect = GetCellRect(hwnd, cells[2]);
        int halfSize = cellSize / 2;
        DrawLine(hdc, startRect.right - halfSize, startRect.bottom - halfSize, endRect.left + halfSize, endRect.top + halfSize);
     }
    //show message
    auto message = victor == TIE ? "A tie!" : victor == CROSS ? "Player 1 won!" : "Player 2 won!";
    MessageBox(hwnd, message, "gameover", MB_OK);
}

void ProccessClick(const HWND& hwnd, const LPARAM& lParam){
    if (turn == GAME_OVER){
        return;
    }
    //windowsx.h
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    int index = GetCellIndex(hwnd, x, y);
    if (index == OUTSIDE){
        return;
    }
    int cellValue = boardValues[index];
    if (cellValue != EMPTY){
        return;
    }
    //> game not over, selected cell is empty;
    boardValues[index] = turn;
    HDC hdc = GetDC(hwnd);
    RECT cellRect = GetCellRect(hwnd, index);
    DrawIconCentered(hdc, cellRect, turn == CROSS ? p1icon : p2icon);
    AdvanceTurn(hwnd, hdc);
}

void NewGame(const HWND& hwnd){
    int result = MessageBox(hwnd, "Start a new game?", "Reset", MB_YESNO | MB_ICONQUESTION);
    if (result != IDYES) return;
    RestartGame();
    Force_WM_PAINT(hwnd);
    Draw(hwnd);
}

void SaveGameDataToFile(TCHAR lpszFileName[]){
	char output[9];
	for (int i = 0; i < 9; i++)
	{
	    //convert int to 'char' by performing ascii magic
		output[i] = boardValues[i] + '0';
	}

	FILE *fp;
	fp = _tfopen(lpszFileName, TEXT("wb"));
	fwrite(output, sizeof(char), sizeof(output), fp);
	fclose(fp);
}

bool GetGameDataFromFile(TCHAR lpszFileName[], char *gameData){
    FILE *fp;
	fp = _tfopen(lpszFileName, TEXT("rb"));
	if (fp == NULL){
        return false;
	}
	fgets(gameData, 10, fp);
	fclose(fp);
	return true;
}

INT_PTR CALLBACK Save(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam){
    TCHAR lpszFileName[32];
	WORD cchFileName;

	UNREFERENCED_PARAMETER(lParam);
	switch (message){
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hdlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		switch (wParam)
		{
		case IDOK:
			cchFileName = (WORD)SendDlgItemMessage(hdlg,IDC_FILE_NAME,EM_LINELENGTH,(WPARAM)0,(LPARAM)0);
			*((LPWORD)lpszFileName) = cchFileName;
			SendDlgItemMessage(hdlg,IDC_FILE_NAME,EM_GETLINE,(WPARAM)0,(LPARAM)lpszFileName);
			lpszFileName[cchFileName] = 0;
			SaveGameDataToFile(lpszFileName);
			MessageBox(hdlg,"Success","Success",MB_OK);
			EndDialog(hdlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

void RestoreGame(char data[10]){
    int turnCount = 0;
    for (int i = 0; i < 9; i++){
        boardValues[i] = data[i] - '0';
        std::cout << data[i] - '0' << std::endl;
        if ((data[i] - '0') != EMPTY){
            turnCount++;
        }
    }
    turn = turnCount % 2 == 0 ? CROSS : NOUGHT;
}

INT_PTR CALLBACK Restore(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam){
    TCHAR lpszFileName[32];
	WORD cchFileName;

	UNREFERENCED_PARAMETER(lParam);
	switch (message){
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hdlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		switch (wParam)
		{
		case IDOK:
			cchFileName = (WORD)SendDlgItemMessage(hdlg,IDC_FILE_NAME,EM_LINELENGTH,(WPARAM)0,(LPARAM)0);
			*((LPWORD)lpszFileName) = cchFileName;
			SendDlgItemMessage(hdlg,IDC_FILE_NAME,EM_GETLINE,(WPARAM)0,(LPARAM)lpszFileName);
			lpszFileName[cchFileName] = 0;
			char data[10];
            bool success = GetGameDataFromFile(lpszFileName, data);
            if (success){
                MessageBox(hdlg,"Success","Success",MB_OK);
                RestoreGame(data);
            }
            else{
				MessageBox(hdlg,"Bad save file","Error",MB_OK);
            }
			EndDialog(hdlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

void SaveState(const HWND& hwnd){
    DialogBox(hInst, MAKEINTRESOURCE(IDD_FILE_NAME), hwnd, Save);
}

void RestoreState(const HWND& hwnd){
    DialogBox(hInst, MAKEINTRESOURCE(IDD_FILE_NAME), hwnd, Restore);
}


void UpdateCells(const HWND& hwnd){
    HDC hdc = GetDC(hwnd);
    for (int i = 0; i < 9; i++){
        if (boardValues[i] == EMPTY)continue;
        RECT cell = GetCellRect(hwnd, i);
        HICON icon = boardValues[i] == CROSS ? p1icon : p2icon;
        DrawIconCentered(hdc, cell, icon);
    }
    DrawPlayerTurn(hwnd, hdc);
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            LoadResources(hwnd);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)){
                case ID_FILE_NEWGAME:
                    NewGame(hwnd);
                break;
                case ID_FILE_EXIT:
                    PostQuitMessage(0);
                    break;
                case ID_FILE_SAVE:
                    SaveState(hwnd);
                    break;
                case ID_FILE_RESTORE:
                    RestoreState(hwnd);
                    Force_WM_PAINT(hwnd);
                    UpdateCells(hwnd);
                    break;
                default:
                break;
            }
            break;
        case WM_PAINT:
            Draw(hwnd);
            break;
        case WM_LBUTTONDOWN:
            ProccessClick(hwnd, lParam);
            break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            FreeLibrary(hInstLib);
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
