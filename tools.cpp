#include "stdafx.h"

using namespace std;

extern BOOL GetFileContext(string strPath, string& strBuf);
extern void SetMsgToClipBoard(HWND hwnd, string strTmp);
extern void StringSplit(const string& s, vector<string>& tokens, const string& delimiters);

extern DWORD WINAPI ThreadFuncGetMsg(LPVOID lpParam);//线程函数


#define CACHE_FILE            _T("pastecontent.csv")

BOOL g_pasteId = FALSE;

int PasteEx()
{
	HWND hwndMain, hwndChild, hwndDlg, hwndConsole;
	int i, j, ret;

	POINT pt;
	string strLine;

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


	hwndConsole = GetConsoleWindow();
	if (!hwndConsole) {
		cout << _T("Unexpected aborted: can't find ConsoleWindow") << endl;
		return -1;
	}

	string strTbl = strDir + CACHE_FILE;
	string strContent;
	if (!GetFileContext(strTbl, strContent)) {
		cout << _T("Unexpected aborted: can't read file") << endl;
		return -1;
	}

	vector<string> vtStrLine;

	StringSplit(strContent, vtStrLine, _T("\n"));

	g_pasteId = FALSE;
	i = 0;
	SetMsgToClipBoard(hwndConsole, vtStrLine[i]);
	while (1) {
		if (g_pasteId) {
			i++;
			if (i < vtStrLine.size()) {
				SetMsgToClipBoard(hwndConsole, vtStrLine[i]);
			}else{
				return 0;
			}
			g_pasteId = FALSE;
		}
		Sleep(100);
	}
	return 0;
}