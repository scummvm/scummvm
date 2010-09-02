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
	SWFBitStream(const byte *pData, uint dataSize) :
		m_Pos(pData), m_End(pData + dataSize), m_WordMask(0)
	{}

	inline uint32 getBits(uint bitCount) {
		if (bitCount == 0 || bitCount > 32) {
			error("SWFBitStream::GetBits() must read at least 1 and at most 32 bits at a time");
		}

		uint32 value = 0;
		while (bitCount) {
			if (m_WordMask == 0)
				flushByte();

			value <<= 1;
			value |= ((m_Word & m_WordMask) != 0) ? 1 : 0;
			m_WordMask >>= 1;

			--bitCount;
		}

		return value;
	}

	inline int32 getSignedBits(uint bitCount) {
		// Bits einlesen
		uint32 temp = getBits(bitCount);

		// Falls das Sign-Bit gesetzt ist, den Rest des Rückgabewertes mit 1-Bits auffüllen (Sign Extension)
		if (temp & 1 << (bitCount - 1))
			return (0xffffffff << bitCount) | temp;
		else
			return temp;
	}

	inline uint32 getUInt32() {
		uint32 byte1 = getByte();
		uint32 byte2 = getByte();
		uint32 byte3 = getByte();
		uint32 byte4 = getByte();

		return byte1 | (byte2 << 8) | (byte3 << 16) | (byte4 << 24);
	}

	inline uint16 getUInt16() {
		uint32 byte1 = getByte();
		uint32 byte2 = getByte();

		return byte1 | (byte2 << 8);
	}

	inline byte getByte() {
		flushByte();
		byte value = m_Word;
		m_WordMask = 0;
		flushByte();

		return value;
	}

	inline void flushByte() {
		if (m_WordMask != 128) {
			if (m_Pos >= m_End) {
				error("Attempted to read past end of file");
			} else {
				m_Word = *m_Pos++;
				m_WordMask = 128;
			}
		}
	}

	inline void skipBytes(uint skipLength) {
		flushByte();
		if (m_Pos + skipLength >= m_End) {
			error("Attempted to read past end of file");
		} else {
			m_Pos += skipLength;
			m_Word = *(m_Pos - 1);
		}
	}

private:
	const byte    *m_Pos;
	const byte    *m_End;

	byte m_Word;
	uint m_WordMask;
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

Common::Rect flashRectToBSRect(VectorImage::SWFBitStream &bs) {
	bs.flushByte();

	// Feststellen mit wie vielen Bits die einzelnen Komponenten kodiert sind
	uint32 bitsPerValue = bs.getBits(5);

	// Die einzelnen Komponenten einlesen
	int32 xMin = bs.getSignedBits(bitsPerValue);
	int32 xMax = bs.getSignedBits(bitsPerValue);
	int32 yMin = bs.getSignedBits(bitsPerValue);
	int32 yMax = bs.getSignedBits(bitsPerValue);

	return Common::Rect(xMin, yMin, xMax + 1, yMax + 1);
}


// -----------------------------------------------------------------------------
// Konvertiert SWF-Farben in AntiGrain Farben
// -----------------------------------------------------------------------------

uint32 flashColorToAGGRGBA8(uint flashColor) {
	uint32 resultColor = Graphics::ARGBToColor<Graphics::ColorMasks<8888> >(flashColor >> 24, (flashColor >> 16) & 0xff, (flashColor >> 8) & 0xff, flashColor & 0xff);

	return resultColor;
}


// -----------------------------------------------------------------------------
// Berechnet die Bounding-Box eines BS_VectorImageElement
// -----------------------------------------------------------------------------

struct CBBGetId {
	CBBGetId(const VectorImageElement &vectorImageElement_) : vectorImageElement(vectorImageElement_) {}
	unsigned operator [](unsigned i) const {
		return vectorImageElement.getPathInfo(i).getId();
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

VectorImage::VectorImage(const byte *pFileData, uint fileSize, bool &success) {
	success = false;

	// Bitstream-Objekt erzeugen
	// Im Folgenden werden die Dateidaten aus diesem ausgelesen.
	SWFBitStream bs(pFileData, fileSize);

	// SWF-Signatur überprüfen
	uint32 signature[3];
	signature[0] = bs.getByte();
	signature[1] = bs.getByte();
	signature[2] = bs.getByte();
	if (signature[0] != 'F' ||
		signature[1] != 'W' ||
		signature[2] != 'S') {
		BS_LOG_ERRORLN("File is not a valid SWF-file");
		return;
	}

	// Versionsangabe überprüfen
	uint32 version = bs.getByte();
	if (version > MAX_ACCEPTED_FLASH_VERSION) {
		BS_LOG_ERRORLN("File is of version %d. Highest accepted version is %d.", version, MAX_ACCEPTED_FLASH_VERSION);
		return;
	}

	// Dateigröße auslesen und mit der tatsächlichen Größe vergleichen
	uint32 storedFileSize = bs.getUInt32();
	if (storedFileSize != fileSize) {
		BS_LOG_ERRORLN("File is not a valid SWF-file");
		return;
	}

	// SWF-Maße auslesen
	Common::Rect movieRect = flashRectToBSRect(bs);

	// Framerate und Frameanzahl auslesen
	/* uint32 frameRate = */bs.getUInt16();
	/* uint32 frameCount = */bs.getUInt16();

	// Tags parsen
	// Da wir uns nur für das erste DefineShape-Tag interessieren
	bool keepParsing = true;
	while (keepParsing) {
		// Tags beginnen immer an Bytegrenzen
		bs.flushByte();

		// Tagtyp und Länge auslesen
		uint16 tagTypeAndLength = bs.getUInt16();
		uint32 tagType = tagTypeAndLength >> 6;
		uint32 tagLength = tagTypeAndLength & 0x3f;
		if (tagLength == 0x3f)
			tagLength = bs.getUInt32();

		switch (tagType) {
		case 2:
			// DefineShape
			success = parseDefineShape(2, bs);
			return;
		case 22:
			// DefineShape2
			success = parseDefineShape(2, bs);
			return;
		case 32:
			success = parseDefineShape(3, bs);
			return;
		default:
			// Unbekannte Tags ignorieren
			bs.skipBytes(tagLength);
		}
	}

	// Die Ausführung darf nicht an dieser Stelle ankommen: Entweder es wird ein Shape gefunden, dann wird die Funktion mit vorher verlassen, oder
	// es wird keines gefunden, dann tritt eine Exception auf sobald über das Ende der Datei hinaus gelesen wird.
	BS_ASSERT(false);
}

// -----------------------------------------------------------------------------

bool VectorImage::parseDefineShape(uint shapeType, SWFBitStream &bs) {
	/*uint32 shapeID = */bs.getUInt16();

	// Bounding Box auslesen
	_boundingBox = flashRectToBSRect(bs);

	// Erstes Image-Element erzeugen
	_elements.resize(1);

	// Styles einlesen
	uint numFillBits;
	uint numLineBits;
	if (!parseStyles(shapeType, bs, numFillBits, numLineBits))
		return false;

	uint lineStyle = 0;
	uint fillStyle0 = 0;
	uint fillStyle1 = 0;

	// Shaperecord parsen
	// ------------------

	bool endOfShapeDiscovered = false;
	while (!endOfShapeDiscovered) {
		uint32 typeFlag = bs.getBits(1);

		// Non-Edge Record
		if (typeFlag == 0) {
			// Feststellen welche Parameter gesetzt werden
			uint32 stateNewStyles = bs.getBits(1);
			uint32 stateLineStyle = bs.getBits(1);
			uint32 stateFillStyle1 = bs.getBits(1);
			uint32 stateFillStyle0 = bs.getBits(1);
			uint32 stateMoveTo = bs.getBits(1);

			// End der Shape-Definition erreicht?
			if (!stateNewStyles && !stateLineStyle && !stateFillStyle0 && !stateFillStyle1 && !stateMoveTo)
				endOfShapeDiscovered = true;
			// Parameter dekodieren
			else {
				int32 moveDeltaX = 0;
				int32 moveDeltaY = 0;
				if (stateMoveTo) {
					uint32 moveToBits = bs.getBits(5);
					moveDeltaX = bs.getSignedBits(moveToBits);
					moveDeltaY = bs.getSignedBits(moveToBits);
				}

				if (stateFillStyle0) {
					if (numFillBits > 0)
						fillStyle0 = bs.getBits(numFillBits);
					else
						fillStyle0 = 0;
				}

				if (stateFillStyle1) {
					if (numFillBits > 0)
						fillStyle1 = bs.getBits(numFillBits);
					else
						fillStyle1 = 0;
				}

				if (stateLineStyle) {
					if (numLineBits)
						lineStyle = bs.getBits(numLineBits);
					else
						numLineBits = 0;
				}

				if (stateNewStyles) {
					// An dieser Stelle werden in Flash die alten Style-Definitionen verworfen und mit den neuen überschrieben.
					// Es wird ein neues Element begonnen.
					_elements.resize(_elements.size() + 1);
					if (!parseStyles(shapeType, bs, numFillBits, numLineBits))
						return false;
				}

				// Ein neuen Pfad erzeugen, es sei denn, es wurden nur neue Styles definiert
				if (stateLineStyle || stateFillStyle0 || stateFillStyle1 || stateMoveTo) {
					// Letzte Zeichenposition merken, beim Aufruf von start_new_path() wird die Zeichenpostionen auf 0, 0 zurückgesetzt
#if 0 // TODO
					double lastX = _elements.back()._paths.last_x();
					double lastY = _elements.back()._paths.last_y();

					// Neue Pfadinformation erzeugen
					_elements.back()._pathInfos.push_back(VectorPathInfo(_elements.back()._paths.start_new_path(), lineStyle, fillStyle0, fillStyle1));

					// Falls eine Bewegung definiert wurde, wird die Zeichenpositionen an die entsprechende Position gesetzt.
					// Ansonsten wird die Zeichenposition auf die letzte Zeichenposition gesetzt.
					if (stateMoveTo)
						_elements.back()._paths.move_to(moveDeltaX, moveDeltaY);
					else
						_elements.back()._paths.move_to(lastX, lastY);
#endif
				}
			}
		} else {
			// Edge Record
			uint32 edgeFlag = bs.getBits(1);
			uint32 numBits = bs.getBits(4) + 2;

			// Curved edge
			if (edgeFlag == 0) {
				/* int32 ControlDeltaX = */bs.getSignedBits(numBits);
				/* int32 ControlDeltaY = */bs.getSignedBits(numBits);
				/* int32 AnchorDeltaX = */bs.getSignedBits(numBits);
				/* int32 AnchorDeltaY = */bs.getSignedBits(numBits);

#if 0 // TODO
				double controlX = _elements.back()._paths.last_x() + controlDeltaX;
				double controlY = _elements.back()._paths.last_y() + controlDeltaY;
				double anchorX = controlX + AnchorDeltaX;
				double anchorY = controlY + AnchorDeltaY;
				_elements.back()._paths.curve3(controlX, controlY, anchorX, anchorY);
#endif
			} else {
				// Staight edge
				int32 deltaX = 0;
				int32 deltaY = 0;

				uint32 generalLineFlag = bs.getBits(1);
				if (generalLineFlag) {
					deltaX = bs.getSignedBits(numBits);
					deltaY = bs.getSignedBits(numBits);
				} else {
					uint32 vertLineFlag = bs.getBits(1);
					if (vertLineFlag)
						deltaY = bs.getSignedBits(numBits);
					else
						deltaX = bs.getSignedBits(numBits);
				}

#if 0 // TODO
				_elements.back()._paths.line_rel(deltaX, deltaY);
#endif
			}
		}
	}

	// Bounding-Boxes der einzelnen Elemente berechnen
	Common::Array<VectorImageElement>::iterator it = _elements.begin();
	for (; it != _elements.end(); ++it)
		it->_boundingBox = CalculateBoundingBox(*it);

	return true;
}


// -----------------------------------------------------------------------------

bool VectorImage::parseStyles(uint shapeType, SWFBitStream &bs, uint &numFillBits, uint &numLineBits) {
	bs.flushByte();

	// Fillstyles parsen
	// -----------------

	// Anzahl an Fillstyles bestimmen
	uint fillStyleCount = bs.getByte();
	if (fillStyleCount == 0xff)
		fillStyleCount = bs.getUInt16();

	// Alle Fillstyles einlesen, falls ein Fillstyle mit Typ != 0 gefunden wird, wird das Parsen abgebrochen.
	// Es wird nur "solid fill" (Typ 0) unterstützt.
	_elements.back()._fillStyles.reserve(fillStyleCount);
	for (uint i = 0; i < fillStyleCount; ++i) {
		byte type = bs.getByte();
		uint32 color;
		if (shapeType == 3) {
			color = (bs.getByte() << 16) | (bs.getByte() << 8) | bs.getByte() | (bs.getByte() << 24);
		} else
			color = bs.getBits(24) | (0xff << 24);
		if (type != 0)
			return false;

		_elements.back()._fillStyles.push_back(flashColorToAGGRGBA8(color));
	}

	// Linestyles parsen
	// -----------------

	// Anzahl an Linestyles bestimmen
	uint lineStyleCount = bs.getByte();
	if (lineStyleCount == 0xff)
		lineStyleCount = bs.getUInt16();

	// Alle Linestyles einlesen
	_elements.back()._lineStyles.reserve(lineStyleCount);
	for (uint i = 0; i < lineStyleCount; ++i) {
		double width = bs.getUInt16();
		uint32 color;
		if (shapeType == 3)
			color = (bs.getByte() << 16) | (bs.getByte() << 8) | bs.getByte() | (bs.getByte() << 24);
		else
			color = bs.getBits(24) | (0xff << 24);

		_elements.back()._lineStyles.push_back(VectorImageElement::LineStyleType(width, flashColorToAGGRGBA8(color)));
	}

	// Bitbreite für die folgenden Styleindizes auslesen
	numFillBits = bs.getBits(4);
	numLineBits = bs.getBits(4);

	return true;
}


// -----------------------------------------------------------------------------

bool VectorImage::fill(const Common::Rect *pFillRect, uint color) {
	BS_LOG_ERRORLN("Fill() is not supported.");
	return false;
}


// -----------------------------------------------------------------------------

uint VectorImage::getPixel(int x, int y) {
	BS_LOG_ERRORLN("GetPixel() is not supported. Returning black.");
	return 0;
}

// -----------------------------------------------------------------------------

bool VectorImage::setContent(const byte *pixeldata, uint size, uint offset, uint stride) {
	BS_LOG_ERRORLN("SetContent() is not supported.");
	return 0;
}

} // End of namespace Sword25
