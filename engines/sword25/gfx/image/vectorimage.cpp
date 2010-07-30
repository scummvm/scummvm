// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/bs_stdint.h"
#include "sword25/gfx/image/vectorimage.h"
#include <vector>
#include <stdexcept>

#include "agg_bounding_rect.h"

using namespace std;

#define BS_LOG_PREFIX "VECTORIMAGE"


// -----------------------------------------------------------------------------
// SWF Datentypen
// -----------------------------------------------------------------------------

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint16_t u16;
typedef int16_t s32;


// -----------------------------------------------------------------------------
// Bitstream Hilfsklasse
// -----------------------------------------------------------------------------
// Das Parsen von SWF-Dateien erfordert sowohl bitweises Auslesen als auch an
// Bytegrenzen ausgerichtetes Lesen.
// Diese Klasse ist speziell dafür ausgestattet.
// -----------------------------------------------------------------------------

class BS_VectorImage::SWFBitStream
{
public:
	SWFBitStream(const unsigned char * pData, unsigned int DataSize) :
	  m_Pos(pData), m_End(pData + DataSize), m_WordMask(0)
	  {}

	  inline u32 GetBits(unsigned int BitCount)
	  {
		  if (BitCount == 0 || BitCount > 32)
		  {
			  throw(runtime_error("SWFBitStream::GetBits() must read at least 1 and at most 32 bits at a time"));
		  }

		  u32 value = 0;
		  while (BitCount)
		  {
			  if (m_WordMask == 0) FlushByte();

			  value <<= 1;
			  value |= ((m_Word & m_WordMask) != 0) ? 1 : 0;
			  m_WordMask >>= 1;

			  --BitCount;
		  }

		  return value;
	  }

	  inline s32 GetSignedBits(unsigned int BitCount)
	  {
		  // Bits einlesen
		  u32 Temp = GetBits(BitCount);

		  // Falls das Sign-Bit gesetzt ist, den Rest des Rückgabewertes mit 1-Bits auffüllen (Sign Extension)
		  if (Temp & 1 << (BitCount - 1))
			  return (0xffffffff << BitCount) | Temp;
		  else
			  return Temp;
	  }

	  inline u32 GetU32()
	  {
		  u32 Byte1 = GetU8();
		  u32 Byte2 = GetU8();
		  u32 Byte3 = GetU8();
		  u32 Byte4 = GetU8();

		  return Byte1 | (Byte2 << 8) | (Byte3 << 16) | (Byte4 << 24);
	  }

	  inline u16 GetU16()
	  {
		  u32 Byte1 = GetU8();
		  u32 Byte2 = GetU8();

		  return Byte1 | (Byte2 << 8);
	  }

	  inline u8 GetU8()
	  {
		  FlushByte();
		  u8 Value = m_Word;
		  m_WordMask = 0;
		  FlushByte();

		  return Value;
	  }

	  inline void FlushByte()
	  {
		  if (m_WordMask != 128)
		  {
			  if (m_Pos >= m_End)
			  {
				  throw(runtime_error("Attempted to read past end of file"));
			  }
			  else
			  {
				  m_Word = *m_Pos++;
				  m_WordMask = 128;
			  }
		  }
	  }

	  inline void SkipBytes(unsigned int SkipLength)
	  {
		  FlushByte();
		  if (m_Pos + SkipLength >= m_End)
		  {
			  throw(runtime_error("Attempted to read past end of file"));
		  }
		  else
		  {
			  m_Pos += SkipLength;
			  m_Word = *(m_Pos - 1);
		  }
	  }

private:
	const unsigned char *	m_Pos;
	const unsigned char *	m_End;

	u8						m_Word;
	unsigned int			m_WordMask;
};


// -----------------------------------------------------------------------------
// Konstanten und Hilfsfunktionen
// -----------------------------------------------------------------------------

namespace
{
	// -----------------------------------------------------------------------------
	// Konstanten
	// -----------------------------------------------------------------------------

	const u32 MAX_ACCEPTED_FLASH_VERSION = 3;	// Die höchste Flash-Dateiversion, die vom Lader akzeptiert wird


	// -----------------------------------------------------------------------------
	// Konvertiert SWF-Rechteckdaten in einem Bitstrom in BS_Rect-Objekte
	// -----------------------------------------------------------------------------

	BS_Rect FlashRectToBSRect(BS_VectorImage::SWFBitStream & bs)
	{
		bs.FlushByte();

		// Feststellen mit wie vielen Bits die einzelnen Komponenten kodiert sind
		u32 BitsPerValue = bs.GetBits(5);

		// Die einzelnen Komponenten einlesen
		s32 XMin = bs.GetSignedBits(BitsPerValue);
		s32 XMax = bs.GetSignedBits(BitsPerValue);
		s32 YMin = bs.GetSignedBits(BitsPerValue);
		s32 YMax = bs.GetSignedBits(BitsPerValue);

		return BS_Rect(XMin, YMin, XMax + 1, YMax + 1);
	}


	// -----------------------------------------------------------------------------
	// Konvertiert SWF-Farben in AntiGrain Farben
	// -----------------------------------------------------------------------------

	agg::rgba8 FlashColorToAGGRGBA8(unsigned int FlashColor)
	{
		agg::rgba8 ResultColor((FlashColor >> 16) & 0xff, (FlashColor >> 8) & 0xff, FlashColor & 0xff, FlashColor >> 24);
		ResultColor.premultiply();
		return ResultColor;
	}


	// -----------------------------------------------------------------------------
	// Berechnet die Bounding-Box eines BS_VectorImageElement
	// -----------------------------------------------------------------------------

	struct CBBGetId
	{
		CBBGetId(const BS_VectorImageElement & VectorImageElement_) : VectorImageElement(VectorImageElement_) {}
		unsigned operator [] (unsigned i) const { return VectorImageElement.GetPathInfo(i).GetID(); }
		const BS_VectorImageElement & VectorImageElement;
	};

	BS_Rect CalculateBoundingBox(const BS_VectorImageElement & VectorImageElement)
	{
		agg::path_storage Path = VectorImageElement.GetPaths();
		CBBGetId IdSource(VectorImageElement);

		double x1, x2, y1, y2;
		agg::bounding_rect(Path, IdSource, 0, VectorImageElement.GetPathCount(), &x1, &y1, &x2, &y2);
		
		return BS_Rect(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2) + 1, static_cast<int>(y2) + 1);
	}
}


// -----------------------------------------------------------------------------
// Konstruktion
// -----------------------------------------------------------------------------

BS_VectorImage::BS_VectorImage(const unsigned char * pFileData, unsigned int FileSize, bool & Success)
{
	Success = false;

	// Bitstream-Objekt erzeugen
	// Im Folgenden werden die Dateidaten aus diesem ausgelesen.
	SWFBitStream bs(pFileData, FileSize);

	try
	{
		// SWF-Signatur überprüfen
		u32 Signature[3];
		Signature[0] = bs.GetU8();
		Signature[1] = bs.GetU8();
		Signature[2] = bs.GetU8();
		if (Signature[0] != 'F' ||
			Signature[1] != 'W' ||
			Signature[2] != 'S')
		{
			BS_LOG_ERRORLN("File is not a valid SWF-file");
			return;
		}

		// Versionsangabe überprüfen
		u32 Version = bs.GetU8();
		if (Version > MAX_ACCEPTED_FLASH_VERSION)
		{
			BS_LOG_ERRORLN("File is of version %d. Highest accepted version is %d.", Version, MAX_ACCEPTED_FLASH_VERSION);
			return;
		}

		// Dateigröße auslesen und mit der tatsächlichen Größe vergleichen
		u32 StoredFileSize = bs.GetU32();
		if (StoredFileSize != FileSize)
		{
			BS_LOG_ERRORLN("File is not a valid SWF-file");
			return;
		}

		// SWF-Maße auslesen
		BS_Rect MovieRect = FlashRectToBSRect(bs);

		// Framerate und Frameanzahl auslesen
		u32 FrameRate = bs.GetU16();
		u32 FrameCount = bs.GetU16();

		// Tags parsen
		// Da wir uns nur für das erste DefineShape-Tag interessieren
		bool KeepParsing = true;
		while (KeepParsing)
		{
			// Tags beginnen immer an Bytegrenzen
			bs.FlushByte();

			// Tagtyp und Länge auslesen
			u16 TagTypeAndLength = bs.GetU16();
			u32 TagType = TagTypeAndLength >> 6;
			u32 TagLength = TagTypeAndLength & 0x3f;
			if (TagLength == 0x3f) TagLength = bs.GetU32();

			switch (TagType)
			{
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
	}

	catch (runtime_error & e)
	{
		// Fehler loggen und Funktion verlassen
		// Success ist somit "false" und signalisiert dem Programmierer, dass die Konstruktion fehlgeschlagen ist.
		BS_LOG_ERRORLN("The following exception occured while parsing a SWF-file: %s", e.what());
		return;
	}

	// Die Ausführung darf nicht an dieser Stelle ankommen: Entweder es wird ein Shape gefunden, dann wird die Funktion mit vorher verlassen, oder
	// es wird keines gefunden, dann tritt eine Exception auf sobald über das Ende der Datei hinaus gelesen wird.
	BS_ASSERT(false);
}

// -----------------------------------------------------------------------------

bool BS_VectorImage::ParseDefineShape(unsigned int ShapeType, SWFBitStream & bs)
{
	u32 ShapeID = bs.GetU16();

	// Bounding Box auslesen
	m_BoundingBox = FlashRectToBSRect(bs);

	// Erstes Image-Element erzeugen
	m_Elements.resize(1);

	// Styles einlesen
	unsigned int NumFillBits;
	unsigned int NumLineBits;
	if (!ParseStyles(ShapeType, bs, NumFillBits, NumLineBits)) return false;

	unsigned int LineStyle = 0;
	unsigned int FillStyle0 = 0;
	unsigned int FillStyle1 = 0;

	// Shaperecord parsen
	// ------------------

	bool EndOfShapeDiscovered = false;
	while (!EndOfShapeDiscovered)
	{
		u32 TypeFlag = bs.GetBits(1);

		// Non-Edge Record
		if (TypeFlag == 0)
		{
			// Feststellen welche Parameter gesetzt werden
			u32 StateNewStyles = bs.GetBits(1);
			u32 StateLineStyle = bs.GetBits(1);
			u32 StateFillStyle1 = bs.GetBits(1);
			u32 StateFillStyle0 = bs.GetBits(1);
			u32 StateMoveTo = bs.GetBits(1);

			// End der Shape-Definition erreicht?
			if (!StateNewStyles && !StateLineStyle && !StateFillStyle0 && !StateFillStyle1 && !StateMoveTo)
				EndOfShapeDiscovered = true;
			// Parameter dekodieren
			else
			{
				s32 MoveDeltaX = 0;
				s32 MoveDeltaY = 0;
				if (StateMoveTo)
				{
					u32 MoveToBits = bs.GetBits(5);
					MoveDeltaX = bs.GetSignedBits(MoveToBits);
					MoveDeltaY = bs.GetSignedBits(MoveToBits);
				}

				if (StateFillStyle0)
				{
					if (NumFillBits > 0)
						FillStyle0 = bs.GetBits(NumFillBits);
					else
						FillStyle0 = 0;
				}

				if (StateFillStyle1)
				{
					if (NumFillBits > 0)
						FillStyle1 = bs.GetBits(NumFillBits);
					else
						FillStyle1 = 0;
				}

				if (StateLineStyle)
				{
					if (NumLineBits)
						LineStyle = bs.GetBits(NumLineBits);
					else
						NumLineBits = 0;
				}

				if (StateNewStyles)
				{
					// An dieser Stelle werden in Flash die alten Style-Definitionen verworfen und mit den neuen überschrieben.
					// Es wird ein neues Element begonnen.
					m_Elements.resize(m_Elements.size() + 1);
					if (!ParseStyles(ShapeType, bs, NumFillBits, NumLineBits)) return false;
				}

				// Ein neuen Pfad erzeugen, es sei denn, es wurden nur neue Styles definiert
				if (StateLineStyle || StateFillStyle0 || StateFillStyle1 || StateMoveTo)
				{
					// Letzte Zeichenposition merken, beim Aufruf von start_new_path() wird die Zeichenpostionen auf 0, 0 zurückgesetzt
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
				}
			}
		}
		// Edge Record
		else
		{
			u32 EdgeFlag = bs.GetBits(1);
			u32 NumBits = bs.GetBits(4) + 2;

			// Curved edge
			if (EdgeFlag == 0)
			{
				s32 ControlDeltaX = bs.GetSignedBits(NumBits);
				s32 ControlDeltaY = bs.GetSignedBits(NumBits);
				s32 AnchorDeltaX = bs.GetSignedBits(NumBits);
				s32 AnchorDeltaY = bs.GetSignedBits(NumBits);

				double ControlX = m_Elements.back().m_Paths.last_x() + ControlDeltaX;
				double ControlY = m_Elements.back().m_Paths.last_y() + ControlDeltaY;
				double AnchorX = ControlX + AnchorDeltaX;
				double AnchorY = ControlY + AnchorDeltaY;
				m_Elements.back().m_Paths.curve3(ControlX, ControlY, AnchorX, AnchorY);
			}
			// Staight edge
			else
			{
				s32 DeltaX = 0;
				s32 DeltaY = 0;

				u32 GeneralLineFlag = bs.GetBits(1);
				if (GeneralLineFlag)
				{
					DeltaX = bs.GetSignedBits(NumBits);
					DeltaY = bs.GetSignedBits(NumBits);
				}
				else
				{
					u32 VertLineFlag = bs.GetBits(1);
					if (VertLineFlag)
						DeltaY = bs.GetSignedBits(NumBits);
					else
						DeltaX = bs.GetSignedBits(NumBits);
				}

				m_Elements.back().m_Paths.line_rel(DeltaX, DeltaY);
			}
		}
	}

	// Bounding-Boxes der einzelnen Elemente berechnen
	vector<BS_VectorImageElement>::iterator it = m_Elements.begin();
	for (; it != m_Elements.end(); ++it) it->m_BoundingBox = CalculateBoundingBox(*it);

	return true;
}


// -----------------------------------------------------------------------------

bool BS_VectorImage::ParseStyles(unsigned int ShapeType, SWFBitStream & bs, unsigned int & NumFillBits, unsigned int & NumLineBits)
{
	bs.FlushByte();

	// Fillstyles parsen
	// -----------------

	// Anzahl an Fillstyles bestimmen
	unsigned int FillStyleCount = bs.GetU8();
	if (FillStyleCount == 0xff) FillStyleCount = bs.GetU16();

	// Alle Fillstyles einlesen, falls ein Fillstyle mit Typ != 0 gefunden wird, wird das Parsen abgebrochen.
	// Es wird nur "solid fill" (Typ 0) unterstützt.
	m_Elements.back().m_FillStyles.reserve(FillStyleCount);
	for (unsigned int i = 0; i < FillStyleCount; ++i)
	{
		u8 Type = bs.GetU8();
		u32 Color;
		if (ShapeType == 3)
		{
			Color = (bs.GetU8() << 16) | (bs.GetU8() << 8) | bs.GetU8() | (bs.GetU8() << 24);
		}
		else
			Color = bs.GetBits(24) | (0xff << 24);
		if (Type != 0) return false;

		m_Elements.back().m_FillStyles.push_back(FlashColorToAGGRGBA8(Color));
	}

	// Linestyles parsen
	// -----------------

	// Anzahl an Linestyles bestimmen
	unsigned int LineStyleCount = bs.GetU8();
	if (LineStyleCount == 0xff) LineStyleCount = bs.GetU16();

	// Alle Linestyles einlesen
	m_Elements.back().m_LineStyles.reserve(LineStyleCount);
	for (unsigned int i = 0; i < LineStyleCount; ++i)
	{
		double Width = bs.GetU16();
		u32 Color;
		if (ShapeType == 3)
			Color = (bs.GetU8() << 16) | (bs.GetU8() << 8) | bs.GetU8() | (bs.GetU8() << 24);
		else
			Color = bs.GetBits(24) | (0xff << 24);

		m_Elements.back().m_LineStyles.push_back(BS_VectorImageElement::LineStyleType(Width, FlashColorToAGGRGBA8(Color)));
	}

	// Bitbreite für die folgenden Styleindizes auslesen
	NumFillBits = bs.GetBits(4);
	NumLineBits = bs.GetBits(4);

	return true;
}


// -----------------------------------------------------------------------------

bool BS_VectorImage::Fill(const BS_Rect* pFillRect, unsigned int Color)
{
	BS_LOG_ERRORLN("Fill() is not supported.");
	return false;
}


// -----------------------------------------------------------------------------

unsigned int BS_VectorImage::GetPixel(int X, int Y)
{
	BS_LOG_ERRORLN("GetPixel() is not supported. Returning black.");
	return 0;
}

// -----------------------------------------------------------------------------

bool BS_VectorImage::SetContent(const std::vector<unsigned char> & Pixeldata, unsigned int Offset, unsigned int Stride)
{
	BS_LOG_ERRORLN("SetContent() is not supported.");
	return 0;
}
