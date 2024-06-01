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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DGDS_IMAGE_H
#define DGDS_IMAGE_H

#include <common/ptr.h>
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

class Image {
public:
	Image(ResourceManager *resourceMan, Decompressor *decompressor);
	virtual ~Image();

	void drawScreen(const Common::String &filename, Graphics::ManagedSurface &dst);

	void loadBitmap(const Common::String &filename);
	int frameCount(const Common::String &filename);
	void drawBitmap(uint frameno, int x, int y, const Common::Rect &drawWin, Graphics::ManagedSurface &dst, bool flip = false) const;

	Common::SharedPtr<Graphics::ManagedSurface> getSurface(uint frameno);

	const Common::Array<Common::SharedPtr<Graphics::ManagedSurface>> &getFrames() { return _frames; }

	int16 width(uint frameno) const;
	int16 height(uint frameno) const;

	int loadedFrameCount() const { return _frames.size(); }

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
};

} // End of namespace Dgds

#endif // DGDS_IMAGE_H
