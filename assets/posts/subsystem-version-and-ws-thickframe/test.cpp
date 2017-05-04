#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

LPCTSTR g_WindowClass = _T("TestApp");
LPCTSTR g_WindowTitle = _T("TestApp");
DWORD g_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
	WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HWND MyCreateWindow();

int main()
{
	ShowWindow(MyCreateWindow(), SW_SHOWNORMAL);
	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
	RECT rcClient = {0};
	switch (msg) {
	case WM_KEYDOWN:
		g_style = GetWindowLong(hwnd, GWL_STYLE);
		if ((g_style & WS_THICKFRAME) == WS_THICKFRAME) {
			g_style &= ~WS_THICKFRAME;
			printf("- WS_THICKFRAME \n");
		}
		else {
			g_style |= WS_THICKFRAME;
			printf("+ WS_THICKFRAME \n");
		}
		SetWindowLong(hwnd, GWL_STYLE, g_style);
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE |
			SWP_FRAMECHANGED);
		GetClientRect(hwnd, &rcClient);
		printf("client size = %i x %i \n",
			rcClient.right - rcClient.left,
			rcClient.bottom - rcClient.top);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, w, l);
	}
	return 0;
}

HWND MyCreateWindow()
{
	WNDCLASSEX wcex = {0};
	wcex.cbSize = sizeof(wcex);
	wcex.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = g_WindowClass;
	wcex.lpfnWndProc = WndProc;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassEx(&wcex);
	HWND hwnd = CreateWindowEx(0,
		g_WindowClass, g_WindowTitle, g_style,
		CW_USEDEFAULT, 0, 300, 300,
		NULL, NULL, NULL, NULL);
	return hwnd;
}
