#include "stdafx.h"

using namespace std;

extern int BookJd_xh(int iParam0, int iParam1);
extern int CtripAddNew();
extern int CtripClear();
extern int CtripBookA(char* pParam);
extern int WechatFindGroupName();
extern int WechatBoardCast();
extern int WechatGetOneByLabel();
extern int WechatFriendAuth(int iParam);
extern int SandBoxTest();
extern int WechatSetLabel(int iParam, char* pParam);
extern int WechaSetTopest(int iParam);

extern int DemoShow();

extern string getuuidex();
extern int CheckAuthor(string& strPrivakey, string& strSerial);
extern void ConnectServer(string strPrivaKey, string strSerial);
extern void AutoUpdate(string strSerial); 
extern void CheckAuthor();

extern void WxEntrance(int argc, _TCHAR* argv[]);
extern int PasteEx();




void CheckAuthor()
{
	string strPriva, strSerial;
	int ret = CheckAuthor(strPriva, strSerial);
	if (ret == 2) {
		ConnectServer(strPriva, strSerial);
		ret = CheckAuthor(strPriva, strSerial);
		if (!ret) {
			cout << _T("Success to get authorization public key.") << endl;
			string strCmd = _T("cscript.exe //nologo readme.vbs > temp");
			system(strCmd.data());

			char szTemp[_MAX_PATH] = { 0 };
			GetModuleFileName(NULL, szTemp, _MAX_PATH);
			if (!strrchr(szTemp, '\\')) {
				cout << _T("Unexpected aborted: can't find path.") << endl;
				exit(1);
			}
			strrchr(szTemp, '\\')[1] = 0;
			string strDir = string(szTemp);

			string strTmp = strDir + _T("temp");
			FILE* fp;
			ret = fopen_s(&fp, strTmp.data(), _T("r"));
			long iSize = _filelength(_fileno(fp));
			char szAscii[4];
			ZeroMemory(szAscii, sizeof(szAscii));
			fread(szAscii, sizeof(char), sizeof(szAscii), fp);
			fclose(fp);

			remove(strTmp.data());

			if (atol(szAscii) != 6) {
				strTmp = strDir + _T("pubkey");
				remove(strTmp.data());
				cout << _T("As you disagree cowork protocol, it isn't work. Please try again.") << endl;
				exit(0);
			}
		}
	}

	if (ret) {
		cout << _T("Didn't have authorized. Please contact the author.") << endl;
		exit(0);
	}

	if (ret == 0) {
		//AutoUpdate(strSerial);
	}

}

void WxTitle()
{
	printf_s(_T("***声明: Cowork不收集,不传输,不转发用户的任何电子信息记录.\n"
		"   除初次使用时需要联网授权外,不会主动建立网络链接.\n"
		"***程序对应微信电脑版本3.7.5\n\n"
		"cowork.exe v0.4.375  made by LOONGWINS \n"
	));
}

void WxUsage()
{
	WxTitle();

	printf_s(_T("使用方法: \n"
		"  cowork.exe [选项] \n"
		"  选项:\n"
		"    -wxtxl       //命令解释:本地获取“微信 - 通讯录管理”下全部联系人的详细信息,\n"
		"                   并且导入到当前路径下的(wechat_contact.csv)文件中\n"
		"    -wxqmc       //命令解释:本地获取微信群名称，保存到当前路径下的(wechat_contact.csv)文件中\n"
		"    -wxgg        //命令解释:按指定名单(wechat_list.csv)发送信息\n"
		"    -wxqx [参数] //命令解释:按指定名单(wechat_list.csv)修改联系人朋友权限\n"
		"                   参数说明:1 仅聊天; 2 聊天朋友圈...; \n"
		"                             6 聊天朋友圈...不让他(她)看; 10 聊天朋友圈...不看他(她)\n"
		"                             12 表示6和10都选 \n"
		"                   举例：cowork -wxqx 12  \n\n"
		"    -wxbq [参数] //命令解释：按指定名单(wechat_list.csv)来设置用户标签\n"
		"                   参数说明：1 \"xxx\" ;重置修改，xxx为标签名\n"
		"                             2 \"xxx\" ;追加修改，xxx为标签名\n"
		"                   举例：cowork -wxbq 1 \"商业伙伴\"  \n\n"
		"    -wxset [参数] //命令解释：按指定名单(wechat_list.csv)设置用户\n"
		"                    参数说明：0 取消置顶 + 关闭消息免打扰; 1 置顶 + 关闭消息免打扰; \n"
		"                              2 取消置顶 + 消息免打扰; 3 置顶 + 消息免打扰; \n"
		"                   举例：cowork -wxset 0 \n\n"
		"\n"
		"***注意:程序运行时,可以按F10键退出程序\n"
	));
}

void CommUsage()
{
	printf_s(_T("CoworkTools.exe v0.1  made by LOONGWINS \n"
		"Usage: \n"
		"  coworktools.exe [option] \n"
		"  option: \n"
		"     -param \n"
		"\n"
		"***注意：程序运行时，可以按F10键退出程序\n"
		
	));

}

void WxEntrance(int argc, _TCHAR* argv[])
{
	if (argc>1)
	{
		if (!strcmp((const char*)argv[1], _T("-privakey")))
		{
			for (int i = 0; i < 20; i++)
				cout << getuuidex().data() << endl;
		}
		else if (!strcmp((const char*)argv[1], _T("-wxqmc")))
		{
			WechatFindGroupName();
		}
		else if (!strcmp((const char*)argv[1], _T("-wxtxl")))
		{
			WechatGetOneByLabel();
		}
		else if (!strcmp((const char*)argv[1], _T("-wxgg")))
		{
			WechatBoardCast();
		}
		else if (!strcmp((const char*)argv[1], _T("-wxqx")))
		{
			if (argc == 3) {
				//cout << argv[2] << 5 << endl;
				WechatFriendAuth(atoi(argv[2]));
			}
		}
		else if (!strcmp((const char*)argv[1], _T("-wxbq")))
		{
			if (argc == 4) {
				//cout << argv[2] << 5 << endl;
				WechatSetLabel(atoi(argv[2]), argv[3]);
			}
		}
		else if (!strcmp((const char*)argv[1], _T("-wxset")))
		{
			if (argc == 3) {
				//cout << argv[2] << 5 << endl;
				WechaSetTopest(atoi(argv[2]));
			}
		}
		else
			WxUsage();
	}
	else
		WxUsage();

}

void ToolEntrance(int argc, _TCHAR* argv[])
{
	if (!strcmp((const char*)argv[1], _T("-privakey")))
	{
		for (int i = 0; i < 20; i++)
			cout << getuuidex().data() << endl;
	}
	else if (!strcmp((const char*)argv[1], _T("-demoshow")))
	{
		DemoShow();
	}
	else if (!strcmp((const char*)argv[1], _T("-test")))
	{
		SandBoxTest();
	}
	else if (!strcmp((const char*)argv[1], _T("-ctrip")))
	{
		if (argc == 3) {
			CtripBookA(argv[2]);
		}
	}
	else if (!strcmp((const char*)argv[1], _T("-ctripclear")))
	{
		CtripClear();
	}
	else if (!strcmp((const char*)argv[1], _T("-ctripadd")))
	{
		CtripAddNew();
	}
	else if (!strcmp((const char*)argv[1], _T("-bookjd")))
	{
		if (argc == 4) {
			//cout << argv[2] << 5 << endl;
			BookJd_xh(atoi(argv[2]), atoi(argv[2]));
		}
	}
	else if (!strcmp((const char*)argv[1], _T("-ctlv")))
	{
		PasteEx();
	}
	else if (!strcmp((const char*)argv[1], _T("/?")))
	{
		CommUsage();
	}
	else
		CommUsage();
}

void PasteUsage()
{
	printf_s(_T("\n"
		"***连续按ctrl-v可以粘贴pastecontent.csv里的内容\n"

	));

}

int _tmain(int argc, _TCHAR* argv[])
{
	

	//检查版本
	//CheckAuthor();


	//微信入口
	//WxEntrance(argc, argv);

	//工具入口
	CommUsage();
	PasteUsage();
	ToolEntrance(argc, argv);

	return 0;
}
