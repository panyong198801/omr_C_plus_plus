#pragma once

#include "stdafx.h"
#include <vector>
using namespace  std;

#define  GRAYVALUE(pBuffer, newWidth, nHeight, x, y) pBuffer[nNewWidth*(nHeight-1-y) + x]

class CModelProcess
{
public:
	CModelProcess(void);
	~CModelProcess(void);
	
	BOOL IsPointInRect(CPoint p1, CRect rect); //判断某点是否在相应的矩形区域
	BOOL PickBoundary(BYTE *pBuffer, int nWidth, int nHeight,int nThreshold, vector<CRect> v_rects);
	BOOL GetRectExpand(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CPoint pStart, vector<CPoint> v_allPoints);

	int GetAllRects(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CSize dstSize, vector<LOCALRECT> &v_AllRects); 
	BOOL GetRect(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CPoint pStart, CSize dstSize,CRect &dstRect);
	BOOL GetLine(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CPoint pStart, int nIndex,  CSize dstSize, CPoint pExtream[2]);
	
	BOOL GetRectsByColumn(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CSize dstSize, vector<CRect> &v_AllRects);
	void SetGrayValue(BYTE *pBuffer, int nWidth, int nHeight, CRect rectArea, int nGrayValue);

	double GetDensity(BYTE *pBuffer, int nWidth, int nHeight, int nThreshold, CRect dstRect);
	BOOL GetValidRects(vector<LOCALRECT> v_srcRects, vector<LOCALRECT> v_modelRects, int nEps);
	double GetLength(CPoint p1, CPoint p2);			

	BOOL GetModelRectsInfo(vector<LOCALRECT> v_modelRects, int nEps); //获取模板矩形信息

	bool GetBrackets(BYTE *pBuffer, int nWidth, int nHeight, CPoint pStart, int nThreshold, BRACKETRECT &dstRect);

};
