/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_REMORA_SPRITE_H_INCLUDED
#define ICB_REMORA_SPRITE_H_INCLUDED

#include "engines/icb/p4.h"
#include "engines/icb/debug.h"
#include "engines/icb/string_vest.h"
#include "engines/icb/common/px_clu_api.h"
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_assert.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_bitmap.h"

namespace ICB {

// Use this structure to pass extra information into the Remora sprite drawing functions.
struct _rs_params {
	uint32 nW, nH;    // PSX only.  Width to draw (use values in sprite if nW==0 and nH==0.
	bool8 bAllFrames; // PSX only.  Used for decompression.
	uint8 r, g, b;    // PSX only.  RGB scaling values.
	bool8 bCentre;    // PC & PSX.  If true, sprite is centred on coordinate.
	uint8 nOpacity;   // PC only (I think).  If not 255, sprite is blended into surface.
	bool8 bUpdate;    // PC & PSX.  If true, frame count is automatically updated.
	uint8 nPad1;

	// Initialisation.
	_rs_params() {
		nW = 0;              // Defaults to zero, so value is read from the sprite.
		nH = 0;              // Ditto.
		bAllFrames = FALSE8; // Whatever it is, it's off.
		r = 128;             // I'll have to check these.  I think 128 means 'no colour scale'.
		g = 128;             // Ditto.
		b = 128;             // Ditto.
		bCentre = TRUE8;     // Centre the sprite by default.
		nOpacity = 255;      // Draw it with no transparency.
		bUpdate = TRUE8;     // Update the frame counter, so it animates.
	}
};

class _remora_sprite {
public:
	// Constructor and destructor.
	_remora_sprite();
	~_remora_sprite() { ; }

	// Copy constructor and assignment.
	_remora_sprite(const _remora_sprite &oX);
	const _remora_sprite &operator=(const _remora_sprite &oOpB);

	// This sets up the object for a particular animating bitmap.
	void InitialiseFromBitmapName(const char *pcBitmapName, const char *pcClusterName, uint32 nClusterash);

	// These return the size of a loaded sprite.
	uint32 GetHeight();
	uint32 GetWidth();

	// These draw sprites on the Remora's screen.
	inline void DrawSprite(const _rs_params *pParams = NULL);
	inline void DrawXYSprite(int32 nX, int32 nY, const _rs_params *pParams = NULL);

	inline bool8 FitsOnScreen(int32 nX, int32 nZ, int32 nScreenWidth, int32 nScreenHeight) const;

private:
	char m_pcName[MAXLEN_URL]; // The full path and name of the sprite.

	uint32 m_nNameHash;                       // The hash version of the name of the sprite.
	char m_pcClusterName[MAXLEN_CLUSTER_URL]; // Name of the cluster the sprite is coming from.
	uint32 m_nClusterHash;                    // Hash of the cluster the sprite is coming from.

	uint32 m_nFramePC;   // Frame counter for running animations.
	uint32 m_nNumFrames; // Number of frames in the animation.

	int32 m_nHalfSpriteWidth;  // These two store half the sprite's width and height
	int32 m_nHalfSpriteHeight; // so we can work out an area that they can legally be plotted.

	// Private functions used only in this class.
	void GenericSpriteDraw(int32 nX, int32 nY, bool8 bPosition, const _rs_params *pParams);
};

inline void _remora_sprite::DrawSprite(const _rs_params *pParams) { GenericSpriteDraw(0, 0, FALSE8, pParams); }

inline void _remora_sprite::DrawXYSprite(int32 nX, int32 nY, const _rs_params *pParams) { GenericSpriteDraw(nX, nY, TRUE8, pParams); }

inline bool8 _remora_sprite::FitsOnScreen(int32 nX, int32 nZ, int32 nScreenWidth, int32 nScreenHeight) const {
	if ((nX > m_nHalfSpriteWidth) && (nX < (nScreenWidth - m_nHalfSpriteWidth)) && (nZ > m_nHalfSpriteHeight) && (nZ < (nScreenHeight - m_nHalfSpriteHeight)))
		return (TRUE8);
	else
		return (FALSE8);

}

} // End of namespace ICB

#endif // #if !defined( REMORA_SPRITE_H_INCLUDED )
