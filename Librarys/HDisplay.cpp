// HDisplay.cpp : 實作檔
//

#include "stdafx.h"
#include "HDisplay.h"
#include <algorithm>



std::string converToMultiChar(const std::wstring& str)
{
	char* pElementText;
	int iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_ACP,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
	::WideCharToMultiByte(CP_ACP,
		0,
		str.c_str(),
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL);
	std::string strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}

std::wstring converToWideChar(const std::string& str)
{
	int len = (int)str.length();
	int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t * pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, (LPWSTR)pUnicode, unicodeLen);
	std::wstring rt = (wchar_t*)pUnicode;
	delete pUnicode;
	return rt;
}

#ifdef USE_OPENCV440

/****************************************************************************************************************/
HDisplay::HDisplay()
:m_pDrawImage(NULL),m_pDrawData(NULL),m_idSelPoint(-1),m_nXDir(1),m_nYDir(1),m_bDrawText(false),m_bMoveData(false)
,m_pPlotImage(NULL),m_pMaskImage(NULL),m_bClickData(true)
, m_dblDefaultZoom(1)
, m_BKFillColor(0)
{
	m_bDrawOrg=false;
	double dblSize=0.4;
	m_font=cvFont(1,1);
	cvInitFont(&m_font,CV_FONT_HERSHEY_SIMPLEX,dblSize,dblSize,0,1);
	m_pBkImage[0]=m_pBkImage[1]=NULL;
	m_Offset=cvPoint3D64f(0,0,0);
	m_ZoomRate=1;
	//m_BackColor=CV_RGB(0,0,0);
	m_BackColor=cvScalar(255,255,255);
	m_SelColor= cvScalar(0,0,255);
	m_SelPoint=cvPoint(0,0);
	
}

HDisplay::~HDisplay()
{

	WaitMaskEvent();
		if(m_pMaskImage!=NULL) cvReleaseImage(&m_pMaskImage);
		if(m_pPlotImage!=NULL) cvReleaseImage(&m_pPlotImage);
	SetMaskEvent();
	WaitDrawEvent();
		if(m_pDrawImage!=NULL) cvReleaseImage(&m_pDrawImage);
	SetDrawEvent();

		if(m_pBkImage[0]!=NULL) cvReleaseImage(&m_pBkImage[0]);
		if(m_pBkImage[1]!=NULL) cvReleaseImage(&m_pBkImage[1]);


	std::map<int,DrawData*>::iterator itMap;
	WaitDatasEvent();
		for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
			delete itMap->second;
		m_DrawDatas.clear();
	SetDatasEvent();


	for(UINT i=0;i<m_SourceImage.size();++i)
	{
		cvReleaseImage(&m_SourceImage[i]);
	}

}


bool	HDisplay::GetCircle(CvPoint3D64f pt1,CvPoint3D64f pt2,CvPoint3D64f pt3,CvPoint3D64f& ptCircle)
{
	// X^2 + Y^2 + aX + bY + c = 0
	// aX + bY + c = - X^2 - Y^2

	double Array1[]={pt1.x,pt1.y,1,	pt2.x,pt2.y,1,	pt3.x,pt3.y,1};
	double Array2[]={-1*pt1.x*pt1.x-pt1.y*pt1.y,-1*pt2.x*pt2.x-pt2.y*pt2.y,-1*pt3.x*pt3.x-pt3.y*pt3.y};
	double Array3[]={0,0,0};

	
	CvMat Matrix1=cvMat(3,3,CV_64FC1,Array1);
    CvMat Matrix2=cvMat(3,1,CV_64FC1,Array2);
    CvMat SolveSet=cvMat(3,1,CV_64FC1,Array3);

	try{
    //cvSetData(Matrix1,Array1,Matrix1->step);
    //cvSetData(Matrix2,Array2,Matrix2->step);

	
    cvSolve(&Matrix1,&Matrix2,&SolveSet,CV_LU);
	
	double a=Array3[0];
	double b=Array3[1];
	double c=Array3[2];

	ptCircle.x=-0.5*a;
	ptCircle.y=-0.5*b;
	ptCircle.z=pow(ptCircle.x,2)+pow(ptCircle.y,2)-c;
	if(ptCircle.z>0) 
		ptCircle.z=pow(ptCircle.z,0.5);
	else
		ptCircle.z=0;


	}catch(...)
	{
		return false;
	}
	return true;
	
}

CvPoint3D64f	HDisplay::Trans2MM(CvPoint ptIn)
{
	CvPoint3D64f pt;
	if(m_nXDir<0)
		pt.x=(float)(m_pDrawImage->width-ptIn.x);
	else
		pt.x=(float)ptIn.x;
	if(m_nYDir<0)
		pt.y=(float)(m_pDrawImage->height-ptIn.y);
	else
		pt.y=(float)ptIn.y;
	pt.x=(float)((pt.x+m_Offset.x)/m_ZoomRate);
	pt.y=(float)((pt.y+m_Offset.y)/m_ZoomRate);
	pt.z=0;
	return pt;
}

CvPoint	HDisplay::Trans2Pixel(CvPoint3D64f ptIn)
{
	CvPoint3D64f pt=Trans2Pixelf(ptIn);
	return cvPoint((int)(pt.x-0.5),(int)(pt.y-0.5));//cvPoint((int)(pt.x+0.5),(int)(pt.y+0.5));
}

CvPoint3D64f	HDisplay::Trans2Pixelf(CvPoint3D64f ptIn)
{
	CvPoint3D64f pt=cvPoint3D64f(ptIn.x*m_ZoomRate-m_Offset.x,ptIn.y*m_ZoomRate-m_Offset.y,0);
	if(m_nXDir<0)
		pt.x=m_pDrawImage->width-pt.x;
	if(m_nYDir<0)
		pt.y=m_pDrawImage->height-pt.y;
	return pt;
}

void HDisplay::TransAngle(CvPoint pt1,CvPoint pt2,bool bOutSide,CvPoint &ptC,double &Angle1,double &Angle2)
{
	CvPoint2D64f	base=cvPoint2D64f(1,0);
	CvPoint			dir=cvPoint(pt2.x-pt1.x,pt2.y-pt1.y);
	double dblSita0=(dir.x*base.x)/sqrt(pow((double)dir.x,2)+pow((double)dir.y,2));
	double dblSita=acos(dblSita0);
	double dblTan=tan(dblSita);
	if(dir.y<0)
		dblTan=dblTan *(-1);

	if(dblTan>=0) 
	{	
		if(dblSita0>=0)
		{
			ptC=cvPoint(pt1.x,pt2.y);
			Angle1=90;
			Angle2=0;
		}
		else
		{
			Angle1=180;
			Angle2=270;
			ptC=cvPoint(pt1.x,pt2.y);
		}
			
	}
	else
	{
		if(dblSita0>=0)
		{
			ptC=cvPoint(pt2.x,pt1.y);
			Angle1=180;
			Angle2=90;
		}
		else
		{
			ptC=cvPoint(pt2.x,pt1.y);
			Angle1=0;
			Angle2=-90;
		}
	}

	
}

void HDisplay::TransAngle(CvPoint3D64f pt,double &Base,double &Angle1,double &Angle2)
{
	double dblTemp[2];
	if(m_nXDir<0)
	{
		if(m_nYDir>=0)
		{
			dblTemp[0]=180+Angle1;
			dblTemp[1]=180+Angle2;
		}
		else
		{
			dblTemp[0]=360-Angle1;
			dblTemp[1]=360-Angle2;
		}
	}
	else
	{
		if(m_nYDir>=0)
		{
			dblTemp[1]=-1*Angle1;
			dblTemp[0]=360-1*Angle2;
		}
		else
		{
			dblTemp[0]=Angle1;
			dblTemp[1]=Angle2;
		}
	}
	
	// 轉成 0~360
	dblTemp[0]=dblTemp[0]+360;
	dblTemp[1]=dblTemp[1]+360;
	Base=0;
	for(int i=0;i<2;++i) 
	{
		if(dblTemp[i]>360)	dblTemp[i]=dblTemp[i]-360;
		if(dblTemp[i]>360)	dblTemp[i]=dblTemp[i]-360;
	}

	if(dblTemp[1]-dblTemp[0]<0)
	{
		//逆時針轉,來源點變目標點,並轉成負
		dblTemp[0]=dblTemp[0]-360;
	}

	Angle1=dblTemp[0];
	Angle2=dblTemp[1];
}

void HDisplay::Init(CvRect rect,CvScalar color)
{
	m_DrawRect=rect;


		cvReleaseImage(&m_pBkImage[0]);
		cvReleaseImage(&m_pBkImage[1]);


	m_BackColor=color;
}

int	HDisplay::cvOnMouseUp(CvPoint pt)
{
	OnLButtonUp(pt);
	m_idSelPoint=-1;
	ChangeCursor(0);
	return 0;
}

int	HDisplay::ChangeSelLayer(int nLayer)
{
	DrawData*		pData=NULL;
	
	// 將原線段顏色恢愎
	if(m_pDrawData!=NULL)
	{
		switch(GetDrawDataType())
		{
		case dtLine:
			DrawLine(GetDrawDataLayer());
			break;
		case dtRect:
			DrawRect(GetDrawDataLayer());
			break;
		case dtCircle:
			DrawCircle(GetDrawDataLayer());
			break;
		case dtArc:
			DrawArc(GetDrawDataLayer());
			break;
		case dtPolyline:
			DrawPolyline(GetDrawDataLayer());
			break;
		case dtCrossLine:
			DrawCrossLine(GetDrawDataLayer());
			break;
		case dtPoint:
			DrawPoint(GetDrawDataLayer());
			break;
		default:
			SetDrawData(NULL);
			return -1;
			break;
		}
	}

	if(nLayer<0)
	{
		// 按到線段外的空白處
		SetDrawData(NULL);
		m_Point=cvPoint(0,0);
		return 0;
	}
	
	// 將選中的線段顏色轉變
	pData=GetData(nLayer);
	if(pData==NULL) 
		return -10;

	switch(pData->nDataType)
	{
	case dtLine:		
		DrawSelLine(pData->nLayer);
		m_idSelPoint=-1;
		break;
	case dtRect:
		DrawSelRect(pData->nLayer);
		m_idSelPoint=-1;
		break;
	
	case dtCircle:
		DrawSelCircle(pData->nLayer);
		m_idSelPoint=-1;
		break;
	case dtArc:
		DrawSelArc(pData->nLayer);
		m_idSelPoint=-1;
		break;
	case dtPolyline:
		DrawSelPolyline(pData->nLayer);
		m_idSelPoint=-1;
		break;
	case dtCrossLine:		
		DrawSelLine(pData->nLayer);
		m_idSelPoint=-1;
		break;
	case dtPoint:
		DrawSelPoint(pData->nLayer);
		m_idSelPoint=-1;
		break;
	default:
		SetDrawData(NULL);
		m_Point=cvPoint(0,0);
		return -2;
		break;
	}
	SetDrawData(pData); 
	ChangeCursor(1);
	return 0;
}


void	HDisplay::SetDrawData(DrawData* pData)
{
	m_pDrawData=pData; 
}

int	HDisplay::GetDrawDataType()
{
	if(m_pDrawData==NULL)
		return -1;
	else
		return m_pDrawData->nDataType;
}

int	HDisplay::GetDrawDataLayer()
{
	if(m_pDrawData==NULL)
		return -1;
	else
		return m_pDrawData->nLayer;
}


int	HDisplay::cvOnLButtonDblClk(CvPoint pt)
{
	OnLButtonDblClk(pt);
	return 0;
}

int	HDisplay::cvOnRMouseDown(CvPoint pt)
{
	return cvMouseDown(false,pt);
}

int	HDisplay::cvOnMouseDown(CvPoint pt)
{
	return cvMouseDown(true,pt);
}

int		HDisplay::cvMouseDown(bool left,CvPoint pt)
{
	int				nLayer;
	DrawData*		pData=NULL;
	DrawDataLine*	pLine=NULL;
	DrawDataRect*	pRect=NULL;
	DrawDataCircle*	pCircle=NULL;
	DrawDataArc*	pArc=NULL;
	DrawDataPolyline*	pPLine=NULL;
	DrawDataCrossLine*	pCLine=NULL;
	PolylineData		*pPlData=NULL;

	std::map<UINT,PolylineData*>::iterator itPData;
	if(left)
		OnLButtonDown(pt);
	else
		OnRButtonDown(pt);
	
	if(!m_bClickData) return -10;

	nLayer=CheckLayer(pt,&pData,m_pDrawData);
	ChangeSelLayer(nLayer);
	// 將原線段顏色恢愎
	if(m_pDrawData!=NULL)
	{
		switch(GetDrawDataType())
		{
		case dtLine:
			DrawLine(GetDrawDataLayer());
			break;
		case dtRect:
			DrawRect(GetDrawDataLayer());
			break;
		case dtCircle:
			DrawCircle(GetDrawDataLayer());
			break;
		case dtArc:
			DrawArc(GetDrawDataLayer());
			break;
		case dtPolyline:
			DrawPolyline(GetDrawDataLayer());
			break;
		case dtCrossLine:
			DrawCrossLine(GetDrawDataLayer());
			break;
		default:
			return -1;
			break;
		}
	}

	if(nLayer<0)
	{
		// 按到線段外的空白處
		SetDrawData(NULL);
		m_Point=cvPoint(0,0);
		return 0;
	}
	
	// 將選中的線段顏色轉變
	switch(pData->nDataType)
	{
	case dtLine:
		if(pData!=m_pDrawData)
			SetDrawData(pData);
		DrawSelLine(GetDrawDataLayer());
		pLine=(DrawDataLine*)m_pDrawData;
		
		if(CheckInPos(pLine->m_pt[0],pt)) 
		{
			m_idSelPoint=0;
			m_SelPoint=pt;
		}
		else if(CheckInPos(pLine->m_pt[1],pt))
		{
			m_idSelPoint=1;
			m_SelPoint=pt;
		}
		else
		{
			m_idSelPoint=-1;
		}
		break;
	case dtRect:
		if(pData!=m_pDrawData)
			SetDrawData(pData);
		DrawSelRect(pData->nLayer);
		pRect=(DrawDataRect*)m_pDrawData;
		if(CheckInPos(pRect->m_pt[0],pt))
		{
			m_idSelPoint=0;
			m_SelPoint=pt;
		}
		else if(CheckInPos(pRect->m_pt[1],pt))
		{
			m_idSelPoint=1;
			m_SelPoint=pt;
		}
		else if(CheckInPos(pRect->m_pt[2],pt))
		{
			m_idSelPoint=2;
			m_SelPoint=pt;
		}
		else if(CheckInPos(pRect->m_pt[3],pt))
		{
			m_idSelPoint=3;
			m_SelPoint=pt;
		}
		else
			m_idSelPoint=-1;
		break;
	
	case dtCircle:
		if(pData!=m_pDrawData)
			SetDrawData(pData);
		DrawSelCircle(pData->nLayer);
		pCircle=(DrawDataCircle*)m_pDrawData;
		if(!CheckInPos(pCircle->m_center,pt))
		{
			m_idSelPoint=0;
			m_SelPoint=pt;
		}
		else
			m_idSelPoint=-1;
		break;
	case dtArc:
		if(pData!=m_pDrawData)
			SetDrawData(pData);
		DrawSelArc(pData->nLayer);
		pArc=(DrawDataArc*)m_pDrawData;
		if(CheckInPos(pArc->m_center,pt))
		{
			// 整弧移動
			m_idSelPoint=-1;
		}
		else if(CheckInPos(pArc->m_pt[0],pt))
		{
			m_idSelPoint=0;
			m_SelPoint=pt;
		}
		else if(CheckInPos(pArc->m_pt[1],pt))
		{
			m_idSelPoint=1;
			m_SelPoint=pt;
		}
		else
		{
			// 改變弧半徑
			m_idSelPoint=255;
			m_SelPoint=pt;
		}
		break;
	case dtPolyline:
		if(pData!=m_pDrawData)
			SetDrawData(pData);
		DrawSelPolyline(GetDrawDataLayer());
		pPLine=(DrawDataPolyline*)m_pDrawData;
		if(pPLine->flags==1)
		{
			//for(int i=0;i<(int)pPLine->m_Pts.size();++i)
			for(itPData=pPLine->m_PDatas.begin();itPData!=pPLine->m_PDatas.end();itPData++)
			{
				pPlData=pPLine->GetPoint(itPData->first);
				if(pPlData!=NULL)
				{
					if(CheckInPos(pPlData->point,pt))
					{
						m_idSelPoint=itPData->first;
						m_SelPoint=pt;
						break;
					}
				}
			}
		}
		else
		{
			((DrawDataPolyline*)m_pDrawData)->m_SelPoint=cvPoint3D64f(pt.x,pt.y,0);
			m_idSelPoint=-1;
		}
		break;

	case dtCrossLine:
		if(pData!=m_pDrawData)
			SetDrawData(pData);
		DrawSelLine(GetDrawDataLayer());
		pCLine=(DrawDataCrossLine*)m_pDrawData;
		
		if(CheckInPos(pCLine->m_pt,pt)) 
		{
			m_idSelPoint=0;
			m_SelPoint=pt;
		}
		else
			m_idSelPoint=-1;
		break;
	default:
		SetDrawData(NULL);
		m_Point=cvPoint(0,0);
		return -2;
		break;
	}
	
	m_Point=pt;
	ChangeCursor(1);
	return 0;
	
}

int	HDisplay::cvOnMouseMove(CvPoint pt,bool bMouseDown)
{
	DrawDataLine		*pLine;
	DrawDataRect		*pRect;
	DrawDataCircle		*pCircle;
	DrawDataArc			*pArc;
	DrawDataCrossLine	*pCLine;
	PolylineData		*pPlData=NULL;
	DrawDataPolyline	*pPLine=NULL;
	CvPoint3D64f		point[2];
	

	OnMouseMove(pt,bMouseDown);

	if(m_pDrawData==NULL) return -1;
	if(!bMouseDown) return -2;
	if(!m_bMoveData) return -4;

	switch(m_pDrawData->nDataType)
	{
	case dtLine:
		pLine=(DrawDataLine*)m_pDrawData;
		if(m_idSelPoint>=0 && m_idSelPoint<=1)
		{
			// 確定端點拖曳點
			point[0]=TransPoint(pLine->m_pt[m_idSelPoint],cvPoint3D64f(pt.x,pt.y,0));
			DrawSelLine(GetDrawDataLayer(),point,m_idSelPoint);
		}
		else
		{
			// 移動整個線段
			point[0]=TransPoint(pLine->m_pt[0],cvPoint3D64f(pt.x,pt.y,0));
			point[1]=TransPoint(pLine->m_pt[1],cvPoint3D64f(pt.x,pt.y,0));
			DrawSelLine(GetDrawDataLayer(),point);
		}
		ChangeCursor(1);
		m_SelPoint=pt;
		m_Point=pt;
		return 0;
		break;
	case dtRect:
		pRect=(DrawDataRect*)m_pDrawData;
		if(m_idSelPoint>=0 && m_idSelPoint<=3)
		{
			point[0]=TransPoint(pRect->m_pt[m_idSelPoint],cvPoint3D64f(pt.x,pt.y,0));
			DrawSelRect(GetDrawDataLayer(),point,m_idSelPoint);
		}
		else
		{
			// 移動整個矩形
			point[0]=TransPoint(pRect->m_pt[0],cvPoint3D64f(pt.x,pt.y,0));
			point[1]=TransPoint(pRect->m_pt[1],cvPoint3D64f(pt.x,pt.y,0));
			DrawSelRect(GetDrawDataLayer(),point);
		}
		ChangeCursor(1);
		m_SelPoint=pt;
		m_Point=pt;
		return 0;
		break;
	case dtCircle:
		pCircle=(DrawDataCircle*)m_pDrawData;
		// 確定圖心拖曳點
		if(m_idSelPoint<0)
		{
			point[0]=Trans2MM(pt);
			DrawSelCircle(GetDrawDataLayer(),&point[0]);			// 移動整個圓形
		}
		else
		{
			point[0]=Trans2MM(pt);
			DrawSelCircle(GetDrawDataLayer(),&point[0],m_idSelPoint);
		}
		ChangeCursor(1);
		m_SelPoint=pt;
		m_Point=pt;
		return 0;
		break;
	case dtArc:
		pArc=(DrawDataArc*)m_pDrawData;
		// 確定圖心拖曳點
		if(m_idSelPoint<0)
		{
			point[0]=TransPoint(pArc->m_center,cvPoint3D64f(pt.x,pt.y,0));
			DrawSelArc(GetDrawDataLayer(),&point[0]);			// 移動整個圓形
		}
		else if(m_idSelPoint>=0 && m_idSelPoint<=2)
		{
			point[0]=TransPoint(pArc->m_pt[m_idSelPoint],cvPoint3D64f(pt.x,pt.y,0));
			DrawSelArc(GetDrawDataLayer(),&point[0],m_idSelPoint);
		}
		else
			DrawSelArc(GetDrawDataLayer(),&cvPoint3D64f(pt.x,pt.y,0),m_idSelPoint);
		ChangeCursor(1);
		m_SelPoint=pt;
		m_Point=pt;
		return 0;
		break;
	case dtPolyline:
		pPLine=(DrawDataPolyline*)m_pDrawData;
		if(m_idSelPoint>=0 && m_idSelPoint<=(int)pPLine->GetSize())
		{
			// 確定端點拖曳點
			pPlData=pPLine->GetPoint(m_idSelPoint);
			if(pPlData!=NULL)				
			{
				point[0]=TransPoint(pPlData->point,cvPoint3D64f(pt.x,pt.y,0));
				DrawSelPolyline(GetDrawDataLayer(),point,m_idSelPoint);
			}
		}
		else
		{
			// 移動整個線段
			point[0]=TransPoint(pPLine->m_SelPoint,cvPoint3D64f(pt.x,pt.y,0));
			point[1]=cvPoint3D64f(point[0].x-pPLine->m_SelPoint.x,point[0].y-pPLine->m_SelPoint.y,0);
			DrawSelPolyline(GetDrawDataLayer(),&point[1]);
			pPLine->m_SelPoint=cvPoint3D64f(point[0].x,point[0].y,0);
		}
		ChangeCursor(1);
		m_SelPoint=pt;
		m_Point=pt;
		return 0;
		break;
	case dtCrossLine:
		pCLine=(DrawDataCrossLine*)m_pDrawData;
		point[0]=TransPoint(pCLine->m_pt,cvPoint3D64f(pt.x,pt.y,0));
		DrawSelCrossLine(GetDrawDataLayer(),point,m_idSelPoint);
		ChangeCursor(1);
		m_SelPoint=pt;
		m_Point=pt;
		return 0;
		break;
	}
	return -3;
}



bool	HDisplay::CheckInPos(CvPoint3D64f pt1,CvPoint pt2)
{
	CvPoint3D64f pt=Trans2Pixelf(cvPoint3D64f(pt1.x,pt1.y,0));
	double dblPitch=pow(pt.x-pt2.x,2) + pow(pt.y-pt2.y,2);
	double dblV=pow((double)MINRANGE,2);
	return (dblPitch <= dblV);
}

CvPoint3D64f	HDisplay::TransPoint(CvPoint3D64f pt,CvPoint3D64f ptNow)
{
	CvPoint3D64f offset=cvPoint3D64f((ptNow.x-m_Point.x)/m_ZoomRate,(ptNow.y-m_Point.y)/m_ZoomRate,0);
	//CvPoint3D64f offset=cvPoint3D64f(ptNow.x-m_Point.x,ptNow.y-m_Point.y,0); 
	offset.x = m_nXDir * offset.x;
	offset.y = m_nYDir * offset.y;
	return cvPoint3D64f((pt.x+offset.x),(pt.y+offset.y),0);
	
}

bool	HDisplay::CheckPointInRect(CvPoint3D64f pt1,DrawDataRect* pRect)
{
	CvPoint			ptKeyDown=Trans2Pixel(pt1);
	DrawDataLine	line[4];

	line[0].SetPoint(pRect->m_pt[0],pRect->m_pt[3]);
	line[1].SetPoint(pRect->m_pt[3],pRect->m_pt[1]);
	line[2].SetPoint(pRect->m_pt[1],pRect->m_pt[2]);
	line[3].SetPoint(pRect->m_pt[0],pRect->m_pt[2]);

	for(int i=0;i<4;++i)
	{
		if(CheckPointInLine(pt1,&line[i])) return true;
	}
	return false;
}

bool	HDisplay::CheckPointInCrossLine(CvPoint3D64f pt1,DrawDataCrossLine* pLine)
{
	CvPoint			ptKeyDown=Trans2Pixel(pt1);
	CvPoint			pt=Trans2Pixel(pLine->m_pt);
	CvPoint2D64f	pt64=cvPoint2D64f(ptKeyDown.x-pt.x,ptKeyDown.y-pt.y);
	double	dblMin=pow(pt64.x,2) + pow(pt64.y,2);
	return dblMin < MINRANGE;
}

bool	HDisplay::CheckPointInLine(CvPoint3D64f pt1,DrawDataLine* pLine)
{
	CvPoint			point[2];
	double			dblTemp;
	CvPoint			ptKeyDown=Trans2Pixel(pt1);
	CvPoint			pt2=Trans2Pixel(cvPoint3D64f(pLine->m_pt[0].x,pLine->m_pt[0].y,0));
	CvPoint			pt3=Trans2Pixel(cvPoint3D64f(pLine->m_pt[1].x,pLine->m_pt[1].y,0));	

	//垂直線
	if(abs(pt2.x-pt3.x)<=0.1)
	{
		if(pt2.y<=pt3.y)
		{
			point[0]=pt2;
			point[1]=pt3;
		}
		else
		{
			point[0]=pt3;
			point[1]=pt2;
		}
		if(ptKeyDown.y<(point[0].y-MINRANGE) || ptKeyDown.y>(point[1].y+MINRANGE))
			return false;
		if(abs(ptKeyDown.x-pt2.x)<MINRANGE)
			return true;
		else
			return false;
	}

	//水平線
	if(abs(pt2.y-pt3.y)<=0.1)
	{
		if(pt2.x<=pt3.x)
		{
			point[0]=pt2;
			point[1]=pt3;
		}
		else
		{
			point[0]=pt3;
			point[1]=pt2;
		}
		if(ptKeyDown.x<(point[0].x-MINRANGE) || ptKeyDown.x>(point[1].x+MINRANGE))
			return false;
		if(abs(ptKeyDown.y-pt2.y)<MINRANGE)
			return true;
		else
			return false;
	}
	
	// 其它
	double dblX,dblY,dblXEnd,dblM,dblMin=9999;
	dblM=((double)(pt3.y-pt2.y))/(pt3.x-pt2.x);
	if(pt2.x<pt3.x)
	{
		dblX=pt2.x;
		dblXEnd=pt3.x;
	}
	else
	{
		dblX=pt3.x;
		dblXEnd=pt2.x;
	}
	while(dblX<dblXEnd)
	{
		dblY=dblM*dblX + pt2.y - dblM*pt2.x;
		dblTemp=pow(ptKeyDown.x-dblX,2) + pow(ptKeyDown.y-dblY,2);
		if(dblTemp<dblMin) dblMin=dblTemp;
		dblX=dblX+0.1;
	}
	return dblMin < MINRANGE;
}

bool	HDisplay::CheckPointInPolyline(CvPoint3D64f pt1,DrawDataPolyline* pPLine)
{
	double dblPitch;
	
	//std::map<UINT,CvPoint3D64f>::iterator itPt,itPt2;
	std::map<UINT,PolylineData*>::iterator itPData;
	//std::vector<CvPoint3D64f>::iterator itPt;
	//for(itPt=pPLine->m_Pts.begin();itPt!=pPLine->m_Pts.end();itPt++)
	for(itPData=pPLine->m_PDatas.begin();itPData!=pPLine->m_PDatas.end();itPData++)
	{
		dblPitch=sqrt(pow(pt1.x-itPData->second->point.x,2)+pow(pt1.y-itPData->second->point.y,2));
		if(dblPitch<MINRANGE)
			return true;
	}
	return false;
}

bool	HDisplay::CheckPointInCircle(CvPoint3D64f pt1,DrawDataCircle* pCircle)
{
	double	dblPos=sqrt(pow(pt1.x-pCircle->m_center.x,2)+pow(pt1.y-pCircle->m_center.y,2));
	double	dblMin=pow(dblPos-pCircle->m_Radius,2);
	return  (dblMin < MINRANGE) || (dblPos<MINRANGE);
}



DrawData*	HDisplay::GetDrawData(int nLayer)
{
	DrawData* pData=NULL;
	return GetData(nLayer);
}

DrawData*	HDisplay::GetHightlightData()
{
	DrawData* pData=NULL;
	std::map<int,DrawData*>::iterator	itMap;
	WaitDatasEvent();
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		if(itMap->second->IsHilight()) 
		{
			pData=itMap->second;
			break;
		}
	}
	SetDatasEvent();
	return pData;
}

IplImage*	HDisplay::GetSourceImage(int nID)
{
	int size=(int)m_SourceImage.size();
	if(nID<0 || nID>=size) return NULL;
	return m_SourceImage[size-nID-1];
}

IplImage*	HDisplay::GetDrawImage()
{
	IplImage* pImage;
	if(m_pDrawImage!=NULL && WaitDrawEvent())
	{
		pImage=cvCloneImage(m_pDrawImage);
		SetDrawEvent();
		return pImage;
	}
	return NULL;
}

bool	HDisplay::GetLayers(std::vector<int>* pLayers)
{
	std::map<int,DrawData*>::iterator	itMap;
	if(m_DrawDatas.size()<=0 || pLayers->size()>0) return false;
	WaitDatasEvent();
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
		pLayers->push_back(itMap->second->nLayer);
	SetDatasEvent();
	return true;
}

bool	HDisplay::GetIndexs(std::vector<int>* pLayers)
{
	DrawData	*pData;
	std::map<int,DrawData*>::iterator	itMap;
	if(m_DrawDatas.size()<=0 || pLayers->size()>0) return false;
	WaitDatasEvent(); 
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		pData=itMap->second;
		if(pData->nDataType<=DrawLib::dtPolyline)
			pLayers->push_back(pData->nIndex);
	}
	std::sort(pLayers->begin(),pLayers->end());
	SetDatasEvent();
	return true;
}	

bool	HDisplay::CheckPointInArc(CvPoint3D64f pt1,DrawDataArc* pArc)
{
	CvPoint2D64f	ptMin;
	
	double			dblX,dblY,dblMin=999,dblValue;
	double			dblAngle[2];
	int				nDir=1;
	bool			bFirst=true,bOneCycle=true;
	
	dblValue=pow(pArc->m_center.x-pt1.x,2)+pow(pArc->m_center.y-pt1.y,2);
	if(dblValue<MINRANGE) return true;

	dblAngle[0]=pArc->m_Angle[0]*PI/180;
	dblAngle[1]=pArc->m_Angle[1]*PI/180;
	
	if((pArc->m_pt[0].x * pArc->m_pt[1].x > 0) && (pArc->m_pt[1].y * pArc->m_pt[1].y > 0))
	{
		//第1,2點同一象限
		if(dblAngle[0]>dblAngle[1]) 
		{
			//dblValue=dblAngle[0];
			//dblAngle[0]=dblAngle[1];
			//dblAngle[1]=dblValue;
			nDir=-1;
		}
	}

	while(true)
	{
		dblX=pArc->m_Radius * cos(dblAngle[0]) + pArc->m_center.x;
		dblY=pArc->m_Radius * sin(dblAngle[0]) + pArc->m_center.y;
		if(bFirst)
		{
			ptMin=cvPoint2D64f(dblX,dblY);
			dblMin=pow(pt1.x-dblX,2)+pow(pt1.y-dblY,2);
			bFirst=false;
		}
		else
		{
			dblValue=pow(pt1.x-dblX,2)+pow(pt1.y-dblY,2);
			if(dblValue<dblMin)
			{
				dblMin=dblValue;
				ptMin=cvPoint2D64f(dblX,dblY);
			}
		}
		if(nDir>0)
		{
			if(dblAngle[0]>=dblAngle[1]) break;
			dblAngle[0]=dblAngle[0]+PI/180;
		}
		else
		{
			if(bOneCycle)
			{
				dblAngle[0]=dblAngle[0]+PI/180;
				if(dblAngle[0]>=(2*PI))
				{
					dblAngle[0]=0;
					bOneCycle=false;
				}
			}
			else
			{
				if(dblAngle[0]>=dblAngle[1]) break;
				dblAngle[0]=dblAngle[0]+PI/180;
			}
		}
	}
	return dblMin < MINRANGE;
}

bool HDisplay::CheckInPos(CvPoint3D64f pt1,DrawData* pData)
{
	switch(pData->nDataType)
	{
	case dtLine:
		return CheckPointInLine(pt1,(DrawDataLine*)pData);
		break;
	case dtArc:
		return CheckPointInArc(pt1,(DrawDataArc*) pData);
		break;
	case dtCircle:
		return CheckPointInCircle(pt1,(DrawDataArc*) pData);
		break;
	case dtPolyline:
		return CheckPointInPolyline(pt1,(DrawDataPolyline*) pData);
		break;
	case dtRect:
		return CheckPointInRect(pt1,(DrawDataRect*) pData);
		break;
	case dtCrossLine:
		return CheckPointInCrossLine(pt1,(DrawDataCrossLine*) pData);
		break;
	}
	return false;
}
int		HDisplay::CheckLayer(CvPoint pt,DrawData** pData,DrawData* pDataDefault)
{
	DrawData* pDataOut=NULL;
	std::map<int,DrawData*>::iterator	itMap;
	CvPoint3D64f point =Trans2MM(pt);
	if(pDataDefault!=NULL && CheckInPos(point,pDataDefault))
	{
		*pData=pDataDefault;
		return pDataDefault->nLayer;
	}
	WaitDatasEvent();
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		if(CheckInPos(point,itMap->second))
		{
			pDataOut=itMap->second;
			break;
		}
	}
	if(pDataOut!=NULL)
	{
		*pData=pDataOut;
		SetDatasEvent();
		return pDataOut->nLayer;
	}
	else
	{
		SetDatasEvent();
		return -1;
	}
}

int HDisplay::ChangeLayer(int Layer1,int Layer2)
{
	int			layer[2];
	DrawData*	pData[2];
	std::map<int,DrawData*>::iterator itMap[2];
	if(Layer1==Layer2) return -1;
	layer[0]=Layer1;
	layer[1]=Layer2;
	WaitDatasEvent();
	for(int i=0;i<2;++i)
	{
		for(itMap[i]=m_DrawDatas.begin();itMap[i]!=m_DrawDatas.end();++itMap[i])
		{
			if(itMap[i]->second->nLayer==layer[i])
				break;
		}
	}
	if(itMap[0]!=m_DrawDatas.end() && itMap[1]!=m_DrawDatas.end())
	{
		pData[0]=itMap[0]->second;
		pData[1]=itMap[1]->second;
		m_DrawDatas.erase(itMap[0]);
		m_DrawDatas.erase(itMap[1]);
		pData[0]->nLayer=Layer2;
		pData[1]->nLayer=Layer1;
		m_DrawDatas.insert(std::make_pair(pData[0]->nLayer,pData[0]));
		m_DrawDatas.insert(std::make_pair(pData[1]->nLayer,pData[1]));
		SetDatasEvent();
		return 0;
	}
	SetDatasEvent();
	return -2;
}

DrawData*	HDisplay::GetDrawData(wstring strLayer,std::vector<DrawData*> &datas)
{
	DrawData* pData=NULL;
	WaitDatasEvent();
	std::map<int,DrawData*>::iterator	itMap;
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		if(itMap->second->strLayer==strLayer)
		{
			pData=itMap->second;
			datas.push_back(pData);
		}
	}
	SetDatasEvent();
	if(datas.size()<=0) return NULL;
	return datas[0];
}


DrawData*	HDisplay::GetDrawData2(int nIndex)
{
	DrawData* pData=NULL;
	WaitDatasEvent();
	std::map<int,DrawData*>::iterator	itMap;
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		if(itMap->second->nIndex==nIndex)
		{
			pData=itMap->second;
			break;
		}
	}
	SetDatasEvent();
	return pData;
}

DrawData*		HDisplay::GetData(int Layer)
{
	std::map<int,DrawData*>::iterator	itMap;
	WaitDatasEvent();
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		if(itMap->second->nLayer==Layer)
		{
			SetDatasEvent();
			return itMap->second;
		}
	}
	SetDatasEvent();
	return NULL;
}

DrawData*		HDisplay::GetDataByIndex(int Index)
{
	std::map<int,DrawData*>::iterator	itMap;
	WaitDatasEvent();
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		if(itMap->second->nIndex==Index)
		{
			SetDatasEvent();
			return itMap->second;
		}
	}
	SetDatasEvent();
	return NULL;
}

void HDisplay::ClearLayer(int Layer,bool bClearOffset)
{
	DrawData *pData=NULL;
	std::map<int,DrawData*>::iterator itMap;
	if(Layer<0)
	{
		WaitDatasEvent();
		for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
			delete itMap->second;
		m_DrawDatas.clear();
		SetDatasEvent();
		if(bClearOffset) m_Offset=cvPoint3D64f(0,0,0);
		SetDrawData(NULL);
		return;
	}
	WaitDatasEvent();
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		if(itMap->second->nLayer==Layer)
		{
			pData=itMap->second;
			if(m_pDrawData!=NULL && pData->nLayer==m_pDrawData->nLayer)
				SetDrawData(NULL);
			delete pData;
			m_DrawDatas.erase(itMap);
			break;
		}
	}
	if(bClearOffset) m_Offset=cvPoint3D64f(0,0,0);
	SetDatasEvent();
}

DrawData *HDisplay::CreateNewLayer(int type,int Layer,int index,CvPoint3D64f offset)
{
	DrawData *pData;
	std::map<int,DrawData*>::iterator itMap;

	WaitDatasEvent();
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		if(itMap->second->nLayer==Layer)
		{
			pData=itMap->second;
			delete pData;
			pData=NULL;
			m_DrawDatas.erase(itMap);
			break;
		}
	}
	
	switch(type)
	{
	case DrawLib::dtPoint:
		pData=new DrawDataPoint();
		break;
	case DrawLib::dtLine:
		pData=new DrawDataLine();
		break;
	case DrawLib::dtRect:
		pData=new DrawDataRect();
		break;
	case DrawLib::dtCircle:
		pData=new DrawDataCircle();
		break;
	case DrawLib::dtCircles:
		pData=new DrawDataCircles();
		break;
	case DrawLib::dtArc:
		pData=new DrawDataArc();
		break;
	case DrawLib::dtPolyline:
		pData=new DrawDataPolyline();
		break;
	case DrawLib::dtCrossLine:
		pData=new DrawDataCrossLine();
		break;
	case DrawLib::dtText:
		pData=new DrawDataText();
		break;
	case DrawLib::dtImage:
		pData=new DrawDataImage();
		break;

	case DrawLib::dtLines:
		pData=new DrawDataLines();
		break;
	case DrawLib::dtTexts:
		pData=new DrawDataTexts();
		break;
	case DrawLib::dtRects:
		pData=new DrawDataRects();
		break;
	case DrawLib::dtArcs:
		pData=new DrawDataArcs();
		break;
	case DrawLib::dtPoints:
		pData=new DrawDataPoints();
		break;

	default:
		SetDatasEvent();
		return NULL;
		break;
	}
	pData->nLayer=Layer;
	pData->nDataType=type;
	pData->nIndex=index;
	pData->offset=offset;
	m_DrawDatas.insert(std::make_pair(Layer,pData));
	SetDatasEvent();
	return pData;
}

void HDisplay::ResetZoomOffset()
{
	if(m_pBkImage[0]!=NULL)
		m_pBkImage[1]=cvCloneImage(m_pBkImage[0]);
	m_ZoomRate=1;
	m_Offset=cvPoint3D64f(0,0,0);
}

void HDisplay::SaveDisplay(wstring strFile)
{
	if(m_pDrawImage==NULL) return;
	WaitDrawEvent();
#ifdef _UNICODE
	cv::Mat imgSave = cv::cvarrToMat(m_pDrawImage);
	cv::imwrite(::converToMultiChar(strFile).c_str(), imgSave);
	imgSave.release();
	//cvSaveImage(::converToMultiChar(strFile).c_str(),m_pDrawImage);
#else
	cvSaveImage(strFile.c_str(),m_pDrawImage);
#endif
	SetDrawEvent();
}

void HDisplay::SaveImage(wstring strFile)
{
	int nSize = (int)m_SourceImage.size();
	if(nSize <=0) return;
#ifdef _UNICODE
	cv::Mat imgSave = cv::cvarrToMat(m_SourceImage[nSize-1]);
	cv::imwrite(::converToMultiChar(strFile).c_str(), imgSave);
	imgSave.release();
	//cvSaveImage(::converToMultiChar(strFile).c_str(),m_SourceImage[0]);
#else
	cvSaveImage(strFile.c_str(),m_SourceImage[0]);
#endif
}

void	HDisplay::ReDrawLayers()
{
	bool		bFlag=false,bFirst=true;
	CvMat		ROIMatrix;
	CvPoint		offset,diff;
	IplImage	ImageTemp;
	DrawData	*pData=NULL;
	int			nW=0,nH=0;
	double		dblW,dblH;
	if(m_pDrawImage==NULL) return;
	
	IplImage* pImage=cvCreateImage(cvSize(m_pDrawImage->width,m_pDrawImage->height),8,3);
	IplImage* pImageOut=cvCreateImage(cvSize(m_pDrawImage->width,m_pDrawImage->height),8,3);
	
	// 背景圖
	if(m_pBkImage[0]==NULL || m_pBkImage[1]==NULL) 	
		cvRectangle(pImage,cvPoint(0,0),cvPoint(m_pDrawImage->width,m_pDrawImage->height),m_BackColor,CV_FILLED);
	else
	{
		dblW=m_pBkImage[1]->width;
		dblH=m_pBkImage[1]->height;
		offset=cvPoint((int)m_Offset.x,(int)m_Offset.y);
		memset(pImage->imageData, m_BKFillColor,pImage->imageSize);
		if(pImage->height<dblH)
		{
			if(pImage->width<dblW)
			{
				// 圖	> DISPLAY
				nH=pImage->height;
				nW=pImage->width;
				if((nW+offset.x)<dblW && ((nH+offset.y)<dblH))
					cvGetSubRect(m_pBkImage[1],&ROIMatrix,cvRect(offset.x,offset.y,nW,nH));
				else
					bFlag=true;
			}
			else
			{
				// 圖高	< DISPLAY高
				nH=pImage->height;
				nW=(int)dblW;
				if((nW+offset.x)<dblW && ((nH+offset.y)<dblH))
					cvGetSubRect(m_pBkImage[1],&ROIMatrix,cvRect(0,offset.y,nW,nH));
				else
					bFlag=true;
			}
		}
		else
		{
			if(pImage->width<dblW)
			{
				// 圖寬 > DISPLAY寬
				nH=(int)dblH;
				nW=pImage->width;
				if((nW+offset.x)<dblW && ((nH+offset.y)<dblH))
					cvGetSubRect(m_pBkImage[1],&ROIMatrix,cvRect(offset.x,0,nW,nH));
				else
					bFlag=true;
			}
			else
			{
				// 圖 < DISPLAY
				nH=(int)dblH;
				nW=(int)dblW;
				if((nW+offset.x)<=dblW && ((nH+offset.y)<=dblH))
					cvGetSubRect(m_pBkImage[1],&ROIMatrix,cvRect(0,0,nW,nH));
				else
					bFlag=true;
			}
		}
		if(bFlag)
		{
			diff.x=nW+offset.x-(int)dblW;
			diff.y=nH+offset.y-(int)dblH;
			if(diff.x>=0 && diff.y>=0)
				cvGetSubRect(m_pBkImage[1],&ROIMatrix,cvRect(offset.x-diff.x,	offset.y-diff.y,	nW,nH));
			else if(diff.x>=0 && diff.y<0)
				cvGetSubRect(m_pBkImage[1],&ROIMatrix,cvRect(offset.x-diff.x,	offset.y,			nW,nH));
			else if(diff.x<0 && diff.y>=0)
				cvGetSubRect(m_pBkImage[1],&ROIMatrix,cvRect(offset.x,			offset.y-diff.y,	nW,nH));
			else if(diff.x<0 && diff.y<0)
				cvGetSubRect(m_pBkImage[1],&ROIMatrix,cvRect(offset.x,			offset.y,			nW,nH));
		}
		
		cvGetImage(&ROIMatrix,&ImageTemp);
		if(nH==pImage->height && nW==pImage->width)
		{
			for(int i=0;i<nH;++i)
				memcpy(&pImage->imageData[i*pImage->widthStep],&ImageTemp.imageData[i*ImageTemp.widthStep],pImage->widthStep);
		}
		else
		{
			for(int i=0;i<nH;++i)
			{
				for(int j=0;j<m_pBkImage[1]->widthStep;++j)
				{
					pImage->imageData[i*pImage->widthStep + j] = 
						ImageTemp.imageData[i*ImageTemp.widthStep +j];
				}
			}
		}
	}
	

	// 線段重繪
	bFirst=true;
	std::map<int,DrawData*>::iterator itMap;
	WaitDatasEvent();
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		pData=itMap->second;
		// 先非HILIGHT部分
			if(!pData->IsHilight())
		{
			if(bFirst)
			{
				WaitMaskEvent();
					memset(m_pMaskImage->imageData,255,	m_pMaskImage->imageSize);
					memset(m_pPlotImage->imageData,0,	m_pPlotImage->imageSize);
				SetMaskEvent();
				DrawOtherDatas();
				bFirst=false;
			}
			if(DrawDatas(pData->nLayer)!=NULL)
			{
				WaitMaskEvent();
					cvAnd(pImage,	m_pMaskImage,	pImageOut);
					cvOr(pImageOut,	m_pPlotImage,	pImage);
				SetMaskEvent();
			}
		}
	}
	
	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		pData=itMap->second;
		if(pData->IsHilight())
		{
			if(m_DrawDatas.size()==1)
			{
				WaitMaskEvent();
				memset(m_pMaskImage->imageData,255,	m_pMaskImage->imageSize);
				memset(m_pPlotImage->imageData,0,	m_pPlotImage->imageSize);
				SetMaskEvent();
			}
			if(DrawDatas(pData->nLayer)!=NULL)
			{
				WaitMaskEvent();
				cvAnd(pImage,	m_pMaskImage,	pImageOut);
				cvOr(pImageOut,	m_pPlotImage,	pImage);
				SetMaskEvent();
			}
		}
	}
	SetDatasEvent();

	WaitDrawEvent();
		cvReleaseImage(&m_pDrawImage);
		m_pDrawImage=cvCloneImage(pImage);
	SetDrawEvent();

	cvReleaseImage(&pImage);
	cvReleaseImage(&pImageOut);

}

void HDisplay::Offset(double dblX,double dblY)	// pixel
{
	CvPoint2D64f point=cvPoint2D64f(m_Offset.x + dblX,m_Offset.y + dblY);
	if(point.x<0) point.x=0;
	if(point.y<0) point.y=0;

		if((point.x + m_pDrawImage->width)>m_pBkImage[1]->width)
			point.x=m_Offset.x;
		if((point.y + m_pDrawImage->height)>m_pBkImage[1]->height)
			point.y=m_Offset.y;
		m_Offset=cvPoint3D64f(point.x,point.y,0);
	

}

void HDisplay::Zoom(double dblRate)
{
	double		dblTemp=m_ZoomRate * dblRate;

	if(m_pBkImage[0]!=NULL && dblRate>0 && dblTemp>=0.1 && dblTemp<20)
	{
		m_ZoomRate=dblTemp;
		m_Offset=cvPoint3D64f(0,0,0);//m_Offset.x*dblRate,m_Offset.y*dblRate,m_Offset.z*dblRate);

		cvReleaseImage(&m_pBkImage[1]);

		m_pBkImage[1]=cvCreateImage(cvSize((int)(m_ZoomRate * m_pBkImage[0]->width),(int)(m_ZoomRate * m_pBkImage[0]->height)),8,3);
		cvResize(m_pBkImage[0],m_pBkImage[1]);
	}

}

void	HDisplay::DrawSelPoint(int Layer,CvPoint3D64f *pPt)
{
	if(m_pDrawImage==NULL) return;
	DrawData		*pData=GetData(Layer);
	if(pData!=NULL)
		pData->SetHilight(true);
}

void	HDisplay::DrawSelLine(int Layer,CvPoint3D64f *pPt,int index)
{
	//CString		strFormat;
	//wstring		strText;
	CvPoint3D64f	pt[2];
	DrawDataLine	*pLine;
	DrawData		*pData;
	if(m_pDrawImage==NULL) return;

	pData=GetData(Layer);
	if(pData!=NULL)
	{
		pLine=(DrawDataLine*)pData;
		if(pPt==NULL)
		{
			// 整線變色
			pLine->SetHilight(true);
			return;
		}
		switch(index)
		{
		case 0:
			// 移動端點 0
			pt[0]=(*pPt);
			pt[1]=pLine->m_pt[1];
			pLine->SetPoint(cvPoint3D64f(pt[0].x,pt[0].y,0),pLine->m_pt[1]);
			if(m_bDrawText)
			{
				pt[1]=Trans2Pixelf(cvPoint3D64f(pt[0].x,pt[0].y,0));
				//strText=strFormat.GetBuffer();
			}
			break;
		case 1:
			// 移動端點 1
			pt[0]=pLine->m_pt[0];
			pt[1]=(*pPt);
			pLine->SetPoint(pLine->m_pt[0],cvPoint3D64f(pt[1].x,pt[1].y,0));
			if(m_bDrawText)
			{
				pt[0]=Trans2Pixelf(cvPoint3D64f(pt[1].x,pt[1].y,0));
				pt[1]=Trans2Pixelf(cvPoint3D64f(pt[0].x,pt[0].y,0));
				//strText=::Format(_T("(%.2f,%.2f)"),pt[0].x,pt[0].y);
			}
			break;
		default:
			// 整線移動
			pt[0]=pPt[0];
			pt[1]=pPt[1];
			pLine->SetPoint(cvPoint3D64f(pt[0].x,pt[0].y,0),cvPoint3D64f(pt[1].x,pt[1].y,0));
			if(m_bDrawText)
			{
				for(int i=0;i<2;++i)
				{
					pt[i]=Trans2Pixelf(cvPoint3D64f(pt[i].x,pt[i].y,0));
					//strText=::Format(_T("(%.2f,%.2f)"),pLine->m_pt[i].x,pLine->m_pt[i].y);
				}
			}
			break;
		}
		pLine->SetHilight(true);
	}
}


void	HDisplay::DrawSelCrossLine(int Layer,CvPoint3D64f *pPt,int index)
{
	DrawDataCrossLine	*pLine;
	if(m_pDrawImage==NULL) return;
	pLine=(DrawDataCrossLine*)GetData(Layer);
	if(pLine!=NULL)
		pLine->m_pt=(*pPt);
	pLine->SetHilight(true);
}


void	HDisplay::DrawSelPolyline(int Layer,CvPoint3D64f *pPt,int index)
{
	DrawDataPolyline	*pPLine;
	if(m_pDrawImage==NULL) return;
	DrawData*	pData=GetData(Layer);
	if(pData!=NULL)
	{
		pPLine=(DrawDataPolyline*)pData;
		if(pPt==NULL)
		{
			// 整線變色
		}
		else if(pPLine->flags!=1 && pPt!=NULL)
		{
			// 整線移動
			pPLine->SetOffset(cvPoint3D64f(pPt[0].x,pPt[0].y,0));
		}
		else if(pPLine->flags==1 && index<(int)pPLine->GetSize())
		{
			//pPLine->ChangePoint(index,pPt[0]);
			//pPLine->GetPoint(index,&pt);
		}
		else
			return;

		pPLine->SetHilight(true);
	}
}

void	HDisplay::DrawSelRect(int Layer,CvPoint3D64f *pPt,int index)
{
	CvPoint			pt[2];
	DrawDataRect	*pRect;
	if(m_pDrawImage==NULL) return;

	DrawData* pData=GetData(Layer);
	if(pData!=NULL)
	{
		pRect=(DrawDataRect*)pData;
		if(pPt==NULL)
		{
			// 整線變色
		}
		else if(index==0 && pPt!=NULL)
		{
			// 移動端點 0
			pt[0]=cvPoint((int)pPt->x,			(int)pPt->y);
			pt[1]=cvPoint((int)pRect->m_pt[1].x,(int)pRect->m_pt[1].y);
			pRect->SetPoint(pt[0],pt[1]);
		}
		else if(index==1  && pPt!=NULL)
		{
			// 移動端點 1
			pt[0]=cvPoint((int)pRect->m_pt[0].x,(int)pRect->m_pt[0].y);
			pt[1]=cvPoint((int)pPt->x,			(int)pPt->y);
			pRect->SetPoint(pt[0],pt[1]);
		}
		else if(index==2  && pPt!=NULL)
		{
			// 移動端點 2
			pt[0]=cvPoint((int)pPt->x,			(int)pRect->m_pt[0].y);
			pt[1]=cvPoint((int)pRect->m_pt[1].x,(int)pPt->y);
			pRect->SetPoint(pt[0],pt[1]);
		}
		else if(index==3  && pPt!=NULL)
		{
			// 移動端點 3
			pt[0]=cvPoint((int)pRect->m_pt[0].x,(int)pPt->y);
			pt[1]=cvPoint((int)pPt->x,			(int)pRect->m_pt[1].y);
			pRect->SetPoint(pt[0],pt[1]);
		}
		else if(pPt!=NULL)
		{
			// 整線移動
			pt[0]=cvPoint((int)pPt[0].x,(int)pPt[0].y);
			pt[1]=cvPoint((int)pPt[1].x,(int)pPt[1].y);
			pRect->SetPoint(pt[0],pt[1]);
		}
		else
			return;
		pRect->SetHilight(true);
	}
}


void	HDisplay::DrawSelCircle(int Layer,CvPoint3D64f *pPt,int index)
{
	double			radius;
	DrawDataCircle	*pCircle;
	if(m_pDrawImage==NULL) return;

	DrawData*	pData=GetData(Layer);
	if(pData!=NULL)
	{
		pCircle=(DrawDataCircle*)pData;//itMap->second;
		if(pPt==NULL)
		{
			// 整圓變色
		}
		else if(index==0 && pPt!=NULL)
		{
			// 改變半徑
			radius=sqrt(pow(pPt->x-pCircle->m_center.x,2) + pow(pPt->y-pCircle->m_center.y,2));
			pCircle->m_Radius=radius;
		}
		else if(pPt!=NULL)
		{
			// 整線移動
			pCircle->m_center=cvPoint3D64f(pPt->x,pPt->y,0);
		}
		else
			return;
		pCircle->SetHilight(true);
	}
}


void	HDisplay::DrawSelArc(int Layer,	CvPoint3D64f *pPt,int index)
{
	CvPoint3D64f	point;
	double			radius;
	DrawDataArc		*pArc;
	if(m_pDrawImage==NULL) return;

	DrawData*	pData=GetData(Layer);
	if(pData!=NULL)
	{
		pArc=(DrawDataArc*)pData;
		if(pPt==NULL)
		{
			// 整圓變色
		}
		else if(index==-1 && pPt!=NULL)
		{
			// 整線移動
			pArc->m_center=cvPoint3D64f(pPt->x,pPt->y,0);
			pArc->SetAngle(pArc->m_Angle[0],pArc->m_Angle[1]);
		}
		else if(index==0 && pPt!=NULL)
		{
			// 移動端點 0
			pArc->SetAngle(*pPt,			pArc->m_pt[1]);
			
		}
		else if(index==1 && pPt!=NULL)
		{
			// 移動端點 1
			pArc->SetAngle(pArc->m_pt[0],	*pPt);
			
		}
		else
		{
			// 改變半徑
			point=Trans2MM(cvPoint((int)pPt->x,(int)pPt->y));
			radius=sqrt(pow(pArc->m_center.x-point.x,2) + pow(pArc->m_center.y-point.y,2));
			pArc->m_Radius=radius;
			pArc->SetAngle(pArc->m_Angle[0],pArc->m_Angle[1]);
		}
		pArc->SetHilight(true);
	}
}

void HDisplay::DrawImage(cv::Mat* pImage)
{
	IplImage *pImg = TransImage(*pImage);
	if (pImg != NULL)
	{
		DrawLocalImage(pImg, true);
		cvReleaseImage(&pImg);
	}
}

IplImage	*HDisplay::TransImage(cv::Mat &mat)
{
	IplImage* pNewImg;
	unsigned char *pSource, *pTarget;
	int	nType = mat.type();
	int channel = mat.channels();
	if (mat.cols <= 0 || mat.rows <= 0) return NULL;
	if (nType != 0 && nType != 16 && nType != 0 && nType != 16) return NULL;	// CV_8U:C1,CV_8U:C3,CV_8S:C1,CV_8S:C3
	if (channel != 1 && channel != 3) return NULL;
	
	pNewImg = cvCreateImage(cvSize(mat.cols, mat.rows), 8, channel);
	pSource = (unsigned char*)(mat.data);
	pTarget = (unsigned char*)pNewImg->imageData;
	for (int i = 0; i < mat.rows; i++)
	{
		/*
		for (int j = 0; j < mat.cols; j++)
		{
			addr = i * pNewImg->widthStep + j*channel;
			if (channel == 3)
			{
				for (int k = 0; k < 3; k++)
				{
					ucData = mat.at<cv::Vec3b>(i, j)[k];
					pNewImg->imageData[addr + k] = ucData;
				}
			}
			else if (channel == 1)
			{
				pNewImg->imageData[addr] = mat.at<uchar>(i, j);
			}
		}
		*/
		if(mat.step< pNewImg->widthStep)
			memcpy(pTarget, pSource, mat.step);
		else
			memcpy(pTarget, pSource, pNewImg->widthStep);
		pSource += mat.step;
		pTarget += pNewImg->widthStep;
	}

	return pNewImg;
}

void HDisplay::DrawImage(IplImage* pImage)
{
	DrawLocalImage(pImage,true);
}

void HDisplay::DrawLocalImage(IplImage* pImage,bool bPush2Images)
{
	if(pImage==NULL) return;

	cvReleaseImage(&m_pBkImage[0]);
	cvReleaseImage(&m_pBkImage[1]);

	if(pImage->nChannels==3)
		m_pBkImage[0]=cvCloneImage(pImage);
	else
	{
		m_pBkImage[0]=cvCreateImage(cvSize(pImage->width,pImage->height),pImage->depth,3);
		cvCvtColor(pImage,m_pBkImage[0],CV_GRAY2RGB);
	}
	
	ResetZoomOffset();
	if (pImage->width > 0 && pImage->height > 0)
	{
		double dblW = (double)m_DrawRect.width / pImage->width;
		double dblH = (double)m_DrawRect.height / pImage->height;
		(dblW < dblH) ? (m_dblDefaultZoom = dblW) : (m_dblDefaultZoom = dblH);
	}
	//Zoom(1);
	//Offset(0,0);

	IplImage* pImageTemp;
	if(bPush2Images)
	{
		m_SourceImage.push_back(cvCloneImage(pImage));
		if(m_SourceImage.size()>2)
		{
			pImageTemp=m_SourceImage[0];

			cvReleaseImage(&pImageTemp);

			m_SourceImage.erase(m_SourceImage.begin());
		}
	}

}

DrawData*	HDisplay::DrawDatas(int Layer)
{
	double			dblAngle[2],dblBase=0,*bluges,radius;
	CvScalar		color;
	CvSize			sz;
	DrawData		*pData=NULL;
	DrawDataImage	*pDrawImage;
	DrawDataLine	*pLine;
	DrawDataText	*pText;
	DrawDataPoint	*pPoint;
	DrawDataRect	*pRect;
	DrawDataCircle	*pCircle;
	DrawDataArc		*pArc;
	DrawDataCrossLine	*pCLine;
	DrawDataLines	*pLines;
	DrawDataTexts	*pTexts;
	DrawDataRects	*pRects;
	DrawDataArcs	*pArcs;
	DrawDataPoints	*pPoints;
	DrawDataCircles	*pCircles;

	CvRect			rect;
	CvPoint			*pPt,*ptIndex,point[4],ptCenter,ptSource,*pPPoint,ptFirst;
	CvPoint3D64f	*pPtf,center,off=cvPoint3D64f(0,0,0);
	UINT			size;
	int				nValue,nTemp,nSegmentCount,index,nMin=0,nMax=0,nRadius;
	CvSize			RSize;
	wstring			strMessage;
	PolylineData				*pPData;
	DrawDataPolyline			*pPLine;
	DrawLib::DrawData::TextData txtData;
	std::string		strTemp;

	std::map<int,DrawData*>::iterator itMap;
	//WaitDatasEvent();

	for(itMap=m_DrawDatas.begin();itMap!=m_DrawDatas.end();++itMap)
	{
		if(itMap->second->nLayer==Layer)
			break;
	}
	if(itMap!=m_DrawDatas.end())
	{
		pData=itMap->second;
		switch(pData->nDataType)
		{
		case dtPoints:
			pPoints=(DrawDataPoints*)pData;
			(pPoints->IsHilight())?(color=m_SelColor):(color=pPoints->color);
			WaitMaskEvent();

			for(int j=0;j<(int)pPoints->m_Points.size();j++)
			{
				pPoint=pPoints->GetPoint(j);
				if(pPoint!=NULL)
				{
					point[0]=Trans2Pixel(pPoint->m_pt);
					
					cvLine2(m_pMaskImage,m_pPlotImage,cvPoint(point[0].x-pPoint->length,point[0].y),cvPoint(point[0].x+pPoint->length,point[0].y),color,pPoint->thickness);
					cvLine2(m_pMaskImage,m_pPlotImage,cvPoint(point[0].x,point[0].y-pPoint->length),cvPoint(point[0].x,point[0].y+pPoint->length),color,pPoint->thickness);
					delete pPoint;
				}
			}

			SetMaskEvent();
			break;
		case dtPoint:
			pPoint=(DrawDataPoint*)pData;
			(pPoint->IsHilight())?(color=m_SelColor):(color=pPoint->color);
			point[0]=Trans2Pixel(pPoint->m_pt);
			WaitMaskEvent();
			if(pPoint->thickness==CV_FILLED)
			{
				cvCircle(m_pMaskImage,point[0],pPoint->length, cvScalar(0,0,0),CV_FILLED,CV_AA,0);
				cvCircle(m_pPlotImage,point[0],pPoint->length,color,		CV_FILLED,CV_AA,0);
			}
			else
			{
				cvLine2(m_pMaskImage,m_pPlotImage,cvPoint(point[0].x-pPoint->length,point[0].y),cvPoint(point[0].x+pPoint->length,point[0].y),color,pPoint->thickness);
				cvLine2(m_pMaskImage,m_pPlotImage,cvPoint(point[0].x,point[0].y-pPoint->length),cvPoint(point[0].x,point[0].y+pPoint->length),color,pPoint->thickness);
			}
			SetMaskEvent();
			break;

		case dtText:
			pText=(DrawDataText*)pData;
			if(pText->texts.size()<=0) break;
			color=pText->color;
			point[0]=Trans2Pixel(pText->m_pt[0]);
			WaitMaskEvent();
			cvInitFont(&m_font,CV_FONT_HERSHEY_SIMPLEX,1.0f * pText->dblSize,1.0f * pText->dblSize,0,pText->thickness);
			
#ifdef _UNICODE
			strTemp=::converToMultiChar(pText->texts[0].strText);
#else
			strTemp=pText->texts[0].strText;
#endif
			cvPutText(m_pMaskImage,strTemp.c_str(),point[0],&m_font, cvScalar(0,0,0));
			cvPutText(m_pPlotImage,strTemp.c_str(),point[0],&m_font,color);
			SetMaskEvent();
			break;

		case dtTexts:
			pTexts=(DrawDataTexts*)pData;
			color=pTexts->color;

			WaitMaskEvent();
			cvInitFont(&m_font,CV_FONT_HERSHEY_SIMPLEX,1.0f * pTexts->dblSize,1.0f * pTexts->dblSize,0,pTexts->thickness);

			for(int i=0;i<(int)pTexts->m_Texts.size();i++)
			{
				pText=pTexts->GetText(i);
				if(pText==NULL || pText->texts.size()<=0)
				{
					delete pText;
					continue;
				}
				point[0]=Trans2Pixel(pText->m_pt[0]);
			
#ifdef _UNICODE
				strTemp=::converToMultiChar(pText->texts[0].strText);
#else
				strTemp=pText->texts[0].strText;
#endif
				if(strTemp.size()>0)
				{
					cvPutText(m_pMaskImage,strTemp.c_str(),point[0],&m_font, cvScalar(0,0,0));
					cvPutText(m_pPlotImage,strTemp.c_str(),point[0],&m_font,color);
				}
				delete pText;
			}
			SetMaskEvent();
			break;

		case dtImage:
			pDrawImage=(DrawDataImage*)pData;
			WaitMaskEvent();

			point[0]=cvPoint((int)pDrawImage->ptOrg.x,(int)pDrawImage->ptOrg.y);;
			rect=cvRect(point[0].x,point[0].y,pDrawImage->m_pImage->width,pDrawImage->m_pImage->height);
			cvSetImageROI(m_pMaskImage,rect);
			cvSetImageROI(m_pPlotImage,rect);
			rect=cvRect(0,0,pDrawImage->m_pImage->width,pDrawImage->m_pImage->height);
			cvSetImageROI(pDrawImage->m_pImage,rect);

			cvAddWeighted(pDrawImage->m_pImage,1,m_pMaskImage,0,0,m_pMaskImage);
			cvAddWeighted(pDrawImage->m_pImage,1,m_pPlotImage,0,0,m_pPlotImage);
			
			cvResetImageROI(pDrawImage->m_pImage);
			cvResetImageROI(m_pPlotImage);
			cvResetImageROI(m_pMaskImage);
			
			SetMaskEvent();
			break;

		case dtLine:
			pLine=(DrawDataLine*)pData;
			(pLine->IsHilight())?(color=m_SelColor):(color=pLine->color);
			for(int i=0;i<2;++i)
				point[i]=Trans2Pixel(pLine->m_pt[i]);
			WaitMaskEvent();
			cvLine2(m_pMaskImage,m_pPlotImage,point[0],point[1],color,			pLine->thickness);
			if(m_bMoveData && pLine->IsHilight())
			{
				cvCircle(m_pMaskImage,point[0],RADIUS, cvScalar(0,0,0),	1);
				cvCircle(m_pPlotImage,point[0],RADIUS,color,			1);
				cvCircle(m_pMaskImage,point[1],RADIUS, cvScalar(0,0,0),	1);
				cvCircle(m_pPlotImage,point[1],RADIUS,color,			1);
			}
			SetMaskEvent();
			break;

		case dtLines:
			pLines=(DrawDataLines*)pData;
			(pLines->IsHilight())?(color=m_SelColor):(color=pLines->color);
			WaitMaskEvent();

			for(int j=0;j<(int)pLines->m_Lines.size();j++)
			{
				pLine=pLines->GetLine(j);
				if(pLine!=NULL)
				{
					for(int i=0;i<2;++i)
						point[i]=Trans2Pixel(pLine->m_pt[i]);
					cvLine2(m_pMaskImage,m_pPlotImage,point[0],point[1],pLine->color,	pLine->thickness); 
					delete pLine;
				}
			}

			SetMaskEvent();
			break;

		case dtCircles:
			pCircles=(DrawDataCircles*)pData;
			//(pCircles->IsHilight())?(color=m_SelColor):(color=pCircles->color);
			WaitMaskEvent();

			for(int j=0;j<(int)pCircles->m_Circles.size();j++)
			{
				pCircle=pCircles->GetCircle(j);
				if(pCircle!=NULL)
				{
					ptCenter=Trans2Pixel(pCircle->m_center);

					(pCircle->IsHilight()) ? (color = m_SelColor) : (color = pCircle->color);
					cvCircle(m_pMaskImage,ptCenter,(int)(pCircle->m_Radius*m_ZoomRate), cvScalar(0,0,0),	pCircle->thickness);
					cvCircle(m_pPlotImage,ptCenter,(int)(pCircle->m_Radius*m_ZoomRate),color,			pCircle->thickness);

					delete pCircle;
				}
			}

			SetMaskEvent();
			break;
	
		case dtArc: 
			pArc=(DrawDataArc*)pData; 
			(pArc->IsHilight())?(color=m_SelColor):(color=pArc->color);
			ptCenter=Trans2Pixel(pArc->m_center);
			dblAngle[0]=pArc->GetAngle(0);
			dblAngle[1]=pArc->GetAngle(1);
			TransAngle(pArc->m_center,dblBase,dblAngle[0],dblAngle[1]);
			if(pData->direct==0)
			{
				if(abs(dblAngle[0]-dblAngle[1])<0.0001)
					dblAngle[1]=dblAngle[1] - 360 + 0.0001;
			}
			else
			{
				if(abs(dblAngle[0]-dblAngle[1])<0.0001)
					dblAngle[1]=dblAngle[1] - 360 - 0.0001;
			}
			WaitMaskEvent();

			nRadius=(int)(pArc->m_Radius*m_ZoomRate);
			RSize=cvSize(nRadius,nRadius);
			//cvEllipse(m_pMaskImage,ptCenter,cvSize((int)(pArc->m_Radius*m_ZoomRate),(int)(pArc->m_Radius*m_ZoomRate)),dblBase,dblAngle[0],dblAngle[1],CV_RGB(0,0,0),	pArc->thickness);
			//cvEllipse(m_pPlotImage,ptCenter,cvSize((int)(pArc->m_Radius*m_ZoomRate),(int)(pArc->m_Radius*m_ZoomRate)),dblBase,dblAngle[0],dblAngle[1],color,			pArc->thickness);
			cvEllipse(m_pMaskImage,ptCenter,RSize,dblBase,dblAngle[0],dblAngle[1], cvScalar(0,0,0),	pArc->thickness);
			cvEllipse(m_pPlotImage,ptCenter,RSize,dblBase,dblAngle[0],dblAngle[1],color,			pArc->thickness);

			point[0]=cvPoint(ptCenter.x-CIRCLEPITCH,	ptCenter.y);
			point[1]=cvPoint(ptCenter.x+CIRCLEPITCH,	ptCenter.y);
			point[2]=cvPoint(ptCenter.x,				ptCenter.y-CIRCLEPITCH);
			point[3]=cvPoint(ptCenter.x,				ptCenter.y+CIRCLEPITCH);
			cvLine2(m_pMaskImage,m_pPlotImage,point[0],point[1],color,			1);
			cvLine2(m_pMaskImage,m_pPlotImage,point[2],point[3],color,			1);
			
			if(m_bMoveData && pArc->IsHilight())
			{
				for(int i=0;i<2;++i)
				{
					point[0]=Trans2Pixel(pArc->m_pt[i]);
					cvCircle(m_pMaskImage,point[0],RADIUS, cvScalar(0,0,0),	1);
					cvCircle(m_pPlotImage,point[0],RADIUS,color,			1);
				}
			}
			SetMaskEvent();
			break;
		case dtArcs: 
			pArcs=(DrawDataArcs*)pData; 
			(pArcs->IsHilight())?(color=m_SelColor):(color=pArcs->color);
			WaitMaskEvent();
			
			for(int j=0;j<(int)pArcs->m_Arcs.size();j++)
			{
				pArc=pArcs->GetArc(j);
				if(pArc!=NULL)
				{
					ptCenter=Trans2Pixel(pArc->m_center);
					dblAngle[0]=pArc->GetAngle(0);
					dblAngle[1]=pArc->GetAngle(1);
					TransAngle(pArc->m_center,dblBase,dblAngle[0],dblAngle[1]);
					if(pArc->direct==0)
					{
						if(abs(dblAngle[0]-dblAngle[1])<0.0001)
							dblAngle[1]=dblAngle[1] - 360 + 0.0001;
					}
					else
					{
						if(abs(dblAngle[0]-dblAngle[1])<0.0001)
							dblAngle[1]=dblAngle[1] - 360 - 0.0001;
					}
					nRadius=(int)(pArc->m_Radius*m_ZoomRate);
					RSize=cvSize(nRadius,nRadius);
					if(nRadius>0 && nRadius>=pArc->thickness)
					{
						cvEllipse(m_pMaskImage,ptCenter,RSize,dblBase,dblAngle[0],dblAngle[1], cvScalar(0,0,0),	pArc->thickness);
						cvEllipse(m_pPlotImage,ptCenter,RSize,dblBase,dblAngle[0],dblAngle[1],color,			pArc->thickness);
					}
					delete pArc;
				}
			}
			
			SetMaskEvent();
			break;
		case dtPolyline:
			pPLine=(DrawDataPolyline*)pData;
			(pPLine->IsHilight())?(color=m_SelColor):(color=pPLine->color);
			
			size=(UINT)pPLine->GetSize();					// 點數
			if(size<=0) return NULL;
			pPtf=(CvPoint3D64f*)malloc(size*sizeof(CvPoint3D64f));
			nTemp=sizeof(point[0]);
			pPt=(CvPoint*)malloc(size*sizeof(CvPoint));
			pPPoint=(CvPoint*)malloc(size*sizeof(CvPoint));
			bluges=(double*)malloc(size*sizeof(double));	// 圓導角
			nSegmentCount=0;								// 圓角數目
			
			if(pPLine->GetPoints(size,pPtf,bluges,NULL,off))
			{
				for(UINT i=0;i<size;++i)
				{
					pPt[i]=Trans2Pixel(pPtf[i]);
					pPPoint[i]=pPt[i];
					if(bluges[i]!=0) nSegmentCount++;
				}
				ptFirst=pPt[0];
				ptSource=pPt[0];
				nValue=size;
				if(pPLine->flags==1) // 閉合曲線
				{
					WaitMaskEvent();
					if(nSegmentCount==0)
					{
						// 無圓角,則繪出點集合
						cvPolyLine(m_pMaskImage,&pPt,&nValue,1,1, cvScalar(0,0,0),	pPLine->thickness, CV_AA, 0);
						cvPolyLine(m_pPlotImage,&pPt,&nValue,1,1,color,			pPLine->thickness, CV_AA, 0);
					}
					else
					{
						// nSegmentCount+2塊
						index=0;
						ptIndex=&pPPoint[0];
						for(UINT i=0;i<size;++i)
						{
							if(bluges[i] ==0)
							{
								// 直線
								if(pPLine->GetData(i,&pPData))
								{
									if(i==(size-1))
										cvLine2(m_pMaskImage,m_pPlotImage,pPPoint[i],ptSource,color,			1);
									pPPoint[index++]=pPt[i];
								}
								
							}
							else
							{
								if(index!=0)
								{
									pPPoint[index++]=pPt[i];
									nValue=index;
									cvPolyLine(m_pMaskImage,&ptIndex,&nValue,1,0, cvScalar(0,0,0),	pPLine->thickness, CV_AA, 0);
									cvPolyLine(m_pPlotImage,&ptIndex,&nValue,1,0,color,			pPLine->thickness, CV_AA, 0);
									index=0;
								}
								// 圓切角
								if(pPLine->GetData(i,&pPData))
								{
									ptCenter=Trans2Pixel(pPData->GetCenter());
									radius=pPData->radius*m_ZoomRate;
									dblAngle[0]=pPData->angle[0]*180/PI;
									dblAngle[1]=pPData->angle[1]*180/PI;
									if(pPData->bulge>0)
									{
										TransAngle(pPData->GetCenter(),dblBase,dblAngle[0],dblAngle[1]);
										sz=cvSize((int)(radius+0.5),(int)(radius+0.5));
										if(i>=(size-1))
										{
											if(!(pPt[i].x==ptFirst.x && pPt[i].y==ptFirst.y))
											{
												cvArc(m_pMaskImage,ptCenter,pPt[i],ptFirst,pPLine->direct, cvScalar(0,0,0),pPLine->thickness,CV_AA,0);
												cvArc(m_pPlotImage,ptCenter,pPt[i],ptFirst,pPLine->direct,color,		pPLine->thickness,CV_AA,0);
											}
										}
										else
										{
											if(!(pPt[i].x==pPt[i+1].x && pPt[i].y==pPt[i+1].y))
											{
												cvArc(m_pMaskImage,ptCenter,pPt[i],pPt[i+1],pPLine->direct, cvScalar(0,0,0),pPLine->thickness,CV_AA,0);
												cvArc(m_pPlotImage,ptCenter,pPt[i],pPt[i+1],pPLine->direct,color,		pPLine->thickness,CV_AA,0);
											}
										}
									}
									else if(i<(size-1))
									{ 
										//TransAngle(pPData->GetCenter(),dblBase,dblAngle[1],dblAngle[0]);
										//sz=cvSize((int)(radius+0.5),(int)(radius+0.5));
										if(!(pPt[i].x==pPt[i+1].x && pPt[i].y==pPt[i+1].y))
										{ 
											cvArc(m_pMaskImage,ptCenter,pPt[i],pPt[i+1],pPLine->direct, cvScalar(0,0,0),pPLine->thickness,CV_AA,0);
											cvArc(m_pPlotImage,ptCenter,pPt[i],pPt[i+1],pPLine->direct,color,		pPLine->thickness,CV_AA,0);
										}
									}
									else if(i==(size-1))
									{
										//TransAngle(pPData->GetCenter(),dblBase,dblAngle[1],dblAngle[0]);
										//sz=cvSize((int)(radius+0.5),(int)(radius+0.5));
										cvArc(m_pMaskImage,ptCenter,pPt[i],pPt[0],pPLine->direct, cvScalar(0,0,0),pPLine->thickness,CV_AA,0);
										cvArc(m_pPlotImage,ptCenter,pPt[i],pPt[0],pPLine->direct,color,		pPLine->thickness,CV_AA,0);
									}
								}
							}
						}
					}

					if(m_bMoveData && pPLine->IsHilight())
					{
						for(UINT i=0;i<size;++i)
						{
							cvCircle(m_pMaskImage,pPt[i],RADIUS, cvScalar(0,0,0),	1);
							cvCircle(m_pPlotImage,pPt[i],RADIUS,color,			1);
						}
					}
					SetMaskEvent();
				}
				else
				{
					// NOT閉合曲線
					WaitMaskEvent();
					// nSegmentCount+3塊
					if(nSegmentCount==0)
					{
						// 無圓角,則繪出點集合
						cvPolyLine(m_pMaskImage,&pPt,&nValue,1,0, cvScalar(0,0,0),	pPLine->thickness, CV_AA, 0);
						cvPolyLine(m_pPlotImage,&pPt,&nValue,1,0,color,			pPLine->thickness, CV_AA, 0);
					}
					else
					{
						// nSegmentCount+2塊
						index=0;
						ptIndex=&pPPoint[0];
						for(UINT i=0;i<size;++i)
						{
							if(bluges[i]==0)
							{
								// 直線
								if(i==(size-1))
								{
									if(index!=0)
									{
										pPPoint[index++]=pPt[i];
										nValue=index;
										cvPolyLine(m_pMaskImage,&ptIndex,&nValue,1,0, cvScalar(0,0,0),	pPLine->thickness, CV_AA, 0);
										cvPolyLine(m_pPlotImage,&ptIndex,&nValue,1,0,color,			pPLine->thickness, CV_AA, 0);
										index=0;
									}
									else if(bluges[0]==0)
										cvLine2(m_pMaskImage,m_pPlotImage,pPPoint[i],ptSource,color,			1);
								}
								pPPoint[index++]=pPt[i];							
							}
							else
							{
								if(index!=0)
								{
									pPPoint[index++]=pPt[i];
									nValue=index;
									cvPolyLine(m_pMaskImage,&ptIndex,&nValue,1,0, cvScalar(0,0,0),	pPLine->thickness, CV_AA, 0);
									cvPolyLine(m_pPlotImage,&ptIndex,&nValue,1,0,color,			pPLine->thickness, CV_AA, 0);
									index=0;
								}
								if(i>=(size-1)) break; 
								// 圓切角   
								if(pPLine->GetData(i,&pPData))
								{
									ptCenter=Trans2Pixel(pPData->GetCenter());
									radius=pPData->radius*m_ZoomRate;
									dblAngle[0]=pPData->angle[0]*180/PI;
									dblAngle[1]=pPData->angle[1]*180/PI;
									
									if(pPData->bulge>0) 
									{
										//TransAngle(pPData->GetCenter(),dblBase,dblAngle[0],dblAngle[1]);
										//sz=cvSize((int)(radius+0.5),(int)(radius+0.5));
										//if(size==68 && i>=66)
										{
										cvArc(m_pMaskImage,ptCenter,pPt[i],pPt[i+1],pPLine->direct, cvScalar(0,0,0),pPLine->thickness,CV_AA,0);
										cvArc(m_pPlotImage,ptCenter,pPt[i],pPt[i+1],pPLine->direct,color,		pPLine->thickness,CV_AA,0);
										}
										//cvEllipse(m_pMaskImage,ptCenter,sz,dblBase,dblAngle[0],dblAngle[1],CV_RGB(0,0,0),	pPLine->thickness);
										//cvEllipse(m_pPlotImage,ptCenter,sz,dblBase,dblAngle[0],dblAngle[1],color,			pPLine->thickness);
									}
									else
									{ 
										//TransAngle(pPData->GetCenter(),dblBase,dblAngle[1],dblAngle[0]);
										//sz=cvSize((int)(radius+0.5),(int)(radius+0.5));
										cvArc(m_pMaskImage,ptCenter,pPt[i],pPt[i+1],pPLine->direct, cvScalar(0,0,0),pPLine->thickness,CV_AA,0);
										cvArc(m_pPlotImage,ptCenter,pPt[i],pPt[i+1],pPLine->direct,color,		pPLine->thickness,CV_AA,0);
										//cvEllipse(m_pMaskImage,ptCenter,sz,dblBase,dblAngle[0],dblAngle[1],CV_RGB(0,0,0),	pPLine->thickness);
										//cvEllipse(m_pPlotImage,ptCenter,sz,dblBase,dblAngle[0],dblAngle[1],color,			pPLine->thickness);
									}
								}
							}
						}
					}

					if(m_bMoveData && pPLine->IsHilight())
					{
						for(UINT i=0;i<size;++i)
						{
							cvCircle(m_pMaskImage,pPt[i],RADIUS, cvScalar(0,0,0),	1);
							cvCircle(m_pPlotImage,pPt[i],RADIUS,color,			1);
						}
					}
					SetMaskEvent();
				}
			}
			free(bluges);
			free(pPt);
			free(pPtf);
			free(pPPoint);
			break;
		case dtCircle:
			pCircle=(DrawDataCircle*)pData;
			(pCircle->IsHilight())?(color=m_SelColor):(color=pCircle->color);
			ptCenter=Trans2Pixel(pCircle->m_center);
			WaitMaskEvent();
			cvCircle(m_pMaskImage,ptCenter,(int)(pCircle->m_Radius*m_ZoomRate), cvScalar(0,0,0),	pCircle->thickness);
			cvCircle(m_pPlotImage,ptCenter,(int)(pCircle->m_Radius*m_ZoomRate),color,			pCircle->thickness);
			point[0]=cvPoint(ptCenter.x-CIRCLEPITCH,	ptCenter.y);
			point[1]=cvPoint(ptCenter.x+CIRCLEPITCH,	ptCenter.y);
			point[2]=cvPoint(ptCenter.x,				ptCenter.y-CIRCLEPITCH);
			point[3]=cvPoint(ptCenter.x,				ptCenter.y+CIRCLEPITCH);
			cvLine2(m_pMaskImage,m_pPlotImage,point[0],point[1],color,			1);
			cvLine2(m_pMaskImage,m_pPlotImage,point[2],point[3],color,			1);
			SetMaskEvent();
			break;
		case dtRect:
			pRect=(DrawDataRect*)pData;
			(pRect->IsHilight())?(color=m_SelColor):(color=pRect->color);
			WaitMaskEvent();

			if(pRect->m_angle==0)
			{
				point[0]=Trans2Pixel(pRect->m_pt[0]);
				point[1]=Trans2Pixel(pRect->m_pt[1]);
				//point[1]=cvPoint(point[0].x+pRect->m_width,point[0].y+pRect->m_height);
				cvRectangle(m_pMaskImage,point[0],point[1], cvScalar(0,0,0),	pRect->thickness);
				cvRectangle(m_pPlotImage,point[0],point[1],color,			pRect->thickness);
			}
			else
			{
				point[0]=Trans2Pixel(pRect->m_pt[0]);
				point[1]=cvPoint(point[0].x+(int)pRect->m_width,point[0].y+(int)pRect->m_height);
				point[2]=cvPoint(point[0].x,point[0].y+(int)pRect->m_height);
				point[3]=cvPoint(point[0].x+(int)pRect->m_width,point[0].y);
				center=cvPoint3D64f((point[0].x+point[1].x)/2,(point[0].y+point[1].y)/2,0);
				radius=sqrt(pow(pRect->m_height,2)+pow(pRect->m_width,2));

				for(int k=0;k<4;++k)
				{
					CvPoint2D64f	ptTemp=cvPoint2D64f(
										(point[k].x-center.x)*cos(pRect->m_angle)-(point[k].y-center.y)*sin(pRect->m_angle)+center.x,
										(point[k].x-center.x)*sin(pRect->m_angle)+(point[k].y-center.y)*cos(pRect->m_angle)+center.y);
					point[k]=cvPoint((int)ptTemp.x,(int)ptTemp.y);
				}
				//point[1]=cvPoint(pRect->m_width*cos(pRect->m_angle)+point[0].x,			pRect->m_width*sin(pRect->m_angle)+point[0].y);
				//point[2]=cvPoint(radius*cos(pRect->m_angle+45*PI/180)+point[0].x,		radius*sin(pRect->m_angle+45*PI/180)+point[0].y);
				//point[3]=cvPoint(pRect->m_height*cos(pRect->m_angle+PI/2)+point[0].x,	pRect->m_height*sin(pRect->m_angle+PI/2)+point[0].y);
				cvLine2(m_pMaskImage,m_pPlotImage,point[0],point[3],color,			1);
				cvLine2(m_pMaskImage,m_pPlotImage,point[1],point[3],color,			1);
				cvLine2(m_pMaskImage,m_pPlotImage,point[2],point[1],color,			1);
				cvLine2(m_pMaskImage,m_pPlotImage,point[2],point[0],color,			1); 
			}
			if(m_bMoveData && pRect->IsHilight())
			{
				for(int i=0;i<4;++i)
				{
					point[0]=Trans2Pixel(pRect->m_pt[i]);
					cvCircle(m_pMaskImage,point[0],RADIUS, cvScalar(0,0,0),	1);
					cvCircle(m_pPlotImage,point[0],RADIUS,color,			1);
				}
			}
			SetMaskEvent();
			break;

		case dtRects:
			pRects=(DrawDataRects*)pData;
			(pRects->IsHilight())?(color=m_SelColor):(color=pRects->color);
			WaitMaskEvent();
			
			for(int gg=0;gg<(int)pRects->m_Rects.size();gg++)
			{
				pRect=pRects->GetRect(gg);
				if(pRect==NULL) continue;
				if(pRect->m_angle==0)
				{
					(pRects->IsHilight())?(color=m_SelColor):(color=pRect->color);
					point[0]=Trans2Pixel(pRect->m_pt[0]);
					point[1]=Trans2Pixel(pRect->m_pt[1]);
					cvRectangle(m_pMaskImage,point[0],point[1], cvScalar(0,0,0),	pRect->thickness);
					cvRectangle(m_pPlotImage,point[0],point[1],color,			pRect->thickness);
				}
				else
				{
					(pRects->IsHilight())?(color=m_SelColor):(color=pRect->color);
					point[0]=Trans2Pixel(pRect->m_pt[0]);
					point[1]=cvPoint(point[0].x+ (int)pRect->m_width,point[0].y+ (int)pRect->m_height);
					point[2]=cvPoint(point[0].x,point[0].y+ (int)pRect->m_height);
					point[3]=cvPoint(point[0].x+ (int)pRect->m_width,point[0].y);
					center=cvPoint3D64f((point[0].x+point[1].x)/2,(point[0].y+point[1].y)/2,0);
					radius=sqrt(pow(pRect->m_height,2)+pow(pRect->m_width,2));

					for(int k=0;k<4;++k)
					{	
						point[k]=cvPoint(	(int)((point[k].x-center.x)*cos(pRect->m_angle)-(point[k].y-center.y)*sin(pRect->m_angle)+center.x),
							(int)((point[k].x-center.x)*sin(pRect->m_angle)+(point[k].y-center.y)*cos(pRect->m_angle)+center.y));
					}
					cvLine2(m_pMaskImage,m_pPlotImage,point[0],point[3],color,			1);
					cvLine2(m_pMaskImage,m_pPlotImage,point[1],point[3],color,			1);
					cvLine2(m_pMaskImage,m_pPlotImage,point[2],point[1],color,			1);
					cvLine2(m_pMaskImage,m_pPlotImage,point[2],point[0],color,			1); 
				}
				delete pRect;
			}
			SetMaskEvent();
			break;
		case dtCrossLine:
			pCLine=(DrawDataCrossLine*)pData;
			(pCLine->IsHilight())?(color=m_SelColor):(color=pCLine->color);
			point[0]=Trans2Pixel(pCLine->m_pt);
			WaitMaskEvent();
			if(pCLine->m_length==0)
			{
				cvLine2(m_pMaskImage,m_pPlotImage,cvPoint(0,point[0].y),cvPoint(m_pMaskImage->width-1,point[0].y),color,			pCLine->thickness);
				cvLine2(m_pMaskImage,m_pPlotImage,cvPoint(point[0].x,0),cvPoint(point[0].x,m_pMaskImage->height-1),color,			pCLine->thickness);
			}
			else
			{
				cvLine2(m_pMaskImage,m_pPlotImage,cvPoint(point[0].x-pCLine->m_length,point[0].y),cvPoint(point[0].x+pCLine->m_length,point[0].y),color,			pCLine->thickness);
				cvLine2(m_pMaskImage,m_pPlotImage,cvPoint(point[0].x,point[0].y-pCLine->m_length),cvPoint(point[0].x,point[0].y+pCLine->m_length),color,			pCLine->thickness);
			}
			if(m_bMoveData && pCLine->IsHilight())
			{
				cvCircle(m_pMaskImage,point[0],RADIUS, cvScalar(0,0,0),	1);
				cvCircle(m_pPlotImage,point[0],RADIUS,color,			1);
			}
			SetMaskEvent();
			break;
		default:
			break;
		}

		
		
	}

	

	// 原點
	if(m_bDrawOrg)
	{
		ptCenter=Trans2Pixel(cvPoint3D64f(0,0,0));
		point[0]=cvPoint(ptCenter.x - 10,	ptCenter.y);
		point[1]=cvPoint(ptCenter.x + 10,	ptCenter.y);
		WaitMaskEvent();
		cvLine2(m_pMaskImage,m_pPlotImage,point[0],point[1], cvScalar(255,255,0),1);
		point[0]=cvPoint(ptCenter.x,		ptCenter.y-10);
		point[1]=cvPoint(ptCenter.x,		ptCenter.y+10);
		cvLine2(m_pMaskImage,m_pPlotImage,point[0],point[1], cvScalar(255,255,0),1);
		SetMaskEvent();
	}

	//SetDatasEvent();
	return pData;
}


void	HDisplay::cvLine2(IplImage* pImageM,IplImage* pImageP,CvPoint pt1,CvPoint pt2,CvScalar color,int thickness)
{
	if(pt1.x==pt2.x && pt1.y==pt2.y)
	{
		cvCircle(pImageM,pt1,3, cvScalar(0,0,0),0,8,3);
		cvCircle(pImageP,pt1,3,color,0,8,3); 
	}
	else
	{
		cvLine(pImageM,pt1,pt2, cvScalar(0,0,0),thickness);
		cvLine(pImageP,pt1,pt2,color,thickness);
	}
}

void HDisplay::DrawArc(int Layer,CvPoint3D64f ptCenter,CvPoint3D64f pt1,CvPoint3D64f pt2,CvScalar color,int thickness,int index,int direct,double *pDataF,CvPoint3D64f offset)
{
	double dblRadius[2];
	dblRadius[0]=sqrt(pow(ptCenter.x-pt1.x,2)+pow(ptCenter.y-pt1.y,2));
	dblRadius[1]=sqrt(pow(ptCenter.x-pt2.x,2)+pow(ptCenter.y-pt2.y,2));
	DrawArc(Layer,(dblRadius[0]+dblRadius[1])/2,pt1,pt2,color,thickness,index,direct,pDataF,offset);
}

void	HDisplay::DrawArc(int Layer,double radius,CvPoint3D64f pt1,CvPoint3D64f pt2,CvScalar color,int index,int thickness,int direct,double *pDataF,CvPoint3D64f offset)
{
	CvPoint3D64f	A,B,center[2];
	double			dblSita[3];
	if(radius<=0) return;
	
	double a,b,c,k,q,m,n,p,temp;
	a=-2*(pt1.x-pt2.x);
	b=-2*(pt1.y-pt2.y);
	c=pow(pt1.x,2)-pow(pt2.x,2)+pow(pt1.y,2)-pow(pt2.y,2);
	if(b==0) return;
	k=-1*a/b;
	q=-1*c/b;
	m=1+pow(k,2);
	n=-2*(pt1.x-k*q+pt1.y*k);
	p=pow(pt1.x,2)+pow(q,2)-2*pt1.y*q+pow(pt1.y,2)-pow(radius,2);
	temp=pow(n,2)-4*m*p;
	if(temp<0) return;
	temp=sqrt(temp);
	center[0].x=(float)((-1*n+temp)/2/m);
	center[0].y=(float)(k*center[0].x+q);
	center[0].z=pt1.z;
	center[1].x=(float)((-1*n-temp)/2/m);
	center[1].y=(float)(k*center[1].x+q);
	center[1].z=pt2.z;
	
	A=cvPoint3D64f(pt1.x-center[0].x,pt1.y-center[0].y,0);
	B=cvPoint3D64f(pt2.x-center[0].x,pt2.y-center[0].y,0);
	if((A.x*B.y-A.y*B.x)<0)
		center[0]=center[1];
	
	A=cvPoint3D64f(pt1.x-center[0].x,	pt1.y-center[0].y,	0);
	B=cvPoint3D64f(radius,				0,					0);
	dblSita[0]=acos((A.x*B.x)/pow(radius,2)) * 180/PI;
	if((-A.y*B.x)>0)
		dblSita[0]=-1*dblSita[0];

	A=cvPoint3D64f(pt2.x-center[0].x,	pt2.y-center[0].y,	0);
	B=cvPoint3D64f(radius,				0,					0);
	dblSita[1]=acos((A.x*B.x)/pow(radius,2)) * 180/PI;
	if((-A.y*B.x)>0)
		dblSita[1]=-1*dblSita[1];
	
	DrawArc(Layer,&center[0],(int)radius,dblSita[0],dblSita[1],color,thickness,index,direct,pDataF,offset);
}

void	HDisplay::cvArc2( CvArr* img, CvPoint center,CvPoint ptStart,CvPoint ptEnd,int dir, CvScalar color,int thickness, int line_type, int shift)
{
	CvPoint2D64f	v[2],point;
	double			r[2];
	v[0]=cvPoint2D64f(ptStart.x-center.x,	ptStart.y-center.y);
	v[1]=cvPoint2D64f(ptEnd.x-center.x,		ptEnd.y-center.y);
	r[0]=sqrt(pow((double)(ptStart.x-center.x),2)+pow((double)(ptStart.y-center.y),2));
	r[1]=sqrt(pow((double)(ptEnd.x-center.x),2)+pow((double)(ptEnd.y-center.y),2));

	double dblR= r[0];//(r[0]>=r[1])?(r[0]):(r[1]);
	double dblS[2],ds;
	for(int i=0;i<2;i++)
	{
		if(v[i].y==0)
		{
			if(v[i].x<0)
			{
				if(i==0) 
					dblS[i]=-1*PI;
				else
					dblS[i]=PI;
			}
			else
				dblS[i]=0;
		}
		else
		{
			dblS[i]=acos(v[i].x/dblR);
			if(v[i].y<0) 
				dblS[i]=dblS[i]*(-1);
		}
	}

	double dS,dblDS=acos((v[0].x*v[1].x+v[0].y*v[1].y)/pow(dblR,2));
	int size=(int)abs(dblR*dblDS)/4;
	if(size<=1 || abs(dblDS)<0.005 || v[0].y==v[1].y || dblS[0]==dblS[1])
	{
		cvLine(img,ptStart,ptEnd,color,	thickness);
		return;
	}
 
	CvPoint *pPt=(CvPoint*)malloc(size*sizeof(CvPoint));
	pPt[0]=ptStart;
	pPt[size-1]=ptEnd;

	if(dblS[1]>dblS[0])
		dS=dblDS/size;
	else
		dS=-1*dblDS/size;
	if(dblS[1]>dblS[0])
		ds=dblDS;
	else
		ds=-1*dblDS;
	point.x=dblR*(cos(dblS[0])*cos(ds)-sin(dblS[0])*sin(ds))+center.x;
	if((dblS[0]+ds)>PI)
		point.y=-1*dblR*(sin(dblS[0])*cos(ds)+cos(dblS[0])*sin(ds))+center.y;
	else
		point.y=dblR*(sin(dblS[0])*cos(ds)+cos(dblS[0])*sin(ds))+center.y;

	
	for(int i=1;i<size-1;i++) 
	{
		ds=dS*i;
		//if((dblS[0]+ds)>PI/2)
			point.x=dblR*(cos(dblS[0])*cos(ds)-sin(dblS[0])*sin(ds))+center.x;
		//else
			//point.x=-1*dblR*(cos(dblS[0])*cos(ds)-sin(dblS[0])*sin(ds))+center.x;
		if((dblS[0]+ds)>PI)
			point.y=-1*dblR*(sin(dblS[0])*cos(ds)+cos(dblS[0])*sin(ds))+center.y;
		else
			point.y=dblR*(sin(dblS[0])*cos(ds)+cos(dblS[0])*sin(ds))+center.y;
		pPt[i].x= (int)(point.x+0.5);
		pPt[i].y= (int)(point.y+0.5);
 	}

	cvPolyLine(img,&pPt,&size,1,0,color,thickness, line_type, shift);
	free(pPt);
}

void HDisplay::DrawCircles(int Layer,int count,CvPoint3D64f	*pCenter,double *pRadius,CvScalar *pColor,int thickness,int index,int direct,double *pDataF,CvPoint3D64f offset)
{
	DrawData		*pDData;
	DrawDataCircles	*pCircles;
	if(m_pDrawImage==NULL) return;	
	
	if(pCenter==NULL || pRadius==NULL || count<=0)
	{
		// 重繪
		pDData=GetData(Layer);
		if(pDData==NULL) return;
		if(pDData->nDataType==dtCircles)
		{
			pCircles=(DrawDataCircles*)pDData;
			pCircles->SetHilight(false);
		}
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			if (!CheckCircleData(&pCenter[i], &pRadius[i])) return;
		}

		// 新繪
		pCircles=(DrawDataCircles*)CreateNewLayer(DrawLib::dtCircles,Layer,index,offset);
		if(pCircles==NULL) return;
		
		//pCircles->color=color;
		pCircles->thickness=thickness;
		pCircles->nIndex=index;
		pCircles->offset=offset;
		for(int j=0;j<10;++j)
			(pDataF!=NULL)?(pCircles->DataF[j]=pDataF[j]):(pCircles->DataF[j]=NULL);
		pCircles->SetHilight(false);


		DrawDataCircle*	pNew;
		for(int i=0;i<count;i++)
		{			
			pNew=new DrawDataCircle();
			pNew->m_center=pCenter[i];
			pNew->m_Radius=pRadius[i];
			pNew->color = pColor[i];
			pNew->thickness = thickness;
			pCircles->AddCircle(pNew);
		}
	}
}


void HDisplay::DrawArcs(int Layer,int count,CvPoint3D64f *pt,CvScalar color,int thickness,int index,double *pDataF,CvPoint3D64f offset)
{
	DrawData		*pData;
	DrawDataArcs	*pArcs;
	if(m_pDrawImage==NULL) return;	
	
	if(pt==NULL)
	{
		// 重繪
		pData=GetData(Layer);
		if(pData==NULL) return;
		if(pData->nDataType==dtArcs)
		{
			pArcs=(DrawDataArcs*)pData;
			pArcs->SetHilight(false);
		}
	}
	else
	{
		// 新繪
		pArcs=(DrawDataArcs*)CreateNewLayer(DrawLib::dtArcs,Layer,index,offset);
		if(pArcs==NULL) return;
		
		pArcs->color=color;
		pArcs->thickness=thickness;
		pArcs->nIndex=index;
		pArcs->offset=offset;
		for(int j=0;j<10;++j)
			(pDataF!=NULL)?(pArcs->DataF[j]=pDataF[j]):(pArcs->DataF[j]=NULL);
		pArcs->SetHilight(false);


		DrawDataArc*	pNew;
		for(int i=0;i<count;i++)
		{			
			pNew=new DrawDataArc();

			// 確認正逆轉方向
			CvPoint2D64f A=cvPoint2D64f(pt[3*i+1].x-pt[3*i+0].x,pt[3*i+1].y-pt[3*i+0].y);
			CvPoint2D64f B=cvPoint2D64f(pt[3*i+2].x-pt[3*i+0].x,pt[3*i+2].y-pt[3*i+0].y);
			double cross=A.x*B.y-A.y*B.x;
			if(cross<0)
			{
				pNew->m_pt[1]=pt[3*i+0];
				pNew->m_pt[0]=pt[3*i+2];
			}
			else
			{
				pNew->m_pt[0]=pt[3*i+0];
				pNew->m_pt[1]=pt[3*i+2];
			}
			if(!GetCircle(pt[3*i+0],pt[3*i+1],pt[3*i+2],pNew->m_center))
			{
				delete pNew;
				pArcs->Clear();
				delete pArcs;
				return;
			}
			else
			{
				pNew->m_Radius=pNew->m_center.z;
				pNew->SetAngle(pNew->m_pt[0],pNew->m_pt[1]);

				pNew->color=color;
				pNew->thickness=thickness;
				pNew->nIndex=index;
				pNew->offset=offset;
				for(int j=0;j<10;++j)
					(pDataF!=NULL)?(pNew->DataF[j]=pDataF[j]):(pNew->DataF[j]=NULL);
				pNew->SetHilight(false);
				
				pArcs->AddArc(pNew);
			}
		}
	}
}

void	HDisplay::GetNewCenter(CvPoint center,CvPoint ptStart,CvPoint ptEnd,double dblR,CvPoint2D64f &CenterOut)
{
	CvPoint2D64f	ptTemp[2];
	double	dblPitch[2];
	double	dblX=ptStart.x-ptEnd.x;
	double	dblY=ptStart.y-ptEnd.y;
	if(dblX==0)
	{
		ptTemp[0].y=ptTemp[1].y=(ptStart.y+ptEnd.y)/2.0;
		ptTemp[0].x=sqrt(dblR*dblR-pow((double)(ptStart.y-ptEnd.y)/2,2));
		ptTemp[1].x=ptStart.x+ptTemp[0].x;
		ptTemp[0].x=ptStart.x-ptTemp[0].x;
		dblPitch[0]=abs(ptTemp[0].x-center.x);
		dblPitch[1]=abs(ptTemp[1].x-center.x);
		if(dblPitch[0]<dblPitch[1])
			CenterOut=ptTemp[0];
		else
			CenterOut=ptTemp[1];
		return;
	}
	double dblK=-1*(dblX*dblX+dblY*dblY)/2/dblR;
	double dblA=dblK/dblX;
	double dblB=-1*dblY/dblX;
	double dblTemp=dblA*dblA*dblB*dblB-(dblB*dblB+1)*(dblA*dblA-1);
	if(dblTemp<0)
	{
		CenterOut=cvPoint2D64f(center.x,center.y);
		return;
	}
	double dblSin[2],dblCos[2][2];
	dblSin[0]=(-1*dblA*dblB+sqrt(dblTemp))/(dblB*dblB+1);
	dblSin[1]=(-1*dblA*dblB-sqrt(dblTemp))/(dblB*dblB+1);
	for(int i=0;i<2;i++)
		dblCos[i][0]=sqrt(1-pow(dblSin[i],2));
	for(int i=0;i<2;i++)
		dblCos[i][1]=-1*dblCos[i][0];

	CvPoint2D64f	pt,ptMin;
	double			dblMin,dblValue;
	pt.x=dblR*dblCos[0][0]	+ptStart.x;
	pt.y=dblR*dblSin[0]		+ptStart.y;
	dblMin=pow(pt.x-center.x,2) + pow(pt.y-center.y,2);
	ptMin=pt;

	pt.x=dblR*dblCos[0][1]	+ptStart.x;
	pt.y=dblR*dblSin[0]		+ptStart.y;
	dblValue=pow(pt.x-center.x,2) + pow(pt.y-center.y,2);
	if(dblValue<dblMin)
	{
		dblMin=dblValue;
		ptMin=pt;
	}

	for(int i=0;i<2;i++)
	{
		pt.x=dblR*dblCos[1][i]	+ptStart.x;
		pt.y=dblR*dblSin[1]		+ptStart.y;
		dblValue=pow(pt.x-center.x,2) + pow(pt.y-center.y,2);
		if(dblValue<dblMin)
		{
			dblMin=dblValue;
			ptMin=pt;
		}
	}
	CenterOut=ptMin;
}

void	HDisplay::cvArc( CvArr* img, CvPoint center,CvPoint ptStart,CvPoint ptEnd,int dir, CvScalar color,int thickness, int line_type, int shift)
{
	CvPoint2D64f	v[2];
	double			r[2];
	
	r[0]=sqrt(pow((double)(ptStart.x-center.x),2)+pow((double)(ptStart.y-center.y),2));
	r[1]=sqrt(pow((double)(ptEnd.x-center.x),2)+pow((double)(ptEnd.y-center.y),2));
	double ds,dblR=(r[0]+r[1])/2.0;
	CvPoint2D64f	cen;
	GetNewCenter(center,ptStart,ptEnd,dblR,cen);
	r[0]=sqrt(pow(ptStart.x-cen.x,2)+pow(ptStart.y-cen.y,2));
	r[1]=sqrt(pow(ptEnd.x-cen.x,2)+pow(ptEnd.y-cen.y,2));
	v[0]=cvPoint2D64f(ptStart.x-cen.x,	ptStart.y-cen.y);
	v[1]=cvPoint2D64f(ptEnd.x-cen.x,	ptEnd.y-cen.y);

	double dblDS=acos((v[0].x*v[1].x+v[0].y*v[1].y)/pow(dblR,2));
	if(dblDS<0.01) dblDS=0;
	if((v[0].x*v[1].y-v[0].y*v[1].x)>0)
		dblDS=-1*dblDS;	

	int size=abs(dblR*dblDS)/2.0+2;
	if(size<=2 || abs(dblDS)<0.005)
	{
		cvLine(img,ptStart,ptEnd,color,	thickness); 
		return;
	}
	double dblCos=sqrt(2.0)*(ptStart.x+ptStart.y-cen.x-cen.y)/(2*dblR);
	double dblSin=sqrt(2.0)*(ptStart.x-ptStart.y-cen.x+cen.y)/(2*dblR);
	double dblCos2,dblSin2;
	
	CvPoint2D64f	point;
	double	dS=dblDS/size;
	CvPoint *pPt=(CvPoint*)malloc(size*sizeof(CvPoint));
	for(int i=1;i<size-1;i++)  
	{
		ds=dS*i;
		dblCos2=dblCos*cos(ds)-dblSin*sin(ds);
		dblSin2=dblSin*cos(ds)+dblCos*sin(ds);
		point.x=dblR*(dblCos2+dblSin2)/sqrt(2.0)+cen.x;
		point.y=dblR*(dblCos2-dblSin2)/sqrt(2.0)+cen.y;

		pPt[i].x= (int)(point.x+0.5);
		pPt[i].y= (int)(point.y+0.5);
	}
	pPt[0]=ptStart;
	pPt[size-1]=ptEnd;
	
	ds=dS*(size-1);
	dblCos2=dblCos*cos(ds)-dblSin*sin(ds);
	dblSin2=dblSin*cos(ds)+dblCos*sin(ds);
	pPt[size-1].x= (int)((dblR*dblCos2+dblR*dblSin2)/sqrt(2.0)+cen.x);
	pPt[size-1].y= (int)((dblR*dblCos2-dblR*dblSin2)/sqrt(2.0)+cen.y);

	cvPolyLine(img,&pPt,&size,1,0,color,thickness, line_type, shift);
	free(pPt);
}


void HDisplay::DrawArc(int Layer,CvPoint3D64f	*pCenter,double radius,double start_angle, double end_angle,CvScalar color,int thickness,int index,int direct,double *pDataF,CvPoint3D64f offset)
{
	DrawDataArc	*pArc;
	DrawData	*pData;
	//std::map<int,DrawData*>::iterator itMap;
	if(m_pDrawImage==NULL) return;	

	if(pCenter==NULL)
	{
		// 重繪
		pData=GetData(Layer);
		if(pData==NULL) return;
		pArc=(DrawDataArc*)pData;//itMap->second;
		pArc->SetHilight(false);
	}
	else
	{
		// 新繪
		pArc=(DrawDataArc*)CreateNewLayer(DrawLib::dtArc,Layer,index,offset);
		if(pArc==NULL) return;
		pArc->m_center=cvPoint3D64f(pCenter->x,pCenter->y,pCenter->z);
		pArc->m_Radius=radius;
		pArc->SetAngle(start_angle,end_angle);
		pArc->nLayer=Layer;
		pArc->color=color;
		pArc->direct=direct;
		pArc->nIndex=index;
		pArc->thickness=thickness;
		pArc->offset=offset;
		for(int i=0;i<10;++i)
			(pDataF!=NULL)?(pArc->DataF[i]=pDataF[i]):(pArc->DataF[i]=NULL);
		pArc->SetHilight(false);
	}
}

void HDisplay::DrawPoint(int Layer,CvPoint3D64f *pt,int	len,CvScalar color,int thickness,int index,double *pDataF,CvPoint3D64f offset)
{
	DrawDataPoint	*pPt;
	if(m_pDrawImage==NULL) return;	

	if(pt==NULL)
	{
		// 重繪
		pPt=(DrawDataPoint*)GetData(Layer);
		if(pPt==NULL) return;
		pPt->SetHilight(false);
	}
	else
	{
		// 新繪
		pPt=(DrawDataPoint*)CreateNewLayer(DrawLib::dtPoint,Layer,index,offset);
		if(pPt==NULL) return;
		pPt->m_pt=(*pt);
		pPt->length=len;
		pPt->color=color;
		pPt->nIndex=index;
		pPt->thickness=thickness;
		pPt->offset=offset;
		for(int i=0;i<10;++i)
			(pDataF!=NULL)?(pPt->DataF[i]=pDataF[i]):(pPt->DataF[i]=NULL);
		pPt->SetHilight(false);
	}
}


void HDisplay::DrawPoints(int Layer,int count,CvPoint3D64f *pt,int	len,CvScalar color,int thickness,int index,double *pDataF,CvPoint3D64f offset)
{
	DrawData		*pData;
	DrawDataPoints	*pPoints;
	if(m_pDrawImage==NULL) return;	
	
	if(pt==NULL)
	{
		// 重繪
		pData=GetData(Layer);
		if(pData==NULL) return;
		if(pData->nDataType==dtPoints)
		{
			pPoints=(DrawDataPoints*)pData;
			pPoints->SetHilight(false);
		}
	}
	else
	{
		// 新繪
		pPoints=(DrawDataPoints*)CreateNewLayer(DrawLib::dtPoints,Layer,index,offset);
		if(pPoints==NULL) return;
		pPoints->color=color;
		pPoints->thickness=thickness;
		pPoints->nIndex=index;
		pPoints->offset=offset;
		for(int j=0;j<10;++j)
			(pDataF!=NULL)?(pPoints->DataF[j]=pDataF[j]):(pPoints->DataF[j]=NULL);
		pPoints->SetHilight(false);

		DrawDataPoint*	pNew;
		for(int i=0;i<count;i++)
		{			
			pNew=new DrawDataPoint();
			pNew->m_pt=pt[i];
			pNew->color=color;
			pNew->thickness=thickness;
			pNew->nIndex=index;
			pNew->offset=offset;
			for(int j=0;j<10;++j)
				(pDataF!=NULL)?(pNew->DataF[j]=pDataF[j]):(pNew->DataF[j]=NULL);
			pNew->SetHilight(false);
			
			pPoints->AddPoint(pNew);
		}
	}
}

void HDisplay::AddDrawPolyline(int Layer,CvPoint3D64f	Pt,double bulge,double* pDatas)
{
	DrawDataPolyline *pPLine=(DrawDataPolyline*)GetData(Layer);
	if(pPLine==NULL) return;
	pPLine->AddPoint(Pt,bulge,pDatas);
}

bool	HDisplay::CheckPolylineDatas(CvPoint3D64f	*pPt, int DataSize)
{
	double dblMax = (double)0xFFFFFFFF; // 4 byte
	double dblMin = -1 * dblMax;


	for (int i = 0; i < DataSize; i++)
	{
		if (pPt[i].x > dblMax || pPt[i].x < dblMin ||
			pPt[i].y > dblMax || pPt[i].y < dblMin)
			return false;

		if (isnan(pPt[i].x) || isnan(pPt[i].y)) return false;
	}
	return true;
}

bool	HDisplay::CheckCircleData(CvPoint3D64f *pCenter, double* pRadius)
{
	double dblMax = (double)0xFFFFFFFF; // 4 byte
	double dblMin = -1 * dblMax;
	if (pCenter == 0 || (*pRadius) <= 0) return false;
	if (isnan(*pRadius)) return false;
	if (isnan(pCenter->x) || isnan(pCenter->y)) return false;

	if (pCenter->x > dblMax || pCenter->x < dblMin ||
		pCenter->y > dblMax || pCenter->y < dblMin ||
		(*pRadius)>dblMax || (*pRadius) < dblMin)
	{
		return false;
	}

	return true;
}

void HDisplay::DrawPolyline(int Layer,CvPoint3D64f	*pPt,double *bulges,int count,	CvScalar color,int thickness,int index,int direct,int flags,double *pDataF,CvPoint3D64f offset)
{
	
	CvPoint3D64f		center;
	DrawDataPolyline	*pPLine;
	PolylineData		*pPData=NULL,*pPNext=NULL;
	if(m_pDrawImage==NULL) return;	
	if(pPt==NULL)
	{
		// 重繪
		pPLine=(DrawDataPolyline*)GetData(Layer);
		if(pPLine==NULL) return;
		pPLine->SetHilight(false);
	}
	else
	{
		if (!CheckPolylineDatas(pPt, count)) return;

		// 新繪
		pPLine=(DrawDataPolyline*)CreateNewLayer(DrawLib::dtPolyline,Layer,index,offset);
		if(pPLine==NULL) return;
		for(int i=0;i<count;++i)
		{
			if(bulges==NULL)
				pPLine->AddPoint(pPt[i],0,NULL);
			else
				pPLine->AddPoint(pPt[i],bulges[i],NULL);
		}
		for(int i=0;i<(count-1);++i)
		{
			if(pPLine->GetData(i,&pPData))
			{
				if(pPLine->GetData(i+1,&pPNext) && pPData->bulge!=0)
				{
					if(DrawDataPolyline::GetSpLineData(cvPoint3D64f(pPData->point.x,pPData->point.y,pPData->point.z),cvPoint3D64f(pPNext->point.x,pPNext->point.y,pPNext->point.z),pPData->bulge,center,pPData->radius,pPData->angle[0],pPData->angle[1])==0)
						pPData->SetCenter(center);
				}
			}
		}
		if((flags==1) && (count>2))
		{
			if(pPLine->GetData(count-1,&pPData))
			{
				if(pPLine->GetData(0,&pPNext) && pPData->bulge!=0)
				{
					if(DrawDataPolyline::GetSpLineData(cvPoint3D64f(pPData->point.x,pPData->point.y,pPData->point.z),cvPoint3D64f(pPNext->point.x,pPNext->point.y,pPNext->point.z),pPData->bulge,center,pPData->radius,pPData->angle[0],pPData->angle[1])==0)
						pPData->SetCenter(center);
				}
			}
		}
		pPLine->color=color;
		pPLine->thickness=thickness;
		pPLine->SetHilight(false);
		pPLine->direct=direct;
		pPLine->nIndex=index;
		pPLine->flags=flags;
		pPLine->offset=offset;
		for(int i=0;i<10;++i)
			(pDataF!=NULL)?(pPLine->DataF[i]=pDataF[i]):(pPLine->DataF[i]=NULL);
	}
	
}

void HDisplay::DrawCrossLine(int Layer,CvPoint3D64f	*pPt,CvScalar color,int thickness,int index,double *pDataF,CvPoint3D64f offset,int len)
{
	DrawDataCrossLine	*pLine;
	if(m_pDrawImage==NULL) return;	

	if(pPt==NULL)
	{
		// 重繪
		pLine=(DrawDataCrossLine*)GetData(Layer);
		if(pLine==NULL) return;
		pLine->SetHilight(false);
	}
	else
	{
		// 新繪
		pLine=(DrawDataCrossLine*)CreateNewLayer(DrawLib::dtCrossLine,Layer,index,offset);
		if(pLine==NULL) return;

		pLine->m_pt=cvPoint3D64f(pPt->x,pPt->y,pPt->z);
		pLine->color=color;
		pLine->thickness=thickness;
		pLine->direct=0;
		pLine->nIndex=-1;
		pLine->offset=offset;
		pLine->m_length=len;
		for(int i=0;i<10;++i)
			(pDataF!=NULL)?(pLine->DataF[i]=pDataF[i]):(pLine->DataF[i]=NULL);
		pLine->SetHilight(false);
	}
}


void HDisplay::DrawLine(int Layer,CvPoint3D64f *pPt,CvScalar color,int thickness,int index,int direct,double *pDataF,CvPoint3D64f offset)
{
	//std::map<int,DrawData*>::iterator itMap;
	DrawData	*pData;
	DrawDataLine *pLine;
	if(m_pDrawImage==NULL) return;	
	
	if(pPt==NULL)
	{
		// 重繪
		pData=GetData(Layer);
		if(pData==NULL) return;
		pLine=(DrawDataLine*)pData;
		pLine->SetHilight(false);
	}
	else
	{
		// 新繪
		pLine=(DrawDataLine*)CreateNewLayer(DrawLib::dtLine,Layer,index,offset);
		if(pLine==NULL) return;

		for(int i=0;i<2;++i)
			pLine->m_pt[i]=cvPoint3D64f(pPt[i].x, pPt[i].y,pPt[i].z);
		pLine->color=color;
		pLine->thickness=thickness;
		pLine->direct=direct;
		pLine->nIndex=index;
		pLine->offset=offset;
		for(int i=0;i<10;++i)
			(pDataF!=NULL)?(pLine->DataF[i]=pDataF[i]):(pLine->DataF[i]=NULL);
		pLine->SetHilight(false);
	}
}



void HDisplay::DrawLines(int Layer,int count,CvPoint3D64f *pPt,CvScalar color,int thickness,int index,int direct,double *pDataF,CvPoint3D64f offset)
{
	DrawData	*pData;
	DrawDataLines *pLines;
	if(m_pDrawImage==NULL) return;	
	
	if(pPt==NULL)
	{
		// 重繪
		pData=GetData(Layer);
		if(pData==NULL) return;
		if(pData->nDataType==dtLines)
		{
			pLines=(DrawDataLines*)pData;
			pLines->SetHilight(false);
		}
	}
	else
	{
		// 新繪
		pLines=(DrawDataLines*)CreateNewLayer(DrawLib::dtLines,Layer,index,offset);
		if(pLines==NULL) return;
		pLines->color=color;
		pLines->thickness=thickness;
		pLines->direct=direct;
		pLines->nIndex=index;
		pLines->offset=offset;
		for(int j=0;j<10;++j)
			(pDataF!=NULL)?(pLines->DataF[j]=pDataF[j]):(pLines->DataF[j]=NULL);
		pLines->SetHilight(false);

		DrawDataLine*	pNew;
		for(int i=0;i<count;i++)
		{			
			pNew=new DrawDataLine();
			for(int j=0;j<2;j++)
				pNew->m_pt[j]=cvPoint3D64f(pPt[i*2+j].x, pPt[i*2+j].y,pPt[i*2+j].z);

			if(pNew->m_pt[0].x==pNew->m_pt[1].x && pNew->m_pt[0].y==pNew->m_pt[1].y && pNew->m_pt[0].z==pNew->m_pt[1].z)
			{
				delete pNew;
				continue;
			}

			/*
			if (!CheckInBoundary(pNew->m_pt[0]) || !CheckInBoundary(pNew->m_pt[1]))
			{
				delete pNew;
				continue;
			}
			*/
			pNew->color=color;
			pNew->thickness=thickness;
			pNew->direct=direct;
			pNew->nIndex=index;
			pNew->offset=offset;
			for(int j=0;j<10;++j)
				(pDataF!=NULL)?(pNew->DataF[j]=pDataF[j]):(pNew->DataF[j]=NULL);
			pNew->SetHilight(false);
			
			pLines->AddLine(pNew);
		}
	}
}
bool	HDisplay::CheckInBoundary(CvPoint3D64f point)
{
	if (this->m_pBkImage[0] == NULL) return false;
	if (point.x < 0 || point.x >= m_pBkImage[0]->width) return false;
	if (point.y < 0 || point.y >= m_pBkImage[0]->height) return false;
	return true;
}

void HDisplay::DrawImage(int Layer,CvPoint3D64f *pPt,IplImage* pImage,double size,int index,double *pDataF,CvPoint3D64f offset)
{
	CvSize			ImageSize;
	DrawDataImage	*pDrawImage;
	if(m_pDrawImage==NULL) return;	 
	
	if(pPt==NULL)
	{
		// 重繪
		pDrawImage=(DrawDataImage*)GetData(Layer);
		if(pImage==NULL) return;
	}
	else
	{
		// 新繪
		pDrawImage=(DrawDataImage*)CreateNewLayer(DrawLib::dtImage,Layer,index,offset);
		if(pDrawImage==NULL) return;
		
		pDrawImage->ptOrg=(*pPt);

		cvReleaseImage(&pDrawImage->m_pImage);

		ImageSize=cvGetSize(pImage);
		ImageSize.width=ImageSize.width*size;
		ImageSize.height=ImageSize.height*size;
		pDrawImage->m_pImage=cvCreateImage(ImageSize,pImage->depth,pImage->nChannels);
		cvResize(pImage,pDrawImage->m_pImage);

		pDrawImage->offset=offset;
		for(int i=0;i<10;++i) 
			(pDataF!=NULL)?(pDrawImage->DataF[i]=pDataF[i]):(pDrawImage->DataF[i]=0.0);
	}
}




void HDisplay::DrawTexts(int Layer,int count,CvPoint3D64f *pPt,wstring  *strText,CvScalar color,double size,int thickness,int index,double *pDataF,CvPoint3D64f offset)
{
	DrawData	*pData;
	DrawDataTexts *pTexts;
	DrawLib::DrawData::TextData stcText;

	if(m_pDrawImage==NULL) return;	
	
	if(pPt==NULL)
	{
		// 重繪
		pData=GetData(Layer);
		if(pData==NULL) return;
		if(pData->nDataType==dtTexts)
		{
			pTexts=(DrawDataTexts*)pData;
			pTexts->SetHilight(false);
		}
	}
	else
	{
		// 新繪
		pTexts=(DrawDataTexts*)CreateNewLayer(DrawLib::dtTexts,Layer,index,offset);
		if(pTexts==NULL) return;
		pTexts->color=color; 
		pTexts->thickness=thickness;
		pTexts->nIndex=index;
		pTexts->dblSize=size;
		pTexts->offset=offset;
		for(int i=0;i<10;++i) 
			(pDataF!=NULL)?(pTexts->DataF[i]=pDataF[i]):(pTexts->DataF[i]=0.0);


		DrawDataText*	pNew;
		for(int i=0;i<count;i++)
		{			
			pNew=new DrawDataText();
			pNew->m_pt[0]=pPt[i];
			stcText.pos=cvPoint(0,0);
			stcText.strText=strText[i];

			pNew->texts.push_back(stcText);
			pNew->color=color; 
			pNew->thickness=thickness;
			pNew->nIndex=index;
			pNew->dblSize=size;
			pNew->offset=offset;
			for(int j=0;j<10;++j) 
				(pDataF!=NULL)?(pNew->DataF[j]=pDataF[j]):(pNew->DataF[j]=0.0);
			
			pTexts->AddText(pNew);
		}
	}
}


void HDisplay::DrawText(int Layer,CvPoint3D64f *pPt,wstring  strText,CvScalar color,double size,int thickness,int index,double *pDataF,CvPoint3D64f offset)
{
	DrawDataText	*pText;
	if(m_pDrawImage==NULL) return;	 
	DrawLib::DrawData::TextData stcText;
	if(pPt==NULL)
	{
		// 重繪
		pText=(DrawDataText*)GetData(Layer);
		if(pText==NULL) return;
	}
	else
	{
		// 新繪
		pText=(DrawDataText*)CreateNewLayer(DrawLib::dtText,Layer,index,offset);
		if(pText==NULL) return;
		//pText->m_pt=(*pPt);
		pText->m_pt[0]=(*pPt);
		stcText.pos=cvPoint(0,0);
		stcText.strText=strText;
		pText->texts.push_back(stcText);
		pText->color=color; 
		pText->thickness=thickness;
		pText->nIndex=index;
		pText->dblSize=size;
		pText->thickness=(int)size;
		pText->offset=offset;
		for(int i=0;i<10;++i) 
			(pDataF!=NULL)?(pText->DataF[i]=pDataF[i]):(pText->DataF[i]=0.0);
	}
}


void HDisplay::DrawRects(int Layer,int count,CvRect *pRect,CvScalar *pColor,int thickness,int index,int direct,double *pDataF,double angle,CvPoint3D64f offset)
{
	DrawData	*pData;
	DrawDataRects *pRects;
	CvPoint			pt[2];

	if(m_pDrawImage==NULL) return;	
	
	if(pRect==NULL)
	{
		// 重繪
		pData=GetData(Layer);
		if(pData==NULL) return;
		if(pData->nDataType==dtRects)
		{
			pRects=(DrawDataRects*)pData;
			pRects->SetHilight(false);
		}
	}
	else
	{
		// 新繪
		pRects=(DrawDataRects*)CreateNewLayer(DrawLib::dtRects,Layer,index,offset);
		if(pRects==NULL) return;
		pRects->color= cvScalar(0,0,0);
		pRects->thickness=thickness;
		pRects->nIndex=index;
		pRects->direct=direct;
		pRects->offset=offset;
		for(int i=0;i<10;++i) 
			(pDataF!=NULL)?(pRects->DataF[i]=pDataF[i]):(pRects->DataF[i]=0.0);


		DrawDataRect*	pNew;
		for(int i=0;i<count;i++)
		{
			if(pRect->width<=0) continue;
			pNew=new DrawDataRect();
			pNew->color=pColor[i];
			pNew->thickness=thickness;
			pNew->nIndex=index;
			pNew->offset=offset;
			pNew->direct=direct;
			for(int j=0;j<10;++j) 
				(pDataF!=NULL)?(pNew->DataF[j]=pDataF[j]):(pNew->DataF[j]=0.0);

			pNew->m_angle=angle;
			pNew->m_width=pRect[i].width;
			pNew->m_height=pRect[i].height;
			
			pt[0]=cvPoint(pRect[i].x,pRect[i].y);
			pt[1]=cvPoint(pRect[i].x+pRect[i].width-1,pRect[i].y+pRect[i].height-1);
			pNew->SetPoint(pt[0],pt[1]);
			
			pRects->AddRect(pNew);
		}
	}
}

void HDisplay::DrawRect(int Layer,CvRect *pRect,CvScalar color,int thickness,int index,int direct,double *pDataF,double angle,CvPoint3D64f offset)
{
	DrawDataRect	*pRec;
	DrawData		*pData;
	CvPoint			pt[2];
	if(m_pDrawImage==NULL) return;	

	if(pRect==NULL)
	{
		// 重繪
		pData=GetData(Layer);
		if(pData==NULL) return;
		pRec=(DrawDataRect*)pData;
		pRec->SetHilight(false);
	}
	else
	{
		// 新繪
		pRec=(DrawDataRect*)CreateNewLayer(DrawLib::dtRect,Layer,index,offset);
		if(pRec==NULL) return;
		pt[0]=cvPoint(pRect->x,pRect->y);
		pt[1]=cvPoint(pRect->x+pRect->width-1,pRect->y+pRect->height-1);
		pRec->m_angle=angle;
		pRec->m_width=pRect->width;
		pRec->m_height=pRect->height;
		pRec->SetPoint(pt[0],pt[1]);
		pRec->color=color;
		pRec->thickness=thickness;
		pRec->direct=direct;
		pRec->nIndex=index;
		pRec->offset=offset;
		for(int i=0;i<10;++i)
			(pDataF!=NULL)?(pRec->DataF[i]=pDataF[i]):(pRec->DataF[i]=NULL);
		pRec->SetHilight(false);
	}
}


void HDisplay::DrawCircle(int Layer,CvPoint3D64f *pCenter, double radius,CvScalar color,int thickness,int index,int direct,double *pDataF,CvPoint3D64f offset)
{
	DrawDataCircle *pCircle;
	DrawData	*pData;
	//std::map<int,DrawData*>::iterator itMap;
	if(m_pDrawImage==NULL) return;	

	if(pCenter==NULL)
	{
		// 重繪
		pData=GetData(Layer);
		if(pData==NULL) return;
		pData->SetHilight(false);
	}
	else
	{
		if (!CheckCircleData(pCenter, &radius)) return;

		// 新繪
		pCircle=(DrawDataCircle*)CreateNewLayer(DrawLib::dtCircle,Layer,index,offset);
		if(pCircle==NULL) return;
		pCircle->m_center=(*pCenter);
		pCircle->m_Radius=radius;
		pCircle->nLayer=Layer;
		pCircle->color=color;
		pCircle->direct=direct;
		pCircle->nIndex=index;
		pCircle->thickness=thickness;
		pCircle->offset=offset;
		for(int i=0;i<10;++i)
			(pDataF!=NULL)?(pCircle->DataF[i]=pDataF[i]):(pCircle->DataF[i]=NULL);
		pCircle->SetHilight(false);
	}
}


/**************************************************************************/

IMPLEMENT_DYNAMIC(HDisplayMFC, CStatic)

BEGIN_MESSAGE_MAP(HDisplayMFC, CStatic)
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


HDisplayMFC::HDisplayMFC()
:m_DibInfo(NULL)
{
	m_hDrawEvent=CreateEvent(NULL,FALSE,TRUE,NULL);
	m_hMaskEvent=CreateEvent(NULL,FALSE,TRUE,NULL);
	m_hDatasEvent=CreateEvent(NULL,FALSE,TRUE,NULL);

	m_cursor[0]=m_cursor[1]=NULL;
}

void HDisplayMFC::ChangeCursor(int id)
{
	if(m_cursor[id]!=NULL)
	{
		if(::GetCursor()==m_cursor[id]) return;
		::SetCursor(m_cursor[id]);
	}
}

HDisplayMFC::~HDisplayMFC()
{
	if(m_DibInfo!=NULL) delete[] m_DibInfo;
	
	CloseHandle(m_hDatasEvent);
	CloseHandle(m_hDrawEvent);
	CloseHandle(m_hMaskEvent);

}




CBitmap* HDisplayMFC::IplImage2CBitmap(IplImage* pImg)
{
	CBitmap* pBitmap = new CBitmap();

	DEVMODE stDevmode;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &stDevmode);
	CvSize size = cvSize(pImg->width,pImg->height);
	IplImage* pTmpImg = cvCreateImage(size, IPL_DEPTH_8U, 3);// stDevmode.dmBitsPerPel / IPL_DEPTH_8U);

	cvSetZero(pTmpImg);
	for (int i = 0; i < pImg->height; i++)
	{
		for (int j = 0; j < pImg->width; j++)
		{
			int nIndex = j;
			CvScalar scaler = cvGet2D(pImg, i, j);
			cvSet2D(pTmpImg,i,j, scaler);
		}
	}
	int nLen = pTmpImg->imageSize;
	BYTE* pBytes = new BYTE[nLen];
	memcpy(pBytes, pTmpImg->imageData, nLen);
	(*pBitmap).CreateBitmap(pTmpImg->width, pTmpImg->height, 1, pTmpImg->nChannels*pTmpImg->depth, pBytes);
	delete[] pBytes; pBytes = NULL;
	return pBitmap;
}

void HDisplayMFC::PreSubclassWindow()
{
	CRect rect;
	GetWindowRect(&rect);
	Init(cvRect(rect.left,rect.top,rect.right-rect.left+1,rect.bottom-rect.top+1));
	m_cursor[0]=AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	m_cursor[1]=AfxGetApp()->LoadStandardCursor(IDC_SIZEALL);
	__super::PreSubclassWindow();
}



int	 HDisplayMFC::ChangeSelLayer(int nLayer)
{
	int nRet=::HDisplay::ChangeSelLayer(nLayer);
	ReDrawLayers();
	return nRet;
}

void HDisplayMFC::ReDrawLayers()
{
	HDisplay::ReDrawLayers();
	RedrawWindow();
}

bool	HDisplayMFC::WaitDatasEvent(){return ::WaitForSingleObject(m_hDatasEvent,0)==0;}
void	HDisplayMFC::SetDatasEvent(){SetEvent(m_hDatasEvent);}

bool	HDisplayMFC::WaitMaskEvent(){return ::WaitForSingleObject(m_hMaskEvent,0)==0;}
void	HDisplayMFC::SetMaskEvent(){SetEvent(m_hMaskEvent);}

bool	HDisplayMFC::WaitDrawEvent(){return ::WaitForSingleObject(m_hDrawEvent,0)==0;}
void	HDisplayMFC::SetDrawEvent(){SetEvent(m_hDrawEvent);}


void HDisplayMFC::Init(CvRect rect,CvScalar color)
{
	HDisplay::Init(rect,color);	
	ModifyStyle(0,0x100,0);
	
	if(m_DibInfo!=NULL) delete[] m_DibInfo;
	
	WaitDrawEvent();
		cvReleaseImage(&m_pDrawImage);
		m_pDrawImage=cvCreateImage(cvSize(m_DrawRect.width,m_DrawRect.height),8,3);
	SetDrawEvent();

	WaitMaskEvent();
		cvReleaseImage(&m_pMaskImage);
		m_pMaskImage=cvCreateImage(cvSize(m_DrawRect.width,m_DrawRect.height),8,3);
		cvReleaseImage(&m_pPlotImage);
		m_pPlotImage=cvCreateImage(cvSize(m_DrawRect.width,m_DrawRect.height),8,3);
	SetMaskEvent();

	int nPos=0;
	int iW=m_DrawRect.width;
	int iH=m_DrawRect.height;
	
	int PaletteSize=0;
	int cbHeaderSize=sizeof(BITMAPINFOHEADER) + PaletteSize * sizeof(RGBQUAD);
	
	m_DibInfo = (BITMAPINFO*) new char[cbHeaderSize];
	m_DibInfo->bmiHeader.biBitCount=(short)24;					//m_pImage->depth;
	m_DibInfo->bmiHeader.biClrImportant=PaletteSize;			//關鍵色的數目
	m_DibInfo->bmiHeader.biClrUsed=PaletteSize;					//色彩數
	m_DibInfo->bmiHeader.biCompression=BI_RGB;					//圖檔未壓縮
	m_DibInfo->bmiHeader.biHeight=-iH;							//高
	m_DibInfo->bmiHeader.biPlanes=1;							// fixed
	m_DibInfo->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);		// fixed
	m_DibInfo->bmiHeader.biSizeImage=m_pDrawImage->widthStep;	//圖檔陣列大小
	m_DibInfo->bmiHeader.biWidth=iW;							//寬
	m_DibInfo->bmiHeader.biXPelsPerMeter=120;					//水平解析度
	m_DibInfo->bmiHeader.biYPelsPerMeter=120;					//垂直解析度
	
	WaitDrawEvent();
		cvRectangle(m_pDrawImage,cvPoint(0,0),cvPoint(iW,iH),color,CV_FILLED);
	SetDrawEvent();
	RedrawWindow();
}

IplImage* HDisplayMFCTransCBitmap2IplImage(CBitmap* pBmp)
{
	BITMAP	bmp;
	if(pBmp==NULL) return NULL;
	if(pBmp->GetBitmap(&bmp)!=0) return NULL;
	if(bmp.bmBits==NULL) return NULL;
	IplImage *pImage=cvCreateImage(cvSize(bmp.bmWidth,bmp.bmHeight),8,bmp.bmBitsPixel/8);

	long addrSource=(long)bmp.bmBits;
	long addrTarget=(long)pImage->imageData;
	for(int i=0;i<pImage->height;i++)
	{
		
	}
	

	return NULL;
}

void HDisplayMFC::OnViewDraw(CDC* pDC,CvRect rect)
{
	int iW,iH,ret;
	if(m_DibInfo!=NULL && m_pDrawImage!=NULL && WaitDrawEvent())
	{
		iW=m_DibInfo->bmiHeader.biWidth;
		iH=abs(m_DibInfo->bmiHeader.biHeight);
		if(rect.height<=0 || rect.width<=0) rect=cvRect(0,0,iW,iH);
		ret=::StretchDIBits(pDC->m_hDC,
			rect.x,rect.y,	rect.width,rect.height,			// Target
			0,		0,		iW,iH,							// Source
			m_pDrawImage->imageData,
			m_DibInfo,
			DIB_RGB_COLORS,
			SRCCOPY);
		SetDrawEvent();
	}
}

void HDisplayMFC::OnPaint()
{
	CPaintDC dc(this);
	OnViewDraw(&dc);
	
}

void HDisplayMFC::OnLButtonUp(UINT nFlags, CPoint point)
{
	HDisplay::cvOnMouseUp(cvPoint(point.x,point.y));
	__super::OnLButtonUp(nFlags, point);
}

void	HDisplayMFC::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	HDisplay::cvOnLButtonDblClk(cvPoint(point.x,point.y));
	
	//ReDrawLayers(); 
	SetFocus();
	__super::OnLButtonDblClk(nFlags, point);
}

void HDisplayMFC::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(HDisplay::cvOnMouseDown(cvPoint(point.x,point.y))==0)
	{
	}
	//ReDrawLayers(); 
	SetFocus();
	__super::OnLButtonDown(nFlags, point);
}

void HDisplayMFC::OnRButtonDown(UINT nFlags, CPoint point)
{
	if(HDisplay::cvOnRMouseDown(cvPoint(point.x,point.y))==0)
	{
	}
	//ReDrawLayers(); 
	SetFocus();
	__super::OnRButtonDown(nFlags, point);
}

void HDisplayMFC::OnMouseMove(UINT nFlags, CPoint point)
{
	if(HDisplay::cvOnMouseMove(cvPoint(point.x,point.y),(nFlags & 0x1)==1)==0)
		ReDrawLayers();
	__super::OnMouseMove(nFlags, point);
}

BOOL HDisplayMFC::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	OnMouseWheelRun(cvPoint(pt.x,pt.y),zDelta);
	return __super::OnMouseWheel(nFlags, zDelta, pt);
}


#endif