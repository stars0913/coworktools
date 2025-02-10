#include "stdafx.h"

using namespace std;

extern void UnicodeToUtf8(const WCHAR* pchSrc, char* pchDest, int nDestLen);
extern void Utf8ToUnicode(const char* pchSrc, WCHAR* pchDest, int nDestLen);
extern void AnsiToUtf8(const char* pchSrc, char* pchDest, int nDestLen);
extern bool is_str_utf8(const char* str);
extern bool is_str_gbk(const char* str);
extern UINT GetFileTypeEx(FILE* fp);

//
//函数识别文件编码，并最终得到UTF8编码内容
//
BOOL GetFileContext(string strPath,string& strBuf)
{
	FILE* fp;
	int ret = fopen_s(&fp, strPath.data(), _T("r+"));
	if (ret) {
		cout << _T("Unexpected aborted: can't open file: ") << strPath.data() << endl;
		return FALSE;
	}

	long iSize = _filelength(_fileno(fp));
	char* pTmp = new char[iSize];
	ZeroMemory(pTmp, sizeof(char)*iSize);
	fread(pTmp, sizeof(char), iSize, fp);
	long iLenth = strlen(pTmp);
	UINT uFtype = GetFileTypeEx(fp);
	fclose(fp);

	//cout << _T("uFtype: ") << uFtype << endl;
	//cout << _T("pTmp.len: ") << strlen(pTmp) << endl;

	if (uFtype == 1 && is_str_gbk(pTmp)) {
		char* pUtf8 = new char[iLenth * 2];
		ZeroMemory(pUtf8, sizeof(char)*iLenth * 2);
		AnsiToUtf8(pTmp, pUtf8, iLenth * 2);
		strBuf = pUtf8;
		delete[] pUtf8;
	}
	else if (uFtype == 1) {
		strBuf = pTmp;     //默认为UTF8
	}
	else if (uFtype == 3) {
		strBuf = pTmp + 3; //UTF8-BOM,去掉BOM头
	}
	else {
		cout << _T("Unexpected aborted: unexpect coding file") << endl;
		return FALSE;
	}
	delete[] pTmp;
	//cout << _T("strContent.len: ") << strContent.length() << endl;

	return TRUE;
}

void ctrl_v_low()
{
	string cmd = _T("spaste.exe");
	system(cmd.data());
	//Sleep(2000);
}

void ctrl_c_low()
{
	string cmd = _T("scopy.exe");
	system(cmd.data());
	//Sleep(2000);
}

void escape_low()
{
	string cmd = _T("sesc.exe");
	system(cmd.data());
}

void home_low()
{
	string cmd = _T("shome.exe");
	system(cmd.data());
}

void ctrl_c()
{
	INPUT input[4];
	memset(input, 0, sizeof(input));

	input[0].type = input[1].type = input[2].type = input[3].type = INPUT_KEYBOARD;  //设置键盘模式  

	input[0].ki.wVk = input[3].ki.wVk = VK_CONTROL;
	input[1].ki.wVk = input[2].ki.wVk = 'C';

	input[2].ki.dwFlags = input[3].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(4, input, sizeof(INPUT));
}

void ctrl_v()
{
	INPUT input[4];
	memset(input, 0, sizeof(input));

	input[0].type = input[1].type = input[2].type = input[3].type = INPUT_KEYBOARD;  //设置键盘模式  

	input[0].ki.wVk = input[3].ki.wVk = VK_CONTROL;
	input[1].ki.wVk = input[2].ki.wVk = 'V';

	input[2].ki.dwFlags = input[3].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(4, input, sizeof(INPUT));
}

void ctrl_a()
{
	INPUT input[4];
	memset(input, 0, sizeof(input));

	input[0].type = input[1].type = input[2].type = input[3].type = INPUT_KEYBOARD;  //设置键盘模式  

	input[0].ki.wVk = input[3].ki.wVk = VK_CONTROL;
	input[1].ki.wVk = input[2].ki.wVk = 'A';

	input[2].ki.dwFlags = input[3].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(4, input, sizeof(INPUT));
}

void StringSplit(const string& s, vector<string>& tokens, const string& delimiters)
{
	string::size_type lastPos = s.find_first_not_of(delimiters, 0);
	string::size_type pos = s.find_first_of(delimiters, lastPos);
	while (string::npos != pos || string::npos != lastPos) {
		tokens.push_back(s.substr(lastPos, pos - lastPos));//use emplace_back after C++11
		lastPos = s.find_first_not_of(delimiters, pos);
		pos = s.find_first_of(delimiters, lastPos);
	}
}

BOOL IsInterSection(RECT& rcSrc, RECT& rcDes)
{
	if (rcSrc.left > rcDes.right || rcSrc.right < rcDes.left) {
		return FALSE;
	}
	else {
		if (rcSrc.top > rcDes.bottom || rcSrc.bottom < rcDes.top)
			return FALSE;
	}

	if (rcSrc.top > rcDes.bottom || rcSrc.bottom < rcDes.top) {
		return FALSE;
	}
	else {
		if (rcSrc.left > rcDes.right || rcSrc.right < rcDes.left) 
			return FALSE;
	}
	return TRUE;
}

void SetClientText(HWND hwnd, char* ptrStr, DWORD dwLen)
{
	WCHAR* ptrWStr = new WCHAR[dwLen+1];
	ZeroMemory(ptrWStr, sizeof(WCHAR)*(dwLen+1));
	Utf8ToUnicode(ptrStr, ptrWStr, dwLen+1);

	dwLen = wcslen(ptrWStr);

	for (int i = 0; i < dwLen; i++)
	{
		PostMessage(hwnd, WM_UNICHAR, ptrWStr[i], 0);
		//cout << ptrWStr[i] << endl;
		/*if (0 == ptrWStr[i])
			break;*/
	}
	PostMessage(hwnd, WM_UNICHAR, VK_SPACE, 0);
	//PostMessage(hwnd, WM_CHAR, VK_BACK, 0);

	delete[] ptrWStr;
}

//
//设置剪贴板文本
//
bool SetClipBoardText(LPCSTR text, HWND hWnd)
{
	HANDLE hClip = NULL;
	//打开剪贴板  
	if (!::OpenClipboard(hWnd))
		return false;
	//如果剪贴板有内容则关闭剪贴板
	if (!EmptyClipboard())
	{
		CloseClipboard();
		return false;
	}
	//获取需要设置文本的长度  
	int len = strlen(text);
	//申请剪贴板空间
	hClip = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1)*sizeof(char));
	if (hClip == NULL)
	{
		CloseClipboard();
		return false;
	}
	//给申请的空间加锁
	char* pBuf = (char*)GlobalLock(hClip);
	if (pBuf == NULL)
	{
		GlobalFree(hClip);
		CloseClipboard();
		return false;
	}
	//拷贝文本内容到剪贴板
	memcpy(pBuf, text, len*sizeof(char));
	pBuf[len] = NULL;
	//操作完成，释放锁
	GlobalUnlock(hClip);
	if (NULL == SetClipboardData(CF_TEXT, hClip))
	{
		GlobalFree(hClip);
		CloseClipboard();
		return false;
	}
	CloseClipboard();
	return true;
}

bool SetClipBoardUnicode(WCHAR* text, HWND hWnd)
{
	HANDLE hClip = NULL;
	//打开剪贴板  
	if (!::OpenClipboard(hWnd))
		return false;
	//如果剪贴板有内容则关闭剪贴板
	//if (!EmptyClipboard())
	//{
	//	CloseClipboard();
	//	return false;
	//}

	//获取需要设置文本的长度  
	int len = wcslen(text);
	//申请剪贴板空间
	hClip = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(WCHAR));
	if (hClip == NULL)
	{
		CloseClipboard();
		return false;
	}
	//给申请的空间加锁
	WCHAR* pBuf = (WCHAR*)GlobalLock(hClip);
	if (pBuf == NULL)
	{
		GlobalFree(hClip);
		CloseClipboard();
		return false;
	}
	//拷贝文本内容到剪贴板
	memcpy(pBuf, text, (len+1) * sizeof(WCHAR));
	pBuf[len] = NULL;
	//操作完成，释放锁
	GlobalUnlock(hClip);
	if (NULL == SetClipboardData(CF_UNICODETEXT, hClip))
	{
		GlobalFree(hClip);
		CloseClipboard();
		return false;
	}
	CloseClipboard();
	return true;
}

//
//获取剪贴板文本
//
int GetClipBoardUnicode(HWND hWnd, WCHAR* pDes, int iSize)
{
	int iRet;
	WCHAR* pStr = NULL;
	//判断剪贴板的数据格式是否可以处理。  
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
		return 0;

	//打开剪贴板。          
	if (!::OpenClipboard(hWnd))
		return 0;

	//获取数据  
	HANDLE hMem = GetClipboardData(CF_UNICODETEXT);
	if (hMem != NULL)
	{
		//获取字符串。  
		pStr = (WCHAR*)GlobalLock(hMem);
		if (pStr != NULL)
		{
			iRet = wcslen(pStr);

			if (iRet >= iSize)
				iRet = iSize - 1;

			wcsncpy(pDes, pStr, iRet);

			//释放锁内存  
			GlobalUnlock(hMem);
		}
	}
	//关闭剪贴板        
	CloseClipboard();
	return iRet;
}

//
//获取剪贴板文本
//
int GetClipBoardText(HWND hWnd, LPSTR pDes, int iSize)
{
	int iRet;
	LPSTR pStr = NULL;
	//判断剪贴板的数据格式是否可以处理。  
	if (!IsClipboardFormatAvailable(CF_TEXT))
		return 0;

	//打开剪贴板。          
	if (!::OpenClipboard(hWnd))
		return 0;

	//获取数据  
	HANDLE hMem = GetClipboardData(CF_TEXT);
	if (hMem != NULL)
	{
		//获取字符串。  
		pStr = (LPSTR)GlobalLock(hMem);
		if (pStr != NULL)
		{
			iRet = strlen(pStr);

			if (iRet >= iSize)
				iRet = iSize - 1;

			strncpy(pDes, pStr, iRet);

			//释放锁内存  
			GlobalUnlock(hMem);
		}
	}
	//关闭剪贴板        
	CloseClipboard();
	return iRet;
}



int CmdMode()
{
	char szCmd[64];
	while(TRUE)
	{
		printf_s(_T("Command:\n"));
		scanf_s(_T("%s"), szCmd,32);

		if(!strcmp(szCmd, "pos"))
		{
			POINT pt;
			GetCursorPos(&pt);
			printf_s(_T("Current cursor position: %d, %d \n"), pt.x, pt.y);
		}

		if(!strcmp(szCmd, "quit"))
		{
			return 0;
		}
	}
	return 0;
}


int Split(char* pSrc, char* pSymbol, char* pOut, int iMaxRow, int iMaxCol)
{
	char *s = NULL, *e = NULL;
	char *p = pOut;
	int iCol = 0, iRow = 0;
	s = pSrc;

	while(*s)
	{
		e = strstr(s, pSymbol);
		iCol = 0;
		while(s!=e && *s && iRow < iMaxRow && iCol < iMaxCol)
		{
			if(UINT(*s) >= 30)
				p[iCol++] = *s; 
			s++;
		}
		p += iMaxCol;
		if(e)
			s = e+1;
		iRow++;
	}
	return iRow;
}

int GetCfgString(char* pSrc, char* pSection, char* pS, char* pE, char* pOut, int iMax)
{
	char *sec = NULL, *s = NULL, *e = NULL;
	int iLen = 0;

	sec = strstr(pSrc, pSection) + strlen(pSection);
	if(!sec)
		goto err;

	s = strstr(sec, pS) + strlen(pS);
	if(!s)
		goto err;

	e = strstr(s, pE);
	if(!e)
		goto err;

	while(s!=e && iLen<iMax)
	{
		if(UINT(*s) >= 30)
			pOut[iLen++] = *s;
		s++;
	}
	pOut[iLen++] = 0;
	return iLen;

err:
	return 0;
}

//输入英文字母组合字串，逐个输入
void KeyInput(char* pBuf)
{
	int iCout = strlen(pBuf);

	keybd_event(VK_ESCAPE, 0, 0, 0);
	for(int i=0; i<iCout; i++)
	{
		int k = pBuf[i];
		keybd_event(k, 0, 0, 0);
	}
	keybd_event(VK_RETURN, 0, 0, 0);
}

//读取一行文本或者指定字符数
int ReadOneLine(FILE* fp, char* pBuf, int iMax)
{
	int iCout = 0;
	char p;
	while(!feof(fp))
	{
		p = fgetc(fp);
		if(p == '\n' || iCout + 1 == iMax)
		{
			pBuf[iCout++] = 0;
			return (int)strlen(pBuf);
		}
		if(p>=30)
			pBuf[iCout++] = p;
	}
	pBuf[iCout++] = 0;
	return (int)strlen(pBuf);
}

UINT GetOneLine(char* ptrStr, UINT uSize, UINT uStart = 0)
{
	UINT i;
	char* p = ptrStr + uStart;

	for (i = 0; i < uSize; i++,p++ ) {
		if (*p == '\n') {
			return i;
		}
	}
	return i;
}

void SetMsgToClipBoard(HWND hwnd, string strTmp)
{
	int iLen = strTmp.size()+1;
	WCHAR* pUni = new WCHAR[iLen];
	memset(pUni, 0, sizeof(WCHAR) * iLen);
	Utf8ToUnicode(strTmp.data(), pUni, iLen);
	if (SetClipBoardUnicode(pUni, hwnd) == false) {
		cout << _T("Warning: fail to SetClipBoardUnicode.") << endl;
		delete[] pUni;
		return;
	}
	//cout << pUni << endl;
	delete[] pUni;
}
