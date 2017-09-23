#pragma once 
#pragma pack(1)

#include "stdafx.h"
#include "xml/Markup.h"
#include "include/zyj-dllexport.h"
#include <vector> 
#include "Image/CxImage/ximage.h"
using namespace std;
//#include "include/kadmos.h"

struct OMRRECT{
	CRect rect;
	int nCount;
	OMRRECT()
	{
		nCount = 0;
		rect.SetRect(0,0,0,0);
	}
};

//#include "../../../../code/bin/"
class CImageProcess
{
public:
	CImageProcess(void);

public: 
	~CImageProcess(void);
	CString GetCurrentFolderPath();            //获取当前程序所在文件夹路径
	BOOL IsPointInRect(CPoint p1, CRect rect); //判断某点是否在相应的矩形区域
	CPoint GetMidPoint(CPoint p1, CPoint p2);  //获取两点之间中点
	double GetTrangleAreaPoints(CPoint p1, CPoint p2, CPoint p3); //获取三角形的面积
	double GetAnglePoints(CPoint pCross, CPoint p1, CPoint p2);
	static double GetLength(CPoint p1, CPoint p2);
	CRect NewRect2Rect(NEWRECT newRect);
	void GetBlackCount(const CxImage *img, CRect rectArea,int nThreshold,int &nCount); //获取图像中黑点的个数   pany 2016.03.18
	void GetRatio(NEWPOINT pLocal[3], NEWPOINT pCenter, double &dHRatio, double &dVRatio); //获取某点沿定位点两个方向上的比例
	void GetNewPoint(NEWPOINT pCurLocal[3], double dHRatio, double dVRatio, NEWPOINT &pCenter);
	
	//获取中括号类型边框
	bool GetValidRect(int nType, const CxImage *img, vector<CRect> &v_allRects, int nThreshold, CSize dstSiz); //获取有效的中括号未填涂矩形
	bool GetAllPoint(CxImage *img, vector<CPoint> &v_allPoints, CPoint pStart, int nThreshold, int &nCount); //获取某点周围nStep大小的所有临近点
	bool GetRect(vector<CPoint> v_allPoints, CRect &dstRect); //根据聚集点获取矩形
	//bool loadSrcImg(CString strPath, DWORD DwType, int nThreshold);

	//获取封闭矩形类型边框 2015.12.22
	bool  GetEnclosedRects(int nType, CxImage *img, vector<CRect> &v_allRects, int nThreshold, CSize dstSize); //获取封闭的矩形 

	//获取手写体矩形边框  2016.07.28
	bool GetIcrRects(CxImage srcImg, CSize dstSize, int nThreshold, vector<CRect> &v_allRects);

	//获取横向直线
	BOOL GetHLine(CPoint pExtream[2], CPoint pStart, CRect rectArea, CSize dstSize,const CxImage *img, int nThreshold, int nMaxGapLen);
	//获取横向直线
	BOOL GetHLine_OMR(CPoint pExtream[2], CPoint pStart, CRect rectArea, CSize dstSize,const CxImage *img, int nThreshold, int nMaxGapLen);
	BOOL GetRectFromPoint(CRect &dstRect, CPoint pStart, CRect rectArea, CSize dstsize,const CxImage *img, int nThreshold);
 
   //同步头相关识别
   BOOL GetRectFromTwoRects(CRect &dstRect, CRect rect[2], CRect rectArea, CSize dstSize, CxImage *img, int nThreshold);
   void GetAllRectsFromRects(vector<CRect>&v_dstRects, CRect rect[2], CRect rectArea, CSize dstSize, CxImage *img, int nThreshold);

   //获取三角形定位点
   BOOL GetTriAngleLocalPoint(CxImage *img, const CSize dstSize, const int nThreshold, NEWTRIANGLE &dstTriangle);	
	
   //获取
   BOOL GetPointsCenter(vector<CPoint> v_allPoints, CPoint &pCenter);

   //识别所有题号 2016.01.22
   //获取识别结果
   //CString GetOcrResult(CxImage *img);
   
   //获取题号所在具体矩形区域
   bool GetTitleRect(CxImage *img, vector<TITLENO> v_modelTitleNo, int nThrshold, vector<TITLENO> &v_dstTitleNo);
	
   BOOL GetHLine1(CPoint pExtream[2],CPoint pStart, CRect rectArea, const CxImage *img,CSize minSize, CSize maxSize, const int nThreshold);
   BOOL GetRectFromPoint1(CRect &dstRect, CPoint pStart, CRect rectArea,const CxImage *img, CSize minSize, CSize maxSize, int nIndex, const int nThreshold,  const CxImage* pSrcImg);
   BOOL GetRectFromRect1(CRect &dstRect,
	   CRect rectArea,  
	   const CxImage *img,
	   CSize minSize,
	   CSize maxSize,
	   int nIndex,
	   const CxImage *pSrcImg,
	   const int nThreshold);

   //获取矩形定位点相关
   bool GetRectPoint(CRect &dstRect, 
					 const CxImage *srcImg,
					 const int nIndex, 
					 const CRect rectArea,
					 const CxImage *cropImg,
					 const int nThrshold,
					 const CSize minSize,
					 const CSize maxSize);

   double GetDensity(const CxImage *img,  CRect rect, const int nThreshold); 
   double GetHollowDensity( CxImage *cropImg, const int nThreshold, const CSize centerSize);

   //获取当前dll所在文件路径
   CString GetMoudulePath(); //2016.06.14
   int GetCrossPointsCount(CRect rect1, CRect rect2);//矩形相交区域面积
   bool GetVLinelen( CxImage srcImg, int nThreshold, int &nLen1);
   void GetThinImage(CxImage *cropImg, int nThreshold, int nTimes );

   void GetChainContour_Encolsed(CxImage srcImg,  int nThreshold, CSize dstSize, vector<CRect> &v_AllRects);
   void GetCharContour(CxImage srcImg,  int nThreshold, CRect &dstRect); // 获取文字所在外接矩形
   void GetChainContour(CxImage srcImg,  int nThreshold, CSize dstSize, vector<OMRRECT> &v_AllRects); //获取所有边框外接矩形 2016.06.21
   bool Cximage2Iplimage_1(CxImage *src, IplImage **dst);

   //获取填涂区横向最大高度 pany 2016.06.22
   int GetMaxFillHeight(CxImage cropImg, int nThreshold, CSize dstSize);

   //获取矩形框  2016.06.28
   bool GetIcrRect(CxImage srcImg, CSize dstSize, int nThreshold, CString &strRes);
   bool GetMinRect(CxImage srcImg, CSize minSize, int nThreshold, vector<CRect> &v_allRects);

   //获取白点所在位置 2016.07.28
   bool GetWhiteRect(CxImage srcImg, int nThreshold, CRect rect1, CRect &dstRect);

   bool IsAroundBlack(CxImage srcImg, int nThreshold, CPoint p1);
   bool GetValidExternalCharRect(CxImage srcImg, int nThreshold, CRect &dstRect); //获取字符所在的有效外接矩形 2016.08.05 pany
   bool GetExpandImg(CxImage srcImg, int nNewWidth, int nNewHeight, CxImage &dstImg);
private:
	CSize m_dstBlackSize;
	
	//手写相关 2016.06.14 
	//RelData m_rel;
	//bool  m_bInitIrc;
};
