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
 * Copyright 2020 Google
 *
 */
#include "common/str.h"
#include "common/rect.h"
#include "common/noncopyable.h"

#ifndef HADESCH_AMBIENT_H
#define HADESCH_AMBIENT_H

namespace Hadesch {
class AmbientAnim {
public:
	struct AmbientDesc {
		Common::String _animName;
		Common::String _soundName;
		AmbientDesc(Common::String animName, Common::String soundName) {
			_animName = animName;
			_soundName = soundName;
		}
	};

	enum PanType {
		PAN_ANY,
		PAN_LEFT,
		PAN_RIGHT
	};

  	enum AnimType {
		DISAPPEAR,
		KEEP_LOOP,
		BACK_AND_FORTH
	};

	AmbientAnim(const Common::String &animName,
		    const Common::String &sound, int zValue,
		    int minint, int maxint, AnimType loop,
		    Common::Point offset, PanType pan);
	AmbientAnim(const Common::Array<AmbientDesc> &descs, int zValue,
		    int minint, int maxint, AnimType loop,
		    Common::Point offset, PanType pan);
	AmbientAnim();
	void play(bool reschedule);
	void schedule();
	void start();
	void pause();
	void unpause();
	void hide();
  	void unpauseAndFirstFrame();
    	void selectFirstFrame();
	void playFinished(bool reschedule);
	bool isReady();
private:
	class AmbiantAnimInternal : Common::NonCopyable {
	public:
		Common::Array<AmbientDesc> _descs;
		int _minInterval, _maxInterval;
		int _zValue;
		AnimType _loopType;
		bool _isFwd;
		Common::Point _offset;
		bool _playing;
		bool _paused;
		PanType _pan;
	};

	bool isPanOK();

	Common::SharedPtr<AmbiantAnimInternal> _internal;
};

	
class AmbientAnimWeightedSet {
public:
	void readTableFilePriamSFX(const TextTable &table);
	void readTableFileSFX(const TextTable &table, AmbientAnim::PanType pan);
	void tick();
	void firstFrame();
	void pause(const Common::String &name);
	void unpause(const Common::String &name);
	void unpauseAndFirstFrame(const Common::String &name);
	void hide(const Common::String &name);
	void play(const Common::String &name, bool reschedule);
private:
	struct AmbientAnimWeightedSetElement {
		AmbientAnim anim;
		int weight;
		bool valid;
		Common::String name;
	};
	Common::Array<AmbientAnimWeightedSetElement> _elements;
};

class AnimClickables {
public:
	void playNext(const Common::String &name, const EventHandlerWrapper &event);
	void playChosen(const Common::String &name, int counter, const EventHandlerWrapper &event);
	void setTable(const TextTable table) {
		_table = table;
	}
	void readTable(Common::SharedPtr<Hadesch::VideoRoom> room,
		       const Common::String &name,
		       const TranscribedSound *transcriptionTable);

private:
	TextTable _table;
	Common::HashMap<Common::String, Common::String> _transcriptions;
	Common::HashMap<Common::String, int> _counters;
};


}
#endif
