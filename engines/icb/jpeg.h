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

#ifndef ICB_JPEG_H
#define ICB_JPEG_H

#include "engines/icb/common/px_common.h"

namespace ICB {

class _surface;

static uint32 _bit_count;
static uint32 *_buf_pointer;
static uint32 *_end_buf;

#ifdef _WIN32
static __int64 safeMM0 = 0;
#else
static __int64_t safeMM0 = 0;
#endif

const int JpegMaxHuffmanTables = 4;
const int MaxQuantizationTables = 4;
const int JpegMaxComponentsPerFrame = 255;
const int JpegMaxComponentsPerScan = 4;
const int JpegMinSamplingFrequency = 1;
const int JpegMaxSamplingFrequency = 4;
const int JpegSampleWidth = 8;
const int JpegSampleSize = JpegSampleWidth * JpegSampleWidth;
const int JpegMinSampleValue = 0;
const int JpegMaxSampleValue = 255;      // For 12-Bits this would be 4095
const int JpegMidpointSampleValue = 128; // For 12-Bits this 2048
const int JpegMaxDataUnitsPerMCU = 10;
const int JpegMaxSuccessiveApproximation = 13;
const int JpegMax8BitQuantizationValue = 255;
const int JpegMinQuantizationValue = 1;
const unsigned int JpegMaxHuffmanCodeLength = 16;
const unsigned int JpegMaxNumberOfHuffmanCodes = 256;
extern const unsigned int JpegZigZagInputOrderCodes[JpegSampleSize];
extern const unsigned int JpegZigZagOutputOrderCodes[JpegSampleSize];
typedef int16 JpegDecoderCoefficientBlock[JpegSampleWidth][JpegSampleWidth];

// These definitions do not include the preceding 0xFF byte.
enum JpegMarkers {
	// Start of Frame Markers, Non-Differential Huffman Coding
	SOF0 = 0xC0, // Baseline DCT
	SOF1 = 0xC1, // Sequential DCT
	SOF2 = 0xC2, // Progressive DCT
	SOF3 = 0xC3, // Spatial (sequential) lossless
	// Start of Frame Markers, Differential Huffman Coding
	SOF5 = 0xC5, // Differential Sequential DCT
	SOF6 = 0xC6, // Differential Progressive DCT
	SOF7 = 0xC7, // Differential Spatial
	// Start of Frame Markers, Non-Differential Arithmetic Coding
	SOF9 = 0xC9, // Extended Sequential DCT
	SOFA = 0xCA, // Progressive DCT
	SOFB = 0xCB, // Spacial (sequential) Lossless
	// Start of Frame Markers, Differential Arithmetic Coding
	SOFD = 0xCD, // Differential Sequential DCT
	SOFE = 0xCE, // Differential Progressive DCT
	SOFF = 0xCF, // Differential Spatial
	// Other Markers
	DHT = 0xC4,  // Define Huffman Tables
	DAC = 0xCC,  // Define Arithmetic Coding Conditions
	RST0 = 0xD0, // Restart Marker
	RST1 = 0xD1, // Restart Marker
	RST2 = 0xD2, // Restart Marker
	RST3 = 0xD3, // Restart Marker
	RST4 = 0xD4, // Restart Marker
	RST5 = 0xD5, // Restart Marker
	RST6 = 0xD6, // Restart Marker
	RST7 = 0xD7, // Restart Marker
	SOI = 0xD8,  // Start of Image
	EOI = 0xD9,  // End of Image
	SOS = 0xDA,  // Start of Scan
	DQT = 0xDB,  // Define Quantization Table
	DNL = 0xDC,  // Define Number of Lines
	DRI = 0xDD,  // Define Restart Intervale
	DHP = 0xDE,  // Define Hierarchical Progression
	EXP = 0xDF,  // Expand Reference Components
	APP0 = 0xE0, // Application Segments
	APP1 = 0xE1, // Application Segments
	APP2 = 0xE2, // Application Segments
	APP3 = 0xE3, // Application Segments
	APP4 = 0xE4, // Application Segments
	APP5 = 0xE5, // Application Segments
	APP6 = 0xE6, // Application Segments
	APP7 = 0xE7, // Application Segments
	APP8 = 0xE8, // Application Segments
	APP9 = 0xE9, // Application Segments
	APPA = 0xEA, // Application Segments
	APPB = 0xEB, // Application Segments
	APPC = 0xEC, // Application Segments
	APPD = 0xED, // Application Segments
	APPE = 0xEE, // Application Segments
	APPF = 0xEF, // Application Segments
	// C8, F0-FD, 01, 02-BF reserved
	COM = 0xFE, // Comment
	SOB = 0xFF  // Start of Block - Byte that precedes all others - not in the standard.
};

// To implement 12-bit data the return values would have to be
// at least 12 bits wide.

// Functions for YCbCr/RGB colorspace conversion
#define JPEGSAMPLE_SCALEFACTOR 12
#define JPEGSAMPLE_ROUNDING 2048
#define JPEGSAMPLE_RED_CONST 5743
#define JPEGSAMPLE_GREEN_CONST1 1410
#define JPEGSAMPLE_GREEN_CONST2 2925
#define JPEGSAMPLE_BLUE_CONST 7258

inline void YCbCr_To_RGB(int yy, int cb, int cr, uint8 &r, uint8 &g, uint8 &b) {
	int mcr = cr - JpegMidpointSampleValue;
	int mcb = cb - JpegMidpointSampleValue;

	int a = yy + ((JPEGSAMPLE_RED_CONST * mcr + JPEGSAMPLE_ROUNDING) >> JPEGSAMPLE_SCALEFACTOR);
	r = (uint8)((a < 0) ? 0 : (a > 255) ? 255 : a);

	a = yy - ((JPEGSAMPLE_GREEN_CONST1 * mcb + JPEGSAMPLE_GREEN_CONST2 * mcr + JPEGSAMPLE_ROUNDING) >> JPEGSAMPLE_SCALEFACTOR);
	g = (uint8)((a < 0) ? 0 : (a > 255) ? 255 : a);

	a = yy + ((JPEGSAMPLE_BLUE_CONST * mcb + JPEGSAMPLE_ROUNDING) >> JPEGSAMPLE_SCALEFACTOR);
	b = (uint8)((a < 0) ? 0 : (a > 255) ? 255 : a);
}

// JPEG Decoder Class Implementation - JPEG Decoder Class Implementation -
class JpegDecoderComponent;
class JpegHuffmanDecoder;
class JpegDecoderQuantizationTable;

class JpegDecoder {
public:
	JpegDecoder();
	virtual ~JpegDecoder();

	virtual void ReadImage(unsigned char *inputData, uint32 surface_Id);
	virtual void UpdateImage();

	int cGetBit();

	inline uint8 ReadByte() {
		uint8 value = input_buffer[iPos];
		iPos += sizeof(uint8);
		bit_position = 0;
		return value;
	}

	inline uint16 ReadWord() {
		bit_position = 0;
		uint16 value = *((uint16 *)(&input_buffer[iPos]));
		iPos += sizeof(uint16);
		return value;
	}

	int NextBit();
	int Receive(unsigned int count);

	void Initialize();
	void ReadStreamHeader();
	void ReadMarker();
	void ReadApplication(uint8 type);
	void ReadHuffmanTable();
	void ReadQuantization();
	void ReadStartOfFrame(uint8 type);
	void ReadStartOfScan();
	void CalculateMcuDimensions();
	void FreeAllocatedResources();
	void ReadSequentialNonInterleavedScan();
	void ResetDcDifferences();
	void RefineAcCoefficient(int16 &value, unsigned int ssa);

	// Huffman tables
	JpegHuffmanDecoder *ac_tables;
	JpegHuffmanDecoder *dc_tables;

	// Quantization tables
	JpegDecoderQuantizationTable *quantization_tables;

	// Bit I/O state
	int bit_position;       // Called CNT in Section F.2.2.5
	unsigned char bit_data; // Called B in Section F.2.2.5

	bool eoi_found;
	bool sof_found;

	unsigned char *input_buffer;
	unsigned int iPos;
	unsigned int surfaceId;

	unsigned int frame_type;

	unsigned max_horizontal_frequency;
	unsigned max_vertical_frequency;

	unsigned int component_count;
	JpegDecoderComponent *components;
	unsigned int *component_indices;

	// Progress Counters
	unsigned int current_scan;
	unsigned int scan_count;

	unsigned int mcu_rows;
	unsigned int mcu_cols;

	unsigned int mcu_height;
	unsigned int mcu_width;

	unsigned int scan_component_count;
	JpegDecoderComponent **scan_components;

	friend class JpegDecoderQuantizationTable;
	friend class JpegHuffmanDecoder;
	friend class JpegDecoderComponent;
};

//  JPEG Decoder Huffman Table Class Implementation
//
//    This class represents a Huffman Table used by the JpegDecoder
//    class.
//
class JpegHuffmanDecoder {
public:
	JpegHuffmanDecoder();
	virtual ~JpegHuffmanDecoder() {}

	// This function reads a Huffman table from the input stream.
	unsigned int ReadTable(JpegDecoder &);

	// Function to decode the next value in the input stream.
	int Decode(JpegDecoder &);

	// This function builds the structures needed for Huffman
	// decoding after the table data has been read.
	void MakeTable(uint8 huffbits[JpegMaxHuffmanCodeLength]);

	// Maximum Huffman code value of length N
	int maxcode[JpegMaxHuffmanCodeLength];
	// Minimum Huffman code value of length N
	int mincode[JpegMaxHuffmanCodeLength];
	// Index into "values" for minimum code of length N
	uint8 valptr[JpegMaxHuffmanCodeLength];
	// Huffman values
	uint8 huff_values[JpegMaxNumberOfHuffmanCodes];
};

//  Title:  JPEG Decoder Quantization Table Class Implementation
class JpegDecoderQuantizationTable {
public:
	JpegDecoderQuantizationTable();
	~JpegDecoderQuantizationTable() {}

	// Function to read the quantization table from the input stream.
	void ReadTable(JpegDecoder &decoder, unsigned int precision);

	// This function builds the scaled quantization tables used in
	// fast IDCT implementations.
	void BuildScaledTables();

	// Quantization Values in Zig-Zag Order.
	uint16 data_values[JpegSampleSize];

	// Scaling factor used for the scaled integer values.
	enum { QuantizationIntegerScale = 12 };

	// Scaled quantization values used for the fast IDCT implementations.
	double float_scaling[JpegSampleWidth][JpegSampleWidth];
	int32 integer_scaling[JpegSampleWidth][JpegSampleWidth];
};

//  Title:  JPEG Definitions and Utility Functions
//
//  Decoder Data Unit Class Definition
//
//  Descrition:
//
//    The DataUnit class represents an 8x8 sample block for one
//    component of the JPEG image.
//
//
class JpegDecoderDataUnit {
      public:
	// Declaration of a type for pointers to member functions
	// for implementing the IDCT. The input parameters are
	// The IDCT coefficients and the [de]quantization table.
	typedef JpegDecoderDataUnit &(JpegDecoderDataUnit::*IDctFunction)(JpegDecoderCoefficientBlock, const JpegDecoderQuantizationTable &);

	JpegDecoderDataUnit() {}
	virtual ~JpegDecoderDataUnit() {}

	// General IDCT Function
	JpegDecoderDataUnit &InverseDCT(JpegDecoderCoefficientBlock cb, const JpegDecoderQuantizationTable &qt);

	// These are the IDCT implementations.
	JpegDecoderDataUnit &FloatInverseDCT(JpegDecoderCoefficientBlock, const JpegDecoderQuantizationTable &);
	JpegDecoderDataUnit &IntegerInverseDCT(JpegDecoderCoefficientBlock, const JpegDecoderQuantizationTable &);

	// Operators to retrieve the individual IDCT values.
	// uint8 const* operator [] (unsigned int ii) const;

	// The IDCT values.
	uint8 values[JpegSampleWidth][JpegSampleWidth];

	// This is a pointer to the member function that implements
	// the desired IDCT function.
	static IDctFunction idct_function;
};

inline JpegDecoderDataUnit &JpegDecoderDataUnit::InverseDCT(JpegDecoderCoefficientBlock cb, const JpegDecoderQuantizationTable &qt) { return (this->*idct_function)(cb, qt); }

//
//  Description:
//
//    The IDCT process can produce rounding errors that result in sample
//    values being outside the legal range.  This function clamps
//    sample value into the legal range.
//
//    Unclamped values give strange results when converted to bytes.
//     -1 (0xFFFFFFFF) would be converted to 255 (0xFF) instead of 0 and
//    256 (0x00000100) gets converted to 0.
//
//  Parameters:
//    value: The value to clamp
//
//  Return Value:
//    "value" clamped to MinSampleValue..MaxSampleValue
//
static inline uint8 SampleRange(int32 value) {
	if (value < JpegMinSampleValue)
		return (uint8)JpegMinSampleValue;
	else if (value > JpegMaxSampleValue)
		return (uint8)JpegMaxSampleValue;
	else
		return (uint8)value;
}

//  Title:  JPEG Definitions and Utility Functions
//
//    This class represents a component within the JPEG decoder.
//
class JpegDecoderComponent {
public:
	JpegDecoderComponent();
	~JpegDecoderComponent();

	// We have made the color conversions static because RGB
	// conversion requires the data from three components.
	// Grayscale conversion is static strictly for consistency
	// with RGB.
	static void RGBConvert(JpegDecoderComponent &c1, JpegDecoderComponent &c2, JpegDecoderComponent &c3, uint32 surfaceId);

	void SetQuantizationTable(JpegDecoderQuantizationTable &table);
	void AllocateComponentBuffers(const JpegDecoder &decoder);
	void FreeComponentBuffers();
	void SetHuffmanTables(JpegHuffmanDecoder &dc, JpegHuffmanDecoder &ac);
	void Upsample();

	void DecodeSequential(JpegDecoder &decoder, unsigned int mcurow, unsigned int mcucol);

	void ProgressiveInverseDct();

	// Sampling Frequencies
	unsigned int horizontal_frequency;
	unsigned int vertical_frequency;

	// These values are the numnber of samples to take for each data
	// point. They come from the sampling frequencies and the maximum
	// sampling frequencies of all the components in the image.
	// sampling frequencies of all the components in the image.
	unsigned int v_sampling;
	unsigned int h_sampling;

	// Last encoded DC value.
	int last_dc_value;

	// Entropy tables used by the component.
	JpegHuffmanDecoder *ac_table;
	JpegHuffmanDecoder *dc_table;

	// Quantization table used by the component
	JpegDecoderQuantizationTable *quantization_table;

	// Non-interleaved dimensions.
	unsigned int noninterleaved_rows;
	unsigned int noninterleaved_cols;

	unsigned int du_rows;
	unsigned int du_cols;

	JpegDecoderDataUnit *data_units;
	uint8 *upsample_data;
};

} // End of namespace ICB

#endif
