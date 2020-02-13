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

#ifndef WINTERMUTE_ADRESPONSEBOX_H
#define WINTERMUTE_ADRESPONSEBOX_H


#include "engines/wintermute/base/base_object.h"

namespace Wintermute {

class UIButton;
class UIWindow;
class UIObject;
class AdResponse;
class AdResponseBox : public BaseObject {
public:
	BaseObject *getNextAccessObject(BaseObject *CurrObject);
	BaseObject *getPrevAccessObject(BaseObject *CurrObject);
	bool getObjects(BaseArray<UIObject *> &objects, bool interactiveOnly);

	void addResponse(const AdResponse* response);
	bool handleResponse(const AdResponse *response);
	bool handleResponseNum(uint32 num);
	int32 getIdForResponseNum(uint32 num) const;
	void setLastResponseText(const char *text, const char *textOrig);
	const char *getLastResponseText() const;
	const char *getLastResponseTextOrig() const;

	DECLARE_PERSISTENT(AdResponseBox, BaseObject)
	ScScript *_waitingScript;
	bool listen(BaseScriptHolder *param1, uint32 param2) override;
	typedef enum {
		EVENT_PREV,
		EVENT_NEXT,
		EVENT_RESPONSE
	} TResponseEvent;

	bool weedResponses();
	bool display() override;

	bool createButtons();
	bool invalidateButtons();
	void clearButtons();
	void clearResponses();
	AdResponseBox(BaseGame *inGame);
	~AdResponseBox() override;

	bool loadFile(const char *filename);
	bool loadBuffer(char *buffer, bool complete = true);
	bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;

	UIWindow *getResponseWindow();
	uint32 getNumResponses() const;
private:
	int32 _spacing;
	int32 _scrollOffset;
	BaseFont *_fontHover;
	BaseFont *_font;
	bool _horizontal;
	Rect32 _responseArea;
	int32 _verticalAlign;
	TTextAlign _align;
	BaseArray<UIButton *> _respButtons;
	BaseArray<const AdResponse *> _responses;
	UIWindow *_shieldWindow;
	char *_lastResponseText;
	char *_lastResponseTextOrig;
	UIWindow *_window;
};

} // End of namespace Wintermute

#endif
