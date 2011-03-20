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

#ifndef TSAGE_RINGWORLD_SCENES4_H
#define TSAGE_RINGWORLD_SCENES4_H

#include "common/scummsys.h"
#include "tsage/core.h"
#include "tsage/converse.h"
#include "tsage/ringworld_logic.h"

namespace tSage {

class Scene3500: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
public:
	SpeakerSText _speakerSText;
	SpeakerMText _speakerMText;
	SpeakerQText _speakerQText;
	Action1 _action1;
	Action2 _action2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene3700: public Scene {
	/* Custom classes */
	class Viewer: public SceneObject {
	public:
		GfxSurface _imgList[4];
		int _field88;
		int _percent;
		int _field94;
		int _field96;
		int _field98;

		Viewer();
		virtual void reposition();
		virtual void draw();
	};

	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
public:
	Viewer _viewer;
	Action _action1;
	SceneObject _hotspot1, _hotspot2;
	SpeakerSText _speakerSText;
	SpeakerMText _speakerMText;
	SpeakerMR _speakerMR;
	SoundHandler _soundHandler;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

} // End of namespace tSage

#endif
