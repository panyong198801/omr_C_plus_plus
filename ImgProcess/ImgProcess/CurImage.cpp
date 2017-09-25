<<<<<<< HEAD
#include "StdAfx.h"
#include "CurImage.h"

CurImage::CurImage(void)
{
	m_nThreshold = 200; //阈值 2016.02.17
	m_nAverGrayValue = 200;
	m_dAverDensity = 0.0;
}

CurImage::~CurImage(void)
{
}

void CurImage::GetCurImgRectArray(const CxImage srcImg, const vector<RECTARRAY> v_ModelRA, int nThreshold, vector<RECTARRAY> &v_dstRA)
{
	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();

	//搜索最符合条件的矩形位置
	vector<RECTARRAY> v_RATmp = v_ModelRA;
	int nMaxLen = 8; //搜索上下范围

	CSize modelSize;
	int nSumX = 0;
	int nSumY = 0;
	int nSize = v_RATmp.size();
	for (vector<RECTARRAY>::iterator it = v_RATmp.begin(); it != v_RATmp.end(); it++)
	{
		RECTARRAY RATmp = *it;
		nSumX += RATmp.rect.GetWidth();
		nSumY += RATmp.rect.GetHeight();
	}
	
	if (nSize > 0)
	{
		modelSize.cx = nSumX / nSize;
		modelSize.cy = nSumY / nSize;
	}

	for (int i = 0; i < nMaxLen; i++)
	{
		for (int j = 0; j < nMaxLen; j++)
		{
			
			for (int k = 0; k < 2; k++)
			{
				int nMoveX = 0;
				int nMoveY = 0;

				if (k == 0)
					nMoveX += i;
				else
					nMoveX -= i;

				for (int m = 0; m < 2; m++)
				{
					if (m == 0)
						nMoveY += i;
					else
						nMoveY -= i;

					int nCount = 0;
					vector<RECTARRAY> v_dstRATmp;
					for (vector<RECTARRAY>::iterator it = v_RATmp.begin(); it != v_RATmp.end(); it++)
					{
						RECTARRAY RATmp = *it;
						NEWPOINT p1;
						p1.setPoint(nMoveX, nMoveY);
						NEWRECT rect1 = RATmp.rect;
						rect1.AddPoint(p1);
						if (IsValidRect(srcImg, nThreshold, modelSize, rect1))
						{
							nCount++;
						}
						RATmp.rect.AddPoint(p1);
						v_dstRATmp.push_back(RATmp);
					}

					if (nCount == v_RATmp.size()) //正好找到完全匹配 2016.12.16
					{
						v_dstRA = v_dstRATmp;
						return;
					}
				}
			}
			
		}
	}
}

bool CurImage::IsValidRect(CxImage srcImg, int nThreshold, CSize dstSize, NEWRECT rect1)
{
	CxImage cropImg;
	CRect rectTmp;
	rectTmp.SetRect(CPoint(rect1.pTopLeft.nX, rect1.pTopLeft.nY), CPoint(rect1.pBottomRight.nX, rect1.pBottomRight.nY));
	srcImg.Crop(rectTmp, &cropImg);
	
	int nImagWidth = cropImg.GetWidth();
	int nImgHeight = cropImg.GetHeight();

	CPoint pStart = CPoint(88888, 88888);
	CPoint pEnd = CPoint(0, 0);
	for (int i = 0; i < nImgHeight; i++)
	{
		for (int j = 0; j < nImagWidth; j++)
		{
			if (GETGRAYVALUE2(cropImg, j, i) <= nThreshold) //黑点
			{
				if (pStart.x >= j && pStart.y >= i)
					pStart = CPoint(j, i);

				if (pEnd.x <= j && pEnd.y <= i)
					pEnd = CPoint(j, i);
			}
		}
	}

	int nCurWidth = pEnd.x - pStart.x;
	int nCurHeight = pEnd.y - pStart.y;
	if (nCurWidth > dstSize.cx/2 && nCurHeight > dstSize.cy/2)
	{
		if (abs(nCurWidth - dstSize.cx) <= nEPS && abs(nCurHeight - dstSize.cy) <= nEPS)
			return TRUE;
	}

	return FALSE;
=======
#include "StdAfx.h"
#include "CurImage.h"

CurImage::CurImage(void)
{
	m_nThreshold = 200; //阈值 2016.02.17
	m_nAverGrayValue = 200;
	m_dAverDensity = 0.0;
}

CurImage::~CurImage(void)
{
}

void CurImage::GetCurImgRectArray(const CxImage srcImg, const vector<RECTARRAY> v_ModelRA, int nThreshold, vector<RECTARRAY> &v_dstRA)
{
	int nImgWidth = srcImg.GetWidth();
	int nImgHeight = srcImg.GetHeight();

	//搜索最符合条件的矩形位置
	vector<RECTARRAY> v_RATmp = v_ModelRA;
	int nMaxLen = 8; //搜索上下范围

	CSize modelSize;
	int nSumX = 0;
	int nSumY = 0;
	int nSize = v_RATmp.size();
	for (vector<RECTARRAY>::iterator it = v_RATmp.begin(); it != v_RATmp.end(); it++)
	{
		RECTARRAY RATmp = *it;
		nSumX += RATmp.rect.GetWidth();
		nSumY += RATmp.rect.GetHeight();
	}
	
	if (nSize > 0)
	{
		modelSize.cx = nSumX / nSize;
		modelSize.cy = nSumY / nSize;
	}

	for (int i = 0; i < nMaxLen; i++)
	{
		for (int j = 0; j < nMaxLen; j++)
		{
			
			for (int k = 0; k < 2; k++)
			{
				int nMoveX = 0;
				int nMoveY = 0;

				if (k == 0)
					nMoveX += i;
				else
					nMoveX -= i;

				for (int m = 0; m < 2; m++)
				{
					if (m == 0)
						nMoveY += i;
					else
						nMoveY -= i;

					int nCount = 0;
					vector<RECTARRAY> v_dstRATmp;
					for (vector<RECTARRAY>::iterator it = v_RATmp.begin(); it != v_RATmp.end(); it++)
					{
						RECTARRAY RATmp = *it;
						NEWPOINT p1;
						p1.setPoint(nMoveX, nMoveY);
						NEWRECT rect1 = RATmp.rect;
						rect1.AddPoint(p1);
						if (IsValidRect(srcImg, nThreshold, modelSize, rect1))
						{
							nCount++;
						}
						RATmp.rect.AddPoint(p1);
						v_dstRATmp.push_back(RATmp);
					}

					if (nCount == v_RATmp.size()) //正好找到完全匹配 2016.12.16
					{
						v_dstRA = v_dstRATmp;
						return;
					}
				}
			}
			
		}
	}
}

bool CurImage::IsValidRect(CxImage srcImg, int nThreshold, CSize dstSize, NEWRECT rect1)
{
	CxImage cropImg;
	CRect rectTmp;
	rectTmp.SetRect(CPoint(rect1.pTopLeft.nX, rect1.pTopLeft.nY), CPoint(rect1.pBottomRight.nX, rect1.pBottomRight.nY));
	srcImg.Crop(rectTmp, &cropImg);
	
	int nImagWidth = cropImg.GetWidth();
	int nImgHeight = cropImg.GetHeight();

	CPoint pStart = CPoint(88888, 88888);
	CPoint pEnd = CPoint(0, 0);
	for (int i = 0; i < nImgHeight; i++)
	{
		for (int j = 0; j < nImagWidth; j++)
		{
			if (GETGRAYVALUE2(cropImg, j, i) <= nThreshold) //黑点
			{
				if (pStart.x >= j && pStart.y >= i)
					pStart = CPoint(j, i);

				if (pEnd.x <= j && pEnd.y <= i)
					pEnd = CPoint(j, i);
			}
		}
	}

	int nCurWidth = pEnd.x - pStart.x;
	int nCurHeight = pEnd.y - pStart.y;
	if (nCurWidth > dstSize.cx/2 && nCurHeight > dstSize.cy/2)
	{
		if (abs(nCurWidth - dstSize.cx) <= nEPS && abs(nCurHeight - dstSize.cy) <= nEPS)
			return TRUE;
	}

	return FALSE;
>>>>>>> c3ec4193a47e985f94758967b6bacfb8a4ab020b
}