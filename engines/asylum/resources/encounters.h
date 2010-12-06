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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_ENCOUNTERS_H
#define ASYLUM_ENCOUNTERS_H

#include "asylum/asylum.h"
#include "asylum/eventhandler.h"
#include "asylum/shared.h"

#include "common/array.h"

namespace Asylum {

class AsylumEngine;

class Encounter : public EventHandler {
public:
	Encounter(AsylumEngine *engine);
	virtual ~Encounter();

	void run(int32 encounterIndex, ObjectId objectId1, ObjectId objectId2, ActorIndex actorIndex);

	bool handleEvent(const AsylumEvent &evt);

	void drawScreen();

	void setFlag1(bool state) { _flag1 = state; }
	bool getFlag1() { return _flag1; }
	void setFlag3(bool state) { _flag3 = state; }
	bool getFlag3() { return _flag3; }
	void setFlag5(bool state) { _flag5 = state; }
	bool getFlag6() { return _flag6; }

private:
	AsylumEngine *_vm;

	//////////////////////////////////////////////////////////////////////////
	// Data
	enum EncounterArray {
		kEncounterArray2000 = 0x2000,
		kEncounterArray4000 = 0x4000,
		kEncounterArray8000 = 0x8000
	};

	struct EncounterItem {
		int16 keywordIndex;
		int16 field2;
		ResourceId scriptResourceId;
		int16 keywords[50];
		byte value;
	};

	struct EncounterGraphic {
		int32 frameIndex;
		int32 frameCount;
		Common::Rect rect;
		ResourceId resourceId;
		int32 transTableNum;
		int32 transTableMax;
		int32 speech0;
		int32 speech1;
		int32 speech2;
		int32 speech3;

		EncounterGraphic() {
			frameIndex = 0;
			frameCount = 0;
			resourceId = kResourceNone;
			transTableNum = 0;
			transTableMax = 0;
			speech0 = 0;
			speech1 = 0;
			speech2 = 0;
			speech3 = 0;
		}
	};

	struct EncounterDrawingStruct {
		Common::Point point1;
		Common::Point point2;
		int32 frameIndex;
		int32 transTableNum;
		int32 status;
		ResourceId resourceId;

		EncounterDrawingStruct() {
			frameIndex = 0;
			transTableNum = -1;
			status = 0;
			resourceId = kResourceNone;
		}
	};

	Common::Array<int16> _variables;
	Common::Array<EncounterItem> _items;
	EncounterDrawingStruct _drawingStructs[2];
	int32 _keywordIndexes[50];

	// Background & portrait
	EncounterGraphic _background;
	EncounterGraphic _portrait1;
	EncounterGraphic _portrait2;
	Common::Point _point;

	int32 _rectIndex;

	// Running encounter data
	int32 _index;
	int32 _keywordIndex;
	EncounterItem *_item;
	ObjectId _objectId1;
	ObjectId _objectId2;
	ObjectId _objectId3;
	ActorIndex _actorIndex;

	uint32 _value1;

	// Internal data
	int32 _data_455B14;
	bool _data_455B3C;
	bool _data_455BCC;
	bool _data_455BD0;
	bool _data_455BD4;
	bool _data_455BD8;
	bool _data_455BDC;
	bool _data_455BE0;
	bool _data_455BE4;
	bool _data_455BE8;
	uint32 _data_455BF4;
	bool _data_455BF8;

	// Internal flags
	bool _flag1;
	bool _flag2;
	bool _flag3;
	bool _flag4;
	bool _flag5;
	bool _flag6;

	//////////////////////////////////////////////////////////////////////////
	// Data
	void load();
	void initData();
	void initBackground();
	void initPortraits();
	void initDrawStructs();

	//////////////////////////////////////////////////////////////////////////
	// Message handling
	bool init();
	bool update();
	bool key(const AsylumEvent &evt);
	bool mouse(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Variables
	void setVariable(uint32 index, int32 val);\
	int32 getVariable(uint32 index);
	int32 getVariableInv(int32 index);

	/////////////////////////////////////////////////////////////////////////
	// Actions
	uint32 findKeyword(EncounterItem *item, int16 keyword);
	int32 getKeywordIndex();
	void choose(int32 keywordIndex);
	bool checkKeywords();
	bool checkKeywords2();
	void updateFromRect(int32 rectIndex);
	
	//////////////////////////////////////////////////////////////////////////
	// Speech
	void resetSpeech(uint32 a1, uint32 a2);
	void setupPortraits();
	void setupSpeechText();
	void setupSpeechData(char val, EncounterGraphic *encounterGraphic);
	void setupSpeech(ResourceId textResourceId, ResourceId fontResourceId);
	bool setupSpeech(ResourceId id);
	bool isSpeaking();

	//////////////////////////////////////////////////////////////////////////
	// Drawing
	bool drawBackground();
	bool drawPortraits();
	void drawStructs();
	void drawDialog();
	void drawText(char *text, ResourceId font, int32 y);

	//////////////////////////////////////////////////////////////////////////
	// Misc	
	void exit();
	void setupEntities(bool type4);
	int32 findRect();
	void updateDrawingStatus();
	void updateDrawingStatus1(int32 rectIndex);
	void updateDrawingStatus2(int32 rectIndex);
	bool updateScreen();

	//////////////////////////////////////////////////////////////////////////
	// Scripts
	struct ScriptEntry {
		union {
			byte opcode;
			byte param1;
			uint16 param2;
			uint32 data;
		};

		ScriptEntry(uint32 val) {
			data = val;
		}

		Common::String toString() {
			return Common::String::format("[0x%02X] %d %d", opcode, param1, param2);
		}
	};

	struct ScriptData {
		int32 vars[10];
		uint32 offset;
		int32 counter;
		ResourceId resourceId;

		ScriptData() {
			reset(kResourceNone);
		}

		void reset(ResourceId id) {
			memset(&vars, 0, sizeof(vars));
			offset = 0;
			counter = 0;
			resourceId = id;
		}
	};

	ScriptData _scriptData;

	void initScript(ResourceId resourceId);
	ScriptEntry getScriptEntry(ResourceId resourceId, uint32 offset);
	void runScript();

	friend class Console;
};

} // end of namespace Asylum

#endif
