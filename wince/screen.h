/* (C) 2001  Vasyl Tsvirkunov */

#ifndef SCREEN_H
#define SCREEN_H

#ifndef UBYTE
#define UBYTE unsigned char
#endif

void SetScreenGeometry(int w, int h);
int GraphicsOn(HWND hWndMain);
void GraphicsOff();
void GraphicsSuspend();
void GraphicsResume();

void SetPalEntry(int ent, UBYTE r, UBYTE g, UBYTE b);
void Blt(UBYTE * scr_ptr);

/* meaning: 0 - portrait, 1 - left hand landscape, 2 - right hand landscape */
void SetScreenMode(int mode);
int GetScreenMode();

void Translate(int* x, int* y);

typedef enum ToolbarSelected {
	ToolbarNone,
	ToolbarSaveLoad,
	ToolbarExit,
	ToolbarSkip,
	ToolbarSound
} ToolbarSelected;
	

#endif
