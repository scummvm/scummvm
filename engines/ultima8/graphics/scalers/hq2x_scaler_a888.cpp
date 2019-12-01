/*
 *  Copyright (C) 2004  Ryan Nunn and The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "ultima8/misc/pent_include.h"

#ifdef USE_HQ2X_SCALER

#include "ultima8/graphics/scalers/hq2x_scaler.inc"

namespace Ultima8 {
namespace Pentagram {
template Scaler::ScalerFunc hq2xScaler::GetScaler<uint32, Manip_32_A888, uint32>();
} // End of namespace Pentagram
} // End of namespace Ultima8

#endif
