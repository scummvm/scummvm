/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include "..\resource.h"

#include "qd_dialogs.h"

#include "qda_editor.h"

#include "gr_tile_animation.h"
#include "gdi_gr_dispatcher.h"
#include "mouse_input.h"
#include "keyboard_input.h"
#include "input_wndproc.h"

#include "ResourceDispatcher.h"
#include "FileExtAssociator.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

HINSTANCE xhInstance;

const int xDlgTextBufSize = 1024;
char* xDlgTextBuf;

grDispatcher* gr_D = 0;

qdaEditor* qda_ED;
ResourceDispatcher resD;

HWND hmainWnd;
HACCEL hAccel = NULL;

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine, int nCmdShow)
{
	xhInstance = hInstance;
	hAccel = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));

	FileExtAssociator associator;
	associator.associate(".qda");

	grDispatcher::sys_init();
	gr_D = new GDI_grDispatcher;

	qda_ED = new qdaEditor;

	hmainWnd = CreateDialog(hInstance,MAKEINTRESOURCE(IDD_QDA_EDITOR_DLG),NULL,qdlg_proc);

	qda_ED -> load_options(hmainWnd);

	if(__argc > 1){
		qda_ED -> load_animation(__argv[1]);
		qda_ED -> toggle_changed(hmainWnd,0);
		qdlg_resize(hmainWnd);
	}

	ShowWindow(hmainWnd,SW_MAXIMIZE);
	UpdateWindow(hmainWnd);

	MSG msg;

	resD.attach(new MemberFunctionCallResourceUser<qdaEditor>(*qda_ED,qdaEditor::quant,25));
	resD.start();
	
	while(GetMessage(&msg,NULL,0,0)){ 
		if(!TranslateAccelerator(hmainWnd,hAccel,&msg))
			TranslateMessage(&msg); 

		switch(msg.message){
			case WM_MOUSEMOVE:
				qdlg_mouse_move(msg.hwnd,LOWORD(msg.lParam),HIWORD(msg.lParam),msg.wParam);
				break;
		}

		DispatchMessage(&msg); 
	}
	
	grDispatcher::sys_finit();

	return 0;
}

