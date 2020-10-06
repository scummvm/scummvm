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

#include "engines/icb/remora_sprite.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/res_man.h"
#include "engines/icb/remora.h"

namespace ICB {

void _remora_sprite::GenericSpriteDraw(int32 nX, int32 nY, bool8 bPosition, const _rs_params *pParams) {
	_pxBitmap *psBitmap;
	_pxSprite *psSprite;
	uint8 *pSurfaceBitmap;
	uint32 nPitch;
	uint32 nSurfaceWidth, nSurfaceHeight;
	uint32 nTransparencyKey;
	uint8 nOpacity;
	bool8 bCentre, bUpdate;

	// Get the key to use for transparency.
	nTransparencyKey = g_oIconMenu->GetTransparencyKey();

	// Set function parameters from the parameters block if there is one.
	if (pParams) {
		// There is a parameter block, so get the values from there.
		bCentre = pParams->bCentre;
		nOpacity = pParams->nOpacity;
		bUpdate = pParams->bUpdate;
	} else {
		// No params block, so match the defaults that would be contained in it if user had not set a value.
		bCentre = TRUE8;
		nOpacity = 255;
		bUpdate = TRUE8;
	}

	// Open the resource and get the current frame.
	psBitmap = (_pxBitmap *)rs_remora->Res_open(m_pcName, m_nNameHash, m_pcClusterName, m_nClusterHash);

	if (psBitmap->schema != PC_BITMAP_SCHEMA)
		Fatal_error("Incorrect versions loading [%s] (engine has %d, data has %d", m_pcName, PC_BITMAP_SCHEMA, psBitmap->schema);

	psSprite = psBitmap->Fetch_item_by_number(m_nFramePC);

	// Lock the surface and get the pitch.
	uint32 remoraSurfaceId = g_oRemora->GetRemoraSurfaceId();
	pSurfaceBitmap = surface_manager->Lock_surface(remoraSurfaceId);
	nSurfaceWidth = surface_manager->Get_width(remoraSurfaceId);
	nSurfaceHeight = surface_manager->Get_height(remoraSurfaceId);
	nPitch = surface_manager->Get_pitch(remoraSurfaceId);

	// Draw at sprite position or user-supplied position.
	if (bPosition)
		SpriteXYFrameDraw(pSurfaceBitmap, nPitch, nSurfaceWidth, nSurfaceHeight, psBitmap, nX, nY, m_nFramePC, bCentre, &nTransparencyKey, nOpacity);
	else
		SpriteFrameDraw(pSurfaceBitmap, nPitch, nSurfaceWidth, nSurfaceHeight, psBitmap, m_nFramePC, &nTransparencyKey, nOpacity);

	// Unlock the surface.
	surface_manager->Unlock_surface(remoraSurfaceId);

	// Update the frame counter if required.
	if (bUpdate)
		m_nFramePC = (m_nFramePC + 1) % m_nNumFrames;
}

uint32 _remora_sprite::GetHeight() {
	_pxBitmap *psBitmap;
	_pxSprite *psSprite;

	// Use the count of frames as an indicator whether or not the object has been initialised for use.
	if (m_nNumFrames == 0)
		return (0);

	// Need to open the actual resource to get its size.
	psBitmap = (_pxBitmap *)rs_remora->Res_open(m_pcName, m_nNameHash, m_pcClusterName, m_nClusterHash);

	// Get the first frame and return its height.
	psSprite = psBitmap->Fetch_item_by_number(0);
	return (psSprite->height);
}

uint32 _remora_sprite::GetWidth() {
	_pxBitmap *psBitmap;
	_pxSprite *psSprite;

	// Use the count of frames as an indicator whether or not the object has been initialised for use.
	if (m_nNumFrames == 0)
		return (0);

	// Need to open the actual resource to get its size.
	psBitmap = (_pxBitmap *)rs_remora->Res_open(m_pcName, m_nNameHash, m_pcClusterName, m_nClusterHash);

	// Get the first frame and return its height.
	psSprite = psBitmap->Fetch_item_by_number(0);
	return (psSprite->width);
}

} // End of namespace ICB
