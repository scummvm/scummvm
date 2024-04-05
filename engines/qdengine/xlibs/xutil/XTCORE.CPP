/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "xglobal.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */

struct xtMsgHandlerObject
{
	int ID;

	void (*Handler)(MSG*);

	void* list;
	xtMsgHandlerObject* next;
	xtMsgHandlerObject* prev;

	xtMsgHandlerObject(void (*p)(MSG*),int id);
};

/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */

void xtSetExit(void);
int xtNeedExit(void);

int xtCallXKey(MSG* m);
void xtSysQuant(void);

void xtAddSysObj(XList* lstPtr,void (*fPtr)(void),int id);
void xtDeleteSysObj(XList* lstPtr,int id);
void xtDeactivateSysObj(XList* lstPtr,int id);

void xtReadConsoleInput(void);

void xtRegisterSysMsgFnc(void (*fPtr)(MSG*),int id);
void xtRegisterSysFinitFnc(void (*fPtr)(void),int id);
void xtUnRegisterSysFinitFnc(int id);
void xtDeactivateSysFinitFnc(int id);
void xtSysFinit(void);

void xtPostMessage(HANDLE hWnd,int msg,int wp,int lp);
int xtDispatchMessage(MSG* msg);
void xtProcessMessageBuffer(void);

/* --------------------------- DEFINITION SECTION --------------------------- */

//#define _RTO_LOG_

typedef void (*XFNC)();

#define XT_DEFAULT_TABLE_SIZE	32

XRuntimeObject** XRObjTable = NULL;
int XRObjTableSize = 0;

XRuntimeObject* XRObjFirst = NULL;
XRuntimeObject* XRObjLast = NULL;

char* XToolClassName = "XToolClass";
char* XToolWndName = "XToolWindow";

XConsole XCon;

void* XAppHinst = NULL;
void* XGR_hWnd = NULL;

void* hXConOutput = NULL;
void* hXConInput = NULL;

int XAppMode = 0;

XList XSysQuantLst;
XList XSysFinitLst;
xtList<xtMsgHandlerObject> XSysHandlerLst;

static HANDLE hXActiveWndEvent = 0;
static HANDLE hXNeedExitEvent = 0;

#ifdef _RTO_LOG_
XStream xtRTO_Log;
#endif

int xtSysQuantDisabled = 0;

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{
	int id,prevID,clockDelta,clockCnt,clockNow;

	XRuntimeObject* XObj;

	XAppHinst = hInst;
	XAppMode = sw;
/*
	if(sw != SW_HIDE)
		XCon.initialize(sw);
*/
	XMsgBuf = new XMessageBuffer;

	hXActiveWndEvent = CreateEvent(0,TRUE,TRUE,0);
	hXNeedExitEvent = CreateEvent(0,TRUE,FALSE,0);
	if(!hXActiveWndEvent || !hXNeedExitEvent){
		ErrH.Abort("Can't create control events...");
	}

	ResetEvent(hXNeedExitEvent);

	xt_get_cpuid();
	xt_getMMXstatus();
	initclock();

	prevID = 0;
	id = xtInitApplication();
	XObj = xtGetRuntimeObject(id);
#ifdef _RTO_LOG_
	if(XRec.flags & XRC_PLAY_MODE)
		xtRTO_Log.open("xt_rto_p.log",XS_OUT);
	else
		xtRTO_Log.open("xt_rto_w.log",XS_OUT);
#endif
	while(XObj){
		XObj -> Init(prevID);
		prevID = id;

		id = 0;

		clockCnt = clocki();
		while(!id){
			if(XObj -> Timer){
				clockNow = clocki();
				clockDelta =  clockNow - clockCnt;
				if(clockDelta >= XObj -> Timer){
					clockCnt = clockNow - (clockDelta - XObj -> Timer) % XObj -> Timer;
					id = XObj -> Quant();
				}
			}
			else 
				id = XObj -> Quant();

			if(!xtSysQuantDisabled)
				XRec.Quant(); // впускает внешние события, записывает их или воспроизводит

			if(xtNeedExit()) ErrH.Exit();
		}

		XObj -> Finit();
#ifdef _RTO_LOG_
		xtRTO_Log < "\r\nChange RTO: " <= XObj -> ID < " -> " <= id < " frame -> " <= XRec.frameCount;
#endif
		XObj = xtGetRuntimeObject(id);
	}
	xtDoneApplication();
	xtSysFinit();

	CloseHandle(hXActiveWndEvent);
	CloseHandle(hXNeedExitEvent);

#ifdef _RTO_LOG_
	xtRTO_Log.close();
#endif
	return 0;
}

void xtCreateRuntimeObjectTable(int len)
{
	int i;
	if(!len) len = XT_DEFAULT_TABLE_SIZE;
	XRObjTableSize = len;
	XRObjTable = new XRuntimeObject*[len];

	for(i = 0; i < len; i ++){
		XRObjTable[i] = NULL;
	}
}

XRuntimeObject* xtGetRuntimeObject(int id)
{
	if(id == XT_TERMINATE_ID) return NULL;
	if(!XRObjTable || !XRObjTableSize || id < 1 || id > XRObjTableSize)
		ErrH.Abort("XTool system error...");
	return XRObjTable[id - 1];
}

void xtRegisterRuntimeObject(XRuntimeObject* p)
{
	if(!XRObjFirst){
		XRObjFirst = XRObjLast = p;
	}
	else {
		XRObjLast -> next = p;
		XRObjLast = p;
	}
	XRObjTable[p -> ID - 1] = p;
}

int xtCallXKey(MSG* m)
{
	int rec_flag = 0,ret = 0;
	switch(m -> message){
		case WM_CHAR:
		case WM_SYSCHAR:
			XKey.LastChar = m -> wParam;
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			XKey.PressFnc((int)m -> wParam,m -> lParam);
			rec_flag = 1;

			if((int)m -> wParam == VK_OEM_ALT || (int)m -> wParam == VK_F10) ret = 1;
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			XKey.UnPressFnc((int)m -> wParam,m -> lParam);
			rec_flag = 1;

			if((int)m -> wParam == VK_OEM_ALT || (int)m -> wParam == VK_F10) ret = 1;
			break;
	}
	if(rec_flag && XRec.flags & XRC_RECORD_MODE){
		XRec.PutSysMessage(XRC_SYSTEM_MESSAGE,m -> message,m -> wParam,m -> lParam);
	}

	return ret;
}

XList::XList(void)
{
	ClearList();
}

XList::~XList(void)
{
}

void XList::AddElement(XListElement* p)
{
	if(!fPtr){
		fPtr = lPtr = p;
		p -> prev = p;
		p -> next = NULL;
	}
	else {
		lPtr -> next = p;
		p -> prev = lPtr;
		p -> next = NULL;
		lPtr = p;
		fPtr -> prev = p;
	}
	ListSize ++;
}

void XList::RemoveElement(XListElement* p)
{
	XListElement* pPtr,*nPtr;

	ListSize --;

	if(ListSize){
		pPtr = p -> prev;
		nPtr = p -> next;

		pPtr -> next = nPtr;
		if(nPtr) nPtr -> prev = pPtr;

		if(p == fPtr) fPtr = nPtr;
		if(p == lPtr) lPtr = pPtr;

		lPtr -> next = NULL;
		fPtr -> prev = lPtr;
	}
	else
		ClearList();
}

void xtRegisterSysQuant(void (*qPtr)(void),int id)
{
	xtAddSysObj(&XSysQuantLst,qPtr,id);
}

void xtUnRegisterSysQuant(int id)
{
	xtDeleteSysObj(&XSysQuantLst,id);
}

void xtRegisterSysFinitFnc(void (*fPtr)(void),int id)
{
	xtAddSysObj(&XSysFinitLst,fPtr,id);
}

void xtDeactivateSysFinitFnc(int id)
{
	xtDeactivateSysObj(&XSysFinitLst,id);
}

void xtUnRegisterSysFinitFnc(int id)
{
	xtDeleteSysObj(&XSysFinitLst,id);
}

void xtDeleteSysObj(XList* lstPtr,int id)
{
	XSysObject* p = (XSysObject*)lstPtr -> fPtr;
	while(p){
		if(p -> ID == id){
			lstPtr -> RemoveElement((XListElement*)p);
			delete p;
			return;
		}
		p = (XSysObject*)p -> next;
	}
}

void xtDeactivateSysObj(XList* lstPtr,int id)
{
	XSysObject* p = (XSysObject*)lstPtr -> fPtr;

	while(p){
		if(p -> ID == id)
			p -> flags |= XSYS_OBJ_INACTIVE;
		p = (XSysObject*)p -> next;
	}
}

void xtAddSysObj(XList* lstPtr,void (*fPtr)(void),int id)
{
	XSysObject* p = (XSysObject*)lstPtr -> fPtr;

	while(p){
		if(p -> ID == id) return;
		p = (XSysObject*)p -> next;
	}

	p = new XSysObject;
	p -> ID = id;
	p -> QuantPtr = fPtr;

	lstPtr -> AddElement((XListElement*)p);
}

void xtSysQuant(void)
{
	XSysObject* p = (XSysObject*)XSysQuantLst.fPtr;
	while(p){
		(*(XFNC)(p -> QuantPtr))();
		p = (XSysObject*)p -> next;
	}
}

void xtSysFinit(void)
{
	int i,sz = XSysFinitLst.ListSize;
	XSysObject* p = (XSysObject*)XSysFinitLst.lPtr;
	for(i = 0; i < sz; i ++){
		if(!(p -> flags & XSYS_OBJ_INACTIVE))
			(*(XFNC)(p -> QuantPtr))();
		p = (XSysObject*)p -> prev;
	}
	XRec.Close();
}

int xtIsActive(void)
{
	return (WAIT_OBJECT_0 == WaitForSingleObject(hXActiveWndEvent,0)) ? 1 : 0;
};

int xtNeedExit()
{
	return (WAIT_OBJECT_0 == WaitForSingleObject(hXNeedExitEvent, 0)) ? 1 : 0;
};

void xtSetExit()
{
	ResetEvent(hXActiveWndEvent);
	SetEvent(hXNeedExitEvent);
};

int xtDispatchMessage(MSG* msg)
{
	int ret = 0;

	xtMsgHandlerObject* p = XSysHandlerLst.first();
	while(p){
		(*p -> Handler)(msg);
		p = p -> next;
	}

	ret += xtCallXKey(msg);
	switch(msg -> message){
		case WM_ACTIVATEAPP:
			if(FALSE == BOOL(msg -> wParam)){
				ResetEvent(hXActiveWndEvent);
			} else {
				XKey.clear();
				SetEvent(hXActiveWndEvent);
			}
			break;
		case WM_QUIT:
			xtSetExit();
			break;
	}

	return ret;
}

void xtClearMessageQueue(void)
{
	MSG msg;
	xtReadConsoleInput();
	while(PeekMessage(&msg,0,0,0,PM_REMOVE)){
		TranslateMessage(&msg);
		if(XRec.CheckMessage(msg.message)){
			if(!xtDispatchMessage(&msg))
				DispatchMessage(&msg);
		}
		else
			XMsgBuf -> put(&msg);
	}
}

void xtProcessMessageBuffer(void)
{
	MSG msg;
	while(XMsgBuf -> get(&msg)){
		if(!(XRec.flags & XRC_PLAY_MODE) || XRec.CheckMessage(msg.message)){
			if(!xtDispatchMessage(&msg))
				DispatchMessage(&msg);
		}
	}
}

void xtReadConsoleInput(void)
{
	unsigned i;
	INPUT_RECORD rec;
	KEY_EVENT_RECORD* key_rec;
	unsigned long num_msg = 0,num_read = 0;

	GetNumberOfConsoleInputEvents(hXConInput,&num_msg);
	for(i = 0; i < num_msg; i ++){
		ReadConsoleInput(hXConInput,&rec,1,&num_read);
		if(rec.EventType == KEY_EVENT){
			key_rec = &rec.Event.KeyEvent;
			key_rec -> wVirtualScanCode &= ~0xFFFF;
			key_rec -> wVirtualScanCode |= (key_rec -> wRepeatCount & 0xFFFF);

			if(key_rec -> bKeyDown == TRUE)
				xtPostMessage(NULL,WM_KEYDOWN,key_rec -> wVirtualKeyCode,key_rec -> wVirtualScanCode);
			else
				xtPostMessage(NULL,WM_KEYUP,key_rec -> wVirtualKeyCode,key_rec -> wVirtualScanCode);
		}
	}
}

void xtPostMessage(HANDLE hWnd,int msg,int wp,int lp)
{
	if(XMsgBuf){
		XMsgBuf -> put(hWnd,msg,wp,lp);
	}
}

xtMsgHandlerObject::xtMsgHandlerObject(void (*p)(MSG*),int id)
{
	list = NULL;
	ID = id;

	Handler = p;
}

void xtRegisterSysMsgFnc(void (*fPtr)(MSG*),int id)
{
	xtMsgHandlerObject* p = new xtMsgHandlerObject(fPtr,id);
	XSysHandlerLst.append(p);
}

void win32_break(char* error,char* msg)
{
	XCon < "--------------------------------\n";
	XCon < error < "\n";
	XCon < msg < "\n";
	XCon < "--------------------------------\n";
	_ASSERT(FALSE) ;
}

void* xtGet_hInstance(void)
{
	return XAppHinst;
}

void* xtGet_hWnd(void)
{
	return XGR_hWnd;
}

void xtSet_hWnd(void* hWnd)
{
	XGR_hWnd = hWnd;
}

HWND XDummy_hWnd = NULL;

void xtCreateDummyWindow(int x,int y,int sx,int sy)
{
	WNDCLASS wc;

	if(XDummy_hWnd) ErrH.Abort("Dummy window already opened...");

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = (HINSTANCE)XAppHinst;
	wc.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "XDummyClass";
	if(!RegisterClass(&wc)) ErrH.Abort("RegisterClass() failed",XERR_USER,GetLastError());

	XDummy_hWnd = CreateWindow(
			"XDummyClass",                  //  (lpszClassName) address of registered class name
			"XDummyWindow",                 //  (lppszWindowName) address of window name
			WS_CAPTION	|	 /* Title and Min/Max		*/
			WS_SYSMENU	|	 /* Add system menu box 	*/
			WS_MINIMIZEBOX	|	 /* Add minimize box		*/
			WS_MAXIMIZEBOX	|	 /* Add maximize box		*/
			WS_BORDER	|	 /* thin frame			*/
			WS_CLIPCHILDREN |	  /* don't draw in child windows areas */
			WS_OVERLAPPED,
			x,				//  (x) horizontal position of window
			y,				//  (y) vertical position of window
			sx,				//  (nWidth) window width
			sy,				//  (nHeight) window height
			NULL,				//  (hwndParent) handle of parent or owner window
			NULL,				//  (menu) handle of menu or child-window identifier
			(HINSTANCE)XAppHinst,			//  (hinst) handle of application instance
			NULL				//  (lpvParam) address of window-creation data
	);
	if(!XDummy_hWnd) ErrH.Abort("Can't create dummy window...");

	ShowWindow(XDummy_hWnd,XAppMode);
	UpdateWindow(XDummy_hWnd);
}

void xtCloseDummyWindow(void)
{
	if(XDummy_hWnd){
		CloseWindow(XDummy_hWnd);
		DestroyWindow(XDummy_hWnd);

		XDummy_hWnd = NULL;

		UnregisterClass("XDummyClass",(HINSTANCE)XAppHinst);
	}
}

void xtSysQuantDisable(int v)
{
	xtSysQuantDisabled = v;
}

