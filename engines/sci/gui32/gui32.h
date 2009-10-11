/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_GUI32_GUI32_H
#define SCI_GUI32_GUI32_H

#include "sci/gui/gui.h"

namespace Sci {

class SciGui32 : public SciGui {
public:
	SciGui32(EngineState *s, SciGuiScreen *screen, SciGuiPalette *palette, SciGuiCursor *cursor);
	~SciGui32();

	void init(bool oldGfxFunctions);

	void wait(int16 ticks);
	void setPort(uint16 portPtr);
	void setPortPic(Common::Rect rect, int16 picTop, int16 picLeft);
	reg_t getPort();
	void globalToLocal(int16 *x, int16 *y);
	void localToGlobal(int16 *x, int16 *y);
	int16 coordinateToPriority(int16 y);
	int16 priorityToCoordinate(int16 priority);

	reg_t newWindow(Common::Rect dims, Common::Rect restoreRect, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title);
	void disposeWindow(uint16 windowPtr, int16 arg2);

	void display(const char *text, int argc, reg_t *argv);

	void textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);
	void textFonts(int argc, reg_t *argv);
	void textColors(int argc, reg_t *argv);

	void drawStatus(const char *text, int16 colorPen, int16 colorBack);
	void drawMenuBar();
	void clearMenuBar();
	void drawPicture(GuiResourceId pictureId, int16 animationNr, bool mirroredFlag, bool addToFlag, int16 EGApaletteNo);
	void drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo);
	void drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool hilite);
	void drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, bool hilite);
	void drawControlIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo, int16 style, bool hilite);
	void drawControl(reg_t controlObject, bool highlight);
	void editControl(reg_t controlObject, reg_t eventObject);

	void graphFillBoxForeground(Common::Rect rect);
	void graphFillBoxBackground(Common::Rect rect);	
	void graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control);
	void graphDrawLine(Common::Point startPoint, Common::Point endPoint, int16 color, int16 priority, int16 control);
	reg_t graphSaveBox(Common::Rect rect, uint16 flags);
	void graphRestoreBox(reg_t handle);

	void paletteSet(int resourceNo, int flags);
	int16 paletteFind(int r, int g, int b);
	void paletteSetIntensity(int fromColor, int toColor, int intensity, bool setPalette);
	void paletteAnimate(int fromColor, int toColor, int speed);

	void shakeScreen(uint16 shakeCount, uint16 directions);

	uint16 onControl(byte screenMask, Common::Rect rect);
	void animate(reg_t castListReference, bool cycle, int argc, reg_t *argv);
	void addToPicList(reg_t listReference, int argc, reg_t *argv);
	void addToPicView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control);
	void setNowSeen(reg_t objectReference);
	bool canBeHere(reg_t curObject, reg_t listReference);

	void hideCursor();
	void showCursor();
	void setCursorShape(GuiResourceId cursorId);
	void setCursorPos(Common::Point pos);
	void moveCursor(Common::Point pos);

	bool debugUndither(bool flag);
	bool debugShowMap(int mapNo);

	// FIXME: Don't store EngineState
	virtual void resetEngineState(EngineState *newState) { s = newState; }

private:
	EngineState *s;
	bool _usesOldGfxFunctions;

	GfxDynView *_k_make_dynview_obj(reg_t obj, int options, int nr, int argc, reg_t *argv);
	void _k_make_view_list(GfxList **widget_list, List *list, int options, int argc, reg_t *argv);
	void draw_obj_to_control_map(GfxDynView *view);
	void draw_rect_to_control_map(Common::Rect abs_zone);
	int _k_view_list_dispose_loop(List *list, GfxDynView *widget, int argc, reg_t *argv);
	void _k_set_now_seen(reg_t object);
	void _k_prepare_view_list(GfxList *list, int options);
	void _k_update_signals_in_view_list(GfxList *old_list, GfxList *new_list);
	void _k_view_list_kryptonize(GfxWidget *v);
	void _k_raise_topmost_in_view_list(GfxList *list, GfxDynView *view);
	void _k_redraw_view_list(GfxList *list);
	void _k_draw_view_list(GfxList *list, int flags);
	void _k_view_list_do_postdraw(GfxList *list);
	void animate_do_animation(int argc, reg_t *argv);

	bool _k_animate_ran;	// FIXME: Avoid non-const global vars

	bool activated_icon_bar;	// FIXME: Avoid non-const global vars
	int port_origin_x;	// FIXME: Avoid non-const global vars
	int port_origin_y;	// FIXME: Avoid non-const global vars
};

} // End of namespace Sci

#endif
