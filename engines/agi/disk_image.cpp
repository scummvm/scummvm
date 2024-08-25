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

#include "common/formats/disk_image.h"
#include "common/memstream.h"
#include "common/path.h"
#include "common/textconsole.h"

#include "agi/disk_image.h"

namespace Agi {

/**
 * DiskImageStream is a stream wrapper around Common::DiskImage.
 *
 * This allows DiskImage to lazily decode tracks as a stream is used.
 * This is important for detection, because the .woz format is noticeably
 * expensive to decode all tracks at once, and detection has to read
 * INITDIR to discover which track to read LOGDIR from.
 */
class DiskImageStream : virtual public Common::SeekableReadStream {
public:
	DiskImageStream(Common::DiskImage *diskImage) : _diskImage(diskImage), _stream(_diskImage->getDiskStream()) {}

	~DiskImageStream() {
		delete _diskImage;
	}

	uint32 read(void *dataPtr, uint32 dataSize) override {
		return _diskImage->read(dataPtr, pos(), dataSize);
	}

	bool eos() const override { return _stream->eos(); }
	void clearErr() override { _stream->clearErr(); }

	int64 pos() const override{ return _stream->pos(); }
	int64 size() const override { return _stream->size(); }

	bool seek(int64 offs, int whence = SEEK_SET) override { return _stream->seek(offs, whence); }

private:
	Common::DiskImage *_diskImage;
	Common::SeekableReadStream *_stream;
};

Common::SeekableReadStream *openPCDiskImage(const Common::Path &path, const Common::FSNode &node) {
	Common::SeekableReadStream *stream = node.createReadStream();
	if (stream == nullptr) {
		warning("unable to open disk image: %s", path.baseName().c_str());
		return nullptr;
	}

	// validate disk size
	if (stream->size() != PC_DISK_SIZE) {
		delete stream;
		return nullptr;
	}

	return stream;
}

Common::SeekableReadStream *openA2DiskImage(const Common::Path &path, const Common::FSNode &node, bool loadAllTracks) {
	Common::String name = path.baseName();

	// Open the image with Common::DiskImage, unless the file extension is ".img".
	// DiskImage expects ".img" to be a PC disk image, but it also gets used as
	// an Apple II raw sector disk image, so just open it and and read it.
	Common::SeekableReadStream *stream = nullptr;
	if (name.hasSuffixIgnoreCase(".img")) {
		stream = node.createReadStream();
	} else {
		if (loadAllTracks) {
			// when loading all tracks, open with DiskImage and take the stream.
			Common::DiskImage diskImage;
			if (diskImage.open(node)) {
				stream = diskImage.releaseStream();
			}
		} else {
			// when loading tracks as they're used, create a DiskImage with lazy
			// decoding and wrap it in a stream.
			Common::DiskImage *diskImage = new Common::DiskImage();
			diskImage->setLazyDecoding(true);
			if (diskImage->open(node)) {
				stream = new DiskImageStream(diskImage);
			} else {
				delete diskImage;
			}
		}
	}

	if (stream == nullptr) {
		warning("unable to open disk image: %s", path.baseName().c_str());
		return nullptr;
	}

	// validate disk size
	if (stream->size() != A2_DISK_SIZE) {
		delete stream;
		return nullptr;
	}

	return stream;
}

} // End of namespace Agi
