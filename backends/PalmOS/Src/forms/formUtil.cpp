#include <PalmOS.h>
#include <TxtGlue.h>

#include "forms.h"

Char **itemsText = NULL;
void  *itemsList = NULL;
Char   itemsType = ITEM_TYPE_UNKNOWN;

void FrmReturnToMain(UInt16 updateCode) {
	// if there is a form loaded, prevent crash on OS5
	if (FrmGetFirstForm()) {
		FrmUpdateForm(MainForm, updateCode);
		FrmReturnToForm(MainForm);
	}
}

void FldTrimText(FieldPtr fldP) {
	MemHandle tmpH;
	Char *tmpP;

	tmpH = FldGetTextHandle(fldP);
	FldSetTextHandle(fldP, NULL);
	tmpP = (Char *)MemHandleLock(tmpH);
	TxtGlueStripSpaces(tmpP, true, true);
	MemHandleUnlock(tmpH);
	FldSetTextHandle(fldP, tmpH);
}

/***********************************************************************
 *
 * FUNCTION:    GetObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object in the current
 *              form.
 *
 * PARAMETERS:  formId - id of the form to display
 *
 * RETURNED:    void *
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
void * GetObjectPtr(UInt16 objectID) {
	FormPtr frmP;

	frmP = FrmGetActiveForm();
	return FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID));
}

void FrmList(EventPtr eventP, UInt16 objectID) {
	ListType *listP;
	UInt16 listItem;

	listP = (ListType *)GetObjectPtr(objectID);
	listItem = LstPopupList(listP);
	CtlSetLabel(eventP->data.ctlSelect.pControl, LstGetSelectionText(listP, LstGetSelection(listP)));
}

