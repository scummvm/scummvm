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

#include "common/util.h"
#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {
namespace {

const MuseumHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Door", 141, 468, 7, {301, 302}, {0, 1}},
	{"First aid kit", 196, 451, 1, {303, 304}, {0, 0}},
	{"Glass display case", 310, 457, 1, {305, 306}, {1, 1}},
	{"Button", 204, 448, 1, {307, 308}, {1, 1}},
	{"Mayan mask", 310, 457, 1, {309, 310}, {1, 1}},
	{"Scanner", 912, 435, 0, {311, 312}, {0, 0}},
	{"Tank", 1352, 431, 1, {315, 316}, {0, 0}},
	{"Golden object", 111, 446, 1, {313, 314}, {1, 1}},
	{"Device", 539, 439, 1, {317, 318}, {0, 0}},
	{"Ladel", 1329, 409, 1, {328, 329}, {0, 1}},
};

const MuseumResponse kResponses[] = {
	{kMuseumSecurity, 301, 1, 0},
	{kMuseumSecurity, 303, 2, 0},
	{kMuseumSecurity, 304, 3, 0},
	{kMuseumSecurity, 307, 6, 0},
	{kMuseumSecurity, 312, 11, 0},
	{kMuseumSecurity, 316, 15, 0},
	{kMuseumSecurity, 318, 17, 0},
	{kMuseumSecurity, 319, 18, 0},
	{kMuseumSecurity, 322, 21, 0},
	{kMuseumSecurity, 323, 11, 0},
	{kMuseumSecurity, 324, 22, 0},
	{kMuseumSecurity, 326, 24, 0},
	{kMuseumSecurity, 327, 25, 0},
	{kMuseumSecurity, 328, 26, 0},
};

const MuseumItemAction kItemActions[] = {
	{kMuseumSecurity, 5, 3, 321},
	{kMuseumSecurity, 11, 3, 320},
	{kMuseumSecurity, 0, 3, 319},
	{kMuseumSecurity, 5, 4, 321},
	{kMuseumSecurity, 7, 6, 323},
	{kMuseumSecurity, 0, 6, 322},
	{kMuseumSecurity, 6, 7, 330},
	{kMuseumSecurity, 18, 7, 325},
	{kMuseumSecurity, 0, 7, 324},
	{kMuseumSecurity, 7, 9, 327},
	{kMuseumSecurity, 0, 9, 326},
};

const MuseumExit kExits[] = {
	{kMuseumSecurity, 302, kMuseumHall, 1118, 309, true}
};

} // End of anonymous namespace

const MuseumRoom kRunawayScene4100 = {
	kMuseumSecurity, 1536, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {40, 40, 60}, {30, 30, 50}, {20, 20, 40}},
	Common::Point(141, 468),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	kExits, ARRAYSIZE(kExits)
};

float RunawayMuseumScene::actorHeight4100() const {
	const int x = _position.x, y = _position.y;
	if (x > 1260)
		return MIN(y, 545) * 1.2807018f - 242.982452f;
	if (x > 645)
		return MIN(y, 545) * 0.73333335f + 55.333332f;
	return MIN(y, 545) * 0.9381443f - 56.288658f;
}

void RunawayMuseumScene::setupDepth4100(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 477;
}

} // End of namespace Hollywood
