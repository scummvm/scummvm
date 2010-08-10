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

#include "sword25/gfx/image/vectorimagerenderer.h"
#include "sword25/gfx/image/vectorimage.h"

#if 0 // TODO
#include "agg_conv_curve.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"
#endif

namespace Sword25 {

#if 0 // TODO
// -----------------------------------------------------------------------------
// CompoundShape
// -----------------------------------------------------------------------------

class CompoundShape {
public:
	CompoundShape(const BS_VectorImageElement &VectorImageElement) :
		m_ImageElement(VectorImageElement),
		m_Path(VectorImageElement.GetPaths()),
		m_Affine(),
		m_Curve(m_Path),
		m_Trans(m_Curve, m_Affine)
	{}

	unsigned operator [](unsigned i) const {
		return m_ImageElement.GetPathInfo(i).GetID();
	}

	unsigned paths() const {
		return m_ImageElement.GetPathCount();
	}

	void rewind(unsigned path_id) {
		m_Trans.rewind(path_id);
	}

	unsigned vertex(double *x, double *y) {
		return m_Trans.vertex(x, y);
	}

private:
	const BS_VectorImageElement                                &m_ImageElement;
	agg::path_storage                                           m_Path;
	agg::trans_affine                                           m_Affine;
	agg::conv_curve<agg::path_storage>                          m_Curve;
	agg::conv_transform< agg::conv_curve<agg::path_storage> >   m_Trans;
};


// -----------------------------------------------------------------------------
// StyleHandler
// -----------------------------------------------------------------------------

class StyleHandler {
public:
	StyleHandler(const BS_VectorImageElement &VectorImageElement) : m_ImageElement(VectorImageElement) {}

	bool is_solid(unsigned int style) const {
		return true;
	}

	const agg::rgba8 &color(unsigned style) const {
		return m_ImageElement.GetFillStyleColor(style);
	}

	void generate_span(agg::rgba8 *span, int x, int y, unsigned len, unsigned style) {
		// Wird nicht benutzt
		return;
	}

private:
	const BS_VectorImageElement &m_ImageElement;
};

BS_VectorImageRenderer::BS_VectorImageRenderer() :
	PixelFormat(rbuf) {

}


bool BS_VectorImageRenderer::Render(const BS_VectorImage &VectorImage,
                                    float ScaleFactorX, float ScaleFactorY,
                                    unsigned int &Width, unsigned int &Height,
                                    byte *ImageData,
                                    float LineScaleFactor,
                                    bool NoAlphaShapes) {
	Width = static_cast<unsigned int>(VectorImage.GetWidth() * ScaleFactorX);
	Height = static_cast<unsigned int>(VectorImage.GetHeight() * ScaleFactorY);

	ImageData.resize(Width * Height * 4);
	memset(&ImageData[0], 0, ImageData.size());
	rbuf.attach(reinterpret_cast<agg::int8u *>(&ImageData[0]), Width, Height, Width * 4);

	BaseRenderer.attach(PixelFormat);
	ScanlineRenderer.attach(BaseRenderer);

	// Die SWF-Shapes sind häufig nicht am Ursprung (0, 0) ausgerichtet, daher wird die Shape vor dem Rendern derart verschoben, dass
	// sich die linke obere Ecke der Bounding-Box im Ursprung befindet. Danach wird die Skalierung angewandt.
	Scale = agg::trans_affine_translation(- VectorImage.GetBoundingBox().left, - VectorImage.GetBoundingBox().top);
	Scale *= agg::trans_affine_scaling(ScaleFactorX, ScaleFactorY);

	for (unsigned int element = 0; element < VectorImage.GetElementCount(); ++element) {
		const BS_VectorImageElement &CurImageElement = VectorImage.GetElement(element);

		CompoundShape ImageCompoundShape(CurImageElement);
		StyleHandler ImageStyleHandler(CurImageElement);
		agg::conv_transform<CompoundShape> Shape(ImageCompoundShape, Scale);
		agg::conv_stroke<agg::conv_transform<CompoundShape> > Stroke(Shape);

		// Fill shape
		//----------------------
		CompoundRasterizer.clip_box(0, 0, Width, Height);
		CompoundRasterizer.reset();
		for (unsigned int i = 0; i < CurImageElement.GetPathCount(); ++i) {
			unsigned int FillStyle0 = CurImageElement.GetPathInfo(i).GetFillStyle0();
			unsigned int FillStyle1 = CurImageElement.GetPathInfo(i).GetFillStyle1();

			if (NoAlphaShapes) {
				if (FillStyle0 != 0 && CurImageElement.GetFillStyleColor(FillStyle0 - 1).a != 255) FillStyle0 = 0;
				if (FillStyle1 != 0 && CurImageElement.GetFillStyleColor(FillStyle1 - 1).a != 255) FillStyle1 = 0;
			}

			if (FillStyle0 != 0 || FillStyle1 != 0) {
				CompoundRasterizer.styles(FillStyle0 - 1, FillStyle1 - 1);
				CompoundRasterizer.add_path(Shape, CurImageElement.GetPathInfo(i).GetID());
			}
		}
		agg::render_scanlines_compound_layered(CompoundRasterizer, Scanline, BaseRenderer, Alloc, ImageStyleHandler);


		// Draw strokes
		//----------------------
		Rasterizer.clip_box(0, 0, Width, Height);
		Stroke.line_join(agg::round_join);
		Stroke.line_cap(agg::round_cap);
		for (unsigned int i = 0; i < CurImageElement.GetPathCount(); ++i) {
			Rasterizer.reset();

			unsigned int CurrentLineStyle = CurImageElement.GetPathInfo(i).GetLineStyle();
			if (CurrentLineStyle != 0) {
				Stroke.width(ScaleFactorX * CurImageElement.GetLineStyleWidth(CurrentLineStyle - 1) * LineScaleFactor);
				Rasterizer.add_path(Stroke, CurImageElement.GetPathInfo(i).GetID());
				ScanlineRenderer.color(CurImageElement.GetLineStyleColor(CurrentLineStyle - 1));
				// HACK
				// Die SWF-Frames enthalten zum Teil Reste von grünen Linien, die wohl von Bernd als Umriss benutzt wurden.
				// Damit diese Reste nicht störend auffallen werden grüne Linien schlichtweg ignoriert.
				if (!(CurImageElement.GetLineStyleColor(CurrentLineStyle - 1).a == 255 &&
				        CurImageElement.GetLineStyleColor(CurrentLineStyle - 1).r == 0 &&
				        CurImageElement.GetLineStyleColor(CurrentLineStyle - 1).g == 255 &&
				        CurImageElement.GetLineStyleColor(CurrentLineStyle - 1).b == 0))
					agg::render_scanlines(Rasterizer, Scanline, ScanlineRenderer);
			}
		}
	}

	return true;
}

#else

BS_VectorImageRenderer::BS_VectorImageRenderer() {}

bool BS_VectorImageRenderer::Render(const BS_VectorImage &VectorImage,
                                    float ScaleFactorX, float ScaleFactorY,
                                    unsigned int &Width, unsigned int &Height,
                                    byte *ImageData,
                                    float LineScaleFactor,
                                    bool NoAlphaShapes) {
	return true;
}
#endif

} // End of namespace Sword25
