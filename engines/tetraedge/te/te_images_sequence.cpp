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

#include "tetraedge/te/te_images_sequence.h"

namespace Tetraedge {

TeImagesSequence::TeImagesSequence() : _width(0), _height(0), _curFrame(0) {
}

TeImagesSequence::~TeImagesSequence() {
}

/*static*/
bool TeImagesSequence::matchExtension(const Common::String &extn) {
	return extn == "anim";
}

static bool compareNodes(const Common::FSNode &left, const Common::FSNode &right) {
	return left.getPath() < right.getPath();
}

bool TeImagesSequence::load(const Common::Path &path) {
	Common::FSNode directory(path);
	if (!directory.isDirectory()) {
		warning("TeImagesSequence::load:: not a directory %s", path.toString().c_str());
		return false;
	}

	Common::FSList children;
	if (!directory.getChildren(children, Common::FSNode::kListFilesOnly) || children.empty()) {
		warning("TeImagesSequence::load:: couldn't get children of %s", path.toString().c_str());
		return false;
	}

	Common::sort(children.begin(), children.end(), compareNodes);

	SearchMan.addDirectory(path.toString(), directory);

	for (Common::FSNode &child : children) {
		Common::String filePathStr = child.getPath();

		if (filePathStr.size() <= 10 || filePathStr.substr(filePathStr.size() - 7) != "fps.png")
			continue;

		Common::Path filePath(filePathStr);
		Common::String fname = filePath.getLastComponent().toString();
		Common::String fstart = fname.substr(0, fname.size() - 7);
		int frameno = 0;
		int fps = 0;
		if (sscanf(fstart.c_str(), "%d-%d", &frameno, &fps) != 2) {
			warning("TeImagesSequence::load can't match %s", fname.c_str());
			continue;
		}

		Common::SeekableReadStream *stream = child.createReadStream();
		if (!stream) {
			warning("TeImagesSequence::load can't open %s", filePath.toString().c_str());
			continue;
		}

		// Only do the deep check for the first file to get dimensions.
		if (!_width) {
			Image::PNGDecoder png;
			if (!png.loadStream(*stream)) {
				warning("Image sequence failed to load png %s", filePath.toString().c_str());
				delete stream;
				return false;
			}

			const Graphics::Surface *surf = png.getSurface();
			assert(surf);
			_width = surf->w;
			_height = surf->h;
			_frameRate = fps;
		}

		_files.push_back(child);
		delete stream;
	}

	return true;
}


bool TeImagesSequence::update(unsigned long i, TeImage &imgout) {
	_curFrame = i;

	if (i >= _files.size())
		return false;

	Common::SeekableReadStream *stream = _files[_curFrame].createReadStream();
	if (!stream)
		error("Open %s failed.. it was ok before?", _files[_curFrame].getName().c_str());

	Image::PNGDecoder png;
	if (!png.loadStream(*stream)) {
		warning("Image sequence failed to load png %s", _files[_curFrame].getName().c_str());
		delete stream;
		return false;
	}

	const Graphics::Surface *surf = png.getSurface();
	assert(surf);

	imgout.setAccessName(_files[_curFrame].getPath());

	if (imgout.w == surf->w && imgout.h == surf->h && imgout.format == surf->format) {
		imgout.copyFrom(*surf);
		delete stream;
		return true;
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
