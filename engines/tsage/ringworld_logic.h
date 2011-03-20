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

#define ADD_PLAYER_MOVER(X, Y) { Common::Point pt(X, Y); PlayerMover *mover = new PlayerMover(); \
	_globals->_player.addMover(mover, &pt, this); }
#define ADD_MOVER(OBJ, X, Y) { Common::Point pt(X, Y); NpcMover *mover = new NpcMover(); \
	OBJ.addMover(mover, &pt, this); }
#define ADD_MOVER2(OBJ, X, Y) { Common::Point pt(X, Y); NpcMover *mover = new NpcMover(); \
	OBJ.addMover(mover, &pt, NULL); }


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

class SceneArea: public SavedObject {
public:
	GfxSurface _surface;
	GfxSurface *_savedArea;
	Common::Point _pt;
	int _resNum;
	int _rlbNum;
	int _subNum;
	int _actionId;
	Rect _bounds;
public:
	SceneArea();
	~SceneArea();

	void setup(int resNum, int rlbNum, int subNum, int actionId);
	void draw2();
	void display();
	void restore();

	virtual void synchronise(Serialiser &s);
	virtual void draw(bool flag);
	virtual void wait();
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

class SpeakerPOR: public AnimatedSpeaker {
	class SpeakerAction1: public SpeakerAction {
	public:
		virtual void signal();
	};

public:
	SceneObject _object3;
	SpeakerAction1 _action2;
public:
	SpeakerPOR();
	virtual Common::String getClassName() { return "SpeakerPOR"; }
	virtual void setText(const Common::String &msg);
};	

class SpeakerOText: public SpeakerGText {
public:
	SpeakerOText();

	virtual Common::String getClassName() { return "SpeakerOText"; }
};	

class SpeakerPOText: public ScreenSpeaker {
public:
	SpeakerPOText();

	virtual Common::String getClassName() { return "SpeakerPOText"; }
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

class SpeakerEText: public ScreenSpeaker {
public:
	SpeakerEText();

	virtual Common::String getClassName() { return "SpeakerEText"; }
};

class SpeakerHText: public ScreenSpeaker {
public:
	SpeakerHText();

	virtual Common::String getClassName() { return "SpeakerHText"; }
};

class SpeakerSKText: public ScreenSpeaker {
public:
	SpeakerSKText();

	virtual Common::String getClassName() { return "SpeakerSKText"; }
};

class SpeakerQR: public AnimatedSpeaker {
public:
	SpeakerQR();

	virtual Common::String getClassName() { return "SpeakerQR"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerQU: public AnimatedSpeaker {
public:
	SpeakerQU();

	virtual Common::String getClassName() { return "SpeakerQU"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerSKL: public AnimatedSpeaker {
public:
	SpeakerSKL();

	virtual Common::String getClassName() { return "SpeakerQL"; }
	virtual void setText(const Common::String &msg);
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

class SpeakerML: public AnimatedSpeaker {
public:
	SpeakerML();

	virtual Common::String getClassName() { return "SpeakerML"; }
	virtual void setText(const Common::String &msg);
};	

} // End of namespace tSage

#endif
