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

#ifndef ULTIMA8_GRAPHICS_SCALERS_HQ3XSCALER_H
#define ULTIMA8_GRAPHICS_SCALERS_HQ3XSCALER_H

#ifdef USE_HQ3X_SCALER

#include "ultima8/graphics/scalers/scaler.h"

namespace Pentagram {

class hq3xScaler : public Scaler {
public:
	hq3xScaler();

	virtual uint32    ScaleBits() const;          //< bits for supported integer scaling
	virtual bool      ScaleArbitrary() const;     //< supports arbitrary scaling of any degree

	virtual const char     *ScalerName() const;         //< Name Of the Scaler (1 word)
	virtual const char     *ScalerDesc() const;         //< Desciption of the Scaler
	virtual const char     *ScalerCopyright() const;    //< Scaler Copyright info
private:
	template<class uintX, class Manip, class uintS> ScalerFunc GetScaler();
};

extern const hq3xScaler hq3x_scaler;

};

#endif

#endif
