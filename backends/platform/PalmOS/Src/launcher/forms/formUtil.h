#ifndef __FORMUTIL_H__
#define __FORMUTIL_H__

#define frmRedrawUpdateMS		(frmRedrawUpdateCode + 1)
#define frmRedrawUpdateMSImport	(frmRedrawUpdateCode + 2)

#define NO_ENGINE	-1

// form list draw
#define	ITEM_TYPE_UNKNOWN	'U'
#define ITEM_TYPE_CARD		'C'
#define ITEM_TYPE_SKIN		'S'

extern Char **itemsText;
extern void  *itemsList;
extern Char   itemsType;

void FrmReturnToMain(UInt16 updateCode = frmRedrawUpdateMS);
void * GetObjectPtr(UInt16 objectID);
void FldTrimText(FieldPtr fldP);
void FrmList(EventPtr eventP, UInt16 objectID);

#endif
