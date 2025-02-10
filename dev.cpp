#include "stdafx.h"

#include <opencv2/imgproc/types_c.h>

using namespace std;

extern void StringReplaceAll(string& resource_str, string sub_str, string new_str);

#define ENCODE_KEY _T("001010000010101010001001000100100100010100100010100011000010100")

/* Data structure for MD5 (Message-Digest) computation */
typedef struct {
	ULONG i[2];                          /* number of _bits_ handled mod 2^64 */
	ULONG buf[4];                                           /* scratch buffer */
	unsigned char in[64];                                     /* input buffer */
	unsigned char digest[16];            /* actual digest after MD5Final call */
} MD5_CTX;


#define MD5DIGESTLEN 16

#define PROTO_LIST(list)    list


/*
* MTS: Each of these assumes MD5_CTX is locked against simultaneous use.
*/
typedef void(WINAPI* PMD5Init) PROTO_LIST((MD5_CTX *));
typedef void(WINAPI* PMD5Update) PROTO_LIST((MD5_CTX *, const unsigned char *, unsigned int));
typedef void(WINAPI* PMD5Final)PROTO_LIST((MD5_CTX *));


using namespace cv;
using namespace std;

string GetSysSerialNum()
{
	char     m_Volume[256];//卷标名   
	char     m_FileSysName[256];
	DWORD   m_SerialNum;//序列号   
	DWORD   m_FileNameLength;
	DWORD   m_FileSysFlag;
	::GetVolumeInformation(_T("c:\\"),
		m_Volume,
		256,
		&m_SerialNum,
		&m_FileNameLength,
		&m_FileSysFlag,
		m_FileSysName,
		256);
	char buf[128];
	sprintf(buf, "%04x", m_SerialNum);
	return string(buf);
}

const char* Hex2ASC(const BYTE *Hex, int Len)
{
	 static char ASC[4096 * 2];
	 int i;

		 for (i = 0; i < Len; i++)
		 {
			 ASC[i * 2] = "0123456789abcdef"[Hex[i] >> 4];
			 ASC[i * 2 + 1] = "0123456789abcdef"[Hex[i] & 0x0F];
		 }
	 ASC[i * 2] = '\0';
	 return ASC;
}

string GetCodeMD5(const string& src)
{
	MD5_CTX ctx;
	PMD5Init MD5Init;
	PMD5Update MD5Update;
	PMD5Final MD5Final;

	HINSTANCE hDLL;
	if ((hDLL = LoadLibrary(_T("advapi32.dll"))) > 0)
	{
	    MD5Init = (PMD5Init)GetProcAddress(hDLL, "MD5Init");
	    MD5Update = (PMD5Update)GetProcAddress(hDLL, "MD5Update");
	    MD5Final = (PMD5Final)GetProcAddress(hDLL, "MD5Final");
	
	    MD5Init(&ctx);
	    MD5Update(&ctx, (const unsigned char*)src.data(), src.length());
	    MD5Final(&ctx);
	}
	return  string(Hex2ASC(ctx.digest, 16));
}

string EncodeString(string strMD5PrivKey, string strmD5SerialKey)
{
	string strTmp = strMD5PrivKey + strmD5SerialKey;
	char szKey[] = ENCODE_KEY;
	char szTmp[128] = { 0 };
	char szDes[64] = { 0 };
	strncpy(szTmp, strTmp.data(), strTmp.length());
	for (int i = 0,j=0; i < 64; i++)
	{
		if (szKey[i] == '1')
			szDes[j++] = szTmp[i];
	}
	return string(szDes);
}

int CheckAuthor(string& strPrivakey,string& strSerial)
{
	string strPubkey;
	char szTmp[64];

	char szTemp[_MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szTemp, _MAX_PATH);
	if (!strrchr(szTemp, '\\')) {
		cout << _T("Unexpected aborted: can't find path.") << endl;
		return -1;
	}
	strrchr(szTemp, '\\')[1] = 0;
	string strDir = string(szTemp);

	string strTmp = strDir + _T("privakey");
	FILE* fp;
	int ret = fopen_s(&fp, strTmp.data(), _T("r"));
	if (ret) {
		cout << _T("Error: can't find authorization privacy key")<< endl;
		return -1;
	}
	long lSize = _filelength(_fileno(fp));
	ZeroMemory(szTmp, 64);
	fread(szTmp, sizeof(char), lSize, fp);
	strPrivakey = string(szTmp);
	StringReplaceAll(strPrivakey, string(_T("\n")), string(_T("")));
	fclose(fp);

	strSerial = GetSysSerialNum();

	strTmp = strDir + _T("pubkey");
	
	ret = fopen_s(&fp, strTmp.data(), _T("r"));
	if (ret) {
		cout << _T("Warning: can't find authorization public key...") << endl;
		return 2;
	}
	else {
		long lSize = _filelength(_fileno(fp));
		ZeroMemory(szTmp, 64);
		fread(szTmp, sizeof(char), lSize, fp);
		strPubkey = string(szTmp);
		StringReplaceAll(strPubkey, string(_T("\n")), string(_T("")));
		fclose(fp);

		string strRet = EncodeString(GetCodeMD5(strPrivakey), GetCodeMD5(strSerial));
		//cout << strRet << endl;
		//cout << strPubkey << endl;
		if (strRet == strPubkey) {
			return 0;
		}
	}
	return -1; 
}

void ConnectServer(string strPrivaKey, string strSerial)
{
	cout << _T("Connecting network, please wait...") << endl;

	//https://loongwins.com/cowork/auth?privakey=C6CBE9965C9943ea97520983166CB247&serialkey=1eb20d32
	string strCmd = string(_T("start /MIN /WAIT wget -O ./pubkey \"https://loongwins.com/cowork/auth?privakey=")) + strPrivaKey + string(_T("&serialkey=")) + strSerial+ string(_T("\""));
	//cout << strCmd << endl;
	system(strCmd.data());
}

void AutoUpdate(string strSerial)
{
	cout << _T("Auto update, please wait...") << endl;

	string strCmd = string(_T("start /MIN /WAIT wget -O ./cowork_tmp \"https://loongwins.com/cowork/update?serialkey=")) + strSerial;
	//cout << strCmd << endl;
	system(strCmd.data());

	char szTemp[_MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szTemp, _MAX_PATH);

	strrchr(szTemp, '\\')[1] = 0;
	string strDir = string(szTemp);

	string strTmp= strDir + _T("cowork_tmp");
	string strDes = strDir + _T("cowork_update");

	FILE* file = fopen(strTmp.data(), "rb");
	if (file)
	{
		int size = filelength(fileno(file));
		//cout<< size << endl;
		fclose(file);
		if (size > 1024) {
			CopyFile(strTmp.data(), strDes.data(), false);
			remove(strTmp.data());
			strCmd = string(_T("start /MIN /WAIT wget --directory-prefix=res100 -c -N -r -np --level=1 -nd --reject=html https://loongwins.com/cowork/res100/"));
			system(strCmd.data());
			strCmd = string(_T("start /MIN /WAIT wget --directory-prefix=res125 -c -N -r -np --level=1 -nd --reject=html https://loongwins.com/cowork/res125/"));
			system(strCmd.data());

			cout << _T("Update complete...") << endl;
		}
		else {
			cout << _T("The current is the latest version") << endl;
		}
	}
}

//void ShowProcessMsg(string& strMsg, uint& uType)
//{
//	static char szProc[4] = _T("-/\\|\/");
//
//	if (uType >= 4)
//		uType = 0;
//	if (uType < 0)
//		uType = 0;
//
//	cout << strMsg.data() << _T("...");
//	cout << szProc[uType++];
//}