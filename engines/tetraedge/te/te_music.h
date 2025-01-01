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

#ifndef TETRAEDGE_TE_TE_MUSIC_H
#define TETRAEDGE_TE_TE_MUSIC_H

#include "audio/mixer.h"
#include "common/mutex.h"
#include "common/fs.h"
#include "common/str.h"

#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_signal.h"
#include "tetraedge/tetraedge.h"

namespace Tetraedge {

class TeMusic : public TeResource {
public:
	TeMusic();
	~TeMusic();

	void close() {
		stop();
	}
	void pause();
	bool play();
	bool repeat();
	void repeat(bool val);
	void resume();
	void stop();

	byte currentData();
	void entry();
	const Common::Path &filePath() const {
		return _rawPath;
	}
	bool isPlaying();
	bool load(const Common::Path &path);
	bool onSoundManagerVolumeChanged();
	Common::Path path();
	void setChannelName(const Common::String &name) {
		_channelName = name;
	}
	const Common::String &channelName() const {
		return _channelName;
	}
	void setFilePath(const Common::Path &name);
	void update();
	void volume(float vol);
	float volume();

	TeSignal0Param &onStopSignal() { return _onStopSignal; }

	void setRetain(bool retain) { _retain = retain; }
	bool retain() const { return _retain; }

private:
	Common::Path _rawPath; // Plain name of file requested
	TetraedgeFSNode _fileNode; // file after finding it
	Common::String _channelName;

	bool _repeat;
	byte _currentData;

	bool _isPlaying;
	bool _isPaused;
	bool _retain;

	float _volume;

	Audio::SoundHandle _sndHandle;
	bool _sndHandleValid;

	Common::Mutex _mutex;
	TeSignal0Param _onStopSignal;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_MUSIC_H
