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
 * $URL$
 * $Id$
 */

#ifndef TEENAGENT_ENGINE_H
#define TEENAGENT_ENGINE_H

#include "engines/engine.h"
#include "teenagent/pack.h"
#include "teenagent/resources.h"
#include "teenagent/inventory.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

struct ADGameDescription;

/**
 * This is the namespace of the TeenAgent engine.
 *
 * Status of this engine: ???
 *
 * Supported games:
 * - ???
 */
namespace TeenAgent {

struct Object;
class Scene;
class MusicPlayer;
class Console;

class TeenAgentEngine: public Engine {
public:
	enum Action { kActionNone, kActionExamine, kActionUse };

	TeenAgentEngine(OSystem *system, const ADGameDescription *gd);

	virtual Common::Error run();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const char *desc);
	virtual bool canLoadGameStateCurrently() { return true; }
	virtual bool canSaveGameStateCurrently() { return !scene_busy; }
	virtual bool hasFeature(EngineFeature f) const;

	void init();
	void deinit();

	void examine(const Common::Point &point, Object *object);
	void use(Object *object);
	inline void cancel() { action = kActionNone; }

	bool processCallback(uint16 addr);
	inline Scene *getScene() { return scene; }

	static Common::String parseMessage(uint16 addr);

	//event driven:
	void displayMessage(uint16 addr, byte color = 0xd1, uint16 position = 0);
	void displayMessage(const Common::String &str, byte color = 0xd1, uint16 position = 0);
	void displayAsyncMessage(uint16 addr, uint16 position, uint16 first_frame, uint16 last_frame, byte color = 0xd1);
	void displayAsyncMessageInSlot(uint16 addr, byte slot, uint16 first_frame, uint16 last_frame, byte color = 0xd1);
	void displayCredits(uint16 addr);
	void displayCutsceneMessage(uint16 addr, uint16 position);
	void moveTo(const Common::Point &dst, byte o, bool warp = false);
	void moveTo(uint16 x, uint16 y, byte o, bool warp = false);
	void moveTo(Object *obj);
	void moveRel(int16 x, int16 y, byte o, bool warp = false);
	void playActorAnimation(uint16 id, bool async = false, bool ignore = false);
	void playAnimation(uint16 id, byte slot, bool async = false, bool ignore = false, bool loop = false);
	void loadScene(byte id, const Common::Point &pos, byte o = 0);
	void loadScene(byte id, uint16 x, uint16 y, byte o = 0);
	void setOns(byte id, byte value, byte scene_id = 0);
	void setLan(byte id, byte value, byte scene_id = 0);
	void reloadLan();
	void rejectMessage();

	void playMusic(byte id); //schedules play
	void playSound(byte id, byte skip_frames);
	void playSoundNow(byte id);
	void enableObject(byte id, byte scene_id = 0);
	void disableObject(byte id, byte scene_id = 0);
	void hideActor();
	void showActor();
	void waitAnimation();
	void waitLanAnimationFrame(byte slot, uint16 frame);
	void setTimerCallback(uint16 addr, uint16 frames);
	void shakeScreen();

	Common::RandomSource random;

	Scene *scene;
	Inventory *inventory;
	MusicPlayer *music;
	Console * console;

	void setMusic(byte id);

private:
	void processObject();

	bool scene_busy;
	Action action;
	Object *dst_object;

	Audio::AudioStream *_musicStream;
	Audio::SoundHandle _musicHandle, _soundHandle;
	const ADGameDescription *_gameDescription;
	
	Common::Array<Common::Array<UseHotspot> > use_hotspots;
};

} // End of namespace TeenAgent

#endif
