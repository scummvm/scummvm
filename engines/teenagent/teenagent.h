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
 * $URL: https://www.switchlink.se/svn/teen/tagent.h $
 * $Id: tagent.h 304 2009-09-03 20:10:22Z megath $
 */


#ifndef TEENAGENT_ENGINE_H__
#define TEENAGENT_ENGINE_H__

#include "engines/engine.h"
#include "pack.h"
#include "resources.h"
#include "inventory.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace TeenAgent {

struct Object;
class Scene;
class MusicPlayer;

class TeenAgentEngine: public Engine {
public: 
	enum Action { ActionNone, ActionExamine, ActionUse };
	
	TeenAgentEngine(OSystem * system);

	virtual Common::Error run();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const char *desc);
	virtual bool canLoadGameStateCurrently() { return true; }
	virtual bool canSaveGameStateCurrently() { return !scene_busy; }
	virtual bool hasFeature(EngineFeature f) const;


	void deinit();
	
	Object * findObject(int id, const Common::Point &point);
	
	void examine(const Common::Point &point, Object *object);
	void use(Object *object);

	bool processCallback(uint16 addr);
	inline Scene * getScene() { return scene; }

	//event driven:
	void displayMessage(uint16 addr, byte color = 0xd1);
	void displayMessage(const Common::String &str, byte color = 0xd1);
	void moveTo(const Common::Point & dst, bool warp = false);
	void moveTo(uint16 x, uint16 y, bool warp = false);
	void playAnimation(uint16 id, byte slot = 0, bool async = false);
	void loadScene(byte id, const Common::Point &pos, byte o = 0);
	void loadScene(byte id, uint16 x, uint16 y, byte o = 0);
	void setOns(byte id, byte value, byte scene_id = 0);
	void setLan(byte id, byte value, byte scene_id = 0);
	void reloadLan();
	void rejectMessage();

	void playMusic(byte id); //schedules play
	void playSound(byte id, byte skip_frames = 0);
	void playSoundNow(byte id);
	void enableObject(byte id, byte scene_id = 0);
	void disableObject(byte id, byte scene_id = 0);
	void waitAnimation();

	Common::RandomSource random;

	Scene *scene;
	Inventory *inventory;
	MusicPlayer *music;
	
	void setMusic(byte id);

private: 
	void processObject();
	void anotherMansionTry();

	bool scene_busy;
	Action action;
	Object * dst_object;


	Audio::AudioStream *_musicStream;
	Audio::SoundHandle _musicHandle, _soundHandle;
};

}

#endif

