/* Copyright (C) 1994-2003 Revolution Software Ltd
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

//the usual suspects (the usual suspicions)

#ifndef	_PROTOCOL
#define	_PROTOCOL

#include "driver/driver96.h"
#include "header.h"

uint8			*FetchPalette(uint8 *screenFile);	// Chris 04Oct96
_screenHeader	*FetchScreenHeader(uint8 *screenFile);	//Chris 04Oct96
_layerHeader	*FetchLayerHeader(uint8 *screenFile, uint16 layerNo);	//Chris 04Oct96
uint8			 *FetchShadingMask(uint8 *screenFile);	// James 08apr97

_animHeader		*FetchAnimHeader(uint8 *animFile);	// (25sep96JEL)
_cdtEntry		*FetchCdtEntry(uint8 *animFile, uint16 frameNo);	// (31oct96 JEL)
_frameHeader	*FetchFrameHeader(uint8 *animFile, uint16 frameNo);	// (25sep96JEL)
_parallax		*FetchBackgroundParallaxLayer(uint8 *screenFile, int layer); // Chris 04Oct96
_parallax		*FetchBackgroundLayer(uint8 *screenFile); // Chris 04Oct96
_parallax		*FetchForegroundParallaxLayer(uint8 *screenFile, int layer); // Chris 04Oct96
uint8			*FetchTextLine(uint8 *file, uint32	text_line);	//Tony24Oct96
uint8			CheckTextLine(uint8 *file, uint32	text_line);	// (James26jun97)
uint8			*FetchPaletteMatchTable(uint8 *screenFile);	// James 09dec96
uint8			*FetchObjectName(int32 resourceId);	// James15jan97

#endif
