#include <Windows.h>
#include <Windowsx.h>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <random>

// ��������� �������
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
COLORREF GetRandomColor();
void CreateWinAPIMenu(HWND hwnd);
void UpdateWindowTitle(HWND hwnd);
bool CheckWin(HWND hwnd, char& winner);
void ShowGameOverMessage(HWND hwnd, char winner);
void ResetWindowSettings(HWND hwnd);
void ResetGame(HWND hwnd);

// ��������� ���������
int n = 3; // ������ ����� �� ���������
const int defaultWidth = 320;
const int defaultHeight = 240;
std::vector<std::vector<char>> gridState;
COLORREF bgColor = RGB(64, 121, 245);
COLORREF gridColor = RGB(255, 0, 0);
COLORREF elementColor = RGB(255, 255, 255);
char currentPlayer = 'X';

#define OnClickExit 1
#define OnClickResetSettings 2

// ��������� ��������� �����
std::mt19937 rng(std::random_device{}());
std::uniform_int_distribution<> dist(0, 255);

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

    // ��������� ��������� ��������� ������
    if (pCmdLine && *pCmdLine) {
        n = _wtoi(pCmdLine);
        if (n <= 0 || n >= 20) n = 3; // ������� � �������� �� ��������� ��� ������������ �����
    }

    // ������������� ��������� �����
    gridState.resize(n, std::vector<char>(n, '_'));

    // ����������� ������ ����
    WNDCLASS SoftwareWindowsClass = { 0 };
    SoftwareWindowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    SoftwareWindowsClass.hInstance = hInst;
    SoftwareWindowsClass.lpszClassName = L"TicTacToeClass";
    SoftwareWindowsClass.hbrBackground = CreateSolidBrush(bgColor);
    SoftwareWindowsClass.lpfnWndProc = WindowProc;

    if (!RegisterClassW(&SoftwareWindowsClass)) {
        return -1;
    }

    // �������� ����
    HWND hwnd = CreateWindowW(
        L"TicTacToeClass",
        L"TicTacToe - ��� X",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        defaultWidth,
        defaultHeight,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if (!hwnd) {
        return -1;
    }

    CreateWinAPIMenu(hwnd);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static float cellWidth, cellHeight;

    switch (uMsg) {
    case WM_CREATE:
    {
        CreateWinAPIMenu(hwnd);

        RECT rect;
        GetClientRect(hwnd, &rect);
        cellWidth = (static_cast<float>(rect.right) - rect.left) / n;
        cellHeight = (static_cast<float>(rect.bottom) - rect.top) / n;

        return 0;
    }
    case WM_SIZE:
    {
        RECT rect;
        GetClientRect(hwnd, &rect);
        cellWidth = (static_cast<float>(rect.right) - rect.left) / n;
        cellHeight = (static_cast<float>(rect.bottom) - rect.top) / n;
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // ��������� �����
        HPEN hPen = CreatePen(PS_SOLID, 2, gridColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        for (int i = 1; i < n; ++i) {
            MoveToEx(hdc, i * cellWidth, 0, NULL);
            LineTo(hdc, i * cellWidth, n * cellHeight);

            MoveToEx(hdc, 0, i * cellHeight, NULL);
            LineTo(hdc, n * cellWidth, i * cellHeight);
        }

        // ��������� ��������� � �������
        for (int row = 0; row < n; ++row) {
            for (int col = 0; col < n; ++col) {
                int x = col * cellWidth;
                int y = row * cellHeight;

                if (gridState[row][col] == 'O') {
                    HPEN circlePen = CreatePen(PS_SOLID, 2, elementColor);
                    HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // ���������� ������, ���� ������ ������

                    SelectObject(hdc, circlePen);
                    SelectObject(hdc, hBrush);
                    Ellipse(hdc, x + 0.1f * cellWidth, y + 0.1f * cellHeight, x + 0.9f * cellWidth, y + 0.9f * cellHeight);

                    DeleteObject(circlePen);
                    DeleteObject(hBrush);
                }
                else if (gridState[row][col] == 'X') {
                    HPEN crossPen = CreatePen(PS_SOLID, 2, elementColor);
                    SelectObject(hdc, crossPen);

                    MoveToEx(hdc, x + 0.1f * cellWidth, y + 0.1f * cellHeight, NULL);
                    LineTo(hdc, x + 0.9f * cellWidth, y + 0.9f * cellHeight);
                    MoveToEx(hdc, x + 0.9f * cellWidth, y + 0.1f * cellHeight, NULL);
                    LineTo(hdc, x + 0.1f * cellWidth, y + 0.9f * cellHeight);

                    DeleteObject(crossPen);
                }
            }
        }

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
        EndPaint(hwnd, &ps);

        return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);

        int col = xPos / cellWidth;
        int row = yPos / cellHeight;

        if (row < n && col < n && gridState[row][col] == '_') {
            if (uMsg == WM_LBUTTONDOWN && currentPlayer == 'O') { // ������ ���� ��� O
                gridState[row][col] = 'O';
                currentPlayer = 'X'; // ����� ������
            }
            else if (uMsg == WM_RBUTTONDOWN && currentPlayer == 'X') { // ������ ���� ��� X
                gridState[row][col] = 'X';
                currentPlayer = 'O'; // ����� ������
            }

            InvalidateRect(hwnd, NULL, TRUE); // ����������� ����

            char winner;
            if (CheckWin(hwnd, winner)) {
                ShowGameOverMessage(hwnd, winner);
            }
            else {
                UpdateWindowTitle(hwnd); // ���������� ��������� ����
            }
        }
        return 0;
    }
    case WM_KEYDOWN:
    {
        if ((wParam == 'Q' && (GetKeyState(VK_CONTROL) & 0x8000)) || wParam == VK_ESCAPE) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        else if (wParam == VK_RETURN) {
            bgColor = GetRandomColor();
            HBRUSH hOldBrush = (HBRUSH)GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND);
            HBRUSH hNewBrush = CreateSolidBrush(bgColor);
            SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hNewBrush);
            DeleteObject(hOldBrush);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int r = GetRValue(gridColor);
        int g = GetGValue(gridColor);
        int b = GetBValue(gridColor);

        r = (r + delta / 6 + 256) % 256;
        g = (g + delta / 12 + 256) % 256;
        b = (b + delta / 4 + 256) % 256;

        gridColor = RGB(r, g, b);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_COMMAND:
    {
        switch (wParam) {
        case OnClickExit:
            PostQuitMessage(0);
            break;
        case OnClickResetSettings:
            ResetWindowSettings(hwnd);
            break;
        default:
            break;
        }
        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// ��������� ���������� �����
COLORREF GetRandomColor() {
    return RGB(dist(rng), dist(rng), dist(rng));
}

// �������� ����
void CreateWinAPIMenu(HWND hwnd) {
    HMENU rootMenu = CreateMenu();
    HMENU subMenu = CreateMenu();
    AppendMenu(rootMenu, MF_POPUP, (int)subMenu, L"����");
    AppendMenu(subMenu, MF_STRING, OnClickResetSettings, L"����� �������� ����");
    AppendMenu(subMenu, MF_STRING, OnClickExit, L"�����");

    SetMenu(hwnd, rootMenu);
}

// ����� �������� ���� (��� ������ ����)
void ResetWindowSettings(HWND hwnd) {
    bgColor = RGB(64, 121, 245);
    gridColor = RGB(255, 0, 0);
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(bgColor));
    SetWindowPos(hwnd, NULL, 0, 0, defaultWidth, defaultHeight, SWP_NOMOVE | SWP_NOZORDER);
    InvalidateRect(hwnd, NULL, TRUE);
}

// ���������� �������� ����
void UpdateWindowTitle(HWND hwnd) {
    wchar_t title[50];
    wsprintf(title, L"TicTacToe - ��� %c", currentPlayer);
    SetWindowText(hwnd, title);
}

// �������� �� ���������� ����
bool CheckWin(HWND hwnd, char& winner) {
    winner = '_';

    // �������� ����� � ��������
    for (int i = 0; i < n; ++i) {
        if (gridState[i][0] != '_' && std::all_of(gridState[i].begin(), gridState[i].end(), [i](char c) { return c == gridState[i][0]; }))
            winner = gridState[i][0];
        if (winner == '_' && gridState[0][i] != '_' && std::all_of(gridState.begin(), gridState.end(), [i](std::vector<char>& row) { return row[i] == gridState[0][i]; }))
            winner = gridState[0][i];
    }

    // �������� ����������
    if (winner == '_') {
        bool left_diag = true, right_diag = true;
        for (int i = 0; i < n; ++i) {
            if (gridState[i][i] != gridState[0][0]) left_diag = false;
            if (gridState[i][n - 1 - i] != gridState[0][n - 1]) right_diag = false;
        }

        if (gridState[0][0] != '_' && left_diag) winner = gridState[0][0];
        if (gridState[0][n - 1] != '_' && right_diag) winner = gridState[0][n - 1];
    }

    // �������� �� �����
    if (winner == '_') {
        bool fullFilled = true;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (gridState[i][j] == '_') {
                    fullFilled = false;
                    break;
                }
            }
            if (!fullFilled) break;
        }

        if (fullFilled) {
            winner = 'D';
            return true;
        }
    }

    return winner != '_';
}

// ��������� � ���������� ����
void ShowGameOverMessage(HWND hwnd, char winner) {
    if (winner == 'D') {
        MessageBox(hwnd, L"�����!", L"�����", MB_OK);
    }
    else {
        MessageBox(hwnd, (std::wstring(L"�������� ") + (winner == 'O' ? L"������" : L"��������")).c_str(), L"������", MB_OK);
    }

    ResetGame(hwnd);
}

// ����� ����
void ResetGame(HWND hwnd) {
    // ������� ��������� �����
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            gridState[i][j] = '_';
        }
    }

    // ���������� �������� ������ �� 'X'
    currentPlayer = 'X';

    // ��������� ��������� ����
    UpdateWindowTitle(hwnd);

    // �������������� ����
    InvalidateRect(hwnd, NULL, TRUE);
}