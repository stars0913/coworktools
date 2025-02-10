#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2\imgproc\types_c.h>

using namespace std;
using namespace cv;

extern int MatchTemplOnWindow(HWND hwnd, Mat imgTarg, RECT* ptrRcTarg, CvRect* ptrRc, uint uSize, double dbAccur, uint uFlag = 1);
extern int ClientToScreenEx(HWND hwnd, POINT* ptPoint);
extern Mat GetClientWindowEx(HWND hwnd, RECT* ptrRcTarg = NULL);
extern HWND GetSpecWindow();
extern HWND GetSpecChildWindow();
extern BOOL IsInterSection(RECT& rcSrc, RECT& rcDes);
extern void MarkClientRect(HWND hwnd, CvRect TargRc, Scalar dbColor = Scalar(0, 255, 0));
extern double GetScreenScale();
extern int GetClipBoardUnicode(HWND hWnd, WCHAR* pDes, int iSize);
extern Mat GetClientWindowEx(HWND hwnd, RECT* ptrRcTarg);
extern void Utf8ToAnsi(const char* pchSrc, char* pchDest, int nDestLen);
extern void UnicodeToUtf8(const WCHAR* pchSrc, char* pchDest, int nDestLen);
extern void StringSplit(const string& s, vector<string>& tokens, const string& delimiters);
extern bool is_str_gbk(const char* str);
extern void AnsiToUtf8(const char* pchSrc, char* pchDest, int nDestLen);
extern void Utf8ToUnicode(const char* pchSrc, WCHAR* pchDest, int nDestLen);
extern bool is_str_utf8(const char* str);
extern UINT GetFileTypeEx(FILE* fp);
extern void SetClientText(HWND hwnd, char* ptrStr, DWORD dwLen);
extern void ctrl_a();
extern BOOL GetFileContext(string strPath, string& strBuf);

extern DWORD WINAPI ThreadFuncGetMsg(LPVOID lpParam);//线程函数

#define UINT_SIZEOFRECT		        10
#define POS_WECHATX					0
#define POS_WECHATY					0
#define POS_WECHATWIDTH				800
#define POS_WECHATHEIGHT			650

#define POS_CONX					0
#define POS_CONY					0
#define POS_CONWIDTH				600
#define POS_CONHEIGHT				800

#define WM_KEYSTROKE (WM_USER + 101)
#define WM_KEYINPUT  (WM_USER + 102)
#define SZ_CHAR_SIZE                512



void InitWork_SetLabelAuth()
{
	int iScale = GetScreenScale() * 100;
	int  cx = GetSystemMetrics(SM_CXSCREEN);
	int  cy = GetSystemMetrics(SM_CYSCREEN);

	if ((iScale == 100 || iScale == 125) && cx >= 1400 && cy >= 800) {

	}
	else {
		//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
		cout << _T("Unexpected aborted: not support current screen resolution.") << endl;
		//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN);
		cout << _T(" 支持的屏幕分辨率：宽度 >= 1400, 高度 >= 800") << endl;
		cout << _T(" 支持的屏幕缩放比：100% 或者 125%") << endl;
		//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
		cout << _T("***注意：更改屏幕分辨率后，需要重启微信，服务在会生效***") << endl;
		//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		exit(1);
	}

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN);
	printf(_T("***使用说明***\n"
		"在启动服务前，请阅读下述步骤，做好准备工作\n"
		"1. 请打开微信电脑版，在\"通讯录\"栏 打开\"通讯录管理\"\n"
		"2. 请确认工作目录下已准备好用户名单（wechat_list.csv)\n"
		"3. 个人名单格式：只需在【备注,昵称,微信号】列下，增加记录即可 \n"
		"4. 程序运行服务时，请不要做任何手动操作。如果要终止运行，请按\"F10\"按键\n"
		"   了解上述要求后， 可按任意键继续...\n")
	);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	system(_T("pause"));
}


int WechatSetLabel(int iParam, char* pParam)
{
	char szLabelName[SZ_CHAR_SIZE] = { 0 };
	char szLabelNameUtf8[SZ_CHAR_SIZE] = { 0 };
	strcpy(szLabelName, pParam);
	memset(szLabelName + 128, 0, SZ_CHAR_SIZE - 128);
	AnsiToUtf8(szLabelName, szLabelNameUtf8, SZ_CHAR_SIZE);

	HWND hwndMain, hwndChild, hwndDlg, hwndConsole;
	int i, j, ret;
	Mat matTempl;

	RECT rectFind = { 0, 0, 120, 505 };  //设置比对区域;
	string strLine, strTmp;
	POINT pt;
	RECT* ptrNULL = NULL;
	CvRect szRc[UINT_SIZEOFRECT] = { 0 };
	CvRect szRc1[UINT_SIZEOFRECT] = { 0 };
	RECT rcTemp, rcOut, rectFrame;

	InitWork_SetLabelAuth();

	HANDLE hThread1;
	DWORD uID1;
	hThread1 = CreateThread(0, 0, ThreadFuncGetMsg, NULL, 0, &uID1);
	WaitForSingleObject(hThread1, INFINITY);
	CloseHandle(hThread1);

	char szTemp[_MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szTemp, _MAX_PATH);
	if (!strrchr(szTemp, '\\')) {
		cout << _T("Unexpected aborted: can't find path") << endl;
		return -1;
	}
	strrchr(szTemp, '\\')[1] = 0;
	string strDir = string(szTemp);

	string strRes;
	double dbscale = GetScreenScale();
	if (dbscale == 1.25)
		strRes = _T("res125\\");
	else if (dbscale == 1)
		strRes = _T("res100\\");
	else {
		cout << _T("Unexpected aborted: not support current screen scale.") << endl;
		return -1;
	}

	string strPngSquare = strDir + strRes + _T("square.png");
	if ((_access(strPngSquare.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:square.png") << endl;
		return -1;
	}

	string strPngDelLabel = strDir + strRes + _T("deletelabel.png");
	if ((_access(strPngDelLabel.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:deletelabel.png") << endl;
		return -1;
	}

	string strPngInputLabel = strDir + strRes + _T("inputlabel.png");
	if ((_access(strPngInputLabel.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:inputlabel.png") << endl;
		return -1;
	}

	string strPngSearch = strDir + strRes + _T("wxsearch.png");
	if ((_access(strPngSearch.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxsearch.png") << endl;
		return -1;
	}

	string strPngSwitchoff = strDir + strRes + _T("wxswitchoff.png");
	if ((_access(strPngSwitchoff.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxswitchoff.png") << endl;
		return -1;
	}

	string strPngSwitchon = strDir + strRes + _T("wxswitchon.png");
	if ((_access(strPngSwitchon.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxswitchon.png") << endl;
		return -1;
	}

	string strPngOk = strDir + strRes + _T("wxok.png");
	if ((_access(strPngOk.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxok.png") << endl;
		return -1;
	}

	string strTbl = strDir + _T("wechat_list.csv");
	string strContent;
	if (!GetFileContext(strTbl, strContent)) {
		exit(1);
	}

	//hwndMain = FindWindow(_T("WeChatMainWndForPC"), NULL);
	//if (!hwndMain) {
	//	cout << _T("Unexpected aborted: can't find wechat windows") << endl;
	//	return -1;
	//}

	int  cx = GetSystemMetrics(SM_CXSCREEN);
	int  cy = GetSystemMetrics(SM_CYSCREEN);
	hwndConsole = GetConsoleWindow();
	if (!hwndConsole) {
		cout << _T("Unexpected aborted: can't find ConsoleWindow") << endl;
		return -1;
	}
	SetWindowPos(hwndConsole, NULL, cx - POS_CONWIDTH, POS_CONY, POS_CONWIDTH, POS_CONHEIGHT, SWP_SHOWWINDOW);
	Sleep(500);

	hwndChild = FindWindow(_T("ContactManagerWindow"), NULL);
	if (!hwndChild) {
		cout << _T("Unexpected aborted: can't find ContactManagerWindow") << endl;
		return -1;
	}
	SetWindowPos(hwndChild, NULL, POS_WECHATX, POS_WECHATY, POS_WECHATWIDTH, POS_WECHATHEIGHT, SWP_SHOWWINDOW);
	SetForegroundWindow(hwndChild);
	Sleep(1000);

	string strWxid, strMemo, strKick;
	vector<string> vtStrLine;
	vector<string> vtStrCol;
	string::size_type position;
	StringSplit(strContent, vtStrLine, _T("\n"));

	cout << "Total line count:" << vtStrLine.size() << endl;

	for (i = 0; i < vtStrLine.size(); i++) {
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);

		vtStrCol.clear();
		StringSplit(vtStrLine[i], vtStrCol, _T(","));

		//if ((position = vtStrLine[i].find(_T("备注,昵称,微信号"))) != vtStrLine[i].npos) {
		if (vtStrLine[i] == _T("备注,昵称,微信号")) {
			continue;
		}
		strWxid = _T("");
		if (vtStrCol.size() > 2)
			strWxid = vtStrCol[2];
		strKick = _T("");
		if (vtStrCol.size() > 1)
			strKick = vtStrCol[1];
		strMemo = _T("");
		if (vtStrCol.size() > 0)
			strMemo = vtStrCol[0];

		//cout << strWxid.data() << endl;
		cout << strKick.data() << endl;
		//cout << strMemo.data() << endl;

		if (strWxid != _T("") && strstr(strWxid.data(), _T("wxid_")) == NULL) {
			strTmp = strWxid;
		}
		else if (strMemo != _T("")) {
			strTmp = strMemo;
		}
		else if (strKick != _T("")) {
			strTmp = strKick;
		}

		matTempl = imread(strPngSearch);
		ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		ret = MatchTemplOnWindow(hwndChild, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
		if (!ret) {
			cout << _T("Unexpected aborted: can't find resource data:strPngSearch") << endl;
			exit(1);
		}
		GetWindowRect(hwndChild, &rcTemp);
		pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2 + 50; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
		SetCursorPos(pt.x, pt.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);

		ctrl_a();
		Sleep(500);

		//VK_BACK
		keybd_event(VK_BACK, 0, 0, 0);
		keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
		Sleep(500);

		cout << strTmp.data() << endl;
		SetClientText(GetSpecWindow(), (char*)strTmp.data(), strTmp.length());
		Sleep(2000);

		matTempl = imread(strPngSquare);
		ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		ret = MatchTemplOnWindow(hwndChild, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9, 3);
		if (!ret) {
			cout << _T("Warning: can't find resource data:") << strTmp.data() << endl;
			continue;
		}
		GetWindowRect(hwndChild, &rcTemp);
		pt.x = rcTemp.right - 220; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
		SetCursorPos(pt.x, pt.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);

		static char szTmp[SZ_CHAR_SIZE];
		ZeroMemory(szTmp, sizeof(szTmp[SZ_CHAR_SIZE]));
		Utf8ToAnsi(_T("设置标签"), szTmp, SZ_CHAR_SIZE);
		hwndDlg = FindWindow(_T("StandardConfirmDialog"), szTmp);
		if (!hwndDlg) {
			cout << _T("Unexpected aborted: can't find StandardConfirmDialog") << endl;
			return -1;
		}

		if ((iParam & 0b1) ==1) {
			do {
				matTempl = imread(strPngDelLabel);
				ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
				ret = MatchTemplOnWindow(hwndDlg, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
				if (ret) {
					GetWindowRect(hwndDlg, &rcTemp);
					pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
					SetCursorPos(pt.x, pt.y);
					mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
					Sleep(500);
				}
			} while (ret);
		}
		if ((iParam & 0b10) == 2) {

		}

		matTempl = imread(strPngInputLabel);
		ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		ret = MatchTemplOnWindow(hwndDlg, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
		if (!ret) {
			cout << _T("Unexpected aborted: can't find resource data:strPngInputLabel") << endl;
			exit(1);
		}
		GetWindowRect(hwndDlg, &rcTemp);
		pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
		SetCursorPos(pt.x, pt.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);

		strTmp = szLabelNameUtf8;
		cout << strTmp.data() << endl;
		SetClientText(GetSpecWindow(), (char*)strTmp.data(), strTmp.length());
		Sleep(2000);

		GetWindowRect(hwndDlg, &rcTemp);
		pt.x = rcTemp.left + 235; pt.y = rcTemp.top + 195;
		SetCursorPos(pt.x, pt.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);

	}//end for
	return 0;
}