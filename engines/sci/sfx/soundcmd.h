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

#ifndef SCI_SOUNDCMD_H
#define SCI_SOUNDCMD_H

#include "sci/sci.h"	// for USE_OLD_MUSIC_FUNCTIONS

#include "common/list.h"
#include "sci/engine/state.h"

namespace Sci {

class SciMusic;
class SoundCommandParser;
typedef void (SoundCommandParser::*SoundCommand)(reg_t obj, int16 value);

struct MusicEntryCommand {
	MusicEntryCommand(const char *d, SoundCommand c) : sndCmd(c), desc(d) {}
	SoundCommand sndCmd;
	const char *desc;
};

class SoundCommandParser {
public:
	SoundCommandParser(ResourceManager *resMan, SegManager *segMan, AudioPlayer *audio, SciVersion soundVersion);
	~SoundCommandParser();

#ifdef USE_OLD_MUSIC_FUNCTIONS
	void updateSfxState(SfxState *newState) { _state = newState; }
#endif

	reg_t parseCommand(int argc, reg_t *argv, reg_t acc);

	SciMusic *_music;

private:
	Common::Array<MusicEntryCommand*> _soundCommands;
	ResourceManager *_resMan;
	SegManager *_segMan;
#ifdef USE_OLD_MUSIC_FUNCTIONS
	SfxState *_state;
#endif
	AudioPlayer *_audio;
	bool _hasNodePtr;
	SciVersion _soundVersion;
	int _argc;
	reg_t *_argv;
	reg_t _acc;
	int _midiCmd, _controller, _param;

	void cmdInitHandle(reg_t obj, int16 value);
	void cmdPlayHandle(reg_t obj, int16 value);
	void cmdDummy(reg_t obj, int16 value);
	void cmdMuteSound(reg_t obj, int16 value);
	void cmdPauseHandle(reg_t obj, int16 value);
	void cmdResumeHandle(reg_t obj, int16 value);
	void cmdStopHandle(reg_t obj, int16 value);
	void cmdDisposeHandle(reg_t obj, int16 value);
	void cmdVolume(reg_t obj, int16 value);
	void cmdFadeHandle(reg_t obj, int16 value);
	void cmdGetPolyphony(reg_t obj, int16 value);
	void cmdGetPlayNext(reg_t obj, int16 value);

	void initHandle(reg_t obj, bool isSci1);

	void cmdUpdateHandle(reg_t obj, int16 value);
	void cmdUpdateCues(reg_t obj, int16 value);
	void cmdSendMidi(reg_t obj, int16 value);
	void cmdReverb(reg_t obj, int16 value);
	void cmdHoldHandle(reg_t obj, int16 value);
	void cmdGetAudioCapability(reg_t obj, int16 value);
	void cmdSetHandleVolume(reg_t obj, int16 value);
	void cmdSetHandlePriority(reg_t obj, int16 value);
	void cmdSetHandleLoop(reg_t obj, int16 value);
	void cmdSuspendSound(reg_t obj, int16 value);
	void cmdUpdateVolumePriority(reg_t obj, int16 value);

#ifdef USE_OLD_MUSIC_FUNCTIONS
	void changeHandleStatus(reg_t obj, int newStatus);
#endif
};

} // End of namespace Sci

#endif // SCI_SOUNDCMD_H
