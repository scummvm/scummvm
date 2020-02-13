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

#ifndef CRYO_EDEN_H
#define CRYO_EDEN_H

#include "common/file.h"
#include "common/savefile.h"
#include "common/serializer.h"

#include "cryo/sound.h"
#include "cryo/defs.h"

enum Direction {
	kCryoNorth = 0,
	kCryoEast = 1,
	kCryoSouth = 2,
	kCryoWest = 3
};

namespace Cryo {

class CryoEngine;
class EdenGraphics;

class EdenGame {
private:
	
	EdenGraphics *_graphics;

public:
	EdenGame(CryoEngine *vm);
	~EdenGame();

	void run();
	object_t *getObjectPtr(int16 id);
	void showObjects();
	void saveFriezes();
	void useBank(int16 bank);
	void musicspy();
	void fademusica0(int16 delay);
	void wait(int howlong);
	bool isObjectHere(int16 id);
	void display();

	void setMouseCenterX(uint16 xpos);
	void setMouseCenterY(uint16 ypos);

	void stopMusic();

	void setVolume(uint16 vol);

	uint16 getMouseCenterX();
	uint16 getMouseCenterY();

	bool dialoscansvmas(Dialog *dial);
	void musique();
	void preloadDialogs(int16 vid);
	void loadHnm(uint16 num);
	bool personIsTalking();
	bool animationIsActive();
	byte *getImageDesc();
	byte *getPlaceRawBuf();
	byte getActionCursor(byte value);
	int16 getNumTextLines();
	int16 getScrollPos();
	/* 
	 * Identify person based on current video number
	 */
	perso_t *personSubtitles();

	int16 getGameIconY(int16 index);
	int16 getGameIconX(int16 index);

	byte *getGameDialogs();

	bool getSpecialTextMode();
	void setSpecialTextMode(bool value);

	void setCursorSaved(bool cursorSaved);
	bool getCursorSaved();
	bool getNoPalette();
	int16 getCurBankNum();
	byte *getCurKeepBuf();
	byte *getBankData();
	int16 getCurPosX();
	void setCurPosX(int16 xpos);
	int16 getCurPosY();
	void setCurPosY(int16 ypos);
	byte *getGlowBuffer();
	void setMusicFade(byte value);
	bool isMouseHeld();
	void setMouseHeld();
	void setMouseNotHeld();

	global_t *_globals; // TODO: Make private and use getters
	CryoEngine *_vm;

private:
	void removeConsole();
	void scroll();
	void resetScroll();
	void scrollFrescoes();
	void displayFrescoes();
	void gametofresques();
	void doFrescoes();
	void actionEndFrescoes();
	void scrollMirror();
	void scrollPanel();
	void displayFollower(Follower *follower, int16 x, int16 y);
	void characterInMirror();
	void gameToMirror(byte arg1);
	void flipMode();
	void quitMirror();
	void clictimbre();
	void actionClickValleyPlan();
	void gotoPlace(Goto *go);
	void deplaval(uint16 roomNum);
	void move(Direction dir);
	void move2(Direction dir);
	void actionDinoBlow();
	void actionPlateMonk();
	void actionGraaFrescoe();
	void actionLascFrescoe();
	void actionPushStone();
	void actionMummyHead();
	void actionSkelettonHead();
	void actionSkelettonMoorkong();
	void actionChoose();
	void handleDinaDialog();
	void handleKingDialog();
	void actionKingDialog1();
	void actionKingDialog2();
	void actionKingDialog3();
	void actionGetKnife();
	void actionGetPrism();
	void actionGetMushroom();
	void actionGetBadMushroom();
	void actionGetGold();
	void actionGetFullNest();
	void actionGetEmptyNest();
	void actionGetHorn();
	void actionGetSunStone();
	void actionGetEgg();
	void actionGetTablet();
	void actionLookLake();
	void actionGotoHall();
	void actionLabyrinthTurnAround();
	void actionGotoFullNest();
	void actionGotoVal();
	void actionVisit();
	void actionFinal();
	void actionMoveNorth();
	void actionMoveEast();
	void actionMoveSouth();
	void actionMoveWest();

	void afficher128();


	void restoreFriezes();

	void useMainBank();
	void useCharacterBank();

	void drawTopScreen();
	void displayValleyMap();
	void displayMapMark(int16 index, int16 location);
	void displayAdamMapMark(int16 location);
	void restoreAdamMapMark();
	void saveAdamMapMark(int16 x, int16 y);
	bool istrice(int16 roomNum);
	bool istyran(int16 roomNum);
	void istyranval(Area *area);
	char getDirection(perso_t *perso);
	bool canMoveThere(char loc, perso_t *perso);
	void scramble1(uint8 elem[4]);
	void scramble2(uint8 elem[4]);
	void scrambleDirections();
	bool naitredino(char persoType);
	void newCitadel(char area, int16 level, Room *room);
	void evolveCitadel(int16 level);
	void destroyCitadelRoom(int16 roomNum);
	void narratorBuildCitadel();
	void citadelFalls(char level);
	void buildCitadel();
	void moveDino(perso_t *perso);
	void moveAllDino();
	void newValley();
	char whereIsCita();
	bool isCita(int16 loc);
	void placeVava(Area *area);
	void vivredino();
	void vivreval(int16 areaNum);
	void handleDay();
	void addTime(int16 t);
	void animCharacter();
	void getanimrnd();
	void addanim();
	void removeMouthSprite();
	void AnimEndCharacter();
	void setCharacterSprite(byte *spr);
	void displayCharacter1();
	void displayCharacter();
	void ef_perso();
	void loadCharacter(perso_t *perso);
	void loadCurrCharacter();
	void fin_perso();
	void no_perso();
	void closeCharacterScreen();
	void displayBackgroundFollower();
	void displayNoFollower(int16 bank);
	void displayCharacterBackground1();
	void displayCharacterBackground();
	void setCharacterIcon();
	void showCharacter();
	void displayCharacterPanel();
	void getDataSync();
	int16 readFrameNumber();
	void waitEndSpeak();
	void my_bulle();
	void my_pr_bulle();
	void drawSubtitleChar(byte c, byte color, int16 width);
	
	void patchSentence();
	void vavapers();
	void citadelle();
	void selectZone();
	void showEvents1();
	void showEvents();
	void parle_mfin();
	void parlemoi_normal();
	void parle_moi();
	void initCharacterPointers(perso_t *perso);
	void perso1(perso_t *perso);
	void perso_normal(perso_t *perso);
	void handleCharacterDialog(int16 pers);
	void actionKing();
	void actionDina();
	void actionThoo();
	void actionMonk();
	void actionTormentor();
	void actionMessenger();
	void actionMango();
	void actionEve();
	void actionAzia();
	void actionMammi();
	void actionGuards();
	void actionBamboo();
	void actionKabuka();
	void actionFisher();
	void actionDino();
	void actionTyran();
	void actionMorkus();
	void comment();
	void actionAdam();
	void setChoiceYes();
	void setChoiceNo();
	bool isAnswerYes();
	void specialMushroom(perso_t *perso);
	void specialEmptyNest(perso_t *perso);
	void specialNestWithEggs(perso_t *perso);
	void specialApple(perso_t *perso);
	void specialGold(perso_t *perso);
	void specialPrism(perso_t *perso);
	void specialTalisman(perso_t *perso);
	void specialMask(perso_t *perso);
	void specialBag(perso_t *perso);
	void specialTrumpet(perso_t *perso);
	void specialWeapons(perso_t *perso);
	void specialInstrument(perso_t *perso);
	void specialEgg(perso_t *perso);
	void tyranDies(perso_t *perso);
	void specialObjects(perso_t *perso, char objid);
	void dialautoon();
	void dialautooff();
	void follow();
	void dialonfollow();
	void abortDialogue();
	void subHandleNarrator();
	void handleNarrator();
	void checkPhraseFile();
	byte *getPhrase(int16 id);
	void actionGotoMap();
	void record();
	bool dial_scan(Dialog *dial);

	bool dialogEvent(perso_t *perso);
	void characterStayHere();
	void endDeath(int16 vid);
	void chronoEvent();
	void setChrono(int16 t);


	void verifh(byte *ptr);
	void openbigfile();
	void closebigfile();
	void loadRawFile(uint16 num, byte *buffer);
	void loadIconFile(uint16 num, Icon *buffer);
	void loadRoomFile(uint16 num, Room *buffer);
	int loadSound(uint16 num);
	void convertMacToPC();
	void loadpermfiles();
	bool ReadDataSyncVOC(unsigned int num);
	bool ReadDataSync(uint16 num);
	void loadpartoffile(uint16 num, void *buffer, int32 pos, int32 len);
	void expandHSQ(byte *input, byte *output);
	void addInfo(byte info);
	void unlockInfo();
	void nextInfo();
	void removeInfo(byte info);
	void updateInfoList();
	void initGlobals();

	void closeRoom();
	void displayPlace();
	void loadPlace(int16 num);
	void specialoutside();
	void specialout();
	void specialin();
	void animpiece();
	void getdino(Room *room);
	Room *getRoom(int16 loc);
	void initPlace(int16 roomNum);
	void maj2();
	void updateRoom1(int16 roomNum);
	void updateRoom(uint16 roomNum);
	void allocateBuffers();
	void freebuf();
	void EmergencyExit();
	void edmain();
	void intro();
	void enterGame();
	void signon(const char *s);
	void FRDevents();
	Icon *scan_icon_list(int16 x, int16 y, int16 index);
	void updateCursor();
	void mouse();

	void startmusique(byte num);

	int loadmusicfile(int16 num);
	void persovox();
	void endCharacterSpeech();
	void fademusicup();

	void countObjects();
	void winObject(int16 id);
	void loseObject(int16 id);
	void lostObject();

	void objectmain(int16 id);
	void getObject(int16 id);
	void putObject();
	void newObject(int16 id, int16 arg2);
	void giveobjectal(int16 id);
	void giveObject();
	void actionTakeObject();
	void newMushroom();
	void newEmptyNest();
	void newNestWithEggs();
	void newGold();
	void gotoPanel();
	void noclicpanel();
	void generique();
	void cancel2();
	void testvoice();
	void load();
	void initafterload();
	void save();
	void desktopcolors();
	void panelrestart();
	void reallyquit();
	void confirmer(char mode, char yesId);
	void confirmYes();
	void confirmNo();
	void restart();
	void edenQuit();
	void choseSubtitleOption();
	void changeVolume();
	void changervol();
	void newvol(byte *volptr, int16 delta);
	void playtape();
	void rewindtape();
	void moveTapeCursor();
	void displayTapeCursor();
	void forwardTape();
	void stopTape();
	void clickTapeCursor();
	void displayPanel();
	void displayLanguage();
	void displayVolCursor(int16 x, int16 vol1, int16 vol2);
	void displayCursors();
	void selectCursor(int itemId);
	void displayTopPanel();
	void displayResult();
	void restrictCursorArea(int16 xmin, int16 xmax, int16 ymin, int16 ymax);
	void edenShudown();
	void habitants(perso_t *perso);
	void suiveurs(perso_t *perso);
	void evenements(perso_t *perso);
	void followme(perso_t *perso);
	void rangermammi(perso_t *perso, Room *room);
	void perso_ici(int16 action);
	void setCharacterHere();
	void faire_suivre(int16 roomNum);
	void AddCharacterToParty();
	void addToParty(int16 index);
	void removeCharacterFromParty();
	void removeFromParty(int16 index);
	void handleEloiDeparture();
	bool checkEloiReturn();
	void handleEloiReturn();
	void incPhase();
	void phase113();
	void phase130();
	void phase161();
	void phase226();
	void phase257();
	void phase353();
	void phase369();
	void phase371();
	void phase385();
	void phase418();
	void phase433();
	void phase434();
	void phase513();
	void phase514();
	void phase529();
	void phase545();
	void phase561();
	void bigphase1();
	void bigphase();
	void phase16();
	void phase32();
	void phase48();
	void phase64();
	void phase80();
	void phase96();
	void phase112();
	void phase128();
	void phase144();
	void phase160();
	void phase176();
	void phase192();
	void phase208();
	void phase224();
	void phase240();
	void phase256();
	void phase272();
	void phase288();
	void phase304();
	void phase320();
	void phase336();
	void phase352();
	void phase368();
	void phase384();
	void phase400();
	void phase416();
	void phase432();
	void phase448();
	void phase464();
	void phase480();
	void phase496();
	void phase512();
	void phase528();
	void phase544();
	void phase560();
	void saveGame(char *name);
	void loadrestart();
	void loadgame(char *name);
	void syncGame(Common::Serializer s);
	void syncGlobalPointers(Common::Serializer s);
	void syncGlobalValues(Common::Serializer s);
	void syncCitadelRoomPointers(Common::Serializer s);
	void syncTapePointers(Common::Serializer s);
	char testCondition(int16 index);
	uint16 operAdd(uint16 v1, uint16 v2);
	uint16 operSub(uint16 v1, uint16 v2);
	uint16 operLogicalAnd(uint16 v1, uint16 v2);
	uint16 operLogicalOr(uint16 v1, uint16 v2);
	uint16 operIsEqual(uint16 v1, uint16 v2);
	uint16 operIsSmaller(uint16 v1, uint16 v2);
	uint16 operIsGreater(uint16 v1, uint16 v2);
	uint16 operIsDifferent(uint16 v1, uint16 v2);
	uint16 operIsSmallerOrEqual(uint16 v1, uint16 v2);
	uint16 operIsGreaterOrEqual(uint16 v1, uint16 v2);
	uint16 operFalse(uint16 v1, uint16 v2);
	uint16 operation(byte op, uint16 v1, uint16 v2);
	uint16 fetchValue();
	uint8 getByteVar(uint16 offset);
	uint16 getWordVar(uint16 offset);
	void actionNop();
	void initSinCosTable();
	void makeMatriceFix();
	void projectionFix(Cube *cube, int n);
	void initCubeMac();
	void engineMac();
	void displayObject(Cube *cube);
	void loadMap(int file_id, byte *buffer);
	void NEWcharge_objet_mob(Cube *cube, int fileNum, byte *texturePtr);
	static int nextVal(char **ptr, char *error);
	void selectMap(int16 num);
	void Eden_dep_and_rot();
	void restoreZDEP();
	void displayPolygoneMapping(Cube *cube, CubeFace *face);
	void drawMappingLine(int16 r3, int16 r4, int16 r5, int16 r6, int16 r7, int16 r8, int16 r9, int16 r10, int16 *lines);
	void displayMappingLine(int16 r3, int16 r4, byte *target, byte *texture);
	void LostEdenMac_InitPrefs();

	void initCubePC();
	void enginePC();
	void selectPCMap(int16 num);

	void makeTables();
	void getSinCosTables(unsigned short angle, signed char **cos_table, signed char **sin_table);
	void rotatePoint(XYZ *point, XYZ *rpoint);
	void mapPoint(XYZ *point, short *x, short *y);
	short calcFaceArea(XYZ *face);
	void paintPixel(XYZ *point, unsigned char pixel);
	void paintFace0(XYZ *point);
	void paintFace1(XYZ *point);
	void paintFace2(XYZ *point);
	void paintFace3(XYZ *point);
	void paintFace4(XYZ *point);
	void paintFace5(XYZ *point);
	void paintFaces();
	void renderCube();

	void incAngleX(int step);
	void decAngleX();
	void incAngleY(int step);
	void decAngleY();
	void incZoom();
	void decZoom();

	CubeCursor *_pcCursor;

	int16 tab1[30];
	int16 tab2[30];
	int8 tab3[36][71];
	int16 _angleX, _angleY, _angleZ, _zoomZ, _zoomZStep;

	int8 *_cosX, *_sinX;
	int8 *_cosY, *_sinY;
	int8 *_cosZ, *_sinZ;

	uint8 *_face[6], *_newface[6];
	int16 _faceSkip;

	uint8 _cursor[40 * 40];
	uint8 *_cursorCenter;

	byte  _ownObjects[128];

private:
	int16 _scrollPos;
	int16 _oldScrollPos;
	bool  _frescoTalk;
	byte  _oldPix[8];
	Common::Point _adamMapMarkPos;
	byte  _cursKeepBuf[2500];
	bool  _torchCursor;
	int16 _curBankNum;
	bool  _paletteUpdateRequired;
	bool  _cursorSaved;
	bool  _backgroundSaved;
	byte *_bankData;
	perso_t *_tyranPtr;
	int   _lastAnimFrameNumb;
	int   _curAnimFrameNumb;
	int   _lastAnimTicks;
	int16 _numAnimFrames;
	int16 _maxPersoDesc;
	int16 _numImgDesc;
	bool  _restartAnimation;
	bool  _animationActive;
	byte  _animationDelay;
	byte  _animationIndex;
	byte  _lastAnimationIndex;

	byte *dword_30724;
	byte *dword_30728;   //TODO: rename - something amim-related
	byte *_mouthAnimations;
	byte *_animationTable;
	byte  _imageDesc[512];
	byte *_characterBankData;
	int16 _numTextLines;
	byte  _sentenceBuffer[400];
	byte   phraseIconsBuffer[10];
	byte   _sentenceCoordsBuffer[22];
	byte   *_textOutPtr;
	byte   *textout;
	object_t        *_curSpecialObject;
	bool  _lastDialogChoice;
	bool parlemoiNormalFlag;

	bool _closeCharacterDialog;
	int             dword_30B04;

	char            _lastPhrasesFile;
	byte _dialogSkipFlags;

	byte *_voiceSamplesBuffer;    //TODO: sound sample buffer
	Common::File _bigfile;
	byte  _infoList[16];
	byte *_mainBankBuf;
	byte *_musicBuf;
	byte *_gameLipsync;
	byte *_gamePhrases;
	byte *_gameDialogs;   //TODO: rename to dialogs?
	byte *_gameConditions;
	byte *_placeRawBuf;   //TODO: fixme
	byte *_bankDataBuf;
	Icon *_gameIcons;
	Room *_gameRooms;
	PakHeaderNode *_bigfileHeader;
	byte *_glowBuffer;

	byte *_gameFont;  //TODO: rename to font?

	uint16 _mouseCenterX;
	uint16 _mouseCenterY;
	bool _bufferAllocationErrorFl;
	bool _quitFlag2;
	bool _quitFlag3;
	bool _gameStarted;
	bool _soundAllocated;

	CSoundChannel  *_musicChannel;
	CSoundChannel  *_voiceChannel;
	CSoundChannel *_hnmSoundChannel;

	int   _demoCurrentTicks;
	int   _demoStartTicks;
	int   _currentTime;
	int16 _cirsorPanX;
	int16 _inventoryScrollDelay;
	int16 _cursorPosX;
	int16 _cursorPosY;
	int16 _currCursor;
	Icon *_currSpot;
	Icon *_curSpot2;
	bool  _mouseHeld;
	bool  _normalCursor;

	bool  _specialTextMode;
	int   _voiceSamplesSize;   //TODO: perso vox sample data len
	int16 _musicRightVol;
	int16 _musicLeftVol;

	bool  _animateTalking;
	bool  _personTalking;
	byte  _musicFadeFlag;

	char  _musicSequencePos;
	bool  _musicPlayingFlag;

	byte *_musicSamplesPtr;
	byte *_musicPatternsPtr;  //TODO: sndblock_t ?
	byte *_musSequencePtr;
	bool  _musicEnabledFlag;
	uint16 *_currentObjectLocation;
	bool  byte_31D64;

	bool  _noPalette;
	bool  _gameLoaded;
#define MAX_TAPES 16
	tape_t _tapes[MAX_TAPES];
	byte   _confirmMode;
	byte  *_curSliderValuePtr;
	byte   _lastMenuItemIdLo;
	int16  _lastTapeRoomNum;
	int16  _curSliderX;
	int16  _curSliderY;
	int16  _destinationRoom;
	int16  word_31E7A; // CHECKME: Unused?

	int16  word_378CC; // TODO: set by CLComputer_Init to 0
	int16  word_378CE; // CHECKME: Unused

	int	   _invIconsCount;
	int	   _invIconsBase;
	int	   _roomIconsBase;

	//// cube.c
	int16 _cosTable[361];
	int16 _sinTable[361];
	int _passMat31, _passMat21, _passMat11;
	int _passMat32, _passMat22, _passMat12;
	int _passMat33, _passMat23, _passMat13;
	int16 _rotationAngleY; // CHECKME: USeless?
	int16 _rotationAngleX, _rotationAngleZ;
	float _translationY, _translationX; // CHECKME: Useless?
	Cube _cube;
	int16 _cursCurPCMap;
	int16 _lines[200 * 8];
	byte  _cubeTexture[0x4000];
	int   _cubeFaces;
	uint32 _cursorOldTick, _cursorNewTick;
	byte *_codePtr;

	uint8 tab_2CB1E[8][4];

	const unsigned int kMaxMusicSize;  // largest .mus file size 

	// Loaded from cryo.dat
	Follower _followerList[15];
	byte _labyrinthPath[70];
	char _dinoSpeedForCitadelLevel[16];
	char _tabletView[12];
	char _personRoomBankTable[84];	// special character backgrounds for specific rooms

	// Loaded from cryo.dat - Area transition descriptors
	Goto _gotos[130];
	object_t _objects[42];
	uint16 _objectLocations[45];
	perso_t _persons[58];
	Citadel _citadelList[7];

	// Loaded from cryo.dat
	Common::Rect _characterRects[19];
	byte _characterArray[20][5];
	Area _areasTable[12];
	int16 tab_2CEF0[64];
	int16 tab_2CF70[64];
	byte _actionCursors[299];
	byte _mapMode[12];
	byte _cubeTextureCoords[3][6 * 2 * 3 * 2];

	int32 _translationZ;
	int8 _zDirection;	// 1 (up) or -1 (down)

	// Torch/glow related
	int16 _torchTick;
	int16 _glowIndex;
	int16 _torchCurIndex;

	int _cursCenter;
};

}

#endif
