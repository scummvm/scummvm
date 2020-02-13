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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_UIEDIT_H
#define WINTERMUTE_UIEDIT_H

#include "engines/wintermute/persistent.h"
#include "engines/wintermute/ui/ui_object.h"
#include "common/events.h"

namespace Wintermute {
class BaseFont;
class UIEdit : public UIObject {
public:
	DECLARE_PERSISTENT(UIEdit, UIObject)

	int insertChars(int pos, const byte *chars, int num);
	int deleteChars(int start, int end);

	bool display(int offsetX, int offsetY) override;
	bool handleKeypress(Common::Event *event, bool printable = false) override;

	void setCursorChar(const char *character);

	UIEdit(BaseGame *inGame);
	~UIEdit() override;

	bool loadFile(const char *filename);
	bool loadBuffer(char *buffer, bool complete = true);
	bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;

	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
private:
	uint32 _cursorBlinkRate;
	uint32 _lastBlinkTime;
	int32 _selEnd;
	int32 _selStart;
	int32 _scrollOffset;
	int32 _frameWidth;
	BaseFont *_fontSelected;
	int32 _maxLength;
	bool _cursorVisible;
	char *_cursorChar;
};

} // End of namespace Wintermute

#endif
