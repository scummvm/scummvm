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
	{'G', 3, 0, 0, 6, 0, 0, 1024, 600},
	{'G', 3, 1, 6, 6, 486, 196, 590, 334},
	{'G', 3, 2, 12, 12, 452, 116, 590, 356},
	{'G', 3, 3, 24, 6, 0, 0, 1024, 600},
	{'G', 4, 0, 0, 6, 0, 0, 1024, 600},
	{'H', 4, 0, 0, 12, 144, 118, 862, 600},
	{'H', 4, 1, 12, 6, 396, 206, 767, 600},
	{'H', 4, 2, 18, 12, 333, 128, 799, 600},
	{'H', 7, 0, 0, 12, 0, 0, 288, 288},
	{'H', 7, 1, 12, 12, 0, 0, 288, 288},
	{'H', 7, 2, 24, 12, 0, 0, 288, 288},
	{'H', 8, 0, 0, 12, 314, 122, 759, 600},
	{'H', 8, 1, 12, 12, 285, 64, 743, 600},
	{'H', 8, 2, 24, 12, 303, 100, 726, 600},
	{'H', 9, 0, 0, 6, 456, 190, 608, 356},
	{'H', 9, 1, 6, 6, 752, 190, 936, 376},
	{'H', 9, 2, 12, 6, 112, 120, 280, 352},
	{'I', 2, 0, 0, 12, 90, 79, 182, 181},
	{'I', 2, 1, 12, 12, 382, 42, 484, 146},
	{'I', 2, 2, 24, 6, 682, 150, 760, 260},
	{'I', 2, 3, 30, 12, 608, 191, 688, 282},
	{'I', 2, 4, 42, 12, 156, 123, 350, 371},
	{'I', 2, 5, 54, 12, 405, 101, 712, 453},
	{'I', 2, 6, 66, 12, 147, 15, 427, 329},
	{'I', 2, 7, 78, 12, 457, 43, 798, 418},
	{'I', 3, 0, 0, 12, 341, 103, 544, 352},
	{'I', 3, 1, 12, 12, 309, 89, 586, 417},
	{'I', 3, 2, 24, 6, 554, 238, 596, 290},
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
	_frame = _decodedFrame = nullptr;
	_data.clear();
	_cachedFrames.clear();
	_cacheBytes = 0;
	_heldFrame.free();
}

bool RunawayVideo::isBink() const {
	return _decoder && (READ_BE_UINT32(_data.data()) >> 8) == MKTAG(0, 'B', 'I', 'K');
}

void RunawayVideo::cacheFrames(uint first, uint last) {
	if (!isBink() || first > last || last >= frameCount()) {
		return;
	}
	_cachedFrames.resize(frameCount());
	uint count = 0;
	for (uint i = first; i <= last; ++i) {
		if (!_cachedFrames[i].enabled) {
			++count;
		}
	}
	// Retain short shots revisited by the script, not entire long movies.
	const uint64 bytes = (uint64)count * width() * height() * _frame->format.bytesPerPixel;
	if (_cacheBytes + bytes > 64 * 1024 * 1024) {
		return;
	}
	_cacheBytes += (uint32)bytes;
	for (uint i = first; i <= last; ++i) {
		_cachedFrames[i].enabled = true;
		if ((int)i == _decoder->getCurFrame() && _decodedFrame) {
			_cachedFrames[i].surface.copyFrom(*_decodedFrame);
		}
	}
}

bool RunawayVideo::useCachedFrame(uint frame) {
	if (frame >= _cachedFrames.size() || !_cachedFrames[frame].surface.w) {
		return false;
	}
	_frame = &_cachedFrames[frame].surface.rawSurface();
	return true;
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
	if (useCachedFrame(frame)) {
		return true;
	}
	// Keep Bink's decoded reference frames for forward seeks. The GOG movies
	// have only an initial keyframe, so seeking would decode them from scratch.
	if (_decoder->getCurFrame() != (int)frame &&
			(_decoder->getCurFrame() > (int)frame || READ_BE_UINT32(_data.data()) == MKTAG('R', 'I', 'F', 'F'))) {
		_frame = _decodedFrame = nullptr;
		if (!_decoder->seekToFrame(frame))
			return false;
	}
	return decodeToFrame(frame);
}

bool RunawayVideo::decodeToFrame(uint frame) {
	if (!_decoder || frame >= _decoder->getFrameCount())
		return false;
	if (useCachedFrame(frame)) {
		return true;
	}
	if (_decoder->getCurFrame() > (int)frame)
		return seekToFrame(frame);
	return decodeFrames(frame);
}

bool RunawayVideo::decodeFrames(uint frame) {
	while (_decoder->getCurFrame() < (int)frame && !_decoder->endOfVideo()) {
		const int previousFrame = _decoder->getCurFrame();
		const Graphics::Surface *decoded = _decoder->decodeNextFrame();
		if (decoded && _decoder->getCurFrame() > previousFrame) {
			_decodedFrame = decoded;
			const uint decodedNumber = _decoder->getCurFrame();
			if (decodedNumber < _cachedFrames.size() && _cachedFrames[decodedNumber].enabled) {
				_cachedFrames[decodedNumber].surface.copyFrom(*decoded);
			}
		} else {
			warning("Runaway: unable to decode video frame %d", _decoder->getCurFrame());
			return false;
		}
	}
	_frame = _decodedFrame;
	return _frame != nullptr && _decoder->getCurFrame() == (int)frame;
}

RunawayVideo::FrameStatus RunawayVideo::prepareFrame(uint frame, uint maxFrames) {
	if (!_decoder || frame >= _decoder->getFrameCount()) {
		return kFrameError;
	}
	if (useCachedFrame(frame)) {
		return kFrameReady;
	}
	if (_decodedFrame && _decoder->getCurFrame() == (int)frame) {
		_frame = _decodedFrame;
		return kFrameReady;
	}
	if (!maxFrames) {
		return kFramePending;
	}
	if (_decoder->getCurFrame() > (int)frame) {
		_frame = _decodedFrame = nullptr;
		// Seeking directly to the target can decode the whole movie without yielding.
		if (!_decoder->seekToFrame(0)) {
			return kFrameError;
		}
	}
	const uint next = MIN<int64>(frame, (int64)_decoder->getCurFrame() + maxFrames);
	if (!decodeFrames(next)) {
		return kFrameError;
	}
	return next == frame ? kFrameReady : kFramePending;
}

RunawayVideo::FrameStatus RunawayVideo::prefetchFrame(uint frame, uint maxFrames) {
	const Graphics::Surface *displayed = _frame;
	const FrameStatus status = prepareFrame(frame, 0);
	_frame = displayed;
	if (status != kFramePending || !maxFrames) {
		return status;
	}
	// Decoding ahead must not overwrite the still currently behind a speaker.
	if (displayed && displayed == _decodedFrame) {
		_heldFrame.copyFrom(*displayed);
		displayed = &_heldFrame.rawSurface();
	}
	const FrameStatus prepared = prepareFrame(frame, maxFrames);
	_frame = displayed;
	return prepared;
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
	close();
	_chapter = chapter;
	_movie = movie;
	const Common::String name = Common::String::format("DATAV%c%02u.000", chapter, movie);
	if (Common::File::exists(Common::Path(name))) {
		_loaded = _video.load(name.c_str());
		for (uint i = 0; _loaded && i < ARRAYSIZE(kFilmOverlays); ++i) {
			const FilmOverlay &overlay = kFilmOverlays[i];
			if (overlay.chapter != chapter || overlay.movie != movie) {
				continue;
			}
			if (overlay.portrait >= _portraits.size()) {
				_portraits.resize(overlay.portrait + 1);
			}
			Portrait &portrait = _portraits[overlay.portrait];
			portrait.firstFrame = overlay.first;
			portrait.bounds = Common::Rect(overlay.left, overlay.top, overlay.right, overlay.bottom);
			portrait.faces.resize(overlay.count);
		}
	} else {
		_loaded = _archive.open(Common::Path(Common::String::format("DATAV%c%02u.002", chapter, movie)), 40);
	}
	return _loaded;
}

void RunawayFilmPortrait::close() {
	_video.close();
	_archive.close();
	_portraits.clear();
	_portrait.free();
	_talking.free();
	_loaded = false;
	_loadedPortrait = -1;
	_preparedFrames = 0;
}

RunawayVideo::FrameStatus RunawayFilmPortrait::prepare(uint portrait, uint maxFrames) {
	if (_video.frameCount()) {
		if (portrait >= _portraits.size() || _portraits[portrait].faces.empty()) {
			return RunawayVideo::kFrameError;
		}
		const uint endFrame = _portraits[portrait].firstFrame + _portraits[portrait].faces.size();
		const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
		for (uint i = 0; i < maxFrames && _preparedFrames < endFrame; ++i) {
			if (!_video.decodeToFrame(_preparedFrames)) {
				return RunawayVideo::kFrameError;
			}
			// Keep every face passed on the way to this speaker. Switching back
			// must not rewind the single-keyframe movie and decode it again.
			for (uint j = 0; j < _portraits.size(); ++j) {
				Portrait &pose = _portraits[j];
				if (_preparedFrames < pose.firstFrame || _preparedFrames >= pose.firstFrame + pose.faces.size()) {
					continue;
				}
				Graphics::ManagedSurface &face = pose.faces[_preparedFrames - pose.firstFrame];
				face.create(pose.bounds.width(), pose.bounds.height(), format);
				_video.drawRegion(face, pose.bounds, Common::Rect(face.w, face.h));
			}
			++_preparedFrames;
		}
		return _preparedFrames >= endFrame ? RunawayVideo::kFrameReady : RunawayVideo::kFramePending;
	}
	if (_loadedPortrait != (int)portrait) {
		Common::Array<byte> patch;
		const bool phone = (_chapter == 'D' && _movie == 8) || (_chapter == 'H' && _movie == 7);
		_portrait.create(phone ? 288 : 512, phone ? 288 : 300, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
		_talking.create(_portrait.w, _portrait.h, _portrait.format);
		if (!_archive.readVariableChunk(portrait * 2, patch) || !applyRunawayPatch(patch, _portrait) ||
				!_mouth.load(_archive, portrait * 2 + 1, 12)) {
			return RunawayVideo::kFrameError;
		}
		_loadedPortrait = portrait;
	}
	return RunawayVideo::kFrameReady;
}

bool RunawayFilmPortrait::draw(Graphics::ManagedSurface &surface, const Common::Rect &destination, uint portrait, uint mouth) {
	if (prepare(portrait, _video.frameCount()) != RunawayVideo::kFrameReady) {
		return false;
	}
	if (_video.frameCount()) {
		const Portrait &pose = _portraits[portrait];
		const Common::Rect target(destination.left + pose.bounds.left * destination.width() / _video.width(),
			destination.top + pose.bounds.top * destination.height() / _video.height(),
			destination.left + pose.bounds.right * destination.width() / _video.width(),
			destination.top + pose.bounds.bottom * destination.height() / _video.height());
		surface.blitFrom(pose.faces[mouth % pose.faces.size()], Common::Rect(pose.bounds.width(), pose.bounds.height()), target);
		return true;
	}
	_talking.blitFrom(_portrait);
	_mouth.draw(_talking, mouth, 0, 0);
	surface.blitFrom(_talking, Common::Rect(_talking.w, _talking.h), destination);
	return true;
}

} // namespace Hollywood
