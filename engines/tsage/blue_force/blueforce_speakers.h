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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_BLUEFORCE_SPEAKERS_H
#define TSAGE_BLUEFORCE_SPEAKERS_H

#include "common/scummsys.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/blue_force/blueforce_logic.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class VisualSpeaker: public Speaker {
public:
	NamedObject _object1;
	CountdownObject _object2;
	bool _removeObject1, _removeObject2;
	int _xp;
	int _numFrames;
	Common::Point _offsetPos;
public:
	VisualSpeaker();

	virtual Common::String getClassName() { return "VisualSpeaker"; }
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void proc12(Action *action);
	virtual void setText(const Common::String &msg);
};

class SpeakerGameText: public VisualSpeaker {
public:
	SpeakerGameText();

	virtual Common::String getClassName() { return "SpeakerGameText"; }
};

class SpeakerSutter: public VisualSpeaker {
public:
	SpeakerSutter();

	virtual Common::String getClassName() { return "SpeakerSutter"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerDoug: public VisualSpeaker {
public:
	SpeakerDoug();

	virtual Common::String getClassName() { return "SpeakerDoug"; }
};

class SpeakerJakeNoHead: public VisualSpeaker {
public:
	SpeakerJakeNoHead();

	virtual Common::String getClassName() { return "SpeakerJakeNoHead"; }
};

class SpeakerHarrison: public VisualSpeaker {
public:
	SpeakerHarrison();

	virtual Common::String getClassName() { return "SpeakerHarrison"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerJakeJacket: public VisualSpeaker {
public:
	SpeakerJakeJacket();

	virtual Common::String getClassName() { return "SpeakerJakeJacket"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerJakeUniform: public VisualSpeaker {
public:
	SpeakerJakeUniform();

	virtual Common::String getClassName() { return "SpeakerJakeUniform"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerJailer: public VisualSpeaker {
public:
	SpeakerJailer();

	virtual Common::String getClassName() { return "SpeakerJailer"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerPSutter: public VisualSpeaker {
public:
	SpeakerPSutter();

	virtual Common::String getClassName() { return "SpeakerPSutter"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerJakeRadio: public VisualSpeaker {
public:
	SpeakerJakeRadio();

	virtual Common::String getClassName() { return "SpeakerJakeRadio"; }
};

class SpeakerLyleHat: public VisualSpeaker {
public:
	SpeakerLyleHat();

	virtual Common::String getClassName() { return "SpeakerLyleHat"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerJordan: public VisualSpeaker {
public:
	SpeakerJordan();

	virtual Common::String getClassName() { return "SpeakerJordan"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerSkipB: public VisualSpeaker {
public:
	SpeakerSkipB();

	virtual Common::String getClassName() { return "SpeakerSkipB"; }
	virtual void setText(const Common::String &msg);
};


} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
