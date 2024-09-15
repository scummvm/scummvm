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

#ifndef DGDS_IMAGE_H
#define DGDS_IMAGE_H

#include <common/ptr.h>
#include <common/rect.h>
#include <graphics/palette.h>

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
class ManagedSurface;
}

namespace Dgds {

class Decompressor;
class DgdsChunkReader;
class ResourceManager;

enum ImageFlipMode {
	kImageFlipNone = 0,
	kImageFlipV = 1,
	kImageFlipH = 2,
	kImageFlipHV = 3,
};

class Image {
public:
	Image(ResourceManager *resourceMan, Decompressor *decompressor);
	virtual ~Image();

	void drawScreen(const Common::String &filename, Graphics::ManagedSurface &dst);

	void loadBitmap(const Common::String &filename);
	int frameCount(const Common::String &filename);
	void drawBitmap(uint frameno, int x, int y, const Common::Rect &drawWin, Graphics::ManagedSurface &dst, ImageFlipMode flip = kImageFlipNone, int dstWidth = 0, int dstHeight = 0) const;
	void drawScrollBitmap(int16 x, int16 y, int16 width, int16 height, int16 scrollX, int16 scrollY, const Common::Rect &drawWin, Graphics::ManagedSurface &dst) const;

	Common::SharedPtr<Graphics::ManagedSurface> getSurface(uint frameno) const;

	const Common::Array<Common::SharedPtr<Graphics::ManagedSurface>> &getFrames() const { return _frames; }

	int16 width(uint frameno) const;
	int16 height(uint frameno) const;

	int loadedFrameCount() const { return _frames.size(); }
	bool isLoaded() const { return !_frames.empty(); }

	const Common::String &getFilename() const { return _filename; }

private:
	void loadBitmap4(Graphics::ManagedSurface *surf, uint32 toffset, Common::SeekableReadStream *stream, bool highByte);
	void loadBitmap8(Graphics::ManagedSurface *surf, uint32 toffset, Common::SeekableReadStream *stream);
	uint32 loadVQT(Graphics::ManagedSurface *surf, uint32 toffset, Common::SeekableReadStream *stream);
	bool loadSCN(Graphics::ManagedSurface *surf, Common::SeekableReadStream *stream);

	Common::Array<Common::SharedPtr<Graphics::ManagedSurface>> _frames;
	ResourceManager *_resourceMan;
	Decompressor *_decompressor;

	Common::String _filename; // the file this was loaded from - only used for debugging

	// Used if the image is a scrolling image.
	int16 _matrixX;
	int16 _matrixY;
	Common::Array<uint16> _tileMatrix;
};

} // End of namespace Dgds

#endif // DGDS_IMAGE_H
