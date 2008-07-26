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

#include "parallaction/exec.h"
#include "parallaction/input.h"
#include "parallaction/inventory.h"
#include "parallaction/parser.h"
#include "parallaction/objects.h"
#include "parallaction/disk.h"
#include "parallaction/walk.h"

#define PATH_LEN	200


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

enum EngineFlags {
	kEngineQuit			= (1 << 0),
	kEnginePauseJobs	= (1 << 1),
	kEngineWalking		= (1 << 3),
	kEngineChangeLocation	= (1 << 4),
	kEngineDragging		= (1 << 6),
	kEngineTransformedDonna	= (1 << 7),

	// BRA specific
	kEngineReturn		= (1 << 10)
};

enum {
	kEvNone			= 0,
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






class Debugger;
class Gfx;
class SoundMan;
class Input;
class DialogueManager;
class MenuInputHelper;

struct Location {

	Common::Point	_startPosition;
	uint16			_startFrame;
	char			_name[100];

	CommandList		_aCommands;
	CommandList		_commands;
	char	   *_comment;
	char	   *_endComment;

	ZoneList		_zones;
	AnimationList	_animations;
	ProgramList		_programs;

	bool		_hasSound;
	char		_soundFile[50];

	// NS specific
	PointList	_walkPoints;
	char _slideText[2][MAX_TOKEN_LEN];

	// BRA specific
	int			_zeta0;
	int			_zeta1;
	int			_zeta2;
	CommandList		_escapeCommands;
};




struct Character {
	Parallaction	*_vm;


	AnimationPtr	_ani;
	GfxObj			*_head;
	GfxObj			*_talk;
	GfxObj			*_objs;
	PathBuilder		*_builder;
	PointList		_walkPath;

	Character(Parallaction *vm);
	~Character();

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

	int16		_direction, _step;

public:
	void setName(const char *name);
	const char *getName() const;
	const char *getBaseName() const;
	const char *getFullName() const;
	bool dummy() const;

	void updateDirection(const Common::Point& pos, const Common::Point& to);

};



#define NUM_LOCATIONS 120

class Parallaction : public Engine {
	friend class Debugger;

public:

	Parallaction(OSystem *syst, const PARALLACTIONGameDescription *gameDesc);
	~Parallaction();

	int init();

	virtual bool loadGame() = 0;
	virtual bool saveGame() = 0;

	Input	*_input;

	void		processInput(InputData* data);

	void		pauseJobs();
	void		resumeJobs();

	void		finalizeWalk(Character &character);
	void		clipMove(Common::Point& pos, const Common::Point& to);

	ZonePtr		findZone(const char *name);
	ZonePtr		hitZone(uint32 type, uint16 x, uint16 y);
	uint16		runZone(ZonePtr z);
	void		freeZones();

	AnimationPtr findAnimation(const char *name);
	void		freeAnimations();

	void		setBackground(const char *background, const char *mask, const char *path);
	void		freeBackground();

	Table		*_globalTable;
	Table		*_objectsNames;
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

	CommandExec*	_cmdExec;
	ProgramExec*	_programExec;
	Character		_char;

	void			setLocationFlags(uint32 flags);
	void			clearLocationFlags(uint32 flags);
	void			toggleLocationFlags(uint32 flags);
	uint32			getLocationFlags();

	uint32			_localFlags[NUM_LOCATIONS];
	char			_locationNames[NUM_LOCATIONS][32];
	int16			_currentLocationIndex;
	uint16			_numLocations;
	Location		_location;

	ZonePtr			_activeZone;


	Font		*_labelFont;
	Font		*_menuFont;
	Font		*_introFont;
	Font		*_dialogueFont;

	Common::RandomSource _rnd;

	Debugger	*_debugger;


protected:		// data
	uint32		_baseTime;
	char		_characterName1[50];	// only used in changeCharacter

	Common::String	_saveFileName;


protected:		// members
	bool detectGame(void);

	void		initGlobals();
	void		runGame();
	void		updateView();

	void		doLocationEnterTransition();
	virtual void changeLocation(char *location) = 0;
	virtual void runPendingZones() = 0;
	void		allocateLocationSlot(const char *name);
	void		finalizeLocationParsing();
	void		freeLocation();
	void		showLocationComment(const char *text, bool end);

	void		displayComment(ExamineData *data);

	void		checkDoor(const Common::Point &foot);

	void		freeCharacter();

	int16		pickupItem(ZonePtr z);

	void 		clearSet(OpcodeSet &opcodes);


public:
	void		scheduleLocationSwitch(const char *location);
	virtual void changeCharacter(const char *name) = 0;

	virtual	void callFunction(uint index, void* parm) { }

	virtual void setArrowCursor() = 0;
	virtual void setInventoryCursor(int pos) = 0;

	virtual void parseLocation(const char* name) = 0;

	void updateDoor(ZonePtr z);

	virtual void walk(Character &character) = 0;
	virtual void drawAnimations() = 0;

	void		beep();

	ZonePtr		_zoneTrap;
	PathBuilder* getPathBuilder(Character *ch);

public:
//	const char **_zoneFlagNamesRes;
//	const char **_zoneTypeNamesRes;
//	const char **_commandsNamesRes;
	const char **_callableNamesRes;
	const char **_instructionNamesRes;

	void highlightInventoryItem(ItemPosition pos);
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

	BalloonManager *_balloonMan;

	void setupBalloonManager();

	void hideDialogueStuff();
	DialogueManager	*_dialogueMan;
	void enterDialogueMode(ZonePtr z);
	void exitDialogueMode();
	void runDialogueFrame();

	MenuInputHelper *_menuHelper;
	void runGuiFrame();
	void cleanupGui();

	ZonePtr	_commentZone;
	void enterCommentMode(ZonePtr z);
	void exitCommentMode();
	void runCommentFrame();

	void setInternLanguage(uint id);
	uint getInternLanguage();
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

	void		switchBackground(const char* background, const char* mask);
	void		showSlide(const char *name);
	void 		setArrowCursor();

	// TODO: this should be private!!!!!!!
	bool	_inTestResult;
	void cleanupGame();
	bool allPartsComplete();

private:
	LocationParser_ns		*_locationParser;
	ProgramParser_ns		*_programParser;

	void initFonts();
	void freeFonts();
	void renameOldSavefiles();
	Common::String genSaveFileName(uint slot, bool oldStyle = false);
	Common::InSaveFile *getInSaveFile(uint slot);
	Common::OutSaveFile *getOutSaveFile(uint slot);
	void setPartComplete(const Character& character);

private:
	void changeLocation(char *location);
	void changeCharacter(const char *name);
	void runPendingZones();

	void setInventoryCursor(int pos);


	void doLoadGame(uint16 slot);
	void doSaveGame(uint16 slot, const char* name);
	int  buildSaveFileList(Common::StringList& l);
	int  selectSaveFile(uint16 arg_0, const char* caption, const char* button);

	void initResources();
	void initCursors();

	static byte _resMouseArrow[256];
	byte	*_mouseArrow;
	Frames	*_mouseComposedArrow;

	static const Callable _dosCallables[25];
	static const Callable _amigaCallables[25];

	/*
		game callables data members
	*/

	ZonePtr _moveSarcZone0;
	ZonePtr _moveSarcZone1;
	uint16 num_foglie;
	int16 _introSarcData1;
	uint16	_introSarcData2;		 // sarcophagus stuff to be saved
	uint16	_introSarcData3;		 // sarcophagus stuff to be saved

	ZonePtr _moveSarcZones[5];
	ZonePtr _moveSarcExaZones[5];
	AnimationPtr _rightHandAnim;

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
	void walk(Character &character);
	void drawAnimations();

	void		parseLocation(const char *filename);
	void		loadProgram(AnimationPtr a, const char *filename);

	void		selectStartLocation();

	void		startGui();
	void		startCreditSequence();
	void		startEndPartSequence();
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
	void		changeCharacter(const char *name);
	void setupSubtitles(char *s, char *s2, int y);
	void clearSubtitles();


public:
	Table		*_countersNames;

	const char **_audioCommandsNamesRes;

	int			_part;
	int			_progress;

	int16		_lipSyncVal;
	uint		_subtitleLipSync;
	int			_subtitleY;
	int			_subtitle[2];

	ZonePtr		_activeZone2;

	int32		_counters[32];

	uint32		_zoneFlags[NUM_LOCATIONS][NUM_ZONES];
	void		startPart(uint part);
	void 		setArrowCursor();
private:
	LocationParser_br		*_locationParser;
	ProgramParser_br		*_programParser;

	void		initResources();
	void		initFonts();
	void		freeFonts();

	void setInventoryCursor(int pos);

	void		changeLocation(char *location);
	void 		runPendingZones();

	void		initPart();
	void		freePart();

	void setMousePointer(int16 index);
	void initCursors();

	Frames	*_dinoCursor;
	Frames	*_dougCursor;
	Frames	*_donnaCursor;
	Frames	*_mouseArrow;


	static const char *_partNames[];

	void startGui();

	static const Callable _dosCallables[6];

	void _c_blufade(void*);
	void _c_resetpalette(void*);
	void _c_ferrcycle(void*);
	void _c_lipsinc(void*);
	void _c_albcycle(void*);
	void _c_password(void*);

	const Callable *_callables;

	void parseLocation(const char* name);
	void loadProgram(AnimationPtr a, const char *filename);

#if 0
	void jobWaitRemoveLabelJob(void *parm, Job *job);
	void jobPauseSfx(void *parm, Job *job);
	void jobStopFollower(void *parm, Job *job);
	void jobScroll(void *parm, Job *job);
#endif
};

// FIXME: remove global
extern Parallaction *_vm;


} // namespace Parallaction


#endif
