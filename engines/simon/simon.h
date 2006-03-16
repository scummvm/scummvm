/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef SIMON_H
#define SIMON_H

#include <stdio.h>

#include "base/engine.h"

#include "common/rect.h"
#include "common/util.h"

#include "simon/midi.h"
#include "simon/sound.h"
#include "simon/vga.h"

class GameDetector;

namespace Simon {

/* Various other settings */
//#define DUMP_FILE_NR 8
//#define DUMP_BITMAPS_FILE_NR 8

uint fileReadItemID(Common::File *in);

#define CHECK_BOUNDS(x, y) assert((uint)(x) < ARRAYSIZE(y))
#define NUM_PALETTE_FADEOUT 32

struct Child;
struct Child2;

struct Item;
struct FillOrCopyStruct;
struct Subroutine;
struct SubroutineLine;
struct TimeEvent;

struct TextLocation {
	int16 x, y, width;
	TextLocation() { memset(this, 0, sizeof(*this)); }
};

struct HitArea {
	uint16 x, y;
	uint16 width, height;
	uint16 flags;
	uint16 id;
	FillOrCopyStruct *fcs;
	Item *item_ptr;
	uint16 unk3;
	uint16 layer;
	HitArea() { memset(this, 0, sizeof(*this)); }
};

struct VgaPointersEntry {
	byte *vgaFile1;
	byte *vgaFile2;
	byte *sfxFile;
	VgaPointersEntry() { memset(this, 0, sizeof(*this)); }
};

struct VgaSprite {
	uint16 id;
	uint16 image;
	uint16 palette;
	uint16 x, y;									/* actually signed numbers */
	uint16 flags;
	uint16 priority;
	uint16 windowNum, fileId;
	VgaSprite() { memset(this, 0, sizeof(*this)); }
};

struct VgaSleepStruct {
	uint16 ident;
	const byte *code_ptr;
	uint16 sprite_id;
	uint16 cur_vga_file;
	VgaSleepStruct() { memset(this, 0, sizeof(*this)); }
};

struct VgaTimerEntry {
	uint16 delay;
	const byte *script_pointer;
	uint16 sprite_id;
	uint16 cur_vga_file;
	VgaTimerEntry() { memset(this, 0, sizeof(*this)); }
};

enum SimonTypes {
	GType_SIMON1,
	GType_SIMON2,
	GType_FF
};

struct GameFileDescription {
	const char *fileName;
	uint16 fileType;
};

struct GameDescription {
	const char *name;
	SimonTypes gameType;
	GameIds gameId;
	const char *extra;
	int filesCount;
	GameFileDescription *filesDescriptions;
	uint32 features;
	Common::Language language;
	Common::Platform platform;
};

struct GameSpecificSettings;

class Debugger;

class SimonEngine : public Engine {
	friend class Debugger;

	void errorString(const char *buf_input, char *buf_output);

	typedef void (SimonEngine::*VgaOpcodeProc) ();
	void setupVgaOpcodes();
	const VgaOpcodeProc *_vga_opcode_table;

public:
	GameDescription *_gameDescription;

	bool initGame(void);

	int getGameId() const { return _gameDescription->gameId; }
	int getGameType() const { return _gameDescription->gameType; }
	uint32 getFeatures() const { return _gameDescription->features; }
	Common::Language getLanguage() const { return _gameDescription->language; }
	Common::Platform getPlatform() const { return _gameDescription->platform; }

protected:
	void playSting(uint a);

	const byte *_vcPtr;								/* video code ptr */
	uint16 _vc_get_out_of_code;


	uint32 *_gameOffsetsPtr;

	uint VGA_DELAY_BASE;
	uint TABLE_INDEX_BASE;
	uint TEXT_INDEX_BASE;
	uint NUM_VIDEO_OP_CODES;
	uint VGA_MEM_SIZE;
	uint TABLES_MEM_SIZE;
	uint MUSIC_INDEX_BASE;
	uint SOUND_INDEX_BASE;
	const GameSpecificSettings *gss;

	byte _keyPressed;

	typedef enum {
		FORMAT_NONE,
		FORMAT_MP3,
		FORMAT_WAV,
		FORMAT_VOC
	} SoundFormat;

	Common::File *_gameFile;

	byte *_strippedTxtMem;
	uint _textSize;
	uint _stringTabNum, _stringTabPos, _stringtab_numalloc;
	byte **_stringTabPtr;

	Item **_itemArrayPtr;
	uint _itemArraySize;
	uint _itemArrayInited;

	byte *_itemHeapPtr;
	uint _itemHeapCurPos;
	uint _itemHeapSize;

	byte *_iconFilePtr;

	byte *_tblList;

	const byte *_codePtr;

	byte **_localStringtable;
	uint _stringIdLocalMin, _stringIdLocalMax;

	byte *_tablesHeapPtr, *_tablesHeapPtrOrg, *_tablesheapPtrNew;
	uint _tablesHeapSize, _tablesHeapCurPos, _tablesHeapCurPosOrg;
	uint _tablesHeapCurPosNew;

	Subroutine *_subroutineList, *_subroutineListOrg;
	uint _subroutine;

	uint _dxSurfacePitch;

	uint _recursionDepth;

	uint32 _lastVgaTick;

	uint16 _marks;

	bool _scriptVar2;
	bool _runScriptReturn1;
	bool _skipVgaWait;
	bool _noParentNotify;
	bool _beardLoaded;
	bool _hitarea_unk_3;
	bool _mortalFlag;
	byte _updateScreen;
	bool _usePaletteDelay;
	bool _syncFlag2;
	bool _inCallBack;
	bool _cepeFlag;
	byte _copyPartialMode;
	uint _speed;
	bool _fastMode;
	bool _dxUse3Or4ForLock;

	uint16 _debugMode;
	uint16 _language;
	bool _pause;
	bool _startMainScript;
	bool _continousMainScript;
	bool _startVgaScript;
	bool _continousVgaScript;
	bool _drawImagesDebug;
	bool _dumpImages;
	bool _speech;
	bool _subtitles;
	bool _fade;
	byte _mouseCursor;
	bool _vgaVar9;
	int16 _scriptUnk1;
	bool _vgaVar6;
	int _scrollX, _scrollXMax, _scrollHeight;
	int _scrollY, _scrollYMax, _scrollWidth;
	int _scrollCount, _scrollFlag;
	const byte *_scrollImage;
	byte _vgaVar8;

 	uint16 _hyperLink;
 	uint16 _oracleMaxScrollY, _noOracleScroll;
 
	int16 _scriptVerb, _scriptNoun1, _scriptNoun2;
	int16 _scriptAdj1, _scriptAdj2;

	uint16 _curWindow;
	FillOrCopyStruct *_textWindow;

	Item *_subjectItem, *_objectItem;
	Item *_item1;

	Item *_hitAreaObjectItem;
	HitArea *_lastHitArea;
	HitArea *_lastHitArea2Ptr;
	HitArea *_lastHitArea3;
	byte _leftButtonDown;
	Item *_hitAreaSubjectItem;
	HitArea *_hitAreaPtr5, *_hitAreaPtr7;
	uint _needHitAreaRecalc;
	uint _verbHitArea;
	uint16 _hitAreaUnk4;
	uint _mouseHideCount;

	uint16 _windowNum;

	uint _printCharCurPos, _printCharMaxPos;
	uint _numLettersToPrint;

	uint _lastTime;

	TimeEvent *_firstTimeStruct, *_pendingDeleteTimeEvent;

	uint _base_time;

	uint _mouseX, _mouseY;
	uint _mouseXOld, _mouseYOld;

	Item *_dummyItem1;
	Item *_dummyItem2;
	Item *_dummyItem3;

	volatile uint16 _lockWord;
	uint16 _scrollUpHitArea;
	uint16 _scrollDownHitArea;

	uint16 _videoVar7;
	volatile uint16 _paletteColorCount;

	uint _screenWidth, _screenHeight;

	byte _videoVar4;
	bool _videoVar5;
	bool _fastFadeOutFlag;
	bool _unkPalFlag;
	bool _exitCutscene;
	bool _skipSpeech;
	byte _paletteFlag;

	uint _soundFileId;
	int16 _lastMusicPlayed;
	int16 _nextMusicToPlay;

	bool _showPreposition;
	bool _showMessageFlag;

	uint _videoNumPalColors;

	uint _vgaSpriteChanged;

	byte *_vgaBufFreeStart, *_vgaBufEnd, *_vgaBufStart;
	byte *_vgaFileBufOrg, *_vgaFileBufOrg2;

	byte *_curVgaFile1;
	byte *_curVgaFile2;
	byte *_curSfxFile;

	uint16 _timer1, _timer5, _timer4;

	uint16 _frameRate;

	uint16 _vgaCurFile2;
	uint16 _vgaWaitFor, _vgaCurFileId;
	uint16 _vgaCurSpriteId;
	uint16 _vgaCurSpritePriority;

	int16 _baseY;
	float _scale;
	Common::Rect _feebleRect;
	int _scaleX, _scaleY, _scaleWidth, _scaleHeight;

	VgaTimerEntry *_nextVgaTimerToProcess;

	Item *_vcItemArray[20];
	Item *_itemArray6[20];

	uint16 _stringIdArray2[20];
	uint16 _stringIdArray3[20];
	uint16 _speechIdArray4[20];

	uint16 _bitArray[48];
	int16 _variableArray[256];

	FillOrCopyStruct *_windowArray[8];

	byte _fcsData1[8];
	bool _fcsData2[8];

	TextLocation _textLocation1, _textLocation2, _textLocation3, _textLocation4;

	int _freeStringSlot;

	byte _stringReturnBuffer[2][180];

	HitArea _hitAreas[90];

	VgaPointersEntry _vgaBufferPointers[450];
	VgaSprite _vgaSprites[180];
	VgaSleepStruct _vgaSleepStructs[30];

	const uint16 *_pathFindArray[100];

	uint8 _pathValues[400];
	uint8 _PVCount;
	uint8 _GPVCount;

	uint8 _pathValues1[400];
	uint8 _PVCount1;
	uint8 _GPVCount1;

	uint8 _paletteBackup[1024];
	uint8 _palette[1024];

	byte _videoBuf1[3000];

	VgaTimerEntry _vgaTimerList[95];

	FillOrCopyStruct *_fcs_list;

	byte _lettersToPrintBuf[80];

	MidiPlayer midi;
	bool _native_mt32;

	int _numScreenUpdates;
	int _vgaTickCounter;

	Sound *_sound;

	bool _effectsPaused;
	bool _ambientPaused;
	bool _musicPaused;

	Debugger *_debugger;

	FILE *_dumpFile;

	int _saveLoadRowCurPos;
	int _numSaveGameRows;
	bool _saveDialogFlag;
	bool _saveOrLoad;
	bool _saveLoadFlag;

	byte _saveLoadType, _saveLoadSlot;
	char _saveLoadName[32];

	int _sdlMouseX, _sdlMouseY;

	byte *_sdl_buf_3;
	byte *_sdl_buf;
	byte *_sdl_buf_attached;
	byte *_sdl_buf_scaled;

	Common::RandomSource _rnd;

	const byte *_vc10BasePtrOld;
	byte _hebrew_char_widths[32];

public:
	SimonEngine(OSystem *syst);
	virtual ~SimonEngine();

protected:
	uint16 to16Wrapper(uint value);
	uint16 readUint16Wrapper(const void *src);

	int allocGamePcVars(Common::File *in);
	void loginPlayerHelper(Item *item, int a, int b);
	void loginPlayer();
	void allocateStringTable(int num);
	void setupStringTable(byte *mem, int num);
	void setupLocalStringTable(byte *mem, int num);
	void readGamePcText(Common::File *in);
	void readItemChildren(Common::File *in, Item *item, uint tmp);
	void readItemFromGamePc(Common::File *in, Item *item);
	void loadGamePcFile(const char *filename);

	void palette_fadeout(uint32 *pal_values, uint num);
	
	byte *allocateItem(uint size);
	byte *allocateTable(uint size);
	void alignTableMem();

	Child *findChildOfType(Item *i, uint child);
	Child *allocateChildBlock(Item *i, uint type, uint size);

	void allocItemHeap();
	void allocTablesHeap();

	Subroutine *createSubroutine(uint a);
	void readSubroutine(Common::File *in, Subroutine *sub);
	SubroutineLine *createSubroutineLine(Subroutine *sub, int a);
	void readSubroutineLine(Common::File *in, SubroutineLine *new_table, Subroutine *sub);
	byte *readSingleOpcode(Common::File *in, byte *ptr);
	void readSubroutineBlock(Common::File *in);

	Subroutine *getSubroutineByID(uint subroutine_id);

	/* used in debugger */
	void dumpSubroutines();
	void dumpSubroutine(Subroutine *sub);
	void dumpSubroutineLine(SubroutineLine *sl, Subroutine *sub);
	const byte *dumpOpcode(const byte *p);

	int startSubroutine(Subroutine *sub);
	int startSubroutineEx(Subroutine *sub);

	bool checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub);

	int runScript();

	Item *getNextItemPtr();
	uint getNextItemID();
	uint getItem1ID() {return 1;}
	Item *getItem1Ptr();
	Item *getItemPtrB();

	byte getByte();
	int getNextWord();

	uint getNextVarContents();
	uint getVarOrWord();
	uint getVarOrByte();
	uint readVariable(uint variable);
	void writeNextVarContents(uint16 contents);
	void writeVariable(uint variable, uint16 contents);

	void setItemParent(Item *item, Item *parent);

	uint itemPtrToID(Item *id);

	Item *derefItem(uint item);
	void setItemState(Item *item, int value);

	void showMessageFormat(const char *s, ...);
	const byte *getStringPtrByID(uint string_id);
	const byte *getLocalStringByID(uint string_id);
	uint getNextStringID();

	void addTimeEvent(uint timeout, uint subroutine_id);
	void delTimeEvent(TimeEvent *te);

	bool isRoom(Item *item);
	bool isObject(Item *item);

	void itemChildrenChanged(Item *item);
	void unlinkItem(Item *item);
	void linkItem(Item *item, Item *parent);

	bool o_chance(uint a);

	void o_kill_sprite_simon1(uint a);
	void o_kill_sprite_simon2(uint a, uint b);

	void o_vga_reset();
	void changeWindow(uint a);
	void o_unk_103();
	void closeWindow(uint a);
	void clear_hitarea_bit_0x40(uint hitarea);
	void set_hitarea_bit_0x40(uint hitarea);
	void set_hitarea_x_y(uint hitarea, int x, int y);
	bool is_hitarea_0x40_clear(uint hitarea);
	void delete_hitarea(uint hitarea);
	void addNewHitArea(int id, int x, int y, int width, int height, int flags, int unk3, Item *item_ptr);
	HitArea *findEmptyHitArea();
	void hitarea_proc_1();
	void handle_verb_hitarea(HitArea * ha);
	void hitarea_leave(HitArea * ha);
	void leaveHitAreaById(uint hitarea_id);

	void o_waitForSync(uint a);
	void skipSpeech();
	void o_sync(uint a);
	void o_playMusic();
	void o_saveGame();
	void o_loadGame();
	void o_confirmQuit();
	void o_restoreIconArray(uint a);
	void o_freezeBottom();
	void killAllTimers();

	uint getOffsetOfChild2Param(Child2 *child, uint prop);
	void o_unk_160(uint a);
	void o_playSFX(uint a);
	void o_lockZone();
	void o_unlockZone();
	void o_pathfind(int x, int y, uint var_1, uint var_2);
	void o_mouseOn();
	void o_mouseOff();
	void o_loadBeard();
	void o_unloadBeard();
	void o_unloadZone(uint a);
	void o_unfreezeBottom();
	void o_fadeToBlack();

	TextLocation *getTextLocation(uint a);
	void o_printStr();
	void o_setup_cond_c();
	void setup_cond_c_helper();

	void o_inventory_descriptions();

 	void hyperLinkOn(uint16 x);
 	void hyperLinkOff();
 
	void mouseOff();
	void mouseOn();

	void drawIconArray(uint i, Item *item_ptr, int unk1, int unk2);

	void loadTextIntoMem(uint string_id);
	void loadTablesIntoMem(uint subr_id);

	uint loadTextFile(const char *filename, byte *dst);
	Common::File *openTablesFile(const char *filename);
	void closeTablesFile(Common::File *in);

	uint loadTextFile_simon1(const char *filename, byte *dst);
	Common::File *openTablesFile_simon1(const char *filename);

	uint loadTextFile_gme(const char *filename, byte *dst);
	Common::File *openTablesFile_gme(const char *filename);

	void invokeTimeEvent(TimeEvent *te);
	bool kickoffTimeEvents();

	void defocusHitarea();
	void endCutscene();
	void runSubroutine101();
	void handle_uparrow_hitarea(FillOrCopyStruct *fcs);
	void handle_downarrow_hitarea(FillOrCopyStruct *fcs);
	void hitareaChangedHelper();
	void focusVerb(uint hitarea_id);
	HitArea *findHitAreaByID(uint hitarea_id);

	void showActionString(uint x, const byte *string);
	void video_putchar(FillOrCopyStruct *fcs, byte c, byte b = 0);
	void video_fill_or_copy_from_3_to_2(FillOrCopyStruct *fcs);
	void video_toggle_colors(HitArea * ha, byte a, byte b, byte c, byte d);

	void read_vga_from_datfile_1(uint vga_id);

	uint get_fcs_ptr_3_index(FillOrCopyStruct *fcs);

	void setup_hitarea_from_pos(uint x, uint y, uint mode);
	void new_current_hitarea(HitArea * ha);
	bool hitarea_proc_2(uint a);
	bool hitarea_proc_3(Item *item);
	void f10_key();
	void hitarea_stuff();

	void handle_mouse_moved();
	void pollMouseXY();
	void drawMousePointer();

	void removeIconArray(uint fcs_index);
	void draw_icon_c(FillOrCopyStruct *fcs, uint icon, uint x, uint y);
	bool has_item_childflag_0x10(Item *item);
	uint item_get_icon_number(Item *item);
	uint setup_icon_hit_area(FillOrCopyStruct *fcs, uint x, uint y, uint icon_number, Item *item_ptr);
	void addArrows(FillOrCopyStruct *fcs, uint fcs_index);

	void loadIconFile();
	void processSpecialKeys();
	void hitarea_stuff_helper();

	void startUp_helper_2();
	void showmessage_helper_3(uint a, uint b);
	void showmessage_print_char(byte chr);

	void handle_verb_clicked(uint verb);

	void o_set_video_mode(uint mode, uint vga_res);
	void set_video_mode_internal(uint mode, uint vga_res_id);

	void o_loadZone(uint vga_res);
	void loadZone(uint vga_res);

	void loadSprite(uint windowNum, uint vga_res, uint vga_sprite_id, uint x, uint y, uint palette);
	void o_defineWindow(uint a, uint b, uint c, uint d, uint e, uint f, uint g, uint h);
	void playSpeech(uint speech_id, uint vga_sprite_id);
	void printText(uint vga_sprite_id, uint color, const char *string_ptr, int16 x, int16 y, int16 width);
	FillOrCopyStruct *openWindow(uint x, uint y, uint w, uint h, uint flags, uint fill_color, uint text_color);

	void render_string_amiga(uint vga_sprite_id, uint color, uint width, uint height, const char *txt);
	void render_string(uint vga_sprite_id, uint color, uint width, uint height, const char *txt);

	void setArrowHitAreas(FillOrCopyStruct *fcs, uint fcs_index);

	byte *setup_vga_destination(uint32 size);
	void vga_buf_unk_proc3(byte *end);
	void vga_buf_unk_proc1(byte *end);
	void vga_buf_unk_proc2(uint a, byte *end);
	void delete_memptr_range(byte *end);

	void setup_vga_file_buf_pointers();

	void run_vga_script();

public:
	// Simon1/Simon2 video script opcodes
	void vc1_fadeOut();
	void vc2_call();
	void vc3_loadSprite();
	void vc4_fadeIn();
	void vc5_skip_if_neq();
	void vc6_skip_ifn_sib_with_a();
	void vc7_skip_if_sib_with_a();
	void vc8_skip_if_parent_is();
	void vc9_skip_if_unk3_is();
	void vc10_draw();
	void vc11_clearPathFinder();
	void vc12_delay();
	void vc13_addToSpriteX();
	void vc14_addToSpriteY();
	void vc15_wakeup_id();
	void vc16_sleep_on_id();
	void vc17_setPathfinderItem();
	void vc18_jump();
	void vc19_chain_to_script();
	void vc20_setRepeat();
	void vc21_endRepeat();
	void vc22_setSpritePalette();
	void vc23_setSpritePriority();
	void vc24_setSpriteXY();
	void vc25_halt_sprite();
	void vc26_setSubWindow();
	void vc27_resetSprite();
	void vc28_dummy_op();
	void vc29_stopAllSounds();
	void vc30_setFrameRate();
	void vc31_setWindow();
	void vc32_copyVar();
	void vc33_setMouseOn();
	void vc34_setMouseOff();
	void vc35_clearWindow();
	void vc36_setWindowImage();
	void vc37_addToSpriteY();
	void vc38_skipIfVarZero();
	void vc39_setVar();
	void vc40();
	void vc41();
	void vc42_delayIfNotEQ();
	void vc43_skipIfBitClear();
	void vc44_skipIfBitSet();
	void vc45_setSpriteX();
	void vc46_setSpriteY();
	void vc47_addToVar();
	void vc48_setPathFinder();
	void vc49_setBit();
	void vc50_clearBit();
	void vc51_clear_hitarea_bit_0x40();
	void vc52_playSound();
	void vc53_no_op();
	void vc54_no_op();
	void vc55_offset_hit_area();
	void vc56_delay();
	void vc57_no_op();
	void vc58();
	void vc59();
	void vc60_killSprite();
	void vc61_setMaskImage();
	void vc62_fastFadeOut();
	void vc63_fastFadeIn();

	// Simon2 specific Video Script Opcodes
	void vc64_skipIfSpeechEnded();
	void vc65_slowFadeIn();
	void vc66_skipIfNotEqual();
	void vc67_skipIfGE();
	void vc68_skipIfLE();
	void vc69_playTrack();
	void vc70_queueMusic();
	void vc71_checkMusicQueue();
	void vc72_play_track_2();
	void vc73_setMark();
	void vc74_clearMark();

	// Feeble specific Video Script Opcodes
	void vc75_setScale();
	void vc76_setScaleXOffs();
	void vc77_setScaleYOffs();
	void vc78_computeXY();
	void vc79_computePosNum();
	void vc80_setOverlayImage();
	void vc81_setRandom();
	void vc82_getPathValue();
	void vc83_playSoundLoop();
	void vc84_stopSoundLoop();

protected:
	void drawImages(VC10_state *state);
	void drawImages_Feeble(VC10_state *state);
	bool drawImages_clip(VC10_state *state);
	void scaleClip(int16 h, int16 w, int16 y, int16 x, int16 scrollY);

	void delete_vga_timer(VgaTimerEntry * vte);
	void vcResumeSprite(const byte *code_ptr, uint16 cur_file, uint16 cur_sprite);
	int vcReadVarOrWord();
	uint vcReadNextWord();
	uint vcReadNextByte();
	uint vcReadVar(uint var);
	void vcWriteVar(uint var, int16 value);
	void vcSkipNextInstruction();

	int getScale(int y, int x);

	bool itemIsSiblingOf(uint16 val);
	bool itemIsParentOf(uint16 a, uint16 b);
	bool vc_maybe_skip_proc_1(uint16 a, int16 b);

	void add_vga_timer(uint num, const byte *code_ptr, uint cur_sprite, uint cur_file);
	VgaSprite *findCurSprite();

	bool vcGetBit(uint bit);
	void vcSetBitTo(uint bit, bool value);

	void expire_vga_timers();

	bool isSpriteLoaded(uint16 id, uint16 fileId);

	void fcs_setTextColor(FillOrCopyStruct *fcs, uint value);

	void video_copy_if_flag_0x8_c(FillOrCopyStruct *fcs);
	void delete_hitarea_by_index(uint index);

	void removeArrows(FillOrCopyStruct *fcs, uint fcs_index);
	void fcs_putchar(uint a);

	void copy_img_from_3_to_2(FillOrCopyStruct *fcs);
	void video_erase(FillOrCopyStruct *fcs);

	void dx_copy_rgn_from_3_to_2(uint b, uint r, uint y, uint x);

	byte *getBackBuf();
	byte *getFrontBuf();
	byte *getScaleBuf();

	byte *read_vga_from_datfile_2(uint id, uint type);

	void resfile_read(void *dst, uint32 offs, uint32 size);

	int init(GameDetector &detector);
	int go();
	void openGameFile();

	void timer_callback();
	void timer_proc1();

	void timer_vga_sprites();
	void timer_vga_sprites_2();

	void dx_clear_surfaces(uint num_lines);
	void dx_update_screen_and_palette();

	void dump_video_script(const byte *src, bool one_opcode_only);
	void dump_vga_file(const byte *vga);
	void dump_vga_script(const byte *ptr, uint res, uint sprite_id);
	void dump_vga_script_always(const byte *ptr, uint res, uint sprite_id);
	void dump_vga_bitmaps(const byte *vga, byte *vga1, int res);
	void dump_single_bitmap(int file, int image, const byte *offs, int w, int h, byte base);

	void dx_clear_attached_from_top(uint lines);
	void dx_copy_from_attached_to_2(uint x, uint y, uint w, uint h);
	void dx_copy_from_attached_to_3(uint lines);
	void dx_copy_from_2_to_attached(uint x, uint y, uint w, uint h);

	void print_char_helper_1(const byte *src, uint len);
	void print_char_helper_5(FillOrCopyStruct *fcs);

	void quickLoadOrSave();
	void shutdown();

	byte *vc10_uncompressFlip(const byte *src, uint w, uint h);
	byte *vc10_flip(const byte *src, uint w, uint h);

	Item *getNextItemPtrStrange();

	bool saveGame(uint slot, char *caption);
	bool loadGame(uint slot);

	void showmessage_helper_2();
	void print_char_helper_6(uint i);

	void video_putchar_newline(FillOrCopyStruct *fcs);
	void video_putchar_drawchar(FillOrCopyStruct *fcs, uint x, uint y, byte chr);

	void loadMusic(uint music);
	void checkTimerCallback();
	void delay(uint delay);
	void pause();

	void o_83_helper();
	void o_waitForMark(uint i);
	void timer_vga_sprites_helper();

	void decodeStripA(byte *dst, const byte *src, int height);
	void scroll_timeout();
	void hitarea_stuff_helper_2();
	void fastFadeIn();
	void slowFadeIn();

	void vc_kill_sprite(uint file, uint sprite);

	void set_dummy_cursor();

	void set_volume(int volume);

	void saveOrLoadDialog(bool load);
	void o_unk_132_helper_3();
	int o_unk_132_helper(bool *b, char *buf);
	void o_clearCharacter(FillOrCopyStruct *fcs, int x, byte b = 0);
	void saveGameDialog(char *buf);
	void o_fileError(FillOrCopyStruct *fcs, bool save_error);

	int countSaveGames();
	int displaySaveGameList(int curpos, bool load, char *dst);

	void show_it(void *buf);

	char *gen_savename(int slot);
};

} // End of namespace Simon

#endif
