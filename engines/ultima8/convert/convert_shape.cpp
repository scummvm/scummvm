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

#include "ultima8/misc/pent_include.h"
#include "ultima8/convert/convert_shape.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

//#define COMP_SHAPENUM 39

#ifdef COMP_SHAPENUM
extern int shapenum;
#endif

void ConvertShape::Read(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len)
{
	// Just to be safe
	uint32 start_pos = source->getPos();

	// Read the ident
	if (csf->bytes_ident)
	{
		char ident[4];
		source->read(ident, csf->bytes_ident);

		if (std::memcmp (ident, csf->ident, csf->bytes_ident))
		{
			perr << "Warning: Corrupt shape!" << std::endl;
			return;
		}
	}

	// Read special buffer
	uint8 special[256];
	if (csf->bytes_special) {
		memset(special, 0, 256);
		for (uint32 i = 0; i < csf->bytes_special; i++) special[source->read1()&0xFF] = i+2;
	}

	// Read the header unknown
	if (csf->bytes_header_unk) source->read(header_unknown, csf->bytes_header_unk);

#ifdef COMP_SHAPENUM
	if (shapenum == COMP_SHAPENUM) pout << std::hex;
#endif

	// Now read num_frames
	num_frames = 1;
	if (csf->bytes_num_frames) num_frames = source->readX(csf->bytes_num_frames);
	if (num_frames == 0) num_frames = CalcNumFrames(source,csf,real_len,start_pos);

#ifdef COMP_SHAPENUM
	if (shapenum == COMP_SHAPENUM) pout << "num_frames " << num_frames << std::endl;
#endif

//	if (num_frames == 0xFFFF || num_frames == 0xFFFFFF || num_frames == -1)
//	{
//		perr << "Corrupt shape? " << std::endl;
//		num_frames = 0;
//		frames = 0;
//		return;
//	}

	// Create frames array
	frames = new ConvertShapeFrame[num_frames];
	std::memset (frames, 0, num_frames * sizeof(ConvertShapeFrame));

	// Now read the frames
	for(uint32 f = 0; f < num_frames; ++f) 
	{
#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "Frame " << f << std::endl;
#endif
		ConvertShapeFrame *frame = frames+f;

#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "Seeking to " << (csf->len_header + (csf->len_frameheader*f)) << std::endl;
		if (shapenum == COMP_SHAPENUM) pout << "Real " << (start_pos + csf->len_header + (csf->len_frameheader*f)) << std::endl;
#endif
		// Seek to initial pos
		source->seek(start_pos + csf->len_header + (csf->len_frameheader*f));

#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "seeked to " << source->getPos() << std::endl;
#endif

		// Read the offset
		uint32 frame_offset = csf->len_header + (csf->len_frameheader*f);
		if (csf->bytes_frame_offset) frame_offset = source->readX(csf->bytes_frame_offset);
#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "frame_offset " << frame_offset << std::endl;
#endif

		// Read the unknown
		if (csf->bytes_frameheader_unk) source->read(frame->header_unknown, csf->bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = real_len-frame_offset;
		if (csf->bytes_frame_length) frame_length = source->readX(csf->bytes_frame_length) + csf->bytes_frame_length_kludge;
#ifdef COMP_SHAPENUM
		if (shapenum == COMP_SHAPENUM) pout << "frame_length " << frame_length << std::endl;
#endif

		// Seek to start of frame
		source->seek(start_pos + frame_offset + csf->bytes_special);

		if (csf->bytes_special)
			frame->ReadCmpFrame(source, csf, special, f>0?frames+f-1:0);
		else 
			frame->Read(source, csf, frame_length);
	}

#ifdef COMP_SHAPENUM
	if (shapenum == COMP_SHAPENUM) pout << std::dec;
#endif
}

void ConvertShapeFrame::Read(IDataSource *source, const ConvertShapeFormat *csf, uint32 frame_length)
{
	// Read unknown
	if (csf->bytes_frame_unknown) source->read(unknown, csf->bytes_frame_unknown);

	// Frame details
	compression = source->readX(csf->bytes_frame_compression);
	width = source->readXS(csf->bytes_frame_width);
	height = source->readXS(csf->bytes_frame_height);
	xoff = source->readXS(csf->bytes_frame_xoff);
	yoff = source->readXS(csf->bytes_frame_yoff);

#ifdef COMP_SHAPENUM
	if (width <= 0 || height <= 0  ||shapenum == COMP_SHAPENUM )
	{
		pout << "compression " << compression << std::endl;
		pout << "width " << width << std::endl;
		pout << "height " << height << std::endl;
		pout << "xoff " << xoff << std::endl;
		pout << "yoff " << yoff << std::endl;
	}
#endif

	if (compression != 0 && compression != 1) {
		compression = 0;
		width = 0;
		height = 0;
		xoff = 0;
		yoff = 0;
		//perr << "Corrupt frame? (frame " << f << ")" << std::endl;
		perr << "Corrupt frame?" << std::endl;
	}

	if (height) {
		// Line offsets
		line_offsets = new uint32 [height];

		for(sint32 i = 0; i < height; ++i) 
		{
			line_offsets[i] = source->readX(csf->bytes_line_offset);

			// Now fudge with the value and turn it into an offset into the rle data
			// If required
			if (!csf->line_offset_absolute) 
				line_offsets[i] -= (height-i)*csf->bytes_line_offset;
		}

		// Calculate the number of bytes of RLE data
		bytes_rle = frame_length - (csf->len_frameheader2+(height*csf->bytes_line_offset));

#ifdef COMP_SHAPENUM
		if (bytes_rle < 0)
		{
			bytes_rle = 0;
			perr << "Corrupt frame?" << std::endl;
		}
		
#endif
	}
	else 
		line_offsets = 0;

	// Read the RLE Data
	if (bytes_rle) {
		rle_data = new uint8[bytes_rle];
		source->read(rle_data, bytes_rle);
	}
	else 
		rle_data = 0;
}

void ConvertShapeFrame::ReadCmpFrame(IDataSource *source, const ConvertShapeFormat *csf, const uint8 special[256], ConvertShapeFrame *prev)
{
	static OAutoBufferDataSource *rlebuf = 0;
	uint8 outbuf[512];

	// Read unknown
	if (csf->bytes_frame_unknown) source->read(unknown, csf->bytes_frame_unknown);

	// Frame details
	compression = source->readX(csf->bytes_frame_compression);
	width = source->readXS(csf->bytes_frame_width);
	height = source->readXS(csf->bytes_frame_height);
	xoff = source->readXS(csf->bytes_frame_xoff);
	yoff = source->readXS(csf->bytes_frame_yoff);

	line_offsets = new uint32 [height];

	if (!rlebuf) rlebuf = new OAutoBufferDataSource(1024);
	rlebuf->clear();

	for(sint32 y = 0; y < height; ++y) 
	{
		line_offsets[y] = rlebuf->getPos();

		sint32 xpos = 0;

		do
		{
			uint8 skip = source->read1();
			xpos += skip;

			if (xpos > width) {
				source->skip(-1); 
				skip = width-(xpos-skip);
			}

			rlebuf->write1(skip);

			if (xpos >= width) break;

			uint32 dlen = source->read1();
			uint8 *o = outbuf;

			// Is this required???? It seems hacky and pointless
			if (dlen == 0 || dlen == 1) {
				source->skip(-1); 
				rlebuf->skip(-1);
				rlebuf->write1(skip+(width-xpos));
				break;
			}

			int type = 0;
			
			if (compression)  {
				type = dlen & 1;
				dlen >>= 1;
			}

			if (!type) {

				uint32 extra = 0;

				for (uint32 j = 0; j < dlen; j++) {

					uint8 c = source->read1();

					if (special[c] && prev) {
						sint32 count = special[c];
						prev->GetPixels(o,count,xpos-xoff,y-yoff);
						o+=count;
						extra += count-1;
						xpos += count;
					}
					else if (c == 0xFF && prev) {
						sint32 count = source->read1();
						prev->GetPixels(o,count,xpos-xoff,y-yoff);
						o+=count;
						extra += count-2;
						xpos += count;
						j++;
					}
					else {
						*o++ = c;
						xpos++;
					}
				}

				if (((dlen+extra) << compression) > 255) {
					perr << "Error! Corrupt Frame. RLE dlen too large" << std::endl;
				}

				rlebuf->write1((dlen+extra) << compression);
				rlebuf->write(outbuf,dlen+extra);
			}
			else {
				rlebuf->write1((dlen<<1)|1);
				rlebuf->write1(source->read1());
				xpos+=dlen;
			}

		} while (xpos < width);
	}

	bytes_rle = rlebuf->getPos();
	rle_data = new uint8[bytes_rle];
	memcpy (rle_data, rlebuf->getBuf(), bytes_rle);
}

void ConvertShapeFrame::GetPixels(uint8 *buf, sint32 count, sint32 x, sint32 y)
{
	x += xoff;
	y += yoff;

	if (y > height) return;

	sint32 xpos = 0;
	const uint8 * linedata = rle_data + line_offsets[y];

	do {
		xpos += *linedata++;
	  
		if (xpos == width) break;

		sint32 dlen = *linedata++;
		int type = 0;
		
		if (compression) 
		{
			type = dlen & 1;
			dlen >>= 1;
		}

		if (x >= xpos && x < (xpos+dlen))
		{
			int diff = x-xpos;
			dlen-=diff;
			xpos = x;

			int num = count;
			if (dlen < count) num = dlen;

			if (!type) {
				const uint8 *l = (linedata+=diff);

				while (num--) {
					*buf++ = *l++;
					count--;
					x++;
				}
			}
			else {
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

	} while (xpos < width);
}

int ConvertShape::CalcNumFrames(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len, uint32 start_pos)
{
	int f=0;
	uint32 first_offset = 0xFFFFFFFF;

	uint32 save_pos = source->getPos();

	for (f=0;;f++) {

		// Seek to initial pos
		source->seek(start_pos + csf->len_header + (csf->len_frameheader*f));

		if ((source->getPos()-start_pos) >= first_offset) break;

		// Read the offset
		uint32 frame_offset = csf->len_header + (csf->len_frameheader*f);
		if (csf->bytes_frame_offset) frame_offset = source->readX(csf->bytes_frame_offset) + csf->bytes_special;

		if (frame_offset < first_offset) first_offset = frame_offset;

		// Read the unknown
		if (csf->bytes_frameheader_unk) source->skip(csf->bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = real_len-frame_offset;
		if (csf->bytes_frame_length) frame_length = source->readX(csf->bytes_frame_length) + csf->bytes_frame_length_kludge;
	}

	source->seek(save_pos);

	return f;
}

bool ConvertShape::Check(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len)
{
#if 0
	pout << "Testing " << csf->name << "..." << std::endl;
#endif
	bool result = true;

	// Just to be safe
	int start_pos = source->getPos();

	// Read the ident
	if (csf->bytes_ident)
	{
		char ident[5];
		ident[csf->bytes_ident] = 0;
		source->read(ident, csf->bytes_ident);

		if (std::memcmp (ident, csf->ident, csf->bytes_ident))
		{
			// Return to start position
			source->seek(start_pos);
			return false;
		}
	}

	// Read the header special colour
	if (csf->bytes_special) source->skip(csf->bytes_special);

	// Read the header unknown
	if (csf->bytes_header_unk) source->skip(csf->bytes_header_unk);

	// Now read num_frames
	int num_frames = 1;
	if (csf->bytes_num_frames) num_frames = source->readX(csf->bytes_num_frames);
	if (num_frames == 0) num_frames = CalcNumFrames(source,csf,real_len,start_pos);

	// Create frames array
	ConvertShapeFrame oneframe;
	std::memset (&oneframe, 0, sizeof(ConvertShapeFrame));

	// Now read the frames
	for (int f = 0; f < num_frames; f++) 
	{
		ConvertShapeFrame *frame = &oneframe;

		// Seek to initial pos
		source->seek(start_pos + csf->len_header + (csf->len_frameheader*f));

		// Read the offset
		uint32 frame_offset = csf->len_header + (csf->len_frameheader*f);
		if (csf->bytes_frame_offset) frame_offset = source->readX(csf->bytes_frame_offset) + csf->bytes_special;

		// Read the unknown
		if (csf->bytes_frameheader_unk) source->read(frame->header_unknown, csf->bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = real_len-frame_offset;
		if (csf->bytes_frame_length) frame_length = source->readX(csf->bytes_frame_length) + csf->bytes_frame_length_kludge;

		// Invalid frame length
		if ((frame_length + frame_offset) > real_len)
		{
			result = false;
			break;
		}

		// Seek to start of frame
		source->seek(start_pos + frame_offset);

		// Read unknown
		if (csf->bytes_frame_unknown) source->read(frame->unknown, csf->bytes_frame_unknown);

		// Frame details
		frame->compression = source->readX(csf->bytes_frame_compression);
		frame->width = source->readXS(csf->bytes_frame_width);
		frame->height = source->readXS(csf->bytes_frame_height);
		frame->xoff = source->readXS(csf->bytes_frame_xoff);
		frame->yoff = source->readXS(csf->bytes_frame_yoff);

		if ((frame->compression != 0 && frame->compression != 1) || frame->width < 0 || frame->height < 0)
		{
			frame->compression = 0;
			frame->width = 0;
			frame->height = 0;
			frame->xoff = 0;
			frame->yoff = 0;
			result = false;
			break;
		}

		if (frame->height)
		{
			// Line offsets
			sint32 highest_offset_byte = 0;

			// Calculate the number of bytes of RLE data
			frame->bytes_rle = frame_length - (csf->len_frameheader2+(frame->height*csf->bytes_line_offset));

			// Totally invalid shape
			if (frame->bytes_rle < 0)
			{
				result = false;
				break;
			}

			// Only attempt to decompress the shape if we are not a compressed shapes
			if (!csf->bytes_special) {

				// Seek to first in offset table
				source->seek(start_pos + frame_offset + csf->len_frameheader2);

				// Loop through each of the frames and find the last rle run
				for (int i = 0; i < frame->height; i++) 
				{
					sint32 line_offset = source->readX(csf->bytes_line_offset);

					// Now fudge with the value and turn it into an offset into the rle data
					// if required
					if (!csf->line_offset_absolute) 
						line_offset -= (frame->height-i)*csf->bytes_line_offset;

					if (line_offset > frame->bytes_rle)
					{
						result = false;
						break;
					}

					if (line_offset > highest_offset_byte) highest_offset_byte = line_offset;
				};

				// Failed for whatever reason
				if (result == false) break;

				// Jump to the line offset and calculate the length of the run
				source->seek(highest_offset_byte + start_pos + frame_offset + csf->len_frameheader2 + frame->height*csf->bytes_line_offset);
				int xpos = 0;
				uint32 dlen = 0;

				// Compressed
				if (frame->compression) do
				{
					xpos += source->read1();
					if (xpos == frame->width) break;

					dlen = source->read1();
					int type = dlen & 1;
					dlen >>= 1;

					if (!type) source->skip(dlen);
					else source->skip(1);

					xpos += dlen;

				} while (xpos < frame->width);
				// Uncompressed
				else do
				{
					xpos += source->read1();
					if (xpos == frame->width) break;

					dlen = source->read1();
					source->skip(dlen);

					xpos += dlen;
				} while (xpos < frame->width);

				// Calc 'real' bytes rle
				sint32 highest_rle_byte = source->getPos();
				highest_rle_byte -= start_pos + frame_offset + csf->len_frameheader2 + frame->height*csf->bytes_line_offset;

				// Too many bytes
				if (highest_rle_byte > frame->bytes_rle)
				{
					result = false;
					break;
				}
			}
		}
	}

	// Free frames
	oneframe.Free();
	num_frames = 0;

	// Return to start position
	source->seek(start_pos);

	return result;
}

bool ConvertShape::CheckUnsafe(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len)
{
#if 0
	pout << "Testing " << csf->name << "..." << std::endl;
#endif
	bool result = true;

	// Just to be safe
	int start_pos = source->getPos();

	// Read the ident
	if (csf->bytes_ident)
	{
		char ident[5];
		ident[csf->bytes_ident] = 0;
		source->read(ident, csf->bytes_ident);

		if (std::memcmp (ident, csf->ident, csf->bytes_ident))
		{
			// Return to start position
			source->seek(start_pos);
			return false;
		}
	}

	// Read the header special colour
	if (csf->bytes_special) source->skip(csf->bytes_special);

	// Read the header unknown
	if (csf->bytes_header_unk) source->skip(csf->bytes_header_unk);

	// Now read num_frames
	int num_frames = 1;
	if (csf->bytes_num_frames) num_frames = source->readX(csf->bytes_num_frames);
	if (num_frames == 0) num_frames = CalcNumFrames(source,csf,real_len,start_pos);

	// Create frames array
	ConvertShapeFrame oneframe;
	std::memset (&oneframe, 0, sizeof(ConvertShapeFrame));

	// Now read the frames
	for (int f = 0; f < num_frames; f++) 
	{
		ConvertShapeFrame *frame = &oneframe;

		// Seek to initial pos
		source->seek(start_pos + csf->len_header + (csf->len_frameheader*f));

		// Read the offset
		uint32 frame_offset = csf->len_header + (csf->len_frameheader*f);
		if (csf->bytes_frame_offset) frame_offset = source->readX(csf->bytes_frame_offset) + csf->bytes_special;

		// Read the unknown
		if (csf->bytes_frameheader_unk) source->read(frame->header_unknown, csf->bytes_frameheader_unk);

		// Read frame_length
		uint32 frame_length = real_len-frame_offset;
		if (csf->bytes_frame_length) frame_length = source->readX(csf->bytes_frame_length) + csf->bytes_frame_length_kludge;

		// Invalid frame length
		if ((frame_length + frame_offset) > real_len)
		{
			result = false;
			break;
		}

		// Seek to start of frame
		source->seek(start_pos + frame_offset);

		// Read unknown
		if (csf->bytes_frame_unknown) source->read(frame->unknown, csf->bytes_frame_unknown);

		// Frame details
		frame->compression = source->readX(csf->bytes_frame_compression);
		frame->width = source->readXS(csf->bytes_frame_width);
		frame->height = source->readXS(csf->bytes_frame_height);
		frame->xoff = source->readXS(csf->bytes_frame_xoff);
		frame->yoff = source->readXS(csf->bytes_frame_yoff);

		if ((frame->compression != 0 && frame->compression != 1) || frame->width < 0 || frame->height < 0)
		{
			frame->compression = 0;
			frame->width = 0;
			frame->height = 0;
			frame->xoff = 0;
			frame->yoff = 0;
			result = false;
			break;
		}

		if (frame->height)
		{
			// Calculate the number of bytes of RLE data (may not be accurate but we don't care)
			frame->bytes_rle = frame_length - (csf->len_frameheader2+(frame->height*csf->bytes_line_offset));

			// Totally invalid shape
			if (frame->bytes_rle < 0)
			{
				result = false;
				break;
			}
		}
	}

	// Free frames
	oneframe.Free();
	num_frames = 0;

	// Return to start position
	source->seek(start_pos);

	return result;
}

void ConvertShape::Write(ODataSource *dest, const ConvertShapeFormat *csf, uint32 &write_len)
{
	// Just to be safe
	uint32 start_pos = dest->getPos();

	// Write the ident
	if (csf->bytes_ident) dest->write(csf->ident, csf->bytes_ident);

	// Write the header unknown
	if (csf->bytes_header_unk) dest->write(header_unknown, csf->bytes_header_unk);

	// Now write num_frames
	if (csf->bytes_num_frames) dest->writeX(num_frames, csf->bytes_num_frames);
	else if (!csf->bytes_num_frames && num_frames > 1)
	{
		perr << "Error: Unable to convert multiple frame shapes to " << csf->name << std::endl; 
		return;
	}

	// Write filler space for the frame details
	for (uint32 i = 0; i < num_frames*csf->len_frameheader; i++) dest->write1(0);

	// Now write the frames
	for(uint32 f = 0; f < num_frames; f++) 
	{
		ConvertShapeFrame *frame = frames+f;

		// Get the frame offset
		uint32 frame_offset = dest->getPos() - start_pos;

		// Seek to the frame header pos
		dest->seek(start_pos + csf->len_header + (csf->len_frameheader*f));

		// Write the offset
		if (csf->bytes_frame_offset) dest->writeX(frame_offset, csf->bytes_frame_offset);

		// Write the unknown
		if (csf->bytes_frameheader_unk) dest->write(frame->header_unknown, csf->bytes_frameheader_unk);

		// Calc and write frame_length
		if (csf->bytes_frame_length)
		{
			uint32 frame_length = csf->len_frameheader2 + (frame->height*csf->bytes_line_offset) + frame->bytes_rle;
			dest->writeX(frame_length - csf->bytes_frame_length_kludge, csf->bytes_frame_length);
		}

		// Seek to start of frame
		dest->seek(start_pos + frame_offset);

		// Write unknown
		if (csf->bytes_frame_unknown) dest->write(frame->unknown, csf->bytes_frame_unknown);

		// Frame details
		dest->writeX(frame->compression, csf->bytes_frame_compression);
		dest->writeX(frame->width, csf->bytes_frame_width);
		dest->writeX(frame->height, csf->bytes_frame_height);
		dest->writeX(frame->xoff, csf->bytes_frame_xoff);
		dest->writeX(frame->yoff, csf->bytes_frame_yoff);

		// Line offsets
		for (sint32 i = 0; i < frame->height; i++) 
		{
			sint32 actual_offset = frame->line_offsets[i];
			
			// Unfudge the value and write it, if requiretd
			if (!csf->line_offset_absolute)  
				actual_offset += (frame->height-i)*csf->bytes_line_offset;

			dest->writeX(actual_offset, csf->bytes_line_offset);
		}

		// Write the RLE Data
		dest->write(frame->rle_data, frame->bytes_rle);
	}

	// Just cheat
	write_len = dest->getPos() - start_pos;
}


// Shape format configuration for Pentagram
const ConvertShapeFormat		PentagramShapeFormat =
{
	"Pentagram",
	8,		// header
	"PSHP",	// ident
	4,		// bytes_ident 
	0,		// bytes_special
	0,		// header_unk
	4,		// num_frames

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
