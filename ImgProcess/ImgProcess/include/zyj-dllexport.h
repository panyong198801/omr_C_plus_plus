

/*********************************************************
Explain: omr
Author: panyong 
Time: 2015/12/14
********************************************************/

#pragma once 
#ifndef _ZYJ_DLLEXPORT_H_INCLUDED_
#define _ZYJ_DLLEXPORT_H_INCLUDED_

#include "opencv/cv.h"
#include "opencv/highgui.h"

#pragma pack(push,1)   
# pragma warning (disable:4819)

#define DLL_EXPORT __declspec(dllexport)


#define MAX_ANSWER_SIZE 200	//added by cbj 20160801
#define DEFAULT_THRESHOLD 200
#define MAX_SCORE_SIZE 100
#define MAX_PAGE 4

struct NEWRECT;

typedef struct NEWPOINT
{
	int nX;
	int nY;

	inline bool operator ==(const NEWPOINT &nP)const
	{
		if (nX == nP.nX && nY == nP.nY)
			return TRUE;

		return FALSE;
	}

	void setPoint(int x, int y)
	{
		nX = x;
		nY = y;
	}	
}pNEWPOINT;

typedef struct NEWRECT
{
	NEWPOINT pTopLeft;
	NEWPOINT pBottomRight;

	int nPage;
	int nChoiceType; 
	NEWRECT()
	{
		nPage = -1;
		nChoiceType = 0; //a
	}
	
	int GetWidth()
	{
		int nWidth = pBottomRight.nX - pTopLeft.nX;
		return nWidth;
	}

	int GetHeight()
	{
		int nHeight = pBottomRight.nY - pTopLeft.nY;
		return nHeight;
	}

	void SetRect(NEWPOINT p1, NEWPOINT p2)
	{
		pTopLeft = p1;
		pBottomRight = p2;
	}

	void SetRect(int nX, int nY, int nW, int nH)
	{
		pTopLeft.nX = nX;
		pTopLeft.nY = nY;
		pBottomRight.nX = nX + nW;
		pBottomRight.nY = nY + nH;
	}

	void AddPoint(NEWPOINT p1)
	{
		pTopLeft.nX += p1.nX;
		pTopLeft.nY += p1.nY;
		
		pBottomRight.nX += p1.nX;
		pBottomRight.nY += p1.nY;
	}

	NEWPOINT CenterNewPoint()
	{
		NEWPOINT pCenter;
		pCenter.nX = pTopLeft.nX + GetWidth()/2;
		pCenter.nY = pTopLeft.nY + GetHeight()/2;
		return pCenter;
	}

}pNEWRECT;


typedef struct NEWTRIANGLE
{
	NEWPOINT pRightAngle;  //鐩磋
	NEWPOINT pAcuteAngleH; //閿愯1
	NEWPOINT pAcuteAngleV; //閿愯2

	NEWPOINT CenterNewPoint()
	{
		NEWPOINT pCenter;
		pCenter.nX = (pAcuteAngleH.nX + pAcuteAngleV.nX)/3;
		pCenter.nY = (pAcuteAngleH.nY + pAcuteAngleV.nY)/3;
		return pCenter;
	}

	NEWRECT GetNewRect()
	{
		NEWRECT newRect1;
		int nTop  = min(min(pRightAngle.nY, pAcuteAngleH.nY), pAcuteAngleV.nY);
		int nLeft = min(min(pRightAngle.nX, pAcuteAngleH.nX), pAcuteAngleV.nX);
		int nRight = max(max(pRightAngle.nX, pAcuteAngleH.nX), pAcuteAngleV.nX);
		int nBottom  = max(max(pRightAngle.nY, pAcuteAngleH.nY), pAcuteAngleV.nY);
		newRect1.SetRect(nLeft, nTop, nRight-nLeft+1, nBottom-nTop+1);
		return newRect1;
	}

}pNewTriangle;

typedef struct RECTARRAY
{
	NEWRECT rect;     //对应框所在矩形
	int nQuestionNo;  //对应题号ID
	int nAnswerIndex; //所在答案序号
	short nType;     //填涂矩形类型 0：中括号 1：封闭矩形
	int nZuID;		//对应组ID

	int nFilling;   //填涂情况 -1:未知,  0: 未填涂 1：填涂 2：怀疑
	int nFilling_220; 
	int nFilling_140;
	double dDensity;    //当前区域黑点密度
	int nPix;
	double dDensity_220; //220阈值密度 
	double dDensity_140;
	bool bMinFilling;
	bool bMinFilling_220;
	bool bMinFilling_140;
	int nSelType;     //选项属性 0：单选 1：多选
	int nAverGrayValue; //平均灰度值
	int nABFlag;      //AB卡属性 pany  2016.05.10
	int nWhiteGray;  //白底区域平均灰度值 2016.06.08
	char cTitle[256]; //题号

	RECTARRAY()
	{
		nQuestionNo = -1;
		nZuID = -1;
		nType       = 0;  //锟斤拷锌锟?
		nFilling    = -1; //锟轿粗?
		nFilling_220 = -1;
		nFilling_140 = -1;
		bMinFilling = false;
		bMinFilling_220 = false;
		bMinFilling_140 = false;
		//nCheckFilling = -1;
		dDensity    = 0.0;
		dDensity_220 = 0.0;
		dDensity_140 = 0.0;
		//dCheckDensity = 0.0;
		nSelType    = 0;
		nAverGrayValue = 200;
		nABFlag = -1;
		//dHollowDensity =1.0;
		nWhiteGray = 255;
		memset(cTitle, '\0', 256);
		
	}

	//锟斤拷锟矫撅拷锟斤拷锟斤拷锟?
	void SetRectArray(int nX, int nY, int nW, int nH, int nNo, int nIndex)
	{
		rect.SetRect(nX, nY, nW, nH);
		nQuestionNo = nNo;
		nAnswerIndex = nIndex;
	}

	NEWPOINT CenterNewPoint()
	{
		return rect.CenterNewPoint();
	}

	void AddPoint(NEWPOINT p1)
	{
		rect.AddPoint(p1);
	}
}pRECTARRAY;


//客观题/考号/主观题分数 识别答案 
struct ANS
{
	int nID;
	char cAnswer[26]; //识别结果
	char cTitle[256]; //题号
	NEWRECT dstNewRectFirst; //客观题备选矩形框
	int nLenX; //两备选矩形之间X方向距离
	int nLenY; //两备选矩形之间Y方向距离
	int nSize;  //矩形框个数
	bool bNormal; //是否正常选项
	int nPage; //当前页号
	ANS()
	{
		nID = -1;
		nSize = 0;
		memset(cAnswer, '\0', 26);
		memset(cTitle, '\0', 256);
		bNormal = true;
	}
};

//主观题分数
struct SCORE_ANS
{
	int nID;
	char cScore1[10]; //十位数结果
	char cScore2[10]; //个位数结果
	char cScore3[10]; //小数位结果e
	
	//第二种识别算法得出结果 2016.08.25
	char cScore_New1[10];
	char cScore_New2[10];
	char cScore_New3[10];

	char cTitle[256]; //题号
	NEWRECT dstRect[3];
	bool bNormal;
	SCORE_ANS()
	{
		nID = -1;
		for (int i=0; i<3; i++)
			dstRect[i].SetRect(-1, -1, 0, 0);

		memset(cScore1, '\0', 10);
		memset(cScore2, '\0', 10);
		memset(cScore3, '\0', 10);

		memset(cScore_New1, '\0', 10);
		memset(cScore_New2, '\0', 10);
		memset(cScore_New3, '\0', 10);
		memset(cTitle, '\0', 256);
		bNormal = true;
	}
};


typedef  struct TITLENO
{
	int nTitleNo;
	NEWRECT rectLocal;	

	TITLENO()
	{
		nTitleNo = -1;
		rectLocal.SetRect(0,0 ,0,0);
	}

}pTitleNo;

//切割图像相关
struct CUTIMGINFO
{
	NEWRECT rectArea;    //子图区域
	char cSavePath[256]; //子图路径

	CUTIMGINFO()
	{
		memset(cSavePath, '\0', 256);
	}
};

enum 
{
	TYPE_BMP = 0,
	TYPE_JPG,
	TYPE_PNG,
	TYPE_TIF,
	TYPE_QTHER
};

enum 
{
	RES_SUCCESS = 0,  //成功
	ERROR_IMAGEFILE = 1000, //图像加载失败
	ERROR_MODELFILE,        //模板文件失败
	ERROR_LOCATE,            
	ERROR_RECOGNIZE         
};

enum {
	TYPE_LOCATE_POINT = 0,		// 定位点
	TYPE_PAGE_NUM,				// 页码
	TYPE_CHOICE_LOCATE_POINT,	// 选择题定位点// 统一为题目定位点
	TYPE_AB_FLAG,				// AB卷
	TYPE_ABSENT_FLAG,			// 缺考标记
	TYPE_EXAM_NUM,				// 考号
	TYPE_EXAM_NUM_BAR_CODE,		// 考号条码
	TYPE_CHOICE_ITEM,			// 选择题// 多选题
	TYPE_SUBJECTIVE_ITEM,		// 主观题
	TYPE_SUBJECT_LOCTION,		// 科目名称
	TYPE_CHEAT_FLAG,			// 违纪标记
	TYPE_OPTIONALQUESTION,		// 选做题
	TYPE_CHOICE_ITEM_SINGLE,	// 选择题// 单选题
	TYPE_OPTIONALQUESTIONPOS,	// 选做题定位点
	TYPE_SCORE_ITEM,			//分数
	TYPE_TITLE_QRCODE_ITEM,		//标题二维码
};

#pragma pack(pop)
/*
函数说明:加载模板
*/
extern "C" DLL_EXPORT bool ZYJ_LoadTemplateFile(const char *cModelPath); 

/*
函数说明: 获取客观题识别结果
参数说明:
cModelPath: 模板文件路径
cFilePath:  图像文件路径 
nPage: 当前页号,从1开始
nOmrType: 填涂区类型 0：中括号 1：封闭矩形
nThreshold: 二值化阈值
dstAns: 需识别结果，由外部申请和释放
nABFlag: AB卡属性值 A:0 B:1 非：-1
*/
extern "C" DLL_EXPORT bool ZYJ_GetCurOmrRes1(const char *cFilePath, const int nPage, const int nOmrType,const int nThreshold, const int nRecType, ANS *dstAns, int nABFlag);

//第二套识别算法 2016.12.15
//pCurLocal 当前图像中找到的三点
//pModelLocal 模板中对应的三点 
extern "C" DLL_EXPORT bool ZYJ_GetCurOmrRes3(const char *cFilePath, const int nPage, const int nThreshold, ANS *dstAns, const int nABFlag);

//套红卡客观题识别 2017.02.13
extern "C" DLL_EXPORT bool ZYJ_GetCurOmrRes4(const char *cFilePath, const int nPage, const int nThreshold, ANS *dstAns, const int nABFlag);
 
/*
藕锟斤拷锟剿碉拷:锟斤拷取锟斤拷前锟斤拷锟斤拷锟斤拷锟?
*/
extern "C" DLL_EXPORT int ZYJ_GetLastError(char cResInfo[256]);

//图像句柄保存至文件
extern "C" DLL_EXPORT bool ZYJ_SaveImgHandle(HANDLE hBmp, const char *cPath);

//保存图像文件
extern "C" DLL_EXPORT bool ZYJ_SaveIplImage(IplImage *src, const char *cPath);

//图像转化
extern "C" DLL_EXPORT bool ZYJ_Cximage2Iplimage(HANDLE hBmp, IplImage **dst);

/*
函数说明：原图切割并保存子图图像路径
参数说明:
cFilePath:原图路径
pCutInfo:对应子图的切割路径及切割区域，需事先排列好,需外部申请释放
nSize:切割子图的个数
注:合并的子图也需单个路径及区域存储 
*/
extern "C" DLL_EXPORT bool  ZYJ_CutImg(const char *cFilePath, const CUTIMGINFO *pCutInfo, const int nSize);

//cSrcPath:   某考生所有原图路径 
//cDstFolder: 某考生子图目标文件目录 
extern "C" DLL_EXPORT bool ZYJ_CutImage(char cSrcPath[MAX_PAGE][256], const char *cDstFolder);
 
//判断图像是否完整，防止有折角产生
//cFilePath:图像文件所在路径 
//nMinLen: 折痕的最小长度
//nAngleIndex 角度所在序列  0：左上 1：右上 2:左下 3：右下 
//**注 某一角度本身为折角, 则无需将该
extern "C" DLL_EXPORT bool ZYJ_IsComplete(const char *cFilePath, int nMinLen, int nAngleIndex);  

//产生怀疑客观题二次识别
extern "C" DLL_EXPORT bool ZYJ_GetSecondRecognization(ANS *dstAns); 

//自动获取图像边缘
extern "C" DLL_EXPORT bool ZYJ_GetImgEdge(const char *cPath, int nThreshold);

//自动获取图像四周定位点
extern "C" DLL_EXPORT bool ZYJ_GetAllImgLocalPoints(const char *cPath, int nThreshold, NEWRECT dstNewRect[4]);

//将图像重新定义在同一坐标系下 
extern "C" DLL_EXPORT bool ZYJ_SetImgCoordinateSystem(const char *cPath, NEWPOINT pStart, NEWRECT rectArea);

extern "C" DLL_EXPORT bool ZYJ_GetImgSize(const char *cPath,  int &nImgWidth, int &nImgHeight);

////获取识别手写字符 2016.06.15
//extern "C" DLL_EXPORT bool ZYJ_GetIcrChar(const char *cPath, NEWRECT rectArea, char *cRes[256]);

//识别科目二维码
extern "C" DLL_EXPORT bool ZYJ_GetExamTitle(const char *cPath, int nPage, char cResTitle[256]);

//识别考号条码 
extern "C" DLL_EXPORT bool ZYJ_GetQRKh(const char *cPath, int nPage, char cResKh[36]);

//获取模板中页号 
extern "C" DLL_EXPORT bool ZYJ_GetTemplatePage(int &nPage);

//获取模板中二维码
extern "C" DLL_EXPORT bool ZYJ_GetTemplateQRTitle(char cRes[256]);

//获取模板版本号
extern "C" DLL_EXPORT bool ZYJ_GetTemplateVersion(char cRes[256]);

//获取手写矩形框 2016.06.28
extern "C" DLL_EXPORT bool ZYJ_GetIcrRect(const char *cPath, NEWRECT rectArea, int nThreshold, int nMinWidth, int nMinHeight, NEWRECT dstRect[256], int &nRectCount);

extern "C" DLL_EXPORT bool ZYJ_GetQRCodeRes(const char *cPath, NEWRECT rectArea, char *cRes);

//判断当前页号下客观题是否需要识别 2016.08.15
extern "C" DLL_EXPORT bool ZYJ_IsNeedOmr(int nPage);

//判断当前页号下主观题分数是否需要识别
extern "C" DLL_EXPORT bool ZYJ_IsNeedRecScore(int nPage);

//根据ID号获取相应题号 2016.08.15
extern "C" DLL_EXPORT bool ZYJ_GetTitleFromID(int nID, char cTitleRes[256]);

//根据ID号判断题型 2016.08.16
//nQuestionType:返回0客观题  返回1主观题分数 
extern "C" DLL_EXPORT bool ZYJ_GetQuestionTypeFromID(int nID, int &nQuestionType);

//根据ID号获取所有矩形 
extern "C" DLL_EXPORT bool ZYJ_GetRectsFromID(int nID, NEWRECT rectFirst, NEWRECT *rects, int &nRectSize);

//根据ID号获取当前ID所在第几页
extern "C" DLL_EXPORT bool ZYJ_GetPageFromID(int nID, int &nPageIndex);

//获取元素类型个数
extern "C" DLL_EXPORT int ZYJ_GetElementSize(int nElementType);

//获取元素值
extern "C" DLL_EXPORT bool ZYJ_GetElement(RECTARRAY *dstRectArray, int nElementType);

//获取当前图像所在页号
extern "C" DLL_EXPORT bool ZYJ_CheckPage(const char *cPath, int nCurPage);

//判断当前图像（首页）是否缺考 2017.01.04
extern "C" DLL_EXPORT bool ZYJ_IsCardAbsent(const char *cPath, int nThreshold);

//获取横向/纵向同步头 2017.02.05
//rectArea 框选区域
//nDirection: 0横向 1：竖向
extern "C" DLL_EXPORT bool ZYJ_GetOmrSyncHead(const char *cPath, const NEWRECT rectArea, const  int nThreshold, const int nDirection, int &nDstSize, NEWRECT v_dstRects[]);

extern "C" DLL_EXPORT bool ZYJ_GetOmrSyncHead_1(const IplImage *iplImg, const NEWRECT rectArea, const  int nThreshold, const int nDirection, int &nDstSize, NEWRECT v_dstRects[]);
#endif