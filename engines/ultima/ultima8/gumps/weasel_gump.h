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

#ifndef ULTIMA8_GUMPS_REMORSEMENUGUMP_H
#define ULTIMA8_GUMPS_REMORSEMENUGUMP_H

#include "ultima/shared/std/string.h"
#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

class WeaselDat;

/**
 * Weasel weapon seller Crusader.
 */
class WeaselGump : public ModalGump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	enum WeaselGumpState {
		kWeaselStart,
		kWeaselConfirmPurchaseMovie,
		kWeaselConfirmPurchaseText,
		kWeaselCancelledPurchaseMovie,
		kWeaselCancelledPurchaseText,
		kWeaselCompletedPurchase,
		kWeaselInsufficientFunds,
		kWeaselBrowsing,
		kWeaselClosing,
		kWeaselCheckBuyMoreMovie,
		kWeaselCheckBuyMoreText,
		kWeaselShowIntro
	};

	WeaselGump(uint16 level);
	~WeaselGump() override;

	// Init the gump, call after construction
	void InitGump(Gump *newparent, bool take_focus = true) override;
	void Close(bool no_del = false) override;

	void run() override;

	// Paint the Gump
	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	bool OnKeyDown(int key, int mod) override;
	bool OnTextInput(int unicode) override;
	void ChildNotify(Gump *child, uint32 message) override;

	static uint32 I_showWeaselGump(const uint8 *args, unsigned int /*argsize*/);

private:

	void onButtonClick(int entry);

	void prevItem();
	void nextItem();
	void buyItem();
	void updateForAmmoMode();
	void checkClose();
	void completePurchase();
	void abortPurchase();
	void checkBuyMore();
	void confirmPurchase();
	void setYesNoQuestion(const Std::string &msg);
	void browsingMode(bool browsing);
	int purchasedCount(uint16 shape) const;

	void updateItemDisplay();
	Gump *playMovie(const Std::string &filename);

	/// Gump to hold all the UI items (not the movies)
	Gump *_ui;

	/// Gump for playing movies
	Gump *_movie;

	/// The menu of items on offer
	uint16 _level;

	/// Current gump state
	WeaselGumpState _state;

	const WeaselDat *_weaselDat;

	/// Remaining balance including pending purchases
	int32 _credits;

	/// The list of pending purchases (shape nums)
	Std::vector<uint16> _purchases;

	/// The current item num being browsed
	int _curItem;

	/// Whether we're browsing ammo or weapons
	bool _ammoMode;

	/// Cost of current item
	int32 _curItemCost;

	/// Shape of current item
	uint16 _curItemShape;

	/**
	 * Whether the first intro movie has been played. Remember this between
	 * displays of the gump, but don't save.
	 *
	 * It only ever shows on level 1 so there is only a single way it can be re-shown
	 * (save during the first time at the base and reload after restarting the game).
	 *
	 * TODO: test if original does this to be perfectly faithful
	 */
	static bool _playedIntroMovie;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
