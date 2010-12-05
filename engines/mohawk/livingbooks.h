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

#ifndef MOHAWK_LIVINGBOOKS_H
#define MOHAWK_LIVINGBOOKS_H

#include "mohawk/mohawk.h"
#include "mohawk/console.h"
#include "mohawk/graphics.h"

#include "common/config-file.h"
#include "common/substream.h"
#include "common/rect.h"
#include "common/queue.h"
#include "common/random.h"

#include "sound/mixer.h"

namespace Mohawk {

enum NodeState {
	kLBNodeDone = 0,
	kLBNodeRunning = 1,
	kLBNodeWaiting = 2
};

enum LBMode {
	kLBIntroMode = 1,
	kLBControlMode = 2,
	kLBCreditsMode = 3,
	kLBPreviewMode = 4,
	kLBReadMode = 5,
	kLBPlayMode = 6
};

enum {
	kLBStaticTextItem = 0x1,
	kLBPictureItem = 0x2,
	kLBEditTextItem = 0x14,
	kLBLiveTextItem = 0x15,
	kLBAnimationItem = 0x40,
	kLBSoundItem = 0x41,
	kLBGroupItem = 0x42,
	kLBPaletteItem = 0x45 // v3
};

enum {
	// no 0x1?
	kLBAnimOpNotify = 0x2,
	kLBAnimOpSetTempo = 0x3,
	// no 0x4?
	kLBAnimOpMoveTo = 0x5,
	kLBAnimOpWait = 0x6,
	kLBAnimOpSetCel = 0x7,
	kLBAnimOpSleepUntil = 0x8,
	kLBAnimOpDrawMode = 0x9,
	kLBAnimOpPlaySound = 0xa,
	kLBAnimOpWaitForSound = 0xb,
	kLBAnimOpReleaseSound = 0xc,
	kLBAnimOpResetSound = 0xd,
	kLBAnimOpUnknownE = 0xe,
	kLBAnimOpDelay = 0xf
};

enum {
	kLBActionPhase0 = 0,
	kLBActionPhase1 = 1,
	kLBActionMouseDown = 2,
	kLBActionStarted = 3,
	kLBActionDone = 4,
	kLBActionMouseUp = 5,
	kLBActionPhase2 = 6,
	kLBActionNotified = 7,
	kLBActionPrePhase = 0xff
};

enum {
	kLBGroupData = 0x64,
	kLBLiveTextData = 0x65,
	kLBMsgListScript = 0x66,
	kLBNotifyScript = 0x67,
	kLBSetPlayInfo = 0x68,
	kLBSetRandomLoc = 0x69,  // unused?
	kLBSetDrag = 0x6a,       // unused?
	kLBSetDrawMode = 0x6b,
	kLBSetFont = 0x6c,       // unused?
	kLBSetOneShot = 0x6d,    // unused?
	kLBSetPlayPhase = 0x6e,
	// from here, 2.x+
	kLBUnknown6F = 0x6f,
	kLBCommand = 0x70,
	kLBPaletteAData = 0x71,  // unused?
	kLBPaletteXData = 0x72,
	kLBDisable = 0x73,       // unused?
	kLBEnable = 0x74,        // unused?
	kLBSetNotVisible = 0x75,
	kLBSetVisible = 0x76,    // unused?
	kLBGlobalDisable = 0x77, // unused?
	kLBGlobalEnable = 0x78,  // unused?
	kLBGlobalSetNotVisible = 0x79,
	kLBGlobalSetVisible = 0x7a, // unused?
	kLBSetAmbient = 0x7b,
	kLBUnknown7C = 0x7c,     // unused?
	kLBUnknown7D = 0x7d,
	kLBUnknown7E = 0x7e,     // unused?
	kLBSetParent = 0x7f,     // unused?
	kLBUnknown80 = 0x80      // unused?
};

enum {
	kLBNotifyGUIAction = 1,
	kLBNotifyGoToControls = 2,
	kLBNotifyChangePage = 3,
	kLBNotifyIntroDone = 5,
	kLBNotifyChangeMode = 6,
	kLBNotifyCursorChange = 7
};

class MohawkEngine_LivingBooks;
class LBGraphics;
class LBAnimation;

struct LBScriptEntry {
	LBScriptEntry();
	~LBScriptEntry();

	uint16 type;
	uint16 action;
	uint16 opcode;
	uint16 param;

	uint16 argc;
	uint16 *argvParam;
	uint16 *argvTarget;

	// kLBNotifyChangeMode
	uint16 newUnknown;
	uint16 newMode;
	uint16 newPage;
	uint16 newSubpage;

	// kLBActionNotified
	uint16 matchFrom;
	uint16 matchNotify;

	Common::String command;
	Common::Array<Common::String> conditions;
};

struct LBAnimScriptEntry {
	byte opcode;
	byte size;
	byte *data;
};

class LBAnimationNode {
public:
	LBAnimationNode(MohawkEngine_LivingBooks *vm, LBAnimation *parent, uint16 scriptResourceId);
	~LBAnimationNode();

	void draw(const Common::Rect &_bounds);
	void reset();
	NodeState update(bool seeking = false);
	bool transparentAt(int x, int y);

protected:
	MohawkEngine_LivingBooks *_vm;
	LBAnimation *_parent;

	void loadScript(uint16 resourceId);
	uint _currentEntry;
	Common::Array<LBAnimScriptEntry> _scriptEntries;

	uint _currentCel;
	int16 _xPos, _yPos;
	uint32 _delay;
};

class LBAnimationItem;

class LBAnimation {
public:
	LBAnimation(MohawkEngine_LivingBooks *vm, LBAnimationItem *parent, uint16 resourceId);
	~LBAnimation();

	void draw();
	bool update();

	void start();
	void seek(uint16 pos);
	void stop();

	bool transparentAt(int x, int y);

	void setTempo(uint16 tempo);

	uint getNumResources() { return _shapeResources.size(); }
	uint16 getResource(uint num) { return _shapeResources[num]; }
	Common::Point getOffset(uint num) { return _shapeOffsets[num]; }

	uint32 getCurrentFrame() { return _currentFrame; }

	uint16 getParentId();

protected:
	MohawkEngine_LivingBooks *_vm;
	LBAnimationItem *_parent;

	Common::Rect _bounds, _clip;
	Common::Array<LBAnimationNode *> _nodes;

	uint16 _tempo;
	uint32 _lastTime, _currentFrame;
	bool _running;

	void loadShape(uint16 resourceId);
	Common::Array<uint16> _shapeResources;
	Common::Array<Common::Point> _shapeOffsets;
};

enum LBValueType {
	kLBValueString,
	kLBValueInteger
};

struct LBValue {
	LBValue() { type = kLBValueInteger; integer = 0; }

	LBValueType type;
	Common::String string;
	int integer;

	bool operator==(const LBValue &x) const;
	bool operator!=(const LBValue &x) const;
};

class LBItem {
public:
	LBItem(MohawkEngine_LivingBooks *vm, Common::Rect rect);
	virtual ~LBItem();

	void readFrom(Common::SeekableSubReadStreamEndian *stream);
	virtual void readData(uint16 type, uint16 size, Common::SeekableSubReadStreamEndian *stream);

	virtual void destroySelf(); // 0x2
	virtual void setEnabled(bool enabled); // 0x3
	virtual void setGlobalEnabled(bool enabled);
	virtual bool contains(Common::Point point); // 0x7
	virtual void update(); // 0x8
	virtual void draw() { } // 0x9
	virtual void handleKeyChar(Common::Point pos) { } // 0xA
	virtual void handleMouseDown(Common::Point pos); // 0xB
	virtual void handleMouseMove(Common::Point pos); // 0xC
	virtual void handleMouseUp(Common::Point pos); // 0xD
	virtual bool togglePlaying(bool playing, bool restart = false); // 0xF
	virtual void done(bool onlyNotify); // 0x10
	virtual void init() { } // 0x11
	virtual void seek(uint16 pos) { } // 0x13
	virtual void setFocused(bool focused) { } // 0x14
	virtual void setVisible(bool visible); // 0x17
	virtual void setGlobalVisible(bool enabled);
	virtual void startPhase(uint phase); // 0x18
	virtual void stop(); // 0x19
	virtual void notify(uint16 data, uint16 from); // 0x1A

	uint16 getId() { return _itemId; }

protected:
	MohawkEngine_LivingBooks *_vm;

	void setNextTime(uint16 min, uint16 max);
	void setNextTime(uint16 min, uint16 max, uint32 start);

	Common::Rect _rect;
	Common::String _desc;
	uint16 _resourceId;
	uint16 _itemId;

	bool _visible, _globalVisible, _playing, _enabled, _neverEnabled, _globalEnabled;

	uint32 _nextTime, _startTime;
	uint16 _loops;

	uint16 _phase, _timingMode, _delayMin, _delayMax;
	uint16 _loopMode, _loopCount, _periodMin, _periodMax;
	uint16 _controlMode;
	Common::Point _relocPoint;

	bool _isAmbient;

	Common::Array<LBScriptEntry *> _scriptEntries;
	void runScript(uint id, uint16 data = 0, uint16 from = 0);

	LBValue parseValue(const Common::String &command, uint &pos);
	void runCommand(const Common::String &command);
	bool checkCondition(const Common::String &condition);
};

class LBSoundItem : public LBItem {
public:
	LBSoundItem(MohawkEngine_LivingBooks *_vm, Common::Rect rect);
	~LBSoundItem();

	void update();
	bool togglePlaying(bool playing, bool restart);
	void stop();

protected:
	bool _running;
};

struct GroupEntry {
	uint entryId;
	uint entryType;
};

class LBGroupItem : public LBItem {
public:
	LBGroupItem(MohawkEngine_LivingBooks *_vm, Common::Rect rect);

	void readData(uint16 type, uint16 size, Common::SeekableSubReadStreamEndian *stream);

	void setEnabled(bool enabled);
	void setGlobalEnabled(bool enabled);
	bool contains(Common::Point point);
	bool togglePlaying(bool playing, bool restart);
	// 0x12
	void seek(uint16 pos);
	void setVisible(bool visible);
	void setGlobalVisible(bool visible);
	void startPhase(uint phase);
	void stop();
	
protected:
	bool _starting;

	Common::Array<GroupEntry> _groupEntries;
};

class LBPaletteItem : public LBItem {
public:
	LBPaletteItem(MohawkEngine_LivingBooks *_vm, Common::Rect rect);

	void readData(uint16 type, uint16 size, Common::SeekableSubReadStreamEndian *stream);

	bool togglePlaying(bool playing, bool restart);
	void update();

protected:
	uint16 _fadeInPeriod, _fadeInStep, _drawStart, _drawCount;
	uint32 _fadeInStart, _fadeInCurrent;
	byte _palette[255 * 4];
};

struct LiveTextWord {
	Common::Rect bounds;
	uint16 soundId;
};

struct LiveTextPhrase {
	uint16 wordStart, wordCount;
	uint16 highlightStart, highlightEnd;
	uint16 startId, endId;
};

class LBLiveTextItem : public LBItem {
public:
	LBLiveTextItem(MohawkEngine_LivingBooks *_vm, Common::Rect rect);

	void readData(uint16 type, uint16 size, Common::SeekableSubReadStreamEndian *stream);

	bool contains(Common::Point point);
	void update();
	void draw();
	void handleMouseDown(Common::Point pos);
	bool togglePlaying(bool playing, bool restart);
	void stop();
	void notify(uint16 data, uint16 from);

protected:
	void paletteUpdate(uint16 word, bool on);
	void drawWord(uint word, uint yPos);

	uint16 _currentPhrase, _currentWord;

	byte _backgroundColor[4];
	byte _foregroundColor[4];
	byte _highlightColor[4];
	uint16 _paletteIndex;

	Common::Array<LiveTextWord> _words;
	Common::Array<LiveTextPhrase> _phrases;
};

class LBPictureItem : public LBItem {
public:
	LBPictureItem(MohawkEngine_LivingBooks *_vm, Common::Rect rect);

	void readData(uint16 type, uint16 size, Common::SeekableSubReadStreamEndian *stream);

	bool contains(Common::Point point);
	void draw();
	void init();
};

class LBAnimationItem : public LBItem {
public:
	LBAnimationItem(MohawkEngine_LivingBooks *_vm, Common::Rect rect);
	~LBAnimationItem();

	void setEnabled(bool enabled);
	bool contains(Common::Point point);
	void update();
	void draw();
	bool togglePlaying(bool playing, bool restart);
	void done(bool onlyNotify);
	void init();
	void seek(uint16 pos);
	void startPhase(uint phase);
	void stop();

protected:
	LBAnimation *_anim;
	bool _running;
};

struct NotifyEvent {
	NotifyEvent(uint t, uint p) : type(t), param(p) { }
	uint type;
	uint param;
};

enum DelayedEventType {
	kLBEventDestroy = 0,
	kLBEventSetNotVisible = 1,
	kLBEventDone = 2
};

struct DelayedEvent {
	DelayedEvent(LBItem *i, DelayedEventType t) : item(i), type(t) { }
	LBItem *item;
	DelayedEventType type;
};

class MohawkEngine_LivingBooks : public MohawkEngine {
protected:
	Common::Error run();

public:
	MohawkEngine_LivingBooks(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine_LivingBooks();

	Common::RandomSource *_rnd;

	LBGraphics *_gfx;
	bool _needsRedraw, _needsUpdate;

	void addNotifyEvent(NotifyEvent event);

	Common::SeekableSubReadStreamEndian *wrapStreamEndian(uint32 tag, uint16 id);
	Common::Rect readRect(Common::SeekableSubReadStreamEndian *stream);
	GUI::Debugger *getDebugger() { return _console; }

	LBItem *getItemById(uint16 id);

	void setFocus(LBItem *focus);
	void setEnableForAll(bool enable, LBItem *except = 0);
	void notifyAll(uint16 data, uint16 from);
	void queueDelayedEvent(DelayedEvent event);

	bool isBigEndian() const { return getGameType() != GType_LIVINGBOOKSV1 || getPlatform() == Common::kPlatformMacintosh; }

	LBMode getCurMode() { return _curMode; }

	bool tryLoadPageStart(LBMode mode, uint page);
	void prevPage();
	void nextPage();

	// TODO: make private
	Common::HashMap<Common::String, LBValue> _variables;

private:
	LivingBooksConsole *_console;
	Common::ConfigFile _bookInfoFile;

	Common::String getBookInfoFileName() const;
	void loadBookInfo(const Common::String &filename);

	Common::String stringForMode(LBMode mode);

	bool _readOnly, _introDone;
	LBMode _curMode;
	uint16 _curPage, _curSubPage;
	uint16 _phase;
	Common::Array<LBItem *> _items;
	Common::Queue<DelayedEvent> _eventQueue;
	LBItem *_focus;
	void destroyPage();
	bool loadPage(LBMode mode, uint page, uint subpage);
	void updatePage();

	uint16 getResourceVersion();
	void loadBITL(uint16 resourceId);
	void loadSHP(uint16 resourceId);

	bool tryDefaultPage();

	void handleUIMenuClick(uint controlId);
	void handleUIPoetryMenuClick(uint controlId);
	void handleUIQuitClick(uint controlId);
	void handleUIOptionsClick(uint controlId);

	Common::Queue<NotifyEvent> _notifyEvents;
	void handleNotify(NotifyEvent &event);

	uint16 _screenWidth;
	uint16 _screenHeight;
	uint16 _numLanguages;
	uint16 _numPages;
	Common::String _title;
	Common::String _copyright;
	bool _poetryMode;

	uint16 _curLanguage;
	uint16 _curSelectedPage;
	bool _alreadyShowedIntro;

	// String Manipulation Functions
	Common::String removeQuotesFromString(const Common::String &string);
	Common::String convertMacFileName(const Common::String &string);
	Common::String convertWinFileName(const Common::String &string);

	// Configuration File Functions
	Common::String getStringFromConfig(const Common::String &section, const Common::String &key);
	int getIntFromConfig(const Common::String &section, const Common::String &key);
	Common::String getFileNameFromConfig(const Common::String &section, const Common::String &key);

	// Platform/Version functions
	MohawkArchive *createMohawkArchive() const;
};

} // End of namespace Mohawk

#endif
