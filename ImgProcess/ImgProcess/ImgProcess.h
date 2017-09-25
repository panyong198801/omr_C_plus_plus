<<<<<<< HEAD
// ImgProcess.h : ImgProcess DLL 的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

#include "ImageProcess.h"
#include "CurImage.h"
struct OMR_ZUID
{
    int nPage;
    int nID;
    int nABFlag;
    OMR_ZUID()
    {
        nABFlag = -1;
    }
};




// CImgProcessApp
// 有关此类实现的信息，请参阅 ImgProcess.cpp
//
class CImgProcessApp : public CWinApp
{
public:
    CImgProcessApp();
    // 重写
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    DECLARE_MESSAGE_MAP()
};

//获取填涂密度
bool NP_GetDensity(IplImage* pSrcIn, CvRect cvRectIn, int iThres);
int NP_GetZhanKong(IplImage* pSrcIn, CvRect cvRectIn, int iThres, float& flzkOut);
int GetOption1(IplImage* pSrcIn, vector<CvRect> cvRectInVec,
    int Color, vector<int> uiPixVec,
    int uiType, std::wstring &csOptOutVec,
    int uiThread);

/*
函数说明: 获取目标图像矩形阵列
返回结果:  TRUE 成功  FALSE：失败
参数说明:
rectArea:   某一阵列OMR所在区域
modelArray: 模板当中OMR矩形阵列信息
modelsize:  模板中矩形个数
dstArry:    需获取目标矩形阵列
*/
BOOL ZYJ_GetRectArray(int nOmrType, vector<RECTARRAY> &dstArray, int nRecType, int nZuID, CxImage *srcImg, CurImage *imgPara); //获取客观题矩形阵                                                                                                                     
BOOL ZYJ_IsUnFilling(RECTARRAY &srcRect, int nThreshold, CxImage *srcImg); //函数说明: 判断某一矩形阵列是否填涂,主要针对未填涂
/*
函数说明:获取矩形定位点
参数说明:
rectArea:         图像上矩形框选区域
nModelRectWidth:  模板中定位点矩形宽度
nModelRectHeight: 模板中定位点矩形高度
dstRect         : 待求目标定位点矩形
*/
bool ZYJ_GetRectLocalPoint(NEWRECT rectArea, int nModelRectWidth, int nModelRectHeight, int nIndex, int nRectType, NEWRECT &dstRect, CxImage *srcImg, CurImage *ImgPara); //
																																										  /*
																																										  函数说明:获取客观题识别结果
																																						  */
bool ZYJ_GetOmrResult(vector<RECTARRAY> &srcRects, int nSize, /*int nSelCheck, */vector<RECTARRAY> modelRectArry, int &nFillAverGray);

CRect &NewRect2Rect(NEWRECT newRect);
NEWRECT &Rect2NewRect(CRect rect);
bool GetRectArry(vector<RECTARRAY> &v_DstRectArray, vector<CRect> v_allRects, vector<RECTARRAY> v_modelRects);
bool GetRectArry_1(vector<RECTARRAY> &v_DstRectArray, vector<RECTARRAY> v_curRectArray, vector<RECTARRAY> v_modelRects, CxImage *srcImg, int nThreshold);

double GetGrayDensity(NEWRECT rectArea, int nThrshold, CxImage *srcImg, int &nAverValue);

bool MergeNewImage(CxImage img1, CxImage img2, CString strNewImg);
void sort(vector<RECTARRAY> &v_dstArrays);
bool GetModelRectArray(RECTARRAY rectArray, vector<RECTARRAY> modelRectArray, RECTARRAY &dstModelRArray);
NEWRECT GetRectArea(const RECTARRAY *v_modelrArray, CxImage *srcImg);
NEWRECT GetRectArea1(NEWRECT rect1, int nOmrSize, CxImage *srcImg);

DWORD GetFileType1(const CString strImgPath);
int GetBlackColor(CxImage cxImage);

bool GetCurImgInfo(CxImage *srcImg, const char * cFilePath, const int nPage, const int nOmrType, const int nThreshold, const int nRecType, int nABFlag, CurImage &curImgPara);

//切割矩形转换 2017.01.12
=======
// ImgProcess.h : ImgProcess DLL 的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

#include "ImageProcess.h"
#include "CurImage.h"
struct OMR_ZUID
{
    int nPage;
    int nID;
    int nABFlag;
    OMR_ZUID()
    {
        nABFlag = -1;
    }
};




// CImgProcessApp
// 有关此类实现的信息，请参阅 ImgProcess.cpp
//
class CImgProcessApp : public CWinApp
{
public:
    CImgProcessApp();
    // 重写
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    DECLARE_MESSAGE_MAP()
};

//获取填涂密度
bool NP_GetDensity(IplImage* pSrcIn, CvRect cvRectIn, int iThres);
int NP_GetZhanKong(IplImage* pSrcIn, CvRect cvRectIn, int iThres, float& flzkOut);
int GetOption1(IplImage* pSrcIn, vector<CvRect> cvRectInVec,
    int Color, vector<int> uiPixVec,
    int uiType, std::wstring &csOptOutVec,
    int uiThread);

/*
函数说明: 获取目标图像矩形阵列
返回结果:  TRUE 成功  FALSE：失败
参数说明:
rectArea:   某一阵列OMR所在区域
modelArray: 模板当中OMR矩形阵列信息
modelsize:  模板中矩形个数
dstArry:    需获取目标矩形阵列
*/
BOOL ZYJ_GetRectArray(int nOmrType, vector<RECTARRAY> &dstArray, int nRecType, int nZuID, CxImage *srcImg, CurImage *imgPara); //获取客观题矩形阵                                                                                                                     
BOOL ZYJ_IsUnFilling(RECTARRAY &srcRect, int nThreshold, CxImage *srcImg); //函数说明: 判断某一矩形阵列是否填涂,主要针对未填涂
/*
函数说明:获取矩形定位点
参数说明:
rectArea:         图像上矩形框选区域
nModelRectWidth:  模板中定位点矩形宽度
nModelRectHeight: 模板中定位点矩形高度
dstRect         : 待求目标定位点矩形
*/
bool ZYJ_GetRectLocalPoint(NEWRECT rectArea, int nModelRectWidth, int nModelRectHeight, int nIndex, int nRectType, NEWRECT &dstRect, CxImage *srcImg, CurImage *ImgPara); //
																																										  /*
																																										  函数说明:获取客观题识别结果
																																						  */
bool ZYJ_GetOmrResult(vector<RECTARRAY> &srcRects, int nSize, /*int nSelCheck, */vector<RECTARRAY> modelRectArry, int &nFillAverGray);

CRect &NewRect2Rect(NEWRECT newRect);
NEWRECT &Rect2NewRect(CRect rect);
bool GetRectArry(vector<RECTARRAY> &v_DstRectArray, vector<CRect> v_allRects, vector<RECTARRAY> v_modelRects);
bool GetRectArry_1(vector<RECTARRAY> &v_DstRectArray, vector<RECTARRAY> v_curRectArray, vector<RECTARRAY> v_modelRects, CxImage *srcImg, int nThreshold);

double GetGrayDensity(NEWRECT rectArea, int nThrshold, CxImage *srcImg, int &nAverValue);

bool MergeNewImage(CxImage img1, CxImage img2, CString strNewImg);
void sort(vector<RECTARRAY> &v_dstArrays);
bool GetModelRectArray(RECTARRAY rectArray, vector<RECTARRAY> modelRectArray, RECTARRAY &dstModelRArray);
NEWRECT GetRectArea(const RECTARRAY *v_modelrArray, CxImage *srcImg);
NEWRECT GetRectArea1(NEWRECT rect1, int nOmrSize, CxImage *srcImg);

DWORD GetFileType1(const CString strImgPath);
int GetBlackColor(CxImage cxImage);

bool GetCurImgInfo(CxImage *srcImg, const char * cFilePath, const int nPage, const int nOmrType, const int nThreshold, const int nRecType, int nABFlag, CurImage &curImgPara);

//切割矩形转换 2017.01.12
>>>>>>> c3ec4193a47e985f94758967b6bacfb8a4ab020b
void GetCropRect(CRect rectModel, double dTransfer[6], CRect &dstRect);