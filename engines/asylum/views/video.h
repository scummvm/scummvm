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

#ifndef ASYLUM_VIEWS_VIDEO_H
#define ASYLUM_VIEWS_VIDEO_H

#include "common/array.h"
#include "common/events.h"
#include "common/system.h"
#include "common/list.h"

#include "audio/mixer.h"

#include "graphics/surface.h"

#include "video/video_decoder.h"

#include "asylum/eventhandler.h"
#include "asylum/shared.h"

#include "asylum/system/screen.h"

namespace Asylum {

class AsylumEngine;
class GraphicResource;
class VideoText;
struct GraphicFrame;

struct VideoSubtitle {
	int frameStart;
	int frameEnd;
	ResourceId resourceId;
};

class VideoPlayer : public EventHandler {
public:
	VideoPlayer(AsylumEngine *engine, Audio::Mixer *mixer);
	virtual ~VideoPlayer();

	/**
	 * Plays a video.
	 *
	 * @param videoNumber      The video number.
	 * @param handler          The previous event handler.
	 */
	void play(uint32 videoNumber, EventHandler *handler);

	/**
	 * Handle event.
	 *
	 * @param evt The event.
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool handleEvent(const AsylumEvent &evt);

private:
	AsylumEngine *_vm;

	Video::VideoDecoder *_decoder;
	Common::Array<VideoSubtitle> _subtitles;

	int32 _currentMovie;
	int32 _subtitleIndex;
	int32 _subtitleCounter;
	ResourceId _previousFont;
	bool _done;
	byte _subtitlePalette[PALETTE_SIZE];

	/**
	 * Plays the given file.
	 *
	 * @param filename 		Filename of the file.
	 * @param showSubtitles true to show, false to hide the subtitles.
	 */
	void play(const Common::String &filename, bool showSubtitles);

	/**
	 * Sets up the palette.
	 */
	void setupPalette();

	/**
	 * Loads the subtitles (vids.cap)
	 */
	void loadSubtitles();
};

} // end of namespace Asylum

#endif // ASYLUM_VIEWS_VIDEO_H
