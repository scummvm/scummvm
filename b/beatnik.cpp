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

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/b/beatnik.h"

/**************************************************
 *
 * USED IN:
 * Buddy Brush and the Painted Circus
 * Chalky Bears Reading and Grammer Adventure
 * Webmaster: Fantastic Adventures in the World of the Internet
 * Zoboomafoo: Animal Alphabet
 *
 **************************************************/

/*
-- xtra Beatnik
--
-- Beatnik Xtra musicObject v.1.21 r9
-- http://www.beatnik.com/to/?xtra
-- Powered by the Beatnik Audio Engine: http://www.beatnik.com/to/?bae
-- Copyright (c) 1995-1999 Beatnik, Inc.
--
new object me, *                        -- new( 'beatnik' direct-to-card {TRUE/FALSE (default is FALSE)} )  Create new Music Object instance. 
setPreferredSoundChannel me, int        -- setPreferredSoundChannel( Director-sound-channel {1-4 Windows / 1-8 Mac OS / 0=use any} )  Returns actual channel used. Affects all instances. 
getSampleLibrary object me, *           -- getSampleLibrary( stringVersionNum, stringBankFileName )  Returns path to bank file. Omit params for current bank. 
setSampleLibrary object me, string      -- setSampleLibrary( stringBankPath )  Use this bank file; usually Patches.hsb.  Affects all instances. (Registered Only) 
getSampleRate object me                 -- getSampleRate()  Returns Beatnik Xtra's audio sample rate.  (Registered Only) 
setSampleRate object me, *              -- setSampleRate( symbolRate {#Rate_11k/#Rate_22k/#Rate_44k/#Rate_48k} )  Sets Beatnik Xtra's audio sample rate.  Affects all instances. (Registered Only) 
setSliceSize object me, int             -- setSliceSize( intSliceSize {1-25 (default is 12)} )  Set size of audio chunks sent to Director.  Use 1 or 2 for DTC.  Affects all instances. 
                                        -- Larger numbers mean more latency (slower response to notes & commands); 
                                        -- Smaller numbers risk more sound break-up when using multiple Director sound channels. 
setReady object me                      -- setReady()  Re-initialize Beatnik Audio Engine. Affects all instances. 
closeBeatnik object me                  -- closeBeatnik()  Shut down this Music Object instance. Set your instance variable to VOID after.
isReady object me                       -- isReady()  Returns TRUE if Music Object instance is OK for playback. 
onMetaEvent object me, *                -- setMetaEventCallback( symbolorstringHandler )  Call your handler upon any MIDI MetaEvent. (Registered Only) 
onStop object me, *                     -- setOnStopCallback( symbolorstringHandler )  Call your handler upon end of MIDI data. (Registered Only) 
play object me, *                       -- play( loop {TRUE/FALSE}, stringFilename )  Load music/sound file & start playing.  Omit parms to play last loaded file.  To avoid start-upon-load, setAutostart( FALSE ) first. 
playGroovoid object me, *               -- playGroovoid( loop {TRUE/FALSE}, stringGroovoidName )  Play preset music or sound.  See doc for names (60 in Lite, 120 in Pro). 
stop object me, *                       -- stop( fadeTimeMillisecondsInteger )  Stop playing.  To fade to silence first, use param. 
pause object me, *                      -- pause( fadeTimeMillisecondsInteger )  Pause playback; if already paused, resume playback.  To fade to/from silence, use param. 
setLoop object me, int                  -- setLoop( loop {TRUE/FALSE } )  Always repeat when music or sound ends? 
setAutoStart object me, int             -- setAutoStart( autostart {TRUE/FALSE } )  Start playing file immediately upon load? 
setGlobalMute object me, int            -- setGlobalMute( muteAll {TRUE/FALSE } )  Temporarily suspend/pause Beatnik Xtra audio?  Affects all instances. 
fadeTo object me, int, *                -- fadeTo( endVolume {0-100}, [fadeTimeMilliseconds (default is 100)]) )  Ramp current volume to endVolume over fadeTimeMilliseconds. 
fadeFromTo object me, int, int, *       -- fadeFromTo( startVolume {0-100}, endVolume {0-100}, [fadeTimeMilliseconds (default is 100)] )  Set volume to startVolume, then ramp to endVolume over fadeTimeMilliseconds.  
playNote object me, *                   -- playNote( midiChannel {1-16}, instrumentBank {0-2}, instrument {0-127}, noteNumber {0-127}, velocity {0-127}, durationMilliseconds )  Play note of specific length.  
noteOn object me, *                     -- noteOn( midiChannel {1-16}, instrumentBank {0-2}, instrument {0-127}, noteNumber {0-127}, velocity {0-127}, durationMilliseconds )  Start note of indeterminate length.  Use noteOff() to end. 
noteOff object me, *                    -- noteOn( midiChannel {1-16}, noteNumber {0-127}, velocity {0-127} )  End a note started with noteOn(). 
                                        -- To turn off all notes currently sounding on channel midiChannel, omit noteNumber and velocity.  
getNoteName object me, int              -- getNoteName( noteNumber {0-127})  Returns string with scale name (e.g. 'G5', 'C#3') for this MIDI note number.  (Registered Only)  
getNoteNumber object me, string         -- getNoteNumber( noteScaleNameString )  Returns MIDI note number {0-127} for a valid note scale name (e.g. 'G5', 'C#3').  (Registered Only) 
setVolume object me, int                -- setVolume( volumeLevel {0-100} )  Play music or sound at this volume. 
setEnableTranspose object me, int, int  -- setEnableTranspose( midiChannel {1-16, or 0=all}, transposeFlag {TRUE/FALSE} )  Allow MIDI note transposition for this channel?  Set amount with setTranspose().  
setTranspose object me, int             -- setTranspose( noteNumberShift )  Play all MIDI notes up or down this many note numbers.  setEnableTranspose( FALSE ) blocks this. 
setReverbType object me, int            -- setReverbType( reverbType {1-11})  Use this preset reverb effect.  Affects all instances. (Types 2, 3, 5, 7-11 Registered Only) 
setTempo object me, int                 -- setTempo( integerBeatsPerMinute {0-499} )  Play MIDI and RMF music at this tempo. 
setController object me, int, int, int  -- setController( midiChannel {1-16, or 0=all}, controllerNumber {0-127}, controllerValue {0-127} )  Set controller value for channel.  See doc for numbers & value ranges.
setProgram object me, int, int, *       -- setProgram( midiChannel {1-16, or 0=all}, [bankNumber {0=General MIDI / 1=Beatnik / 2=Inside RMF file} (default is 0),] programNumber {0-127} )  Select instrument for channel. 
                                        -- To access General MIDI instruments only, omit bankNumber. 
setTrackMute object me, int, int        -- setTrackMute( midiFileTrack {1-64, or 0=all}, muteTrack {TRUE/FALSE} )  Turn off notes from this music track? 
setChannelMute object me, int, int      -- setChannelMute( midiChannel {1-16, or 0=all}, muteChannel {TRUE/FALSE} )  Turn off notes from this MIDI channel? 
setTrackSolo object me, int, int        -- setTrackSolo( midiFileTrack {1-64, or 0=all}, soloTrack {TRUE/FALSE}  Listen only to notes from this music track?
setChannelSolo object me, int, int      -- setChannelSolo( midiChannel {1-16, or 0=all}, soloChannel {TRUE/FALSE} )  Listen only to notes from this MIDI channel?
setPosition object me, int              -- setPosition( timeFromFileStartMilliseconds )  Move MIDI/RMF playhead to this time.  Time not guaranteed, returns actual playhead if playing.  (Registered Only) 
setStartTime object me, int             -- setStartTime( timeFromFileStartMilliseconds ) Treat this time as MIDI/RMF starting point.  Time not guaranteed, returns actual new start.  Use only when stopped.  (Registered Only) 
setEndTime object me, int               -- setEndTime( timeFromFileStartMilliseconds )   Treat this time as MIDI/RMF ending point.  Time not guaranteed, returns actual new end.  Use only when stopped.  (Registered Only) 
getLoop object me                       -- getLoop()  Returns current loop flag {TRUE/FALSE}. 
getAutoStart object me                  -- getAutoStart()  Returns current Autostart flag {TRUE/FALSE}. 
getGlobalMute object me                 -- getGlobalMute()  Returns TRUE if Beatnik Xtra is globally suspended/muted, otherwise FALSE.  Reflects all instances. 
getReverbType object me                 -- getReverbType()  Returns current reverbType {1-11}.  Reflects all instances.
getTempo object me                      -- getTempo()   Returns current MIDI/RMF music tempo {0-499}
getEnableTranspose object me, int       -- getEnableTranspose( midiChannel {1-16} )  Returns channel's current transposeFlag {TRUE/FALSE}. 
getTranspose object me                  -- getTranspose()  Returns channel's current transposition offset {signed integer} 
getController object me, int, int       -- getController( midiChannel {1-16}, controllerNumber {0-127} )  Returns current value {0-127} of a controller for a channel. 
getProgram object me, int               -- getProgram( midiChannel {1-16} )  Returns channel's current instrument program number {0-127}. 
getVolume object me                     -- getVolume()  Returns current volume level {0-100}. 
getTrackMute object me, int             -- getTrackMute( midiFileTrack {1-64} )  Returns track's current mute state {TRUE/FALSE}. 
getChannelMute object me, int           -- getChannelMute( midiChannel {1-16} )  Returns channel's current mute state {TRUE/FALSE}. 
getTrackSolo object me, int             -- getTrackSolo( midiFileTrack {1-64} )  Returns track's current solo state {TRUE/FALSE}. 
getChannelSolo object me, int           -- getChannelSolo( midiChannel {1-16} )  Returns channel's current solo state {TRUE/FALSE}. 
getPosition object me                   -- getPosition()  Returns playhead's current timeFromFileStartMilliseconds {integer}.  Valid only if playing. (Registered Only) 
getPlayLength object me                 -- getPlayLength()  Returns file's playing time {integerMilliseconds}, at original tempo.  (Registered Only) 
isPaused object me                      -- isPaused()  Returns current pause state {TRUE/FALSE}. 
isPlaying object me                     -- isPlaying()  Returns current playing state {TRUE/FALSE}. 
getInfo object me, *                    -- getInfo( infoFieldName {'title'/'performer'/'composer'/'copyright'/'publisher'/'use'/'licensee'/'term'/'expiration'/'notes'/'index'} )  Returns info from RMF file {string}. 
getErrorString object me, int           -- getErrorString( integerErrorCode )  Returns string explaining the error code. 
getVersion object me                    -- getVersion()  Returns string with Beatnik Xtra version number, platform, Lite/Pro. Reflects all instances. 
register object me, string              -- register( stringRegistrationCode )  Enables Beatnik Xtra Pro features, if reg code is good. Affects all instances. 
"
 */

namespace Director {

const char *BeatnikXtra::xlibName = "Beatnik";
const XlibFileDesc BeatnikXtra::fileNames[] = {
	{ "beatnik",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BeatnikXtra::m_new,		 -1, 0,	500 },
	{ "setPreferredSoundChannel",				BeatnikXtra::m_setPreferredSoundChannel,		 1, 1,	500 },
	{ "getSampleLibrary",				BeatnikXtra::m_getSampleLibrary,		 -1, 0,	500 },
	{ "setSampleLibrary",				BeatnikXtra::m_setSampleLibrary,		 1, 1,	500 },
	{ "getSampleRate",				BeatnikXtra::m_getSampleRate,		 0, 0,	500 },
	{ "setSampleRate",				BeatnikXtra::m_setSampleRate,		 -1, 0,	500 },
	{ "setSliceSize",				BeatnikXtra::m_setSliceSize,		 1, 1,	500 },
	{ "setReady",				BeatnikXtra::m_setReady,		 0, 0,	500 },
	{ "closeBeatnik",				BeatnikXtra::m_closeBeatnik,		 0, 0,	500 },
	{ "isReady",				BeatnikXtra::m_isReady,		 0, 0,	500 },
	{ "onMetaEvent",				BeatnikXtra::m_onMetaEvent,		 -1, 0,	500 },
	{ "onStop",				BeatnikXtra::m_onStop,		 -1, 0,	500 },
	{ "play",				BeatnikXtra::m_play,		 -1, 0,	500 },
	{ "playGroovoid",				BeatnikXtra::m_playGroovoid,		 -1, 0,	500 },
	{ "stop",				BeatnikXtra::m_stop,		 -1, 0,	500 },
	{ "pause",				BeatnikXtra::m_pause,		 -1, 0,	500 },
	{ "setLoop",				BeatnikXtra::m_setLoop,		 1, 1,	500 },
	{ "setAutoStart",				BeatnikXtra::m_setAutoStart,		 1, 1,	500 },
	{ "setGlobalMute",				BeatnikXtra::m_setGlobalMute,		 1, 1,	500 },
	{ "fadeTo",				BeatnikXtra::m_fadeTo,		 -1, 0,	500 },
	{ "fadeFromTo",				BeatnikXtra::m_fadeFromTo,		 -1, 0,	500 },
	{ "playNote",				BeatnikXtra::m_playNote,		 -1, 0,	500 },
	{ "noteOn",				BeatnikXtra::m_noteOn,		 -1, 0,	500 },
	{ "noteOff",				BeatnikXtra::m_noteOff,		 -1, 0,	500 },
	{ "getNoteName",				BeatnikXtra::m_getNoteName,		 1, 1,	500 },
	{ "getNoteNumber",				BeatnikXtra::m_getNoteNumber,		 1, 1,	500 },
	{ "setVolume",				BeatnikXtra::m_setVolume,		 1, 1,	500 },
	{ "setEnableTranspose",				BeatnikXtra::m_setEnableTranspose,		 2, 2,	500 },
	{ "setTranspose",				BeatnikXtra::m_setTranspose,		 1, 1,	500 },
	{ "setReverbType",				BeatnikXtra::m_setReverbType,		 1, 1,	500 },
	{ "setTempo",				BeatnikXtra::m_setTempo,		 1, 1,	500 },
	{ "setController",				BeatnikXtra::m_setController,		 3, 3,	500 },
	{ "setProgram",				BeatnikXtra::m_setProgram,		 -1, 0,	500 },
	{ "setTrackMute",				BeatnikXtra::m_setTrackMute,		 2, 2,	500 },
	{ "setChannelMute",				BeatnikXtra::m_setChannelMute,		 2, 2,	500 },
	{ "setTrackSolo",				BeatnikXtra::m_setTrackSolo,		 2, 2,	500 },
	{ "setChannelSolo",				BeatnikXtra::m_setChannelSolo,		 2, 2,	500 },
	{ "setPosition",				BeatnikXtra::m_setPosition,		 1, 1,	500 },
	{ "setStartTime",				BeatnikXtra::m_setStartTime,		 1, 1,	500 },
	{ "setEndTime",				BeatnikXtra::m_setEndTime,		 1, 1,	500 },
	{ "getLoop",				BeatnikXtra::m_getLoop,		 0, 0,	500 },
	{ "getAutoStart",				BeatnikXtra::m_getAutoStart,		 0, 0,	500 },
	{ "getGlobalMute",				BeatnikXtra::m_getGlobalMute,		 0, 0,	500 },
	{ "getReverbType",				BeatnikXtra::m_getReverbType,		 0, 0,	500 },
	{ "getTempo",				BeatnikXtra::m_getTempo,		 0, 0,	500 },
	{ "getEnableTranspose",				BeatnikXtra::m_getEnableTranspose,		 1, 1,	500 },
	{ "getTranspose",				BeatnikXtra::m_getTranspose,		 0, 0,	500 },
	{ "getController",				BeatnikXtra::m_getController,		 2, 2,	500 },
	{ "getProgram",				BeatnikXtra::m_getProgram,		 1, 1,	500 },
	{ "getVolume",				BeatnikXtra::m_getVolume,		 0, 0,	500 },
	{ "getTrackMute",				BeatnikXtra::m_getTrackMute,		 1, 1,	500 },
	{ "getChannelMute",				BeatnikXtra::m_getChannelMute,		 1, 1,	500 },
	{ "getTrackSolo",				BeatnikXtra::m_getTrackSolo,		 1, 1,	500 },
	{ "getChannelSolo",				BeatnikXtra::m_getChannelSolo,		 1, 1,	500 },
	{ "getPosition",				BeatnikXtra::m_getPosition,		 0, 0,	500 },
	{ "getPlayLength",				BeatnikXtra::m_getPlayLength,		 0, 0,	500 },
	{ "isPaused",				BeatnikXtra::m_isPaused,		 0, 0,	500 },
	{ "isPlaying",				BeatnikXtra::m_isPlaying,		 0, 0,	500 },
	{ "getInfo",				BeatnikXtra::m_getInfo,		 -1, 0,	500 },
	{ "getErrorString",				BeatnikXtra::m_getErrorString,		 1, 1,	500 },
	{ "getVersion",				BeatnikXtra::m_getVersion,		 0, 0,	500 },
	{ "register",				BeatnikXtra::m_register,		 1, 1,	500 },
	{ """,				BeatnikXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BeatnikXtraObject::BeatnikXtraObject(ObjectType ObjectType) :Object<BeatnikXtraObject>("Beatnik") {
	_objType = ObjectType;
}

bool BeatnikXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BeatnikXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BeatnikXtra::xlibName);
	warning("BeatnikXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BeatnikXtra::open(ObjectType type, const Common::Path &path) {
    BeatnikXtraObject::initMethods(xlibMethods);
    BeatnikXtraObject *xobj = new BeatnikXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BeatnikXtra::close(ObjectType type) {
    BeatnikXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BeatnikXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BeatnikXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BeatnikXtra::m_setPreferredSoundChannel, 0)
XOBJSTUB(BeatnikXtra::m_getSampleLibrary, 0)
XOBJSTUB(BeatnikXtra::m_setSampleLibrary, 0)
XOBJSTUB(BeatnikXtra::m_getSampleRate, 0)
XOBJSTUB(BeatnikXtra::m_setSampleRate, 0)
XOBJSTUB(BeatnikXtra::m_setSliceSize, 0)
XOBJSTUB(BeatnikXtra::m_setReady, 0)
XOBJSTUB(BeatnikXtra::m_closeBeatnik, 0)
XOBJSTUB(BeatnikXtra::m_isReady, 0)
XOBJSTUB(BeatnikXtra::m_onMetaEvent, 0)
XOBJSTUB(BeatnikXtra::m_onStop, 0)
XOBJSTUB(BeatnikXtra::m_play, 0)
XOBJSTUB(BeatnikXtra::m_playGroovoid, 0)
XOBJSTUB(BeatnikXtra::m_stop, 0)
XOBJSTUB(BeatnikXtra::m_pause, 0)
XOBJSTUB(BeatnikXtra::m_setLoop, 0)
XOBJSTUB(BeatnikXtra::m_setAutoStart, 0)
XOBJSTUB(BeatnikXtra::m_setGlobalMute, 0)
XOBJSTUB(BeatnikXtra::m_fadeTo, 0)
XOBJSTUB(BeatnikXtra::m_fadeFromTo, 0)
XOBJSTUB(BeatnikXtra::m_playNote, 0)
XOBJSTUB(BeatnikXtra::m_noteOn, 0)
XOBJSTUB(BeatnikXtra::m_noteOff, 0)
XOBJSTUB(BeatnikXtra::m_getNoteName, 0)
XOBJSTUB(BeatnikXtra::m_getNoteNumber, 0)
XOBJSTUB(BeatnikXtra::m_setVolume, 0)
XOBJSTUB(BeatnikXtra::m_setEnableTranspose, 0)
XOBJSTUB(BeatnikXtra::m_setTranspose, 0)
XOBJSTUB(BeatnikXtra::m_setReverbType, 0)
XOBJSTUB(BeatnikXtra::m_setTempo, 0)
XOBJSTUB(BeatnikXtra::m_setController, 0)
XOBJSTUB(BeatnikXtra::m_setProgram, 0)
XOBJSTUB(BeatnikXtra::m_setTrackMute, 0)
XOBJSTUB(BeatnikXtra::m_setChannelMute, 0)
XOBJSTUB(BeatnikXtra::m_setTrackSolo, 0)
XOBJSTUB(BeatnikXtra::m_setChannelSolo, 0)
XOBJSTUB(BeatnikXtra::m_setPosition, 0)
XOBJSTUB(BeatnikXtra::m_setStartTime, 0)
XOBJSTUB(BeatnikXtra::m_setEndTime, 0)
XOBJSTUB(BeatnikXtra::m_getLoop, 0)
XOBJSTUB(BeatnikXtra::m_getAutoStart, 0)
XOBJSTUB(BeatnikXtra::m_getGlobalMute, 0)
XOBJSTUB(BeatnikXtra::m_getReverbType, 0)
XOBJSTUB(BeatnikXtra::m_getTempo, 0)
XOBJSTUB(BeatnikXtra::m_getEnableTranspose, 0)
XOBJSTUB(BeatnikXtra::m_getTranspose, 0)
XOBJSTUB(BeatnikXtra::m_getController, 0)
XOBJSTUB(BeatnikXtra::m_getProgram, 0)
XOBJSTUB(BeatnikXtra::m_getVolume, 0)
XOBJSTUB(BeatnikXtra::m_getTrackMute, 0)
XOBJSTUB(BeatnikXtra::m_getChannelMute, 0)
XOBJSTUB(BeatnikXtra::m_getTrackSolo, 0)
XOBJSTUB(BeatnikXtra::m_getChannelSolo, 0)
XOBJSTUB(BeatnikXtra::m_getPosition, 0)
XOBJSTUB(BeatnikXtra::m_getPlayLength, 0)
XOBJSTUB(BeatnikXtra::m_isPaused, 0)
XOBJSTUB(BeatnikXtra::m_isPlaying, 0)
XOBJSTUB(BeatnikXtra::m_getInfo, 0)
XOBJSTUB(BeatnikXtra::m_getErrorString, 0)
XOBJSTUB(BeatnikXtra::m_getVersion, 0)
XOBJSTUB(BeatnikXtra::m_register, 0)
XOBJSTUB(BeatnikXtra::m_", 0)

}
