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

#ifndef SWORD25_VECTORIMAGERENDERER_H
#define SWORD25_VECTORIMAGERENDERER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include <vector>

#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_rasterizer_compound_aa.h"
#include "agg_scanline_u.h"
#include "agg_scanline_bin.h"
#include "agg_trans_affine.h"
#include "agg_span_allocator.h"

class BS_VectorImage;


/**
	@brief Rendert BS_VectorImage Objekte
*/

class BS_VectorImageRenderer
{
public:
	BS_VectorImageRenderer();

	bool Render(const BS_VectorImage & VectorImage,
				float ScaleFactorX, float ScaleFactorY, 
				unsigned int & Width, unsigned int & Height,
				std::vector<char> & ImageData,
				float LineScaleFactor = 1.0f,
				bool NoAlphaShapes = false);

private:
	typedef agg::pixfmt_rgba32_pre PixelFormatType;
	typedef agg::renderer_base<PixelFormatType> BaseRendererType;
	typedef agg::renderer_scanline_aa_solid<BaseRendererType> ScanlineRendererType;

	agg::rendering_buffer rbuf;
	PixelFormatType PixelFormat;
	BaseRendererType BaseRenderer;
	ScanlineRendererType ScanlineRenderer;
	agg::rasterizer_scanline_aa<agg::rasterizer_sl_clip_dbl> Rasterizer;
	agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_dbl> CompoundRasterizer;
	agg::scanline_u8 Scanline;
	agg::scanline_bin ScanlineBin;
	agg::trans_affine Scale;
	agg::span_allocator<agg::rgba8> Alloc;
};

#endif
