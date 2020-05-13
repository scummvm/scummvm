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

#ifndef WINTERMUTE_BASE_FRAME_H
#define WINTERMUTE_BASE_FRAME_H

#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/coll_templ.h"
#include "graphics/transform_struct.h"

namespace Wintermute {
class BaseSound;
class BaseSubFrame;
class BaseObject;
class ScScript;
class ScStack;
class BaseFrame: public BaseScriptable {
public:
	bool _killSound;
	void stopSound();
	bool oneTimeDisplay(BaseObject *owner, bool muted = false);
	DECLARE_PERSISTENT(BaseFrame, BaseScriptable)

	bool getBoundingRect(Rect32 *rect, int x, int y, float scaleX = 100, float scaleY = 100);
	bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;
	int32 _moveY;
	int32 _moveX;
	uint32 _delay;
	BaseArray<BaseSubFrame *> _subframes;
	bool draw(int x, int y, BaseObject *registerOwner = nullptr, float zoomX = 100, float zoomY = 100, bool precise = true, uint32 alpha = 0xFFFFFFFF, bool allFrames = false, float rotate = 0.0f, Graphics::TSpriteBlendMode blendMode = Graphics::BLEND_NORMAL);
	bool loadBuffer(char *buffer, int lifeTime, bool keepLoaded);

	BaseFrame(BaseGame *inGame);
	~BaseFrame() override;

	BaseArray<const char *> _applyEvent;

	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
	Common::String debuggerToString() const override;

private:
	bool _keyframe;
	bool _editorExpanded;
	BaseSound *_sound;
};

} // End of namespace Wintermute

#endif
