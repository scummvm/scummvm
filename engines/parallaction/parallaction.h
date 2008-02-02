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

#ifndef PARALLACTION_H
#define PARALLACTION_H

#include "common/str.h"
#include "common/stack.h"
#include "common/array.h"
#include "common/savefile.h"

#include "engines/engine.h"

#include "parallaction/defs.h"
#include "parallaction/inventory.h"
#include "parallaction/parser.h"
#include "parallaction/objects.h"
#include "parallaction/disk.h"
#include "parallaction/walk.h"


extern OSystem *g_system;

namespace Parallaction {

enum {
	kDebugDisk = 1 << 0,
	kDebugWalk = 1 << 1,
	kDebugParser = 1 << 2,
	kDebugDialogue = 1 << 3,
	kDebugGraphics = 1 << 4,
	kDebugExec = 1 << 5,
	kDebugInput = 1 << 6,
	kDebugAudio = 1 << 7,
	kDebugMenu = 1 << 8,
	kDebugInventory = 1 << 9
};

enum {
	GF_DEMO = 1 << 0,
	GF_LANG_EN = 1 << 1,
	GF_LANG_FR = 1 << 2,
	GF_LANG_DE = 1 << 3,
	GF_LANG_IT = 1 << 4,
	GF_LANG_MULT = 1 << 5
};


// high values mean high priority

enum {
	kPriority0 = 0,
	kPriority1 = 1,
	kPriority2 = 2,
	kPriority3 = 3,
	kPriority4 = 4,
	kPriority5 = 5,
	kPriority6 = 6,
	kPriority7 = 7,
	kPriority8 = 8,
	kPriority9 = 9,
	kPriority10 = 10,
	kPriority11 = 11,
	kPriority12 = 12,
	kPriority13 = 13,
	kPriority14 = 14,
	kPriority15 = 15,
	kPriority16 = 16,
	kPriority17 = 17,
	kPriority18 = 18,
	kPriority19 = 19,
	kPriority20 = 20,
	kPriority21 = 21
};

enum {
	kMouseNone			= 0,
	kMouseLeftUp		= 1,
	kMouseLeftDown		= 2,
	kMouseRightUp		= 3,
	kMouseRightDown		= 4
};

enum EngineFlags {
	kEngineQuit			= (1 << 0),
	kEnginePauseJobs	= (1 << 1),
	kEngineInventory	= (1 << 2),
	kEngineWalking		= (1 << 3),
	kEngineChangeLocation	= (1 << 4),
	kEngineBlockInput	= (1 << 5),
	kEngineDragging		= (1 << 6),
	kEngineTransformedDonna	= (1 << 7)
};

enum {
	kEvNone			= 0,
	kEvEnterZone		= 1,
	kEvExitZone		= 2,
	kEvAction		= 3,
	kEvOpenInventory	= 4,
	kEvCloseInventory	= 5,
	kEvHoverInventory	= 6,
	kEvWalk			= 7,
	kEvQuitGame		= 1000,
	kEvSaveGame		= 2000,
	kEvLoadGame		= 4000
};

enum {
	kCursorArrow = -1
};

enum ParallactionGameType {
	GType_Nippon = 1,
	GType_BRA
};

struct PARALLACTIONGameDescription;



extern uint16		_mouseButtons;
extern char			_password[8];
extern uint16		_score;
extern uint16		_language;
extern uint32		_engineFlags;
extern Command		*_forwardedCommands[];
extern char			_forwardedAnimationNames[][20];
extern uint16		_numForwards;
extern char			_slideText[][40];
extern uint16		_introSarcData3;		 // sarcophagus stuff to be saved
extern uint16		_introSarcData2;		 // sarcophagus stuff to be saved
extern char			_saveData1[];
extern uint32		_commandFlags;
extern const char	*_dinoName;
extern const char	*_donnaName;
extern const char	*_doughName;
extern const char	*_drkiName;
extern const char	*_minidinoName;
extern const char	*_minidonnaName;
extern const char	*_minidoughName;
extern const char	*_minidrkiName;


void waitUntilLeftClick();




class Debugger;
class Gfx;
class SoundMan;


struct Location {

	Common::Point	_startPosition;
	uint16			_startFrame;
	char			_name[100];

	CommandList		_aCommands;
	CommandList		_commands;
	char	   *_comment;
	char	   *_endComment;

	// NS specific
	WalkNodeList	_walkNodes;

	// BRA specific
	CommandList		_escapeCommands;
};




struct Character {
	Parallaction	*_vm;


	Animation		_ani;
	Frames			*_head;
	Frames			*_talk;
	Frames			*_objs;
	PathBuilder		_builder;
	WalkNodeList	*_walkPath;

	Character(Parallaction *vm);
	void getFoot(Common::Point &foot);
	void setFoot(const Common::Point &foot);
	void scheduleWalk(int16 x, int16 y);

	void free();

protected:
	const char *_prefix;
	const char *_suffix;

	bool _dummy;

	char _name[30];
	char _baseName[30];
	char _fullName[30];
	static const char _prefixMini[];
	static const char _suffixTras[];
	static const char _empty[];

public:
	void setName(const char *name);
	const char *getName() const;
	const char *getBaseName() const;
	const char *getFullName() const;
	bool dummy() const;
};





class Opcode {

public:
	virtual void operator()() const = 0;
	virtual ~Opcode() { }
};

template <class T>
class OpcodeImpl : public Opcode {

	typedef void (T::*Fn)();

	T*	_instance;
	Fn	_fn;

public:
	OpcodeImpl(T* instance, const Fn &fn) : _instance(instance), _fn(fn) { }

	void operator()() const {
		(_instance->*_fn)();
	}

};

typedef Common::Array<const Opcode*>	OpcodeSet;



#define DECLARE_UNQUALIFIED_ZONE_PARSER(sig) void locZoneParse_##sig()
#define DECLARE_UNQUALIFIED_ANIM_PARSER(sig) void locAnimParse_##sig()
#define DECLARE_UNQUALIFIED_COMMAND_PARSER(sig) void cmdParse_##sig()
#define DECLARE_UNQUALIFIED_LOCATION_PARSER(sig) void locParse_##sig()
#define DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(sig) void instParse_##sig()

#define DECLARE_UNQUALIFIED_COMMAND_OPCODE(op) void cmdOp_##op()
#define DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(op) void instOp_##op()


#define NUM_LOCATIONS 120

class Parallaction : public Engine {
	friend class Debugger;

public:

	Parallaction(OSystem *syst, const PARALLACTIONGameDescription *gameDesc);
	~Parallaction();

	int init();

	virtual bool loadGame() = 0;
	virtual bool saveGame() = 0;

	uint16		readInput();
	void		updateInput();

	void		waitTime(uint32 t);

	enum {
		kInputModeGame = 0,
		kInputModeComment = 1
	};

	int		_inputMode;

	void updateGameInput();
	void updateCommentInput();

	uint	_lookup;
	Common::Stack<OpcodeSet*>	_opcodes;
	Common::Stack<Table*>		_statements;
	OpcodeSet	*_currentOpcodes;
	Table		*_currentStatements;
	void	pushParserTables(OpcodeSet *opcodes, Table* statements);
	void	popParserTables();
	void	parseStatement();

	OpcodeSet	_commandOpcodes;

	struct {
		Command	*cmd;
		Zone	*z;
	} _cmdRunCtxt;

	OpcodeSet	_instructionOpcodes;

	struct {
		Animation	*a;
		InstructionList::iterator inst;
		uint16		modCounter;
		bool		suspend;
	} _instRunCtxt;


	void		showCursor(bool visible);

	void		pauseJobs();
	void		resumeJobs();

	void		finalizeWalk(WalkNodeList *list);
	int16		selectWalkFrame(const Common::Point& pos, const WalkNode* from);
	void		clipMove(Common::Point& pos, const WalkNode* from);

	Zone		*findZone(const char *name);
	Zone		*hitZone(uint32 type, uint16 x, uint16 y);
	uint16		runZone(Zone*);
	void		freeZones();

	void		runDialogue(SpeakData*);

	void		runCommands(CommandList& list, Zone *z = NULL);

	Animation	*findAnimation(const char *name);
	void		freeAnimations();

	void		setBackground(const char *background, const char *mask, const char *path);
	void		freeBackground();

	Table		*_globalTable;
	Table		*_objectsNames;
	Table		*_zoneTypeNames;
	Table		*_zoneFlagNames;
	Table		*_callableNames;
	Table		*_localFlagNames;

public:
	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

private:
	const PARALLACTIONGameDescription *_gameDescription;

public:
	// info
	int32			_screenWidth;
	int32			_screenHeight;
	int32			_screenSize;

	PathBuffer		*_pathBuffer;

	SoundMan		*_soundMan;

	Gfx*			_gfx;
	Disk*			_disk;

	Character		_char;

	uint32			_localFlags[NUM_LOCATIONS];
	char			_locationNames[NUM_LOCATIONS][32];
	int16			_currentLocationIndex;
	uint16			_numLocations;
	Location		_location;

	InventoryItem	_activeItem;

	Common::Point	_mousePos;
	void			getCursorPos(Common::Point& p) {
		p = _mousePos;
	}

	Zone			*_activeZone;

	ZoneList		_zones;
	AnimationList	_animations;

	Font		*_labelFont;
	Font		*_menuFont;
	Font		*_introFont;
	Font		*_dialogueFont;

	Common::RandomSource _rnd;

protected:		// data

	Debugger	*_debugger;

	struct InputData {
		uint16			_event;
		Common::Point	_mousePos;
		int16		_inventoryIndex;
		Zone*		_zone;
		Label*			_label;
	};

	bool		_mouseHidden;

	// input-only
	InputData	_input;
	bool		_actionAfterWalk;  // actived when the character needs to move before taking an action

	// these two could/should be merged as they carry on the same duty in two member functions,
	// respectively processInput and translateInput
	int16		_procCurrentHoverItem;
	int16		_transCurrentHoverItem;

	uint32		_baseTime;
	char		_characterName1[50];	// only used in changeCharacter

	Common::String	_saveFileName;

	bool		_hasLocationSound;
	char		_locationSound[50];

	Zone		*_hoverZone;


protected:		// members
	bool detectGame(void);

	void		initGlobals();
	void		runGame();
	void		updateView();
	uint32		getElapsedTime();
	void		resetTimer();

	InputData	*translateInput();
	bool		translateGameInput();
	bool		translateInventoryInput();
	void		processInput(InputData*);


	void		scheduleLocationSwitch(const char *location);
	void		doLocationEnterTransition();
	virtual void changeLocation(char *location) = 0;
	virtual void changeCharacter(const char *name) = 0;
	void		allocateLocationSlot(const char *name);
	void		finalizeLocationParsing();
	void		freeLocation();
	void		showLocationComment(const char *text, bool end);

	void		displayComment(ExamineData *data);

	uint16		checkDoor();

	void		freeCharacter();

	int16		pickupItem(Zone *z);

public:
	virtual	void callFunction(uint index, void* parm) { }

	virtual void setArrowCursor() = 0;
	virtual void setInventoryCursor(int pos) = 0;

	virtual void parseLocation(const char* name) = 0;

	void updateDoor(Zone *z);

	virtual void runScripts() = 0;
	virtual void walk() = 0;
	virtual void drawAnimations() = 0;

	void		beep();

public:
	const char **_zoneFlagNamesRes;
	const char **_zoneTypeNamesRes;
	const char **_commandsNamesRes;
	const char **_callableNamesRes;
	const char **_instructionNamesRes;

	void highlightInventoryItem(ItemPosition pos, byte color);
	int16 getHoverInventoryItem(int16 x, int16 y);
	int addInventoryItem(ItemName item);
	int addInventoryItem(ItemName item, uint32 value);
	void dropItem(uint16 v);
	bool isItemInInventory(int32 v);
	const InventoryItem* getInventoryItem(int16 pos);
	int16 getInventoryItemIndex(int16 pos);
	void initInventory();
	void destroyInventory();
	void cleanInventory(bool keepVerbs = true);
	void openInventory();
	void closeInventory();

	Inventory *_inventory;
	InventoryRenderer *_inventoryRenderer;

};


class LocationName {

	Common::String _slide;
	Common::String _character;
	Common::String _location;

	bool _hasCharacter;
	bool _hasSlide;
	char *_buf;

public:
	LocationName();
	~LocationName();

	void bind(const char*);

	const char *location() const {
		return _location.c_str();
	}

	bool hasCharacter() const {
		return _hasCharacter;
	}

	const char *character() const {
		return _character.c_str();
	}

	bool hasSlide() const {
		return _hasSlide;
	}

	const char *slide() const {
		return _slide.c_str();
	}

	const char *c_str() const {
		return _buf;
	}
};


class Parallaction_ns : public Parallaction {

public:
	Parallaction_ns(OSystem* syst, const PARALLACTIONGameDescription *gameDesc) : Parallaction(syst, gameDesc) { }
	~Parallaction_ns();

	int init();
	int go();

public:
	typedef void (Parallaction_ns::*Callable)(void*);

	virtual	void callFunction(uint index, void* parm);
	void setMousePointer(uint32 value);

	bool loadGame();
	bool saveGame();


private:
	void initFonts();
	void freeFonts();
	void renameOldSavefiles();
	Common::String genSaveFileName(uint slot, bool oldStyle = false);
	Common::InSaveFile *getInSaveFile(uint slot);
	Common::OutSaveFile *getOutSaveFile(uint slot);
	bool allPartsComplete();
	void setPartComplete(const Character& character);

private:
	void changeLocation(char *location);
	void changeCharacter(const char *name);
	void cleanupGame();

	void setArrowCursor();
	void setInventoryCursor(int pos);


	void doLoadGame(uint16 slot);
	void doSaveGame(uint16 slot, const char* name);
	int  buildSaveFileList(Common::StringList& l);
	int  selectSaveFile(uint16 arg_0, const char* caption, const char* button);

	void initResources();
	void initCursors();

	static byte			_mouseArrow[256];
	Frames			*_mouseComposedArrow;

	static const Callable _dosCallables[25];
	static const Callable _amigaCallables[25];

	// common callables
	void _c_play_boogie(void*);
	void _c_startIntro(void*);
	void _c_endIntro(void*);
	void _c_moveSheet(void*);
	void _c_sketch(void*);
	void _c_shade(void*);
	void _c_score(void*);
	void _c_fade(void*);
	void _c_moveSarc(void*);
	void _c_contaFoglie(void*);
	void _c_zeroFoglie(void*);
	void _c_trasformata(void*);
	void _c_offMouse(void*);
	void _c_onMouse(void*);
	void _c_setMask(void*);
	void _c_endComment(void*);
	void _c_frankenstein(void*);
	void _c_finito(void*);
	void _c_ridux(void*);
	void _c_testResult(void*);

	// dos specific callables
	void _c_null(void*);

	// amiga specific callables
	void _c_projector(void*);
	void _c_HBOff(void*);
	void _c_offSound(void*);
	void _c_startMusic(void*);
	void _c_closeMusic(void*);
	void _c_HBOn(void*);

	const Callable *_callables;

protected:
	void runScripts();
	void walk();
	void drawAnimations();


	// location parser
	OpcodeSet	_locationParsers;
	OpcodeSet	_locationZoneParsers;
	OpcodeSet	_locationAnimParsers;
	OpcodeSet	_commandParsers;
	Table		*_commandsNames;
	Table		*_locationStmt;
	Table		*_locationZoneStmt;
	Table		*_locationAnimStmt;

	struct LocationParserContext {
		bool		end;

		const char	*filename;
		Script		*script;
		Zone		*z;
		Animation	*a;
		int			nextToken;
		CommandList *list;
		bool		endcommands;
		Command		*cmd;

		// BRA specific
		int numZones;
	} _locParseCtxt;

	void warning_unexpected();

	DECLARE_UNQUALIFIED_LOCATION_PARSER(endlocation);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(location);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(disk);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(nodes);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(zone);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(animation);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(localflags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(commands);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(acommands);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(flags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(comment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(endcomment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(sound);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(music);
	DECLARE_UNQUALIFIED_ZONE_PARSER(limits);
	DECLARE_UNQUALIFIED_ZONE_PARSER(moveto);
	DECLARE_UNQUALIFIED_ZONE_PARSER(type);
	DECLARE_UNQUALIFIED_ZONE_PARSER(commands);
	DECLARE_UNQUALIFIED_ZONE_PARSER(label);
	DECLARE_UNQUALIFIED_ZONE_PARSER(flags);
	DECLARE_UNQUALIFIED_ZONE_PARSER(endzone);
	DECLARE_UNQUALIFIED_ZONE_PARSER(null);
	DECLARE_UNQUALIFIED_ANIM_PARSER(script);
	DECLARE_UNQUALIFIED_ANIM_PARSER(commands);
	DECLARE_UNQUALIFIED_ANIM_PARSER(type);
	DECLARE_UNQUALIFIED_ANIM_PARSER(label);
	DECLARE_UNQUALIFIED_ANIM_PARSER(flags);
	DECLARE_UNQUALIFIED_ANIM_PARSER(file);
	DECLARE_UNQUALIFIED_ANIM_PARSER(position);
	DECLARE_UNQUALIFIED_ANIM_PARSER(moveto);
	DECLARE_UNQUALIFIED_ANIM_PARSER(endanimation);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(flags);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(animation);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(zone);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(location);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(drop);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(call);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(simple);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(move);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(endcommands);

	virtual void parseGetData(Script &script, Zone *z);
	virtual void parseExamineData(Script &script, Zone *z);
	virtual void parseDoorData(Script &script, Zone *z);
	virtual void parseMergeData(Script &script, Zone *z);
	virtual void parseHearData(Script &script, Zone *z);
	virtual void parseSpeakData(Script &script, Zone *z);

	void		parseLocation(const char *filename);
	char		*parseComment(Script &script);
	char		*parseDialogueString(Script &script);
	Dialogue	*parseDialogue(Script &script);
	void		resolveDialogueForwards(Dialogue *dialogue, uint numQuestions, Table &forwards);
	Answer		*parseAnswer(Script &script);
	Question	*parseQuestion(Script &script);

	void		parseZone(Script &script, ZoneList &list, char *name);
	void		parseZoneTypeBlock(Script &script, Zone *z);
	void		parseWalkNodes(Script& script, WalkNodeList &list);
	Animation	*parseAnimation(Script &script, AnimationList &list, char *name);
	void		parseCommands(Script &script, CommandList&);
	void		parseCommandFlags();
	void		createCommand(uint id);
	void		addCommand();
	void		initOpcodes();
	void		initParsers();

	// program parser
	OpcodeSet	_instructionParsers;
	Table		*_instructionNames;

	struct {
		bool		end;
		Animation	*a;
		Instruction *inst;
		LocalVariable *locals;
		Program		*program;

		// BRA specific
		Instruction *openIf;
	} _instParseCtxt;

	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(defLocal);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(animation);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(loop);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(x);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(y);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(z);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(f);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(inc);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(set);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(move);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(put);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(call);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(sound);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(null);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(endscript);

	void		parseInstruction(Animation *a, LocalVariable *locals);
	void		loadProgram(Animation *a, const char *filename);
	void		parseLValue(ScriptVar &var, const char *str);
	virtual void	parseRValue(ScriptVar &var, const char *str);
	void		wrapLocalVar(LocalVariable *local);

	DECLARE_UNQUALIFIED_COMMAND_OPCODE(invalid);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(set);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(clear);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(start);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(speak);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(get);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(location);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(open);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(close);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(on);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(off);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(call);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(toggle);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(drop);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(quit);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(move);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(stop);

	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(invalid);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(on);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(off);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(loop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endloop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(null);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(call);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(inc);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(set);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(put);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(wait);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(start);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(sound);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(move);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endscript);

	void		selectStartLocation();

	void		guiStart();
	int			guiSelectCharacter();
	void		guiSplash();
	int			guiNewGame();
	uint16		guiChooseLanguage();
	uint16		guiSelectGame();
	int			guiGetSelectedBlock(const Common::Point &p);

	void		switchBackground(const char* background, const char* mask);
	void		showSlide(const char *name);
};



#define NUM_ZONES	100

class Parallaction_br : public Parallaction_ns {

	typedef Parallaction_ns Super;

public:
	Parallaction_br(OSystem* syst, const PARALLACTIONGameDescription *gameDesc) : Parallaction_ns(syst, gameDesc) { }
	~Parallaction_br();

	int init();
	int go();

public:
	typedef void (Parallaction_br::*Callable)(void*);
	virtual	void callFunction(uint index, void* parm);

public:
	Table		*_countersNames;

	Table		*_audioCommandsNames;
	const char **_audioCommandsNamesRes;

	int			_part;
	int			_progress;

	int			_zeta0;
	int			_zeta1;
	int			_zeta2;

	int16		_lipSyncVal;
	uint		_subtitleLipSync;

	Label		_subtitle0;
	Label		_subtitle1;

	Zone		*_activeZone2;

	int32		_counters[32];

	uint32		_zoneFlags[NUM_LOCATIONS][NUM_ZONES];

private:
	void		initResources();
	void		initFonts();
	void		freeFonts();
	void		initOpcodes();
	void		initParsers();

	void setArrowCursor();
	void setInventoryCursor(int pos);

	void		changeLocation(char *location);
	void		changeCharacter(const char *name);

	void		initPart();
	void		freePart();
	void		startPart();

	void setMousePointer(int16 index);
	void initCursors();

	Frames	*_dinoCursor;
	Frames	*_dougCursor;
	Frames	*_donnaCursor;
	Frames	*_mouseArrow;


	static const char *_partNames[];

	void guiStart();
	int guiShowMenu();
	void guiSplash(const char *name);
	Frames* guiRenderMenuItem(const char *text);

	static const Callable _dosCallables[6];

	void _c_blufade(void*);
	void _c_resetpalette(void*);
	void _c_ferrcycle(void*);
	void _c_lipsinc(void*);
	void _c_albcycle(void*);
	void _c_password(void*);

	const Callable *_callables;

	DECLARE_UNQUALIFIED_LOCATION_PARSER(location);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(zone);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(animation);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(localflags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(flags);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(comment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(endcomment);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(sound);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(music);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(redundant);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(ifchar);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(character);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(mask);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(path);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(escape);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(zeta);
	DECLARE_UNQUALIFIED_LOCATION_PARSER(null);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(ifchar);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(endif);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(zone);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(location);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(toggle);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(string);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(math);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(test);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(music);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(zeta);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(swap);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(give);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(text);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(unary);

	void parseLocation(const char* name);

	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(zone);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(color);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(mask);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(print);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(text);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(if_op);
	DECLARE_UNQUALIFIED_INSTRUCTION_PARSER(endif);

	virtual void parseRValue(ScriptVar &var, const char *str);

	DECLARE_UNQUALIFIED_COMMAND_OPCODE(location);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(open);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(close);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(on);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(off);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(call);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(drop);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(move);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(start);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(stop);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(character);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(followme);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(onmouse);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(offmouse);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(add);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(leave);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(inc);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(dec);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(ifeq);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(iflt);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(ifgt);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(let);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(music);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(fix);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(unfix);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(zeta);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(scroll);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(swap);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(give);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(text);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(part);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(testsfx);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(ret);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(onsave);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(offsave);
	DECLARE_UNQUALIFIED_ZONE_PARSER(limits);
	DECLARE_UNQUALIFIED_ZONE_PARSER(moveto);
	DECLARE_UNQUALIFIED_ZONE_PARSER(type);
	DECLARE_UNQUALIFIED_ANIM_PARSER(file);
	DECLARE_UNQUALIFIED_ANIM_PARSER(position);
	DECLARE_UNQUALIFIED_ANIM_PARSER(moveto);
	DECLARE_UNQUALIFIED_ANIM_PARSER(endanimation);

	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(on);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(off);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(loop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(inc);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(dec);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(set);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(put);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(wait);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(start);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(process);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(move);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(color);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(mask);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(print);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(text);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(mul);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(div);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(ifeq);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(iflt);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(ifgt);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endif);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(stop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endscript);

	void setupSubtitles(char *s, char *s2, int y);
#if 0
	void jobWaitRemoveLabelJob(void *parm, Job *job);
	void jobDisplaySubtitle(void *parm, Job *job);
	void jobEraseSubtitle(void *parm, Job *job);
	void jobWaitRemoveSubtitleJob(void *parm, Job *job);
	void jobPauseSfx(void *parm, Job *job);
	void jobStopFollower(void *parm, Job *job);
	void jobScroll(void *parm, Job *job);
#endif
};

// FIXME: remove global
extern Parallaction *_vm;


} // namespace Parallaction


#endif
