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

#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "hollywood/runaway/video.h"
#include "video/avi_decoder.h"
#include "video/bink_decoder.h"

namespace Hollywood {

namespace {

struct FilmOverlay {
	char chapter;
	byte movie, portrait, first, count;
	int16 left, top, right, bottom;
};

// Crop only the animated face; the GOG movies retain the surrounding still.
const FilmOverlay kFilmOverlays[] = {
	{'B', 1, 0, 0, 12, 536, 222, 614, 300},
	{'B', 2, 0, 0, 12, 526, 260, 657, 411},
	{'B', 2, 1, 12, 12, 681, 121, 788, 282},
	{'B', 7, 0, 0, 12, 406, 46, 531, 177},
	{'B', 7, 1, 12, 12, 270, 46, 751, 599},
	{'B', 7, 2, 24, 12, 280, 26, 655, 599},
	{'B', 8, 0, 0, 12, 587, 74, 648, 162},
	{'B', 8, 1, 12, 12, 258, 318, 314, 374},
	{'B', 8, 2, 24, 12, 402, 28, 532, 230},
	{'B', 9, 0, 0, 12, 554, 309, 687, 456},
	{'B', 9, 1, 12, 12, 747, 55, 887, 219},
	{'B', 9, 2, 24, 12, 317, 144, 726, 598},
	{'B', 9, 3, 36, 6, 1, 1, 230, 248},
	{'B', 9, 4, 42, 12, 312, 163, 438, 319},
	{'B', 9, 5, 54, 12, 256, 144, 708, 600},
	{'B', 9, 6, 66, 12, 80, 76, 172, 170},
	{'B', 9, 7, 78, 12, 174, 24, 564, 592},
	{'B', 9, 8, 90, 12, 364, 8, 904, 562},
	{'B', 9, 9, 102, 12, 280, 36, 402, 184},
	{'B', 9, 10, 114, 12, 464, 12, 580, 164},
	{'B', 9, 11, 126, 12, 226, 16, 338, 158},
	{'B', 9, 12, 138, 6, 531, 323, 641, 454},
	{'B', 9, 13, 144, 12, 881, 0, 1024, 161},
	{'D', 1, 0, 0, 12, 554, 6, 632, 134},
	{'D', 1, 1, 12, 12, 302, 184, 362, 284},
	{'D', 1, 2, 24, 12, 339, 112, 408, 197},
	{'D', 1, 3, 36, 12, 360, 178, 570, 530},
	{'D', 1, 4, 48, 12, 395, 148, 706, 557},
	{'D', 1, 5, 60, 12, 318, 118, 581, 485},
	{'D', 5, 0, 0, 12, 190, 444, 458, 600},
	{'D', 5, 1, 12, 12, 170, 442, 476, 600},
	{'D', 7, 0, 0, 12, 360, 104, 548, 400},
	{'D', 7, 1, 12, 12, 278, 268, 468, 428},
	{'D', 8, 0, 0, 6, 79, 67, 176, 170},
	{'D', 10, 0, 0, 6, 180, 46, 698, 600},
	{'D', 9, 0, 0, 12, 474, 36, 578, 234},
	{'E', 2, 0, 0, 6, 490, 192, 586, 300},
	{'E', 2, 1, 6, 6, 532, 38, 646, 172},
	{'E', 2, 2, 12, 12, 627, 170, 840, 388},
	{'E', 2, 3, 24, 12, 590, 240, 641, 296},
	{'E', 2, 4, 36, 12, 426, 254, 480, 314},
	{'E', 6, 0, 0, 12, 388, 196, 530, 408},
	{'F', 1, 0, 0, 12, 644, 122, 704, 204},
	{'F', 1, 1, 12, 12, 516, 104, 566, 170},
	{'F', 2, 0, 0, 12, 618, 198, 668, 282},
	{'F', 2, 1, 12, 6, 581, 164, 825, 534},
	{'F', 2, 2, 18, 6, 329, 74, 406, 177},
	{'F', 2, 3, 24, 6, 772, 278, 916, 506},
	{'F', 2, 4, 30, 12, 584, 98, 632, 164},
	{'G', 4, 0, 0, 6, 0, 0, 1024, 600},
};

} // End of anonymous namespace

bool RunawayVideo::load(const char *name) {
	close();
	Common::File file;
	if (!file.open(Common::Path(name)))
		return false;
	const int64 fileSize = file.size();
	if (fileSize < 1024 || fileSize > 256 * 1024 * 1024)
		return false;
	_data.resize((uint)fileSize);
	if (file.read(_data.data(), _data.size()) != _data.size())
		return false;
	const uint32 tag = READ_BE_UINT32(_data.data());
	if (tag != MKTAG('R', 'I', 'F', 'F') && (tag >> 8) != MKTAG(0, 'B', 'I', 'K')) {
		Common::File codebook;
		byte entry[2048];
		bool found = false;
		if (codebook.open("DATAVC00.000")) {
			// GOG stores named Bink headers, XOR chained with the previous byte.
			const byte key = codebook.readUint32LE();
			const uint count = codebook.readUint32LE();
			if (count > 256 || codebook.size() != 8 + count * (15 + sizeof(entry)))
				return false;
			for (uint i = 0; i < count; ++i) {
				byte encodedName[15];
				codebook.seek(8 + i * sizeof(encodedName));
				if (codebook.read(encodedName, sizeof(encodedName)) != sizeof(encodedName))
					return false;
				char decodedName[16] = {};
				byte previous = key;
				for (uint j = 0; j < sizeof(encodedName); ++j) {
					decodedName[j] = encodedName[j] ^ previous;
					previous = encodedName[j];
				}
				if (!Common::String(decodedName).equalsIgnoreCase(name))
					continue;
				codebook.seek(8 + count * 15 + i * sizeof(entry));
				if (codebook.read(entry, sizeof(entry)) != sizeof(entry) || memcmp(entry + 1024, _data.data(), 1024))
					return false;
				previous = key;
				for (uint j = 0; j < 1024; ++j) {
					_data[j] = entry[j] ^ previous;
					previous = entry[j];
				}
				found = true;
				break;
			}
		} else if (codebook.open("DATAVC00.001")) {
			// CD releases store AVI headers as sums of the original and replacement.
			while (codebook.read(entry, sizeof(entry)) == sizeof(entry)) {
				if (memcmp(entry + 1024, _data.data(), 1024))
					continue;
				for (uint i = 0; i < 1024; ++i)
					_data[i] = entry[i] - entry[1024 + i];
				found = true;
				break;
			}
		}
		if (!found)
			return false;
	}
	if ((READ_BE_UINT32(_data.data()) >> 8) == MKTAG(0, 'B', 'I', 'K')) {
#ifdef USE_BINK
		_decoder.reset(new Video::BinkDecoder());
#else
		warning("Runaway: this release requires Bink video support");
		return false;
#endif
	} else if (READ_BE_UINT32(_data.data()) == MKTAG('R', 'I', 'F', 'F') &&
			READ_BE_UINT32(_data.data() + 8) == MKTAG('A', 'V', 'I', ' ')) {
		_decoder.reset(new Video::AVIDecoder());
	} else {
		return false;
	}
	if (!_decoder->loadStream(new Common::MemoryReadStream(_data.data(), _data.size())) ||
			_decoder->getAudioTrackCount() || !_decoder->getFrameCount()) {
		close();
		return false;
	}
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	if (!_decoder->setOutputPixelFormat(format) || !decodeToFrame(0)) {
		close();
		return false;
	}
	return true;
}

void RunawayVideo::close() {
	_decoder.reset();
	_frame = nullptr;
	_data.clear();
}

uint RunawayVideo::frameAtTime(uint32 milliseconds) const {
	if (!_decoder)
		return 0;
	const Audio::Timestamp duration = _decoder->getDuration();
	const uint frames = duration.totalNumberOfFrames();
	return frames ? (uint64)milliseconds * frameCount() * duration.framerate() / (1000ULL * frames) : 0;
}

bool RunawayVideo::seekToFrame(uint frame) {
	if (!_decoder || frame >= _decoder->getFrameCount())
		return false;
	// Keep Bink's decoded reference frames for forward seeks. The GOG movies
	// have only an initial keyframe, so seeking would decode them from scratch.
	if (_decoder->getCurFrame() != (int)frame &&
			(_decoder->getCurFrame() > (int)frame || READ_BE_UINT32(_data.data()) == MKTAG('R', 'I', 'F', 'F'))) {
		_frame = nullptr;
		if (!_decoder->seekToFrame(frame))
			return false;
	}
	return decodeToFrame(frame);
}

bool RunawayVideo::decodeToFrame(uint frame) {
	if (!_decoder || frame >= _decoder->getFrameCount())
		return false;
	if (_decoder->getCurFrame() > (int)frame)
		return seekToFrame(frame);
	while (_decoder->getCurFrame() < (int)frame && !_decoder->endOfVideo()) {
		const Graphics::Surface *decoded = _decoder->decodeNextFrame();
		if (decoded)
			_frame = decoded;
		else {
			warning("Runaway: unable to decode video frame %d", _decoder->getCurFrame());
			return false;
		}
	}
	return _frame != nullptr && _decoder->getCurFrame() == (int)frame;
}

void RunawayVideo::draw(Graphics::ManagedSurface &surface, const Common::Rect &destination) const {
	if (_frame)
		surface.blitFrom(*_frame, Common::Rect(_frame->w, _frame->h), destination);
}

void RunawayVideo::drawRegion(Graphics::ManagedSurface &surface, const Common::Rect &source, const Common::Rect &destination) const {
	if (_frame && Common::Rect(_frame->w, _frame->h).contains(source))
		surface.blitFrom(*_frame, source, destination);
}

bool RunawayFilmPortrait::load(char chapter, uint movie) {
	if (_loaded && _chapter == chapter && _movie == movie)
		return true;
	_chapter = chapter;
	_movie = movie;
	_video.close();
	_faces.clear();
	_loadedPortrait = -1;
	const Common::String name = Common::String::format("DATAV%c%02u.000", chapter, movie);
	if (Common::File::exists(Common::Path(name)))
		_loaded = _video.load(name.c_str());
	else
		_loaded = _archive.open(Common::Path(Common::String::format("DATAV%c%02u.002", chapter, movie)), 40);
	return _loaded;
}

bool RunawayFilmPortrait::draw(Graphics::ManagedSurface &surface, const Common::Rect &destination, uint portrait, uint mouth) {
	if (_video.frameCount()) {
		if (_loadedPortrait != (int)portrait) {
			const FilmOverlay *overlay = nullptr;
			for (uint i = 0; i < ARRAYSIZE(kFilmOverlays); ++i) {
				if (kFilmOverlays[i].chapter == _chapter && kFilmOverlays[i].movie == _movie && kFilmOverlays[i].portrait == portrait)
					overlay = &kFilmOverlays[i];
			}
			if (!overlay)
				return false;
			_faceRect = Common::Rect(overlay->left, overlay->top, overlay->right, overlay->bottom);
			_faces.resize(overlay->count);
			for (uint i = 0; i < _faces.size(); ++i) {
				if (!_video.decodeToFrame(overlay->first + i))
					return false;
				_faces[i].create(_faceRect.width(), _faceRect.height(), surface.format);
				_video.drawRegion(_faces[i], _faceRect, Common::Rect(_faceRect.width(), _faceRect.height()));
			}
			_loadedPortrait = portrait;
		}
		const Common::Rect target(destination.left + _faceRect.left * destination.width() / _video.width(),
			destination.top + _faceRect.top * destination.height() / _video.height(),
			destination.left + _faceRect.right * destination.width() / _video.width(),
			destination.top + _faceRect.bottom * destination.height() / _video.height());
		surface.blitFrom(_faces[mouth % _faces.size()], Common::Rect(_faceRect.width(), _faceRect.height()), target);
		return true;
	}
	if (_loadedPortrait != (int)portrait) {
		Common::Array<byte> patch;
		const bool phone = _chapter == 'D' && _movie == 8;
		_portrait.create(phone ? 288 : 512, phone ? 288 : 300, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
		_talking.create(_portrait.w, _portrait.h, _portrait.format);
		if (!_archive.readVariableChunk(portrait * 2, patch) || !applyRunawayPatch(patch, _portrait) ||
				!_mouth.load(_archive, portrait * 2 + 1, 12))
			return false;
		_loadedPortrait = portrait;
	}
	_talking.blitFrom(_portrait);
	_mouth.draw(_talking, mouth, 0, 0);
	surface.blitFrom(_talking, Common::Rect(_talking.w, _talking.h), destination);
	return true;
}

} // namespace Hollywood
