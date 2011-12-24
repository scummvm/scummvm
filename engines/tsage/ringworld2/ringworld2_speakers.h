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

#ifndef TSAGE_RINGWORLD2_SPEAKERS_H
#define TSAGE_RINGWORLD2_SPEAKERS_H

#include "common/scummsys.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/ringworld2/ringworld2_logic.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

class VisualSpeaker : public Speaker {
public:
	SceneActor _object1;
	SceneObject *_object2;
	int _fieldF6, _fieldF8;
	int _displayMode;
	int _soundId;
	int _delayAmount;
	bool _removeObject;
	int _frameNumber;
	int _numFrames;
private:
	void setFrame(int numFrames);
public:
	VisualSpeaker();

	virtual Common::String getClassName() { return "VisualSpeaker"; }
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void setText(const Common::String &msg);
	virtual void proc15() {}
	virtual void proc16();

	void setDelay(int delay);
};

class SpeakerMiranda300 : public VisualSpeaker {
public:
	SpeakerMiranda300();

	virtual Common::String getClassName() { return "SpeakerMiranda300"; }
	virtual void proc15();
};

class SpeakerSeeker300 : public VisualSpeaker {
public:
	SpeakerSeeker300();

	virtual Common::String getClassName() { return "SpeakerSeeker300"; }
	virtual void proc15();
};

class SpeakerSeekerL : public VisualSpeaker {
public:
	SpeakerSeekerL();

	virtual Common::String getClassName() { return "SpeakerSeekerL"; }
};

class SpeakerQuinnL : public VisualSpeaker {
public:
	SpeakerQuinnL();

	virtual Common::String getClassName() { return "SpeakerQuinnL"; }
};

class SpeakerQuinn300 : public VisualSpeaker {
public:
	SpeakerQuinn300();

	virtual Common::String getClassName() { return "SpeakerQuinn300"; }
	virtual void proc15();
};

class SpeakerTeal300 : public VisualSpeaker {
public:
	SpeakerTeal300();

	virtual Common::String getClassName() { return "SpeakerTeal300"; }
	virtual void proc15();
};

class SpeakerSoldier300 : public VisualSpeaker {
public:
	SpeakerSoldier300();

	virtual Common::String getClassName() { return "SpeakerSoldier300"; }
	virtual void proc15();
};

class SpeakerQuinn2350 : public VisualSpeaker {
public:
	SpeakerQuinn2350();

	virtual Common::String getClassName() { return "SpeakerQuinn2350"; }
};

class SpeakerPharisha2350 : public VisualSpeaker {
public:
	SpeakerPharisha2350();

	virtual Common::String getClassName() { return "SpeakerPharisha2350"; }
};

class SpeakerQuinn2435 : public VisualSpeaker {
public:
	SpeakerQuinn2435();

	virtual Common::String getClassName() { return "SpeakerQuinn2435"; }
	virtual void proc15();
};

class SpeakerSeeker2435 : public VisualSpeaker {
public:
	SpeakerSeeker2435();

	virtual Common::String getClassName() { return "SpeakerSeeker2435"; }
	virtual void proc15();
};

class SpeakerPharisha2435 : public VisualSpeaker {
public:
	SpeakerPharisha2435();

	virtual Common::String getClassName() { return "SpeakerPharisha2435"; }
	virtual void proc15();
};

class SpeakerQuinn2450 : public VisualSpeaker {
public:
	SpeakerQuinn2450();

	virtual Common::String getClassName() { return "SpeakerQuinn2450"; }
	virtual void proc15();
};

class SpeakerSeeker2450 : public VisualSpeaker {
public:
	SpeakerSeeker2450();

	virtual Common::String getClassName() { return "SpeakerSeeker2450"; }
	virtual void proc15();
};

class SpeakerCaretaker2450 : public VisualSpeaker {
public:
	SpeakerCaretaker2450();

	virtual Common::String getClassName() { return "SpeakerCaretaker2450"; }
};

class SpeakerQuinn2500 : public VisualSpeaker {
public:
	SpeakerQuinn2500();

	virtual Common::String getClassName() { return "SpeakerQuinn2500"; }
};

class SpeakerSeeker2500 : public VisualSpeaker {
public:
	SpeakerSeeker2500();

	virtual Common::String getClassName() { return "SpeakerSeeker2500"; }
};

class SpeakerMiranda2500 : public VisualSpeaker {
public:
	SpeakerMiranda2500();

	virtual Common::String getClassName() { return "SpeakerMiranda2500"; }
};

class SpeakerWebbster2500 : public VisualSpeaker {
public:
	SpeakerWebbster2500();

	virtual Common::String getClassName() { return "SpeakerWebbster2500"; }
};

class SpeakerQuinn2700 : public VisualSpeaker {
public:
	SpeakerQuinn2700();

	virtual Common::String getClassName() { return "SpeakerQuinn2700"; }
	virtual void proc15();
};

class SpeakerNej2700 : public VisualSpeaker {
public:
	SpeakerNej2700();

	virtual Common::String getClassName() { return "SpeakerNej2700"; }
	virtual void proc15();
};

class SpeakerQuinn2750 : public VisualSpeaker {
public:
	SpeakerQuinn2750();

	virtual Common::String getClassName() { return "SpeakerQuinn2750"; }
	virtual void proc15();
};

class SpeakerNej2750 : public VisualSpeaker {
public:
	SpeakerNej2750();

	virtual Common::String getClassName() { return "SpeakerNej2750"; }
	virtual void proc15();
};

class SpeakerQuinn2800 : public VisualSpeaker {
public:
	SpeakerQuinn2800();

	virtual Common::String getClassName() { return "SpeakerQuinn2800"; }
	virtual void proc15();
};

class SpeakerNej2800 : public VisualSpeaker {
public:
	SpeakerNej2800();

	virtual Common::String getClassName() { return "SpeakerNej2800"; }
	virtual void proc15();
};

class SpeakerGuard2800 : public VisualSpeaker {
public:
	SpeakerGuard2800();

	virtual Common::String getClassName() { return "SpeakerGuard2800"; }
	virtual void proc15();
};

class SpeakerGuard3100 : public VisualSpeaker {
public:
	SpeakerGuard3100();

	virtual Common::String getClassName() { return "SpeakerGuard3100"; }
};

class SpeakerRocko3200 : public VisualSpeaker {
public:
	SpeakerRocko3200();

	virtual Common::String getClassName() { return "SpeakerRocko3200"; }
	virtual void proc15();
};

class SpeakerJocko3200 : public VisualSpeaker {
public:
	SpeakerJocko3200();

	virtual Common::String getClassName() { return "SpeakerJocko3200"; }
	virtual void proc15();
};

class SpeakerSocko3200 : public VisualSpeaker {
public:
	SpeakerSocko3200();

	virtual Common::String getClassName() { return "SpeakerSocko3200"; }
	virtual void proc15();
};

class SpeakerCaptain3210 : public VisualSpeaker {
public:
	SpeakerCaptain3210();

	virtual Common::String getClassName() { return "SpeakerCaptain3210"; }
	virtual void proc15();
};

class SpeakerPrivate3210 : public VisualSpeaker {
public:
	SpeakerPrivate3210();

	virtual Common::String getClassName() { return "SpeakerPrivate3210"; }
	virtual void proc15();
};

class SpeakerRocko3220 : public VisualSpeaker {
public:
	SpeakerRocko3220();

	virtual Common::String getClassName() { return "SpeakerRocko3220"; }
	virtual void proc15();
};

class SpeakerJocko3220 : public VisualSpeaker {
public:
	SpeakerJocko3220();

	virtual Common::String getClassName() { return "SpeakerJocko3220"; }
	virtual void proc15();
};

class SpeakerRocko3230 : public VisualSpeaker {
public:
	SpeakerRocko3230();

	virtual Common::String getClassName() { return "SpeakerRocko3230"; }
	virtual void proc15();
};

class SpeakerJocko3230 : public VisualSpeaker {
public:
	SpeakerJocko3230();

	virtual Common::String getClassName() { return "SpeakerJocko3230"; }
	virtual void proc15();
};

class SpeakerTeal3240 : public VisualSpeaker {
public:
	SpeakerTeal3240();

	virtual Common::String getClassName() { return "SpeakerTeal3240"; }
	virtual void proc15();
};

class SpeakerWebbster3240 : public VisualSpeaker {
public:
	SpeakerWebbster3240();

	virtual Common::String getClassName() { return "SpeakerWebbster3240"; }
	virtual void proc15();
};

class SpeakerRalf3245 : public VisualSpeaker {
public:
	SpeakerRalf3245();

	virtual Common::String getClassName() { return "SpeakerRalf3245"; }
	virtual void proc15();
};

class SpeakerTomko3245 : public VisualSpeaker {
public:
	SpeakerTomko3245();

	virtual Common::String getClassName() { return "SpeakerTomko3245"; }
	virtual void proc15();
};

class SpeakerQuinn3255 : public VisualSpeaker {
public:
	SpeakerQuinn3255();

	virtual Common::String getClassName() { return "SpeakerQuinn3255"; }
	virtual void proc15();
};

class SpeakerMiranda3255 : public VisualSpeaker {
public:
	SpeakerMiranda3255();

	virtual Common::String getClassName() { return "SpeakerMiranda3255"; }
	virtual void proc15();
};
} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
