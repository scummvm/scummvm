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

#include "engines/engine.h"

#include "parallaction/defs.h"
#include "parallaction/inventory.h"
#include "parallaction/parser.h"
#include "parallaction/objects.h"
#include "parallaction/disk.h"
#include "parallaction/walk.h"

namespace GUI {
	class ListWidget;
	class CommandSender;
}

extern OSystem *g_system;

namespace Parallaction {

enum {
	kDebugDisk = 1 << 0,
	kDebugWalk = 1 << 1,
	kDebugLocation = 1 << 2,
	kDebugDialogue = 1 << 3,
	kDebugGraphics = 1 << 4,
	kDebugJobs = 1 << 5,
	kDebugInput = 1 << 6,
	kDebugAudio = 1 << 7,
	kDebugMenu = 1 << 8
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
	kMouseRightDown 	= 4
};

enum EngineFlags {
	kEngineQuit 			= (1 << 0),
	kEnginePauseJobs		= (1 << 1),
	kEngineInventory		= (1 << 2),
	kEngineWalking			= (1 << 3),
	kEngineChangeLocation	= (1 << 4),
	kEngineBlockInput		= (1 << 5),
	kEngineDragging 		= (1 << 6),
	kEngineTransformedDonna		= (1 << 7)
};

enum {
	kEvNone 			= 0,
	kEvEnterZone   		= 1,
	kEvExitZone    		= 2,
	kEvAction	  		= 3,
	kEvOpenInventory	= 4,
	kEvCloseInventory	= 5,
	kEvHoverInventory	= 6,
	kEvWalk 	  		= 7,
	kEvQuitGame 		= 1000,
	kEvSaveGame 		= 2000,
	kEvLoadGame 		= 4000
};

enum {
	kCursorArrow = -1
};

enum ParallactionGameType {
	GType_Nippon = 1,
	GType_BRA
};

struct PARALLACTIONGameDescription;



struct Job;
typedef void (*JobFn)(void*, Job*);

struct Job {
	uint16		_count; 		// # of executions left
	uint16		_tag;			// used for ordering
	uint16		_finished;
	void *		_parm;
	JobFn		_fn;

public:
	Job() : _count(0), _tag(0), _finished(0), _parm(NULL), _fn(NULL) {
	}
};

typedef Job* JobPointer;
typedef ManagedList<JobPointer> JobList;

extern uint16 		_mouseButtons;
extern uint16 		_score;
extern uint16 		_language;
extern uint32 		_engineFlags;
extern Command 		*_forwardedCommands[];
extern char 		_forwardedAnimationNames[][20];
extern uint16 		_numForwards;
extern char 		_slideText[][40];
extern uint16 		_introSarcData3;		 // sarcophagus stuff to be saved
extern uint16 		_introSarcData2;		 // sarcophagus stuff to be saved
extern char 		_saveData1[];
extern uint32 		_commandFlags;
extern const char 	*_dinoName;
extern const char 	*_donnaName;
extern const char 	*_doughName;
extern const char 	*_drkiName;
extern const char 	*_minidinoName;
extern const char 	*_minidonnaName;
extern const char 	*_minidoughName;
extern const char 	*_minidrkiName;

// Various ways of detecting character modes used to exist
// inside the engine, so they have been unified in the two
// following macros.
// Mini characters are those used in far away shots, like
// the highway scenery, while Dummy characters are a mere
// workaround to keep the engine happy when showing slides.
// As a sidenote, standard sized characters' names start
// with a lowercase 'd'.
#define IS_MINI_CHARACTER(s) (((s)[0] == 'm'))
#define IS_DUMMY_CHARACTER(s) (((s)[0] == 'D'))

void waitUntilLeftClick();


void jobRemovePickedItem(void*, Job *j);
void jobDisplayDroppedItem(void*, Job *j);
void jobToggleDoor(void*, Job *j);
void jobEraseAnimations(void *arg_0, Job *j);
void jobWalk(void*, Job *j);
void jobRunScripts(void*, Job *j);
void jobDisplayAnimations(void*, Job *j);
void jobDisplayLabel(void *parm, Job *j);
void jobWaitRemoveJob(void *parm, Job *j);
void jobShowInventory(void *parm, Job *j);
void jobHideInventory(void *parm, Job *j);
void jobEraseLabel(void *parm, Job *j);



class Debugger;
class Gfx;
class Menu;
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
	Animation		_ani;
	Graphics::Surface		*_head;
	Frames		    *_talk;
	Frames 			*_objs;
	PathBuilder		_builder;

	Character() : _builder(&_ani) {
		_talk = NULL;
		_head = NULL;
		_objs = NULL;

		_ani._left = 150;
		_ani._top = 100;
		_ani._z = 10;
		_ani._oldPos.x = -1000;
		_ani._oldPos.y = -1000;
		_ani._frame = 0;
		_ani._flags = kFlagsActive | kFlagsNoName;
		_ani._type = kZoneYou;
		_ani._label._cnv.pixels = NULL;
		_ani._label._text = strdup("yourself");
	}

};


class Table {

	char	**_data;
	uint16	_size;
	uint16	_used;
	bool	_disposeMemory;

public:
	Table(uint32 size);
	Table(uint32 size, const char** data);

	~Table();

	enum {
		notFound = 0
	};

	void addData(const char* s);

	uint16 lookup(const char* s);
};

struct BackgroundInfo {
	uint width;
	uint height;

	Graphics::Surface	bg;
	MaskBuffer			mask;
	PathBuffer			path;

	Palette				palette;
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

	Parallaction(OSystem *syst);
	~Parallaction();

	int init();

	void loadGame();
	void saveGame();

	uint16 		updateInput();

	void 		waitTime(uint32 t);

	uint	_lookup;
	Common::Stack<OpcodeSet*>	_opcodes;
	Common::Stack<Table*> 		_statements;
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


	void 		changeCursor(int32 index);
	void		showCursor(bool visible);


	Job 		*addJob(JobFn fn, void *parm, uint16 tag);
	void 		removeJob(Job *j);
	void 		pauseJobs();
	void 		resumeJobs();
	void 		runJobs();

	void 		finalizeWalk(WalkNodeList *list);
	int16 		selectWalkFrame(const Common::Point& pos, const WalkNode* from);
	void 		clipMove(Common::Point& pos, const WalkNode* from);

	Zone 		*findZone(const char *name);
	Zone   		*hitZone(uint32 type, uint16 x, uint16 y);
	uint16		runZone(Zone*);
	void 		freeZones();

	void 		runDialogue(SpeakData*);

	void 		runCommands(CommandList& list, Zone *z = NULL);

	Animation  	*findAnimation(const char *name);
	void		sortAnimations();
	void 		freeAnimations();

	void 		showSlide(const char *name);
	void 		setBackground(const char *background, const char *mask, const char *path);
	void 		freeBackground();

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
	int32 			_screenHeight;
	int32 			_screenSize;

	PathBuffer		*_pathBuffer;

	SoundMan		*_soundMan;

	Gfx*			_gfx;
	Disk*			_disk;

	Character		_char;
	char			_characterName[30];

	uint32			_localFlags[NUM_LOCATIONS];
	char			_locationNames[NUM_LOCATIONS][32];
	int16			_currentLocationIndex;
	uint16			_numLocations;
	Location		_location;

	InventoryItem	_activeItem;

	Common::Point	_mousePos;

	Zone	   		*_activeZone;

	ZoneList 		_zones;
	AnimationList 	_animations;

	Font		*_labelFont;
	Font		*_menuFont;
	Font		*_dialogueFont;


protected:		// data

	Debugger	*_debugger;

	struct InputData {
		uint16			_event;
		Common::Point	_mousePos;
		int16       	_inventoryIndex;
		Zone*       	_zone;
		Label*  		_label;
	};

	bool 		_mouseHidden;

	// input-only
	InputData	 _input;
	bool		_actionAfterWalk;  // actived when the character needs to move before taking an action

	// these two could/should be merged as they carry on the same duty in two member functions,
	// respectively processInput and translateInput
	int16		_procCurrentHoverItem;
	int16		_transCurrentHoverItem;

	uint32		_baseTime;
	char		_characterName1[50]; 	// only used in changeCharacter

	int16 _keyDown;

	JobList		_jobs;

	Common::String      _saveFileName;

	bool		_hasLocationSound;
	char		_locationSound[50];

	BackgroundInfo	*_backgroundInfo;

	Job	   *_jDrawLabel;
	Job	   *_jEraseLabel;
	Zone    *_hoverZone;

protected:		// members
	bool detectGame(void);

	void		initGlobals();
	void		runGame();
	uint32		getElapsedTime();
	void		resetTimer();

	InputData 	*translateInput();
	void		processInput(InputData*);

	int         buildSaveFileList(Common::StringList& l);
	int         selectSaveFile(uint16 arg_0, const char* caption, const char* button);
	void		doLoadGame(uint16 slot);
	void		doSaveGame(uint16 slot, const char* name);

	void		doLocationEnterTransition();
	virtual void changeLocation(char *location) = 0;
	virtual void changeCharacter(const char *name) = 0;
	void		allocateLocationSlot(const char *name);
	void 		finalizeLocationParsing();
	void 		switchBackground(const char* background, const char* mask);
	void 		freeLocation();
	void 		showLocationComment(const char *text, bool end);

	void 		displayCharacterComment(ExamineData *data);
	void 		displayItemComment(ExamineData *data);

	uint16 		checkDoor();

	void 		freeCharacter();

	int 		addInventoryItem(uint16 item);
	void 		dropItem(uint16 item);
	int16 		pickupItem(Zone *z);
	int16 		isItemInInventory(int32 v);
	int16		getHoverInventoryItem(int16 x, int16 y);

public:
	virtual	void callFunction(uint index, void* parm) { }
	virtual void renderLabel(Graphics::Surface *cnv, char *text) { }
	virtual void setMousePointer(int16 index) = 0;


	virtual void parseLocation(const char* name) = 0;

public:
	const char **_zoneFlagNamesRes;
	const char **_zoneTypeNamesRes;
	const char **_commandsNamesRes;
	const char **_callableNamesRes;
	const char **_instructionNamesRes;

};


class Parallaction_ns : public Parallaction {

public:
	Parallaction_ns(OSystem* syst) : Parallaction(syst) { }
	~Parallaction_ns();

	int init();
	int go();

public:
	typedef void (Parallaction_ns::*Callable)(void*);

	virtual	void callFunction(uint index, void* parm);
	void renderLabel(Graphics::Surface *cnv, char *text);
	void setMousePointer(int16 index);

private:
	Menu*			_menu;

	void initFonts();
	void freeFonts();

private:
	void changeLocation(char *location);
	void changeCharacter(const char *name);

	void initResources();
	void initCursors();

	static byte			_mouseArrow[256];
	Graphics::Surface			*_mouseComposedArrow;

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
		Zone 		*z;
		Animation 	*a;
		int			nextToken;
		CommandList *list;
		bool		endcommands;
		Command		*cmd;
	} _locParseCtxt;

	DECLARE_UNQUALIFIED_LOCATION_PARSER(invalid);
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
	DECLARE_UNQUALIFIED_LOCATION_PARSER(redundant);
	DECLARE_UNQUALIFIED_ZONE_PARSER(invalid);
	DECLARE_UNQUALIFIED_ZONE_PARSER(limits);
	DECLARE_UNQUALIFIED_ZONE_PARSER(moveto);
	DECLARE_UNQUALIFIED_ZONE_PARSER(type);
	DECLARE_UNQUALIFIED_ZONE_PARSER(commands);
	DECLARE_UNQUALIFIED_ZONE_PARSER(label);
	DECLARE_UNQUALIFIED_ZONE_PARSER(flags);
	DECLARE_UNQUALIFIED_ZONE_PARSER(endzone);
	DECLARE_UNQUALIFIED_ANIM_PARSER(invalid);
	DECLARE_UNQUALIFIED_ANIM_PARSER(script);
	DECLARE_UNQUALIFIED_ANIM_PARSER(commands);
	DECLARE_UNQUALIFIED_ANIM_PARSER(type);
	DECLARE_UNQUALIFIED_ANIM_PARSER(label);
	DECLARE_UNQUALIFIED_ANIM_PARSER(flags);
	DECLARE_UNQUALIFIED_ANIM_PARSER(file);
	DECLARE_UNQUALIFIED_ANIM_PARSER(position);
	DECLARE_UNQUALIFIED_ANIM_PARSER(moveto);
	DECLARE_UNQUALIFIED_ANIM_PARSER(endanimation);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(invalid);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(flags);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(animation);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(zone);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(location);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(drop);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(call);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(simple);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(move);
	DECLARE_UNQUALIFIED_COMMAND_PARSER(endcommands);

	void 		parseLocation(const char *filename);
	char   		*parseComment(Script &script);
	char   		*parseDialogueString(Script &script);
	Dialogue	*parseDialogue(Script &script);
	void		parseZone(Script &script, ZoneList &list, char *name);
	void		parseZoneTypeBlock(Script &script, Zone *z);
	void 		parseWalkNodes(Script& script, WalkNodeList &list);
	Animation	*parseAnimation(Script &script, AnimationList &list, char *name);
	void		parseCommands(Script &script, CommandList&);
	void		parseCommandFlags();
	void		createCommand(uint id);
	void		addCommand();
	void 		initOpcodes();
	void 		initParsers();


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
	void 		wrapLocalVar(LocalVariable *local);

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
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(end);

};



#define NUM_ZONES	100

class Parallaction_br : public Parallaction_ns {

	typedef Parallaction_ns Super;

public:
	Parallaction_br(OSystem* syst) : Parallaction_ns(syst) { }
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

	Zone		*_activeZone2;

	int32		_counters[32];

	uint32		_zoneFlags[NUM_LOCATIONS][NUM_ZONES];

	struct LocationParserContext_br : public LocationParserContext {
		int numZones;
	} _locParseCtxt;

private:
	void 		initResources();
	void 		initFonts();
	void 		freeFonts();
	void 		initOpcodes();
	void 		initParsers();

	void 		changeLocation(char *location);
	void		changeCharacter(const char *name);

	void		initPart();
	void		freePart();
	void		startPart();

	void setMousePointer(int16 index);
	void initCursors();

	Graphics::Surface 	*_dinoCursor;
	Graphics::Surface 	*_dougCursor;
	Graphics::Surface 	*_donnaCursor;
	Graphics::Surface 	*_mouseArrow;


	int showMenu();
	void renderMenuItem(Graphics::Surface &surf, const char *text);
	void invertMenuItem(Graphics::Surface &surf);

	void splash(const char *name);

	static const Callable _dosCallables[6];

	void _c_blufade(void*);
	void _c_resetpalette(void*);
	void _c_ferrcycle(void*);
	void _c_lipsinc(void*);
	void _c_albcycle(void*);
	void _c_password(void*);

	const Callable *_callables;
/*
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
*/
};

// FIXME: remove global
extern Parallaction *_vm;


} // namespace Parallaction


#endif
