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

#include "mutationofjb/animationdecoder.h"
#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/util.h"
#include "common/debug.h"

namespace MutationOfJB {

AnimationDecoder::AnimationDecoder(const Common::String &fileName) : _fileName(fileName), _fromFrame(-1), _toFrame(-1), _threshold(0xFF) {
	_surface.create(IMAGE_WIDTH, IMAGE_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_owningSurface = true;
}

AnimationDecoder::AnimationDecoder(const Common::String &fileName, const Graphics::Surface &outSurface) : _fileName(fileName), _surface(outSurface), _owningSurface(false), _fromFrame(-1), _toFrame(-1), _threshold(0xFF) {}

bool AnimationDecoder::decode(AnimationDecoderCallback *callback) {
	EncryptedFile file;
	file.open(_fileName);

	if (!file.isOpen()) {
		reportFileMissingError(_fileName.c_str());

		return false;
	}

	file.seek(0, SEEK_END);
	const int32 endPos = file.pos();

	// Skip header - we don't need it anyway.
	file.seek(0x80);

	int frameNo = 0;

	while (file.pos() != endPos) {
		// Record.
		const uint32 length = file.readUint32LE();
		const uint16 recordId = file.readUint16LE();
		const uint16 subrecords = file.readUint16LE();

		// Skip 8 empty bytes.
		file.seek(8, SEEK_CUR);

		// Subrecords.
		if (recordId == 0xF1FA) {
			if ((_fromFrame != -1 && frameNo < _fromFrame) || (_toFrame != -1 && frameNo > _toFrame)) {
				file.seek(length - 16, SEEK_CUR);
			} else {
				if (subrecords == 0) {
					if (callback) {
						callback->onFrame(frameNo, _surface); // Empty record, frame identical to the previous one.
					}
				} else {
					for (int i = 0; i < subrecords; ++i) {
						int32 filePos = file.pos();

						const uint32 subLength = file.readUint32LE();
						const uint16 type = file.readUint16LE();

						if (type == 0x0B) {
							loadPalette(file);
							if (callback) {
								callback->onPaletteUpdated(_palette);
							}
						} else if (type == 0x0F) {
							loadFullFrame(file, subLength - 6);
							if (callback) {
								callback->onFrame(frameNo, _surface);
							}
						} else if (type == 0x0C) {
							loadDiffFrame(file, subLength - 6);
							if (callback) {
								callback->onFrame(frameNo, _surface);
							}
						} else {
							debug("Unsupported record type %02X.", type);
							file.seek(subLength - 6, SEEK_CUR);
						}

						// Makes decoding more robust, because for some reason records might have extra data at the end.
						file.seek(filePos + subLength, SEEK_SET);
					}
				}
			}
			frameNo++;
		} else {
			file.seek(length - 16, SEEK_CUR);
		}
	}
	file.close();

	return true;
}

void AnimationDecoder::setPartialMode(int fromFrame, int toFrame, const Common::Rect area, uint8 threshold) {
	_fromFrame = fromFrame;
	_toFrame = toFrame;
	_area = area;
	_threshold = threshold;
}

void AnimationDecoder::loadPalette(Common::SeekableReadStream &file) {
	uint16 packets = file.readUint16LE();
	const uint8 skipCount = file.readByte();
	int copyCount = file.readByte();
	if (copyCount == 0) {
		copyCount = PALETTE_COLORS;
	}

	while (packets--) {
		file.read(_palette + skipCount * 3, copyCount * 3);

		for (int j = skipCount * 3; j < (skipCount + copyCount) * 3; ++j) {
			_palette[j] <<= 2; // Uses 6-bit colors.
		}
	}
}

void AnimationDecoder::loadFullFrame(EncryptedFile &file, uint32 size) {
	uint8 *const pixels = reinterpret_cast<uint8 *>(_surface.getPixels());
	uint8 *ptr = pixels;
	uint32 readBytes = 0;
	uint32 lines = 0;

	while (readBytes != size) {
		if (lines == 200) {
			// Some full frames have an unknown byte at the end,
			// so break when we encounter all 200 lines.
			break;
		}

		uint8 no = file.readByte();
		readBytes++;
		while (no--) {
			uint8 n = file.readByte();
			readBytes++;
			if (n < 0x80) {
				// RLE - Copy color n times.
				uint8 color = file.readByte();
				readBytes++;
				while (n--) {
					*ptr++ = color;
				}
			} else {
				// Take next 0x100 - n bytes as they are.
				const uint32 rawlen = 0x100 - n;
				file.read(ptr, rawlen);
				readBytes += rawlen;
				ptr += rawlen;
			}
		}
		lines++;
	}
}

void AnimationDecoder::loadDiffFrame(EncryptedFile &file, uint32) {
	const uint16 firstLine = file.readUint16LE();
	const uint16 numLines = file.readUint16LE();

	for (uint16 line = firstLine; line < firstLine + numLines; ++line) {
		uint8 *imageData = reinterpret_cast<uint8 *>(_surface.getBasePtr(0, line));
		uint16 lineOffset = 0;

		// Optimization for skipping the whole line if outside of confined area.
		const bool skipLineOutput = !_area.isEmpty() && (line < _area.top || line >= _area.bottom);
		uint8 buf[0x80];

		uint8 runs = file.readByte();
		while (runs--) {
			uint8 localOffset = file.readByte();
			uint8 num = file.readByte();

			imageData += localOffset;
			lineOffset += localOffset;
			if (num == 0) {
				// Ignore?
				debug("Zero RLE number found.");
			} else if (num < 0x80) {
				if (!skipLineOutput) {
					if (_area.isEmpty() && _threshold == 0xFF) {
						file.read(imageData, num);
					} else {
						file.read(buf, num);
						for (uint16 i = 0; i < num; i++) {
							if ((_area.isEmpty() || _area.contains(lineOffset + i, line)) && imageData[i] <= _threshold)
								imageData[i] = buf[i];
						}
					}
				} else {
					file.skip(num);
				}

				imageData += num;
				lineOffset += num;
			} else {
				const uint8 color = file.readByte();
				const int no = 0x100 - num;
				if (!skipLineOutput) {
					if (_area.isEmpty() && _threshold == 0xFF) {
						memset(imageData, color, no);
					} else {
						for (int i = 0; i < no; i++) {
							if ((_area.isEmpty() || _area.contains(lineOffset + i, line)) && imageData[i] <= _threshold)
								imageData[i] = color;
						}
					}
				}
				imageData += no;
				lineOffset += no;
			}

		}
	}
}

AnimationDecoder::~AnimationDecoder() {
	if (_owningSurface)
		_surface.free();
}

}
