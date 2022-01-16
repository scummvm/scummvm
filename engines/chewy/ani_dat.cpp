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

#include "chewy/defines.h"
#include "chewy/global.h"

namespace Chewy {

#define STERNE_ANI 17
#define TUER_ZU_ANI 3
#define GITTER_BLITZEN 7

AniBlock ablock0[5] = {
	{ 6, 1, ANI_VOR, ANI_WAIT, 0},
	{ 7, 3, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock1[2] = {
	{10, 1, ANI_VOR, ANI_WAIT, 0},
	{13, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock2[2] = {
	{12, 1, ANI_VOR, ANI_WAIT, 0},
	{GITTER_BLITZEN, 1, ANI_VOR, ANI_GO, 0}
};

AniBlock ablock3[2] = {
	{ 4, 2, ANI_VOR, ANI_GO, 0},
	{11, 255, ANI_VOR, ANI_GO, 0}
};

AniBlock ablock4[2] = {
	{GITTER_BLITZEN, 3, ANI_VOR, ANI_WAIT, 0},
	{GITTER_BLITZEN, 12, ANI_VOR, ANI_GO, 0}
};

AniBlock ablock5[3] = {
	{ 0, 1, ANI_VOR, ANI_WAIT, 0},
	{ 8, 9, ANI_VOR, ANI_GO, 0},
	{ 1, 1, ANI_VOR, ANI_WAIT, 0},

};

AniBlock ablock6[3] = {
	{ 3, 1, ANI_VOR, ANI_GO, 0 },
	{13, 1, ANI_VOR, ANI_WAIT, 0},
	{14, 4, ANI_VOR, ANI_GO, 0},
};

AniBlock ablock7[2] = {
	{ 1, 2, ANI_VOR, ANI_WAIT, 0},
	{ 2, 1, ANI_VOR, ANI_GO, 0},
};

AniBlock ablock8[3] = {
	{ 10, 1, ANI_VOR, ANI_WAIT, 0},
	{ 12, 1, ANI_VOR, ANI_WAIT, 0},
	{ 11, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock9[3] = {
	{ 10, 1, ANI_VOR, ANI_WAIT, 0},
	{ 13, 1, ANI_VOR, ANI_WAIT, 0},
	{ 11, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock10[4] = {

	{ 15, 2, ANI_VOR, ANI_WAIT, 0},
	{ 16, 1, ANI_VOR, ANI_WAIT, 0},
	{ 17, 2, ANI_VOR, ANI_WAIT, 0},
	{ 18, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock17[2] = {
	{8, 1, ANI_VOR, ANI_WAIT, 0},
	{9, 255, ANI_VOR, ANI_GO, 0},
};

AniBlock ablock18[2] = {
	{7, 1, ANI_VOR, ANI_WAIT, 0},
	{7, 1, ANI_RUECK, ANI_WAIT, 0},
};

AniBlock ablock16[2] = {
	{0, 1, ANI_VOR, ANI_WAIT, 0},
	{1, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock20[3] = {
	{5, 3, ANI_VOR, ANI_GO, 0},
	{6, 3, ANI_VOR, ANI_GO, 0},
	{7, 3, ANI_VOR, ANI_WAIT, 0}
};

AniBlock ablock21[2] = {
	{8, 1, ANI_VOR, ANI_WAIT, 0},
	{23, 255, ANI_RUECK, ANI_GO, 0},
};

AniBlock ablock22[2] = {
	{13, 1, ANI_VOR, ANI_WAIT, 0},
	{14, 13, ANI_VOR, ANI_GO, 0},
};

AniBlock ablock23[2] = {
	{11, 1, ANI_VOR, ANI_WAIT, 0},
	{12, 1, ANI_VOR, ANI_GO, 0},
};

AniBlock ablock24[2] = {
	{15, 1, ANI_VOR, ANI_WAIT, 0},
	{16, 7, ANI_VOR, ANI_GO, 0},
};

AniBlock ablock19[3] = {
	{12, 1, ANI_VOR, ANI_WAIT, 0},
	{13, 4, ANI_VOR, ANI_WAIT, 0},
	{14, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock15[4] = {
	{0, 2, ANI_VOR, ANI_GO, 0},
	{1, 255, ANI_VOR, ANI_GO, 0},
	{2, 255, ANI_VOR, ANI_GO, 0},
	{3, 255, ANI_VOR, ANI_GO, 0},
};

AniBlock ablock26[4] = {
	{2, 255, ANI_VOR, ANI_GO, 0},
	{0, 1, ANI_VOR, ANI_WAIT, 0},
	{0, 1, ANI_RUECK, ANI_WAIT, 0},
	{0, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock27[5] = {
	{0, 1, ANI_RUECK, ANI_WAIT, 0},
	{0, 1, ANI_VOR, ANI_WAIT, 0},
	{0, 1, ANI_RUECK, ANI_WAIT, 0},
	{0, 1, ANI_VOR, ANI_WAIT, 0},
	{0, 1, ANI_RUECK, ANI_WAIT, 0},
};

AniBlock ablock30[3] = {
	{0, 1, ANI_VOR, ANI_WAIT, 0},
	{1, 4, ANI_VOR, ANI_WAIT, 0},
	{2, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock28[2] = {
	{4, 1, ANI_VOR, ANI_WAIT, 0},
	{5, 3, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock31[3] = {
	{0, 1, ANI_VOR, ANI_GO, 0},
	{1, 2, ANI_VOR, ANI_GO, 0},
	{5, 1, ANI_RUECK, ANI_WAIT, 0},
};

AniBlock ablock29[2] = {
	{8, 1, ANI_VOR, ANI_WAIT, 0},
	{9, 4, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock33[2] = {
	{2, 1, ANI_VOR, ANI_WAIT, 0},
	{3, 255, ANI_VOR, ANI_GO, 0},
};

AniBlock ablock32[2] = {

	{6, 255, ANI_VOR, ANI_GO, 0},
	{2, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock34[2] = {
	{7, 1, ANI_VOR, ANI_WAIT, 0},
	{6, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock36[2] = {
	{6, 1, ANI_VOR, ANI_WAIT, 0},
	{7, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock37[5] = {
	{6, 1, ANI_VOR, ANI_WAIT, 0},
	{7, 3, ANI_VOR, ANI_WAIT, 0},
	{6, 1, ANI_RUECK, ANI_WAIT, 0},
	{5, 1, ANI_RUECK, ANI_WAIT, 0},
	{2, 1, ANI_RUECK, ANI_WAIT, 0},
};

AniBlock ablock35[7] = {
	{2, 1, ANI_VOR, ANI_WAIT, 0},
	{7, 1, ANI_VOR, ANI_WAIT, 0},
	{8, 3, ANI_VOR, ANI_WAIT, 0},
	{4, 1, ANI_VOR, ANI_WAIT, 0},
	{5, 1, ANI_VOR, ANI_WAIT, 0},
	{9, 1, ANI_VOR, ANI_WAIT, 0},
	{5, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock38[2] = {
	{5, 1, ANI_VOR, ANI_WAIT, 0},
	{6, 1, ANI_VOR, ANI_WAIT, 0},
};

AniBlock ablock39[3] = {
	{15, 1, ANI_VOR, ANI_WAIT, 0},
	{17, 1, ANI_VOR, ANI_WAIT, 0},
	{15, 1, ANI_RUECK, ANI_WAIT, 0},
};

} // namespace Chewy
