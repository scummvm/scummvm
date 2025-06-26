/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LASTEXPRESS_SHARED_H
#define LASTEXPRESS_SHARED_H

#include "common/func.h"

namespace LastExpress {

//////////////////////////////////////////////////////////////////////////
// Sound
//////////////////////////////////////////////////////////////////////////

enum SoundTag : uint {
	kSoundTagNone        = 0,
	kSoundTagAmbient     = 1,
	kSoundTagOldAmbient  = 2,
	kSoundTagWalla       = 3,
	kSoundTagOldWalla    = 4,
	kSoundTagConcert     = 5,
	// 6 is unused
	kSoundTagLink        = 7,
	kSoundTagOldLink     = 8,
	kSoundTagNIS         = 9,
	kSoundTagOldNIS      = 10,
	kSoundTagIntro       = 11,
	// 12 is unused
	kSoundTagMenu        = 13,
	kSoundTagOldMenu     = 14,
	kSoundTagCredits     = 15,
	kSoundTagFirstNormal = 16
	// every normal sound gets its own tag from an incrementing counter
	// initialized as kSoundTagFirstNormal,
	// so tags can have values not covered by this enum
};

/*
    These are the flags used by the original game
    to keep track of sound entry status.

    They are directly exposed via savefiles,
    so we should be aware of them
    even though we don't use some of them internally.

    Sound playback is asynchronous.
    We have threads and mutexes for synchronization,
    DOS games have main code and IRQ/interrupt handlers instead,
    some flags come in pairs to deal with this:
    the main code sets kSoundFlagXxxRequested as a signal
    to the interrupt handler, the interrupt handler processes it
    (e.g. stops using the associated buffer for Close and Mute requests)
    and sets the corresponding result flag. The main code can proceed then
    (e.g. release the associated buffer).

    The original game has a limited number of sound buffers (namely, 6)
    (plus 16 versions of ADPCM decoder in assembly language,
    one for every non-zero volume, so I suppose the performance was an issue).
    The original game has also many events that could happen in different areas
    of the train at the same time, some of them are synchronized via the sound
    (kCharacterActionEndSound). To deal with it, the original game uses kSoundFlagMute:
    muted sounds don't have their own buffer, don't participate in mixing the channels,
    but the interrupt handler still tracks their progress.
    Non-audible sounds (e.g. because the corresponding event goes on in another car)
    are always muted; if the number of audible sounds exceeds the number of buffers,
    least-priority sounds are muted as well (the priority is the sum of a static
    constant from the entry constructor and the current volume).

    Normally the sound duration is read from (one of the fields
    in the header of) the associated file. However, if the sound entry
    is started as muted, the buffer is not allocated and no data are read;
    in this case, the duration is estimated from file size.
    Since HPF archives store all sizes as counts of 0x800-byte blocks,
    this loses some precision, but nothing to really care about.
    If a started-as-muted sound is unmuted later (Cath enters the car
    where a dialog takes place), the exact duration is loaded from the file;
    kSoundFlagHeaderProcessed says that the duration is exact.

    We have more sound channels available, we are not so limited
    by the performance, and we lose some control of how exactly the backend
    processes the sound as a payment for portability, so we can afford
    to just mix the silence without special processing of muted entries.
*/
enum SoundFlag : uint {
	kSoundVolumeEntityDefault = 0xFFFFFFFF, // special value for SoundManager::playSound; choose volume based on distance to the character

	kVolumeNone               = 0x0,
	kVolume1                  = 0x1,
	kVolume2                  = 0x2,
	kVolume3                  = 0x3,
	kVolume4                  = 0x4,
	kVolume5                  = 0x5,
	kVolume6                  = 0x6,
	kVolume7                  = 0x7,
	kVolume8                  = 0x8,
	kVolume9                  = 0x9,
	kVolume10                 = 0xA,
	kVolume11                 = 0xB,
	kVolume12                 = 0xC,
	kVolume13                 = 0xD,
	kVolume14                 = 0xE,
	kVolume15                 = 0xF,
	kVolumeFull               = 0x10,

	kSoundVolumeMask          = 0x1F,

	kSoundFlagPlayRequested   = 0x20,
	kSoundFlagPlaying         = 0x40, // IRQ handler has seen kSoundFlagPlayRequested and has started the playback
	kSoundFlagMuteRequested   = 0x80,
	kSoundFlagMuteProcessed   = 0x100, // IRQ handler has seen kSoundFlagMuteRequested
	kSoundFlagMute            = kSoundFlagMuteRequested | kSoundFlagMuteProcessed,
	kSoundFlagCloseRequested  = 0x200, // close requested, waiting for IRQ handler to confirm
	kSoundFlagClosed          = 0x400, // IRQ handler has seen kSoundFlagClosing and is completely done with this sound
	kSoundFlagCloseOnDataEnd  = 0x800, // used as the opposite of kSoundFlagLooped
	kSoundFlagLooped          = 0x1000,
	kSoundFlagCyclicBuffer    = 0x2000, // when the decoder reaches the end of buffer, the decoder should continue from the beginning of buffer
	kSoundFlagHasUnreadData   = 0x4000, // stream has more data
	kSoundFlagDelayedActivate = 0x8000, // start playing at _activateTime
	kSoundFlagHasLinkAfter    = 0x10000, // _linkAfter is valid and should be activated after this sound; used by xxx.NIS sounds for xxx.LNK
	kSoundFlagHasSubtitles    = 0x20000,
	kSoundFlagPaused          = 0x40000, // IRQ handler has seen kSoundFlagPauseRequested and does not use the buffer anymore
	kSoundFlagFixedVolume     = 0x80000, // Turns off the logic of volume adjusting for character-related sounds when distance to character is changed
	kSoundFlagVolumeChanging  = 0x100000, // smooth changing of the volume is in progress
	kSoundFlagHeaderProcessed = 0x200000, // count of blocks is the accurate value from the header
	kSoundFlagPauseRequested  = 0x400000, // used when the reader needs to change the buffer
	kSoundFlagDecodeStall     = 0x800000, // the decoder has stopped because the reader is too slow and has not yet provided further data

	kSoundTypeNormal          = 0x0000000, // everything not included in any specific category
	kSoundTypeAmbient         = 0x1000000, // train sounds, steam, wind, restaurant sounds
	kSoundTypeConcert         = 0x2000000, // 1917.LNK
	kSoundTypeMenu            = 0x3000000, // menu screen, blinking egg after time travel; excluded from savefiles
	kSoundTypeLink            = 0x4000000, // xxx.LNK linked after NIS sound, except for 1917.LNK
	kSoundTypeIntro           = 0x5000000, // intro at game start before showing the menu
	kSoundTypeWalla           = 0x6000000, // LOOP8A.SND by kCharacterTableC
	kSoundTypeNIS             = 0x7000000, // special entry managed by NIS code

	kSoundTypeMask            = 0x7000000,

	kSoundFlagKeepAfterFinish = 0x8000000, // don't free the entry when it has stopped playing; used for kSoundTypeNIS
	kSoundFlagDecodeError     = 0x20000000, // error in compressed stream
	kSoundFlagFading          = 0x40000000, // prevents attempts to unfade once fade is requested
	kSoundFlagUnmuteRequested = 0x80000000  // purely informational
};

enum AmbientSoundState : uint {
	kAmbientSoundEnabled  = 1,
	kAmbientSoundSteam    = 2,
	kAmbientLooping       = 69
};

enum SoundDriverFlag : uint {
	kSoundDriverInitState            = 0x1,
	kSoundDriverNISHasRequestedDelay = 0x2,
	kSoundDriverClearBufferRequested = 0x4,
	kSoundDriverClearBufferProcessed = 0x8,
	kSoundDriverNISHasRequestedFade  = 0x20,
	kSoundDriverStarted              = 0x8000
};

enum MouseFlags : int {
	kMouseFlagLeftButton  = 0x1,
	kMouseFlagRightButton = 0x2,
	kMouseFlagLeftDown    = 0x8,
	kMouseFlagRightDown   = 0x10,
	kMouseFlagDoubleClick = 0x20,
	kMouseFlagLeftUp      = 0x80,
	kMouseFlagRightUp     = 0x100
};

enum EventChannel : int {
	kEventChannelMouse  = 1,
	kEventChannelTimer  = 3,
	kEventChannelEngine = 4
};

enum NisFlags : uint {
	kNisFlagHasSound            = 0x1,     // Set when a valid NIS sound exists
	kNisFlagDataChunksAvailable = 0x100,   // Used during initialization and chunk loading
	kNisFlagPlaying             = 0x200,   // NIS is currently playing
	kNisFlagBaseFlag            = 0x4000,  // Flag with which doNIS is always called
	kNisFlagSoundFade           = 0x8000,  // Flag for sound fading operations
	kNisFlagSoundInitialized    = 0x40000, // First sound chunk has been loaded and initialized
	kNisFlagAbortRequested      = 0x80000  // Abort has been requested (e.g., via right mouse click)
};

enum NISEventTypes : uint {
	kNISEventNone = 0,
	kNISEventUnknown1 = 1,
	kNISEventUnknown2 = 2,
	kNISEventAudioInfo = 3,
	kNISEventUnknown4 = 4,
	kNISEventUnknown5 = 5,
	kNISEventBackground1 = 10,
	kNISEventSelectBackground1 = 11,
	kNISEventBackground2 = 12,
	kNISEventSelectBackground2 = 13,
	kNISEventOverlay = 20,
	kNISEventUpdate = 21,
	kNISEventUpdateTransition = 22,
	kNISEventSound1 = 30,
	kNISEventSound2 = 31,
	kNISEventAudioData = 32,
	kNISEventAudioEnd = 99
};

enum SubtitlesFlags : int32 {
	kSubFlagDrawOnScreen = 0x1,
	kSubFlagLoaded       = 0x2,
	kSubFlagStatusKilled = 0x400
};

//////////////////////////////////////////////////////////////////////////
// Time values
//////////////////////////////////////////////////////////////////////////

// Time is measured in ticks, with 15 ticks per second. One minute is 900
// ticks, one hour is 54,000 ticks, and one day is 1,296,000 ticks.

enum TimeValue : uint {
	kTimeNone                 = 0,
	kTime5933                 = 5933,

	kTimeCityParis            = 1037700,	// Day 1, 19:13
	kTime1039500              = 1039500,	// Day 1, 19:15
	kTimeStartGame            = 1061100,	// Day 1, 19:39

	// Chapter 1
	kTimeChapter1             = 1062000,	// Day 1, 19:40
	kTime1071000              = 1071000,	// Day 1, 19:50
	kTimeParisEpernay         = 1075500,	// Day 1, 19:55
	kTime1080000              = 1080000,	// Day 1, 20:00
	kTime1084500              = 1084500,	// Day 1, 20:05
	kTime1089000              = 1089000,	// Day 1, 20:10
	kTime1093500              = 1093500,	// Day 1, 20:15
	kTime1094400              = 1094400,	// Day 1, 20:16
	kTime1096200              = 1096200,	// Day 1, 20:18
	kTime1098000              = 1098000,	// Day 1, 20:20
	kTime1102500              = 1102500,	// Day 1, 20:25
	kTime1107000              = 1107000,	// Day 1, 20:30
	kTime1111500              = 1111500,	// Day 1, 20:35
	kTime1120500              = 1120500,	// Day 1, 20:45
	kTime1125000              = 1125000,	// Day 1, 20:50
	kTime1134000              = 1134000,	// Day 1, 21:00
	kTime1138500              = 1138500,	// Day 1, 21:05
	kTime1143000              = 1143000,	// Day 1, 21:10
	kTimeEnterEpernay         = 1147500,	// Day 1, 21:15
	kTimeCityEpernay          = 1148400,	// Day 1, 21:16
	kTimeExitEpernay          = 1150200,	// Day 1, 21:18
	kTime1156500              = 1156500,	// Day 1, 21:25
	kTime1161000              = 1161000,	// Day 1, 21:30
	kTime1162800              = 1162800,	// Day 1, 21:32
	kTime1165500              = 1165500,	// Day 1, 21:35
	kTime1167300              = 1167300,	// Day 1, 21:37
	kTimeEnterChalons         = 1170000,	// Day 1, 21:40
	kTimeCityChalons          = 1170900,	// Day 1, 21:41
	kTimeExitChalons          = 1173600,	// Day 1, 21:44
	kTime1174500              = 1174500,	// Day 1, 21:45
	kTime1179000              = 1179000,	// Day 1, 21:50
	kTime1183500              = 1183500,	// Day 1, 21:55
	kTime1184400              = 1184400,	// Day 1, 21:56
	kTime1188000              = 1188000,	// Day 1, 22:00
	kTime1189800              = 1189800,	// Day 1, 22:02
	kTime1192500              = 1192500,	// Day 1, 22:05
	kTime1197000              = 1197000,	// Day 1, 22:10
	kTime1201500              = 1201500,	// Day 1, 22:15
	kTime1206000              = 1206000,	// Day 1, 22:20
	kTime1215000              = 1215000,	// Day 1, 22:30
	kTime1224000              = 1224000,	// Day 1, 22:40
	kTime1225800              = 1225800,	// Day 1, 22:42
	kTimeCityBarLeDuc         = 1228500,	// Day 1, 22:45
	kTimeExitBarLeDuc         = 1231200,	// Day 1, 22:48
	kTime1233000              = 1233000,	// Day 1, 22:50
	kTime1242000              = 1242000,	// Day 1, 23:00
	kTime1260000              = 1260000,	// Day 1, 23:20
	kTimeCityNancy            = 1303200,	// Day 2, 00:08
	kTimeExitNancy            = 1307700,	// Day 2, 00:13
	kTime1323000              = 1323000,	// Day 2, 00:30
	kTimeCityLuneville        = 1335600,	// Day 2, 00:44
	kTimeExitLuneville        = 1338300,	// Day 2, 00:47
	kTimeCityAvricourt        = 1359900,	// Day 2, 01:11
	kTimeExitAvricourt        = 1363500,	// Day 2, 01:15
	kTimeCityDeutschAvricourt = 1367100,	// Day 2, 01:19
	kTimeExitDeutschAvricourt = 1370700,	// Day 2, 01:23
	kTime1386000              = 1386000,	// Day 2, 01:40
	kTimeBedTime              = 1404000,	// Day 2, 02:00
	kTime1417500              = 1417500,	// Day 2, 02:15
	kTimeEnterStrasbourg      = 1424700,	// Day 2, 02:23
	kTime1449000              = 1449000,	// Day 2, 02:50
	kTime1458000              = 1458000,	// Day 2, 03:00
	kTime1485000              = 1485000,	// Day 2, 03:30
	kTime1489500              = 1489500,	// Day 2, 03:35
	kTimeCityStrasbourg       = 1490400,	// Day 2, 03:36
	kTime1492200              = 1492200,	// Day 2, 03:38
	kTimeExitStrasbourg       = 1493100,	// Day 2, 03:39
	kTimeChapter1End          = 1494000,	// Day 2, 03:40
	kTime1503000              = 1503000,	// Day 2, 03:50
	kTime1512000              = 1512000,	// Day 2, 04:00
	kTimeCityBadenOos         = 1539000,	// Day 2, 04:30
	kTimeExitBadenOos         = 1541700,	// Day 2, 04:33
	kTimeCityKarlsruhe        = 1563300,	// Day 2, 04:57
	kTimeCityStuttgart        = 1656000,	// Day 2, 06:40
	kTimeChapter1End2         = 1647000,	// Day 2, 06:30
	kTimeChapter1End3         = 1674000,	// Day 2, 07:00
	kTimeCityGeislingen       = 1713600,	// Day 2, 07:44
	kTime1714500              = 1714500,	// Day 2, 07:45
	kTimeCityUlm              = 1739700,	// Day 2, 08:13

	// Chapter 2
	kTimeChapter2             = 1750500,	// Day 2, 08:25
	kTime1759500              = 1759500,	// Day 2, 08:35
	kTime1755000              = 1755000,	// Day 2, 08:30
	kTime1764000              = 1764000,	// Day 2, 08:40
	kTime1768500              = 1768500,	// Day 2, 08:45
	kTime1773000              = 1773000,	// Day 2, 08:50
	kTime1777500              = 1777500,	// Day 2, 08:55
	kTime1782000              = 1782000,	// Day 2, 09:00
	kTime1786500              = 1786500,	// Day 2, 09:05
	kTime1791000              = 1791000,	// Day 2, 09:10
	kTime1800000              = 1800000,	// Day 2, 09:20
	kTime1801800              = 1801800,	// Day 2, 09:22
	kTime1806300              = 1806300,	// Day 2, 09:27
	kTime1809000              = 1809000,	// Day 2, 09:30
	kTimeCityAugsburg         = 1809900,	// Day 2, 09:31
	kTime1813500              = 1813500,	// Day 2, 09:35
	kTime1818000              = 1818000,	// Day 2, 09:40
	kTime1818900              = 1818900,	// Day 2, 09:41
	kTime1820700              = 1820700,	// Day 2, 09:43
	kTime1822500              = 1822500,	// Day 2, 09:45
	kTime1827000              = 1827000,	// Day 2, 09:50
	kTime1831500              = 1831500,	// Day 2, 09:55
	kTime1836000              = 1836000,	// Day 2, 10:00
	kTime1845000              = 1845000,	// Day 2, 10:10
	kTime1849500              = 1849500,	// Day 2, 10:15
	kTimeCityMunich           = 1852200,	// Day 2, 10:18

	// Chapter 3
	kTimeChapter3             = 1944000,	// Day 2, 12:00
	kTime1953000              = 1953000,	// Day 2, 12:10
	kTime1966500              = 1966500,	// Day 2, 12:25
	kTime1969200              = 1969200,	// Day 2, 12:28
	kTime1971000              = 1971000,	// Day 2, 12:30
	kTimeEnterSalzbourg       = 1982700,	// Day 2, 12:43
	kTime1983600              = 1983600,	// Day 2, 12:44
	kTimeCitySalzbourg        = 1984500,	// Day 2, 12:45
	kTime1989000              = 1989000,	// Day 2, 12:50
	kTimeExitSalzbourg        = 1989900,	// Day 2, 12:51
	kTime1993500              = 1993500,	// Day 2, 12:55
	kTime1998000              = 1998000,	// Day 2, 13:00
	kTime2002500              = 2002500,	// Day 2, 13:05
	kTime2011500              = 2011500,	// Day 2, 13:15
	kTime2016000              = 2016000,	// Day 2, 13:20
	kTime2020500              = 2020500,	// Day 2, 13:25
	kTime2025000              = 2025000,	// Day 2, 13:30
	kTime2034000              = 2034000,	// Day 2, 13:40
	kTime2038500              = 2038500,	// Day 2, 13:45
	kTime2040300              = 2040300,	// Day 2, 13:47
	kTime2043000              = 2043000,	// Day 2, 13:50
	kTimeEnterAttnangPuchheim = 2047500,	// Day 2, 13:55
	kTimeCityAttnangPuchheim  = 2049300,	// Day 2, 13:57
	kTime2052000              = 2052000,	// Day 2, 14:00
	kTimeExitAttnangPuchheim  = 2052900,	// Day 2, 14:01
	kTime2056500              = 2056500,	// Day 2, 14:05
	kTime2061000              = 2061000,	// Day 2, 14:10
	kTime2062800              = 2062800,	// Day 2, 14:12
	kTime2065500              = 2065500,	// Day 2, 14:15
	kTime2070000              = 2070000,	// Day 2, 14:20
	kTimeEnterWels            = 2073600,	// Day 2, 14:24
	kTimeCityWels             = 2075400,	// Day 2, 14:26
	kTime2079000              = 2079000,	// Day 2, 14:30
	kTimeExitWels             = 2079900,	// Day 2, 14:31
	kTime2083500              = 2083500,	// Day 2, 14:35
	kTime2088000              = 2088000,	// Day 2, 14:40
	kTime2088900              = 2088900,	// Day 2, 14:41
	kTime2092500              = 2092500,	// Day 2, 14:45
	kTime2097000              = 2097000,	// Day 2, 14:50
	kTimeEnterLinz            = 2099700,	// Day 2, 14:53
	kTimeCityLinz             = 2101500,	// Day 2, 14:55
	kTimeExitLinz             = 2105100,    // Day 2, 14:59
	kTime2106000              = 2106000,	// Day 2, 15:00
	kTime2110500              = 2110500,	// Day 2, 15:05
	kTime2115000              = 2115000,	// Day 2, 15:10
	kTime2117700              = 2117700,	// Day 2, 15:13
	kTime2119500              = 2119500,	// Day 2, 15:15
	kTime2124000              = 2124000,	// Day 2, 15:20
	kTime2133000              = 2133000,	// Day 2, 15:30
	kTime2138400              = 2138400,	// Day 2, 15:36
	kTime2142000              = 2142000,	// Day 2, 15:40
	kTime2146500              = 2146500,	// Day 2, 15:45
	kTime2147400              = 2147400,	// Day 2, 15:46
	kTime2151000              = 2151000,	// Day 2, 15:50
	kTimeCityAmstetten        = 2154600,	// Day 2, 15:54
	kTime2155500              = 2155500,	// Day 2, 15:55
	kTime2160000              = 2160000,	// Day 2, 16:00
	kTime2169000              = 2169000,	// Day 2, 16:10
	kTime2173500              = 2173500,	// Day 2, 16:15
	kTime2187000              = 2187000,	// Day 2, 16:30
	kTime2182500              = 2182500,	// Day 2, 16:25
	kTime2196000              = 2196000,	// Day 2, 16:40
	kTime2200500              = 2200500,	// Day 2, 16:45
	kTime2205000              = 2205000,	// Day 2, 16:50
	kTime2214000              = 2214000,	// Day 2, 17:00
	kTime2218500              = 2218500,	// Day 2, 17:05
	kTime2223000              = 2223000,	// Day 2, 17:10
	kTime2227500              = 2227500,	// Day 2, 17:15
	kTime2241000              = 2241000,	// Day 2, 17:30
	kTime2248200              = 2248200,	// Day 2, 17:38
	kTime2250000              = 2250000,	// Day 2, 17:40
	kTime2254500              = 2254500,	// Day 2, 17:45
	kTime2259000              = 2259000,	// Day 2, 17:50
	kTime2263500              = 2263500,	// Day 2, 17:55
	kTime2266200              = 2266200,	// Day 2, 17:58
	kTimeCityVienna           = 2268000,	// Day 2, 18:00

	// Chapter 4
	kTime2349000              = 2349000,	// Day 2, 19:30
	kTimeChapter4             = 2353500,	// Day 2, 19:35
	kTime2354400              = 2354400,	// Day 2, 19:36
	kTime2356200              = 2356200,	// Day 2, 19:38
	kTime2358000              = 2358000,	// Day 2, 19:40
	kTime2360700              = 2360700,	// Day 2, 19:43
	kTime2362500              = 2362500,	// Day 2, 19:45
	kTime2361600              = 2361600,	// Day 2, 19:44
	kTime2367000              = 2367000,	// Day 2, 19:50
	kTime2370600              = 2370600,	// Day 2, 19:54
	kTime2378700              = 2378700,	// Day 2, 20:03
	kTimeEnterPoszony         = 2381400,	// Day 2, 20:06
	kTimeCityPoszony          = 2383200,	// Day 2, 20:08
	kTime2385000              = 2385000,	// Day 2, 20:10
	kTimeExitPoszony          = 2386800,	// Day 2, 20:12
	kTime2389500              = 2389500,	// Day 2, 20:15
	kTime2394000              = 2394000,	// Day 2, 20:20
	kTime2398500              = 2398500,	// Day 2, 20:25
	kTime2403000              = 2403000,	// Day 2, 20:30
	kTime2407500              = 2407500,	// Day 2, 20:35
	kTime2410200              = 2410200,	// Day 2, 20:38
	kTime2412000              = 2412000,	// Day 2, 20:40
	kTime2414700              = 2414700,	// Day 2, 20:43
	kTime2415600              = 2415600,	// Day 2, 20:44
	kTimeEnterGalanta         = 2416500,	// Day 2, 20:45
	kTimeCityGalanta          = 2418300,	// Day 2, 20:47
	kTime2421000              = 2421000,	// Day 2, 20:50
	kTimeExitGalanta          = 2421900,	// Day 2, 20:51
	kTime2422800              = 2422800,	// Day 2, 20:52
	kTime2428200              = 2428200,	// Day 2, 20:58
	kTime2425500              = 2425500,	// Day 2, 20:55
	kTime2430000              = 2430000,	// Day 2, 21:00
	kTime2434500              = 2434500,	// Day 2, 21:05
	kTime2439000              = 2439000,	// Day 2, 21:10
	kTime2443500              = 2443500,	// Day 2, 21:15
	kTime2448000              = 2448000,	// Day 2, 21:20
	kTime2452500              = 2452500,	// Day 2, 21:25
	kTime2455200              = 2455200,	// Day 2, 21:28
	kTime2457000              = 2457000,	// Day 2, 21:30
	kTime2466000              = 2466000,	// Day 2, 21:40
	kTime2470500              = 2470500,	// Day 2, 21:45
	kTime2475000              = 2475000,	// Day 2, 21:50
	kTime2479500              = 2479500,	// Day 2, 21:55
	kTime2484000              = 2484000,	// Day 2, 22:00
	kTime2488500              = 2488500,	// Day 2, 22:05
	kTime2493000              = 2493000,	// Day 2, 22:10
	kTime2506500              = 2506500,	// Day 2, 22:25
	kTime2507400              = 2507400,	// Day 2, 22:26
	kTime2511000              = 2511000,	// Day 2, 22:30
	kTime2511900              = 2511900,	// Day 2, 22:31
	kTime2517300              = 2517300,	// Day 2, 22:37
	kTime2519100              = 2519100,	// Day 2, 22:39
	kTime2520000              = 2520000,	// Day 2, 22:40
	kTime2533500              = 2533500,	// Day 2, 22:55
	kTime2535300              = 2535300,	// Day 2, 22:57
	kTime2538000              = 2538000,	// Day 2, 23:00
	kTimeCityBudapest         = 2551500,	// Day 2, 23:15

	// Chapter 5
	kTimeChapter5             = 2844000,	// Day 3, 04:40
	kTimeTrainStopped         = 2898000,	// Day 3, 05:40
	kTime2907000              = 2907000,	// Day 3, 05:50
	kTime2916000              = 2916000,	// Day 3, 06:00
	kTime2934000              = 2934000,    // Day 3, 06:20
	kTimeTrainStopped2        = 2943000,	// Day 3, 06:30
	kTime2949300              = 2949300,    // Day 3, 06:37
	kTimeCityBelgrade         = 2952000,	// Day 3, 06:40
	kTime2983500              = 2983500,	// Day 3, 07:15
	kTimeCityNish             = 3205800,	// Day 3, 11:22
	kTimeCityTzaribrod        = 3492000,	// Day 3, 16:40
	kTime3645000              = 3645000,	// Day 3, 19:30
	kTimeCitySofia            = 3690000,	// Day 3, 20:20
	kTimeCityAdrianople       = 4320900,	// Day 4, 08:01
	kTime4914000              = 4914000,    // Day 4, 19:00
	kTime4920300              = 4920300,    // Day 4, 19:07
	kTime4923000              = 4923000,	// Day 4, 19:10
	kTime4929300              = 4929300,	// Day 4, 19:17
	kTimeCityConstantinople   = 4941000,	// Day 4, 19:30


	kTime10881000             = 10881000,
	kTimeEnd                  = 15803100,
	kTime16451100             = 16451100,

	kTimeInvalid              = 0x7FFFFFFF
};

//////////////////////////////////////////////////////////////////////////
// Archive & Chapter ID
//////////////////////////////////////////////////////////////////////////
enum ArchiveIndex {
	kArchiveAll = 0,
	kArchiveCd1 = 1,
	kArchiveCd2 = 2,
	kArchiveCd3 = 3
};

enum ChapterIndex {
	kChapterAll = 0,
	kChapter1   = 1,
	kChapter2   = 2,
	kChapter3   = 3,
	kChapter4   = 4,
	kChapter5   = 5
};

//////////////////////////////////////////////////////////////////////////
// Index of scenes
//////////////////////////////////////////////////////////////////////////
enum SceneIndex : uint {
	kSceneNone                    = 0,
	kSceneMenu                    = 1,

	kSceneIntro                   = 30,

	// Inventory
	kSceneMatchbox                = 31,
	kSceneTelegram                = 32,
	kScenePassengerList           = 33,
	kSceneScarf                   = 34,
	kSceneParchemin               = 35,
	kSceneArticle                 = 36,
	kScenePaper                   = 37,
	kSceneFirebird                = 38,
	kSceneBriefcase               = 39,

	// Normal scenes
	kSceneDefault                 = 40,
	kScene41                      = 41,
	kSceneCompartmentCorpse       = 42,     // Tyler compartment with corpse on floor

	// Fight
	kSceneFightMilos              = 43,
	kSceneFightMilosBedOpened     = 44,
	kSceneFightAnna               = 45,
	kSceneFightIvo                = 46,
	kSceneFightSalko              = 47,
	kSceneFightVesna              = 48,

	kSceneEuropeMap               = 49,

	// Game over
	kSceneGameOverStopPolice      = 50,
	kSceneGameOverTrainStopped    = 51,
	kSceneGameOverTrainStopped2   = 52,
	kSceneGameOverTrainExplosion  = 53,
	kSceneGameOverTrainExplosion2 = 54,
	kSceneGameOverBloodJacket     = 55,
	kSceneGameOverPolice          = 56,
	kSceneGameOverPolice1         = 57,
	kSceneGameOverAnnaDied        = 58,
	kSceneGameOverVienna          = 59,
	kSceneGameOverVienna1         = 60,
	kSceneGameOverVienna2         = 61,
	kSceneGameOverAlarm           = 62,
	kSceneGameOverPolice2         = 63,
	kSceneGameOverAlarm2          = 64,

	// Start screen
	kSceneStartScreen             = 65,

	kSceneBeetle                  = 128,

	kSceneFightDefault            = 820,

	kSceneInvalid                 = 0xffffffff
};

//////////////////////////////////////////////////////////////////////////
// Jacket
//////////////////////////////////////////////////////////////////////////
enum JacketType {
	kJacketOriginal   = 0,
	kJacketBlood      = 1,
	kJacketGreen      = 2
};

//////////////////////////////////////////////////////////////////////////
// City
//////////////////////////////////////////////////////////////////////////
enum CityIndex {
	kCityEpernay = 0,
	kCityChalons,
	kCityBarleduc,
	kCityNancy,
	kCityLuneville,
	kCityAvricourt,                // 5
	kCityDeutschAvricourt,
	kCityStrasbourg,
	kCityBadenOos,
	kCitySalzbourg,
	kCityAttnangPuchheim,          // 10
	kCityWels,
	kCityLinz,
	kCityVienna,
	kCityPoszony,
	kCityGalanta,                  // 15
	kCityPolice
};

//////////////////////////////////////////////////////////////////////////
// Savegame ID
//////////////////////////////////////////////////////////////////////////
enum GameId {
	kGameBlue = 0,
	kGameRed,
	kGameGreen,
	kGamePurple,
	kGameTeal,
	kGameGold
};

enum SavegameType {
	kSavegameTypeIndex = 0,
	kSavegameTypeTime = 1,
	kSavegameTypeEvent = 2,
	kSavegameTypeEvent2 = 3,
	kSavegameTypeAuto = 4,
	kSavegameTypeTickInterval = 5
};

//////////////////////////////////////////////////////////////////////////
// Cursor style
//////////////////////////////////////////////////////////////////////////
enum CursorStyle {
	kCursorNormal,
	kCursorForward,
	kCursorBackward,
	kCursorTurnRight,
	kCursorTurnLeft,
	kCursorUp,
	kCursorDown,
	kCursorLeft,
	kCursorRight,
	kCursorHand,
	kCursorHandKnock,                     // 10
	kCursorMagnifier,
	kCursorHandPointer,
	kCursorSleep,
	kCursorTalk,
	kCursorTalk2,     // Need better name

	// Items
	kCursorMatchBox,
	kCursorTelegram,
	kCursorPassengerList,
	kCursorArticle,
	kCursorScarf,     // 20
	kCursorPaper,
	kCursorParchemin,
	kCursorMatch,
	kCursorWhistle,
	kCursorKey,
	kCursorBomb,
	kCursorFirebird,
	kCursorBriefcase,
	kCursorCorpse,

	// Combat
	kCursorPunchLeft,                      // 30
	kCursorPunchRight,

	// Portraits
	kCursorPortrait,                      // 32
	kCursorPortraitSelected,
	kCursorPortraitGreen,
	kCursorPortraitGreenSelected,
	kCursorPortraitYellow,
	kCursorPortraitYellowSelected,
	kCursorHourGlass,
	kCursorEggBlue,
	kCursorEggRed,                        // 40
	kCursorEggGreen,
	kCursorEggPurple,
	kCursorEggTeal,
	kCursorEggGold,
	kCursorEggClock,
	kCursorNormal2,
	kCursorBlank,
	kCursorMAX,

	// Special
	kCursorProcess = 128,
	kCursorKeepValue = 255
};

//////////////////////////////////////////////////////////////////////////
// Position - should be between 0 & 100
//////////////////////////////////////////////////////////////////////////
typedef unsigned char PositionOld;

//////////////////////////////////////////////////////////////////////////
// EntityPosition
//////////////////////////////////////////////////////////////////////////
enum EntityPosition {
	kPositionNone     = 0,
	kPosition_1     = 1,
	kPosition_3     = 3,
	kPosition_4     = 4,
	kPosition_500   = 500,
	kPosition_540   = 540,
	kPosition_750   = 750,
	kPosition_849   = 849,
	kPosition_850   = 850,
	kPosition_851   = 851,
	kPosition_1200  = 1200,
	kPosition_1430  = 1430,
	kPosition_1500  = 1500,
	kPosition_1540  = 1540,
	kPosition_1750  = 1750,
	kPosition_2000  = 2000,
	kPosition_2087  = 2087,
	kPosition_2086  = 2086,
	kPosition_2088  = 2088,
	kPosition_2110  = 2110,
	kPosition_2300  = 2300,
	kPosition_2330  = 2330,
	kPosition_2410  = 2410,
	kPosition_2436  = 2436,
	kPosition_2490  = 2490,
	kPosition_2500  = 2500,
	kPosition_2587  = 2587,
	kPosition_2588  = 2588,
	kPosition_2690  = 2690,
	kPosition_2740  = 2740,
	kPosition_2830  = 2830,
	kPosition_2980  = 2980,
	kPosition_3050  = 3050,
	kPosition_3110  = 3110,
	kPosition_3390  = 3390,
	kPosition_3450  = 3450,
	kPosition_3500  = 3500,
	kPosition_3550  = 3550,
	kPosition_3650  = 3650,
	kPosition_3760  = 3760,
	kPosition_3820  = 3820,
	kPosition_3890  = 3890,
	kPosition_3969  = 3969,
	kPosition_3970  = 3970,
	kPosition_4070  = 4070,
	kPosition_4100  = 4100,
	kPosition_4370  = 4370,
	kPosition_4455  = 4455,
	kPosition_4460  = 4460,
	kPosition_4500  = 4500,
	kPosition_4590  = 4590,
	kPosition_4680  = 4680,
	kPosition_4689  = 4689,
	kPosition_4690  = 4690,
	kPosition_4691  = 4691,
	kPosition_4770  = 4470,
	kPosition_4840  = 4840,
	kPosition_5000  = 5000,
	kPosition_5090  = 5090,
	kPosition_5140  = 5140,
	kPosition_5419  = 5419,
	kPosition_5420  = 5420,
	kPosition_5440  = 5440,
	kPosition_5500  = 5500,
	kPosition_5540  = 5540,
	kPosition_5610  = 5610,
	kPosition_5790  = 5790,
	kPosition_5799  = 5799,
	kPosition_5800  = 5800,
	kPosition_5810  = 5810,
	kPosition_5890  = 5890,
	kPosition_5900  = 5900,
	kPosition_5970  = 5970,
	kPosition_6000  = 6000,
	kPosition_6130  = 6130,
	kPosition_6160  = 6160,
	kPosition_6220  = 6220,
	kPosition_6410  = 6410,
	kPosition_6460  = 6460,
	kPosition_6469  = 6469,
	kPosition_6470  = 6470,
	kPosition_6471  = 6471,
	kPosition_6800  = 6800,
	kPosition_6850  = 6850,
	kPosition_7000  = 7000,
	kPosition_7160  = 7160,
	kPosition_7250  = 7250,
	kPosition_7320  = 7320,
	kPosition_7500  = 7500,
	kPosition_7510  = 7510,
	kPosition_7850  = 7850,
	kPosition_7870  = 7870,
	kPosition_7900  = 7900,
	kPosition_7950  = 7950,
	kPosition_8000  = 8000,
	kPosition_8012  = 8012,
	kPosition_8013  = 8013,
	kPosition_8160  = 8160,
	kPosition_8200  = 8200,
	kPosition_8500  = 8500,
	kPosition_8512  = 8512,
	kPosition_8513  = 8513,
	kPosition_8514  = 8514,
	kPosition_8800  = 8800,
	kPosition_9020  = 9020,
	kPosition_9269  = 9269,
	kPosition_9250  = 9250,
	kPosition_9270  = 9270,
	kPosition_9271  = 9271,
	kPosition_9460  = 9460,
	kPosition_9500  = 9500,
	kPosition_9510  = 9510,
	kPosition_30000 = 30000
};

//////////////////////////////////////////////////////////////////////////
// Location
//////////////////////////////////////////////////////////////////////////
enum Location {
	kLocationOutsideCompartment = 0,
	kLocationInsideCompartment = 1,
	kLocationOutsideTrain = 2
};

//////////////////////////////////////////////////////////////////////////
// Car
//////////////////////////////////////////////////////////////////////////
enum CarIndex {
	kCarNone = 0,
	kCarBaggageRear = 1,
	kCarKronos = 2,
	kCarGreenSleeping = 3,
	kCarRedSleeping = 4,
	kCarRestaurant = 5,
	kCarBaggage = 6,
	kCarCoalTender = 7,
	kCarLocomotive = 8,
	kCarVestibule = 9
};

//////////////////////////////////////////////////////////////////////////
// Clothes
//////////////////////////////////////////////////////////////////////////
enum ClothesIndex {
	kClothesDefault = 0,
	kClothes1 = 1,
	kClothes2 = 2,
	kClothes3 = 3,

	kClothesInvalid
};

//////////////////////////////////////////////////////////////////////////
// Objects (doors)
//////////////////////////////////////////////////////////////////////////
enum ObjectLocation {
	kObjectLocationNone = 0,
	kObjectLocation1    = 1, // Floor?
	kObjectLocation2    = 2, // Bed ?
	kObjectLocation3    = 3,
	kObjectLocation4    = 4, // Window ?
	kObjectLocation5 = 5,
	kObjectLocation6 = 6,
	kObjectLocation7 = 7,
	kObjectLocation8 = 8,
	kObjectLocation9 = 9,
	kObjectLocation10 = 10,
	kObjectLocation18 = 18
};

enum ObjectModel {
	kObjectModelNone = 0,
	kObjectModel1    = 1,
	kObjectModel2    = 2,
	kObjectModel3    = 3,
	kObjectModel4    = 4,
	kObjectModel5    = 5,
	kObjectModel6    = 6,
	kObjectModel7    = 7,
	kObjectModel8    = 8,
	kObjectModel9    = 9,
	kObjectModel10   = 10
};

//////////////////////////////////////////////////////////////////////////
// Entity direction
//////////////////////////////////////////////////////////////////////////
enum EntityDirection {
	kDirectionNone   = 0,
	kDirectionUp     = 1,
	kDirectionDown   = 2,
	kDirectionLeft   = 3,
	kDirectionRight  = 4,
	kDirectionSwitch = 5
};

//////////////////////////////////////////////////////////////////////////
// Combat
//////////////////////////////////////////////////////////////////////////
enum FightType {
	kFightMilos   = 2001,
	kFightAnna    = 2002,
	kFightIvo     = 2003,
	kFightSalko   = 2004,
	kFightVesna   = 2005
};

//////////////////////////////////////////////////////////////////////////
// Index of items in inventory data
//////////////////////////////////////////////////////////////////////////
enum InventoryItem {
	kItemNone,
	kItemMatchBox,
	kItem2,
	kItem3,
	kItemTelegram,
	kItem5,               // 5
	kItemPassengerList,
	kItem7,
	kItemScarf,
	kItem9,
	kItemParchemin,       // 10
	kItem11,
	kItemMatch,
	kItemWhistle,
	kItemBeetle,
	kItemKey,             // 15
	kItemBomb,
	kItem17,
	kItemFirebird,
	kItemBriefcase,
	kItemCorpse,          // 20
	kItemGreenJacket,
	kItem22,
	kItemPaper,
	kItemArticle,
	kItem25,             // 25
	kItem26,
	kItem27,
	kItem28,
	kItem29,
	kItem30,            // 30
	kItem31,

	// Portrait (not an index)
	kPortraitOriginal  = 32,
	kPortraitGreen     = 34,
	kPortraitYellow    = 36,

	kItemInvalid       = 128,

	kItem146           = 146,
	kItem147           = 147,

	// Toggles
	kItemToggleHigh    = 0x7F,
	kItemToggleLow     = 0xF7
};

//////////////////////////////////////////////////////////////////////////
// Door ID
//////////////////////////////////////////////////////////////////////////
enum ObjectIndex {
	kObjectNone,
	kObjectCompartment1,
	kObjectCompartment2,
	kObjectCompartment3,
	kObjectCompartment4,
	kObjectCompartment5,                 // 5
	kObjectCompartment6,
	kObjectCompartment7,
	kObjectCompartment8,
	kObjectOutsideTylerCompartment,
	kObject10,                           // 10
	kObject11,
	kObject12,
	kObject13,
	kObject14,
	kObject15,                           // 15
	kObject16,
	kObjectHandleBathroom,
	kObjectHandleInsideBathroom,
	kObjectKitchen,
	kObject20,                          // 20
	kObject21,
	kObject22,
	kObjectTrainTimeTable,
	kObjectRedSleepingCar,
	kObject25,                          // 25
	kObjectHandleOutsideLeft,
	kObjectHandleOutsideRight,
	kObject28,
	kObject29,
	kObject30,                          // 30
	kObject31,
	kObjectCompartmentA,
	kObjectCompartmentB,
	kObjectCompartmentC,
	kObjectCompartmentD,                // 35
	kObjectCompartmentE,
	kObjectCompartmentF,
	kObjectCompartmentG,
	kObjectCompartmentH,
	kObject40,                          // 40
	kObject41,
	kObject42,
	kObject43,
	kObjectOutsideBetweenCompartments,
	kObjectOutsideAnnaCompartment,     // 45
	kObject46,
	kObject47,
	kObject48, // might be the egg
	kObject49,
	kObject50,                          // 50
	kObject51,
	kObject52,
	kObject53,
	kObject54,
	kObjectRestaurantCar,               // 55
	kObject56,
	kObject57,
	kObject58,
	kObject59,
	kObject60,                          // 60
	kObject61,
	kObject62,
	kObject63,
	kObject64,
	kObject65,                          // 65
	kObject66,
	kObject67,
	kObject68,
	kObject69,
	kObject70,                          // 70
	kObject71,
	kObject72,
	kObjectCeiling,
	kObject74,
	kObjectCompartmentKronos,           // 75
	kObject76,
	kObject77,
	kObject78,
	kObject79,
	kObject80,                          // 80
	kObject81,
	kObject82,
	kObject83,
	kObject84,
	kObject85,                          // 85
	kObject86,
	kObject87,
	kObject88,
	kObject89,
	kObject90,                          // 90
	kObject91,
	kObject92,
	kObject93,
	kObject94,
	kObject95,                          // 95
	kObject96,
	kObject97,
	kObject98,
	kObject99,
	kObject100,                         // 100
	kObject101,
	kObject102,
	kObject103,
	kObject104,
	kObject105,                         // 105
	kObject106,
	kObject107,
	kObject108,
	kObjectCageMax,
	kObject110,                         // 110
	kObject111,
	kObject112,
	kObject113,
	kObject114,
	kObject115,                         // 115
	kObject116,
	kObject117,
	kObject118,
	kObject119,
	kObject120,                         // 120
	kObject121,
	kObject122,
	kObject123,
	kObject124,
	kObject125,                         // 125
	kObject126,
	kObject127,
	kObjectMax
};

//////////////////////////////////////////////////////////////////////////
// Character ID
//////////////////////////////////////////////////////////////////////////
enum CharacterIndex {
	kCharacterCath,
	kCharacterAnna,
	kCharacterAugust,
	kCharacterCond1,
	kCharacterCond2,
	kCharacterHeadWait,          // 5
	kCharacterWaiter1,
	kCharacterWaiter2,
	kCharacterCook,
	kCharacterTrainM,
	kCharacterTatiana,           // 10
	kCharacterVassili,
	kCharacterAlexei,
	kCharacterAbbot,
	kCharacterMilos,
	kCharacterVesna,             // 15
	kCharacterIvo,
	kCharacterSalko,
	kCharacterKronos,
	kCharacterKahina,
	kCharacterFrancois,          // 20
	kCharacterMadame,
	kCharacterMonsieur,
	kCharacterRebecca,
	kCharacterSophie,
	kCharacterMahmud,            // 25
	kCharacterYasmin,
	kCharacterHadija,
	kCharacterAlouan,
	kCharacterPolice,
	kCharacterMax,               // 30
	kCharacterMaster,
	kCharacterClerk,
	kCharacterTableA,
	kCharacterTableB,
	kCharacterTableC,            // 35
	kCharacterTableD,
	kCharacterTableE,
	kCharacterTableF,
	kCharacterMitchell,

	kCharacterSteam = 255
};

//////////////////////////////////////////////////////////////////////////
// Events
//   - a single D at the end means that Cath is on the right of the "scene" (D = Down the train, U = Up the train)
//   - DD: during the day, coming down the train
//   - DU: during the day, coming up the train
//   - ND: during the night, coming down the train
//   - NU: during the night, coming up the train
//////////////////////////////////////////////////////////////////////////
enum EventIndex : uint {
	kEventNone = 0,
	kEventGotALight = 1,
	kEventGotALightD = 2,
	kEventDinerMindJoin = 3,
	kEventDinerAugustOriginalJacket = 4,
	kEventDinerAugust = 5,
	kEventDinerAugustAlexeiBackground = 6,
	kEventMeetAugustTylerCompartment = 7,
	kEventMeetAugustTylerCompartmentBed = 8,
	kEventMeetAugustHisCompartment = 9,
	kEventMeetAugustHisCompartmentBed = 10,
	kEventAugustFindCorpse = 11,
	kEventAugustPresentAnna = 12,
	kEventAugustPresentAnnaFirstIntroduction = 13,
	kEventAnnaIntroductionRejected = 14,
	kEventAnnaConversationGoodNight = 15,
	kEventAnnaVisitToCompartmentGun = 16,
	kEventInvalid_17 = 17,
	kEventAnnaGoodNight = 18,
	kEventAnnaGoodNightInverse = 19,
	kEventAugustGoodMorning = 20,
	kEventAugustMerchandise = 21,
	kEventAugustTalkGold = 22,
	kEventAugustTalkGoldDay = 23,
	kEventAugustTalkCompartmentDoor = 24,
	kEventAugustTalkCompartmentDoorBlueRedingote = 25,
	kEventAugustLunch = 26,
	kEventKronosVisit = 27,
	kEventAnnaSearchingCompartment = 28,
	kEventAugustBringEgg = 29,
	kEventAugustBringBriefcase = 30,
	kEventAugustTalkCigar = 31,
	kEventAnnaBaggageArgument = 32,
	kEventAnnaBaggagePart2 = 33,
	kEventAnnaConversation_34 = 34,
	kEventAugustDrink = 35,
	kEventAnnaTired = 36,
	kEventAnnaTiredKiss = 37,
	kEventAnnaBaggageTies = 38,
	kEventAnnaBaggageTies2 = 39,
	kEventAnnaBaggageTies3 = 40,
	kEventAnnaBaggageTies4 = 41,
	kEventAugustUnhookCarsBetrayal = 42,
	kEventAugustUnhookCars = 43,
	kEventLocomotiveAnnaStopsTrain = 44,
	kEventInvalid_45 = 45,
	kEventTrainStopped = 46,
	kEventAnnaKissTrainHijacked = 47,
	kEventTrainHijacked = 48,
	kEventAnnaKilled = 49,
	kEventKronosGoingToInvitation = 50,
	kEventKronosConversation = 51,
	kEventKahinaAskSpeakFirebird = 52,
	kEventKahinaAskSpeak = 53,
	kEventKronosConversationFirebird = 54,
	kEventKahinaGunYellow = 55,
	kEventKahinaGunBlue = 56,
	kEventKahinaGun = 57,
	kEventKronosBringEggCeiling = 58,
	kEventKronosBringEgg = 59,
	kEventKronosBringNothing = 60,
	kEventKronosReturnBriefcase = 61,
	kEventKronosHostageAnna = 62,
	kEventKronosGiveFirebird = 63,
	kEventKahinaPunchBaggageCarEntrance = 64,
	kEventKahinaPunchBlue = 65,
	kEventKahinaPunchYellow = 66,
	kEventKahinaPunchSalon = 67,
	kEventKahinaPunchKitchen = 68,
	kEventKahinaPunchBaggageCar = 69,
	kEventKahinaPunchCar = 70,
	kEventKahinaPunchSuite4 = 71,
	kEventKahinaPunchRestaurant = 72,
	kEventKronosHostageAnnaNoFirebird = 73,
	kEventKahinaPunch = 74,
	kEventKahinaWrongDoor = 75,
	kEventAlexeiDiner = 76,
	kEventAlexeiDinerOriginalJacket = 77,
	kEventAlexeiSalonVassili = 78,
	kEventAlexeiSalonCath = 79,
	kEventAlexeiSalonPoem = 80,
	kEventTatianaAskMatchSpeakRussian = 81,
	kEventTatianaAskMatch = 82,
	kEventTatianaGivePoem = 83,
	kEventVassiliSeizure = 84,
	kEventTatianaBreakfastAlexei = 85,
	kEventTatianaBreakfast = 86,
	kEventTatianaBreakfastGivePoem = 87,
	kEventTatianaAlexei = 88,
	kEventTatianaCompartmentStealEgg = 89,
	kEventTatianaCompartment = 90,
	kEventVassiliCompartmentStealEgg = 91,
	kEventTatianaTylerCompartment = 92,
	kEventTylerCastleDream= 93,
	kEventVassiliDeadAlexei = 94,
	kEventCathFreePassengers = 95,
	kEventTatianaVassiliTalk = 96,
	kEventTatianaVassiliTalkNight = 97,
	kEventMilosTylerCompartmentVisit = 98,
	kEventMilosTylerCompartmentBedVisit = 99,
	kEventMilosTylerCompartment = 100,
	kEventMilosTylerCompartmentBed = 101,
	kEventMilosTylerCompartmentDefeat = 102,
	kEventMilosCorpseFloor = 103,
	kEventMilosCompartmentVisitAugust = 104,
	kEventMilosCorridorThanks = 105,
	kEventMilosCorridorThanksD = 106,
	kEventMilosCompartmentVisitTyler = 107,
	kEventLocomotiveMilosDay = 108,
	kEventLocomotiveMilosNight = 109,
	kEventAbbotIntroduction = 110,
	kEventAbbotWrongCompartment = 111,
	kEventAbbotWrongCompartmentBed = 112,
	kEventAbbotInvitationDrink = 113,
	kEventAbbotDrinkGiveDetonator = 114,
	kEventTrainExplosionBridge = 115,
	kEventDefuseBomb = 116,
	kEventAbbotDrinkDefuse = 117,
	kEventMertensLastCar = 118,
	kEventMertensLastCarOriginalJacket = 119,
	kEventMertensKronosInvitation = 120,
	kEventMertensKronosInvitationCompartment = 121,
	kEventMertensKronosInvitationClosedWindows = 122,
	kEventMertensBloodJacket = 123,
	kEventCoudertBloodJacket = 124,
	kEventMertensCorpseFloor = 125,
	kEventMertensCorpseBed = 126,
	kEventMertensDontMakeBed = 127,
	kEventInvalid_128 = 128,
	kEventGendarmesArrestation = 129,
	kEventVergesSuitcase = 130,
	kEventVergesSuitcaseStart = 131,
	kEventVergesSuitcaseOtherEntry = 132,
	kEventVergesSuitcaseOtherEntryStart = 133,
	kEventVergesSuitcaseNight = 134,
	kEventVergesSuitcaseNightStart = 135,
	kEventVergesSuitcaseNightOtherEntry = 136,
	kEventVergesSuitcaseNightOtherEntryStart = 137,
	kEventMertensAskTylerCompartment = 138,
	kEventMertensAskTylerCompartmentD = 139,
	kEventMertensPushCallNight = 140,
	kEventMertensPushCall = 141,
	kEventMertensAugustWaiting = 142,
	kEventMertensAugustWaitingCompartment = 143,
	kEventIntroBroderbrund = 144,
	kEventCoudertAskTylerCompartment = 145,
	kEventMertensKronosConcertInvitation = 146,
	kEventCoudertGoingOutOfVassiliCompartment = 147,
	kEventLocomotiveConductorsDiscovered = 148,
	kEventLocomotiveConductorsLook = 149,
	kEventMahmudWrongDoor = 150,
	kEventMahmudWrongDoorOriginalJacket = 151,
	kEventMahmudWrongDoorDay = 152,
	kEventVergesEscortToDiningCar = 153,
	kEventVergesBaggageCarOffLimits = 154,
	kEventVergesCanIHelpYou = 155,
	kEventCoudertBaggageCar = 156,
	kEventCathTurningDay = 157,
	kEventCathTurningNight = 158,
	kEventIntro = 159,
	kEventCathDream = 160,
	kEventCorpseDropBridge = 161,
	kEventTrainPassing = 162,
	kEventVergesAnnaDead = 163,
	kEventViennaAugustUnloadGuns = 164,
	kEventViennaKronosFirebird = 165,
	kEventViennaContinueGame = 166,
	kEventCathVesnaRestaurantKilled = 167,
	kEventCathMaxCage = 168,
	kEventCathMaxFree = 169,
	kEventCathMaxLickHand = 170,
	kEventCathIvoFight = 171,
	kEventCathSalkoTrainTopFight = 172,
	kEventCathVesnaTrainTopFight = 173,
	kEventCathVesnaTrainTopKilled = 174,
	kEventCathVesnaTrainTopWin = 175,
	kEventCathSalkoTrainTopWin = 176,
	kEventFrancoisWhistle = 177,
	kEventFrancoisWhistleD = 178,
	kEventFrancoisWhistleNight = 179,
	kEventFrancoisWhistleNightD = 180,
	kEventFrancoisShowBeetle = 181,
	kEventFrancoisShowBeetleD = 182,
	kEventFrancoisTradeWhistle = 183,
	kEventFrancoisTradeWhistleD = 184,
	kEventFrancoisShowEgg = 185,
	kEventFrancoisShowEggD = 186,
	kEventFrancoisShowEggNightD = 187,
	kEventFrancoisShowEggNight = 188,
	kEventKronosBringFirebird = 189,
	kEventKronosOpenFirebird = 190,
	kEventFinalSequence = 191,
	kEventLocomotiveRestartTrain = 192,
	kEventLocomotiveOldBridge = 193,
	kEventLocomotiveAbbotGetSomeRest = 194,
	kEventLocomotiveAbbotShoveling = 195,
	kEventLocomotiveMilosShovelingDay = 196,
	kEventLocomotiveMilosShovelingNight = 197,
	kEventAnnaGiveScarf = 198,
	kEventAnnaGiveScarfDiner = 199,
	kEventAnnaGiveScarfSalon = 200,
	kEventAnnaGiveScarfMonogram = 201,
	kEventAnnaGiveScarfDinerMonogram = 202,
	kEventAnnaGiveScarfSalonMonogram = 203,
	kEventAnnaGiveScarfAsk = 204,
	kEventAnnaGiveScarfDinerAsk = 205,
	kEventAnnaGiveScarfSalonAsk = 206,
	kEventAugustArrivalInMunich = 207,
	kEventAnnaDialogGoToJerusalem = 208,
	kEventConcertStart = 209,
	kEventConcertEnd = 210,
	kEventCathFallingAsleep = 211,
	kEventCathWakingUp = 212,
	kEventConcertCough = 213,
	kEventConcertSit = 214,
	kEventConcertLeaveWithBriefcase = 215,
	kEventCorpseDropFloorOriginal = 216,
	kEventCorpseDropFloorGreen = 217,
	kEventCorpsePickFloorOriginal = 218,
	kEventCorpsePickFloorGreen = 219,
	kEventCorpsePickFloorOpenedBedOriginal = 220,
	kEventCorpsePickBedOriginal = 221,
	kEventCorpsePickBedGreen = 222,
	kEventCorpseDropBedOriginal = 223,
	kEventCorpseDropBedGreen = 224,
	kEventCorpseDropWindowOriginal = 225,
	kEventCorpseDropWindowGreen = 226,
	kEventCathFindCorpse = 227,
	kEventCathLookOutsideWindowDay = 228,
	kEventCathLookOutsideWindowNight = 229,
	kEventCathGoOutsideTylerCompartmentDay = 230,
	kEventCathGoOutsideTylerCompartmentNight = 231,
	kEventCathGoOutsideDay = 232,
	kEventCathGoOutsideNight = 233,
	kEventCathSlipTylerCompartmentDay = 234,
	kEventCathSlipTylerCompartmentNight = 235,
	kEventCathSlipDay = 236,
	kEventCathSlipNight = 237,
	kEventCathGetInsideTylerCompartmentDay = 238,
	kEventCathGetInsideTylerCompartmentNight = 239,
	kEventCathGetInsideDay = 240,
	kEventCathGetInsideNight = 241,
	kEventCathGettingInsideAnnaCompartment = 242,
	kEventCathClimbUpTrainGreenJacket = 243,
	kEventCathClimbUpTrainNoJacketNight = 244,
	kEventCathClimbUpTrainNoJacketDay = 245,
	kEventCathClimbDownTrainGreenJacket = 246,
	kEventCathClimbDownTrainNoJacketNight = 247,
	kEventCathClimbDownTrainNoJacketDay= 248,
	kEventCathTopTrainGreenJacket = 249,
	kEventCathTopTrainNoJacketNight = 250,
	kEventCathTopTrainNoJacketDay = 251,
	kEventCathBreakCeiling = 252,
	kEventCathJumpDownCeiling = 253,
	kEventCathJumpUpCeilingBriefcase = 254,
	kEventCathJumpUpCeiling = 255,
	kEventPickGreenJacket = 256,
	kEventPickScarfGreen = 257,
	kEventPickScarfOriginal = 258,
	kEventCloseMatchbox = 259,
	kEventCathStruggleWithBonds = 260,
	kEventCathBurnRope = 261,
	kEventCathRemoveBonds = 262,
	kEventCathStruggleWithBonds2 = 263,
	kEventCathDefusingBomb = 264,
	kEventCathSmokeNight = 265,
	kEventCathSmokeDay = 266,
	kEventCathOpenEgg = 267,
	kEventCathOpenEggNoBackground = 268,
	kEventCathCloseEgg = 269,
	kEventCathCloseEggNoBackground = 270,
	kEventCathUseWhistleOpenEgg = 271,
	kEventCathUseWhistleOpenEggNoBackground = 272
};

//////////////////////////////////////////////////////////////////////////
// Character Actions
//////////////////////////////////////////////////////////////////////////
enum CharacterActions : uint {
	kCharacterActionNone            = 0,
	kCharacterAction1               = 1,
	kCharacterActionEndSound        = 2,
	kCharacterActionExitCompartment = 3,
	kCharacterAction4               = 4,
	kCharacterActionExcuseMeCath    = 5,
	kCharacterActionExcuseMe        = 6,
	kCharacterActionKnock           = 8,
	kCharacterActionOpenDoor        = 9,
	kCharacterAction10              = 10,
	kCharacterAction11              = 11,
	kCharacterActionDefault         = 12,
	kCharacterAction16              = 16,
	kCharacterActionDrawScene       = 17,
	kCharacterActionCallback        = 18,

	/////////////////////////////
	// Abbot
	/////////////////////////////
	kCharacterAction100969180 = 100969180,    // Anna
	kCharacterAction101169422 = 101169422,
	kCharacterAction104060776 = 104060776,
	kCharacterAction135600432 = 135600432,
	kCharacterAction136196244 = 136196244,
	kCharacterAction157159392 = 157159392,
	kCharacterAction157489665 = 157489665,
	kCharacterAction158480160 = 158480160,
	kCharacterAction192054567 = 192054567,
	kCharacterAction203073664 = 203073664,
	kCharacterAction222609266 = 222609266,

	/////////////////////////////
	// Alexei
	/////////////////////////////
	kCharacterAction100906246 = 100906246,
	kCharacterAction123536024 = 123536024,
	kCharacterAction124697504 = 124697504,
	kCharacterAction135664192 = 135664192,
	kCharacterAction135854208 = 135854208,
	kCharacterAction188784532 = 188784532,
	kCharacterAction221617184 = 221617184,

	/////////////////////////////
	// Alouan
	/////////////////////////////
	kCharacterAction189489753 = 189489753,
	kCharacterAction190219584 = 190219584,    // Francois

	/////////////////////////////
	// Anna
	/////////////////////////////
	kCharacterAction136702400 = 136702400,
	kCharacterAction139254416 = 139254416,
	kCharacterAction156049968 = 156049968,
	kCharacterAction157370960 = 157370960,
	kCharacterAction157894320 = 157894320,
	kCharacterAction159332865 = 159332865,   // August
	kCharacterAction189299008 = 189299008,
	kCharacterAction191668032 = 191668032,   // some action during or before concert?
	kCharacterAction201437056 = 201437056,
	kCharacterAction235856512 = 235856512,
	kCharacterAction236060709 = 236060709,
	kCharacterAction238936000 = 238936000,
	kCharacterAction259136835 = 259136835,
	kCharacterAction291662081 = 291662081,


	/////////////////////////////
	// August
	/////////////////////////////
	kCharacterAction123793792 = 123793792,
	kCharacterAction134611040 = 134611040,
	kCharacterAction168046720 = 168046720,
	kCharacterAction168627977 = 168627977,
	kCharacterAction169032608 = 169032608,
	kCharacterAction189426612 = 189426612,
	kCharacterAction203859488 = 203859488,
	kCharacterAction219522616 = 219522616,    // Waiter1
	kCharacterAction225182640 = 225182640,
	kCharacterAction235257824 = 235257824,

	/////////////////////////////
	// Boutarel
	/////////////////////////////
	kCharacterAction125039808 = 125039808,
	kCharacterAction134466544 = 134466544,
	kCharacterAction135854206 = 135854206,
	kCharacterAction159003408 = 159003408,
	kCharacterAction203520448 = 203520448,
	kCharacterAction237889408 = 237889408,

	/////////////////////////////
	// Chapters
	/////////////////////////////
	kCharacterAction135800432 = 135800432,
	kCharacterActionChapter3  = 139122728,
	kCharacterActionChapter5  = 139254416,
	kCharacterAction156435676 = 156435676,
	kCharacterAction169629818 = 169629818,
	kCharacterAction171843264 = 171843264,
	kCharacterAction190346110 = 190346110,

	/////////////////////////////
	// Cooks
	/////////////////////////////
	kCharacterAction101632192 = 101632192,
	kCharacterAction224849280 = 224849280,
	kCharacterAction236976550 = 236976550,

	/////////////////////////////
	// Coudert
	/////////////////////////////
	kCharacterAction123733488 = 123733488,
	kCharacterAction154005632 = 154005632,
	kCharacterAction155991520 = 155991520,
	kCharacterAction157026693 = 157026693,
	kCharacterAction168253822 = 168253822,
	kCharacterAction168254872 = 168254872,
	kCharacterAction168316032 = 168316032,    // Tatiana
	kCharacterAction169557824 = 169557824,
	kCharacterAction171394341 = 171394341,    // Mertens
	kCharacterAction185671840 = 185671840,
	kCharacterAction185737168 = 185737168,
	kCharacterAction188570113 = 188570113,
	kCharacterAction189026624 = 189026624,
	kCharacterAction189750912 = 189750912,
	kCharacterAction192063264 = 192063264,    // Anna
	kCharacterAction201431954 = 201431954,    // Mertens / Verges
	kCharacterAction201439712 = 201439712,
	kCharacterAction205033696 = 205033696,
	kCharacterAction205346192 = 205346192,    // Francois
	kCharacterAction219971920 = 219971920,    // Anna
	kCharacterAction223068211 = 223068211,    // MmeBoutarel
	kCharacterAction225932896 = 225932896,
	kCharacterAction226031488 = 226031488,    // Verges
	kCharacterAction235061888 = 235061888,    // Tatiana
	kCharacterAction238358920 = 238358920,    // Anna
	kCharacterAction253868128 = 253868128,    // Anna
	kCharacterAction285528346 = 285528346,    // Rebecca
	kCharacterAction292048641 = 292048641,
	kCharacterAction305159806 = 305159806,
	kCharacterAction326348944 = 326348944,
	kCharacterAction339669520 = 339669520,    // Verges

	/////////////////////////////
	// Francois
	/////////////////////////////
	kCharacterAction100901266 = 100901266,
	kCharacterAction100957716 = 100957716,
	kCharacterAction101107728 = 101107728,
	kCharacterAction189872836 = 189872836,
	kCharacterAction190390860 = 190390860,

	/////////////////////////////
	// Gendarmes
	/////////////////////////////
	kCharacterAction168710784 = 168710784,
	kCharacterAction169499649 = 169499649,

	/////////////////////////////
	// Kahina
	/////////////////////////////
	kCharacterAction92186062  = 92186062,
	kCharacterAction137503360 = 137503360,
	kCharacterAction237555748 = 237555748,

	/////////////////////////////
	// Kronos
	/////////////////////////////
	kCharacterAction137685712 = 137685712,
	kCharacterAction138085344 = 138085344,
	kCharacterAction171849314 = 171849314,
	kCharacterAction235599361 = 235599361,

	/////////////////////////////
	// Mahmud
	/////////////////////////////
	kCharacterAction102227384 = 102227384,    // Mertens
	kCharacterAction156567128 = 156567128,
	kCharacterAction170483072 = 170483072,
	kCharacterAction225563840 = 225563840,

	/////////////////////////////
	// Max
	/////////////////////////////
	kCharacterAction71277948  = 71277948,
	kCharacterAction158007856 = 158007856,
	kCharacterAction101687594 = 101687594,
	kCharacterAction122358304 = 122358304,    // also Waiter2/Boutarel?
	kCharacterActionMaxFreeFromCage = 135204609,
	kCharacterAction156622016 = 156622016,

	/////////////////////////////
	// Mertens
	/////////////////////////////
	kCharacterAction155604840 = 155604840,    // MmeBoutarel
	kCharacterAction169633856 = 169633856,
	kCharacterAction188635520 = 188635520,
	kCharacterAction190082817 = 190082817,
	kCharacterAction192849856 = 192849856,
	kCharacterAction204379649 = 204379649,
	kCharacterAction224122407 = 224122407,
	kCharacterAction238732837 = 238732837,
	kCharacterAction238790488 = 238790488,    // Tatiana
	kCharacterAction269436673 = 269436673,
	kCharacterAction269624833 = 269624833,
	kCharacterAction302614416 = 302614416,
	kCharacterAction303343617 = 303343617,

	/////////////////////////////
	// Milos
	/////////////////////////////
	kCharacterAction88652208 = 88652208,      // Coudert
	kCharacterAction122865568 = 122865568,
	kCharacterAction123852928 = 123852928,
	kCharacterAction123199584 = 123199584,    // Coudert
	kCharacterAction157691176 = 157691176,
	kCharacterAction208228224 = 208228224,
	kCharacterAction221683008 = 221683008,
	kCharacterAction259125998 = 259125998,

	/////////////////////////////
	// Mme Boutarel
	/////////////////////////////
	kCharacterAction102484312 = 102484312,
	kCharacterAction102752636 = 102752636,
	kCharacterAction134289824 = 134289824,
	kCharacterAction168986720 = 168986720,
	kCharacterAction202221040 = 202221040,
	kCharacterAction242526416 = 242526416,

	/////////////////////////////
	// Pascale
	/////////////////////////////
	kCharacterAction101824388 = 101824388,
	kCharacterAction136059947 = 136059947,
	kCharacterAction169750080 = 169750080,
	kCharacterAction190605184 = 190605184,
	kCharacterAction191604416 = 191604416,
	kCharacterAction207769280 = 207769280,
	kCharacterAction223262556 = 223262556,
	kCharacterAction239072064 = 239072064,
	kCharacterAction257489762 = 257489762,
	kCharacterAction269479296 = 269479296,
	kCharacterAction352703104 = 352703104,
	kCharacterAction352768896 = 352768896,

	/////////////////////////////
	// Rebecca
	/////////////////////////////
	kCharacterAction125496184 = 125496184,
	kCharacterAction155465152 = 155465152,
	kCharacterAction155980128 = 155980128,
	kCharacterAction169358379 = 169358379,
	kCharacterAction224253538 = 224253538,
	kCharacterAction254915200 = 254915200,

	/////////////////////////////
	// Salko
	/////////////////////////////
	kCharacterAction55996766  = 55996766,
	kCharacterAction101169464 = 101169464,
	kCharacterAction102675536 = 102675536,    // Ivo
	kCharacterAction136184016 = 136184016,

	/////////////////////////////
	// Servers 0
	/////////////////////////////
	kCharacterAction170016384 = 170016384,
	kCharacterAction188893625 = 188893625,
	kCharacterAction201964801 = 201964801,    // August
	kCharacterAction204704037 = 204704037,
	kCharacterAction207330561 = 207330561,
	kCharacterAction218128129 = 218128129,
	kCharacterAction218586752 = 218586752,
	kCharacterAction218983616 = 218983616,
	kCharacterAction223712416 = 223712416,
	kCharacterAction237485916 = 237485916,
	kCharacterAction252568704 = 252568704,
	kCharacterAction268773672 = 268773672,    // Anna / August
	kCharacterAction270068760 = 270068760,
	kCharacterAction270410280 = 270410280,
	kCharacterAction286403504 = 286403504,
	kCharacterAction286534136 = 286534136,
	kCharacterAction292758554 = 292758554,
	kCharacterAction304061224 = 304061224,
	kCharacterAction337548856 = 337548856,

	/////////////////////////////
	// Servers 1
	/////////////////////////////
	kCharacterAction101106391 = 101106391,
	kCharacterAction122288808 = 122288808,    // Boutarel
	kCharacterAction123712592 = 123712592,    // Ivo
	kCharacterAction125826561 = 125826561,    // August
	kCharacterAction134486752 = 134486752,    // August
	kCharacterAction168717392 = 168717392,    // Boutarel
	kCharacterAction189688608 = 189688608,
	kCharacterAction219377792 = 219377792,
	kCharacterAction223002560 = 223002560,
	kCharacterAction236237423 = 236237423,
	kCharacterAction256200848 = 256200848,
	kCharacterAction258136010 = 258136010,
	kCharacterAction269485588 = 269485588,
	kCharacterAction291721418 = 291721418,
	kCharacterAction302203328 = 302203328,
	kCharacterAction302996448 = 302996448,
	kCharacterAction326144276 = 326144276,

	/////////////////////////////
	// Sophie
	/////////////////////////////
	kCharacterActionProceedChapter5  = 70549068,
	kCharacterAction123668192 = 123668192,
	kCharacterAction125242096 = 125242096,
	kCharacterAction136654208 = 136654208,
	kCharacterAction259921280 = 259921280,
	kCharacterAction292775040 = 292775040,

	/////////////////////////////
	// Tables
	/////////////////////////////
	kCharacterActionDrawTablesWithChairs = 103798704,
	kCharacterAction136455232 = 136455232,

	/////////////////////////////
	// Tatiana
	/////////////////////////////
	kCharacterAction69239528  = 69239528,
	kCharacterAction123857088 = 123857088,
	kCharacterAction124973510 = 124973510,
	kCharacterAction154071333 = 154071333,
	kCharacterAction156444784 = 156444784,
	kCharacterAction169360385 = 169360385,
	kCharacterAction191198209 = 191198209,
	kCharacterAction223183000 = 223183000,    // August
	kCharacterAction236053296 = 236053296,    // Alexei
	kCharacterAction236241630 = 236241630,    // Anna
	kCharacterAction236517970 = 236517970,    // Anna
	kCharacterAction268620864 = 268620864,    // August
	kCharacterAction290869168 = 290869168,

	/////////////////////////////
	// Train
	/////////////////////////////
	kCharacterAction191070912 = 191070912,
	kCharacterActionTrainStopRunning = 191350523,
	kCharacterActionCatchBeetle = 202613084,
	kCharacterAction203339360 = 203339360,
	kCharacterActionTrainStartRunning = 203419131,
	kCharacterAction203863200 = 203863200,
	kCharacterAction222746496 = 222746496,
	kCharacterActionBreakCeiling = 225056224,
	kCharacterAction290410610 = 290410610,
	kCharacterActionJumpDownCeiling = 338494260,

	/////////////////////////////
	// Verges
	/////////////////////////////
	kCharacterAction125233040 = 125233040,   // Abbot
	kCharacterAction125499160 = 125499160,
	kCharacterAction155853632 = 155853632,
	kCharacterAction158617345 = 158617345,
	kCharacterAction167854368 = 167854368,
	kCharacterAction168187490 = 168187490,
	kCharacterAction168255788 = 168255788,
	kCharacterActionDeliverMessageToTyler = 191337656,
	kCharacterAction202558662 = 202558662,

	/////////////////////////////
	// Vassili
	/////////////////////////////
	kCharacterAction122732000 = 122732000,
	kCharacterAction168459827 = 168459827,
	kCharacterAction191477936 = 191477936,

	/////////////////////////////
	// Vesna
	/////////////////////////////
	kCharacterAction124190740 = 124190740,
	kCharacterAction134427424 = 134427424,
	kCharacterAction135024800 = 135024800,
	kCharacterAction137165825 = 137165825,
	kCharacterAction155913424 = 155913424,
	kCharacterAction190412928 = 190412928,
	kCharacterAction203663744 = 203663744,
	kCharacterAction204832737 = 204832737,

	/////////////////////////////
	// Misc
	/////////////////////////////
	kCharacterAction158610240 = 158610240,
	kCharacterAction167992577 = 167992577,
	kCharacterAction168646401 = 168646401,
	kCharacterAction169300225 = 169300225,
	kCharacterAction169773228 = 169773228,
	kCharacterActionEndChapter = 190346110,
	kCharacterAction191001984 = 191001984,
	kCharacterAction192637492 = 192637492,
	kCharacterAction201959744 = 201959744,
	kCharacterAction202621266 = 202621266,
	kCharacterAction202884544 = 202884544,
	kCharacterAction203078272 = 203078272,
	kCharacterAction205034665 = 205034665,
	kCharacterAction205294778 = 205294778,
	kCharacterActionUseWhistle = 270751616,
	kCharacterAction272177921 = 272177921,
	kCharacterAction224309120 = 224309120,
	kCharacterAction225358684 = 225358684,
	kCharacterAction225367984 = 225367984,
	kCharacterAction226078300 = 226078300, // Whistle

	kCharacterActionEnd
};

//////////////////////////////////////////////////////////////////////////
// Menu Action IDs
//////////////////////////////////////////////////////////////////////////
enum MenuActions {
	kMenuActionNone = 0,
	kMenuActionPlayGame,
	kMenuActionCredits,
	kMenuActionQuit,
	kMenuAction4,
	kMenuAction5, // 5
	kMenuActionSwitchEggs,
	kMenuActionRewind,
	kMenuActionFastForward,
	kMenuAction9,
	kMenuActionGoToParis, // 10
	kMenuActionGoToStrasbourg,
	kMenuActionGoToMunich,
	kMenuActionGoToVienna,
	kMenuActionGoToBudapest,
	kMenuActionGoToBelgrad, // 15
	kMenuActionGoToCostantinople,
	kMenuActionVolumeDown,
	kMenuActionVolumeUp,
	kMenuActionBrightnessDown,
	kMenuActionBrightnessUp, // 20
};

//////////////////////////////////////////////////////////////////////////
// Action IDs
//////////////////////////////////////////////////////////////////////////
enum Actions {
	kActionNone = 0,
	kActionInventory,
	kActionSendCathMessage,
	kActionPlaySound,
	kActionPlayMusic,
	kActionKnock,          // 5
	kActionCompartment,
	kActionPlaySounds,
	kActionPlayAnimation,
	kActionSetDoor,
	kActionSetModel,       // 10
	kActionSetItem,
	kActionKnockInside,
	kActionTakeItem,
	kActionDropItem,
	kActionLinkOnGlobal,   // 15
	kActionRattle,
	kActionDummyAction1,
	kActionLeanOutWindow,
	kActionAlmostFall,
	kActionClimbInWindow,  // 20
	kActionClimbLadder,
	kActionClimbDownTrain,
	kActionKronosSanctum,
	kActionEscapeBaggage,
	kActionEnterBaggage,   // 25
	kActionBombPuzzle,
	kActionConductors,
	kActionKronosConcert,
	kActionLetterInAugustSuitcase,
	kActionCatchBeetle,    // 30
	kActionExitCompartment,
	kActionOutsideTrain,
	kActionFirebirdPuzzle,
	kActionOpenMatchBox,
	kActionOpenBed,        // 35
	kActionDummyAction2,
	kActionHintDialog,
	kActionMusicEggBox,
	kActionFindEggUnderSink,
	kActionBed,            // 40
	kActionPlayMusicChapter,
	kActionPlayMusicChapterSetupTrain,
	kActionSwitchChapter,
	kActionEasterEgg
};

//////////////////////////////////////////////////////////////////////////
// Node properties
//////////////////////////////////////////////////////////////////////////
enum NodeProperties {
	kNodeHasDoor = 1,
	kNodeHasItem,
	kNodeHas2Items,
	kNodeHasDoorItem,
	kNodeHas3Items,
	kNodeModelPad,
	kNodeSoftPoint,
	kNodeSoftPointItem,

	kNodeAutoWalk = 128,
	kNodeSleepingOnBed,
	kNodeBeetle,
	kNodePullingStop,
	kNodeRebeccaDiary,
	kNodeExitFastWalk
};

//////////////////////////////////////////////////////////////////////////
// Game Progress
//////////////////////////////////////////////////////////////////////////
enum GameGlobals {
	kGlobalJacket = 1,
	kGlobalCorpseMovedFromFloor,
	kGlobalReadLetterInAugustSuitcase,
	kGlobalFoundCorpse,
	kGlobalCharacterSearchingForCath,
	kGlobalPhaseOfTheNight,
	kGlobalCathIcon,
	kGlobalCorpseHasBeenThrown,
	kGlobalFrancoisHasSeenCorpseThrown,
	kGlobalAnnaIsEating,
	kGlobalChapter,
	kGlobalDoneSavePointAfterLeftCompWithNewJacket,
	kGlobalMetAugust,
	kGlobalIsDayTime,
	kGlobalPoliceHasBoardedAndGone,
	kGlobalConcertIsHappening,
	kGlobalKahinaKillTimeoutActive,
	kGlobalMaxHasToStayInBaggage,
	kGlobalUnknownDebugFlag,
	kGlobalTrainIsRunning,
	kGlobalAnnaIsInBaggageCar,
	kGlobalDoneSavePointAfterLeavingSuitcaseInCathComp,
	kGlobalTatianaFoundOutEggStolen,
	kGlobalOverheardAugustInterruptingAnnaAtDinner,
	kGlobalMetTatianaAndVassili,
	kGlobalOverheardTatianaAndAlexeiAtBreakfast,
	kGlobalKnowAboutAugust,
	kGlobalKnowAboutKronos,
	kGlobalEggIsOpen,
	kGlobalCanPlayKronosSuitcaseLeftInCompMusic,
	kGlobalCanPlayEggSuitcaseMusic,
	kGlobalCanPlayEggUnderSinkMusic,
	kGlobalCathInSpecialState,
	kGlobalOverheardAlexeiTellingTatianaAboutBomb,
	kGlobalOverheardAlexeiTellingTatianaAboutWantingToKillVassili,
	kGlobalOverheardTatianaAndAlexeiPlayingChess,
	kGlobalOverheardMilosAndVesnaConspiring,
	kGlobalOverheardVesnaAndMilosDebatingAboutCath,
	kGlobalFrancoisSawABlackBeetle,
	kGlobalOverheardMadameAndFrancoisTalkingAboutWhistle,
	kGlobalMadameDemandedMaxInBaggage,
	kGlobalMadameComplainedAboutMax,
	kGlobalMetMadame,
	kGlobalKnowAboutRebeccaDiary,
	kGlobalOverheardSophieTalkingAboutCath,
	kGlobalMetSophieAndRebecca,
	kGlobalKnowAboutRebeccaAndSophieRelationship,
	kGlobalRegisteredTimeAtWhichCathGaveFirebirdToKronos,
	kGlobalMetMahmud,
	kGlobalAlmostFallActionIsAvailable,
	kGlobalMetMilos,
	kGlobalMetMonsieur,
	kGlobalMetHadija,
	kGlobalMetYasmin,
	kGlobalMetAlouan,
	kGlobalMetFatima,
	kGlobalTatianaScheduledToVisitCath,

	kGlobalCount = 128
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SHARED_H
