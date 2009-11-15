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

#include "common/list.h"
#include "sci/engine/state.h"

namespace Sci {

class SciMusic;
class SoundCommandParser;
typedef void (SoundCommandParser::*SoundCommand)(reg_t obj, SongHandle handle, int value);

struct SciSoundCommand {
	SciSoundCommand(const char* d, SoundCommand c) : desc(d), sndCmd(c) {}
	const char* desc;
	SoundCommand sndCmd;
};

class SoundCommandParser {
public:
	SoundCommandParser(ResourceManager *resMan, SegManager *segMan, SfxState *state, AudioPlayer *audio, SciVersion doSoundVersion);
	~SoundCommandParser();

	reg_t parseCommand(int argc, reg_t *argv, reg_t acc);

private:
	SciMusic *_music;
	Common::Array<SciSoundCommand*> _soundCommands;
	ResourceManager *_resMan;
	SegManager *_segMan;
	SfxState *_state;
	AudioPlayer *_audio;
	bool _hasNodePtr;
	SciVersion _doSoundVersion;
	reg_t _acc;
	int _midiCmd, _controller, _param;

	Common::List<reg_t> _soundList;

	void cmdInitHandle(reg_t obj, SongHandle handle, int value);
	void cmdPlayHandle(reg_t obj, SongHandle handle, int value);
	void cmdDummy(reg_t obj, SongHandle handle, int value);
	void cmdMuteSound(reg_t obj, SongHandle handle, int value);
	void cmdSuspendHandle(reg_t obj, SongHandle handle, int value);
	void cmdResumeHandle(reg_t obj, SongHandle handle, int value);
	void cmdStopHandle(reg_t obj, SongHandle handle, int value);
	void cmdDisposeHandle(reg_t obj, SongHandle handle, int value);
	void cmdVolume(reg_t obj, SongHandle handle, int value);
	void cmdHandlePriority(reg_t obj, SongHandle handle, int value);
	void cmdFadeHandle(reg_t obj, SongHandle handle, int value);
	void cmdGetPolyphony(reg_t obj, SongHandle handle, int value);
	void cmdGetPlayNext(reg_t obj, SongHandle handl, int valuee);

	void initHandle(reg_t obj, SongHandle handle, bool isSci1);
	void changeHandleStatus(reg_t obj, SongHandle handle, int newStatus);

	void cmdUpdateHandle(reg_t obj, SongHandle handle, int value);
	void cmdUpdateCues(reg_t obj, SongHandle handle, int value);
	void cmdSendMidi(reg_t obj, SongHandle handle, int value);
	void cmdReverb(reg_t obj, SongHandle handle, int value);
	void cmdHoldHandle(reg_t obj, SongHandle handle, int value);
	void cmdGetAudioCapability(reg_t obj, SongHandle handle, int value);
	void cmdSetHandleVolume(reg_t obj, SongHandle handle, int value);
	void cmdSetHandlePriority(reg_t obj, SongHandle handle, int value);
	void cmdSetHandleLoop(reg_t obj, SongHandle handle, int value);
	void cmdSuspendSound(reg_t obj, SongHandle handle, int value);
	void cmdUpdateVolumePriority(reg_t obj, SongHandle handle, int value);
};

} // End of namespace Sci

#endif // SCI_SOUNDCMD_H
