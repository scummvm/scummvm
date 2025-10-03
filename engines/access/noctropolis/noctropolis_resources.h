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

#ifndef ACCESS_NOCTROPOLIS_NOCTROPOLIS_RESOURCES_H
#define ACCESS_NOCTROPOLIS_NOCTROPOLIS_RESOURCES_H

#include "access/resources.h"

namespace Access {

class Font;

namespace Noctropolis {

class NoctropolisEngine;

class NoctropolisResources : public Resources {
public:
	NoctropolisResources(AccessEngine *_vm);
	~NoctropolisResources();

	const byte *getCursor(int num) const override;
	const char *getEgoName() const override;
	int getRMouse(int i, int j) const override { return 0; };
	int inButtonXRange(int x) const override { return 0; };

	void load(Common::SeekableReadStream &s) override;

	int getCursorWidth(int num) const override;
	int getCursorHeight(int num) const override;

private:
	Font *_fontChaleteu;
	Font *_fontSystemeu;
	Font *_fontSml3x5;
	Font *_fontNaples12;
	Font *_fontGothiceu;
	Font *_fontChaletse;
	Font *_fontComicseu;
};

} // end namespace Noctropolis

} // end namespace Access

#endif // ACCESS_NOCTROPOLIS_NOCTROPOLIS_RESOURCES_H
