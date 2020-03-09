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

#ifndef ULTIMA8_CONVERT_CONVERTSHAPE_H
#define ULTIMA8_CONVERT_CONVERTSHAPE_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima8 {

class IDataSource;
class ODataSource;

// Convert shape C

/********** Update AutoShapeFormat in shapeconv/ShapeConv.cpp when changed! **********/
struct ConvertShapeFormat {
	const char *		_name;
													//	U8		U8 Gump	U8.SKF	Cru		Cru2D	Pent	Comp
	uint32				_len_header;				//	6		6		2		6		6		8		11
	const char *		_ident;						//  ""		""		"\2\0"	""		""		"PSHP"	""
	uint32				_bytes_ident;				//	0		0		2		0		0		4		0
	uint32				_bytes_special;				//	0		0		0		0		0		0		5
	uint32				_bytes_header_unk;			//	4		4		0		4		4		0		4
	uint32				_bytes_num_frames;			//	2		2		0		2		2		4		2

	uint32				_len_frameheader;			//	6		6		0		8		8		8		6
	uint32				_bytes_frame_offset;		//	3		3		0		3		3		4		4
	uint32				_bytes_frameheader_unk;		//	1		2		0		2		2		0		0
	uint32				_bytes_frame_length;		//	2		2		0		3		3		4		2
	uint32				_bytes_frame_length_kludge;	//	0		8		0		0		0		0		0

	uint32				_len_frameheader2;			//	18		18		10		28		20		20		10
	uint32				_bytes_frame_unknown;		//	8		8		0		8		0		0		0
	uint32				_bytes_frame_compression;	//	2		2		2		4		4		4		2
	uint32				_bytes_frame_width;			//	2		2		2		4		4		4		2
	uint32				_bytes_frame_height;		//	2		2		2		4		4		4		2
	uint32				_bytes_frame_xoff;			//	2		2		2		4		4		4		2
	uint32				_bytes_frame_yoff;			//	2		2		2		4		4		4		2

	uint32				_bytes_line_offset;			//	2		2		2		4		4		4		0
	uint32				_line_offset_absolute;		//	0		0		0		0		0		1		0
};

// ConvertShapeFrame structure

struct ConvertShapeFrame {
	uint8				_header_unknown[2];

	uint8				_unknown[8];
	uint32				_compression;
	int32				_width;
	int32				_height;
	int32				_xoff;
	int32				_yoff;

	uint32				*_line_offsets;		// Note these are offsets into rle_data

	int32				_bytes_rle;			// Number of bytes of RLE Data
	uint8				*_rle_data;

	void Free();

	void Read(IDataSource *source, const ConvertShapeFormat *csf, uint32 frame_length);

	void ReadCmpFrame(IDataSource *source, const ConvertShapeFormat *csf, const uint8 special[256], ConvertShapeFrame *prev);

	void GetPixels(uint8 *buf, int32 count, int32 x, int32 y);
};


// ConvertShape structure

class ConvertShape
{
	uint8				_header_unknown[4];
	uint32				_num_frames;
	ConvertShapeFrame	*_frames;

public:
	ConvertShape();

	~ConvertShape()
	{
		Free();
	}

	void Free();

	void Read(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len);
	void Write(ODataSource *source, const ConvertShapeFormat *csf, uint32 &write_len);

	// This will check to see if a Shape is of a certain type. Return true if ok, false if bad
	static bool Check(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len);

	// This will also check to see if a shape is of a certain type. However it won't check
	// the rle data, it only checks headers. Return true if ok, false if bad
	static bool CheckUnsafe(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len);

	// Algorithmically calculate the number of frames
	static int CalcNumFrames(IDataSource *source, const ConvertShapeFormat *csf, uint32 real_len, uint32 start_pos);
};

// Shape format configuration for Pentagram format
extern const ConvertShapeFormat		PentagramShapeFormat;

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
