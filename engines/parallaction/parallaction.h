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
#include "common/func.h"
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


enum EngineFlags {
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

enum ParallactionGameType {
	GType_Nippon = 1,
	GType_BRA
};

struct PARALLACTIONGameDescription;




extern char			_password[8];
extern uint16		_score;
extern uint32		_engineFlags;
extern char			_saveData1[];
extern uint32		_globalFlags;
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
	Common::String	_comment;
	Common::String	_endComment;

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

protected:
	void freeAnimations();
	void freeZones(bool removeAll);

public:
	Location();
	~Location();

	AnimationPtr findAnimation(const char *name);
	ZonePtr findZone(const char *name);

	void cleanup(bool removeAll);
};


class CharacterName {
	const char *_prefix;
	const char *_suffix;
	bool _dummy;
	char _name[30];
	char _baseName[30];
	char _fullName[30];
	static const char _prefixMini[];
	static const char _suffixTras[];
	static const char _empty[];
	void dummify();
public:
	CharacterName();
	CharacterName(const char *name);
	void bind(const char *name);
	const char *getName() const;
	const char *getBaseName() const;
	const char *getFullName() const;
	bool dummy() const;
};


struct Character {
	Parallaction	*_vm;


	AnimationPtr	_ani;
	GfxObj			*_head;
	GfxObj			*_talk;
	GfxObj			*_objs;
	PathBuilder		*_builder;
	PathWalker		*_walker;
	PointList		_walkPath;

	Character(Parallaction *vm);
	~Character();

	void getFoot(Common::Point &foot);
	void setFoot(const Common::Point &foot);
	void scheduleWalk(int16 x, int16 y);

	void free();

protected:
	CharacterName	_name;

	int16		_direction, _step;

public:
	void setName(const char *name);
	const char *getName() const;
	const char *getBaseName() const;
	const char *getFullName() const;
	bool dummy() const;

	void updateDirection(const Common::Point& pos, const Common::Point& to);

};


class SaveLoad;

#define NUM_LOCATIONS 120

class Parallaction : public Engine {
	friend class Debugger;

public:
	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

protected:		// members
	bool detectGame(void);

private:
	const PARALLACTIONGameDescription *_gameDescription;
	uint16	_language;

public:
	Parallaction(OSystem *syst, const PARALLACTIONGameDescription *gameDesc);
	~Parallaction();

	// Engine APIs
	virtual Common::Error init();
	virtual bool hasFeature(EngineFeature f) const;

	// info
	int32			_screenWidth;
	int32			_screenHeight;
	int32			_screenSize;

	// subsystems
	Gfx				*_gfx;
	Disk			*_disk;
	Input			*_input;
	SoundMan		*_soundMan;
	Debugger		*_debugger;
	SaveLoad		*_saveLoad;
	MenuInputHelper *_menuHelper;
	Common::RandomSource _rnd;

	// fonts
	Font		*_labelFont;
	Font		*_menuFont;
	Font		*_introFont;
	Font		*_dialogueFont;

	// game utilities
	Table				*_globalFlagsNames;
	Table				*_objectsNames;
	Table				*_callableNames;
	Table				*_localFlagNames;
	CommandExec			*_cmdExec;
	ProgramExec			*_programExec;
	Inventory 			*_inventory;
	BalloonManager 		*_balloonMan;
	DialogueManager		*_dialogueMan;
	InventoryRenderer 	*_inventoryRenderer;

	// game data
	Character		_char;
	uint32			_localFlags[NUM_LOCATIONS];
	char			_locationNames[NUM_LOCATIONS][32];
	int16			_currentLocationIndex;
	uint16			_numLocations;
	Location		_location;
	ZonePtr			_activeZone;
	char			_characterName1[50];	// only used in changeCharacter
	ZonePtr			_zoneTrap;
	ZonePtr			_commentZone;

protected:
	void	runGame();
	void 	runGuiFrame();
	void 	cleanupGui();
	void 	runDialogueFrame();
	void 	exitDialogueMode();
	void 	runCommentFrame();
	void 	enterCommentMode(ZonePtr z);
	void 	exitCommentMode();
	void	processInput(int event);
	void	updateView();
	void 	drawAnimations();
	void	freeCharacter();
	void	doLocationEnterTransition();
	void	allocateLocationSlot(const char *name);
	void	finalizeLocationParsing();
	void	showLocationComment(const Common::String &text, bool end);
	void 	setupBalloonManager();

public:
	void	beep();
	void	pauseJobs();
	void	resumeJobs();
	void 	hideDialogueStuff();
	uint 	getInternLanguage();
	void 	setInternLanguage(uint id);
	void 	enterDialogueMode(ZonePtr z);
	void	scheduleLocationSwitch(const char *location);
	void	showSlide(const char *name, int x = 0, int y = 0);

public:
	void		setLocationFlags(uint32 flags);
	void		clearLocationFlags(uint32 flags);
	void		toggleLocationFlags(uint32 flags);
	uint32		getLocationFlags();
	bool 		checkSpecialZoneBox(ZonePtr z, uint32 type, uint x, uint y);
	bool 		checkZoneBox(ZonePtr z, uint32 type, uint x, uint y);
	bool 		checkLinkedAnimBox(ZonePtr z, uint32 type, uint x, uint y);
	ZonePtr		hitZone(uint32 type, uint16 x, uint16 y);
	void		runZone(ZonePtr z);
	bool		pickupItem(ZonePtr z);
	void 		updateDoor(ZonePtr z, bool close);
	void 		showZone(ZonePtr z, bool visible);
	void		setBackground(const char *background, const char *mask, const char *path);
	void 		highlightInventoryItem(ItemPosition pos);
	int16 		getHoverInventoryItem(int16 x, int16 y);
	int 		addInventoryItem(ItemName item);
	int 		addInventoryItem(ItemName item, uint32 value);
	void 		dropItem(uint16 v);
	bool 		isItemInInventory(int32 v);
	const 		InventoryItem* getInventoryItem(int16 pos);
	int16 		getInventoryItemIndex(int16 pos);
	void 		initInventory();
	void 		destroyInventory();
	void 		cleanInventory(bool keepVerbs = true);
	void 		openInventory();
	void 		closeInventory();

	virtual void parseLocation(const char* name) = 0;
	virtual void changeLocation(char *location) = 0;
	virtual void changeCharacter(const char *name) = 0;
	virtual	void callFunction(uint index, void* parm) = 0;
	virtual void runPendingZones() = 0;
	virtual void cleanupGame() = 0;
};



class Parallaction_ns : public Parallaction {

public:
	Parallaction_ns(OSystem* syst, const PARALLACTIONGameDescription *gameDesc);
	~Parallaction_ns();

	// Engine APIs
	virtual Common::Error init();
	virtual Common::Error go();

public:
	virtual void 	parseLocation(const char *filename);
	virtual void 	changeLocation(char *location);
	virtual void 	changeCharacter(const char *name);
	virtual void 	callFunction(uint index, void* parm);
	virtual void 	runPendingZones();
	virtual void 	cleanupGame();


	void 	switchBackground(const char* background, const char* mask);

private:
	bool				_inTestResult;
	LocationParser_ns	*_locationParser;
	ProgramParser_ns	*_programParser;

private:
	void 	initFonts();
	void 	freeFonts();
	void 	initResources();
	void	startGui();
	void	startCreditSequence();
	void	startEndPartSequence();
	void	loadProgram(AnimationPtr a, const char *filename);
	void	freeLocation(bool removeAll);


	//  callables data
	typedef void (Parallaction_ns::*Callable)(void*);
	const Callable *_callables;
	ZonePtr _moveSarcZone0;
	ZonePtr _moveSarcZone1;
	uint16 num_foglie;
	int16 _introSarcData1;
	uint16	_introSarcData2;		 // sarcophagus stuff to be saved
	uint16	_introSarcData3;		 // sarcophagus stuff to be saved
	ZonePtr _moveSarcZones[5];
	ZonePtr _moveSarcExaZones[5];
	AnimationPtr _rightHandAnim;
	bool _intro;
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
};



#define NUM_ZONES	100

class Parallaction_br : public Parallaction_ns {

public:
	Parallaction_br(OSystem* syst, const PARALLACTIONGameDescription *gameDesc);
	~Parallaction_br();

	Common::Error init();
	Common::Error go();

public:
	virtual void parseLocation(const char* name);
	virtual void changeLocation(char *location);
	virtual void changeCharacter(const char *name);
	virtual	void callFunction(uint index, void* parm);
	virtual void runPendingZones();
	virtual void cleanupGame();


	void setupSubtitles(char *s, char *s2, int y);
	void clearSubtitles();

public:
	Table		*_countersNames;
	const char **_audioCommandsNamesRes;
	static const char *_partNames[];
	int			_part;
#if 0	// disabled since I couldn't find any references to lip sync in the scripts
	int16		_lipSyncVal;
	uint		_subtitleLipSync;
#endif
	int			_subtitleY;
	int			_subtitle[2];
	ZonePtr		_activeZone2;
	int32		_counters[32];
	uint32		_zoneFlags[NUM_LOCATIONS][NUM_ZONES];

private:
	LocationParser_br		*_locationParser;
	ProgramParser_br		*_programParser;

private:
	void	initResources();
	void	initFonts();
	void	freeFonts();
	void	freeLocation(bool removeAll);
	void 	loadProgram(AnimationPtr a, const char *filename);
	void 	startGui(bool showSplash);

	typedef void (Parallaction_br::*Callable)(void*);
	const Callable *_callables;
	static const Callable _dosCallables[6];

	// dos callables
	void _c_blufade(void*);
	void _c_resetpalette(void*);
	void _c_ferrcycle(void*);
	void _c_lipsinc(void*);
	void _c_albcycle(void*);
	void _c_password(void*);
};

// FIXME: remove global
extern Parallaction *_vm;


} // namespace Parallaction


#endif
