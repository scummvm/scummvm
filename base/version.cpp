/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "base/internal_version.h"
#include "base/version.h"

const char *gResidualVersion = RESIDUAL_VERSION;
const char *gResidualBuildDate = __DATE__ " " __TIME__;
const char *gResidualVersionDate = RESIDUAL_VERSION " (" __DATE__ " " __TIME__ ")";
const char *gResidualFullVersion = "Residual " RESIDUAL_VERSION " (" __DATE__ " " __TIME__ ")";
const char *gResidualFeatures = ""
#ifdef USE_TREMOR
#ifdef USE_TREMOLO
	// libTremolo is used on WinCE for better ogg performance
	"Tremolo "
#else
	"Tremor "
#endif
#else
#ifdef USE_VORBIS
	"Vorbis "
#endif
#endif

#ifdef USE_FLAC
	"FLAC "
#endif

#ifdef USE_MAD
	"MP3 "
#endif

#ifdef USE_ZLIB
	"zLib "
#endif
	;

