#ifndef __FORMTABS_H__
#define __FORMTABS_H__

typedef struct {
	FormPtr srcP;
	UInt16 first;
	UInt16 last;
} TabDataType, *TabDataPtr;

typedef struct {
	UInt16 count, active;
	Coord width;
	TabDataPtr tabs;
} TabType, *TabPtr;

typedef void (TabProc)(FormType *);

TabType	*TabNewTabs		(UInt16 cnt);
void 	 TabDeleteTabs	(TabType *tabP);
Err 	 TabAddContent	(FormType **frmP, TabType *tabP, const Char *nameP, UInt16 rscID, TabProc *preInit = 0);
void 	 TabSetActive	(FormType *frmP, TabType *tabP, UInt16 num);

#endif