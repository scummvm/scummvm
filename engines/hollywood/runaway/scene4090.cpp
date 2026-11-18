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
	{"Exhibition hall", 1100, 204, 1, {302, 301}, {0, 3}},
	{"Statue", 826, 442, 3, {303, 304}, {1, 0}},
	{"Statue", 572, 420, 1, {305, 306}, {1, 0}},
	{"Mayan objects", 572, 420, 1, {307, 308}, {1, 1}},
	{"Mayan arch", 316, 382, 7, {309, 310}, {0, 0}},
	{"Mayan arch", 532, 238, 0, {311, 312}, {0, 0}},
	{"Mayan object", 280, 491, 7, {313, 314}, {1, 0}},
	{"Mayan object", 302, 426, 7, {315, 316}, {1, 0}},
	{"Mayan object", 316, 382, 7, {317, 318}, {1, 0}},
};

const MuseumResponse kResponses[] = {
	{kMuseumExhibition, 302, 1, 0},
	{kMuseumExhibition, 303, 2, 0},
	{kMuseumExhibition, 304, 3, 0},
	{kMuseumExhibition, 305, 4, 0},
	{kMuseumExhibition, 306, 3, 0},
	{kMuseumExhibition, 307, 5, 0},
	{kMuseumExhibition, 309, 7, 0},
	{kMuseumExhibition, 310, 8, 0},
	{kMuseumExhibition, 311, 9, 0},
	{kMuseumExhibition, 312, 8, 0},
	{kMuseumExhibition, 313, 10, 0},
	{kMuseumExhibition, 314, 11, 0},
	{kMuseumExhibition, 315, 12, 0},
	{kMuseumExhibition, 316, 11, 0},
	{kMuseumExhibition, 317, 13, 0},
	{kMuseumExhibition, 318, 11, 0},
};

const MuseumExit kExits[] = {
	{kMuseumExhibition, 301, kMuseumHall, 1400, 600, false, false},
};

} // End of anonymous namespace

const MuseumRoom kRunawayScene4090 = {
	kMuseumExhibition, 1124, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {70, 70, 90}, {60, 60, 80}, {50, 50, 70}},
	Common::Point(881, 392),
	kResponses, ARRAYSIZE(kResponses),
	nullptr, 0,
	kExits, ARRAYSIZE(kExits)
};

float RunawayMuseumScene::actorHeight4090() const {
	const int y = _position.y;
	return MIN(y, 390) * 0.2f + 122;
}

void RunawayMuseumScene::setupDepth4090(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 374;
}

} // End of namespace Hollywood
