#ifndef __FORMTABS_H__
#define __FORMTABS_H__

typedef void (TabProc)();

typedef struct {
	FormPtr srcP;
	UInt16 first;
	UInt16 last;
	TabProc *drawFunc;
} TabDataType, *TabDataPtr;

typedef struct {
	UInt16 count, active;
	Coord width;
	TabDataPtr tabs;
} TabType, *TabPtr;

TabType	*TabNewTabs		(UInt16 cnt);
void	 TabDeleteTabs	(TabType *tabP);
Err		 TabAddContent	(FormType **frmP, TabType *tabP, const Char *nameP, UInt16 rscID, TabProc *drawFunc = 0);
void	 TabSetActive	(FormType *frmP, TabType *tabP, UInt16 num);
void	TabMoveUpObject	(FormType *frmP, UInt16 objID, Coord amount);
#endif
