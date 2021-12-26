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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_RESOURCES_ENCOUNTERS_H
#define ASYLUM_RESOURCES_ENCOUNTERS_H

#include "common/array.h"
#include "common/serializer.h"

#include "asylum/eventhandler.h"
#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;

struct EncounterItem : public Common::Serializable {
	uint32 speechResourceId;
	ResourceId scriptResourceId;
	int16 keywords[50];
	byte variable2;

	EncounterItem() {
		speechResourceId = 0;
		scriptResourceId = kResourceNone;
		memset(&keywords, 0, sizeof(keywords));
		variable2 = 0;
	}

	virtual ~EncounterItem() {}

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s) {
		s.syncAsSint32LE(speechResourceId);
		s.syncAsSint32LE(scriptResourceId);

		for (int32 i = 0; i < ARRAYSIZE(keywords); i++)
			s.syncAsSint16LE(keywords[i]);

		s.syncAsByte(variable2);
	}
};

class EncounterVariables : public Common::Array<int16>, public Common::Serializable {
public:
	virtual ~EncounterVariables() {}

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s) {
		for (uint i = 0; i < _size; i++)
			s.syncAsSint16LE(_storage[i]);
	}
};

class EncounterItems : public Common::Array<EncounterItem>, public Common::Serializable {
public:
	virtual ~EncounterItems() {}

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s) {
		for (uint i = 0; i < _size; i++)
			_storage[i].saveLoadWithSerializer(s);
	}
};

class Encounter : public EventHandler {
public:
	Encounter(AsylumEngine *engine);
	virtual ~Encounter() {};

	void run(int32 encounterIndex, ObjectId objectId1, ObjectId objectId2, ActorIndex actorIndex);

	bool handleEvent(const AsylumEvent &evt);

	void drawScreen();

	void setShouldEnablePlayer(bool state) { _shouldEnablePlayer = state; }
	bool shouldEnablePlayer() { return _shouldEnablePlayer; }

	// Accessors (for saving game)
	EncounterItems *items() { return &_items; }
	EncounterVariables *variables() { return &_variables; }

private:
	AsylumEngine *_vm;

	//////////////////////////////////////////////////////////////////////////
	// Data
	enum KeywordOptions {
		kKeywordOptionsDisabled = 0x20,
		kKeywordOptionsUnknown  = 0x40,
		kKeywordOptionsVisible  = 0x80
	};

	struct EncounterGraphic {
		uint32 frameIndex;
		uint32 frameCount;
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
		uint32 frameIndex;
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

	EncounterVariables _variables;
	EncounterItems _items;
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
	int32 _speechResourceId;
	ResourceId _soundResourceId;
	EncounterItem *_item;
	ObjectId _objectId1;
	ObjectId _objectId2;
	ObjectId _objectId3;
	ActorIndex _actorIndex;

	int16 _value1;
	uint32 _tick;

	// Internal data
	int32 _data_455B14;
	int16 _data_455B3C;
	int16 _data_455B70;
	bool _data_455BCC;
	bool _isDialogOpen;
	bool _shouldCloseDialog;
	bool _data_455BD8;
	bool _data_455BDC;
	bool _data_455BE0;
	bool _shouldCloseBackground;
	bool _data_455BE8;
	int16 _data_455BF0;
	uint32 _data_455BF4;
	uint32 _keywordStartIndex;
	uint32 _keywordsOffset;

	// Internal flags
	bool _shouldEnablePlayer;
	bool _wasPlayerDisabled;
	bool _isClosing;
	bool _isScriptRunning;

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
	void setVariable(uint32 index, int16 val);
	int16 getVariable(uint32 index);
	int16 getVariableInv(int16 index);

	/////////////////////////////////////////////////////////////////////////
	// Actions
	uint32 findKeyword(EncounterItem *item, int16 keyword) const;
	int32 getKeywordIndex();
	void choose(int32 keywordIndex);
	bool checkKeywords() const;
	bool checkKeywords2() const;
	void updateFromRect(int32 rectIndex);

	//////////////////////////////////////////////////////////////////////////
	// Speech
	void resetSpeech(int16 a1, int16 a2);
	void setupPortraits();
	void setupSpeechText();
	void setupSpeechData(char val, EncounterGraphic *encounterGraphic) const;
	void setupSpeech(ResourceId textResourceId, ResourceId fontResourceId);
	bool setupSpeechTest(ResourceId id);
	bool isSpeaking();

	//////////////////////////////////////////////////////////////////////////
	// Drawing
	bool drawBackground();
	bool drawPortraits();
	void drawStructs();
	void drawDialogOptions();
	void drawSubtitle(char *text, ResourceId font, int16 y);

	//////////////////////////////////////////////////////////////////////////
	// Misc
	void exitEncounter();
	void setupEntities(bool type4);
	int32 findRect();
	void updateDrawingStatus();
	void updateDrawingStatus1(int32 rectIndex);
	void updateDrawingStatus2(int32 rectIndex);
	bool updateScreen();
	void updatePalette1();
	void updatePalette2();

	bool isKeywordVisible(int16 keyword) const  { return (bool)(BYTE1(keyword) & kKeywordOptionsVisible); }
	bool isKeywordDisabled(int16 keyword) const { return (bool)(BYTE1(keyword) & kKeywordOptionsDisabled); }

	//////////////////////////////////////////////////////////////////////////
	// Scripts
	enum EncounterOpcode {
		kOpcodeEncounterReturn                  = 0,
		kOpcodeSetScriptVariable                = 1,
		kOpcodeSetCounterFromVars               = 2,
		kOpcodeSetOffset                        = 3,
		kOpcodeSetOffsetIfCounterNegative       = 4,
		kOpcodeSetOffsetIfCounterNegativeOrNull = 5,
		kOpcodeSetOffsetIfCounterIsNull         = 6,
		kOpcodeSetOffsetIfCounterIsNotNull      = 7,
		kOpcodeSetOffsetIfCounterPositiveOrNull = 8,
		kOpcodeSetOffsetIfCounterPositive       = 9,
		kOpcodeSetCurrentItemOptions            = 10,
		kOpcodeClearCurrentItemOptions          = 11,
		kOpcodeSetItemOptions                   = 12,
		kOpcodeCloseDialog                      = 13,
		kOpcodeResetSpeech                      = 14,
		kOpcodeSetVariable                      = 15,
		kOpcodeIncrementScriptVariable          = 16,
		kOpcodeProcessVariable3                 = 17,
		kOpcodeAddRemoveInventoryItem           = 18,
		kOpcodeSetCounterIfInventoryOmits       = 21,
		kOpcodePrepareMovie                     = 23,
		kOpcodeSetClearGameFlag                 = 24,
		kOpcodeSetCounterFromGameFlag           = 25
	};

	struct ScriptEntry {
		byte opcode;
		byte param1;
		uint16 param2;

		ScriptEntry(byte *data) {
			opcode = *data;
			param1 = *(data + 1);
			param2 = READ_LE_UINT16(data + 2);
		}

		Common::String toString();
	};

	struct ScriptData {
		int32 vars[40];
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

#endif // ASYLUM_RESOURCES_ENCOUNTERS_H
