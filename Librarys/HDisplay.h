#pragma once
#include <opencv2/imgproc/imgproc_c.h>
#include "drawdata.h"
#define MINRANGE		10
#define RADIUS			5
#define CIRCLEPITCH		5

#ifdef USE_OPENCV440

using namespace DrawLib;
using namespace std;



class HDisplay
{
public:
	HDisplay();
	virtual ~HDisplay();
	
public:
	virtual void DrawImage(cv::Mat* pImage);
	virtual void DrawImage(IplImage* pImage);
	virtual int ChangeLayer(int Layer1,int Layer2);
	virtual int	cvOnMouseUp(CvPoint pt);
	virtual int	cvOnMouseDown(CvPoint pt);
	virtual int	cvOnRMouseDown(CvPoint pt);
	virtual int	cvOnMouseMove(CvPoint pt,bool bMouseDown=false);
	virtual int	cvOnLButtonDblClk(CvPoint pt);
	virtual void ChangeCursor(int id){};
	virtual void DrawOtherDatas(){};
	virtual int	 ChangeSelLayer(int nLayer);

	virtual void OnLButtonUp(CvPoint pt){};
	virtual void OnLButtonDown(CvPoint pt){};
	virtual void OnRButtonDown(CvPoint pt){};
	virtual void OnLButtonDblClk(CvPoint pt){};
	virtual void OnMouseMove(CvPoint pt,bool bMouseDown){};
	virtual void OnMouseWheelRun(CvPoint pt,int z){};
	virtual void ResetZoomOffset();
	virtual void ClearLayer(int Layer,bool bClearOffset=true);

	void Init(CvRect rect,CvScalar color=cvScalar(0,0,0));
	void ReDrawLayers();
	virtual void Zoom(double dblRate);
	void Offset(double dblX,double dblY);
	
	virtual void DrawLine(int Layer,CvPoint3D64f		*pPt=NULL,					CvScalar color=cvScalar(0,0,0),int thickness=1,int index=0,int direct=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawRect(int Layer,CvRect				*pRect=NULL,				CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,int direct=0,double *pData=NULL,double angle=0,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawCircle(int Layer,CvPoint3D64f		*pCenter=NULL,double radius=0,CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,int direct=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	
	virtual void DrawPolyline(int Layer,CvPoint3D64f	*pPt=NULL,double *bulges=NULL,int count=0,		CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,int direct=0,int flags=1,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawArc(int Layer,CvPoint3D64f	*pCenter=NULL,double radius=0,double start_angle=0, double end_angle=0,CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,int direct=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawArc(int Layer,double radius,CvPoint3D64f pt1,CvPoint3D64f pt2,CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,int direct=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawArc(int Layer,CvPoint3D64f ptCenter,CvPoint3D64f pt1,CvPoint3D64f pt2,CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,int direct=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	
	virtual void DrawArcs(int Layer,int count,CvPoint3D64f		*pPt=NULL,CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawCircles(int Layer,int count,CvPoint3D64f	*pCenter=NULL,double *pRadius=0,CvScalar *pColor= NULL,int thickness=1,int index=0,int direct=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));

	virtual void DrawPoint(int Layer,CvPoint3D64f		*pPt=NULL,	int	len=5,		CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawPoints(int Layer,int count,CvPoint3D64f		*pPt=NULL,	int	len=5,		CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawCrossLine(int Layer,CvPoint3D64f	*pPt=NULL,					CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0),int len=0);
	virtual void DrawText(int Layer,CvPoint3D64f *pPt,wstring strText,CvScalar color,double size=0.5,int thickness=1,int index=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawImage(int Layer,CvPoint3D64f *pPt,IplImage* pImage,double size=1,int index=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));

	virtual void DrawLines(int Layer,int count,CvPoint3D64f		*pPt=NULL,			CvScalar color= cvScalar(0,0,0),int thickness=1,int index=0,int direct=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawTexts(int Layer,int count,CvPoint3D64f		*pPt,wstring *strText,CvScalar color,double size=0.5,int thickness=1,int index=0,double *pData=NULL,CvPoint3D64f offset=cvPoint3D64f(0,0,0));
	virtual void DrawRects(int Layer,int count,CvRect			*pRect=NULL,		CvScalar *pColor=NULL,int thickness=1,int index=0,int direct=0,double *pData=NULL,double angle=0,CvPoint3D64f offset=cvPoint3D64f(0,0,0));

	void AddDrawPolyline(int Layer,CvPoint3D64f	Pt,double bulge,double* pDatas);
	
	void SaveDisplay(wstring strFile);
	void SaveImage(wstring strFile);

	bool GetLayers(std::vector<int>* pLayers);
	bool GetIndexs(std::vector<int>* pLayers);
	DrawData	*CreateNewLayer(int type,int Layer,int index,CvPoint3D64f offset);
	DrawData	*GetData(int Layer);
	DrawData	*GetDataByIndex(int Index);

	DrawData	*GetHightlightData();
	DrawData	*GetDrawData(int nLayer);
	DrawData	*GetDrawData2(int nIndex);
	DrawData	*GetDrawData(wstring strLayer,std::vector<DrawData*> &datas);
	IplImage	*GetSourceImage(int nID);
	IplImage	*GetDrawImage();
	
	IplImage	*TransImage(cv::Mat &mat);

protected:
	
	CvPoint3D64f	Trans2MM(CvPoint ptIn);
	CvPoint			Trans2Pixel(CvPoint3D64f ptIn);
	CvPoint3D64f	Trans2Pixelf(CvPoint3D64f ptIn);
	void			TransAngle(CvPoint3D64f pt,double &Base,double &Angle1,double &Angle2);
	void			TransAngle(CvPoint pt1,CvPoint pt2,bool bOutSide,CvPoint &ptC,double &Angle1,double &Angle2);
	bool	WaitDrawEvent(){return true;};
	virtual void		SetDrawEvent(){};	
	bool	WaitMaskEvent(){return true;};
	virtual void		SetMaskEvent(){};
	bool	WaitDatasEvent(){return true;};
	virtual void		SetDatasEvent(){};

	virtual DrawData*	DrawDatas(int Layer);
	CvPoint3D64f		TransPoint(CvPoint3D64f pt,CvPoint3D64f ptNow);
	
	inline void		SetDrawData(DrawData* pData);
	inline int		GetDrawDataLayer();
	inline int		GetDrawDataType();

	void	DrawLocalImage(IplImage* pImage,bool bPush2Images);
	void	DrawSelLine(int Layer,	CvPoint3D64f *pPt=NULL,int index=-1);
	void	DrawSelRect(int Layer,	CvPoint3D64f *pPt=NULL,int index=-1);
	void	DrawSelCircle(int Layer,CvPoint3D64f *pPt=NULL,int index=-1);
	void	DrawSelArc(int Layer,	CvPoint3D64f *pPt=NULL,int index=-1);
	void	DrawSelPolyline(int Layer,	CvPoint3D64f *pPt=NULL,int index=-1);
	void	DrawSelCrossLine(int Layer,	CvPoint3D64f *pPt=NULL,int index=-1);
	void	DrawSelPoint(int Layer,CvPoint3D64f *pPt=NULL);
	

	void	cvLine2(IplImage* pImageM,IplImage* pImageP,CvPoint pt1,CvPoint pt2,CvScalar color,int thickness);
	
	bool	GetCircle(CvPoint3D64f pt1,CvPoint3D64f pt2,CvPoint3D64f pt3,CvPoint3D64f& ptCircle);
	int		cvMouseDown(bool left,CvPoint pt);

	bool	CheckInBoundary(CvPoint3D64f point);

	bool	CheckPolylineDatas(CvPoint3D64f	*pPt, int DataSize);
	bool	CheckCircleData(CvPoint3D64f *pCenter, double *pRadius);
private:
	
	
	bool	CheckPointInLine(CvPoint3D64f pt1,DrawDataLine* pLine);
	bool	CheckPointInArc(CvPoint3D64f pt1,DrawDataArc* pArc);
	bool	CheckPointInCircle(CvPoint3D64f pt1,DrawDataCircle* pCircle);
	bool	CheckPointInPolyline(CvPoint3D64f pt1,DrawDataPolyline* pPLine);
	bool	CheckPointInRect(CvPoint3D64f pt1,DrawDataRect* pLine);
	bool	CheckPointInCrossLine(CvPoint3D64f pt1,DrawDataCrossLine* pLine);

	int		CheckLayer(CvPoint pt,DrawData** pData,DrawData* pDataDefault);
	bool	CheckInPos(CvPoint3D64f pt1,DrawData* pData);
	bool	CheckInPos(CvPoint3D64f pt1,CvPoint pt2);
	
	void	cvArc( CvArr* img, CvPoint center,CvPoint ptStart,CvPoint ptEnd,int dir, CvScalar color,int thickness=1, int line_type=8, int shift=0);
	void	cvArc2( CvArr* img, CvPoint center,CvPoint ptStart,CvPoint ptEnd,int dir, CvScalar color,int thickness=1, int line_type=8, int shift=0);
	void	GetNewCenter(CvPoint center,CvPoint ptStart,CvPoint ptEnd,double dblR,CvPoint2D64f &CenterOut);
	
public:
	std::map<int,DrawData*>	m_DrawDatas;	// 圖層資料
	bool					m_bMoveData;	// 是否可移動圖形
	double					m_dblDefaultZoom;

protected:
	DrawData*		m_pDrawData;			//目前操作的圖層
	CvFont			m_font;
	CvScalar		m_SelColor;
	IplImage		*m_pBkImage[2];			// 背景圖
	//IplImage		*m_pSourceImage[2];
	std::vector<IplImage*>	m_SourceImage;
	IplImage		*m_pDrawImage;			// 最終畫在 Display 上之圖(1:1)
	IplImage		*m_pMaskImage,*m_pPlotImage;
	CvRect			m_DrawRect;
	CvScalar		m_BackColor;
	CvPoint			m_Point,m_SelPoint;
	int				m_idSelPoint;
	double			m_ZoomRate;
	CvPoint3D64f	m_Offset;
	
	bool			m_bDrawText;			// 是否畫出座標
	bool			m_bDrawOrg;				// 是否畫出原點
	bool			m_bClickData;

	int				m_nXDir,m_nYDir;

	BYTE			m_BKFillColor;
};



	

/**************************************************************************/

class HDisplayMFC : public CStatic,public HDisplay
{
	DECLARE_DYNAMIC(HDisplayMFC)

public:
	HDisplayMFC();
	virtual ~HDisplayMFC();

	static CBitmap* IplImage2CBitmap(IplImage* pImg);
	static IplImage* TransCBitmap2IplImage(CBitmap* pBmp);

protected:
	DECLARE_MESSAGE_MAP()


protected:
	HANDLE			m_hDrawEvent,m_hMaskEvent,m_hDatasEvent;
	BITMAPINFO*		m_DibInfo;		// 標頭
	HCURSOR			m_cursor[2];


protected:
	virtual void	PreSubclassWindow();
	virtual bool	WaitDrawEvent();
	virtual void	SetDrawEvent();
	
	virtual bool	WaitMaskEvent();
	virtual void	SetMaskEvent();
	
	virtual bool	WaitDatasEvent();
	virtual void	SetDatasEvent();
	
	

public:
	virtual void	ChangeCursor(int id);
	virtual void	Init(CvRect rect,CvScalar color= cvScalar(0,0,0));
	virtual int		ChangeSelLayer(int nLayer);
	afx_msg void	OnPaint();
	
	void			ReDrawLayers();
	virtual void	OnViewDraw(CDC* pDC,CvRect rect=cvRect(0,0,0,0));

	afx_msg void	OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonDblClk(UINT nFlags, CPoint point);
	
	afx_msg void	OnRButtonDown(UINT nFlags, CPoint point);

	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL	OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};

#endif