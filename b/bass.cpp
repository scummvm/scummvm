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
#include "director/lingo/xtras/b/bass.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra BASS -- version 1.0.0
-- http://cubesoftwaresolutions.com
new object me

-- Config
xaBASS_GetConfig object me, string option -- Retrieves the value of a config option.
xaBASS_SetConfig object me, string option, integer value -- Sets the value of a config option.
xaBASS_ErrorGetCode object me -- Retrieves the error code for the most recent BASS function call in the current thread.

-- Initialization, info, etc...
xaBASS_Free object me -- Frees all resources used by the output device, including all it's samples, streams, and MOD musics.
xaBASS_GetCPU object me -- Retrieves the current CPU usage of BASS.
xaBASS_GetDevice object me -- Retrieves the device setting in the current thread.
xaBASS_GetDeviceDescription object me, integer device -- Retrieves the text description of a device.
xaBASS_GetInfo object me -- Retrieves information on the device being used.
xaBASS_GetVersion object me -- Initializes an output device.
xaBASS_GetVolume object me -- Retrieves the current master volume level.
xaBASS_Init object me, integer sounddevice, integer freqency, integer flags -- Initializes an output device.
xaBASS_Pause object me -- Stops the output, pausing all musics/samples/streams.
xaBASS_SetDevice object me, integer device -- Sets the device to use for subsequent calls in the current thread.
xaBASS_SetVolume object me, integer volume -- Sets the output master volume.
xaBASS_Start object me -- Starts (or resumes) the output.
xaBASS_Stop object me -- Stops the output, stopping all musics/samples/streams.
xaBASS_Update object me -- Manually updates the HSTREAM and HMUSIC channel buffers.

-- 3D & EAX
xaBASS_Apply3D object me -- Applies changes made to the 3D system.
xaBASS_Get3DFactors object me -- Retrieves the factors that affect the calculations of 3D sound.
xaBASS_Get3DPosition object me -- Retrieves the position, velocity, and orientation of the listener.
xaBASS_GetEAXParameters object me -- Retrieves the current type of EAX environment and it's parameters.
xaBASS_Set3DFactors object me, float distf, float rollf, float doppf -- Sets the factors that affect the calculations of 3D sound.
xaBASS_Set3DPosition object me, vectorlist pos, vectorlist val, vectorlist front, vectorlist top -- Sets the position, velocity, and orientation of the listener (ie. the player).
xaBASS_SetEAXParameters object me, string env, float vol, float decay, float damp -- Sets the type of EAX environment and it's parameters.

-- Samples
xaBASS_SampleFree	 object me -- Frees a sample's resources.
xaBASS_SampleGetInfo	object me, integer samplehandle -- Retrieves a sample's default attributes and other information.
xaBASS_SampleLoad	object me, bool mem, string file, integer offset, integer length, integer max, integer flags -- Loads a WAV / MP3 / MP2 / MP1 / OGG sample.
xaBASS_SamplePlay	object me, integer samplehandle -- Plays a sample, using the sample's default attributes.
xaBASS_SamplePlay3D	object me, integer samplehandle, vectorlist pos, vectorlist orient, vectorlist vel -- Plays a sample, using the sample's default attributes.
xaBASS_SamplePlay3DEx	object me, integer samplehandle, vectorlist pos, vectorlist orient, vectorlist vel, integer start, integer freq, integer volume, boolean loop -- Plays a sample, using the sample's default attributes.
xaBASS_SamplePlayEx	object me, integer samplehandle, integer start, integer freq, integer volume, integer pan, bool loop -- Plays a sample from an offset, overriding the sample's default attributes with those specified.
xaBASS_SampleSetInfo	object me, integer samplehandle, BASS_SAMPLE samplelist -- Used with BASS_SampleGetInfo and BASS_SampleSetInfo to retrieve and set the default playback attributes of a sample.
xaBASS_SampleStop	object me, integer samplehandle -- Stops all instances of a sample.

-- Streams
xaBASS_StreamCreate object me, integer freq, integer chans, integer flags -- Creates a user sample stream.
xaBASS_StreamCreateFile object me, boolean mem, string file, integer offset, integer length, integer flags -- Creates a sample stream from an MP3, MP2, MP1, OGG or WAV file.
xaBASS_StreamCreateURL object me, string url, integer offset, integer flags -- Creates a sample stream from an MP3, MP2, MP1, OGG or WAV file on the internet, optionally receiving the downloaded data in a callback.
xaBASS_StreamFree object me, integer streamhandle -- Frees a sample stream's resources, including any sync/DSP/FX it has.
xaBASS_StreamGetFilePosition object me, integer streamhandle, string mode -- Retrieves the decoding/download/end position of a file stream.
xaBASS_StreamGetLength object me, integer streamhandle -- Retrieves the playback length (in bytes) of a file stream.
xaBASS_StreamGetTags object me, integer streamhandle, string tags -- Retrieves the requested tags/headers from a stream, if they are available.
xaBASS_StreamPlay object me, integer streamhandle, boolean flush, integer flags -- Plays a sample stream.
xaBASS_StreamPreBuf object me, integer streamhandle -- Pre-buffer initial sample data ready for playback of a stream.

-- MOD/M03 music
xaBASS_MusicFree	object me, integer musichandle -- Frees a MOD music's resources, including any sync/DSP/FX it has.
xaBASS_MusicGetLength	object me, integer musichandle, boolean playlen -- Retrieves the length of a MOD music.
xaBASS_MusicGetName	object me, integer musichandle -- Retrieves a MOD music's name.
xaBASS_MusicGetVolume	object me, integer musichandle, boolean type, integer chanins -- Retrieves the volume level of a channel or instrument in a MOD music.
xaBASS_MusicLoad	object me, boolean mem, string file, integer offset, integer length, integer flags, integer freq -- Loads a MOD music file - MO3 / IT / XM / S3M / MTM / MOD / UMX formats.
xaBASS_MusicPlay	object me, integer musichandle -- Plays a MOD music.
xaBASS_MusicPlayEx	object me, integer musichandle, integer order, integer row, integer flags, boolean reset -- Plays a MOD music, using the specified start position and flags.
xaBASS_MusicPreBuf	object me, integer musichandle -- Pre-buffer initial sample data ready for playback of a MOD music.
xaBASS_MusicSetAmplify	object me, integer musichandle, integer amp -- Sets a MOD music's amplification level.
xaBASS_MusicSetPanSep	object me, integer musichandle, integer pan -- Sets a MOD music's pan separation level.
xaBASS_MusicSetPositionScaler	object me, integer musichandle, integer scale -- Sets a MOD music's BASS_ChannelGetPosition scaler.
xaBASS_MusicSetVolume	object me, integer musichandle, boolean type, integer chanins, integer volume -- Sets the volume level of a channel or instrument in a MOD music.

-- Recording
xaBASS_RecordFree	object me -- Frees all resources used by the recording device.
xaBASS_RecordGetDevice	object me -- Retrieves the recording device setting in the current thread.
xaBASS_RecordGetDeviceDescription	object me, integer device -- Retrieves the text description of a recording device.
xaBASS_RecordGetInfo	object me -- Retrieves information on the recording device being used.
xaBASS_RecordGetInput	object me, integer input -- Retrieves the current settings of a recording input source.
xaBASS_RecordGetInputName	object me, integer input -- Retrieves the text description of a recording input source.
xaBASS_RecordInit	object me, integer device -- Initializes a recording device.
xaBASS_RecordSetDevice	object me, integer device -- Sets the recording device to use for subsequent calls in the current thread.
xaBASS_RecordSetInput	object me, integer input, integer setting -- Adjusts the settings of a recording input source.
xaBASS_RecordStart	object me, integer freq, integer flags -- Starts recording.
xaBASS_RecordStop object me -- Stops recording.
xaBASS_RecordPlay object me -- Plays the recording.
xaBASS_RecordWrite object me, string file -- Writes recording.

-- Channels
xaBASS_ChannelBytes2Seconds object me, integer streamhandle, integer pos -- Translates a byte position into time (seconds), based on a channel's format.
xaBASS_ChannelGet3DAttributes object me, integer streamhandle -- Retrieves the 3D attributes of a sample, stream, or MOD music channel with 3D functionality.
xaBASS_ChannelGet3DPosition object me, integer streamhandle -- Retrieves the 3D position of a sample, stream, or MOD music channel with 3D functionality.
xaBASS_ChannelGetAttributes object me, integer streamhandle -- Retrieves the basic attributes of a sample, stream or MOD music.
xaBASS_ChannelGetData object me, integer streamhandle, integer length, string constant -- Retrieves the immediate sample data (or an FFT representation of it) of a playing stream, or MOD music channel. Can also be used with a recording channel.
xaBASS_ChannelGetDevice object me, integer streamhandle -- Retrieves the device that the channel is using.
xaBASS_ChannelGetEAXMix object me, integer streamhandle -- Retrieves the wet (reverb) / dry (no reverb) mix ratio on a sample, stream, or MOD music channel with 3D functionality.
xaBASS_ChannelGetInfo object me, integer streamhandle -- Retrieves information on a channel.
xaBASS_ChannelGetLevel object me, integer streamhandle -- Retrieves the level of a playing stream, or MOD music channel. Can also be used with a recording channel. left is at index 1, right is at index 2.
xaBASS_ChannelGetPosition object me, integer streamhandle, boolean bmusichandle -- Retrieves the playback position of a sample, stream, or MOD music. Can also be used with a recording channel.
xaBASS_ChannelIsActive object me, integer streamhandle -- Checks if a sample, stream, or MOD music is active (playing) or stalled. Can also check if a recording is in progress.
xaBASS_ChannelIsSliding object me, integer streamhandle -- Checks if a sample, stream, MOD music, or CD channel has any of it's basic attributes sliding.
xaBASS_ChannelPause object me, integer streamhandle -- Pauses a sample, stream, MOD music, or recording.
xaBASS_ChannelRemoveFX object me, integer streamhandle, integer fxhandle -- Removes a DX8 effect on a stream, MOD music, or recording channel.
xaBASS_ChannelRemoveLink object me, integer streamhandle, integer chanhandle -- Removes a links between two MOD music or stream channels.
xaBASS_ChannelResume object me, integer streamhandle -- Resumes a paused sample, stream, MOD music, or recording.
xaBASS_ChannelSeconds2Bytes object me, integer streamhandle, float pos -- Translates a time (seconds) position into bytes, based on a channel's format.
xaBASS_ChannelSet3DAttributes object me, integer streamhandle, integer mode, float min, float max, integer iangle, integer oangle, integer outvol -- Sets the 3D attributes of a sample, stream, or MOD music channel with 3D functionality.
xaBASS_ChannelSet3DPosition object me, integer streamhandle, vectorlist pos, vectorlist orient, vectorlist vel, -- Sets the 3D position of a sample, stream, or MOD music channel with 3D functionality.
xaBASS_ChannelSetAttributes object me, integer streamhandle, integer freq, integer volume, integer pan -- Sets the basic attributes of a sample, stream or MOD music.
xaBASS_ChannelSetEAXMix object me, integer streamhandle, float mix -- Sets the wet (reverb) / dry (no reverb) mix ratio on a sample, stream, or MOD music channel with 3D functionality.
xaBASS_ChannelSetFX object me, integer streamhandle, string type -- Sets up a DX8 (DMO) effect on a stream, MOD music, or recording channel.
xaBASS_ChannelSlideAttributes object me, integer streamhandle, integer freq, integer volume, integer pan, integer time -- Slides the basic attributes of a sample, stream, MOD music, or CD channel, from the current values to the ones specified.
xaBASS_ChannelSetLink object me, integer streamhandle, integer chanhandle -- Links two MOD music or stream channels together.
xaBASS_ChannelSetMusicPosition object me, integer streamhandle, integer order, integer row -- Sets the playback position of a MOD music.
xaBASS_ChannelSetPosition object me, integer streamhandle, integer pos -- Sets the playback position of a sample, or stream.
xaBASS_ChannelStop object me, integer streamhandle -- Stops a sample, stream, MOD music, or recording.

-- DX8/DMO effects
xaBASS_FXGetParameters object me, integer handle, string effect -- Retrieves the parameters of an effect.
xaBASS_FXSetParameters object me, integer handle, string effect, propertylist par -- Sets the parameters of an effect.

-- Registration & Misc
xaClose object me -- Cleans up all used memory and resources.
xaRegister object me, string name, string serialkey -- Register xtra for commercial use.
"
 */

namespace Director {

const char *BassXtra::xlibName = "Bass";
const XlibFileDesc BassXtra::fileNames[] = {
	{ "bass",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BassXtra::m_new,		 0, 0,	900 },
	{ "xaBASS_GetConfig",				BassXtra::m_xaBASS_GetConfig,		 1, 1,	900 },
	{ "xaBASS_SetConfig",				BassXtra::m_xaBASS_SetConfig,		 2, 2,	900 },
	{ "xaBASS_ErrorGetCode",				BassXtra::m_xaBASS_ErrorGetCode,		 0, 0,	900 },
	{ "xaBASS_Free",				BassXtra::m_xaBASS_Free,		 0, 0,	900 },
	{ "xaBASS_GetCPU",				BassXtra::m_xaBASS_GetCPU,		 0, 0,	900 },
	{ "xaBASS_GetDevice",				BassXtra::m_xaBASS_GetDevice,		 0, 0,	900 },
	{ "xaBASS_GetDeviceDescription",				BassXtra::m_xaBASS_GetDeviceDescription,		 1, 1,	900 },
	{ "xaBASS_GetInfo",				BassXtra::m_xaBASS_GetInfo,		 0, 0,	900 },
	{ "xaBASS_GetVersion",				BassXtra::m_xaBASS_GetVersion,		 0, 0,	900 },
	{ "xaBASS_GetVolume",				BassXtra::m_xaBASS_GetVolume,		 0, 0,	900 },
	{ "xaBASS_Init",				BassXtra::m_xaBASS_Init,		 3, 3,	900 },
	{ "xaBASS_Pause",				BassXtra::m_xaBASS_Pause,		 0, 0,	900 },
	{ "xaBASS_SetDevice",				BassXtra::m_xaBASS_SetDevice,		 1, 1,	900 },
	{ "xaBASS_SetVolume",				BassXtra::m_xaBASS_SetVolume,		 1, 1,	900 },
	{ "xaBASS_Start",				BassXtra::m_xaBASS_Start,		 0, 0,	900 },
	{ "xaBASS_Stop",				BassXtra::m_xaBASS_Stop,		 0, 0,	900 },
	{ "xaBASS_Update",				BassXtra::m_xaBASS_Update,		 0, 0,	900 },
	{ "xaBASS_Apply3D",				BassXtra::m_xaBASS_Apply3D,		 0, 0,	900 },
	{ "xaBASS_Get3DFactors",				BassXtra::m_xaBASS_Get3DFactors,		 0, 0,	900 },
	{ "xaBASS_Get3DPosition",				BassXtra::m_xaBASS_Get3DPosition,		 0, 0,	900 },
	{ "xaBASS_GetEAXParameters",				BassXtra::m_xaBASS_GetEAXParameters,		 0, 0,	900 },
	{ "xaBASS_Set3DFactors",				BassXtra::m_xaBASS_Set3DFactors,		 3, 3,	900 },
	{ "xaBASS_Set3DPosition",				BassXtra::m_xaBASS_Set3DPosition,		 4, 4,	900 },
	{ "xaBASS_SetEAXParameters",				BassXtra::m_xaBASS_SetEAXParameters,		 4, 4,	900 },
	{ "xaBASS_SampleFree	",				BassXtra::m_xaBASS_SampleFree	,		 0, 0,	900 },
	{ "xaBASS_SampleGetInfo	object",				BassXtra::m_xaBASS_SampleGetInfo	object,		 1, 1,	900 },
	{ "xaBASS_SampleLoad	object",				BassXtra::m_xaBASS_SampleLoad	object,		 6, 6,	900 },
	{ "xaBASS_SamplePlay	object",				BassXtra::m_xaBASS_SamplePlay	object,		 1, 1,	900 },
	{ "xaBASS_SamplePlay3D	object",				BassXtra::m_xaBASS_SamplePlay3D	object,		 4, 4,	900 },
	{ "xaBASS_SamplePlay3DEx	object",				BassXtra::m_xaBASS_SamplePlay3DEx	object,		 8, 8,	900 },
	{ "xaBASS_SamplePlayEx	object",				BassXtra::m_xaBASS_SamplePlayEx	object,		 6, 6,	900 },
	{ "xaBASS_SampleSetInfo	object",				BassXtra::m_xaBASS_SampleSetInfo	object,		 2, 2,	900 },
	{ "xaBASS_SampleStop	object",				BassXtra::m_xaBASS_SampleStop	object,		 1, 1,	900 },
	{ "xaBASS_StreamCreate",				BassXtra::m_xaBASS_StreamCreate,		 3, 3,	900 },
	{ "xaBASS_StreamCreateFile",				BassXtra::m_xaBASS_StreamCreateFile,		 5, 5,	900 },
	{ "xaBASS_StreamCreateURL",				BassXtra::m_xaBASS_StreamCreateURL,		 3, 3,	900 },
	{ "xaBASS_StreamFree",				BassXtra::m_xaBASS_StreamFree,		 1, 1,	900 },
	{ "xaBASS_StreamGetFilePosition",				BassXtra::m_xaBASS_StreamGetFilePosition,		 2, 2,	900 },
	{ "xaBASS_StreamGetLength",				BassXtra::m_xaBASS_StreamGetLength,		 1, 1,	900 },
	{ "xaBASS_StreamGetTags",				BassXtra::m_xaBASS_StreamGetTags,		 2, 2,	900 },
	{ "xaBASS_StreamPlay",				BassXtra::m_xaBASS_StreamPlay,		 3, 3,	900 },
	{ "xaBASS_StreamPreBuf",				BassXtra::m_xaBASS_StreamPreBuf,		 1, 1,	900 },
	{ "xaBASS_MusicFree	object",				BassXtra::m_xaBASS_MusicFree	object,		 1, 1,	900 },
	{ "xaBASS_MusicGetLength	object",				BassXtra::m_xaBASS_MusicGetLength	object,		 2, 2,	900 },
	{ "xaBASS_MusicGetName	object",				BassXtra::m_xaBASS_MusicGetName	object,		 1, 1,	900 },
	{ "xaBASS_MusicGetVolume	object",				BassXtra::m_xaBASS_MusicGetVolume	object,		 3, 3,	900 },
	{ "xaBASS_MusicLoad	object",				BassXtra::m_xaBASS_MusicLoad	object,		 6, 6,	900 },
	{ "xaBASS_MusicPlay	object",				BassXtra::m_xaBASS_MusicPlay	object,		 1, 1,	900 },
	{ "xaBASS_MusicPlayEx	object",				BassXtra::m_xaBASS_MusicPlayEx	object,		 5, 5,	900 },
	{ "xaBASS_MusicPreBuf	object",				BassXtra::m_xaBASS_MusicPreBuf	object,		 1, 1,	900 },
	{ "xaBASS_MusicSetAmplify	object",				BassXtra::m_xaBASS_MusicSetAmplify	object,		 2, 2,	900 },
	{ "xaBASS_MusicSetPanSep	object",				BassXtra::m_xaBASS_MusicSetPanSep	object,		 2, 2,	900 },
	{ "xaBASS_MusicSetPositionScaler	object",				BassXtra::m_xaBASS_MusicSetPositionScaler	object,		 2, 2,	900 },
	{ "xaBASS_MusicSetVolume	object",				BassXtra::m_xaBASS_MusicSetVolume	object,		 4, 4,	900 },
	{ "xaBASS_RecordFree	object",				BassXtra::m_xaBASS_RecordFree	object,		 0, 0,	900 },
	{ "xaBASS_RecordGetDevice	object",				BassXtra::m_xaBASS_RecordGetDevice	object,		 0, 0,	900 },
	{ "xaBASS_RecordGetDeviceDescription	object",				BassXtra::m_xaBASS_RecordGetDeviceDescription	object,		 1, 1,	900 },
	{ "xaBASS_RecordGetInfo	object",				BassXtra::m_xaBASS_RecordGetInfo	object,		 0, 0,	900 },
	{ "xaBASS_RecordGetInput	object",				BassXtra::m_xaBASS_RecordGetInput	object,		 1, 1,	900 },
	{ "xaBASS_RecordGetInputName	object",				BassXtra::m_xaBASS_RecordGetInputName	object,		 1, 1,	900 },
	{ "xaBASS_RecordInit	object",				BassXtra::m_xaBASS_RecordInit	object,		 1, 1,	900 },
	{ "xaBASS_RecordSetDevice	object",				BassXtra::m_xaBASS_RecordSetDevice	object,		 1, 1,	900 },
	{ "xaBASS_RecordSetInput	object",				BassXtra::m_xaBASS_RecordSetInput	object,		 2, 2,	900 },
	{ "xaBASS_RecordStart	object",				BassXtra::m_xaBASS_RecordStart	object,		 2, 2,	900 },
	{ "xaBASS_RecordStop",				BassXtra::m_xaBASS_RecordStop,		 0, 0,	900 },
	{ "xaBASS_RecordPlay",				BassXtra::m_xaBASS_RecordPlay,		 0, 0,	900 },
	{ "xaBASS_RecordWrite",				BassXtra::m_xaBASS_RecordWrite,		 1, 1,	900 },
	{ "xaBASS_ChannelBytes2Seconds",				BassXtra::m_xaBASS_ChannelBytes2Seconds,		 2, 2,	900 },
	{ "xaBASS_ChannelGet3DAttributes",				BassXtra::m_xaBASS_ChannelGet3DAttributes,		 1, 1,	900 },
	{ "xaBASS_ChannelGet3DPosition",				BassXtra::m_xaBASS_ChannelGet3DPosition,		 1, 1,	900 },
	{ "xaBASS_ChannelGetAttributes",				BassXtra::m_xaBASS_ChannelGetAttributes,		 1, 1,	900 },
	{ "xaBASS_ChannelGetData",				BassXtra::m_xaBASS_ChannelGetData,		 3, 3,	900 },
	{ "xaBASS_ChannelGetDevice",				BassXtra::m_xaBASS_ChannelGetDevice,		 1, 1,	900 },
	{ "xaBASS_ChannelGetEAXMix",				BassXtra::m_xaBASS_ChannelGetEAXMix,		 1, 1,	900 },
	{ "xaBASS_ChannelGetInfo",				BassXtra::m_xaBASS_ChannelGetInfo,		 1, 1,	900 },
	{ "xaBASS_ChannelGetLevel",				BassXtra::m_xaBASS_ChannelGetLevel,		 1, 1,	900 },
	{ "xaBASS_ChannelGetPosition",				BassXtra::m_xaBASS_ChannelGetPosition,		 2, 2,	900 },
	{ "xaBASS_ChannelIsActive",				BassXtra::m_xaBASS_ChannelIsActive,		 1, 1,	900 },
	{ "xaBASS_ChannelIsSliding",				BassXtra::m_xaBASS_ChannelIsSliding,		 1, 1,	900 },
	{ "xaBASS_ChannelPause",				BassXtra::m_xaBASS_ChannelPause,		 1, 1,	900 },
	{ "xaBASS_ChannelRemoveFX",				BassXtra::m_xaBASS_ChannelRemoveFX,		 2, 2,	900 },
	{ "xaBASS_ChannelRemoveLink",				BassXtra::m_xaBASS_ChannelRemoveLink,		 2, 2,	900 },
	{ "xaBASS_ChannelResume",				BassXtra::m_xaBASS_ChannelResume,		 1, 1,	900 },
	{ "xaBASS_ChannelSeconds2Bytes",				BassXtra::m_xaBASS_ChannelSeconds2Bytes,		 2, 2,	900 },
	{ "xaBASS_ChannelSet3DAttributes",				BassXtra::m_xaBASS_ChannelSet3DAttributes,		 7, 7,	900 },
	{ "xaBASS_ChannelSet3DPosition",				BassXtra::m_xaBASS_ChannelSet3DPosition,		 5, 5,	900 },
	{ "xaBASS_ChannelSetAttributes",				BassXtra::m_xaBASS_ChannelSetAttributes,		 4, 4,	900 },
	{ "xaBASS_ChannelSetEAXMix",				BassXtra::m_xaBASS_ChannelSetEAXMix,		 2, 2,	900 },
	{ "xaBASS_ChannelSetFX",				BassXtra::m_xaBASS_ChannelSetFX,		 2, 2,	900 },
	{ "xaBASS_ChannelSlideAttributes",				BassXtra::m_xaBASS_ChannelSlideAttributes,		 5, 5,	900 },
	{ "xaBASS_ChannelSetLink",				BassXtra::m_xaBASS_ChannelSetLink,		 2, 2,	900 },
	{ "xaBASS_ChannelSetMusicPosition",				BassXtra::m_xaBASS_ChannelSetMusicPosition,		 3, 3,	900 },
	{ "xaBASS_ChannelSetPosition",				BassXtra::m_xaBASS_ChannelSetPosition,		 2, 2,	900 },
	{ "xaBASS_ChannelStop",				BassXtra::m_xaBASS_ChannelStop,		 1, 1,	900 },
	{ "xaBASS_FXGetParameters",				BassXtra::m_xaBASS_FXGetParameters,		 2, 2,	900 },
	{ "xaBASS_FXSetParameters",				BassXtra::m_xaBASS_FXSetParameters,		 3, 3,	900 },
	{ "xaClose",				BassXtra::m_xaClose,		 0, 0,	900 },
	{ "xaRegister",				BassXtra::m_xaRegister,		 2, 2,	900 },
	{ """,				BassXtra::m_",		 -1, -1,	900 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BassXtraObject::BassXtraObject(ObjectType ObjectType) :Object<BassXtraObject>("Bass") {
	_objType = ObjectType;
}

bool BassXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BassXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BassXtra::xlibName);
	warning("BassXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BassXtra::open(ObjectType type, const Common::Path &path) {
    BassXtraObject::initMethods(xlibMethods);
    BassXtraObject *xobj = new BassXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BassXtra::close(ObjectType type) {
    BassXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BassXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BassXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BassXtra::m_xaBASS_GetConfig, 0)
XOBJSTUB(BassXtra::m_xaBASS_SetConfig, 0)
XOBJSTUB(BassXtra::m_xaBASS_ErrorGetCode, 0)
XOBJSTUB(BassXtra::m_xaBASS_Free, 0)
XOBJSTUB(BassXtra::m_xaBASS_GetCPU, 0)
XOBJSTUB(BassXtra::m_xaBASS_GetDevice, 0)
XOBJSTUB(BassXtra::m_xaBASS_GetDeviceDescription, 0)
XOBJSTUB(BassXtra::m_xaBASS_GetInfo, 0)
XOBJSTUB(BassXtra::m_xaBASS_GetVersion, 0)
XOBJSTUB(BassXtra::m_xaBASS_GetVolume, 0)
XOBJSTUB(BassXtra::m_xaBASS_Init, 0)
XOBJSTUB(BassXtra::m_xaBASS_Pause, 0)
XOBJSTUB(BassXtra::m_xaBASS_SetDevice, 0)
XOBJSTUB(BassXtra::m_xaBASS_SetVolume, 0)
XOBJSTUB(BassXtra::m_xaBASS_Start, 0)
XOBJSTUB(BassXtra::m_xaBASS_Stop, 0)
XOBJSTUB(BassXtra::m_xaBASS_Update, 0)
XOBJSTUB(BassXtra::m_xaBASS_Apply3D, 0)
XOBJSTUB(BassXtra::m_xaBASS_Get3DFactors, 0)
XOBJSTUB(BassXtra::m_xaBASS_Get3DPosition, 0)
XOBJSTUB(BassXtra::m_xaBASS_GetEAXParameters, 0)
XOBJSTUB(BassXtra::m_xaBASS_Set3DFactors, 0)
XOBJSTUB(BassXtra::m_xaBASS_Set3DPosition, 0)
XOBJSTUB(BassXtra::m_xaBASS_SetEAXParameters, 0)
XOBJSTUB(BassXtra::m_xaBASS_SampleFree	, 0)
XOBJSTUB(BassXtra::m_xaBASS_SampleGetInfo	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_SampleLoad	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_SamplePlay	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_SamplePlay3D	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_SamplePlay3DEx	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_SamplePlayEx	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_SampleSetInfo	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_SampleStop	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_StreamCreate, 0)
XOBJSTUB(BassXtra::m_xaBASS_StreamCreateFile, 0)
XOBJSTUB(BassXtra::m_xaBASS_StreamCreateURL, 0)
XOBJSTUB(BassXtra::m_xaBASS_StreamFree, 0)
XOBJSTUB(BassXtra::m_xaBASS_StreamGetFilePosition, 0)
XOBJSTUB(BassXtra::m_xaBASS_StreamGetLength, 0)
XOBJSTUB(BassXtra::m_xaBASS_StreamGetTags, 0)
XOBJSTUB(BassXtra::m_xaBASS_StreamPlay, 0)
XOBJSTUB(BassXtra::m_xaBASS_StreamPreBuf, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicFree	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicGetLength	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicGetName	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicGetVolume	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicLoad	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicPlay	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicPlayEx	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicPreBuf	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicSetAmplify	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicSetPanSep	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicSetPositionScaler	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_MusicSetVolume	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordFree	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordGetDevice	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordGetDeviceDescription	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordGetInfo	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordGetInput	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordGetInputName	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordInit	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordSetDevice	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordSetInput	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordStart	object, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordStop, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordPlay, 0)
XOBJSTUB(BassXtra::m_xaBASS_RecordWrite, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelBytes2Seconds, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelGet3DAttributes, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelGet3DPosition, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelGetAttributes, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelGetData, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelGetDevice, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelGetEAXMix, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelGetInfo, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelGetLevel, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelGetPosition, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelIsActive, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelIsSliding, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelPause, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelRemoveFX, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelRemoveLink, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelResume, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelSeconds2Bytes, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelSet3DAttributes, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelSet3DPosition, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelSetAttributes, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelSetEAXMix, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelSetFX, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelSlideAttributes, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelSetLink, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelSetMusicPosition, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelSetPosition, 0)
XOBJSTUB(BassXtra::m_xaBASS_ChannelStop, 0)
XOBJSTUB(BassXtra::m_xaBASS_FXGetParameters, 0)
XOBJSTUB(BassXtra::m_xaBASS_FXSetParameters, 0)
XOBJSTUB(BassXtra::m_xaClose, 0)
XOBJSTUB(BassXtra::m_xaRegister, 0)
XOBJSTUB(BassXtra::m_", 0)

}
