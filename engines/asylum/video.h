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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_VIDEO_H_
#define ASYLUM_VIDEO_H_

#include "sound/mixer.h"
#include "graphics/surface.h"
#include "graphics/video/smk_decoder.h"
#include "graphics/video/video_player.h"

#include "common/array.h"
#include "common/events.h"
#include "common/system.h"
#include "common/list.h"

#include "asylum/asylum.h"

namespace Asylum {

struct VideoSubtitle {
	int frameStart;
	int frameEnd;
	int textRes;
};

enum VideoSubtitles {
	kSubtitlesOff = 0,
	kSubtitlesOn = 1
};

class VideoText;

class VideoPlayer : public Graphics::VideoPlayer {
public:
	VideoPlayer(Graphics::VideoDecoder* decoder);
	virtual ~VideoPlayer();

	bool playVideoWithSubtitles(Common::List<Common::Event> &stopEvents, int videoNumber);

private:
	void performPostProcessing(byte *screen);

	VideoText					*_text;
	Common::Array<VideoSubtitle> _subtitles;
};

class Video {
public:
	Video(Audio::Mixer *mixer);
	virtual ~Video();

	bool playVideo(int number, VideoSubtitles subtitles);

private:
	Common::List<Common::Event> _stopEvents;
	Graphics::SmackerDecoder	*_smkDecoder;
	VideoPlayer					*_player;
}; // end of class Video

// The VideoText class has some methods from the Text class, 
// but it differs from the text class: this class draws text
// to a predefined screen buffer, whereas the Text class draws
// text directly to the screen
class VideoText {
public:
	VideoText();
	~VideoText();

	void loadFont(ResourcePack *resPack, uint32 resId);
	void drawMovieSubtitle(byte *screenBuffer, uint32 resId);

private:
	uint32 getTextWidth(const char *text);

	void drawText(byte *screenBuffer, int x, int y, const char *text);
	void copyToVideoFrame(byte *screenBuffer, GraphicFrame *frame, int x, int y);

	GraphicResource *_fontResource;
	ResourcePack	*_textPack;
	uint8           _curFontFlags;

}; // end of class VideoText

} // end of namespace Asylum

#endif
