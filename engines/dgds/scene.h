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

#ifndef DGDS_SCENE_H
#define DGDS_SCENE_H

#include "common/stream.h"
#include "common/array.h"

namespace Dgds {

// TODO: Use Common::Rect instead.
struct Rect {
	int x;
	int y;
	int width;
	int height;

	Common::String dump(const Common::String &indent) const;
};

enum SceneCondition {
	kSceneCondNone = 0,
	kSceneCondLessThan = 1,
	kSceneCondEqual = 2,
	kSceneCondNegate = 4,
	kSceneCondSceneAbsVal = 8,
	kSceneCondAlwaysTrue = 0x10,
	kSceneCondNeedItemField14 = 0x20,
	kSceneCondNeedItemField12 = 0x40,
	kSceneCond80 = 0x80
};

struct SceneConditions {
	uint16 _num;
	SceneCondition _flags; /* eg, see usage in FUN_1f1a_2106 */
	uint16 _val;

	Common::String dump(const Common::String &indent) const;
};

struct SceneStruct2 {
	struct Rect rect;
	uint16 field1_0x8;
	uint16 field2_0xa;
	Common::Array<struct SceneConditions> struct1List;
	Common::Array<struct SceneOp> opList1;
	Common::Array<struct SceneOp> opList2;
	Common::Array<struct SceneOp> opList3;

	virtual Common::String dump(const Common::String &indent) const;
};

enum SceneOpCode {
	kSceneOpNone = 0,
	kSceneOpChangeScene = 1,  	// args: scene num
	kSceneOpNoop = 2,		 	// args: none. Maybe should close dialogue?
	kSceneOp3 = 3,				// args: array of uints
	kSceneOp4 = 4,				// args: array of uint pairs [op arg, op arg], term with 0,0. a script within a script (see disasm at 1f1a:4b51)
	kSceneOp5 = 5,				// args: [item num, ??, ??]. give item?
	kSceneOp6 = 6,				// args: item num?
	kSceneOp7 = 7,				// args: none.
	kSceneOpShowDlg = 8,		// args: dialogue number. show dialogue?
	kSceneOp9 = 9,				// args: none.
	kSceneOp10 = 10,			// args: none. Looks through the struct2 list for something.
	kSceneOpEnableTrigger = 11,	// args: trigger num
	kSceneOp12 = 12,			// args: none. Change scene to stored number (previous?)
	kSceneOp13 = 13,			// args: none.
	kSceneOp14 = 14,			// args: none.
	kSceneOp15 = 15,			// args: none.
	kSceneOp16 = 16,			// args: none.
	kSceneOp17 = 17,			// args: none.
	kSceneOp18 = 18,			// args: none.
	kSceneOp19 = 19,			// args: none.
	kSceneOp100 = 100,			// args: none.
	kSceneOpMeanwhile = 101,	// args: none. Clears screen and displays "meanwhile".
	kSceneOp102 = 102,			// args: none.
	kSceneOp103 = 103,			// args: none.
	kSceneOp104 = 104,			// args: none.
	kSceneOp105 = 105,			// args: none. Draw some number at 141, 56
	kSceneOp106 = 106,			// args: none. Draw some number at 42, 250
	kSceneOp107 = 107,			// args: none.
	kSceneOp108 = 108,			// args: none.
};

struct SceneOp {
	Common::Array<struct SceneConditions> _conditionList;
	Common::Array<uint16> _args;
	SceneOpCode _opCode;

	Common::String dump(const Common::String &indent) const;
};

struct GameItem : public SceneStruct2 {
	Common::Array<struct SceneOp> opList5;
	Common::Array<struct SceneOp> opList6;
	uint16 field10_0x24;
	uint16 _iconNum;
	uint16 field12_0x28;
	uint16 field13_0x2a;
	uint16 field14_0x2c;

	Common::String dump(const Common::String &indent) const override;
};

struct MouseCursor {
	uint16 _hotX;
	uint16 _hotY;
	// pointer to cursor image
	//Common::SharedPtr<Image> _img;

	Common::String dump(const Common::String &indent) const;
};

struct SceneStruct4 {
	uint16 val1;
	uint16 val2;
	Common::Array<struct SceneOp> opList;

	Common::String dump(const Common::String &indent) const;
};

enum DialogueFlags {
	kDlgFlagNone = 0,
	kDlgFlagFlatBg = 1,
	kDlgFlagLeftJust = 2,
	kDlgFlagVisible = 0x8000,
};

enum DialogueFrameType {
	kDlgFramePlain = 1,
	kDlgFrameBorder = 2,
	kDlgFrameThought = 3,
	kDlgFrameRounded = 4
};

class Dialogue {
public:
	Dialogue();
	uint16 _num;
	Rect _rect;
	uint16 _bgColor;
	uint16 _fontColor;
	uint16 _field7_0xe;
	uint16 _field8_0x10;
	uint16 _fontSize;
	DialogueFlags _flags;
	DialogueFrameType _frameType;
	uint16 _time;
	uint16 _nextDialogNum;
	Common::Array<struct DialogueAction> _subStrings;
	uint16 _field15_0x22;
	Common::String _str;
	uint16 _field18_0x28;
	
	uint _hideTime;

 	void draw(Graphics::Surface *dst, int mode);
 	void addFlag(DialogueFlags flg);
 	void clearFlag(DialogueFlags flg);
 	bool hasFlag(DialogueFlags flg) const;

	Common::String dump(const Common::String &indent) const;

private:
	Common::Rect _textDrawRect; // Calculated while drawing the background.

	void drawType1(Graphics::Surface *dst, int mode);
	void drawType2(Graphics::Surface *dst, int mode);
	void drawType3(Graphics::Surface *dst, int mode);
	void drawType4(Graphics::Surface *dst, int mode);

	void drawStage2(Graphics::Surface *dst);
	void drawStage3(Graphics::Surface *dst);
	void drawStage4(Graphics::Surface *dst, uint16 fontcol, const Common::String &txt);
};

struct SceneTrigger {
	uint16 _num;
	bool _enabled;
	Common::Array<struct SceneConditions> conditionList;
	Common::Array<struct SceneOp> sceneOpList;

	Common::String dump(const Common::String &indent) const;
};

struct DialogueAction {
	// The game initializes str offsets to pointers, but let's be a bit nicer.
	uint16 strStart; /// The start of the clickable text for this action
	uint16 strEnd;	 /// End of clickable text for this action
	byte unk[8]; /* Not initialized in loader */
	Common::Array<struct SceneOp> sceneOpList;
	uint val; /* First entry initialized to 1 in loader */

	Common::String dump(const Common::String &indent) const;
};


/**
 * A scene is described by an SDS file, which points to the ADS script to load
 * and holds the dialogue info.
 */
class Scene {
public:
	Scene();
	virtual ~Scene() {};

	virtual bool parse(Common::SeekableReadStream *s) = 0;

	bool isVersionOver(const char *version) const;
	bool isVersionUnder(const char *version) const;

	uint32 getMagic() const { return _magic; }
	const Common::String &getVersion() const { return _version; }

protected:
	bool readConditionList(Common::SeekableReadStream *s, Common::Array<SceneConditions> &list) const;
	bool readStruct2(Common::SeekableReadStream *s, SceneStruct2 &dst) const;
	bool readStruct2List(Common::SeekableReadStream *s, Common::Array<SceneStruct2> &list) const;
	bool readGameItemList(Common::SeekableReadStream *s, Common::Array<GameItem> &list) const;
	bool readMouseHotspotList(Common::SeekableReadStream *s, Common::Array<MouseCursor> &list) const;
	bool readStruct4List(Common::SeekableReadStream *s, Common::Array<SceneStruct4> &list) const;
	bool readOpList(Common::SeekableReadStream *s, Common::Array<SceneOp> &list) const;
	bool readDialogueList(Common::SeekableReadStream *s, Common::Array<Dialogue> &list) const;
	bool readTriggerList(Common::SeekableReadStream *s, Common::Array<SceneTrigger> &list) const;
	bool readDialogSubstringList(Common::SeekableReadStream *s, Common::Array<DialogueAction> &list) const;

	void runOps(const Common::Array<SceneOp> &ops);
	bool checkConditions(const Common::Array<struct SceneConditions> &cond);

	virtual void enableTrigger(uint16 num) {}
	virtual void showDialog(uint16 num) {}

	uint32 _magic;
	Common::String _version;
};

class ResourceManager;
class Decompressor;

class GDSScene : public Scene {
public:
	GDSScene();

	bool load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor);
	bool parse(Common::SeekableReadStream *s) override;
	bool parseInf(Common::SeekableReadStream *s);
	const Common::String &getIconFile() const { return _iconFile; }

	Common::String dump(const Common::String &indent) const;

	void runStartGameOps() { runOps(_startGameOps); }
	void runQuitGameOps() { runOps(_quitGameOps); }

private:
	//byte _unk[32];
	Common::String _iconFile;
	Common::Array<struct GameItem> _gameItems;
	Common::Array<struct SceneOp> _startGameOps;
	Common::Array<struct SceneOp> _quitGameOps;
	Common::Array<struct SceneOp> _opList3;
	Common::Array<struct SceneOp> _opList4;
	Common::Array<struct SceneOp> _opList5;
	Common::Array<struct SceneStruct4> _struct4List1;
	Common::Array<struct SceneStruct4> _struct4List2;
};

class SDSScene : public Scene {
public:
	SDSScene();

	bool load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor);
	bool parse(Common::SeekableReadStream *s) override;
	void unload();

	const Common::String &getAdsFile() const { return _adsFile; }
	void runEnterSceneOps() { runOps(_enterSceneOps); }
	void runLeaveSceneOps() { runOps(_leaveSceneOps); }
	void checkTriggers();

	int getNum() const { return _num; }
	Common::String dump(const Common::String &indent) const;

	bool checkDialogActive();
	bool drawActiveDialog(Graphics::Surface *dst, int mode);

private:
	void enableTrigger(uint16 num) override;
	void showDialog(uint16 num) override;

	int _num;
	Common::Array<struct SceneOp> _enterSceneOps;
	Common::Array<struct SceneOp> _leaveSceneOps;
	Common::Array<struct SceneOp> _opList3;
	Common::Array<struct SceneOp> _opList4;
	//uint _field5_0x12;
	uint _field6_0x14;
	Common::String _adsFile;
	//uint _field8_0x23;
	Common::Array<struct SceneStruct2> _struct2List;
	Common::Array<struct SceneStruct4> _struct4List1;
	Common::Array<struct SceneStruct4> _struct4List2;
	//uint _field12_0x2b;
	Common::Array<class Dialogue> _dialogs;
	Common::Array<struct SceneTrigger> _triggers;
	//uint _field15_0x33;
};

} // End of namespace Dgds

#endif // DGDS_SCENE_H
