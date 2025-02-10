#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgcodecs.hpp>

#define DBMIN_COMPAREVAL 0.000001

using namespace cv;
using namespace std;

HWND GetSpecWindow()
{
	POINT pt;
	GetCursorPos(&pt);
	return WindowFromPoint(pt);
}

HWND GetSpecChildWindow()
{
	POINT pt;
	GetCursorPos(&pt);

	HWND hwnd = WindowFromPoint(pt);
	return ChildWindowFromPoint(hwnd,pt);
}

string getuuid()
{
	char buffer[64] = { 0 };
	GUID guid;

	if (CoCreateGuid(&guid))
	{
		cout << _T("create guid error") << endl;
		return string(_T(""));
	}
	_snprintf(buffer, sizeof(buffer),
		_T("%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X"),
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	//printf("guid: %s\n", buffer);

	return buffer;
}

string getuuidex()
{
	char buffer[64] = { 0 };
	GUID guid;

	if (CoCreateGuid(&guid))
	{
		cout << _T("create guid error") << endl;
		return _T("");
	}
	_snprintf(buffer, sizeof(buffer),
		_T("%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X"),
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	//printf("guid: %s\n", buffer);

	return buffer;
}

// 直方图方法//
void getRGBvector(const Mat&src, vector<unsigned int>& count)// 得到64维向量//
{
	int nRows = src.rows, nCols = src.cols * src.channels();
	const uchar* p;
	for (int i = 0; i < nRows; ++i)
	{
		p = src.ptr<uchar>(i);
		for (int j = 0; j < nCols; j += 3)
		{
			int r = int(p[j]) / 64;
			int g = int(p[j + 1]) / 64;
			int b = int(p[j + 2]) / 64;
			count[r * 16 + g * 4 + b]++;
		}
	}
}

double getVectorLength(vector<unsigned int> &vec)
{
	double res = 0;
	for (int i = 0; i < vec.size(); i++)
		res += vec[i] * vec[i];
	return sqrt(res);
}

double getcos(vector<unsigned int> &count1, vector<unsigned int> &count2)
{
	double len1 = getVectorLength(count1);
	double len2 = getVectorLength(count2);
	assert(len1 != 0 && len2 != 0);
	long long sum = 0;
	for (int i = 0; i < count1.size(); i++)
		sum += count1[i] * count2[i];
	return (double)sum / len1 / len2 >0 ? (double)sum / len1 / len2 : 0;
}

double getsimilarity(const Mat&src1, const Mat&src2)
{
	vector<unsigned int> count1(64), count2(64);
	getRGBvector(src1, count1);
	getRGBvector(src2, count2);
	double res = getcos(count1, count2);
	return res;
}

//pHash算法//
string pHashValue(Mat &src)
{
	Mat img, dst;
	string rst(64, '\0');
	double dIdex[64];
	double mean = 0.0;
	int k = 0;
	if (src.channels() == 3)
	{
		cvtColor(src, src, CV_BGR2GRAY);
		img = Mat_<double>(src);
	}
	else
	{
		img = Mat_<double>(src);
	}

	///* 第一步，缩放尺寸 */
	resize(img, img, Size(8, 8));

	///* 第二步，离散余弦变换，DCT系数求取 */
	dct(img, dst);

	///* 第三步，求取DCT系数均值（左上角8*8区块的DCT系数）*/
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j)
		{
			dIdex[k] = dst.at<double>(i, j);
			mean += dst.at<double>(i, j) / 64;
			++k;
		}
	}

	///* 第四步，计算哈希值。*/
	for (int i = 0; i<64; ++i)
	{
		if (dIdex[i] >= mean)
		{
			rst[i] = '1';
		}
		else
		{
			rst[i] = '0';
		}
	}
	return rst;
}

//汉明距离计算//
int HanmingDistance(string &str1, string &str2)
{
	if ((str1.size() != 64) || (str2.size() != 64))
		return -1;
	int difference = 0;
	for (int i = 0; i<64; i++)
	{
		if (str1[i] != str2[i])
			difference++;
	}
	return difference;
}

//MSSIM//
double getMSSIM(Mat  inputimage1, Mat inputimage2)
{
	Mat i1 = inputimage1;
	Mat i2 = inputimage2;
	const double C1 = 6.5025, C2 = 58.5225;
	int d = CV_32F;
	Mat I1, I2;
	i1.convertTo(I1, d);
	i2.convertTo(I2, d);
	Mat I2_2 = I2.mul(I2);
	Mat I1_2 = I1.mul(I1);
	Mat I1_I2 = I1.mul(I2);
	Mat mu1, mu2;
	GaussianBlur(I1, mu1, Size(11, 11), 1.5);
	GaussianBlur(I2, mu2, Size(11, 11), 1.5);
	Mat mu1_2 = mu1.mul(mu1);
	Mat mu2_2 = mu2.mul(mu2);
	Mat mu1_mu2 = mu1.mul(mu2);
	Mat sigma1_2, sigma2_2, sigma12;
	GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
	sigma1_2 -= mu1_2;
	GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
	sigma2_2 -= mu2_2;
	GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
	sigma12 -= mu1_mu2;
	Mat t1, t2, t3;
	t1 = 2 * mu1_mu2 + C1;
	t2 = 2 * sigma12 + C2;
	t3 = t1.mul(t2);
	t1 = mu1_2 + mu2_2 + C1;
	t2 = sigma1_2 + sigma2_2 + C2;
	t1 = t1.mul(t2);
	Mat ssim_map;
	divide(t3, t1, ssim_map);
	Scalar mssim = mean(ssim_map);
	return mssim[0];
}


void CoverTarget(Mat &result, Point ptCov, int covVal, int cols, int rows)
{

	// 先将第一个最小值点附近两倍模板宽度和高度的都设置为最大值防止产生干扰//
	int startX = ptCov.x - cols;
	int startY = ptCov.y - rows;
	int endX = ptCov.x + cols;
	int endY = ptCov.y + rows;
	if (startX < 0)
	{
		startX = 0;
	}
	if (startY < 0)
	{
		startY = 0;
	}
	if (endX > result.cols - 1)
	{
		endX = result.cols - 1;
	}
	if (endY > result.rows - 1)
	{
		endY = result.rows - 1;
	}

	int y, x;
	for (y = startY; y < endY; y++)
	{
		for (x = startX; x < endX; x++)
		{
			result.at<float>(y, x) = covVal;		//覆盖//
		}
	}
}


BOOL GetFixMat(const Mat& src_img, Mat& dst_img, unsigned int dc_heigh, unsigned int dc_width)
{
	if (!src_img.data)
	{
		return FALSE;
	}
	unsigned int img_rows(src_img.rows);
	unsigned int img_cols(src_img.cols);
	unsigned int fix_heigh(std::min(img_rows, dc_heigh));
	unsigned int fix_width(std::min(img_cols, dc_width));

	float ratio_w = static_cast<float>(fix_width) / static_cast<float>(img_cols);
	float ratio_h = static_cast<float>(fix_heigh) / static_cast<float>(img_rows);
	float ratio = std::min(ratio_w, ratio_h);

	int show_width = static_cast<unsigned int>(ratio * img_cols);
	int show_height = static_cast<unsigned int>(ratio * img_rows);

	cv::resize(src_img, dst_img, cv::Size(show_width, show_height), (0.8), (0.8), cv::INTER_LINEAR);

	return TRUE;
}

HBITMAP CaptureScreen(LPRECT lpRect)//lpRect为NULL时表示全屏幕截图//
{
	HDC      hdc, hdcMem;
	HBITMAP  hemfCopy;
	HWND     hwndScr;
	int      dstcx, dstcy;
	if (lpRect)
	{
		dstcx = lpRect->right - lpRect->left;
		dstcy = lpRect->bottom - lpRect->top;
	}
	else
	{
		dstcx = GetSystemMetrics(SM_CXSCREEN);
		dstcy = GetSystemMetrics(SM_CYSCREEN);
	}
	if (LockWindowUpdate(hwndScr = GetDesktopWindow()))
	{
		hdc = GetDCEx(hwndScr, NULL, DCX_CACHE | DCX_LOCKWINDOWUPDATE);
		hdcMem = CreateCompatibleDC(hdc);
		if (NULL == hdcMem)
		{
			ReleaseDC(hwndScr, hdc);
			LockWindowUpdate(NULL);
			return NULL;
		}
		hemfCopy = CreateCompatibleBitmap(hdc, abs(dstcx), abs(dstcy));
		if (NULL == hemfCopy)
		{
			DeleteDC(hdcMem);
			ReleaseDC(hwndScr, hdc);
			LockWindowUpdate(NULL);
			return NULL;
		}
		SelectObject(hdcMem, hemfCopy);
		if (lpRect)
			StretchBlt(hdcMem, 0, 0, abs(dstcx), abs(dstcy),
				hdc, lpRect->left, lpRect->top, dstcx, dstcy, SRCCOPY | CAPTUREBLT);
		else
			BitBlt(hdcMem, 0, 0, dstcx, dstcy,
				hdc, 0, 0, SRCCOPY | CAPTUREBLT);

		DeleteDC(hdcMem);
		ReleaseDC(hwndScr, hdc);
		LockWindowUpdate(NULL);
		return hemfCopy;
	}
	return NULL;
}

double GetScreenScale()
{
	// 获取窗口当前显示的监视器//
	// 使用桌面的句柄.//
	HWND hWnd = GetDesktopWindow();
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	// 获取监视器逻辑宽度与高度//
	MONITORINFOEX miex;
	miex.cbSize = sizeof(miex);
	GetMonitorInfo(hMonitor, &miex);
	int cxLogical = (miex.rcMonitor.right - miex.rcMonitor.left);
	int cyLogical = (miex.rcMonitor.bottom - miex.rcMonitor.top);

	// 获取监视器物理宽度与高度//
	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	dm.dmDriverExtra = 0;
	EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm);
	int cxPhysical = dm.dmPelsWidth;
	int cyPhysical = dm.dmPelsHeight;
	// 缩放比例计算  实际上使用任何一个即可//
	double horzScale = ((double)cxPhysical / (double)cxLogical);
	double vertScale = ((double)cyPhysical / (double)cyLogical);
	//printf("%lf  %lf\n", horzScale, vertScale);
	assert(horzScale == vertScale); // 宽或高这个缩放值应该是相等的//

	return horzScale;
}

int ClientToScreenEx(HWND hwnd, POINT* ptPoint)
{
	double dbScale = GetScreenScale();
	RECT rc;

	if (!GetWindowRect(hwnd, &rc))
		return -1;

	ptPoint->x = rc.left + ptPoint->x / dbScale;
	ptPoint->y = rc.top + ptPoint->y / dbScale;

	return 0;
}
//
//CompareImage：比对原始图像和模板图像，返回比对成功数。根据需要函数返回最优结果的数组位置; 0：无匹配, >0匹配成功
//参数1：需要比对的原始图像
//参数2：需要比对的模板图像
//参数3：找到符合要求的比对结果位置数组，可返回拖个比对成功结果
//参数4：指定位置数组数量
//参数5：指定比对精度
//参数6：比对方式  0：返回全部数组位置，不排序  1：最上，2：最左 3:最下 4:最右
//
int CompareImage(Mat src, Mat templ, CvRect* ptrRc, uint uSize, double dbAccur, uint uFlag = 1)
{
	double minVal, maxVal, dbRet[32] = { 0 };
	Point minLoc;
	Point maxLoc;
	Mat ret1, ret2, ret3;
	//CvRect szRc[32] = { 0 };

	int i, iCount, ret, x, y;

	//
	//模板匹配
	//参数1:src用于搜索的输入图像, 8U 或 32F, 大小 W-H
	//参数2:用于匹配的模板，和src类型相同， 大小 w-h
	//参数3:匹配结果图像, 类型 32F, 大小 (W-w+1)-(H-h+1)
	//参数4:用于比较的方法(有六种)
	//cv::TM_SQDIFF=0 该方法使用平方差进行匹配，因此最佳的匹配结果在结果为0处，值越大匹配结果越差
	//cv::TM_SQDIFF_NORMED=1：该方法使用归一化的平方差进行匹配，最佳匹配也在结果为0处
	//cv::TM_CCORR=2：相关性匹配方法，该方法使用源图像与模板图像的卷积结果进行匹配，因此，最佳
	//匹配位置在值最大处，值越小匹配结果越差  【个人测试：匹配性很差】
	//cv::TM_CCORR_NORMED=3：归一化的相关性匹配方法，与相关性匹配方法类似，最佳匹配位置也是在值最大处
	//cv::TM_CCOEFF=4：相关性系数匹配方法，该方法使用源图像与其均值的差、模板与其均值的差二者之间的相
	//关性进行匹配，最佳匹配结果在值等于1处，最差匹配结果在值等于-1处，值等于0直接表示二者不相关
	//cv::TM_CCOEFF_NORMED=5：归一化的相关性系数匹配方法，正值表示匹配的结果较好，负值则表示匹配的效
	//果较差，也是值越大，匹配效果也好
	//imshow("CompareImage:p1", src), imshow("CompareImage:p2", templ);
	//waitKey(0);
	//

	matchTemplate(src, templ, ret1, TM_CCOEFF_NORMED);
	cout << _T("CompareImage::matchTemplate==========================") << endl;

	//可以不归一化
	normalize(ret1, ret2, 1, 0, NORM_MINMAX);
	//cout << _T("normalize:") << endl;

	//找到最佳匹配点
	//从匹配结果图像中找出最佳匹配点,存入数组，支持多个匹配
	//
	for (i = 0; i < uSize; i++) {
		minMaxLoc(ret2, &minVal, &maxVal, &minLoc, &maxLoc);
		ret3 = src(cv::Rect(maxLoc.x, maxLoc.y, templ.cols, templ.rows));
		//dbRet[i] = HanmingDistance(pHashValue(ret3), pHashValue(templ));
		//cout << _T("CompareImage::HanmingDistance:") << dbRet[i] << endl;
		//dbSim = getsimilarity(ret3, templ);
		//cout << _T("CompareImage::getsimilarity:") << dbSim << endl;
		dbRet[i] = getMSSIM(ret3, templ);
		cout << _T("CompareImage::getMSSIM:") << dbRet[i] << endl;
		if (dbRet[i] - dbAccur < DBMIN_COMPAREVAL) {
			break;
		}
		if (minVal == maxVal) {
			//cout << _T("CompareImage::minVal == maxVal:") << endl;
			i++;
			break;
		}
		ptrRc[i] = cvRect(maxLoc.x, maxLoc.y, templ.cols, templ.rows);
		//cout << _T("CompareImage::ptrRc:") << ptrRc[i].x << "," << ptrRc[i].y << "," << ptrRc[i].width <<","<< ptrRc[i].height << endl;
		CoverTarget(ret2, maxLoc, minVal, templ.cols, templ.rows);
	}

	iCount = i;
	cout << _T("CompareImage::NumOfMatches:") << iCount << endl;

	ret = 0;
	if (2 == uFlag) {//按左-右排序，返回第一个//
		for (i = 0; i < iCount; i++) {
			if (0 == i) {
				x = ptrRc[i].x;
				ret = i;
			}
			else if (x >ptrRc[i].x) {
				x = ptrRc[i].x;
				ret = i;
			}
		}
		ptrRc[0] = ptrRc[ret];
	}
	else if (1 == uFlag) {//按上-下排序，返回第一个//
		for (i = 0; i < iCount; i++) {
			if (0 == i) {
				y = ptrRc[i].y;
				ret = i;
			}
			else if (y >ptrRc[i].y) {
				y = ptrRc[i].y;
				ret = i;
			}
		}
		ptrRc[0] = ptrRc[ret];
	}
	else if (3 == uFlag) {//按下-上排序，返回第一个//
		for (i = 0; i < iCount; i++) {
			if (0 == i) {
				y = ptrRc[i].y;
				ret = i;
			}
			else if (y <ptrRc[i].y) {
				y = ptrRc[i].y;
				ret = i;
			}
		}
		ptrRc[0] = ptrRc[ret];
	}
	else if (4 == uFlag) {//按右-左排序，返回第一个//
		for (i = 0; i < iCount; i++) {
			if (0 == i) {
				x = ptrRc[i].x;
				ret = i;
			}
			else if (x <ptrRc[i].x) {
				x = ptrRc[i].x;
				ret = i;
			}
		}
		ptrRc[0] = ptrRc[ret];
	}
	else if (0 == uFlag) {//返回最佳匹配//

	}

	return iCount;
}
//
//MatchTemplOnWindow：比对原始图像和模板图像，返回比对成功数。根据需要函数返回最优结果的数组位置; 0：无匹配, >0匹配成功
//参数1：需要比对的窗口句柄
//参数2：需要比对的模板图像
//参数3：指定窗口区域; NULL: 默认为窗口全部区域
//参数4：找到符合要求的比对结果位置数组，可返回多个比对成功结果
//参数5：指定位置数组数量
//参数6：指定比对精度
//参数7：比对方式 0：返回全部数组位置，1：按自上而下，返回最上位置，2：按自左而右，返回最左位置
//
int MatchTemplOnWindow(HWND hwnd, Mat imgTarg, RECT* ptrRcTarg, CvRect* ptrRc, uint uSize, double dbAccur, uint uFlag = 1)
{
	HDC hdc,hdcMem; 
	HBITMAP hbitmap;
	RECT rc;
	CvRect cvRc;
	POINT pt;
	double dbscale;
	int i,j,ret, iWidth, iHeight;
	void* ptrBitmap = NULL;
	void* ptrBitmapCp = NULL;

	char szTempPng[_MAX_PATH] = { 0 };

	GetModuleFileName(NULL, szTempPng, _MAX_PATH);
	strrchr(szTempPng, '\\')[1] = 0;
	string strTmp = string(szTempPng) + getuuidex() + string(_T(".png"));
	//cout << "GetModuleFileName:" << strTmp << endl;
	//return 0;
	
	hdc = GetDC(hwnd);
	GetClientRect(hwnd, &rc);
	dbscale = GetScreenScale();
	iWidth = rc.right * dbscale;
	iHeight = rc.bottom * dbscale; 

	cvRc.x = 0; cvRc.y = 0; cvRc.width = iWidth; cvRc.height = iHeight;
	if (ptrRcTarg) {
		cvRc.x = ptrRcTarg->left; cvRc.y = ptrRcTarg->top; 
		cvRc.width = ptrRcTarg->right - ptrRcTarg->left; cvRc.height = ptrRcTarg->bottom - ptrRcTarg->top;
		cvRc.x *= dbscale; cvRc.y *= dbscale; cvRc.width *= dbscale; cvRc.height *= dbscale;
	}

	if (cvRc.width < imgTarg.cols || cvRc.height < imgTarg.rows)
		return 0;

	//cout << "MatchTemplOnWindow::cvRc:" << cvRc.x << "," << cvRc.y << "," << cvRc.width<< "," << cvRc.height << endl;

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = cvRc.width;
	bi.bmiHeader.biHeight = -cvRc.height;  //negative so (0,0) is at top left
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;

	ptrBitmapCp = new char[cvRc.width * cvRc.height * 4];

	hdcMem = CreateCompatibleDC(hdc);
	hbitmap = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &ptrBitmap, NULL, 0);
	SelectObject(hdcMem, hbitmap);

	Mat matBitmap = Mat(cvRc.height, cvRc.width, CV_8UC4, ptrBitmap, 0);
	//BitBlt(hdcMem, 0, 0, cvRc.width, cvRc.height, hdc, cvRc.x, cvRc.y, SRCCOPY);
	StretchBlt(hdcMem, 0, 0, cvRc.width, cvRc.height, hdc, cvRc.x, cvRc.y, cvRc.width, cvRc.height, SRCCOPY);
	memcpy(ptrBitmapCp, ptrBitmap, cvRc.width * cvRc.height * 4);

	imwrite(strTmp, matBitmap);
	Mat src = Mat(imread(strTmp));

	//imshow("1",src);
	//waitKey(0);

	remove(strTmp.data()); //删除临时文件
	//cout << "src:" << src.depth() << "," << src.type() << "," << src.dims << endl;
	//cout << "imgTarg:" << imgTarg.depth() << "," << imgTarg.type() << "," << imgTarg.dims << endl;

	ret = CompareImage(src, imgTarg, ptrRc, uSize, dbAccur, uFlag);
	if (ret && uFlag >0 ) {
		rectangle(matBitmap, ptrRc[0], Scalar(0, 255, 0), 2);
		StretchBlt(hdc, cvRc.x, cvRc.y, cvRc.width, cvRc.height, hdcMem, 0, 0, cvRc.width, cvRc.height, SRCCOPY);

		Sleep(300);
		memcpy(ptrBitmap, ptrBitmapCp, cvRc.width * cvRc.height * 4);
		StretchBlt(hdc, cvRc.x, cvRc.y, cvRc.width, cvRc.height, hdcMem, 0, 0, cvRc.width, cvRc.height, SRCCOPY);

		ptrRc[0].x = (cvRc.x + ptrRc[0].x) / dbscale;
		ptrRc[0].y = (cvRc.y + ptrRc[0].y) / dbscale;
		ptrRc[0].width /= dbscale;
		ptrRc[0].height /= dbscale;
	}
	else if (ret && 0 == uFlag) {
		for (i = 0; i < ret; i++) {
			rectangle(matBitmap, ptrRc[i], Scalar(255, 0, 0), 2);

			ptrRc[i].x = (cvRc.x + ptrRc[i].x) / dbscale;
			ptrRc[i].y = (cvRc.y + ptrRc[i].y) / dbscale;
			ptrRc[i].width /= dbscale;
			ptrRc[i].height /= dbscale;
		}
		StretchBlt(hdc, cvRc.x, cvRc.y, cvRc.width, cvRc.height, hdcMem, 0, 0, cvRc.width, cvRc.height, SRCCOPY);
		Sleep(300);
		memcpy(ptrBitmap, ptrBitmapCp, cvRc.width * cvRc.height * 4);
		StretchBlt(hdc, cvRc.x, cvRc.y, cvRc.width, cvRc.height, hdcMem, 0, 0, cvRc.width, cvRc.height, SRCCOPY);
	}

	delete[] ptrBitmapCp;
	DeleteObject(hbitmap);
	DeleteDC(hdcMem);
	ReleaseDC(hwnd, hdc);
	
	return ret;
}

Mat GetClientWindowEx(HWND hwnd, RECT* ptrRcTarg)
{
	HDC hdc, hdcMem;
	HBITMAP hbitmap;
	RECT rc;
	POINT pt;
	CvRect cvRc;
	double dbscale;
	int i, j, ret, iWidth, iHeight;
	void* ptrBitmap = NULL;

	char szTempPng[_MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szTempPng, _MAX_PATH);
	strrchr(szTempPng, '\\')[1] = 0;
	string strTmp = string(szTempPng) + getuuidex() + string(_T(".png"));

	hdc = GetDC(hwnd);
	GetClientRect(hwnd, &rc);
	dbscale = GetScreenScale();
	iWidth = rc.right * dbscale;
	iHeight = rc.bottom * dbscale;

	cvRc.x = 0; cvRc.y = 0; cvRc.width = iWidth; cvRc.height = iHeight;
	if (ptrRcTarg) {
		cvRc.x = ptrRcTarg->left; cvRc.y = ptrRcTarg->top;
		cvRc.width = ptrRcTarg->right - ptrRcTarg->left; cvRc.height = ptrRcTarg->bottom - ptrRcTarg->top;
		cvRc.x *= dbscale; cvRc.y *= dbscale; cvRc.width *= dbscale; cvRc.height *= dbscale;
	}

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = iWidth;
	bi.bmiHeader.biHeight = -iHeight;  //negative so (0,0) is at top left
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;


	hdcMem = CreateCompatibleDC(hdc);
	hbitmap = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &ptrBitmap, NULL, 0);
	SelectObject(hdcMem, hbitmap);

	Mat matBitmap = Mat(iHeight, iWidth, CV_8UC4, ptrBitmap, 0);
	BitBlt(hdcMem, 0, 0, iWidth, iHeight, hdc, 0, 0, SRCCOPY);

	imwrite(strTmp, matBitmap);
	Mat matDes = Mat(imread(strTmp), cvRc);

	remove(strTmp.data()); //删除临时文件//
	DeleteObject(hbitmap);
	DeleteDC(hdcMem);
	ReleaseDC(hwnd, hdc);

	return matDes;
}


void MarkClientRect(HWND hwnd,  CvRect TargRc, Scalar dbColor)
{
	HDC hdc, hdcMem;
	HBITMAP hbitmap;
	RECT rc;
	CvRect cvRc;
	POINT pt;
	double dbscale;
	int i, j, ret, iWidth, iHeight;
	void* ptrBitmap = NULL;
	void* ptrBitmapCp = NULL;


	hdc = GetDC(hwnd);
	GetClientRect(hwnd, &rc);
	dbscale = GetScreenScale();
	iWidth = rc.right * dbscale;
	iHeight = rc.bottom * dbscale;

	TargRc.x *= dbscale; TargRc.y *= dbscale; TargRc.width *= dbscale; TargRc.height *= dbscale;

	cvRc.x = 0; cvRc.y = 0; cvRc.width = iWidth; cvRc.height = iHeight;

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = cvRc.width;
	bi.bmiHeader.biHeight = -cvRc.height;  //negative so (0,0) is at top left
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;

	ptrBitmapCp = new char[cvRc.width * cvRc.height * 4];

	hdcMem = CreateCompatibleDC(hdc);
	hbitmap = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &ptrBitmap, NULL, 0);
	SelectObject(hdcMem, hbitmap);

	//BitBlt(hdcMem, 0, 0, cvRc.width, cvRc.height, hdc, cvRc.x, cvRc.y, SRCCOPY);
	StretchBlt(hdcMem, 0, 0, cvRc.width, cvRc.height, hdc, cvRc.x, cvRc.y, cvRc.width, cvRc.height, SRCCOPY);
	memcpy(ptrBitmapCp, ptrBitmap, cvRc.width * cvRc.height * 4);

	Mat matBitmap = Mat(cvRc.height, cvRc.width, CV_8UC4, ptrBitmap, 0);

	rectangle(matBitmap, TargRc, dbColor, 2);
	StretchBlt(hdc, cvRc.x, cvRc.y, cvRc.width, cvRc.height, hdcMem, 0, 0, cvRc.width, cvRc.height, SRCCOPY);

	Sleep(300);
	memcpy(ptrBitmap, ptrBitmapCp, cvRc.width * cvRc.height * 4);
	StretchBlt(hdc, cvRc.x, cvRc.y, cvRc.width, cvRc.height, hdcMem, 0, 0, cvRc.width, cvRc.height, SRCCOPY);

	delete[] ptrBitmapCp;
	DeleteObject(hbitmap);
	DeleteDC(hdcMem);
	ReleaseDC(hwnd, hdc);
}