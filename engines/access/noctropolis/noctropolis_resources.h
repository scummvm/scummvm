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
#include "access/polygon.h"

namespace Access {

class Font;
namespace Noctropolis {

class NoctropolisEngine;
class ComicResource;

extern const byte ICON_PALETTE[];

extern const int16 MENU_POLYS[];

extern const int16 TRAV_ROOMS[];

extern const int16 TRAV_MAN_POS[];

extern const int16 TRAV_ICONS[];

class NoctropolisResources : public Resources {
public:
	NoctropolisResources(AccessEngine *vm);
	~NoctropolisResources();

	const byte *getCursor(int num) const override;
	const char *getEgoName() const override;
	int getRMouse(int i, int j) const override { return 0; };
	int inButtonXRange(int x) const override { return 0; };

	void load(Common::SeekableReadStream &s) override;

	int getCursorWidth(int num) const override;
	int getCursorHeight(int num) const override;

	const char *getGeneralMessage(int command) const override;

	const char *getEndMessage() const;
	const char *getMeanwhileMessage() const;
	const char *getShotoTitle() const;
	const char *getShotoText() const;
	const char *getStilMessage() const;
	const char *getResponseTitle() const;
	const char *getAskItem(int num) const;
	const char *getMoreItemsText() const;
	const char *getPlaceName(int num) const;
	const char *getDeathTitle() const;
	const char *getImprisonedTitle() const;
	const char *getDeathText(int num) const;

	const ComicResource *getLastComicResource() const;
	const ComicResource *getSpecialComicResource() const;

	int menuAt(int16 x, int16 y) const;

private:
	Font *_fontChaleteu;
	Font *_fontSystemeu;
	Font *_fontSml3x5;
	Font *_fontNaples12;
	Font *_fontGothiceu;
	Font *_fontChaletse;
	Font *_fontComicseu;

	PolygonArray _menus;
};

} // end namespace Noctropolis

} // end namespace Access

#endif // ACCESS_NOCTROPOLIS_NOCTROPOLIS_RESOURCES_H
