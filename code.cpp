#include "stdafx.h"

using namespace std;


UINT GetFileTypeEx(FILE* fp) 
{
	if (fp == NULL)
		return 0;

	long iSize = _filelength(_fileno(fp));
	if (iSize < 4)
		return 0;

	char szTmp[4] = { 0 };
	fseek(fp, 0, 0);
	fread(szTmp, sizeof(char), 3, fp);
	UINT p = szTmp[0] & 0xff;
	p = (p << 8) + UINT(szTmp[1] & 0xff);
	p = (p << 8) + UINT(szTmp[2] & 0xff);

	//printf("FILE HEAD: %x \n", p);

	switch ((p>>8) & 0xffff)//判断文本前两个字节
	{
	case 0xfffe://65534 "Unicode";
		return 4;
		//break;
	case 0xfeff://65279  "Unicode big endian";
		return 5;
		//break;
	case 0xefbb://utf8-bom
		if(p == 0xefbbbf)
			return 3;
		//break;
	default:
		return 1;
	}
}

/*
函数说明：对字符串中所有指定的子串进行替换
参数：
string resource_str           //源字符串
string sub_str                //被替换子串
string new_str                //替换子串
返回值: string
*/
void StringReplaceAll(string& resource_str, string sub_str, string new_str)
{
	string::size_type pos = 0;
	while ((pos = resource_str.find(sub_str)) != string::npos) 
	{
		resource_str.replace(pos, sub_str.length(), new_str);
	}
}

void Utf8EraseChar(char* pDes, char sub)
{
	char* p;
	if (sub > 127)
		return;

	while ((p=strchr(pDes,sub)) != NULL)
	{
		while (*p != 0) {
			*p = *(p + 1);
			p++;
		}
	}
}

bool is_str_utf8(const char* str)
{
	unsigned int nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节//
	unsigned char chr = *str;
	bool bAllAscii = true;
	for (unsigned int i = 0; str[i] != '\0'; ++i) {
		chr = *(str + i);
		//判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx//
		if (nBytes == 0 && (chr & 0x80) != 0) {
			bAllAscii = false;
		}
		if (nBytes == 0) {
			//如果不是ASCII码,应该是多字节符,计算字节数//
			if (chr >= 0x80) {
				if (chr >= 0xFC && chr <= 0xFD) {
					nBytes = 6;
				}
				else if (chr >= 0xF8) {
					nBytes = 5;
				}
				else if (chr >= 0xF0) {
					nBytes = 4;
				}
				else if (chr >= 0xE0) {
					nBytes = 3;
				}
				else if (chr >= 0xC0) {
					nBytes = 2;
				}
				else {
					return false;
				}//
				nBytes--;
			}
		}
		else {
			//多字节符的非首字节,应为 10xxxxxx//
			if ((chr & 0xC0) != 0x80) {
				return false;
			}
			//减到为零为止//
			nBytes--;
		}
	}
	//违返UTF8编码规则//
	if (nBytes != 0) {
		return false;
	}
	if (bAllAscii) { //如果全部都是ASCII, 也是UTF8//
		return true;
	}
	return true;
}

bool is_str_gbk(const char* str)
{
	unsigned int nBytes = 0;//GBK可用1-2个字节编码,中文两个 ,英文一个
	unsigned char chr = *str;
	bool bAllAscii = true; //如果全部都是ASCII,
	for (unsigned int i = 0; str[i] != '\0'; ++i) {
		chr = *(str + i);
		if ((chr & 0x80) != 0 && nBytes == 0) {// 判断是否ASCII编码,如果不是,说明有可能是GBK
			bAllAscii = false;
		}
		if (nBytes == 0) {
			if (chr >= 0x80) {
				if (chr >= 0x81 && chr <= 0xFE) {
					nBytes = +2;
				}
				else {
					return false;
				}
				nBytes--;
			}
		}
		else {
			if (chr < 0x40 || chr>0xFE) {
				return false;
			}
			nBytes--;
		}//else end
	}
	if (nBytes != 0) {   //违返规则
		return false;
	}
	if (bAllAscii) { //如果全部都是ASCII, 也是GBK
		return true;
	}
	return true;
}

/*=============================================================================
函 数 名: Utf8ToAnsi
功    能: 实现将char型buffer(utf8编码)中的内容安全地拷贝到指定的char型的buffer(ANSI编码)中
参    数: char* pchSrc [in]          源字符串
		  char* pchDest [out]        目标buf
		  int nDestLen [in]          目标buf长度(注意：以字节为单位，不是以字符个数为单位)
注    意: 无
返 回 值: 无
=============================================================================*/
void Utf8ToAnsi(const char* pchSrc, char* pchDest, int nDestLen)
{
	if (pchSrc == NULL || pchDest == NULL)
	{
		return;
	}

	// 先将utf8转成Unicode//
	int nUnicdeBufLen = MultiByteToWideChar(CP_UTF8, 0, pchSrc, -1, NULL, 0);
	WCHAR* pUnicodeTmpBuf = new WCHAR[nUnicdeBufLen + 1];
	memset(pUnicodeTmpBuf, 0, (nUnicdeBufLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, pchSrc, -1, pUnicodeTmpBuf, nUnicdeBufLen + 1);

	// 再将Unicode转成Ansi
	int nAnsiBuflen = WideCharToMultiByte(CP_ACP, 0, pUnicodeTmpBuf, -1, NULL, 0, NULL, NULL);
	char* pAnsiTmpBuf = new char[nAnsiBuflen + 1];
	memset(pAnsiTmpBuf, 0, nAnsiBuflen + 1);
	WideCharToMultiByte(CP_ACP, 0, pUnicodeTmpBuf, -1, pAnsiTmpBuf, nAnsiBuflen + 1, NULL, NULL);

	int nLen = strlen(pAnsiTmpBuf);
	if (nLen + 1 > nDestLen)
	{
		strncpy(pchDest, pAnsiTmpBuf, nDestLen - 1);
		pchDest[nDestLen - 1] = 0;
	}
	else
	{
		strcpy(pchDest, pAnsiTmpBuf);
	}

	delete[]pAnsiTmpBuf;
	delete[]pUnicodeTmpBuf;
}


/*=============================================================================
函 数 名: UnicodeToUtf8
功    能: 实现将WCHAR型buffer(Unicode编码)中的内容安全地拷贝到指定的char型的buffer(utf8编码)中
参    数: WCAHR* pchSrc [in]          源字符串
		  char* pchDest [out]         目标buf
		  int nDestLen [in]           目标buf长度(注意：以字节为单位，不是以字符个数为单位)
注    意: 无
返 回 值: 无
=============================================================================*/
void UnicodeToUtf8(const WCHAR* pchSrc, char* pchDest, int nDestLen)
{
	if (pchDest == NULL || pchSrc == NULL)
	{
		return;
	}

	const WCHAR* pWStrSRc = pchSrc;
	int nTmplen = WideCharToMultiByte(CP_UTF8, 0, pWStrSRc, -1, NULL, 0, NULL, NULL);
	char* pTemp = new char[nTmplen + 1];
	memset(pTemp, 0, nTmplen + 1);
	WideCharToMultiByte(CP_UTF8, 0, pWStrSRc, -1, pTemp, nTmplen + 1, NULL, NULL);

	int nLen = strlen(pTemp);
	if (nLen + 1 > nDestLen)
	{
		strncpy(pchDest, pTemp, nDestLen - 1);
		pchDest[nDestLen - 1] = 0;
	}
	else
	{
		strcpy(pchDest, pTemp);
	}

	delete[] pTemp;
}

/*=============================================================================
函 数 名: UnicodeToAnsi
功    能: 实现将WCHAR型buffer（Unicode编码）中的内容安全地拷贝到指定的char型（ANSI编码）的buffer中
参    数: WCHAR*  pchSrc [in]         源字符串
	      char*   pchDest[out]        目标buf
		  int nDestLen [in]           目标buf长度(注意：以字节为单位，不是以字符个数为单位)
注    意: 无
返 回 值: 无
=============================================================================*/
void UnicodeToAnsi(const WCHAR* pchSrc, char* pchDest, int nDestLen)
{
	if (pchDest == NULL || pchSrc == NULL)
	{
		return;
	}

	const WCHAR* pWStrSRc = pchSrc;
	int nTmplen = WideCharToMultiByte(CP_ACP, 0, pWStrSRc, -1, NULL, 0, NULL, NULL);
	char* pTemp = new char[nTmplen + 1];
	memset(pTemp, 0, nTmplen + 1);
	WideCharToMultiByte(CP_ACP, 0, pWStrSRc, -1, pTemp, nTmplen + 1, NULL, NULL);

	int nLen = strlen(pTemp);
	if (nLen + 1 > nDestLen)
	{
		strncpy(pchDest, pTemp, nDestLen - 1);
		pchDest[nDestLen - 1] = 0;
	}
	else
	{
		strcpy(pchDest, pTemp);
	}

	delete[] pTemp;
}


/*=============================================================================
函 数 名: AnsiToUnicode
功    能: 实现将char型buffer（ANSI编码）中的内容安全地拷贝到指定的WChar型（Unicode编码）的buffer中
参    数: char*  pchSrc [in]          源字符串
	  	  WCAHR* pchDest [out]        目标buf
		  int nDestLen [in]           目标buf长度(注意：以字节为单位，不是以字符个数为单位)
注    意: 无
返 回 值: 无
=============================================================================*/
void AnsiToUnicode(const char* pchSrc, WCHAR* pchDest, int nDestLen)
{
	if (pchSrc == NULL || pchDest == NULL)
	{
		return;
	}

	int nTmpLen = MultiByteToWideChar(CP_ACP, 0, pchSrc, -1, NULL, 0);
	WCHAR* pWTemp = new WCHAR[nTmpLen + 1];
	memset(pWTemp, 0, (nTmpLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, pchSrc, -1, pWTemp, nTmpLen + 1);

	//UINT nLen = wcslen(pWTemp);
	//if (nLen + 1 > (nDestLen / sizeof(WCHAR)))
	//{
	//	wcsncpy(pchDest, pWTemp, nDestLen / sizeof(WCHAR) - 1);
	//	pchDest[nDestLen / sizeof(WCHAR) - 1] = 0;
	//}
	//else
	//{
	//	wcscpy(pchDest, pWTemp);
	//}

	UINT nLen = wcslen(pWTemp);
	if (nLen + 1 > nDestLen)
	{
		wcsncpy(pchDest, pWTemp, nDestLen);
		pchDest[nDestLen] = 0;
	}
	else
	{
		wcscpy(pchDest, pWTemp);
	}

	delete[] pWTemp;
}

/*=============================================================================
函 数 名: Utf8ToUnicode
功    能: 实现将char型的buffer(utf8编码)中的内容安全地拷贝到指定的WCHAR型buffer(Unicode编码)中
参    数: char* pchSrc [in]           源字符串
		  WCHAR* pchDest [out]        目标buf
		  int nDestLen [in]           目标buf长度(注意：以字节为单位，不是以字符个数为单位)
注    意: 无
返 回 值: 无
=============================================================================*/
void Utf8ToUnicode(const char* pchSrc, WCHAR* pchDest, int nDestLen)
{
	if (pchSrc == NULL || pchDest == NULL)
	{
		return;
	}

	int nTmpLen = MultiByteToWideChar(CP_UTF8, 0, pchSrc, -1, NULL, 0);
	WCHAR* pWTemp = new WCHAR[nTmpLen + 1];
	memset(pWTemp, 0, (nTmpLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, pchSrc, -1, pWTemp, nTmpLen + 1);

	//UINT nLen = wcslen(pWTemp);
	//if (nLen + 1 > (nDestLen / sizeof(WCHAR)))
	//{
	//	wcsncpy(pchDest, pWTemp, nDestLen / sizeof(WCHAR) - 1);
	//	pchDest[nDestLen / sizeof(WCHAR) - 1] = 0;
	//}
	//else
	//{
	//	wcscpy(pchDest, pWTemp);
	//}

	UINT nLen = wcslen(pWTemp);
	if (nLen + 1 > nDestLen)
	{
		wcsncpy(pchDest, pWTemp, nDestLen);
		pchDest[nDestLen] = 0;
	}
	else
	{
		wcscpy(pchDest, pWTemp);
	}

	delete[] pWTemp;
}

/*=============================================================================
函 数 名: AnsiToUtf8
功    能: 实现将char型buffer（ANSI编码）中的内容安全地拷贝到指定的char型（utf8编码）的buffer中
参    数: char*  pchSrc [in]          源字符串
	      char*  pchDest [out]        目标buf
          int nDestLen [in]           目标buf长度(注意：以字节为单位，不是以字符个数为单位)
注    意: 无
返 回 值: 无
=============================================================================*/
void AnsiToUtf8(const char* pchSrc, char* pchDest, int nDestLen)
{
	if (pchSrc == NULL || pchDest == NULL)
	{
		return;
	}

	WCHAR* pUnicodeTmpBuf = new WCHAR[nDestLen];
	ZeroMemory(pUnicodeTmpBuf, sizeof(WCHAR)*nDestLen);

	// 先将Ansi转成Unicode//
	AnsiToUnicode(pchSrc, pUnicodeTmpBuf, nDestLen);

	//cout << "unicode.len:" << wcslen(pUnicodeTmpBuf) << endl;

	// 再将Unicode转成Utf8//
	UnicodeToUtf8(pUnicodeTmpBuf, pchDest, nDestLen);

	//cout << "Utf8.len:" << strlen(pchDest) << endl;

	delete[] pUnicodeTmpBuf;
}