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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/bs_stdint.h"
#include "sword25/gfx/image/vectorimage.h"

#include "graphics/colormasks.h"

namespace Sword25 {

#define BS_LOG_PREFIX "VECTORIMAGE"


// -----------------------------------------------------------------------------
// SWF Datentypen
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Bitstream Hilfsklasse
// -----------------------------------------------------------------------------
// Das Parsen von SWF-Dateien erfordert sowohl bitweises Auslesen als auch an
// Bytegrenzen ausgerichtetes Lesen.
// Diese Klasse ist speziell dafür ausgestattet.
// -----------------------------------------------------------------------------

class VectorImage::SWFBitStream {
public:
	SWFBitStream(const unsigned char *pData, unsigned int DataSize) :
		m_Pos(pData), m_End(pData + DataSize), m_WordMask(0)
	{}

	inline uint32 GetBits(unsigned int BitCount) {
		if (BitCount == 0 || BitCount > 32) {
			error("SWFBitStream::GetBits() must read at least 1 and at most 32 bits at a time");
		}

		uint32 value = 0;
		while (BitCount) {
			if (m_WordMask == 0) FlushByte();

			value <<= 1;
			value |= ((m_Word & m_WordMask) != 0) ? 1 : 0;
			m_WordMask >>= 1;

			--BitCount;
		}

		return value;
	}

	inline int32 GetSignedBits(unsigned int BitCount) {
		// Bits einlesen
		uint32 Temp = GetBits(BitCount);

		// Falls das Sign-Bit gesetzt ist, den Rest des Rückgabewertes mit 1-Bits auffüllen (Sign Extension)
		if (Temp & 1 << (BitCount - 1))
			return (0xffffffff << BitCount) | Temp;
		else
			return Temp;
	}

	inline uint32 GetUInt32() {
		uint32 Byte1 = GetByte();
		uint32 Byte2 = GetByte();
		uint32 Byte3 = GetByte();
		uint32 Byte4 = GetByte();

		return Byte1 | (Byte2 << 8) | (Byte3 << 16) | (Byte4 << 24);
	}

	inline uint16 GetUInt16() {
		uint32 Byte1 = GetByte();
		uint32 Byte2 = GetByte();

		return Byte1 | (Byte2 << 8);
	}

	inline byte GetByte() {
		FlushByte();
		byte Value = m_Word;
		m_WordMask = 0;
		FlushByte();

		return Value;
	}

	inline void FlushByte() {
		if (m_WordMask != 128) {
			if (m_Pos >= m_End) {
				error("Attempted to read past end of file");
			} else {
				m_Word = *m_Pos++;
				m_WordMask = 128;
			}
		}
	}

	inline void SkipBytes(unsigned int SkipLength) {
		FlushByte();
		if (m_Pos + SkipLength >= m_End) {
			error("Attempted to read past end of file");
		} else {
			m_Pos += SkipLength;
			m_Word = *(m_Pos - 1);
		}
	}

private:
	const unsigned char    *m_Pos;
	const unsigned char    *m_End;

	byte                      m_Word;
	unsigned int            m_WordMask;
};


// -----------------------------------------------------------------------------
// Konstanten und Hilfsfunktionen
// -----------------------------------------------------------------------------

namespace {
// -----------------------------------------------------------------------------
// Konstanten
// -----------------------------------------------------------------------------

const uint32 MAX_ACCEPTED_FLASH_VERSION = 3;   // Die höchste Flash-Dateiversion, die vom Lader akzeptiert wird


// -----------------------------------------------------------------------------
// Konvertiert SWF-Rechteckdaten in einem Bitstrom in Common::Rect-Objekte
// -----------------------------------------------------------------------------

Common::Rect FlashRectToBSRect(VectorImage::SWFBitStream &bs) {
	bs.FlushByte();

	// Feststellen mit wie vielen Bits die einzelnen Komponenten kodiert sind
	uint32 BitsPerValue = bs.GetBits(5);

	// Die einzelnen Komponenten einlesen
	int32 XMin = bs.GetSignedBits(BitsPerValue);
	int32 XMax = bs.GetSignedBits(BitsPerValue);
	int32 YMin = bs.GetSignedBits(BitsPerValue);
	int32 YMax = bs.GetSignedBits(BitsPerValue);

	return Common::Rect(XMin, YMin, XMax + 1, YMax + 1);
}


// -----------------------------------------------------------------------------
// Konvertiert SWF-Farben in AntiGrain Farben
// -----------------------------------------------------------------------------

uint32 FlashColorToAGGRGBA8(unsigned int FlashColor) {
	uint32 ResultColor = Graphics::ARGBToColor<Graphics::ColorMasks<8888> >(FlashColor >> 24, (FlashColor >> 16) & 0xff, (FlashColor >> 8) & 0xff, FlashColor & 0xff);

	return ResultColor;
}


// -----------------------------------------------------------------------------
// Berechnet die Bounding-Box eines BS_VectorImageElement
// -----------------------------------------------------------------------------

struct CBBGetId {
	CBBGetId(const VectorImageElement &vectorImageElement_) : vectorImageElement(vectorImageElement_) {}
	unsigned operator [](unsigned i) const {
		return vectorImageElement.GetPathInfo(i).GetID();
	}
	const VectorImageElement &vectorImageElement;
};

Common::Rect CalculateBoundingBox(const VectorImageElement &vectorImageElement) {
#if 0 // TODO
	agg::path_storage Path = vectorImageElement.GetPaths();
	CBBGetId IdSource(vectorImageElement);

	double x1, x2, y1, y2;
	agg::bounding_rect(Path, IdSource, 0, vectorImageElement.GetPathCount(), &x1, &y1, &x2, &y2);
#else
	double x1, x2, y1, y2;
	x1 = x2 = y1 = y2 = 0;
#endif
	return Common::Rect(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2) + 1, static_cast<int>(y2) + 1);
}
}


// -----------------------------------------------------------------------------
// Konstruktion
// -----------------------------------------------------------------------------

VectorImage::VectorImage(const unsigned char *pFileData, unsigned int FileSize, bool &Success) {
	Success = false;

	// Bitstream-Objekt erzeugen
	// Im Folgenden werden die Dateidaten aus diesem ausgelesen.
	SWFBitStream bs(pFileData, FileSize);

	// SWF-Signatur überprüfen
	uint32 Signature[3];
	Signature[0] = bs.GetByte();
	Signature[1] = bs.GetByte();
	Signature[2] = bs.GetByte();
	if (Signature[0] != 'F' ||
		Signature[1] != 'W' ||
		Signature[2] != 'S') {
		BS_LOG_ERRORLN("File is not a valid SWF-file");
		return;
	}

	// Versionsangabe überprüfen
	uint32 Version = bs.GetByte();
	if (Version > MAX_ACCEPTED_FLASH_VERSION) {
		BS_LOG_ERRORLN("File is of version %d. Highest accepted version is %d.", Version, MAX_ACCEPTED_FLASH_VERSION);
		return;
	}

	// Dateigröße auslesen und mit der tatsächlichen Größe vergleichen
	uint32 StoredFileSize = bs.GetUInt32();
	if (StoredFileSize != FileSize) {
		BS_LOG_ERRORLN("File is not a valid SWF-file");
		return;
	}

	// SWF-Maße auslesen
	Common::Rect MovieRect = FlashRectToBSRect(bs);

	// Framerate und Frameanzahl auslesen
	/* uint32 FrameRate = */bs.GetUInt16();
	/* uint32 FrameCount = */bs.GetUInt16();

	// Tags parsen
	// Da wir uns nur für das erste DefineShape-Tag interessieren
	bool KeepParsing = true;
	while (KeepParsing) {
		// Tags beginnen immer an Bytegrenzen
		bs.FlushByte();

		// Tagtyp und Länge auslesen
		uint16 TagTypeAndLength = bs.GetUInt16();
		uint32 TagType = TagTypeAndLength >> 6;
		uint32 TagLength = TagTypeAndLength & 0x3f;
		if (TagLength == 0x3f)
			TagLength = bs.GetUInt32();

		switch (TagType) {
		case 2:
			// DefineShape
			Success = ParseDefineShape(2, bs);
			return;
		case 22:
			// DefineShape2
			Success = ParseDefineShape(2, bs);
			return;
		case 32:
			Success = ParseDefineShape(3, bs);
			return;
		default:
			// Unbekannte Tags ignorieren
			bs.SkipBytes(TagLength);
		}
	}

	// Die Ausführung darf nicht an dieser Stelle ankommen: Entweder es wird ein Shape gefunden, dann wird die Funktion mit vorher verlassen, oder
	// es wird keines gefunden, dann tritt eine Exception auf sobald über das Ende der Datei hinaus gelesen wird.
	BS_ASSERT(false);
}

// -----------------------------------------------------------------------------

bool VectorImage::ParseDefineShape(unsigned int ShapeType, SWFBitStream &bs) {
	/*uint32 ShapeID = */bs.GetUInt16();

	// Bounding Box auslesen
	m_BoundingBox = FlashRectToBSRect(bs);

	// Erstes Image-Element erzeugen
	m_Elements.resize(1);

	// Styles einlesen
	unsigned int NumFillBits;
	unsigned int NumLineBits;
	if (!ParseStyles(ShapeType, bs, NumFillBits, NumLineBits))
		return false;

	unsigned int LineStyle = 0;
	unsigned int FillStyle0 = 0;
	unsigned int FillStyle1 = 0;

	// Shaperecord parsen
	// ------------------

	bool EndOfShapeDiscovered = false;
	while (!EndOfShapeDiscovered) {
		uint32 TypeFlag = bs.GetBits(1);

		// Non-Edge Record
		if (TypeFlag == 0) {
			// Feststellen welche Parameter gesetzt werden
			uint32 StateNewStyles = bs.GetBits(1);
			uint32 StateLineStyle = bs.GetBits(1);
			uint32 StateFillStyle1 = bs.GetBits(1);
			uint32 StateFillStyle0 = bs.GetBits(1);
			uint32 StateMoveTo = bs.GetBits(1);

			// End der Shape-Definition erreicht?
			if (!StateNewStyles && !StateLineStyle && !StateFillStyle0 && !StateFillStyle1 && !StateMoveTo)
				EndOfShapeDiscovered = true;
			// Parameter dekodieren
			else {
				int32 MoveDeltaX = 0;
				int32 MoveDeltaY = 0;
				if (StateMoveTo) {
					uint32 MoveToBits = bs.GetBits(5);
					MoveDeltaX = bs.GetSignedBits(MoveToBits);
					MoveDeltaY = bs.GetSignedBits(MoveToBits);
				}

				if (StateFillStyle0) {
					if (NumFillBits > 0)
						FillStyle0 = bs.GetBits(NumFillBits);
					else
						FillStyle0 = 0;
				}

				if (StateFillStyle1) {
					if (NumFillBits > 0)
						FillStyle1 = bs.GetBits(NumFillBits);
					else
						FillStyle1 = 0;
				}

				if (StateLineStyle) {
					if (NumLineBits)
						LineStyle = bs.GetBits(NumLineBits);
					else
						NumLineBits = 0;
				}

				if (StateNewStyles) {
					// An dieser Stelle werden in Flash die alten Style-Definitionen verworfen und mit den neuen überschrieben.
					// Es wird ein neues Element begonnen.
					m_Elements.resize(m_Elements.size() + 1);
					if (!ParseStyles(ShapeType, bs, NumFillBits, NumLineBits)) return false;
				}

				// Ein neuen Pfad erzeugen, es sei denn, es wurden nur neue Styles definiert
				if (StateLineStyle || StateFillStyle0 || StateFillStyle1 || StateMoveTo) {
					// Letzte Zeichenposition merken, beim Aufruf von start_new_path() wird die Zeichenpostionen auf 0, 0 zurückgesetzt
#if 0 // TODO
					double LastX = m_Elements.back().m_Paths.last_x();
					double LastY = m_Elements.back().m_Paths.last_y();

					// Neue Pfadinformation erzeugen
					m_Elements.back().m_PathInfos.push_back(BS_VectorPathInfo(m_Elements.back().m_Paths.start_new_path(), LineStyle, FillStyle0, FillStyle1));

					// Falls eine Bewegung definiert wurde, wird die Zeichenpositionen an die entsprechende Position gesetzt.
					// Ansonsten wird die Zeichenposition auf die letzte Zeichenposition gesetzt.
					if (StateMoveTo)
						m_Elements.back().m_Paths.move_to(MoveDeltaX, MoveDeltaY);
					else
						m_Elements.back().m_Paths.move_to(LastX, LastY);
#endif
				}
			}
		} else {
			// Edge Record
			uint32 EdgeFlag = bs.GetBits(1);
			uint32 NumBits = bs.GetBits(4) + 2;

			// Curved edge
			if (EdgeFlag == 0) {
				/* int32 ControlDeltaX = */bs.GetSignedBits(NumBits);
				/* int32 ControlDeltaY = */bs.GetSignedBits(NumBits);
				/* int32 AnchorDeltaX = */bs.GetSignedBits(NumBits);
				/* int32 AnchorDeltaY = */bs.GetSignedBits(NumBits);

#if 0 // TODO
				double ControlX = m_Elements.back().m_Paths.last_x() + ControlDeltaX;
				double ControlY = m_Elements.back().m_Paths.last_y() + ControlDeltaY;
				double AnchorX = ControlX + AnchorDeltaX;
				double AnchorY = ControlY + AnchorDeltaY;
				m_Elements.back().m_Paths.curve3(ControlX, ControlY, AnchorX, AnchorY);
#endif
			} else {
				// Staight edge
				int32 DeltaX = 0;
				int32 DeltaY = 0;

				uint32 GeneralLineFlag = bs.GetBits(1);
				if (GeneralLineFlag) {
					DeltaX = bs.GetSignedBits(NumBits);
					DeltaY = bs.GetSignedBits(NumBits);
				} else {
					uint32 VertLineFlag = bs.GetBits(1);
					if (VertLineFlag)
						DeltaY = bs.GetSignedBits(NumBits);
					else
						DeltaX = bs.GetSignedBits(NumBits);
				}

#if 0 // TODO
				m_Elements.back().m_Paths.line_rel(DeltaX, DeltaY);
#endif
			}
		}
	}

	// Bounding-Boxes der einzelnen Elemente berechnen
	Common::Array<VectorImageElement>::iterator it = m_Elements.begin();
	for (; it != m_Elements.end(); ++it) it->m_BoundingBox = CalculateBoundingBox(*it);

	return true;
}


// -----------------------------------------------------------------------------

bool VectorImage::ParseStyles(unsigned int ShapeType, SWFBitStream &bs, unsigned int &NumFillBits, unsigned int &NumLineBits) {
	bs.FlushByte();

	// Fillstyles parsen
	// -----------------

	// Anzahl an Fillstyles bestimmen
	unsigned int FillStyleCount = bs.GetByte();
	if (FillStyleCount == 0xff) FillStyleCount = bs.GetUInt16();

	// Alle Fillstyles einlesen, falls ein Fillstyle mit Typ != 0 gefunden wird, wird das Parsen abgebrochen.
	// Es wird nur "solid fill" (Typ 0) unterstützt.
	m_Elements.back().m_FillStyles.reserve(FillStyleCount);
	for (unsigned int i = 0; i < FillStyleCount; ++i) {
		byte Type = bs.GetByte();
		uint32 Color;
		if (ShapeType == 3) {
			Color = (bs.GetByte() << 16) | (bs.GetByte() << 8) | bs.GetByte() | (bs.GetByte() << 24);
		} else
			Color = bs.GetBits(24) | (0xff << 24);
		if (Type != 0) return false;

		m_Elements.back().m_FillStyles.push_back(FlashColorToAGGRGBA8(Color));
	}

	// Linestyles parsen
	// -----------------

	// Anzahl an Linestyles bestimmen
	unsigned int LineStyleCount = bs.GetByte();
	if (LineStyleCount == 0xff)
		LineStyleCount = bs.GetUInt16();

	// Alle Linestyles einlesen
	m_Elements.back().m_LineStyles.reserve(LineStyleCount);
	for (unsigned int i = 0; i < LineStyleCount; ++i) {
		double Width = bs.GetUInt16();
		uint32 Color;
		if (ShapeType == 3)
			Color = (bs.GetByte() << 16) | (bs.GetByte() << 8) | bs.GetByte() | (bs.GetByte() << 24);
		else
			Color = bs.GetBits(24) | (0xff << 24);

		m_Elements.back().m_LineStyles.push_back(VectorImageElement::LineStyleType(Width, FlashColorToAGGRGBA8(Color)));
	}

	// Bitbreite für die folgenden Styleindizes auslesen
	NumFillBits = bs.GetBits(4);
	NumLineBits = bs.GetBits(4);

	return true;
}


// -----------------------------------------------------------------------------

bool VectorImage::Fill(const Common::Rect *pFillRect, unsigned int Color) {
	BS_LOG_ERRORLN("Fill() is not supported.");
	return false;
}


// -----------------------------------------------------------------------------

unsigned int VectorImage::GetPixel(int X, int Y) {
	BS_LOG_ERRORLN("GetPixel() is not supported. Returning black.");
	return 0;
}

// -----------------------------------------------------------------------------

bool VectorImage::SetContent(const byte *Pixeldata, uint size, unsigned int Offset, unsigned int Stride) {
	BS_LOG_ERRORLN("SetContent() is not supported.");
	return 0;
}

} // End of namespace Sword25
