#include "stdafx.h"
#include "TakeBreak.h"
#include "Settings.h"

#define MAX_LOADSTRING 100
#define WM_TRAY_MESSAGE (WM_USER + 1)
#define ID_TRACK_EXIT (WM_USER + 2)
#define ID_TRACK_SETTINGS (WM_USER + 3)

// Global Variables:
HINSTANCE hInst;								// current instance
WCHAR szTitle[MAX_LOADSTRING];					// The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
WCHAR szBreakMessage[MAX_LOADSTRING];
WCHAR szBreakMessageHeader[MAX_LOADSTRING];
HWND hWnd;
HANDLE hEvent;
HANDLE hCleanEvent;
NOTIFYICONDATA notifydData;
HMENU hMenu;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

const int Minutes = 60 * 1000;
const int Seconds = 1000;
const int FollowUpTime = 5 * Minutes;
const int MeasuredTime = 25 * Seconds;

int GetNotifyTime()
{
    Settings::Data data;
    Settings::GetData(&data);
    return data.interval * Minutes;
}

DWORD WINAPI TrackWork(LPVOID lpParam){
    int totalTime = GetNotifyTime();
	int counter = 0;
	int extendedCounter = 0;
	for (;;){
		if (WaitForSingleObject(hEvent, MeasuredTime) != WAIT_TIMEOUT) {
			break;
		}

		LASTINPUTINFO lii;
		lii.cbSize = sizeof(LASTINPUTINFO);
		GetLastInputInfo(&lii);
		if ((GetTickCount() - lii.dwTime) < MeasuredTime){
			counter += MeasuredTime;
			extendedCounter += MeasuredTime;
		}else{
			counter = 0;
			extendedCounter = 0;
            totalTime = GetNotifyTime();
		}

		if (counter >= totalTime){
            totalTime = FollowUpTime;
            counter = 0;

			WCHAR szMessage[MAX_LOADSTRING] = { 0 };
			swprintf_s(szMessage, szBreakMessage, extendedCounter / Minutes);

			notifydData.uTimeout = 10000;
			notifydData.uFlags = NIF_INFO;
			notifydData.dwInfoFlags = NIIF_INFO;
			wcscpy_s(notifydData.szInfoTitle, sizeof(wchar_t) * wcslen(szBreakMessageHeader), szBreakMessageHeader);
			wcscpy_s(notifydData.szInfo, sizeof(wchar_t) * wcslen(szMessage), szMessage);
			Shell_NotifyIcon(NIM_MODIFY, &notifydData);
		}
	}
	SetEvent(hCleanEvent);
	return 0;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow){
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TAKEBREAK, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_BREAK_MESSAGE, szBreakMessage, MAX_LOADSTRING);
	LoadString(hInstance, IDS_BREAK_MESSAGE_HEADER, szBreakMessageHeader, MAX_LOADSTRING);

	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)){
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TAKEBREAK));

	DWORD dwThreadId;
	CreateThread(NULL, 0, TrackWork, NULL, 0, &dwThreadId);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)){
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	WaitForSingleObject(hCleanEvent, INFINITE);
	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance){
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TAKEBREAK));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_TAKEBREAK);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow){
	hInst = hInstance;
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd){
		return FALSE;
	}

	hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_TRACK_SETTINGS, TEXT("Settings..."));
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, ID_TRACK_EXIT, TEXT("Exit"));

	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hCleanEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	notifydData.cbSize = sizeof(NOTIFYICONDATA);
	notifydData.hWnd = hWnd;
	notifydData.uID = IDR_MAINFRAME;
	notifydData.uFlags = NIF_ICON | NIF_MESSAGE;
	notifydData.uCallbackMessage = WM_TRAY_MESSAGE;
	notifydData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TAKEBREAK));
	Shell_NotifyIcon(NIM_ADD, &notifydData);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	int wmId, wmEvent;

	switch (message){
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId){
		case ID_TRACK_EXIT:
			DestroyWindow(hWnd);
			break;

        case ID_TRACK_SETTINGS:
            Settings::LaunchSettings();
            break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		SetEvent(hEvent);
		Shell_NotifyIcon(NIM_DELETE, &notifydData);
		PostQuitMessage(0);
		break;
	case WM_TRAY_MESSAGE:
		switch (lParam){
		case  WM_RBUTTONDOWN:
			POINT point;
			GetCursorPos(&point);
			TrackPopupMenu(hMenu, TPM_RIGHTALIGN, point.x, point.y, 0, hWnd, NULL);
			break;
		default:
			break;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

