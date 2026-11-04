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

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"

#include "olga/detection.h"
#include "olga/lbm.h"

namespace Olga {

static const uint32 kForm = MKTAG('F', 'O', 'R', 'M');
static const uint32 kList = MKTAG('L', 'I', 'S', 'T');
static const uint32 kProp = MKTAG('P', 'R', 'O', 'P');
static const uint32 kPbm = MKTAG('P', 'B', 'M', ' ');
static const uint32 kBmhd = MKTAG('B', 'M', 'H', 'D');
static const uint32 kCmap = MKTAG('C', 'M', 'A', 'P');
static const uint32 kBody = MKTAG('B', 'O', 'D', 'Y');

namespace {

struct Bmhd {
	uint16 width = 0, height = 0;
	byte masking = 0, compression = 0;
	uint16 key = 0;
};

// ByteRun1 (PackBits): n < 128 copies n + 1 bytes, n > 128 repeats the next
// byte 257 - n times, 128 does nothing
bool unpackByteRun1(const byte *src, uint32 srcSize, byte *dst, uint32 dstSize) {
	uint32 i = 0, o = 0;
	while (o < dstSize) {
		if (i >= srcSize)
			return false;
		byte c = src[i++];
		if (c < 128) {
			uint32 n = c + 1;
			if (i + n > srcSize || o + n > dstSize)
				return false;
			memcpy(dst + o, src + i, n);
			i += n;
			o += n;
		} else if (c > 128) {
			uint32 n = 257 - c;
			if (i >= srcSize || o + n > dstSize)
				return false;
			memset(dst + o, src[i++], n);
			o += n;
		}
	}
	return true;
}

} // End of anonymous namespace

bool LbmFile::readForm(const byte *data, uint32 start, uint32 end) {
	Bmhd bmhd;
	bool haveBmhd = false;
	uint32 i = start;
	while (i + 8 <= end) {
		uint32 id = READ_BE_UINT32(data + i);
		uint32 size = READ_BE_UINT32(data + i + 4);
		const byte *body = data + i + 8;
		if (i + 8 + size > end)
			return false;
		if (id == kBmhd && size >= 20) {
			bmhd.width = READ_BE_UINT16(body);
			bmhd.height = READ_BE_UINT16(body + 2);
			bmhd.masking = body[9];
			bmhd.compression = body[10];
			bmhd.key = READ_BE_UINT16(body + 12);
			haveBmhd = true;
		} else if (id == kCmap) {
			memset(_palette, 0, sizeof(_palette));
			memcpy(_palette, body, MIN<uint32>(size, sizeof(_palette)));
			_hasPalette = true;
		} else if (id == kBody) {
			if (!haveBmhd)
				return false;
			// Rows are stored padded to an even width; the pad pixel is dropped
			uint32 stride = bmhd.width + (bmhd.width & 1);
			uint32 n = stride * bmhd.height;
			Common::Array<byte> raw(n);
			if (bmhd.compression == 1) {
				if (!unpackByteRun1(body, size, raw.data(), n))
					return false;
			} else if (bmhd.compression == 0) {
				if (size < n)
					return false;
				memcpy(raw.data(), body, n);
			} else {
				return false;
			}
			Common::SharedPtr<LbmImage> img(new LbmImage());
			img->surface.create(bmhd.width, bmhd.height, Graphics::PixelFormat::createFormatCLUT8());
			for (uint y = 0; y < bmhd.height; y++)
				memcpy(img->surface.getBasePtr(0, y), raw.data() + y * stride, bmhd.width);
			img->keyed = bmhd.masking == 2;
			img->key = (byte)bmhd.key;
			_images.push_back(img);
		}
		i += 8 + size + (size & 1);
	}
	return true;
}

bool LbmFile::load(const Common::Path &name) {
	_images.clear();
	_hasPalette = false;

	Common::File f;
	if (!f.open(name)) {
		warning("Olga: cannot open %s", name.toString().c_str());
		return false;
	}
	uint32 fileSize = f.size();
	Common::Array<byte> buf(fileSize);
	if (f.read(buf.data(), fileSize) != fileSize)
		return false;
	const byte *data = buf.data();
	if (fileSize < 12 || READ_BE_UINT32(data + 8) != kPbm)
		return false;

	uint32 tag = READ_BE_UINT32(data);
	uint32 end = MIN<uint32>(8 + READ_BE_UINT32(data + 4), fileSize);
	bool ok = true;
	if (tag == kForm) {
		ok = readForm(data, 12, end);
	} else if (tag == kList) {
		uint32 i = 12;
		while (ok && i + 12 <= end) {
			uint32 id = READ_BE_UINT32(data + i);
			uint32 size = READ_BE_UINT32(data + i + 4);
			if ((id != kProp && id != kForm) || READ_BE_UINT32(data + i + 8) != kPbm ||
				i + 8 + size > end)
				ok = false;
			else
				ok = readForm(data, i + 12, i + 8 + size);
			i += 8 + size + (size & 1);
		}
	} else {
		ok = false;
	}
	if (!ok)
		warning("Olga: bad LBM file %s", name.toString().c_str());
	debugC(1, kDebugResources, "%s: %d images, palette %s", name.toString().c_str(),
		   _images.size(), _hasPalette ? "yes" : "no");
	return ok && !_images.empty();
}

} // End of namespace Olga
