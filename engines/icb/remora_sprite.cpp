/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/icb/remora_sprite.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/res_man.h"

namespace ICB {

void _remora_sprite::InitialiseFromBitmapName(const char *pcBitmapName, const char *pcClusterName, uint32 nClusterHash) {
	_pxBitmap *psBitmap;

	_pxSprite *psSprite;

	// Set name.
	strcpy(m_pcName, pcBitmapName);
	m_nNameHash = NULL_HASH;
	strcpy(m_pcClusterName, pcClusterName);
	m_nClusterHash = nClusterHash;

	// Get the number of frames (don't forget to check schema number is correct).
	psBitmap = (_pxBitmap *)rs_remora->Res_open(m_pcName, m_nNameHash, m_pcClusterName, m_nClusterHash);

	m_nNumFrames = FROM_LE_32(psBitmap->num_sprites);

	if (m_nNumFrames == 0)
		Fatal_error("Bitmap %s has no frames.", pcBitmapName);

	// Here we work out half the sprite's width and height so we can avoid plotting it in positions where it
	// would run off the edge of a surface.  Note that this is based on the first frame; it would need to be
	// made more sophisticated to deal with sprites that change in size as they are played.
	psSprite = (_pxSprite *)((byte *)psBitmap + FROM_LE_32(psBitmap->sprite_offsets[0]));

	m_nHalfSpriteWidth = FROM_LE_32(psSprite->width) / 2;
	m_nHalfSpriteHeight = FROM_LE_32(psSprite->height) / 2;

	// Frame PC starts at 0.
	m_nFramePC = 0;
}

_remora_sprite::_remora_sprite() {
	m_nFramePC = 0;
	m_nNumFrames = 0;

	m_nNameHash = NULL_HASH;

	if (MAXLEN_CLUSTER_URL)
		memset((char *)m_pcClusterName, 0, MAXLEN_CLUSTER_URL * sizeof(char));

	m_nClusterHash = NULL_HASH;

	m_nHalfSpriteWidth = 0;
	m_nHalfSpriteHeight = 0;
}

_remora_sprite::_remora_sprite(const _remora_sprite &oX) {
	m_nFramePC = oX.m_nFramePC;
	m_nNumFrames = oX.m_nNumFrames;

	strcpy(m_pcName, oX.m_pcName);
	m_nNameHash = oX.m_nNameHash;
	strcpy(m_pcClusterName, oX.m_pcClusterName);
	m_nClusterHash = oX.m_nClusterHash;
}

const _remora_sprite &_remora_sprite::operator=(const _remora_sprite &oOpB) {
	m_nFramePC = oOpB.m_nFramePC;
	m_nNumFrames = oOpB.m_nNumFrames;

	strcpy(m_pcName, oOpB.m_pcName);
	m_nNameHash = oOpB.m_nNameHash;
	strcpy(m_pcClusterName, oOpB.m_pcClusterName);
	m_nClusterHash = oOpB.m_nClusterHash;

	return (*this);
}

} // End of namespace ICB
