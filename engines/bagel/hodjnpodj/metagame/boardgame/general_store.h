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

#ifndef HODJNPODJ_METAGAME_GENERAL_STORE_H
#define HODJNPODJ_METAGAME_GENERAL_STORE_H

#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/views/dialog.h"
#include "bagel/hodjnpodj/gfx/button.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

class GeneralStore : public Dialog {
private:
	OkButton _okButton;
	Common::Rect _scrollTopRect, _scrollBottomRect;
	Common::Rect _titleRect, _blurbRect;
	int nFirstSlot = 0;
	int nDialog_DX = 0, nDialog_DY = 0;
	int nItemsPerRow = 0;
	int nItemsPerColumn = 0;
	int nItem_DDX = 0, nItem_DDY = 0;
	int _selectedIndex = -1;
	CItem *_selectedItem = nullptr;
	CInventory *pGeneralStore = nullptr;
	CInventory *pInventory = nullptr;
	int _buyMessageCtr = 0;
	Common::String _buyMessage;
	CBofSound *_buySound = nullptr;
	bool bPlayingHodj = false;

	void updateContent();
	Common::Rect getItemRect(int index) const;
	void drawItems(GfxSurface &s);
	void drawItem(GfxSurface &s, CItem *pItem, int nX, int nY);
	void drawMore(GfxSurface &s);
	void drawBlurb(GfxSurface &s);
	bool hasPriorPage() const {
		return nFirstSlot > 0;
	}
	bool hasNextPage() const;
	int getItemAtPos(const Common::Point &point) const;
	void purchaseItem();

public:
	GeneralStore();
	~GeneralStore() override {}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
	bool tick() override;

	void draw() override;

	static void show(CInventory *pStore, CInventory *pInvent);
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
