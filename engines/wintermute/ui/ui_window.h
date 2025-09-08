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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_UIWINDOW_H
#define WINTERMUTE_UIWINDOW_H


#include "engines/wintermute/ui/ui_object.h"
#include "common/events.h"

namespace Wintermute {

class UIButton;
class BaseViewport;
class UIWindow : public UIObject {
public:
	//CBObject *GetPrevAccessObject(CBObject *CurrObject);
	//CBObject *GetNextAccessObject(CBObject *CurrObject);
	bool getWindowObjects(BaseArray<UIObject *> &objects, bool interactiveOnly);

	bool _pauseMusic;
	void cleanup();
	void makeFreezable(bool freezable) override;
	BaseViewport *_viewport;
	bool _clipContents;
	bool _inGame;
	bool _isMenu;
	bool _fadeBackground;
	uint32 _fadeColor;
	bool handleMouseWheel(int32 delta) override;
	UIWindow *_shieldWindow;
	UIButton *_shieldButton;
	bool close();
	bool goSystemExclusive();
	bool goExclusive();
	TWindowMode _mode;
	bool moveFocus(bool forward = true);
	bool handleMouse(TMouseEvent event, TMouseButton button) override;
	Common::Point32 _dragFrom;
	bool _dragging;
	DECLARE_PERSISTENT(UIWindow, UIObject)
	bool _transparent;
	bool showWidget(const char *name, bool visible = true);
	bool enableWidget(const char *name, bool enable = true);
	Common::Rect32 _titleRect;
	Common::Rect32 _dragRect;
	bool display(int offsetX = 0, int offsetY = 0) override;
	UIWindow(BaseGame *inGame);
	~UIWindow() override;
	bool handleKeypress(Common::Event *event, bool printable = false) override;
	BaseArray<UIObject *> _widgets;
	TTextAlign _titleAlign;
	bool loadFile(const char *filename);
	bool loadBuffer(char *buffer, bool complete = true);
	UITiledImage *_backInactive;
	BaseFont *_fontInactive;
	BaseSprite *_imageInactive;
	bool listen(BaseScriptHolder *param1, uint32 param2) override;
	bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;

	// scripting interface
	ScValue *scGetProperty(const char *name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
};

} // End of namespace Wintermute

#endif
