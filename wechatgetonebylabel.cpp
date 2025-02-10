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
extern BOOL IsInterSection(RECT& rcSrc, RECT& rcDes);
extern void MarkClientRect(HWND hwnd, CvRect TargRc, Scalar dbColor = Scalar(0, 255, 0));
extern double GetScreenScale();
extern int GetClipBoardUnicode(HWND hWnd, WCHAR* pDes, int iSize);
extern Mat GetClientWindowEx(HWND hwnd, RECT* ptrRcTarg);
extern void Utf8ToAnsi(const char* pchSrc, char* pchDest, int nDestLen);
extern void UnicodeToUtf8(const WCHAR* pchSrc, char* pchDest, int nDestLen);
extern void StringReplaceAll(string& resource_str, string sub_str, string new_str);
extern void Utf8EraseChar(char* pDes, char sub);

extern DWORD WINAPI ThreadFuncGetMsg(LPVOID lpParam);//线程函数


#define SP  _T(",")
#define UINT_SIZEOFRECT		32
#define POS_WECHATX					0
#define POS_WECHATY					0
#define POS_WECHATWIDTH				800
#define POS_WECHATHEIGHT			650

#define POS_CONX					0
#define POS_CONY					0
#define POS_CONWIDTH				600
#define POS_CONHEIGHT				800

#define WX_CONTACT_FILE _T("wechat_contact.csv")
#define SZ_CHAR_SIZE                256
#define LISTSIZE                    64


string g_strWxid_save[LISTSIZE] = { "" };
string g_strWxid_cur[LISTSIZE] = { "" };

BOOL CheckWxIDExist(const char* szID)
{
	string strWxid = szID;
	for (int i = 0; i < LISTSIZE; i++)
	{
		if (g_strWxid_save[i] == strWxid) {
			return TRUE;
		}
	}
	return FALSE;
}

string Tesseract(HWND hwnd, RECT rcTemp, CvRect& szRc, string strDir)
{

	GetClientRect(hwnd, &rcTemp);
	rcTemp.left = szRc.x + szRc.width; rcTemp.top = szRc.y;
	rcTemp.right = rcTemp.right; rcTemp.bottom = rcTemp.top + szRc.height;

	string strCmd = string(_T("start /MIN /WAIT tesseract ")) + string(_T("./tmp220601.png tmp220601 ")) + string(_T(" -l chi_sim --psm 7"));
	//cout << _T("strCmd:") << strCmd.data() << endl;
	string strTxt = strDir + _T("tmp220601.txt");
	string strImg = strDir + _T("tmp220601.png");

	Mat matTempl = GetClientWindowEx(hwnd, &rcTemp);
	imwrite(strImg, matTempl);

	system(strCmd.data());

	FILE* fp;
	int ret = fopen_s(&fp, strTxt.data(), _T("r+"));
	if (ret) {
		cout << _T("Unexpected aborted: can't open temporary file") << endl;
		exit(1);
	}

	long iSize = _filelength(_fileno(fp));
	static char pUtf8[SZ_CHAR_SIZE];
	ZeroMemory(pUtf8, sizeof(char)*SZ_CHAR_SIZE);
	fread(pUtf8, sizeof(char), iSize, fp);
	fclose(fp);

	//cout << _T("GetContactData::WxSource:") << pUtf8 << endl;
	Utf8EraseChar(pUtf8, ' ');
	Utf8EraseChar(pUtf8, '\n');
	string strTmp = string(pUtf8);

	remove(strTxt.data()); //删除临时文件
	remove(strImg.data());

	return strTmp;
}

//
//return 0: success. -1:break down
//
int GetContactDetail(string& strDir,string& strWxid, string& strWxmemo, string& strWxsource,string& strWxarea, 
	string& strWxlabel, string& strWxmale, string& strWxfemale, string& strFriend, string& strKickname, int iRec, uint& uIndex) 
{
	int i, j, ret = 0, ret1 = 0;
	Mat matTempl;
	RECT rcTemp;
	POINT pt;
	string strLine,strTmp;
	RECT* ptrNULL = NULL;
	CvRect szRc[UINT_SIZEOFRECT] = { 0 };
	CvRect szRc1[UINT_SIZEOFRECT] = { 0 };
	char szTmp[SZ_CHAR_SIZE];
	WCHAR szTmpW[SZ_CHAR_SIZE];

	HWND hwndCur = GetForegroundWindow();

	//编号//
	strLine = string(_itoa(iRec, szTmp, 10));

	//备注//
	matTempl = imread(strWxmemo);
	ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
	ret = MatchTemplOnWindow(hwndCur, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.95);
	if (ret) {
		GetWindowRect(hwndCur, &rcTemp);
		pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
		SetCursorPos(pt.x+60, pt.y);
		
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);

		//ctrl-c
		keybd_event(VK_CONTROL, 0, 0, 0);
		keybd_event(0x43, 0, 0, 0);
		keybd_event(0x43, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
		Sleep(100);

		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(200);

		ZeroMemory(szTmp, SZ_CHAR_SIZE);
		ZeroMemory(szTmpW, sizeof(WCHAR)*SZ_CHAR_SIZE);
		GetClipBoardUnicode(hwndCur, szTmpW, SZ_CHAR_SIZE);
		UnicodeToUtf8(szTmpW, szTmp, SZ_CHAR_SIZE);
		strTmp = string(szTmp);
		StringReplaceAll(strTmp, _T(","), _T("，"));
		strLine = strLine + _T(",") + strTmp;
		cout << _T("GetContactData::Remark:") << szTmp << endl;
	}
	else {
		strLine = strLine + _T(",");
	}

	//昵称//
	matTempl = imread(strKickname);
	ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
	ret = MatchTemplOnWindow(hwndCur, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.95);
	if (ret) {
		GetWindowRect(hwndCur, &rcTemp);
		pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
		SetCursorPos(pt.x+35, pt.y);
	}
	else {
		GetWindowRect(hwndCur, &rcTemp);
		pt.x = rcTemp.left + 105; pt.y = rcTemp.top + 35;
		SetCursorPos(pt.x, pt.y);
	}
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(500);

	//ctrl-c
	keybd_event(VK_CONTROL, 0, 0, 0);
	keybd_event(0x43, 0, 0, 0);
	keybd_event(0x43, 0, KEYEVENTF_KEYUP, 0);
	keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
	Sleep(100);

	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(200);

	ZeroMemory(szTmp, SZ_CHAR_SIZE);
	ZeroMemory(szTmpW, sizeof(WCHAR)*SZ_CHAR_SIZE);
	GetClipBoardUnicode(hwndCur, szTmpW, SZ_CHAR_SIZE);
	UnicodeToUtf8(szTmpW, szTmp, SZ_CHAR_SIZE);
	strTmp = string(szTmp);
	StringReplaceAll(strTmp, _T(","), _T("，"));
	strLine = strLine + _T(",") + strTmp;
	cout << _T("GetContactData::KickName:") << szTmp << endl;

	//微信号//
	matTempl = imread(strWxid);
	ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
	ret = MatchTemplOnWindow(hwndCur, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.95);
	if (ret) {
		GetWindowRect(hwndCur, &rcTemp);
		pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
		SetCursorPos(pt.x + 50, pt.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);

		//ctrl-c
		keybd_event(VK_CONTROL, 0, 0, 0);
		keybd_event(0x43, 0, 0, 0);
		keybd_event(0x43, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
		Sleep(100);

		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(200);

		ZeroMemory(szTmp, SZ_CHAR_SIZE);
		ZeroMemory(szTmpW, sizeof(WCHAR)*SZ_CHAR_SIZE);
		GetClipBoardUnicode(hwndCur, szTmpW, SZ_CHAR_SIZE);
		UnicodeToUtf8(szTmpW, szTmp, SZ_CHAR_SIZE);
		strTmp = string(szTmp);
		//StringReplaceAll(strTmp, string(_T(",")), string(_T("，")));
		strLine = strLine + _T(",") + strTmp;
		cout << _T("GetContactData::WxID:") << szTmp << endl;

		if (CheckWxIDExist(szTmp)) {
			return -1;
			cout << _T("CheckWxIDExist::TRUE");
		}
		else
			g_strWxid_cur[uIndex++] = string(szTmp);
	}
	else {
		strLine = strLine + _T(",");
	}

	//性别//
	matTempl = imread(strWxmale);
	//cout << strWxmale.data() << endl;
	ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
	ret = MatchTemplOnWindow(hwndCur, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.95);
	if (ret) {
		strLine = strLine + _T(",男");
		cout << _T("GetContactData::Wxsex:男") << endl;
	}
	else
	{
		matTempl = imread(strWxfemale);
		ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
		ret = MatchTemplOnWindow(hwndCur, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.95);
		if (ret) {
			strLine = strLine + _T(",女");
			cout << _T("GetContactData::Wxsex:女") << endl;
		}
		else
		{
			strLine = strLine + _T(",未知");
			cout << _T("GetContactData::Wxsex:未知") << endl;
		}
	}

	//地区//
	matTempl = imread(strWxarea);
	ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
	ret = MatchTemplOnWindow(hwndCur, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.95);
	if (ret) {
		GetWindowRect(hwndCur, &rcTemp);
		pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
		SetCursorPos(pt.x + 40, pt.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);

		//ctrl-c
		keybd_event(VK_CONTROL, 0, 0, 0);
		keybd_event(0x43, 0, 0, 0);
		keybd_event(0x43, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
		Sleep(100);

		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(200);

		ZeroMemory(szTmp, SZ_CHAR_SIZE);
		ZeroMemory(szTmpW, sizeof(WCHAR)*SZ_CHAR_SIZE);
		GetClipBoardUnicode(hwndCur, szTmpW, SZ_CHAR_SIZE);
		UnicodeToUtf8(szTmpW, szTmp, SZ_CHAR_SIZE);
		strTmp = string(szTmp);
		StringReplaceAll(strTmp, _T(","), _T("，"));
		strLine = strLine + _T(",") + strTmp;
		cout << _T("GetContactData::WxArea:") << szTmp << endl;
	}
	else {
		strLine = strLine + _T(",");
	}

	//标签//
	matTempl = imread(strWxlabel);
	ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
	ret = MatchTemplOnWindow(hwndCur, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.95);
	if (ret) {
		GetWindowRect(hwndCur, &rcTemp);
		pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
		SetCursorPos(pt.x + 70, pt.y);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(500);

		//ctrl-c
		keybd_event(VK_CONTROL, 0, 0, 0);
		keybd_event(0x43, 0, 0, 0);
		keybd_event(0x43, 0, KEYEVENTF_KEYUP, 0);
		keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
		Sleep(100);

		mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		Sleep(200);

		ZeroMemory(szTmp, SZ_CHAR_SIZE);
		ZeroMemory(szTmpW, sizeof(WCHAR)*SZ_CHAR_SIZE);
		GetClipBoardUnicode(hwndCur, szTmpW, SZ_CHAR_SIZE);
		UnicodeToUtf8(szTmpW, szTmp, SZ_CHAR_SIZE);
		strTmp = string(szTmp);
		StringReplaceAll(strTmp, _T(","), _T("，"));
		strLine = strLine + _T(",") + strTmp;
		cout << _T("GetContactData::WxLabel:") << szTmp << endl;
	}
	else {
		strLine = strLine + _T(",");
	}

	//来源//
	matTempl = imread(strWxsource);
	ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
	ret = MatchTemplOnWindow(hwndCur, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.95);
	if (ret) {
		GetWindowRect(hwndCur, &rcTemp);
		strTmp = Tesseract(hwndCur, rcTemp, szRc[0], strDir);
		cout << _T("GetContactData::WxSource:") << strTmp.data() << endl;
		strLine = strLine + _T(",") + strTmp;
	}
	else {
		strLine = strLine + string(_T(","));
	}

	//焦点返回窗口//
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	Sleep(500);


	StringReplaceAll(strLine, _T("\n"), _T(""));

	strLine = strLine + string(_T(",")) + strFriend + string(_T("\n"));

	string strContact = strDir + WX_CONTACT_FILE;

	FILE* fp;
	ret = fopen_s(&fp, strContact.data(), _T("a+"));
	if (ret) {
		cout << _T("Unexpected aborted: can't open temporary file") << endl;
		exit(1);
	}
	fwrite(strLine.data(), sizeof(char), strLine.length(), fp);
	fclose(fp);

	return 0;
}

void InitWork_GetContact()
{
	int iScale = GetScreenScale()*100;
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
		"1. 请打开微信电脑版，点击\"通信录\"栏目，然后打开\"通信录管理\"窗口\n"
		"2. 整理通讯录可能需要比较长的时间，请耐心等待运行结束。如果要终止运行，请按\"F10\"按键\n"
		"3. 结束后请查看工作目录下的wechat_contact.csv文件，可用Excel或者WPS打开文件进行操作\n"
		"4. 程序运行服务时，请不要做任何手动操作。如果要终止运行，请按\"F10\"按键\n"
		"   了解上述要求后， 可按任意键继续...\n")
	);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	system(_T("pause"));
}

int WechatGetOneByLabel()
{
	HWND hwndWechat = NULL, hwndConsole = NULL;
	uint uLen;
	int i, j, ret = 0, ret1 = 0;
	Mat matTempl;

	string strTmp;
	POINT pt;
	RECT* ptrNULL = NULL;
	CvRect szRc[UINT_SIZEOFRECT] = { 0 };
	CvRect szRc1[UINT_SIZEOFRECT] = { 0 };
	RECT rcTemp, rcTemp1, rectWechat, rcOut;

	RECT rectKickName = { 130, 40, 350, 80 };  //设置比对区域;

	InitWork_GetContact();

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
	else if(dbscale == 1)
		strRes = _T("res100\\");
	else {
		cout << _T("Unexpected aborted: not support current screen scale.") << endl;
		return -1;
	}

	string strPngKickname = strDir +strRes+_T("kickname.png");
	if ((_access(strPngKickname.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:kickname.png") << endl;
		return -1;
	}

	string strPngSquare = strDir +strRes+_T("square.png");
	if ((_access(strPngSquare.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:square.png") << endl;
		return -1;
	}

	string strPngJustTalk = strDir +strRes+_T("wxjusttalk.png");
	if ((_access(strPngJustTalk.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxjusttalk.png") << endl;
		return -1;
	}

	string strPngBanOther = strDir +strRes+_T("wxbanother.png");
	if ((_access(strPngBanOther.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxbanother.png") << endl;
		return -1;
	}

	string strPngBanMySelf = strDir +strRes+_T("wxbanmyself.png");
	if ((_access(strPngBanMySelf.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxbanmyself.png") << endl;
		return -1;
	}

	string strPngMemo = strDir +strRes+_T("wxmemo.png");
	if ((_access(strPngMemo.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxmemo.png") << endl;
		return -1;
	}

	string strPngLabel = strDir +strRes+_T("wxlabel.png");
	if ((_access(strPngLabel.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxlabel.png") << endl;
		return -1;
	}

	//string strPngMore = strDir +strRes+_T("wxmore.png");
	//if ((_access(strPngMore.data(), 0)) != 0) {
	//	cout << _T("Unexpected aborted: can't read resource data:wxmore.png") << endl;
	//	return -1;
	//}

	string strPngMale = strDir +strRes+_T("wxmale.png");
	if ((_access(strPngMale.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxmale.png") << endl;
		return -1;
	}

	string strPngFemale = strDir +strRes+_T("wxfemale.png");
	if ((_access(strPngFemale.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxfemale.png") << endl;
		return -1;
	}

	string strPngSource = strDir +strRes+_T("wxsource.png");
	if ((_access(strPngSource.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxsource.png") << endl;
		return -1;
	}

	string strPngArea = strDir +strRes+_T("wxarea.png");
	if ((_access(strPngArea.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxarea.png") << endl;
		return -1;
	}

	string strPngId = strDir +strRes+_T("wxid.png");
	if ((_access(strPngId.data(), 0)) != 0) {
		cout << _T("Unexpected aborted: can't read resource data:wxid.png") << endl;
		return -1;
	}

	double dbScale = GetScreenScale();

	string strContact = strDir + WX_CONTACT_FILE;
	FILE* fp;
	ret = fopen_s(&fp, strContact.data(), _T("w+"));
	if (ret) {
		cout << _T("Unexpected aborted: : can't open temporary file") << endl;
		exit(1);
	}
	char utf8bom[] = { 0xEF,0xBB,0xBF };
	fwrite(utf8bom, sizeof(char), 3, fp);
	strTmp = _T("编号,备注,昵称,微信号,性别,地区,标签,来源,朋友权限\n");
	fwrite(strTmp.data(), sizeof(char), strTmp.length(), fp);
	fclose(fp);

	int  cx = GetSystemMetrics(SM_CXSCREEN);
	int  cy = GetSystemMetrics(SM_CYSCREEN);
	hwndConsole = GetConsoleWindow();
	if (!hwndConsole) {
		cout << _T("Unexpected aborted: can't find ConsoleWindow") << endl;
		return -1;
	}
	SetWindowPos(hwndConsole, NULL, cx - POS_CONWIDTH, POS_CONY, POS_CONWIDTH, POS_CONHEIGHT, SWP_SHOWWINDOW);
	Sleep(500);

	hwndWechat= FindWindow(_T("ContactManagerWindow"), NULL);
	if (!hwndWechat) {
		cout << _T("Unexpected aborted: can't find Wechat - ContactManagerWindow") << endl;
		return -1;
	}
	GetWindowRect(hwndWechat,&rectWechat);
	SetWindowPos(hwndWechat, NULL, POS_WECHATX, POS_WECHATY, POS_WECHATWIDTH, POS_WECHATHEIGHT, SWP_SHOWWINDOW);
	SetForegroundWindow(hwndWechat);
	Sleep(1000);

	HANDLE hThread1;
	DWORD uID1;
	hThread1 = CreateThread(0, 0, ThreadFuncGetMsg, NULL, 0, &uID1);
	WaitForSingleObject(hThread1, INFINITY);
	CloseHandle(hThread1);

	int iRecord = 1;
	uint uIndex = 0;

	if (hwndWechat) {

		while (TRUE) {
			if (GetForegroundWindow() != hwndWechat) {
				SetForegroundWindow(hwndWechat);
				Sleep(1000);
			}
			SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
			cout << _T("WechatGetOneByLabel::square.png:") << endl;
			//matTempl = imread(strPngSquare);
			//ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
			//ret = MatchTemplOnWindow(hwndWechat, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9);
			//if (ret) {
			//	GetWindowRect(hwndWechat, &rcTemp);
			//	pt.x = rcTemp.left + szRc[0].x + szRc[0].width / 2; pt.y = rcTemp.top + szRc[0].y + szRc[0].height / 2;
			//	SetCursorPos(pt.x, pt.y);
			//	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//	Sleep(1000);
			//	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			//	Sleep(1000);
			//}
			matTempl = imread(strPngSquare);
			ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
			ret = MatchTemplOnWindow(hwndWechat, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.9, 0);
			if (ret) {
				for (i = 0; i < ret; i++) {
					/*if (GetForegroundWindow() != hwndWechat) {
						SetForegroundWindow(hwndWechat);
						Sleep(1000);
					}*/
					rcOut.left = szRc[i].x; rcOut.top = szRc[i].y; rcOut.right = szRc[i].x + szRc[i].width; rcOut.bottom = szRc[i].y + szRc[i].height;
					//cout << _T("WechatFindGroupName::rcOut:") << rcOut.left <<SP << rcOut.top<<SP << rcOut.right<<SP<< rcOut.bottom<<  endl;
					//cout << _T("WechatFindGroupName::rectKickName:") << rectKickName.left << SP << rectKickName.top << SP << rectKickName.right << SP << rectKickName.bottom << endl;
					if (!IsInterSection(rectKickName, rcOut)) {
						MarkClientRect(hwndWechat, szRc[i], Scalar(54, 81, 231));
						//cout << _T("WechatFindGroupName::szRc[i]:") << szRc[i].x << SP << szRc[i].y << endl;
						//cout << _T("WechatFindGroupName::SetCursorPos:") << pt.x << SP << pt.y << endl;
						GetWindowRect(hwndWechat, &rcTemp);
						rcTemp1.left = rcTemp.left + szRc[i].x; rcTemp1.top = rcTemp.top + szRc[i].y - 10; 
						rcTemp1.right = rcTemp.right; rcTemp1.bottom = rcTemp.top + szRc[i].y + szRc[i].height + 10;
						matTempl = imread(strPngJustTalk);
						ZeroMemory(szRc1, sizeof(CvRect[UINT_SIZEOFRECT]));
						int ret2 = MatchTemplOnWindow(hwndWechat, matTempl, &rcTemp1, szRc1, UINT_SIZEOFRECT, 0.95);
						matTempl = imread(strPngBanOther);
						ZeroMemory(szRc1, sizeof(CvRect[UINT_SIZEOFRECT]));
						int ret3 = MatchTemplOnWindow(hwndWechat, matTempl, &rcTemp1, szRc1, UINT_SIZEOFRECT, 0.95);
						matTempl = imread(strPngBanMySelf);
						ZeroMemory(szRc1, sizeof(CvRect[UINT_SIZEOFRECT]));
						int ret4 = MatchTemplOnWindow(hwndWechat, matTempl, &rcTemp1, szRc1, UINT_SIZEOFRECT, 0.95);

						string strFriend;
						if (ret2) {
							strFriend = _T("仅聊天");
						}
						else if (ret3 & ret4) {
							strFriend = _T("不让他（她）看，不看他（她）");
						}
						else if (ret3) {
							strFriend = _T("不让他（她）看");
						}
						else if (ret4) {
							strFriend = _T("不看他（她）");
						}
						else {
							strFriend = _T("聊天、朋友圈、微信运动等");
						}

						GetWindowRect(hwndWechat, &rcTemp);
						pt.x = rcTemp.left + szRc[i].x + szRc[i].width / 2; pt.y = rcTemp.top + szRc[i].y + szRc[i].height / 2;
						SetCursorPos(pt.x + 40, pt.y);
						mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
						Sleep(1000);

						GetContactDetail(strDir, strPngId, strPngMemo, strPngSource, strPngArea, strPngLabel, strPngMale, strPngFemale, 
							strFriend, strPngKickname, iRecord++, uIndex);

						if (GetForegroundWindow() != hwndWechat) {
							//ESCAPE
							keybd_event(VK_ESCAPE, 0, 0, 0);
							keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
							//cout << _T("WechatGetOneByLabel::keybd_event:VK_ESCAPE") << endl;
							Sleep(500);
						}
						//pt.x = rcTemp.left + 40; pt.y = rcTemp.top + 15;
						//SetCursorPos(pt.x, pt.y);
						//mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
						//Sleep(500);
					}//end if
				}// end for
			}//if square

			//cout << _T("WechatGetOneByLabel::GetClientWindowEx.png") << endl;
			matTempl = GetClientWindowEx(hwndWechat);

			//PAGE DOWN
			keybd_event(VK_NEXT, 0, 0, 0);
			keybd_event(VK_NEXT, 0, KEYEVENTF_KEYUP, 0);
			cout << _T("WechatGetOneByLabel::keybd_event:VK_NEXT") << endl;
			Sleep(1000);

			ZeroMemory(szRc, sizeof(CvRect[UINT_SIZEOFRECT]));
			ret = MatchTemplOnWindow(hwndWechat, matTempl, ptrNULL, szRc, UINT_SIZEOFRECT, 0.99);
			if (ret)
				break;

			for (i = 0; i < LISTSIZE; i++)
			{
				g_strWxid_save[i] = g_strWxid_cur[i];
				g_strWxid_cur[i] = _T("");
			}
			uIndex = 0;
		}//while
	}

	return 0;
}
