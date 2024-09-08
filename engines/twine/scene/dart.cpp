/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "twine/scene/dart.h"
#include "twine/audio/sound.h"
#include "twine/renderer/redraw.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/scene.h"

namespace TwinE {

void Dart::InitDarts() {
#if 0
	T_DART *ptrd;
	uint8 *ptrbody;
	uint32 t;
	int32 x0, x1, y0, y1, z0, z1;
	int32 size;
	T_BODY_HEADER *ptr;

	ptrbody = (uint8 *)GivePtrObjFix(BODY_3D_DART);
	if (!ptrbody) {
		char tmpFilePath[ADELINE_MAX_PATH];
		GetResPath(tmpFilePath, ADELINE_MAX_PATH, OBJFIX_HQR_NAME);
		TheEndCheckFile(tmpFilePath);
	}
	// Calcule ZV des flechettes
	ptr = (T_BODY_HEADER *)ptrbody;

	x0 = ptr->XMin;
	x1 = ptr->XMax;
	y0 = ptr->YMin;
	y1 = ptr->YMax;
	z0 = ptr->ZMin;
	z1 = ptr->ZMax;

	// Average
	size = ((x1 - x0) + (z1 - z0)) / 4;

	ptrd = ListDart;

	for (t = 0; t < MAX_DARTS; t++, ptrd++) {
		ptrd->Body = BODY_3D_DART;

		ptrd->XMin = -size;
		ptrd->XMax = size;
		ptrd->YMin = y0;
		ptrd->YMax = y1;
		ptrd->ZMin = -size;
		ptrd->ZMax = size;

		ptrd->Flags = 0;
		ptrd->NumCube = -1;
	}
#endif
}

int32 Dart::GetDart() {
	T_DART *ptrd;
	int32 t;

	ptrd = ListDart;

	for (t = 0; t < MAX_DARTS; t++, ptrd++) {
		if (ptrd->Flags & DART_TAKEN) {
			return t;
		}
	}

	return -1;
}

void Dart::TakeAllDarts() {
	T_DART *ptrd;
	int32 n;

	ptrd = ListDart;

	for (n = 0; n < MAX_DARTS; n++, ptrd++) {
		ptrd->Flags |= DART_TAKEN;
	}

	_engine->_gameState->setDarts(MAX_DARTS);
}

void Dart::CheckDartCol(ActorStruct *ptrobj) {
	int32 n;
	T_DART *ptrd;
	int32 x0, y0, z0, x1, y1, z1;
	int32 xt0, yt0, zt0, xt1, yt1, zt1;

	if (ptrobj->_staticFlags.bIsInvisible)
		return;

	x0 = ptrobj->_posObj.x + ptrobj->_boundingBox.mins.x;
	x1 = ptrobj->_posObj.x + ptrobj->_boundingBox.maxs.x;
	y0 = ptrobj->_posObj.y + ptrobj->_boundingBox.mins.y;
	y1 = ptrobj->_posObj.y + ptrobj->_boundingBox.maxs.y;
	z0 = ptrobj->_posObj.z + ptrobj->_boundingBox.mins.z;
	z1 = ptrobj->_posObj.z + ptrobj->_boundingBox.maxs.z;

	ptrd = ListDart;

	for (n = 0; n < MAX_DARTS; n++, ptrd++) {
		if (ptrd->NumCube == _engine->_scene->_currentSceneIdx && !(ptrd->Flags & DART_TAKEN)) {
			xt0 = ptrd->PosX + ptrd->XMin;
			xt1 = ptrd->PosX + ptrd->XMax;
			yt0 = ptrd->PosY + ptrd->YMin;
			yt1 = ptrd->PosY + ptrd->YMax;
			zt0 = ptrd->PosZ + ptrd->ZMin;
			zt1 = ptrd->PosZ + ptrd->ZMax;

			if (x0 < xt1 && x1 > xt0 && y0 < yt1 && y1 > yt0 && z0 < zt1 && z1 > zt0) {
				ptrd->Flags |= DART_TAKEN;

				_engine->_gameState->addDart();

#if 0
				_engine->_sound->playSample(SAMPLE_BONUS_TROUVE, 0x1000, 0, 1,
											ptrd->PosX, ptrd->PosY, ptrd->PosZ);

				_engine->_redraw->addOverlay(OverlayType::koSprite | INCRUST_YCLIP,
											 SPRITE_DART,
											 15, 30,
											 0, 0, 2);
#endif
			}
		}
	}
}

} // namespace TwinE
