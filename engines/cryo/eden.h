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

#include "cryo/sound.h"
#include "cryo/defs.h"

enum Direction {
	kCryoNorth = 0,
	kCryoEast = 1,
	kCryoSouth = 2,
	kCryoWest = 3
};

#define EDEN_DEBUG

namespace Cryo {

class CryoEngine;

class EdenGame {
private:
	CryoEngine *_vm;

public:
	void run();

	EdenGame(CryoEngine *vm);

private:
	void removeConsole();
	void scroll();
	void resetScroll();
	void scrollFrescoes();
	void displayFrescoes();
	void gametofresques();
	void doFrescoes();
	void endFrescoes();
	void scrollMirror();
	void scrollpano();
	void displayFollower(Follower *follower, int16 x, int16 y);
	void characterInMirror();
	void gametomiroir(byte arg1);
	void flipMode();
	void quitMirror();
	void clictimbre();
	void clicplanval();
	void gotolieu(Goto *go);
	void deplaval(uint16 roomNum);
	void move(Direction dir);
	void move2(Direction dir);
	void dinosoufle();
	void plaquemonk();
	void fresquesgraa();
	void fresqueslasc();
	void pushpierre();
	void tetemomie();
	void tetesquel();
	void squelmoorkong();
	void choisir();
	void handleDinaDialog();
	void handleKingDialog();
	void kingDialog1();
	void kingDialog2();
	void kingDialog3();
	void getKnife();
	void getPrism();
	void getMushroom();
	void getBadMushroom();
	void getGold();
	void getFullNest();
	void getNest();
	void getHorn();
	void getSunStone();
	void getEgg();
	void getTablet();
	void voirlac();
	void gotohall();
	void demitourlabi();
	void gotonido();
	void gotoval();
	void visiter();
	void final();
	void moveNorth();
	void moveEast();
	void moveSouth();
	void moveWest();
	void afficher();
	void afficher128();
	void saveFriezes();
	void saveTopFrieze(int16 x);
	void saveBottomFrieze();
	void restoreFriezes();
	void restoreTopFrieze();
	void restoreBottomFrieze();
	void useMainBank();
	void useCharacterBank();
	void useBank(int16 bank);
	void sundcurs(int16 x, int16 y);
	void rundcurs();
	void noclipax(int16 index, int16 x, int16 y);
	void noclipax_avecnoir(int16 index, int16 x, int16 y);
	void getglow(int16 x, int16 y, int16 w, int16 h);
	void unglow();
	void glow(int16 index);
	void readPalette(byte *ptr);
	void spriteOnSubtitle(int16 index, int16 x, int16 y);
	void bars_out();
	void showBars();
	void sauvefondbouche();
	void restaurefondbouche();
	void drawBlackBars();
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
	void melangedir();
	bool naitredino(char persoType);
	void newCitadel(char area, int16 level, Room *room);
	void citaevol(int16 level);
	void destroyCitadelRoom(int16 roomNum);
	void buildCitadel();
	void citatombe(char level);
	void constcita();
	void moveDino(perso_t *perso);
	void moveAllDino();
	void newValley();
	char whereiscita();
	bool iscita(int16 loc);
	void lieuvava(Area *area);
	void vivredino();
	void vivreval(int16 areaNum);
	void chaquejour();
	void temps_passe(int16 t);
	void heurepasse();
	void anim_perso();
	void getanimrnd();
	void addanim();
	void removeMouthSprite();
	void anim_perfin();
	void perso_spr(byte *spr);
	void displayImage();
	void af_perso1();
	void af_perso();
	void ef_perso();
	void loadCharacter(perso_t *perso);
	void load_perso_cour();
	void fin_perso();
	void no_perso();
	void close_perso();
	void displayBackgroundFollower();
	void af_fondperso1();
	void af_fondperso();
	void setCharacterIcon();
	void showCharacter();
	void displayCharacterPanel();
	void getDataSync();
	int16 ReadNombreFrames();
	void waitEndSpeak();
	void my_bulle();
	void my_pr_bulle();
	void charsurbulle(byte c, byte color, int16 width);
	void displaySubtitles();
	void savePhylacteryBackground(int16 y);
	void restorePhylacteryBackground();
	void af_subtitlehnm();
	void patchPhrase();
	void vavapers();
	void citadelle();
	void choixzone();
	void showevents1();
	void showEvents();
	void parle_mfin();
	void parlemoi_normal();
	void parle_moi();
	void init_perso_ptr(perso_t *perso);
	void perso1(perso_t *perso);
	void perso_normal(perso_t *perso);
	void handleCharacterDialog(int16 pers);
	void roi();
	void dina();
	void thoo();
	void monk();
	void bourreau();
	void messager();
	void mango();
	void eve();
	void azia();
	void mammi();
	void gardes();
	void bambou();
	void kabuka();
	void fisher();
	void dino();
	void tyran();
	void morkus();
	void comment();
	void adam();
	void setChoiceYes();
	void setChoiceNo();
	bool isAnswerYes();
	void specialMushroom(perso_t *perso);
	void specialNidv(perso_t *perso);
	void specialNido(perso_t *perso);
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
	void abortdial();
	void narrateur();
	void vrf_phrases_file();
	byte *gettxtad(int16 id);
	void gotocarte();
	void record();
	bool dial_scan(dial_t *dial);
	bool dialoscansvmas(dial_t *dial);
	bool dialo_even(perso_t *perso);
	void stay_here();
	void mort(int16 vid);
	void evenchrono();
	void setChrono(int16 t);
	void prechargephrases(int16 vid);
	void effet1();
	void effet2();
	void effet3();
	void effet4();
	void ClearScreen();
	void colimacon(int16 pattern[16]);
	void fadeToBlack(int delay);
	void fadetoblack128(int delay);
	void fadefromblack128(int delay);
	void rectanglenoir32();
	void setRS1(int16 sx, int16 sy, int16 ex, int16 ey);
	void setRD1(int16 sx, int16 sy, int16 ex, int16 ey);
	void wait(int howlong);
	void effetpix();
	void verifh(void *ptr);
	void openbigfile();
	void closebigfile();
	void loadRawFile(uint16 num, byte *buffer);
	void loadIconFile(uint16 num, Icon *buffer);
	void loadRoomFile(uint16 num, Room *buffer);
	void shnmfl(uint16 num);
	int ssndfl(uint16 num);
	void ConvertMacToPC();
	void loadpermfiles();
	bool ReadDataSyncVOC(unsigned int num);
	bool ReadDataSync(uint16 num);
	void loadpartoffile(uint16 num, void *buffer, int32 pos, int32 len);
	void Expand_hsq(void *input, void *output);
	void addInfo(byte info);
	void unlockInfo();
	void nextInfo();
	void removeInfo(byte info);
	void updateInfoList();
	void init_globals();
	void initRects();
	void closesalle();
	void displaySingleRoom(Room *room);
	void displayRoom();
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
	void openWindow();
	void EmergencyExit();
	void edmain();
	void intro();
	void entergame();
	void signon(const char *s);
	void testPommeQ();
	void FRDevents();
	Icon *scan_icon_list(int16 x, int16 y, int16 index);
	void update_cursor();
	void mouse();
	void showMovie(char arg1);
	void playHNM(int16 num);
	void displayHNMSubtitles();
	void musique();
	void startmusique(byte num);
	void musicspy();
	int loadmusicfile(int16 num);
	void persovox();
	void endpersovox();
	void fademusicup();
	void fademusica0(int16 delay);
	object_t *getObjectPtr(int16 id);
	void countObjects();
	void showObjects();
	void winObject(int16 id);
	void loseObject(int16 id);
	void lostObject();
	bool objecthere(int16 id);
	void objectmain(int16 id);
	void getObject(int16 id);
	void putObject();
	void newObject(int16 id, int16 arg2);
	void giveobjectal(int16 id);
	void giveObject();
	void takeObject();
	void newMushroom();
	void newnidv();
	void newnido();
	void newor();
	void gotopanel();
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
	void confirmyes();
	void confirmno();
	void restart();
	void edenQuit();
	void choseSubtitleOption();
	void changeVolume();
	void changervol();
	void newvol(byte *volptr, int16 delta);
	void playtape();
	void rewindtape();
	void depcurstape();
	void affcurstape();
	void forwardtape();
	void stoptape();
	void cliccurstape();
	void paneltobuf();
	void cursbuftopanel();
	void langbuftopanel();
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
	void setpersohere();
	void faire_suivre(int16 roomNum);
	void AddCharacterToParty();
	void AddToParty(int16 index);
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
	void savegame(char *name);
	void loadrestart();
	void loadgame(char *name);
	void vavaoffsetout();
	void vavaoffsetin();
	void lieuoffsetout();
	void lieuoffsetin();
	void bandeoffsetout();
	void bandeoffsetin();
	char testcondition(int16 index);
	uint16 opera_add(uint16 v1, uint16 v2);
	uint16 opera_sub(uint16 v1, uint16 v2);
	uint16 opera_and(uint16 v1, uint16 v2);
	uint16 opera_or(uint16 v1, uint16 v2);
	uint16 opera_egal(uint16 v1, uint16 v2);
	uint16 opera_petit(uint16 v1, uint16 v2);
	uint16 opera_grand(uint16 v1, uint16 v2);
	uint16 opera_diff(uint16 v1, uint16 v2);
	uint16 opera_petega(uint16 v1, uint16 v2);
	uint16 opera_graega(uint16 v1, uint16 v2);
	uint16 opera_faux(uint16 v1, uint16 v2);
	uint16 operation(byte op, uint16 v1, uint16 v2);
	uint16 cher_valeur();
	void ret();
	void make_tabcos();
	void make_matrice_fix();
	void projection_fix(cube_t *cube, int n);
	void initCubeMac();
	void moteur();
	void affiche_objet(cube_t *cube);
	void NEWcharge_map(int file_id, byte *buffer);
	void NEWcharge_objet_mob(cube_t *cube, int file_id, byte *texptr);
	static int next_val(char **ptr, char *error);
	void selectmap(int16 num);
	void Eden_dep_and_rot();
	void restoreZDEP();
	void affiche_polygone_mapping(cube_t *cube, cubeface_t *face);
	void trace_ligne_mapping(int16 r3, int16 r4, int16 r5, int16 r6, int16 r7, int16 r8, int16 r9, int16 r10, int16 *lines);
	void affiche_ligne_mapping(int16 r3, int16 r4, byte *target, byte *texture);
	int16 OpenDialog(void *arg1, void *arg2);
	void LostEdenMac_InitPrefs();

	void initCubePC();
	void pc_moteur();
	void pc_selectmap(int16 num);

	void MakeTables();
	void GetSinCosTables(unsigned short angle, signed char **cos_table, signed char **sin_table);
	void RotatePoint(XYZ *point, XYZ *rpoint);
	void MapPoint(XYZ *point, short *x, short *y);
	short CalcFaceArea(XYZ *face);
	void PaintPixel(XYZ *point, unsigned char pixel);
	void PaintFace0(XYZ *point);
	void PaintFace1(XYZ *point);
	void PaintFace2(XYZ *point);
	void PaintFace3(XYZ *point);
	void PaintFace4(XYZ *point);
	void PaintFace5(XYZ *point);
	void PaintFaces();
	void RenderCube();

	void incAngleX(int step);
	void decAngleX();
	void incAngleY(int step);
	void decAngleY();
	void incZoom();
	void decZoom();

	cubeCursor *pc_cursor;

	signed short tab1[30];
	signed short tab2[30];
	signed char tab3[36][71];
	short angle_x, angle_y, angle_z, zoom, zoom_step;

	signed char *cos_x, *sin_x;
	signed char *cos_y, *sin_y;
	signed char *cos_z, *sin_z;

	unsigned char *_face[6], *_newface[6];
	short faceskip;

	unsigned char cursor[40 * 40];
	unsigned char *cursorcenter;


private:
	int16  _scrollPos;
	int16  _oldScrollPos;
	bool   _frescoTalk;
	byte   _oldPix[8];
	Common::Point _adamMapMarkPos;
	byte   _cursKeepBuf[2500];
	Common::Point _cursKeepPos;
	bool  _torchCursor;
	int16 _curBankNum;
	int16 _glowX;
	int16 _glowY;
	int16 _glowW;
	int16 _glowH;
	bool _paletteUpdateRequired;
	bool _cursorSaved;
	bool _showBlackBars;
	bool _backgroundSaved;
	byte *_bankData;
	color3_t        pal_entry;
	color_t         global_palette[256];    //TODO palette_t
	perso_t *_tyranPtr;
	int     _lastAnimFrameNumb;
	int     _curAnimFrameNumb;
	int     _lastAnimTicks;
	prect_t *_curPersoRect;
	int16 _numAnimFrames;
	int16 _maxPersoDesc;
	int16 _numImgDesc;
	bool _restartAnimation;
	bool _animationActive;
	byte _animationDelay;
	byte _animationIndex;
	byte _lastAnimationIndex;

	byte   *dword_30724;
	byte   *dword_30728;   //TODO: rename - something amim-related
	byte   *_mouthAnimations;
	byte   *animationTable;
	byte   _imageDesc[512];
	byte   *_characterBankData;
	bool _savedUnderSubtitles;
	int16           num_text_lines;
	byte   _sentenceBuffer[400];
	byte   *text_ptr;
	byte   phraseIconsBuffer[10];
	byte   phraseCoordsBuffer[22];
	byte   *textoutptr;
	byte   *textout;
	object_t        *_curSpecialObject;
	bool  _lastDialogChoice;
	bool parlemoiNormalFlag;

	bool _closeCharacterDialog;
	int             dword_30B04;

	char            lastPhrasesFile;
	byte dialogSkipFlags;

	color3_t        newColor;
	color_t         oldPalette[256];    // TODO palette_t ?
	color_t         newPalette[256];
	Common::Rect          rect_dst, rect_src;
	void            *voiceSamplesBuffer;    //TODO: sound sample buffer
	Common::File h_bigfile;
	byte   _infoList[16];
	bool needToFade;
	byte   lastMusicNum;
	byte   *_mainBankBuf;
	byte   *_musicBuf;
	byte   *gameLipsync;
	byte   *gamePhrases;
	byte   *gameDialogs;   //TODO: rename to dialogs?
	byte   *gameConditions;
	byte *_placeRawBuf;   //TODO: fixme
	byte *_bankDataBuf;
	Icon *_gameIcons;
	Room *_gameRooms;
	PakHeaderNode *bigfile_header;
	byte *_glowBuffer;
	byte *p_mainview_buf;
	byte *p_view2_buf;
	byte *_gameFont;  //TODO: rename to font?
	byte *p_subtitlesview_buf;
	byte *p_underSubtitlesView_buf;
	global_t *p_global;
	uint16 _mouseCenterX;
	uint16 _mouseCenterY;
	bool bufferAllocationErrorFl;
	bool _quitFlag2;
	bool _quitFlag3;
	bool _gameStarted;
	bool _soundAllocated;

	CSoundChannel  *_musicChannel;
	CSoundChannel  *_voiceChannel;
	soundchannel_t *_hnmSoundChannel;
	sound_t        *_voiceSound;

	View  *p_view2;
	View  *p_underSubtitlesView;
	View  *p_subtitlesview;
	View  *p_underBarsView;
	View  *_mainView;
	View  *_hnmView;
	hnm_t *_hnmContext;
	Common::Rect _underSubtitlesBackupRect;
	Common::Rect _underSubtitlesScreenRect;
	Common::Rect _underBottomBarBackupRect;
	Common::Rect _underBottomBarScreenRect;
	Common::Rect _underTopBarBackupRect;
	Common::Rect _underTopBarScreenRect;
	int   _demoCurrentTicks;
	int   _demoStartTicks;
	int   _currentTime;
	bool  _doubledScreen;
	int16 _cirsorPanX;
	int16 _inventoryScrollDelay;
	int16 _cursorPosX;
	int16 _cursorPosY;
	int16 _currCursor;
	Icon *_currSpot;
	Icon *_curSpot2;
	bool pomme_q;
	bool _keyboardHeld;
	bool _mouseHeld;
	bool _normalCursor;
	byte *_hnmViewBuf;
	bool _showVideoSubtitle;
	bool _videoCanceledFlag;  //TODO: hnm_canceled
	bool _specialTextMode;
	int  _hnmFrameNum;
	int  _voiceSamplesSize;   //TODO: perso vox sample data len
	int16 _musicRightVol;
	int16 _musicLeftVol;


	bool _animateTalking;
	bool _personTalking;
	byte _musicFadeFlag;

	char _musicSequencePos;
	bool _musicPlayingFlag;

	byte *_musicSamplesPtr;
	byte *_musicPatternsPtr;  //TODO: sndblock_t ?
	byte *_musSequencePtr;
	bool _musicEnabledFlag;
	uint16 *pCurrentObjectLocation;
	byte   own_objects[128];
	bool   byte_31D64;

	bool _noPalette;
	bool _gameLoaded;
#define MAX_TAPES 16
	tape_t tapes[MAX_TAPES];
	byte   confirmMode;
	byte   *cur_slider_value_ptr;
	byte   _lastMenuItemIdLo;
	int16  _lastTapeRoomNum;
	int16  _curSliderX;
	int16           cur_slider_y;
	int16           destinationRoom;
	int16           word_31E7A;

	int16           word_378CC; // TODO: set by CLComputer_Init to 0
	int16           word_378CE; // CHECKME: Unused

	int		_invIconsCount;
	int		_invIconsBase;
	int		_roomIconsBase;

	//// cube.c
	int16 tabcos[361 * 2];
	int dword_32424, dword_32428, dword_3242C;
	int dword_32430, dword_32434, dword_32438;
	int dword_3243C, dword_32440, dword_32444;
	int16 word_32448;
	int16 word_3244A, word_3244C;
	float flt_32450, flt_32454;
	cube_t cube;
	int16 curs_cur_map;
	int16 _lines[200 * 8];
	byte cube_texture[0x4000];
	int cube_faces;
	int32 curs_old_tick, curs_new_tick;


};

}

#endif
