/*
	About   : OpenCV matching and click system for Flax.
	Auther  : teonsen
	Date    : 2019/12/18
	Version : 1.0
*/
//*******************************************************************************
// Project settings.
// 1. Project > Property > C/C++ > additional include dir > C:\opencv348exe\build\include
// 2. Project > Property > C/C++ > Code generator > runtime libraly > Muiti thread (/MT)
// 3. Project > Property > Linker > Genaral > additional lib dir > C:\opencv-3.4.8\build_static_x64\lib\Release;
//                                                                 C:\opencv-3.4.8\build_static_x64\3rdparty\lib\Release
// 4. Project > Property > Linker > System > Subsystem > Windows (/SUBSYSTEM:WINDOWS)
//*******************************************************************************

#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define ID_MYTIMER			5611
#define NO_IMAGE_FOUND		0
#define ERR_NO_TPLIMAGE		-1
#define ERR_COMMAND_INVALID -3
#define TIME_OUT			-4
#define EXIT_BY_USER		-5
#define ERR_LOGIC_FAILED	-6
#define ERR_NO_SRCIMAGE		-7
#define MAXARGS 10

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <olectl.h>

#include <wingdi.h>
#include <iostream>
#include "resource.h"

#include <opencv2/opencv.hpp>
#define _OPENCV_BUILD_STATIC_
#include "./libopencv.hpp"

enum cmdAction {none, leftClick, dblClick, rightClick, mMove} mouseAction;
struct _tMatched {
	int iposX;
	int iposY;
	int iCenterX;
	int iCenterY;
	int iWide;
	int iHigh;
	int iMatchedLev;
} tPos;

struct {
	INT			iCmdID;					// Command(0-5)
	LPSTR		lpstrTplPath1;			// Template image path
	UINT		iSimiler;				// OpenCV matching threshold(80-100)
	UINT		iShotInterval;			// Screen capture interval[ms]
	UINT		iNumberOfTimes;			// Retry
	BOOL		RunMinimize;			// Minimize when start
	INT			iCapturePosX;
	INT			iCapturePosY;
	INT			iCaptureWidth;
	INT			iCaptureHeight;
	LPSTR		lpstrSoucePath;			// Compared image path
} _args; // Arguments

char		gCommandText[8][32]={
				"Action  : Do Nothing",
				"Action  : Left Click",
				"Action  : Double Click",
				"Action  : Right Click",
				"Action  : Move",
				"Action  : Matching"
};
// TemplateImage
IplImage *_imgTemplate, *_imgSource;
char    *_CommandLineArgs[MAXARGS];

void myDelay(int msec)
{
	clock_t	sclock;
	sclock = clock();
	while(clock() - sclock < msec);
}

void MoveMyMouse(int x, int y)
{
	INPUT input[1];
	ZeroMemory(input, sizeof(input));
	input[0].type = INPUT_MOUSE;
	input[0].mi.dx = x * 65536 / GetSystemMetrics(SM_CXSCREEN);
	input[0].mi.dy = y * 65536 / GetSystemMetrics(SM_CYSCREEN);
	input[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
	input[0].mi.dwExtraInfo = GetMessageExtraInfo();
	::SendInput(1, input, sizeof(INPUT));
}

void MoveMyMouseOffset(int offsetX, int offsetY)
{
	POINT pos;

	GetCursorPos( &pos );
	MoveMyMouse( pos.x + offsetX, pos.y + offsetY );
}

void LClickMouse(void)
{
	INPUT input[2];
	ZeroMemory(input, sizeof(input));
	input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input[0].mi.dwExtraInfo = GetMessageExtraInfo();
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    input[1].mi.dwExtraInfo = GetMessageExtraInfo();
	::SendInput(2, input, sizeof(INPUT));
}

void MoveAndClick(int offsetX, int offsetY)
{
	MoveMyMouseOffset(offsetX, offsetY);
	myDelay(50);
	LClickMouse();
}

void justLeftDown(void)
{
	INPUT input[1];
	ZeroMemory(input, sizeof(input));
	input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input[0].mi.dwExtraInfo = GetMessageExtraInfo();
	::SendInput(1, input, sizeof(INPUT));
}

void justLeftUp(void)
{
	INPUT input[1];
	ZeroMemory(input, sizeof(input));
	input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    input[0].mi.dwExtraInfo = GetMessageExtraInfo();
	::SendInput(1, input, sizeof(INPUT));
}

void LClickMouse2(void)
{
	justLeftDown();
	myDelay(50);
	justLeftUp();
}

void DClickMouse(void)
{
	LClickMouse();
	myDelay(150);
	LClickMouse();
}

void RClickMouse(void)
{
	INPUT input[2];
	ZeroMemory(input, sizeof(input));
	input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    input[0].mi.dwExtraInfo = GetMessageExtraInfo();
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    input[1].mi.dwExtraInfo = GetMessageExtraInfo();
	::SendInput(2, input, sizeof(INPUT));
}

void SetTheWindowForeground(HWND hWnd)
{
	SetForegroundWindow(hWnd);
}

void ClickForegroundWindow(void)
{
	HWND	hFgWnd;
	RECT	lpRect;

	hFgWnd = GetForegroundWindow();
	if (GetWindowRect(hFgWnd, (LPRECT)&lpRect)) {
		//sprintf(msg,"Left=%d, Top=%d", lpRect.left, lpRect.top);
		//::MessageBoxA( NULL, msg, "", MB_OK );
		MoveMyMouse((int)lpRect.left + 30, (int)lpRect.top + 10);
		myDelay(100);
		LClickMouse();
	}
}

int execMatchingImages( LPCSTR lpTemplatePath, LPCSTR lpSorceImgPath)
{
	double		dblMinVal, dblMaxVal;
	CvPoint		cvminLoc, cvmaxLoc;
	CvSize		dst_size;
	IplImage	*imgCapture, *imgSrc3ch, *imgDst;
	char		dbgMsg[1024] = {'\0'};
	int			iRet;

	HWND hWndDesktop;
	RECT rectCaptureArea;
	int iDtWidth, iDtHeight;

	hWndDesktop = GetDesktopWindow();
	GetWindowRect(hWndDesktop, &rectCaptureArea);
	iDtWidth = _args.iCaptureWidth;
	iDtHeight = _args.iCaptureHeight;

	tPos.iposX = 0;
	tPos.iposY = 0;
	tPos.iWide = iDtWidth;
	tPos.iHigh = iDtHeight;

	if (strlen(lpSorceImgPath) == 0) {
		BITMAPINFO bmpInfo;
		HDC hdc;
		BITMAP bmBitmap;
		LPDWORD lpPixel;
		HDC hMemDC;
		HBITMAP hBitmap;

		bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmpInfo.bmiHeader.biWidth = iDtWidth;
		bmpInfo.bmiHeader.biHeight = iDtHeight;
		bmpInfo.bmiHeader.biPlanes = 1;
		bmpInfo.bmiHeader.biBitCount = 32;
		bmpInfo.bmiHeader.biCompression = BI_RGB;

		hdc = GetDC(hWndDesktop);
		hBitmap = CreateDIBSection(hdc, &bmpInfo, DIB_RGB_COLORS, (void**)&lpPixel, NULL, 0);
		hMemDC = CreateCompatibleDC(hdc);
		SelectObject(hMemDC, hBitmap);

		// Copy screen to DIBSection
		hdc = GetDC(hWndDesktop);
		BitBlt(hMemDC, 0, 0, iDtWidth, iDtHeight, hdc, _args.iCapturePosX, _args.iCapturePosY, SRCCOPY);
		ReleaseDC(hWndDesktop, hdc);

		GetObject(hBitmap, sizeof(BITMAP), &bmBitmap);

		// Copy Bitmap structure to IplImage
		imgCapture = cvCreateImage(cvSize(iDtWidth, iDtHeight), IPL_DEPTH_8U, 4);
		memcpy(imgCapture->imageData, bmBitmap.bmBits, iDtWidth * iDtHeight * 4);
		cvFlip(imgCapture, imgCapture, 0);

		imgSrc3ch = cvCreateImage(cvSize(iDtWidth, iDtHeight), IPL_DEPTH_8U, 3);
		// Convert capture image to 3 channel
		cvCvtColor(imgCapture, imgSrc3ch, CV_BGRA2BGR);
		DeleteDC(hMemDC);
		DeleteObject(hBitmap);
	}
	else {
		imgSrc3ch = cvLoadImage(lpSorceImgPath, CV_LOAD_IMAGE_COLOR);
	}

	dst_size = cvSize (imgSrc3ch->width - _imgTemplate->width + 1, imgSrc3ch->height - _imgTemplate->height + 1);
	imgDst = cvCreateImage (dst_size, IPL_DEPTH_32F, 1);
	cvMatchTemplate (imgSrc3ch, _imgTemplate, imgDst, CV_TM_CCOEFF_NORMED);
	cvMinMaxLoc (imgDst, &dblMinVal, &dblMaxVal, &cvminLoc, &cvmaxLoc, NULL);

	tPos.iposX = tPos.iposX + cvmaxLoc.x + _args.iCapturePosX;
	tPos.iposY = tPos.iposY + cvmaxLoc.y + _args.iCapturePosY;
	tPos.iCenterX = tPos.iposX + _imgTemplate->width / 2;
	tPos.iCenterY = tPos.iposY + _imgTemplate->height / 2;
	tPos.iWide = _imgTemplate->width;
	tPos.iHigh = _imgTemplate->height;

	iRet = (INT)(dblMaxVal * 100);
	memset(dbgMsg, 0x00, sizeof(dbgMsg));
	sprintf_s(dbgMsg, "FlaxCV:Result=%d, Pos( x=%04d, y=%04d ). TPL=%s\n", iRet, tPos.iCenterX, tPos.iCenterY, lpTemplatePath);
	::OutputDebugString( (TCHAR*)dbgMsg );

	cvReleaseImage (&imgCapture);
	cvReleaseImage (&imgSrc3ch);
	cvReleaseImage (&imgDst);
	return iRet;
}

int execMatchingTwoImages()
{
	double		dblMinVal, dblMaxVal;
	CvPoint		cvminLoc, cvmaxLoc;
	CvSize		dst_size;
	IplImage	*imgDst;
	char		dbgMsg[1024] = { '\0' };
	int			iRet;

	dst_size = cvSize(_imgSource->width - _imgTemplate->width + 1, _imgSource->height - _imgTemplate->height + 1);
	imgDst = cvCreateImage(dst_size, IPL_DEPTH_32F, 1);
	cvMatchTemplate(_imgSource, _imgTemplate, imgDst, CV_TM_CCOEFF_NORMED);
	cvMinMaxLoc(imgDst, &dblMinVal, &dblMaxVal, &cvminLoc, &cvmaxLoc, NULL);

	tPos.iposX = cvmaxLoc.x;
	tPos.iposY = cvmaxLoc.y;
	tPos.iCenterX = tPos.iposX + _imgTemplate->width / 2;
	tPos.iCenterY = tPos.iposY + _imgTemplate->height / 2;
	tPos.iWide = _imgTemplate->width;
	tPos.iHigh = _imgTemplate->height;

	iRet = (INT)(dblMaxVal * 100);
	memset(dbgMsg, 0x00, sizeof(dbgMsg));
	sprintf_s(dbgMsg, "FlaxCV:Compare Result=%d, Pos( x=%04d, y=%04d ). TPL=%s, Src=%s\n", iRet, tPos.iCenterX, tPos.iCenterY, _args.lpstrTplPath1, _args.lpstrSoucePath);
	::OutputDebugString((TCHAR*)dbgMsg);
	cvReleaseImage(&imgDst);
	return iRet;
}

int CompareHist()
{
	IplImage *src1 = 0, *src2 = 0;
	IplImage *dst1[4] = { 0, 0, 0, 0 }, *dst2[4] = { 0, 0, 0, 0 };
	CvHistogram *hist1, *hist2;
	int sch1 = 0, sch2 = 0;
	int hist_size = 256;
	float range_0[] = { 0, 256 };
	float *ranges[] = { range_0 };

	src1 = cvLoadImage(_args.lpstrTplPath1, CV_LOAD_IMAGE_COLOR);
	src2 = cvLoadImage(_args.lpstrSoucePath, CV_LOAD_IMAGE_COLOR);
	if (src1 == 0 || src2 == 0) {
		return -1;
	}

	sch1 = src1->nChannels;
	sch2 = src2->nChannels;
	if (sch1 != sch2) {
		return -1;
	}

	for (int i = 0; i<sch1; i++) {
		dst1[i] = cvCreateImage(cvSize(src1->width, src1->height), src1->depth, 1);
		dst2[i] = cvCreateImage(cvSize(src2->width, src2->height), src2->depth, 1);
	}

	hist1 = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
	hist2 = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);

	if (sch1 == 1) {
		cvCopy(src1, dst1[0], NULL);
		cvCopy(src2, dst2[0], NULL);
	}
	else {
		cvSplit(src1, dst1[0], dst1[1], dst1[2], dst1[3]);
		cvSplit(src2, dst2[0], dst2[1], dst2[2], dst2[3]);
	}
	for (int i = 0; i<sch1; i++) {
		cvCalcHist(&dst1[i], hist1, 0, NULL);
		cvCalcHist(&dst2[i], hist2, 0, NULL);
		cvNormalizeHist(hist1, 1.0);
		cvNormalizeHist(hist2, 1.0);
	}
	return (int)(cvCompareHist(hist1, hist2, CV_COMP_CORREL) * 100);
}


int doMouseAction(int actMode, int centerX, int centerY)
{
	int		ret = 0;
	char	dbgMsg[256] = {'\0'};

	switch(actMode) {
	case none:
		::OutputDebugString( (TCHAR*)"FlaxCV: Image Found! but do nothing.\n" );
		break;
	case leftClick:
		MoveMyMouse( centerX, centerY );
		myDelay(50);
		//LClickMouse();
		LClickMouse2();
		::OutputDebugString( (TCHAR*)"FlaxCV: Left Click!\n" );
		break;
	case dblClick:
		MoveMyMouse( centerX, centerY );
		myDelay(50);
		DClickMouse();
		::OutputDebugString( (TCHAR*)"FlaxCV: Double Click!\n" );
		break;
	case rightClick:
		MoveMyMouse( centerX, centerY );
		myDelay(50);
		RClickMouse();
		::OutputDebugString( (TCHAR*)"FlaxCV: Right Click!\n" );
		break;
	case mMove:
		MoveMyMouse( centerX, centerY );
		myDelay(50);
		::OutputDebugString( (TCHAR*)"FlaxCV: Move Mouse!\n" );
		break;
	default:
		sprintf_s(dbgMsg, "FlaxCV:Command '%d' is out of order.\n", actMode);
		::OutputDebugString( (TCHAR*)dbgMsg );
		ret = -1;
		break;
	}
	return ret;
}

BOOL FoundPointIsNotOnMyself(void)
{
	HWND	hWnd;
	RECT	pRect;

	hWnd = FindWindow(NULL, "Matching..");
	if (hWnd != NULL)
	{
		if ( GetWindowRect(hWnd, &pRect) ){
			if ( (tPos.iCenterX > pRect.left) && (tPos.iCenterX < pRect.right) && (tPos.iCenterY > pRect.top) && (tPos.iCenterY < pRect.bottom) )
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

int ActionOnImage(UINT   iCommandID, 
				  LPCSTR lpTemplatePath, 
				  LPCSTR lpSorceImgPath,
				  UINT   iThreshold)
{
	UINT	iMatchedLev = 0;
	int		intRet = 0;
	char	dbgMsg[256] = {'\0'};

	iMatchedLev = execMatchingImages( lpTemplatePath, lpSorceImgPath );
	intRet = (int)(iMatchedLev * pow((double)2, 24) + tPos.iCenterX * pow((double)2, 12) + tPos.iCenterY);
	if ( iMatchedLev >= iThreshold && FoundPointIsNotOnMyself() ) {
		// Mouse action
		if ( doMouseAction( iCommandID, tPos.iCenterX, tPos.iCenterY ) < 0 ) {
			intRet = ERR_COMMAND_INVALID;
		}
		else {
		}
	}
	else {
		intRet = NO_IMAGE_FOUND;
	}
	return intRet;
}


HBITMAP MakePict(HGLOBAL hGlbl,DWORD nSize) {
    IPicture*iPict; IStream*iStrm; HBITMAP hBit=0; static GUID iid=
    {0x7BF80980,0xBF32,0x101A,{0x8B,0xBB,0,0xAA,0,0x30,0xC,0xAB}};
    CreateStreamOnHGlobal(hGlbl,TRUE,&iStrm);
#ifdef __cplusplus
    if (OleLoadPicture(iStrm,nSize,0,iid,(void**)&iPict)==S_OK) {
        iPict->get_Handle((OLE_HANDLE*)&hBit);
        hBit=(HBITMAP)CopyImage(hBit,IMAGE_BITMAP,0,0,0);
    }
    iStrm->Release(); iPict->Release();
#else
    if (OleLoadPicture(iStrm,nSize,0,&iid,(void**)&iPict)==S_OK) {
        iPict->lpVtbl->get_Handle(iPict,(OLE_HANDLE*)&hBit);
        hBit=CopyImage(hBit,IMAGE_BITMAP,0,0,0);
    }
    iStrm->lpVtbl->Release(iStrm); iPict->lpVtbl->Release(iPict);
#endif
    GlobalFree(hGlbl); return hBit;
}

HBITMAP ReadPict(TCHAR*fname) {
    HANDLE hFile; HGLOBAL hGlbl; DWORD nSize,nRead; int r;
    if (fname==0||*fname==0) return 0;
    hFile=CreateFile(fname,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
    if (hFile==INVALID_HANDLE_VALUE) return 0;
    nSize=GetFileSize(hFile,0); hGlbl=GlobalAlloc(GHND,nSize);
    if (hGlbl==0) { CloseHandle(hFile); return 0; }
    r=ReadFile(hFile,GlobalLock(hGlbl),nSize,&nRead,0);
    CloseHandle(hFile);
    if (r==0) { GlobalFree(hGlbl); return 0; }
    GlobalUnlock(hGlbl); return MakePict(hGlbl,nSize);
}

HBITMAP LoadPict(TCHAR*res,TCHAR*type) {
    HRSRC hFind; HGLOBAL hLoad,hGlbl; DWORD nSize;
    HMODULE hInst=GetModuleHandle(0);
    if ((hFind=FindResource(hInst,res,type))==0) return 0;
    if ((hLoad=LoadResource(hInst,hFind))==0) return 0;
    if ((nSize=SizeofResource(hInst,hFind))==0) return 0;
    if ((hGlbl=GlobalAlloc(GHND,nSize))==0) return 0;
    CopyMemory(GlobalLock(hGlbl),LockResource(hLoad),nSize);
    GlobalUnlock(hGlbl); return MakePict(hGlbl,nSize);
}

int GetCx(HBITMAP hBit)
{ BITMAP bi; GetObject(hBit,sizeof bi,&bi); return bi.bmWidth; }

int GetCy(HBITMAP hBit)
{ BITMAP bi; GetObject(hBit,sizeof bi,&bi); return bi.bmHeight; }

DWORD DispPict(HWND hwnd,HBITMAP hBit,int x,int y) {
    BITMAP bi; int cx,cy;
    HDC hdc=GetDC(hwnd),hdc2=CreateCompatibleDC(hdc);
    GetObject(hBit,sizeof bi,&bi); cx=bi.bmWidth; cy=bi.bmHeight;
    SelectObject(hdc2,hBit); BitBlt(hdc,x,y,cx,cy,hdc2,0,0,SRCCOPY);
    DeleteDC(hdc2); ReleaseDC(hwnd,hdc); return cx+(cy<<16);
}

DWORD DispFile(HWND hwnd, TCHAR*fname, int x, int y) {
    HBITMAP hBit=ReadPict(fname); DWORD r;
    if (hBit==0) return 0;
    r=DispPict(hwnd,hBit,x,y); DeleteObject(hBit); return r;
}

DWORD DispRes(HWND hwnd,TCHAR*res,TCHAR*type,int x,int y) {
    HBITMAP hBit=LoadPict(res,type); DWORD r;
    if (hBit==0) return 0;
    r=DispPict(hwnd,hBit,x,y); DeleteObject(hBit); return r;
}


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM InitApp(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
char		szClassName[] = "layer02";
HINSTANCE	hInst;

void getArgs(void)
{
	int		i;

	// 1.Command(0-4)
	// 2.Template image path
	// 3.Matching threshold(80-100)
	// 4.Screen capture interval[ms]
	// 5.Retry
	// 6.Minimize when start
	// 7.Capture start point x
	// 8.Capture start point y
	// 9.Capture width
	// 10.Capture height
	// 11.Compared image
	if (__argc >= 10+1) {
		for ( i = 1; i < __argc; i++) {
			switch (i) {
			case 1:
				_args.iCmdID = atoi(&*__argv[i]);
				break;
			case 2:
				_args.lpstrTplPath1 = &*__argv[i];
				break;
			case 3:
				_args.iSimiler = atoi(&*__argv[i]);
				break;
			case 4:
				_args.iShotInterval = atoi(&*__argv[i]);
				break;
			case 5:
				_args.iNumberOfTimes = atoi(&*__argv[i]);
				break;
			case 6:
				_args.RunMinimize = (BOOL)atoi(&*__argv[i]);
				break;
			case 7:
				_args.iCapturePosX = atoi(&*__argv[i]);
				break;
			case 8:
				_args.iCapturePosY = atoi(&*__argv[i]);
				break;
			case 9:
				_args.iCaptureWidth = atoi(&*__argv[i]);
				break;
			case 10:
				_args.iCaptureHeight = atoi(&*__argv[i]);
				break;
			case 11:
				_args.lpstrSoucePath = &*__argv[i];
				break;
			default:
				break;
			}
		}
	}
}

int split( char *str, const char *delim, char *outlist[] ) {
    char    *tk;
    int     cnt = 0;

    tk = strtok_s( str, delim, NULL );
    while( tk != NULL && cnt < MAXARGS ) {
        outlist[cnt++] = tk;
        tk = strtok_s( NULL, delim, NULL );
    }
    return cnt;
}

void QuitProc(HWND hWnd, int h_iResult)
{
	char	dbgMsg[512] = {'\0'};

	sprintf_s(dbgMsg, "FlaxCV: Return=%d. Quit.", h_iResult);
	::OutputDebugString( (TCHAR*)dbgMsg );
	KillTimer(hWnd, ID_MYTIMER);
	DestroyWindow(hWnd);
	ExitProcess( h_iResult );
}

void FlaxCV_Main(HWND hWnd)
{
	int		iretMatching;
	UINT	iMatchedLev;
	char	dbgMsg[512] = {'\0'};

	iretMatching = ActionOnImage(_args.iCmdID, _args.lpstrTplPath1, _args.lpstrSoucePath, _args.iSimiler );
	iMatchedLev = (UINT)(iretMatching / pow((double)2, 24));
	if ( iMatchedLev >= _args.iSimiler && iMatchedLev <= 100 ) {
		// Quit if image found
		sprintf_s(dbgMsg, "FlaxCV: Image Found. (%d).", iMatchedLev);
		::OutputDebugString( (TCHAR*)dbgMsg );
		QuitProc(hWnd, iretMatching);
	}
	else if ( iretMatching == NO_IMAGE_FOUND ) {
		// Do nothing
	}
	else if ( iretMatching < 0 ) {
		QuitProc(hWnd, iretMatching);
	}
}

ATOM InitApp(HINSTANCE hInst)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));

	wc.hCursor = (HCURSOR)LoadImage(NULL,
		MAKEINTRESOURCE(IDC_ARROW),
		IMAGE_CURSOR,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szClassName;
	wc.hIconSm = (HICON)LoadImage(NULL,
		MAKEINTRESOURCE(IDI_APPLICATION),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);

	return (RegisterClassEx(&wc));
}

BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	HWND	hWnd;
	int		ret, posX, posY, wndWidth, wndHeight;

	wndWidth = 205;
	wndHeight = 140;
	posX = GetSystemMetrics(SM_CXFULLSCREEN) - wndWidth - 10;
	posY = GetSystemMetrics(SM_CYFULLSCREEN) - wndHeight + 10;

	hWnd = CreateWindowEx(WS_EX_LAYERED,
		szClassName,
		"Searching..",	// Title bar
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX &  ~WS_SIZEBOX,
		posX,
		posY,
		wndWidth,
		wndHeight,
		NULL,
		NULL,
		hInst,
		NULL);
	if (!hWnd)
		return FALSE;

	SetWindowPos(hWnd, HWND_BOTTOM, posX, posY, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
	ShowWindow(hWnd, nCmdShow);
	if (_args.iCmdID >= 5) {
		// Minimize the window
		SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
	}
	else {
		// Show the template image
		ret = DispFile(hWnd, _args.lpstrTplPath1, 0, 0);
		// Minimize the window
		if (_args.RunMinimize) {
			SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
		}
		UpdateWindow(hWnd);
	}
	return TRUE;
}

HFONT SetMyFont(HDC hdc)
{
    HFONT hFont;
    hFont = CreateFont(12,						//font height
						0,						//character width
						0,						//degree
						0,						//
						FW_BOLD,				//bold
						FALSE,					//italic
						FALSE,					//underline
						FALSE,					//
						SHIFTJIS_CHARSET,		//encoding
						OUT_DEFAULT_PRECIS,		//output accuracy
						CLIP_DEFAULT_PRECIS,	//clipping accuracy
						PROOF_QUALITY,			//quality
						FIXED_PITCH | FF_MODERN,//pitch and family
						NULL);					//font type
    return hFont;
}

static int _cnt = 0;
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	//HWND		hTargetWnd;
	HDC			hdc;
	HPEN		hPen, hOldPen;
	PAINTSTRUCT	ps;
	HFONT		hFont;
	int			ret, iTPX=4, iTPY=40;
	static int	nAlpha = 80; // 0:Clear
	static int	iTextSpan = 16;
	char		dbgMsg[512] = {'\0'};
	char		bufMsg[64] = {'\0'};
	char		posMsg[64] = {'\0'};
	static BOOL	isReady = TRUE;
	static int	lPrevCounter;

	switch (msg) {
		case WM_CREATE:
			// Set interval timer
			SetTimer(hWnd, ID_MYTIMER, 200, NULL);
			// Set initial alpha
			SetLayeredWindowAttributes(hWnd, 0, nAlpha, LWA_ALPHA);
			break;
       case WM_TIMER:
			if (wp == ID_MYTIMER) {
				if (_cnt == 0)
				{
					SetTimer(hWnd, ID_MYTIMER, _args.iShotInterval, NULL);
					_cnt++;
				}
				if ( _args.iNumberOfTimes <= 0 ) {
					// Quit if timeout
					if ( _args.iNumberOfTimes == 0 && lPrevCounter == 0 ) {
						sprintf_s(dbgMsg, "FlaxCV: MainCounter=%d, PrevCounter=%d. Logic Failed.", _args.iNumberOfTimes, lPrevCounter);
						::OutputDebugString( (TCHAR*)dbgMsg );
						QuitProc(hWnd, ERR_LOGIC_FAILED);
					}
					else {
						sprintf_s(dbgMsg, "FlaxCV: MainCounter=%d. Timeout.", _args.iNumberOfTimes);
						::OutputDebugString( (TCHAR*)dbgMsg );
						QuitProc(hWnd, TIME_OUT);
					}
				}
				// Do matching interval
				if ( isReady ) {
					FlaxCV_Main(hWnd);
				}
				lPrevCounter = _args.iNumberOfTimes;
				_args.iNumberOfTimes --;
				ZeroMemory(dbgMsg, sizeof(dbgMsg));
				//sprintf_s(dbgMsg, "FlaxCV: MainCounter=%d, PrevCounter=%d", _args.iNumberOfTimes, lPrevCounter);
				//::OutputDebugString( (TCHAR*)dbgMsg );
				// Refresh
				InvalidateRect(hWnd, NULL, FALSE);
			}
			else {
				return DefWindowProc(hWnd, msg, wp, lp);
			}

			SetLayeredWindowAttributes(hWnd, 0, (BYTE)nAlpha, LWA_ALPHA);
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// Show the template image.
			ret = DispFile(hWnd, _args.lpstrTplPath1, 0, 0);

			// Draw line to avoid matching.
			hPen = CreatePen(PS_SOLID, 4, RGB(0x27, 0x26, 0x72)); // 4px DarkBlue pen
			hOldPen = (HPEN)SelectObject(hdc, hPen);
			MoveToEx(hdc, 0, 4, NULL);							// Move to start point
			LineTo(hdc, 120, 4);
			SelectObject(hdc, hOldPen);
			DeleteObject(hPen);

			// Font setting
			hFont = SetMyFont(hdc);
			SetBkColor(hdc, RGB(0x27, 0x26, 0x72));		// DarkBlue
			SelectObject(hdc, hFont);

			// Show the capture area
			SetTextColor(hdc, RGB(0x74, 0xA9, 0xD6));	// LightBlue
			sprintf_s(posMsg, "x=%d, y=%d, w=%d, h=%d", _args.iCapturePosX, _args.iCapturePosY, _args.iCaptureWidth, _args.iCaptureHeight);
			TextOut(hdc, iTPX, iTPY, posMsg, (int)strlen(posMsg));
			// Show command type
			SetBkColor(hdc, RGB(0x27, 0x26, 0x72));		// DarkBlue
			SetTextColor(hdc, RGB(0x74, 0xA9, 0xD6));	// LightBlue
			TextOut(hdc, iTPX, iTPY+(iTextSpan * 1), gCommandText[_args.iCmdID], (int)strlen(gCommandText[_args.iCmdID]));
			
			SetTextColor(hdc, RGB(0x74, 0xA9, 0xD6));	// LightBlue
            // Similer / Interval
			sprintf_s(bufMsg, "Similer : %d / %d[ms]", _args.iSimiler, _args.iShotInterval);
			TextOut(hdc, iTPX, iTPY+(iTextSpan * 2), bufMsg, (int)strlen(bufMsg));
            
			// 
			ZeroMemory(bufMsg, sizeof(bufMsg));
			sprintf_s(bufMsg, "Time Out: %d[s]", (_args.iShotInterval / 1000) * _args.iNumberOfTimes);
			TextOut(hdc, iTPX, iTPY+(iTextSpan * 3), bufMsg, (int)strlen(bufMsg));

			SelectObject(hdc, GetStockObject(SYSTEM_FONT));		// Restore font
            DeleteObject(hFont);
			EndPaint(hWnd, &ps);
			break;
		case WM_KEYDOWN:
			if( wp == VK_F5 ) {
				// Press F5 Key is same as timeout
				QuitProc(hWnd, TIME_OUT);
			}
			break;
       case WM_CLOSE:
			KillTimer(hWnd, ID_MYTIMER);
			QuitProc(hWnd, EXIT_BY_USER);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return (DefWindowProc(hWnd, msg, wp, lp));
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst, LPSTR lpsCmdLine, int nCmdShow)
{
	MSG		msg;
	BOOL	bRet;
	char	dbgMsg[1024] = {'\0'};

	hInst = hCurInst;
	getArgs();
	
	if (!InitApp(hCurInst))
		return FALSE;
	if (!InitInstance(hCurInst, nCmdShow))
		return FALSE;

	//sprintf_s(dbgMsg, "FlaxCV: Started.'%s'\n", _args.lpstrSoucePath);
	//::OutputDebugString((TCHAR*)dbgMsg);
	if ((_imgTemplate = cvLoadImage (_args.lpstrTplPath1, CV_LOAD_IMAGE_COLOR)) == 0) {
		sprintf_s(dbgMsg, "FlaxCV: Template Image Not Found.'%s'\n", _args.lpstrTplPath1);
		::OutputDebugString( (TCHAR*)dbgMsg );
		return ERR_NO_TPLIMAGE;
	}

	if (_args.iCmdID >= 5) {
		// Image compare mode
		int intRet = 0;
		if ((_imgSource = cvLoadImage(_args.lpstrSoucePath, CV_LOAD_IMAGE_COLOR)) == 0) {
			sprintf_s(dbgMsg, "FlaxCV: Source Image Not Found.'%s'\n", _args.lpstrSoucePath);
			::OutputDebugString((TCHAR*)dbgMsg);
			return ERR_NO_SRCIMAGE;
		}
		if (_args.iCmdID == 5) {
			sprintf_s(dbgMsg, "FlaxCV: Compare started.'%s'\n", _args.lpstrSoucePath);
			::OutputDebugString((TCHAR*)dbgMsg);
			int iMatchedLev = execMatchingTwoImages();
			intRet = (int)(iMatchedLev * pow((double)2, 24) + tPos.iCenterX * pow((double)2, 12) + tPos.iCenterY);
		}
		else if (_args.iCmdID == 6) {
			sprintf_s(dbgMsg, "FlaxCV: CompareHist started.'%s'\n", _args.lpstrSoucePath);
			::OutputDebugString((TCHAR*)dbgMsg);
			int histRet = CompareHist();
			sprintf_s(dbgMsg, "FlaxCV: CompareHist Result = '%d'\n", histRet);
			::OutputDebugString((TCHAR*)dbgMsg);
			intRet = histRet * pow((double)2, 24);
		}
		cvReleaseImage(&_imgSource);
		ExitProcess(intRet);
	}
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	cvReleaseImage (&_imgTemplate);
	return (int)msg.wParam;
}
