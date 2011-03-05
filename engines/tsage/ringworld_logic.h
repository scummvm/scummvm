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
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/scene_logic.h $
 * $Id: scene_logic.h 232 2011-02-12 11:56:38Z dreammaster $
 *
 */

#ifndef TSAGE_RINGWORLD_LOGIC_H
#define TSAGE_RINGWORLD_LOGIC_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace tSage {

class SceneFactory {
public:
	static Scene *createScene(int sceneNumber);
};

class DisplayHotspot: public SceneObject {
private:
	Common::Array<int> _actions;
	bool performAction(int action);
public:
	DisplayHotspot(int regionId, ...);

	virtual void doAction(int action) { 
		if (!performAction(action))
			SceneHotspot::doAction(action);
	}
};

class DisplayObject: public SceneObject {
private:
	Common::Array<int> _actions;
	bool performAction(int action);
public:
	DisplayObject(int firstAction, ...);

	virtual void doAction(int action) { 
		if (!performAction(action))
			SceneHotspot::doAction(action);
	}
};

/*--------------------------------------------------------------------------*/
// Ringworld specific game speakers

class SpeakerGText: public Speaker {
public:
	SceneObject _sceneObject;
public:
	SpeakerGText();

	virtual Common::String getClassName() { return "SpeakerGText"; }
	virtual void setText(const Common::String &msg);
	virtual void removeText();
};	

class SpeakerOText: public SpeakerGText {
public:
	SpeakerOText();

	virtual Common::String getClassName() { return "SpeakerOText"; }
};	

class SpeakerSText: public ScreenSpeaker {
public:
	SpeakerSText();

	virtual Common::String getClassName() { return "SpeakerSText"; }
};

class SpeakerQText: public ScreenSpeaker {
public:
	SpeakerQText();

	virtual Common::String getClassName() { return "SpeakerQText"; }
};

class SpeakerMText: public ScreenSpeaker {
public:
	SpeakerMText();

	virtual Common::String getClassName() { return "SpeakerMText"; }
};

class SpeakerCText: public ScreenSpeaker {
public:
	SpeakerCText();

	virtual Common::String getClassName() { return "SpeakerCText"; }
};

class SpeakerHText: public ScreenSpeaker {
public:
	SpeakerHText();

	virtual Common::String getClassName() { return "SpeakerHText"; }
};

class SpeakerQL: public AnimatedSpeaker {
public:
	SpeakerQL();

	virtual Common::String getClassName() { return "SpeakerQL"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerSR: public AnimatedSpeaker {
public:
	SceneObject _object3;
public:
	SpeakerSR();

	virtual Common::String getClassName() { return "SpeakerSR"; }
	void setText(const Common::String &msg);
};

class SpeakerSL: public AnimatedSpeaker {
public:
	SpeakerSL();

	virtual Common::String getClassName() { return "SpeakerSL"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerQR: public AnimatedSpeaker {
public:
	SpeakerQR();

	virtual Common::String getClassName() { return "SpeakerQR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerCR: public AnimatedSpeaker {
public:
	SpeakerCR();

	virtual Common::String getClassName() { return "SpeakerCR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerMR: public AnimatedSpeaker {
public:
	SpeakerMR();

	virtual Common::String getClassName() { return "SpeakerMR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerSAL: public AnimatedSpeaker {
public:
	SpeakerSAL();

	virtual Common::String getClassName() { return "SpeakerSAL"; }
	virtual void setText(const Common::String &msg);
};	

} // End of namespace tSage

#endif
