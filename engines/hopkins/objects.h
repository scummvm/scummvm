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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_OBJECTS_H
#define HOPKINS_OBJECTS_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

class ObjectsManager {
private:
	HopkinsEngine *_vm;

	int PRIORITY;
public:
	ObjectsManager();
	void setParent(HopkinsEngine *vm);

	byte *CHANGE_OBJET(int objIndex);
	byte *CAPTURE_OBJET(int objIndex, int mode);

	int Get_Largeur(const byte *objectData, int idx);
	int Get_Hauteur(const byte *objectData, int idx);
	int sprite_alone(const byte *objectData, byte *sprite, int objIndex);
	byte *DEL_FICHIER_OBJ();

	byte *CHARGE_SPRITE(const Common::String &file);
	void set_offsetxy(byte *data, int idx, int xp, int yp, bool isSize);

	int capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex);
	int AJOUTE_OBJET(int objIndex);

	void INIT_BOB();
	void BOB_ZERO(int idx);
	void BOB_VISU(int idx);
	void BOB_OFF(int idx);
};

} // End of namespace Hopkins

#endif /* HOPKINS_OBJECTS_H */
