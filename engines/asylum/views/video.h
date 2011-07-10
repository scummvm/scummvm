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
 */

#ifndef ASYLUM_VIDEO_H
#define ASYLUM_VIDEO_H

#include "asylum/eventhandler.h"
#include "asylum/shared.h"

#include "common/array.h"
#include "common/events.h"
#include "common/system.h"
#include "common/list.h"

#include "audio/mixer.h"

#include "graphics/surface.h"

#include "video/smk_decoder.h"

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
	 * @param videoNumber 	   The video number.
	 * @param handler          The previous event handler.
	 */
	void play(int32 videoNumber, EventHandler *handler);

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

	Video::SmackerDecoder *_smkDecoder;
	Common::Array<VideoSubtitle> _subtitles;

	int32 _currentMovie;
	int32 _subtitleIndex;
	int32 _subtitleCounter;
	ResourceId _previousFont;
	bool _done;


	/**
	 * Plays the given file.
	 *
	 * @param filename 		Filename of the file.
	 * @param showSubtitles true to show, false to hide the subtitles.
	 */
	void play(Common::String filename, bool showSubtitles);

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

#endif
