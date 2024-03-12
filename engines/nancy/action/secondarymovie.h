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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_SECONDARYMOVIE_H
#define NANCY_ACTION_SECONDARYMOVIE_H

#include "engines/nancy/action/actionrecord.h"

namespace Video {
class VideoDecoder;
}

namespace Nancy {
namespace Action {

class InteractiveVideo;

// Plays an AVF or Bink video. Optionally supports:
// - playing a sound;
// - reverse playback;
// - moving with the scene's background frame;
// - hiding of player cursor (and thus, disabling input);
// - setting event flags on a specific frame, as well as at the end of the video;
// - changing the scene after playback ends
// Mostly used for cinematics, with some occasional uses for background animations
class PlaySecondaryMovie : public RenderActionRecord {
	friend class InteractiveVideo;
public:
	static const byte kMovieSceneChange			= 5;
	static const byte kMovieNoSceneChange		= 6;

	static const byte kPlayerCursorAllowed		= 1;
	static const byte kNoPlayerCursorAllowed	= 2;

	static const byte kPlayMovieForward			= 1;
	static const byte kPlayMovieReverse			= 2;

	struct FlagAtFrame {
		int16 frameID;
		FlagDescription flagDesc;
	};

	PlaySecondaryMovie() : RenderActionRecord(8) {}
	virtual ~PlaySecondaryMovie();

	void init() override;
	void onPause(bool pause) override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::Path _videoName;
	Common::Path _paletteName;
	Common::Path _bitmapOverlayName;

	uint16 _videoType = kVideoPlaytypeAVF;
	uint16 _videoFormat = kLargeVideoFormat;
	uint16 _videoSceneChange = kMovieNoSceneChange;
	byte _playerCursorAllowed = kPlayerCursorAllowed;
	byte _playDirection = kPlayMovieForward;
	uint16 _firstFrame = 0;
	uint16 _lastFrame = 0;
	Common::Array<FlagAtFrame> _frameFlags;
	MultiEventFlagDescription _triggerFlags;

	SoundDescription _sound;

	SceneChangeDescription _sceneChange;
	Common::Array<SecondaryVideoDescription> _videoDescs;

	Video::VideoDecoder *_decoder = nullptr;

protected:
	Common::String getRecordTypeName() const override { return "PlaySecondaryMovie"; }
	bool isViewportRelative() const override { return true; }

	Graphics::ManagedSurface _fullFrame;
	int _curViewportFrame = -1;
	bool _isFinished = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SECONDARYMOVIE_H
