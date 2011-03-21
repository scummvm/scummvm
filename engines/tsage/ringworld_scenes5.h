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

#ifndef TSAGE_RINGWORLD_SCENES5_H
#define TSAGE_RINGWORLD_SCENES5_H

#include "common/scummsys.h"
#include "tsage/core.h"
#include "tsage/converse.h"
#include "tsage/ringworld_logic.h"

namespace tSage {

class Scene4000: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
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
	class Action7: public Action {
	public:
		virtual void signal();
	};
	class Action8: public Action {
	public:
		virtual void signal();
	};
	class Action9: public Action {
	public:
		virtual void signal();
	};
	class Action10: public Action {
	public:
		virtual void signal();
	};
	class Action11: public Action {
	public:
		virtual void signal();
	};
	class Action12: public Action {
	public:
		virtual void signal();
	};
	class Action13: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	SoundHandler _soundHandler1, _soundHandler2;
	SpeakerQR _speakerQR;
	SpeakerML _speakerML;
	SpeakerMR _speakerMR;
	SpeakerSR _speakerSR;
	SpeakerCHFL _speakerCHFL;
	SpeakerPL _speakerPL;
	SpeakerPText _speakerPText;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	SpeakerMText _speakerMText;
	SpeakerCHFR _speakerCHFR;
	SpeakerQL _speakerQL;
	SpeakerCHFText _speakerCHFText;
	SceneObject _hotspot1, _hotspot2, _hotspot3, _hotspot4, _hotspot5;
	SceneObject _hotspot6, _hotspot7, _hotspot8, _hotspot9, _hotspot10;
	SceneObject _hotspot11, _hotspot12, _hotspot13, _hotspot14, _hotspot15;
	SceneObject _hotspot16, _hotspot17, _hotspot18, _hotspot19, _hotspot20;
	SceneObject _hotspot21, _hotspot22, _hotspot23, _hotspot24, _hotspot25;
	SceneObject _hotspot26, _hotspot27;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	Action9 _action9;
	Action10 _action10;
	Action11 _action11;
	Action12 _action12;
	Action13 _action13;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

} // End of namespace tSage

#endif
