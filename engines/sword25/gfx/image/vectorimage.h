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

#ifndef SWORD25_VECTORIMAGE_H
#define SWORD25_VECTORIMAGE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/gfx/image/image.h"
#include "common/rect.h"

#if 0
#include "agg_path_storage.h"
#endif

namespace Sword25 {

class VectorImage;

/**
    @brief Pfadinformationen zu BS_VectorImageElement Objekten

    Jedes BS_VectorImageElement besteht aus Kantenzügen, oder auch Pfaden. Jeder dieser Pfad hat Eigenschaften, die in Objekten diesen Typs
    gespeichert werden.
*/

class VectorPathInfo {
public:
	VectorPathInfo(unsigned int ID, unsigned int LineStyle, unsigned int FillStyle0, unsigned int FillStyle1) :
		m_ID(ID), m_LineStyle(LineStyle), m_FillStyle0(FillStyle0), m_FillStyle1(FillStyle1) {}

	VectorPathInfo() {
		m_ID = m_LineStyle = m_FillStyle0 = m_FillStyle1 = 0;
	}

	unsigned int GetID() const {
		return m_ID;
	}
	unsigned int GetLineStyle() const {
		return m_LineStyle;
	}
	unsigned int GetFillStyle0() const {
		return m_FillStyle0;
	}
	unsigned int GetFillStyle1() const {
		return m_FillStyle1;
	}

private:
	unsigned int m_ID;
	unsigned int m_LineStyle;
	unsigned int m_FillStyle0;
	unsigned int m_FillStyle1;
};


/**
    @brief Ein Element eines Vektorbild. Ein BS_VectorImage besteht aus diesen Elementen, die jeweils einen Teil der Graphik definieren.
           Werden alle Elemente eines Vektorbildes übereinandergelegt, ergibt sich das komplette Bild.
*/
class VectorImageElement {
	friend class VectorImage;
public:
#if 0 // TODO
	const agg::path_storage &GetPaths() const {
		return m_Paths;
	}
#endif

	unsigned int GetPathCount() const {
		return m_PathInfos.size();
	}
	const VectorPathInfo &GetPathInfo(unsigned int PathNr) const {
		BS_ASSERT(PathNr < GetPathCount());
		return m_PathInfos[PathNr];
	}

	double GetLineStyleWidth(unsigned int LineStyle) const {
		BS_ASSERT(LineStyle < m_LineStyles.size());
		return m_LineStyles[LineStyle].Width;
	}

	unsigned int GetLineStyleCount() const {
		return m_LineStyles.size();
	}

	uint32 GetLineStyleColor(unsigned int LineStyle) const {
		BS_ASSERT(LineStyle < m_LineStyles.size());
		return m_LineStyles[LineStyle].Color;
	}

	unsigned int GetFillStyleCount() const {
		return m_FillStyles.size();
	}

	uint32 GetFillStyleColor(unsigned int FillStyle) const {
		BS_ASSERT(FillStyle < m_FillStyles.size());
		return m_FillStyles[FillStyle];
	}

	const Common::Rect &GetBoundingBox() const {
		return m_BoundingBox;
	}

private:
	struct LineStyleType {
		LineStyleType(double Width_, uint32 Color_) : Width(Width_), Color(Color_) {}
		LineStyleType() { Width = 0; Color = 0; }
		double Width;
		uint32 Color;
	};

#if 0 // TODO
	agg::path_storage m_Paths;
#endif
	Common::Array<VectorPathInfo> m_PathInfos;
	Common::Array<LineStyleType> m_LineStyles;
	Common::Array<uint32>  m_FillStyles;
	Common::Rect m_BoundingBox;
};


/**
    @brief Eine Vektorgraphik

    Objekte dieser Klasse enthalten die Informationen eines SWF-Shapes.
*/

class VectorImage : public Image {
public:
	VectorImage(const unsigned char *pFileData, unsigned int FileSize, bool &Success);

	unsigned int GetElementCount() const {
		return m_Elements.size();
	}
	const VectorImageElement &GetElement(unsigned int ElementNr) const {
		BS_ASSERT(ElementNr < m_Elements.size());
		return m_Elements[ElementNr];
	}
	const Common::Rect &GetBoundingBox() const {
		return m_BoundingBox;
	}

	//
	// Die abstrakten Methoden von BS_Image
	//
	virtual int GetWidth() const {
		return m_BoundingBox.width();
	}
	virtual int GetHeight() const {
		return m_BoundingBox.height();
	}
	virtual GraphicEngine::COLOR_FORMATS GetColorFormat() const {
		return GraphicEngine::CF_ARGB32;
	}
	virtual bool Fill(const Common::Rect *pFillRect = 0, unsigned int Color = BS_RGB(0, 0, 0));
	virtual unsigned int GetPixel(int X, int Y);
	virtual bool IsBlitSource() const {
		return true;
	}
	virtual bool IsBlitTarget() const {
		return false;
	}
	virtual bool IsScalingAllowed() const {
		return true;
	}
	virtual bool IsFillingAllowed() const {
		return false;
	}
	virtual bool IsAlphaAllowed() const {
		return true;
	}
	virtual bool IsColorModulationAllowed() const {
		return true;
	}
	virtual bool IsSetContentAllowed() const {
		return false;
	}
	virtual bool SetContent(const byte *Pixeldata, uint size, unsigned int Offset, unsigned int Stride);
	virtual bool Blit(int PosX = 0, int PosY = 0,
	                  int Flipping = FLIP_NONE,
	                  Common::Rect *pPartRect = NULL,
	                  unsigned int Color = BS_ARGB(255, 255, 255, 255),
	                  int Width = -1, int Height = -1);

	class SWFBitStream;

private:
	bool ParseDefineShape(unsigned int ShapeType, SWFBitStream &bs);
	bool ParseStyles(unsigned int ShapeType, SWFBitStream &bs, unsigned int &NumFillBits, unsigned int &NumLineBits);

	Common::Array<VectorImageElement>    m_Elements;
	Common::Rect                             m_BoundingBox;
};

} // End of namespace Sword25

#endif
