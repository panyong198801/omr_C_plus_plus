<<<<<<< HEAD
#include "StdAfx.h"
#include "ModelProcess.h"

struct LOCALPOINT
{
	CPoint point;
	int  nLink;

	LOCALPOINT()
	{
		point = CPoint(-1, -1);
		nLink = -1;
	} 
};

CModelProcess::CModelProcess(void)
{
}

CModelProcess::~CModelProcess(void)
{
}

BOOL CModelProcess::PickBoundary(BYTE *pBuffer, int nWidth, int nHeight,int nThreshold, vector<CRect> v_rects)
{
	int nNewWidth = BYTESPERLINE(nWidth, 8);

	//递归搜索	
	LOCALPOINT pLocalTmp;
	for (int i=0; i<nHeight-1; i++)
	{
		for (int j=0; j<nNewWidth-1; j++)
		{
			int nGrayValue = pBuffer[nNewWidth*(nHeight-1-i) + j];
			
			if (nGrayValue <= nThreshold)
			{
				pLocalTmp.point = CPoint(j, i);
				pLocalTmp.nLink++;
			}
			else 
			{

				if (pLocalTmp.nLink > 0)
				{

				}

				//pLocalTmp.point = CPoint(-1, -1);
				//pLocalTmp.nLink = -1;
			}
		}
	}
		

	return FALSE;
}

BOOL CModelProcess::GetRectExpand(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CPoint pStart, vector<CPoint> v_allPoints)
{
	int nNewWidth = BYTESPERLINE(nWidth, 8);

	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nWidth-1 || pStart.y > nHeight-1)
		return FALSE;

	
	bool bFind = TRUE;
	CPoint pStartTmp = pStart;
	v_allPoints.push_back(pStartTmp);
	while(bFind)
	{
		//逆时针查找
		CPoint pStartNew[8];
		for (int i=0; i<8; i++)
			pStartNew[i] = pStartTmp;
		pStartNew[0].x  -= 1;

		pStartNew[1].x -= 1;
		pStartNew[1].y += 1;

		pStartNew[2].y += 1;

		pStartNew[3].x += 1;
		pStartNew[3].y += 1;

		pStartNew[4].x += 1;

		pStartNew[5].x += 1;
		pStartNew[5].y -= 1;

		pStartNew[6].y -= 1;

		pStartNew[7].x -= 1;
		pStartNew[7].y -= 1;
		
		bool bFindNext = FALSE;
		for (int i=0; i<8; i++)
		{
			if (pStartNew[i].x < 0 || pStartNew[i].y < 0)
				continue;

			if (pStartNew[i].x > nWidth-1 || pStartNew[i].y < 0)
				continue;

			int nGrayValue = pBuffer[nNewWidth*(nHeight-1-pStartNew[i].y) + pStartNew[i].x];
			
			if (nGrayValue <= nThreshold)
			{
				if (pStartNew[i] == v_allPoints[v_allPoints.size()-1])
					continue;

				if (pStartNew[i] == v_allPoints[0])
					return TRUE;

				bFindNext = TRUE;
				pStartTmp = pStartNew[i];
				break;
			}
		}

		if (!bFindNext)
		{
			bFind = FALSE;
		}

	}

	return FALSE;
}

BOOL CModelProcess::GetRect(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CPoint pStart, CSize dstSize,CRect &dstRect)
{
	
	CPoint pStartTmp = pStart;
	CPoint pExtream1[2];
	if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStart, 0, dstSize, pExtream1))
	{
		CPoint pStartTmp1 = pExtream1[0];
		CPoint pStartTmp2 = pExtream1[1];

		CPoint pVLine1[2] = {CPoint(-1, -1), CPoint(-1, -1)};
		CPoint pVLine2[2] = {CPoint(-1, -1), CPoint(-1, -1)};
		bool bFindV[2] = {FALSE, FALSE};
		for (int i=0; i<2; i++)
		{
			pStartTmp1.x = pExtream1[0].x - i;
			pStartTmp2.x = pExtream1[0].x + i;

			if (!bFindV[0])
			{
				if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp1, 1, dstSize, pVLine1))
				{
					bFindV[0] = TRUE;
				}
				else 
				{
					pStartTmp1.x = pExtream1[0].x + i;
					if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp1, 1, dstSize, pVLine1))
					{
						bFindV[0] = TRUE;
					}
				}
			}

			if (!bFindV[1])
			{
				if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp2, 1, dstSize, pVLine2))
				{
					bFindV[1] = TRUE;
				}
				else
				{
					pStartTmp2.x = pExtream1[1].x - i;
					if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp2, 1, dstSize, pVLine2))
					{
						bFindV[1] = TRUE;
					}
				}
			}

			if (bFindV[0] && bFindV[1])
				break;
		}

		bool bFindH = FALSE;
		CPoint pHLine[2] = {CPoint(-1 -1), CPoint(-1, -1)};
		if (bFindV[0] && bFindV[1])
		{
			pStartTmp1 = pVLine1[1];
			for (int i=0; i<2; i++)
			{
				pStartTmp1.y = pVLine1[1].y - i;
				if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp1,0, dstSize, pHLine))
				{
					bFindH = TRUE;
					CPoint pTopLeft;
					CPoint pBottomRight;
					pTopLeft.x = min(pVLine1[0].x, pExtream1[0].x);
					pTopLeft.y = min(pVLine1[0].y, pExtream1[0].y);
					pBottomRight.x = max(pVLine2[1].x, pHLine[1].x);
					pBottomRight.y = max(pVLine2[1].y, pHLine[1].y); 
					dstRect.SetRect(pTopLeft, pBottomRight);
					dstRect.SetRect(pTopLeft, pBottomRight);

					if (dstRect.Width() * dstRect.Height() <= 0)
						return FALSE;

					//TRACE("矩形：topleft x0:%d, y0;%d, x1:%d, y1:%d\n", pTopLeft.x, pTopLeft.y, pBottomRight.x, pBottomRight.y);
					return TRUE;
				}
				else 
				{
					pStartTmp1.y = pVLine1[1].y + i;

					if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp1,0, dstSize, pHLine))
					{
						bFindH = TRUE;
						CPoint pTopLeft;
						CPoint pBottomRight;
						pTopLeft.x = min(pVLine1[0].x, pExtream1[0].x);
						pTopLeft.y = min(pVLine1[0].y, pExtream1[0].y);
						pBottomRight.x = max(pVLine2[1].x, pHLine[1].x);
						pBottomRight.y = max(pVLine2[1].y, pHLine[1].y); 
						dstRect.SetRect(pTopLeft, pBottomRight);
						
						if (dstRect.Width() * dstRect.Height() <= 0)
							return FALSE;

						return TRUE;
					}
				}
				
			}
		}
	}
	
	return FALSE;
}

BOOL CModelProcess::GetLine(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CPoint pStart, int nIndex, CSize dstSize,CPoint pExtream[2])
{
	CPoint pStartTmp = pStart;
	int  nNewWidth = BYTESPERLINE(nWidth, 8);
	for (int i=0; i<2; i++)
	{
		pExtream[i] = pStart;
	}

	if (nIndex == 0) //横向搜索
	{
		int nCount = 0;
		while(1)
		{
			pStartTmp.x --;
			if (pStartTmp.x > 0 )
			{
				int nGrayValue = pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + pStartTmp.x];
				if (nGrayValue <= nThreshold)
				{
					pExtream[0] = pStartTmp;
					nCount = 0;
				}
				else 
				{
					bool bFind = false;
					for (int i=0; i<2; i++)
					{
						int nY1 = pStartTmp.y - i;
						int nY2  = pStartTmp.y + i;

						if (nY1 >= 0)
						{
							if (pBuffer[(nHeight-1-nY1)*nNewWidth + pStartTmp.x] <= nThreshold)
							{
								pExtream[0] = pStartTmp;
								bFind = TRUE;
								nCount = 0;
								break;
							}
						}

						if (nY2 < nHeight-1)
						{
							if (pBuffer[(nHeight-1-nY2)*nNewWidth + pStartTmp.x] <= nThreshold)
							{
								pExtream[0] = pStartTmp;
								nCount = 0;
								bFind = TRUE;
								break;
							}
						}
					}
					

					if (!bFind)
					{
						nCount++;

						if (nCount > 3)
							break;
					}
				}
			}
			else 
				break;
		}

		pStartTmp = pStart;

		nCount = 0;
		while(1)
		{
			pStartTmp.x++;
			if (pStartTmp.x < nWidth-1)
			{
				int nGrayValue = pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + pStartTmp.x];
				if (nGrayValue <= nThreshold)
				{
					pExtream[1] = pStartTmp;
					nCount = 0;
				}
				else 
				{
					bool bFind = false;
					for (int i=0; i<2; i++)
					{
						int nY1 = pStartTmp.y - i;
						int nY2  = pStartTmp.y + i;

						if (nY1 >= 0)
						{
							if (pBuffer[(nHeight-1-nY1)*nNewWidth + pStartTmp.x] <= nThreshold)
							{
								pExtream[1] = pStartTmp;
								nCount = 0;
								bFind = TRUE;
								break;
							}
						}

						if (nY2 < nHeight-1)
						{
							if (pBuffer[(nHeight-1-nY2)*nNewWidth + pStartTmp.x] <= nThreshold)
							{
								pExtream[1] = pStartTmp;
								nCount = 0;
								bFind = TRUE;
								break;
							}
						}
					}
						

					if (!bFind)
					{
						nCount++;

						if (nCount > 3)
							break;
					}
				}
			}
			else 
				break;
		}

		int nLen = pExtream[1].x - pExtream[0].x;

		if (nLen <= 0)
			return FALSE;

		if (abs(nLen - dstSize.cx) <= 5)
			return TRUE;
	}
	else if (nIndex == 1) //竖向搜索
	{
		int nCount = 0;
		while(1)
		{
			pStartTmp.y --;
			if (pStartTmp.y >= 0 )
			{
				int nGrayValue = pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + pStartTmp.x];
				if (nGrayValue <= nThreshold)
				{
					nCount =  0;
					pExtream[0] = pStartTmp;
				}
				else 
				{
					bool bFind = false;
					for (int i=0; i<2; i++)
					{
						int nX1 = pStartTmp.x - i;
						int nX2  = pStartTmp.x + i;

						if (nX1 >= 0)
						{
							if (pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + nX1] <= nThreshold)
							{
								pExtream[0] = pStartTmp;
								bFind = TRUE;
								nCount =  0;
								break;
							}
						}

						if (nX2 < nWidth-1)
						{
							if (pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + nX1] <= nThreshold)
							{
								pExtream[0] = pStartTmp;
								bFind = TRUE;
								nCount =  0;
								break;
							}
						}
					}
					if (!bFind)
					{
						nCount++;
						if (nCount > 3)
							break;
					}
				}
			}
			else 
				break;
		}

		pStartTmp = pStart;

		nCount = 0;
		while(1)
		{
			pStartTmp.y++;
			if (pStartTmp.y < nHeight-1)
			{
				int nGrayValue = pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + pStartTmp.x];
				if (nGrayValue <= nThreshold)
				{
					pExtream[1] = pStartTmp;
					nCount = 0;
				}
				else 
				{
					bool bFind = false;
					for (int i=0; i<2; i++)
					{
						int nX1 = pStartTmp.x - i;
						int nX2  = pStartTmp.x + i;

						if (nX1 >= 0)
						{
							if (pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + nX1] <= nThreshold)
							{
								pExtream[1] = pStartTmp;
								bFind = TRUE;
								nCount = 0;
								break;
							}
						}

						if (nX2 < nWidth-1)
						{
							if (pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + nX1] <= nThreshold)
							{
								pExtream[1] = pStartTmp;
								bFind = TRUE;
								nCount = 0;
								break;
							}
						}
					}

					if (!bFind)
					{
						nCount++;
						if (nCount > 3)
							break;
					}
				}
			}
			else 
				break;
		}

		int nLen = pExtream[1].y - pExtream[0].y;

		if (nLen <= 0)
			return FALSE;

		if (abs(nLen - dstSize.cy) <= 5)
			return TRUE;
	}
	
	return FALSE;
}

int CModelProcess::GetAllRects(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CSize dstSize, vector<LOCALRECT> &v_AllRects)
{
	int nNewWidth = BYTESPERLINE(nWidth, 8);
	for (int i=0; i<nHeight; i++)
	{
		for (int j=0; j<nWidth; j++)
		{
			int nGrayValue = pBuffer[(nHeight-1-i)*nNewWidth + j];
			if (nGrayValue <= nThreshold)
			{
				//TRACE("x:%d, y:%d, %d\n", j, i, nCount);
				CPoint pStart = CPoint(j, i);
				bool bFind = false;
				CRect dstRect;

				if (GetRect(pBuffer, nWidth, nHeight, nThreshold, pStart, dstSize, dstRect))
				{
					bFind = false;
					j = dstRect.BottomRight().x + 10; 
					

					LOCALRECT localRect;
					localRect.rect = dstRect;
					localRect.dDensity = GetDensity(pBuffer, nWidth, nHeight, nThreshold, dstRect);
					/*TRACE("矩形：topleft x0:%d, y0;%d, x1:%d, y1:%d, density:%f\n", 
						dstRect.TopLeft().x, dstRect.TopLeft().y, dstRect.BottomRight().x, dstRect.BottomRight().y, localRect.dDensity);*/
					v_AllRects.push_back(localRect);
					SetGrayValue(pBuffer, nWidth, nHeight, dstRect, 255);
				}
			}
		}
	}

	return v_AllRects.size();
}

BOOL  CModelProcess::IsPointInRect(CPoint p1, CRect rect)
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

BOOL CModelProcess::GetRectsByColumn(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CSize dstSize, vector<CRect> &v_AllRects)
{
	
	return FALSE;
}

void CModelProcess::SetGrayValue(BYTE *pBuffer, int nWidth, int nHeight, CRect rectArea, int nGrayValue)
{
	int nNewWidth = BYTESPERLINE(nWidth, 8);
	for (int i=rectArea.TopLeft().x; i<rectArea.BottomRight().x; i++)
	{
		for (int j=rectArea.TopLeft().y; j<rectArea.BottomRight().y; j++)
		{
			CPoint pStart = CPoint(i, j);
			pBuffer[(nHeight-1-pStart.y)*nNewWidth + pStart.x] = nGrayValue;
		}
	}
}

double CModelProcess::GetDensity(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CRect dstRect)
{
	double dDensity = 0.0;
	int nNewWidth = BYTESPERLINE(nWidth, 8);
	int nCount = 0;
	for (int i=dstRect.TopLeft().x; i<dstRect.BottomRight().x; i++)
	{
		for (int j=dstRect.TopLeft().y; j<dstRect.BottomRight().y; j++)
		{
			if (pBuffer[(nHeight-1-j)*nNewWidth + i] <= nThreshold)
			{
				nCount++;
			}
		}
	}

	dDensity = double(nCount) / double(dstRect.Width()*dstRect.Height());
	return dDensity;
}


BOOL CModelProcess::GetValidRects(vector<LOCALRECT> v_srcRects, vector<LOCALRECT> v_modelRects, int nEps)
{
	if (v_srcRects.size() < v_modelRects.size())
		return FALSE;

	int nCount = 0;
	for (vector<LOCALRECT>::iterator itModel = v_modelRects.begin(); itModel != v_modelRects.end(); itModel++)
	{
		LOCALRECT rectModel = *itModel;
		bool bFind = false;
		
		for (vector<LOCALRECT>::iterator itSrc = v_srcRects.begin(); itSrc != v_srcRects.end(); itSrc++)
		{
			LOCALRECT rectSrc = *itSrc;
			if (rectSrc.nRow < 0 || rectSrc.nColumn < 0)
			{
				if (GetLength(rectModel.rect.CenterPoint(), rectSrc.rect.CenterPoint()) <= nEps)
				{
					bFind = TRUE;
					nCount++;
					rectSrc.nRow = rectModel.nRow;
					rectSrc.nColumn = rectModel.nColumn;
					break;
				}
			}
		}

		if (!bFind)
		{
			return FALSE;
		}
	}

	if (nCount != v_modelRects.size())
		return FALSE;

	return TRUE;
}

double CModelProcess::GetLength(CPoint p1, CPoint p2)
{
	double dLength = 0.0;
	double dLenX = double(p2.x-p1.x);
	double dLenY = double(p2.y-p1.y);
	double dX = pow(dLenX, 2.0);
	double dY = pow(dLenY, 2.0);
	dLength = sqrt(dX + dY);
	return dLength;
}

BOOL CModelProcess::GetModelRectsInfo(vector<LOCALRECT> v_modelRects, int nEps)
{
	for (vector<LOCALRECT>::iterator it1 = v_modelRects.begin(); it1 != v_modelRects.end(); it1++)
	{
		for (vector<LOCALRECT>::iterator it2 = it1+1; it2 != v_modelRects.end(); it2++)
		{
			CRect rect1 = *it1->rect;
			CRect rect2 = *it2->rect;
			if (abs(rect1.CenterPoint().x - rect2.CenterPoint().x) > nEps)
			{
				if (rect1.CenterPoint().x > rect2.CenterPoint().x)
				{
					 //交换
					LOCALRECT localRect = *it1;
					*it1 = *it2;
					*it2 = localRect;
				}
			}
		}
	}

	int nIndex = 0;
	for (vector<LOCALRECT>::iterator it=v_modelRects.begin(); it != v_modelRects.end(); it++)
	{
		for (vector<LOCALRECT>::iterator it2 = it+1; it2 != v_modelRects.end(); it2++)
		{
			if (abs(it2->rect.CenterPoint().x - it->rect.CenterPoint().x) <= nEps)
			{
				it->nColumn = nIndex;
				it2->nColumn = nIndex;
			}
			else
			{
				nIndex++;
				it = it2;
				it->nColumn = nIndex;
			}
		}
	}

	for (vector<LOCALRECT>::iterator it1 = v_modelRects.begin(); it1 != v_modelRects.end(); it1++)
	{
		for (vector<LOCALRECT>::iterator it2 = it1+1; it2 != v_modelRects.end(); it2++)
		{
			CRect rect1 = *it1->rect;
			CRect rect2 = *it2->rect;

			if (abs(rect1.CenterPoint().y - rect2.CenterPoint().y) > nEps)
			{
				if (rect1.CenterPoint().y > rect2.CenterPoint().y)
				{
					//交换
					LOCALRECT localRect = *it1;
					*it1 = *it2;
					*it2 = localRect;
				}
			}
		}
	}

	nIndex = 0;
	for (vector<LOCALRECT>::iterator it=v_modelRects.begin(); it != v_modelRects.end(); it++)
	{
		for (vector<LOCALRECT>::iterator it2 = it+1; it2 != v_modelRects.end(); it2++)
		{
			if (abs(it2->rect.CenterPoint().y - it->rect.CenterPoint().y) <= nEps)
			{
				it->nRow = nIndex;
				it2->nRow = nIndex;
			}
			else
			{
				nIndex++;
				it = it2;
				it->nRow = nIndex;
			}
		}
	}
	
	for (vector<LOCALRECT>::iterator it = v_modelRects.begin(); it != v_modelRects.end(); it++)
	{
		LOCALRECT localRect = *it;
		TRACE("x:%d, y:%d, nRow:%d, nColumn:%d, Density:%f\n", localRect.rect.CenterPoint().x, localRect.rect.CenterPoint().y,
			localRect.nRow, localRect.nColumn, localRect.dDensity);
	}

	return TRUE;
}

bool CModelProcess::GetBrackets(BYTE *pBuffer, int nWidth, int nHeight, CPoint pStart, int nThreshold, BRACKETRECT &dstRect)
{
	int nNewWidth = BYTESPERLINE(nWidth, 8);

	//if (pBuffer[(nHeight-1-pStart.y)*nNewWidth + pStart.x]) 
	if (GRAYVALUE(pBuffer, nNewWidth, nHeight, pStart.x, pStart.y) > nThreshold)
		return FALSE;

	//获取横向
	CPoint pExtreamHTop[2];
	CPoint pExtreamHBottom[2];

	//纵向
	CPoint pExtreamVLeft[2];
	CPoint pExtreamVRight[2];
	CSize dstSize;
	if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStart, 0, dstSize, pExtreamHTop))
	{
		//获取竖向点
		
		
	}
	else //获取竖向 
	{
		CPoint pExtream2[2];
		GetLine(pBuffer, nWidth, nHeight, nThreshold, pStart, 1, dstSize, pExtream2);

		//获取横向点
		//GetLine(pBuffer, nWidth, nHeight, nThreshold, pExtream2[0], 0, dstSize, pExtream1);
		//GetLine(pBuffer, nWidth, nHeight, nThreshold, pExtream2[1], 0, dstSize, pExtream1);
	}
	



	return FALSE;
}
=======
#include "StdAfx.h"
#include "ModelProcess.h"

struct LOCALPOINT
{
	CPoint point;
	int  nLink;

	LOCALPOINT()
	{
		point = CPoint(-1, -1);
		nLink = -1;
	} 
};

CModelProcess::CModelProcess(void)
{
}

CModelProcess::~CModelProcess(void)
{
}

BOOL CModelProcess::PickBoundary(BYTE *pBuffer, int nWidth, int nHeight,int nThreshold, vector<CRect> v_rects)
{
	int nNewWidth = BYTESPERLINE(nWidth, 8);

	//递归搜索	
	LOCALPOINT pLocalTmp;
	for (int i=0; i<nHeight-1; i++)
	{
		for (int j=0; j<nNewWidth-1; j++)
		{
			int nGrayValue = pBuffer[nNewWidth*(nHeight-1-i) + j];
			
			if (nGrayValue <= nThreshold)
			{
				pLocalTmp.point = CPoint(j, i);
				pLocalTmp.nLink++;
			}
			else 
			{

				if (pLocalTmp.nLink > 0)
				{

				}

				//pLocalTmp.point = CPoint(-1, -1);
				//pLocalTmp.nLink = -1;
			}
		}
	}
		

	return FALSE;
}

BOOL CModelProcess::GetRectExpand(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CPoint pStart, vector<CPoint> v_allPoints)
{
	int nNewWidth = BYTESPERLINE(nWidth, 8);

	if (pStart.x < 0 || pStart.y < 0)
		return FALSE;

	if (pStart.x > nWidth-1 || pStart.y > nHeight-1)
		return FALSE;

	
	bool bFind = TRUE;
	CPoint pStartTmp = pStart;
	v_allPoints.push_back(pStartTmp);
	while(bFind)
	{
		//逆时针查找
		CPoint pStartNew[8];
		for (int i=0; i<8; i++)
			pStartNew[i] = pStartTmp;
		pStartNew[0].x  -= 1;

		pStartNew[1].x -= 1;
		pStartNew[1].y += 1;

		pStartNew[2].y += 1;

		pStartNew[3].x += 1;
		pStartNew[3].y += 1;

		pStartNew[4].x += 1;

		pStartNew[5].x += 1;
		pStartNew[5].y -= 1;

		pStartNew[6].y -= 1;

		pStartNew[7].x -= 1;
		pStartNew[7].y -= 1;
		
		bool bFindNext = FALSE;
		for (int i=0; i<8; i++)
		{
			if (pStartNew[i].x < 0 || pStartNew[i].y < 0)
				continue;

			if (pStartNew[i].x > nWidth-1 || pStartNew[i].y < 0)
				continue;

			int nGrayValue = pBuffer[nNewWidth*(nHeight-1-pStartNew[i].y) + pStartNew[i].x];
			
			if (nGrayValue <= nThreshold)
			{
				if (pStartNew[i] == v_allPoints[v_allPoints.size()-1])
					continue;

				if (pStartNew[i] == v_allPoints[0])
					return TRUE;

				bFindNext = TRUE;
				pStartTmp = pStartNew[i];
				break;
			}
		}

		if (!bFindNext)
		{
			bFind = FALSE;
		}

	}

	return FALSE;
}

BOOL CModelProcess::GetRect(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CPoint pStart, CSize dstSize,CRect &dstRect)
{
	
	CPoint pStartTmp = pStart;
	CPoint pExtream1[2];
	if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStart, 0, dstSize, pExtream1))
	{
		CPoint pStartTmp1 = pExtream1[0];
		CPoint pStartTmp2 = pExtream1[1];

		CPoint pVLine1[2] = {CPoint(-1, -1), CPoint(-1, -1)};
		CPoint pVLine2[2] = {CPoint(-1, -1), CPoint(-1, -1)};
		bool bFindV[2] = {FALSE, FALSE};
		for (int i=0; i<2; i++)
		{
			pStartTmp1.x = pExtream1[0].x - i;
			pStartTmp2.x = pExtream1[0].x + i;

			if (!bFindV[0])
			{
				if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp1, 1, dstSize, pVLine1))
				{
					bFindV[0] = TRUE;
				}
				else 
				{
					pStartTmp1.x = pExtream1[0].x + i;
					if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp1, 1, dstSize, pVLine1))
					{
						bFindV[0] = TRUE;
					}
				}
			}

			if (!bFindV[1])
			{
				if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp2, 1, dstSize, pVLine2))
				{
					bFindV[1] = TRUE;
				}
				else
				{
					pStartTmp2.x = pExtream1[1].x - i;
					if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp2, 1, dstSize, pVLine2))
					{
						bFindV[1] = TRUE;
					}
				}
			}

			if (bFindV[0] && bFindV[1])
				break;
		}

		bool bFindH = FALSE;
		CPoint pHLine[2] = {CPoint(-1 -1), CPoint(-1, -1)};
		if (bFindV[0] && bFindV[1])
		{
			pStartTmp1 = pVLine1[1];
			for (int i=0; i<2; i++)
			{
				pStartTmp1.y = pVLine1[1].y - i;
				if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp1,0, dstSize, pHLine))
				{
					bFindH = TRUE;
					CPoint pTopLeft;
					CPoint pBottomRight;
					pTopLeft.x = min(pVLine1[0].x, pExtream1[0].x);
					pTopLeft.y = min(pVLine1[0].y, pExtream1[0].y);
					pBottomRight.x = max(pVLine2[1].x, pHLine[1].x);
					pBottomRight.y = max(pVLine2[1].y, pHLine[1].y); 
					dstRect.SetRect(pTopLeft, pBottomRight);
					dstRect.SetRect(pTopLeft, pBottomRight);

					if (dstRect.Width() * dstRect.Height() <= 0)
						return FALSE;

					//TRACE("矩形：topleft x0:%d, y0;%d, x1:%d, y1:%d\n", pTopLeft.x, pTopLeft.y, pBottomRight.x, pBottomRight.y);
					return TRUE;
				}
				else 
				{
					pStartTmp1.y = pVLine1[1].y + i;

					if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStartTmp1,0, dstSize, pHLine))
					{
						bFindH = TRUE;
						CPoint pTopLeft;
						CPoint pBottomRight;
						pTopLeft.x = min(pVLine1[0].x, pExtream1[0].x);
						pTopLeft.y = min(pVLine1[0].y, pExtream1[0].y);
						pBottomRight.x = max(pVLine2[1].x, pHLine[1].x);
						pBottomRight.y = max(pVLine2[1].y, pHLine[1].y); 
						dstRect.SetRect(pTopLeft, pBottomRight);
						
						if (dstRect.Width() * dstRect.Height() <= 0)
							return FALSE;

						return TRUE;
					}
				}
				
			}
		}
	}
	
	return FALSE;
}

BOOL CModelProcess::GetLine(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CPoint pStart, int nIndex, CSize dstSize,CPoint pExtream[2])
{
	CPoint pStartTmp = pStart;
	int  nNewWidth = BYTESPERLINE(nWidth, 8);
	for (int i=0; i<2; i++)
	{
		pExtream[i] = pStart;
	}

	if (nIndex == 0) //横向搜索
	{
		int nCount = 0;
		while(1)
		{
			pStartTmp.x --;
			if (pStartTmp.x > 0 )
			{
				int nGrayValue = pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + pStartTmp.x];
				if (nGrayValue <= nThreshold)
				{
					pExtream[0] = pStartTmp;
					nCount = 0;
				}
				else 
				{
					bool bFind = false;
					for (int i=0; i<2; i++)
					{
						int nY1 = pStartTmp.y - i;
						int nY2  = pStartTmp.y + i;

						if (nY1 >= 0)
						{
							if (pBuffer[(nHeight-1-nY1)*nNewWidth + pStartTmp.x] <= nThreshold)
							{
								pExtream[0] = pStartTmp;
								bFind = TRUE;
								nCount = 0;
								break;
							}
						}

						if (nY2 < nHeight-1)
						{
							if (pBuffer[(nHeight-1-nY2)*nNewWidth + pStartTmp.x] <= nThreshold)
							{
								pExtream[0] = pStartTmp;
								nCount = 0;
								bFind = TRUE;
								break;
							}
						}
					}
					

					if (!bFind)
					{
						nCount++;

						if (nCount > 3)
							break;
					}
				}
			}
			else 
				break;
		}

		pStartTmp = pStart;

		nCount = 0;
		while(1)
		{
			pStartTmp.x++;
			if (pStartTmp.x < nWidth-1)
			{
				int nGrayValue = pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + pStartTmp.x];
				if (nGrayValue <= nThreshold)
				{
					pExtream[1] = pStartTmp;
					nCount = 0;
				}
				else 
				{
					bool bFind = false;
					for (int i=0; i<2; i++)
					{
						int nY1 = pStartTmp.y - i;
						int nY2  = pStartTmp.y + i;

						if (nY1 >= 0)
						{
							if (pBuffer[(nHeight-1-nY1)*nNewWidth + pStartTmp.x] <= nThreshold)
							{
								pExtream[1] = pStartTmp;
								nCount = 0;
								bFind = TRUE;
								break;
							}
						}

						if (nY2 < nHeight-1)
						{
							if (pBuffer[(nHeight-1-nY2)*nNewWidth + pStartTmp.x] <= nThreshold)
							{
								pExtream[1] = pStartTmp;
								nCount = 0;
								bFind = TRUE;
								break;
							}
						}
					}
						

					if (!bFind)
					{
						nCount++;

						if (nCount > 3)
							break;
					}
				}
			}
			else 
				break;
		}

		int nLen = pExtream[1].x - pExtream[0].x;

		if (nLen <= 0)
			return FALSE;

		if (abs(nLen - dstSize.cx) <= 5)
			return TRUE;
	}
	else if (nIndex == 1) //竖向搜索
	{
		int nCount = 0;
		while(1)
		{
			pStartTmp.y --;
			if (pStartTmp.y >= 0 )
			{
				int nGrayValue = pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + pStartTmp.x];
				if (nGrayValue <= nThreshold)
				{
					nCount =  0;
					pExtream[0] = pStartTmp;
				}
				else 
				{
					bool bFind = false;
					for (int i=0; i<2; i++)
					{
						int nX1 = pStartTmp.x - i;
						int nX2  = pStartTmp.x + i;

						if (nX1 >= 0)
						{
							if (pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + nX1] <= nThreshold)
							{
								pExtream[0] = pStartTmp;
								bFind = TRUE;
								nCount =  0;
								break;
							}
						}

						if (nX2 < nWidth-1)
						{
							if (pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + nX1] <= nThreshold)
							{
								pExtream[0] = pStartTmp;
								bFind = TRUE;
								nCount =  0;
								break;
							}
						}
					}
					if (!bFind)
					{
						nCount++;
						if (nCount > 3)
							break;
					}
				}
			}
			else 
				break;
		}

		pStartTmp = pStart;

		nCount = 0;
		while(1)
		{
			pStartTmp.y++;
			if (pStartTmp.y < nHeight-1)
			{
				int nGrayValue = pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + pStartTmp.x];
				if (nGrayValue <= nThreshold)
				{
					pExtream[1] = pStartTmp;
					nCount = 0;
				}
				else 
				{
					bool bFind = false;
					for (int i=0; i<2; i++)
					{
						int nX1 = pStartTmp.x - i;
						int nX2  = pStartTmp.x + i;

						if (nX1 >= 0)
						{
							if (pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + nX1] <= nThreshold)
							{
								pExtream[1] = pStartTmp;
								bFind = TRUE;
								nCount = 0;
								break;
							}
						}

						if (nX2 < nWidth-1)
						{
							if (pBuffer[(nHeight-1-pStartTmp.y)*nNewWidth + nX1] <= nThreshold)
							{
								pExtream[1] = pStartTmp;
								bFind = TRUE;
								nCount = 0;
								break;
							}
						}
					}

					if (!bFind)
					{
						nCount++;
						if (nCount > 3)
							break;
					}
				}
			}
			else 
				break;
		}

		int nLen = pExtream[1].y - pExtream[0].y;

		if (nLen <= 0)
			return FALSE;

		if (abs(nLen - dstSize.cy) <= 5)
			return TRUE;
	}
	
	return FALSE;
}

int CModelProcess::GetAllRects(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CSize dstSize, vector<LOCALRECT> &v_AllRects)
{
	int nNewWidth = BYTESPERLINE(nWidth, 8);
	for (int i=0; i<nHeight; i++)
	{
		for (int j=0; j<nWidth; j++)
		{
			int nGrayValue = pBuffer[(nHeight-1-i)*nNewWidth + j];
			if (nGrayValue <= nThreshold)
			{
				//TRACE("x:%d, y:%d, %d\n", j, i, nCount);
				CPoint pStart = CPoint(j, i);
				bool bFind = false;
				CRect dstRect;

				if (GetRect(pBuffer, nWidth, nHeight, nThreshold, pStart, dstSize, dstRect))
				{
					bFind = false;
					j = dstRect.BottomRight().x + 10; 
					

					LOCALRECT localRect;
					localRect.rect = dstRect;
					localRect.dDensity = GetDensity(pBuffer, nWidth, nHeight, nThreshold, dstRect);
					/*TRACE("矩形：topleft x0:%d, y0;%d, x1:%d, y1:%d, density:%f\n", 
						dstRect.TopLeft().x, dstRect.TopLeft().y, dstRect.BottomRight().x, dstRect.BottomRight().y, localRect.dDensity);*/
					v_AllRects.push_back(localRect);
					SetGrayValue(pBuffer, nWidth, nHeight, dstRect, 255);
				}
			}
		}
	}

	return v_AllRects.size();
}

BOOL  CModelProcess::IsPointInRect(CPoint p1, CRect rect)
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

BOOL CModelProcess::GetRectsByColumn(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CSize dstSize, vector<CRect> &v_AllRects)
{
	
	return FALSE;
}

void CModelProcess::SetGrayValue(BYTE *pBuffer, int nWidth, int nHeight, CRect rectArea, int nGrayValue)
{
	int nNewWidth = BYTESPERLINE(nWidth, 8);
	for (int i=rectArea.TopLeft().x; i<rectArea.BottomRight().x; i++)
	{
		for (int j=rectArea.TopLeft().y; j<rectArea.BottomRight().y; j++)
		{
			CPoint pStart = CPoint(i, j);
			pBuffer[(nHeight-1-pStart.y)*nNewWidth + pStart.x] = nGrayValue;
		}
	}
}

double CModelProcess::GetDensity(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CRect dstRect)
{
	double dDensity = 0.0;
	int nNewWidth = BYTESPERLINE(nWidth, 8);
	int nCount = 0;
	for (int i=dstRect.TopLeft().x; i<dstRect.BottomRight().x; i++)
	{
		for (int j=dstRect.TopLeft().y; j<dstRect.BottomRight().y; j++)
		{
			if (pBuffer[(nHeight-1-j)*nNewWidth + i] <= nThreshold)
			{
				nCount++;
			}
		}
	}

	dDensity = double(nCount) / double(dstRect.Width()*dstRect.Height());
	return dDensity;
}


BOOL CModelProcess::GetValidRects(vector<LOCALRECT> v_srcRects, vector<LOCALRECT> v_modelRects, int nEps)
{
	if (v_srcRects.size() < v_modelRects.size())
		return FALSE;

	int nCount = 0;
	for (vector<LOCALRECT>::iterator itModel = v_modelRects.begin(); itModel != v_modelRects.end(); itModel++)
	{
		LOCALRECT rectModel = *itModel;
		bool bFind = false;
		
		for (vector<LOCALRECT>::iterator itSrc = v_srcRects.begin(); itSrc != v_srcRects.end(); itSrc++)
		{
			LOCALRECT rectSrc = *itSrc;
			if (rectSrc.nRow < 0 || rectSrc.nColumn < 0)
			{
				if (GetLength(rectModel.rect.CenterPoint(), rectSrc.rect.CenterPoint()) <= nEps)
				{
					bFind = TRUE;
					nCount++;
					rectSrc.nRow = rectModel.nRow;
					rectSrc.nColumn = rectModel.nColumn;
					break;
				}
			}
		}

		if (!bFind)
		{
			return FALSE;
		}
	}

	if (nCount != v_modelRects.size())
		return FALSE;

	return TRUE;
}

double CModelProcess::GetLength(CPoint p1, CPoint p2)
{
	double dLength = 0.0;
	double dLenX = double(p2.x-p1.x);
	double dLenY = double(p2.y-p1.y);
	double dX = pow(dLenX, 2.0);
	double dY = pow(dLenY, 2.0);
	dLength = sqrt(dX + dY);
	return dLength;
}

BOOL CModelProcess::GetModelRectsInfo(vector<LOCALRECT> v_modelRects, int nEps)
{
	for (vector<LOCALRECT>::iterator it1 = v_modelRects.begin(); it1 != v_modelRects.end(); it1++)
	{
		for (vector<LOCALRECT>::iterator it2 = it1+1; it2 != v_modelRects.end(); it2++)
		{
			CRect rect1 = *it1->rect;
			CRect rect2 = *it2->rect;
			if (abs(rect1.CenterPoint().x - rect2.CenterPoint().x) > nEps)
			{
				if (rect1.CenterPoint().x > rect2.CenterPoint().x)
				{
					 //交换
					LOCALRECT localRect = *it1;
					*it1 = *it2;
					*it2 = localRect;
				}
			}
		}
	}

	int nIndex = 0;
	for (vector<LOCALRECT>::iterator it=v_modelRects.begin(); it != v_modelRects.end(); it++)
	{
		for (vector<LOCALRECT>::iterator it2 = it+1; it2 != v_modelRects.end(); it2++)
		{
			if (abs(it2->rect.CenterPoint().x - it->rect.CenterPoint().x) <= nEps)
			{
				it->nColumn = nIndex;
				it2->nColumn = nIndex;
			}
			else
			{
				nIndex++;
				it = it2;
				it->nColumn = nIndex;
			}
		}
	}

	for (vector<LOCALRECT>::iterator it1 = v_modelRects.begin(); it1 != v_modelRects.end(); it1++)
	{
		for (vector<LOCALRECT>::iterator it2 = it1+1; it2 != v_modelRects.end(); it2++)
		{
			CRect rect1 = *it1->rect;
			CRect rect2 = *it2->rect;

			if (abs(rect1.CenterPoint().y - rect2.CenterPoint().y) > nEps)
			{
				if (rect1.CenterPoint().y > rect2.CenterPoint().y)
				{
					//交换
					LOCALRECT localRect = *it1;
					*it1 = *it2;
					*it2 = localRect;
				}
			}
		}
	}

	nIndex = 0;
	for (vector<LOCALRECT>::iterator it=v_modelRects.begin(); it != v_modelRects.end(); it++)
	{
		for (vector<LOCALRECT>::iterator it2 = it+1; it2 != v_modelRects.end(); it2++)
		{
			if (abs(it2->rect.CenterPoint().y - it->rect.CenterPoint().y) <= nEps)
			{
				it->nRow = nIndex;
				it2->nRow = nIndex;
			}
			else
			{
				nIndex++;
				it = it2;
				it->nRow = nIndex;
			}
		}
	}
	
	for (vector<LOCALRECT>::iterator it = v_modelRects.begin(); it != v_modelRects.end(); it++)
	{
		LOCALRECT localRect = *it;
		TRACE("x:%d, y:%d, nRow:%d, nColumn:%d, Density:%f\n", localRect.rect.CenterPoint().x, localRect.rect.CenterPoint().y,
			localRect.nRow, localRect.nColumn, localRect.dDensity);
	}

	return TRUE;
}

bool CModelProcess::GetBrackets(BYTE *pBuffer, int nWidth, int nHeight, CPoint pStart, int nThreshold, BRACKETRECT &dstRect)
{
	int nNewWidth = BYTESPERLINE(nWidth, 8);

	//if (pBuffer[(nHeight-1-pStart.y)*nNewWidth + pStart.x]) 
	if (GRAYVALUE(pBuffer, nNewWidth, nHeight, pStart.x, pStart.y) > nThreshold)
		return FALSE;

	//获取横向
	CPoint pExtreamHTop[2];
	CPoint pExtreamHBottom[2];

	//纵向
	CPoint pExtreamVLeft[2];
	CPoint pExtreamVRight[2];
	CSize dstSize;
	if (GetLine(pBuffer, nWidth, nHeight, nThreshold, pStart, 0, dstSize, pExtreamHTop))
	{
		//获取竖向点
		
		
	}
	else //获取竖向 
	{
		CPoint pExtream2[2];
		GetLine(pBuffer, nWidth, nHeight, nThreshold, pStart, 1, dstSize, pExtream2);

		//获取横向点
		//GetLine(pBuffer, nWidth, nHeight, nThreshold, pExtream2[0], 0, dstSize, pExtream1);
		//GetLine(pBuffer, nWidth, nHeight, nThreshold, pExtream2[1], 0, dstSize, pExtream1);
	}
	



	return FALSE;
}
>>>>>>> c3ec4193a47e985f94758967b6bacfb8a4ab020b
