#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2\imgproc\types_c.h>

using namespace std;
using namespace cv;


extern void SetClientText(HWND hwnd, char* ptrStr, DWORD dwLen);
extern UINT GetOneLine(char* ptrStr, UINT uSize, UINT uStart = 0);
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
void Utf8ToAnsi(const char* pchSrc, char* pchDest, int nDestLen);
extern void UnicodeToUtf8(const WCHAR* pchSrc, char* pchDest, int nDestLen);
extern void StringSplit(const string& s, vector<string>& tokens, const string& delimiters);
extern bool is_str_gbk(const char* str);
extern void AnsiToUtf8(const char* pchSrc, char* pchDest, int nDestLen);
extern bool is_str_utf8(const char* str);
extern UINT GetFileTypeEx(FILE* fp);
extern BOOL GetFileContext(string strPath, string& strBuf);

extern DWORD WINAPI ThreadFuncGetMsg(LPVOID lpParam);//线程函数

#define UINT_SIZEOFRECT		10

#define POS_WECHATX					0
#define POS_WECHATY					0
#define POS_WECHATWIDTH				700
#define POS_WECHATHEIGHT			500

#define POS_CONX					0
#define POS_CONY					0
#define POS_CONWIDTH				600
#define POS_CONHEIGHT				800

#define WM_KEYSTROKE (WM_USER + 101)
#define WM_KEYINPUT  (WM_USER + 102)
#define SZ_CHAR_SIZE                256


void InitWork_Boardcast()
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
		"1. 请打开微信电脑版\n"
		"2. 请将需要对外发送的内容（图片、链接、视频等）先发送到自己的聊天记录下\n"
		"3. 请确认工作目录下已准备好用户名单（wechat_list.csv)\n"
		"4. 个人名单格式：只需在【备注,昵称,微信号】列下，增加记录即可 \n"
		"5. 群名单格式：只要在【备注】列下增加记录即可\n"
		"6. 程序运行服务时，请不要做任何手动操作。如果要终止运行，请按\"F10\"按键\n"
		"   了解上述要求后， 可按任意键继续...\n")
	);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	system(_T("pause"));
}


int WechatBoardCast()
{
	HWND hwndWechat = NULL, hwndConsole = NULL;
	int i, j, ret;
	Mat matTempl;

	string strTmp;
	POINT pt;
	RECT* ptrNULL = NULL;
	RECT rectFind = { 0, 0, 160, 140 };  //设置比对区域;
	CvRect szRc[UINT_SIZEOFRECT] = { 0 };
	RECT rcTemp, rectWechat;

	InitWork_Boardcast();

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

	string strPngCpy = strDir +strRes+_T("copy.png");
	if ((_access(strPngCpy.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data: copy.png") << endl;
		return -1;
	}

	string strPngPos = strDir +strRes+_T("wxposbd.png");
	if ((_access(strPngPos.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data: wxposbd.png") << endl;
		return -1;
	}

	string strPngMiss = strDir +strRes+_T("miss.png");
	if ((_access(strPngMiss.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data: miss.png") << endl;
		return -1;
	}

	string strTbl = strDir + _T("wechat_list.csv");
	string strContent;
	if (!GetFileContext(strTbl, strContent)) {
		exit(1);
	}

	int  cx = GetSystemMetrics(SM_CXSCREEN);
	int  cy = GetSystemMetrics(SM_CYSCREEN);
	hwndConsole = GetConsoleWindow();
	if (!hwndConsole) {
		cout << _T("Unexpected aborted: can't find ConsoleWindow") << endl;
		return -1;
	}
	SetWindowPos(hwndConsole, NULL, cx - POS_CONWIDTH, POS_CONY, POS_CONWIDTH, POS_CONHEIGHT, SWP_SHOWWINDOW);
	Sleep(500);

	hwndWechat = FindWindow(_T("WeChatMainWndForPC"), NULL);
	if (!hwndWechat) {
		cout << _T("Unexpected aborted: can't find wechat windows") << endl;
		return -1;
	}
	GetWindowRect(hwndWechat, &rectWechat);
	SetWindowPos(hwndWechat, NULL, POS_WECHATX, POS_WECHATY, POS_WECHATWIDTH, POS_WECHATHEIGHT,  SWP_SHOWWINDOW);
	SetForegroundWindow(hwndWechat);
	Sleep(500);

	GetWindowRect(hwndWechat, &rcTemp);
	pt.x = rcTemp.left + 20; pt.y = rcTemp.top + 50;
	SetCursorPos(pt.x , pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);

	GetWindowRect(hwndWechat, &rcTemp);
	pt.x = rcTemp.left + 150; pt.y = rcTemp.top + 210;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);

	matTempl = imread(strPngPos);
	ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
	ret = MatchTemplOnWindow(hwndWechat, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
	if (!ret) {
		cout << _T("Unexpected aborted: can't find resource data:strPngPos") << endl;
		exit(1);
	}
	GetWindowRect(hwndWechat, &rcTemp);
	pt.x = rcTemp.right - 100; pt.y = rcTemp.top + szRc[0].y-80;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	Sleep(1000);
	SetCursorPos(pt.x+5, pt.y+5);


	//cout << _T("WechatFindGroupName::strPngCpy.png:") << endl;
	matTempl = imread(strPngCpy);
	ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
	ret = MatchTemplOnWindow(GetSpecWindow(), matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
	if (!ret) {
		cout << _T("Unexpected aborted: can't find resource data:strPngCpy") << endl;
		exit(1);
	}
	GetWindowRect(GetSpecWindow(), &rcTemp);
	pt.x = rcTemp.left + szRc[0].x; pt.y = rcTemp.top + szRc[0].y;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(500);

	string strWxid, strMemo, strKick;
	vector<string> vtStrLine;
	vector<string> vtStrCol;
	string::size_type position;
	StringSplit(strContent, vtStrLine, _T("\n"));

	cout << "Total line count:" << vtStrLine.size() << endl;

	for (i = 0; i < vtStrLine.size();i++) {
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

		if (strWxid != _T("") && strstr(strWxid.data(), _T("wxid_"))==NULL){
			strTmp = strWxid;
		}
		else if (strMemo != _T("")) {
			strTmp = strMemo;
		}
		else if(strKick != _T("")) {
			strTmp = strKick;
		}

		GetWindowRect(hwndWechat, &rcTemp);
		pt.x = rcTemp.left + 105; pt.y = rcTemp.top + 35;
		SetCursorPos(pt.x, pt.y);

		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(1000);

		cout << strTmp.data() << endl;
		SetClientText(GetSpecWindow(), (char*)strTmp.data(), strTmp.length());
		Sleep(1000);

		//cout << _T("WechatFindGroupName::strPngMiss.png:") << endl;
		matTempl = imread(strPngMiss);
		ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		ret = MatchTemplOnWindow(hwndWechat, matTempl, &rectFind, szRc, UINT_SIZEOFRECT, 0.9);
		if (ret) {
			//ESCAPE
			keybd_event(VK_ESCAPE, 0, 0, 0);
			keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
			Sleep(500);
		}else {
			GetWindowRect(hwndWechat, &rcTemp);
			pt.x = rcTemp.left + 140; pt.y = rcTemp.top + 120;
			SetCursorPos(pt.x, pt.y);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(2000);

			matTempl = imread(strPngPos);
			ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
			ret = MatchTemplOnWindow(hwndWechat, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
			if (ret) {
				GetWindowRect(hwndWechat, &rcTemp);
				pt.x = rcTemp.right - 100; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2 + 50;
				SetCursorPos(pt.x, pt.y);
				mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
				Sleep(500);

				//pt.x = rcTemp.right - 100; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
				SetCursorPos(pt.x + 10, pt.y + 10);
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				Sleep(1000);

				pt.x = rcTemp.left + 620; pt.y = rcTemp.top + 470;
				SetCursorPos(pt.x, pt.y);
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				Sleep(2000);
			}
		}
	}//end for
	return 0;
}