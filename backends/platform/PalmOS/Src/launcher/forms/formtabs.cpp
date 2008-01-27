#include <PalmOS.h>
#include <CtlGlue.h>
#include <FrmGlue.h>
#include <LstGlue.h>

#include "globals.h"
#include "formTabs.h"

TabType *TabNewTabs(UInt16 cnt) {
	TabType *newP = new TabType;

	newP->count = 0;
	newP->width = 0;
	newP->tabs = new TabDataType[cnt];

	return newP;
}

void TabMoveUpObject(FormType *frmP, UInt16 objID, Coord amount) {
	Coord x, y;
	FrmGetObjectPosition(frmP, FrmGetObjectIndex(frmP, objID), &x, &y);
	FrmSetObjectPosition(frmP, FrmGetObjectIndex(frmP, objID), x, y - amount);
}

void TabDeleteTabs(TabType *tabP) {
	UInt16 cnt, num;
	num = MemPtrSize(tabP->tabs) / sizeof(TabDataType);

	for (cnt = 0; cnt < num; cnt++)
		FrmDeleteForm(tabP->tabs[cnt].srcP);

	delete tabP->tabs;
	delete tabP;
}

Err TabAddContent(FormType **frmP, TabType *tabP, const Char *nameP, UInt16 rscID, TabProc *drawFunc) {
	FormType *srcP;
	UInt16 cnt;
	void *objP, **dstP;
	UInt16 objNum;
	Coord x, y, w, h;
	UInt16 id;
	RectangleType r;

	dstP = (void **)frmP;
	srcP = FrmInitForm(rscID);

	objNum = FrmGetNumberOfObjects(srcP);

	// save tab data

	// it's required to keep to source forms active
	// while the tab form is not close for list data ptr (items text)
	// TODO : fix this !
	tabP->tabs[tabP->count].srcP	= srcP;
	tabP->tabs[tabP->count].first	= FrmGetObjectId(srcP, 0);
	tabP->tabs[tabP->count].last	= FrmGetObjectId(srcP, objNum - 1);
	tabP->tabs[tabP->count].drawFunc= drawFunc;
	tabP->count++;
	tabP->active = tabP->count;

	// create tab
	FntSetFont(stdFont);
	x = 4 + tabP->width;
	y = 16;
	w = FntCharsWidth(nameP, StrLen(nameP)) + 6;
	h = 12;
	ControlType *addP = CtlNewControl(dstP, (FrmGetFormId(*frmP) + tabP->count), buttonCtl, nameP, x, y, w, h, stdFont, 0, true);
	CtlGlueSetFrameStyle(addP, noButtonFrame);
	tabP->width += w + 3;

	// create tab content
	for (cnt = 0; cnt < objNum; cnt++) {
		objP = FrmGetObjectPtr(srcP, cnt);
		id = FrmGetObjectId(srcP, cnt);
		FrmGetObjectBounds(srcP, cnt, &r);
		x = r.topLeft.x - 2;
		y = r.topLeft.y + 30;
		w = r.extent.x;
		h = r.extent.y;

		FrmSetObjectPosition(srcP, cnt, x, y);

		switch (FrmGetObjectType(srcP, cnt)) {
			case frmListObj: {
				// HOWTO : to auto link the list to a popup trigger the listID must be popupID + 1 if popup id
				// desn't exist the triggerID will be 0 and the list will be shown, but take care to not define
				// ListID - 1 object if you want to show the list.
				ListType *newP;
				Char **itemsP = LstGlueGetItemsText((ListType *)objP);
				FontID font = LstGlueGetFont((ListType *)objP);
				UInt16 visible = LstGetVisibleItems((ListType *)objP);
				UInt16 items = LstGetNumberOfItems((ListType *)objP);
				UInt16 trigger = id - 1;
				trigger = (FrmGetObjectIndex((FormType *)*dstP, trigger) != frmInvalidObjectId) ? trigger : 0;

				LstNewList(dstP, id, x, y, w, h, font, visible, trigger);
				newP = (ListType *)FrmGetObjectPtr((FormType *)*dstP, FrmGetObjectIndex((FormType *)*dstP, id));
				LstSetListChoices(newP, itemsP, items);
				FrmHideObject((FormType *)*dstP, FrmGetObjectIndex((FormType *)*dstP, id));
				break;
			}
			case frmFieldObj: {
				FieldAttrType attr;
				FontID font = FldGetFont((FieldType *)objP);
				UInt16 maxChars = FldGetMaxChars((FieldType *)objP);
				FldGetAttributes((FieldType *)objP, &attr);
				FldNewField(dstP, id, x, y, w, h, font, maxChars, attr.editable, attr.underlined, attr.singleLine, attr.dynamicSize, (JustificationType)attr.justification, attr.autoShift, attr.hasScrollBar, attr.numeric);
				FrmHideObject((FormType *)*dstP, FrmGetObjectIndex((FormType *)*dstP, id));
				break;
			}
			case frmControlObj: {
				const Char *textP = CtlGetLabel((ControlType *)objP);
				ControlStyleType style = CtlGlueGetControlStyle((ControlType *)objP);
				FontID font = CtlGlueGetFont((ControlType *)objP);
				ControlType *newP = CtlNewControl(dstP, id, style, textP, x, y, w, h, font, 0, true);
				FrmHideObject((FormType *)*dstP, FrmGetObjectIndex((FormType *)*dstP, id));
				switch (style) {
					case sliderCtl:
					{
							UInt16 min, max, page, val;
							CtlGetSliderValues((ControlType *)objP, &min, &max, &page, &val);
							CtlSetSliderValues(newP, &min, &max, &page, &val);
							break;
					}
				}
				break;
			}
			case frmLabelObj: {
				const Char *textP = FrmGetLabel(srcP, id);
				FontID font = FrmGlueGetLabelFont(srcP, id);
				FrmNewLabel((FormType **)dstP, id, textP, x, y, font);
				FrmHideObject((FormType *)*dstP, FrmGetObjectIndex((FormType *)*dstP, id));
				break;
			}

			case frmBitmapObj: {
/*				UInt16 family = id + 1;
				FrmNewBitmap((FormType **)dstP, id, id, x, y);
				//FrmHideObject((FormType *)*dstP, FrmGetObjectIndex((FormType *)*dstP, id));
*/				break;
			}
		}
	}

	frmP = (FormType **)dstP;	// save new form ptr
	return errNone;
}

void TabSetActive(FormType *frmP, TabType *tabP, UInt16 num) {
	UInt16 cnt, idx;
	RectangleType r;
	TabDataPtr dataP;

	if (tabP->active == num)
		return;

	dataP = &tabP->tabs[tabP->active];

	// hide active tab objects
	if (tabP->active != tabP->count) {
		for (cnt = dataP->first; cnt <= dataP->last; cnt++) {
			if ((idx = FrmGetObjectIndex(frmP, cnt)) != frmInvalidObjectId)
				FrmHideObject(frmP, idx);
		}
	}

	// save active tab
	tabP->active = num;
	dataP = &tabP->tabs[num];

	// draw tab limit
	WinSetForeColor(UIColorGetTableEntryIndex(UIFormFill));
	WinDrawLine(1, 14, 154,14);
	WinDrawLine(1, 15, 154,15);
	RctSetRectangle(&r, 1, 30, 154, 100);
	WinDrawRectangle(&r, 0);
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectFrame));
	WinDrawLine(1, 28, 154,28);

	// set tabs size
	for (cnt = 0; cnt < tabP->count; cnt++) {
		idx = FrmGetObjectIndex (frmP, (FrmGetFormId(frmP) + cnt + 1));

		if (idx != frmInvalidObjectId) {
			FrmGetObjectBounds(frmP, idx, &r);
			r.topLeft.y	= (num == cnt) ? 17 : 17;
			r.extent.y	= (num == cnt) ? 12 : 11;
			FrmSetObjectBounds(frmP, idx, &r);

			if (num == cnt) {
				RGBColorType yellow = {0,255,192,0};

				UInt8 line = /*(UIColorGetTableEntryIndex(UIFormFill) == UIColorGetTableEntryIndex(UIFieldTextHighlightBackground)) ?
							WinRGBToIndex(&yellow) :*/
							UIColorGetTableEntryIndex(UIFieldTextHighlightBackground);

				r.topLeft.y	-= 1;
				WinSetForeColor(UIColorGetTableEntryIndex(UIObjectFrame));
				WinDrawRectangleFrame(simpleFrame, &r);
				WinSetForeColor(line);
				WinDrawLine(r.topLeft.x, r.topLeft.y, r.topLeft.x + r.extent.x - 1, r.topLeft.y);
				FrmShowObject(frmP, idx);

			} else {
				UInt8 frame = UIColorGetTableEntryIndex(UIObjectFrame);
				RGBColorType light;
				WinIndexToRGB(frame, &light);
				light.r = (255 - light.r) > 72 ? light.r + 72 : 255;
				light.g = (255 - light.g) > 72 ? light.g + 72 : 255;
				light.b = (255 - light.b) > 72 ? light.b + 72 : 255;

				WinSetForeColor(WinRGBToIndex(&light));
				WinDrawRectangleFrame(simpleFrame, &r);
				WinSetForeColor(frame);
				WinDrawLine(r.topLeft.x - 1, r.topLeft.y + r.extent.y, r.topLeft.x + r.extent.x, r.topLeft.y + r.extent.y);
			}

			// round corner
			WinSetForeColor(UIColorGetTableEntryIndex(UIFormFill));
			if (OPTIONS_TST(kOptModeHiDensity)) {
				WinSetCoordinateSystem(kCoordinatesNative);
				WinDrawPixel((r.topLeft.x - 1) * 2, (r.topLeft.y - 1) * 2);
				WinDrawPixel((r.topLeft.x + r.extent.x) * 2 + 1, (r.topLeft.y - 1) * 2);
				WinSetCoordinateSystem(kCoordinatesStandard);

			} else {
				// TODO
			}
		}
	}

	// show objects
	for (cnt = dataP->first; cnt <= dataP->last; cnt++) {
		if ((idx = FrmGetObjectIndex(frmP, cnt)) != frmInvalidObjectId)
			FrmShowObject(frmP, idx);
	}

	// post draw function
	if (dataP->drawFunc)
		(dataP->drawFunc)();
}
