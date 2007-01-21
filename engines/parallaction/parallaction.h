/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "engines/engine.h"
#include "parallaction/defs.h"
#include "parallaction/inventory.h"


namespace Parallaction {

// high values mean high priority

#define JOBPRIORITY_DRAWLABEL			0
#define JOBPRIORITY_DRAWMOUSE			1
#define JOBPRIORITY_SHOWINVENTORY		2
#define JOBPRIORITY_DRAWANIMATIONS		3
#define JOBPRIORITY_RUNSTUFF			15
#define JOBPRIORITY_ADDREMOVEITEMS		17
#define JOBPRIORITY_TOGGLEDOOR			18
#define JOBPRIORITY_WALK				19
#define JOBPRIORITY_ERASEANIMATIONS		20
#define JOBPRIORITY_HIDEINVENTORY		20
#define JOBPRIORITY_ERASEMOUSE			21

enum {
	kMouseNone			= 0,
	kMouseLeftUp		= 1,
	kMouseLeftDown		= 2,
	kMouseRightUp		= 3,
	kMouseRightDown 	= 4
};


enum ParallactionGameType {
	GType_Nippon = 1,
	GType_BRA
};

struct PARALLACTIONGameDescription;

struct Job;
typedef void (*JobFn)(void*, Job*);

struct Job {
	Node		_node;
	uint16		_count; 		// # of executions left
	uint16		_tag;			// used for ordering
	uint16		_finished;
	void *		_parm;
	JobFn		_fn;
};

extern Point _mousePos;
extern uint16 _mouseButtons;

extern uint16 _score;
extern uint16 _language;
extern Zone *_activeZone;
extern uint32 _engineFlags;
extern callable _callables[];
extern Animation _yourself;
extern Node _zones;
extern Node _animations;
extern uint32 _localFlags[];
extern Command *_forwardedCommands[];
extern char _forwardedAnimationNames[][20];
extern uint16 _numForwards;
extern char _soundFile[];
extern char _slideText[][40];
extern uint16 _introSarcData3;		 // sarcophagus stuff to be saved
extern uint16 _introSarcData2;		 // sarcophagus stuff to be saved
extern char _newLocation[];
extern char *_globalTable[];
extern char _saveData1[];
extern Point _firstPosition;		 // starting position after load game??
extern uint16 _firstFrame;	 // starting frame after load game??
extern Cnv _characterFace;
extern byte _mouseHidden;
extern uint32 _commandFlags;
extern Cnv _tempFrames;

extern char _location[];
extern Node _locationWalkNodes;
extern Command *_locationACommands;
extern Command *_locationCommands;
extern char *_locationComment;
extern char *_locationEndComment;

extern char *_objectsNames[];
extern const char *_zoneTypeNames[];
extern const char *_zoneFlagNames[];
extern char *_localFlagNames[];
extern const char *commands_names[];

extern const char *_instructionNames[];
extern const char *_callableNames[];

void waitUntilLeftClick();

void addNode(Node *list, Node *n);
void removeNode(Node *n);
void freeNodeList(Node *list);

Command *parseCommands(ArchivedFile *file);
void runCommands(Command *list, Zone *z = NULL);
void freeCommands(Command*);

void freeZones(Node *list);

void freeLocation();

void runDialogue(SpeakData*);
Dialogue *parseDialogue(ArchivedFile *file);

void changeCharacter(const char *name);

WalkNode *buildWalkPath(uint16 x, uint16 y);

Job *addJob(JobFn fn, void *parm, uint16 tag);
void removeJob(Job *j);
void runJobs();
void pauseJobs();
void resumeJobs();

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


enum EngineFlags {
	kEngineQuit 			= (1 << 0),
	kEnginePauseJobs		= (1 << 1),
	kEngineInventory		= (1 << 2),
	kEngineWalking			= (1 << 3),
	kEngineChangeLocation	= (1 << 4),
	kEngineMouse			= (1 << 5),
	kEngineDragging 		= (1 << 6),
	kEngineMiniDonna		= (1 << 7)
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



class Graphics;
class Menu;

class Parallaction : public Engine {

public:

	Parallaction(OSystem *syst);
	~Parallaction();

	int init();

	int go();


	void loadGame();
	void saveGame();

	uint16 updateInput();

	void waitTime(uint32 t);

	static void initTable(const char *path, char **table);
	static void freeTable(char** table);
	static int16 searchTable(const char *s, const char **table);

	void parseLocation(const char *filename);
	void changeCursor(int32 index);
	void changeCharacter(const char *name);

public:
	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

private:
	const PARALLACTIONGameDescription *_gameDescription;

public:

	Graphics*	_graphics;
	Menu*	_menu;
	char	_characterName[30];
	char	_languageDir[6];
	char	_disk[6];

	char	_locationNames[120][32];
	int16	_currentLocationIndex;
	uint16	_numLocations;

	InventoryItem	_activeItem;

protected:		// data

	struct InputData {
		uint16		_event;
		Point		_mousePos;

		int16       _inventoryIndex;
		Zone*       _zone;
        StaticCnv*  _cnv;
	};

	bool		_skipMenu;

	// input-only
	InputData	 _input;
	bool		_actionAfterWalk;  // actived when the character needs to move before taking an action

	// these two could/should be merged as they carry on the same duty in two member functions,
	// respectively processInput and translateInput
	int16		_procCurrentHoverItem;
	int16		_transCurrentHoverItem;

	uint32		_baseTime;

	uint16		_musicData1;	  // only used in changeLocation
	char		_characterName1[50]; 	// only used in changeCharacter

	int16 _keyDown;

protected:		// members
	bool detectGame(void);

	void		initGame();
	void		initGlobals();

	void		doLoadGame(uint16 _di);
	void		doSaveGame(uint16 _di);

	void		runGame();
	InputData * translateInput();
	void		processInput(InputData*);

	int16		getHoverInventoryItem(int16 x, int16 y);

	uint32		getElapsedTime();
	void		resetTimer();

	void		doLocationEnterTransition();
	void		parseZone(ArchivedFile *file, Node *list, char *name);
	Animation * parseAnimation(ArchivedFile *file, Node *list, char *name);
	void		parseScriptLine(Instruction *inst, Animation *a, LocalVariable *locals);
	void		parseZoneTypeBlock(ArchivedFile *file, Zone *z);
	void		loadProgram(Animation *a, char *filename);
	void		changeLocation(char *location);

};

// FIXME: remove global
extern Parallaction *_vm;


} // namespace Parallaction


#endif
