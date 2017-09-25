<<<<<<< HEAD
#pragma once
#include  "include/zyj-dllexport.h"
#include <vector>
using namespace std;

struct LOCALPOINT_DATA
{
	char cCurImgPath[256];
	NEWRECT pLocalRects[3];

	LOCALPOINT_DATA()
	{
		memset(cCurImgPath, '\0', 256);
	}
};

class CurImage
{
public:
	CurImage(void);
	~CurImage(void);
	
	int m_nThreshold; //阈值 
	vector<NEWRECT> m_rectLocal; //定位点
	//int m_nLocalType; //定位点类型 1：三角形  2：矩形

	vector<RECTARRAY> v_modelRectArrays; //模板中客观题区域
	vector<NEWRECT>   v_rectModelLocal;    //模板中三个定位点所在矩形位置
	vector<int>  v_OMRZuIDs; // 模板中组ID信息
	vector<RECTARRAY> v_modelKhRectArray; //模板中考号所在矩形位置
	vector<RECTARRAY> v_modelScoreRectArray; //模板中分数矩形框所在位置
	vector<RECTARRAY> v_modelTitleQRRectArray; //模板中标题二维码
	vector<RECTARRAY> v_modelKhQRRectArray;   //模板中考号条码所在矩形
	vector<RECTARRAY> v_modelPageRectArray;   //模板中页号所在矩形
	RECTARRAY  modelAbsentRectArray;          //模板中缺考所在矩形

	int m_nAverGrayValue; 
	double m_dAverDensity;

	//根据最佳匹配获取客观题上位置点 2016.12.15
	void GetCurImgRectArray(const CxImage srcImg, const vector<RECTARRAY> v_ModelRA, int nThreshold, vector<RECTARRAY> &v_dstRA);
	bool IsValidRect(CxImage srcImg, int nThreshold, CSize dstSize, NEWRECT rect1); //获取当前有效矩形 2016.12.15

};
=======
#pragma once
#include  "include/zyj-dllexport.h"
#include <vector>
using namespace std;

struct LOCALPOINT_DATA
{
	char cCurImgPath[256];
	NEWRECT pLocalRects[3];

	LOCALPOINT_DATA()
	{
		memset(cCurImgPath, '\0', 256);
	}
};

class CurImage
{
public:
	CurImage(void);
	~CurImage(void);
	
	int m_nThreshold; //阈值 
	vector<NEWRECT> m_rectLocal; //定位点
	//int m_nLocalType; //定位点类型 1：三角形  2：矩形

	vector<RECTARRAY> v_modelRectArrays; //模板中客观题区域
	vector<NEWRECT>   v_rectModelLocal;    //模板中三个定位点所在矩形位置
	vector<int>  v_OMRZuIDs; // 模板中组ID信息
	vector<RECTARRAY> v_modelKhRectArray; //模板中考号所在矩形位置
	vector<RECTARRAY> v_modelScoreRectArray; //模板中分数矩形框所在位置
	vector<RECTARRAY> v_modelTitleQRRectArray; //模板中标题二维码
	vector<RECTARRAY> v_modelKhQRRectArray;   //模板中考号条码所在矩形
	vector<RECTARRAY> v_modelPageRectArray;   //模板中页号所在矩形
	RECTARRAY  modelAbsentRectArray;          //模板中缺考所在矩形

	int m_nAverGrayValue; 
	double m_dAverDensity;

	//根据最佳匹配获取客观题上位置点 2016.12.15
	void GetCurImgRectArray(const CxImage srcImg, const vector<RECTARRAY> v_ModelRA, int nThreshold, vector<RECTARRAY> &v_dstRA);
	bool IsValidRect(CxImage srcImg, int nThreshold, CSize dstSize, NEWRECT rect1); //获取当前有效矩形 2016.12.15

};
>>>>>>> c3ec4193a47e985f94758967b6bacfb8a4ab020b
