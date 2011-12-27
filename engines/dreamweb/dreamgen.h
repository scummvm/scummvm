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
static const uint16 kRoomad = 89;
static const uint16 kWithobject = 91;
static const uint16 kWithtype = 92;
static const uint16 kLookcounter = 93;
static const uint16 kCommand = 95;
static const uint16 kCommandtype = 96;
static const uint16 kOldcommandtype = 97;
static const uint16 kObjecttype = 98;
static const uint16 kGetback = 99;
static const uint16 kInvopen = 100;
static const uint16 kMainmode = 101;
static const uint16 kPickup = 102;
static const uint16 kLastinvpos = 103;
static const uint16 kExamagain = 104;
static const uint16 kNewtextline = 105;
static const uint16 kOpenedob = 106;
static const uint16 kOpenedtype = 107;
static const uint16 kOldmapadx = 108;
static const uint16 kOldmapady = 110;
static const uint16 kMapadx = 112;
static const uint16 kMapady = 114;
static const uint16 kMapoffsetx = 116;
static const uint16 kMapoffsety = 118;
static const uint16 kMapxstart = 120;
static const uint16 kMapystart = 122;
static const uint16 kMapxsize = 124;
static const uint16 kMapysize = 125;
static const uint16 kHavedoneobs = 126;
static const uint16 kManisoffscreen = 127;
static const uint16 kRainspace = 128;
static const uint16 kFacing = 129;
static const uint16 kLeavedirection = 130;
static const uint16 kTurntoface = 131;
static const uint16 kTurndirection = 132;
static const uint16 kMaintimer = 133;
static const uint16 kIntrocount = 135;
static const uint16 kArrowad = 136;
static const uint16 kCurrentkey = 138;
static const uint16 kOldkey = 139;
static const uint16 kUseddirection = 140;
static const uint16 kTimercount = 141;
static const uint16 kOldtimercount = 142;
static const uint16 kMapx = 143;
static const uint16 kMapy = 144;
static const uint16 kNewscreen = 145;
static const uint16 kRyanx = 146;
static const uint16 kRyany = 147;
static const uint16 kLastflag = 148;
static const uint16 kOffsetx = 149;
static const uint16 kOffsety = 151;
static const uint16 kCurrentob = 153;
static const uint16 kDestpos = 154;
static const uint16 kReallocation = 155;
static const uint16 kRoomnum = 156;
static const uint16 kNowinnewroom = 157;
static const uint16 kResetmanxy = 158;
static const uint16 kNewlocation = 159;
static const uint16 kAutolocation = 160;
static const uint16 kDoorcheck1 = 161;
static const uint16 kDoorcheck2 = 162;
static const uint16 kDoorcheck3 = 163;
static const uint16 kDoorcheck4 = 164;
static const uint16 kMousex = 165;
static const uint16 kMousey = 167;
static const uint16 kMousebutton = 169;
static const uint16 kMousebutton1 = 171;
static const uint16 kMousebutton2 = 173;
static const uint16 kMousebutton3 = 175;
static const uint16 kMousebutton4 = 177;
static const uint16 kOldbutton = 179;
static const uint16 kOldx = 181;
static const uint16 kOldy = 183;
static const uint16 kLastbutton = 185;
static const uint16 kOldpointerx = 187;
static const uint16 kOldpointery = 189;
static const uint16 kDelherex = 191;
static const uint16 kDelherey = 193;
static const uint16 kPointerxs = 195;
static const uint16 kPointerys = 196;
static const uint16 kDelxs = 197;
static const uint16 kDelys = 198;
static const uint16 kPointerframe = 199;
static const uint16 kPointerpower = 200;
static const uint16 kAuxpointerframe = 201;
static const uint16 kPointermode = 202;
static const uint16 kPointerspeed = 203;
static const uint16 kPointercount = 204;
static const uint16 kInmaparea = 205;
static const uint16 kTalkmode = 206;
static const uint16 kTalkpos = 207;
static const uint16 kCharacter = 208;
static const uint16 kWatchdump = 209;
static const uint16 kLogonum = 210;
static const uint16 kOldlogonum = 211;
static const uint16 kPressed = 212;
static const uint16 kPresspointer = 213;
static const uint16 kGraphicpress = 215;
static const uint16 kPresscount = 216;
static const uint16 kLightcount = 217;
static const uint16 kFolderpage = 218;
static const uint16 kDiarypage = 219;
static const uint16 kMenucount = 220;
static const uint16 kSymboltopx = 221;
static const uint16 kSymboltopnum = 222;
static const uint16 kSymboltopdir = 223;
static const uint16 kSymbolbotx = 224;
static const uint16 kSymbolbotnum = 225;
static const uint16 kSymbolbotdir = 226;
static const uint16 kDumpx = 227;
static const uint16 kDumpy = 229;
static const uint16 kWalkandexam = 231;
static const uint16 kWalkexamtype = 232;
static const uint16 kWalkexamnum = 233;
static const uint16 kCurslocx = 234;
static const uint16 kCurslocy = 236;
static const uint16 kCurpos = 238;
static const uint16 kMonadx = 240;
static const uint16 kMonady = 242;
static const uint16 kNumtodo = 244;
static const uint16 kTimecount = 246;
static const uint16 kCounttotimed = 248;
static const uint16 kTimedseg = 250;
static const uint16 kTimedoffset = 252;
static const uint16 kTimedy = 254;
static const uint16 kTimedx = 255;
static const uint16 kNeedtodumptimed = 256;
static const uint16 kLoadingorsave = 257;
static const uint16 kCurrentslot = 258;
static const uint16 kCursorpos = 259;
static const uint16 kColourpos = 260;
static const uint16 kFadedirection = 261;
static const uint16 kNumtofade = 262;
static const uint16 kFadecount = 263;
static const uint16 kAddtogreen = 264;
static const uint16 kAddtored = 265;
static const uint16 kAddtoblue = 266;
static const uint16 kLastsoundreel = 267;
static const uint16 kSpeechloaded = 269;
static const uint16 kVolume = 270;
static const uint16 kVolumeto = 271;
static const uint16 kVolumedirection = 272;
static const uint16 kVolumecount = 273;
static const uint16 kWongame = 274;
static const uint16 kLasthardkey = 275;
static const uint16 kBufferin = 276;
static const uint16 kBufferout = 278;
static const uint16 kWorkspace = 280;
static const uint16 kMainsprites = 282;
static const uint16 kBackdrop = 284;
static const uint16 kRecordspace = 286;
static const uint16 kBlinkframe = 288;
static const uint16 kBlinkcount = 289;
static const uint16 kReasseschanges = 290;
static const uint16 kPointerspath = 291;
static const uint16 kManspath = 292;
static const uint16 kPointerfirstpath = 293;
static const uint16 kFinaldest = 294;
static const uint16 kDestination = 295;
static const uint16 kLinestartx = 296;
static const uint16 kLinestarty = 298;
static const uint16 kLineendx = 300;
static const uint16 kLineendy = 302;
static const uint16 kLinepointer = 304;
static const uint16 kLinedirection = 305;
static const uint16 kLinelength = 306;
static const uint16 kCh0playing = 307;
static const uint16 kCh0repeat = 308;
static const uint16 kCh1playing = 309;
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
};

} // End of namespace DreamGen

#endif
