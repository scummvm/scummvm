/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"

#include "director/director.h"
#include "director/sound.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/directsound.h"

/**************************************************
 *
 * USED IN:
 * safecracker
 *
 **************************************************/

/*
-- xtra DirectSound
new object me
--
-- DirectSound Xtra(tm) version 1.3.
--
-- Programmed by Tomer Berda, Hed Arzi - Multimedia.
-- Copyright 1996.
--
* dsOpen
* dsNewSound string fileName, integer Mode
* dsDelSound integer ID
* dsDupSound integer ID
* dsPlay integer ID
* dsStop integer ID
* dsGetSize integer ID
* dsGetFreq integer ID
* dsSetFreq integer ID, integer Frequency
* dsGetVolume integer ID
* dsSetVolume integer ID, integer Volume
* dsGetPan integer ID
* dsSetPan integer ID, integer Pan
* dsGetPosition integer ID
* dsSetPosition integer ID, integer Pos
* dsGetLoop integer ID
* dsSetLoop integer ID, integer loopFlag
* dsIsPlaying integer ID
* dsGetCaps
--
* ds3DOpen
* ds3DLGetPosition
* ds3DLSetPosition float X, float Y, float Z
* ds3DLGetOrientation
* ds3DLSetOrientation float xFront,float yFront,float zFront,float xTop,float yTop,float zTop
* ds3DLGetVelocity
* ds3DLSetVelocity float X, float Y, float Z
* ds3DLGetDopplerFactor
* ds3DLSetDopplerFactor float DopplerFactor
* ds3DLGetRolloffFactor
* ds3DLSetRolloffFactor float RolloffFactor
--
* ds3DGetPosition integer ID
* ds3DSetPosition integer ID,float X,float Y,float Z
* ds3DGetOrientation integer ID
* ds3DSetOrientation integer ID, float X, float Y, float Z
* ds3DGetVelocity integer ID
* ds3DSetVelocity integer ID,float X,float Y,float Z
* ds3DGetOutsideVolume integer ID
* ds3DSetOutsideVolume integer ID, integer Volume
* ds3DGetAngles integer ID
* ds3DSetAngles integer ID, integer Inside, integer Outside
* ds3DGetMaxDistance integer ID
* ds3DSetMaxDistance integer ID, float MaxDistance
* ds3DGetMinDistance integer ID
* ds3DSetMinDistance integer ID, float MinDistance
* dsClose

 */

namespace Director {

const char *const DirectsoundXtra::xlibName = "Directsound";
const XlibFileDesc DirectsoundXtra::fileNames[] = {
	{ "directsound",	nullptr },
	{ "Dsound_r", 		nullptr },
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				DirectsoundXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ "dsOpen", DirectsoundXtra::m_dsOpen, 0, 0, 500, HBLTIN },
	{ "dsNewSound", DirectsoundXtra::m_dsNewSound, 2, 2, 500, HBLTIN },
	{ "dsDelSound", DirectsoundXtra::m_dsDelSound, 1, 1, 500, HBLTIN },
	{ "dsDupSound", DirectsoundXtra::m_dsDupSound, 1, 1, 500, HBLTIN },
	{ "dsPlay", DirectsoundXtra::m_dsPlay, 1, 1, 500, HBLTIN },
	{ "dsStop", DirectsoundXtra::m_dsStop, 1, 1, 500, HBLTIN },
	{ "dsGetSize", DirectsoundXtra::m_dsGetSize, 1, 1, 500, HBLTIN },
	{ "dsGetFreq", DirectsoundXtra::m_dsGetFreq, 1, 1, 500, HBLTIN },
	{ "dsSetFreq", DirectsoundXtra::m_dsSetFreq, 2, 2, 500, HBLTIN },
	{ "dsGetVolume", DirectsoundXtra::m_dsGetVolume, 1, 1, 500, HBLTIN },
	{ "dsSetVolume", DirectsoundXtra::m_dsSetVolume, 2, 2, 500, HBLTIN },
	{ "dsGetPan", DirectsoundXtra::m_dsGetPan, 1, 1, 500, HBLTIN },
	{ "dsSetPan", DirectsoundXtra::m_dsSetPan, 2, 2, 500, HBLTIN },
	{ "dsGetPosition", DirectsoundXtra::m_dsGetPosition, 1, 1, 500, HBLTIN },
	{ "dsSetPosition", DirectsoundXtra::m_dsSetPosition, 2, 2, 500, HBLTIN },
	{ "dsGetLoop", DirectsoundXtra::m_dsGetLoop, 1, 1, 500, HBLTIN },
	{ "dsSetLoop", DirectsoundXtra::m_dsSetLoop, 2, 2, 500, HBLTIN },
	{ "dsIsPlaying", DirectsoundXtra::m_dsIsPlaying, 1, 1, 500, HBLTIN },
	{ "dsGetCaps", DirectsoundXtra::m_dsGetCaps, 0, 0, 500, HBLTIN },
	{ "ds3DOpen", DirectsoundXtra::m_ds3DOpen, 0, 0, 500, HBLTIN },
	{ "ds3DLGetPosition", DirectsoundXtra::m_ds3DLGetPosition, 0, 0, 500, HBLTIN },
	{ "ds3DLSetPosition", DirectsoundXtra::m_ds3DLSetPosition, 3, 3, 500, HBLTIN },
	{ "ds3DLGetOrientation", DirectsoundXtra::m_ds3DLGetOrientation, 0, 0, 500, HBLTIN },
	{ "ds3DLSetOrientation", DirectsoundXtra::m_ds3DLSetOrientation, 6, 6, 500, HBLTIN },
	{ "ds3DLGetVelocity", DirectsoundXtra::m_ds3DLGetVelocity, 0, 0, 500, HBLTIN },
	{ "ds3DLSetVelocity", DirectsoundXtra::m_ds3DLSetVelocity, 3, 3, 500, HBLTIN },
	{ "ds3DLGetDopplerFactor", DirectsoundXtra::m_ds3DLGetDopplerFactor, 0, 0, 500, HBLTIN },
	{ "ds3DLSetDopplerFactor", DirectsoundXtra::m_ds3DLSetDopplerFactor, 1, 1, 500, HBLTIN },
	{ "ds3DLGetRolloffFactor", DirectsoundXtra::m_ds3DLGetRolloffFactor, 0, 0, 500, HBLTIN },
	{ "ds3DLSetRolloffFactor", DirectsoundXtra::m_ds3DLSetRolloffFactor, 1, 1, 500, HBLTIN },
	{ "ds3DGetPosition", DirectsoundXtra::m_ds3DGetPosition, 1, 1, 500, HBLTIN },
	{ "ds3DSetPosition", DirectsoundXtra::m_ds3DSetPosition, 4, 4, 500, HBLTIN },
	{ "ds3DGetOrientation", DirectsoundXtra::m_ds3DGetOrientation, 1, 1, 500, HBLTIN },
	{ "ds3DSetOrientation", DirectsoundXtra::m_ds3DSetOrientation, 4, 4, 500, HBLTIN },
	{ "ds3DGetVelocity", DirectsoundXtra::m_ds3DGetVelocity, 1, 1, 500, HBLTIN },
	{ "ds3DSetVelocity", DirectsoundXtra::m_ds3DSetVelocity, 4, 4, 500, HBLTIN },
	{ "ds3DGetOutsideVolume", DirectsoundXtra::m_ds3DGetOutsideVolume, 1, 1, 500, HBLTIN },
	{ "ds3DSetOutsideVolume", DirectsoundXtra::m_ds3DSetOutsideVolume, 2, 2, 500, HBLTIN },
	{ "ds3DGetAngles", DirectsoundXtra::m_ds3DGetAngles, 1, 1, 500, HBLTIN },
	{ "ds3DSetAngles", DirectsoundXtra::m_ds3DSetAngles, 3, 3, 500, HBLTIN },
	{ "ds3DGetMaxDistance", DirectsoundXtra::m_ds3DGetMaxDistance, 1, 1, 500, HBLTIN },
	{ "ds3DSetMaxDistance", DirectsoundXtra::m_ds3DSetMaxDistance, 2, 2, 500, HBLTIN },
	{ "ds3DGetMinDistance", DirectsoundXtra::m_ds3DGetMinDistance, 1, 1, 500, HBLTIN },
	{ "ds3DSetMinDistance", DirectsoundXtra::m_ds3DSetMinDistance, 2, 2, 500, HBLTIN },
	{ "dsClose", DirectsoundXtra::m_dsClose, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DirectsoundXtraObject::DirectsoundXtraObject(ObjectType ObjectType) :Object<DirectsoundXtraObject>("Directsound") {
	_objType = ObjectType;
}

bool DirectsoundXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum DirectsoundXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(DirectsoundXtra::xlibName);
	warning("DirectsoundXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void DirectsoundXtra::open(ObjectType type, const Common::Path &path) {
	DirectsoundXtraObject::initMethods(xlibMethods);
	DirectsoundXtraObject *xobj = new DirectsoundXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
	g_lingo->initBuiltIns(xlibBuiltins);
}

void DirectsoundXtra::close(ObjectType type) {
    DirectsoundXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();
}

void DirectsoundXtra::m_new(int nargs) {
	ARGNUMCHECK(0);
	g_lingo->push(g_lingo->_state->me);
}

void DirectsoundXtra::m_dsOpen(int nargs) {
	ARGNUMCHECK(0);
	g_lingo->push(1); // We are always open
}

void DirectsoundXtra::m_dsNewSound(int nargs) {
	ARGNUMCHECK(2);

	DirectsoundXtraObject *me = (DirectsoundXtraObject *)g_lingo->_globalvars[xlibName].u.obj;

	DirectsoundXtraObject::DXSound newSound;
	newSound.parameter = g_lingo->pop().asInt();
	newSound.fname = g_lingo->pop().asString();

	int newId = -1;

	for (uint i = 0; i < me->_sounds.size(); i++) {
		if (me->_sounds[i].free) {
			newId = i;
			break;
		}
	}

	if (newId == -1) {
		newSound.channel = 1000 + me->_sounds.size();
		me->_sounds.push_back(newSound);
		newId = me->_sounds.size() - 1;
	} else {
		newSound.channel = me->_sounds[newId].channel; // Reuse sound channel
		me->_sounds[newId] = newSound;
	}

	g_lingo->push(Common::String::format("DSoundXtra:%d", newId));
}

static int parseId(Common::String id) {
	if (id.empty() || id.equals("0"))
		return -1;

	if (!id.hasPrefix("DSoundXtra:")) {
		warning("DirectsoundXtra: Malformed sound reference: %s", id.c_str());
		return -1;
	}

	return atoi(&id.c_str()[11]);
}

void DirectsoundXtra::m_dsDelSound(int nargs) {
	ARGNUMCHECK(1);

	DirectsoundXtraObject *me = (DirectsoundXtraObject *)g_lingo->_globalvars[xlibName].u.obj;
	int id = parseId(g_lingo->pop().asString());

	if (id == -1)
		return;

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();

	if (me->_sounds[id].channel != -1)
		sound->stopSound(me->_sounds[id].channel);

	me->_sounds[id].free = true;
}

XOBJSTUB(DirectsoundXtra::m_dsDupSound, 0)

void DirectsoundXtra::m_dsPlay(int nargs) {
	ARGNUMCHECK(1);

	DirectsoundXtraObject *me = (DirectsoundXtraObject *)g_lingo->_globalvars[xlibName].u.obj;
	int id = parseId(g_lingo->pop().asString());

	if (id == -1)
		return;

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();

	if (me->_sounds[id].channel != -1)
		sound->playFile(me->_sounds[id].fname, me->_sounds[id].channel);
}

XOBJSTUB(DirectsoundXtra::m_dsStop, 0)
XOBJSTUB(DirectsoundXtra::m_dsGetSize, 0)
XOBJSTUB(DirectsoundXtra::m_dsGetFreq, 0)
XOBJSTUB(DirectsoundXtra::m_dsSetFreq, 0)
XOBJSTUB(DirectsoundXtra::m_dsGetVolume, 0)

void DirectsoundXtra::m_dsSetVolume(int nargs) {
	ARGNUMCHECK(2);

	DirectsoundXtraObject *me = (DirectsoundXtraObject *)g_lingo->_globalvars[xlibName].u.obj;
	int vol = g_lingo->pop().asInt();
	int id = parseId(g_lingo->pop().asString());

	if (id == -1)
		return;

	// original range is 0..-10000
	vol = (10000 + vol) * 256 / 10000;

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();

	if (me->_sounds[id].channel != -1)
		sound->setChannelVolume(me->_sounds[id].channel, vol);
}

XOBJSTUB(DirectsoundXtra::m_dsGetPan, 0)
XOBJSTUB(DirectsoundXtra::m_dsSetPan, 0)
XOBJSTUB(DirectsoundXtra::m_dsGetPosition, 0)
XOBJSTUB(DirectsoundXtra::m_dsSetPosition, 0)
XOBJSTUB(DirectsoundXtra::m_dsGetLoop, 0)

void DirectsoundXtra::m_dsSetLoop(int nargs) {
	ARGNUMCHECK(2);

	int loops = g_lingo->pop().asInt();
	int id = parseId(g_lingo->pop().asString());

	if (id == -1)
		return;

	if (loops > 1)
		warning("STUB: DirectsoundXtra::m_dsSetLoop(\"DSoundXtra:%d\", %d)", id, loops);
}

void DirectsoundXtra::m_dsIsPlaying(int nargs) {
	ARGNUMCHECK(1);

	DirectsoundXtraObject *me = (DirectsoundXtraObject *)g_lingo->_globalvars[xlibName].u.obj;
	int id = parseId(g_lingo->pop().asString());

	if (id == -1) {
		g_lingo->push(0);
		return;
	}

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();

	if (me->_sounds[id].channel != -1)
		g_lingo->push(sound->isChannelActive(me->_sounds[id].channel) ? 1 : 0);
	else
		g_lingo->push(0);
}

XOBJSTUB(DirectsoundXtra::m_dsGetCaps, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DOpen, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DLGetPosition, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DLSetPosition, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DLGetOrientation, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DLSetOrientation, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DLGetVelocity, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DLSetVelocity, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DLGetDopplerFactor, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DLSetDopplerFactor, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DLGetRolloffFactor, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DLSetRolloffFactor, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DGetPosition, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DSetPosition, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DGetOrientation, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DSetOrientation, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DGetVelocity, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DSetVelocity, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DGetOutsideVolume, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DSetOutsideVolume, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DGetAngles, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DSetAngles, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DGetMaxDistance, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DSetMaxDistance, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DGetMinDistance, 0)
XOBJSTUB(DirectsoundXtra::m_ds3DSetMinDistance, 0)

void DirectsoundXtra::m_dsClose(int nargs) {
	ARGNUMCHECK(0);

	DirectsoundXtraObject *me = (DirectsoundXtraObject *)g_lingo->_globalvars[xlibName].u.obj;
	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();

	for (uint i = 0; i < me->_sounds.size(); i++) {
		if (me->_sounds[i].channel != -1)
			sound->stopSound(me->_sounds[i].channel);

		me->_sounds[i].free = true;
	}
}

}
