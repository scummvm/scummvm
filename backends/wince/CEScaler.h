/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef CESCALER
#define CESCALER

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/scaler.h"
#include "common/scaler/intern.h"

DECLARE_SCALER(PocketPCPortrait);
DECLARE_SCALER(PocketPCHalf);
DECLARE_SCALER(PocketPCHalfZoom);
//#ifdef WIN32_PLATFORM_WFSP
DECLARE_SCALER(SmartphoneLandscape);
//#endif

void initCEScaler(void);

#endif
