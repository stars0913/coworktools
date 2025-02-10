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
extern BOOL IsInterSection(RECT& rcSrc, RECT& rcDes);
extern void MarkClientRect(HWND hwnd, CvRect TargRc, Scalar dbColor = Scalar(0, 255, 0));
extern double GetScreenScale();
extern int GetClipBoardUnicode(HWND hWnd, WCHAR* pDes, int iSize);
extern Mat GetClientWindowEx(HWND hwnd, RECT* ptrRcTarg);
extern void Utf8ToAnsi(const char* pchSrc, char* pchDest, int nDestLen);
extern void UnicodeToUtf8(const WCHAR* pchSrc, char* pchDest, int nDestLen);
extern void StringReplaceAll(string& resource_str, string sub_str, string new_str);
extern void Utf8EraseChar(char* pDes, char sub);
extern void ctrl_a();
extern void ctrl_c();

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

#define WX_CONTACT_FILE            _T("wechat_contact.csv")
#define SZ_CHAR_SIZE                256
#define LISTSIZE                    64

#define YOFFSET                     65
#define WHELL                       -180

void InitWork_Group()
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
		"2. 整理通讯录可能需要比较长的时间，请耐心等待运行结束。如果要终止运行，请按\"F10\"按键\n"
		"3. 结束后请查看工作目录下的wechat_contact.csv文件，可用Excel或者WPS打开文件进行操作\n"
		"4. 程序运行服务时，请不要做任何手动操作。如果要终止运行，请按\"F10\"按键\n"
		"   了解上述要求后， 可按任意键继续...\n")
	);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	system(_T("pause"));
}

int WechatFindGroupName()
{
	HWND hwndMain, hwndChild, hwndDlg, hwndConsole;
	int i, j, ret;
	Mat matTempl;

	POINT pt;
	RECT* ptrNULL = NULL;
	CvRect szRc[UINT_SIZEOFRECT] = { 0 };
	RECT rcTemp, rcOut, rectFrame;
	RECT rectFind = { 60, 55, 260, 110 };  //设置比对区域;
	char szTmp[SZ_CHAR_SIZE];
	WCHAR szTmpW[SZ_CHAR_SIZE];
	string strLine;

	HANDLE hThread1;
	DWORD uID1;
	hThread1 = CreateThread(0, 0, ThreadFuncGetMsg, NULL, 0, &uID1);
	WaitForSingleObject(hThread1, INFINITY);
	CloseHandle(hThread1);

	InitWork_Group();

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
		strRes = _T("res125\\");
	else if (dbscale == 1)
		strRes = _T("res100\\");
	else {
		cout << _T("Unexpected aborted: not support current screen scale.") << endl;
		return -1;
	}

	string strPngZd = strDir  + strRes + _T("wxzd.png");
	if ((_access(strPngZd.data(), 0)) != 0) {
		cout << _T("can't read resource data :wxzd.png") << endl;
		return -1;
	}

	string strPngModify = strDir  + strRes + _T("modifygroup.png");
	if ((_access(strPngModify.data(), 0)) != 0) {
		cout << _T("can't read resource data: modifygroup.png") << endl;
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
	
	hwndMain = FindWindow(_T("WeChatMainWndForPC"), NULL);
	if (!hwndMain) {
		cout << _T("can't find wechat windows") << endl;
		return -1;
	}

	GetWindowRect(hwndMain, &rcTemp);
	SetWindowPos(hwndMain, NULL, POS_WECHATX, POS_WECHATY, POS_WECHATWIDTH, POS_WECHATHEIGHT, SWP_SHOWWINDOW);
	SetForegroundWindow(hwndMain);
	Sleep(500);

	string strContact = strDir + WX_CONTACT_FILE;
	FILE* fp;
	ret = fopen_s(&fp, strContact.data(), _T("w+"));
	if (ret) {
		cout << _T("Unexpected aborted: can't open temporary file") << endl;
		exit(1);
	}
	char utf8bom[] = { 0xEF,0xBB,0xBF };
	fwrite(utf8bom, sizeof(char), 3, fp);
	string strTmp = _T("编号,备注,昵称,微信号,性别,地区,标签,来源,朋友权限\n");
	fwrite(strTmp.data(), sizeof(char), strTmp.length(), fp);
	fclose(fp);

	//matTempl = imread(strPngPosbottom);
	//ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));7
	//ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
	//if (!ret) {
	//	cout << _T("Unexpected aborted: can't find resource data: strPngPosbottom") << endl;
	//	exit(1);
	//}

	GetWindowRect(hwndMain, &rcTemp);
	pt.x = rcTemp.left + 30; pt.y = rcTemp.top + 110;
	SetCursorPos(pt.x, pt.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(1000);


	POINT ptBottom = pt;
	UINT uPauseCount = 0;
	int iYoffset = -YOFFSET;
	UINT uWhellCnt = 0;

	while (TRUE) {

		//if (GetForegroundWindow() != hwndMain) {
		//	SetWindowPos(hwndMain, NULL, POS_WECHATX, POS_WECHATY, POS_WECHATWIDTH, POS_WECHATHEIGHT, SWP_SHOWWINDOW);
		//	SetForegroundWindow(hwndMain);
		//	Sleep(500);
		//}

		//防止系统睡眠
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);

		strLine = _T("");

		if (iYoffset >= YOFFSET*5) {

			matTempl = GetClientWindowEx(hwndMain, &rectFind);

			//鼠标滚轮
			pt.x = rectFind.left + 100; pt.y = rectFind.top + 140;
			SetCursorPos(pt.x, pt.y);
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			Sleep(500);
			if (uWhellCnt >= 3) {
				mouse_event(MOUSEEVENTF_WHEEL, 0, 0, WHELL + 9, 0);
				uWhellCnt = 0;
			}
			else {
				mouse_event(MOUSEEVENTF_WHEEL, 0, 0, WHELL, 0);
				uWhellCnt++;
			}
			Sleep(500);

			ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
			ret = MatchTemplOnWindow(hwndMain, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.99);
			if (ret) {
				if (uPauseCount < 5)
					uPauseCount++;
				else
					break;
				continue;
			}
			uPauseCount = 0;

		}
		else {
			iYoffset = iYoffset + YOFFSET;
		}

		matTempl = imread(strPngZd);
		ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		ret = MatchTemplOnWindow(hwndMain, matTempl, &rectFind, szRc, UINT_SIZEOFRECT, 0.9);
		if (ret) {
			continue;
		}

		//cout << _T("iYoffset::") << iYoffset << endl;
		pt.x = rectFind.left + 100; pt.y = rectFind.top + 30 + iYoffset;
		SetCursorPos(pt.x, pt.y);
		mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
		Sleep(500);
		SetCursorPos(pt.x+10, pt.y+10);


		matTempl = imread(strPngModify);
		ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		ret = MatchTemplOnWindow(GetSpecWindow(), matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
		if (!ret) {
			//ESCAPE
			keybd_event(VK_ESCAPE, 0, 0, 0);
			keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
			Sleep(500);

			SetCursorPos(ptBottom.x, ptBottom.y);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			Sleep(500);
			continue;
		}
		GetWindowRect(GetSpecWindow(), &rcTemp);
		pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
		SetCursorPos(pt.x, pt.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);

		static char szTmp[SZ_CHAR_SIZE];
		ZeroMemory(szTmp, sizeof(szTmp[SZ_CHAR_SIZE]));
		Utf8ToAnsi(_T("微信"), szTmp, SZ_CHAR_SIZE);
		hwndDlg = FindWindow(_T("RoomInfoModifyDialog"), szTmp);
		if (!hwndDlg) {
			cout << _T("Unexpected aborted: can't find RoomInfoModifyDialog") << endl;
			return -1;
		}
		GetWindowRect(hwndDlg, &rcTemp);
		pt.x = rcTemp.left + 140; pt.y = rcTemp.top + 115;
		SetCursorPos(pt.x, pt.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(300);

		ctrl_a();
		Sleep(300);

		ctrl_c();
		Sleep(300);

		//ESCAPE
		keybd_event(VK_ESCAPE, 0, 0, 0);
		keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
		Sleep(500);

		ZeroMemory(szTmp, SZ_CHAR_SIZE);
		ZeroMemory(szTmpW, sizeof(WCHAR)*SZ_CHAR_SIZE);
		GetClipBoardUnicode(hwndMain, szTmpW, SZ_CHAR_SIZE);
		UnicodeToUtf8(szTmpW, szTmp, SZ_CHAR_SIZE);
		strTmp = string(szTmp);
		StringReplaceAll(strTmp, _T(","), _T("，"));
		StringReplaceAll(strTmp, _T("\n"), _T(""));

		strLine = strLine + string(_T(",")) + strTmp +string(_T(",,,,,,,\n"));
		cout << _T("FindGroupName::") << szTmp << endl;

		FILE* fp;
		ret = fopen_s(&fp, strContact.data(), _T("a+"));
		if (ret) {
			cout << _T("Unexpected aborted: can't open temporary file") << endl;
			exit(1);
		}
		fwrite(strLine.data(), sizeof(char), strLine.length(), fp);
		fclose(fp);


	}//while
	return 0;
}