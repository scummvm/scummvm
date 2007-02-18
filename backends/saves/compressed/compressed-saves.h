/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#ifndef BACKEND_SAVES_COMPRESSED_H
#define BACKEND_SAVES_COMPRESSED_H

#include "common/stdafx.h"
#include "common/savefile.h"

/**
 * Take an arbitrary InSaveFile and wrap it in a high level InSaveFile which
 * provides transparent on-the-fly decompression support.
 * Assumes the data it retrieves from the wrapped savefile to be either
 * uncompressed or in gzip format. In the former case, the original
 * savefile is returned unmodified (and in particular, not wrapped).
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 */
Common::InSaveFile *wrapInSaveFile(Common::InSaveFile *toBeWrapped);

/**
 * Take an arbitrary OutSaveFile and wrap it in a high level OutSaveFile which
 * provides transparent on-the-fly compression support.
 * The compressed data is written in the gzip format.
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 */
Common::OutSaveFile *wrapOutSaveFile(Common::OutSaveFile *toBeWrapped);

#endif
