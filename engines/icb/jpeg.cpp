/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

//
// Copyright (c) 1997,1998 Colosseum Builders, Inc.
// All rights reserved.
//
// Colosseum Builders, Inc. makes no warranty, expressed or implied
// with regards to this software. It is provided as is.
//
// See the README.TXT file that came with this software for restrictions
// on the use and redistribution of this file or send E-mail to
// info@colosseumbuilders.com
//

#include "engines/icb/jpeg.h"
#include "engines/icb/global_objects_pc.h"

#ifndef _WIN32
#define _flushall() fflush(NULL)
#endif

namespace ICB {

// A.3.6 Figure A.6
// These values are the inverse of those shown in the
// JPEG standard.
const unsigned int JpegZigZagInputOrderCodes[JpegSampleSize] = {0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,  12, 19, 26, 33, 40, 48,
                                                                41, 34, 27, 20, 13, 6,  7,  14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23,
                                                                30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};

const unsigned int JpegZigZagOutputOrderCodes[JpegSampleSize] = {0,  1,  5,  6,  14, 15, 27, 28, 2,  4,  7,  13, 16, 26, 29, 42, 3,  8,  12, 17, 25, 30,
                                                                 41, 43, 9,  11, 18, 24, 31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 21, 33, 38,
                                                                 46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63};

JpegDecoder::JpegDecoder() {
	Initialize();
	return;
}

JpegDecoder::~JpegDecoder() {
	delete[] ac_tables;
	ac_tables = NULL;
	delete[] dc_tables;
	dc_tables = NULL;

	delete[] quantization_tables;
	quantization_tables = NULL;
	delete[] components;
	components = NULL;
	delete[] component_indices;
	component_indices = NULL;
	delete[] scan_components;
	scan_components = NULL;
}

void JpegDecoder::Initialize() {
	ac_tables = new JpegHuffmanDecoder[JpegMaxHuffmanTables];
	dc_tables = new JpegHuffmanDecoder[JpegMaxHuffmanTables];

	quantization_tables = new JpegDecoderQuantizationTable[MaxQuantizationTables];
	components = new JpegDecoderComponent[JpegMaxComponentsPerFrame];
	component_indices = new unsigned int[JpegMaxComponentsPerFrame];

	scan_components = new JpegDecoderComponent *[JpegMaxComponentsPerScan];

	bit_position = 0;
}

void JpegDecoder::ReadMarker() {
	while (1) {
		uint8 type = ReadByte();
		switch (type) {
		case SOI:
			return; // SOI has no data.
		case DQT:
			ReadQuantization();
			return;
		// The only difference between a Sequential DCT Frame
		// (SOF0) and an extended Sequential DCT Frame (SOF1)
		// is that a baseline frame may only have 2 DC and 2 AC
		// Huffman tables per scan (F.1.2) and and extended
		// Sequential Frame may have up to 4 DC and 4 AC Huffman
		// tables per scan (F.1.3). Both are decoded identically
		// for 8-bit precision. Extended Sequential frames may
		// use 12-bit precision (F, Table B.2) which we do not
		// support.
		case SOF0:
		case SOF1:
			ReadStartOfFrame(type);
			return;
		// These are markers for frames using arithmetic coding.
		// Arithmetic coding is covered by patents so we ignore
		// this type.
		case SOF9:
		case SOFA:
		case SOFB:
		case SOFD:
		case SOFE:
		case DHT:
			ReadHuffmanTable();
			return;
		case SOS:
			ReadStartOfScan();
			return;
		case EOI:
			eoi_found = true;
			return; // End of Image marker has no data
		default:
			// We call ReadByte to make sure the problem
			// is not a premature EOF.
			(void)ReadByte();
			_flushall();
			// throw ("Unknown, unsupported, or reserved marker encountered");
		}
	}
	_flushall();
}

void JpegDecoder::ReadHuffmanTable() {
	// Section B.2.4.2
	uint16 length = ReadWord();
	unsigned int remaining = length - sizeof(length);
	while (remaining > 0) {
		uint8 data = ReadByte();
		--remaining;

		// Tc in standard 0=>DC, 1=>AC
		unsigned int tableclass = data >> 4;
		unsigned int id = data & 0x0F; // Th in standard

		JpegHuffmanDecoder *table;
		if (tableclass != 0)
			table = &ac_tables[id];
		else
			table = &dc_tables[id];

		// Read the table data into the table object
		remaining -= table->ReadTable(*this);
	}
}

void JpegDecoder::ReadQuantization() {
	// Defined in Section B.2.4.1
	uint16 length = ReadWord();
	uint8 data;

	// Maintain a counter for the number of bytes remaining to be read in
	// the quantization table.
	int remaining = length - sizeof(length);

	while (remaining > 0) {
		data = ReadByte();
		--remaining;
		unsigned int precision = data >> 4; // Pq in standard
		unsigned int index = data & 0x0F;   // Tq in standard

		switch (precision) {
		case 1:
			remaining -= sizeof(uint16) * JpegSampleSize;
			break;
		case 0:
			remaining -= sizeof(uint8) * JpegSampleSize;
			break;
		}

		// Read the table data into the table object
		quantization_tables[index].ReadTable(*this, precision);
	}
}

void JpegDecoder::ReadStartOfFrame(uint8 type) {
	// Section B.2.2
	// Read in the image dimensions
	/* unsigned int length = */ ReadWord();
	/* unsigned int dataprecision = */ ReadByte(); // P in standard

	component_count = ReadByte(); // Nf in standard

	frame_type = type;

	// Rread the component descriptions
	max_horizontal_frequency = 0;
	max_vertical_frequency = 0;
	for (unsigned int ii = 0; ii < component_count; ++ii) {
		unsigned int ID = ReadByte(); // Ci in standard
		unsigned int qtable;

		component_indices[ii] = ID;

		uint8 data = ReadByte();
		components[ID].horizontal_frequency = (data >> 4); // Hi in standard
		components[ID].vertical_frequency = (data & 0xF);  // Vi in standard
		qtable = ReadByte();                               // Tqi in standard

		components[ID].SetQuantizationTable(quantization_tables[qtable]);

		// Keep track of the largest values for horizontal and vertical
		// frequency.
		if (components[ID].horizontal_frequency > max_horizontal_frequency) {
			max_horizontal_frequency = components[ID].horizontal_frequency;
		}

		if (components[ID].vertical_frequency > max_vertical_frequency) {
			max_vertical_frequency = components[ID].vertical_frequency;
		}
	}

	CalculateMcuDimensions();

	sof_found = true;
}

void JpegDecoder::ReadStartOfScan() {
	unsigned int ii;

	// Section B.2.3
	/*uint16 length = */ ReadWord();

	scan_component_count = ReadByte(); // Ns in standard

	// Right now we can only handle up to three components.
	for (ii = 0; ii < scan_component_count; ++ii) {
		uint8 componentID = ReadByte(); // Csi in standard

		scan_components[ii] = &components[componentID];
		// If the horizontal frequency is zero then the component was not
		// defined in the SOFx marker.
		uint8 rb = ReadByte();
		unsigned int actable = rb & 0x0F;
		unsigned int dctable = rb >> 4;

		scan_components[ii]->SetHuffmanTables(dc_tables[dctable], ac_tables[actable]);
	}

	/* unsigned int spectralselectionstart =*/ReadByte(); // Ss in standard
	/* unsigned int spectralselectionend = */ ReadByte(); // Se in standard

	/* uint8 ssa = */ ReadByte();
	//  unsigned int successiveapproximationhigh = ssa >> 4;  // Ah in standard
	//  unsigned int successiveapproximationlow = ssa & 0x0F; // Al in standard

	for (ii = 0; ii < scan_component_count; ++ii) {
		scan_components[ii]->AllocateComponentBuffers(*this);
	}

	++current_scan;
	ReadSequentialNonInterleavedScan();
}

void JpegDecoder::CalculateMcuDimensions() {
	mcu_height = max_vertical_frequency * JpegSampleWidth;
	mcu_width = max_horizontal_frequency * JpegSampleWidth;
	mcu_rows = (480 + mcu_height - 1) / mcu_height;
	mcu_cols = (640 + mcu_width - 1) / mcu_width;
	return;
}

void JpegDecoder::ReadSequentialNonInterleavedScan() {
	ResetDcDifferences();

	for (unsigned int row = 0; row < scan_components[0]->noninterleaved_rows; ++row) {
		for (unsigned int col = 0; col < scan_components[0]->noninterleaved_cols; ++col) {
			scan_components[0]->DecodeSequential(*this, row, col);
		}
	}
}

void JpegDecoder::ResetDcDifferences() {
	for (unsigned int ii = 0; ii < scan_component_count; ++ii)
		scan_components[ii]->last_dc_value = 0;
}

void JpegDecoder::ReadImage(unsigned char *inputData, uint32 surface_Id) {
	uint8 data;

	input_buffer = inputData;
	surfaceId = surface_Id;
	iPos = 0;
	current_scan = 0;
	scan_count = 0;

	eoi_found = false;
	sof_found = false;

	_bit_count = 0;
	_buf_pointer = (uint32 *)input_buffer;
	_end_buf = (uint32 *)(input_buffer + (1024 * 1024)); // blantantly wrong... need to make cash sized buffer

	data = ReadByte();
	while (!eoi_found) {
		if (data == SOB) {
			ReadMarker();

			if (eoi_found)
				break;
		} else
			data = 0xff;
		data = ReadByte();
	}

	UpdateImage();

	FreeAllocatedResources();
	return;
}

void JpegDecoder::UpdateImage() {
	components[component_indices[0]].Upsample();
	components[component_indices[1]].Upsample();
	components[component_indices[2]].Upsample();

	JpegDecoderComponent::RGBConvert(components[component_indices[0]], components[component_indices[1]], components[component_indices[2]], surfaceId);
}

void JpegDecoder::FreeAllocatedResources() {
	for (unsigned int ii = 0; ii < component_count; ++ii) {
		components[component_indices[ii]].FreeComponentBuffers();
	}
}

int JpegDecoder::cGetBit() {
	// Section F.2.2.5 Figure F.18.
	// CNT is called bitposition
	// B is called bitdata
	if (bit_position == 0) {
		// We are out of data so read the next byte from the input stream.
		bit_data = input_buffer[iPos++];

		// Reset the bit read position starting with the highest order bit. We
		// read high to low.
		bit_position = sizeof(bit_data) << 3;

		// The value FF is stored as FF00 to distinguish it from markers.
		// So this bit of code skips the following 0x00 after the 0xff
		//          if (bit_data == 0xFF)
		//              iPos++;
	}

	// Consume one bit of the input.
	--bit_position;

	// Shift the value to the low order bit position.
	uint8 result = (uint8)((bit_data >> bit_position) & 1);

	return result;
}

int JpegDecoder::NextBit() {
	int result = cGetBit();
	return result;
}

// Extracts the next "count" bits from the input stream.
int JpegDecoder::Receive(unsigned int count) {
	int result = 0;
	for (unsigned int ii = 0; ii < count; ++ii) {
		result <<= 1;
		result |= cGetBit();
	}
	return result;
}

JpegHuffmanDecoder::JpegHuffmanDecoder() {}

unsigned int JpegHuffmanDecoder::ReadTable(JpegDecoder &decoder) {
	// We declare this here because MSVC++ does not handle for
	// statement scoping rules correctly.
	unsigned int jj;

	// B.2.4.2
	uint8 huffbits[JpegMaxHuffmanCodeLength];

	unsigned int count = 0; // Count of codes in the Huffman table.

	// Read the 16 1-byte length counts and count the number of
	// codes in the table.
	for (jj = 0; jj < JpegMaxHuffmanCodeLength; ++jj) {
		// These values are called Li in the standard.
		huffbits[jj] = decoder.ReadByte();
		count += huffbits[jj];
	}

	// Read the Huffman values.
	for (jj = 0; jj < count; ++jj) {
		// These values are called Vi in the standard.
		huff_values[jj] = decoder.ReadByte();
	}

	// Generate the Structures for Huffman Decoding.
	MakeTable(huffbits);

	return JpegMaxHuffmanCodeLength + count;
}

void JpegHuffmanDecoder::MakeTable(uint8 huffbits[JpegMaxHuffmanCodeLength]) {
	// We have to declare the loop indices here because MSVC++ does not
	// handle scoping in for statements correctly.
	unsigned int ii, jj, kk;

	// These values in these arrays correspond to the elements of the
	// "values" array. The Huffman code for values [N] is huffcodes [N]
	// and the length of the code is huffsizes [N].

	uint16 huffcodes[JpegMaxNumberOfHuffmanCodes];
	unsigned int huffsizes[JpegMaxNumberOfHuffmanCodes + 1];

	// Section C.2 Figure C.1
	// Convert the array "huff_bits" containing the count of codes
	// for each length 1..16 into an array containing the length for each
	// code.
	for (ii = 0, kk = 0; ii < JpegMaxHuffmanCodeLength; ++ii) {
		for (jj = 0; jj < huffbits[ii]; ++jj) {
			huffsizes[kk] = ii + 1;
			++kk;
		}
		huffsizes[kk] = 0;
	}

	// Section C.2 Figure C.2
	// Calculate the Huffman code for each Huffman value.
	uint16 code = 0;
	unsigned int si;
	for (kk = 0, si = huffsizes[0]; huffsizes[kk] != 0; ++si, code <<= 1) {
		for (; huffsizes[kk] == si; ++code, ++kk) {
			huffcodes[kk] = code;
		}
	}

	// Section F.2.2. Figure F.15
	// Create three arrays.
	// mincode [n] : The smallest Huffman code of length n + 1.
	// maxcode [n] : The largest Huffman code of length n + 1.
	// valptr [n] : Index into the values array. First value with a code
	//                    of length n + 1.
	for (ii = 0, jj = 0; ii < JpegMaxHuffmanCodeLength; ++ii) {
		// ii is the index into Huffman code lengths
		// jj is the index into Huffman code values
		if (huffbits[ii] != 0) {
			// The jj'th Huffman value is the first with a Huffman code
			// of length ii.
			valptr[ii] = (uint8)jj;
			mincode[ii] = huffcodes[jj];
			jj += huffbits[ii];
			maxcode[ii] = huffcodes[jj - 1];
		} else {
			// There are no Huffman codes of length (ii + 1).
			maxcode[ii] = -1;
			// An illegal value > maxcode[]
			mincode[ii] = JpegMaxNumberOfHuffmanCodes + 1;
			valptr[ii] = 0;
		}
	}
}

int JpegHuffmanDecoder::Decode(JpegDecoder &decoder) {
	// This function decodes the next byte in the input stream using this
	// Huffman table.

	// Section A F.2.2.3 Figure F.16

	uint16 code = (uint16)decoder.NextBit();
	int codelength; // Called I in the standard.

	// Here we are taking advantage of the fact that 1 bits are used as
	// a prefix to the longer codes.
	for (codelength = 0; (code > maxcode[codelength] && codelength < JpegMaxHuffmanCodeLength); ++codelength) {
		code = (uint16)((code << 1) | decoder.NextBit());
	}

	// Now we have a Huffman code of length (codelength + 1) that
	// is somewhere in the range
	// mincode [codelength]..maxcode [codelength].

	// This code is the (offset + 1)'th code of (codelength + 1);
	int offset = code - mincode[codelength];

	// valptr [codelength] is the first code of length (codelength + 1)
	// so now we can look up the value for the Huffman code in the table.
	int index = valptr[codelength] + offset;
	return huff_values[index];
}

//
//  Description:
//
//   This function tells if the Huffman table has been defined
//   by the JPEG input stream.  It is used to detect corrupt
//   streams that have scans that use a Huffman table before
//   it has been defined.
//
//  Return Value:
//
//    true => The table has been defind
//    false => The table has not been defined
//

// JPEG Decoder Quantization Table Class Implementation

//
// This table consists of the values
//
//   F (i, j) = X (i) X (j) / 8
//
// where
//
//  X (n) = 1, n = 0, 4
//  X (n) = 1 / sqrt(2) / cos (n*PI/16)
//

static const double floatscaling[JpegSampleWidth][JpegSampleWidth] = {
    {
        0.125, 0.09011997775086849627, 0.09567085809127244544, 0.1063037618459070632, 0.125, 0.159094822571604233, 0.2309698831278216846, 0.4530637231764438333,
    },
    {
        0.09011997775086849627, 0.0649728831185362593, 0.0689748448207357645, 0.07664074121909414394, 0.09011997775086849627, 0.1147009749634507608, 0.1665200058287998886,
        0.3266407412190940884,
    },
    {
        0.09567085809127244544, 0.0689748448207357645, 0.0732233047033631207, 0.08136137691302557096, 0.09567085809127244544, 0.1217659055464329343, 0.1767766952966368932,
        0.3467599613305368256,
    },
    {
        0.1063037618459070632, 0.07664074121909414394, 0.08136137691302557096, 0.09040391826073060355, 0.1063037618459070632, 0.135299025036549253, 0.1964237395967755595,
        0.3852990250365491698,
    },
    {
        0.125, 0.09011997775086849627, 0.09567085809127244544, 0.1063037618459070632, 0.125, 0.159094822571604233, 0.2309698831278216846, 0.4530637231764438333,
    },
    {
        0.159094822571604233, 0.1147009749634507608, 0.1217659055464329343, 0.135299025036549253, 0.159094822571604233, 0.2024893005527218515, 0.2939689006048396558,
        0.5766407412190940329,
    },
    {
        0.2309698831278216846, 0.1665200058287998886, 0.1767766952966368932, 0.1964237395967755595, 0.2309698831278216846, 0.2939689006048396558, 0.4267766952966368654,
        0.8371526015321518744,
    },
    {
        0.4530637231764438333, 0.3266407412190940884, 0.3467599613305368256, 0.3852990250365491698, 0.4530637231764438333, 0.5766407412190940329, 0.8371526015321518744,
        1.642133898068010689,
    },
};

//
//  Description:
//
//    Class Default Constructor
//
JpegDecoderQuantizationTable::JpegDecoderQuantizationTable() { memset(data_values, 0, sizeof(data_values)); }

//
//  Description:
//
//    This function reads a quantization table from a JPEG stream.
//
//  Parameters:
//    decoder:  The JPEG decoder that owns the table and the JPEG stream.
//    precision: The quantization table precision
//
void JpegDecoderQuantizationTable::ReadTable(JpegDecoder &decoder, unsigned int) {
	// Read 8-bit values.
	for (unsigned int ii = 0; ii < JpegSampleSize; ++ii) {
		data_values[ii] = decoder.ReadByte();
	}

	BuildScaledTables();
}

//
//  Description:
//
//    This function creates scaled quantization tables that
//    allow quantization to be merged with the IDCT process.
//    We factor the DCT matrix so that the first step in the
//    IDCT is to multiply each value by a constant. Here we
//    merge that constant with the quantization table valus.
//
void JpegDecoderQuantizationTable::BuildScaledTables() {
	unsigned int ii; // Overcome MSVC++ Wierdness

	for (ii = 0; ii < JpegSampleWidth; ++ii) {
		for (int jj = 0; jj < JpegSampleWidth; ++jj) {
			float_scaling[ii][jj] = data_values[JpegZigZagOutputOrderCodes[ii * 8 + jj]] * floatscaling[ii][jj];
		}
	}

	for (ii = 0; ii < JpegSampleWidth; ++ii) {
		for (int jj = 0; jj < JpegSampleWidth; ++jj) {
			integer_scaling[ii][jj] = (int32)((1 << QuantizationIntegerScale) * floatscaling[ii][jj] * data_values[JpegZigZagOutputOrderCodes[ii * 8 + jj]]);
		}
	}
}

//
// JPEG Decoder Data Unit class implementation
//
// This class represents an 8x8 sample block for one
// component of the JPEG image. Its main function is to perform the
// Inverse Discrete Cosine Transform.
//
// We have two IDCT implementation defined here. One uses floating point
// and the other uses scaled integers. The integer implementation is much
// faster but it is slightly less precise than the floating point.
//
// (Right now the choice of the two is made at compile time. In the
//  future there may be a run-time switch).
//
// The algorithm is a matrix factorization that makes use extensive of the
// cosine product formula. The first phase of the IDCT is merged with
// quantization.
//
JpegDecoderDataUnit::IDctFunction JpegDecoderDataUnit::idct_function = &JpegDecoderDataUnit::IntegerInverseDCT;

const int IntegerScale = 6;

const int32 IC4 = (int32)((1 << IntegerScale) * cos(M_PI * 4.0 / 16.0));
const int32 ISEC2 = (int32)((1 << (IntegerScale - 1)) / cos(M_PI * 2.0 / 16.0));
const int32 ISEC6 = (int32)((1 << (IntegerScale - 1)) / cos(M_PI * 6.0 / 16.0));

const double FC4 = cos(M_PI * 4.0 / 16.0);
const double FSEC2 = 0.5 / cos(M_PI * 2.0 / 16.0);
const double FSEC6 = 0.5 / cos(M_PI * 6.0 / 16.0);

//
//  Description:
//
//    This is a floating point implementation of the Inverse
//    Discrete Cosine Transform (IDCT).
//
//    This implementation uses a factorization of the DCT matrix.
//    The first steps in this factorization is a matrix multiplication
//    is the multiplication of each row/column by a scale. This
//    scalor multipliation has been combined with quantization
//    to eliminate 128 multiplication steps.
//
//    We use a lot of temporaries here in order to clearly
//    show the matrix multiplication steps.  Hopefully
//    your compiler will optimize out the unnecessary
//    intermediate variables.
//
//    If your compiler does not aggressively optimize. It is possible
//    to reorder the operations to reduce the number of temporaries
//    required.
//
//  Parameters:
//    data: The 8x8 matrix to perform the IDCT on.
//    qt: The prescaled quantization table.
//
JpegDecoderDataUnit &JpegDecoderDataUnit::FloatInverseDCT(JpegDecoderCoefficientBlock data, const JpegDecoderQuantizationTable &qt) {
	double tmp[JpegSampleWidth][JpegSampleWidth];
	unsigned int ii;
	for (ii = 0; ii < JpegSampleWidth; ++ii) {
		double a0 = data[ii][0] * qt.float_scaling[ii][0];
		double a1 = data[ii][4] * qt.float_scaling[ii][4];
		double a2 = data[ii][2] * qt.float_scaling[ii][2];
		double a3 = data[ii][6] * qt.float_scaling[ii][6];
		double a4 = data[ii][1] * qt.float_scaling[ii][1];
		double a5 = data[ii][5] * qt.float_scaling[ii][5];
		double a6 = data[ii][3] * qt.float_scaling[ii][3];
		double a7 = data[ii][7] * qt.float_scaling[ii][7];

		double b0 = a0;
		double b1 = a1;
		double b2 = a2 - a3;
		double b3 = a2 + a3;
		double b4 = a4 - a7;
		double b5 = a5 + a6;
		double b6 = a5 - a6;
		double b7 = a4 + a7;

		double c0 = b0;
		double c1 = b1;
		double c2 = b2;
		double c3 = b3;
		double c4 = FSEC2 * b4;
		double c5 = b7 - b5;
		double c6 = FSEC6 * b6;
		double c7 = b5 + b7;

		double d0 = c0;
		double d1 = c1;
		double d2 = c2;
		double d3 = c3;
		double d4 = c4 + c6;
		double d5 = c5;
		double d6 = c4 - c6;
		double d7 = c7;

		double e0 = d0 + d1;
		double e1 = d0 - d1;
		double e2 = d2 * FC4;
		double e3 = d3;
		double e4 = d4 * FC4;
		double e5 = d5 * FC4;
		double e6 = d6;
		double e7 = d7;

		double f0 = e0;
		double f1 = e1;
		double f2 = e2;
		double f3 = e3;
		double f4 = e4;
		double f5 = e5;
		double f6 = e4 + e6;
		double f7 = e7;

		double g0 = f0;
		double g1 = f1;
		double g2 = f2;
		double g3 = f2 + f3;
		double g4 = f4;
		double g5 = f4 + f5;
		double g6 = f5 + f6;
		double g7 = f6 + f7;

		double h0 = g0 + g3;
		double h1 = g1 + g2;
		double h2 = g1 - g2;
		double h3 = g0 - g3;
		double h4 = g4;
		double h5 = g5;
		double h6 = g6;
		double h7 = g7;

		tmp[ii][0] = h0 + h7;
		tmp[ii][1] = h1 + h6;
		tmp[ii][2] = h2 + h5;
		tmp[ii][3] = h3 + h4;
		tmp[ii][4] = h3 - h4;
		tmp[ii][5] = h2 - h5;
		tmp[ii][6] = h1 - h6;
		tmp[ii][7] = h0 - h7;
	}

	for (ii = 0; ii < JpegSampleWidth; ++ii) {
		double a0 = tmp[0][ii];
		double a1 = tmp[4][ii];
		double a2 = tmp[2][ii];
		double a3 = tmp[6][ii];
		double a4 = tmp[1][ii];
		double a5 = tmp[5][ii];
		double a6 = tmp[3][ii];
		double a7 = tmp[7][ii];

		double b0 = a0;
		double b1 = a1;
		double b2 = a2 - a3;
		double b3 = a2 + a3;
		double b4 = a4 - a7;
		double b5 = a5 + a6;
		double b6 = a5 - a6;
		double b7 = a4 + a7;

		double c0 = b0;
		double c1 = b1;
		double c2 = b2;
		double c3 = b3;
		double c4 = FSEC2 * b4;
		double c5 = b7 - b5;
		double c6 = FSEC6 * b6;
		double c7 = b5 + b7;

		double d0 = c0;
		double d1 = c1;
		double d2 = c2;
		double d3 = c3;
		double d4 = c4 + c6;
		double d5 = c5;
		double d6 = c4 - c6;
		double d7 = c7;

		double e0 = d0 + d1;
		double e1 = d0 - d1;
		double e2 = d2 * FC4;
		double e3 = d3;
		double e4 = d4 * FC4;
		double e5 = d5 * FC4;
		double e6 = d6;
		double e7 = d7;

		double f0 = e0;
		double f1 = e1;
		double f2 = e2;
		double f3 = e3;
		double f4 = e4;
		double f5 = e5;
		double f6 = e4 + e6;
		double f7 = e7;

		double g0 = f0;
		double g1 = f1;
		double g2 = f2;
		double g3 = f2 + f3;
		double g4 = f4;
		double g5 = f4 + f5;
		double g6 = f5 + f6;
		double g7 = f6 + f7;

		double h0 = g0 + g3;
		double h1 = g1 + g2;
		double h2 = g1 - g2;
		double h3 = g0 - g3;
		double h4 = g4;
		double h5 = g5;
		double h6 = g6;
		double h7 = g7;

		static const double rounding = JpegMidpointSampleValue + 0.5;
		values[0][ii] = SampleRange((int32)((h0 + h7) + rounding));
		values[1][ii] = SampleRange((int32)((h1 + h6) + rounding));
		values[2][ii] = SampleRange((int32)((h2 + h5) + rounding));
		values[3][ii] = SampleRange((int32)((h3 + h4) + rounding));
		values[4][ii] = SampleRange((int32)((h3 - h4) + rounding));
		values[5][ii] = SampleRange((int32)((h2 - h5) + rounding));
		values[6][ii] = SampleRange((int32)((h1 - h6) + rounding));
		values[7][ii] = SampleRange((int32)((h0 - h7) + rounding));
	}
	return *this;
}

//
//  Description:
//
//    This is a scaled integer implementation of the Inverse
//    Discrete Cosine Transform (IDCT).
//
//    This implementation uses a factorization of the DCT matrix.
//    The first steps in this factorization is a matrix multiplication
//    is the multiplication of each row/column by a scale. This
//    scalor multipliation has been combined with quantization
//    to eliminate 128 multiplication steps.
//
//    We use a lot of temporaries here in order to clearly
//    show the matrix multiplication steps.  Hopefully
//    your compiler will optimize out the unnecessary
//    intermediate variables.
//
//    If your compiler does not aggressively optimize. It is possible
//    to reorder the operations to reduce the number of temporaries
//    required.
//
//  Parameters:
//    data: The 8x8 matrix to perform the IDCT on.
//    qt: The prescaled quantization table.
//
JpegDecoderDataUnit &JpegDecoderDataUnit::IntegerInverseDCT(JpegDecoderCoefficientBlock data, const JpegDecoderQuantizationTable &qt) {
	unsigned int ii;
	int32 tmp[JpegSampleWidth][JpegSampleWidth];

	for (ii = 0; ii < JpegSampleWidth; ++ii) {
		// This optimization does not seem to be worth the trouble in the
		// second loop.
		if ((data[ii][1] | data[ii][2] | data[ii][3] | data[ii][4] | data[ii][5] | data[ii][6] | data[ii][7]) == 0) {
			tmp[ii][0] = data[ii][0] * qt.integer_scaling[ii][0];
			tmp[ii][1] = tmp[ii][0];
			tmp[ii][2] = tmp[ii][0];
			tmp[ii][3] = tmp[ii][0];
			tmp[ii][4] = tmp[ii][0];
			tmp[ii][5] = tmp[ii][0];
			tmp[ii][6] = tmp[ii][0];
			tmp[ii][7] = tmp[ii][0];
		} else {
			int32 a0 = data[ii][0] * qt.integer_scaling[ii][0];
			int32 a1 = data[ii][4] * qt.integer_scaling[ii][4];
			int32 a2 = data[ii][2] * qt.integer_scaling[ii][2];
			int32 a3 = data[ii][6] * qt.integer_scaling[ii][6];
			int32 a4 = data[ii][1] * qt.integer_scaling[ii][1];
			int32 a5 = data[ii][5] * qt.integer_scaling[ii][5];
			int32 a6 = data[ii][3] * qt.integer_scaling[ii][3];
			int32 a7 = data[ii][7] * qt.integer_scaling[ii][7];

			int32 b2 = a2 - a3;
			int32 b3 = a2 + a3;
			int32 b4 = a4 - a7;
			int32 b5 = a5 + a6;
			int32 b6 = a5 - a6;
			int32 b7 = a4 + a7;

			int32 c4 = (ISEC2 * b4) >> IntegerScale;
			int32 c5 = b7 - b5;
			int32 c6 = (ISEC6 * b6) >> IntegerScale;
			int32 c7 = b5 + b7;

			int32 d4 = c4 + c6;
			int32 d6 = c4 - c6;

			int32 e0 = a0 + a1;
			int32 e1 = a0 - a1;
			int32 e2 = (b2 * IC4) >> IntegerScale;
			int32 e4 = (d4 * IC4) >> IntegerScale;
			int32 e5 = (c5 * IC4) >> IntegerScale;

			int32 f6 = e4 + d6;

			int32 g3 = e2 + b3;
			int32 g5 = e4 + e5;
			int32 g6 = e5 + f6;
			int32 g7 = f6 + c7;

			int32 h0 = e0 + g3;
			int32 h1 = e1 + e2;
			int32 h2 = e1 - e2;
			int32 h3 = e0 - g3;

			tmp[ii][0] = h0 + g7;
			tmp[ii][1] = h1 + g6;
			tmp[ii][2] = h2 + g5;
			tmp[ii][3] = h3 + e4;
			tmp[ii][4] = h3 - e4;
			tmp[ii][5] = h2 - g5;
			tmp[ii][6] = h1 - g6;
			tmp[ii][7] = h0 - g7;
		}
	}

	for (ii = 0; ii < JpegSampleWidth; ++ii) {
		int32 a0 = tmp[0][ii];
		int32 a1 = tmp[4][ii];
		int32 a2 = tmp[2][ii];
		int32 a3 = tmp[6][ii];
		int32 a4 = tmp[1][ii];
		int32 a5 = tmp[5][ii];
		int32 a6 = tmp[3][ii];
		int32 a7 = tmp[7][ii];

		int32 b2 = a2 - a3;
		int32 b3 = a2 + a3;
		int32 b4 = a4 - a7;
		int32 b5 = a5 + a6;
		int32 b6 = a5 - a6;
		int32 b7 = a4 + a7;

		int32 c4 = (ISEC2 * b4) >> IntegerScale;
		int32 c5 = b7 - b5;
		int32 c6 = (ISEC6 * b6) >> IntegerScale;
		int32 c7 = b5 + b7;

		int32 d4 = c4 + c6;
		int32 d6 = c4 - c6;

		int32 e0 = a0 + a1;
		int32 e1 = a0 - a1;
		int32 e2 = (b2 * IC4) >> IntegerScale;
		int32 e4 = (d4 * IC4) >> IntegerScale;
		int32 e5 = (c5 * IC4) >> IntegerScale;

		int32 f6 = e4 + d6;

		const int32 rounding = (JpegMaxSampleValue + 2) << (JpegDecoderQuantizationTable::QuantizationIntegerScale - 1);
		int32 g0 = e0 + rounding;
		int32 g1 = e1 + rounding;
		int32 g3 = e2 + b3;
		int32 g5 = e4 + e5;
		int32 g6 = e5 + f6;
		int32 g7 = f6 + c7;

		int32 h0 = g0 + g3;
		int32 h1 = g1 + e2;
		int32 h2 = g1 - e2;
		int32 h3 = g0 - g3;

		values[0][ii] = SampleRange((h0 + g7) >> JpegDecoderQuantizationTable::QuantizationIntegerScale);
		values[1][ii] = SampleRange((h1 + g6) >> JpegDecoderQuantizationTable::QuantizationIntegerScale);
		values[2][ii] = SampleRange((h2 + g5) >> JpegDecoderQuantizationTable::QuantizationIntegerScale);
		values[3][ii] = SampleRange((h3 + e4) >> JpegDecoderQuantizationTable::QuantizationIntegerScale);
		values[4][ii] = SampleRange((h3 - e4) >> JpegDecoderQuantizationTable::QuantizationIntegerScale);
		values[5][ii] = SampleRange((h2 - g5) >> JpegDecoderQuantizationTable::QuantizationIntegerScale);
		values[6][ii] = SampleRange((h1 - g6) >> JpegDecoderQuantizationTable::QuantizationIntegerScale);
		values[7][ii] = SampleRange((h0 - g7) >> JpegDecoderQuantizationTable::QuantizationIntegerScale);
	}
	return *this;
}

// JPEG Decoder Component Class Implementation
//
//  Description:
//
//    SequentialOnly
//
//    This function extends the sign bit of a decoded value.
//
//  Parameters:
//    vv: The bit value
//    tt: The length of the bit value
//
static inline int Extend(int vv, int t) {
	// Extend function defined in Section F.2.2.1 Figure F.12
	// The tt'th bit of vv is the sign bit. One is for
	// positive values and zero is for negative values.
	int vt = 1 << (t - 1);
	if (vv < vt) {
		vt = (-1 << t) + 1;
		return vv + vt;
	} else {
		return vv;
	}
}

//
//  Description:
//
//    Class default constructor
//
JpegDecoderComponent::JpegDecoderComponent() {
	horizontal_frequency = 0;
	vertical_frequency = 0;
	v_sampling = 0;
	h_sampling = 0;
	last_dc_value = 0;
	ac_table = NULL;
	dc_table = NULL;
	quantization_table = NULL;
	noninterleaved_rows = 0;
	noninterleaved_cols = 0;
	data_units = NULL;
	upsample_data = NULL;
	return;
}

//
//  Description:
//
//    Class Destructor
//
JpegDecoderComponent::~JpegDecoderComponent() {
	delete[] data_units;
	data_units = NULL;
	delete[] upsample_data;
	upsample_data = NULL;
}

//
//  Description:
//
//    This function associates a quantization table with the component.
//
//  Parameters:
//    table:  The quantization table
//
void JpegDecoderComponent::SetQuantizationTable(JpegDecoderQuantizationTable &table) { quantization_table = &table; }

//
//  Description:
//
//    This function determines the dimensions for the component and allocates
//    the storage to hold the component's data.
//
//  Parameters:
//    decoder:  The jpeg decoder this component belongs to.
//
void JpegDecoderComponent::AllocateComponentBuffers(const JpegDecoder &decoder) {
	if (data_units == NULL) {
		// Determine sampling for the component. This is the amount of
		// stretching needed for the component.
		v_sampling = decoder.max_vertical_frequency / vertical_frequency;
		h_sampling = decoder.max_horizontal_frequency / horizontal_frequency;

		// Determine the component's dimensions in a non-interleaved scan.
		noninterleaved_rows = (480 + v_sampling * JpegSampleWidth - 1) / (v_sampling * JpegSampleWidth);
		noninterleaved_cols = (640 + h_sampling * JpegSampleWidth - 1) / (h_sampling * JpegSampleWidth);

		du_rows = decoder.mcu_rows * vertical_frequency;
		du_cols = decoder.mcu_cols * horizontal_frequency;

		data_units = new JpegDecoderDataUnit[du_rows * du_cols];
	}
}

//
//  Description:
//
//    This function frees the memory allocated by the component
//    during the decompression process.
//
void JpegDecoderComponent::FreeComponentBuffers() {
	delete[] data_units;
	data_units = NULL;
	delete[] upsample_data;
	upsample_data = NULL;
}

//
//  Description:
//
//    This function asigned Huffman tables to the component.
//
//  Parameters:
//    dc:  The DC Huffman table
//    ac:  The AC Huffman table
//
void JpegDecoderComponent::SetHuffmanTables(JpegHuffmanDecoder &dc, JpegHuffmanDecoder &ac) {
	dc_table = &dc;
	ac_table = &ac;
}

//
//  Description:
//
//    This function decodes a data unit in a sequential scan.
//
//  Parameters:
//    decoder: The decoder that owns this component
//    mcurow, mcucol:  The row and column for this data unit.
//
void JpegDecoderComponent::DecodeSequential(JpegDecoder &decoder, unsigned int mcurow, unsigned int mcucol) {
	JpegDecoderCoefficientBlock data;
	memset(&data, 0, sizeof(data));

	// Decode the DC differce value.
	// Section F.2.2.1
	unsigned int count; // called T in F.2.2.1
	count = dc_table->Decode(decoder);
	int bits = decoder.Receive(count);
	int diff = Extend(bits, count);

	// Create the DC value from the difference and the previous DC value.
	int dc = diff + last_dc_value;
	last_dc_value = dc;
	data[0][0] = (int16)dc;

	// Decode the AC coefficients.
	// Section F.2.2.2 Figure F.13
	for (unsigned int kk = 1; kk < JpegSampleSize; ++kk) {
		uint16 rs = (uint16)ac_table->Decode(decoder);
		uint16 ssss = (uint16)(rs & 0xF);
		uint16 rrrr = (uint16)(rs >> 0x4);

		if (ssss == 0) {
			// ssss is zero then rrrr should either be 15 or zero according to
			// Figure F.1. 0 means that the rest of the coefficients are zero
			// while 15 means the next 16 coefficients are zero. We are not checking
			// for other values because Figure F.13 shows values other than 15
			// as being treated as zero.
			if (rrrr != 15)
				break;
			kk += 15; // Actually 16 since one more gets added by the loop.
		} else {
			// If ssss is non-zero then rrrr gives the number of zero coefficients
			// to skip.

			kk += rrrr;

			// Receive and extend the additional bits.
			// Section F2.2.2 Figure F.14
			int bit = decoder.Receive(ssss);
			int value = Extend(bit, ssss);
			(&data[0][0])[JpegZigZagInputOrderCodes[kk]] = (int16)value;
		}
	}
	data_units[mcurow * du_cols + mcucol].InverseDCT(data, *quantization_table);
}

//
//  Description:
//
//    This function upsamples the data for the component. Here we take
//    the values from the data_units array and copy it to the
//    upsample_data. If the horizontal or vertical sampling frequencies
//    are less than the maximum for the image then we need to
//    stretch the data during the copy.
//
void JpegDecoderComponent::Upsample() {
	unsigned int imagesize = du_rows * v_sampling * du_cols * h_sampling * JpegSampleSize;
	if (imagesize == 0)
		return; // No data for this component yet.

	if (upsample_data == NULL)
		upsample_data = new uint8[imagesize];

	// Simple case where component does not need to be upsampled.
	if (v_sampling == 1 && h_sampling == 1) {
		unsigned output = 0;
		unsigned int startdu = 0;
		for (unsigned int durow = 0; durow < du_rows; ++durow) {
			for (unsigned int ii = 0; ii < JpegSampleWidth; ++ii) {
				unsigned int du = startdu;
				for (unsigned int ducol = 0; ducol < du_cols; ++ducol) {
					upsample_data[output] = data_units[du].values[ii][0];
					++output;
					upsample_data[output] = data_units[du].values[ii][1];
					++output;
					upsample_data[output] = data_units[du].values[ii][2];
					++output;
					upsample_data[output] = data_units[du].values[ii][3];
					++output;
					upsample_data[output] = data_units[du].values[ii][4];
					++output;
					upsample_data[output] = data_units[du].values[ii][5];
					++output;
					upsample_data[output] = data_units[du].values[ii][6];
					++output;
					upsample_data[output] = data_units[du].values[ii][7];
					++output;
					++du;
				}
			}
			startdu += du_cols;
		}
	} else {
		unsigned output = 0;
		unsigned int startdu = 0;
		for (unsigned int durow = 0; durow < du_rows; ++durow) {
			for (unsigned int ii = 0; ii < JpegSampleWidth; ++ii) {
				for (unsigned int vv = 0; vv < v_sampling; ++vv) {
					unsigned int du = startdu;
					for (unsigned int ducol = 0; ducol < du_cols; ++ducol) {
						unsigned int jj;
						for (jj = 0; jj < h_sampling; ++jj) {
							upsample_data[output] = data_units[du].values[ii][0];
							++output;
						}
						for (jj = 0; jj < h_sampling; ++jj) {
							upsample_data[output] = data_units[du].values[ii][1];
							++output;
						}
						for (jj = 0; jj < h_sampling; ++jj) {
							upsample_data[output] = data_units[du].values[ii][2];
							++output;
						}
						for (jj = 0; jj < h_sampling; ++jj) {
							upsample_data[output] = data_units[du].values[ii][3];
							++output;
						}
						for (jj = 0; jj < h_sampling; ++jj) {
							upsample_data[output] = data_units[du].values[ii][4];
							++output;
						}
						for (jj = 0; jj < h_sampling; ++jj) {
							upsample_data[output] = data_units[du].values[ii][5];
							++output;
						}
						for (jj = 0; jj < h_sampling; ++jj) {
							upsample_data[output] = data_units[du].values[ii][6];
							++output;
						}
						for (jj = 0; jj < h_sampling; ++jj) {
							upsample_data[output] = data_units[du].values[ii][7];
							++output;
						}
						++du;
					}
				}
			}
			startdu += du_cols;
		}
	}
	return;
}

//
//  Description:
//
//    This static member function converts the upsample_data in three
//    components from YCbCr to RGB and writes it to an image.
//
//  Parameters:
//    c1: The component containing the Y data
//    c2: Ditto for Cb
//    c3: Ditto for Cr
//    image: The output image
//
void JpegDecoderComponent::RGBConvert(JpegDecoderComponent &c1, JpegDecoderComponent &c2, JpegDecoderComponent &c3, uint32 surfaceId) {
	uint8 *pDst8 = surface_manager->Lock_surface(surfaceId);
	uint32 pitch = surface_manager->Get_pitch(surfaceId);
	uint32 bpp = surface_manager->Get_BytesPP(surfaceId);

	unsigned int rowstart = 0;
	for (unsigned int ii = 480; ii; ii--) {
		unsigned int offset = rowstart;
		uint8 *outrow = pDst8;
		pDst8 += pitch;

		for (unsigned int jj = 0; jj < (uint32)(bpp * 640); jj += bpp) {
			YCbCr_To_RGB(c1.upsample_data[offset], c2.upsample_data[offset], c3.upsample_data[offset], outrow[jj + 2], outrow[jj + 1], outrow[jj + 0]);
			++offset;
		}
		rowstart += c1.du_cols * c1.h_sampling * JpegSampleWidth;
	}

	surface_manager->Unlock_surface(surfaceId);
}

} // End of namespace ICB
