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
 */

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#ifndef MACVENTURE_WORLD_H
#define MACVENTURE_WORLD_H

#include "macventure/container.h"
#include "macventure/text.h"

namespace MacVenture {

typedef uint32 ObjID;
typedef uint16 Attribute;
typedef Common::Array<Attribute> AttributeGroup;
class TextAsset;

enum ObjectAttributeID {
	kAttrParentObject = 0,
	kAttrPosX = 1,
	kAttrPosY = 2,
	kAttrInvisible = 3,
	kAttrUnclickable = 4,
	kAttrUndraggable = 5,
	kAttrContainerOpen = 6,
	kAttrPrefixes = 7,
	kAttrIsExit = 8,
	kAttrExitX = 9,
	kAttrExitY = 10,
	kAttrHiddenExit = 11,
	kAttrOtherDoor = 12,
	kAttrIsOpen = 13,
	kAttrIsLocked = 14,
	kAttrWeight = 16,
	kAttrSize = 17,
	kAttrHasDescription = 19,
	kAttrIsDoor = 20,
	kAttrIsContainer = 22,
	kAttrIsOperable = 23,
	kAttrIsEnterable = 24,
	kAttrIsEdible = 25
};

class SaveGame {
public:
	SaveGame(MacVentureEngine *engine, Common::SeekableReadStream *res);
	~SaveGame();

	Attribute getAttr(ObjID objID, uint32 attrID);
	void setAttr(uint32 attrID, ObjID objID, Attribute value);

	void setGlobal(uint32 attrID, Attribute value);
	const Common::Array<uint16> &getGlobals();

	const Common::Array<AttributeGroup> &getGroups();
	const AttributeGroup *getGroup(uint32 groupID);
	const Common::String &getText();

	void saveInto(Common::OutSaveFile *file);

private:
	void loadGroups(MacVentureEngine *engine, Common::SeekableReadStream *res);
	void loadGlobals(MacVentureEngine *engine, Common::SeekableReadStream *res);
	void loadText(MacVentureEngine *engine, Common::SeekableReadStream *res);

private:
	Common::Array<AttributeGroup> _groups;
	Common::Array<uint16> _globals;
	Common::String _text;
};

class World {
public:
	World(MacVentureEngine *engine, Common::MacResManager *resMan);
	~World();

	void startNewGame();

	void setObjAttr(ObjID objID, uint32 attrID, Attribute value);
	void setGlobal(uint32 attrID, Attribute value);
	void updateObj(ObjID objID);
	void captureChildren(ObjID objID);
	void releaseChildren(ObjID objID);

	uint32 getObjAttr(ObjID objID, uint32 attrID);
	Attribute getGlobal(uint32 attrID);
	Common::String getText(ObjID objID, ObjID source, ObjID target);

	bool isObjActive(ObjID objID);

	ObjID getAncestor(ObjID objID);
	Common::Array<ObjID> getFamily(ObjID objID, bool recursive);
	Common::Array<ObjID> getChildren(ObjID objID, bool recursive);

	void loadGameFrom(Common::InSaveFile *file);
	void saveGameInto(Common::OutSaveFile *file);

private:
	bool isObjDraggable(ObjID objID);
	bool intersects(ObjID objID, Common::Rect rect);

	void calculateObjectRelations();
	void setParent(ObjID child, ObjID newParent);

private:
	MacVentureEngine *_engine;
	Common::MacResManager *_resourceManager;

	Common::String _startGameFileName;

	SaveGame *_saveGame;

	Container *_objectConstants;
	Container *_gameText;

	Common::Array<ObjID> _relations; // Parent-child relations, stored in Williams Heap format
};

} // End of namespace MacVenture

#endif
