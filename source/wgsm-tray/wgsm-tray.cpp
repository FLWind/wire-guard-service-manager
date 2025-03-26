// wgsm-tray.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "wgsm-tray.h"
#include "service.h"

#define MAX_LOADSTRING 100
#define	WM_USER_SHELLICON WM_USER + 1

#define SWM_TRAYMSG				WM_APP//		the message ID sent to our window
#define SWM_EXIT				WM_APP + 1//	close the window
#define SWM_CONNECT				WM_APP + 2//	start WireGuard service
#define SWM_DISCONNECT			WM_APP + 3//	stop WireGuard service

#define UPDATE_STATUS_TIMER	1001

// Global Variables:
HINSTANCE hInst;                                // current instance
NOTIFYICONDATA nidApp;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

CService g_cService;

enState		g_enState = enPending;
UINT		g_uiIcon = IDI_PENDING;
UINT		g_uiHint = IDS_PENDING_STATE;

void UpdateState()
{
	enState enState = enPending;
	g_cService.GetState(enState); // Ignore errors, save Pending state if error

	switch (enState)
	{
	case enConnect:
		g_uiIcon = IDI_CONNECT;
		g_uiHint = IDS_CONNECT_STATE;
		break;

	case enDisconnect:
		g_uiIcon = IDI_DISCONNECT;
		g_uiHint = IDS_DISCONNECT_STATE;
		break;

	default:
		g_uiIcon = IDI_PENDING;
		g_uiHint = IDS_PENDING_STATE;
		break;
	}

	g_enState = enState;
}

void UpdateTray()
{
	UpdateState();

	nidApp.hIcon = LoadIcon(hInst, (LPCTSTR)MAKEINTRESOURCE(g_uiIcon)); // handle of the Icon to be displayed, obtained from LoadIcon 
	LoadString(hInst, g_uiHint, nidApp.szTip, MAX_LOADSTRING);

	Shell_NotifyIcon(NIM_MODIFY, &nidApp);
}

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
	LoadStringW(hInstance, IDC_WGSMTRAY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WGSMTRAY));

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

	return (int)msg.wParam;
}

class CContextMenu
{
	std::wstring LoadModuleString(UINT uID)
	{
		static const DWORD dwModuleStringLen = 4096;
		WCHAR strModuleString[dwModuleStringLen];
		if (0 == LoadString(hInst, uID, strModuleString, dwModuleStringLen))
		{
			return L"";
		}

		return std::wstring(strModuleString);
	}

	CContextMenu()
	{
		switch (g_enState)
		{
		case enConnect:
			m_vMenu.push_back(std::make_pair(SWM_EXIT, LoadModuleString(IDS_DISCONNECT)));
			break;

		case enDisconnect:
			m_vMenu.push_back(std::make_pair(SWM_EXIT, LoadModuleString(IDS_CONNECT)));
			break;

		default:
			break;
		}

		m_vMenu.push_back(std::make_pair(SWM_EXIT, LoadModuleString(IDS_EXIT)));


	}

public:
	CContextMenu(const CContextMenu&) = delete;
	CContextMenu(CContextMenu&&) = delete;
	CContextMenu& operator=(const CContextMenu&) = delete;
	CContextMenu& operator=(CContextMenu&&) = delete;

	static CContextMenu& Inst()
	{
		static CContextMenu gInst;
		return gInst;
	}

	void ShowContextMenu(HWND hWnd)
	{
		POINT pt;
		GetCursorPos(&pt);
		HMENU hMenu = CreatePopupMenu();
		if (hMenu)
		{
			auto Insert = [hMenu](const std::pair<UINT_PTR, std::wstring>& e) { InsertMenu(hMenu, -1, MF_BYPOSITION, e.first, e.second.c_str()); };
			std::for_each(m_vMenu.begin(), m_vMenu.end(), Insert);

			// note:	must set window to the foreground or the
			//			menu won't disappear when it should
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
			DestroyMenu(hMenu);
		}
	}

private:
	std::vector<std::pair<UINT_PTR, std::wstring>> m_vMenu;
};



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WGSMTRAY));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WGSMTRAY);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

	//ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);
	UpdateState();

	nidApp.cbSize = sizeof(NOTIFYICONDATA); // sizeof the struct in bytes 
	nidApp.hWnd = (HWND)hWnd;              //handle of the window which will process this app. messages 
	nidApp.uID = IDI_CONNECT;           //ID of the icon that willl appear in the system tray 
	nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; //ORing of all the flags 
	nidApp.hIcon = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(g_uiIcon)); // handle of the Icon to be displayed, obtained from LoadIcon 
	nidApp.uCallbackMessage = WM_USER_SHELLICON;
	LoadString(hInstance, g_uiHint, nidApp.szTip, MAX_LOADSTRING);
	Shell_NotifyIcon(NIM_ADD, &nidApp);

	return TRUE;
}

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
	case WM_INITDIALOG:
		SetTimer(hWnd, UPDATE_STATUS_TIMER, 5000, nullptr);
		break;
	case WM_USER_SHELLICON:
		// systray msg callback 
		switch (LOWORD(lParam))
		{
		case WM_LBUTTONDBLCLK:
			switch (g_enState)
			{
			case enConnect:
				g_cService.Disconnect();
				UpdateTray();
				break;
			case enDisconnect:
				g_cService.Connect();
				UpdateTray();
				break;
			default:
				break;
			}
			break;
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			CContextMenu::Inst().ShowContextMenu(hWnd);
			break;
		}
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case SWM_EXIT:
			Shell_NotifyIcon(NIM_DELETE, &nidApp);
			DestroyWindow(hWnd);
			break;
		case SWM_CONNECT:
			g_cService.Connect();
			UpdateTray();
			break;
		case SWM_DISCONNECT:
			g_cService.Disconnect();
			UpdateTray();
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
	case WM_TIMER:
		UpdateTray();
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
