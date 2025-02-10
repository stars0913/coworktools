#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2\imgproc\types_c.h>

using namespace cv;
using namespace std;

extern int MatchTemplOnWindow(HWND hwnd, Mat imgTarg, RECT* ptrRcTarg, CvRect* ptrRc, uint uSize, double dbAccur, uint uFlag = 1);
extern int ClientToScreenEx(HWND hwnd, POINT* ptPoint);
extern Mat GetClientWindowEx(HWND hwnd, RECT* ptrRcTarg = NULL);
extern HWND GetSpecWindow();
extern HWND GetSpecChildWindow();
extern void Utf8ToUnicode(const char* pchSrc, WCHAR* pchDest, int nDestLen);
extern void Utf8ToAnsi(const char* pchSrc, char* pchDest, int nDestLen);
extern void SetClientText(HWND hwnd, char* ptrStr, DWORD dwLen);
extern bool is_str_utf8(const char* str);
extern void UnicodeToAnsi(const WCHAR* pchSrc, char* pchDest, int nDestLen);
extern void MarkClientRect(HWND hwnd, CvRect TargRc, Scalar dbColor = Scalar(0, 255, 0));
extern int GetClipBoardText(HWND hWnd, LPSTR pDes, int iSize);
extern Mat GetClientWindowEx(HWND hwnd, RECT* ptrRcTarg);
extern string getuuidex();
extern void Utf8ToAnsi(const char* pchSrc, char* pchDest, int nDestLen);
extern void StringReplaceAll(string& resource_str, string sub_str, string new_str);
extern string GetCodeMD5(const string& src);
extern string EncodeString(string strMD5PrivKey, string strmD5SerialKey);
extern int CheckAuthor(string& strPrivakey, string& strSerial);
extern void ConnectServer(string strPrivaKey, string strSerial);
extern double GetScreenScale();
extern bool SetClipBoardText(LPCSTR text, HWND hWnd);
extern bool SetClipBoardUnicode(WCHAR* text, HWND hWnd);
extern int GetClipBoardUnicode(HWND hWnd, WCHAR* pDes, int iSize);
extern void SetMsgToClipBoard(HWND hwnd, string strTmp);
extern void ctrl_c();
extern void ctrl_v();
extern void ctrl_a();
extern void ctrl_v_low();
extern void ctrl_c_low();
extern void escape_low();

extern DWORD WINAPI ThreadFuncGetMsg(LPVOID lpParam);//线程函数


#define SP  _T(",")
#define UINT_SIZEOFRECT		32

#define POS_MAINX				0
#define POS_MAINY				0
#define POS_MAINWIDTH			530
#define POS_MAINHEIGHT			650

#define POS_CHILDX				540
#define POS_CHILDY				0
#define POS_CHILDWIDTH			450
#define POS_CHILDHEIGHT			500

#define WX_CONTACT_FILE _T("wechat_contact.csv")
#define SZ_CHAR_SIZE                256
#define LISTSIZE                    64


int DemoShow()
{
	HWND hwndMain, hwndChild, hwndConsole;
	int i, j,ret;
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
		cout << _T("can't find path") << endl;
		return -1;
	}
	strrchr(szTemp, '\\')[1] = 0;
	string strDir = string(szTemp);

	string strRes;
	double dbscale = GetScreenScale();
	if (dbscale == 1.25)
		strRes = _T("resdemo\\");
	else if (dbscale == 1)
		strRes = _T("resdemo\\");
	else {
		cout << _T("Unexpected aborted: not support current screen scale.") << endl;
		return -1;
	}

	string strWx5 = strDir + strRes + _T("wxmi5.png");
	if ((_access(strWx5.data(), 0)) != 0) {
		cout << _T("can't read resource data:wxmi5.png::") << strWx5.data()<<endl;
		return -1;
	}

	string strWxk30 = strDir + strRes + _T("wxrm30.png");
	if ((_access(strWxk30.data(), 0)) != 0) {
		cout << _T("can't read resource data:wxrm30.png") << endl;
		return -1;
	}

	string strFind = strDir + strRes + _T("find.png");
	if ((_access(strFind.data(), 0)) != 0) {
		cout << _T("can't read resource data:find.png") << endl;
		return -1;
	}

	string strMi5input = strDir + strRes + _T("mi5_input.png");
	if ((_access(strMi5input.data(), 0)) != 0) {
		cout << _T("can't read resource data:mi5_input.png") << endl;
		return -1;
	}

	string strRm30input = strDir + strRes + _T("rm30_input.png");
	if ((_access(strRm30input.data(), 0)) != 0) {
		cout << _T("can't read resource data:rm30_input.png") << endl;
		return -1;
	}

	string strMainpsn = strDir + strRes + _T("mainpsn.png");
	if ((_access(strMainpsn.data(), 0)) != 0) {
		cout << _T("can't read resource data:mainpsn.png") << endl;
		return -1;
	}

	string strChildpsn = strDir + strRes + _T("childpsn.png");
	if ((_access(strChildpsn.data(), 0)) != 0) {
		cout << _T("can't read resource data:childpsn.png") << endl;
		return -1;
	}

	string strCpy = strDir + strRes + _T("ccopy.png");
	if ((_access(strCpy.data(), 0)) != 0) {
		cout << _T("can't read resource data:ccopy.png") << endl;
		return -1;
	}

	string strMi5send = strDir + strRes + _T("mi5send.png");
	if ((_access(strMi5send.data(), 0)) != 0) {
		cout << _T("can't read resource data:mi5send.png") << endl;
		return -1;
	}

	string strRm30send = strDir + strRes + _T("rm30send.png");
	if ((_access(strRm30send.data(), 0)) != 0) {
		cout << _T("can't read resource data:rm30send.png") << endl;
		return -1;
	}

	string strCpy0 = strDir + strRes + _T("ccopy0.png");
	if ((_access(strCpy0.data(), 0)) != 0) {
		cout << _T("can't read resource data:ccopy0.png") << endl;
		return -1;
	}

	hwndConsole = FindWindow(_T("ConsoleWindowClass"), NULL);
	if (!hwndConsole) {
		cout << _T("can't find hwndConsole windows") << endl;
		return -1;
	}

	hwndMain = FindWindow(NULL, _T("MI 5"));
	if (!hwndMain) {
		cout << _T("can't find hwndMain windows") << endl;
		return -1;
	}

	hwndChild = FindWindow(NULL, _T("Redmi K30 pro"));
	if (!hwndChild) {
		cout << _T("can't find hwndChild windows") << endl;
		return -1;
	}
	SetWindowPos(hwndMain, NULL, POS_MAINX, POS_MAINY, POS_MAINWIDTH, POS_MAINHEIGHT, SWP_NOSIZE | SWP_SHOWWINDOW);
	SetForegroundWindow(hwndMain);

	SetWindowPos(hwndChild, NULL, POS_CHILDX, POS_CHILDY, POS_CHILDWIDTH, POS_CHILDHEIGHT, SWP_NOSIZE | SWP_SHOWWINDOW);
	SetForegroundWindow(hwndChild);
	Sleep(500);

	matTempl = imread(strWx5);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
	if (!ret) {
		cout << _T("can't find strWx5 windows") << endl;
		return -1;
	}
	GetWindowRect(hwndMain, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);

	GetWindowRect(hwndChild, &rcTemp);
	pt.x = rcTemp.left + 160; pt.y = rcTemp.top + 600;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);

	matTempl = imread(strWxk30);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	ret = MatchTemplOnWindow(hwndChild, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
	if (!ret) {
		cout << _T("can't find strWxk30 windows") << endl;
		return -1;
	}
	GetWindowRect(hwndChild, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);

	//step 2
	MarkClientRect(hwndMain, cvRect(15, 110, 360, 80), Scalar(54, 81, 231));
	Sleep(1000);
	GetWindowRect(hwndMain, &rcTemp);
	pt.x = rcTemp.left + 100; pt.y = rcTemp.top + 180;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);
	

	matTempl = imread(strFind);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	GetClientRect(hwndChild, &rectFrame);
	rectFrame.bottom = 120;
	ret = MatchTemplOnWindow(hwndChild, matTempl, &rectFrame, szRc, UINT_SIZEOFRECT, 0.9);
	if (!ret) {
		cout << _T("can't find strFind windows") << endl;
		return -1;
	}
	SetMsgToClipBoard(hwndConsole, _T("预约账号"));
	GetWindowRect(hwndChild, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2 + 35;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(2000);

	ctrl_v_low();
	Sleep(1000);

	MarkClientRect(hwndChild, cvRect(7, 135, 148, 50), Scalar(54, 81, 231));
	Sleep(1000);
	GetWindowRect(hwndChild, &rcTemp);
	pt.x = rcTemp.left + 80; pt.y = rcTemp.top + 195;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);

	//step3. 接龙游戏
	matTempl = imread(strMi5input);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
	if (!ret) {
		cout << _T("can't find strMi5input windows") << endl;
		return -1;
	}
	SetMsgToClipBoard(hwndConsole, _T("接龙游戏：序号1"));
	GetWindowRect(hwndMain, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2 + 35;
	SetCursorPos(pt.x+50, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);

	ctrl_v_low();
	Sleep(1000);

	matTempl = imread(strMi5send);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
	if (!ret) {
		cout << _T("can't find strMi5send windows") << endl;
		return -1;
	}
	GetWindowRect(hwndMain, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
	SetCursorPos(pt.x , pt.y + 35);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(3000);

	matTempl = imread(strMainpsn);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	ret = MatchTemplOnWindow(hwndChild, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9,3);
	if (!ret) {
		cout << _T("can't find strFind windows") << endl;
		return -1;
	}
	GetWindowRect(hwndChild, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
	SetCursorPos(pt.x + 50, pt.y+35);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	Sleep(1000);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(500);

	matTempl = imread(strCpy);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	ret = MatchTemplOnWindow(hwndChild, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9,2);
	if (!ret) {
		cout << _T("can't find strFind windows") << endl;
		return -1;
	}
	GetWindowRect(hwndChild, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
	SetCursorPos(pt.x, pt.y+35);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);

	matTempl = imread(strRm30input);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	ret = MatchTemplOnWindow(hwndChild, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
	if (!ret) {
		cout << _T("can't find strFind windows") << endl;
		return -1;
	}
	GetWindowRect(hwndChild, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2+50; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2+35;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);

	ctrl_v_low();
	Sleep(1000);

	SetMsgToClipBoard(hwndConsole, _T("\n接龙游戏：序号2"));
	Sleep(1000);

	ctrl_v_low();
	Sleep(1000);

	matTempl = imread(strRm30send);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	ret = MatchTemplOnWindow(hwndChild, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
	if (!ret) {
		cout << _T("can't find strRm30send windows") << endl;
		return -1;
	}
	GetWindowRect(hwndChild, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
	SetCursorPos(pt.x, pt.y+30);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(3000);

	matTempl = imread(strChildpsn);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9,3);
	if (!ret) {
		cout << _T("can't find strChildpsn windows") << endl;
		return -1;
	}
	GetWindowRect(hwndMain, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
	SetCursorPos(pt.x + 55, pt.y+30);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	Sleep(1000);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(500);

	matTempl = imread(strCpy0);
	ZeroMemory(szRc, sizeof(CvRect)*UINT_SIZEOFRECT);
	ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
	if (!ret) {
		cout << _T("can't find strCpy0 windows") << endl;
		return -1;
	}
	GetWindowRect(hwndMain, &rcTemp);
	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
	SetCursorPos(pt.x, pt.y + 30);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(2000);

	system("start Notepad");
	Sleep(1000);

	ctrl_v_low();

	return 0;
}