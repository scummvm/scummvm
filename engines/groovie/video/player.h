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

#ifndef GROOVIE_VIDEO_PLAYER_H
#define GROOVIE_VIDEO_PLAYER_H

#include "common/system.h"
#include "video/subtitles.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Groovie {

class GroovieEngine;

class VideoPlayer {
public:
	VideoPlayer(GroovieEngine *vm);
	virtual ~VideoPlayer() {}

	bool load(Common::SeekableReadStream *file, uint16 flags);
	bool playFrame();
	virtual void resetFlags() {}
	virtual void setOrigin(int16 x, int16 y) {}
	virtual void stopAudioStream() = 0;
	void fastForward();
	bool isFastForwarding();
	virtual void drawString(Graphics::Surface *surface, const Common::String text, int posx, int posy, uint32 color, bool blackBackground) {}
	virtual void copyfgtobg(uint8 arg) {}
	void setOverrideSpeed(bool isOverride);

	void loadSubtitles(const char *fname) { _subtitles.loadSRTFile(fname); }
	void unloadSubtitles();

	virtual bool isFileHandled() { return false; }

protected:
	// To be implemented by subclasses
	virtual uint16 loadInternal() = 0;
	virtual bool playFrameInternal() = 0;

	bool getOverrideSpeed() const { return _overrideSpeed; }

	GroovieEngine *_vm;
	OSystem *_syst;
	Common::SeekableReadStream *_file;
	uint16 _flags;
	Audio::QueuingAudioStream *_audioStream;


private:
	// Synchronization stuff
	bool _begunPlaying;
	bool _overrideSpeed;
	uint16 _fps;
	float _millisBetweenFrames;
	uint32 _lastFrameTime;
	float _frameTimeDrift;
	uint32 _startTime;

	Video::Subtitles _subtitles;

protected:
	virtual void waitFrame();
};

} // End of Groovie namespace

#endif // GROOVIE_VIDEO_PLAYER_H
