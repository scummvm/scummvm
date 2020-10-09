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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/convert/convert_shape.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

//#define COMP_SHAPENUM 39

#ifdef COMP_SHAPENUM
extern int shapenum;
#endif

void ConvertShapeFrame::Free() {
	delete [] _line_offsets;
	_line_offsets = nullptr;

	delete [] _rle_data;
	_rle_data = nullptr;
}

ConvertShape::ConvertShape() : _num_frames(0), _frames(nullptr) {
}

void ConvertShape::Free() {
	if (_frames)
		for(uint32 i = 0; i < _num_frames; ++i)
			_frames[i].Free();

	delete [] _frames;
	_frames = nullptr;
	_num_frames = 0;
}

void ConvertShape::Read(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len) {
	// Just to be safe
	uint32 start_pos = source->pos();

	// Read the ident
	if (csf->_bytes_ident) {
		char ident[4];
		source->read(ident, csf->_bytes_ident);

		if (Std::memcmp (ident, csf->_ident, csf->_bytes_ident)) {
			perr << "Warning: Corrupt shape!" << Std::endl;
			return;
		}
	}

	// Read special buffer
	uint8 special[256];
	if (csf->_bytes_special) {
		memset(special, 0, 256);
		for (uint32 i = 0; i < csf->_bytes_special; i++) special[source->readByte()&0xFF] = i + 2;
	}

	// Read the header unknown
	if (csf->_bytes_header_unk) source->read(_header_unknown, csf->_bytes_header_unk);

#ifdef COMP_SHAPENUM
	if (shapenum == COMP_SHAPENUM) pout << Std::hex;
#endif

	// Now read _num_frames
	_num_frames = 1;
	if (csf->_bytes_num_frames) _num_frames = source->readX(csf->_bytes_num_frames);
	if (_num_frames == 0) _num_frames = CalcNumFrames(source,csf,real_len,start_pos);

#ifdef COMP_SHAPENUM
	if (shapenum == COMP_SHAPENUM) pout << "_num_frames " << _num_frames << Std::endl;
#endif

//	if (_num_frames == 0xFFFF || _num_frames == 0xFFFFFF || _num_frames == -1)
//	{
//		perr << "Corrupt shape? " << Std::endl;
//		_num_frames = 0;
//		_frames = 0;
//		return;
//	}

	// Create _frames array
	_frames = new ConvertShapeFrame[_num_frames];
	Std::memset (_frames, 0, _num_frames * sizeof(ConvertShapeFrame));

	// Now read the _frames
	for(uint32 f = 0; f < _num_frames; ++f) {
#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "Frame " << f << Std::endl;
#endif
		ConvertShapeFrame *frame = _frames + f;

#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "Seeking to " << (csf->_len_header + (csf->_len_frameheader * f)) << Std::endl;
		if (shapenum == COMP_SHAPENUM) pout << "Real " << (start_pos + csf->_len_header + (csf->_len_frameheader * f)) << Std::endl;
#endif
		// Seek to initial pos
		source->seek(start_pos + csf->_len_header + (csf->_len_frameheader * f));

#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "seeked to " << source->pos() << Std::endl;
#endif

		// Read the offset
		uint32 frame_offset = csf->_len_header + (csf->_len_frameheader * f);
		if (csf->_bytes_frame_offset) frame_offset = source->readX(csf->_bytes_frame_offset);
#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "frame_offset " << frame_offset << Std::endl;
#endif

		// Read the unknown
		if (csf->_bytes_frameheader_unk) source->read(frame->_header_unknown, csf->_bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = real_len-frame_offset;
		if (csf->_bytes_frame_length) frame_length = source->readX(csf->_bytes_frame_length) + csf->_bytes_frame_length_kludge;
#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "frame_length " << frame_length << Std::endl;
#endif

		// Seek to start of frame
		source->seek(start_pos + frame_offset + csf->_bytes_special);

		if (csf->_bytes_special)
			frame->ReadCmpFrame(source, csf, special, f > 0 ? _frames + f - 1 : 0);
		else
			frame->Read(source, csf, frame_length);
	}

#ifdef COMP_SHAPENUM
	if (shapenum == COMP_SHAPENUM) pout << Std::dec;
#endif
}

void ConvertShapeFrame::Read(IDataSource *source, const ConvertShapeFormat *csf, uint32 frame_length) {
	// Read unknown
	if (csf->_bytes_frame_unknown) source->read(_unknown, csf->_bytes_frame_unknown);

	// Frame details
	_compression = source->readX(csf->_bytes_frame_compression);
	_width = source->readXS(csf->_bytes_frame_width);
	_height = source->readXS(csf->_bytes_frame_height);
	_xoff = source->readXS(csf->_bytes_frame_xoff);
	_yoff = source->readXS(csf->_bytes_frame_yoff);

#ifdef COMP_SHAPENUM
	if (_width <= 0 || _height <= 0  ||shapenum == COMP_SHAPENUM )
	{
		pout << "_compression " << _compression << Std::endl;
		pout << "_width " << _width << Std::endl;
		pout << "_height " << _height << Std::endl;
		pout << "_xoff " << _xoff << Std::endl;
		pout << "_yoff " << _yoff << Std::endl;
	}
#endif

	if (_compression != 0 && _compression != 1) {
		_compression = 0;
		_width = 0;
		_height = 0;
		_xoff = 0;
		_yoff = 0;
		//perr << "Corrupt frame? (frame " << f << ")" << Std::endl;
		perr << "Corrupt frame?" << Std::endl;
	}

	if (_height) {
		// Line offsets
		_line_offsets = new uint32 [_height];

		for (int32 i = 0; i < _height; ++i)  {
			_line_offsets[i] = source->readX(csf->_bytes_line_offset);

			// Now fudge with the value and turn it into an offset into the rle data
			// If required
			if (!csf->_line_offset_absolute)
				_line_offsets[i] -= (_height - i) * csf->_bytes_line_offset;
		}

		// Calculate the number of bytes of RLE data
		_bytes_rle = frame_length - (csf->_len_frameheader2 + (_height * csf->_bytes_line_offset));

#ifdef COMP_SHAPENUM
		if (_bytes_rle < 0) {
			_bytes_rle = 0;
			perr << "Corrupt frame?" << Std::endl;
		}

#endif
	} else
		_line_offsets = nullptr;

	// Read the RLE Data
	if (_bytes_rle) {
		_rle_data = new uint8[_bytes_rle];
		source->read(_rle_data, _bytes_rle);
	} else
		_rle_data = nullptr;
}

void ConvertShapeFrame::ReadCmpFrame(IDataSource *source, const ConvertShapeFormat *csf, const uint8 special[256], ConvertShapeFrame *prev) {
	static OAutoBufferDataSource *rlebuf = nullptr;
	uint8 outbuf[512];

	// Read unknown
	if (csf->_bytes_frame_unknown) source->read(_unknown, csf->_bytes_frame_unknown);

	// Frame details
	_compression = source->readX(csf->_bytes_frame_compression);
	_width = source->readXS(csf->_bytes_frame_width);
	_height = source->readXS(csf->_bytes_frame_height);
	_xoff = source->readXS(csf->_bytes_frame_xoff);
	_yoff = source->readXS(csf->_bytes_frame_yoff);

	_line_offsets = new uint32 [_height];

	if (!rlebuf) rlebuf = new OAutoBufferDataSource(1024);
	rlebuf->clear();

	for(int32 y = 0; y < _height; ++y) {
		_line_offsets[y] = rlebuf->pos();

		int32 xpos = 0;

		do {
			uint8 skip = source->readByte();
			xpos += skip;

			if (xpos > _width) {
				source->seek(-1, SEEK_CUR);
				skip = _width-(xpos-skip);
			}

			rlebuf->writeByte(skip);

			if (xpos >= _width) break;

			uint32 dlen = source->readByte();
			uint8 *o = outbuf;

			// Is this required???? It seems hacky and pointless
			if (dlen == 0 || dlen == 1) {
				source->seek(-1, SEEK_CUR);
				rlebuf->seek(-1, SEEK_CUR);
				rlebuf->writeByte(skip + (_width - xpos));
				break;
			}

			int type = 0;

			if (_compression)  {
				type = dlen & 1;
				dlen >>= 1;
			}

			if (!type) {

				uint32 extra = 0;

				for (uint32 j = 0; j < dlen; j++) {

					uint8 c = source->readByte();

					if (special[c] && prev) {
						int32 count = special[c];
						prev->GetPixels(o, count, xpos - _xoff, y - _yoff);
						o+=count;
						extra += count - 1;
						xpos += count;
					} else if (c == 0xFF && prev) {
						int32 count = source->readByte();
						prev->GetPixels(o, count, xpos - _xoff, y - _yoff);
						o+=count;
						extra += count - 2;
						xpos += count;
						j++;
					} else {
						*o++ = c;
						xpos++;
					}
				}

				if (((dlen+extra) << _compression) > 255) {
					perr << "Error! Corrupt Frame. RLE dlen too large" << Std::endl;
				}

				rlebuf->writeByte((dlen+extra) << _compression);
				rlebuf->write(outbuf,dlen+extra);
			} else {
				rlebuf->writeByte((dlen << 1) | 1);
				rlebuf->writeByte(source->readByte());
				xpos+=dlen;
			}

		} while (xpos < _width);
	}

	_bytes_rle = rlebuf->pos();
	_rle_data = new uint8[_bytes_rle];
	memcpy (_rle_data, rlebuf->getData(), _bytes_rle);
}

void ConvertShapeFrame::GetPixels(uint8 *buf, int32 count, int32 x, int32 y) {
	x += _xoff;
	y += _yoff;

	if (y > _height) return;

	int32 xpos = 0;
	const uint8 * linedata = _rle_data + _line_offsets[y];

	do {
		xpos += *linedata++;

		if (xpos == _width) break;

		int32 dlen = *linedata++;
		int type = 0;

		if (_compression) {
			type = dlen & 1;
			dlen >>= 1;
		}

		if (x >= xpos && x < (xpos+dlen)) {
			int diff = x-xpos;
			dlen-=diff;
			xpos = x;

			int num = count;
			if (dlen < count) num = dlen;

			if (!type) {
				const uint8 *l = (linedata += diff);

				while (num--) {
					*buf++ = *l++;
					count--;
					x++;
				}
			} else {
				uint8 l = *linedata;

				while (num--) {
					*buf++ = l;
					count--;
					x++;
				}
			}

			if (count == 0) return;
		}

		if (!type) linedata+=dlen;
		else linedata++;

		xpos += dlen;

	} while (xpos < _width);
}

int ConvertShape::CalcNumFrames(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len, uint32 start_pos) {
	int f = 0;
	uint32 first_offset = 0xFFFFFFFF;

	uint32 save_pos = source->pos();

	for (f = 0;; f++) {

		// Seek to initial pos
		source->seek(start_pos + csf->_len_header + (csf->_len_frameheader * f));

		if ((source->pos()-start_pos) >= first_offset) break;

		// Read the offset
		uint32 frame_offset = csf->_len_header + (csf->_len_frameheader * f);
		if (csf->_bytes_frame_offset) frame_offset = source->readX(csf->_bytes_frame_offset) + csf->_bytes_special;

		if (frame_offset < first_offset) first_offset = frame_offset;

		// Read the unknown
		if (csf->_bytes_frameheader_unk) source->skip(csf->_bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = real_len-frame_offset;
		if (csf->_bytes_frame_length)
			frame_length = source->readX(csf->_bytes_frame_length) + csf->_bytes_frame_length_kludge;
		debug(MM_INFO, "Frame %d length = %xh", f, frame_length);
	}

	source->seek(save_pos);

	return f;
}

bool ConvertShape::Check(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len) {
#if 0
	pout << "Testing " << csf->_name << "..." << Std::endl;
#endif
	bool result = true;

	// Just to be safe
	int start_pos = source->pos();

	// Read the ident
	if (csf->_bytes_ident) {
		char ident[5];
		ident[csf->_bytes_ident] = 0;
		source->read(ident, csf->_bytes_ident);

		if (Std::memcmp (ident, csf->_ident, csf->_bytes_ident)) {
			// Return to start position
			source->seek(start_pos);
			return false;
		}
	}

	// Read the header special colour
	if (csf->_bytes_special) source->skip(csf->_bytes_special);

	// Read the header unknown
	if (csf->_bytes_header_unk) source->skip(csf->_bytes_header_unk);

	// Now read _num_frames
	int numFrames = 1;
	if (csf->_bytes_num_frames) numFrames = source->readX(csf->_bytes_num_frames);
	if (numFrames == 0) numFrames = CalcNumFrames(source,csf,real_len,start_pos);

	// Create _frames array
	ConvertShapeFrame oneframe;
	Std::memset (&oneframe, 0, sizeof(ConvertShapeFrame));

	// Now read the _frames
	for (int f = 0; f < numFrames; f++) {
		ConvertShapeFrame *frame = &oneframe;

		// Seek to initial pos
		source->seek(start_pos + csf->_len_header + (csf->_len_frameheader * f));

		// Read the offset
		uint32 frame_offset = csf->_len_header + (csf->_len_frameheader * f);
		if (csf->_bytes_frame_offset) frame_offset = source->readX(csf->_bytes_frame_offset) + csf->_bytes_special;

		// Read the unknown
		if (csf->_bytes_frameheader_unk) source->read(frame->_header_unknown, csf->_bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = real_len-frame_offset;
		if (csf->_bytes_frame_length) frame_length = source->readX(csf->_bytes_frame_length) + csf->_bytes_frame_length_kludge;

		// Invalid frame length
		if ((frame_length + frame_offset) > real_len) {
			result = false;
			break;
		}

		// Seek to start of frame
		source->seek(start_pos + frame_offset);

		// Read unknown
		if (csf->_bytes_frame_unknown) source->read(frame->_unknown, csf->_bytes_frame_unknown);

		// Frame details
		frame->_compression = source->readX(csf->_bytes_frame_compression);
		frame->_width = source->readXS(csf->_bytes_frame_width);
		frame->_height = source->readXS(csf->_bytes_frame_height);
		frame->_xoff = source->readXS(csf->_bytes_frame_xoff);
		frame->_yoff = source->readXS(csf->_bytes_frame_yoff);

		if ((frame->_compression != 0 && frame->_compression != 1) || frame->_width < 0 || frame->_height < 0) {
			frame->_compression = 0;
			frame->_width = 0;
			frame->_height = 0;
			frame->_xoff = 0;
			frame->_yoff = 0;
			result = false;
			break;
		}

		if (frame->_height) {
			// Line offsets
			int32 highest_offset_byte = 0;

			// Calculate the number of bytes of RLE data
			frame->_bytes_rle = frame_length - (csf->_len_frameheader2 + (frame->_height * csf->_bytes_line_offset));

			// Totally invalid shape
			if (frame->_bytes_rle < 0) {
				result = false;
				break;
			}

			// Only attempt to decompress the shape if we are not a compressed shapes
			if (!csf->_bytes_special) {

				// Seek to first in offset table
				source->seek(start_pos + frame_offset + csf->_len_frameheader2);

				// Loop through each of the _frames and find the last rle run
				for (int i = 0; i < frame->_height; i++) {
					int32 line_offset = source->readX(csf->_bytes_line_offset);

					// Now fudge with the value and turn it into an offset into the rle data
					// if required
					if (!csf->_line_offset_absolute)
						line_offset -= (frame->_height - i) * csf->_bytes_line_offset;

					if (line_offset > frame->_bytes_rle) {
						result = false;
						break;
					}

					if (line_offset > highest_offset_byte) highest_offset_byte = line_offset;
				};

				// Failed for whatever reason
				if (result == false) break;

				// Jump to the line offset and calculate the length of the run
				source->seek(highest_offset_byte + start_pos + frame_offset + csf->_len_frameheader2 + frame->_height * csf->_bytes_line_offset);
				int xpos = 0;
				uint32 dlen = 0;

				// Compressed
				if (frame->_compression) {
					do {
						xpos += source->readByte();
						if (xpos == frame->_width) break;

						dlen = source->readByte();
						int type = dlen & 1;
						dlen >>= 1;

						if (!type) source->skip(dlen);
						else source->skip(1);

						xpos += dlen;

					} while (xpos < frame->_width);
				// Uncompressed
				} else {
					do {
						xpos += source->readByte();
						if (xpos == frame->_width) break;

						dlen = source->readByte();
						source->skip(dlen);

						xpos += dlen;
					} while (xpos < frame->_width);
				}

				// Calc 'real' bytes rle
				int32 highest_rle_byte = source->pos();
				highest_rle_byte -= start_pos + frame_offset + csf->_len_frameheader2 + frame->_height * csf->_bytes_line_offset;

				// Too many bytes
				if (highest_rle_byte > frame->_bytes_rle) {
					result = false;
					break;
				}
			}
		}
	}

	// Free frames
	oneframe.Free();

	// Return to start position
	source->seek(start_pos);

	return result;
}

bool ConvertShape::CheckUnsafe(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len) {
#if 0
	pout << "Testing " << csf->_name << "..." << Std::endl;
#endif
	bool result = true;

	// Just to be safe
	const uint32 start_pos = source->pos();

	// Read the ident
	if (csf->_bytes_ident) {
		char ident[5];
		ident[csf->_bytes_ident] = 0;
		source->read(ident, csf->_bytes_ident);

		if (Std::memcmp (ident, csf->_ident, csf->_bytes_ident)) {
			// Return to start position
			source->seek(start_pos);
			return false;
		}
	}

	// Read the header special colour
	if (csf->_bytes_special) source->skip(csf->_bytes_special);

	// Read the header unknown
	if (csf->_bytes_header_unk) source->skip(csf->_bytes_header_unk);

	// Now read _num_frames
	int numFrames = 1;
	if (csf->_bytes_num_frames) numFrames = source->readX(csf->_bytes_num_frames);
	if (numFrames == 0) numFrames = CalcNumFrames(source,csf,real_len,start_pos);

	// Create _frames array
	ConvertShapeFrame oneframe;
	Std::memset (&oneframe, 0, sizeof(ConvertShapeFrame));

	// Now read the _frames
	for (int f = 0; f < numFrames; f++) {
		ConvertShapeFrame *frame = &oneframe;

		// Seek to initial pos
		source->seek(start_pos + csf->_len_header + (csf->_len_frameheader * f));

		// Read the offset
		uint32 frame_offset = csf->_len_header + (csf->_len_frameheader * f);
		if (csf->_bytes_frame_offset) frame_offset = source->readX(csf->_bytes_frame_offset) + csf->_bytes_special;

		// Read the unknown
		if (csf->_bytes_frameheader_unk) source->read(frame->_header_unknown, csf->_bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = real_len-frame_offset;
		if (csf->_bytes_frame_length) frame_length = source->readX(csf->_bytes_frame_length) + csf->_bytes_frame_length_kludge;

		// Invalid frame length
		if ((frame_length + frame_offset) > real_len) {
			result = false;
			break;
		}

		// Seek to start of frame
		source->seek(start_pos + frame_offset);

		// Read unknown
		if (csf->_bytes_frame_unknown) source->read(frame->_unknown, csf->_bytes_frame_unknown);

		// Frame details
		frame->_compression = source->readX(csf->_bytes_frame_compression);
		frame->_width = source->readXS(csf->_bytes_frame_width);
		frame->_height = source->readXS(csf->_bytes_frame_height);
		frame->_xoff = source->readXS(csf->_bytes_frame_xoff);
		frame->_yoff = source->readXS(csf->_bytes_frame_yoff);

		if ((frame->_compression != 0 && frame->_compression != 1) || frame->_width < 0 || frame->_height < 0) {
			frame->_compression = 0;
			frame->_width = 0;
			frame->_height = 0;
			frame->_xoff = 0;
			frame->_yoff = 0;
			result = false;
			break;
		}

		if (frame->_height) {
			// Calculate the number of bytes of RLE data (may not be accurate but we don't care)
			frame->_bytes_rle = frame_length - (csf->_len_frameheader2 + (frame->_height * csf->_bytes_line_offset));

			// Totally invalid shape
			if (frame->_bytes_rle < 0) {
				result = false;
				break;
			}
		}
	}

	// Free _frames
	oneframe.Free();

	// Return to start position
	source->seek(start_pos);

	return result;
}

void ConvertShape::Write(ODataSource *dest, const ConvertShapeFormat *csf, uint32 &write_len) {
	// Just to be safe
	const uint32 start_pos = dest->pos();

	// Write the ident
	if (csf->_bytes_ident) dest->write(csf->_ident, csf->_bytes_ident);

	// Write the header unknown
	if (csf->_bytes_header_unk) dest->write(_header_unknown, csf->_bytes_header_unk);

	// Now write _num_frames
	if (csf->_bytes_num_frames) dest->writeX(_num_frames, csf->_bytes_num_frames);
	else if (!csf->_bytes_num_frames && _num_frames > 1) {
		perr << "Error: Unable to convert multiple frame shapes to " << csf->_name << Std::endl;
		return;
	}

	// Write filler space for the frame details
	for (uint32 i = 0; i < _num_frames*csf->_len_frameheader; i++) dest->writeByte(0);

	// Now write the _frames
	for(uint32 f = 0; f < _num_frames; f++) {
		ConvertShapeFrame *frame = _frames + f;

		// Get the frame offset
		uint32 frame_offset = dest->pos() - start_pos;

		// Seek to the frame header pos
		dest->seek(start_pos + csf->_len_header + (csf->_len_frameheader * f));

		// Write the offset
		if (csf->_bytes_frame_offset) dest->writeX(frame_offset, csf->_bytes_frame_offset);

		// Write the unknown
		if (csf->_bytes_frameheader_unk) dest->write(frame->_header_unknown, csf->_bytes_frameheader_unk);

		// Calc and write frame_length
		if (csf->_bytes_frame_length) {
			uint32 frame_length = csf->_len_frameheader2 + (frame->_height * csf->_bytes_line_offset) + frame->_bytes_rle;
			dest->writeX(frame_length - csf->_bytes_frame_length_kludge, csf->_bytes_frame_length);
		}

		// Seek to start of frame
		dest->seek(start_pos + frame_offset);

		// Write unknown
		if (csf->_bytes_frame_unknown) dest->write(frame->_unknown, csf->_bytes_frame_unknown);

		// Frame details
		dest->writeX(frame->_compression, csf->_bytes_frame_compression);
		dest->writeX(frame->_width, csf->_bytes_frame_width);
		dest->writeX(frame->_height, csf->_bytes_frame_height);
		dest->writeX(frame->_xoff, csf->_bytes_frame_xoff);
		dest->writeX(frame->_yoff, csf->_bytes_frame_yoff);

		// Line offsets
		for (int32 i = 0; i < frame->_height; i++) {
			int32 actual_offset = frame->_line_offsets[i];

			// Unfudge the value and write it, if requiretd
			if (!csf->_line_offset_absolute)
				actual_offset += (frame->_height - i) * csf->_bytes_line_offset;

			dest->writeX(actual_offset, csf->_bytes_line_offset);
		}

		// Write the RLE Data
		dest->write(frame->_rle_data, frame->_bytes_rle);
	}

	// Just cheat
	write_len = dest->pos() - start_pos;
}


// Shape format configuration for Pentagram
const ConvertShapeFormat		PentagramShapeFormat = {
	"Pentagram",
	8,		// header
	"PSHP",	// ident
	4,		// bytes_ident
	0,		// bytes_special
	0,		// header_unk
	4,		// _num_frames

	8,		// frameheader
	4,		// frame_offset
	0,		// frameheader_unk
	4,		// frame_length
	0,		// frame_length_kludge

	20,		// frameheader2
	0,		// frame_unknown
	4,		// frame_compression
	4,		// frame_width
	4,		// frame_height
	4,		// frame_xoff
	4,		// frame_yoff

	4,		// line_offset
	1		// line_offset_absolute
};

} // End of namespace Ultima8
} // End of namespace Ultima
