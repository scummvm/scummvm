/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_chdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink
#define FORBIDDEN_SYMBOL_EXCEPTION_getwd
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir

#include "common/endian.h"

#include "engines/grim/lua_v2.h"
#include "engines/grim/lua/luadebug.h"
#include "engines/grim/lua/lauxlib.h"

#include "engines/grim/grim.h"
#include "engines/grim/actor.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/sound.h"
#include "engines/grim/costume.h"
#include "engines/grim/costume/chore.h"

#include "engines/grim/movie/movie.h"

namespace Grim {

void Lua_V2::UndimAll() {
	warning("Lua_V2::UndimAll: stub");
}

void Lua_V2::SetActorLocalAlpha() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	// FIXME: implement missing code
	warning("Lua_V2::SetActorLocalAlpha: stub, actor: %s", actor->getName().c_str());
}

void Lua_V2::UndimRegion() {
	lua_Object regionObj = lua_getparam(1);

	if (lua_isnumber(regionObj)) {
		int region = (int)lua_getnumber(regionObj);
		// FIXME func(region);
		warning("Lua_V2::UndimRegion: region: %d", region);
	} else {
		lua_pushnil();
	}
}

void Lua_V2::SleepFor() {
	lua_Object msObj = lua_getparam(1);

	if (lua_isnumber(msObj)) {
		int ms = (int)lua_getnumber(msObj);
		// FIXME func(ms);
		warning("Lua_V2::SleepFor: ms: %d", ms);
	}
}

void Lua_V2::DimScreen() {
	lua_Object dimObj = lua_getparam(1);
	float dim = 0.6999f;

	if (lua_isnumber(dimObj))
		dim = lua_getnumber(dimObj);

	// FIXME func(dim);
	warning("Lua_V2::DimScreen: dim: %f", dim);
}

void Lua_V2::MakeCurrentSetup() {
	lua_Object setupObj = lua_getparam(1);
	if (lua_isnumber(setupObj)) {
		int num = (int)lua_getnumber(setupObj);
		g_grim->makeCurrentSetup(num);
	} else if (lua_isstring(setupObj)) {
		const char *setupName = lua_getstring(setupObj);
		error("Lua_V2::MakeCurrentSetup: Not implemented case: setup: %s", setupName);
	}
}

void Lua_V2::SetActorGlobalAlpha() {
	lua_Object actorObj = lua_getparam(1);
//	lua_Object alphaModeObj = lua_getparam(2);
//	lua_Object valueObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	warning("Lua_V2::SetActorGlobalAlpha: actor: %s", actor->getName().c_str());

	/* Only when actor has primitives
	if (!actor->primities)
			return;
	if (lua_isnumber(alphaModeObj) {
		int alphaMode = (int)lua_getnumber(alphaModeObj);
		if (!lua_isnil(valueObj) && lua_isstring(valueObj)) {
				// TODO: missing part
		}
		// TODO
	}
	*/
}

void Lua_V2::ImGetMillisecondPosition() {
	lua_Object soundObj = lua_getparam(1);

	if (lua_isnumber(soundObj)) {
		int sound = (int)lua_getnumber(soundObj);
		// FIXME int ms = func(sound);
		// lua_pushnumber(ms);
		// push -1 for now
		// Currently a bit of guesswork, and probably wrong, as the stateId
		// is ignored by emisound (which only has one music-track now).
		uint32 msPos = g_sound->getMsPos(sound);
		warning("Lua_V2::ImGetMillisecondPosition: sound: %d ms: %d", sound, msPos);
		lua_pushnumber(msPos);
	}
}

void Lua_V2::RemoveActorFromOverworld() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	warning("Lua_V2::RemoveActorFromOverworld: actor: %s", actor->getName().c_str());
	// FIXME actor->func();
}

void Lua_V2::UnloadActor() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	warning("Lua_V2::UnloadActor: actor: %s", actor->getName().c_str());
	// FIXME actor->func();
}

void Lua_V2::SetActorWalkRate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object rateObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isnumber(rateObj))
		return;

	Actor *actor = getactor(actorObj);
	float rate = lua_getnumber(rateObj);
	// const below only differ from grim
	actor->setWalkRate(rate * 3.279999971389771);
}

void Lua_V2::GetActorWalkRate() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	// const below only differ from grim
	lua_pushnumber(actor->getWalkRate() * 0.3048780560493469);
}

void Lua_V2::SetReverb() {
	lua_Object eaxObj = lua_getparam(1);
	lua_Object decayObj = lua_getparam(2);
	lua_Object mixObj = lua_getparam(3);
	lua_Object predelayObj = lua_getparam(4);
	lua_Object dampingObj = lua_getparam(5);

	if (!lua_isnumber(eaxObj))
		return;

	int eax = (int)lua_getnumber(eaxObj);
	int param = 0;
	float decay = -1;
	float mix = -1;
	float predelay = -1;
	float damping = -1;

	if (eax == 60) {
		param = 26;
	} else if (eax == 70) {
		param = 27;
	} else if (eax >= 0 && eax <= 25) {
		param = eax;
		// there is some table, initialy is like eax
	} else {
		return;
	}

	if (lua_isnumber(decayObj))
		decay = lua_getnumber(decayObj);
	if (lua_isnumber(mixObj))
		mix = lua_getnumber(mixObj);
	if (lua_isnumber(predelayObj))
		predelay = lua_getnumber(predelayObj);
	if (lua_isnumber(dampingObj))
		damping = lua_getnumber(dampingObj);

	warning("Lua_V2::SetReverb, eax: %d, decay: %f, mix: %f, predelay: %f, damping: %f", param, decay, mix, predelay, damping);
	// FIXME: func(param, decay, mix, predelay, damping);
}

void Lua_V2::LockBackground() {
	lua_Object filenameObj = lua_getparam(1);

	if (!lua_isstring(filenameObj)) {
		lua_pushnil();
		return;
	}
	const char *filename = lua_getstring(filenameObj);
	warning("Lua_V2::LockBackground, filename: %s", filename);
	// FIXME: implement missing rest part of code
}

void Lua_V2::UnLockBackground() {
	lua_Object filenameObj = lua_getparam(1);

	if (!lua_isstring(filenameObj)) {
		lua_pushnil();
		return;
	}

	const char *filename = lua_getstring(filenameObj);
	// FIXME: implement missin code
	warning("Lua_V2::UnLockBackground: stub, filename: %s", filename);
}

void Lua_V2::LockChore() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object filenameObj = lua_getparam(2);

	if (!lua_isstring(nameObj) || !lua_isstring(filenameObj)) {
		lua_pushnil();
		return;
	}

	const char *name = lua_getstring(nameObj);
	const char *filename = lua_getstring(filenameObj);
	warning("Lua_V2::LockChore, name: %s, filename: %s", name, filename);
	// FIXME: implement missing rest part of code
}

void Lua_V2::IsChoreValid() {
	lua_Object paramObj = lua_getparam(1);

	if (!lua_isnumber(paramObj))
		return;

	int num = (int)lua_getnumber(paramObj);

	Chore *c = PoolChore::getPool().getObject(num);
	pushbool(c != 0);
}

void Lua_V2::IsChorePlaying() {
	lua_Object paramObj = lua_getparam(1);

	if (!lua_isnumber(paramObj))
		return;

	int num = (int)lua_getnumber(paramObj);

	Chore *c = PoolChore::getPool().getObject(num);
	pushbool(c->isPlaying());
}

void Lua_V2::StopChore() {
	lua_Object choreObj = lua_getparam(1);
	lua_Object timeObj = lua_getparam(2);

	if (!lua_isnumber(choreObj) || !lua_isnumber(timeObj))
		return;

	int chore = (int)lua_getnumber(choreObj);
	float time = lua_getnumber(timeObj);
	// FIXME: implement missing rest part of code
	warning("Lua_V2::StopChore: stub, chore: %d time: %f", chore, time);
}

void Lua_V2::AdvanceChore() {
	lua_Object choreObj = lua_getparam(1);
	lua_Object timeObj = lua_getparam(2);

	if (!lua_isnumber(choreObj) || !lua_isnumber(timeObj))
		return;

	int chore = (int)lua_getnumber(choreObj);
	float time = lua_getnumber(timeObj);
	// FIXME: implement missong code
	warning("Lua_V2::AdvanceChore: stub, chore: %d time: %f", chore, time);
}

void Lua_V2::SetActorSortOrder() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object modeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	if (!lua_isnumber(modeObj))
		return;

	Actor *actor = getactor(actorObj);
	int mode = (int)lua_getnumber(modeObj);
	warning("Lua_V2::SetActorSortOrder, actor: %s, mode: %d", actor->getName().c_str(), mode);
	// FIXME: actor->func(mode);
}

void Lua_V2::ActorActivateShadow() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object qualityObj = lua_getparam(2);
	lua_Object planeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;
	int quality = (int)lua_getnumber(qualityObj);
	const char *plane = "NULL";
	if (lua_isstring(planeObj))
		plane = lua_getstring(planeObj);
	warning("Lua_V2::ActorActivateShadow, actor: %s, aquality: %d, plane: %s", actor->getName().c_str(), quality, plane);
	// FIXME: implement missing rest part of code
}

void Lua_V2::ActorStopMoving() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	warning("Lua_V2::ActorStopMoving, actor: %s", actor->getName().c_str());
	// FIXME: implement missing rest part of code
}

void Lua_V2::PutActorInOverworld() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	warning("Lua_V2::PutActorInOverworld, actor: %s", actor->getName().c_str());
	// FIXME: implement missing func
	//actor->func();
}

void Lua_V2::GetActorWorldPos() {
	warning("Lua_V2::GetActorWorldPos: Currently runs Lua_V1::GetActorPos");
	Lua_V1::GetActorPos();
}

void Lua_V2::MakeScreenTextures() {
	lua_Object indexObj = lua_getparam(1);

	if (!lua_isnil(indexObj) && lua_isnumber(indexObj)) {
		int index = (int)lua_getnumber(indexObj);
		warning("Lua_V2::MakeScreenTextures, index: %d", index);
		// FIXME: implement missing function
//		if (func(index)) {
			lua_pushnumber(1.0);
			return;
//		}
	}
	lua_pushnil();
}

void Lua_V2::PutActorInSet() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object setObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	if (!lua_isstring(setObj) && !lua_isnil(setObj)) {
		lua_pushnil();
		return;
	}

	const char *set = lua_getstring(setObj);

	// FIXME verify adding actor to set
	if (!set) {
		actor->putInSet("");
		lua_pushnil();
	} else {
		if (!actor->isInSet(set)) {
			actor->putInSet(set);
		}
		lua_pushnumber(1.0);
	}
}

void Lua_V2::LoadBundle() {
	lua_Object paramObj = lua_getparam(1);
	if (lua_isstring(paramObj) || lua_isnil(paramObj)) {
		const char *name = lua_getstring(paramObj);
		// FIXME: implement missing function
/*		if (!func(name))
			lua_pushnil();
		else*/
			lua_pushnumber(1.0);
		warning("Lua_V2::LoadBundle: stub, name: %s", name);
	}
}

void Lua_V2::AreWeInternational() {
	if (g_grim->getGameLanguage() != Common::EN_ANY)
		lua_pushnumber(1.0);
}

void Lua_V2::ImSetState() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	int state = (int)lua_getnumber(stateObj);
	g_imuseState = state;
	warning("Lua_V2::ImSetState: stub, state: %d", state);
}

void Lua_V2::EnableVoiceFX() {
	lua_Object stateObj = lua_getparam(1);

	bool state = false;
	if (!lua_isnil(stateObj))
		state = true;

	// FIXME: func(state);
	warning("Lua_V2::EnableVoiceFX: implement opcode, state: %d", (int)state);
}

void Lua_V2::SetGroupVolume() {
	lua_Object groupObj = lua_getparam(1);
	lua_Object volumeObj = lua_getparam(2);

	if (!lua_isnumber(groupObj))
		return;
	int group = (int)lua_getnumber(groupObj);

	int volume = 100;
	if (lua_isnumber(volumeObj))
		volume = (int)lua_getnumber(volumeObj);

	// FIXME: func(group, volume);
	warning("Lua_V2::SetGroupVolume: implement opcode, group: %d, volume %d", group, volume);
}

void Lua_V2::EnableAudioGroup() {
	lua_Object groupObj = lua_getparam(1);
	lua_Object stateObj = lua_getparam(2);

	if (!lua_isnumber(groupObj))
		return;
	int group = (int)lua_getnumber(groupObj);

	bool state = false;
	if (!lua_isnil(stateObj))
		state = true;

	// FIXME: func(group, state);
	warning("Lua_V2::EnableAudioGroup: implement opcode, group: %d, state %d", group, (int)state);
}

void Lua_V2::ImSelectSet() {
	lua_Object qualityObj = lua_getparam(1);

	if (lua_isnumber(qualityObj)) {
		int quality = (int)lua_getnumber(qualityObj);
		// FIXME: func(quality);
		warning("Lua_V2::ImSelectSet: implement opcode, quality mode: %d", quality);
	}
}

void Lua_V2::GetActorChores() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	Actor *actor = getactor(actorObj);

	lua_Object result = lua_createtable();
	lua_pushobject(result);

	lua_pushstring("count");
	lua_pushnumber(0.0);
	lua_settable();

	lua_pushobject(result);
}

void Lua_V2::PlayActorChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	lua_Object modeObj = lua_getparam(4);
	lua_Object paramObj = lua_getparam(5);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	if (!lua_isstring(choreObj) || !lua_isstring(costumeObj))
		lua_pushnil();

	bool mode = false;
	float param = 0.0;

	if (!lua_isnil(modeObj)) {
		if (lua_getnumber(modeObj) != 0.0)
			mode = true;
		if (!lua_isnil(paramObj))
			if (lua_isnumber(paramObj))
				param = lua_getnumber(paramObj);
	}

	const char *choreName = lua_getstring(choreObj);
	const char *costumeName = lua_getstring(costumeObj);

	actor->setCostume(costumeName);
	Costume *costume = actor->getCurrentCostume();
	Chore *chore = costume->getChore(choreName);
	costume->playChore(choreName);
	if (chore) {
		lua_pushnumber(chore->getId());
	} else {
		lua_pushnil();
	}

}

void Lua_V2::StopActorChores() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object paramObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	//FIXME: What does the second param actually do
	bool p = lua_isnil(paramObj) != 0;
	Costume *costume = actor->getCurrentCostume();
	if (costume) {
		costume->stopChores();
	}
}

void Lua_V2::SetActorLighting() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object lightModeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (lua_isnil(lightModeObj) || !lua_isnumber(lightModeObj))
		return;

	int lightMode = (int)lua_getnumber(lightModeObj);
	if (lightMode != 0) {
		if (lightMode == 1) {
			//FIXME actor->
			warning("Lua_V2::SetActorLighting: case param 1(LIGHT_FASTDYN), actor: %s", actor->getName().c_str());
		} else if (lightMode == 2) {
			//FIXME actor->
			warning("Lua_V2::SetActorLighting: case param 2(LIGHT_NORMDYN), actor: %s", actor->getName().c_str());
		} else {
			//FIXME actor->
			warning("Lua_V2::SetActorLighting: case param %d(LIGHT_NONE), actor: %s", lightMode, actor->getName().c_str());
		}
	} else {
		//FIXME actor->
		warning("Lua_V2::SetActorLighting: case param 0(LIGHT_STATIC), actor: %s", actor->getName().c_str());
	}
}

void Lua_V2::SetActorCollisionMode() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object modeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	assert(actor);
	int mode = (int)lua_getnumber(modeObj);
	// From _actors.lua
	// COLLISION_OFF = 0
	// COLLISION_BOX = 1
	// COLLISION_SPHERE = 2

	// FIXME: set collision mode
	//actor->func(mode);
	warning("Lua_V2::SetActorCollisionMode: implement opcode. Mode %d", mode);
}

void Lua_V2::SetActorCollisionScale() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object scaleObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	assert(actor);

	float scale = lua_getnumber(scaleObj);
	// FIXME: set collision scale
	//actor->func(scale);
	warning("Lua_V2::SetActorCollisionScale: implement opcode. Scale %f", scale);
}

void Lua_V2::GetActorPuckVector() {
	// stub this for now as the regular one crashes.
	warning("Lua_V2::GetActorPuckVector: just returns 0, 0, 0");
	lua_pushnumber(0);
	lua_pushnumber(0);
	lua_pushnumber(0);
}

void Lua_V2::SetActorHeadLimits() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object param2Obj = lua_getparam(2);
	lua_Object param3Obj = lua_getparam(3);
	lua_Object param4Obj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (lua_isnumber(param2Obj) && lua_isnumber(param3Obj) && lua_isnumber(param4Obj)) {
		float param2 = lua_getnumber(param2Obj); // belows needs multiply by some runtime value
		float param3 = lua_getnumber(param3Obj);
		float param4 = lua_getnumber(param4Obj);
		// FIXME: implement missing func
		//actor->func(param2, param3, param4);
		warning("Lua_V2::SetActorHeadLimits: implement opcode. actor: %s, params: %f, %f, %f", actor->getName().c_str(), param2, param3, param4);
	}
}

void Lua_V2::SetActorFOV() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object fovObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (lua_isnumber(fovObj)) {
		float fov = lua_getnumber(fovObj);
		// FIXME: implement missing code
		//actor->func(fov); // cos(fov * some tuntime val * 0.5)
		warning("Lua_V2::SetActorFOV: implement opcode. actor: %s, param: %f", actor->getName().c_str(), fov);
	}
}

void Lua_V2::AttachActor() {
	// Missing lua parts
	warning("Lua_V2::AttachActor: implement opcode");
}

void Lua_V2::DetachActor() {
	// Missing lua parts
	warning("Lua_V2::DetachActor: implement opcode");
}

void Lua_V2::GetCPUSpeed() {
	lua_pushnumber(500); // anything above 333 make best configuration
}

// This should be correct, judging by the Demo
// the only real difference from L1 is the lack of looping
void Lua_V2::StartMovie() {
	lua_Object name = lua_getparam(1);
	if (!lua_isstring(name)) {
		lua_pushnil();
		return;
	}
	Lua_V1::CleanBuffer();

	GrimEngine::EngineMode prevEngineMode = g_grim->getMode();
	g_grim->setMode(GrimEngine::SmushMode);
	bool result = g_movie->play(lua_getstring(name), false, 0, 0);
	if (!result)
		g_grim->setMode(prevEngineMode);
	pushbool(result);
	g_grim->setMode(GrimEngine::SmushMode);
}

void Lua_V2::EscapeMovie() {
	g_movie->stop();
}

void Lua_V2::IsMoviePlaying() {
	pushbool(g_movie->isPlaying());
}

void Lua_V2::SetActiveCD() {
	lua_Object cdObj = lua_getparam(1);
	int cd = (int)lua_getnumber(cdObj);

	if (cd == 1 || cd == 2) {
		warning("Lua_V2::GetActiveCD: set to CD: %d", cd);
		// FIXME
		lua_pushnumber(1.0);
	}
}

void Lua_V2::GetActiveCD() {
	// FIXME: return current CD number 1 or 2, original can also avoid push any numer
	warning("Lua_V2::GetActiveCD: return const CD 1");
	lua_pushnumber(1);
}

void Lua_V2::PurgeText() {
	TextObject::getPool().deleteObjects();
}

void Lua_V2::GetTextObjectDimensions() {
	lua_Object textObj = lua_getparam(1);

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		lua_pushnumber(textObject->getBitmapWidth()/640.f);
		lua_pushnumber(textObject->getBitmapHeight()/480.f);
	}
}

void Lua_V2::ImFlushStack() {
	// FIXME
	warning("Lua_V2::ImFlushStack: implement opcode");
}

void Lua_V2::LoadSound() {
	lua_Object strObj = lua_getparam(1);

	if (!lua_isstring(strObj))
		return;

	const char *str = lua_getstring(strObj);
	// FIXME: implement code
	warning("Lua_V2::LoadSound: stub, wants to load %s", str);
}

void Lua_V2::ImSetMusicVol() {
	warning("Lua_V2::ImSetMusicVol: implement opcode");
}

void Lua_V2::ImSetSfxVol() {
	warning("Lua_V2::ImSetSfxVol: implement opcode");
}

void Lua_V2::ImSetVoiceVol() {
	warning("Lua_V2::ImSetVoiceVol: implement opcode");
}

void Lua_V2::ImSetVoiceEffect() {
	warning("Lua_V2::ImSetVoiceEffect: implement opcode");
}

void Lua_V2::ToggleOverworld() {
	warning("Lua_V2::ToggleOverworld: implement opcode");
}

void Lua_V2::ScreenshotForSavegame() {
	warning("Lua_V2::ScreenshotForSavegame: implement opcode");
}
	
void Lua_V2::ImStateHasEnded() {
	warning("Lua_V2::ImStateHasEnded: Just returns true");
	pushbool(true);
}

void Lua_V2::PlayLoadedSound() {
	warning("Lua_V2::PlayLoadedSound: implement opcode");
}

void Lua_V2::EngineDisplay() {
	// dummy
}

void Lua_V2::SetAmbientLight() {
	// dummy
}

void Lua_V2::Display() {
	// dummy
}

void Lua_V2::RenderModeUser() {
	lua_Object param1 = lua_getparam(1);
	g_movie->pause(!lua_isnil(param1));
}

// Stub function for builtin functions not yet implemented
static void stubWarning(const char *funcName) {
	warning("Stub function: %s", funcName);
}

static void stubError(const char *funcName) {
	error("Stub function: %s", funcName);
}

#define STUB_FUNC(name) void name() { stubWarning(#name); }
#define STUB_FUNC2(name) void name() { stubError(#name); }

// Opcodes more or less differ to Grim Lua_V1::* LUA_OPCODEs
// STUB_FUNC2(Lua_V2::SetActorWalkChore)
// STUB_FUNC2(Lua_V2::SetActorTurnChores)
// STUB_FUNC2(Lua_V2::SetActorRestChore)
// STUB_FUNC2(Lua_V2::SetActorMumblechore)
// STUB_FUNC2(Lua_V2::SetActorTalkChore)
// STUB_FUNC2(Lua_V2::WalkActorVector)
// STUB_FUNC2(Lua_V2::SetActorLookRate)
// STUB_FUNC2(Lua_V2::GetActorLookRate)
// STUB_FUNC2(Lua_V2::GetVisibleThings)
// STUB_FUNC2(Lua_V2::GetActorRot)
// STUB_FUNC2(Lua_V2::LockSet)
// STUB_FUNC2(Lua_V2::UnLockSet)
// STUB_FUNC2(Lua_V2::PlaySound)
// STUB_FUNC2(Lua_V2::IsSoundPlaying)
// STUB_FUNC2(Lua_V2::MakeSectorActive)
// STUB_FUNC2(Lua_V2::TurnActorTo)
// STUB_FUNC2(Lua_V2::GetAngleBetweenActors)
// STUB_FUNC2(Lua_V2::ImStartSound)
// STUB_FUNC2(Lua_V2::ImGetSfxVol)
// STUB_FUNC2(Lua_V2::ImGetVoiceVol)
// STUB_FUNC2(Lua_V2::ImGetMusicVol)
// STUB_FUNC2(Lua_V2::ImSetSequence)
// STUB_FUNC2(Lua_V2::ChangeTextObject)
// STUB_FUNC2(Lua_V2::GetTextCharPosition)
// STUB_FUNC2(Lua_V2::SetOffscreenTextPos)
// STUB_FUNC2(Lua_V2::FadeInChore)
// STUB_FUNC2(Lua_V2::FadeOutChore)
// STUB_FUNC2(Lua_V2::SetLightPosition)
// STUB_FUNC2(Lua_V2::GetAngleBetweenVectors)
// STUB_FUNC2(Lua_V2::IsPointInSector)

// Monkey specific LUA_OPCODEs
STUB_FUNC2(Lua_V2::ThumbnailFromFile)
STUB_FUNC2(Lua_V2::ClearSpecialtyTexture)
STUB_FUNC2(Lua_V2::ClearOverworld)
STUB_FUNC2(Lua_V2::EnableActorPuck)
STUB_FUNC2(Lua_V2::GetActorSortOrder)
STUB_FUNC2(Lua_V2::IsChoreLooping)
STUB_FUNC2(Lua_V2::PlayChore)
STUB_FUNC2(Lua_V2::PauseChore)
STUB_FUNC2(Lua_V2::CompleteChore)
STUB_FUNC2(Lua_V2::UnlockChore)
STUB_FUNC2(Lua_V2::LockChoreSet)
STUB_FUNC2(Lua_V2::UnlockChoreSet)
STUB_FUNC2(Lua_V2::StopAllSounds)
STUB_FUNC2(Lua_V2::FreeSound)
STUB_FUNC2(Lua_V2::GetSoundVolume)
STUB_FUNC2(Lua_V2::SetSoundVolume)
STUB_FUNC2(Lua_V2::PlaySoundFrom)
STUB_FUNC2(Lua_V2::PlayLoadedSoundFrom)
STUB_FUNC2(Lua_V2::UpdateSoundPosition)
STUB_FUNC2(Lua_V2::ImStateHasLooped)
STUB_FUNC2(Lua_V2::ImPushState)
STUB_FUNC2(Lua_V2::ImPopState)
STUB_FUNC2(Lua_V2::GetSectorName)
STUB_FUNC2(Lua_V2::GetCameraYaw)
STUB_FUNC2(Lua_V2::YawCamera)
STUB_FUNC2(Lua_V2::GetCameraPitch)
STUB_FUNC2(Lua_V2::PitchCamera)
STUB_FUNC2(Lua_V2::RollCamera)
STUB_FUNC2(Lua_V2::NewLayer)
STUB_FUNC2(Lua_V2::FreeLayer)
STUB_FUNC2(Lua_V2::SetLayerSortOrder)
STUB_FUNC2(Lua_V2::SetLayerFrame)
STUB_FUNC2(Lua_V2::AdvanceLayerFrame)
STUB_FUNC2(Lua_V2::PushText)
STUB_FUNC2(Lua_V2::PopText)
STUB_FUNC2(Lua_V2::NukeAllScriptLocks)
STUB_FUNC2(Lua_V2::ToggleDebugDraw)
STUB_FUNC2(Lua_V2::ToggleDrawCameras)
STUB_FUNC2(Lua_V2::ToggleDrawLights)
STUB_FUNC2(Lua_V2::ToggleDrawSectors)
STUB_FUNC2(Lua_V2::ToggleDrawBBoxes)
STUB_FUNC2(Lua_V2::ToggleDrawFPS)
STUB_FUNC2(Lua_V2::ToggleDrawPerformance)
STUB_FUNC2(Lua_V2::ToggleDrawActorStats)
STUB_FUNC2(Lua_V2::SectEditSelect)
STUB_FUNC2(Lua_V2::SectEditPlace)
STUB_FUNC2(Lua_V2::SectEditDelete)
STUB_FUNC2(Lua_V2::SectEditInsert)
STUB_FUNC2(Lua_V2::SectEditSortAdd)
STUB_FUNC2(Lua_V2::SectEditForgetIt)
STUB_FUNC2(Lua_V2::FRUTEY_Begin)
STUB_FUNC2(Lua_V2::FRUTEY_End)

struct luaL_reg monkeyMainOpcodes[] = {
	// Monkey specific LUA_OPCODEs:
	{ "ScreenshotForSavegame", LUA_OPCODE(Lua_V2, ScreenshotForSavegame) },
	{ "GetActorWorldPos", LUA_OPCODE(Lua_V2, GetActorWorldPos) },
	{ "SetActiveCD", LUA_OPCODE(Lua_V2, SetActiveCD) },
	{ "GetActiveCD", LUA_OPCODE(Lua_V2, GetActiveCD) },
	{ "AreWeInternational", LUA_OPCODE(Lua_V2, AreWeInternational) },
	{ "MakeScreenTextures", LUA_OPCODE(Lua_V2, MakeScreenTextures) },
	{ "ThumbnailFromFile", LUA_OPCODE(Lua_V2, ThumbnailFromFile) },
	{ "ClearSpecialtyTexture", LUA_OPCODE(Lua_V2, ClearSpecialtyTexture) },
	{ "UnloadActor", LUA_OPCODE(Lua_V2, UnloadActor) },
	{ "PutActorInOverworld", LUA_OPCODE(Lua_V2, PutActorInOverworld) },
	{ "RemoveActorFromOverworld", LUA_OPCODE(Lua_V2, RemoveActorFromOverworld) },
	{ "ClearOverworld", LUA_OPCODE(Lua_V2, ClearOverworld) },
	{ "ToggleOverworld", LUA_OPCODE(Lua_V2, ToggleOverworld) },
	{ "ActorStopMoving", LUA_OPCODE(Lua_V2, ActorStopMoving) },
	{ "SetActorFOV", LUA_OPCODE(Lua_V2, SetActorFOV) },
	{ "SetActorLighting", LUA_OPCODE(Lua_V2, SetActorLighting) },
	{ "SetActorHeadLimits", LUA_OPCODE(Lua_V2, SetActorHeadLimits) },
	{ "ActorActivateShadow", LUA_OPCODE(Lua_V2, ActorActivateShadow) },
	{ "EnableActorPuck", LUA_OPCODE(Lua_V2, EnableActorPuck) },
	{ "SetActorGlobalAlpha", LUA_OPCODE(Lua_V2, SetActorGlobalAlpha) },
	{ "SetActorLocalAlpha", LUA_OPCODE(Lua_V2, SetActorLocalAlpha) },
	{ "SetActorSortOrder", LUA_OPCODE(Lua_V2, SetActorSortOrder) },
	{ "GetActorSortOrder", LUA_OPCODE(Lua_V2, GetActorSortOrder) },
	{ "AttachActor", LUA_OPCODE(Lua_V2, AttachActor) },
	{ "DetachActor", LUA_OPCODE(Lua_V2, DetachActor) },
	{ "IsChoreValid", LUA_OPCODE(Lua_V2, IsChoreValid) },
	{ "IsChorePlaying", LUA_OPCODE(Lua_V2, IsChorePlaying) },
	{ "IsChoreLooping", LUA_OPCODE(Lua_V2, IsChoreLooping) },
	{ "StopActorChores", LUA_OPCODE(Lua_V2, StopActorChores) },
	{ "PlayChore", LUA_OPCODE(Lua_V2, PlayChore) },
	{ "StopChore", LUA_OPCODE(Lua_V2, StopChore) },
	{ "PauseChore", LUA_OPCODE(Lua_V2, PauseChore) },
	{ "AdvanceChore", LUA_OPCODE(Lua_V2, AdvanceChore) },
	{ "CompleteChore", LUA_OPCODE(Lua_V2, CompleteChore) },
	{ "LockChore", LUA_OPCODE(Lua_V2, LockChore) },
	{ "UnlockChore", LUA_OPCODE(Lua_V2, UnlockChore) },
	{ "LockChoreSet", LUA_OPCODE(Lua_V2, LockChoreSet) },
	{ "UnlockChoreSet", LUA_OPCODE(Lua_V2, UnlockChoreSet) },
	{ "LockBackground", LUA_OPCODE(Lua_V2, LockBackground) },
	{ "UnLockBackground", LUA_OPCODE(Lua_V2, UnLockBackground) },
	{ "EscapeMovie", LUA_OPCODE(Lua_V2, EscapeMovie) },
	{ "StopAllSounds", LUA_OPCODE(Lua_V2, StopAllSounds) },
	{ "LoadSound", LUA_OPCODE(Lua_V2, LoadSound) },
	{ "FreeSound", LUA_OPCODE(Lua_V2, FreeSound) },
	{ "PlayLoadedSound", LUA_OPCODE(Lua_V2, PlayLoadedSound) },
	{ "SetGroupVolume", LUA_OPCODE(Lua_V2, SetGroupVolume) },
	{ "GetSoundVolume", LUA_OPCODE(Lua_V2, GetSoundVolume) },
	{ "SetSoundVolume", LUA_OPCODE(Lua_V2, SetSoundVolume) },
	{ "EnableAudioGroup", LUA_OPCODE(Lua_V2, EnableAudioGroup) },
	{ "EnableVoiceFX", LUA_OPCODE(Lua_V2, EnableVoiceFX) },
	{ "PlaySoundFrom", LUA_OPCODE(Lua_V2, PlaySoundFrom) },
	{ "PlayLoadedSoundFrom", LUA_OPCODE(Lua_V2, PlayLoadedSoundFrom) },
	{ "SetReverb", LUA_OPCODE(Lua_V2, SetReverb) },
	{ "UpdateSoundPosition", LUA_OPCODE(Lua_V2, UpdateSoundPosition) },
	{ "ImSelectSet", LUA_OPCODE(Lua_V2, ImSelectSet) },
	{ "ImStateHasLooped", LUA_OPCODE(Lua_V2, ImStateHasLooped) },
	{ "ImStateHasEnded", LUA_OPCODE(Lua_V2, ImStateHasEnded) },
	{ "ImPushState", LUA_OPCODE(Lua_V2, ImPushState) },
	{ "ImPopState", LUA_OPCODE(Lua_V2, ImPopState) },
	{ "ImFlushStack", LUA_OPCODE(Lua_V2, ImFlushStack) },
	{ "ImGetMillisecondPosition", LUA_OPCODE(Lua_V2, ImGetMillisecondPosition) },
	{ "GetSectorName", LUA_OPCODE(Lua_V2, GetSectorName) },
	{ "GetCameraYaw", LUA_OPCODE(Lua_V2, GetCameraYaw) },
	{ "YawCamera", LUA_OPCODE(Lua_V2, YawCamera) },
	{ "GetCameraPitch", LUA_OPCODE(Lua_V2, GetCameraPitch) },
	{ "PitchCamera", LUA_OPCODE(Lua_V2, PitchCamera) },
	{ "RollCamera", LUA_OPCODE(Lua_V2, RollCamera) },
	{ "UndimAll", LUA_OPCODE(Lua_V2, UndimAll) },
	{ "UndimRegion", LUA_OPCODE(Lua_V2, UndimRegion) },
	{ "GetCPUSpeed", LUA_OPCODE(Lua_V2, GetCPUSpeed) },
	{ "NewLayer", LUA_OPCODE(Lua_V2, NewLayer) },
	{ "FreeLayer", LUA_OPCODE(Lua_V2, FreeLayer) },
	{ "SetLayerSortOrder", LUA_OPCODE(Lua_V2, SetLayerSortOrder) },
	{ "SetLayerFrame", LUA_OPCODE(Lua_V2, SetLayerFrame) },
	{ "AdvanceLayerFrame", LUA_OPCODE(Lua_V2, AdvanceLayerFrame) },
	{ "PushText", LUA_OPCODE(Lua_V2, PushText) },
	{ "PopText", LUA_OPCODE(Lua_V2, PopText) },
	{ "NukeAllScriptLocks", LUA_OPCODE(Lua_V2, NukeAllScriptLocks) },
	{ "ToggleDebugDraw", LUA_OPCODE(Lua_V2, ToggleDebugDraw) },
	{ "ToggleDrawCameras", LUA_OPCODE(Lua_V2, ToggleDrawCameras) },
	{ "ToggleDrawLights", LUA_OPCODE(Lua_V2, ToggleDrawLights) },
	{ "ToggleDrawSectors", LUA_OPCODE(Lua_V2, ToggleDrawSectors) },
	{ "ToggleDrawBBoxes", LUA_OPCODE(Lua_V2, ToggleDrawBBoxes) },
	{ "ToggleDrawFPS", LUA_OPCODE(Lua_V2, ToggleDrawFPS) },
	{ "ToggleDrawPerformance", LUA_OPCODE(Lua_V2, ToggleDrawPerformance) },
	{ "ToggleDrawActorStats", LUA_OPCODE(Lua_V2, ToggleDrawActorStats) },
	{ "SectEditSelect", LUA_OPCODE(Lua_V2, SectEditSelect) },
	{ "SectEditPlace", LUA_OPCODE(Lua_V2, SectEditPlace) },
	{ "SectEditDelete", LUA_OPCODE(Lua_V2, SectEditDelete) },
	{ "SectEditInsert", LUA_OPCODE(Lua_V2, SectEditInsert) },
	{ "SectEditSortAdd", LUA_OPCODE(Lua_V2, SectEditSortAdd) },
	{ "SectEditForgetIt", LUA_OPCODE(Lua_V2, SectEditForgetIt) },
	{ "FRUTEY_Begin", LUA_OPCODE(Lua_V2, FRUTEY_Begin) },
	{ "FRUTEY_End", LUA_OPCODE(Lua_V2, FRUTEY_End) },
	{ "sleep_for", LUA_OPCODE(Lua_V2, SleepFor) }
};

void Lua_V2::registerOpcodes() {
	Lua_V1::registerOpcodes();

	// Register main opcodes functions
	luaL_openlib(monkeyMainOpcodes, ARRAYSIZE(monkeyMainOpcodes));
}

} // end of namespace Grim
