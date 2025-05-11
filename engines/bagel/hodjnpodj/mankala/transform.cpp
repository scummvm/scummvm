#include <windows.h>
#include <windowsx.h>
#include <memory.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#include "trnsform.h"


/*
dft computes discrete Fourier Transform on an input u, 
and saves the result in v. The length of each record is len.
The len can't exceed 64.
*/

BOOL dft(double *v,const int *u, int len){
	int n, k;                       
	struct _complex z[64];
	
	for(k=0; k<len; k++){
		for(n=0,v[k]=0.0; n<len; n++){
			
			z[k].y+=(double)u[n]*sin(-(2.0*M_PI*n*k)/(double)len);
			z[k].x+=(double)u[n]*cos(-(2.0*M_PI*n*k)/(double)len);
		}// end for- n 
		z[k].y/=(double)len;
		z[k].x/=(double)len;
		v[k]=MAG(z[k]);
	}// end for -k                                     
	return 0;			
}                                           

#define _ACTIVATED 1           
#if _ACTIVATED
BOOL graph(HWND hWndParent, HGLOBAL hglbX, HGLOBAL hglbY, int len){
	int i;
	WNDCLASS wc;                    
	HWND hWnd;   
	WORD maxID=0, 
			minID=0,
			wIdentifier=0,
			wChildID=1;                            
	
	/*
	go thru the list of all child windows of hWndParent 
	and get an unused child Identifier, to be used to create
	another child window
	*/                                                             
	while(EnumChildWindows(hWndParent, (WNDENUMPROC) EnumChildWndProc, (LPARAM) (LPVOID)&wIdentifier)){
		maxID=max(maxID,(WORD)wIdentifier);
		minID=min(minID, (WORD) wIdentifier);                                                                                                
	}// while Enum...                          
	
	if(maxID!=USHRT_MAX-1){
		wChildID=maxID+1;
	}else{
		if(minID!=2){
			wChildID=1;    //smallest child ID
		}
		else{
			return(FALSE);
		}
	}// end if maxID
	
	
	
	/*
	create a window for drawing graph. GraphWndProc is the
	window procedure for this window.
	*/
	memset(&wc, 0x0, sizeof(WNDCLASS));
	wc.style=CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc= GraphWndProc;
	wc.cbWndExtra=10;						//extra bytes for the data handles.
	wc.hInstance=GetWindowWord(hWndParent, GWW_HINSTANCE);
	wc.hIcon=GetClassWord(hWndParent,GCW_HICON);
	wc.hbrBackground= (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszClassName="Graph";
	RegisterClass(&wc);                 
	
	hWnd=CreateWindow("Graph",
									"Plot",
									WS_VISIBLE|WS_POPUP|WS_EX_NOPARENTNOTIFY,
									0,0,100,100,
									hWndParent,
									(HMENU)wChildID,
									GetWindowWord(hWndParent,GWW_HINSTANCE),
									NULL
								);    
	#define OFFSET_X 0
	#define OFFSET_Y 2
	SetWindowWord(hWnd, OFFSET_X, (WORD)hglbX); 
	SetWindowWord(hWnd, OFFSET_Y, (WORD)hglbY); 
	
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}                                            

LRESULT _far _pascal GraphWndProc(HWND hWnd, UINT  msg, WPARAM wParam, LPARAM lParam){
	static HPEN hPen;
	HDC hdc;
	PAINTSTRUCT ps;   
	int nWindowWidth,
		nWindowHeight;
	
	switch(msg){
		case WM_CREATE:
				hPen=(HPEN) GetStockObject(BLACK_PEN);
				PostMessage(GetParent(hWnd), WM_PARENTNOTIFY, WM_CREATE, MAKELPARAM(0,GetWindowWord(hWnd, GWW_ID)));
				return 0L;

		case WM_PAINT:
				//y=(double _far*)GlobalLock((HGLOBAL) GetWindowWord(hWnd, OFFSET_Y));
				//x=(double _far*)GlobalLock((HGLOBAL) GetWindowWord(hWnd, OFFSET_X));
				
				hdc=BeginPaint(hWnd,&ps);   
				SelectPen(hdc, hPen);
				nWindowWidth=LOWORD(GetWindowExt(hdc));
				nWindowHeight=HIWORD(GetWindowExt(hdc));
				
				MoveTo(hdc,nWindowWidth>>3, nWindowHeight>>1);		//x Axis
				LineTo(hdc,(nWindowWidth*7)>>3, nWindowHeight>>1);
				
				MoveTo(hdc,nWindowWidth>>1, nWindowHeight>>3);		//y Axis
				LineTo(hdc,nWindowWidth>>1, (nWindowHeight*7)>>3);
				
				//GlobalUnlock((HGLOBAL) GetWindowWord(hWnd, OFFSET_Y));
				//GlobalUnlock((HGLOBAL) GetWindowWord(hWnd, OFFSET_X));
				return 0L;                
		case WM_CLOSE: 
				PostMessage((HWND)GetParent(hWnd), WM_PARENTNOTIFY, WM_DESTROY, MAKELPARAM(0,GetWindowWord(hWnd, GWW_ID)));
				DestroyWindow(hWnd);
				return 0L;
		default:
				break;
	}			
	return(DefWindowProc(hWnd, msg, wParam, lParam));
}           

BOOL _far _pascal EnumChildWndProc(HWND hWnd, LPARAM lParam){
	WORD _far* lpID;

	lpID=(WORD _far*)(LPVOID)lParam;
	*lpID=GetWindowWord(hWnd,GWW_ID);
	return(TRUE);
}
#endif
	
									
									
							
	
	