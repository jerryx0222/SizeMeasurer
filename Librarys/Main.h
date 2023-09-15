#pragma once




// 訊息
#define WM_MACHINEMESSAGE	WM_USER+2000
#define WM_VIEWCHANGE		WM_USER+2001
#define WM_LISTBTNDOWN		WM_USER+2002
#define WM_LISTDBLCLK		WM_USER+2003
#define WM_REPORTITEMEDIT	WM_USER+2004
#define WM_CHANGELANGUAGE	WM_USER+2005
#define WM_ONMESSAGESEND	WM_USER+2006
#define WM_ONPLCDATASEND	WM_USER+2007
#define WM_SHOWCONNECT		WM_USER+2008
#define WM_ERRORHAPPEN		WM_USER+2009
#define WM_USERLOGIN		WM_USER+2010
#define WM_WORKDATACHANGE	WM_USER+2011


// Timer
#define TMID_LANGUAGE	3000

// UI 頁面
#define VCOMMVIEW	5000
#define IDC_TABCTRL	5001
#define CREPORTCTRL	5002

// UI 按鈕
#define IDD_BUTTONAUTO	5100	//5100~5115
#define IDD_BUTTONOPT	5116	//5106~5130
#define IDD_EDTDATE		5200
#define IDD_EDTTIME		5201
#define IDD_TIME1000	5202
#define IDD_BTNLOAD		5203
#define IDD_BTNSAVE		5204
#define IDD_TIMEHOME	5205

#define IDC_BTNCANCEL	5210
#define IDC_BTNOK		5211
#define IDC_STCDESCRIPT	5212
#define IDC_ETCTITLE	5213
#define IDC_STCMEMO		5214
#define IDC_ETCMEMO		5215
#define IDC_STCSOLUTION	5216
#define IDC_CMBSELECT	5217
#define IDC_STCALARM	5218
#define IDC_BTNCHANGE	5219
#define IDC_STCUSERNAME	5220
#define IDC_STCPASSWORD	5221
#define IDC_STCNEWPWD	5222
#define IDC_STCPWDCHECK	5223
#define IDC_EDTUSERNAME	5224
#define IDC_EDTPASSWORD	5225
#define IDC_EDTNEWPWD	5226
#define IDC_EDTPWDCHECK	5227
#define IDC_BTNNEW		5228
#define IDC_BTNDELETE	5229
#define IDC_EDTNEWFILE	5230
#define IDC_STCPICTURE	5231

// 顏色定義
#define LightGrayColor		RGB(225,225,225)
#define DarkGrayColor		RGB(125,125,125)
#define RedColor			RGB(225,0,0)
#define BlueColor			RGB(0,0,225)
#define LBlueColor			RGB(0,255,225)
#define LightYellowColor	RGB(255, 255,0)
#define LightGreenColor		RGB(0,225,0)
#define GreenColor			RGB(0,100,0)
#define WhiteColor			RGB(255,255,255)
#define BlackColor			RGB(0,0,0)

enum DATATYPE
{
	dtString,
	dtInt,
	dtDouble,

};

enum vLANGUAGE
{
	vEnglish,
	vChineseT,
	vChineseS,
	vJapanese,

};
enum VIEWID
{
	vTop,
	vOper,
	vMenu,

	vAuto,
	vProduct,
	vMachineData,
	vWorkData,
	vManual,
	vVision,
	vMaintain,
	vError,


	vEnd,
};


struct BTNSTATUS
{
	int id;
	bool enable;
	CString text;
	bool check;
	CButton *pBtn;
	int  resource;
};



#ifdef USE_OPENCV440
#include <opencv2/opencv.hpp>
#pragma comment(lib,"opencv_world440d.lib")
/*
#pragma comment(lib,"opencv_highgui440d.lib")
#pragma comment(lib,"opencv_imgproc440d.lib")
#pragma comment(lib,"opencv_core440d.lib")
#pragma comment(lib,"opencv_imgcodecs440d.lib")
*/
#endif


