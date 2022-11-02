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

#ifndef BURIED_VIDEO_WINDOW_H
#define BURIED_VIDEO_WINDOW_H

#include "buried/window.h"

namespace Graphics {
struct Surface;
}

namespace Video {
class VideoDecoder;
}

namespace Buried {

class VideoWindow : public Window {
public:
	VideoWindow(BuriedEngine *vm, Window *parent = 0);
	~VideoWindow();

	// ScummVM-specific interface
	void updateVideo();
	void pauseVideo();
	void resumeVideo();

	// VFW interface
	bool playVideo(); // MCIWndPlay
	bool playToFrame(int frame); // MCIWndPlayTo
	bool seekToFrame(int frame); // MCIWndSeek
	void stopVideo(); // MCIWndStop
	int getCurFrame(); // MCIWndGetPosition
	int getFrameCount(); // MCIWndGetLength
	void setSourceRect(const Common::Rect &srcRect); // MCIWndPutSource
	void setDestRect(const Common::Rect &dstRect); // MCIWndPutDest
	void setAudioTrack(int track); // MCIWndSendString + "setaudio stream to %d"

	bool openVideo(const Common::String &fileName); // MCIWndOpen
	void closeVideo(); // MCIWndClose

	enum Mode {
		kModeClosed, // "Not ready" is stupid
		kModeOpen,
		kModePaused,
		kModePlaying,
		kModeSeeking,
		kModeStopped
	};

	Mode getMode() const { return _mode; } // MCIWndGetMode

	// Window interface
	void onPaint();
	void onKeyUp(const Common::KeyState &key, uint flags);

private:
	Video::VideoDecoder *_video;
	const Graphics::Surface *_lastFrame;
	Mode _mode;
	Graphics::Surface *_ownedFrame;
	bool _needsPalConversion;
	Common::Rect _srcRect, _dstRect;
};

} // End of namespace Buried

#endif
