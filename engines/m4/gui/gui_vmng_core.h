
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

#ifndef M4_GUI_GUI_VMNG_CORE_H
#define M4_GUI_GUI_VMNG_CORE_H

#include "m4/m4_types.h"
#include "m4/gui/gui_univ.h"

namespace M4 {

/**
 * To initialize the GUI view manager.
 * @returns		The success of the call
 * @remarks		Should be called once during program initialization,
 *				after dpmi_init_mem() has been called. 
 */
bool vmng_init();

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
ScreenContext *vmng_screen_create(int32 x1, int32 y1, int32 x2, int32 y2, int32 scrnType, uint32 scrnFlags,
	void *scrnContent, RefreshFunc redraw, EventHandler evtHandler);

/**
 * Return the ScreenContext * associated with a window, and to determine whether it is active,
 * exists but is inactive (invisible, no events are received or blocked), or is undefined
 * (does not exist).
 * @param scrnContent	The window identifier.
 * @param status		If not nullptr, the status: SCRN_ACTIVE, SCRN_INACTIVE, or SCRN_UNDEFN
 * is recoreded here.
 * @returns		the ScreenContext* associated with the window which was created for
 * the structure scrnContent. returns nullptr if now window was found.
 */
ScreenContext *vmng_screen_find(void *scrnContent, int32 *status);	// was FindScreen

/**
 * Remove a window from the active list, and place it on the inactive list
 * @param		The window identifier
 */
void vmng_screen_hide(void *scrnContent); // was HideScreen

/**
 * Place a window at the front of its layer on the active list.
 * @param scrnContent		The window Identifier.
 * @remarks		If the window is currently inactive, it will be activated and placed
 * into the list at the front of its layer (SF_BACKGRND, SF_DRIFTER, SF_FLOATER, or SF_SURFACE).
 * If the window is already active, it will be moved to the front of its layer.
 */
void vmng_screen_show(void *scrnContent); // was ShowScreen

/**
 * Place a window at the back of its layer.
 * @param scrnContent	The window Identifier.
 * @remarks		Essentially this procedure does the same as vmng_screen_show(),
 * the only difference is that the window is at the back of its layer.
 */
void vmng_screen_to_back(void *scrnContent); // was MoveScreenToBack

/**
 * Release all resources associated with the window.
 * @param scrnContent	The window Identifier.
 */
void vmng_screen_dispose(void *scrnContent);  // was DestroyScreen

void vmng_refresh_video(int32 scrnX, int32 scrnY, int32 x1, int32 y1, int32 x2, int32 y2, Buffer *srcBuffer);

/**
 * Remove the window from either the active list of windows, or the inactive list,
 * wherever it was found.
 */
ScreenContext *ExtractScreen(void *scrnContent, int32 status);

} // End of namespace M4

#endif
