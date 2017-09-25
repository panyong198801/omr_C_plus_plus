<<<<<<< HEAD
#include "stdafx.h"
#include "ImageProcess.h"

//#ifndef ZYJ_ICR
//#define  ZYJ_ICR 0
//#endif


//#define RECOGNIZER "c:\\re\\numplus.rec"

#if _MSC_VER >= 1300    // for VC 7.0
// from ATL 7.0 sources
#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif
#endif

static HMODULE GetCurrentModule()
{
#if _MSC_VER < 1300    // earlier than .NET compiler (VC 6.0)

	// Here's a trick that will get you the handle of the module
	// you're running in without any a-priori knowledge:
	MEMORY_BASIC_INFORMATION mbi;
	static int dummy;
	VirtualQuery( &dummy, &mbi, sizeof(mbi) );

	return reinterpret_cast<HMODULE>(mbi.AllocationBase);
#else    // VC 7.0
	// from ATL 7.0 sources
	return reinterpret_cast<HMODULE>(&__ImageBase);
#endif
}

//int g_nImgindex = -1;
CImageProcess::CImageProcess(void)
{
	
	m_dstBlackSize = CSize(-1, -1);
}



CImageProcess::~CImageProcess(void)
{

}

BOOL  CImageProcess::IsPointInRect(CPoint p1, CRect rect)
{
	BOOL bRes = FALSE;
	if (p1.x >= rect.TopLeft().x && p1.x <= rect.BottomRight().x)
	{
		if (p1.y >= rect.TopLeft().y && p1.y <= rect.BottomRight().y)
		{
			bRes = TRUE;
		}
	}
	return bRes;
}

double CImageProcess::GetLength(CPoint p1, CPoint p2)
{
	double dLength = 0.0;
	double dLenX = double(p2.x-p1.x);
	double dLenY = double(p2.y-p1.y);
	double dX = pow(dLenX, 2.0);
	double dY = pow(dLenY, 2.0);
	dLength = sqrt(dX + dY);
	return dLength;
}

bool CImageProcess::GetAllPoint(CxImage *img, vector<CPoint> &v_allPoints,  CPoint pStart, int nThreshold, int &nCount)
{
	if (v_allPoints.size() > 1000)
		return FALSE;

	if (pStart.x < 0 || pStart.y <0)
		return false;

	if (pStart.x > img->GetWidth()-1 || pStart.y >img->GetHeight()-1)
		return false;

	if (GETGRAYVALUE1(img, pStart.x, pStart.y) > nThreshold)
		return FALSE;

	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();

	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
		return FALSE;

	if (v_allPoints.size() >0 )
	{
		v_allPoints.push_back(pStart);
		if (v_allPoints.size() > 1000)
			return FALSE;

		nCount++;
	}
	else 
	{
		v_allPoints.push_back(pStart);
		if (v_allPoints.size() > 1000)
			return FALSE;

		nCount++;
	}

	img->info.pImage[(nImgHeight-1-pStart.y)*BYTESPERLINE(nImgWidth, 8) + pStart.x] = 255;
	
	//递归搜索	
	CPoint pStartTmp[8];
	for (int i=0; i<8; i++)
		pStartTmp[i] = pStart;

	pStartTmp[0].x -= 1;
	pStartTmp[1].x += 1;
	pStartTmp[2].y -= 1;
	pStartTmp[3].y += 1;

	pStartTmp[4].x -= 1;
	pStartTmp[4].y -= 1;

	pStartTmp[5].x -= 1;
	pStartTmp[5].y += 1;

	pStartTmp[6].x += 1;
	pStartTmp[6].y -= 1;

	pStartTmp[7].x += 1;
	pStartTmp[7].y += 1;

	bool bFind = FALSE;
	for (int i=0; i<8; i++)
	{
		if (pStartTmp[i].x < 0 || pStartTmp[i].y < 0)
			continue;

		if (pStartTmp[i].x > nImgWidth-1 || pStartTmp[i].y > nImgHeight-1)
			continue;

		if (GETGRAYVALUE1(img, pStartTmp[i].x, pStartTmp[i].y) > nThreshold)
			continue;

		if (!GetAllPoint(img, v_allPoints, pStartTmp[i], nThreshold, nCount))
			continue;
	}

	return TRUE;
}

bool  CImageProcess::GetRect(vector<CPoint> v_allPoints, CRect &dstRect)
{
	if (v_allPoints.size() <= 5)
		return FALSE;

	int nX[2] = {v_allPoints[0].x, v_allPoints[0].x};
	int nY[2] = {v_allPoints[0].y, v_allPoints[0].y};
	for (vector<CPoint>::iterator it = v_allPoints.begin(); it != v_allPoints.end(); it++)
	{
		CPoint p1 = *it;
		if (nX[0] > p1.x)
			nX[0] = p1.x;

		if (nX[1] < p1.x)
			nX[1] = p1.x;

		if (nY[0] > p1.y)
			nY[0] = p1.y;

		if (nY[1] < p1.y)
			nY[1] = p1.y;
	}

	dstRect.SetRect(CPoint(nX[0], nY[0]), CPoint(nX[1]+1, nY[1]+1));
	return TRUE;
}



bool  CImageProcess::GetValidRect(int nType, const CxImage *img, vector<CRect> &v_allRects, int nThreshold, CSize dstSize)
{
	int nWidth = img->GetWidth();
	int nHeight = img->GetHeight();
	CxImage *img1 = new CxImage();
	img1->Copy(*img);
	img1->Threshold(nThreshold);
	img1->GrayScale();
	vector<CPoint> v_allPoints;
	vector<OMRRECT> v_tempRects;
	GetChainContour(*img1, nThreshold, dstSize, v_tempRects);

	if (v_tempRects.size() == 0)
	{
		delete img1;
		img1 = NULL;
		return FALSE;
	}
	
	//将现有矩形排序
	for (vector<OMRRECT>::iterator it1 = v_tempRects.begin(); it1 != v_tempRects.end(); it1++)
	{
		//将高度差在定义的高度范围内从左至右依次排序
		for (vector<OMRRECT>::iterator it2 = it1 +1; it2 != v_tempRects.end(); it2++)
		{
			OMRRECT rect1 = *it1;
			OMRRECT rect2 = *it2;
			CPoint pCenter1 = rect1.rect.CenterPoint();
			CPoint pCenter2 = rect2.rect.CenterPoint();

			if (abs(pCenter1.y - pCenter2.y) >= nEPS)
			{
				if (pCenter1.y > pCenter2.y)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
			else 
			{
				if (pCenter1.x > pCenter2.x)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
		}
	}

	int i = 0;
	for (vector<OMRRECT>::iterator it = v_tempRects.begin(); it != v_tempRects.end(); it++)
	{
		//存储
		if (i+2 > v_tempRects.size()-1)
		{
			break;
		}
		i++;
		CRect dstRect;
		OMRRECT rect[3] = {*it, *(it+1), *(it+2)};
		if (abs(rect[0].rect.CenterPoint().y - rect[1].rect.CenterPoint().y) > nEPS ||
			abs(rect[0].rect.CenterPoint().y - rect[2].rect.CenterPoint().y) > nEPS)
			continue;
		else 
		{
			int nLen1 = rect[1].rect.CenterPoint().x - rect[0].rect.CenterPoint().x;
			int nLen2 = rect[2].rect.CenterPoint().x - rect[1].rect.CenterPoint().x;
			//TRACE("%d, %d\n", nLen1, nLen2);
			if (abs(nLen1 - nLen2) > min(nEPS, 5))
				continue;
			else 
			{
				int nRectWidth = rect[2].rect.BottomRight().x - rect[0].rect.TopLeft().x;
				int nRectHeigt = rect[2].rect.BottomRight().y - rect[0].rect.TopLeft().y;

				if (abs(nRectWidth - dstSize.cx) <= max(nEPS, 5) &&
					abs(nRectHeigt - dstSize.cy) <= max(nEPS, 5) )
				{
					int nTop  = min(min(rect[0].rect.top, rect[1].rect.top), rect[2].rect.top);
					int nLeft = min(min(rect[0].rect.left, rect[1].rect.left), rect[2].rect.left);
					int nBottom = max(max(rect[0].rect.bottom, rect[1].rect.bottom), rect[2].rect.bottom);
					int nRight =  max(max(rect[0].rect.right, rect[1].rect.right), rect[2].rect.right);;

					dstRect.SetRect(CPoint(nLeft, nTop), CPoint(nRight, nBottom));
					/*if (abs(rect[2].nCount - rect[0].nCount) > max(2*nEPS, 20))
						continue;*/

					//判断是否有边框 2016.06.16
					CxImage cropImg1, cropImg2, cropImg3;
					((CxImage *)img)->Crop(rect[2].rect, &cropImg1);
					((CxImage *)img)->Crop(rect[0].rect, &cropImg2);
					//((CxImage *)img)->Crop(rect[1].rect, &cropImg3);
					//cropImg1.Save(L"D:\\test2_old.jpg", CXIMAGE_FORMAT_JPG);
					GetThinImage(&cropImg1, nThreshold, 3);
					GetThinImage(&cropImg2, nThreshold, 3);
					int nLen1 = 0;
					if (!GetVLinelen(cropImg1, nThreshold, nLen1))
						continue;

					int nLen2 = 0;
					if (!GetVLinelen(cropImg2, nThreshold, nLen2))
						continue;

					if (abs(nLen1 - nLen2) > max(nEPS, 5))
						continue;

					if (nLen1 < dstSize.cy/2 || nLen2 < dstSize.cy/2)
						continue;

					if (abs(nLen1 - dstSize.cy) > nEPS)
						continue;

					if (abs(nLen2 - dstSize.cy) > nEPS)
						continue;

					v_allRects.push_back(dstRect);
				
					if (i+2 > v_tempRects.size()-1)
					{
						break;
					}
					else 
					{ 
						it += 2;
						i += 2;
					}
				}
			}
		}
	}
	
	if (v_allRects.size() == 0)
	{
		delete img1;
		img1 = NULL;
		return FALSE;
	}

	delete img1;
	img1 = NULL;

	return TRUE;
}

bool CImageProcess::GetEnclosedRects(int nType, CxImage *img, vector<CRect> &v_allRects, int nThreshold, CSize dstSize)
{
	int nWidth  = img->GetWidth();
	int nHeight = img->GetHeight();

	vector<CRect> v_tempRects;

	GetChainContour_Encolsed(*img, nThreshold, dstSize, v_tempRects);
	if (v_tempRects.size() == 0)
		return FALSE;

	//按中心点进行排序
	for (vector<CRect>::iterator it1 = v_tempRects.begin(); it1 != v_tempRects.end(); it1++)
	{
		//将高度差在定义的高度范围内从左至右依次排序
		for (vector<CRect>::iterator it2 = it1 +1; it2 != v_tempRects.end(); it2++)
		{
			CRect rect1 = *it1;
			CRect rect2 = *it2;
			CPoint pCenter1 = rect1.CenterPoint();
			CPoint pCenter2 = rect2.CenterPoint();

			if (abs(pCenter1.y - pCenter2.y) >= dstSize.cy)
			{
				if (pCenter1.y > pCenter2.y)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
			else 
			{
				if (pCenter1.x > pCenter2.x)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
		}
	}

	v_allRects = v_tempRects;
	if (v_allRects.size() <= 0)
		return FALSE;

	return TRUE;
}

BOOL CImageProcess::GetRectFromPoint(CRect &dstRect, CPoint pStart, CRect rectArea, CSize dstsize,const CxImage *img, int nThreshold)
{
	int nGrayValue;
	int nImgWidth = img->GetWidth();
	int nImgHeight  = img->GetHeight();
	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
		return FALSE;

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);
	if (nGrayValue >  nThreshold)
		return FALSE;

	CPoint pTopExtream[2];
	CPoint pBottomExtream[2];
	for (int i=0; i<2; i++)
	{
		pTopExtream[i]    = pStart;
		pBottomExtream[i] = pStart;
	}

	CPoint pStartTmp = pStart;
	int nFind = 0;
	while (nFind <= 5)
	{
		CPoint pTmp[2];
		if (GetHLine(pTmp, pStartTmp, rectArea, dstsize, img, nThreshold, 5))
		{
			nFind = 0;
			for (int i=0; i<2; i++)
			{
				pTopExtream[i] = pTmp[i];
			}

			pStartTmp.x = (pTopExtream[0].x + pTopExtream[1].x)/2;
		}
		else
		{
			nFind++;
		}
		
		pStartTmp.y--; 
		if (pStartTmp.y < rectArea.TopLeft().y)
			break;
	}

	pStartTmp = pStart;
	nFind = 0;
	while (nFind <= 5)
	{
		CPoint pTmp[2];
		if (GetHLine(pTmp, pStartTmp, rectArea, dstsize, img, nThreshold, 5))
		{
			nFind = 0;
			for (int i=0; i<2; i++)
			{
				pBottomExtream[i] = pTmp[i];
			}
			pStartTmp.x = (pBottomExtream[0].x + pBottomExtream[1].x)/2;
		}
		else
		{
			nFind++;
		}

		pStartTmp.y++; 
		if (pStartTmp.y > rectArea.BottomRight().y-1)
			break;
	}

	CPoint pTopleft;
	CPoint pBottomright;
	pTopleft.x = min(pTopExtream[0].x, pBottomExtream[0].x);
	pTopleft.y = pTopExtream[0].y;
	pBottomright.x = max(pTopExtream[1].x, pBottomExtream[1].x);
	pBottomright.y = pBottomExtream[1].y;

	CRect rectTmp;
	rectTmp.SetRect(pTopleft, pBottomright);
	int nWidth = rectTmp.Width();
	int nHeight = rectTmp.Height();

	if (nWidth <= 0 || nHeight <= 0)
		return FALSE;
	
	if (abs(nWidth - dstsize.cx) <= nEPS &&
		abs(nHeight - dstsize.cy) <= nEPS)
	{
		//判断矩形有效性
		CRect rectExpand; 
		int nExpand = min(rectTmp.Width()/2, rectTmp.Height()/2);
		if (nExpand == 0)
		{
			//double dBalckDensity = GetDensity(img, rectTmp);
			//TRACE("考号竖向大定位点密度:%f", dBalckDensity);
			return FALSE;
		}
		CPoint p1 = CPoint(rectTmp.TopLeft().x-nExpand, rectTmp.TopLeft().y - nExpand);
		if (p1.x < 0)
			p1.x = 0;

		if (p1.y < 0)
			p1.y = 0;

		CPoint p2 = CPoint(p1.x + rectTmp.Width() + 2*nExpand, p1.y + rectTmp.Height() + 2*nExpand);

		if (p1.x < 0)
			p1.x =0;

		if (p1.y < 0)
			p1.y = 0;

		if (p2.x > nImgWidth-1)
			p2.x = nImgWidth-1;

		if (p2.y > nImgHeight-1)
			p2.y = nImgHeight-1;

		//计算区域内白点密度
		rectExpand.SetRect(p1, p2);
		int nCount = 0;
		for (int i=p1.x; i<p1.x + rectExpand.Width(); i++)
		{
			for (int j=p1.y; j<p1.y + rectExpand.Height(); j++)
			{
				if (i < 0 || j< 0)
					continue;

				if (i>img->GetWidth()-1 || j>img->GetHeight()-1)
					continue;

				nGrayValue = GETGRAYVALUE1(img, i, j);//img->GetPixelGray(i, nImgHeight-1-j); 
				if (nGrayValue > nThreshold && !IsPointInRect(CPoint(i, j), rectTmp))
				{
					nCount++;
				}
			}
		}
		double dDensity = double(nCount+rectTmp.Width()*rectTmp.Height())/double(rectExpand.Width()*rectExpand.Height());
		if (dDensity >= 0.75) //防止时裁切黑边 2015.12.29
		{
			dstRect = rectTmp;
			return TRUE;
		}

	}
	return FALSE;
}

BOOL CImageProcess::GetHLine(CPoint pExtream[2], CPoint pStart, CRect rectArea, CSize dstSize,const CxImage *img, int nThreshold, int nMaxGapLen)
{
	CPoint pTmp[2];
	int nGrayValue;
	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();

	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
		return FALSE;

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);
	if (nGrayValue > nThreshold)
		return FALSE;

	for (int i=0; i<2; i++)
	{
		pTmp[i] = pStart;
	}

	CPoint pTmpExtream = pStart;
	int nFind = 0;
	while(nFind <= nMaxGapLen)
	{
		pTmpExtream.x -= 1;
		if (pTmpExtream.x < rectArea.TopLeft().x || pTmpExtream.x > rectArea.BottomRight().x)
			break;

		if (pTmpExtream.y < rectArea.TopLeft().y || pTmpExtream.y > rectArea.BottomRight().y)
			break;


		if (pTmpExtream.x < 0 || pTmpExtream.y < 0)
		{
			nFind++;
			break;
		}

		if (pTmpExtream.x > nImgWidth-1 || pTmpExtream.y > nImgHeight-1)
		{
			nFind++;
			continue;
		}

		nGrayValue = GETGRAYVALUE1(img, pTmpExtream.x, pTmpExtream.y);//img->GetPixelGray(pTmpExtream.x, nImgHeight-1-pTmpExtream.y);
		if (nGrayValue <= nThreshold)
		{
			nFind = 0;
			pTmp[0] = pTmpExtream;
		}
		else 
			nFind++;
	}

	pTmpExtream = pStart;
	nGrayValue  = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);

	nFind = 0;
	while(nFind <= nMaxGapLen)
	{
		pTmpExtream.x += 1;
		if (pTmpExtream.x < rectArea.TopLeft().x || pTmpExtream.x > rectArea.BottomRight().x)
			break;

		if (pTmpExtream.y < rectArea.TopLeft().y || pTmpExtream.y > rectArea.BottomRight().y)
			break;

		if (pTmpExtream.x < 0 || pTmpExtream.y < 0)
		{
			nFind++;
			continue;
		}

		if (pTmpExtream.x > nImgWidth-1 || pTmpExtream.y > nImgHeight-1)
		{
			nFind++;
			break;
		}

		nGrayValue = GETGRAYVALUE1(img, pTmpExtream.x, pTmpExtream.y);//img->GetPixelGray(pTmpExtream.x, nImgHeight-1-pTmpExtream.y);
		if (nGrayValue <= nThreshold)
		{
			nFind = 0;
			pTmp[1] = pTmpExtream;
		}
		else 
			nFind++;
	}

	int nLen = pTmp[1].x - pTmp[0].x;
	if (abs(nLen - dstSize.cx) <= nEPS)
	{
		for (int i=0; i<2; i++)
		{
			pExtream[i] = pTmp[i];
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CImageProcess::GetHLine_OMR(CPoint pExtream[2], CPoint pStart, CRect rectArea, CSize dstSize,const CxImage *img, int nThreshold, int nMaxGapLen)
{
	CPoint pTmp[2];
	int nGrayValue;
	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();

	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
		return FALSE;

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);
	if (nGrayValue > nThreshold)
		return FALSE;

	for (int i=0; i<2; i++)
	{
		pTmp[i] = pStart;
	}

	CPoint pTmpExtream = pStart;
	int nFind = 0;
	while(nFind <= nMaxGapLen)
	{
		pTmpExtream.x -= 1;
		if (pTmpExtream.x < rectArea.TopLeft().x || pTmpExtream.x > rectArea.BottomRight().x)
			break;

		if (pTmpExtream.y < rectArea.TopLeft().y || pTmpExtream.y > rectArea.BottomRight().y)
			break;


		if (pTmpExtream.x < 0 || pTmpExtream.y < 0)
		{
			nFind++;
			break;
		}

		if (pTmpExtream.x > nImgWidth-1 || pTmpExtream.y > nImgHeight-1)
		{
			nFind++;
			continue;
		}

		nGrayValue = GETGRAYVALUE1(img, pTmpExtream.x, pTmpExtream.y);//img->GetPixelGray(pTmpExtream.x, nImgHeight-1-pTmpExtream.y);
		if (nGrayValue <= nThreshold)
		{
			nFind = 0;
			pTmp[0] = pTmpExtream;
		}
		else 
			nFind++;
	}

	pTmpExtream = pStart;
	nGrayValue  = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);

	nFind = 0;
	while(nFind <= nMaxGapLen)
	{
		pTmpExtream.x += 1;
		if (pTmpExtream.x < rectArea.TopLeft().x || pTmpExtream.x > rectArea.BottomRight().x)
			break;

		if (pTmpExtream.y < rectArea.TopLeft().y || pTmpExtream.y > rectArea.BottomRight().y)
			break;

		if (pTmpExtream.x < 0 || pTmpExtream.y < 0)
		{
			nFind++;
			continue;
		}

		if (pTmpExtream.x > nImgWidth-1 || pTmpExtream.y > nImgHeight-1)
		{
			nFind++;
			break;
		}

		nGrayValue = GETGRAYVALUE1(img, pTmpExtream.x, pTmpExtream.y);//img->GetPixelGray(pTmpExtream.x, nImgHeight-1-pTmpExtream.y);
		if (nGrayValue <= nThreshold)
		{
			nFind = 0;
			pTmp[1] = pTmpExtream;
		}
		else 
			nFind++;
	}

	int nLen = pTmp[1].x - pTmp[0].x;
	if (nLen >= dstSize.cx/2)
	{
		for (int i=0; i<2; i++)
		{
			pExtream[i] = pTmp[i];
		}
		return TRUE;
	}
	return FALSE;


}


void CImageProcess::GetRatio(NEWPOINT pLocal[3], NEWPOINT pCenter, double &dHRatio, double &dVRatio)
{
	int nEpsX[2], nEpsY[2];
	nEpsX[0] = pLocal[1].nX - pLocal[0].nX;
	nEpsY[0] = pLocal[1].nY - pLocal[0].nY;

	nEpsX[1] = pLocal[2].nX - pLocal[0].nX;
	nEpsY[1] = pLocal[2].nY - pLocal[0].nY;

	int nX = pCenter.nX - pLocal[0].nX;
	int nY = pCenter.nY - pLocal[0].nY;

   dHRatio = double(nEpsY[1]*nX - nEpsX[1]*nY)/double(nEpsX[0]*nEpsY[1] - nEpsY[0]*nEpsX[1]);
   dVRatio = double(-nEpsY[0]*nX + nEpsX[0]*nY)/double(nEpsX[0]*nEpsY[1] - nEpsY[0]*nEpsX[1]);
}


void CImageProcess::GetNewPoint(NEWPOINT pCurLocal[3], double dHRatio, double dVRatio, NEWPOINT &pCenter)
{
	int nEpsX[2], nEpsY[2];
	nEpsX[0] = pCurLocal[1].nX - pCurLocal[0].nX;
	nEpsY[0] = pCurLocal[1].nY - pCurLocal[0].nY;

	nEpsX[1] = pCurLocal[2].nX - pCurLocal[0].nX;
	nEpsY[1] = pCurLocal[2].nY - pCurLocal[0].nY;

	pCenter.nX = dHRatio*double(nEpsX[0]) + dVRatio*double(nEpsX[1]) + pCurLocal[0].nX;
	pCenter.nY = dHRatio*double(nEpsY[0]) + dVRatio*double(nEpsY[1]) + pCurLocal[0].nY;
}

void CImageProcess::GetAllRectsFromRects(vector<CRect>&v_dstRects, CRect rect[2], CRect rectArea, CSize dstSize, CxImage *img, int nThreshold)
{
	CRect dstRectTmp;
	bool bRes = TRUE;
	CRect rectsTmp[2];
	for (int i=0; i<2; i++)
	{
		rectsTmp[i] = rect[i];
	}

	while (bRes)
	{
		bRes = GetRectFromTwoRects(dstRectTmp, rectsTmp, rectArea, dstSize, img, nThreshold);
		if (bRes)
		{
			rectsTmp[0] = dstRectTmp;
			v_dstRects.push_back(dstRectTmp);
		}
		else 
			break;
	}
}

BOOL CImageProcess::GetRectFromTwoRects(CRect &dstRect, CRect rect[2], CRect rectArea, CSize dstSize, CxImage *img, int nThreshold)
{
	CPoint pCenter[2];
	for (int i=0; i<2; i++)
	{
		CPoint p1 = rect[i].TopLeft();
		int nWidth = rect[i].Width();
		int nHeight = rect[i].Height();
		pCenter[i] = CPoint(p1.x+nWidth/2, p1.y + nHeight/2);
	}

	//计算单位向量
	double dLenX = double(pCenter[1].x-pCenter[0].x);
	double dLenY = double(pCenter[1].y-pCenter[0].y);
	double dX = pow(dLenX, 2.0);
	double dY = pow(dLenY, 2.0);
	double dTmp = sqrt(dX + dY);
	double dTmpX = dLenX/dTmp;
	double dTmpY = dLenY/dTmp;
	double dMaxLen = dTmp;

	//沿线搜索黑点
	CPoint pStart = pCenter[0];
	int nCount = 0;
	//int ii =0;
	while (dTmp > 5.0)
	{
		nCount += 2;
		double dTmp1 = double(nCount)*dTmpX;
		double dTmp2 = double(nCount)*dTmpY;

		pStart.x = pCenter[0].x + (int)dTmp1;
		pStart.y = pCenter[0].y + (int)dTmp2;
		dTmp = GetLength(pStart, pCenter[1]);

		if (IsPointInRect(pStart, rect[1]))
			break;

		if (IsPointInRect(pStart, rect[0]))
			continue;

		int nImgWidth  = img->GetWidth();
		int nImgHeight = img->GetHeight();

		if (pStart.x < 0 || pStart.y < 0)
			continue;

		if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
			continue;

		int nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);
		if (nGrayValue <= nThreshold)
		{
			CRect rectTmp;
			if (GetRectFromPoint(rectTmp, pStart, rectArea, dstSize, img, nThreshold))
			{
				//判断获取的矩形是有重叠
				if (IsPointInRect(rectTmp.CenterPoint(), rect[1]))
					break;
				else if (IsPointInRect(rectTmp.CenterPoint(), rect[0]))
				{
					//ii++;
					continue;
				}

				dstRect = rectTmp;
				return TRUE;
			}
		}
	}
	return FALSE;
}

//CString CImageProcess::GetOcrResult(CxImage *img)
//{
//	CString strRes = _T("");
//	img->IncreaseBpp(24);
//	HBITMAP hBmp = img->MakeBitmap(NULL);
//	if (!hBmp)
//		return strRes;
//
//#ifdef ZYJ_ICR > 0
//	//图像结构体转化
//	re_hbitmap2image(hBmp, &m_rel.image);
//	unsigned char RejectLevel=0;
//	char ResStr[256], Result[32];
//	if (rel_do(&m_rel)!=RE_SUCCESS)
//		return strRes;
//
//	//rec_collect_kernel(m_rel);
//	rel_textline(&m_rel, ResStr, MAX_CHARS, RejectLevel, '~', 0);
//	re_ClearError();
//	DeleteObject(hBmp);
//	//rel_freeimages(&m_rel);
//	USES_CONVERSION;
//	strRes = A2T(ResStr);
//#endif
//	return strRes;
//}


CRect CImageProcess::NewRect2Rect(NEWRECT newRect)
{
	CRect rect;
	rect.top = newRect.pTopLeft.nY;
	rect.bottom = newRect.pBottomRight.nY;
	rect.left = newRect.pTopLeft.nX;
	rect.right = newRect.pBottomRight.nX;
	return rect;
}

//
//bool CImageProcess::GetTitleRect(CxImage *img, vector<TITLENO> v_modelTitleNo, int nThrshold, vector<TITLENO> &v_dstTitleNo)
//{
//	//分割字符
//	int nImgWidth = img->GetWidth();
//	int nImgHeight = img->GetHeight();
//	CxImage imgTmp;
//	CRect rectArea;
//	rectArea.SetRect(0, 0, nImgWidth, nImgHeight);
//	img->Crop(rectArea, &imgTmp);
//	vector<TITLENO> v_titleNoTmp;
//
//	USES_CONVERSION;
//	for (int i=0; i<nImgHeight; i++)
//	{
//		for (int j=0; j<nImgWidth; j++)
//		{
//			int nGrayValue = GETGRAYVALUE2(imgTmp, j, i);
//			CPoint pStart = CPoint(j, i);
//			vector<CPoint> v_allPoints;
//			if (nGrayValue <= nThrshold)
//			{
//				int nCount = 0;
//				if (GetAllPoint(&imgTmp, v_allPoints, pStart, nThrshold, nCount))
//				{
//					CRect rect1;
//					GetRect(v_allPoints, rect1);
//
//					if (rect1.Width()*rect1.Height() < 5)
//						continue;
//
//					CxImage img1;
//					img->Crop(rect1, &img1);
//					CString strRes = GetOcrResult(&img1);
//					strRes.Trim();
//
//					if (strRes.Find('~') >= 0 || strRes.GetLength() >= 2 || strRes.IsEmpty())
//					{
//						int nTitleNo1 = atoi(T2A(strRes));
//						NEWRECT rectLocal;
//						rectLocal.SetRect(rect1.left, rect1.top, rect1.Width(), rect1.Height());
//
//						TITLENO titleNoTmp;
//						titleNoTmp.nTitleNo = nTitleNo1;
//						titleNoTmp.rectLocal = rectLocal;
//
//						v_titleNoTmp.push_back(titleNoTmp);
//					}
//				}
//
//			}
//		}
//	}
//	
//	//从上之下 从左至右排序
//	for (vector<TITLENO>::iterator it = v_titleNoTmp.begin(); it != v_titleNoTmp.end(); it++)
//	{
//		for (vector<TITLENO>::iterator it2 = it+1; it2 != v_dstTitleNo.end(); it2++)
//		{
//			TITLENO titleNoTmp1 = *it;
//			TITLENO titleNoTmp2 = *it2;
//			int nX1 = titleNoTmp1.rectLocal.CenterNewPoint().nX;
//			int nY1 = titleNoTmp1.rectLocal.CenterNewPoint().nY;
//
//			int nX2 = titleNoTmp2.rectLocal.CenterNewPoint().nX;
//			int nY2 = titleNoTmp2.rectLocal.CenterNewPoint().nY;
//			
//			if (abs(nY2 - nY1) > nEPS )
//			{
//				if (nY1 > nY2)
//				{
//					*it  = titleNoTmp2;
//					*it2 = titleNoTmp1;
//				}
//			}
//			else 
//			{
//				if (nX1 > nX2)
//				{
//					*it  = titleNoTmp2;
//					*it2 = titleNoTmp1;
//				}
//			}
//		}
//	}
//
//	vector<TITLENO> v_dstTitleTmp;
//	bool bFind = FALSE;
//
//	//判断有效性 2016.01.26
//	for (vector<TITLENO>::iterator it1 = v_titleNoTmp.begin();it1 != v_titleNoTmp.end(); it1++)
//	{
//		v_dstTitleTmp.clear();
//		TITLENO titleNoTmp1 = v_modelTitleNo[0];
//		TITLENO titleNoTmp2 = *it1;
//		
//		int nMoveX = 0;
//		int nMoveY = 0;
//		if (!bFind)
//		{
//			if (titleNoTmp1.nTitleNo == titleNoTmp2.nTitleNo)
//			{
//				bFind = TRUE;
//			
//				//计算两图向量差
//				nMoveX = titleNoTmp2.rectLocal.CenterNewPoint().nX - titleNoTmp1.rectLocal.CenterNewPoint().nX;
//				nMoveY = titleNoTmp2.rectLocal.CenterNewPoint().nY - titleNoTmp1.rectLocal.CenterNewPoint().nY;
//			}
//		}
//		else 
//		{
//			vector<TITLENO> v_titleNoTmp2 = v_titleNoTmp;
//			for (vector<TITLENO>::iterator it2 = v_modelTitleNo.begin(); it2 != v_modelTitleNo.end(); it2++)
//			{
//				bool bFindTmp1 = FALSE;
//				TITLENO titleNoTmp3 = *it2;
//				NEWPOINT p1;
//				p1.setPoint(titleNoTmp3.rectLocal.CenterNewPoint().nX + nMoveX, titleNoTmp3.rectLocal.CenterNewPoint().nY + nMoveY);
//				for (int i=0; i<v_titleNoTmp2.size(); i++)
//				{
//					CPoint p2 = CPoint(p1.nX, p1.nY);
//					CRect rectCur = NewRect2Rect(v_titleNoTmp2[i].rectLocal);
//					if (GetLength(p2, rectCur.CenterPoint()) <= nEPS)
//					{
//						if (titleNoTmp3.nTitleNo == v_titleNoTmp2[i].nTitleNo)
//						{
//							bFindTmp1 = TRUE;
//							v_dstTitleTmp.push_back(v_titleNoTmp2[i]);
//							v_titleNoTmp2.erase(v_titleNoTmp2.begin()+i);
//							break;
//						}
//					}
//				}
//
//				if (!bFindTmp1)
//				{
//					bFind = FALSE;
//					break;
//				}
//			}
//
//		}
//		
//	}
//
//	if (v_dstTitleTmp.size() == v_modelTitleNo.size())
//	{
//		v_dstTitleNo = v_dstTitleTmp;
//		return TRUE;
//	}
//
//	return FALSE;
//}

BOOL CImageProcess::GetTriAngleLocalPoint(CxImage *img, const CSize dstSize, const int nThreshold, NEWTRIANGLE &dstTriangle)
{
	int nImgHeight = img->GetHeight();
	int nImgWidth = img->GetWidth();
	
	vector<CPoint> v_allPoints;
	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			if (GETGRAYVALUE1(img, i, j) <= nThreshold)
			{
				if (v_allPoints.size() > 0)
					v_allPoints.clear();

				CPoint pStart = CPoint(i, j);
				int nCount = 0;
				GetAllPoint(img, v_allPoints, pStart, nThreshold, nCount);
				CRect dstRect;
				GetRect(v_allPoints, dstRect);

				if (abs(dstSize.cx - dstRect.Width()) > nEPS ||
					abs(dstSize.cy - dstRect.Height()) > nEPS)
					continue;

				if (v_allPoints.size() >= dstRect.Width()*dstRect.Height()*0.4 &&
					v_allPoints.size() <= dstRect.Width()*dstRect.Height()*0.6)
				{
					//三角形
					//获取中心
					CPoint pCenter;
					if (GetPointsCenter(v_allPoints, pCenter))
					{
						//算出最终三角形的各个顶点 2016.02.15
						if (pCenter.x < dstRect.CenterPoint().x &&
							pCenter.y < dstRect.CenterPoint().y)
						{
							dstTriangle.pRightAngle.setPoint(dstRect.left, dstRect.top);
							dstTriangle.pAcuteAngleH.setPoint(dstRect.right, dstRect.top); 
							dstTriangle.pAcuteAngleV.setPoint(dstRect.left, dstRect.bottom);
						}
						else if (pCenter.x < dstRect.CenterPoint().x &&
								 pCenter.y > dstRect.CenterPoint().y)
						{
							dstTriangle.pRightAngle.setPoint(dstRect.left, dstRect.bottom);
							dstTriangle.pAcuteAngleH.setPoint(dstRect.right, dstRect.bottom);
							dstTriangle.pAcuteAngleV.setPoint(dstRect.left, dstRect.top);

						}
						else if (pCenter.x > dstRect.CenterPoint().x &&
							     pCenter.y < dstRect.CenterPoint().y)
						{
							dstTriangle.pRightAngle.setPoint(dstRect.right, dstRect.top);
							dstTriangle.pAcuteAngleH.setPoint(dstRect.left, dstRect.top);
							dstTriangle.pAcuteAngleV.setPoint(dstRect.right, dstRect.bottom);
						}
						else if (pCenter.x > dstRect.CenterPoint().x &&
							     pCenter.y > dstRect.CenterPoint().y)
						{
							dstTriangle.pRightAngle.setPoint(dstRect.right, dstRect.bottom);
							dstTriangle.pAcuteAngleH.setPoint(dstRect.left, dstRect.bottom);
							dstTriangle.pAcuteAngleV.setPoint(dstRect.right, dstRect.top);
						}
						else 
							continue;

						return TRUE;
					}
				}
			}
		}
	}
	

	return TRUE;
}


BOOL CImageProcess::GetPointsCenter(vector<CPoint> v_allPoints, CPoint &pCenter)
{
	int nSize = v_allPoints.size();
	if (nSize <= 0)
		return FALSE;

	int nTotal[2] = {0, 0};
	for (int i=0; i<nSize; i++)
	{
		nTotal[0] += v_allPoints[i].x;
		nTotal[1] += v_allPoints[i].y;
	}

	pCenter.x = nTotal[0]/nSize;
	pCenter.y = nTotal[1]/nSize;
	return TRUE;
}

void CImageProcess::GetBlackCount(const CxImage *img, CRect rectArea, int nThreshold, int &nCount)
{
	CxImage *img1 = (CxImage *)img;
	/*img1->Save(L"C:\\test.jpg", CXIMAGE_FORMAT_JPG);*/
	int nWidth = img->GetWidth();
	int nHeight = img->GetHeight();
	nCount = 0;
	int nLeft = max(rectArea.TopLeft().x, 0);
	int nRight = min(rectArea.BottomRight().x, nWidth);
	int nTop = max(rectArea.TopLeft().y, 0);
	int nBottom = min(rectArea.BottomRight().y, nHeight);

	for (int i=nLeft; i<=nRight; i++)
	{
		for (int j=nTop; j<=nBottom; j++)
		{
			if (GETGRAYVALUE1(img, i, j) <= nThreshold)
			{
				nCount++;
			}
		}
	}
}


bool CImageProcess::GetRectPoint(CRect &dstRect,
								 const CxImage *srcImg,
								 const int nIndex,
								 const CRect rectArea,
								 const CxImage *cropImg,
								 const int nThrshold,
								 const CSize minSize,
								 const CSize maxSize)
{	
	int nSearchLen = 5;
	int nCropWidth = cropImg->GetWidth();
	int nCropHeight = cropImg->GetHeight();
	CRect dstRectTmp;
	bool bExchange = TRUE; //相互切换标志
	CPoint pStartRect; 
	CPoint pEndRect;
	if (nIndex == 0)
	{
		pStartRect = rectArea.TopLeft();
		pEndRect = pStartRect;
	}
	else if (nIndex == 1)
	{
		CPoint p1 = rectArea.TopLeft();
		CPoint p2 = rectArea.BottomRight();
		pStartRect = CPoint(p2.x, p1.y);
		pEndRect = pStartRect;
	}
	else if (nIndex == 2)
	{
		CPoint p1 = rectArea.TopLeft();
		CPoint p2 = rectArea.BottomRight();
		pStartRect = CPoint(p1.x, p2.y);
		pEndRect  = pStartRect;
	}
	else if (nIndex == 3)
	{
		CPoint p2 = rectArea.BottomRight();
		pStartRect = p2;
		pEndRect = p2;
	}
	
	while(1)
	{
		bool bStop[2] = {FALSE, FALSE};
		if (nIndex == 0)
		{
			pStartRect.x += nSearchLen;
			
			if (pStartRect.x >= rectArea.BottomRight().x-1)
			{
				pStartRect.x = rectArea.BottomRight().x-1;
				pStartRect.y += nSearchLen;
			}

			pEndRect.y += nSearchLen;

			if (pEndRect.y >= rectArea.BottomRight().y-1)
			{
				pEndRect.y = rectArea.BottomRight().y -1;
				pEndRect.x += nSearchLen;
			}
		}
		else if (nIndex == 1)
		{
			pStartRect.x -= nSearchLen;
			pEndRect.y += nSearchLen;

			if (pStartRect.x <= rectArea.TopLeft().x+1)
			{
				pStartRect.x = rectArea.TopLeft().x + 1;
				pStartRect.y += nSearchLen;
			}

			if (pEndRect.y >= rectArea.BottomRight().y - 1)
			{
				pEndRect.y = rectArea.BottomRight().y - 1;
				pEndRect.x -= nSearchLen;
			}
		}
		else if (nIndex == 2)
		{
			pStartRect.x += nSearchLen;
			
			if (pStartRect.x >= rectArea.BottomRight().x-1)
			{
				pStartRect.x = rectArea.BottomRight().x-1;
				pStartRect.y -= nSearchLen;
			}

			pEndRect.y -= nSearchLen;

			if (pEndRect.y <= rectArea.TopLeft().y)
			{
				pEndRect.y = rectArea.TopLeft().y;
				pEndRect.x += nSearchLen;
			}
		}
		else if (nIndex == 3)
		{
			pStartRect.x -= nSearchLen;
			pEndRect.y -= nSearchLen;

			if (pStartRect.x <= rectArea.TopLeft().x)
			{
				pStartRect.x = rectArea.TopLeft().x;
				pStartRect.y -= nSearchLen;
			}

			if (pStartRect.y < rectArea.TopLeft().y)
			{
				pStartRect.y = rectArea.TopLeft().y;
				pStartRect.x -= nSearchLen;
			}
		}
		
		if (!IsPointInRect(pStartRect, rectArea))
			break;

		if (!IsPointInRect(pEndRect, rectArea))
			break;
	
		//在点p1,p2连线上查找
		CPoint pMid = CPoint((pStartRect.x + pEndRect.x)/2, (pStartRect.y + pEndRect.y)/2);//GetMidPoint(pStartRect, pEndRect);
		int nCount[2] = {0, 0};
		while (1)
		{
			CPoint pTmp;
			CRect rect;
			if (nIndex == 0)
			{
				if (bExchange)
				{
					bExchange = FALSE;
					pTmp.x = pMid.x - nCount[0]*nSearchLen;
					pTmp.y = pMid.y + nCount[0]*nSearchLen;
					nCount[0]++;
				}
				else
				{
					bExchange = TRUE;
					pTmp.x = pMid.x + nCount[1]*nSearchLen;
					pTmp.y = pMid.y - nCount[1]*nSearchLen;
					nCount[1]++;
				}

				if (pTmp.x > pStartRect.x || pTmp.y < pStartRect.y)
				{
					break;
				}

				if (pTmp.x < pEndRect.x || pTmp.y > pEndRect.y)
				{
					break;
				}

			}
			else if (nIndex == 1)
			{
				if (bExchange)
				{
					bExchange = FALSE;
					pTmp.x = pMid.x - nCount[0]*nSearchLen;
					pTmp.y = pMid.y - nCount[0]*nSearchLen;
					nCount[0]++;
				}
				else
				{
					bExchange = TRUE;
					pTmp.x = pMid.x + nCount[1]*nSearchLen;
					pTmp.y = pMid.y + nCount[1]*nSearchLen;
					nCount[1]++;
				}

				if (pTmp.x < pStartRect.x || pTmp.y < pStartRect.y)
					break;

				if (pTmp.x > pEndRect.x || pTmp.y > pEndRect.y)
					break;
			}
			else if (nIndex == 2)
			{	
				if (bExchange)
				{
					bExchange = FALSE;
					pTmp.x = pMid.x - nCount[0]*nSearchLen;
					pTmp.y = pMid.y - nCount[0]*nSearchLen;
					nCount[0]++;
				}
				else
				{
					bExchange = TRUE;
					pTmp.x = pMid.x + nCount[1]*nSearchLen;
					pTmp.y = pMid.y + nCount[1]*nSearchLen;
					nCount[1]++;
				}
				
				if (pTmp.x > pStartRect.x || pTmp.y > pStartRect.y)
					break;

				if (pTmp.x < pEndRect.x || pTmp.y < pEndRect.y)
					break;
			} 
			else if (nIndex == 3)
			{
				if (bExchange)
				{
					bExchange = FALSE;
					pTmp.x = pMid.x - nCount[0]*nSearchLen;
					pTmp.y = pMid.y + nCount[0]*nSearchLen;
					nCount[0]++;
				}
				else
				{
					bExchange = TRUE;
					pTmp.x = pMid.x + nCount[1]*nSearchLen;
					pTmp.y = pMid.y - nCount[1]*nSearchLen;
					nCount[1]++;
				}

				if (pTmp.x < pStartRect.x || pTmp.y > pStartRect.y)
					break;

				if (pTmp.x > pEndRect.x || pTmp.y < pEndRect.y)
					break;
			}
			
			bool bFindFirst = FALSE;
			if (pTmp.x  <  0|| pTmp.y < 0)
				break;

			if (pTmp.x > cropImg->GetWidth()-1 || pTmp.y > cropImg->GetHeight()-1)
				break;

			int nGrayValue = GETGRAYVALUE1(cropImg, pTmp.x, pTmp.y);//cropImg->GetPixelGray(pTmp.x, nCropHeight-1-pTmp.y);

			rect = rectArea;

			if (nGrayValue > nThrshold) //搜索白点
			{
				bFindFirst = TRUE;
			     BOOL bRes = FALSE;

				if (nIndex == 0)
				{
					rect.SetRect(pTmp, rectArea.BottomRight());
				}
				else if (nIndex == 1)
				{
					rect.SetRect(CPoint(rectArea.TopLeft().x, pTmp.y), CPoint(pTmp.x, rectArea.BottomRight().y-1));
				}
				else if (nIndex == 2)
				{
					rect.SetRect(CPoint(pTmp.x, rectArea.TopLeft().y), CPoint(rectArea.BottomRight().x, pTmp.y));
				}
				else if (nIndex == 3)
				{
					rect.SetRect(rectArea.TopLeft(), pTmp);
				}


				bRes = GetRectFromRect1(dstRectTmp, rect, cropImg, minSize, maxSize, nIndex, srcImg, nThrshold);
			
				if (bRes)
				{
					/*CPoint pStart1 = dstRectTmp.TopLeft();
					CPoint pEnd1 = dstRectTmp.BottomRight();

					pStart1.x += rectArea.TopLeft().x;
					pStart1.y += rectArea.TopLeft().y;

					pEnd1.x += rectArea.TopLeft().x;
					pEnd1.y += rectArea.TopLeft().y;
					dstRect.SetRect(pStart1, pEnd1);*/
					dstRect = dstRectTmp;
					return TRUE;
				}
			}
			else
			{
			
				if (bFindFirst)
				{
					BOOL bRes = GetRectFromRect1(dstRectTmp, rect, cropImg, minSize, maxSize, nIndex, srcImg, nThrshold);
					if (bRes)
					{
						dstRect = dstRectTmp;
						//获取矩形
						if (nIndex == 0 /*|| nIndex == 1*/)
						{
							//判断正上方是否含有矩形
							CRect rectArea1;
							CPoint p1 = dstRectTmp.CenterPoint();
							CPoint pStart1 = CPoint(p1.x, p1.y - 2*dstRectTmp.Height());

							if (pStart1.x < 0)
								pStart1.x = 0;

							if (pStart1.y < 0)
								pStart1.y = 0;

							CPoint pTopLeft1 = CPoint(pStart1.x - dstRectTmp.Width()/2-10, pStart1.y - dstRectTmp.Height()/2-10);
							CPoint pBottomRight1 = CPoint(pStart1.x + dstRectTmp.Width()/2+ 10, pStart1.y + dstRectTmp.Height()/2+10);

							if (pBottomRight1.x > cropImg->GetWidth()-1)
								pBottomRight1.x = cropImg->GetWidth()-1;

							if (pBottomRight1.y > cropImg->GetHeight()-1)
								pBottomRight1.y = cropImg->GetHeight()-1;
							rectArea1.SetRect(pTopLeft1, pBottomRight1);

							int nGrayValue =  GETGRAYVALUE1(cropImg, pStart1.x, pStart1.y);// cropImg->GetPixelGray(pStart1.x, nCropHeight-1-pStart1.y);
							if (nGrayValue <= nThrshold)
							{
								//int nThreshold = m_nThreshold;
								bool bRes1 = GetRectFromPoint(dstRectTmp, pStart1, rectArea1, CSize(dstRectTmp.Width(), dstRectTmp.Height()), cropImg, nThrshold);
								if (bRes1 && !IsPointInRect(dstRectTmp.CenterPoint(), dstRect))
								{
									/*CPoint pStart1 = dstRectTmp.TopLeft();
									CPoint pEnd1 = dstRectTmp.BottomRight();

									pStart1.x += rectArea.TopLeft().x;
									pStart1.y += rectArea.TopLeft().y;

									pEnd1.x += rectArea.TopLeft().x;
									pEnd1.y += rectArea.TopLeft().y;
									dstRect.SetRect(pStart1, pEnd1);*/

									dstRect = dstRectTmp;
								}
							}
						}
						
						return TRUE;
					}
					else
					{
						bFindFirst = FALSE;
						break;
					}
				}
			}
		}
	}
	return FALSE;
}

BOOL CImageProcess::GetRectFromRect1(CRect &dstRect, 
									 CRect rectArea,
									 const CxImage *img,
									 CSize minSize, 
									 CSize maxSize, 
									 int nIndex,
									 const CxImage *pSrcImg,
									 const int nThreshold)
{
	int imgWidth = img->GetWidth();
	int imgHeight = img->GetHeight();
	
	if (nIndex == 0)
	{
		for (int i=rectArea.TopLeft().y; i<rectArea.BottomRight().y; i++)
		{
			for (int j=rectArea.TopLeft().x; j<rectArea.BottomRight().x; j++)
			{
				CPoint pStart = CPoint(j, i);
				if (pStart.x < 0 || pStart.y  <0)
					continue;

				if (pStart.x  > imgWidth-1 || pStart.y > imgHeight-1)
					continue;

				int nGrayValue = GETGRAYVALUE1(img, j, i);//img->GetPixelGray(j, imgHeight-1-i);
				if (nGrayValue <= nThreshold)
				{
					CRect dstRectTmp;
					CPoint pStart = CPoint(j, i);

					if (pStart.x >= 120)
						int ii =0;
				    
					bool bRes = GetRectFromPoint1(dstRectTmp, pStart, rectArea, img, minSize, maxSize, nIndex, nThreshold, pSrcImg);
					if (bRes)
					{
						if (GetDensity(img, dstRectTmp, nThreshold) < 0.85)
							continue;

						if (dstRectTmp.Width() < dstRectTmp.Height())
							continue;

						dstRect = dstRectTmp;
						return TRUE;
					}
					j += minSize.cx;	
				}	
			}
		}
	}
	else if (nIndex == 1)
	{
		//img->Save(L"D:\\testTmp.jpg", CXIMAGE_FORMAT_JPG);
		for (int i=rectArea.TopLeft().y; i<rectArea.BottomRight().y; i++)
		{
			for (int j=rectArea.BottomRight().x-1; j>=rectArea.TopLeft().x; j--)
			{
				CPoint pStart = CPoint(j, i);
				if (pStart.x < 0 || pStart.y < 0)
					continue;

				if (pStart.x > imgWidth-1 || pStart.y > imgHeight-1)
					continue;
				int nGrayValue = GETGRAYVALUE1(img, j, i);//img->GetPixelGray(j, imgHeight-1-i);
				if (nGrayValue <= nThreshold)
				{
				
					CRect dstRectTmp;
					CPoint pStart = CPoint(j, i);
					bool bRes = GetRectFromPoint1(dstRectTmp, pStart, rectArea, img, minSize, maxSize, nIndex, nThreshold, pSrcImg);
					if (bRes)
					{
						/*double dDensity = GetDensity(img, dstRectTmp, nThreshold);*/
						if (GetDensity(img, dstRectTmp, nThreshold) < 0.85)
							continue;

						if (dstRectTmp.Width() < dstRectTmp.Height())
							continue;

						dstRect = dstRectTmp;
						//TRACE("%d, %d\n",nIndex, GetDensity(img, dstRectTmp));
						return TRUE;
					}
					j -= minSize.cx;

				}

			}
		}
	}
	else if (nIndex == 2)
	{
		for (int i=rectArea.BottomRight().y-1; i>=rectArea.TopLeft().y; i--)
		{
			for (int j=rectArea.TopLeft().x; j < rectArea.BottomRight().x; j++)
			{
				CPoint pStart = CPoint(j, i);
				if (pStart.x< 0 || pStart.y<0)
					continue;

				if (pStart.x > imgWidth-1 || pStart.y > imgHeight-1)
					continue;
				int nGrayValue = GETGRAYVALUE1(img, j, i);//img->GetPixelGray(j, imgHeight-1-i);
				if (nGrayValue <= nThreshold)
				{
					CRect dstRectTmp;
					CPoint pStart = CPoint(j, i);

					bool bRes = GetRectFromPoint1(dstRectTmp, pStart, rectArea, img, minSize, maxSize, nIndex, nThreshold, pSrcImg);
					if (bRes)
					{
						if (GetDensity(img, dstRectTmp, nThreshold) < 0.85)
							continue;

						if (dstRectTmp.Width() < dstRectTmp.Height())
							continue;

						dstRect = dstRectTmp;
						return TRUE;
					}
					j += minSize.cx;

				}
			}
		}
	}
	else if (nIndex == 3)
	{
		for (int i=rectArea.BottomRight().y-1; i >= rectArea.TopLeft().y; i--)
		{
			for (int j=rectArea.BottomRight().x-1; j>=rectArea.TopLeft().x; j--)
			{
				CPoint pStart = CPoint(j, i);
				if (pStart.x < 0 || pStart.y <0)
					continue;

				if (pStart.x> imgWidth-1 || pStart.y > imgHeight-1)
					continue;

				int nGrayValue = GETGRAYVALUE1(img, j, i);//img->GetPixelGray(j, imgHeight-1-i);
				if (nGrayValue <= nThreshold)
				{
					CRect dstRectTmp;
					CPoint pStart = CPoint(j, i);

					bool bRes = GetRectFromPoint1(dstRectTmp, pStart, rectArea, img, minSize, maxSize, nIndex, nThreshold, pSrcImg);
					if (bRes)
					{
						if (GetDensity(img, dstRectTmp, nThreshold) < 0.85)
							continue;

						if (dstRectTmp.Width() < dstRectTmp.Height())
							continue;

						dstRect = dstRectTmp;
						return TRUE;
					}
					j -= minSize.cx;
				}
			}
		}
	}
	return FALSE;
}

BOOL CImageProcess::GetRectFromPoint1(CRect &dstRect, CPoint pStart, CRect rectArea,const CxImage *img, CSize minSize, CSize maxSize, int nIndex,const int nThreshold, const CxImage* pSrcImg)
{
	int nGrayValue;
	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();

	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart .y > nImgHeight-1)
		return FALSE;

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);

	if (nGrayValue > nThreshold)
		return FALSE;

	CPoint pTopExtream[2];
	CPoint pBottomExtream[2];
	for (int i=0; i<2; i++)
	{
		pTopExtream[i]    = pStart;
		pBottomExtream[i] = pStart;
	}

	CPoint pStartTmp = pStart;
	int nFind = -1;
	while (nFind<=3) //向上搜索
	{
		CPoint pTmp[2];
		if (GetHLine1(pTmp, pStartTmp, rectArea, img, minSize, maxSize, nThreshold))
		{
			for (int i=0; i<2; i++)
			{
				pTopExtream[i] = pTmp[i];

			}
			pStartTmp.x = (pTopExtream[0].x + pTopExtream[1].x)/2;
			nFind = 0;
		}
		else 
		{
			nFind++;
		}

		pStartTmp.y -= 2; 
		if (pStartTmp.y < rectArea.TopLeft().y)
			break;

		if (pStartTmp.x < 0 || pStartTmp.y < 0)
		{
			nFind++;
			continue;
		}

		if (pStartTmp.x > nImgWidth-1 || pStartTmp.y > nImgHeight-1)
		{
			nFind++;
			continue;
		}
		nGrayValue = GETGRAYVALUE1(img, pStartTmp.x, pStartTmp.y);
	}

	//判断有效性
	if (pTopExtream[1].x - pTopExtream[0].x < minSize.cx ||
		pTopExtream[1].x - pTopExtream[0].x > maxSize.cx)
		return FALSE;

	pStartTmp = pStart;
	nFind = -1;
	while(nFind<=3)
	{
		CPoint pTmp[2];
		if (GetHLine1(pTmp, pStartTmp, rectArea, img, minSize, maxSize, nThreshold))
		{
			for (int i=0; i<2; i++)
			{
				pBottomExtream[i] = pTmp[i];
			}
			pStartTmp.x = (pBottomExtream[0].x + pBottomExtream[1].x)/2;
			nFind = 0;
		}
		else 
		{
			nFind++;
		}

		pStartTmp.y += 2;
		if (pStartTmp.y > rectArea.BottomRight().y-1)
			break;
	}

	if (pBottomExtream[1].x - pBottomExtream[0].x < minSize.cx ||
		pBottomExtream[1].x - pBottomExtream[0].x > maxSize.cx )
		return FALSE;

	CPoint pTopleft;
	CPoint pBottomright;
	pTopleft.x = min(pTopExtream[0].x, pBottomExtream[0].x);
	pTopleft.y = pTopExtream[0].y;
	pBottomright.x = max(pTopExtream[1].x, pBottomExtream[1].x);
	pBottomright.y = pBottomExtream[1].y;

	CRect rectTmp;
	rectTmp.SetRect(pTopleft, pBottomright);
	int nWidth = rectTmp.Width();
	int nHeight = rectTmp.Height();

	int nSrcWidth = pSrcImg->GetWidth();
	int nSrcHeight = pSrcImg->GetHeight();
	if (nWidth >= minSize.cx && nHeight >= minSize.cy && 
		nWidth <= maxSize.cx && nHeight <= maxSize.cy)
	{

		if (m_dstBlackSize.cx > 0 &&
			m_dstBlackSize.cy > 0)
		{
			if (abs(nWidth  -  m_dstBlackSize.cx) <= nEPS &&
				abs(nHeight - m_dstBlackSize.cy)  <= nEPS)	
			{
				dstRect = rectTmp;
				//return TRUE;
			}
			else 
				return FALSE;

		}

		//判断矩形是否合法
		CRect rectExpand;
		int nExpand = min(rectTmp.Width()/2, rectTmp.Height()/2);
		if (nExpand == 0)
			nExpand = 5;
		CPoint p1 = CPoint(rectTmp.TopLeft().x-nExpand, rectTmp.TopLeft().y - nExpand);
		if (p1.x < 0)
			p1.x = 0;

		if (p1.y < 0)
			p1.y = 0;

		CPoint p2 = CPoint(p1.x + rectTmp.Width() + 2*nExpand, p1.y + rectTmp.Height() + 2*nExpand);

		int nNewWidth = p2.x - p1.x;
		int nNewHeight = p2.y - p1.y;

		if (p1.x < 0)
			p1.x =0;

		if (p1.y < 0)
			p1.y = 0;

		if (p2.x > nSrcWidth-1)
			p2.x = nSrcWidth-1;

		if (p2.y > nSrcHeight-1)
			p2.y = nSrcHeight-1;

		if (nIndex == 0)
		{
			p2.x = p1.x + nNewWidth;
			p2.y = p1.y + nNewHeight;
		}
		else if (nIndex == 1)
		{
			p1.x = p2.x - nNewWidth;
			p2.y = p1.y + nNewHeight;
		}
		else if (nIndex == 2)
		{
			p2.x = p1.x + nNewWidth;
			p1.y = p2.y - nNewHeight;
		}
		else if (nIndex == 3)
		{
			p1.x = p2.x - nNewWidth;
			p1.y = p2.y - nNewHeight;
		}

		//计算区域内白点密度
		rectExpand.SetRect(p1, p2);
		int nCount = 0;
		for (int i=p1.x; i<p1.x + rectExpand.Width(); i++)
		{
			for (int j=p1.y; j<p1.y + rectExpand.Height(); j++)
			{
				CPoint pStart1 = CPoint(i, j);
				if (pStart1.x < 0 || pStart1.y < 0)
					continue;

				if (pStart1.x > nImgWidth-1 || pStart1.y > nImgHeight-1)
					continue;

				nGrayValue = GETGRAYVALUE1(img, i, j);
				if (nGrayValue > nThreshold && !IsPointInRect(CPoint(i, j), rectTmp))
				{
					nCount++;
				}
			}
		}
		double dDensity = double(nCount+rectTmp.Width()*rectTmp.Height())/double(rectExpand.Width()*rectExpand.Height());
		if (dDensity >= 0.80)
		{
			/*if (m_dstBlackSize.cx < 0|| m_dstBlackSize.cy < 0)
			{
				m_dstBlackSize = CSize(rectTmp.Width(), rectTmp.Height());
			}*/

			dstRect = rectTmp;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL  CImageProcess::GetHLine1(CPoint pExtream[2], CPoint pStart, CRect rectArea, const CxImage *img, CSize minSize, CSize maxSize, const int nThreshold)
{
	CPoint pTmp[2];
	int nGrayValue;
	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();
	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
		return FALSE;

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);
	if (nGrayValue > nThreshold)
		return FALSE;

	for (int i=0; i<2; i++)
	{
		pTmp[i] = pStart;
	}

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);
	while (nGrayValue <= nThreshold)
	{
		pTmp[0].x -= 2;
		if (pTmp[0].x < rectArea.TopLeft().x || pTmp[0].x > rectArea.BottomRight().x-1)
			break;
		if (pTmp[0].y < rectArea.TopLeft().y || pTmp[0].y > rectArea.BottomRight().y-1)
			break;

		if (pTmp[0].x <0 || pTmp[0].y < 0)
			break;

		if (pTmp[0].x > nImgWidth-1 || pTmp[0].y > nImgHeight-1)
			break;

		nGrayValue = GETGRAYVALUE1(img, pTmp[0].x, pTmp[0].y);
	}

	nGrayValue = GETGRAYVALUE1(img, pTmp[1].x, pTmp[1].y);
	while (nGrayValue <= nThreshold)
	{
		pTmp[1].x+= 2;
		if (pTmp[1].x < rectArea.TopLeft().x || pTmp[1].x > rectArea.BottomRight().x-1)
			break;

		if (pTmp[1].y < rectArea.TopLeft().y || pTmp[1].y > rectArea.BottomRight().y-1)
			break;

		if (pTmp[1].x <0 || pTmp[1].y < 0)
			break;

		if (pTmp[1].x > nImgWidth-1 || pTmp[1].y > nImgHeight-1)
			break;

		nGrayValue = GETGRAYVALUE1(img, pTmp[1].x, pTmp[1].y);
	}
	int nLen = pTmp[1].x - pTmp[0].x;
	if (nLen >= minSize.cx && nLen <= maxSize.cx)
	{
		pExtream[0] = pTmp[0];
		pExtream[1] = pTmp[1];
		return TRUE;
	}

	return FALSE;
}

double CImageProcess::GetDensity(const CxImage *img,  CRect rect, const int nThreshold)
{
	double dDensity = 0.0;
	int nWidth = rect.Width();
	int nHeigtht = rect.Height();
	if (nWidth*nHeigtht == 0)
		return dDensity;

	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();
	int nCount = 0;
	for (int i=rect.TopLeft().x; i<rect.TopLeft().x + nWidth; i++)
	{ 
		for (int j=rect.TopLeft().y; j<rect.TopLeft().y + nHeigtht; j++)
		{

			CPoint pStart =CPoint(i, j);
			if (pStart.x < 0 || pStart.y < 0)
				continue;

			if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
				continue;
			int nGrayValue = GETGRAYVALUE1(img, i, j);
			if (nGrayValue <= nThreshold)
				nCount++;
		}
	}

	dDensity = double(nCount)/double(max(nWidth*nHeigtht, 1)); //获取填涂密度值
	return dDensity;
}

double CImageProcess::GetHollowDensity( CxImage *cropImg, const int nThreshold, const CSize centerSize)
{
	double dDensity = 0.0;
	int nWidth = cropImg->GetWidth();
	int nHeight = cropImg->GetHeight();
	cropImg->Threshold(nThreshold);
	cropImg->GrayScale();

	int *nValue = new int[nWidth*nHeight];
	memset(nValue, 1, nWidth*nHeight);
	for (int i=0; i<nHeight; i++)
	{
		if (i==0 || i==nHeight-1)
			continue;

		for (int j=0; j<nWidth; j++)
		{
			if (GETGRAYVALUE1(cropImg, j, i) == 0)
			{
				nValue[i*nWidth + j] = 0;
				if (j==0 || j==nWidth-1)
					continue;

				if (GETGRAYVALUE1(cropImg,j,i-1) == 0 && GETGRAYVALUE1(cropImg,j-1, i) == 0 &&
					GETGRAYVALUE1(cropImg,j,i+1) == 0 && GETGRAYVALUE1(cropImg,j+1, i) == 0)
				{
						nValue[i*nWidth + j] = 1;
				}
				else 
					nValue[i*nWidth + j] = 0;
			}
			else
				nValue[i*nWidth + j] = 1;
		}
	}

	int nBlackCount = 0;
	for (int i=max(0, nWidth/2-centerSize.cx/2); i<min(nWidth-1, nWidth/2+centerSize.cx/2); i++)
	{
		for (int j=max(0, nHeight/2-centerSize.cy/2); j<min(nHeight-1, nHeight/2 + centerSize.cy/2); j++)
		{
			if (nValue[j*nWidth+i] == 0)
			{
				nBlackCount++;
			}
		}
	}

	delete []nValue;
	nValue = NULL;
	dDensity = double(nBlackCount)/double(max(1, centerSize.cx*centerSize.cy));
	return dDensity;
}


int CImageProcess::GetCrossPointsCount(CRect rect1, CRect rect2)
{
	int nCount = 0;
	for (int i=rect1.TopLeft().x; i <= rect1.BottomRight().x; i++)
	{
		for (int j=rect1.TopLeft().y; j <= rect1.BottomRight().y; j++)
		{
			CPoint p1 = CPoint(i, j);
			if (IsPointInRect(p1, rect2))
				nCount++;
		}
	}
	
	return nCount;
}

bool  CImageProcess::GetVLinelen( CxImage srcImg, int nThreshold, int &nLen1)
{
	int nWidth = srcImg.GetWidth();
	int nHeight = srcImg.GetHeight();

	//// 计算链码 2016.06.21
	//// 计算右上角点 
	//CPoint pStart = CPoint(0, 0); //定义起始点
	//int nMinLen = 8000;
	//for (int i=0; i<nHeight; i++)
	//{
	//	for (int j=0; j<nWidth; j++)
	//	{
	//		if (GETGRAYVALUE2(srcImg, j, i) <= nThreshold)
	//		{
	//			int nLen = (int)GetLength(CPoint(j, i), CPoint(nWidth-1, 0));
	//			//TRACE("%d, %d, len:%d\n", j, i, nLen);
	//			if (nLen < nMinLen)
	//			{
	//				pStart = CPoint(j, i); 
	//				nMinLen = nLen;
	//			}
	//		}
	//	}
	//}

	////计算链码 
	////struct CHAINCODE
	////{
	////	CPoint pCodeStart;
	////	int nDirIndex; //链码方向
	////	
	////	CHAINCODE()
	////	{
	////		nDirIndex = -1;
	////	}
	////};

	//vector<CPoint> v_AllPoints;
	//int nLastIndex = -1;
	//CPoint pStatrTmp = pStart;
	//CString strIndex = _T("");
	//v_AllPoints.push_back(pStatrTmp);
	//for (int i=0; i<nHeight; i++)
	//{
	//	for (int j=0; j<nWidth; j++)
	//	{
	//		CPoint p[8];
	//		for (int k=0; k<8; k++)
	//		{
	//			p[k] = pStatrTmp;
	//		}

	//		p[0].x += 1;
	//		p[1].x += 1;
	//		p[1].y -= 1;
	//		p[2].y -= 1;
	//		p[3].x -= 1;
	//		p[3].y -= 1;
	//		p[4].x -= 1;
	//		p[5].x -= 1;
	//		p[5].y += 1;
	//		p[6].y += 1;
	//		p[7].x += 1;
	//		p[7].y += 1;

	//		for (int k=0; k<8; k++)
	//		{

	//			if (abs(nLastIndex - k) == 4 && nLastIndex > 0)
	//				continue;

	//			if (p[k].x < 0 || p[k].x > nWidth-1)
	//				continue;

	//			if (p[k].y < 0 || p[k].y > nHeight-1)
	//				continue;

	//			bool bFind = false;
	//			for (vector<CPoint>::iterator it = v_AllPoints.begin(); it != v_AllPoints.end(); it++)
	//			{
	//				CPoint p1 = *it;
	//				if (p1 == p[k])
	//				{
	//					bFind = true;
	//					break;
	//				}
	//			}

	//			if (bFind)
	//				continue;

	//			if (GETGRAYVALUE2(srcImg, p[k].x, p[k].y) <= nThreshold)
	//			{
	//				nLastIndex = k;
	//				pStatrTmp = p[k];
	//				CString strTmp;
	//				strTmp.Format(L"%d", k);
	//				strIndex += strTmp;
	//				v_AllPoints.push_back(p[k]);
	//				break;
	//			}

	//		}

	//	}
	//}
	//	
	int nMaxCount = 0;
	int k =0;
	int nLastBlackCount = 0;
	for (int i=0; i<nWidth; i++)
	{
		int nBlackCount = 0;
		for (int j=0; j<nHeight; j++)
		{
			if (GETGRAYVALUE2(srcImg, i, j) <= nThreshold)
			{
				nBlackCount++;
			}
		}

		if (nBlackCount > nMaxCount)
		{
			nMaxCount = nBlackCount;
			k = i;
		}
		
		nLastBlackCount = nBlackCount;
	}

	for (int i=5 ; i<=nHeight-5; i++)
	{
		bool bFindWhite = false;
		bool bFindBlack[2] ={false, false};
		for (int j=0; j<nWidth; j++)
		{
			if (GETGRAYVALUE2(srcImg, j, i) > nThreshold)
			{
				if (bFindBlack[0])
					bFindWhite = true;
			}
			else 
			{
				if (!bFindWhite)
					bFindBlack[0] = true;
				else 
					bFindBlack[1] = true;
			}

			if (bFindBlack[0] && bFindBlack[1])
				return false;
		}
		
	}

	nLen1 = nMaxCount;
	return true;
}



void CImageProcess::GetThinImage(CxImage *cropImg, int nThreshold, int nTimes )
{
	cropImg->Threshold(nThreshold);
	cropImg->GrayScale();
	
	int nImgWidth = cropImg->GetWidth();
	int nImgHeight = cropImg->GetHeight();
	int nWidth = cropImg->GetWidth()-1;
	int nHeight = cropImg->GetHeight()-1;

	int *nValue = new int[nImgWidth*nImgHeight]; 
	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			if (GETGRAYVALUE1(cropImg, i, j) == 255)
				nValue[j*nImgWidth + i] = 0;
			else 
				nValue[j*nImgWidth + i] = 1;
		}
	}
	
	int p[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	for (int n=0; n<nTimes; n++)
	{
		 for (int i=1; i<nWidth; i++)
		 {
			 for (int j=1; j<nHeight; j++)
			 {
				 int p1 = nValue[j*nImgWidth + i];
				 if (p1 == 0)
					 continue;

				 p[0] = nValue[(j-1)*nImgWidth + i];
				 p[1] = nValue[(j-1)*nImgWidth + i+1];
				 p[2] = nValue[j*nImgWidth + i+1];
				 p[3] = nValue[(j+1)*nImgWidth + i+1];
				 p[4] = nValue[(j+1)*nImgWidth + i];
				 p[5] = nValue[(j+1)*nImgWidth + i-1];
				 p[6] = nValue[j*nImgWidth + i-1];
				 p[7] = nValue[(j-1)*nImgWidth + i-1];

				int nCount = 0;
				int nTotal = 0;
				for (int m=0; m<8; m++)
					nCount += p[m];

				if (p[0] == 0 && p[1] == 1)
					nTotal++;
				
				if (p[1] == 0 && p[2] == 1)
					nTotal++;

				if (p[2] == 0 && p[3] == 1)
					nTotal++;

				if (p[3] == 0 && p[4] == 1)
					nTotal++;

				if (p[4] == 0 && p[5] == 1)
					nTotal++;
 
				if (p[5] == 0 && p[6] == 1)
					nTotal++;

				if (p[6] == 0 && p[7] == 1)
					nTotal++;

				if (p[7] == 0 && p[0] == 1)
					nTotal++;

				if (nCount >= 2 && nCount <= 6
					&& nTotal == 1
					&& p[0]*p[2]*p[6] == 0 
					&& p[0]*p[4]*p[6] == 0
					)
				{ 
					p1 = 0;
					nValue[j*nImgWidth + i] = 0;
				}
				
			 }
		 }
	}

	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			if (nValue[j*nImgWidth + i] == 0)
				SETGRAYVALUE(cropImg->info.pImage, nImgWidth, nImgHeight, i, j, 255);
			else
				SETGRAYVALUE(cropImg->info.pImage, nImgWidth, nImgHeight, i, j, 0);
		}
	}

	delete []nValue;
	nValue = NULL;
}


CString CImageProcess::GetMoudulePath()
{
	CString strRetun = _T("");
#ifdef _UNICODE
	TCHAR szBuff[MAX_PATH];
	HMODULE module = GetCurrentModule();
	//HMODULE module = GetModuleHandle(0); 
	GetModuleFileName(module, szBuff, sizeof(szBuff)); 
	strRetun.Format(_T("%s"),szBuff);
#else
	HMODULE module = GetModuleHandle(0); 
	CHAR szBuff[MAX_PATH]; 
	GetModuleFileName(module, szBuff, sizeof(szBuff)); 
	strRetun.Format(_T("%s"),szBuff);
#endif 
	int pos = strRetun.ReverseFind(_T('\\'));
	if(pos != -1)
	{
		strRetun = strRetun.Left(pos);
	}
	return strRetun;
}

void CImageProcess::GetChainContour(CxImage srcImg, int nThreshold, CSize dstSize, vector<OMRRECT> &v_AllRects)
{
	IplImage *src = NULL;
	Cximage2Iplimage_1(&srcImg, &src);
	IplImage *dsw = cvCreateImage(cvGetSize(src), 8, 1);       
	CvMemStorage *storage = cvCreateMemStorage(0);    
	CvSeq *first_contour = NULL;    
	cvThreshold(src, dsw, nThreshold, 255, CV_THRESH_BINARY);    
	cvFindContours(dsw, storage, &first_contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);    
	for(; first_contour != 0; first_contour = first_contour->h_next)    
	{      
		CvRect rect = cvBoundingRect(first_contour,0);  
		CRect rect1;
		
		if (rect.height < dstSize.cy/2)
			continue;
		
		if (rect.width > dstSize.cx + nEPS)
			continue;

		if (abs(rect.height - dstSize.cy) > nEPS)
			continue;

		if (rect.width > 2*dstSize.cx)
			continue;

		rect1.SetRect(rect.x, rect.y, rect.x+rect.width, rect.y + rect.height);
		OMRRECT omrRect1;
		omrRect1.rect = rect1;
		v_AllRects.push_back(omrRect1);
	}  
	cvReleaseImage(&src);
	src = NULL;
	cvReleaseImage(&dsw);
	dsw = NULL;
	cvClearMemStorage(storage);
	cvReleaseMemStorage(&storage);  
}

 void CImageProcess::GetCharContour(CxImage srcImg,  int nThreshold, CRect &dstRect)
 {
	 dstRect.SetRect(0, 0, srcImg.GetWidth()-1, srcImg.GetHeight()-1);
	 vector<CRect> v_AllRects;
	 IplImage *src = NULL;   
	 Cximage2Iplimage_1(&srcImg, &src);
	 IplImage *dsw = cvCreateImage(cvGetSize(src), 8, 1);       
	 CvMemStorage *storage = cvCreateMemStorage(0);    
	 CvSeq *first_contour = NULL;    
	 cvThreshold(src, dsw, nThreshold, 255, CV_THRESH_BINARY);    
	 cvFindContours(dsw, storage, &first_contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);    
	 for(; first_contour != 0; first_contour = first_contour->h_next)    
	 {      
		 CvRect rect = cvBoundingRect(first_contour,0);  
		 CRect rect1;

		 if (rect.height*rect.width < nEPS)
			 continue;
		
		 if (rect.width*rect.height >= (srcImg.GetWidth()-2)*(srcImg.GetHeight()-2))
			 continue;

		 rect1.SetRect(rect.x, rect.y, rect.x+rect.width, rect.y + rect.height);
		 v_AllRects.push_back(rect1);
	 }    

	 ////删除重叠项 2016.07.28
	 //for (int i=0; i<v_AllRects.size(); i++)
	 //{
		// for (int j=i+1; j<v_AllRects.size(); j++)
		// {
		//	 CRect rect1 = v_AllRects[i];
		//	 CRect rect2 = v_AllRects[j];

		//	 if (IsPointInRect(rect1.CenterPoint(), rect2))
		//	 {
		//		 if(rect1.Width()*rect1.Height() < rect2.Width()*rect2.Height())
		//		 {
		//			 v_AllRects.erase(v_AllRects.begin()+i);
		//			 i -= 1;
		//			 break;
		//		 }
		//		 else
		//		 {
		//			 v_AllRects.erase(v_AllRects.begin()+j);
		//			 j -= 1;
		//		 }
		//	 }
		// }
	 //}

	 if (v_AllRects.size() > 0)
	 {
		 dstRect = v_AllRects[0];

		 CPoint pTopLeft = v_AllRects[0].TopLeft();
		 CPoint pBottomRight = v_AllRects[0].BottomRight();
		 for (vector<CRect>::iterator it = v_AllRects.begin(); it != v_AllRects.end(); it++)
		 {
			 CRect rectTmp = *it;
			 if (pTopLeft.x + pTopLeft.y > rectTmp.TopLeft().x + rectTmp.TopLeft().y)
				 pTopLeft = rectTmp.TopLeft();

			 if (pBottomRight.x + pBottomRight.y < rectTmp.BottomRight().x + rectTmp.BottomRight().y)
				 pBottomRight = rectTmp.BottomRight();

			 /*if (dstRect.Width()*dstRect.Height() < rectTmp.Width()*rectTmp.Height())
				 dstRect = rectTmp;*/
		 }

		 dstRect.SetRect(pTopLeft, pBottomRight);
	 }
	
	 cvReleaseMemStorage(&storage);  
	 storage = NULL;
	 cvReleaseImage(&dsw);
	 dsw = NULL;
	 cvReleaseImage(&src);
	 src = NULL;
 }

void CImageProcess::GetChainContour_Encolsed(CxImage srcImg,  int nThreshold, CSize dstSize, vector<CRect> &v_AllRects)
{
	IplImage *src = NULL;   
	Cximage2Iplimage_1(&srcImg, &src);

	IplImage *dsw = cvCreateImage(cvGetSize(src), 8, 1);       
	CvMemStorage *storage = cvCreateMemStorage(0);    
	CvSeq *first_contour = NULL;    
	cvThreshold(src, dsw, nThreshold, 255, CV_THRESH_BINARY);    
	cvFindContours(dsw, storage, &first_contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);    
	for(; first_contour != 0; first_contour = first_contour->h_next)    
	{      
		CvRect rect = cvBoundingRect(first_contour,0);  
		CRect rect1;

		if (rect.height < dstSize.cy/2)
			continue;

		if (rect.width < dstSize.cx/2)
			continue;

		if (abs(rect.height - dstSize.cy) > nEPS/*max(nEPS, dstSize.cy*0.1)*/)
			continue;

		if (abs(rect.width - dstSize.cx) > nEPS/*max(nEPS, dstSize.cx*0.1)*/)
			continue;
		rect1.SetRect(rect.x, rect.y, rect.x+rect.width, rect.y + rect.height);
		v_AllRects.push_back(rect1);
	}    

	//删除重叠项 2016.07.28
	for (int i=0; i<v_AllRects.size(); i++)
	{
		for (int j=i+1; j<v_AllRects.size(); j++)
		{
			CRect rect1 = v_AllRects[i];
			CRect rect2 = v_AllRects[j];

			if (IsPointInRect(rect1.CenterPoint(), rect2))
			{
				if(rect1.Width()*rect1.Height() < rect2.Width()*rect2.Height())
				{
					v_AllRects.erase(v_AllRects.begin()+i);
					i -= 1;
					break;
				}
				else
				{
					v_AllRects.erase(v_AllRects.begin()+j);
					j -= 1;
				}
			}
		}
	}

	
	cvReleaseMemStorage(&storage);  
	storage = NULL;
	cvReleaseImage(&dsw);
	dsw = NULL;
	cvReleaseImage(&src);
	src = NULL;
	
}

bool CImageProcess::Cximage2Iplimage_1(CxImage *src, IplImage **dst)
{
	src->GrayScale();
	int nPalatteCount = src->GetPaletteSize()/sizeof(RGBQUAD);;    
	RGBQUAD *pPal = src->GetPalette();    
	//int iBackColor = GetBlackColor(*src);    
	long i = 0,j = 0;    
	long nImageWidth = 0,nImageHeight = 0;    
	nImageWidth = src->GetWidth();    
	nImageHeight = src->GetHeight();    
	long nBitCunt = src->GetBpp();    
 
    if(nBitCunt<=8)    
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
	return true;    
}


int CImageProcess::GetMaxFillHeight(CxImage cropImg, int nThreshold, CSize dstSize)
{
	int nWidth = cropImg.GetWidth();
	int nHeight = cropImg.GetHeight();
	CRect rectArea;
	rectArea.SetRect(0, 0, nWidth-1, nHeight-1);
	int nMaxHeight = 0;
	for (int i=0; i<nHeight; i++)
	{
		for (int j=0; j<nWidth; j++)
		{
			CPoint pExtream[2];
			CPoint pStart = CPoint(j, i);
			if (GETGRAYVALUE2(cropImg, pStart.x, pStart.y) <= nThreshold)
			{
				if (GetHLine_OMR(pExtream, pStart, rectArea, dstSize, &cropImg, nThreshold, 3))
				{
					nMaxHeight++;
					break;
				}
			}

		}

	}
	return nMaxHeight;
}

////获取矩形框  2016.06.28
//bool CImageProcess::GetIcrRect(CxImage srcImg, CSize dstSize, int nThreshold, CString &strRes)
//{
//	int nImgWidth = srcImg.GetWidth();
//	int nImgHeight = srcImg.GetHeight();
//
//	for (int i=0; i<nImgWidth; i++)
//	{
//		for (int j=0; j<nImgHeight; j++)
//		{
//			if (GETGRAYVALUE2(srcImg, i, j) <= nThreshold)
//			{
//				vector<CRect> v_dstRects;
//				GetChainContour_Encolsed(srcImg, nThreshold, dstSize, v_dstRects);
//
//				for(vector<CRect>::iterator it = v_dstRects.begin(); it != v_dstRects.end(); it++)
//				{
//					CRect rect1 = *it;
//					CPoint pTopLeft1, pBottomRight1;
//					pTopLeft1.x = rect1.TopLeft().x + 5/2;
//					pTopLeft1.y = rect1.TopLeft().y + 5/2;
//
//					pBottomRight1.x = rect1.BottomRight().x - 5/2;
//					pBottomRight1.y = rect1.BottomRight().y - 5/2;
//
//					CRect rectCrop;
//					rectCrop.SetRect(pTopLeft1, pBottomRight1);
//					CxImage cropImage;
//					srcImg.Crop(rectCrop, &cropImage);
//					strRes = GetOcrResult(&cropImage);
//
//					if (!strRes.IsEmpty() && strRes.Find('~') < 0)
//						return true;
//				}
//			}
//		}
//	}
//
//	return false;
//}

bool CImageProcess::GetMinRect(CxImage srcImg, CSize minSize, int nThreshold, vector<CRect> &v_allRects)
{
	IplImage *src = NULL;
	Cximage2Iplimage_1(&srcImg, &src);
	IplImage *dsw = cvCreateImage(cvGetSize(src), 8, 1);       
	CvMemStorage *storage = cvCreateMemStorage(0);    
	CvSeq *first_contour = NULL;    
	cvThreshold(src, dsw, nThreshold, 255, CV_THRESH_BINARY);    
	cvFindContours(dsw, storage, &first_contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);    
	for(; first_contour != 0; first_contour = first_contour->h_next)    
	{      
		CvRect rect = cvBoundingRect(first_contour,0);  
		CRect rect1;
		if (rect.height < minSize.cy)
			continue;

		if (rect.width < minSize.cx)
			continue;

		if (rect.width > 2*minSize.cx)
			continue;

		if (rect.height > 2*minSize.cy)
			continue;

		rect1.SetRect(rect.x, rect.y, rect.x+rect.width, rect.y + rect.height);
		v_allRects.push_back(rect1);
	}  

	for (int i=0; i<v_allRects.size(); i++)
	{
		for (int j=i+1; j<v_allRects.size(); j++)
		{
			CRect rect1 = v_allRects[i];
			CRect rect2 = v_allRects[j];
			
			if (IsPointInRect(rect1.CenterPoint(), rect2))
			{
				if(rect1.Width()*rect1.Height() < rect2.Width()*rect2.Height())
				{
					v_allRects.erase(v_allRects.begin()+i);
					i -= 1;
					break;
				}
				else
				{
					v_allRects.erase(v_allRects.begin()+j);
					j -= 1;
				}
			}
		}
	}

	//将框按从左到右排序 2016.07.19
	for (int i=0; i<v_allRects.size(); i++)
	{
		for (int j=i+1; j<v_allRects.size(); j++)
		{
			if (v_allRects[i].CenterPoint().x > v_allRects[j].CenterPoint().x)
			{
				CRect rectTmp = v_allRects[j];
				v_allRects[j] = v_allRects[i];
				v_allRects[i] = rectTmp;
			}
		}
	}
	
	cvReleaseImage(&src);
	src = NULL;
	cvReleaseImage(&dsw);
	dsw = NULL;
	cvClearMemStorage(storage);
	cvReleaseMemStorage(&storage);  
	return true;
}

bool CImageProcess::GetIcrRects(CxImage srcImg, CSize dstSize, int nThreshold, vector<CRect> &v_allRects)
{
	int nWidth  = srcImg.GetWidth();
	int nHeight = srcImg.GetHeight();
	vector<CRect> v_tempRects;
	GetChainContour_Encolsed(srcImg, nThreshold, dstSize, v_tempRects);
	if (v_tempRects.size() == 0)
		return FALSE;

	//按中心点进行排序
	for (vector<CRect>::iterator it1 = v_tempRects.begin(); it1 != v_tempRects.end(); it1++)
	{
		//将高度差在定义的高度范围内从左至右依次排序
		for (vector<CRect>::iterator it2 = it1 +1; it2 != v_tempRects.end(); it2++)
		{
			CRect rect1 = *it1;
			CRect rect2 = *it2;
			CPoint pCenter1 = rect1.CenterPoint();
			CPoint pCenter2 = rect2.CenterPoint();

			if (abs(pCenter1.y - pCenter2.y) >= dstSize.cy)
			{
				if (pCenter1.y > pCenter2.y)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
			else 
			{
				if (pCenter1.x > pCenter2.x)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
		}
	}

	v_allRects = v_tempRects;
	if (v_allRects.size() <= 0)
		return FALSE;

	return true;
}


bool CImageProcess::GetWhiteRect(CxImage srcImg, int nThreshold, CRect rect1, CRect &dstRect)
{
	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();

	bool bFindFirst = false;
	CPoint pTopLeft, pBottomRight;
	
	//判断白色有效点 2016.07.28
	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			int nGrayValue = GETGRAYVALUE2(srcImg, i, j);
			CPoint pTmp = CPoint(i, j);
			if (nGrayValue > nThreshold)
			{
				int nTime1 = GetTickCount();
				if (IsAroundBlack(srcImg, nThreshold, pTmp))
				{
					//TRACE("耗时:%d\n", GetTickCount() - nTime1);
					if (!bFindFirst)
					{
						bFindFirst = true;
						pTopLeft = pTmp;
						pBottomRight = pTmp;
					}
					else 
					{
						if ((pTopLeft.x + pTopLeft.y) > (pTmp.x + pTmp.y))
							pTopLeft = pTmp;

						if ((pBottomRight.x + pBottomRight.y) < (pTmp.x + pTmp.y))
							pBottomRight = pTmp;
					}
				}
				//else
					//TRACE("耗时:%d\n", GetTickCount() - nTime1);
			}

			
			j += 5;
		}

		i+=5;
	}

	dstRect.SetRect(pTopLeft, pBottomRight);
	return true;
}

bool CImageProcess::IsAroundBlack(CxImage srcImg, int nThreshold, CPoint p1)
{
	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();

	bool bFind[4] = {false, false, false, false};
	int nLen[4] = {1, 1, 1, 1};
	
	for (int i=0; i<4; i++)
	{
		CPoint pTmp = p1;
		if (!bFind[i])
		{
			while(1)
			{
				nLen[i]++;
				if (i == 0)
				{
					pTmp.x -= nLen[i];
					if (pTmp.x <= 0)
						return false;
				}
				else if (i == 1)
				{
					pTmp.x += nLen[i];
					if (pTmp.x > nImgWidth-1)
						return false;
				}
				else if (i == 2)
				{
					pTmp.y -= nLen[i];
					if (pTmp.y <= 0)
						return false;
				}
				else if (i == 3)
				{
					pTmp.y += nLen[i];
					if (pTmp.y > nImgHeight-1)
						return false;
				}

				if (GETGRAYVALUE2(srcImg, pTmp.x, pTmp.y) <= nThreshold)
				{
					bFind[i] = true;
					break;
				}
			}
		}
	}
	

	for (int i=0; i<4; i++)
	{
		if (!bFind[i])
			return false;
	}

	return true;
}

bool CImageProcess::GetValidExternalCharRect(CxImage srcImg, int nThreshold, CRect &dstRect)
{
	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();
    bool bFind = false;
    CPoint pTopLeft, pBottomRight;

	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			CPoint pStart = CPoint(i, j);
			int nGrayValue = GETGRAYVALUE2(srcImg, i, j);
			if (nGrayValue <= nThreshold) //黑点
			{
				if (!bFind)
				{
					bFind = true;
					pTopLeft = pStart;
					pBottomRight = pStart;
				}
				else
				{
					if (pTopLeft.x + pTopLeft.y > pStart.x + pStart.y)
						pTopLeft = pStart;

					if (pBottomRight.x + pBottomRight.y < pStart.x + pStart.y)
						pBottomRight = pStart;
				}
				
			}
		}
	}
	
	dstRect.SetRect(pTopLeft.x+8, pTopLeft.y+8, pBottomRight.x-8, pBottomRight.y-8);
	return true;
}

bool CImageProcess::GetExpandImg(CxImage srcImg, int nNewWidth, int nNewHeight, CxImage &dstImg)
{
	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();

	dstImg.Create(nNewWidth, nNewHeight, 8, CXIMAGE_FORMAT_BMP);
	dstImg.SetGrayPalette();
	dstImg.GrayScale();
	memset(dstImg.info.pImage, 255, BYTESPERLINE(nNewWidth, 8)*nNewHeight);

	CPoint pStart1 = CPoint(nNewWidth/2 - nImgWidth/2, nNewHeight/2 - nImgHeight/2);
	CPoint pEnd1 = CPoint(nNewWidth/2 + nImgWidth/2, nNewHeight/2 + nImgHeight/2);

	for (int m=pStart1.y; m<pEnd1.y; m++)
	{
		memcpy(dstImg.info.pImage + (nNewHeight - 1 - m)*BYTESPERLINE(nNewWidth, 8) + pStart1.x,
			srcImg.info.pImage + (nImgHeight-1-(m-pStart1.y))*BYTESPERLINE(nImgWidth, 8),
			nImgWidth);
	}


	return true;
}

=======
#include "stdafx.h"
#include "ImageProcess.h"

//#ifndef ZYJ_ICR
//#define  ZYJ_ICR 0
//#endif


//#define RECOGNIZER "c:\\re\\numplus.rec"

#if _MSC_VER >= 1300    // for VC 7.0
// from ATL 7.0 sources
#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif
#endif

static HMODULE GetCurrentModule()
{
#if _MSC_VER < 1300    // earlier than .NET compiler (VC 6.0)

	// Here's a trick that will get you the handle of the module
	// you're running in without any a-priori knowledge:
	MEMORY_BASIC_INFORMATION mbi;
	static int dummy;
	VirtualQuery( &dummy, &mbi, sizeof(mbi) );

	return reinterpret_cast<HMODULE>(mbi.AllocationBase);
#else    // VC 7.0
	// from ATL 7.0 sources
	return reinterpret_cast<HMODULE>(&__ImageBase);
#endif
}

//int g_nImgindex = -1;
CImageProcess::CImageProcess(void)
{
	
	m_dstBlackSize = CSize(-1, -1);
}



CImageProcess::~CImageProcess(void)
{

}

BOOL  CImageProcess::IsPointInRect(CPoint p1, CRect rect)
{
	BOOL bRes = FALSE;
	if (p1.x >= rect.TopLeft().x && p1.x <= rect.BottomRight().x)
	{
		if (p1.y >= rect.TopLeft().y && p1.y <= rect.BottomRight().y)
		{
			bRes = TRUE;
		}
	}
	return bRes;
}

double CImageProcess::GetLength(CPoint p1, CPoint p2)
{
	double dLength = 0.0;
	double dLenX = double(p2.x-p1.x);
	double dLenY = double(p2.y-p1.y);
	double dX = pow(dLenX, 2.0);
	double dY = pow(dLenY, 2.0);
	dLength = sqrt(dX + dY);
	return dLength;
}

bool CImageProcess::GetAllPoint(CxImage *img, vector<CPoint> &v_allPoints,  CPoint pStart, int nThreshold, int &nCount)
{
	if (v_allPoints.size() > 1000)
		return FALSE;

	if (pStart.x < 0 || pStart.y <0)
		return false;

	if (pStart.x > img->GetWidth()-1 || pStart.y >img->GetHeight()-1)
		return false;

	if (GETGRAYVALUE1(img, pStart.x, pStart.y) > nThreshold)
		return FALSE;

	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();

	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
		return FALSE;

	if (v_allPoints.size() >0 )
	{
		v_allPoints.push_back(pStart);
		if (v_allPoints.size() > 1000)
			return FALSE;

		nCount++;
	}
	else 
	{
		v_allPoints.push_back(pStart);
		if (v_allPoints.size() > 1000)
			return FALSE;

		nCount++;
	}

	img->info.pImage[(nImgHeight-1-pStart.y)*BYTESPERLINE(nImgWidth, 8) + pStart.x] = 255;
	
	//递归搜索	
	CPoint pStartTmp[8];
	for (int i=0; i<8; i++)
		pStartTmp[i] = pStart;

	pStartTmp[0].x -= 1;
	pStartTmp[1].x += 1;
	pStartTmp[2].y -= 1;
	pStartTmp[3].y += 1;

	pStartTmp[4].x -= 1;
	pStartTmp[4].y -= 1;

	pStartTmp[5].x -= 1;
	pStartTmp[5].y += 1;

	pStartTmp[6].x += 1;
	pStartTmp[6].y -= 1;

	pStartTmp[7].x += 1;
	pStartTmp[7].y += 1;

	bool bFind = FALSE;
	for (int i=0; i<8; i++)
	{
		if (pStartTmp[i].x < 0 || pStartTmp[i].y < 0)
			continue;

		if (pStartTmp[i].x > nImgWidth-1 || pStartTmp[i].y > nImgHeight-1)
			continue;

		if (GETGRAYVALUE1(img, pStartTmp[i].x, pStartTmp[i].y) > nThreshold)
			continue;

		if (!GetAllPoint(img, v_allPoints, pStartTmp[i], nThreshold, nCount))
			continue;
	}

	return TRUE;
}

bool  CImageProcess::GetRect(vector<CPoint> v_allPoints, CRect &dstRect)
{
	if (v_allPoints.size() <= 5)
		return FALSE;

	int nX[2] = {v_allPoints[0].x, v_allPoints[0].x};
	int nY[2] = {v_allPoints[0].y, v_allPoints[0].y};
	for (vector<CPoint>::iterator it = v_allPoints.begin(); it != v_allPoints.end(); it++)
	{
		CPoint p1 = *it;
		if (nX[0] > p1.x)
			nX[0] = p1.x;

		if (nX[1] < p1.x)
			nX[1] = p1.x;

		if (nY[0] > p1.y)
			nY[0] = p1.y;

		if (nY[1] < p1.y)
			nY[1] = p1.y;
	}

	dstRect.SetRect(CPoint(nX[0], nY[0]), CPoint(nX[1]+1, nY[1]+1));
	return TRUE;
}



bool  CImageProcess::GetValidRect(int nType, const CxImage *img, vector<CRect> &v_allRects, int nThreshold, CSize dstSize)
{
	int nWidth = img->GetWidth();
	int nHeight = img->GetHeight();
	CxImage *img1 = new CxImage();
	img1->Copy(*img);
	img1->Threshold(nThreshold);
	img1->GrayScale();
	vector<CPoint> v_allPoints;
	vector<OMRRECT> v_tempRects;
	GetChainContour(*img1, nThreshold, dstSize, v_tempRects);

	if (v_tempRects.size() == 0)
	{
		delete img1;
		img1 = NULL;
		return FALSE;
	}
	
	//将现有矩形排序
	for (vector<OMRRECT>::iterator it1 = v_tempRects.begin(); it1 != v_tempRects.end(); it1++)
	{
		//将高度差在定义的高度范围内从左至右依次排序
		for (vector<OMRRECT>::iterator it2 = it1 +1; it2 != v_tempRects.end(); it2++)
		{
			OMRRECT rect1 = *it1;
			OMRRECT rect2 = *it2;
			CPoint pCenter1 = rect1.rect.CenterPoint();
			CPoint pCenter2 = rect2.rect.CenterPoint();

			if (abs(pCenter1.y - pCenter2.y) >= nEPS)
			{
				if (pCenter1.y > pCenter2.y)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
			else 
			{
				if (pCenter1.x > pCenter2.x)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
		}
	}

	int i = 0;
	for (vector<OMRRECT>::iterator it = v_tempRects.begin(); it != v_tempRects.end(); it++)
	{
		//存储
		if (i+2 > v_tempRects.size()-1)
		{
			break;
		}
		i++;
		CRect dstRect;
		OMRRECT rect[3] = {*it, *(it+1), *(it+2)};
		if (abs(rect[0].rect.CenterPoint().y - rect[1].rect.CenterPoint().y) > nEPS ||
			abs(rect[0].rect.CenterPoint().y - rect[2].rect.CenterPoint().y) > nEPS)
			continue;
		else 
		{
			int nLen1 = rect[1].rect.CenterPoint().x - rect[0].rect.CenterPoint().x;
			int nLen2 = rect[2].rect.CenterPoint().x - rect[1].rect.CenterPoint().x;
			//TRACE("%d, %d\n", nLen1, nLen2);
			if (abs(nLen1 - nLen2) > min(nEPS, 5))
				continue;
			else 
			{
				int nRectWidth = rect[2].rect.BottomRight().x - rect[0].rect.TopLeft().x;
				int nRectHeigt = rect[2].rect.BottomRight().y - rect[0].rect.TopLeft().y;

				if (abs(nRectWidth - dstSize.cx) <= max(nEPS, 5) &&
					abs(nRectHeigt - dstSize.cy) <= max(nEPS, 5) )
				{
					int nTop  = min(min(rect[0].rect.top, rect[1].rect.top), rect[2].rect.top);
					int nLeft = min(min(rect[0].rect.left, rect[1].rect.left), rect[2].rect.left);
					int nBottom = max(max(rect[0].rect.bottom, rect[1].rect.bottom), rect[2].rect.bottom);
					int nRight =  max(max(rect[0].rect.right, rect[1].rect.right), rect[2].rect.right);;

					dstRect.SetRect(CPoint(nLeft, nTop), CPoint(nRight, nBottom));
					/*if (abs(rect[2].nCount - rect[0].nCount) > max(2*nEPS, 20))
						continue;*/

					//判断是否有边框 2016.06.16
					CxImage cropImg1, cropImg2, cropImg3;
					((CxImage *)img)->Crop(rect[2].rect, &cropImg1);
					((CxImage *)img)->Crop(rect[0].rect, &cropImg2);
					//((CxImage *)img)->Crop(rect[1].rect, &cropImg3);
					//cropImg1.Save(L"D:\\test2_old.jpg", CXIMAGE_FORMAT_JPG);
					GetThinImage(&cropImg1, nThreshold, 3);
					GetThinImage(&cropImg2, nThreshold, 3);
					int nLen1 = 0;
					if (!GetVLinelen(cropImg1, nThreshold, nLen1))
						continue;

					int nLen2 = 0;
					if (!GetVLinelen(cropImg2, nThreshold, nLen2))
						continue;

					if (abs(nLen1 - nLen2) > max(nEPS, 5))
						continue;

					if (nLen1 < dstSize.cy/2 || nLen2 < dstSize.cy/2)
						continue;

					if (abs(nLen1 - dstSize.cy) > nEPS)
						continue;

					if (abs(nLen2 - dstSize.cy) > nEPS)
						continue;

					v_allRects.push_back(dstRect);
				
					if (i+2 > v_tempRects.size()-1)
					{
						break;
					}
					else 
					{ 
						it += 2;
						i += 2;
					}
				}
			}
		}
	}
	
	if (v_allRects.size() == 0)
	{
		delete img1;
		img1 = NULL;
		return FALSE;
	}

	delete img1;
	img1 = NULL;

	return TRUE;
}

bool CImageProcess::GetEnclosedRects(int nType, CxImage *img, vector<CRect> &v_allRects, int nThreshold, CSize dstSize)
{
	int nWidth  = img->GetWidth();
	int nHeight = img->GetHeight();

	vector<CRect> v_tempRects;

	GetChainContour_Encolsed(*img, nThreshold, dstSize, v_tempRects);
	if (v_tempRects.size() == 0)
		return FALSE;

	//按中心点进行排序
	for (vector<CRect>::iterator it1 = v_tempRects.begin(); it1 != v_tempRects.end(); it1++)
	{
		//将高度差在定义的高度范围内从左至右依次排序
		for (vector<CRect>::iterator it2 = it1 +1; it2 != v_tempRects.end(); it2++)
		{
			CRect rect1 = *it1;
			CRect rect2 = *it2;
			CPoint pCenter1 = rect1.CenterPoint();
			CPoint pCenter2 = rect2.CenterPoint();

			if (abs(pCenter1.y - pCenter2.y) >= dstSize.cy)
			{
				if (pCenter1.y > pCenter2.y)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
			else 
			{
				if (pCenter1.x > pCenter2.x)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
		}
	}

	v_allRects = v_tempRects;
	if (v_allRects.size() <= 0)
		return FALSE;

	return TRUE;
}

BOOL CImageProcess::GetRectFromPoint(CRect &dstRect, CPoint pStart, CRect rectArea, CSize dstsize,const CxImage *img, int nThreshold)
{
	int nGrayValue;
	int nImgWidth = img->GetWidth();
	int nImgHeight  = img->GetHeight();
	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
		return FALSE;

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);
	if (nGrayValue >  nThreshold)
		return FALSE;

	CPoint pTopExtream[2];
	CPoint pBottomExtream[2];
	for (int i=0; i<2; i++)
	{
		pTopExtream[i]    = pStart;
		pBottomExtream[i] = pStart;
	}

	CPoint pStartTmp = pStart;
	int nFind = 0;
	while (nFind <= 5)
	{
		CPoint pTmp[2];
		if (GetHLine(pTmp, pStartTmp, rectArea, dstsize, img, nThreshold, 5))
		{
			nFind = 0;
			for (int i=0; i<2; i++)
			{
				pTopExtream[i] = pTmp[i];
			}

			pStartTmp.x = (pTopExtream[0].x + pTopExtream[1].x)/2;
		}
		else
		{
			nFind++;
		}
		
		pStartTmp.y--; 
		if (pStartTmp.y < rectArea.TopLeft().y)
			break;
	}

	pStartTmp = pStart;
	nFind = 0;
	while (nFind <= 5)
	{
		CPoint pTmp[2];
		if (GetHLine(pTmp, pStartTmp, rectArea, dstsize, img, nThreshold, 5))
		{
			nFind = 0;
			for (int i=0; i<2; i++)
			{
				pBottomExtream[i] = pTmp[i];
			}
			pStartTmp.x = (pBottomExtream[0].x + pBottomExtream[1].x)/2;
		}
		else
		{
			nFind++;
		}

		pStartTmp.y++; 
		if (pStartTmp.y > rectArea.BottomRight().y-1)
			break;
	}

	CPoint pTopleft;
	CPoint pBottomright;
	pTopleft.x = min(pTopExtream[0].x, pBottomExtream[0].x);
	pTopleft.y = pTopExtream[0].y;
	pBottomright.x = max(pTopExtream[1].x, pBottomExtream[1].x);
	pBottomright.y = pBottomExtream[1].y;

	CRect rectTmp;
	rectTmp.SetRect(pTopleft, pBottomright);
	int nWidth = rectTmp.Width();
	int nHeight = rectTmp.Height();

	if (nWidth <= 0 || nHeight <= 0)
		return FALSE;
	
	if (abs(nWidth - dstsize.cx) <= nEPS &&
		abs(nHeight - dstsize.cy) <= nEPS)
	{
		//判断矩形有效性
		CRect rectExpand; 
		int nExpand = min(rectTmp.Width()/2, rectTmp.Height()/2);
		if (nExpand == 0)
		{
			//double dBalckDensity = GetDensity(img, rectTmp);
			//TRACE("考号竖向大定位点密度:%f", dBalckDensity);
			return FALSE;
		}
		CPoint p1 = CPoint(rectTmp.TopLeft().x-nExpand, rectTmp.TopLeft().y - nExpand);
		if (p1.x < 0)
			p1.x = 0;

		if (p1.y < 0)
			p1.y = 0;

		CPoint p2 = CPoint(p1.x + rectTmp.Width() + 2*nExpand, p1.y + rectTmp.Height() + 2*nExpand);

		if (p1.x < 0)
			p1.x =0;

		if (p1.y < 0)
			p1.y = 0;

		if (p2.x > nImgWidth-1)
			p2.x = nImgWidth-1;

		if (p2.y > nImgHeight-1)
			p2.y = nImgHeight-1;

		//计算区域内白点密度
		rectExpand.SetRect(p1, p2);
		int nCount = 0;
		for (int i=p1.x; i<p1.x + rectExpand.Width(); i++)
		{
			for (int j=p1.y; j<p1.y + rectExpand.Height(); j++)
			{
				if (i < 0 || j< 0)
					continue;

				if (i>img->GetWidth()-1 || j>img->GetHeight()-1)
					continue;

				nGrayValue = GETGRAYVALUE1(img, i, j);//img->GetPixelGray(i, nImgHeight-1-j); 
				if (nGrayValue > nThreshold && !IsPointInRect(CPoint(i, j), rectTmp))
				{
					nCount++;
				}
			}
		}
		double dDensity = double(nCount+rectTmp.Width()*rectTmp.Height())/double(rectExpand.Width()*rectExpand.Height());
		if (dDensity >= 0.75) //防止时裁切黑边 2015.12.29
		{
			dstRect = rectTmp;
			return TRUE;
		}

	}
	return FALSE;
}

BOOL CImageProcess::GetHLine(CPoint pExtream[2], CPoint pStart, CRect rectArea, CSize dstSize,const CxImage *img, int nThreshold, int nMaxGapLen)
{
	CPoint pTmp[2];
	int nGrayValue;
	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();

	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
		return FALSE;

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);
	if (nGrayValue > nThreshold)
		return FALSE;

	for (int i=0; i<2; i++)
	{
		pTmp[i] = pStart;
	}

	CPoint pTmpExtream = pStart;
	int nFind = 0;
	while(nFind <= nMaxGapLen)
	{
		pTmpExtream.x -= 1;
		if (pTmpExtream.x < rectArea.TopLeft().x || pTmpExtream.x > rectArea.BottomRight().x)
			break;

		if (pTmpExtream.y < rectArea.TopLeft().y || pTmpExtream.y > rectArea.BottomRight().y)
			break;


		if (pTmpExtream.x < 0 || pTmpExtream.y < 0)
		{
			nFind++;
			break;
		}

		if (pTmpExtream.x > nImgWidth-1 || pTmpExtream.y > nImgHeight-1)
		{
			nFind++;
			continue;
		}

		nGrayValue = GETGRAYVALUE1(img, pTmpExtream.x, pTmpExtream.y);//img->GetPixelGray(pTmpExtream.x, nImgHeight-1-pTmpExtream.y);
		if (nGrayValue <= nThreshold)
		{
			nFind = 0;
			pTmp[0] = pTmpExtream;
		}
		else 
			nFind++;
	}

	pTmpExtream = pStart;
	nGrayValue  = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);

	nFind = 0;
	while(nFind <= nMaxGapLen)
	{
		pTmpExtream.x += 1;
		if (pTmpExtream.x < rectArea.TopLeft().x || pTmpExtream.x > rectArea.BottomRight().x)
			break;

		if (pTmpExtream.y < rectArea.TopLeft().y || pTmpExtream.y > rectArea.BottomRight().y)
			break;

		if (pTmpExtream.x < 0 || pTmpExtream.y < 0)
		{
			nFind++;
			continue;
		}

		if (pTmpExtream.x > nImgWidth-1 || pTmpExtream.y > nImgHeight-1)
		{
			nFind++;
			break;
		}

		nGrayValue = GETGRAYVALUE1(img, pTmpExtream.x, pTmpExtream.y);//img->GetPixelGray(pTmpExtream.x, nImgHeight-1-pTmpExtream.y);
		if (nGrayValue <= nThreshold)
		{
			nFind = 0;
			pTmp[1] = pTmpExtream;
		}
		else 
			nFind++;
	}

	int nLen = pTmp[1].x - pTmp[0].x;
	if (abs(nLen - dstSize.cx) <= nEPS)
	{
		for (int i=0; i<2; i++)
		{
			pExtream[i] = pTmp[i];
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CImageProcess::GetHLine_OMR(CPoint pExtream[2], CPoint pStart, CRect rectArea, CSize dstSize,const CxImage *img, int nThreshold, int nMaxGapLen)
{
	CPoint pTmp[2];
	int nGrayValue;
	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();

	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
		return FALSE;

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);
	if (nGrayValue > nThreshold)
		return FALSE;

	for (int i=0; i<2; i++)
	{
		pTmp[i] = pStart;
	}

	CPoint pTmpExtream = pStart;
	int nFind = 0;
	while(nFind <= nMaxGapLen)
	{
		pTmpExtream.x -= 1;
		if (pTmpExtream.x < rectArea.TopLeft().x || pTmpExtream.x > rectArea.BottomRight().x)
			break;

		if (pTmpExtream.y < rectArea.TopLeft().y || pTmpExtream.y > rectArea.BottomRight().y)
			break;


		if (pTmpExtream.x < 0 || pTmpExtream.y < 0)
		{
			nFind++;
			break;
		}

		if (pTmpExtream.x > nImgWidth-1 || pTmpExtream.y > nImgHeight-1)
		{
			nFind++;
			continue;
		}

		nGrayValue = GETGRAYVALUE1(img, pTmpExtream.x, pTmpExtream.y);//img->GetPixelGray(pTmpExtream.x, nImgHeight-1-pTmpExtream.y);
		if (nGrayValue <= nThreshold)
		{
			nFind = 0;
			pTmp[0] = pTmpExtream;
		}
		else 
			nFind++;
	}

	pTmpExtream = pStart;
	nGrayValue  = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);

	nFind = 0;
	while(nFind <= nMaxGapLen)
	{
		pTmpExtream.x += 1;
		if (pTmpExtream.x < rectArea.TopLeft().x || pTmpExtream.x > rectArea.BottomRight().x)
			break;

		if (pTmpExtream.y < rectArea.TopLeft().y || pTmpExtream.y > rectArea.BottomRight().y)
			break;

		if (pTmpExtream.x < 0 || pTmpExtream.y < 0)
		{
			nFind++;
			continue;
		}

		if (pTmpExtream.x > nImgWidth-1 || pTmpExtream.y > nImgHeight-1)
		{
			nFind++;
			break;
		}

		nGrayValue = GETGRAYVALUE1(img, pTmpExtream.x, pTmpExtream.y);//img->GetPixelGray(pTmpExtream.x, nImgHeight-1-pTmpExtream.y);
		if (nGrayValue <= nThreshold)
		{
			nFind = 0;
			pTmp[1] = pTmpExtream;
		}
		else 
			nFind++;
	}

	int nLen = pTmp[1].x - pTmp[0].x;
	if (nLen >= dstSize.cx/2)
	{
		for (int i=0; i<2; i++)
		{
			pExtream[i] = pTmp[i];
		}
		return TRUE;
	}
	return FALSE;


}


void CImageProcess::GetRatio(NEWPOINT pLocal[3], NEWPOINT pCenter, double &dHRatio, double &dVRatio)
{
	int nEpsX[2], nEpsY[2];
	nEpsX[0] = pLocal[1].nX - pLocal[0].nX;
	nEpsY[0] = pLocal[1].nY - pLocal[0].nY;

	nEpsX[1] = pLocal[2].nX - pLocal[0].nX;
	nEpsY[1] = pLocal[2].nY - pLocal[0].nY;

	int nX = pCenter.nX - pLocal[0].nX;
	int nY = pCenter.nY - pLocal[0].nY;

   dHRatio = double(nEpsY[1]*nX - nEpsX[1]*nY)/double(nEpsX[0]*nEpsY[1] - nEpsY[0]*nEpsX[1]);
   dVRatio = double(-nEpsY[0]*nX + nEpsX[0]*nY)/double(nEpsX[0]*nEpsY[1] - nEpsY[0]*nEpsX[1]);
}


void CImageProcess::GetNewPoint(NEWPOINT pCurLocal[3], double dHRatio, double dVRatio, NEWPOINT &pCenter)
{
	int nEpsX[2], nEpsY[2];
	nEpsX[0] = pCurLocal[1].nX - pCurLocal[0].nX;
	nEpsY[0] = pCurLocal[1].nY - pCurLocal[0].nY;

	nEpsX[1] = pCurLocal[2].nX - pCurLocal[0].nX;
	nEpsY[1] = pCurLocal[2].nY - pCurLocal[0].nY;

	pCenter.nX = dHRatio*double(nEpsX[0]) + dVRatio*double(nEpsX[1]) + pCurLocal[0].nX;
	pCenter.nY = dHRatio*double(nEpsY[0]) + dVRatio*double(nEpsY[1]) + pCurLocal[0].nY;
}

void CImageProcess::GetAllRectsFromRects(vector<CRect>&v_dstRects, CRect rect[2], CRect rectArea, CSize dstSize, CxImage *img, int nThreshold)
{
	CRect dstRectTmp;
	bool bRes = TRUE;
	CRect rectsTmp[2];
	for (int i=0; i<2; i++)
	{
		rectsTmp[i] = rect[i];
	}

	while (bRes)
	{
		bRes = GetRectFromTwoRects(dstRectTmp, rectsTmp, rectArea, dstSize, img, nThreshold);
		if (bRes)
		{
			rectsTmp[0] = dstRectTmp;
			v_dstRects.push_back(dstRectTmp);
		}
		else 
			break;
	}
}

BOOL CImageProcess::GetRectFromTwoRects(CRect &dstRect, CRect rect[2], CRect rectArea, CSize dstSize, CxImage *img, int nThreshold)
{
	CPoint pCenter[2];
	for (int i=0; i<2; i++)
	{
		CPoint p1 = rect[i].TopLeft();
		int nWidth = rect[i].Width();
		int nHeight = rect[i].Height();
		pCenter[i] = CPoint(p1.x+nWidth/2, p1.y + nHeight/2);
	}

	//计算单位向量
	double dLenX = double(pCenter[1].x-pCenter[0].x);
	double dLenY = double(pCenter[1].y-pCenter[0].y);
	double dX = pow(dLenX, 2.0);
	double dY = pow(dLenY, 2.0);
	double dTmp = sqrt(dX + dY);
	double dTmpX = dLenX/dTmp;
	double dTmpY = dLenY/dTmp;
	double dMaxLen = dTmp;

	//沿线搜索黑点
	CPoint pStart = pCenter[0];
	int nCount = 0;
	//int ii =0;
	while (dTmp > 5.0)
	{
		nCount += 2;
		double dTmp1 = double(nCount)*dTmpX;
		double dTmp2 = double(nCount)*dTmpY;

		pStart.x = pCenter[0].x + (int)dTmp1;
		pStart.y = pCenter[0].y + (int)dTmp2;
		dTmp = GetLength(pStart, pCenter[1]);

		if (IsPointInRect(pStart, rect[1]))
			break;

		if (IsPointInRect(pStart, rect[0]))
			continue;

		int nImgWidth  = img->GetWidth();
		int nImgHeight = img->GetHeight();

		if (pStart.x < 0 || pStart.y < 0)
			continue;

		if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
			continue;

		int nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);//img->GetPixelGray(pStart.x, nImgHeight-1-pStart.y);
		if (nGrayValue <= nThreshold)
		{
			CRect rectTmp;
			if (GetRectFromPoint(rectTmp, pStart, rectArea, dstSize, img, nThreshold))
			{
				//判断获取的矩形是有重叠
				if (IsPointInRect(rectTmp.CenterPoint(), rect[1]))
					break;
				else if (IsPointInRect(rectTmp.CenterPoint(), rect[0]))
				{
					//ii++;
					continue;
				}

				dstRect = rectTmp;
				return TRUE;
			}
		}
	}
	return FALSE;
}

//CString CImageProcess::GetOcrResult(CxImage *img)
//{
//	CString strRes = _T("");
//	img->IncreaseBpp(24);
//	HBITMAP hBmp = img->MakeBitmap(NULL);
//	if (!hBmp)
//		return strRes;
//
//#ifdef ZYJ_ICR > 0
//	//图像结构体转化
//	re_hbitmap2image(hBmp, &m_rel.image);
//	unsigned char RejectLevel=0;
//	char ResStr[256], Result[32];
//	if (rel_do(&m_rel)!=RE_SUCCESS)
//		return strRes;
//
//	//rec_collect_kernel(m_rel);
//	rel_textline(&m_rel, ResStr, MAX_CHARS, RejectLevel, '~', 0);
//	re_ClearError();
//	DeleteObject(hBmp);
//	//rel_freeimages(&m_rel);
//	USES_CONVERSION;
//	strRes = A2T(ResStr);
//#endif
//	return strRes;
//}


CRect CImageProcess::NewRect2Rect(NEWRECT newRect)
{
	CRect rect;
	rect.top = newRect.pTopLeft.nY;
	rect.bottom = newRect.pBottomRight.nY;
	rect.left = newRect.pTopLeft.nX;
	rect.right = newRect.pBottomRight.nX;
	return rect;
}

//
//bool CImageProcess::GetTitleRect(CxImage *img, vector<TITLENO> v_modelTitleNo, int nThrshold, vector<TITLENO> &v_dstTitleNo)
//{
//	//分割字符
//	int nImgWidth = img->GetWidth();
//	int nImgHeight = img->GetHeight();
//	CxImage imgTmp;
//	CRect rectArea;
//	rectArea.SetRect(0, 0, nImgWidth, nImgHeight);
//	img->Crop(rectArea, &imgTmp);
//	vector<TITLENO> v_titleNoTmp;
//
//	USES_CONVERSION;
//	for (int i=0; i<nImgHeight; i++)
//	{
//		for (int j=0; j<nImgWidth; j++)
//		{
//			int nGrayValue = GETGRAYVALUE2(imgTmp, j, i);
//			CPoint pStart = CPoint(j, i);
//			vector<CPoint> v_allPoints;
//			if (nGrayValue <= nThrshold)
//			{
//				int nCount = 0;
//				if (GetAllPoint(&imgTmp, v_allPoints, pStart, nThrshold, nCount))
//				{
//					CRect rect1;
//					GetRect(v_allPoints, rect1);
//
//					if (rect1.Width()*rect1.Height() < 5)
//						continue;
//
//					CxImage img1;
//					img->Crop(rect1, &img1);
//					CString strRes = GetOcrResult(&img1);
//					strRes.Trim();
//
//					if (strRes.Find('~') >= 0 || strRes.GetLength() >= 2 || strRes.IsEmpty())
//					{
//						int nTitleNo1 = atoi(T2A(strRes));
//						NEWRECT rectLocal;
//						rectLocal.SetRect(rect1.left, rect1.top, rect1.Width(), rect1.Height());
//
//						TITLENO titleNoTmp;
//						titleNoTmp.nTitleNo = nTitleNo1;
//						titleNoTmp.rectLocal = rectLocal;
//
//						v_titleNoTmp.push_back(titleNoTmp);
//					}
//				}
//
//			}
//		}
//	}
//	
//	//从上之下 从左至右排序
//	for (vector<TITLENO>::iterator it = v_titleNoTmp.begin(); it != v_titleNoTmp.end(); it++)
//	{
//		for (vector<TITLENO>::iterator it2 = it+1; it2 != v_dstTitleNo.end(); it2++)
//		{
//			TITLENO titleNoTmp1 = *it;
//			TITLENO titleNoTmp2 = *it2;
//			int nX1 = titleNoTmp1.rectLocal.CenterNewPoint().nX;
//			int nY1 = titleNoTmp1.rectLocal.CenterNewPoint().nY;
//
//			int nX2 = titleNoTmp2.rectLocal.CenterNewPoint().nX;
//			int nY2 = titleNoTmp2.rectLocal.CenterNewPoint().nY;
//			
//			if (abs(nY2 - nY1) > nEPS )
//			{
//				if (nY1 > nY2)
//				{
//					*it  = titleNoTmp2;
//					*it2 = titleNoTmp1;
//				}
//			}
//			else 
//			{
//				if (nX1 > nX2)
//				{
//					*it  = titleNoTmp2;
//					*it2 = titleNoTmp1;
//				}
//			}
//		}
//	}
//
//	vector<TITLENO> v_dstTitleTmp;
//	bool bFind = FALSE;
//
//	//判断有效性 2016.01.26
//	for (vector<TITLENO>::iterator it1 = v_titleNoTmp.begin();it1 != v_titleNoTmp.end(); it1++)
//	{
//		v_dstTitleTmp.clear();
//		TITLENO titleNoTmp1 = v_modelTitleNo[0];
//		TITLENO titleNoTmp2 = *it1;
//		
//		int nMoveX = 0;
//		int nMoveY = 0;
//		if (!bFind)
//		{
//			if (titleNoTmp1.nTitleNo == titleNoTmp2.nTitleNo)
//			{
//				bFind = TRUE;
//			
//				//计算两图向量差
//				nMoveX = titleNoTmp2.rectLocal.CenterNewPoint().nX - titleNoTmp1.rectLocal.CenterNewPoint().nX;
//				nMoveY = titleNoTmp2.rectLocal.CenterNewPoint().nY - titleNoTmp1.rectLocal.CenterNewPoint().nY;
//			}
//		}
//		else 
//		{
//			vector<TITLENO> v_titleNoTmp2 = v_titleNoTmp;
//			for (vector<TITLENO>::iterator it2 = v_modelTitleNo.begin(); it2 != v_modelTitleNo.end(); it2++)
//			{
//				bool bFindTmp1 = FALSE;
//				TITLENO titleNoTmp3 = *it2;
//				NEWPOINT p1;
//				p1.setPoint(titleNoTmp3.rectLocal.CenterNewPoint().nX + nMoveX, titleNoTmp3.rectLocal.CenterNewPoint().nY + nMoveY);
//				for (int i=0; i<v_titleNoTmp2.size(); i++)
//				{
//					CPoint p2 = CPoint(p1.nX, p1.nY);
//					CRect rectCur = NewRect2Rect(v_titleNoTmp2[i].rectLocal);
//					if (GetLength(p2, rectCur.CenterPoint()) <= nEPS)
//					{
//						if (titleNoTmp3.nTitleNo == v_titleNoTmp2[i].nTitleNo)
//						{
//							bFindTmp1 = TRUE;
//							v_dstTitleTmp.push_back(v_titleNoTmp2[i]);
//							v_titleNoTmp2.erase(v_titleNoTmp2.begin()+i);
//							break;
//						}
//					}
//				}
//
//				if (!bFindTmp1)
//				{
//					bFind = FALSE;
//					break;
//				}
//			}
//
//		}
//		
//	}
//
//	if (v_dstTitleTmp.size() == v_modelTitleNo.size())
//	{
//		v_dstTitleNo = v_dstTitleTmp;
//		return TRUE;
//	}
//
//	return FALSE;
//}

BOOL CImageProcess::GetTriAngleLocalPoint(CxImage *img, const CSize dstSize, const int nThreshold, NEWTRIANGLE &dstTriangle)
{
	int nImgHeight = img->GetHeight();
	int nImgWidth = img->GetWidth();
	
	vector<CPoint> v_allPoints;
	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			if (GETGRAYVALUE1(img, i, j) <= nThreshold)
			{
				if (v_allPoints.size() > 0)
					v_allPoints.clear();

				CPoint pStart = CPoint(i, j);
				int nCount = 0;
				GetAllPoint(img, v_allPoints, pStart, nThreshold, nCount);
				CRect dstRect;
				GetRect(v_allPoints, dstRect);

				if (abs(dstSize.cx - dstRect.Width()) > nEPS ||
					abs(dstSize.cy - dstRect.Height()) > nEPS)
					continue;

				if (v_allPoints.size() >= dstRect.Width()*dstRect.Height()*0.4 &&
					v_allPoints.size() <= dstRect.Width()*dstRect.Height()*0.6)
				{
					//三角形
					//获取中心
					CPoint pCenter;
					if (GetPointsCenter(v_allPoints, pCenter))
					{
						//算出最终三角形的各个顶点 2016.02.15
						if (pCenter.x < dstRect.CenterPoint().x &&
							pCenter.y < dstRect.CenterPoint().y)
						{
							dstTriangle.pRightAngle.setPoint(dstRect.left, dstRect.top);
							dstTriangle.pAcuteAngleH.setPoint(dstRect.right, dstRect.top); 
							dstTriangle.pAcuteAngleV.setPoint(dstRect.left, dstRect.bottom);
						}
						else if (pCenter.x < dstRect.CenterPoint().x &&
								 pCenter.y > dstRect.CenterPoint().y)
						{
							dstTriangle.pRightAngle.setPoint(dstRect.left, dstRect.bottom);
							dstTriangle.pAcuteAngleH.setPoint(dstRect.right, dstRect.bottom);
							dstTriangle.pAcuteAngleV.setPoint(dstRect.left, dstRect.top);

						}
						else if (pCenter.x > dstRect.CenterPoint().x &&
							     pCenter.y < dstRect.CenterPoint().y)
						{
							dstTriangle.pRightAngle.setPoint(dstRect.right, dstRect.top);
							dstTriangle.pAcuteAngleH.setPoint(dstRect.left, dstRect.top);
							dstTriangle.pAcuteAngleV.setPoint(dstRect.right, dstRect.bottom);
						}
						else if (pCenter.x > dstRect.CenterPoint().x &&
							     pCenter.y > dstRect.CenterPoint().y)
						{
							dstTriangle.pRightAngle.setPoint(dstRect.right, dstRect.bottom);
							dstTriangle.pAcuteAngleH.setPoint(dstRect.left, dstRect.bottom);
							dstTriangle.pAcuteAngleV.setPoint(dstRect.right, dstRect.top);
						}
						else 
							continue;

						return TRUE;
					}
				}
			}
		}
	}
	

	return TRUE;
}


BOOL CImageProcess::GetPointsCenter(vector<CPoint> v_allPoints, CPoint &pCenter)
{
	int nSize = v_allPoints.size();
	if (nSize <= 0)
		return FALSE;

	int nTotal[2] = {0, 0};
	for (int i=0; i<nSize; i++)
	{
		nTotal[0] += v_allPoints[i].x;
		nTotal[1] += v_allPoints[i].y;
	}

	pCenter.x = nTotal[0]/nSize;
	pCenter.y = nTotal[1]/nSize;
	return TRUE;
}

void CImageProcess::GetBlackCount(const CxImage *img, CRect rectArea, int nThreshold, int &nCount)
{
	CxImage *img1 = (CxImage *)img;
	/*img1->Save(L"C:\\test.jpg", CXIMAGE_FORMAT_JPG);*/
	int nWidth = img->GetWidth();
	int nHeight = img->GetHeight();
	nCount = 0;
	int nLeft = max(rectArea.TopLeft().x, 0);
	int nRight = min(rectArea.BottomRight().x, nWidth);
	int nTop = max(rectArea.TopLeft().y, 0);
	int nBottom = min(rectArea.BottomRight().y, nHeight);

	for (int i=nLeft; i<=nRight; i++)
	{
		for (int j=nTop; j<=nBottom; j++)
		{
			if (GETGRAYVALUE1(img, i, j) <= nThreshold)
			{
				nCount++;
			}
		}
	}
}


bool CImageProcess::GetRectPoint(CRect &dstRect,
								 const CxImage *srcImg,
								 const int nIndex,
								 const CRect rectArea,
								 const CxImage *cropImg,
								 const int nThrshold,
								 const CSize minSize,
								 const CSize maxSize)
{	
	int nSearchLen = 5;
	int nCropWidth = cropImg->GetWidth();
	int nCropHeight = cropImg->GetHeight();
	CRect dstRectTmp;
	bool bExchange = TRUE; //相互切换标志
	CPoint pStartRect; 
	CPoint pEndRect;
	if (nIndex == 0)
	{
		pStartRect = rectArea.TopLeft();
		pEndRect = pStartRect;
	}
	else if (nIndex == 1)
	{
		CPoint p1 = rectArea.TopLeft();
		CPoint p2 = rectArea.BottomRight();
		pStartRect = CPoint(p2.x, p1.y);
		pEndRect = pStartRect;
	}
	else if (nIndex == 2)
	{
		CPoint p1 = rectArea.TopLeft();
		CPoint p2 = rectArea.BottomRight();
		pStartRect = CPoint(p1.x, p2.y);
		pEndRect  = pStartRect;
	}
	else if (nIndex == 3)
	{
		CPoint p2 = rectArea.BottomRight();
		pStartRect = p2;
		pEndRect = p2;
	}
	
	while(1)
	{
		bool bStop[2] = {FALSE, FALSE};
		if (nIndex == 0)
		{
			pStartRect.x += nSearchLen;
			
			if (pStartRect.x >= rectArea.BottomRight().x-1)
			{
				pStartRect.x = rectArea.BottomRight().x-1;
				pStartRect.y += nSearchLen;
			}

			pEndRect.y += nSearchLen;

			if (pEndRect.y >= rectArea.BottomRight().y-1)
			{
				pEndRect.y = rectArea.BottomRight().y -1;
				pEndRect.x += nSearchLen;
			}
		}
		else if (nIndex == 1)
		{
			pStartRect.x -= nSearchLen;
			pEndRect.y += nSearchLen;

			if (pStartRect.x <= rectArea.TopLeft().x+1)
			{
				pStartRect.x = rectArea.TopLeft().x + 1;
				pStartRect.y += nSearchLen;
			}

			if (pEndRect.y >= rectArea.BottomRight().y - 1)
			{
				pEndRect.y = rectArea.BottomRight().y - 1;
				pEndRect.x -= nSearchLen;
			}
		}
		else if (nIndex == 2)
		{
			pStartRect.x += nSearchLen;
			
			if (pStartRect.x >= rectArea.BottomRight().x-1)
			{
				pStartRect.x = rectArea.BottomRight().x-1;
				pStartRect.y -= nSearchLen;
			}

			pEndRect.y -= nSearchLen;

			if (pEndRect.y <= rectArea.TopLeft().y)
			{
				pEndRect.y = rectArea.TopLeft().y;
				pEndRect.x += nSearchLen;
			}
		}
		else if (nIndex == 3)
		{
			pStartRect.x -= nSearchLen;
			pEndRect.y -= nSearchLen;

			if (pStartRect.x <= rectArea.TopLeft().x)
			{
				pStartRect.x = rectArea.TopLeft().x;
				pStartRect.y -= nSearchLen;
			}

			if (pStartRect.y < rectArea.TopLeft().y)
			{
				pStartRect.y = rectArea.TopLeft().y;
				pStartRect.x -= nSearchLen;
			}
		}
		
		if (!IsPointInRect(pStartRect, rectArea))
			break;

		if (!IsPointInRect(pEndRect, rectArea))
			break;
	
		//在点p1,p2连线上查找
		CPoint pMid = CPoint((pStartRect.x + pEndRect.x)/2, (pStartRect.y + pEndRect.y)/2);//GetMidPoint(pStartRect, pEndRect);
		int nCount[2] = {0, 0};
		while (1)
		{
			CPoint pTmp;
			CRect rect;
			if (nIndex == 0)
			{
				if (bExchange)
				{
					bExchange = FALSE;
					pTmp.x = pMid.x - nCount[0]*nSearchLen;
					pTmp.y = pMid.y + nCount[0]*nSearchLen;
					nCount[0]++;
				}
				else
				{
					bExchange = TRUE;
					pTmp.x = pMid.x + nCount[1]*nSearchLen;
					pTmp.y = pMid.y - nCount[1]*nSearchLen;
					nCount[1]++;
				}

				if (pTmp.x > pStartRect.x || pTmp.y < pStartRect.y)
				{
					break;
				}

				if (pTmp.x < pEndRect.x || pTmp.y > pEndRect.y)
				{
					break;
				}

			}
			else if (nIndex == 1)
			{
				if (bExchange)
				{
					bExchange = FALSE;
					pTmp.x = pMid.x - nCount[0]*nSearchLen;
					pTmp.y = pMid.y - nCount[0]*nSearchLen;
					nCount[0]++;
				}
				else
				{
					bExchange = TRUE;
					pTmp.x = pMid.x + nCount[1]*nSearchLen;
					pTmp.y = pMid.y + nCount[1]*nSearchLen;
					nCount[1]++;
				}

				if (pTmp.x < pStartRect.x || pTmp.y < pStartRect.y)
					break;

				if (pTmp.x > pEndRect.x || pTmp.y > pEndRect.y)
					break;
			}
			else if (nIndex == 2)
			{	
				if (bExchange)
				{
					bExchange = FALSE;
					pTmp.x = pMid.x - nCount[0]*nSearchLen;
					pTmp.y = pMid.y - nCount[0]*nSearchLen;
					nCount[0]++;
				}
				else
				{
					bExchange = TRUE;
					pTmp.x = pMid.x + nCount[1]*nSearchLen;
					pTmp.y = pMid.y + nCount[1]*nSearchLen;
					nCount[1]++;
				}
				
				if (pTmp.x > pStartRect.x || pTmp.y > pStartRect.y)
					break;

				if (pTmp.x < pEndRect.x || pTmp.y < pEndRect.y)
					break;
			} 
			else if (nIndex == 3)
			{
				if (bExchange)
				{
					bExchange = FALSE;
					pTmp.x = pMid.x - nCount[0]*nSearchLen;
					pTmp.y = pMid.y + nCount[0]*nSearchLen;
					nCount[0]++;
				}
				else
				{
					bExchange = TRUE;
					pTmp.x = pMid.x + nCount[1]*nSearchLen;
					pTmp.y = pMid.y - nCount[1]*nSearchLen;
					nCount[1]++;
				}

				if (pTmp.x < pStartRect.x || pTmp.y > pStartRect.y)
					break;

				if (pTmp.x > pEndRect.x || pTmp.y < pEndRect.y)
					break;
			}
			
			bool bFindFirst = FALSE;
			if (pTmp.x  <  0|| pTmp.y < 0)
				break;

			if (pTmp.x > cropImg->GetWidth()-1 || pTmp.y > cropImg->GetHeight()-1)
				break;

			int nGrayValue = GETGRAYVALUE1(cropImg, pTmp.x, pTmp.y);//cropImg->GetPixelGray(pTmp.x, nCropHeight-1-pTmp.y);

			rect = rectArea;

			if (nGrayValue > nThrshold) //搜索白点
			{
				bFindFirst = TRUE;
			     BOOL bRes = FALSE;

				if (nIndex == 0)
				{
					rect.SetRect(pTmp, rectArea.BottomRight());
				}
				else if (nIndex == 1)
				{
					rect.SetRect(CPoint(rectArea.TopLeft().x, pTmp.y), CPoint(pTmp.x, rectArea.BottomRight().y-1));
				}
				else if (nIndex == 2)
				{
					rect.SetRect(CPoint(pTmp.x, rectArea.TopLeft().y), CPoint(rectArea.BottomRight().x, pTmp.y));
				}
				else if (nIndex == 3)
				{
					rect.SetRect(rectArea.TopLeft(), pTmp);
				}


				bRes = GetRectFromRect1(dstRectTmp, rect, cropImg, minSize, maxSize, nIndex, srcImg, nThrshold);
			
				if (bRes)
				{
					/*CPoint pStart1 = dstRectTmp.TopLeft();
					CPoint pEnd1 = dstRectTmp.BottomRight();

					pStart1.x += rectArea.TopLeft().x;
					pStart1.y += rectArea.TopLeft().y;

					pEnd1.x += rectArea.TopLeft().x;
					pEnd1.y += rectArea.TopLeft().y;
					dstRect.SetRect(pStart1, pEnd1);*/
					dstRect = dstRectTmp;
					return TRUE;
				}
			}
			else
			{
			
				if (bFindFirst)
				{
					BOOL bRes = GetRectFromRect1(dstRectTmp, rect, cropImg, minSize, maxSize, nIndex, srcImg, nThrshold);
					if (bRes)
					{
						dstRect = dstRectTmp;
						//获取矩形
						if (nIndex == 0 /*|| nIndex == 1*/)
						{
							//判断正上方是否含有矩形
							CRect rectArea1;
							CPoint p1 = dstRectTmp.CenterPoint();
							CPoint pStart1 = CPoint(p1.x, p1.y - 2*dstRectTmp.Height());

							if (pStart1.x < 0)
								pStart1.x = 0;

							if (pStart1.y < 0)
								pStart1.y = 0;

							CPoint pTopLeft1 = CPoint(pStart1.x - dstRectTmp.Width()/2-10, pStart1.y - dstRectTmp.Height()/2-10);
							CPoint pBottomRight1 = CPoint(pStart1.x + dstRectTmp.Width()/2+ 10, pStart1.y + dstRectTmp.Height()/2+10);

							if (pBottomRight1.x > cropImg->GetWidth()-1)
								pBottomRight1.x = cropImg->GetWidth()-1;

							if (pBottomRight1.y > cropImg->GetHeight()-1)
								pBottomRight1.y = cropImg->GetHeight()-1;
							rectArea1.SetRect(pTopLeft1, pBottomRight1);

							int nGrayValue =  GETGRAYVALUE1(cropImg, pStart1.x, pStart1.y);// cropImg->GetPixelGray(pStart1.x, nCropHeight-1-pStart1.y);
							if (nGrayValue <= nThrshold)
							{
								//int nThreshold = m_nThreshold;
								bool bRes1 = GetRectFromPoint(dstRectTmp, pStart1, rectArea1, CSize(dstRectTmp.Width(), dstRectTmp.Height()), cropImg, nThrshold);
								if (bRes1 && !IsPointInRect(dstRectTmp.CenterPoint(), dstRect))
								{
									/*CPoint pStart1 = dstRectTmp.TopLeft();
									CPoint pEnd1 = dstRectTmp.BottomRight();

									pStart1.x += rectArea.TopLeft().x;
									pStart1.y += rectArea.TopLeft().y;

									pEnd1.x += rectArea.TopLeft().x;
									pEnd1.y += rectArea.TopLeft().y;
									dstRect.SetRect(pStart1, pEnd1);*/

									dstRect = dstRectTmp;
								}
							}
						}
						
						return TRUE;
					}
					else
					{
						bFindFirst = FALSE;
						break;
					}
				}
			}
		}
	}
	return FALSE;
}

BOOL CImageProcess::GetRectFromRect1(CRect &dstRect, 
									 CRect rectArea,
									 const CxImage *img,
									 CSize minSize, 
									 CSize maxSize, 
									 int nIndex,
									 const CxImage *pSrcImg,
									 const int nThreshold)
{
	int imgWidth = img->GetWidth();
	int imgHeight = img->GetHeight();
	
	if (nIndex == 0)
	{
		for (int i=rectArea.TopLeft().y; i<rectArea.BottomRight().y; i++)
		{
			for (int j=rectArea.TopLeft().x; j<rectArea.BottomRight().x; j++)
			{
				CPoint pStart = CPoint(j, i);
				if (pStart.x < 0 || pStart.y  <0)
					continue;

				if (pStart.x  > imgWidth-1 || pStart.y > imgHeight-1)
					continue;

				int nGrayValue = GETGRAYVALUE1(img, j, i);//img->GetPixelGray(j, imgHeight-1-i);
				if (nGrayValue <= nThreshold)
				{
					CRect dstRectTmp;
					CPoint pStart = CPoint(j, i);

					if (pStart.x >= 120)
						int ii =0;
				    
					bool bRes = GetRectFromPoint1(dstRectTmp, pStart, rectArea, img, minSize, maxSize, nIndex, nThreshold, pSrcImg);
					if (bRes)
					{
						if (GetDensity(img, dstRectTmp, nThreshold) < 0.85)
							continue;

						if (dstRectTmp.Width() < dstRectTmp.Height())
							continue;

						dstRect = dstRectTmp;
						return TRUE;
					}
					j += minSize.cx;	
				}	
			}
		}
	}
	else if (nIndex == 1)
	{
		//img->Save(L"D:\\testTmp.jpg", CXIMAGE_FORMAT_JPG);
		for (int i=rectArea.TopLeft().y; i<rectArea.BottomRight().y; i++)
		{
			for (int j=rectArea.BottomRight().x-1; j>=rectArea.TopLeft().x; j--)
			{
				CPoint pStart = CPoint(j, i);
				if (pStart.x < 0 || pStart.y < 0)
					continue;

				if (pStart.x > imgWidth-1 || pStart.y > imgHeight-1)
					continue;
				int nGrayValue = GETGRAYVALUE1(img, j, i);//img->GetPixelGray(j, imgHeight-1-i);
				if (nGrayValue <= nThreshold)
				{
				
					CRect dstRectTmp;
					CPoint pStart = CPoint(j, i);
					bool bRes = GetRectFromPoint1(dstRectTmp, pStart, rectArea, img, minSize, maxSize, nIndex, nThreshold, pSrcImg);
					if (bRes)
					{
						/*double dDensity = GetDensity(img, dstRectTmp, nThreshold);*/
						if (GetDensity(img, dstRectTmp, nThreshold) < 0.85)
							continue;

						if (dstRectTmp.Width() < dstRectTmp.Height())
							continue;

						dstRect = dstRectTmp;
						//TRACE("%d, %d\n",nIndex, GetDensity(img, dstRectTmp));
						return TRUE;
					}
					j -= minSize.cx;

				}

			}
		}
	}
	else if (nIndex == 2)
	{
		for (int i=rectArea.BottomRight().y-1; i>=rectArea.TopLeft().y; i--)
		{
			for (int j=rectArea.TopLeft().x; j < rectArea.BottomRight().x; j++)
			{
				CPoint pStart = CPoint(j, i);
				if (pStart.x< 0 || pStart.y<0)
					continue;

				if (pStart.x > imgWidth-1 || pStart.y > imgHeight-1)
					continue;
				int nGrayValue = GETGRAYVALUE1(img, j, i);//img->GetPixelGray(j, imgHeight-1-i);
				if (nGrayValue <= nThreshold)
				{
					CRect dstRectTmp;
					CPoint pStart = CPoint(j, i);

					bool bRes = GetRectFromPoint1(dstRectTmp, pStart, rectArea, img, minSize, maxSize, nIndex, nThreshold, pSrcImg);
					if (bRes)
					{
						if (GetDensity(img, dstRectTmp, nThreshold) < 0.85)
							continue;

						if (dstRectTmp.Width() < dstRectTmp.Height())
							continue;

						dstRect = dstRectTmp;
						return TRUE;
					}
					j += minSize.cx;

				}
			}
		}
	}
	else if (nIndex == 3)
	{
		for (int i=rectArea.BottomRight().y-1; i >= rectArea.TopLeft().y; i--)
		{
			for (int j=rectArea.BottomRight().x-1; j>=rectArea.TopLeft().x; j--)
			{
				CPoint pStart = CPoint(j, i);
				if (pStart.x < 0 || pStart.y <0)
					continue;

				if (pStart.x> imgWidth-1 || pStart.y > imgHeight-1)
					continue;

				int nGrayValue = GETGRAYVALUE1(img, j, i);//img->GetPixelGray(j, imgHeight-1-i);
				if (nGrayValue <= nThreshold)
				{
					CRect dstRectTmp;
					CPoint pStart = CPoint(j, i);

					bool bRes = GetRectFromPoint1(dstRectTmp, pStart, rectArea, img, minSize, maxSize, nIndex, nThreshold, pSrcImg);
					if (bRes)
					{
						if (GetDensity(img, dstRectTmp, nThreshold) < 0.85)
							continue;

						if (dstRectTmp.Width() < dstRectTmp.Height())
							continue;

						dstRect = dstRectTmp;
						return TRUE;
					}
					j -= minSize.cx;
				}
			}
		}
	}
	return FALSE;
}

BOOL CImageProcess::GetRectFromPoint1(CRect &dstRect, CPoint pStart, CRect rectArea,const CxImage *img, CSize minSize, CSize maxSize, int nIndex,const int nThreshold, const CxImage* pSrcImg)
{
	int nGrayValue;
	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();

	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart .y > nImgHeight-1)
		return FALSE;

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);

	if (nGrayValue > nThreshold)
		return FALSE;

	CPoint pTopExtream[2];
	CPoint pBottomExtream[2];
	for (int i=0; i<2; i++)
	{
		pTopExtream[i]    = pStart;
		pBottomExtream[i] = pStart;
	}

	CPoint pStartTmp = pStart;
	int nFind = -1;
	while (nFind<=3) //向上搜索
	{
		CPoint pTmp[2];
		if (GetHLine1(pTmp, pStartTmp, rectArea, img, minSize, maxSize, nThreshold))
		{
			for (int i=0; i<2; i++)
			{
				pTopExtream[i] = pTmp[i];

			}
			pStartTmp.x = (pTopExtream[0].x + pTopExtream[1].x)/2;
			nFind = 0;
		}
		else 
		{
			nFind++;
		}

		pStartTmp.y -= 2; 
		if (pStartTmp.y < rectArea.TopLeft().y)
			break;

		if (pStartTmp.x < 0 || pStartTmp.y < 0)
		{
			nFind++;
			continue;
		}

		if (pStartTmp.x > nImgWidth-1 || pStartTmp.y > nImgHeight-1)
		{
			nFind++;
			continue;
		}
		nGrayValue = GETGRAYVALUE1(img, pStartTmp.x, pStartTmp.y);
	}

	//判断有效性
	if (pTopExtream[1].x - pTopExtream[0].x < minSize.cx ||
		pTopExtream[1].x - pTopExtream[0].x > maxSize.cx)
		return FALSE;

	pStartTmp = pStart;
	nFind = -1;
	while(nFind<=3)
	{
		CPoint pTmp[2];
		if (GetHLine1(pTmp, pStartTmp, rectArea, img, minSize, maxSize, nThreshold))
		{
			for (int i=0; i<2; i++)
			{
				pBottomExtream[i] = pTmp[i];
			}
			pStartTmp.x = (pBottomExtream[0].x + pBottomExtream[1].x)/2;
			nFind = 0;
		}
		else 
		{
			nFind++;
		}

		pStartTmp.y += 2;
		if (pStartTmp.y > rectArea.BottomRight().y-1)
			break;
	}

	if (pBottomExtream[1].x - pBottomExtream[0].x < minSize.cx ||
		pBottomExtream[1].x - pBottomExtream[0].x > maxSize.cx )
		return FALSE;

	CPoint pTopleft;
	CPoint pBottomright;
	pTopleft.x = min(pTopExtream[0].x, pBottomExtream[0].x);
	pTopleft.y = pTopExtream[0].y;
	pBottomright.x = max(pTopExtream[1].x, pBottomExtream[1].x);
	pBottomright.y = pBottomExtream[1].y;

	CRect rectTmp;
	rectTmp.SetRect(pTopleft, pBottomright);
	int nWidth = rectTmp.Width();
	int nHeight = rectTmp.Height();

	int nSrcWidth = pSrcImg->GetWidth();
	int nSrcHeight = pSrcImg->GetHeight();
	if (nWidth >= minSize.cx && nHeight >= minSize.cy && 
		nWidth <= maxSize.cx && nHeight <= maxSize.cy)
	{

		if (m_dstBlackSize.cx > 0 &&
			m_dstBlackSize.cy > 0)
		{
			if (abs(nWidth  -  m_dstBlackSize.cx) <= nEPS &&
				abs(nHeight - m_dstBlackSize.cy)  <= nEPS)	
			{
				dstRect = rectTmp;
				//return TRUE;
			}
			else 
				return FALSE;

		}

		//判断矩形是否合法
		CRect rectExpand;
		int nExpand = min(rectTmp.Width()/2, rectTmp.Height()/2);
		if (nExpand == 0)
			nExpand = 5;
		CPoint p1 = CPoint(rectTmp.TopLeft().x-nExpand, rectTmp.TopLeft().y - nExpand);
		if (p1.x < 0)
			p1.x = 0;

		if (p1.y < 0)
			p1.y = 0;

		CPoint p2 = CPoint(p1.x + rectTmp.Width() + 2*nExpand, p1.y + rectTmp.Height() + 2*nExpand);

		int nNewWidth = p2.x - p1.x;
		int nNewHeight = p2.y - p1.y;

		if (p1.x < 0)
			p1.x =0;

		if (p1.y < 0)
			p1.y = 0;

		if (p2.x > nSrcWidth-1)
			p2.x = nSrcWidth-1;

		if (p2.y > nSrcHeight-1)
			p2.y = nSrcHeight-1;

		if (nIndex == 0)
		{
			p2.x = p1.x + nNewWidth;
			p2.y = p1.y + nNewHeight;
		}
		else if (nIndex == 1)
		{
			p1.x = p2.x - nNewWidth;
			p2.y = p1.y + nNewHeight;
		}
		else if (nIndex == 2)
		{
			p2.x = p1.x + nNewWidth;
			p1.y = p2.y - nNewHeight;
		}
		else if (nIndex == 3)
		{
			p1.x = p2.x - nNewWidth;
			p1.y = p2.y - nNewHeight;
		}

		//计算区域内白点密度
		rectExpand.SetRect(p1, p2);
		int nCount = 0;
		for (int i=p1.x; i<p1.x + rectExpand.Width(); i++)
		{
			for (int j=p1.y; j<p1.y + rectExpand.Height(); j++)
			{
				CPoint pStart1 = CPoint(i, j);
				if (pStart1.x < 0 || pStart1.y < 0)
					continue;

				if (pStart1.x > nImgWidth-1 || pStart1.y > nImgHeight-1)
					continue;

				nGrayValue = GETGRAYVALUE1(img, i, j);
				if (nGrayValue > nThreshold && !IsPointInRect(CPoint(i, j), rectTmp))
				{
					nCount++;
				}
			}
		}
		double dDensity = double(nCount+rectTmp.Width()*rectTmp.Height())/double(rectExpand.Width()*rectExpand.Height());
		if (dDensity >= 0.80)
		{
			/*if (m_dstBlackSize.cx < 0|| m_dstBlackSize.cy < 0)
			{
				m_dstBlackSize = CSize(rectTmp.Width(), rectTmp.Height());
			}*/

			dstRect = rectTmp;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL  CImageProcess::GetHLine1(CPoint pExtream[2], CPoint pStart, CRect rectArea, const CxImage *img, CSize minSize, CSize maxSize, const int nThreshold)
{
	CPoint pTmp[2];
	int nGrayValue;
	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();
	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
		return FALSE;

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);
	if (nGrayValue > nThreshold)
		return FALSE;

	for (int i=0; i<2; i++)
	{
		pTmp[i] = pStart;
	}

	nGrayValue = GETGRAYVALUE1(img, pStart.x, pStart.y);
	while (nGrayValue <= nThreshold)
	{
		pTmp[0].x -= 2;
		if (pTmp[0].x < rectArea.TopLeft().x || pTmp[0].x > rectArea.BottomRight().x-1)
			break;
		if (pTmp[0].y < rectArea.TopLeft().y || pTmp[0].y > rectArea.BottomRight().y-1)
			break;

		if (pTmp[0].x <0 || pTmp[0].y < 0)
			break;

		if (pTmp[0].x > nImgWidth-1 || pTmp[0].y > nImgHeight-1)
			break;

		nGrayValue = GETGRAYVALUE1(img, pTmp[0].x, pTmp[0].y);
	}

	nGrayValue = GETGRAYVALUE1(img, pTmp[1].x, pTmp[1].y);
	while (nGrayValue <= nThreshold)
	{
		pTmp[1].x+= 2;
		if (pTmp[1].x < rectArea.TopLeft().x || pTmp[1].x > rectArea.BottomRight().x-1)
			break;

		if (pTmp[1].y < rectArea.TopLeft().y || pTmp[1].y > rectArea.BottomRight().y-1)
			break;

		if (pTmp[1].x <0 || pTmp[1].y < 0)
			break;

		if (pTmp[1].x > nImgWidth-1 || pTmp[1].y > nImgHeight-1)
			break;

		nGrayValue = GETGRAYVALUE1(img, pTmp[1].x, pTmp[1].y);
	}
	int nLen = pTmp[1].x - pTmp[0].x;
	if (nLen >= minSize.cx && nLen <= maxSize.cx)
	{
		pExtream[0] = pTmp[0];
		pExtream[1] = pTmp[1];
		return TRUE;
	}

	return FALSE;
}

double CImageProcess::GetDensity(const CxImage *img,  CRect rect, const int nThreshold)
{
	double dDensity = 0.0;
	int nWidth = rect.Width();
	int nHeigtht = rect.Height();
	if (nWidth*nHeigtht == 0)
		return dDensity;

	int nImgWidth = img->GetWidth();
	int nImgHeight = img->GetHeight();
	int nCount = 0;
	for (int i=rect.TopLeft().x; i<rect.TopLeft().x + nWidth; i++)
	{ 
		for (int j=rect.TopLeft().y; j<rect.TopLeft().y + nHeigtht; j++)
		{

			CPoint pStart =CPoint(i, j);
			if (pStart.x < 0 || pStart.y < 0)
				continue;

			if (pStart.x > nImgWidth-1 || pStart.y > nImgHeight-1)
				continue;
			int nGrayValue = GETGRAYVALUE1(img, i, j);
			if (nGrayValue <= nThreshold)
				nCount++;
		}
	}

	dDensity = double(nCount)/double(max(nWidth*nHeigtht, 1)); //获取填涂密度值
	return dDensity;
}

double CImageProcess::GetHollowDensity( CxImage *cropImg, const int nThreshold, const CSize centerSize)
{
	double dDensity = 0.0;
	int nWidth = cropImg->GetWidth();
	int nHeight = cropImg->GetHeight();
	cropImg->Threshold(nThreshold);
	cropImg->GrayScale();

	int *nValue = new int[nWidth*nHeight];
	memset(nValue, 1, nWidth*nHeight);
	for (int i=0; i<nHeight; i++)
	{
		if (i==0 || i==nHeight-1)
			continue;

		for (int j=0; j<nWidth; j++)
		{
			if (GETGRAYVALUE1(cropImg, j, i) == 0)
			{
				nValue[i*nWidth + j] = 0;
				if (j==0 || j==nWidth-1)
					continue;

				if (GETGRAYVALUE1(cropImg,j,i-1) == 0 && GETGRAYVALUE1(cropImg,j-1, i) == 0 &&
					GETGRAYVALUE1(cropImg,j,i+1) == 0 && GETGRAYVALUE1(cropImg,j+1, i) == 0)
				{
						nValue[i*nWidth + j] = 1;
				}
				else 
					nValue[i*nWidth + j] = 0;
			}
			else
				nValue[i*nWidth + j] = 1;
		}
	}

	int nBlackCount = 0;
	for (int i=max(0, nWidth/2-centerSize.cx/2); i<min(nWidth-1, nWidth/2+centerSize.cx/2); i++)
	{
		for (int j=max(0, nHeight/2-centerSize.cy/2); j<min(nHeight-1, nHeight/2 + centerSize.cy/2); j++)
		{
			if (nValue[j*nWidth+i] == 0)
			{
				nBlackCount++;
			}
		}
	}

	delete []nValue;
	nValue = NULL;
	dDensity = double(nBlackCount)/double(max(1, centerSize.cx*centerSize.cy));
	return dDensity;
}


int CImageProcess::GetCrossPointsCount(CRect rect1, CRect rect2)
{
	int nCount = 0;
	for (int i=rect1.TopLeft().x; i <= rect1.BottomRight().x; i++)
	{
		for (int j=rect1.TopLeft().y; j <= rect1.BottomRight().y; j++)
		{
			CPoint p1 = CPoint(i, j);
			if (IsPointInRect(p1, rect2))
				nCount++;
		}
	}
	
	return nCount;
}

bool  CImageProcess::GetVLinelen( CxImage srcImg, int nThreshold, int &nLen1)
{
	int nWidth = srcImg.GetWidth();
	int nHeight = srcImg.GetHeight();

	//// 计算链码 2016.06.21
	//// 计算右上角点 
	//CPoint pStart = CPoint(0, 0); //定义起始点
	//int nMinLen = 8000;
	//for (int i=0; i<nHeight; i++)
	//{
	//	for (int j=0; j<nWidth; j++)
	//	{
	//		if (GETGRAYVALUE2(srcImg, j, i) <= nThreshold)
	//		{
	//			int nLen = (int)GetLength(CPoint(j, i), CPoint(nWidth-1, 0));
	//			//TRACE("%d, %d, len:%d\n", j, i, nLen);
	//			if (nLen < nMinLen)
	//			{
	//				pStart = CPoint(j, i); 
	//				nMinLen = nLen;
	//			}
	//		}
	//	}
	//}

	////计算链码 
	////struct CHAINCODE
	////{
	////	CPoint pCodeStart;
	////	int nDirIndex; //链码方向
	////	
	////	CHAINCODE()
	////	{
	////		nDirIndex = -1;
	////	}
	////};

	//vector<CPoint> v_AllPoints;
	//int nLastIndex = -1;
	//CPoint pStatrTmp = pStart;
	//CString strIndex = _T("");
	//v_AllPoints.push_back(pStatrTmp);
	//for (int i=0; i<nHeight; i++)
	//{
	//	for (int j=0; j<nWidth; j++)
	//	{
	//		CPoint p[8];
	//		for (int k=0; k<8; k++)
	//		{
	//			p[k] = pStatrTmp;
	//		}

	//		p[0].x += 1;
	//		p[1].x += 1;
	//		p[1].y -= 1;
	//		p[2].y -= 1;
	//		p[3].x -= 1;
	//		p[3].y -= 1;
	//		p[4].x -= 1;
	//		p[5].x -= 1;
	//		p[5].y += 1;
	//		p[6].y += 1;
	//		p[7].x += 1;
	//		p[7].y += 1;

	//		for (int k=0; k<8; k++)
	//		{

	//			if (abs(nLastIndex - k) == 4 && nLastIndex > 0)
	//				continue;

	//			if (p[k].x < 0 || p[k].x > nWidth-1)
	//				continue;

	//			if (p[k].y < 0 || p[k].y > nHeight-1)
	//				continue;

	//			bool bFind = false;
	//			for (vector<CPoint>::iterator it = v_AllPoints.begin(); it != v_AllPoints.end(); it++)
	//			{
	//				CPoint p1 = *it;
	//				if (p1 == p[k])
	//				{
	//					bFind = true;
	//					break;
	//				}
	//			}

	//			if (bFind)
	//				continue;

	//			if (GETGRAYVALUE2(srcImg, p[k].x, p[k].y) <= nThreshold)
	//			{
	//				nLastIndex = k;
	//				pStatrTmp = p[k];
	//				CString strTmp;
	//				strTmp.Format(L"%d", k);
	//				strIndex += strTmp;
	//				v_AllPoints.push_back(p[k]);
	//				break;
	//			}

	//		}

	//	}
	//}
	//	
	int nMaxCount = 0;
	int k =0;
	int nLastBlackCount = 0;
	for (int i=0; i<nWidth; i++)
	{
		int nBlackCount = 0;
		for (int j=0; j<nHeight; j++)
		{
			if (GETGRAYVALUE2(srcImg, i, j) <= nThreshold)
			{
				nBlackCount++;
			}
		}

		if (nBlackCount > nMaxCount)
		{
			nMaxCount = nBlackCount;
			k = i;
		}
		
		nLastBlackCount = nBlackCount;
	}

	for (int i=5 ; i<=nHeight-5; i++)
	{
		bool bFindWhite = false;
		bool bFindBlack[2] ={false, false};
		for (int j=0; j<nWidth; j++)
		{
			if (GETGRAYVALUE2(srcImg, j, i) > nThreshold)
			{
				if (bFindBlack[0])
					bFindWhite = true;
			}
			else 
			{
				if (!bFindWhite)
					bFindBlack[0] = true;
				else 
					bFindBlack[1] = true;
			}

			if (bFindBlack[0] && bFindBlack[1])
				return false;
		}
		
	}

	nLen1 = nMaxCount;
	return true;
}



void CImageProcess::GetThinImage(CxImage *cropImg, int nThreshold, int nTimes )
{
	cropImg->Threshold(nThreshold);
	cropImg->GrayScale();
	
	int nImgWidth = cropImg->GetWidth();
	int nImgHeight = cropImg->GetHeight();
	int nWidth = cropImg->GetWidth()-1;
	int nHeight = cropImg->GetHeight()-1;

	int *nValue = new int[nImgWidth*nImgHeight]; 
	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			if (GETGRAYVALUE1(cropImg, i, j) == 255)
				nValue[j*nImgWidth + i] = 0;
			else 
				nValue[j*nImgWidth + i] = 1;
		}
	}
	
	int p[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	for (int n=0; n<nTimes; n++)
	{
		 for (int i=1; i<nWidth; i++)
		 {
			 for (int j=1; j<nHeight; j++)
			 {
				 int p1 = nValue[j*nImgWidth + i];
				 if (p1 == 0)
					 continue;

				 p[0] = nValue[(j-1)*nImgWidth + i];
				 p[1] = nValue[(j-1)*nImgWidth + i+1];
				 p[2] = nValue[j*nImgWidth + i+1];
				 p[3] = nValue[(j+1)*nImgWidth + i+1];
				 p[4] = nValue[(j+1)*nImgWidth + i];
				 p[5] = nValue[(j+1)*nImgWidth + i-1];
				 p[6] = nValue[j*nImgWidth + i-1];
				 p[7] = nValue[(j-1)*nImgWidth + i-1];

				int nCount = 0;
				int nTotal = 0;
				for (int m=0; m<8; m++)
					nCount += p[m];

				if (p[0] == 0 && p[1] == 1)
					nTotal++;
				
				if (p[1] == 0 && p[2] == 1)
					nTotal++;

				if (p[2] == 0 && p[3] == 1)
					nTotal++;

				if (p[3] == 0 && p[4] == 1)
					nTotal++;

				if (p[4] == 0 && p[5] == 1)
					nTotal++;
 
				if (p[5] == 0 && p[6] == 1)
					nTotal++;

				if (p[6] == 0 && p[7] == 1)
					nTotal++;

				if (p[7] == 0 && p[0] == 1)
					nTotal++;

				if (nCount >= 2 && nCount <= 6
					&& nTotal == 1
					&& p[0]*p[2]*p[6] == 0 
					&& p[0]*p[4]*p[6] == 0
					)
				{ 
					p1 = 0;
					nValue[j*nImgWidth + i] = 0;
				}
				
			 }
		 }
	}

	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			if (nValue[j*nImgWidth + i] == 0)
				SETGRAYVALUE(cropImg->info.pImage, nImgWidth, nImgHeight, i, j, 255);
			else
				SETGRAYVALUE(cropImg->info.pImage, nImgWidth, nImgHeight, i, j, 0);
		}
	}

	delete []nValue;
	nValue = NULL;
}


CString CImageProcess::GetMoudulePath()
{
	CString strRetun = _T("");
#ifdef _UNICODE
	TCHAR szBuff[MAX_PATH];
	HMODULE module = GetCurrentModule();
	//HMODULE module = GetModuleHandle(0); 
	GetModuleFileName(module, szBuff, sizeof(szBuff)); 
	strRetun.Format(_T("%s"),szBuff);
#else
	HMODULE module = GetModuleHandle(0); 
	CHAR szBuff[MAX_PATH]; 
	GetModuleFileName(module, szBuff, sizeof(szBuff)); 
	strRetun.Format(_T("%s"),szBuff);
#endif 
	int pos = strRetun.ReverseFind(_T('\\'));
	if(pos != -1)
	{
		strRetun = strRetun.Left(pos);
	}
	return strRetun;
}

void CImageProcess::GetChainContour(CxImage srcImg, int nThreshold, CSize dstSize, vector<OMRRECT> &v_AllRects)
{
	IplImage *src = NULL;
	Cximage2Iplimage_1(&srcImg, &src);
	IplImage *dsw = cvCreateImage(cvGetSize(src), 8, 1);       
	CvMemStorage *storage = cvCreateMemStorage(0);    
	CvSeq *first_contour = NULL;    
	cvThreshold(src, dsw, nThreshold, 255, CV_THRESH_BINARY);    
	cvFindContours(dsw, storage, &first_contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);    
	for(; first_contour != 0; first_contour = first_contour->h_next)    
	{      
		CvRect rect = cvBoundingRect(first_contour,0);  
		CRect rect1;
		
		if (rect.height < dstSize.cy/2)
			continue;
		
		if (rect.width > dstSize.cx + nEPS)
			continue;

		if (abs(rect.height - dstSize.cy) > nEPS)
			continue;

		if (rect.width > 2*dstSize.cx)
			continue;

		rect1.SetRect(rect.x, rect.y, rect.x+rect.width, rect.y + rect.height);
		OMRRECT omrRect1;
		omrRect1.rect = rect1;
		v_AllRects.push_back(omrRect1);
	}  
	cvReleaseImage(&src);
	src = NULL;
	cvReleaseImage(&dsw);
	dsw = NULL;
	cvClearMemStorage(storage);
	cvReleaseMemStorage(&storage);  
}

 void CImageProcess::GetCharContour(CxImage srcImg,  int nThreshold, CRect &dstRect)
 {
	 dstRect.SetRect(0, 0, srcImg.GetWidth()-1, srcImg.GetHeight()-1);
	 vector<CRect> v_AllRects;
	 IplImage *src = NULL;   
	 Cximage2Iplimage_1(&srcImg, &src);
	 IplImage *dsw = cvCreateImage(cvGetSize(src), 8, 1);       
	 CvMemStorage *storage = cvCreateMemStorage(0);    
	 CvSeq *first_contour = NULL;    
	 cvThreshold(src, dsw, nThreshold, 255, CV_THRESH_BINARY);    
	 cvFindContours(dsw, storage, &first_contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);    
	 for(; first_contour != 0; first_contour = first_contour->h_next)    
	 {      
		 CvRect rect = cvBoundingRect(first_contour,0);  
		 CRect rect1;

		 if (rect.height*rect.width < nEPS)
			 continue;
		
		 if (rect.width*rect.height >= (srcImg.GetWidth()-2)*(srcImg.GetHeight()-2))
			 continue;

		 rect1.SetRect(rect.x, rect.y, rect.x+rect.width, rect.y + rect.height);
		 v_AllRects.push_back(rect1);
	 }    

	 ////删除重叠项 2016.07.28
	 //for (int i=0; i<v_AllRects.size(); i++)
	 //{
		// for (int j=i+1; j<v_AllRects.size(); j++)
		// {
		//	 CRect rect1 = v_AllRects[i];
		//	 CRect rect2 = v_AllRects[j];

		//	 if (IsPointInRect(rect1.CenterPoint(), rect2))
		//	 {
		//		 if(rect1.Width()*rect1.Height() < rect2.Width()*rect2.Height())
		//		 {
		//			 v_AllRects.erase(v_AllRects.begin()+i);
		//			 i -= 1;
		//			 break;
		//		 }
		//		 else
		//		 {
		//			 v_AllRects.erase(v_AllRects.begin()+j);
		//			 j -= 1;
		//		 }
		//	 }
		// }
	 //}

	 if (v_AllRects.size() > 0)
	 {
		 dstRect = v_AllRects[0];

		 CPoint pTopLeft = v_AllRects[0].TopLeft();
		 CPoint pBottomRight = v_AllRects[0].BottomRight();
		 for (vector<CRect>::iterator it = v_AllRects.begin(); it != v_AllRects.end(); it++)
		 {
			 CRect rectTmp = *it;
			 if (pTopLeft.x + pTopLeft.y > rectTmp.TopLeft().x + rectTmp.TopLeft().y)
				 pTopLeft = rectTmp.TopLeft();

			 if (pBottomRight.x + pBottomRight.y < rectTmp.BottomRight().x + rectTmp.BottomRight().y)
				 pBottomRight = rectTmp.BottomRight();

			 /*if (dstRect.Width()*dstRect.Height() < rectTmp.Width()*rectTmp.Height())
				 dstRect = rectTmp;*/
		 }

		 dstRect.SetRect(pTopLeft, pBottomRight);
	 }
	
	 cvReleaseMemStorage(&storage);  
	 storage = NULL;
	 cvReleaseImage(&dsw);
	 dsw = NULL;
	 cvReleaseImage(&src);
	 src = NULL;
 }

void CImageProcess::GetChainContour_Encolsed(CxImage srcImg,  int nThreshold, CSize dstSize, vector<CRect> &v_AllRects)
{
	IplImage *src = NULL;   
	Cximage2Iplimage_1(&srcImg, &src);

	IplImage *dsw = cvCreateImage(cvGetSize(src), 8, 1);       
	CvMemStorage *storage = cvCreateMemStorage(0);    
	CvSeq *first_contour = NULL;    
	cvThreshold(src, dsw, nThreshold, 255, CV_THRESH_BINARY);    
	cvFindContours(dsw, storage, &first_contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);    
	for(; first_contour != 0; first_contour = first_contour->h_next)    
	{      
		CvRect rect = cvBoundingRect(first_contour,0);  
		CRect rect1;

		if (rect.height < dstSize.cy/2)
			continue;

		if (rect.width < dstSize.cx/2)
			continue;

		if (abs(rect.height - dstSize.cy) > nEPS/*max(nEPS, dstSize.cy*0.1)*/)
			continue;

		if (abs(rect.width - dstSize.cx) > nEPS/*max(nEPS, dstSize.cx*0.1)*/)
			continue;
		rect1.SetRect(rect.x, rect.y, rect.x+rect.width, rect.y + rect.height);
		v_AllRects.push_back(rect1);
	}    

	//删除重叠项 2016.07.28
	for (int i=0; i<v_AllRects.size(); i++)
	{
		for (int j=i+1; j<v_AllRects.size(); j++)
		{
			CRect rect1 = v_AllRects[i];
			CRect rect2 = v_AllRects[j];

			if (IsPointInRect(rect1.CenterPoint(), rect2))
			{
				if(rect1.Width()*rect1.Height() < rect2.Width()*rect2.Height())
				{
					v_AllRects.erase(v_AllRects.begin()+i);
					i -= 1;
					break;
				}
				else
				{
					v_AllRects.erase(v_AllRects.begin()+j);
					j -= 1;
				}
			}
		}
	}

	
	cvReleaseMemStorage(&storage);  
	storage = NULL;
	cvReleaseImage(&dsw);
	dsw = NULL;
	cvReleaseImage(&src);
	src = NULL;
	
}

bool CImageProcess::Cximage2Iplimage_1(CxImage *src, IplImage **dst)
{
	src->GrayScale();
	int nPalatteCount = src->GetPaletteSize()/sizeof(RGBQUAD);;    
	RGBQUAD *pPal = src->GetPalette();    
	//int iBackColor = GetBlackColor(*src);    
	long i = 0,j = 0;    
	long nImageWidth = 0,nImageHeight = 0;    
	nImageWidth = src->GetWidth();    
	nImageHeight = src->GetHeight();    
	long nBitCunt = src->GetBpp();    
 
    if(nBitCunt<=8)    
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
	return true;    
}


int CImageProcess::GetMaxFillHeight(CxImage cropImg, int nThreshold, CSize dstSize)
{
	int nWidth = cropImg.GetWidth();
	int nHeight = cropImg.GetHeight();
	CRect rectArea;
	rectArea.SetRect(0, 0, nWidth-1, nHeight-1);
	int nMaxHeight = 0;
	for (int i=0; i<nHeight; i++)
	{
		for (int j=0; j<nWidth; j++)
		{
			CPoint pExtream[2];
			CPoint pStart = CPoint(j, i);
			if (GETGRAYVALUE2(cropImg, pStart.x, pStart.y) <= nThreshold)
			{
				if (GetHLine_OMR(pExtream, pStart, rectArea, dstSize, &cropImg, nThreshold, 3))
				{
					nMaxHeight++;
					break;
				}
			}

		}

	}
	return nMaxHeight;
}

////获取矩形框  2016.06.28
//bool CImageProcess::GetIcrRect(CxImage srcImg, CSize dstSize, int nThreshold, CString &strRes)
//{
//	int nImgWidth = srcImg.GetWidth();
//	int nImgHeight = srcImg.GetHeight();
//
//	for (int i=0; i<nImgWidth; i++)
//	{
//		for (int j=0; j<nImgHeight; j++)
//		{
//			if (GETGRAYVALUE2(srcImg, i, j) <= nThreshold)
//			{
//				vector<CRect> v_dstRects;
//				GetChainContour_Encolsed(srcImg, nThreshold, dstSize, v_dstRects);
//
//				for(vector<CRect>::iterator it = v_dstRects.begin(); it != v_dstRects.end(); it++)
//				{
//					CRect rect1 = *it;
//					CPoint pTopLeft1, pBottomRight1;
//					pTopLeft1.x = rect1.TopLeft().x + 5/2;
//					pTopLeft1.y = rect1.TopLeft().y + 5/2;
//
//					pBottomRight1.x = rect1.BottomRight().x - 5/2;
//					pBottomRight1.y = rect1.BottomRight().y - 5/2;
//
//					CRect rectCrop;
//					rectCrop.SetRect(pTopLeft1, pBottomRight1);
//					CxImage cropImage;
//					srcImg.Crop(rectCrop, &cropImage);
//					strRes = GetOcrResult(&cropImage);
//
//					if (!strRes.IsEmpty() && strRes.Find('~') < 0)
//						return true;
//				}
//			}
//		}
//	}
//
//	return false;
//}

bool CImageProcess::GetMinRect(CxImage srcImg, CSize minSize, int nThreshold, vector<CRect> &v_allRects)
{
	IplImage *src = NULL;
	Cximage2Iplimage_1(&srcImg, &src);
	IplImage *dsw = cvCreateImage(cvGetSize(src), 8, 1);       
	CvMemStorage *storage = cvCreateMemStorage(0);    
	CvSeq *first_contour = NULL;    
	cvThreshold(src, dsw, nThreshold, 255, CV_THRESH_BINARY);    
	cvFindContours(dsw, storage, &first_contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);    
	for(; first_contour != 0; first_contour = first_contour->h_next)    
	{      
		CvRect rect = cvBoundingRect(first_contour,0);  
		CRect rect1;
		if (rect.height < minSize.cy)
			continue;

		if (rect.width < minSize.cx)
			continue;

		if (rect.width > 2*minSize.cx)
			continue;

		if (rect.height > 2*minSize.cy)
			continue;

		rect1.SetRect(rect.x, rect.y, rect.x+rect.width, rect.y + rect.height);
		v_allRects.push_back(rect1);
	}  

	for (int i=0; i<v_allRects.size(); i++)
	{
		for (int j=i+1; j<v_allRects.size(); j++)
		{
			CRect rect1 = v_allRects[i];
			CRect rect2 = v_allRects[j];
			
			if (IsPointInRect(rect1.CenterPoint(), rect2))
			{
				if(rect1.Width()*rect1.Height() < rect2.Width()*rect2.Height())
				{
					v_allRects.erase(v_allRects.begin()+i);
					i -= 1;
					break;
				}
				else
				{
					v_allRects.erase(v_allRects.begin()+j);
					j -= 1;
				}
			}
		}
	}

	//将框按从左到右排序 2016.07.19
	for (int i=0; i<v_allRects.size(); i++)
	{
		for (int j=i+1; j<v_allRects.size(); j++)
		{
			if (v_allRects[i].CenterPoint().x > v_allRects[j].CenterPoint().x)
			{
				CRect rectTmp = v_allRects[j];
				v_allRects[j] = v_allRects[i];
				v_allRects[i] = rectTmp;
			}
		}
	}
	
	cvReleaseImage(&src);
	src = NULL;
	cvReleaseImage(&dsw);
	dsw = NULL;
	cvClearMemStorage(storage);
	cvReleaseMemStorage(&storage);  
	return true;
}

bool CImageProcess::GetIcrRects(CxImage srcImg, CSize dstSize, int nThreshold, vector<CRect> &v_allRects)
{
	int nWidth  = srcImg.GetWidth();
	int nHeight = srcImg.GetHeight();
	vector<CRect> v_tempRects;
	GetChainContour_Encolsed(srcImg, nThreshold, dstSize, v_tempRects);
	if (v_tempRects.size() == 0)
		return FALSE;

	//按中心点进行排序
	for (vector<CRect>::iterator it1 = v_tempRects.begin(); it1 != v_tempRects.end(); it1++)
	{
		//将高度差在定义的高度范围内从左至右依次排序
		for (vector<CRect>::iterator it2 = it1 +1; it2 != v_tempRects.end(); it2++)
		{
			CRect rect1 = *it1;
			CRect rect2 = *it2;
			CPoint pCenter1 = rect1.CenterPoint();
			CPoint pCenter2 = rect2.CenterPoint();

			if (abs(pCenter1.y - pCenter2.y) >= dstSize.cy)
			{
				if (pCenter1.y > pCenter2.y)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
			else 
			{
				if (pCenter1.x > pCenter2.x)
				{
					*it1 = rect2;
					*it2 = rect1;
				}
			}
		}
	}

	v_allRects = v_tempRects;
	if (v_allRects.size() <= 0)
		return FALSE;

	return true;
}


bool CImageProcess::GetWhiteRect(CxImage srcImg, int nThreshold, CRect rect1, CRect &dstRect)
{
	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();

	bool bFindFirst = false;
	CPoint pTopLeft, pBottomRight;
	
	//判断白色有效点 2016.07.28
	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			int nGrayValue = GETGRAYVALUE2(srcImg, i, j);
			CPoint pTmp = CPoint(i, j);
			if (nGrayValue > nThreshold)
			{
				int nTime1 = GetTickCount();
				if (IsAroundBlack(srcImg, nThreshold, pTmp))
				{
					//TRACE("耗时:%d\n", GetTickCount() - nTime1);
					if (!bFindFirst)
					{
						bFindFirst = true;
						pTopLeft = pTmp;
						pBottomRight = pTmp;
					}
					else 
					{
						if ((pTopLeft.x + pTopLeft.y) > (pTmp.x + pTmp.y))
							pTopLeft = pTmp;

						if ((pBottomRight.x + pBottomRight.y) < (pTmp.x + pTmp.y))
							pBottomRight = pTmp;
					}
				}
				//else
					//TRACE("耗时:%d\n", GetTickCount() - nTime1);
			}

			
			j += 5;
		}

		i+=5;
	}

	dstRect.SetRect(pTopLeft, pBottomRight);
	return true;
}

bool CImageProcess::IsAroundBlack(CxImage srcImg, int nThreshold, CPoint p1)
{
	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();

	bool bFind[4] = {false, false, false, false};
	int nLen[4] = {1, 1, 1, 1};
	
	for (int i=0; i<4; i++)
	{
		CPoint pTmp = p1;
		if (!bFind[i])
		{
			while(1)
			{
				nLen[i]++;
				if (i == 0)
				{
					pTmp.x -= nLen[i];
					if (pTmp.x <= 0)
						return false;
				}
				else if (i == 1)
				{
					pTmp.x += nLen[i];
					if (pTmp.x > nImgWidth-1)
						return false;
				}
				else if (i == 2)
				{
					pTmp.y -= nLen[i];
					if (pTmp.y <= 0)
						return false;
				}
				else if (i == 3)
				{
					pTmp.y += nLen[i];
					if (pTmp.y > nImgHeight-1)
						return false;
				}

				if (GETGRAYVALUE2(srcImg, pTmp.x, pTmp.y) <= nThreshold)
				{
					bFind[i] = true;
					break;
				}
			}
		}
	}
	

	for (int i=0; i<4; i++)
	{
		if (!bFind[i])
			return false;
	}

	return true;
}

bool CImageProcess::GetValidExternalCharRect(CxImage srcImg, int nThreshold, CRect &dstRect)
{
	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();
    bool bFind = false;
    CPoint pTopLeft, pBottomRight;

	for (int i=0; i<nImgWidth; i++)
	{
		for (int j=0; j<nImgHeight; j++)
		{
			CPoint pStart = CPoint(i, j);
			int nGrayValue = GETGRAYVALUE2(srcImg, i, j);
			if (nGrayValue <= nThreshold) //黑点
			{
				if (!bFind)
				{
					bFind = true;
					pTopLeft = pStart;
					pBottomRight = pStart;
				}
				else
				{
					if (pTopLeft.x + pTopLeft.y > pStart.x + pStart.y)
						pTopLeft = pStart;

					if (pBottomRight.x + pBottomRight.y < pStart.x + pStart.y)
						pBottomRight = pStart;
				}
				
			}
		}
	}
	
	dstRect.SetRect(pTopLeft.x+8, pTopLeft.y+8, pBottomRight.x-8, pBottomRight.y-8);
	return true;
}

bool CImageProcess::GetExpandImg(CxImage srcImg, int nNewWidth, int nNewHeight, CxImage &dstImg)
{
	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();

	dstImg.Create(nNewWidth, nNewHeight, 8, CXIMAGE_FORMAT_BMP);
	dstImg.SetGrayPalette();
	dstImg.GrayScale();
	memset(dstImg.info.pImage, 255, BYTESPERLINE(nNewWidth, 8)*nNewHeight);

	CPoint pStart1 = CPoint(nNewWidth/2 - nImgWidth/2, nNewHeight/2 - nImgHeight/2);
	CPoint pEnd1 = CPoint(nNewWidth/2 + nImgWidth/2, nNewHeight/2 + nImgHeight/2);

	for (int m=pStart1.y; m<pEnd1.y; m++)
	{
		memcpy(dstImg.info.pImage + (nNewHeight - 1 - m)*BYTESPERLINE(nNewWidth, 8) + pStart1.x,
			srcImg.info.pImage + (nImgHeight-1-(m-pStart1.y))*BYTESPERLINE(nImgWidth, 8),
			nImgWidth);
	}


	return true;
}

>>>>>>> c3ec4193a47e985f94758967b6bacfb8a4ab020b
