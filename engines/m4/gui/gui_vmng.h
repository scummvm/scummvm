
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef M4_GUI_GUI_VMNG_H
#define M4_GUI_GUI_VMNG_H

#include "m4/m4_types.h"
#include "m4/gui/gui_univ.h"

namespace M4 {

enum {
	SCRN_DLG = 0, SCRN_BUF, SCRN_TEXT, SCRN_TRANS
};
enum {
	SCRN_ANY = 0, SCRN_ACTIVE, SCRN_INACTIVE, SCRN_UNDEFN
};

#define SF_LAYER       0x000f
#define SF_BACKGRND    0x0000
#define SF_DRIFTER     0x0001
#define SF_FLOATER     0x0002
#define SF_SURFACE     0x0003
#define SF_MOUSE       0x000e

#define SF_GET_NONE    0x0000
#define SF_GET_KEY     0x0010
#define SF_GET_MOUSE   0x0020
#define SF_GET_ALL     0x0030

#define SF_BLOCK_NONE  0x0000
#define SF_BLOCK_KEY   0x0040
#define SF_BLOCK_MOUSE 0x0080
#define SF_BLOCK_ALL   0x00c0

#define SF_IMMOVABLE    0x0100		// if set, it ain't draggable
#define SF_OFFSCRN		0x0200		// can be dragged off screen, if it's draggable
#define SF_TRANSPARENT	0x0400		// if the screen is transparent

#define SF_DEFAULT	SF_DRIFTER | SF_GET_ALL | SF_BLOCK_NONE
#define SF_ALERT	SF_FLOATER | SF_GET_ALL | SF_BLOCK_ALL

/**
 * To initialize the GUI view manager.
 * @returns		The success of the call
 * @remarks		Should be called once during program initialization,
 *				after dpmi_init_mem() has been called. 
 */
extern bool vmng_init();

/**
 * Shutdown the GUI view manager, and release all resources.
 *
 * Since applications can be written in which Dialogs are designed, created, and
 * Their callback procedures are executed by the view manager, it is not always
 * necessary for the application to retain the (Dialog*)s which it created.
 * Therefore, Any windows which contain a (Dialog*) and still exist also destroy
 * The Dialog for which they were created.  The same goes for (TextScrn*)s.
 */
void vmng_shutdown();

/**
 * Create a new window for the identifier given (scrnContent).
 * @param x1	Top left x
 * @param y1	Top left y
 * @param x2	Bottom right x
 * @param y2	Bottom right y
 * @param scrnType	One of: SCRN_DLG (Dialog), SCRN_BUF (Buffer),
 * SCRN_TEXT (TextScrn). to identify scrnContent type.
 * @param		See the overview at the top of this file for an explanation of scrnFlags.
 * @param scrnContent	The identifier for the new window.
 * @param redraw		The function pointer which will be called when the view
 * manager requests the window to redraw a portion of itself.
 * @param evtHandler	The function pointer call when the view manager
 * determines that an event should be processed by this window.
 * @remarks If TRUE is returned, the window was successfully created, and is currently
 * in the list of inactive windows. A call to vmng_screen_show() will activate
 * (make visible) this window.
 */
extern ScreenContext *vmng_screen_create(int32 x1, int32 y1, int32 x2, int32 y2, int32 scrnType, uint32 scrnFlags,
	void *scrnContent, RefreshFunc redraw, EventHandler evtHandler);

/**
 * Return the ScreenContext * associated with a window, and to determine whether it is active,
 * exists but is inactive (invisible, no events are received or blocked), or is undefined
 * (does not exist).
 * @param scrnContent	The window identifier.
 * @param status		If not NULL, the status: SCRN_ACTIVE, SCRN_INACTIVE, or SCRN_UNDEFN
 * is recoreded here.
 * @returns		the ScreenContext* associated with the window which was created for
 * the structure scrnContent. returns NULL if now window was found.
 */
extern ScreenContext *vmng_screen_find(void *scrnContent, int32 *status);	// was FindScreen

/**
 * Remove a window from the active list, and place it on the inactive list
 * @param		The window identifier
 */
extern void vmng_screen_hide(void *scrnContent); // was HideScreen

/**
 * Place a window at the front of its layer on the active list.
 * @param scrnContent		The window Identifier.
 * @remarks		If the window is currently inactive, it will be activated and placed
 * into the list at the front of its layer (SF_BACKGRND, SF_DRIFTER, SF_FLOATER, or SF_SURFACE).
 * If the window is already active, it will be moved to the front of its layer.
 */
extern void vmng_screen_show(void *scrnContent); // was ShowScreen

/**
 * Place a window at the back of its layer.
 * @param scrnContent	The window Identifier.
 * @remarks		Essentially this procedure does the same as vmng_screen_show(),
 * the only difference is that the window is at the back of its layer.
 */
extern void vmng_screen_to_back(void *scrnContent); // was MoveScreenToBack

/**
 * Release all resources associated with the window.
 * @param scrnContent	The window Identifier.
 */
extern void vmng_screen_dispose(void *scrnContent);  // was DestroyScreen

extern void vmng_refresh_video(int32 scrnX, int32 scrnY, int32 x1, int32 y1, int32 x2, int32 y2, Buffer *srcBuffer);

// Rectangle routines
extern RectList *vmng_CreateNewRect(int32 x1, int32 y1, int32 x2, int32 y2);
extern void vmng_AddRectToRectList(RectList **scrnRectList, int32 x1, int32 y1, int32 x2, int32 y2);
extern RectList *vmng_DuplicateRectList(RectList *myRectList);
extern bool vmng_RectIntersectsRectList(RectList *myRectList, int32 x1, int32 y1, int32 x2, int32 y2);
extern bool vmng_RectListValid(RectList *myRectList);
extern bool vmng_ClipRectList(RectList **myRectList, int32 clipX1, int32 clipY1, int32 clipX2, int32 clipY2);
extern void vmng_DisposeRectList(RectList **rectList);
extern void vmng_RemoveRectFromRectList(RectList **scrnRectList, int32 x1, int32 y1, int32 x2, int32 y2);

} // End of namespace M4

#endif
