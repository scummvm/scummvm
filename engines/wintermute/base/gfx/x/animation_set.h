/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_ANIMATION_SET_H
#define WINTERMUTE_ANIMATION_SET_H

#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/base/gfx/x/animation.h"
#include "engines/wintermute/base/gfx/x/frame_node.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/persistent.h"

namespace Wintermute {

class ModelX;
class XFileLexer;

class AnimationSet : public BaseNamedObject {
public:
	//////////////////////////////////////////////////////////////////////////
	class AnimationEvent {
	public:
		char *_eventName;
		int32 _frame;

		AnimationEvent() {
			_eventName = nullptr;
			_frame = -1;
		}

		AnimationEvent(char *name, int frame) {
			_eventName = nullptr;
			int size = strlen(name);
			_eventName = new char[size];
			Common::copy(name, name + size, _eventName);
			_frame = frame;
		}

		virtual ~AnimationEvent() {
			if (_eventName) {
				delete[] _eventName;
			}
		}

		bool persist(BasePersistenceManager *persistMgr) {
			persistMgr->transferCharPtr(TMEMBER(_eventName));
			persistMgr->transferSint32(TMEMBER(_frame));
			return true;
		}
	};

	AnimationSet(BaseGame *inGame, ModelX *model);
	virtual ~AnimationSet();

	bool loadFromX(XFileLexer &lexer, const Common::String &filename);
	bool findBones(FrameNode *rootFrame);
	bool addAnimation(Animation *anim);
	bool addEvent(AnimationEvent *event);
	bool update(int slot, uint32 localTime, float lerpValue);
	bool onFrameChanged(int currentFrame, int prevFrame);

	int getFrameTime();
	uint32 getTotalTime();

	bool _looping;

	bool persist(BasePersistenceManager *persistMgr);

protected:
	BaseArray<Animation *> _animations;
	BaseArray<AnimationEvent *> _events;

	int _frameTime;
	uint32 _totalTime;
	ModelX *_model;
};

} // namespace Wintermute

#endif
