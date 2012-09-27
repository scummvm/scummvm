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

#ifndef HOPKINS_TALK_H
#define HOPKINS_TALK_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

class TalkManager {
private:
	HopkinsEngine *_vm;
public:
	Common::String FQUEST;
	Common::String FREPON;
	byte *BUFFERPERSO;
	byte *PALPERSO;
	byte *PERSOSPR;
	size_t TAILLEPERSO;
	int STATI;
	int PLIGNE1, PLIGNE2;
	int PLIGNE3, PLIGNE4;
	int PCHERCHE;
public:
	void setParent(HopkinsEngine *vm);

	void PARLER_PERSO2(const Common::String &filename);
	void RENVOIE_FICHIER(int srcStart, Common::String &dest, const char *srcData);
	int DIALOGUE();
	int DIALOGUE_REP(int idx);
	void CHERCHE_PAL(int a1, int a2);
	void VISU_WAIT();
	void FIN_VISU_WAIT();
	void FIN_VISU_PARLE(int a1);
	int VERIF_BOITE(__int16 a1, const Common::String &a2, __int16 a3);
	void VISU_PARLE();
	void BOB_VISU_PARLE(int idx);
};

} // End of namespace Hopkins

#endif /* HOPKINS_TALK_H */
