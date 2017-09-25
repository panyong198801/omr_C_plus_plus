// ImgProcess.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "ImgProcess.h"

#include "include/zyj-dllexport.h"
#include <shlwapi.h>
#include <map>
#include <math.h>
#include "ImgRecQR.h"

#include "Image/CxImage/ximadef.h"
#pragma comment(lib,"Shlwapi.lib")

BEGIN_MESSAGE_MAP(CImgProcessApp, CWinApp)
END_MESSAGE_MAP()

static int g_nLocalType/* = 2*/;
static int g_nModelPage /*= 1*/;
static int g_nModelWidth /*= 0*/;
static int g_nModelHeight /*= 0*/;
static CString g_strModleQRTitle/* = _T("")*/; //首页二维码标题 2016.01.04
static RECTARRAY g_RaAbsent; //缺考所在的矩形框 2016.01.04
static CString g_strModelVersion; //模板版本
static CString g_strModelPageCount; //题卡页数
static vector<RECTARRAY> g_v_modelRectArrays; //模板中客观题区域
static vector<NEWRECT>   g_v_rectModelLocal;   //模板中三个定位点所在矩形位置
static vector<OMR_ZUID> g_v_omrZuIDs; //客观题对应各组ID
static vector<RECTARRAY> g_v_modelKhRA; //模板中考号区域
static vector<RECTARRAY> g_v_modelScoreRA; //主观题分数
static vector<RECTARRAY> g_v_modelSubjectRA; //主观题切块
static vector<RECTARRAY> g_v_modelQRTitle;  //二维码标题
static vector<RECTARRAY> g_v_modelQRKh;  //条码考号
static vector<RECTARRAY> g_v_modelPage; //页号
//static vector<LOCALPOINT_DATA> g_v_LocalPointData;  //已存储图像定位点值
static vector<RECTARRAY> g_v_modelSyncHead; //同步头 2017.02.13
static CMutex g_mute;


// CImgProcessApp 构造

CImgProcessApp::CImgProcessApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CImgProcessApp 对象

CImgProcessApp theApp;



BOOL CImgProcessApp::InitInstance()
{
	CWinApp::InitInstance();	
	return TRUE;
}

 int CImgProcessApp::ExitInstance()
 {
	 CWinApp::ExitInstance();
	 g_strModleQRTitle = _T("");
	 g_v_modelRectArrays.clear(); //模板中客观题区域
	 g_v_rectModelLocal.clear();    //模板中三个定位点所在矩形位置
	 g_v_rectModelLocal.clear(); //客观题对应各组ID
	 g_v_modelKhRA.clear(); //模板中考号区域
	 g_v_modelScoreRA.clear(); //主观题分数
	 g_v_modelSubjectRA.clear(); //主观题切块
	 g_v_modelQRTitle.clear();  //二维码标题
	 g_v_modelQRKh.clear();  //条码考号
	 return 0;
 }

 DWORD GetFileType1(const CString strImgPath)
 {
	 DWORD dFileType = CXIMAGE_FORMAT_UNKNOWN;
	 CString strTmp = strImgPath;
	 strTmp = strTmp.MakeLower();

	 if (strTmp.Find(L".jpg") >= 0 || strTmp.Find(L".jpeg") >= 0)
		 dFileType = CXIMAGE_FORMAT_JPG;
	 else if (strTmp.Find(L".bmp") >= 0)
		 dFileType = CXIMAGE_FORMAT_BMP;
	 else if (strTmp.Find(L".png") >= 0)
		 dFileType = CXIMAGE_FORMAT_PNG;
	 else if (strTmp.Find(L".tif") >= 0 || strTmp.Find(L".tiff") >= 0)
		 dFileType = CXIMAGE_FORMAT_TIF;
	 else if (strTmp.Find(L".gif") >= 0)
		 dFileType = CXIMAGE_FORMAT_GIF;
	 else 
		 dFileType = CXIMAGE_FORMAT_UNKNOWN;

	 return dFileType;
 }

 bool GetModelRectArray(RECTARRAY rectArray, vector<RECTARRAY> modelRectArray, RECTARRAY &dstModelRArray)
 {
	int nSize = modelRectArray.size();
	for (int i=0; i<nSize; i++)
	{
		RECTARRAY rArray1 = modelRectArray[i];
		if (rectArray.nQuestionNo == rArray1.nQuestionNo)
		{
			if (rectArray.nAnswerIndex == rArray1.nAnswerIndex)
			{
				dstModelRArray = rArray1;
				return TRUE;
			}
		}
	}

	 return FALSE;
 }

 BOOL ZYJ_IsUnFilling(RECTARRAY &srcRect, int nThreshold, CxImage *srcImg)
 {
	 if (!srcImg)
		 return FALSE;

	 RECT rectArea;
	 NEWRECT rect = srcRect.rect;
	 rectArea.top = rect.pTopLeft.nY;
	 rectArea.bottom = rect.pBottomRight.nY;
	 rectArea.left = rect.pTopLeft.nX;
	 rectArea.right = rect.pBottomRight.nX;
	 CxImage cropImg; 
	 srcImg->Crop(rectArea, &cropImg);
	 cropImg.GrayScale();

	 int nImgWidth = cropImg.GetWidth();
	 int nImgHeight = cropImg.GetHeight();

	 if (nImgHeight*nImgWidth <= 0)
		 return FALSE;

	 int nWhilteWidth = 0;
	 bool bFindWhite[2] = {FALSE , FALSE};
	 for (int i=0; i<nImgWidth/2; i++)
	 {
		 int nCount = 0; 
		 for (int j=0; j<nImgHeight; j++)
		 {
			 if (GETGRAYVALUE2(cropImg, i, j) > nThreshold)
			 {
				 nCount++;
			 }
		 }

		 if (nCount > nImgHeight/2 &&  abs(nCount - nImgHeight) <= 8)
			 nWhilteWidth++;
		 else
			 nWhilteWidth = 0;

		 if (nWhilteWidth >= 5)
		 {
			 bFindWhite[0] = TRUE;
			 break;
		 }
	 }

	 nWhilteWidth = 0;
	 for (int i=nImgWidth/2 +1; i<nImgWidth; i++)
	 {
		 int nCount = 0;
		 for (int j=0; j<nImgHeight; j++)
		 {
			 if (GETGRAYVALUE2(cropImg, i, j) > nThreshold)
			 {
				 nCount++;
			 }
		 }

		 if (nCount > nImgHeight/2 &&  abs(nCount - nImgHeight) <= 8)
			 nWhilteWidth++;
		 else
			 nWhilteWidth = 0;

		 if (nWhilteWidth >= 5)
		 {
			 bFindWhite[1] = TRUE;
			 break;
		 }
	 }

	 for (int i=0; i<2; i++)
	 {
		 if (!bFindWhite[i])
		 {
			 return FALSE;
		 }
	 }

	 srcRect.nFilling = 0; // 未填涂
	 return TRUE;
 }

 //合并图像
 bool MergeNewImage(CxImage img1,  CxImage img2, CString strNewImg)
 {
	 img1.GrayScale();
	 img2.GrayScale();
	// DWORD dFileType = GetFileType1(strNewImg);

	 BYTE *ps = (BYTE *)img1.info.pImage;
	 int ByteCount1 = BYTESPERLINE(img1.GetWidth(), img1.GetBpp());
	 BYTE *pd = (BYTE *)img2.info.pImage;
	 int ByteCount2 = BYTESPERLINE(img2.GetWidth(), img2.GetBpp());
	 int nW = img2.GetWidth();
	 int nH = img2.GetHeight();
	 int nWW = img1.GetWidth();
	 int nHH = img1.GetHeight();
	 int newW = max(nW, nWW);
	 int newH = nH + nHH;

	 CxImage pXimg;
	 pXimg.Create(newW, newH, img1.GetBpp(), CXIMAGE_FORMAT_BMP);
	 pXimg.SetPalette(img1.GetPalette(), 256); //设置调色板   2016.01.28
	 RGBQUAD *rgb = pXimg.GetPalette();
	 int newBC = BYTESPERLINE(pXimg.GetWidth(), img1.GetBpp());
	 BYTE *pImage = (BYTE *)pXimg.info.pImage;
	 int nBpp =  pXimg.GetBpp();
	 memset(pImage, 255, newBC*newH);
	 int i;
	 for (i=0; i<nH; i++)
	 {
		 memcpy(pImage+i*newBC, pd+i*ByteCount2, ByteCount2);
	 }

	 for (i=0; i<nHH; i++)
	 {
		 memcpy(pImage+(nH +i)*newBC, ps+i*ByteCount1, ByteCount1);
	 }

	 bool bRes = TRUE; //pXimg.Save(strNewImg, dFileType);
	 img2.Transfer(pXimg);
	 return bRes;
 }

 void sort(vector<RECTARRAY> &v_dstArrays)
 {
	 int nSize = v_dstArrays.size();
	 for (int i=0; i<nSize; i++)
	 {
		 vector<RECTARRAY>::iterator it1 = v_dstArrays.begin()+i;
		 RECTARRAY rArray1 = *it1;
		for (int j=i+1; j<nSize; j++)
		{
			vector<RECTARRAY>::iterator it2  = v_dstArrays.begin()+j;
			RECTARRAY rArray2 = *it2;
			if (rArray1.nQuestionNo > rArray2.nQuestionNo)
			{
				*it1 = rArray2;
			    *it2 = rArray1;

				rArray1 = *it1;
				rArray2 = *it2;
			}
			else if (rArray1.nQuestionNo == rArray2.nQuestionNo)
			{
				if (rArray1.nAnswerIndex > rArray2.nAnswerIndex)
				{
					*it1 = rArray2;
					*it2 = rArray1;

					rArray1 = *it1;
					rArray2 = *it2;
				}
			}
		}
	 }
 }


bool GetRectArry(vector<RECTARRAY> &v_DstRectArray, vector<CRect> v_allRects, vector<RECTARRAY> v_modelRects) //获取排序后的对应矩形阵列
{
	if (v_modelRects.size() <= 0)
		return FALSE;

	CSize dstSize;
	dstSize.SetSize(v_modelRects[0].rect.GetWidth(), v_modelRects[0].rect.GetHeight());
	
	//将其按照每列排序
	vector<RECTARRAY> v_dstRectArrayTmps;
	int nMaxLen = (int)sqrt(pow(double(dstSize.cx)/2.0, 2) + pow(double(dstSize.cy)/2.0, 2));
	int nMinLen = (dstSize.cx + dstSize.cy)/2;
	int nMaxLen1 = max(nEPS, 5); 
	
	for (vector<CRect>::iterator it1 = v_allRects.begin(); it1 != v_allRects.end(); it1++)
	{
		CRect rectTmp1 = *it1;
		for (int j=0; j<v_modelRects.size(); j++)
		{
			RECTARRAY rectModelArray = v_modelRects[j];
			CPoint pCenter1 = rectTmp1.CenterPoint();
			CPoint pCenter2 = CPoint(rectModelArray.CenterNewPoint().nX, rectModelArray.CenterNewPoint().nY);
			
			if ((int)CImageProcess::GetLength(pCenter1, pCenter2) <= nMaxLen1)
			{
				RECTARRAY rectArrayTmp;
				CPoint p1 = rectTmp1.TopLeft();
				CPoint p2 = rectTmp1.BottomRight();
				NEWPOINT pTopLeft;
				NEWPOINT pBottomRight;
				pTopLeft.setPoint(p1.x, p1.y);
				pBottomRight.setPoint(p2.x, p2.y);

				rectArrayTmp.nType = rectModelArray.nType;
				rectArrayTmp.rect.SetRect(pTopLeft, pBottomRight);
				rectArrayTmp.nQuestionNo = rectModelArray.nQuestionNo;
				strcpy(rectArrayTmp.cTitle, rectModelArray.cTitle);
				rectArrayTmp.nZuID = rectModelArray.nZuID;
				rectArrayTmp.nAnswerIndex = rectModelArray.nAnswerIndex;
				rectArrayTmp.nSelType  = rectModelArray.nSelType;
				v_dstRectArrayTmps.push_back(rectArrayTmp);
				//TRACE("%d, %d, %d, %d\n",rectArrayTmp.nQuestionNo, rectArrayTmp.nAnswerIndex, rectArrayTmp.rect.pTopLeft.nX,  rectArrayTmp.rect.pTopLeft.nY);
				break;
			}
		}
	}

	if (v_dstRectArrayTmps.size() <= 0 )
	{
		for (vector<CRect>::iterator it1 = v_allRects.begin(); it1 != v_allRects.end(); it1++)
		{
			CRect rectTmp1 = *it1;
			int nCrossCount = 0;
			int k = 0;
			for (int j=0; j<v_modelRects.size(); j++)
			{
				RECTARRAY rectModelArray = v_modelRects[j];
				CRect rect2 = NewRect2Rect(rectModelArray.rect);
				//CPoint pCenter1 = rectTmp1.CenterPoint();
				//CPoint pCenter2 = CPoint(rectModelArray.CenterNewPoint().nX, rectModelArray.CenterNewPoint().nY);
				CImageProcess imgProcessTmp;
				int nCurCross = imgProcessTmp.GetCrossPointsCount(rectTmp1, rect2);

				if (nCurCross > nCrossCount)
				{
					nCrossCount = nCurCross;
					k = j;
				}				
			}

			if (nCrossCount != 0)
			{
				RECTARRAY rectModelArray = v_modelRects[k];
				RECTARRAY rectArrayTmp;
				CPoint p1 = rectTmp1.TopLeft();
				CPoint p2 = rectTmp1.BottomRight();
				NEWPOINT pTopLeft;
				NEWPOINT pBottomRight;
				pTopLeft.setPoint(p1.x, p1.y);
				pBottomRight.setPoint(p2.x, p2.y);

				rectArrayTmp.nType = rectModelArray.nType;
				rectArrayTmp.rect.SetRect(pTopLeft, pBottomRight);
				rectArrayTmp.nQuestionNo = rectModelArray.nQuestionNo;
				rectArrayTmp.nAnswerIndex = rectModelArray.nAnswerIndex;
				rectArrayTmp.nSelType  = rectModelArray.nSelType;
				strcpy(rectArrayTmp.cTitle, rectModelArray.cTitle);
				rectArrayTmp.nZuID = rectModelArray.nZuID;
				v_dstRectArrayTmps.push_back(rectArrayTmp);
				//TRACE("%d, %d, %d, %d\n",rectArrayTmp.nQuestionNo, rectArrayTmp.nAnswerIndex, rectArrayTmp.rect.pTopLeft.nX,  rectArrayTmp.rect.pTopLeft.nY);
				break;
			}
		}
		//return FALSE;
	}
	
	if (v_dstRectArrayTmps.size() <= 0)
	{
		//既不在指定范围内，也无与模板所在矩形相交 pany 2016.06.17
		for (vector<CRect>::iterator it1 = v_allRects.begin(); it1 != v_allRects.end(); it1++)
		{
			CRect rectTmp1 = *it1;
			int nMinLen = 8000;
			for (int j=0; j<v_modelRects.size(); j++)
			{
				RECTARRAY rectModelArray = v_modelRects[j];
				CPoint pCenter1 = rectTmp1.CenterPoint();
				CPoint pCenter2 = CPoint(rectModelArray.CenterNewPoint().nX, rectModelArray.CenterNewPoint().nY);
				int nLen = (int)CImageProcess::GetLength(pCenter1, pCenter2);

				if (nLen < nMinLen)
				{
					nMinLen = nLen;
					RECTARRAY rectArrayTmp;
					CPoint p1 = rectTmp1.TopLeft();
					CPoint p2 = rectTmp1.BottomRight();
					NEWPOINT pTopLeft;
					NEWPOINT pBottomRight;
					pTopLeft.setPoint(p1.x, p1.y);
					pBottomRight.setPoint(p2.x, p2.y);

					rectArrayTmp.nType = rectModelArray.nType;
					rectArrayTmp.rect.SetRect(pTopLeft, pBottomRight);
					rectArrayTmp.nQuestionNo = rectModelArray.nQuestionNo;
					strcpy(rectArrayTmp.cTitle, rectModelArray.cTitle);
					rectArrayTmp.nZuID = rectModelArray.nZuID;
					rectArrayTmp.nAnswerIndex = rectModelArray.nAnswerIndex;
					rectArrayTmp.nSelType  = rectModelArray.nSelType;
					v_dstRectArrayTmps.push_back(rectArrayTmp);
					//TRACE("%d, %d, %d, %d\n",rectArrayTmp.nQuestionNo, rectArrayTmp.nAnswerIndex, rectArrayTmp.rect.pTopLeft.nX,  rectArrayTmp.rect.pTopLeft.nY);
					//break;
				}
			}
		}
	}
	
	if (v_dstRectArrayTmps.size() <= 0 )
		return false;

	//排序 按题号从小到大 按备选答案序号从小到大
	sort(v_dstRectArrayTmps);
	
	//判断是否有漏缺
	for (int i=0; i<v_modelRects.size(); i++)
	{
		bool bFind = FALSE;
		int nInsert = 0;
		RECTARRAY rArrayTmp1 = v_modelRects[i];
		
		int nIndex = 0;
		for (int j=0; j<v_dstRectArrayTmps.size(); j++)
		{
			RECTARRAY rArrayTmp2 = v_dstRectArrayTmps[j];
			if (rArrayTmp1.nQuestionNo == rArrayTmp2.nQuestionNo)
			{
				if (rArrayTmp1.nAnswerIndex == rArrayTmp2.nAnswerIndex)
				{
					bFind = TRUE;
					break;
				}
				else if (rArrayTmp1.nAnswerIndex < rArrayTmp2.nAnswerIndex)
				{
					nIndex = j;
					nInsert = j;
					break;
				}
				nIndex = j;
			}
			else if (rArrayTmp1.nQuestionNo < rArrayTmp2.nQuestionNo)
			{
				nIndex = j;
				nInsert = j;
				if (nInsert < 0)
					nInsert = 0;
				break;
			}

			nIndex = j;
			nInsert = j+1;
		}
 
		if (!bFind)
		{
			//获取临近位置处
			RECTARRAY dstModelRArray;
			RECTARRAY dstRArrayTmp;
			GetModelRectArray(v_dstRectArrayTmps[nIndex], v_modelRects, dstModelRArray);
			
			NEWPOINT p1;
			p1.setPoint(-dstModelRArray.CenterNewPoint().nX + rArrayTmp1.CenterNewPoint().nX,
				-dstModelRArray.CenterNewPoint().nY + rArrayTmp1.CenterNewPoint().nY);

			dstRArrayTmp = rArrayTmp1;
			dstRArrayTmp.rect = v_dstRectArrayTmps[nIndex].rect;
			dstRArrayTmp.AddPoint(p1);
			v_dstRectArrayTmps.insert(v_dstRectArrayTmps.begin()+nInsert, dstRArrayTmp);
		} 
	}

	//判断是否有重复 2016.01.06
	for (int i=0; i<v_dstRectArrayTmps.size(); i++)
	{
		for (int j=i+1; j<v_dstRectArrayTmps.size(); j++)
		{
			if (v_dstRectArrayTmps[i].nQuestionNo == v_dstRectArrayTmps[j].nQuestionNo)
			{
				if (v_dstRectArrayTmps[i].nAnswerIndex == v_dstRectArrayTmps[j].nAnswerIndex)
				{
					v_dstRectArrayTmps.erase(v_dstRectArrayTmps.begin()+j);
					j-=1;
					//break;
				}
			}
		}
	}

	v_DstRectArray = v_dstRectArrayTmps;

	return TRUE;
}

bool GetRectArry_1(vector<RECTARRAY> &v_DstRectArray, vector<RECTARRAY> v_curRectArray,vector<RECTARRAY> v_modelRects,  CxImage *srcImg, int nThreshold)
{
	if (v_curRectArray.size() == v_modelRects.size())
	{
		v_DstRectArray = v_curRectArray;
		return TRUE;
	}

	if (v_curRectArray.size() == 0)
		return FALSE;

	vector<int> v_allNoFind;
	RECTARRAY rectModelStart;
	RECTARRAY rectCurStart;
	bool bStart = false;
	v_DstRectArray = v_curRectArray;
	for (int i=0; i<v_modelRects.size(); i++)
	{
		bool bFind = false;
		for (int j=0; j<v_curRectArray.size(); j++)
		{
			if (v_modelRects[i].nQuestionNo == v_curRectArray[j].nQuestionNo &&
				v_modelRects[i].nAnswerIndex == v_curRectArray[j].nAnswerIndex)
			{
				if (!bStart)
				{
					bStart = TRUE;
					rectModelStart = v_modelRects[i];
					rectCurStart = v_curRectArray[j];
				}

				bFind = TRUE;
			}
		}

		if (!bFind)
		{
			v_allNoFind.push_back(i);
		}
	}

	if (!bStart)
		return FALSE;

	for (int i=0; i<v_allNoFind.size(); i++)
	{
		int nIndex = v_allNoFind[i];
		int nMoveX = v_modelRects[nIndex].rect.CenterNewPoint().nX - rectModelStart.CenterNewPoint().nX;
		int nMoveY = v_modelRects[nIndex].rect.CenterNewPoint().nY - rectModelStart.CenterNewPoint().nY;

		RECTARRAY rectArrayTmp = v_modelRects[nIndex];
		rectArrayTmp.rect = rectCurStart.rect;
		
		NEWPOINT p1;
		p1.setPoint(nMoveX, nMoveY);
		rectArrayTmp.AddPoint(p1);
		int nAver =0;
		rectArrayTmp.dDensity = GetGrayDensity(rectArrayTmp.rect, nThreshold, srcImg,  nAver);
		rectArrayTmp.nAverGrayValue = nAver;
		ZYJ_IsUnFilling(rectArrayTmp, nThreshold, srcImg);//判断是否为无

		v_DstRectArray.push_back(rectArrayTmp);
	}

	if (v_DstRectArray.size() != v_modelRects.size())
		return false;

	//排序
	sort(v_DstRectArray);

	return TRUE;
}

NEWRECT GetRectArea(const RECTARRAY *v_modelrArray, int nOmrSize, CxImage *srcImg)
{
	 NEWRECT rectArea;
	 RECTARRAY rArray1 = v_modelrArray[0];
	 int nTop    = rArray1.rect.pTopLeft.nY;
	 int nBottom = rArray1.rect.pBottomRight.nY; 
	 int nLeft   = rArray1.rect.pTopLeft.nX;
	 int nRight  = rArray1.rect.pBottomRight.nX;
	
	 for (int i=0; i<nOmrSize; i++)
	 {
		 NEWRECT rect1 = v_modelrArray[i].rect;
		 int nY1 = rect1.pTopLeft.nY;
		 int nY2 = rect1.pBottomRight.nY;
		 int nX1 = rect1.pTopLeft.nX;
		 int nX2 = rect1.pBottomRight.nX;
		 if (nTop > nY1)
			 nTop = nY1;

		 if (nBottom < nY2)
			 nBottom = nY2;

		 if (nLeft > nX1)
			 nLeft = nX1;

		 if (nRight < nX2)
			 nRight = nX2;
	 }

	 //扩大区域
	 nTop -= 20;
	 nBottom += 20;
	 nLeft -= 20;
	 nRight += 20;

	 int nImgWidth = srcImg->GetWidth();
	 int nImgHeight = srcImg->GetHeight();

	 if (nTop < 0)
		 nTop = 0;

	 if (nLeft < 0)
		 nLeft = 0;

	 if (nBottom > nImgHeight-1)
		 nBottom = nImgHeight-1;

	 if (nRight > nImgWidth-1)
		 nRight = nImgWidth-1;

	 rectArea.SetRect(nLeft, nTop, nRight-nLeft+1, nBottom-nTop+1);
	 return rectArea;

}

NEWRECT GetRectArea1(NEWRECT rect1, CxImage *srcImg)
{
	NEWRECT rectArea;
	int nTop = rect1.pTopLeft.nY;
	int nBottom = rect1.pBottomRight.nY;
	int nLeft = rect1.pTopLeft.nX;
	int nRight = rect1.pBottomRight.nX;

	nTop -= 150;
	nBottom += 150;
	nLeft -= 150;
	nRight += 150;

	int nImgWidth = srcImg->GetWidth();
	int nImgHeight = srcImg->GetHeight();

	if (nTop < 0)
		nTop = 0;

	if (nBottom > nImgHeight-1)
		nBottom = nImgHeight-1;

	if (nLeft < 0)
		nLeft = 0;

	if (nRight > nImgWidth-1)
		nRight = nImgWidth-1;

	rectArea.SetRect(nLeft, nTop, nRight-nLeft+1, nBottom-nTop+1);
	return rectArea;
}


 BOOL ZYJ_GetRectArray(int nOmrType, vector<RECTARRAY> &dstArray, int nRecType, int nZuID, CxImage *srcImg,  CurImage *imgPara)
{
	int nModelSize = 0;
	CImageProcess imgProcess;
	vector<RECTARRAY> v_CurZuRectArray;

	if (nRecType == 0) //客观题
	{
		for (int i=0; i<imgPara->v_modelRectArrays.size(); i++)
		{
			if (imgPara->v_modelRectArrays[i].nZuID == nZuID)
			{
				nModelSize++;
				v_CurZuRectArray.push_back(imgPara->v_modelRectArrays[i]);
			}
		}
	}
	else if (nRecType == 1) //考号
	{
		for (int i=0; i<imgPara->v_modelKhRectArray.size(); i++)
		{
			if (imgPara->v_modelKhRectArray[i].nZuID == nZuID)
			{
				nModelSize++;
				v_CurZuRectArray.push_back(imgPara->v_modelKhRectArray[i]);
			}
		}
	}
	else if (nRecType == 2) //缺考
	{
        nModelSize++;
		v_CurZuRectArray.push_back(imgPara->modelAbsentRectArray);
	}

	if (nModelSize <= 0)
		return FALSE;

	 RECTARRAY *modelArray = new RECTARRAY[nModelSize];  
     for (int i=0; i<nModelSize; i++)
	 {
		modelArray[i] = v_CurZuRectArray[i];
	 }

	NEWRECT rectArea = GetRectArea(modelArray, nModelSize, srcImg); //当前所有客观题所在区域
	int nThreshold = imgPara->m_nThreshold;
	int nOMRType = nOmrType;
	/*vector<RECTARRAY> v_modelArray;*/
	CxImage *srcImage = srcImg; 
	if (!srcImage)
	{
		delete []modelArray;
		modelArray = NULL;
		return FALSE;
	}

	CxImage img;
	RECT rect;
	rect.top    = rectArea.pTopLeft.nY;
	rect.bottom = rectArea.pBottomRight.nY;
	rect.left   = rectArea.pTopLeft.nX;
	rect.right  = rectArea.pBottomRight.nX;
	if (!srcImage->Crop(rect, &img)) //获取目标区域图像 2016.01.06
	{
		delete []modelArray;
		modelArray = NULL;
		return FALSE;
	}

	CSize dstSize;
	int nTotalX = 0;
	int nTotalY = 0;
	
	for (int i=0; i<nModelSize; i++)
	{
		RECTARRAY rArrayTmp = modelArray[i];
		nTotalX += rArrayTmp.rect.GetWidth();
		nTotalY += rArrayTmp.rect.GetHeight();
		//v_modelArray.push_back(modelArray[i]);
	}
	
	dstSize.cx = nTotalX/nModelSize;
	dstSize.cy = nTotalY/nModelSize;
	
	//判断填涂类型 
	//int nTime1 = GetTickCount();
	vector<CRect> v_curRect; //当前待识别
	vector<CRect> v_curRect1;
	if (nOMRType == 0) //中括号
	{
		bool bRes = FALSE;
		bool bRes1 = FALSE;
		bRes = imgProcess.GetValidRect(0, &img, v_curRect, nThreshold, dstSize);
		if (v_curRect.size() < min(nModelSize/2, 5) && v_curRect.size() >= 0)
		{
			bRes1 = imgProcess.GetEnclosedRects(0, &img, v_curRect1, nThreshold, dstSize); 
		}
		
		if (v_curRect1.size() > v_curRect.size())
			v_curRect = v_curRect1;

		if (!bRes && !bRes1)
		{
			delete []modelArray;
			modelArray = NULL;
			return FALSE;
		}
		//DebugString("获取区域内矩形耗时%d\n", GetTickCount()-time1);
	}
	else if (nOMRType == 1) //封闭矩形
	{
		bool bRes = FALSE; 
		bRes = imgProcess.GetEnclosedRects(0, &img, v_curRect, nThreshold, dstSize);
		
		if (!bRes)
		{
			delete []modelArray;
			modelArray = NULL;
			return FALSE;
		}
	}

	//TRACE("耗时:%d\n", GetTickCount() -nTime1);
	vector<RECTARRAY> v_DstRectArray;
	vector<RECTARRAY> v_ModelArrayTmp = v_CurZuRectArray;//v_modelArray;
	NEWPOINT p1;
	p1.setPoint(-rectArea.pTopLeft.nX, -rectArea.pTopLeft.nY);

	NEWPOINT pLocalModel[3]; //模板定位点
	NEWPOINT pCurLocal[3]; //当前图像定位点
	for (int i=0; i<3; i++)
	{
		pLocalModel[i] = imgPara->v_rectModelLocal[i].CenterNewPoint();
		pCurLocal[i] = imgPara->m_rectLocal[i].CenterNewPoint(); 
	}

	for (int i=0; i<v_ModelArrayTmp.size(); i++)
	{
		double dHRatio, dVRatio;
		NEWPOINT pNewCenter1;
		NEWPOINT pAdd;
		NEWPOINT pModelCenter = v_ModelArrayTmp[i].CenterNewPoint();
		imgProcess.GetRatio(pLocalModel, v_ModelArrayTmp[i].CenterNewPoint(), dHRatio, dVRatio);

		imgProcess.GetNewPoint(pCurLocal, dHRatio, dVRatio, pNewCenter1);
		pAdd.setPoint(pNewCenter1.nX - pModelCenter.nX, pNewCenter1.nY - pModelCenter.nY);
		v_ModelArrayTmp[i].AddPoint(pAdd);	
		v_ModelArrayTmp[i].AddPoint(p1);	
	}

	if (!GetRectArry(v_DstRectArray, v_curRect, v_ModelArrayTmp))
	{
		delete []modelArray;
		modelArray = NULL;
		return FALSE;
	}

	if (v_DstRectArray.size() != nModelSize)
	{
		delete []modelArray;
		modelArray = NULL;
		return FALSE;
	}
	
    dstArray = v_DstRectArray;
	for (int k=0; k<v_DstRectArray.size(); k++)
	{
		//v_DstRectArray[k].nWhiteGray = nWhiteAverValue;
		NEWRECT dstRect1 = v_DstRectArray[k].rect;
		dstArray[k] = v_DstRectArray[k];
		dstArray[k].rect.AddPoint(rectArea.pTopLeft);
		dstRect1.AddPoint(rectArea.pTopLeft);

	    ZYJ_IsUnFilling(dstArray[k], imgPara->m_nThreshold, srcImg);//判断是否为无填涂
		dstArray[k].dDensity = GetGrayDensity(dstRect1, imgPara->m_nThreshold, srcImg, dstArray[k].nAverGrayValue);
		int nAverGrayValueTmp1;
		dstArray[k].dDensity_220 = GetGrayDensity(dstRect1, 220, srcImg, nAverGrayValueTmp1);
		dstArray[k].dDensity_140 = GetGrayDensity(dstRect1, 140, srcImg, nAverGrayValueTmp1);
		
		CxImage cropImg1[3]; // = new CxImage();
		for (int kk=0; kk<3; kk++)
		 srcImg->Crop(NewRect2Rect(dstRect1), &cropImg1[kk]);
		if (dstRect1.GetWidth() >= 5 &&  dstRect1.GetHeight() >= 5)
		{
			int nMaxFillingHeight =  imgProcess.GetMaxFillHeight(cropImg1[0], nThreshold, dstSize);
			if (nMaxFillingHeight >= dstSize.cy/2)
				dstArray[k].bMinFilling = true;

			int nMaxFillHeight_220 = imgProcess.GetMaxFillHeight(cropImg1[1], 220, dstSize);
			if (nMaxFillHeight_220 >= dstSize.cy/2)
				dstArray[k].bMinFilling_220 = true;

			int nMaxFillHeight_140 = imgProcess.GetMaxFillHeight(cropImg1[2], 140, dstSize);
			if (nMaxFillHeight_140 >= dstSize.cy/2)
				dstArray[k].bMinFilling_140 = true;
	
		}
	}

	delete []modelArray;
	modelArray = NULL;
	return TRUE;
}

CRect &NewRect2Rect(NEWRECT newRect)
{
	CRect rect;
	rect.top = newRect.pTopLeft.nY;
	rect.bottom = newRect.pBottomRight.nY;
	rect.left = newRect.pTopLeft.nX;
	rect.right = newRect.pBottomRight.nX;
	return rect;
}

NEWRECT &Rect2NewRect(CRect rect)
{
   NEWPOINT p1, p2;
   p1.setPoint(rect.left, rect.top);
   p2.setPoint(rect.right, rect.bottom);
   NEWRECT dstRect;
   dstRect.SetRect(p1, p2);
   return dstRect;
}


double GetGrayDensity(NEWRECT rectArea, int nThrshold, CxImage *srcImg, int &nAverValue)
{
	double dDensity = 0.0;
	int nTolalGray = 0;
	CRect rect = NewRect2Rect(rectArea);
	CxImage cropImg;
	srcImg->Crop(rect, &cropImg);
	int nImgWidth = cropImg.GetWidth();
	int nImgHeight = cropImg.GetHeight();
	
	int nCount =0;
	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			int nGrayValue = GETGRAYVALUE2(cropImg, i, j);
			if (nGrayValue <= nThrshold)
			{
				nCount++;
			}

			nTolalGray += nGrayValue;
		}
	}
	
	if (nImgHeight*nImgWidth > 0)
	{
		dDensity = double(nCount) / double(nImgWidth*nImgHeight);
		nAverValue = nTolalGray/(nImgWidth*nImgHeight);
	}

	return dDensity;
}


bool ZYJ_GetRectLocalPoint(NEWRECT rectArea,int nModelRectWidth, int nModelRectHeight, int nIndex, int nRectType, NEWRECT &dstRect, CxImage *srcImg, CurImage *ImgPara)
{
	CxImage cropImg;
	CImageProcess imgProcess;
	CRect rectArea1 = NewRect2Rect(rectArea);
	int nThreshold = ImgPara->m_nThreshold;
	CSize dstSize = CSize(nModelRectWidth, nModelRectHeight);
	srcImg->Crop(rectArea1, &cropImg);
	
	CRect dstRectArea;
	dstRectArea.SetRect(0, 0, rectArea1.Width()-1, rectArea1.Height()-1); //新的目标定位点所在矩形
	int nWidth  = cropImg.GetWidth();
	int nHeight = cropImg.GetHeight();

	if (nRectType == 2) //定位点矩形类型
	{
		for (int i=0; i<nHeight; i++)
		{
			for (int j=0; j<nWidth; j++)
			{
				CPoint pStart; 
				if (nIndex == 0)
				{
					pStart = CPoint(j, i);
				}
				else if (nIndex == 1)
				{
					pStart = CPoint(nWidth-1-j, i);
				}
				else if (nIndex == 2)
				{
					pStart = CPoint(j, nHeight-1-i);
				}
				else if (nIndex == 3)
				{
					pStart = CPoint(nWidth-1-j, nHeight-1-i);
				}

				if (GETGRAYVALUE2(cropImg, pStart.x, pStart.y) <= nThreshold)
				{	
					CRect dstRectTmp;					
					if (imgProcess.GetRectFromPoint(dstRectTmp, pStart, dstRectArea, dstSize, &cropImg, nThreshold))
					{

						CRect rectTmp1;
						rectTmp1.SetRect(dstRectTmp.left- dstRectTmp.Width()/2, dstRectTmp.top - dstRectTmp.Height()/2,
							dstRectTmp.right + dstRectTmp.Width()/2, dstRectTmp.bottom+ dstRectTmp.Height()/2);

						CPoint p1 = rectTmp1.TopLeft();
						CPoint p2 = rectTmp1.BottomRight();

						if (rectTmp1.left < 0)
							p1.x = 0;

						if (rectTmp1.right > nWidth-1)
							p2.x = nWidth-1;

						if (rectTmp1.top < 0)
							p1.y = 0;

						if (rectTmp1.bottom > nHeight-1)
							p2.y = nHeight-1;

						CRect dstRectTmp1;
						dstRectTmp1.SetRect(p1, p2);

						int nCount1 = 0;
						int nCount2 = 0;
						imgProcess.GetBlackCount(&cropImg, dstRectTmp1, ImgPara->m_nThreshold,  nCount1);
						imgProcess.GetBlackCount(&cropImg, dstRectTmp, ImgPara->m_nThreshold,  nCount2);

						if (double(nCount1 - nCount2) / double(max(dstRectTmp.Width()*dstRectTmp.Height(), 1)) >= 0.25)
							continue;

						CPoint pCenter1 = dstRectArea.CenterPoint();
						CPoint pCenter2 = dstRectTmp.CenterPoint();
						if (imgProcess.GetLength(pCenter1, pCenter2) > 2*max(dstSize.cx, dstSize.cy))
							continue;

						dstRect = Rect2NewRect(dstRectTmp);
						dstRect.AddPoint(rectArea.pTopLeft);
						ImgPara->m_rectLocal.push_back(dstRect);
						return TRUE;
					}
				}
				else
					j += max(nModelRectWidth/2, 1);
			}

			i += max(nModelRectHeight/2, 1);
		}


		for (int i=0; i<nHeight; i++)
		{
			for (int j=0; j<nWidth; j++)
			{
				CPoint pStart; 
				if (nIndex == 0)
				{
					pStart = CPoint(j, i);
				}
				else if (nIndex == 1)
				{
					pStart = CPoint(nWidth-1-j, i);
				}
				else if (nIndex == 2)
				{
					pStart = CPoint(j, nHeight-1-i);
				}
				else if (nIndex == 3)
				{
					pStart = CPoint(nWidth-1-j, nHeight-1-i);
				}

				if (GETGRAYVALUE2(cropImg, pStart.x, pStart.y) <= nThreshold)
				{	
					
					//CPoint pStart = CPoint(j, i);
					CRect dstRectTmp;		

					vector<CPoint> v_AllPoints;
					int nCount = 0;
					if (imgProcess.GetAllPoint(&cropImg, v_AllPoints, pStart, nThreshold, nCount))
					{
						
						CRect dstRectTmp;	
						bool bRes1 = imgProcess.GetRect(v_AllPoints, dstRectTmp);
						if (bRes1)
						{
							if (dstRectTmp.Width() > dstSize.cx/2 && dstRectTmp.Height() > dstSize.cy /2)
							{
								if (abs(dstRectTmp.Width() - dstSize.cx) <= nEPS &&
									abs(dstRectTmp.Height() - dstSize.cy) <= nEPS)
								{

									//判断矩形有效性
									CRect rectTmp1;
									rectTmp1.SetRect(dstRectTmp.left- dstRectTmp.Width()/2, dstRectTmp.top - dstRectTmp.Height()/2,
										dstRectTmp.right + dstRectTmp.Width()/2, dstRectTmp.bottom+ dstRectTmp.Height()/2);

									CPoint p1 = rectTmp1.TopLeft();
									CPoint p2 = rectTmp1.BottomRight();

									if (rectTmp1.left < 0)
										p1.x = 0;

									if (rectTmp1.right > nWidth-1)
										p2.x = nWidth-1;

									if (rectTmp1.top < 0)
										p1.y = 0;

									if (rectTmp1.bottom > nHeight-1)
										p2.y = nHeight-1;

									CRect dstRectTmp1;
									dstRectTmp1.SetRect(p1, p2);

									int nCount1 = 0;
									int nCount2 = 0;
									imgProcess.GetBlackCount(&cropImg, dstRectTmp1, ImgPara->m_nThreshold,  nCount1);
									imgProcess.GetBlackCount(&cropImg, dstRectTmp, ImgPara->m_nThreshold,  nCount2);

									if (double(nCount1 - nCount2) / double(max(dstRectTmp.Width()*dstRectTmp.Height(), 1)) >= 0.25)
										continue;

									dstRect = Rect2NewRect(dstRectTmp);
									dstRect.AddPoint(rectArea.pTopLeft);
									ImgPara->m_rectLocal.push_back(dstRect);
									return TRUE;
								}
							}
						}
					}
				}
				else
					j += max(nModelRectWidth/2, 1);
			}

			i += max(nModelRectHeight/3, 1);
		}

	}
	else if (nRectType == 1) //定位点三角形类型
	{
		for (int j=0; j<nWidth; j++)
		{
			for (int i=0; i<nHeight; i++)
			{
				CPoint pStart; 
				if (nIndex == 0)
				{
					pStart = CPoint(j, i);
				}
				else if (nIndex == 1)
				{
					pStart = CPoint(nWidth-1-j, i);
				}
				else if (nIndex == 2)
				{
					pStart = CPoint(j, nHeight-1-i);
				}
				else if (nIndex == 3)
				{
					pStart = CPoint(nWidth-1-j, nHeight-1-i);
				}

				if (GETGRAYVALUE2(cropImg, pStart.x, pStart.y) <= nThreshold)
				{	
					//CPoint pStart = CPoint(j, i);
					CRect dstRectTmp;										
					NEWTRIANGLE dstNewTriangle;
					if (imgProcess.GetTriAngleLocalPoint(&cropImg, dstSize, nThreshold, dstNewTriangle))
					{
						dstRect = dstNewTriangle.GetNewRect();
						dstRect.AddPoint(rectArea.pTopLeft);
						ImgPara->m_rectLocal.push_back(dstRect);
						return TRUE;
					}
				}
				else
					i += max(nModelRectHeight/3, 1);
			}
			j += max(nModelRectWidth/3, 1);
		}
	}

	return FALSE;
}


bool ZYJ_GetOmrResult(vector<RECTARRAY> &srcRects, int nSize, /*int nSelCheck, */ vector<RECTARRAY> modelRectArry, int &nFillAverGray)
{
	//按备选答案序号索引排序计算未填涂部分平均密度
	map<int, double> mapDensity;
	map<int, double> mapModelDensity;
	for (int i=0; i<26; i++)
	{
		bool bFind = FALSE;
		double dTotal = 0.0;
		int nNum = 0;
		for (int j=0; j<nSize; j++)
		{
			if (srcRects[j].nAnswerIndex == i)
			{
				bFind = TRUE;
				
				if (srcRects[j].nFilling == 0)//未填涂
				{
					dTotal += srcRects[j].dDensity;
					nNum++;
				}
			}
		}

		if (!bFind) //未找到
			break;

		double dAverDensity = 0.0;		
		if (nNum > 0)
			dAverDensity = dTotal/double(nNum);//平均密度
		
		mapDensity.insert(pair<int, double>(i, dAverDensity));
	}

	for (int i=0; i<26; i++)
	{
		bool bFind = FALSE;
		double dTotal = 0.0;
		int nNum = 0;
		for (int j=0; j<modelRectArry.size(); j++)
		{
			if (modelRectArry[j].nAnswerIndex == i)
			{
				bFind = TRUE;
				modelRectArry[j].nFilling = 0;
				dTotal += modelRectArry[j].dDensity;
				nNum++;
			}
		}
		if (!bFind) //未找到
			break;

		double dAverDensity = 0.0;		
		if (nNum > 0)
			dAverDensity = dTotal/double(nNum);//平均密度

		mapModelDensity.insert(pair<int, double>(i, dAverDensity));
	}
	
	//计算模板里面未填涂的密度
	//当前未填涂的模板平均密度
	//modelRectArry
	int nTotalGray = 0;
	int nCount = 0;
	for (int i=0; i<nSize; i++)
	{
		map<int, double>::iterator it;
		map<int, double>::iterator itModel;
		it = mapDensity.find(srcRects[i].nAnswerIndex);
		//TRACE("nIndex:%d, 密度:%.4f\n", it->first, it->second);

		itModel = mapModelDensity.find(modelRectArry[i].nAnswerIndex);
		
		double dUnFilling = it->second; //未填涂平均密度
		double dUnFillModel = itModel->second;

		double dTmp = srcRects[i].dDensity;
		double dTmp_220 = srcRects[i].dDensity_220;
		double dTmp_140 = srcRects[i].dDensity_140;
		double dEps = 0.20;
		if (max(dUnFillModel, dUnFilling) >= 0.5)
			dEps = 0.15;

		double dAverUnFillTmp = dUnFillModel;
		if (dUnFilling > 0.0001) //有效
			dAverUnFillTmp =  dUnFilling;
		else 
			dAverUnFillTmp = max(dUnFillModel, dUnFilling); 

		if (dTmp - dAverUnFillTmp >= dEps || dTmp >= 0.75)
		{
			srcRects[i].nFilling = 1;
			nTotalGray  += srcRects[i].nAverGrayValue;
			nCount++;
		}
		else 
		{
			if (srcRects[i].bMinFilling)
			{
				srcRects[i].nFilling = 1;
				nTotalGray  += srcRects[i].nAverGrayValue;
				nCount++;
			}
			else 
				srcRects[i].nFilling = 0;
		}

		if (dTmp_220 - dAverUnFillTmp >= dEps || dTmp_220 >= 0.75)
		{
			srcRects[i].nFilling_220 = 1;
		}
		else 
		{
			if (srcRects[i].bMinFilling_220)
				srcRects[i].nFilling_220 = 1;
			else 
				srcRects[i].nFilling_220 = 0;
		}

		if (dTmp_140 - dAverUnFillTmp >= dEps || dTmp_140 >= 0.75)
		{
			srcRects[i].nFilling_140 = 1;
		}
		else 
		{
			if (srcRects[i].bMinFilling_140)
				srcRects[i].nFilling_140 = 1;
			else 
				srcRects[i].nFilling_140 = 0;
		}
	}

	//计算已填涂的平均灰度值
	if (nCount > 0)
		nFillAverGray = nTotalGray/nCount;
	
	//判断单选题多选情况
	int k = 0;
	int k1 = 0;
	for (int i=0; i<nSize; i++)
	{
		int nCout1 = 0;
		int nFillCount = 0;
		
		if (srcRects[i].nSelType == 0) //判断起始
			k = i;
		k1 = i;
		if (srcRects[i].nFilling == 1)
		{
			nFillCount++;

			if (srcRects[i].nSelType == 0)
				nCout1++;
		}

		for (int j=i+1; j<nSize; j++)
		{
			if (srcRects[i].nQuestionNo != srcRects[j].nQuestionNo)
			{
				i = j-1;
				break;
			}

			if (srcRects[j].nFilling == 1)
			{
				nFillCount++;
				if (srcRects[i].nSelType == 0)
					nCout1++;
			}

			if (j == nSize-1)
			{
				i = j;
				break;
			}
		}

		if (nCout1 >= 2)
		{
			//单选题多选
			bool bFindIndex = false;
			map<int, bool> map_fillingIndex;
			for (int m=k; m<nSize; m++)
			{
				if (srcRects[k].nQuestionNo != srcRects[m].nQuestionNo)
					break;

				if (srcRects[m].nFilling == 1)
				{
					//int nGrayTmp = srcRects[m].nAverGrayValue;
					if (srcRects[m].bMinFilling)
					{
						if (srcRects[m].bMinFilling_140 && srcRects[m].nFilling_140 == 1)
						{
							bFindIndex = true;
							map_fillingIndex.insert(pair<int, bool>(m, true));
						}
						else 
							map_fillingIndex.insert(pair<int, bool>(m, false));

						srcRects[m].nFilling = 1;
					}
					else 
						srcRects[m].nFilling = 0;
					
				}
			}

			if (bFindIndex)
			{
				for (map<int, bool>::iterator it1 = map_fillingIndex.begin(); it1 != map_fillingIndex.end(); it1++)
				{
					int nCurIndex = it1->first;
					bool bFilling = it1->second;

					if (bFilling)
						srcRects[nCurIndex].nFilling = 1;
					else 
						srcRects[nCurIndex].nFilling = 0;
				}
			}
		}

		if (nFillCount == 0) //未填涂
		{
			for (int m=k1; m<nSize; m++)
			{
				if (srcRects[k1].nQuestionNo != srcRects[m].nQuestionNo)
					break;

				if (srcRects[m].nFilling_220 == 1)
				{
					/*int nGrayTmp = srcRects[m].nAverGrayValue;*/
					if (srcRects[m].bMinFilling_220)
						srcRects[m].nFilling = 1;
					else 
						srcRects[m].nFilling = 0;
				}
			}
		}
	}

	return TRUE;
}


 extern "C" DLL_EXPORT bool ZYJ_CutImg(const char *cFilePath, const CUTIMGINFO *pCutInfo, const int nSize)
{
	CxImage srcImg;
	bool bRes = FALSE;
	USES_CONVERSION;
	CString strSrcPath = A2T(cFilePath);
	strSrcPath.MakeLower();
	
	DWORD nFileType = GetFileType1(strSrcPath);
	bRes = srcImg.Load(strSrcPath, nFileType); //加载原图
	if (!bRes)	
		return FALSE;

	if (nSize <= 0)
		return FALSE;

	for (int i=0; i<nSize; i++)
	{
		CUTIMGINFO cutInfoTmp = pCutInfo[i];
		CString strSaveChildPath; // 保存子图路径
		strSaveChildPath = A2T(cutInfoTmp.cSavePath);
		strSaveChildPath = strSaveChildPath.MakeLower();

		DWORD dSaveType = GetFileType1(strSaveChildPath);
		CRect rectSave = NewRect2Rect(cutInfoTmp.rectArea);
		CxImage imgSave;
		srcImg.Crop(rectSave, &imgSave);
		if (::GetFileAttributes(strSaveChildPath)==-1)//文件未存在 2016.01.28
		{
			imgSave.Save(strSaveChildPath, dSaveType);
		}
		else //子图文件已存在
		{
			CxImage imgExist;
			bRes = imgExist.Load(strSaveChildPath, dSaveType);

			if (!bRes)
				return FALSE;
			bRes = MergeNewImage(imgExist, imgSave, strSaveChildPath);
			if (!bRes)
				return FALSE;
		}
	}

	return TRUE;
}

 bool GetCurImgInfo(CxImage *srcImg, const char * cFilePath, const int nPage, const int nOmrType, const int nThreshold, const int nRecType, int nABFlag, CurImage &curImgPara)
 {
	// CurImage curImgPara;
	 //加载当期模板
	 USES_CONVERSION;
	 bool bRes = TRUE;
	 curImgPara.m_nThreshold = nThreshold;
	
	 int nRectType = g_nLocalType;
	 curImgPara.v_rectModelLocal.clear();
	 curImgPara.v_modelRectArrays.clear();
	 curImgPara.v_OMRZuIDs.clear();
	 curImgPara.v_modelKhRectArray.clear();
	 curImgPara.m_rectLocal.clear();
	 curImgPara.v_modelTitleQRRectArray.clear();

	 for (int i = 0; i < g_v_rectModelLocal.size(); i++)
	 {
		 if (g_v_rectModelLocal[i].nPage == nPage)
		 {
			 curImgPara.v_rectModelLocal.push_back(g_v_rectModelLocal[i]);
		 }
	 }

	 for (int i = 0; i < g_v_modelRectArrays.size(); i++)
	 {
		 if (g_v_modelRectArrays[i].rect.nPage == nPage && g_v_modelRectArrays[i].nABFlag == nABFlag)
		 {
			 curImgPara.v_modelRectArrays.push_back(g_v_modelRectArrays[i]);
		 }
	 }

	 for (int i = 0; i < g_v_omrZuIDs.size(); i++)
	 {
		 if (g_v_omrZuIDs[i].nPage == nPage && g_v_omrZuIDs[i].nABFlag == nABFlag)
		 {
			 curImgPara.v_OMRZuIDs.push_back(g_v_omrZuIDs[i].nID);
		 }
	 }

	 for (int i = 0; i < g_v_modelKhRA.size(); i++)
	 {
		 if (g_v_modelKhRA[i].rect.nPage == nPage)
		 {
			 curImgPara.v_modelKhRectArray.push_back(g_v_modelKhRA[i]);
		 }
	 }

	 for (int i = 0; i < g_v_modelQRTitle.size(); i++)
	 {
		 if (g_v_modelQRTitle[i].rect.nPage == nPage)
		 {
			 curImgPara.v_modelTitleQRRectArray.push_back(g_v_modelQRTitle[i]);
		 }
	 }

     //添加缺考相关信息 2017.02.07
     if (nPage == 1) //首页
     {
         curImgPara.modelAbsentRectArray = g_RaAbsent;
     }


	 if (curImgPara.v_rectModelLocal.size() < 3)
	 {
		 //g_imgProcess->m_nStatueRes = ERROR_LOCATE;
    /*     if (srcImg != NULL)
         {
             delete srcImg;
             srcImg = NULL;
         }*/
		 return FALSE;
	 }


	 //将定位点排序 以考虑图像上任意三个定位点问题
	 CRect rectLocals[3];
	 double dLenth[3] = { 0.0, 0.0, 0.0 };
	 for (int i = 0; i < 3; i++)
	 {
		 rectLocals[i] = NewRect2Rect(curImgPara.v_rectModelLocal[i]);
	 }
	 dLenth[0] = CImageProcess::GetLength(rectLocals[0].CenterPoint(), rectLocals[1].CenterPoint());
	 dLenth[1] = CImageProcess::GetLength(rectLocals[0].CenterPoint(), rectLocals[2].CenterPoint());
	 dLenth[2] = CImageProcess::GetLength(rectLocals[1].CenterPoint(), rectLocals[2].CenterPoint());

	 for (int i = 0; i < 3; i++)
	 {
		 if (abs(max(max(dLenth[0], dLenth[1]), dLenth[2]) - dLenth[i]) <= 0.00000001) //判断斜边选
		 {
			 if (i == 0)
			 {
				 NEWRECT modelTmps = curImgPara.v_rectModelLocal[0];
				 curImgPara.v_rectModelLocal[0] = curImgPara.v_rectModelLocal[2];
				 curImgPara.v_rectModelLocal[2] = modelTmps;
			 }
			 else if (i == 1)
			 {
				 NEWRECT modelTmps = curImgPara.v_rectModelLocal[0];
				 curImgPara.v_rectModelLocal[0] = curImgPara.v_rectModelLocal[1];
				 curImgPara.v_rectModelLocal[1] = modelTmps;
			 }
		 }
	 }

	 //获取三个角定位点相关
	 //int nTime1 = GetTickCount();

	 //g_mute.Lock();
	 bool bFindLocal = false;
     /*for (vector<LOCALPOINT_DATA>::iterator it = g_v_LocalPointData.begin();
         it != g_v_LocalPointData.end(); it++)
     {
         LOCALPOINT_DATA localPointData = *it;
         if (strcmp(localPointData.cCurImgPath, cFilePath) == 0)
         {
             curImgPara.m_rectLocal.clear();
             for (int i = 0; i < 3; i++)
                 curImgPara.m_rectLocal.push_back(localPointData.pLocalRects[i]);
             bFindLocal = true;
             break;
         }
     }*/
	 //g_mute.Unlock();
	 if (!bFindLocal)
	 {
		 for (int i = 0; i < 3; i++)
		 {
			 NEWRECT rectArea1;
			 NEWRECT dstRect;
            // CxImage *pImgTmp = (CxImage *)srcImg;
			 rectArea1 = GetRectArea1(curImgPara.v_rectModelLocal[i], srcImg);
			 int nDirIndex = 0;
			 if (rectArea1.CenterNewPoint().nX > srcImg->GetWidth() / 2 && rectArea1.CenterNewPoint().nY < srcImg->GetHeight() / 2)
				 nDirIndex = 1;
			 else if (rectArea1.CenterNewPoint().nX < srcImg->GetWidth() / 2 && rectArea1.CenterNewPoint().nY > srcImg->GetHeight() / 2)
				 nDirIndex = 2;
			 else if (rectArea1.CenterNewPoint().nX > srcImg->GetWidth() / 2 && rectArea1.CenterNewPoint().nY > srcImg->GetHeight() / 2)
				 nDirIndex = 3;

			 bRes = ZYJ_GetRectLocalPoint(rectArea1, curImgPara.v_rectModelLocal[i].GetWidth(), curImgPara.v_rectModelLocal[i].GetHeight(),
				 nDirIndex, nRectType, dstRect, srcImg, &curImgPara);

			 if (!bRes)
			 {
				 //g_imgProcess->m_nStatueRes = ERROR_LOCATE;
				 //delete srcImg;
				// srcImg = NULL;
				 return FALSE;
			 }
		 }


		 //if (g_v_LocalPointData.size() >= 20)
		 //{
			// g_mute.Lock();
			// g_v_LocalPointData.erase(g_v_LocalPointData.begin());
			// g_mute.Unlock();
		 //}


		 LOCALPOINT_DATA localPointData;
		 strcpy(localPointData.cCurImgPath, cFilePath);
		 for (int i = 0; i < 3; i++)
			 localPointData.pLocalRects[i] = curImgPara.m_rectLocal[i];
		 //g_v_LocalPointData.push_back(localPointData);
	 }

	 return true;
 }

//识别客观题结果
extern "C" DLL_EXPORT bool ZYJ_GetCurOmrRes1(const char *cFilePath, const int nPage, const int nOmrType, const int nThreshold, const int nRecType, ANS *dstAns, int nABFlag)
{
    int t1 = GetTickCount();
	CxImage *srcImg = new CxImage();
	bool bRes = false;
	int nFileType = CXIMAGE_FORMAT_UNKNOWN;
	USES_CONVERSION;
	CString strFilePath = A2T(cFilePath);
    strFilePath.Replace('//', '/');
	nFileType = GetFileType1(strFilePath);
	bRes = srcImg->Load(strFilePath, nFileType);

	if (!bRes)
	{
        if (srcImg != NULL)
        {
            delete srcImg;
            srcImg = NULL;
        }
		return FALSE;
	}
	srcImg->GrayScale();
	CurImage curImgPara;
	if (!GetCurImgInfo(srcImg, cFilePath, nPage, nOmrType, nThreshold, nRecType, nABFlag, curImgPara))
	{
        if (srcImg != NULL)
        {
            delete srcImg;
            srcImg = NULL;
        }
		return FALSE;
	}
    //TRACE("获取信息:%d\n", GetTickCount() - t1);
	vector<RECTARRAY> v_dstRectArry;
	
	if (nRecType == 1)  //考号 pany  2016.04.08
	{
		vector<RECTARRAY> v_dstArrayZuID;
		if (curImgPara.v_modelKhRectArray.size() <= 0)
		{
            if (srcImg != NULL)
            {
                delete srcImg;
                srcImg = NULL;
            }
			return FALSE;
		}

		bRes = ZYJ_GetRectArray(nOmrType, v_dstArrayZuID, 1, curImgPara.v_modelKhRectArray[0].nZuID, srcImg, &curImgPara);
        if (!bRes)
		{
            if (srcImg != NULL)
            {
                delete srcImg;
                srcImg = NULL;
            }
			return FALSE;
		}

		for (int j=0; j<v_dstArrayZuID.size(); j++)
			v_dstRectArry.push_back(v_dstArrayZuID[j]);

		int nSize1 = curImgPara.v_modelKhRectArray.size();
		if (nSize1  == 0)
		{
            if (srcImg != NULL)
            {
                delete srcImg;
                srcImg = NULL;
            }
			return FALSE;
		}

		//RECTARRAY *dstRectArray1 =  new RECTARRAY[nSize1];
		if (v_dstRectArry.size() == 0) //未成功获取到
		{
            if (srcImg != NULL)
            {
                delete srcImg;
                srcImg = NULL;
            }
			return FALSE;
		}

		if (v_dstRectArry.size() != nSize1 )
		{
			vector<RECTARRAY> v_curRectArrayTmp1 = v_dstRectArry;
			if (!GetRectArry_1(v_dstRectArry, v_curRectArrayTmp1, curImgPara.v_modelKhRectArray, srcImg, curImgPara.m_nThreshold))
			{
                if (srcImg != NULL)
                {
                    delete srcImg;
                    srcImg = NULL;
                }
				return FALSE;
			}

            //TRACE("获取矩形2:%d\n", GetTickCount() - t1);
		}

		ZYJ_GetOmrResult(v_dstRectArry, nSize1, /*0,*/ curImgPara.v_modelKhRectArray, curImgPara.m_nAverGrayValue);
        //TRACE("获取结果1:%d\n", GetTickCount() - t1);
        //ZYJ_GetOmrResult(v_dstRectArry, nSize1, 1, curImgPara);
		int nKhLen = v_dstRectArry.size()/10;
		CString strTmp = _T("");
		int nCount = 0; //每列填涂的个数
		CString strTmp1 = _T("");
		double dDensity = 0.0;
		for (int j=0; j<v_dstRectArry.size(); j++)
		{
			if (j%10 == 0)
			{
				nCount = 0;
				strTmp1 = _T("-");
				dDensity = 0.0;
			}

			if (v_dstRectArry[j].nFilling == 1)
			{
				nCount++;
				if (nCount > 1)
				{
					if (v_dstRectArry[j].dDensity > dDensity)
					{
						dDensity = v_dstRectArry[j].dDensity;
						strTmp1.Format(L"%d", j%10);
					}
				}
				else
				{
					dDensity = v_dstRectArry[j].dDensity;
					strTmp1.Format(L"%d", j%10);
				}
				
			}

			if (j%10 ==  9)
			{
				strTmp += strTmp1;
			}
		}

		if (strTmp.GetLength() > 26)
		{
            if (srcImg != NULL)
            {
                delete srcImg;
                srcImg = NULL;
            }
			return FALSE;
		}

		dstAns[0].nID = curImgPara.v_modelKhRectArray[0].nZuID;
		memcpy(dstAns[0].cAnswer, T2A(strTmp), strTmp.GetLength());

        if (srcImg != NULL)
        {
            delete srcImg;
            srcImg = NULL;
        }

        //TRACE("识别耗时:%d\n", GetTickCount() - t1);
		return TRUE;
	}

	////////////////////////////////////////////////////////////////////////////////
	int nSize = curImgPara.v_modelRectArrays.size();
	if (nSize  == 0)
	{
        if (srcImg != NULL)
        {
            delete srcImg;
            srcImg = NULL;
        }
		return FALSE;
	}

	for (int i=0; i<curImgPara.v_OMRZuIDs.size(); i++)
	{
		vector<RECTARRAY> v_dstArrayZuID;
		bRes = ZYJ_GetRectArray(nOmrType, v_dstArrayZuID, 0, curImgPara.v_OMRZuIDs[i], srcImg, &curImgPara);
	
		if (!bRes)
			continue;

		for (int j=0; j<v_dstArrayZuID.size(); j++)
			v_dstRectArry.push_back(v_dstArrayZuID[j]);
	}

	if (v_dstRectArry.size() == 0) //未成功获取到
	{
        if (srcImg != NULL)
        {
            delete srcImg;
            srcImg = NULL;
        }
		return FALSE;

	}

	if (v_dstRectArry.size() != nSize )
	{
		vector<RECTARRAY> v_curRectArrayTmp = v_dstRectArry;
		if (!GetRectArry_1(v_dstRectArry, v_curRectArrayTmp, curImgPara.v_modelRectArrays, srcImg, curImgPara.m_nThreshold))
		{
            if (srcImg != NULL)
            {
                delete srcImg;
                srcImg = NULL;
            }
			return FALSE;
		}
	}
	
	ZYJ_GetOmrResult(v_dstRectArry, nSize,/* 0,*/ curImgPara.v_modelRectArrays, curImgPara.m_nAverGrayValue);

	//将识别结果分配至对应ID当中
	int nID1 = -1;
	ANS ansTmp;
	CString strAns = L"";
	CString strCheckAns = L"";
	vector<ANS> v_ans;
	vector<ANS> v_ansCheck;
	int nCount1 = 0;
	bool bFind1 = false;
	for (int i=0; i < nSize; i++)
	{
		RECTARRAY rArrayTmp = v_dstRectArry[i];

		if (rArrayTmp.nQuestionNo == -1)
			return FALSE;

		nCount1++;
		bFind1 = true;
		if (rArrayTmp.nQuestionNo > nID1 )
		{
			bFind1 = false;
			//nCount1++;
			ansTmp.nID = rArrayTmp.nQuestionNo;
			nID1 = rArrayTmp.nQuestionNo;
			strcpy(ansTmp.cTitle, rArrayTmp.cTitle);
			ansTmp.dstNewRectFirst.SetRect(rArrayTmp.rect.pTopLeft, rArrayTmp.rect.pBottomRight);
			ansTmp.nSize = nCount1;
			ansTmp.nPage = nPage;
			if (v_ans.size() > 0)
			{
				memcpy(v_ans[v_ans.size() - 1].cAnswer, T2A(strAns), strAns.GetLength());
				v_ans[v_ans.size() - 1].nLenX = ansTmp.nLenX;
				v_ans[v_ans.size() - 1].nLenY = ansTmp.nLenY;
				v_ans[v_ans.size() - 1].nSize = nCount1;
			}
			
			
			v_ans.push_back(ansTmp);
			strAns = L"";
			nCount1 = 0;
		}
		else if (bFind1)
		{
			ansTmp.nLenX = max(rArrayTmp.rect.pTopLeft.nX - v_dstRectArry[i-1].rect.pTopLeft.nX, 0);
			if (abs(ansTmp.nLenX) < nEPS)
				ansTmp.nLenX = 0;
		
			ansTmp.nLenY = max(rArrayTmp.rect.pTopLeft.nY - v_dstRectArry[i - 1].rect.pTopLeft.nY, 0);
			if (abs(ansTmp.nLenY) < nEPS)
				ansTmp.nLenY = 0;
		}

		if (rArrayTmp.nFilling == 1) //已填涂
		{
			char c[2];
			c[0] = 'A' + rArrayTmp.nAnswerIndex;
			c[1] = '\0';
			strAns += A2T(c);
		}
		else  //未填涂 
			strAns += L"";

		if (i == nSize -1)
		{
			nCount1++;
			strcpy(ansTmp.cTitle, rArrayTmp.cTitle);
			if (v_ans.size() > 0)
			{
				memcpy(v_ans[v_ans.size() - 1].cAnswer, T2A(strAns), strAns.GetLength());
				v_ans[v_ans.size() - 1].nLenX = ansTmp.nLenX;
				v_ans[v_ans.size() - 1].nLenY = ansTmp.nLenY;
				v_ans[v_ans.size() - 1].nSize = nCount1;
			}
		}
	}

	if (v_ans.size() > 0)
	{
		for (int i=0; i<v_ans.size(); i++)
		{
			dstAns[i] = v_ans[i];

			CString strText1 = A2T(dstAns[i].cAnswer) ;
			if (strText1.IsEmpty())
			{
				strText1 = _T("#");
				memcpy(dstAns[i].cAnswer, T2A(strText1), strText1.GetLength());
			}
			//TRACE("ID:%d, 答案：%s\n", dstAns[i].nID, dstAns[i].cAnswer);
		}
	}

    if (srcImg != NULL)
    {
        delete srcImg;
        srcImg = NULL;
    }

	//delete curImgPara;
	//curImgPara = NULL;
    //TRACE("识别耗时:%d\n", GetTickCount()-t1);
	return TRUE;
}

extern "C" DLL_EXPORT int ZYJ_GetLastError(char cResInfo[256])
{
	int nResState = 0;//g_imgProcess->m_nStatueRes;
	/*memset(cResInfo, '\0', 256);
	CString strRes = L"";
	USES_CONVERSION;
	if (nResState == RES_SUCCESS)
		strRes = _T("识别成功");
	else if (nResState == ERROR_IMAGEFILE)
		strRes = _T("加载图像失败");
	else if (nResState == ERROR_MODELFILE)
		strRes = _T("加载模板失败");
	else if (nResState == ERROR_LOCATE) 
		strRes = _T("定位点失败");
	else if (nResState == ERROR_RECOGNIZE)
		strRes = _T("识别选项失败");
	strcpy(cResInfo, T2A(strRes));*/
	return nResState;
}

extern "C" DLL_EXPORT bool ZYJ_SaveImgHandle(HANDLE hBmp, const char *cPath)
{
	CxImage img;
	bool bres = img.CreateFromHANDLE(hBmp);
	if (!bres)
	{
		return FALSE;
	}

	USES_CONVERSION;
	CString strFilePath = A2T(cPath);
	int nFileType;
	nFileType = GetFileType1(strFilePath);
	bres = img.GrayScale();
	if (!bres)
		return FALSE;

	bres = img.SetCodecOption(7);
	if (!bres)
		return FALSE;

	if (nFileType == CXIMAGE_FORMAT_JPG)
		img.SetJpegQuality(75);
	bres = img.Save(strFilePath, nFileType);
	return bres;
}

extern "C" DLL_EXPORT bool ZYJ_LoadTemplateFile(const char *cModelPath)
{
	//g_imgProcess->m_nStatueRes = -1; //初始化状态
	g_v_modelRectArrays.clear();
	g_v_rectModelLocal.clear();
	g_v_omrZuIDs.clear();
	g_v_modelKhRA.clear();

	//add 2016.07.26
	g_v_modelQRTitle.clear();
	g_v_modelScoreRA.clear();
	g_v_modelQRKh.clear();
	g_v_modelSubjectRA.clear();

	g_strModleQRTitle = _T("");
	g_nModelHeight = 0;
	g_nModelWidth = 0;
	g_nModelPage = 1;

	int nCurPage = -1;
	USES_CONVERSION;
	CString strPath = A2T(cModelPath);
	bool bRes = FALSE;
	CMarkup makeupXml;
	bRes = makeupXml.Load(strPath);
	if (!bRes)
	{
		return false;
	}

	CString strTagName = _T(""); 
	CString strData = _T("");  
	bool bFind = FALSE;
	makeupXml.IntoElem();
	bFind = makeupXml.FindElem(L"ExamInfo");
	if (!bFind)
	{
		return FALSE;
	}

	//获取定位点类型 2016.02.17
	bFind = makeupXml.FindChildElem(L"Exam");
	if (!bFind)
	{
		return FALSE;
	}

	makeupXml.IntoElem();
	bFind = makeupXml.FindChildElem(L"Width");
	if (!bFind)
		return FALSE;

	strData = makeupXml.GetChildAttrib(L"val");
	int nModelWidth = atoi(T2A(strData));
	g_nModelWidth = nModelWidth;

	bFind = makeupXml.FindChildElem(L"Height");
	if (!bFind)
		return FALSE;
	
	strData = makeupXml.GetChildAttrib(L"val");
	int nModelHeight= atoi(T2A(strData));
	g_nModelHeight = nModelHeight;

	bFind = makeupXml.FindChildElem(L"Location");
	if (!bFind)
	{
		return FALSE;
	}

	strData = makeupXml.GetChildAttrib(L"val");
	g_nLocalType = atoi(T2A(strData)); //定位点类型
	if (g_nLocalType != 1 && g_nLocalType != 2)
		return FALSE;

	bFind = makeupXml.FindChildElem(L"pageCount"); //新增  pany 2016.03.14
	if (!bFind)
	{
		return FALSE;
	}

	strData = makeupXml.GetChildAttrib(L"val");
	int nPageCount = atoi(T2A(strData));
	g_nModelPage = nPageCount;	

	bFind = makeupXml.FindChildElem(L"paperTypeCount"); //新增 pany 2016.05.10
	int nPaperTypeCount = 1;
	if (!bFind) //兼容之前 
	{
		nPaperTypeCount = 1;
	}
	else 
	{
		strData = makeupXml.GetChildAttrib(L"val");
		nPaperTypeCount = atoi(T2A(strData));
	}

	bFind = makeupXml.FindChildElem(L"version");
	if (bFind)
	{
		strData = makeupXml.GetChildData();
		g_strModelVersion = strData;
	}

	makeupXml.OutOfElem();

	bFind = makeupXml.FindChildElem(L"Info");
	if (!bFind)
	{
		return FALSE;
	}

	strData = makeupXml.GetChildData();  

	makeupXml.IntoElem();
	while (makeupXml.FindChildElem(L"Zuinfo"))
	{
		strData = makeupXml.GetChildAttrib(L"pageid");
		nCurPage = atoi(T2A(strData));
		if (nCurPage <= 0 || nCurPage > nPageCount) //无效页码
			continue;

		strData = makeupXml.GetChildAttrib(L"zustyle"); //获取属性
		int nX, nY, nWidth, nHeight, nPix;
		if (atoi(T2A(strData)) == 0) //定位点
		{
			makeupXml.IntoElem();
			strData = makeupXml.GetElemContent();
			bFind = makeupXml.FindChildElem(L"Area");
			makeupXml.IntoElem();
			bFind = makeupXml.FindChildElem(L"Area0");
			strData = makeupXml.GetChildAttrib(L"x");
			nX = atoi(T2A(strData));

			strData = makeupXml.GetChildAttrib(L"y");
			nY = atoi(T2A(strData));

			strData = makeupXml.GetChildAttrib(L"width");
			nWidth = atoi(T2A(strData));

			strData = makeupXml.GetChildAttrib(L"height");
			nHeight = atoi(T2A(strData));
			NEWRECT rect1;
			rect1.SetRect(nX, nY, nWidth, nHeight);
			rect1.nPage = nCurPage;

			if (nWidth*nHeight > 0)
				g_v_rectModelLocal.push_back(rect1); //有效 pany 2016.05.19
			makeupXml.OutOfElem();
			makeupXml.OutOfElem();
		}
		else if (atoi(T2A(strData)) == 7  || atoi(T2A(strData)) == 12) //客观题 单选 多选
		{
			int nSelType = atoi(T2A(strData));
			//存储组ID
			strData = makeupXml.GetChildAttrib(L"zuid");
			int nZuId = atoi(T2A(strData));
			int nABFlag = -1;
			
			if (nPaperTypeCount == 2) //AB
			{
				strData = makeupXml.GetChildAttrib(L"ABFlag");

				if (strData.Compare(L"A") == 0)
					nABFlag = 0;
				else if (strData.Compare(L"B") == 0)
					nABFlag = 1;
			}

			OMR_ZUID omrZuID;
			omrZuID.nID = nZuId;
			omrZuID.nPage = nCurPage;
			omrZuID.nABFlag = nABFlag;
			g_v_omrZuIDs.push_back(omrZuID);
			makeupXml.IntoElem();
			while( makeupXml.FindChildElem(L"Quesid"))
			{
				RECTARRAY rectArrayTmp; 
				int nAnswerNo;
				CString strCurTitle;
				strData = makeupXml.GetChildAttrib(L"val"); //对应题号
				nAnswerNo = atoi(T2A(strData));
				
				//新增 2016.06.17
				bFind = makeupXml.FindChildElem(L"Questitle");
				strData = makeupXml.GetChildAttrib(L"val");
				strCurTitle = strData;
			
				bFind = makeupXml.FindChildElem(L"Area");

				makeupXml.IntoElem();
				for (int i=0; i<26; i++)
				{
					CString strTmp;
					strTmp.Format(L"Area%d", i);
					bFind = makeupXml.FindChildElem(strTmp);
					if (!bFind)
						break;

					strData = makeupXml.GetChildAttrib(L"x");
					nX = atoi(T2A(strData));
					strData = makeupXml.GetChildAttrib(L"y");
					nY = atoi(T2A(strData));
					strData = makeupXml.GetChildAttrib(L"width");
					nWidth = atoi(T2A(strData));
					strData = makeupXml.GetChildAttrib(L"height");
					nHeight = atoi(T2A(strData));

					strData = makeupXml.GetChildAttrib(L"pix");
					nPix = atoi(T2A(strData));
					rectArrayTmp.SetRectArray(nX, nY, nWidth, nHeight, nAnswerNo, i);
					rectArrayTmp.rect.nPage = nCurPage;
					int nLen1 = strCurTitle.GetLength();
					wchar_t *wch= strCurTitle.GetBuffer(0); 
					int size=WideCharToMultiByte(CP_ACP,0,wch,-1,NULL,0,NULL,NULL); 
					if(!WideCharToMultiByte(CP_ACP,0,wch,-1,rectArrayTmp.cTitle,size,NULL,NULL)) 
					{ 
						return false; 
					}
					rectArrayTmp.nZuID = nZuId;
					rectArrayTmp.nABFlag = nABFlag;
					rectArrayTmp.nPix = nPix;
					rectArrayTmp.dDensity = double(nPix)/ double(max(nWidth*nHeight, 1));

					if (nSelType == 12)
						rectArrayTmp.nSelType = 0; //单选
					else 
						rectArrayTmp.nSelType = 1;
					g_v_modelRectArrays.push_back(rectArrayTmp);
				}
				makeupXml.OutOfElem();
			}
			makeupXml.OutOfElem();
		}
		else if (atoi(T2A(strData)) == 5) //考号 2016.04.08
		{
			strData = makeupXml.GetChildAttrib(L"zuid");
			int nZuId = atoi(T2A(strData));
			makeupXml.IntoElem();
			int nAnswerNo1 = 0;
			RECTARRAY rectArrayTmp; 

			while (makeupXml.FindChildElem(L"Area"))
			{
				makeupXml.IntoElem();
				for (int i=0; i<10; i++)
				{
					CString strTmp;
					strTmp.Format(L"Area%d", i);
					bFind = makeupXml.FindChildElem(strTmp);
					if (!bFind)
						return FALSE;

					strData = makeupXml.GetChildAttrib(L"x");
					nX = atoi(T2A(strData));
					strData = makeupXml.GetChildAttrib(L"y");
					nY = atoi(T2A(strData));
					strData = makeupXml.GetChildAttrib(L"width");
					nWidth = atoi(T2A(strData));
					strData = makeupXml.GetChildAttrib(L"height");
					nHeight = atoi(T2A(strData));
					rectArrayTmp.SetRectArray(nX, nY, nWidth, nHeight, nAnswerNo1, i);
					rectArrayTmp.rect.nPage = nCurPage;
					rectArrayTmp.nZuID = nZuId;
					rectArrayTmp.nABFlag = -1;
					g_v_modelKhRA.push_back(rectArrayTmp);

				}
				nAnswerNo1++;
				makeupXml.OutOfElem();
			}
			makeupXml.OutOfElem();
		}
		else if (atoi(T2A(strData)) == 14) //主观题分数框
		{
			strData = makeupXml.GetChildAttrib(L"zuid");
			int nZuId = atoi(T2A(strData));
			makeupXml.IntoElem();
			RECTARRAY rectArrayTmp; 
			bFind = makeupXml.FindChildElem(L"ScoreTitle");
			strData = makeupXml.GetChildAttrib(L"val");
			CString strScoreTitle = strData;
			
			if (makeupXml.FindChildElem(L"Area"))
			{
				makeupXml.IntoElem();
				for (int i=0; i<3; i++)
				{
					CString strTmp;
					strTmp.Format(L"Area%d", i);
					bFind = makeupXml.FindChildElem(strTmp);
					if (!bFind)
						return FALSE;

					strData = makeupXml.GetChildAttrib(L"x");
					nX = atoi(T2A(strData));
					strData = makeupXml.GetChildAttrib(L"y");
					nY = atoi(T2A(strData));
					strData = makeupXml.GetChildAttrib(L"width");
					nWidth = atoi(T2A(strData));
					strData = makeupXml.GetChildAttrib(L"height");
					nHeight = atoi(T2A(strData));
					rectArrayTmp.SetRectArray(nX, nY, nWidth, nHeight, 0, 0);
					rectArrayTmp.rect.nPage = nCurPage;
					rectArrayTmp.nZuID = nZuId;
					rectArrayTmp.nQuestionNo = nZuId;
					rectArrayTmp.nABFlag = -1;
					strcpy(rectArrayTmp.cTitle, T2A(strScoreTitle));
					g_v_modelScoreRA.push_back(rectArrayTmp);
				}
				makeupXml.OutOfElem();
			}
			makeupXml.OutOfElem();
		}
		else 
		{
			int nCurType = atoi(T2A(strData));
			strData = makeupXml.GetChildAttrib(L"zuStyleName");
			CString strStyleRes = strData;

			strData = makeupXml.GetChildAttrib(L"zuid");
			int nZuId = atoi(T2A(strData));
			makeupXml.IntoElem();
			RECTARRAY rectArrayTmp; 

			if (nCurType == 8)
			{
				//新增 2016.06.17
				bFind = makeupXml.FindChildElem(L"Questitle");
				strData = makeupXml.GetChildAttrib(L"val");
				strcpy(rectArrayTmp.cTitle, T2A(strData));
			}
			else if (nCurType == 15) //二维码
			{
				bFind = makeupXml.FindChildElem(L"QrCode");
				strData = makeupXml.GetChildAttrib(L"val");
				//strcpy(rectArrayTmp.cTitle, T2A(strData));
				strStyleRes = strData;
			}

			if (makeupXml.FindChildElem(L"Area"))
			{
				makeupXml.IntoElem();
				CString strTmp = _T("Area0");
				bFind = makeupXml.FindChildElem(strTmp);
				if (!bFind)
					return FALSE;

				strData = makeupXml.GetChildAttrib(L"x");
				nX = atoi(T2A(strData));
				strData = makeupXml.GetChildAttrib(L"y");
				nY = atoi(T2A(strData));
				strData = makeupXml.GetChildAttrib(L"width");
				nWidth = atoi(T2A(strData));
				strData = makeupXml.GetChildAttrib(L"height");
				nHeight = atoi(T2A(strData));
				strData = makeupXml.GetChildAttrib(L"pix");
				nPix = atoi(T2A(strData));
				rectArrayTmp.SetRectArray(nX, nY, nWidth, nHeight, 0, 0);
				rectArrayTmp.rect.nPage = nCurPage;
				rectArrayTmp.nZuID = nZuId;
				rectArrayTmp.nABFlag = -1;
				rectArrayTmp.nPix = nPix;

				if (nCurType == 8) //主观题 考虑拼题情况
					g_v_modelSubjectRA.push_back(rectArrayTmp);
				else if (nCurType == 6) //考号条码
					g_v_modelQRKh.push_back(rectArrayTmp);
				else if (nCurType == 15) //二维码
				{
					strcpy(rectArrayTmp.cTitle, T2A(strStyleRes));	
					if (nCurPage == 1) //首页 2016.11.09
						g_strModleQRTitle = strStyleRes;
					g_v_modelQRTitle.push_back(rectArrayTmp);
				}
				else if (nCurType == 4) //缺考标记  2016.01.04
				{
					if (nCurPage == 1) //首页
						g_RaAbsent = rectArrayTmp;
				}
				makeupXml.OutOfElem();
			}
			makeupXml.OutOfElem();
		}
	}
	makeupXml.OutOfElem();
	return TRUE;
}


extern "C" DLL_EXPORT bool ZYJ_IsComplete(const char *cFilePath, int nMinLen, int nAngleIndex)
{
	USES_CONVERSION;
	CString strFilePath = A2T(cFilePath);
	int nFileType = GetFileType1(strFilePath);

	CxImage srcImg;
	bool bRes = srcImg.Load(strFilePath, nFileType);
	srcImg.GrayScale();
	if (!bRes)
	{
		return FALSE;
	}

	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();
	
	int nSearchWidth = min(nImgWidth, nImgHeight)/4;
	if (nSearchWidth < 100 && nSearchWidth < min(nImgWidth, nImgHeight))
		nSearchWidth = 100;

	CRect rectAreas[4];
	rectAreas[0].SetRect(0, 0, nSearchWidth, nSearchWidth);
	rectAreas[1].SetRect(nImgWidth-1-nSearchWidth, 0, nImgWidth-1, nSearchWidth);
	rectAreas[2].SetRect(0, nImgHeight-1-nSearchWidth, nSearchWidth, nImgHeight-1);
	rectAreas[3].SetRect(nImgWidth-1-nSearchWidth, nImgHeight-1-nSearchWidth, nImgWidth-1, nImgHeight-1);

	CxImage *cropImg =  new CxImage();
	if (nAngleIndex < 0 || nAngleIndex > 3)
		nAngleIndex = 0;

	switch(nAngleIndex)
	{
		bRes = srcImg.Crop(rectAreas[nAngleIndex], cropImg);
		case  0: 
			break;
		case  1:
			cropImg->RotateLeft(cropImg);
			break;
		case  2:
			cropImg->RotateRight(cropImg);
			break;
		case  3:
			cropImg->Rotate180(cropImg);
			break;
		default:
			break;

	}
	if (!bRes)
	{
		delete cropImg;
		cropImg = NULL;
		return FALSE;
	}
	
	//统计当前黑点个数
	int nCount = 0;
	int nTop = 0;
	int nBottom = 0;
	int nRight = 0;
	bool bStart = TRUE;
	for (int n=0; n<=nSearchWidth-1; n++)
	{
		bool bContinue = FALSE;
		int nLen = 0;
		for (int m=0; m<=nSearchWidth-1; m++)
		{
			int nGrayValue = GETGRAYVALUE1(cropImg, m, n);
			if (nGrayValue == 0)
			{
				if (m > nRight)
					nRight = m;

				if (n>nBottom)
					nBottom = n;
				bContinue = TRUE;
				nCount++;
				m += 5;

				if (m >= nMinLen)
					break;
				//TRACE("m:%d\n", m);
			}
			else 
			{
				if (m == 0)
					bStart = FALSE;

				if (nLen > 0 && abs(nLen - m) > 50) //突变
				{
				   bStart = FALSE; //结束
				}
				nLen = m;
				bContinue = FALSE;
				break;
			}
		}
			
		if (!bStart)
			break;

		n+= 5;	

		if (n>=nMinLen)
			break;
	}
	delete cropImg;
	cropImg = NULL;
	double nLen = CImageProcess::GetLength(CPoint(0, nBottom), CPoint(nRight, 0));

	if (nLen >= nMinLen && nBottom >= 50 && nRight >= 50)
	{
	/*	double dDensity = double(nCount)/double(nBottom*nRight*0.5);
		if (int(dDensity*100.0) >= 80)*/
		return FALSE;
	}
	return TRUE;
}


int GetBlackColor(CxImage cxImage)    
{    
	long i;    
	int iBlackFlag = 0;    
	RGBQUAD *pData = cxImage.GetPalette();    
	long nPaletteSize = cxImage.GetPaletteSize()/sizeof(RGBQUAD);    
	for(i=0;i<nPaletteSize;i++)    
	{    
		if(pData[i].rgbBlue==0 && pData[i].rgbGreen==0 && pData[i].rgbRed==0)    
		{    
			iBlackFlag = i;    
			break;    
		}    
	}    
	return iBlackFlag;    
}   


extern "C" DLL_EXPORT bool ZYJ_Cximage2Iplimage(HANDLE hBmp, IplImage **dst)
{
	CxImage *src = new CxImage();
	bool bRet = src->CreateFromHANDLE(hBmp);
	
	if (!bRet)
	{
		delete src;
		src = NULL;
		return FALSE;
	}
	    
	if(!src || !src->IsValid())    
	{    
		delete src;
		src = NULL;
		bRet = false;    
		return bRet;    
	}    
	src->GrayScale();
	int nPalatteCount = src->GetPaletteSize()/sizeof(RGBQUAD);;    
	RGBQUAD *pPal = src->GetPalette();    
	int iBackColor = GetBlackColor(*src);    
	long i = 0,j = 0;    
	long nImageWidth = 0,nImageHeight = 0;    
	nImageWidth = src->GetWidth();    
	nImageHeight = src->GetHeight();    
	long nBitCunt = src->GetBpp();    
	if(nBitCunt<=1)    
	{    
		*dst = cvCreateImage(cvSize(nImageWidth, nImageHeight),IPL_DEPTH_8U,1);     
		cvZero(*dst);    
		//转换Cximage to IplImage     
		for(j=0;j<nImageHeight;j++)    
		{    
			for(i=0;i<nImageWidth;i++)    
			{    
				if(src->GetPixelIndex(i,j)==iBackColor)    
				{    
					CV_IMAGE_ELEM(*dst,uchar,nImageHeight-1-j,i) = 0;    
				}    
				else    
				{    
					CV_IMAGE_ELEM(*dst,uchar,nImageHeight-1-j,i) = 255;    
				}    
			}    
		}    
	}    
	else if(nBitCunt<=8)    
	{    
		*dst = cvCreateImage(cvSize(nImageWidth,nImageHeight),IPL_DEPTH_8U,1);     
		cvZero(*dst); 
		//src->Mirror();
		for (int i=0; i<nImageHeight; i++)
		{
			memcpy((*dst)->imageData+i*BYTESPERLINE(nImageWidth, 8), src->info.pImage+(nImageHeight-1-i)*BYTESPERLINE(nImageWidth, 8), BYTESPERLINE(nImageWidth, 8));
		}
	}    
	else if(nBitCunt<=16)    
	{    
		*dst = cvCreateImage(cvSize(nImageWidth,nImageHeight),IPL_DEPTH_16U,1);     
		(*dst)->origin = 1;//底—左结构 (Windows bitmaps 风格)      
		cvZero(*dst);    
		//转换Cximage to IplImage     
		for(j=0;j<nImageHeight;j++)    
		{    
			for(i=0;i<nImageWidth;i++)    
			{    
				BYTE *pSrc = src->GetBits(j) + 2*i;    
				CV_IMAGE_ELEM(*dst,ushort,j,i) = (*pSrc) + (*(pSrc+1))*256;    
			}    
		}    
	}    
	else //24色     
	{    
		//src->Mirror();
		*dst = cvCreateImage(cvSize(nImageWidth,nImageHeight),IPL_DEPTH_8U,3);     
		(*dst)->origin = 1;//底—左结构 (Windows bitmaps 风格)      
		cvZero(*dst);    
		//转换Cximage to IplImag     
		memcpy((*dst)->imageData, src->GetBits(0), src->GetSize());  
	}    
	delete src;
	src = NULL;
	return bRet;    

}


extern "C" DLL_EXPORT bool ZYJ_SaveIplImage(IplImage *src, const char *cPath)
{
	if (src == NULL)
		return FALSE;

	int nWidth = src->width;
	int nHeight = src->height;

	if (nWidth*nHeight <= 0)
		return FALSE;

	CxImage *dstImg =  new CxImage();
	dstImg->Create(nWidth, nHeight, src->depth, CXIMAGE_FORMAT_BMP);
	dstImg->SetGrayPalette(); 
	dstImg->GrayScale();

	 for (int i=0; i<nHeight; i++)
	 {
		 memcpy(dstImg->info.pImage+i*BYTESPERLINE(nWidth, 8), src->imageData+(nHeight-1-i)*BYTESPERLINE(nWidth, 8), BYTESPERLINE(nWidth, 8));   	
	 }
	 
	 USES_CONVERSION;
	 dstImg->GrayScale();
	 dstImg->SetCodecOption(7);
	 dstImg->SetJpegQuality(75);
	 dstImg->Save(A2T(cPath), CXIMAGE_FORMAT_JPG);
	 delete dstImg;
	 dstImg = NULL;
	 return TRUE;
}


extern "C" DLL_EXPORT bool ZYJ_GetImgEdge(const char *cPath, int nThreshold)
{
	CxImage *srcImg = new CxImage();
	USES_CONVERSION;
	int nFileType = GetFileType1(A2T(cPath));
	srcImg->Load(A2T(cPath), CXIMAGE_FORMAT_JPG);
	srcImg->GrayScale();
	srcImg->Threshold(nThreshold);
	srcImg->GrayScale();

	/*int *nValue = new int[srcImg->GetWidth()*srcImg->GetHeight()];
	int nWidth = srcImg->GetWidth();
	int nHeight = srcImg->GetHeight();

	for (int i=0; i<srcImg->GetHeight(); i++)
	{
	
		if (i==0 || i==srcImg->GetHeight()-1)
			continue;

		for (int j=0; j<srcImg->GetWidth(); j++)
		{
			if (GETGRAYVALUE1(srcImg, j, i) == 0)
			{
				nValue[i*srcImg->GetWidth() + j] = 0;
				if (j==0 || j==srcImg->GetWidth()-1)
					continue;

				//TRACE("i:%d, j:%d\n", i, j);


				if (GETGRAYVALUE1(srcImg,j,i-1) == 0 && GETGRAYVALUE1(srcImg,j-1, i) == 0 &&
					GETGRAYVALUE1(srcImg,j,i+1) == 0 && GETGRAYVALUE1(srcImg,j+1, i) == 0)
				{
						nValue[i*srcImg->GetWidth() + j] = 1;
				}
				else 
					nValue[i*srcImg->GetWidth() + j] = 0;

			}
			else
				nValue[i*srcImg->GetWidth() + j] = 1;

		}
	}

	for (int i=0; i<srcImg->GetHeight(); i++)
	{
		for (int j=0; j<srcImg->GetWidth(); j++)
		{
			if (nValue[i*srcImg->GetWidth() + j] == 1)
				SETGRAYVALUE(srcImg->info.pImage, srcImg->GetWidth(), srcImg->GetHeight(), j, i, 255);
			else 
				SETGRAYVALUE(srcImg->info.pImage, srcImg->GetWidth(), srcImg->GetHeight(), j, i, 0);
		}
	}*/

	//g_nIndex++;
	CString strDest;
	CRect cropRect;
	cropRect.SetRect(244, 386, 244+ 437, 386+105);
	srcImg->Crop(cropRect, srcImg);
	
	//计算图像在竖直方向上投影
	int nWidth = srcImg->GetWidth();
	int nHeight = srcImg->GetHeight();
	int *nHValue = new int[nHeight];
	bool bFind1 = false;
	for (int i=0; i<nHeight; i++)
	{
		nHValue[i] = 0;
		for (int j=0; j<nWidth; j++)
		{
			//统计当前横向
			if (GETGRAYVALUE1(srcImg, j, i) == 0)
			{
				nHValue[i]++;
			}	
		}

		if (abs(nHValue[i] - 355) <= nEPS)
		{
			
			//将其上下5个像素过滤干净
			for (int m=0; m<3; m++)
			{
				int nH1 = i-m;
				int nH2 = i+m;
				if (nH1 < 0)
					nH1 = 0;

				if (nH2 > nHeight-1)
					nH2 = nHeight-1;
			
				memset(srcImg->info.pImage + (nHeight-1-nH1)*BYTESPERLINE(nWidth, 8), 255, BYTESPERLINE(nWidth, 8));
				memset(srcImg->info.pImage + (nHeight-1-nH2)*BYTESPERLINE(nWidth, 8), 255, BYTESPERLINE(nWidth, 8));
				
			}
		}
		//TRACE("竖向黑点个数:%d, y：%d\n", nHValue[i], i);
	}
	
	int *nVValue = new int[nWidth];
	//计算在水平方向投影
	for (int i=0; i<nWidth; i++)
	{
		nVValue[i] = 0;
		for (int j=0; j<nHeight; j++)
		{
			if (GETGRAYVALUE1(srcImg, i, j) == 0)
			{
				nVValue[i]++;
			}
		}

		if (abs(nVValue[i] - 55) <= nEPS)
		{
			for (int m=0; m<3; m++)
			{
				int nV1 = i-m;
				int nV2 = i+m;
				if (nV1 < 0)
					nV1 = 0;

				if (nV2 > nWidth-1)
					nV2 = nWidth-1;

				for (int k=0; k<nHeight; k++)
				{
					memset(srcImg->info.pImage + (nHeight-1-k)*BYTESPERLINE(nWidth, 8) + nV1, 255, 1);
					memset(srcImg->info.pImage + (nHeight-1-k)*BYTESPERLINE(nWidth, 8) + nV2, 255, 1);
				}

			}
		}

		//TRACE("横向黑点个数:%d, x: %d\n", nVValue[i], i);
	}


	/*strDest.Format(L"c:\\test_%d.jpg", 0);
	srcImg->Save(strDest, CXIMAGE_FORMAT_JPG);*/
	delete srcImg;
	srcImg = NULL;

	delete []nHValue;
	nHValue = NULL;

	delete []nVValue;
	nVValue = NULL;
	//delete []nValue;
	//nValue = 0;

	return TRUE;
}

//自动获取图像四周定位点
extern "C" DLL_EXPORT bool ZYJ_GetAllImgLocalPoints(const char *cPath, int nThreshold, NEWRECT dstNewRect[4])
{
	CxImage *pSrcImg = new CxImage();
	bool bRes = false;
	USES_CONVERSION; 
	CString strPath = A2T(cPath);
	DWORD nFileType = GetFileType1(strPath);
	bRes = pSrcImg->Load(strPath, nFileType);

	if (!bRes)
	{
		delete pSrcImg;
		pSrcImg = NULL;
		return false;
	}

	pSrcImg->GrayScale();
	int nWidth = pSrcImg->GetWidth();
	int nHeight = pSrcImg->GetHeight();

	int nAreaWidth = max(nWidth/4, 1);
	int nAreaHeight = max(nHeight/4, 1);
	
	int nCount = 0;
	CImageProcess imgProcsessTmp;
	CRect rectAreas[4];
	rectAreas[0].SetRect(0, 0, nAreaWidth-1, nAreaHeight-1);
	rectAreas[1].SetRect(nWidth-1-nAreaWidth+1, 0, nWidth-1, nAreaHeight-1);
	rectAreas[2].SetRect(0, nHeight-1-nAreaHeight+1, nWidth-1, nHeight-1);
	rectAreas[3].SetRect(nWidth-1-nAreaWidth+1, nHeight-1-nAreaHeight+1, nWidth-1, nHeight-1);

	for (int i=0; i<4; i++)
	{
		CRect dstRectTmp;
		CxImage imgCrop;
		pSrcImg->Crop(rectAreas[i], &imgCrop);
		CRect rect1;
		rect1.SetRect(0, 0, rectAreas[i].Width()-1, rectAreas[i].Height()-1);
		bRes = imgProcsessTmp.GetRectPoint(dstRectTmp, pSrcImg, i, rect1, &imgCrop, nThreshold, CSize(20, 20), CSize(100, 80));
		
		if (bRes)
		{
			CPoint pStart1 = dstRectTmp.TopLeft();
			CPoint pEnd1 = dstRectTmp.BottomRight();
			pStart1.x += rectAreas[i].TopLeft().x;
			pStart1.y += rectAreas[i].TopLeft().y;
			pEnd1.x += rectAreas[i].TopLeft().x;
			pEnd1.y += rectAreas[i].TopLeft().y;
			dstRectTmp.SetRect(pStart1, pEnd1);
			//TRACE("x:%d, y:%d, w:%d, h:%d\n", dstRectTmp.TopLeft().x, dstRectTmp.TopLeft().y, dstRectTmp.Width(), dstRectTmp.Height());
			
			CPoint pStartTmp, pBottomRight;
			if (i == 0)
			{
				pStartTmp = rectAreas[1].TopLeft();
				pStartTmp.y = dstRectTmp.TopLeft().y - 50;
				pBottomRight = rectAreas[1].BottomRight();
				pBottomRight.y = dstRectTmp.BottomRight().y + 50;
				rectAreas[1].SetRect(pStartTmp, pBottomRight);
			
				pStartTmp = rectAreas[2].TopLeft();
				pStartTmp.x = dstRectTmp.TopLeft().x - 50;
				pBottomRight = rectAreas[2].BottomRight();
				pBottomRight.x = dstRectTmp.BottomRight().x + 50;
				rectAreas[2].SetRect(pStartTmp, pBottomRight);
			}
			else if (i == 1)
			{
				pStartTmp = rectAreas[3].TopLeft();
				pStartTmp.x = dstRectTmp.TopLeft().x -50;
				pBottomRight = rectAreas[3].BottomRight();
				pBottomRight.x = dstRectTmp.BottomRight().x + 50;
				rectAreas[3].SetRect(pStartTmp, pBottomRight);
			}
			else if (i == 2)
			{
				CPoint pStartTmp, pBottomRight;
				pStartTmp = rectAreas[3].TopLeft();
				pStartTmp.y = dstRectTmp.TopLeft().y -50;
				pBottomRight = rectAreas[2].BottomRight();
				pBottomRight.y = dstRectTmp.BottomRight().y + 50;
				rectAreas[2].SetRect(pStartTmp, pBottomRight);
			}

			nCount++;
		}
		else 
		{
			dstRectTmp.SetRect(0, 0, 0, 0);
		}

		dstNewRect[i] = Rect2NewRect(dstRectTmp);
	}

	if (nCount < 3)
	{
		delete pSrcImg;
		pSrcImg = NULL;
		return false;
	}

	delete pSrcImg;
	pSrcImg = NULL;
	return true;

}

extern "C" DLL_EXPORT bool ZYJ_SetImgCoordinateSystem(const char *cPath, NEWPOINT pStart, NEWRECT rectArea)
{
	CxImage *pSrcImg = new CxImage();
	bool bRes = false;
	USES_CONVERSION;
	DWORD nFileType = GetFileType1(A2T(cPath));

	bRes = pSrcImg->Load(A2T(cPath), nFileType);
	if (!bRes)
	{
		delete pSrcImg;
		pSrcImg = NULL;
		return false;
	}
	
	pSrcImg->GrayScale();
	int nWidth = pSrcImg->GetWidth();
	int nHeight = pSrcImg->GetHeight();

	CxImage newImg;
	newImg.Create(2*nWidth, 2*nHeight, 8, CXIMAGE_FORMAT_BMP);
	newImg.SetGrayPalette();
	newImg.GrayScale();
	memset(newImg.info.pImage, 255, BYTESPERLINE(2*nWidth, 8)*2*nHeight);

	for (int i=nHeight/2; i<3*nHeight/2; i++)
	{
		memcpy(newImg.info.pImage + (2*nHeight - 1 - i)*BYTESPERLINE(2*nWidth, 8) + nWidth/2,
			pSrcImg->info.pImage + (nHeight-1-(i-nHeight/2))*BYTESPERLINE(nWidth, 8), BYTESPERLINE(nWidth, 8));
	}

	CRect rectArea1;
	rectArea1 = NewRect2Rect(rectArea);

	CPoint pStartNew;
	pStartNew.SetPoint(rectArea1.TopLeft().x - pStart.nX + nWidth/2, rectArea1.TopLeft().y - pStart.nY + nHeight/2);
	CRect rectArea2;
	rectArea2.SetRect(pStartNew.x, pStartNew.y, pStartNew.x + nWidth, pStartNew.y + nHeight);
	
	CxImage dstImg;
	newImg.Crop(rectArea2, &dstImg);
	dstImg.GrayScale();
	dstImg.SetCodecOption(7);
	dstImg.SetJpegQuality(75);
	dstImg.Save(A2T(cPath), nFileType);
	delete pSrcImg;
	pSrcImg = NULL;

	return true;
}

extern "C" DLL_EXPORT bool ZYJ_GetImgSize(const char *cPath,  int &nImgWidth, int &nImgHeight)
{
	CxImage srcImg;
	USES_CONVERSION;
	CString strPath = A2T(cPath);
	int nFileType = GetFileType1(strPath);
	bool bRes = srcImg.Load(strPath, nFileType);

	if (!bRes)
		return false;

	srcImg.GrayScale();
	nImgWidth = srcImg.GetWidth();
	nImgHeight = srcImg.GetHeight();
	return TRUE;
}

//static int g_index= 0;
//识别所有分数选项 2016.07.27

extern "C" DLL_EXPORT bool ZYJ_GetIcrRect(const char *cPath, NEWRECT rectArea,  int nThreshold, int nMinWidth, int nMinHeight, NEWRECT dstRect[256], int &nRectCount)
{
	CxImage srcImg;
	USES_CONVERSION;
	CString strPath = A2T(cPath);
	int nFileType = GetFileType1(strPath);
	bool bRes = srcImg.Load(strPath, nFileType);

	if (!bRes)
		return false;

	srcImg.GrayScale();
	CRect rect1 = NewRect2Rect(rectArea);
	CxImage cropImage;
	srcImg.Crop(rect1, &cropImage);
	CImageProcess imgProcess;
	CSize minSize(nMinWidth, nMinHeight);
	vector<CRect> v_dstRects;
	imgProcess.GetMinRect(cropImage, minSize,  nThreshold, v_dstRects);

	if (v_dstRects.size() > 256)
		return false;

	if (v_dstRects.size() == 0)
		return false;

	if (v_dstRects.size() != 3)
		return false;

	nRectCount = v_dstRects.size();
	int i=0;
	for (vector<CRect>::iterator it = v_dstRects.begin(); it != v_dstRects.end(); it++)
	{
		dstRect[i] = Rect2NewRect(*it);
		dstRect[i].AddPoint(rectArea.pTopLeft);
		i++;
	}
	
	return true;
}

extern "C" DLL_EXPORT bool ZYJ_GetQRKh(const char *cPath, int nPage, char cResKh[36])
{
	memset(cResKh, '\0', 36);

	CxImage srcImg;
	USES_CONVERSION;
	int nFileType = GetFileType1(A2T(cPath));
	bool bRes = srcImg.Load(A2T(cPath), nFileType);
	if (!bRes)
		return false;

	srcImg.GrayScale();
	CurImage curImgPara;
	int nRectType = g_nLocalType;
	curImgPara.v_rectModelLocal.clear();
	curImgPara.v_modelRectArrays.clear();
	curImgPara.v_OMRZuIDs.clear();
	curImgPara.v_modelKhRectArray.clear();
	curImgPara.m_rectLocal.clear();

	for (int i=0; i<g_v_rectModelLocal.size(); i++)
	{
		if (g_v_rectModelLocal[i].nPage == nPage)
		{
			curImgPara.v_rectModelLocal.push_back(g_v_rectModelLocal[i]);
		}
	}

	for (int i=0; i<g_v_modelQRKh.size(); i++)
	{
		if (g_v_modelQRKh[i].rect.nPage == nPage)
		{
			curImgPara.v_modelKhQRRectArray.push_back(g_v_modelQRKh[i]);
		}
	}

	if (curImgPara.v_rectModelLocal.size() < 3)
		return FALSE;

	if (curImgPara.v_modelKhQRRectArray.size() != 1)
		return FALSE;


	CRect rectLocals[3];
	double dLenth[3] = {0.0, 0.0, 0.0};
	for (int i=0; i<3; i++)
	{
		rectLocals[i] = NewRect2Rect(curImgPara.v_rectModelLocal[i]);
	}
	dLenth[0] = CImageProcess::GetLength(rectLocals[0].CenterPoint(), rectLocals[1].CenterPoint());
	dLenth[1] = CImageProcess::GetLength(rectLocals[0].CenterPoint(), rectLocals[2].CenterPoint());
	dLenth[2] = CImageProcess::GetLength(rectLocals[1].CenterPoint(), rectLocals[2].CenterPoint());

	for (int i=0; i<3; i++)
	{
		if (abs(max(max(dLenth[0],dLenth[1]), dLenth[2]) - dLenth[i]) <= 0.00000001) //判断斜边选
		{
			if (i == 0)
			{
				NEWRECT modelTmps = curImgPara.v_rectModelLocal[0];
				curImgPara.v_rectModelLocal[0] = curImgPara.v_rectModelLocal[2];
				curImgPara.v_rectModelLocal[2] = modelTmps;
			}
			else if (i==1)
			{
				NEWRECT modelTmps = curImgPara.v_rectModelLocal[0];
				curImgPara.v_rectModelLocal[0] = curImgPara.v_rectModelLocal[1];
				curImgPara.v_rectModelLocal[1] = modelTmps;
			}
		}
	}

	//g_mute.Lock();
	bool bFindLocal = false;
	//for (vector<LOCALPOINT_DATA>::iterator it = g_v_LocalPointData.begin(); 
	//	it != g_v_LocalPointData.end(); it++)
	//{
	//	LOCALPOINT_DATA localPointData = *it;
	//	if (strcmp(localPointData.cCurImgPath, cPath) == 0)
	//	{
	//		curImgPara.m_rectLocal.clear();
	//		for (int i=0; i<3; i++)
	//			curImgPara.m_rectLocal.push_back(localPointData.pLocalRects[i]);
	//		
	//		break;
	//	}
	//}
	//g_mute.Unlock();

	//获取三个角定位点相关
	if (!bFindLocal)
	{
		for (int i=0; i<3; i++)
		{
			NEWRECT rectArea1;
			NEWRECT dstRect;
			rectArea1 = GetRectArea1(curImgPara.v_rectModelLocal[i], &srcImg);
			int nDirIndex = 0;
			if (rectArea1.CenterNewPoint().nX > srcImg.GetWidth()/2 && rectArea1.CenterNewPoint().nY < srcImg.GetHeight()/2)
				nDirIndex = 1;
			else if (rectArea1.CenterNewPoint().nX < srcImg.GetWidth()/2 && rectArea1.CenterNewPoint().nY > srcImg.GetHeight()/2)
				nDirIndex = 2;
			else if (rectArea1.CenterNewPoint().nX > srcImg.GetWidth()/2 && rectArea1.CenterNewPoint().nY > srcImg.GetHeight()/2)
				nDirIndex = 3;

			bRes = ZYJ_GetRectLocalPoint(rectArea1, curImgPara.v_rectModelLocal[i].GetWidth(), curImgPara.v_rectModelLocal[i].GetHeight(),
				nDirIndex, nRectType, dstRect, &srcImg, &curImgPara);

			if (!bRes)
			{
				return FALSE;
			}
		}

		//g_mute.Lock();
		//if (g_v_LocalPointData.size() >= 20)
		//	g_v_LocalPointData.erase(g_v_LocalPointData.begin());


		//LOCALPOINT_DATA localPointData;
		//strcpy(localPointData.cCurImgPath, cPath);
		//for (int i=0; i<3; i++)
		//	localPointData.pLocalRects[i] = curImgPara.m_rectLocal[i];
		////g_v_LocalPointData.push_back(localPointData);
		//g_mute.Unlock();

	}

	NEWPOINT pLocalModel[3] = {curImgPara.v_rectModelLocal[0].CenterNewPoint(),
		curImgPara.v_rectModelLocal[1].CenterNewPoint(), 
		curImgPara.v_rectModelLocal[2].CenterNewPoint()};
	NEWPOINT pLocalCur[3] =  {curImgPara.m_rectLocal[0].CenterNewPoint(),
		curImgPara.m_rectLocal[1].CenterNewPoint(), 
		curImgPara.m_rectLocal[2].CenterNewPoint()};

	//获取三个矩形框所在区域
	RECTARRAY rArray1 = curImgPara.v_modelKhQRRectArray[0];
	CSize dstSize;
	CImageProcess imgProcessTmp;
	double dHRatio, dVRatio;
	NEWPOINT p1, p2;
	imgProcessTmp.GetRatio(pLocalModel, rArray1.rect.pTopLeft,dHRatio, dVRatio);
	imgProcessTmp.GetNewPoint(pLocalCur, dHRatio, dVRatio, p1);
	imgProcessTmp.GetRatio(pLocalModel, rArray1.rect.pBottomRight,dHRatio, dVRatio);
	imgProcessTmp.GetNewPoint(pLocalCur, dHRatio, dVRatio, p2);
	NEWRECT rectArea2; //区域外扩矩形
	p1.nX -= 20;
	p1.nY -= 20;
	p2.nX += 20;
	p2.nY += 20;
	rectArea2.SetRect(p1, p2);

	CxImage cropImg;
	srcImg.Crop(NewRect2Rect(rectArea2), &cropImg);
	IplImage *dst = cvCreateImage(cvSize(cropImg.GetWidth(),cropImg.GetHeight()),IPL_DEPTH_8U,1); 
	cvZero(dst); 
	for (int i=0; i<cropImg.GetHeight(); i++)
	{
		memcpy(dst->imageData+i*BYTESPERLINE(cropImg.GetWidth(), 8), cropImg.info.pImage+(cropImg.GetHeight()-1-i)*BYTESPERLINE(cropImg.GetWidth(), 8), BYTESPERLINE(cropImg.GetWidth(), 8));
	}

	CvMat temp;
	CvMat* mat = cvGetMat(dst, &temp);    //深拷贝
	CvMatrix matrix(mat);
	string strRes;
	int nLib;
	CImgRecQR imgRecQR;
	imgRecQR.decode_image(matrix, strRes, nLib);
	memcpy(cResKh, strRes.c_str(), strRes.length());
	matrix.release();
	cvReleaseImage(&dst);
	dst = NULL;
	ImageReaderSource::ReleaseBuffer();

	return true;
}

extern "C" DLL_EXPORT bool ZYJ_GetQRCodeRes(const char *cPath, NEWRECT rectArea, char *cRes)
{
	CImgRecQR imgRecQR; //二维码识别相关
	USES_CONVERSION;
	CString strPath = A2T(cPath);
	CvMatrix matImage;
	CxImage srcImg1, srcImg;
	int nFileType = GetFileType1(strPath);
	srcImg1.Load(strPath, nFileType);
	srcImg1.GrayScale(); 
	long i = 0,j = 0;    
	long nImageWidth = 0,nImageHeight = 0; 
	CRect rect1;
	rect1 = NewRect2Rect(rectArea);
	srcImg1.Crop(rect1, &srcImg);
	srcImg.GrayScale();
	nImageWidth = srcImg.GetWidth();    
	nImageHeight = srcImg.GetHeight();    

	IplImage *dst = cvCreateImage(cvSize(nImageWidth,nImageHeight),IPL_DEPTH_8U,1);     
	cvZero(dst); 
	for (int i=0; i<nImageHeight; i++)
	{
		memcpy(dst->imageData+i*BYTESPERLINE(nImageWidth, 8), srcImg.info.pImage+(nImageHeight-1-i)*BYTESPERLINE(nImageWidth, 8), BYTESPERLINE(nImageWidth, 8));
	}

	CvMat temp;
	CvMat* mat = cvGetMat(dst, &temp);    //深拷贝
	CvMatrix matrix(mat);
	string strRes;
	int nLib;
	imgRecQR.decode_image(matrix, strRes, nLib);
	memcpy(cRes, strRes.c_str(), strRes.length());
	matrix.release();
	cvReleaseImage(&dst);
	dst = NULL;
	ImageReaderSource::ReleaseBuffer();

	return true;
}

extern "C" DLL_EXPORT bool ZYJ_GetExamTitle(const char *cPath, int nPage, char cResTitle[256])
{
	CxImage *srcImg = new CxImage();
	bool bRes = false;
	int nFileType = CXIMAGE_FORMAT_UNKNOWN;
	USES_CONVERSION;
	CString strFilePath = A2T(cPath);
	nFileType = GetFileType1(strFilePath);
	bRes = srcImg->Load(strFilePath, nFileType);
	if (!bRes)
	{
		delete srcImg;
		srcImg = NULL;
		return FALSE;
	}
	srcImg->GrayScale();
	CurImage curImgPara;
	if (!GetCurImgInfo(srcImg, cPath, nPage, 0, DEFAULT_THRESHOLD, -1, -1, curImgPara))
	{
		delete srcImg;
		srcImg = NULL;
		return FALSE;
	}

	NEWPOINT pLocalModel[3] = { curImgPara.v_rectModelLocal[0].CenterNewPoint(),
		curImgPara.v_rectModelLocal[1].CenterNewPoint(),
		curImgPara.v_rectModelLocal[2].CenterNewPoint() };
	NEWPOINT pLocalCur[3] = { curImgPara.m_rectLocal[0].CenterNewPoint(),
		curImgPara.m_rectLocal[1].CenterNewPoint(),
		curImgPara.m_rectLocal[2].CenterNewPoint() };

	//获取三个矩形框所在区域
	RECTARRAY rArray1 = curImgPara.v_modelTitleQRRectArray[0];
	CSize dstSize;
	CImageProcess imgProcessTmp;
	double dHRatio, dVRatio;
	NEWPOINT p1, p2;
	imgProcessTmp.GetRatio(pLocalModel, rArray1.rect.pTopLeft,dHRatio, dVRatio);
	imgProcessTmp.GetNewPoint(pLocalCur, dHRatio, dVRatio, p1);
	imgProcessTmp.GetRatio(pLocalModel, rArray1.rect.pBottomRight,dHRatio, dVRatio);
	imgProcessTmp.GetNewPoint(pLocalCur, dHRatio, dVRatio, p2);
	NEWRECT rectArea2; //区域外扩矩形
	p1.nX -= 20;
	p1.nY -= 20;
	p2.nX += 20;
	p2.nY += 20;
	rectArea2.SetRect(p1, p2);

	CxImage cropImg;
	srcImg->Crop(NewRect2Rect(rectArea2), &cropImg);
	IplImage *dst = cvCreateImage(cvSize(cropImg.GetWidth(),cropImg.GetHeight()),IPL_DEPTH_8U,1); 
	cvZero(dst); 
	for (int i=0; i<cropImg.GetHeight(); i++)
	{
		memcpy(dst->imageData+i*BYTESPERLINE(cropImg.GetWidth(), 8), cropImg.info.pImage+(cropImg.GetHeight()-1-i)*BYTESPERLINE(cropImg.GetWidth(), 8), BYTESPERLINE(cropImg.GetWidth(), 8));
	}

	CvMat temp;
	CvMat* mat = cvGetMat(dst, &temp); //深拷贝
	CvMatrix matrix(mat);
	string strRes;
	int nLib;
	CImgRecQR imgRecQR;
	imgRecQR.decode_image(matrix, strRes, nLib);
	memcpy(cResTitle, strRes.c_str(), strRes.length());
	matrix.release();
	cvReleaseImage(&dst);
	dst = NULL;
	ImageReaderSource::ReleaseBuffer();
	delete srcImg;
	srcImg = NULL;
	return true;
}

extern "C" DLL_EXPORT bool ZYJ_GetTemplatePage(int &nPage)
{
	nPage =  g_nModelPage;
	return true;
}

//获取模板中二维码
extern "C" DLL_EXPORT bool ZYJ_GetTemplateQRTitle(char cRes[256])
{
	memset(cRes, '\0', 256);
	USES_CONVERSION;
	strcpy(cRes, T2A( g_strModleQRTitle));
	return true;
}

//获取模板中模板版本号 2016.08.08
extern "C" DLL_EXPORT bool ZYJ_GetTemplateVersion(char cRes[256])
{
	memset(cRes, '\0', 256);
	USES_CONVERSION;
	strcpy(cRes, T2A( g_strModelVersion));
	return true;
}

extern "C" DLL_EXPORT bool ZYJ_IsNeedOmr(int nPage)
{
	int nCount = 0;
	for (int i=0; i< g_v_modelRectArrays.size(); i++)
	{
		if ( g_v_modelRectArrays[i].rect.nPage == nPage )
		{
			nCount++;
		}
	}

	if (nCount > 0)
		return TRUE;

	return FALSE;

}


extern "C" DLL_EXPORT bool ZYJ_IsNeedRecScore(int nPage)
{
	int nCount = 0;
	for (int i=0; i< g_v_modelScoreRA.size(); i++)
	{
		if ( g_v_modelScoreRA[i].rect.nPage == nPage )
		{
			nCount++;
		}
	}

	if (nCount > 0)
		return TRUE;

	return FALSE;
}

extern "C" DLL_EXPORT bool ZYJ_GetTitleFromID(int nID, char cTitleRes[256])
{
	//客观题
	for (vector<RECTARRAY>::iterator it =  g_v_modelRectArrays.begin();
         it !=  g_v_modelRectArrays.end(); it++) 
	{
		RECTARRAY rectArrayTmp = *it;
		if (rectArrayTmp.nQuestionNo == nID)
		{
			strcpy(cTitleRes, rectArrayTmp.cTitle);
			return TRUE;
		}
	}
	
	//主观题
	for (vector<RECTARRAY>::iterator it = g_v_modelScoreRA.begin();
		  it !=  g_v_modelScoreRA.end(); it++)
	{
		RECTARRAY rectArrayTmp = *it;
		if (rectArrayTmp.nQuestionNo == nID)
		{
			strcpy(cTitleRes, rectArrayTmp.cTitle);
			return TRUE;
		}
	}

	for (vector<RECTARRAY>::iterator it = g_v_modelSubjectRA.begin();
		it !=  g_v_modelSubjectRA.end(); it++)
	{
		RECTARRAY rectArrayTmp = *it;
		if (rectArrayTmp.nZuID == nID)
		{
			strcpy(cTitleRes, rectArrayTmp.cTitle);
			return TRUE;
		}
	}


	return TRUE;
}

extern "C" DLL_EXPORT bool ZYJ_GetRectsFromID(int nID, NEWRECT rectFirst, NEWRECT *rects, int &nRectSize)
{
	bool bFind = FALSE;
	int nCount = 0;
	NEWRECT rectStartTmp;

	//TRACE("客观题数量:%d\n", g_v_modelRectArrays.size());
	//客观题
	for (vector<RECTARRAY>::iterator it =  g_v_modelRectArrays.begin();
		it !=  g_v_modelRectArrays.end(); it++)
	{
		RECTARRAY rectArrayTmp = *it;
		if (rectArrayTmp.nQuestionNo == nID)
		{
			nCount++;
			if (!bFind)
			{
				bFind = TRUE;
				rectStartTmp = rectArrayTmp.rect;
				rects[0] = rectFirst;
				
			}
			else
			{
				NEWPOINT pAdd;
				pAdd.setPoint(rectArrayTmp.CenterNewPoint().nX - rectStartTmp.CenterNewPoint().nX, 
					rectArrayTmp.CenterNewPoint().nY - rectStartTmp.CenterNewPoint().nY);
				rects[nCount-1] = rectFirst;
				rects[nCount-1].AddPoint(pAdd);
			}

		}
		else 
		{
			if (bFind)
			{
				nRectSize = nCount;
				return TRUE;
			}
		}
	}

	if (bFind)
	{
		nRectSize = nCount;
		return TRUE;
	}
	
	//主观题分数
	for (vector<RECTARRAY>::iterator it =  g_v_modelScoreRA.begin();
		it !=  g_v_modelScoreRA.end(); it++)
	{
		RECTARRAY rectArrayTmp = *it;
		if (rectArrayTmp.nQuestionNo == nID)
		{
			nCount++;
			if (!bFind)
			{
				bFind = TRUE;
				rectStartTmp = rectArrayTmp.rect;
				rects[0] = rectFirst;
			}
			else
			{
				NEWPOINT pAdd;
				pAdd.setPoint(rectArrayTmp.CenterNewPoint().nX - rectStartTmp.CenterNewPoint().nX, 
					rectArrayTmp.CenterNewPoint().nY - rectStartTmp.CenterNewPoint().nY);
				rects[nCount-1] = rectFirst;
				rects[nCount-1].AddPoint(pAdd);
			}
		}
		else 
		{
			if (bFind)
			{
				nRectSize = nCount;
				return TRUE;
			}
		}
	}

	if (bFind)
	{
		nRectSize = nCount;
		return TRUE;
	}

	return FALSE;
}

//根据ID号判断题型 2016.08.16
//nQuestionType:返回0客观题  返回1主观题分数 
extern "C" DLL_EXPORT bool ZYJ_GetQuestionTypeFromID(int nID, int &nQuestionType)
{
	nQuestionType = -1;

	//客观题
	for (vector<RECTARRAY>::iterator it =  g_v_modelRectArrays.begin();
		it !=  g_v_modelRectArrays.end(); it++) 
	{
		RECTARRAY rectArrayTmp = *it;
		if (rectArrayTmp.nQuestionNo == nID)
		{
			nQuestionType = 0;
			return TRUE;
		}
	}

	//主观题分数
	for (vector<RECTARRAY>::iterator it =  g_v_modelScoreRA.begin();
		it !=  g_v_modelScoreRA.end(); it++)
	{
		RECTARRAY rectArrayTmp = *it;
		if (rectArrayTmp.nQuestionNo == nID)
		{
			nQuestionType = 1;
			return TRUE;
		}
	}

	return FALSE;
}

//根据ID号获取当前ID所在第几页
extern "C" bool ZYJ_GetPageFromID(int nID, int &nPageIndex)
{

	nPageIndex = -1;

	//客观题
	for (vector<RECTARRAY>::iterator it =  g_v_modelRectArrays.begin();
		it !=  g_v_modelRectArrays.end(); it++) 
	{
		RECTARRAY rectArrayTmp = *it;
		if (rectArrayTmp.nQuestionNo == nID)
		{
			nPageIndex = rectArrayTmp.rect.nPage;
			return TRUE;
		}
	}

	//主观题分数
	for (vector<RECTARRAY>::iterator it =  g_v_modelScoreRA.begin();
		it !=  g_v_modelScoreRA.end(); it++)
	{
		RECTARRAY rectArrayTmp = *it;
		if (rectArrayTmp.nQuestionNo == nID)
		{
			nPageIndex = rectArrayTmp.rect.nPage;
			return TRUE;
		}
	}

 
	return FALSE;
}

extern "C" DLL_EXPORT int ZYJ_GetElementSize(int nElementType)
{
	int nCount = 0;
	if (nElementType == TYPE_LOCATE_POINT)
		nCount =  g_v_rectModelLocal.size();
	else if(nElementType == TYPE_PAGE_NUM)
		nCount =  g_v_modelPage.size();
	else if (nElementType == TYPE_EXAM_NUM)
		nCount =  g_v_modelKhRA.size();
	else if (nElementType == TYPE_EXAM_NUM_BAR_CODE)
		nCount =  g_v_modelQRKh.size();
	else if (nElementType == TYPE_CHOICE_ITEM || TYPE_CHOICE_ITEM_SINGLE == nElementType)
		nCount =  g_v_modelRectArrays.size();
	else if (nElementType == TYPE_SUBJECTIVE_ITEM)
		nCount =  g_v_modelSubjectRA.size();
	else if (nElementType == TYPE_SCORE_ITEM)
		nCount =  g_v_modelScoreRA.size();
	else if (nElementType == TYPE_TITLE_QRCODE_ITEM)
		nCount =  g_v_modelQRTitle.size();

	return nCount;
}

//获取元素值
extern "C" DLL_EXPORT bool ZYJ_GetElement(RECTARRAY *dstRectArray, int nElementType)
{
	int nCount= 0;
	if (nElementType == TYPE_LOCATE_POINT)
	{
		for (vector<NEWRECT>::iterator it =  g_v_rectModelLocal.begin(); 
			it !=  g_v_rectModelLocal.end();
			it++)
		{

		}
	}
	else if(nElementType == TYPE_PAGE_NUM)
		nCount =  g_v_modelPage.size();
	else if (nElementType == TYPE_EXAM_NUM)
		nCount =  g_v_modelKhRA.size();
	else if (nElementType == TYPE_EXAM_NUM_BAR_CODE)
		nCount =  g_v_modelQRKh.size();
	else if (nElementType == TYPE_CHOICE_ITEM || TYPE_CHOICE_ITEM_SINGLE == nElementType)
		nCount =  g_v_modelRectArrays.size();
	else if (nElementType == TYPE_SUBJECTIVE_ITEM)
		nCount =  g_v_modelSubjectRA.size();
	else if (nElementType == TYPE_SCORE_ITEM)
		nCount =  g_v_modelScoreRA.size();
	else if (nElementType == TYPE_TITLE_QRCODE_ITEM)
		nCount =  g_v_modelQRTitle.size();

	return TRUE;
}


//获取当前图像所在页号
extern "C" DLL_EXPORT bool ZYJ_CheckPage(const char *cPath, int nCurPage)
{
	CxImage *srcImg = new CxImage();
	bool bRes = false;
	int nFileType = CXIMAGE_FORMAT_UNKNOWN;
	USES_CONVERSION;
	CString strFilePath = A2T(cPath);
	nFileType = GetFileType1(strFilePath);
	bRes = srcImg->Load(strFilePath, nFileType);
	if (!bRes)
	{
        if (srcImg != NULL)
        {
            delete srcImg;
            srcImg = NULL;
        }
		return FALSE;
	}
	srcImg->GrayScale();
	CurImage curImgPara;
	if (!GetCurImgInfo(srcImg, cPath, nCurPage, 0, DEFAULT_THRESHOLD, -1, -1, curImgPara))
	{
        if (srcImg != NULL)
        {
            delete srcImg;
            srcImg = NULL;
        }
		return FALSE;
	}


	NEWPOINT pLocalModel[3] = {curImgPara.v_rectModelLocal[0].CenterNewPoint(),
		curImgPara.v_rectModelLocal[1].CenterNewPoint(), 
		curImgPara.v_rectModelLocal[2].CenterNewPoint()};
	NEWPOINT pLocalCur[3] =  {curImgPara.m_rectLocal[0].CenterNewPoint(),
		curImgPara.m_rectLocal[1].CenterNewPoint(), 
		curImgPara.m_rectLocal[2].CenterNewPoint()};

	//获取三个矩形框所在区域
	int nPageSize = curImgPara.v_modelPageRectArray.size();
	RECTARRAY rArray1 = curImgPara.v_modelPageRectArray[0];
	RECTARRAY rArray2 = curImgPara.v_modelScoreRectArray[nPageSize-1];
	//RECTARRAY rArray3 = curImgPara.v_modelPageRectArray[nPageSize-1];
	CSize dstSize;
	CImageProcess imgProcessTmp;
	double dHRatio, dVRatio;
	NEWPOINT p1, p2;
	imgProcessTmp.GetRatio(pLocalModel, rArray1.rect.pTopLeft,dHRatio, dVRatio);
	imgProcessTmp.GetNewPoint(pLocalCur, dHRatio, dVRatio, p1);
	
	imgProcessTmp.GetRatio(pLocalModel, rArray2.rect.pBottomRight,dHRatio, dVRatio);
	imgProcessTmp.GetNewPoint(pLocalCur, dHRatio, dVRatio, p2);
	NEWRECT rectArea2; //分数区域外扩矩形
	p1.nX -= 20;
	p1.nY -= 20;

	p2.nX += 20;
	p2.nY += 20;
	rectArea2.SetRect(p1, p2);
	CxImage cropImg;
	dstSize.cx = (rArray1.rect.GetWidth()+ rArray2.rect.GetWidth())/2;
	dstSize.cy = (rArray1.rect.GetHeight()+ rArray2.rect.GetHeight())/2;
	srcImg->Crop(NewRect2Rect(rectArea2), &cropImg);
	
	vector <CRect> v_allRects;
	//cropImg.Save(L"D:\\test.jpg", CXIMAGE_FORMAT_JPG);
	imgProcessTmp.GetEnclosedRects(0, &cropImg, v_allRects, DEFAULT_THRESHOLD, dstSize);
	if (v_allRects.size() != nPageSize)
	{
        if (srcImg != NULL)
        {
            delete srcImg;
            srcImg = NULL;
        }
		return FALSE;
	}

	int nIndex= 0;
	for (vector<CRect>::iterator it = v_allRects.begin(); it != v_allRects.end(); it++)
	{
		CRect rect1 = *it;
		double dDensity = imgProcessTmp.GetDensity(&cropImg, rect1, DEFAULT_THRESHOLD); //密度
		
		if (dDensity >= 0.7)
		{
			if (nIndex == nCurPage)
			{
                if (srcImg != NULL)
                {
                    delete srcImg;
                    srcImg = NULL;
                }
				return TRUE;
			}
		}
		
		nIndex++;
	}

    if (srcImg != NULL)
    {
        delete srcImg;
        srcImg = NULL;
    }
	return bRes;
}

//cSrcPath:   某考生所有原图路径 
//cDstFolder: 某考生子图目标文件目录 
extern "C" DLL_EXPORT bool ZYJ_CutImage(char cSrcPath[MAX_PAGE][256], const char *cDstFolder)
{
	int nIndex = 0;
	int nSize = g_v_modelSubjectRA.size();
	RECTARRAY rArrayLast;
	CxImage imgLast;
	CString strFolder;
	USES_CONVERSION;
	strFolder = A2T(cDstFolder);
	strFolder += _T("\\");
	int nLastPage = -1;
	CxImage srcImg;
	for(vector<RECTARRAY>::iterator it = g_v_modelSubjectRA.begin(); it!= g_v_modelSubjectRA.end(); it++)
	{
		RECTARRAY rArrayCur = *it;
		CRect rectArea = NewRect2Rect(rArrayCur.rect);
        CRect rectNewArea = rectArea;
		CxImage imgCur;
		int nPage = rArrayCur.rect.nPage;
		bool bRes = false;

		if (nLastPage != nPage) //加载原图
		{
			nLastPage = nPage;

			char cPath1[256];
			memset(cPath1, '\0', 256);
			strcpy(cPath1, cSrcPath[nPage-1]);
			CString strPath = A2T(cPath1);
			int nFileType = GetFileType1(strPath);
			bRes = srcImg.Load(strPath, nFileType);
			if (!bRes)
				return false;

			bRes = srcImg.GrayScale();
			if (!bRes)
				return false;


			//获取定位点 2017.01.12
			CurImage curImgPara;
			if (!GetCurImgInfo(&srcImg, cSrcPath[nPage - 1], nPage, 0, DEFAULT_THRESHOLD, -1, -1, curImgPara))
			{
				return FALSE;
			}

			CvMat* warp_mat = cvCreateMat(2, 3, CV_32FC1);
			CvPoint2D32f srcTri[3], dstTri[3];
			//计算变换矩阵
			for (int i = 0; i < 3; i++)
			{
				srcTri[i].x = curImgPara.m_rectLocal[i].CenterNewPoint().nX;
				srcTri[i].y = curImgPara.m_rectLocal[i].CenterNewPoint().nY;
				dstTri[i].x = curImgPara.v_rectModelLocal[i].CenterNewPoint().nX;
				dstTri[i].y = curImgPara.v_rectModelLocal[i].CenterNewPoint().nY;
			}
			cvGetAffineTransform(dstTri, srcTri,warp_mat);

            double dTmps[6];
            for (int i = 0; i < 6; i++)
            {
                dTmps[i] = warp_mat->data.fl[(i/3) * 3 + i%3];
            }
          
			//调用函数cvWarpAffine（）
            GetCropRect(rectArea, dTmps, rectNewArea);
			cvReleaseMat(&warp_mat);
		}
       
		srcImg.Crop(rectNewArea, &imgCur);
		
		if (rArrayLast.nZuID != rArrayCur.nZuID)
		{
				if (rArrayLast.nZuID != -1)
				{
					CString strPath;
					strPath = A2T(rArrayLast.cTitle);
					strPath += _T(".jpg");
					strPath = strFolder + strPath;
					imgLast.Save(strPath, CXIMAGE_FORMAT_JPG);
					//TRACE("%s\n", T2A(strPath));
				}
		}
		else  //合并图像
		{
			CString strPath = A2T(rArrayLast.cTitle);
			strPath += _T(".jpg");
			strPath = strFolder + strPath;
			MergeNewImage(imgLast, imgCur, strPath);
			//TRACE("%s\n", T2A(strPath));
		}

		if (nIndex == nSize-1)
		{
			CString strPath;
			strPath = A2T(rArrayCur.cTitle);
			strPath += _T(".jpg");
			strPath = strFolder + strPath;
			imgCur.Save(strPath, CXIMAGE_FORMAT_JPG);
		}
		imgLast.Transfer(imgCur);
		rArrayLast = rArrayCur;
		nIndex++;
	}

	return TRUE;
}

void GetCropRect(CRect rectModel, double dTransfer[6], CRect &dstRect)
{
    CPoint p1 = rectModel.TopLeft();
    CPoint p2 = rectModel.BottomRight();
    CPoint p11 = CPoint(p2.x, p1.y);
    CPoint p22 = CPoint(p1.x, p2.y);

    CPoint p3 = CPoint(dTransfer[0] * p1.x + dTransfer[1] * p1.y + dTransfer[2],\
    dTransfer[3]*p1.x + dTransfer[4]*p1.y + dTransfer[5]);

    CPoint p4 = CPoint(dTransfer[0] * p2.x + dTransfer[1] * p2.y + dTransfer[2], \
        dTransfer[3] * p2.x + dTransfer[4] * p2.y + dTransfer[5]);
    
    CPoint p33 = CPoint(dTransfer[0] * p11.x + dTransfer[1] * p11.y + dTransfer[2], \
        dTransfer[3] * p11.x + dTransfer[4] * p11.y + dTransfer[5]);

    CPoint p44 = CPoint(dTransfer[0] * p22.x + dTransfer[1] * p22.y + dTransfer[2], \
        dTransfer[3] * p22.x + dTransfer[4] * p22.y + dTransfer[5]);
     
    p3 = CPoint(min(p3.x, p44.x), min(p3.y, p33.y));
    p4 = CPoint(max(p4.x, p33.x), max(p4.y, p44.y));
    dstRect.SetRect(p3, p4);
}

//static int g_index = 1;
extern "C" DLL_EXPORT bool ZYJ_GetCurOmrRes3(const char *cFilePath, \
	const int nPage, const int nThreshold, ANS *dstAns, const int nABFlag)
{
	CxImage srcImg;
	USES_CONVERSION;
	CString strImgPath = A2T(cFilePath);
	int nFileType = GetFileType1(strImgPath);
	bool bRes = srcImg.Load(strImgPath, nFileType);
	if (!bRes)
		return FALSE;

	CurImage curImgTmp;
	int nRectType = g_nLocalType;
	curImgTmp.v_rectModelLocal.clear();
	curImgTmp.v_modelRectArrays.clear();
	curImgTmp.v_OMRZuIDs.clear();
	curImgTmp.v_modelKhRectArray.clear();
	curImgTmp.m_rectLocal.clear();

	for (int i = 0; i < g_v_rectModelLocal.size(); i++)
	{
		if (g_v_rectModelLocal[i].nPage == nPage)
		{
			curImgTmp.v_rectModelLocal.push_back(g_v_rectModelLocal[i]);
		}
	}

	for (int i = 0; i < g_v_modelRectArrays.size(); i++)
	{
		if (g_v_modelRectArrays[i].rect.nPage == nPage && g_v_modelRectArrays[i].nABFlag == nABFlag)
		{
			curImgTmp.v_modelRectArrays.push_back(g_v_modelRectArrays[i]);
		}
	}

	if (curImgTmp.v_rectModelLocal.size() < 3)
	{
		return FALSE;
	}

	//将定位点排序 以考虑图像上任意三个定位点问题
	CRect rectLocals[3];
	double dLenth[3] = { 0.0, 0.0, 0.0 };
	for (int i = 0; i < 3; i++)
	{
		rectLocals[i] = NewRect2Rect(curImgTmp.v_rectModelLocal[i]);
	}
	dLenth[0] = CImageProcess::GetLength(rectLocals[0].CenterPoint(), rectLocals[1].CenterPoint());
	dLenth[1] = CImageProcess::GetLength(rectLocals[0].CenterPoint(), rectLocals[2].CenterPoint());
	dLenth[2] = CImageProcess::GetLength(rectLocals[1].CenterPoint(), rectLocals[2].CenterPoint());

	for (int i = 0; i < 3; i++)
	{
		if (abs(max(max(dLenth[0], dLenth[1]), dLenth[2]) - dLenth[i]) <= 0.00000001) //判断斜边选
		{
			if (i == 0)
			{
				NEWRECT modelTmps = curImgTmp.v_rectModelLocal[0];
				curImgTmp.v_rectModelLocal[0] = curImgTmp.v_rectModelLocal[2];
				curImgTmp.v_rectModelLocal[2] = modelTmps;
			}
			else if (i == 1)
			{
				NEWRECT modelTmps = curImgTmp.v_rectModelLocal[0];
				curImgTmp.v_rectModelLocal[0] = curImgTmp.v_rectModelLocal[1];
				curImgTmp.v_rectModelLocal[1] = modelTmps;
			}
		}
	}

	//获取三个角定位点相关
	//int nTime1 = GetTickCount();
	//g_mute.Lock();
    bool bFindLocal = false;
	if (!bFindLocal)
	{
		for (int i = 0; i < 3; i++)
		{
			NEWRECT rectArea1;
			NEWRECT dstRect;
			rectArea1 = GetRectArea1(curImgTmp.v_rectModelLocal[i], &srcImg);
			int nDirIndex = 0;
			if (rectArea1.CenterNewPoint().nX > srcImg.GetWidth() / 2 && rectArea1.CenterNewPoint().nY < srcImg.GetHeight() / 2)
				nDirIndex = 1;
			else if (rectArea1.CenterNewPoint().nX < srcImg.GetWidth() / 2 && rectArea1.CenterNewPoint().nY > srcImg.GetHeight() / 2)
				nDirIndex = 2;
			else if (rectArea1.CenterNewPoint().nX > srcImg.GetWidth() / 2 && rectArea1.CenterNewPoint().nY > srcImg.GetHeight() / 2)
				nDirIndex = 3;

			bRes = ZYJ_GetRectLocalPoint(rectArea1, curImgTmp.v_rectModelLocal[i].GetWidth(), curImgTmp.v_rectModelLocal[i].GetHeight(),
				nDirIndex, nRectType, dstRect, &srcImg, &curImgTmp);

			if (!bRes)
			{
				//g_imgProcess->m_nStatueRes = ERROR_LOCATE;
				
				return FALSE;
			}
		}

        /*
            if (g_v_LocalPointData.size() >= 20)
            {
                g_mute.Lock();
                g_v_LocalPointData.erase(g_v_LocalPointData.begin());
                g_mute.Unlock();
            }*/

            /*LOCALPOINT_DATA localPointData;
            strcpy(localPointData.cCurImgPath, cFilePath);
            for (int i = 0; i < 3; i++)
                localPointData.pLocalRects[i] = curImgTmp.m_rectLocal[i];*/
		//g_v_LocalPointData.push_back(localPointData);
		
	}
	
	//图像仿射 2016.12.16
	//数组声明
	CvPoint2D32f srcTri[3], dstTri[3];
	//创建指针
	CvMat* warp_mat = cvCreateMat(2, 3, CV_32FC1);
	IplImage *src;
	src = cvLoadImage(cFilePath, CV_LOAD_IMAGE_GRAYSCALE);
    IplImage *dst;// = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	dst = cvCloneImage(src);
	dst->origin = src->origin;
	//cvZero(dst);
	cvSet(dst, cvScalar(255, 255, 255));
	
	//计算变换矩阵
	for (int i = 0; i < 3; i++)
	{
		srcTri[i].x = curImgTmp.m_rectLocal[i].CenterNewPoint().nX;
		srcTri[i].y = curImgTmp.m_rectLocal[i].CenterNewPoint().nY;
		dstTri[i].x = curImgTmp.v_rectModelLocal[i].CenterNewPoint().nX;
		dstTri[i].y = curImgTmp.v_rectModelLocal[i].CenterNewPoint().nY;
	}
	cvGetAffineTransform(srcTri, dstTri, warp_mat);
	//调用函数cvWarpAffine（）
	cvWarpAffine(src, dst, warp_mat);
	//char cPath[256];
	//memset(cPath, '\0', 256);
	//sprintf(cPath, "c:\\test_%d.jpg", 1);
	////g_index++;
	//cvSaveImage(cPath, dst);

	vector<CvRect> cvRectInVec;
	vector<int> vecPix;

	int nLastID = -1;
	char cLastTitle[256];
	memset(cLastTitle, '/0', 256);
	int nIndex = 0;
	//存储当前页号的客观题
	for (vector<RECTARRAY>::iterator it = g_v_modelRectArrays.begin(); it != g_v_modelRectArrays.end(); it++)
	{
		RECTARRAY rectArry = *it;
		if (rectArry.rect.nPage == nPage && rectArry.nABFlag == nABFlag)
		{
			if (nLastID != rectArry.nQuestionNo)
			{
				if (nLastID != -1)
				{
					wstring strTmp1;
					int iret = GetOption1(dst,cvRectInVec,1,vecPix,0,strTmp1, nThreshold);
					if (iret != 0)
					{
						cvReleaseImage(&src);
						cvReleaseImage(&dst);
						cvReleaseMat(&warp_mat);
						return FALSE;
					}
					else
					{
						dstAns[nIndex].nID = nLastID;
						CString strAns = strTmp1.c_str();
						strAns.Replace(_T(","), _T(""));
						strAns = strAns.Trim();
						if (strAns.IsEmpty())
							strAns = _T("#");
						strcpy(dstAns[nIndex].cAnswer, T2A(strAns));
						strcpy(dstAns[nIndex].cTitle, cLastTitle);
						dstAns[nIndex].dstNewRectFirst.SetRect(cvRectInVec[0].x, cvRectInVec[0].y, 
							cvRectInVec[0].width, cvRectInVec[0].height);
						dstAns[nIndex].nSize = cvRectInVec.size();
						dstAns[nIndex].nPage = nPage;
						if (cvRectInVec.size() > 1)
						{
							dstAns[nIndex].nLenX = cvRectInVec[1].x - cvRectInVec[0].x;
							dstAns[nIndex].nLenY = cvRectInVec[1].y - cvRectInVec[0].y;
						}

						nIndex++;
					}
				}

				cvRectInVec.clear();
				vecPix.clear();
				nLastID = rectArry.nQuestionNo;
				memset(cLastTitle, '/0', 256);
				strcpy(cLastTitle, rectArry.cTitle);
			}

			CvRect rect_cv = cvRect(rectArry.rect.pTopLeft.nX, rectArry.rect.pTopLeft.nY,\
				rectArry.rect.GetWidth(), rectArry.rect.GetHeight());
			cvRectInVec.push_back(rect_cv);
			int uPix = (int) rectArry.nPix;
			vecPix.push_back(uPix);

			if (it == g_v_modelRectArrays.end()- 1)
			{
				wstring strTmp1;
				int iret = GetOption1(dst, cvRectInVec, 1, vecPix, 0, strTmp1, nThreshold);
				if (iret != 0)
				{
					cvReleaseImage(&src);
					cvReleaseImage(&dst);
					cvReleaseMat(&warp_mat);
					return FALSE;
				}

				dstAns[nIndex].nID = nLastID;
				CString strAns = strTmp1.c_str();
				strAns.Replace(_T(","), _T(""));
				strAns = strAns.Trim();
				if (strAns.IsEmpty())
					strAns = _T("#");

				strcpy(dstAns[nIndex].cAnswer, T2A(strAns));
				strcpy(dstAns[nIndex].cTitle, rectArry.cTitle);
				dstAns[nIndex].dstNewRectFirst.SetRect(cvRectInVec[0].x, cvRectInVec[0].y,
					cvRectInVec[0].width, cvRectInVec[0].height);
				dstAns[nIndex].nSize = cvRectInVec.size();
				dstAns[nIndex].nPage = nPage;
				if (cvRectInVec.size() > 1)
				{
					dstAns[nIndex].nLenX = cvRectInVec[1].x - cvRectInVec[0].x;
					dstAns[nIndex].nLenY = cvRectInVec[1].y - cvRectInVec[0].y;
				}
			}
		}
	}

	cvReleaseImage(&src);
	cvReleaseImage(&dst);
	cvReleaseMat(&warp_mat);
	return TRUE;
}

int GetOption1(IplImage* pSrcIn, vector<CvRect> cvRectInVec,
	int Color, vector<int> uiPixVec,
	int uiType, std::wstring &csOptOutVec,
	int uiThread)
{
	double flTplMul = 1.98;
	double flSamMul = 1.7;
	double iRefindTimes = 2;
	double flRefindStep = 0.1;
	double flMinSelcet = 0.35;
	int iThres = uiThread;

	if (pSrcIn == NULL)
	{
		return -1;
	}
	if (iRefindTimes < 1)
	{
		return -1;
	}
	if (flTplMul < 1)
	{
		return -1;
	}
	if (flRefindStep < 0)
	{
		return -1;
	}

	vector<pair<CvRect, float > > cvRectVec;
	//int uiThread = iThres;
	for (int i = 0; i < cvRectInVec.size(); ++i) {
		pair<CvRect, float> fRect;
		fRect.first = cvRectInVec[i];
		fRect.second = (float)uiPixVec[i] / (float)(fRect.first.width*fRect.first.height);
		cvRectVec.push_back(fRect);
	}

	pair<CvRect, std::wstring > cvrstrp;
	vector<pair<CvRect, float > > temcvrectv;//一道题目的区域
	multimap<float, pair<float, int> > OptInfoMap;//一道题目的答案占空比，阈值，选项编号
	pair<float, int> pzb;
	pair<float, pair<float, int> > OptInfo;//一个选项的占空比，yuzhi和编号
	float OptTplThres = 0;	//样本阈值
	int selected = 0;	//样本选项中填涂的选项个数
	int unselected = 0;	//样本选项中未填涂的选项个数
	std::wstring strAns;	//一道题目的答案
	float basenum = 1;
	float zhankongbi = 0;
	float avgsam = 0;
	pair<CvRect, float > cvfp;
	CvRect drawrec;
	float MarkSecect = 0;	//单选记录已选择的占空比
	float OptSamThres;
	float Optvalue;

	for (int j = 0; j < cvRectVec.size(); j++)
	{
		cvfp = cvRectVec.at(j);
		drawrec = cvfp.first;
		OptTplThres = cvfp.second + 0.28;

		//#define TEST_OBJECT
		if (NP_GetZhanKong(pSrcIn, drawrec, iThres, zhankongbi) < 0)
			return -1;

		bool bDensity = NP_GetDensity(pSrcIn, drawrec, iThres);

		if ((zhankongbi > flMinSelcet) && (!bDensity))
		{
			selected++;
		}
		else
		{
			avgsam += zhankongbi;
			unselected++;
		}
		pzb.first = OptTplThres;
		pzb.second = j;
		OptInfo.first = zhankongbi;
		OptInfo.second = pzb;
		OptInfoMap.insert(OptInfo);
	}

	bool checkedfg = 0;	//控制若识别出则停止。

	if (unselected > 0)
	{
		avgsam /= unselected;
		avgsam += 0.26;
	}
	float firstM = 0;	//用来判断选项是否识别有误
	bool setfirst = 0;

	for (int k = 0; k < iRefindTimes&&checkedfg == 0; k++)
	{
		for (multimap<float, pair<float, int> >::reverse_iterator OptInfoIter = OptInfoMap.rbegin(); OptInfoIter != OptInfoMap.rend(); OptInfoIter++)
		{
			Optvalue = OptInfoIter->first;
			if (setfirst == 0)
			{
				firstM = Optvalue;
				setfirst = 1;
			}
			pzb = OptInfoIter->second;
			int Optnum = pzb.second;

			if (checkedfg == 0)
			{
				OptTplThres = pzb.first*basenum;
				OptSamThres = avgsam*basenum;
			}
			else
			{
				OptTplThres = MarkSecect*(float)0.7 > pzb.first*basenum ? MarkSecect*(float)0.7 : pzb.first*basenum;
				OptSamThres = MarkSecect*(float)0.67 > avgsam*basenum ? MarkSecect*(float)0.67 : avgsam*basenum;
			}
			if (Optvalue >= flMinSelcet)
			{
				if (Optvalue > OptTplThres)
				{
					checkedfg = 1;
					char org;
					if (uiType != 0)
						org = '0';
					else
						org = 'A';
					MarkSecect = Optvalue;

					org += Optnum;
					strAns += org;
				}
				else if (Optvalue > OptSamThres)
				{
					checkedfg = 1;
					char org;
					if (uiType != 0)
						org = '0';
					else
						org = 'A';
					org += Optnum;
					strAns += org;
					MarkSecect = Optvalue;
				}
			}
			else
			{
				break;
			}
		}
		basenum -= (float)flRefindStep;
	}

	if (strAns.length() >= 1)
	{
		for (int n = 0; n < strAns.length() - 1; n++)//排序
		{
			for (int m = n + 1; m < strAns.length(); m++)
			{
				if (strAns[n] > strAns[m])
				{
					wchar_t c = strAns[n];
					strAns[n] = strAns[m];
					strAns[m] = c;
				}
			}
		}
	}

	if (uiType == 0)
	{ //选择题
		std::wstring strTemp;
		if (strAns.length() > 1)
		{
			for (int n = 0; n < strAns.length(); ++n)//加,
			{
				if (n == strAns.length() - 1)
				{
					strTemp += strAns[n];
				}
				else
				{
					strTemp += strAns[n];
					strTemp += _T(",");
				}
			}
		}
		else
		{
			strTemp = strAns;
		}

		strAns = strTemp;
	}
	OptInfoMap.clear();
	selected = 0;
	unselected = 0;
	avgsam = 0;
	basenum = 1.0;
	setfirst = 0;
	OptSamThres = 0;
	csOptOutVec = strAns;
	return 0;
}

int GetOptionPixThread(IplImage* pSrcIn,
	vector<CvRect> cvRectIn,
	int uicolor,
	vector<int> &uiOutPix,
	vector<int> &uiOutThread)
{
	if (pSrcIn == NULL)
	{
		return -1;
	}
	CvPoint lu, rd;
	CvScalar s;
	int blacknum = 0;
	for (int i = 0; i < cvRectIn.size(); ++i)
	{
		CvRect drawrec = cvRectIn.at(i);
		lu.x = drawrec.x;
		lu.y = drawrec.y;
		rd.x = lu.x + drawrec.width;
		rd.y = lu.y + drawrec.height;
		for (int n = lu.x, m = lu.y; n < rd.x; n++)//识别用灰度
		{
			m = lu.y;
			for (; m < rd.y; m++)
			{
				s = cvGet2D(pSrcIn, m, n);
				if (s.val[0] <= 190)
					blacknum++;
			}
		}
		uiOutPix.push_back(blacknum);
		uiOutThread.push_back(190);
		blacknum = 0;
	}

	blacknum = 0;
	return 0;
}


int NP_GetZhanKong(IplImage* pSrcIn, CvRect cvRectIn, int iThres, float& flzkOut)
{
	if (cvRectIn.height*cvRectIn.width <= 0)
	{
		return -1;
	}
	if (cvRectIn.x < 0 || cvRectIn.y < 0)
	{
		return -1;
	}
	CvPoint lu, rd;
	int blacknum = 0, whitenum = 0;
	CvScalar s;
	lu.x = cvRectIn.x;
	lu.y = cvRectIn.y;
	rd.x = lu.x + cvRectIn.width;
	rd.y = lu.y + cvRectIn.height;
	for (int n = lu.x, m = lu.y; n < rd.x; n++)//识别用灰度
	{
		m = lu.y;
		for (; m < rd.y; m++)
		{
			s = cvGet2D(pSrcIn, m, n);
			if (s.val[0] <= iThres)
				blacknum++;
			else
				whitenum++;
		}
	}
	flzkOut = (float)blacknum / (float)(whitenum + blacknum);
	return 0;
}

//获取填涂密度
bool NP_GetDensity(IplImage* pSrcIn, CvRect cvRectIn, int iThres)
{
	bool bDensity = false;
	if (cvRectIn.height*cvRectIn.width <= 0)
	{
		return true;
	}
	if (cvRectIn.x < 0 || cvRectIn.y < 0)
	{
		return true;
	}
	CvPoint lu, rd;
	int blacknum = 0, whitenum = 0;
	CvScalar s;
	lu.x = cvRectIn.x;
	lu.y = cvRectIn.y;
	rd.x = lu.x + cvRectIn.width;
	rd.y = lu.y + cvRectIn.height;
	std::vector<float> vectDensityx;
	//得到X轴的密度
	for (int n = lu.x, m = lu.y; n < rd.x; n++)//识别用灰度
	{
		m = lu.y;
		blacknum = 0;
		for (; m < rd.y; m++)
		{
			s = cvGet2D(pSrcIn, m, n);
			if (s.val[0] <= iThres)
				blacknum++;
		}
		vectDensityx.push_back((float)blacknum);
	}

	//正向擦除0的值
	for (std::vector<float>::iterator vi = vectDensityx.begin(); vi != vectDensityx.end();)
	{
		if (*vi == 0)
		{
			vi = vectDensityx.erase(vi);
		}
		else
		{
			break;
		}
	}
	//逆向擦除0的值
	int num = 0;
	for (std::vector<float>::reverse_iterator ri = vectDensityx.rbegin(); ri != vectDensityx.rend();)
	{
		if (*ri == 0)
		{
			++ri;
			num++;
		}
		else
		{
			if (num)
				vectDensityx.erase(vectDensityx.begin() + (vectDensityx.size() - num - 1), vectDensityx.end());

			break;
		}
	}

	//得到Y轴的密度
	std::vector<float> vectDensityy;
	for (int n = lu.y, m = lu.x; n < rd.y; n++)//识别用灰度
	{
		m = lu.x;
		blacknum = 0;
		for (; m < rd.x; m++)
		{
			s = cvGet2D(pSrcIn, n, m);
			if (s.val[0] <= iThres)
				blacknum++;
		}
		vectDensityy.push_back((float)blacknum);
	}

	//正向擦除0的值
	for (std::vector<float>::iterator vi = vectDensityy.begin(); vi != vectDensityy.end();)
	{
		if (*vi == 0)
		{
			vi = vectDensityy.erase(vi);
		}
		else
		{
			break;
		}
	}
	//逆向擦除0的值
	num = 0;
	for (std::vector<float>::reverse_iterator ri = vectDensityy.rbegin(); ri != vectDensityy.rend();)
	{
		if (*ri == 0)
		{
			++ri;
			num++;
		}
		else
		{
			if (num)
				vectDensityy.erase(vectDensityy.begin() + (vectDensityy.size() - num - 1), vectDensityy.end());

			break;
		}
	}


	int uiWidth = vectDensityx.size();
	int uiHeight = vectDensityy.size();
	int uiThreadVal = 0;
	if ((uiWidth < (int)cvRectIn.width / 3) || (uiHeight < (int)cvRectIn.height / 3))
	{
		return true;
	}

	for (int i = 0; i < vectDensityx.size(); ++i)
	{
		if (vectDensityx[i] > (float)uiHeight / 3.0 + 3.0)
			uiThreadVal++;
	}

	return (uiThreadVal < uiWidth / 2.0 + 4.0);
}


extern "C" DLL_EXPORT bool ZYJ_IsCardAbsent(const char *cFilePath, int nThreshold)
{
	CxImage srcImg;// = new CxImage();
	bool bRes = false;
	int nFileType = CXIMAGE_FORMAT_UNKNOWN;
	USES_CONVERSION;
	CString strFilePath = A2T(cFilePath);
	nFileType = GetFileType1(strFilePath);
	bRes = srcImg.Load(strFilePath, nFileType);
	if (!bRes)
		return FALSE;

	srcImg.GrayScale();
	CurImage curImgPara;
	int nPage = 1;
	if (!GetCurImgInfo(&srcImg, cFilePath, nPage, 0, nThreshold, -1, -1, curImgPara))
		return FALSE;
	
	curImgPara.modelAbsentRectArray = g_RaAbsent;
	vector<RECTARRAY> dstArray;
	bRes = ZYJ_GetRectArray(1, dstArray, 2, -1, &srcImg, &curImgPara);
	if (bRes)
	{
        if (dstArray[0].bMinFilling)
            return true;
	}
    return FALSE;
}

extern "C" DLL_EXPORT bool ZYJ_GetOmrSyncHead(const char *cPath, const NEWRECT rectArea, const  int nThreshold, const int nDirection, int &nDstSize, NEWRECT v_dstRects[])
{
    CxImage srcImg;// = new CxImage();
    bool bRes = false;
    int nFileType = CXIMAGE_FORMAT_UNKNOWN;
    USES_CONVERSION;
    CString strFilePath = A2T(cPath);
    nFileType = GetFileType1(strFilePath);
    bRes = srcImg.Load(strFilePath, nFileType);
    if (!bRes)
        return FALSE;

    srcImg.GrayScale();
    CImageProcess imgProcessTmp;
    CSize minSize = CSize(5, 5);
    vector<CRect> dstRectsTmp;
    imgProcessTmp.GetMinRect(srcImg, minSize, nThreshold, dstRectsTmp);

    if (nDirection == 0) //横向 
    {

    }
    else if (nDirection == 1) //竖向
    {
        for (int i = 0; i < dstRectsTmp.size(); i++)
        {
            for (int j = i + 1; j < dstRectsTmp.size(); j++)
            {
                if (dstRectsTmp[i].CenterPoint().y > dstRectsTmp[j].CenterPoint().y)
                {
                    CRect rectTmp = dstRectsTmp[j];
                    dstRectsTmp[j] = dstRectsTmp[i];
                    dstRectsTmp[i] = rectTmp;
                }
            }
        }
    }

}

extern "C" DLL_EXPORT bool ZYJ_GetOmrSyncHead_1(const IplImage *iplImg, const NEWRECT rectArea, const  int nThreshold, const int nDirection, int &nDstSize, NEWRECT v_dstRects[])
{
    USES_CONVERSION;
    CString strPath;
    char *c = T2A(strPath);

    //ZYJ_GetOmrSyncHead();
    return true;
}

extern "C" DLL_EXPORT bool ZYJ_GetCurOmrRes4(const char *cFilePath, const int nPage, const int nThreshold, ANS *dstAns, const int nABFlag)
{
    CxImage srcImg;
    USES_CONVERSION;
    CString strImgPath = A2T(cFilePath);
    int nFileType = GetFileType1(strImgPath);
    bool bRes = srcImg.Load(strImgPath, nFileType);
    if (!bRes)
        return FALSE;

    CurImage curImgTmp;
    int nRectType = g_nLocalType;
    curImgTmp.v_rectModelLocal.clear();
    curImgTmp.v_modelRectArrays.clear();
    curImgTmp.v_OMRZuIDs.clear();
    curImgTmp.v_modelKhRectArray.clear();
    curImgTmp.m_rectLocal.clear();

    for (int i = 0; i < g_v_rectModelLocal.size(); i++)
    {
        if (g_v_rectModelLocal[i].nPage == nPage)
        {
            curImgTmp.v_rectModelLocal.push_back(g_v_rectModelLocal[i]);
        }
    }

    for (int i = 0; i < g_v_modelRectArrays.size(); i++)
    {
        if (g_v_modelRectArrays[i].rect.nPage == nPage && g_v_modelRectArrays[i].nABFlag == nABFlag)
        {
            curImgTmp.v_modelRectArrays.push_back(g_v_modelRectArrays[i]);
        }
    }

    if (curImgTmp.v_rectModelLocal.size() < 3)
    {
        return FALSE;
    }

    //将定位点排序 以考虑图像上任意三个定位点问题
    CRect rectLocals[3];
    double dLenth[3] = { 0.0, 0.0, 0.0 };
    for (int i = 0; i < 3; i++)
    {
        rectLocals[i] = NewRect2Rect(curImgTmp.v_rectModelLocal[i]);
    }
    dLenth[0] = CImageProcess::GetLength(rectLocals[0].CenterPoint(), rectLocals[1].CenterPoint());
    dLenth[1] = CImageProcess::GetLength(rectLocals[0].CenterPoint(), rectLocals[2].CenterPoint());
    dLenth[2] = CImageProcess::GetLength(rectLocals[1].CenterPoint(), rectLocals[2].CenterPoint());

    for (int i = 0; i < 3; i++)
    {
        if (abs(max(max(dLenth[0], dLenth[1]), dLenth[2]) - dLenth[i]) <= 0.00000001) //判断斜边选
        {
            if (i == 0)
            {
                NEWRECT modelTmps = curImgTmp.v_rectModelLocal[0];
                curImgTmp.v_rectModelLocal[0] = curImgTmp.v_rectModelLocal[2];
                curImgTmp.v_rectModelLocal[2] = modelTmps;
            }
            else if (i == 1)
            {
                NEWRECT modelTmps = curImgTmp.v_rectModelLocal[0];
                curImgTmp.v_rectModelLocal[0] = curImgTmp.v_rectModelLocal[1];
                curImgTmp.v_rectModelLocal[1] = modelTmps;
            }
        }
    }

    //获取三个角定位点相关
    for (int i = 0; i < 3; i++)
    {
        NEWRECT rectArea1;
        NEWRECT dstRect;
        rectArea1 = GetRectArea1(curImgTmp.v_rectModelLocal[i], &srcImg);
        int nDirIndex = 0;
        if (rectArea1.CenterNewPoint().nX > srcImg.GetWidth() / 2 && rectArea1.CenterNewPoint().nY < srcImg.GetHeight() / 2)
            nDirIndex = 1;
        else if (rectArea1.CenterNewPoint().nX < srcImg.GetWidth() / 2 && rectArea1.CenterNewPoint().nY > srcImg.GetHeight() / 2)
            nDirIndex = 2;
        else if (rectArea1.CenterNewPoint().nX > srcImg.GetWidth() / 2 && rectArea1.CenterNewPoint().nY > srcImg.GetHeight() / 2)
            nDirIndex = 3;

        bRes = ZYJ_GetRectLocalPoint(rectArea1, curImgTmp.v_rectModelLocal[i].GetWidth(), curImgTmp.v_rectModelLocal[i].GetHeight(),
            nDirIndex, nRectType, dstRect, &srcImg, &curImgTmp);

        if (!bRes)
        {
            return FALSE;
        }
    }

    CurImage curImgPara;
    if (!GetCurImgInfo(&srcImg, cFilePath, nPage, 0, nThreshold, -1, nABFlag, curImgTmp))
        return FALSE;
   

    //获取同步头
    vector<RECTARRAY> v_H_RectArrayHeader; //横向
    vector<RECTARRAY> v_V_RectArrayHeader; //竖向
    CRect rectArea; 
    
    //根据同步头识别客观题 2017.02.13
    //计算偏移量 
    CSize dstSize; //通过横纵向同步头计算客观题大小
    NEWPOINT pCenter1 = v_V_RectArrayHeader[0].CenterNewPoint();
    NEWPOINT pCenter2 = v_V_RectArrayHeader[v_V_RectArrayHeader.size()-1].CenterNewPoint();
    int nMoveVx = pCenter2.nX - pCenter1.nX;
    int nMoveVy = pCenter2.nY - pCenter1.nY;

    pCenter1 = v_H_RectArrayHeader[0].CenterNewPoint();
    pCenter2 = v_H_RectArrayHeader[v_H_RectArrayHeader.size()-1].CenterNewPoint();
    int nMoveHx = pCenter2.nX - pCenter1.nX;
    int nMoveHy = pCenter2.nY - pCenter1.nY;

    int nMoveVLen = max(sqrt(pow(nMoveVx,2)+pow(nMoveVy,2)), 1);
    int nMoveHLen = max(sqrt(pow(nMoveHx,2)+pow(nMoveHy,2)), 1);
   
    NEWPOINT pStartTmp1 = v_V_RectArrayHeader[0].CenterNewPoint();
    NEWPOINT pStartTmp2 = v_H_RectArrayHeader[0].CenterNewPoint();
    vector<RECTARRAY> v_dstAnsRectArray;
    for (vector<RECTARRAY>::iterator it1 = v_V_RectArrayHeader.begin(); it1 != v_V_RectArrayHeader.end(); it1++)
    {
        for (vector<RECTARRAY>::iterator it2 = v_H_RectArrayHeader.begin(); it2 != v_H_RectArrayHeader.end(); it2++)
        {
            RECTARRAY rectArrayTmp1 = *it1;
            RECTARRAY rectArrayTmp2 = *it2;
            NEWPOINT pCenterTmp1 = rectArrayTmp1.CenterNewPoint();
            NEWPOINT pCenterTmp2 = rectArrayTmp2.CenterNewPoint();

            int nLen1 = sqrt(pow(pCenterTmp1.nX - pStartTmp1.nX,2) + pow(pCenterTmp1.nY-pStartTmp1.nY,2));
            int nLen2 = sqrt(pow(pCenterTmp2.nX - pStartTmp2.nX,2) + pow(pCenterTmp2.nY-pStartTmp2.nY,2));
            int nCenterX1 = nMoveVx*nLen1/nMoveVLen + nMoveHx*nLen2/nMoveHLen + pStartTmp1.nX;
            int nCenterY1 = nMoveVy*nLen1/nMoveVLen + nMoveHy*nLen2/nMoveHLen + pStartTmp1.nY;
            
            RECTARRAY rectArrayAnsTmp;
            rectArrayAnsTmp.rect.SetRect(nCenterX1-dstSize.cx/2, nCenterY1-dstSize.cy/2, dstSize.cx, dstSize.cy);
            CRect rectTmp1 = NewRect2Rect(rectArrayAnsTmp.rect);
            //计算其不同阈值填涂密度  2017.02.14
            CImageProcess imgProcessTmp;
            ZYJ_IsUnFilling(rectArrayAnsTmp, nThreshold, &srcImg);//判断是否为无填涂
            
            rectArrayAnsTmp.dDensity = GetGrayDensity(rectArrayAnsTmp.rect, nThreshold, &srcImg, rectArrayAnsTmp.nAverGrayValue);
            rectArrayAnsTmp.dDensity_140 = imgProcessTmp.GetDensity(&srcImg, rectTmp1, 140);
            rectArrayAnsTmp.dDensity_220 = imgProcessTmp.GetDensity(&srcImg, rectTmp1, 220);
            v_dstAnsRectArray.push_back(rectArrayAnsTmp);
        }
    }

    int nDstAnsSize = v_dstAnsRectArray.size();
    ZYJ_GetOmrResult(v_dstAnsRectArray, nDstAnsSize, curImgTmp.v_modelRectArrays, curImgTmp.m_nAverGrayValue);

    //将识别结果分配至对应ID当中
    int nID1 = -1;
    ANS ansTmp;
    CString strAns = L"";
    CString strCheckAns = L"";
    vector<ANS> v_ans;
    vector<ANS> v_ansCheck;
    int nCount1 = 0;
    bool bFind1 = false;
    for (int i = 0; i < nDstAnsSize; i++)
    {
        RECTARRAY rArrayTmp = v_dstAnsRectArray[i];

        if (rArrayTmp.nQuestionNo == -1)
            return FALSE;

        nCount1++;
        bFind1 = true;
        if (rArrayTmp.nQuestionNo > nID1)
        {
            bFind1 = false;
            //nCount1++;
            ansTmp.nID = rArrayTmp.nQuestionNo;
            nID1 = rArrayTmp.nQuestionNo;
            strcpy(ansTmp.cTitle, rArrayTmp.cTitle);
            ansTmp.dstNewRectFirst.SetRect(rArrayTmp.rect.pTopLeft, rArrayTmp.rect.pBottomRight);
            ansTmp.nSize = nCount1;
            ansTmp.nPage = nPage;
            if (v_ans.size() > 0)
            {
                memcpy(v_ans[v_ans.size() - 1].cAnswer, T2A(strAns), strAns.GetLength());
                v_ans[v_ans.size() - 1].nLenX = ansTmp.nLenX;
                v_ans[v_ans.size() - 1].nLenY = ansTmp.nLenY;
                v_ans[v_ans.size() - 1].nSize = nCount1;
            }


            v_ans.push_back(ansTmp);
            strAns = L"";
            nCount1 = 0;
        }
        else if (bFind1)
        {
            ansTmp.nLenX = max(rArrayTmp.rect.pTopLeft.nX - v_dstAnsRectArray[i - 1].rect.pTopLeft.nX, 0);
            if (abs(ansTmp.nLenX) < nEPS)
                ansTmp.nLenX = 0;

            ansTmp.nLenY = max(rArrayTmp.rect.pTopLeft.nY - v_dstAnsRectArray[i - 1].rect.pTopLeft.nY, 0);
            if (abs(ansTmp.nLenY) < nEPS)
                ansTmp.nLenY = 0;
        }

        if (rArrayTmp.nFilling == 1) //已填涂1
        {
            char c[2];
            c[0] = 'A' + rArrayTmp.nAnswerIndex;
            c[1] = '\0';
            strAns += A2T(c);
        }
        else  //未填涂 
            strAns += L"";

        if (i == nDstAnsSize - 1)
        {
            nCount1++;
            strcpy(ansTmp.cTitle, rArrayTmp.cTitle);
            if (v_ans.size() > 0)
            {
                memcpy(v_ans[v_ans.size() - 1].cAnswer, T2A(strAns), strAns.GetLength());
                v_ans[v_ans.size() - 1].nLenX = ansTmp.nLenX;
                v_ans[v_ans.size() - 1].nLenY = ansTmp.nLenY;
                v_ans[v_ans.size() - 1].nSize = nCount1;
            }
        }
    }

    if (v_ans.size() > 0)
    {
        for (int i = 0; i < v_ans.size(); i++)
        {
            dstAns[i] = v_ans[i];

            CString strText1 = A2T(dstAns[i].cAnswer);
            if (strText1.IsEmpty())
            {
                strText1 = _T("#");
                memcpy(dstAns[i].cAnswer, T2A(strText1), strText1.GetLength());
            }
            //TRACE("ID:%d, 答案：%s\n", dstAns[i].nID, dstAns[i].cAnswer);
        }
    }

    return true;
}