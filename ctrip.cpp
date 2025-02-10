//adb devices                         :Get devices serialno
//adb -s [serialno] tcpip [port]      
//adb connect 192.168.xxx.xxx:port
//scrcpy -s 192.168.xxx.xxx:port -m 1024 -b 2M --prefer-text(not use) --max-fps=15       //长宽最大比例限制1024 不能超过笔记本分辨率，否则截图就失真了。比特率2M

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
extern BOOL GetFileContext(string strPath, string& strBuf);
extern void SetMsgToClipBoard(HWND hwnd, string strTmp);
extern void ctrl_c();
extern void ctrl_v();
extern void ctrl_a();
extern void ctrl_v_low();
extern void ctrl_c_low();
extern void escape_low();

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

int CtripClear()
{
	char szLabelName[SZ_CHAR_SIZE] = { 0 };
	char szLabelNameUtf8[SZ_CHAR_SIZE] = { 0 };

	HWND hwndMain, hwndConsole;
	int i, j, k, ret;
	Mat matTempl;

	RECT rectFind = { 0, 0, 120, 505 };  //设置比对区域;
	string strLine, strTmp;
	POINT pt;
	RECT* ptrNULL = NULL;
	CvRect szRc[UINT_SIZEOFRECT] = { 0 };
	CvRect szRc1[UINT_SIZEOFRECT] = { 0 };
	RECT rcTemp, rcOut, rectFrame;

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
	if (dbscale == 1)
		strRes = _T("resctrip\\");
	else {
		cout << _T("Unexpected aborted: not support current screen scale.") << endl;
		return -1;
	}

	string strPngDelete = strDir + strRes + _T("delete.png");
	if ((_access(strPngDelete.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:delete.png") << endl;
		return -1;
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

	hwndMain = FindWindow(NULL, _T("Mi Note 3"));
	//hwndMain = FindWindow(NULL, _T("Redmi K30 Pro"));
	if (!hwndMain) {
		cout << _T("can't find hwndMain windows") << endl;
		return -1;
	}
	SetForegroundWindow(hwndMain);
	Sleep(1000);

	ret = 1;
	while (ret) {
		matTempl = imread(strPngDelete);
		ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
		if (ret) {
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
			SetCursorPos(pt.x, pt.y + 30);
			Sleep(500);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			SetCursorPos(rcTemp.left + 370, rcTemp.top + 590);
			Sleep(500);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
		}
	}
	return 0;
}

int CtripAddNew()
{
	char szLabelName[SZ_CHAR_SIZE] = { 0 };
	char szLabelNameUtf8[SZ_CHAR_SIZE] = { 0 };

	HWND hwndMain, hwndConsole;
	int i, j, k, ret;
	Mat matTempl;

	RECT rectFind = { 0, 0, 120, 505 };  //设置比对区域;
	string strLine, strTmp;
	POINT pt;
	RECT* ptrNULL = NULL;
	CvRect szRc[UINT_SIZEOFRECT] = { 0 };
	CvRect szRc1[UINT_SIZEOFRECT] = { 0 };
	RECT rcTemp, rcOut, rectFrame;

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
	if (dbscale == 1)
		strRes = _T("resctrip\\");
	else {
		cout << _T("Unexpected aborted: not support current screen scale.") << endl;
		return -1;
	}


	string strAdd = strDir + strRes + _T("addnew1.png");
	if ((_access(strAdd.data(), 0)) != 0) {
		cout << _T("can't read resource data:paste.png") << endl;
		return -1;
	}

	string strTbl = strDir + _T("ctrip_list.csv");
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

	hwndMain = FindWindow(NULL, _T("Mi Note 3"));
	//hwndMain = FindWindow(NULL, _T("Redmi K30 Pro"));
	if (!hwndMain) {
		cout << _T("can't find hwndMain windows") << endl;
		return -1;
	}
	SetForegroundWindow(hwndMain);
	Sleep(1000);

	ret = 1;

	string strName, strId, strPhone;
	vector<string> vtStrLine;
	vector<string> vtStrCol;
	string::size_type position;
	StringSplit(strContent, vtStrLine, _T("\n"));

	cout << "Total line count:" << vtStrLine.size() << endl;

	i = 0;
	while (i < vtStrLine.size()) {
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
		GetWindowRect(hwndMain, &rcTemp);

		//添加联系人
		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 240; pt.y = rcTemp.top + 520;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(2000);

		for (; i < vtStrLine.size(); i++) {
			vtStrCol.clear();
			StringSplit(vtStrLine[i], vtStrCol, _T(","));

			strPhone = _T("");
			if (vtStrCol.size() > 2)
				strPhone = vtStrCol[2];
			strId = _T("");
			if (vtStrCol.size() > 1)
				strId = vtStrCol[1];
			strName = _T("");
			if (vtStrCol.size() > 0)
				strName = vtStrCol[0];

			//cout << strId.data() << endl;
			//cout << strName.data() << endl;
			//cout << strPhone.data() << endl;

			//添加预约人
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + 250; pt.y = rcTemp.bottom - 30;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(2000);

			//姓名
			SetMsgToClipBoard(hwndConsole, strName);
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + 200; pt.y = rcTemp.top + 280;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			ctrl_v_low();
			Sleep(1000);

			//电话
			SetMsgToClipBoard(hwndConsole, strPhone);
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + 200; pt.y = rcTemp.top + 355;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			ctrl_v_low();
			Sleep(1000);

			//身份证
			SetMsgToClipBoard(hwndConsole, strId);
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + 200; pt.y = rcTemp.top + 495;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			ctrl_v_low();
			Sleep(1000);

			pt.x = rcTemp.left + 400; pt.y = rcTemp.top + 560;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(2000);

			pt.x = rcTemp.left + 260; pt.y = rcTemp.top + 655;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(5000);

			matTempl = imread(strAdd);
			ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
			ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
			if (ret) {
				pt.x = rcTemp.left + 30; pt.y = rcTemp.top + 100;
				SetCursorPos(pt.x, pt.y);
				Sleep(1000);
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				Sleep(4000);

				cout << _T("ERROR: can't add user:") << strName << endl;
			}
		}
	}
	return 0;
}

/*
int CtripBookA(char* pParam)
{
	char szLabelName[SZ_CHAR_SIZE] = { 0 };
	char szLabelNameUtf8[SZ_CHAR_SIZE] = { 0 };
	strcpy(szLabelName, pParam);
	memset(szLabelName + 128, 0, SZ_CHAR_SIZE - 128);
	AnsiToUtf8(szLabelName, szLabelNameUtf8, SZ_CHAR_SIZE);

	HWND hwndMain, hwndConsole;
	int i, j, k, ret;
	Mat matTempl;

	RECT rectFind = { 0, 0, 120, 505 };  //设置比对区域;
	string strLine, strTmp;
	POINT pt;
	RECT* ptrNULL = NULL;
	CvRect szRc[UINT_SIZEOFRECT] = { 0 };
	CvRect szRc1[UINT_SIZEOFRECT] = { 0 };
	RECT rcTemp, rcOut, rectFrame;

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
	if (dbscale == 1)
		strRes = _T("resctrip\\");
	else {
		cout << _T("Unexpected aborted: not support current screen scale.") << endl;
		return -1;
	}

	string strPngNoSelect = strDir + strRes + _T("noselect.png");
	if ((_access(strPngNoSelect.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:noselect.png") << endl;
		return -1;
	}

	string strPngPromise = strDir + strRes + _T("promise.png");
	if ((_access(strPngPromise.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:promise.png") << endl;
		return -1;
	}

	string strPngAreyouSure = strDir + strRes + _T("areyousure.png");
	if ((_access(strPngAreyouSure.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:areyousure.png") << endl;
		return -1;
	}

	string strPngSucess = strDir + strRes + _T("sucess.png");
	if ((_access(strPngSucess.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:sucess.png") << endl;
		return -1;
	}

	string strPaste = strDir + strRes + _T("paste.png");
	if ((_access(strPaste.data(), 0)) != 0) {
		cout << _T("can't read resource data:paste.png") << endl;
		return -1;
	}

	string strAddNew = strDir + strRes + _T("addnew.png");
	if ((_access(strAddNew.data(), 0)) != 0) {
		cout << _T("can't read resource data:addnew.png") << endl;
		return -1;
	}

	string strTbl = strDir + _T("ctrip_list.csv");
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

	hwndMain = FindWindow(NULL, _T("MI 5"));
	//hwndMain = FindWindow(NULL, _T("Redmi K30 Pro"));
	if (!hwndMain) {
		cout << _T("can't find hwndMain windows") << endl;
		return -1;
	}
	SetForegroundWindow(hwndMain);
	Sleep(1000);

	string strName, strId, strPhone;
	vector<string> vtStrLine;
	vector<string> vtStrCol;
	string::size_type position;
	StringSplit(strContent, vtStrLine, _T("\n"));

	cout << "Total line count:" << vtStrLine.size() << endl;

	i = 0;
	while (i < vtStrLine.size()){
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 240; pt.y = rcTemp.top + 100;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(2000);

		//场馆名称
		SetMsgToClipBoard(hwndConsole, szLabelNameUtf8);
		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 90; pt.y = rcTemp.top + 160;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(1000);
		ctrl_v_low();
		Sleep(1000);

		pt.x = rcTemp.left + 480; pt.y = rcTemp.top + 930;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(2000);

		pt.x = rcTemp.left + 300; pt.y = rcTemp.top + 280;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(2000);

		pt.x = rcTemp.left + 400; pt.y = rcTemp.top + 940;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(2000);

		//入园时间
		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 90; pt.y = rcTemp.top + 350;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(3000);

		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 480; pt.y = rcTemp.top + 170;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(2000);

		//选择日期(修改)
		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 186; pt.y = rcTemp.top + 858;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(3000);

		//选择时段(修改)
		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 130; pt.y = rcTemp.top +235;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(3000);

		//确定
		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 260; pt.y = rcTemp.top + 950;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(2000);

		//添加联系人
		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 240; pt.y = rcTemp.top + 520;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(2000);

		int count;
		for (j = 0, count = 0; j < 5 && i < vtStrLine.size(); j++, i++) {
			vtStrCol.clear();
			StringSplit(vtStrLine[i], vtStrCol, _T(","));

			strPhone = _T("");
			if (vtStrCol.size() > 2)
				strPhone = vtStrCol[2];
			strId = _T("");
			if (vtStrCol.size() > 1)
				strId = vtStrCol[1];
			strName = _T("");
			if (vtStrCol.size() > 0)
				strName = vtStrCol[0];

			//cout << strId.data() << endl;
			//cout << strName.data() << endl;
			//cout << strPhone.data() << endl;

			//添加预约人
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + 250; pt.y = rcTemp.top + 180;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(2000);

			//姓名
			SetMsgToClipBoard(hwndConsole, strName);
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + 200; pt.y = rcTemp.top + 280;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			ctrl_v_low();
			Sleep(1000);

			//电话
			SetMsgToClipBoard(hwndConsole, strPhone);
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + 200; pt.y = rcTemp.top + 355;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			ctrl_v_low();
			Sleep(1000);

			//身份证
			SetMsgToClipBoard(hwndConsole, strId);
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + 200; pt.y = rcTemp.top + 495;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(1000);
			ctrl_v_low();
			Sleep(1000);

			pt.x = rcTemp.left + 190; pt.y = rcTemp.top + 560;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(2000);

			pt.x = rcTemp.left + 260; pt.y = rcTemp.top + 655;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(5000);

			matTempl = imread(strAddNew);
			ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
			ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
			if (!ret) {
				pt.x = rcTemp.left + 30; pt.y = rcTemp.top + 100;
				SetCursorPos(pt.x, pt.y);
				Sleep(1000);
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				Sleep(4000);

				cout << _T("ERROR: can't add user:") << strName << endl;
			}
			else {
				count++;
			}

		}

		for (k = 0; k < count; k++) {
			matTempl = imread(strPngNoSelect);
			ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
			ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
			if (ret) {
				GetWindowRect(hwndMain, &rcTemp);
				pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
				SetCursorPos(pt.x, pt.y+30);
				Sleep(1000);
				mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
				Sleep(1000);
			}
		}
		//确定
		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 260; pt.y = rcTemp.top + 940;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(3000);

		//if (k == 4) {

		//}
		//else {
		//	//不足5人，返回再确定
		//	GetWindowRect(hwndMain, &rcTemp);
		//	pt.x = rcTemp.left + 90; pt.y = rcTemp.top + 100;
		//	SetCursorPos(pt.x, pt.y);
		//	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		//	Sleep(2000);

		//}

		//立即预约
		GetWindowRect(hwndMain, &rcTemp);
		pt.x = rcTemp.left + 260; pt.y = rcTemp.top + 940;
		SetCursorPos(pt.x, pt.y);
		Sleep(1000);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(2000);

		//matTempl = imread(strPngPromise);
		//ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		//ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
		//if (ret) {
		//	GetWindowRect(hwndMain, &rcTemp);
		//	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
		//	SetCursorPos(pt.x, pt.y+30);
		//	Sleep(1000);
		//	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		//	Sleep(3000);
		//}

		matTempl = imread(strPngAreyouSure);
		ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
		if (ret) {
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + 380; pt.y = rcTemp.top + 590;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(5000);
		}

		matTempl = imread(strPngSucess);
		ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
		if (ret) {
			GetWindowRect(hwndMain, &rcTemp);
			pt.x = rcTemp.left + 360; pt.y = rcTemp.top + 515;
			SetCursorPos(pt.x, pt.y);
			Sleep(1000);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(5000);
		}
	}//end while
	return 0;
}
*/