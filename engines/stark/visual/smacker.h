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

#ifndef STARK_VISUAL_SMACKER_H
#define STARK_VISUAL_SMACKER_H

#include "engines/stark/visual/visual.h"

#include "common/rect.h"
#include "common/stream.h"

namespace Video {
class VideoDecoder;
}

namespace Graphics {
struct Surface;
}

namespace Stark {

namespace Gfx {
class Driver;
class SurfaceRenderer;
class Texture;
}

class VisualSmacker : public Visual {
public:
	static const VisualType TYPE = Visual::kSmackerStream;

	VisualSmacker(Gfx::Driver *gfx);
	virtual ~VisualSmacker();

	void loadSmacker(Common::SeekableReadStream *stream);
	void loadBink(Common::SeekableReadStream *stream);
	void update();
	void render(const Common::Point &position);
	bool isDone();

	/** Reset the video to resume playing from the beginning */
	void rewind();

	/** Perform a transparency hit test on a point */
	bool isPointSolid(const Common::Point &point) const;

	/**
	 * Load the size from a Smacker video
	 */
	void readOriginalSize(Common::SeekableReadStream *stream);

	int getWidth() const;
	int getHeight() const;

	int getFrameNumber() const;
	Common::Point getPosition() const { return _position; }
	void setPosition(const Common::Point &pos) { _position = pos; }

	/** Get the total duration in milliseconds for the video */
	uint32 getDuration() const;

	/** Get the current time in the video since the beginning */
	uint32 getCurrentTime() const;

	void overrideFrameRate(int32 framerate);

	/** Pause or resume the video */
	void pause(bool pause);

private:
	void init();

	Video::VideoDecoder *_decoder;
	const Graphics::Surface *_surface;

	Common::Point _position;
	int32 _originalWidth;
	int32 _originalHeight;

	Gfx::Driver *_gfx;
	Gfx::SurfaceRenderer *_surfaceRenderer;
	Gfx::Texture *_texture;
	int32 _overridenFramerate;
};

} // End of namespace Stark

#endif // STARK_VISUAL_SMACKER_H
