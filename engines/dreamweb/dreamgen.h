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

static const uint16 offset_operand1 = 0x0194;
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
static const uint16 kWithobject = 92;
static const uint16 kWithtype = 93;
static const uint16 kLookcounter = 94;
static const uint16 kCommand = 96;
static const uint16 kCommandtype = 97;
static const uint16 kOldcommandtype = 98;
static const uint16 kObjecttype = 99;
static const uint16 kGetback = 100;
static const uint16 kInvopen = 101;
static const uint16 kMainmode = 102;
static const uint16 kPickup = 103;
static const uint16 kLastinvpos = 104;
static const uint16 kExamagain = 105;
static const uint16 kNewtextline = 106;
static const uint16 kOpenedob = 107;
static const uint16 kOpenedtype = 108;
static const uint16 kOldmapadx = 109;
static const uint16 kOldmapady = 111;
static const uint16 kMapadx = 113;
static const uint16 kMapady = 115;
static const uint16 kMapoffsetx = 117;
static const uint16 kMapoffsety = 119;
static const uint16 kMapxstart = 121;
static const uint16 kMapystart = 123;
static const uint16 kMapxsize = 125;
static const uint16 kMapysize = 126;
static const uint16 kHavedoneobs = 127;
static const uint16 kManisoffscreen = 128;
static const uint16 kRainspace = 129;
static const uint16 kFacing = 130;
static const uint16 kLeavedirection = 131;
static const uint16 kTurntoface = 132;
static const uint16 kTurndirection = 133;
static const uint16 kMaintimer = 134;
static const uint16 kIntrocount = 136;
static const uint16 kArrowad = 137;
static const uint16 kCurrentkey = 139;
static const uint16 kOldkey = 140;
static const uint16 kUseddirection = 141;
static const uint16 kTimercount = 142;
static const uint16 kOldtimercount = 143;
static const uint16 kMapx = 144;
static const uint16 kMapy = 145;
static const uint16 kNewscreen = 146;
static const uint16 kRyanx = 147;
static const uint16 kRyany = 148;
static const uint16 kLastflag = 149;
static const uint16 kOffsetx = 150;
static const uint16 kOffsety = 152;
static const uint16 kCurrentob = 154;
static const uint16 kDestpos = 155;
static const uint16 kReallocation = 156;
static const uint16 kRoomnum = 157;
static const uint16 kNowinnewroom = 158;
static const uint16 kResetmanxy = 159;
static const uint16 kNewlocation = 160;
static const uint16 kAutolocation = 161;
static const uint16 kDoorcheck1 = 162;
static const uint16 kDoorcheck2 = 163;
static const uint16 kDoorcheck3 = 164;
static const uint16 kDoorcheck4 = 165;
static const uint16 kMousex = 166;
static const uint16 kMousey = 168;
static const uint16 kMousebutton = 170;
static const uint16 kMousebutton1 = 172;
static const uint16 kMousebutton2 = 174;
static const uint16 kMousebutton3 = 176;
static const uint16 kMousebutton4 = 178;
static const uint16 kOldbutton = 180;
static const uint16 kOldx = 182;
static const uint16 kOldy = 184;
static const uint16 kLastbutton = 186;
static const uint16 kOldpointerx = 188;
static const uint16 kOldpointery = 190;
static const uint16 kDelherex = 192;
static const uint16 kDelherey = 194;
static const uint16 kPointerxs = 196;
static const uint16 kPointerys = 197;
static const uint16 kDelxs = 198;
static const uint16 kDelys = 199;
static const uint16 kPointerframe = 200;
static const uint16 kPointerpower = 201;
static const uint16 kAuxpointerframe = 202;
static const uint16 kPointermode = 203;
static const uint16 kPointerspeed = 204;
static const uint16 kPointercount = 205;
static const uint16 kInmaparea = 206;
static const uint16 kTalkmode = 207;
static const uint16 kTalkpos = 208;
static const uint16 kCharacter = 209;
static const uint16 kWatchdump = 210;
static const uint16 kLogonum = 211;
static const uint16 kOldlogonum = 212;
static const uint16 kNetseg = 213;
static const uint16 kNetpoint = 215;
static const uint16 kCursorstate = 217;
static const uint16 kPressed = 218;
static const uint16 kPresspointer = 219;
static const uint16 kGraphicpress = 221;
static const uint16 kPresscount = 222;
static const uint16 kLightcount = 223;
static const uint16 kFolderpage = 224;
static const uint16 kDiarypage = 225;
static const uint16 kMenucount = 226;
static const uint16 kSymboltopx = 227;
static const uint16 kSymboltopnum = 228;
static const uint16 kSymboltopdir = 229;
static const uint16 kSymbolbotx = 230;
static const uint16 kSymbolbotnum = 231;
static const uint16 kSymbolbotdir = 232;
static const uint16 kSymboltolight = 233;
static const uint16 kSymbol1 = 234;
static const uint16 kSymbol2 = 235;
static const uint16 kSymbol3 = 236;
static const uint16 kSymbolnum = 237;
static const uint16 kDumpx = 238;
static const uint16 kDumpy = 240;
static const uint16 kWalkandexam = 242;
static const uint16 kWalkexamtype = 243;
static const uint16 kWalkexamnum = 244;
static const uint16 kCurslocx = 245;
static const uint16 kCurslocy = 247;
static const uint16 kCurpos = 249;
static const uint16 kMonadx = 251;
static const uint16 kMonady = 253;
static const uint16 kMonsource = 255;
static const uint16 kNumtodo = 257;
static const uint16 kTimecount = 259;
static const uint16 kCounttotimed = 261;
static const uint16 kTimedseg = 263;
static const uint16 kTimedoffset = 265;
static const uint16 kTimedy = 267;
static const uint16 kTimedx = 268;
static const uint16 kNeedtodumptimed = 269;
static const uint16 kLoadingorsave = 270;
static const uint16 kCurrentslot = 271;
static const uint16 kCursorpos = 272;
static const uint16 kColourpos = 273;
static const uint16 kFadedirection = 274;
static const uint16 kNumtofade = 275;
static const uint16 kFadecount = 276;
static const uint16 kAddtogreen = 277;
static const uint16 kAddtored = 278;
static const uint16 kAddtoblue = 279;
static const uint16 kLastsoundreel = 280;
static const uint16 kSpeechloaded = 282;
static const uint16 kSpeechlength = 283;
static const uint16 kVolume = 285;
static const uint16 kVolumeto = 286;
static const uint16 kVolumedirection = 287;
static const uint16 kVolumecount = 288;
static const uint16 kWongame = 289;
static const uint16 kLasthardkey = 290;
static const uint16 kBufferin = 291;
static const uint16 kBufferout = 293;
static const uint16 kExtras = 295;
static const uint16 kWorkspace = 297;
static const uint16 kMapstore = 299;
static const uint16 kCharset1 = 301;
static const uint16 kMainsprites = 303;
static const uint16 kBackdrop = 305;
static const uint16 kMapdata = 307;
static const uint16 kSounddata = 309;
static const uint16 kSounddata2 = 311;
static const uint16 kRecordspace = 313;
static const uint16 kFreedat = 315;
static const uint16 kSetdat = 317;
static const uint16 kReel1 = 319;
static const uint16 kReel2 = 321;
static const uint16 kReel3 = 323;
static const uint16 kRoomdesc = 325;
static const uint16 kFreedesc = 327;
static const uint16 kSetdesc = 329;
static const uint16 kBlockdesc = 331;
static const uint16 kSetframes = 333;
static const uint16 kFreeframes = 335;
static const uint16 kPeople = 337;
static const uint16 kReels = 339;
static const uint16 kCommandtext = 341;
static const uint16 kPuzzletext = 343;
static const uint16 kTraveltext = 345;
static const uint16 kTempgraphics = 347;
static const uint16 kTempgraphics2 = 349;
static const uint16 kTempgraphics3 = 351;
static const uint16 kTempsprites = 353;
static const uint16 kTextfile1 = 355;
static const uint16 kTextfile2 = 357;
static const uint16 kTextfile3 = 359;
static const uint16 kBlinkframe = 361;
static const uint16 kBlinkcount = 362;
static const uint16 kReasseschanges = 363;
static const uint16 kPointerspath = 364;
static const uint16 kManspath = 365;
static const uint16 kPointerfirstpath = 366;
static const uint16 kFinaldest = 367;
static const uint16 kDestination = 368;
static const uint16 kLinestartx = 369;
static const uint16 kLinestarty = 371;
static const uint16 kLineendx = 373;
static const uint16 kLineendy = 375;
static const uint16 kLinepointer = 377;
static const uint16 kLinedirection = 378;
static const uint16 kLinelength = 379;
static const uint16 kCh0blockstocopy = 380;
static const uint16 kCh0playing = 382;
static const uint16 kCh0repeat = 383;
static const uint16 kCh1playing = 384;
static const uint16 kCh1blockstocopy = 385;
static const uint16 kSoundbufferwrite = 387;
static const uint16 kCurrentsample = 389;
static const uint16 kRoomssample = 390;
static const uint16 kBasicsample = 391;
static const uint16 kCurrentfile = 418;
static const uint16 kInputline = 493;
static const uint16 kQuitrequested = 621;
static const uint16 kSubtitles = 622;
static const uint16 kForeignrelease = 623;
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

	void dirFile();
	void transferMap();
	void dreamweb();
	void read();
	void searchForString();
	void parser();
};

} // End of namespace DreamGen

#endif
