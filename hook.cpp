#include "stdafx.h"

#define  GWL_WNDPROC (-4)
#define  GWL_HINSTANCE (-6)
#define  GWL_HWNDPARENT (-8)
#define  GWL_STYLE (-16)
#define  GWL_EXSTYLE (-20)
#define  GWL_USERDATA (-21)
#define  GWL_ID (-12)


#define  DWL_MSGRESULT 0
#define  DWL_DLGPROC 4
#define  DWL_USER 8

using namespace std;

extern BOOL g_pasteId;


HHOOK g_hHook=NULL;

// 钩子回调函数（钩子过程）
LRESULT CALLBACK KeyboardProc(
	int code,       // hook code
	WPARAM wParam,  // virtual-key code
	LPARAM lParam   // keystroke-message information
	)
{
	KBDLLHOOKSTRUCT *ks = (KBDLLHOOKSTRUCT*)lParam;		// 包含低级键盘输入事件信息

	if (code<0 || code == HC_NOREMOVE)
		CallNextHookEx(g_hHook, code, wParam, lParam);

	if (wParam == WM_KEYDOWN) {
		switch (ks->vkCode) {
	    cout << _T("press keyboard:") << ks->vkCode << endl;

		case 86:
			if(GetKeyState(VK_CONTROL) & 0x8000)
				g_pasteId = TRUE;
			break;

		case 118:
			if (GetKeyState(VK_CONTROL) & 0x8000)
				g_pasteId = TRUE;
			break;

		case VK_F10:
			UnhookWindowsHookEx(g_hHook);
			exit(0);
			break;
		}
	}
	return CallNextHookEx(g_hHook, code, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(int nCode,  WPARAM wParam, LPARAM lParam)
{
	char szText[32];
	MSG msg;

	if (nCode < 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	else if (nCode == HC_ACTION)
	{
		MSG* lpMsg = (MSG*)lParam;
		switch (lpMsg->message) {
			case WM_PAINT:
				cout << _T("WM_PAINT:") << endl;
			default:
				;
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

DWORD WINAPI ThreadFuncGetMsg(LPVOID lpParam)//线程函数
{
	MSG msg;
	
	g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		cout << _T("GetMessage:") << msg.message << endl;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}