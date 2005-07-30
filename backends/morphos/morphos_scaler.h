/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM Team
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef MORPHOS_MORPHOSSCALER_H
#define MORPHOS_MORPHOSSCALER_H

#include <graphics/gfx.h>

typedef enum { ST_INVALID = 0, ST_NONE, ST_POINT, ST_ADVMAME2X, ST_SUPEREAGLE, ST_SUPER2XSAI } SCALERTYPE;

class MorphOSScaler
{
	public:
		MorphOSScaler(APTR buffer, int width, int height, ULONG *col_table, UWORD *col_table16, BitMap *bmap);
		virtual ~MorphOSScaler();

		bool Prepare(BitMap *render_bmap);
		void Finish();

		virtual void Scale(uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height) = 0;

		static MorphOSScaler *Create(SCALERTYPE st, APTR buffer, int width, int height, ULONG *col_table, UWORD *col_table16, BitMap *bmap);

		static SCALERTYPE FindByName(const char *ScalerName);
		static SCALERTYPE FindByIndex(int index);
		static const char *GetParamName(SCALERTYPE type);

	protected:
		struct GfxScaler
		{
			STRPTR 		gs_Name;
			STRPTR 		gs_ParamName;
			SCALERTYPE	gs_Type;
		};

		static GfxScaler ScummScalers[11];

		static uint32 MakeColor(int pixfmt, int r, int g, int b);

		byte *dest;
		uint32 dest_bpp;
		uint32 dest_pitch;
		uint32 dest_pixfmt;
		APTR handle;

		uint32 colorMask;
		uint32 lowPixelMask;
		uint32 qcolorMask;
		uint32 qlowpixelMask;
		uint32 redblueMask;
		uint32 greenMask;
		int PixelsPerMask;
		byte *src_line[4];
		byte *dst_line[2];
		bool ScummPCMode;

		APTR Buffer;
		ULONG	BufferWidth;
		ULONG	BufferHeight;
		ULONG *ScummColors;
		USHORT *ScummColors16;
};

#define DECLARE_SCALER(scaler_name)	class scaler_name ## Scaler : public MorphOSScaler \
	{ public: scaler_name ## Scaler(APTR buffer, int width, int height, ULONG *col_table, UWORD *col_table16, BitMap *bmap) : MorphOSScaler(buffer, width, height, col_table, col_table16, bmap) {} \
	  void Scale(uint32 src_x, uint32 src_y, uint32 dest_x, uint32 dest_y, uint32 width, uint32 height); \
	};


DECLARE_SCALER(Point)
DECLARE_SCALER(AdvMame2x)
DECLARE_SCALER(SuperEagle)
DECLARE_SCALER(Super2xSaI)

#endif

