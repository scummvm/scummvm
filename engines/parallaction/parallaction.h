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


#include "engines/engine.h"

#include "parallaction/defs.h"
#include "parallaction/inventory.h"
#include "parallaction/parser.h"
#include "parallaction/disk.h"
#include "parallaction/walk.h"
#include "parallaction/zone.h"

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

typedef void (*callable)(void*);

extern uint16 		_mouseButtons;
extern uint16 		_score;
extern uint16 		_language;
extern Zone 		*_activeZone;
extern uint32 		_engineFlags;
extern callable 	_callables[];
extern uint32 		_localFlags[];
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
	WalkNodeList	_walkNodes;
	char			_name[100];

	CommandList		_aCommands;
	CommandList		_commands;
	char	   *_comment;
	char	   *_endComment;

};

struct Character {
	Animation		_ani;
	Graphics::Surface		*_head;
	Cnv		    	*_talk;
	Cnv 			*_objs;
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

	void addData(const char* s);

	int16 lookup(const char* s);
};

struct BackgroundInfo {
	uint width;
	uint height;

	Graphics::Surface	bg;
	MaskBuffer			mask;
	PathBuffer			path;

	Palette				palette;
};


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

	void 		parseLocation(const char *filename);
	void 		changeCursor(int32 index);
	void		showCursor(bool visible);
	void 		changeCharacter(const char *name);

	char   		*parseComment(Script &script);
	char   		*parseDialogueString(Script &script);
	Dialogue	*parseDialogue(Script &script);

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
	Table		*_commandsNames;
	Table		*_callableNames;
	Table		*_instructionNames;
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

	char			_locationNames[120][32];
	int16			_currentLocationIndex;
	uint16			_numLocations;
	Location		_location;

	InventoryItem	_activeItem;

	Common::Point	_mousePos;

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
	void		changeLocation(char *location);
	void 		resolveLocationForwards();
	void 		switchBackground(const char* background, const char* mask);
	void 		freeLocation();
	void 		showLocationComment(const char *text, bool end);

	void		parseZone(Script &script, ZoneList &list, char *name);
	void		parseZoneTypeBlock(Script &script, Zone *z);
	void 		displayCharacterComment(ExamineData *data);
	void 		displayItemComment(ExamineData *data);

	void 		parseWalkNodes(Script& script, WalkNodeList &list);
	uint16 		checkDoor();

	Animation * parseAnimation(Script &script, AnimationList &list, char *name);
	void		parseScriptLine(Instruction *inst, Animation *a, LocalVariable *locals);
	void		loadProgram(Animation *a, char *filename);
	LValue		getLValue(Instruction *inst, char *str, LocalVariable *locals, Animation *a);

	void		parseCommands(Script &script, CommandList&);

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


public:
	Menu*			_menu;

private:
	void initFonts();
	void freeFonts();

private:
	void 		initResources();
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
};

class Parallaction_br : public Parallaction {

public:
	Parallaction_br(OSystem* syst) : Parallaction(syst) { }
	~Parallaction_br();

	int init();
	int go();

public:
	typedef void (Parallaction_br::*Callable)(void*);
	virtual	void callFunction(uint index, void* parm);

public:
	Table		*_audioCommandsNames;
	const char **_audioCommandsNamesRes;

	int			_part;
	int			_progress;

private:
	void 		initResources();
	void 		initFonts();
	void 		freeFonts();

	void		initPart();
	void		freePart();

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
};

// FIXME: remove global
extern Parallaction *_vm;


} // namespace Parallaction


#endif
