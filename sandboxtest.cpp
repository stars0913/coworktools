#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2\imgproc\types_c.h>

using namespace cv;
using namespace std;

extern int Split(char* pSrc, char* pSymbol, char* pOut, int iMaxRow, int iMaxCol);
extern int GetCfgString(char* pSrc, char* pSection, char* pS, char* pE, char* pOut, int iMax);
extern int MatchTemplOnWindow(HWND hwnd, Mat imgTarg, RECT* ptrRcTarg, CvRect* ptrRc, uint uSize, double dbAccur, uint uFlag = 1);
extern int ClientToScreenEx(HWND hwnd, POINT* ptPoint);
extern Mat GetClientWindowEx(HWND hwnd, RECT* ptrRcTarg = NULL);
extern HWND GetSpecWindow();
extern HWND GetSpecChildWindow();
extern void Utf8ToUnicode(const char* pchSrc, WCHAR* pchDest, int nDestLen);
extern void Utf8ToAnsi(const char* pchSrc, char* pchDest, int nDestLen);
extern void UnicodeToAnsi(const WCHAR* pchSrc, char* pchDest, int nDestLen);
extern void AnsiToUtf8(const char* pchSrc, char* pchDest, int nDestLen);
extern bool is_str_utf8(const char* str);

extern void MarkClientRect(HWND hwnd, CvRect TargRc, Scalar dbColor = Scalar(0, 255, 0));
extern int GetClipBoardText(HWND hWnd, LPSTR pDes, int iSize);
extern Mat GetClientWindowEx(HWND hwnd, RECT* ptrRcTarg);
extern string getuuidex();

extern void StringReplaceAll(string& resource_str, string sub_str, string new_str);
extern string GetCodeMD5(const string& src);
extern string EncodeString(string strMD5PrivKey, string strmD5SerialKey);
extern int CheckAuthor(string& strPrivakey, string& strSerial);
extern void ConnectServer(string strPrivaKey, string strSerial);
extern double GetScreenScale();
extern bool SetClipBoardText(LPCSTR text, HWND hWnd);
extern bool SetClipBoardUnicode(WCHAR* text, HWND hWnd);
extern void ctrl_c();
extern void ctrl_v();
extern void ctrl_a();
extern void ctrl_v_low();

extern LRESULT CALLBACK KeyboardProc(
	int code,       // hook code
	WPARAM wParam,  // virtual-key code
	LPARAM lParam   // keystroke-message information
	);
extern HHOOK g_hHook;

extern string GetSysSerialNum();


#define UINT_SIZEOFRECT		10

#define POS_MAINX				0
#define POS_MAINY				0
#define POS_MAINWIDTH			530
#define POS_MAINHEIGHT			650

#define POS_CHILDX				540
#define POS_CHILDY				0
#define POS_CHILDWIDTH			450
#define POS_CHILDHEIGHT			500

#define POS_CONX				0
#define POS_CONY				0
#define POS_CONWIDTH			600
#define POS_CONHEIGHT		    800

#define SZ_CHAR_SIZE            256


#define WM_KEYSTROKE (WM_USER + 101)
#define WM_KEYINPUT  (WM_USER + 102)

HHOOK g_hHookSD = NULL;

int SandBoxShow(HWND hwnd)
{
	char szAscii[_MAX_PATH] = { 0 };

	int iLen = SendMessage(GetSpecWindow(), WM_GETTEXTLENGTH, 0, 0);
	SendMessage(GetSpecWindow(), WM_GETTEXT, iLen, (LPARAM)szAscii);

	cout << _T("SandBoxTest::SandBoxShow:") << szAscii << endl;

	return 0;
}

LRESULT CALLBACK KeyboardProcSD(
	int code,       // hook code
	WPARAM wParam,  // virtual-key code
	LPARAM lParam   // keystroke-message information
	)
{
	KBDLLHOOKSTRUCT *ks = (KBDLLHOOKSTRUCT*)lParam;		// 包含低级键盘输入事件信息

	if (code<0 || code == HC_NOREMOVE)
		CallNextHookEx(g_hHookSD, code, wParam, lParam);

	if (wParam == WM_KEYDOWN) {
		switch (ks->vkCode) {
			case VK_F10:
				UnhookWindowsHookEx(g_hHookSD);
				exit(0);
				break;

			case VK_SPACE:
				//SandBoxShow(GetSpecWindow());
				break;

			default:
				break;
		}
	}
	return CallNextHookEx(g_hHookSD, code, wParam, lParam);
}

DWORD WINAPI ThreadFuncGetMsgSD(LPVOID lpParam)//线程函数
{
	MSG msg;

	g_hHookSD = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		cout << _T("GetMessage:") << msg.message << endl;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

int SandBoxTest()
{
	HWND hwndMain = NULL, hwndChild = NULL, hwndConsole = NULL;
	int i, j;
	Mat matTempl;

	RECT rectFind = { 0, 0, 120, 505 };  //设置比对区域;
	string strLine, strTmp;
	POINT pt;
	RECT* ptrNULL = NULL;
	CvRect szRc[UINT_SIZEOFRECT] = { 0 };
	CvRect szRc1[UINT_SIZEOFRECT] = { 0 };
	RECT rcTemp, rcOut, rectWechat;

	HANDLE hThread1;
	DWORD uID1;
	hThread1 = CreateThread(0, 0, ThreadFuncGetMsgSD, NULL, 0, &uID1);
	WaitForSingleObject(hThread1, INFINITY);
	CloseHandle(hThread1);

	char szTemp[_MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szTemp, _MAX_PATH);
	if (!strrchr(szTemp, '\\')) {
		cout << _T("can't find path") << endl;
		return -1;
	}
	strrchr(szTemp, '\\')[1] = 0;
	string strDir = string(szTemp);



	int  cx = GetSystemMetrics(SM_CXSCREEN);
	int  cy = GetSystemMetrics(SM_CYSCREEN);
	hwndConsole = GetConsoleWindow();
	if (!hwndConsole) {
		cout << _T("Unexpected aborted: can't find ConsoleWindowClass") << endl;
		return -1;
	}
	SetWindowPos(hwndConsole, NULL, cx - POS_CONWIDTH, POS_CONY, POS_CONWIDTH, POS_CONHEIGHT, SWP_SHOWWINDOW);
	Sleep(500);


	return 0;
}
