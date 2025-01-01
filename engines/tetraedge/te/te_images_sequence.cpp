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

#include "common/file.h"
#include "image/png.h"
#include "graphics/surface.h"
#include "graphics/managed_surface.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_images_sequence.h"

namespace Tetraedge {

TeImagesSequence::TeImagesSequence() : _width(0), _height(0), _curFrame(0), _frameRate(0) {
}

TeImagesSequence::~TeImagesSequence() {
	for (auto surf : _cachedSurfaces) {
		if (surf)
			delete surf;
	}
}

/*static*/
bool TeImagesSequence::matchExtension(const Common::String &extn) {
	return extn == "anim" || extn == "animcached";
}

static bool compareNodes(const TetraedgeFSNode &left, const TetraedgeFSNode &right) {
	return left.getPath().toString('/') < right.getPath().toString('/');
}

bool TeImagesSequence::load(const TetraedgeFSNode &dir) {
	if (!dir.isDirectory()) {
		warning("TeImagesSequence::load:: not a directory %s", dir.toString().c_str());
		return false;
	}

	TetraedgeFSList children;
	if (!dir.getChildren(children, Common::FSNode::kListFilesOnly) || children.empty()) {
		warning("TeImagesSequence::load:: couldn't get children of %s", dir.toString().c_str());
		return false;
	}

	Common::sort(children.begin(), children.end(), compareNodes);
	dir.maybeAddToSearchMan();

	for (TetraedgeFSNode &child : children) {
		const Common::String fileName = child.getName();

		if (fileName.size() <= 10 || fileName.substr(fileName.size() - 7) != "fps.png")
			continue;

		Common::String fstart = fileName.substr(0, fileName.size() - 7);
		int frameno = 0;
		int fps = 0;
		if (sscanf(fstart.c_str(), "%d-%d", &frameno, &fps) != 2) {
			warning("TeImagesSequence::load can't match %s", fileName.c_str());
			continue;
		}

		Common::SeekableReadStream *stream = child.createReadStream();
		if (!stream) {
			warning("TeImagesSequence::load can't open %s", child.toString().c_str());
			continue;
		}

		// Only do the deep check for the first file to get dimensions.
		// If the images are small then cache them to avoid reloading each frame each time.
		if (!_width || (_width < 100 && _height < 100)) {
			Image::PNGDecoder png;
			if (!png.loadStream(*stream)) {
				warning("Image sequence failed to load png %s", child.toString().c_str());
				delete stream;
				return false;
			}

			_frameRate = fps;
			const Graphics::Surface *pngsurf = png.getSurface();
			assert(pngsurf);
			_width = pngsurf->w;
			_height = pngsurf->h;
			if (_width < 100 && _height < 100) {
				Graphics::ManagedSurface *surf = new Graphics::ManagedSurface();
				surf->copyFrom(*pngsurf);
				_cachedSurfaces.push_back(surf);
			} else {
				_cachedSurfaces.push_back(nullptr);
			}
		} else {
			_cachedSurfaces.push_back(nullptr);
		}

		_files.push_back(child);
		delete stream;
	}

	return true;
}


bool TeImagesSequence::update(uint i, TeImage &imgout) {
	_curFrame = i;

	if (i >= _files.size())
		return false;

	if (_cachedSurfaces[i] == nullptr) {
		Common::SeekableReadStream *stream = _files[i].createReadStream();
		if (!stream)
			error("Open %s failed.. it was ok before?", _files[i].getName().c_str());

		Image::PNGDecoder png;
		if (!png.loadStream(*stream)) {
			warning("Image sequence failed to load png %s", _files[i].getName().c_str());
			delete stream;
			return false;
		}

		const Graphics::Surface *surf = png.getSurface();
		assert(surf);

		imgout.setAccessName(_files[i].getPath());

		if (imgout.w == surf->w && imgout.h == surf->h && imgout.format == surf->format) {
			imgout.copyFrom(*surf);
			delete stream;
			return true;
		}
	} else {
		const Graphics::ManagedSurface *surf = _cachedSurfaces[i];
		if (imgout.w == surf->w && imgout.h == surf->h && imgout.format == surf->format) {
			imgout.setAccessName(_files[i].getPath());
			imgout.copyFrom(*surf);
			return true;
		}
	}

	error("TODO: Implement TeImagesSequence::update for different sizes");
}

bool TeImagesSequence::isAtEnd() {
	return _curFrame >= _files.size();
}

TeImage::Format TeImagesSequence::imageFormat() {
	return TeImage::RGBA8;
}

} // end namespace Tetraedge
