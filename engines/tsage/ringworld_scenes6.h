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
 * $URL$
 * $Id$
 *
 */

#ifndef TSAGE_RINGWORLD_SCENES6_H
#define TSAGE_RINGWORLD_SCENES6_H

#include "common/scummsys.h"
#include "tsage/ringworld_logic.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace tSage {

class Scene5000: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public Action {
	public:
		virtual void signal();
	};
	class Action4: public Action {
	public:
		virtual void signal();
	};
	class Action5: public Action {
	public:
		virtual void signal();
	};
	class Action6: public Action {
	public:
		virtual void signal();
	};

	/* Hotspots */
	class Hotspot7: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot8: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class HotspotGroup1: public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	SoundHandler _soundHandler;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	DisplayHotspot _hotspot1;
	SceneObject _hotspot2, _hotspot3, _hotspot4, _hotspot5, _hotspot6;
	Hotspot7 _hotspot7;
	Hotspot8 _hotspot8;
	HotspotGroup1 _hotspot9, _hotspot10, _hotspot11;
	DisplayHotspot _hotspot12, _hotspot13, _hotspot14, _hotspot15;
	DisplayHotspot _hotspot16, _hotspot17, _hotspot18;

	Scene5000();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

} // End of namespace tSage

#endif
