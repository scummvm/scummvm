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

#ifndef ULTIMA8_GRAPHICS_SCALERS_BILINEARSCALER_H
#define ULTIMA8_GRAPHICS_SCALERS_BILINEARSCALER_H

#include "ultima8/graphics/scalers/scaler.h"

namespace Pentagram {

class BilinearScaler : public Scaler {
public:
	BilinearScaler();

	virtual uint32    ScaleBits() const;          //< bits for supported integer scaling
	virtual bool      ScaleArbitrary() const;     //< supports arbitrary scaling of any degree

	virtual const char     *ScalerName() const;         //< Name Of the Scaler (1 word)
	virtual const char     *ScalerDesc() const;         //< Desciption of the Scaler
	virtual const char     *ScalerCopyright() const;    //< Scaler Copyright info
};

extern const BilinearScaler bilinear_scaler;

#ifdef COMPILE_GAMMA_CORRECT_SCALERS

class GC_BilinearScaler : public Scaler {
public:
	GC_BilinearScaler();

	virtual const uint32    ScaleBits() const;          //< bits for supported integer scaling
	virtual const bool      ScaleArbitrary() const;     //< supports arbitrary scaling of any degree

	virtual const char     *ScalerName() const;         //< Name Of the Scaler (1 word)
	virtual const char     *ScalerDesc() const;         //< Desciption of the Scaler
	virtual const char     *ScalerCopyright() const;    //< Scaler Copyright info
};

extern const GC_BilinearScaler GC_bilinear_scaler;
#endif

};

#endif
