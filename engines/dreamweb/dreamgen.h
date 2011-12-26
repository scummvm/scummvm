#ifndef TASMRECOVER_DREAMGEN_STUBS_H__
#define TASMRECOVER_DREAMGEN_STUBS_H__

/* PLEASE DO NOT MODIFY THIS FILE. ALL CHANGES WILL BE LOST! LOOK FOR README FOR DETAILS */

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
 */

#include "dreamweb/runtime.h"

#include "dreamweb/structs.h"
#include "dreamweb/dreambase.h"

namespace DreamGen {

static const uint16 offset_rootdir = 0x01a6;
static const uint16 offset_operand1 = 0x0198;
static const uint16 kStartvars = 0;
static const uint16 kProgresspoints = 1;
static const uint16 kWatchon = 2;
static const uint16 kShadeson = 3;
static const uint16 kSecondcount = 4;
static const uint16 kMinutecount = 5;
static const uint16 kHourcount = 6;
static const uint16 kZoomon = 7;
static const uint16 kLocation = 8;
static const uint16 kExpos = 9;
static const uint16 kExframepos = 10;
static const uint16 kExtextpos = 12;
static const uint16 kCard1money = 14;
static const uint16 kListpos = 16;
static const uint16 kRyanpage = 18;
static const uint16 kWatchingtime = 19;
static const uint16 kReeltowatch = 21;
static const uint16 kEndwatchreel = 23;
static const uint16 kSpeedcount = 25;
static const uint16 kWatchspeed = 26;
static const uint16 kReeltohold = 27;
static const uint16 kEndofholdreel = 29;
static const uint16 kWatchmode = 31;
static const uint16 kDestafterhold = 32;
static const uint16 kNewsitem = 33;
static const uint16 kLiftflag = 34;
static const uint16 kLiftpath = 35;
static const uint16 kLockstatus = 36;
static const uint16 kDoorpath = 37;
static const uint16 kCounttoopen = 38;
static const uint16 kCounttoclose = 39;
static const uint16 kRockstardead = 40;
static const uint16 kGeneraldead = 41;
static const uint16 kSartaindead = 42;
static const uint16 kAidedead = 43;
static const uint16 kBeenmugged = 44;
static const uint16 kGunpassflag = 45;
static const uint16 kCanmovealtar = 46;
static const uint16 kTalkedtoattendant = 47;
static const uint16 kTalkedtosparky = 48;
static const uint16 kTalkedtoboss = 49;
static const uint16 kTalkedtorecep = 50;
static const uint16 kCardpassflag = 51;
static const uint16 kMadmanflag = 52;
static const uint16 kKeeperflag = 53;
static const uint16 kLasttrigger = 54;
static const uint16 kMandead = 55;
static const uint16 kSeed = 56;
static const uint16 kNeedtotravel = 59;
static const uint16 kThroughdoor = 60;
static const uint16 kNewobs = 61;
static const uint16 kRyanon = 62;
static const uint16 kCombatcount = 63;
static const uint16 kLastweapon = 64;
static const uint16 kDreamnumber = 65;
static const uint16 kRoomafterdream = 66;
static const uint16 kShakecounter = 67;
static const uint16 kSpeechcount = 68;
static const uint16 kCharshift = 69;
static const uint16 kKerning = 71;
static const uint16 kBrightness = 72;
static const uint16 kRoomloaded = 73;
static const uint16 kDidzoom = 74;
static const uint16 kLinespacing = 75;
static const uint16 kTextaddressx = 77;
static const uint16 kTextaddressy = 79;
static const uint16 kTextlen = 81;
static const uint16 kLastxpos = 82;
static const uint16 kIcontop = 84;
static const uint16 kIconleft = 86;
static const uint16 kItemframe = 88;
static const uint16 kItemtotran = 89;
static const uint16 kRoomad = 90;
static const uint16 kOldsubject = 92;
static const uint16 kWithobject = 94;
static const uint16 kWithtype = 95;
static const uint16 kLookcounter = 96;
static const uint16 kCommand = 98;
static const uint16 kCommandtype = 99;
static const uint16 kOldcommandtype = 100;
static const uint16 kObjecttype = 101;
static const uint16 kGetback = 102;
static const uint16 kInvopen = 103;
static const uint16 kMainmode = 104;
static const uint16 kPickup = 105;
static const uint16 kLastinvpos = 106;
static const uint16 kExamagain = 107;
static const uint16 kNewtextline = 108;
static const uint16 kOpenedob = 109;
static const uint16 kOpenedtype = 110;
static const uint16 kOldmapadx = 111;
static const uint16 kOldmapady = 113;
static const uint16 kMapadx = 115;
static const uint16 kMapady = 117;
static const uint16 kMapoffsetx = 119;
static const uint16 kMapoffsety = 121;
static const uint16 kMapxstart = 123;
static const uint16 kMapystart = 125;
static const uint16 kMapxsize = 127;
static const uint16 kMapysize = 128;
static const uint16 kHavedoneobs = 129;
static const uint16 kManisoffscreen = 130;
static const uint16 kRainspace = 131;
static const uint16 kFacing = 132;
static const uint16 kLeavedirection = 133;
static const uint16 kTurntoface = 134;
static const uint16 kTurndirection = 135;
static const uint16 kMaintimer = 136;
static const uint16 kIntrocount = 138;
static const uint16 kArrowad = 139;
static const uint16 kCurrentkey = 141;
static const uint16 kOldkey = 142;
static const uint16 kUseddirection = 143;
static const uint16 kTimercount = 144;
static const uint16 kOldtimercount = 145;
static const uint16 kMapx = 146;
static const uint16 kMapy = 147;
static const uint16 kNewscreen = 148;
static const uint16 kRyanx = 149;
static const uint16 kRyany = 150;
static const uint16 kLastflag = 151;
static const uint16 kOffsetx = 152;
static const uint16 kOffsety = 154;
static const uint16 kCurrentob = 156;
static const uint16 kDestpos = 157;
static const uint16 kReallocation = 158;
static const uint16 kRoomnum = 159;
static const uint16 kNowinnewroom = 160;
static const uint16 kResetmanxy = 161;
static const uint16 kNewlocation = 162;
static const uint16 kAutolocation = 163;
static const uint16 kDoorcheck1 = 164;
static const uint16 kDoorcheck2 = 165;
static const uint16 kDoorcheck3 = 166;
static const uint16 kDoorcheck4 = 167;
static const uint16 kMousex = 168;
static const uint16 kMousey = 170;
static const uint16 kMousebutton = 172;
static const uint16 kMousebutton1 = 174;
static const uint16 kMousebutton2 = 176;
static const uint16 kMousebutton3 = 178;
static const uint16 kMousebutton4 = 180;
static const uint16 kOldbutton = 182;
static const uint16 kOldx = 184;
static const uint16 kOldy = 186;
static const uint16 kLastbutton = 188;
static const uint16 kOldpointerx = 190;
static const uint16 kOldpointery = 192;
static const uint16 kDelherex = 194;
static const uint16 kDelherey = 196;
static const uint16 kPointerxs = 198;
static const uint16 kPointerys = 199;
static const uint16 kDelxs = 200;
static const uint16 kDelys = 201;
static const uint16 kPointerframe = 202;
static const uint16 kPointerpower = 203;
static const uint16 kAuxpointerframe = 204;
static const uint16 kPointermode = 205;
static const uint16 kPointerspeed = 206;
static const uint16 kPointercount = 207;
static const uint16 kInmaparea = 208;
static const uint16 kTalkmode = 209;
static const uint16 kTalkpos = 210;
static const uint16 kCharacter = 211;
static const uint16 kWatchdump = 212;
static const uint16 kLogonum = 213;
static const uint16 kOldlogonum = 214;
static const uint16 kNetseg = 215;
static const uint16 kNetpoint = 217;
static const uint16 kCursorstate = 219;
static const uint16 kPressed = 220;
static const uint16 kPresspointer = 221;
static const uint16 kGraphicpress = 223;
static const uint16 kPresscount = 224;
static const uint16 kLightcount = 225;
static const uint16 kFolderpage = 226;
static const uint16 kDiarypage = 227;
static const uint16 kMenucount = 228;
static const uint16 kSymboltopx = 229;
static const uint16 kSymboltopnum = 230;
static const uint16 kSymboltopdir = 231;
static const uint16 kSymbolbotx = 232;
static const uint16 kSymbolbotnum = 233;
static const uint16 kSymbolbotdir = 234;
static const uint16 kSymboltolight = 235;
static const uint16 kSymbol1 = 236;
static const uint16 kSymbol2 = 237;
static const uint16 kSymbol3 = 238;
static const uint16 kSymbolnum = 239;
static const uint16 kDumpx = 240;
static const uint16 kDumpy = 242;
static const uint16 kWalkandexam = 244;
static const uint16 kWalkexamtype = 245;
static const uint16 kWalkexamnum = 246;
static const uint16 kCurslocx = 247;
static const uint16 kCurslocy = 249;
static const uint16 kCurpos = 251;
static const uint16 kMonadx = 253;
static const uint16 kMonady = 255;
static const uint16 kMonsource = 257;
static const uint16 kNumtodo = 259;
static const uint16 kTimecount = 261;
static const uint16 kCounttotimed = 263;
static const uint16 kTimedseg = 265;
static const uint16 kTimedoffset = 267;
static const uint16 kTimedy = 269;
static const uint16 kTimedx = 270;
static const uint16 kNeedtodumptimed = 271;
static const uint16 kLoadingorsave = 272;
static const uint16 kCurrentslot = 273;
static const uint16 kCursorpos = 274;
static const uint16 kColourpos = 275;
static const uint16 kFadedirection = 276;
static const uint16 kNumtofade = 277;
static const uint16 kFadecount = 278;
static const uint16 kAddtogreen = 279;
static const uint16 kAddtored = 280;
static const uint16 kAddtoblue = 281;
static const uint16 kLastsoundreel = 282;
static const uint16 kSpeechloaded = 284;
static const uint16 kSpeechlength = 285;
static const uint16 kVolume = 287;
static const uint16 kVolumeto = 288;
static const uint16 kVolumedirection = 289;
static const uint16 kVolumecount = 290;
static const uint16 kWongame = 291;
static const uint16 kLasthardkey = 292;
static const uint16 kBufferin = 293;
static const uint16 kBufferout = 295;
static const uint16 kExtras = 297;
static const uint16 kWorkspace = 299;
static const uint16 kMapstore = 301;
static const uint16 kCharset1 = 303;
static const uint16 kBuffers = 305;
static const uint16 kMainsprites = 307;
static const uint16 kBackdrop = 309;
static const uint16 kMapdata = 311;
static const uint16 kSounddata = 313;
static const uint16 kSounddata2 = 315;
static const uint16 kRecordspace = 317;
static const uint16 kFreedat = 319;
static const uint16 kSetdat = 321;
static const uint16 kReel1 = 323;
static const uint16 kReel2 = 325;
static const uint16 kReel3 = 327;
static const uint16 kRoomdesc = 329;
static const uint16 kFreedesc = 331;
static const uint16 kSetdesc = 333;
static const uint16 kBlockdesc = 335;
static const uint16 kSetframes = 337;
static const uint16 kFreeframes = 339;
static const uint16 kPeople = 341;
static const uint16 kReels = 343;
static const uint16 kCommandtext = 345;
static const uint16 kPuzzletext = 347;
static const uint16 kTraveltext = 349;
static const uint16 kTempgraphics = 351;
static const uint16 kTempgraphics2 = 353;
static const uint16 kTempgraphics3 = 355;
static const uint16 kTempsprites = 357;
static const uint16 kTextfile1 = 359;
static const uint16 kTextfile2 = 361;
static const uint16 kTextfile3 = 363;
static const uint16 kBlinkframe = 365;
static const uint16 kBlinkcount = 366;
static const uint16 kReasseschanges = 367;
static const uint16 kPointerspath = 368;
static const uint16 kManspath = 369;
static const uint16 kPointerfirstpath = 370;
static const uint16 kFinaldest = 371;
static const uint16 kDestination = 372;
static const uint16 kLinestartx = 373;
static const uint16 kLinestarty = 375;
static const uint16 kLineendx = 377;
static const uint16 kLineendy = 379;
static const uint16 kLinepointer = 381;
static const uint16 kLinedirection = 382;
static const uint16 kLinelength = 383;
static const uint16 kCh0blockstocopy = 384;
static const uint16 kCh0playing = 386;
static const uint16 kCh0repeat = 387;
static const uint16 kCh1playing = 388;
static const uint16 kCh1blockstocopy = 389;
static const uint16 kSoundbufferwrite = 391;
static const uint16 kCurrentsample = 393;
static const uint16 kRoomssample = 394;
static const uint16 kBasicsample = 395;
static const uint16 kCurrentfile = 436;
static const uint16 kInputline = 511;
static const uint16 kQuitrequested = 639;
static const uint16 kSubtitles = 640;
static const uint16 kForeignrelease = 641;
static const uint16 kBlocktextdat = (0);
static const uint16 kPersonframes = (0);
static const uint16 kDebuglevel1 = (0);
static const uint16 kDebuglevel2 = (0);
static const uint16 kPlayback = (0);
static const uint16 kMap = (0);
static const uint16 kSettextdat = (0);
static const uint16 kSpanish = (0);
static const uint16 kRecording = (0);
static const uint16 kFlags = (0);
static const uint16 kGerman = (0);
static const uint16 kTextunder = (0);
static const uint16 kPathdata = (0);
static const uint16 kDemo = (0);
static const uint16 kFramedata = (0);
static const uint16 kExframedata = (0);
static const uint16 kIntextdat = (0);
static const uint16 kFreetextdat = (0);
static const uint16 kFrframedata = (0);
static const uint16 kSettext = (0+(130*2));
static const uint16 kOpeninvlist = (0+(228*13));
static const uint16 kRyaninvlist = (0+(228*13)+32);
static const uint16 kPointerback = (0+(228*13)+32+60);
static const uint16 kMapflags = (0+(228*13)+32+60+(32*32));
static const uint16 kStartpal = (0+(228*13)+32+60+(32*32)+(11*10*3));
static const uint16 kEndpal = (0+(228*13)+32+60+(32*32)+(11*10*3)+768);
static const uint16 kMaingamepal = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768);
static const uint16 kSpritetable = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768);
static const uint16 kSetlist = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32));
static const uint16 kFreelist = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5));
static const uint16 kExlist = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5));
static const uint16 kPeoplelist = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5));
static const uint16 kZoomspace = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
static const uint16 kPrintedlist = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40));
static const uint16 kListofchanges = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
static const uint16 kUndertimedtext = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4));
static const uint16 kRainlist = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30));
static const uint16 kInitialreelrouts = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64));
static const uint16 kInitialvars = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+457);
static const uint16 kLengthofbuffer = (0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+457+68-0);
static const uint16 kReellist = (0+(36*144));
static const uint16 kIntext = (0+(38*2));
static const uint16 kLengthofmap = (0+(66*60));
static const uint16 kFreetext = (0+(82*2));
static const uint16 kBlocktext = (0+(98*2));
static const uint16 kBlocks = (0+192);
static const uint16 kFrframes = (0+2080);
static const uint16 kExframes = (0+2080);
static const uint16 kFrames = (0+2080);
static const uint16 kExdata = (0+2080+30000);
static const uint16 kExtextdat = (0+2080+30000+(16*114));
static const uint16 kExtext = (0+2080+30000+(16*114)+((114+2)*2));
static const uint16 kLengthofextra = (0+2080+30000+(16*114)+((114+2)*2)+18000);
static const uint16 kPersontxtdat = (0+24);
static const uint16 kPersontext = (0+24+(1026*2));
static const uint16 kInputport = (0x63);
static const uint16 kForeign = (1);
static const uint16 kCd = (1);
static const uint16 kNumexobjects = (114);
static const uint16 kUndertextsizey = (13);
static const uint16 kZoomy = (132);
static const uint16 kFreedatlen = (16*80);
static const uint16 kExtextlen = (18000);
static const uint16 kLenofmapstore = (22*8*20*8);
static const uint16 kUndertextsizex = (228);
static const uint16 kNumchanges = (250);
static const uint16 kUndertimedysize = (30);
static const uint16 kExframeslen = (30000);
static const uint16 kTablesize = (32);
static const uint16 kScreenwidth = (320);
static const uint16 kKeypadx = (36+112);
static const uint16 kItempicsize = (44);
static const uint16 kLenofreelrouts = (457);
static const uint16 kDiaryy = (48+12);
static const uint16 kOpsy = (52);
static const uint16 kSymboly = (56);
static const uint16 kInventy = (58);
static const uint16 kMenuy = (60);
static const uint16 kOpsx = (60);
static const uint16 kMaplength = (60);
static const uint16 kSymbolx = (64);
static const uint16 kSetdatlen = (64*128);
static const uint16 kMapwidth = (66);
static const uint16 kTextstart = (66*2);
static const uint16 kMaplen = (66*60);
static const uint16 kDiaryx = (68+24);
static const uint16 kLengthofvars = (68-0);
static const uint16 kKeypady = (72);
static const uint16 kZoomx = (8);
static const uint16 kInventx = (80);
static const uint16 kMenux = (80+40);
static const uint16 kHeaderlen = (96);


class DreamGenContext : public DreamBase, public Context {
public:
	DreamGenContext(DreamWeb::DreamWebEngine *en) : DreamBase(en), Context(this) {}

	void __start();
#include "stubs.h" // Allow hand-reversed functions to have a signature different than void f()

	void dirCom();
	void getAnyAd();
	void getFreeAd();
	void dirFile();
	void pickupConts();
	void transferMap();
	void getSetAd();
	void findAllOpen();
	void fillOpen();
	void dreamweb();
	void read();
	void searchForString();
	void getExAd();
	void parser();
	void transferConToEx();
};

} // End of namespace DreamGen

#endif
