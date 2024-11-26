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

#include "ultima/ultima8/gumps/weasel_gump.h"
#include "ultima/ultima8/gumps/weasel_dat.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/gumps/movie_gump.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item_factory.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(WeaselGump)

static const uint16 WEASEL_CANT_BUY_SFXNO = 0xb0;
static const int WEASEL_FONT = 6;
static const int WEASEL_SHAPE_TOP = 22;

enum WeaselUiElements {
	kBtnLeft = 0,
	kBtnBlank = 1,
	kBtnRight = 2,
	kBtnYes = 3,
	kBtnNo = 4,
	kBtnBuy = 5,
	kBtnAmmo = 6,
	kBtnWeapons = 7,
	kBtnExit = 8,
	kTxtCredits = 9,
	kIconItem = 10,
	kTxtItemName = 11,
	kTxtItemCost = 12,
	kTxtItemPurch = 13,
	kTxtItemOwned = 14,
	kTxtQuestion = 15
};
// Coords and shapes for above list of buttons
static const int WEASEL_BTN_X[] = { 14,  76, 138,  18, 113,  20,  19,  19,  44};
static const int WEASEL_BTN_Y[] = {213, 213, 213, 237, 237, 280, 319, 319, 368};
static const int WEASEL_BTN_SHAPES[] = {13, 26, 14, 16, 15, 28, 27, 83, 29};

static const char *const FIRST_INTRO_MOVIE = "17A";
static const char *const INTRO_MOVIES[] = {"18A", "18B", "18C"};
static const char *const BUYMORE_MOVIES[] = {"21A", "21B"};
static const char *const CONFIRM_BUY_MOVIES[] = {"21A", "21B"};
static const char *const CANCELLED_PURCHASE_MOVIES[] = {"19C", "19D"};
static const char *const COMPLETED_PURCHASE_MOVIES[] = {"21C", "21D"};
static const char *const INSUFFICIENT_FUND_MOVIES[] = {"20C", "20D"};


namespace {
// A small container gump that doesn't do anything except pass notifications to the parent
class WeaselUIContainerGump : public Gump {
	void ChildNotify(Gump *child, uint32 message) override {
		_parent->ChildNotify(child, message);
	}
};

static void _closeIfExists(Gump *gump) {
	if (gump)
		gump->Close();
}

static const char *_getRandomMovie(const char *const *movies, int nmovies) {
	Common::RandomSource &rs = Ultima8Engine::get_instance()->getRandomSource();
	int offset = rs.getRandomNumber(nmovies - 1);
	return movies[offset];
}
}

bool WeaselGump::_playedIntroMovie = false;

WeaselGump::WeaselGump(uint16 level)
	: ModalGump(0, 0, 640, 480), _credits(0), _level(level),
	  _state(kWeaselStart), _curItem(0), _ammoMode(false), _curItemCost(1),
	  _curItemShape(0), _ui(nullptr), _movie(nullptr), _weaselDat(nullptr) {
	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor(Mouse::MOUSE_HAND);
}

WeaselGump::~WeaselGump() {
}


void WeaselGump::Close(bool no_del) {
	Mouse *mouse = Mouse::get_instance();
	mouse->popMouseCursor();
	ModalGump::Close(no_del);
}

void WeaselGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	GumpShapeArchive *shapeArchive = GameData::get_instance()->getGumps();

	const Shape *top = shapeArchive->getShape(WEASEL_SHAPE_TOP);
	const Shape *midhi = shapeArchive->getShape(WEASEL_SHAPE_TOP + 1);
	const Shape *midlo = shapeArchive->getShape(WEASEL_SHAPE_TOP + 2);
	const Shape *bot = shapeArchive->getShape(WEASEL_SHAPE_TOP + 3);

	if (!top || !midhi || !midlo || !bot) {
		error("Couldn't load shapes for weasel");
		return;
	}

	const ShapeFrame *tFrame = top->getFrame(0);
	const ShapeFrame *mhFrame = midhi->getFrame(0);
	const ShapeFrame *mlFrame = midlo->getFrame(0);
	const ShapeFrame *bFrame = bot->getFrame(0);
	if (!tFrame || !mhFrame || !mlFrame || !bFrame) {
		error("Couldn't load shape frames for weasel");
		return;
	}

	_ui = new WeaselUIContainerGump();
	_ui->SetDims(Rect(0, 0, mhFrame->_width,
					  tFrame->_height + mhFrame->_height + mlFrame->_height + bFrame->_height));
	_ui->InitGump(this, false);
	_ui->setRelativePosition(CENTER);

	Gump *tGump = new Gump(3, 0, tFrame->_width, tFrame->_height);
	tGump->SetShape(top, 0);
	tGump->InitGump(_ui, false);
	Gump *mhGump = new Gump(0, tFrame->_height, mhFrame->_width, mhFrame->_height);
	mhGump->SetShape(midhi, 0);
	mhGump->InitGump(_ui, false);
	Gump *mlGump = new Gump(5, tFrame->_height + mhFrame->_height, mlFrame->_width, mlFrame->_height);
	mlGump->SetShape(midlo, 0);
	mlGump->InitGump(_ui, false);
	Gump *bGump = new Gump(9, tFrame->_height + mhFrame->_height + mlFrame->_height, bFrame->_width, bFrame->_height);
	bGump->SetShape(bot, 0);
	bGump->InitGump(_ui, false);

	for (int i = 0; i < ARRAYSIZE(WEASEL_BTN_X); i++) {
		uint32 buttonShapeNum = WEASEL_BTN_SHAPES[i];
		const Shape *buttonShape = shapeArchive->getShape(buttonShapeNum);
		if (!buttonShape) {
			error("Couldn't load shape for weasel button %d", i);
			return;
		}

		const ShapeFrame *buttonFrame = buttonShape->getFrame(0);
		if (!buttonFrame || buttonShape->frameCount() != 2) {
			error("Couldn't load shape frame for weasel button %d", i);
			return;
		}

		FrameID frame_up(GameData::GUMPS, buttonShapeNum, 0);
		FrameID frame_down(GameData::GUMPS, buttonShapeNum, 1);
		Gump *widget = new ButtonWidget(WEASEL_BTN_X[i], WEASEL_BTN_Y[i], frame_up, frame_down, false);
		widget->InitGump(_ui, false);
		widget->SetIndex(i);
		// some buttons start hidden, the browsingMode() call below does that.
	}

	MainActor *av = getMainActor();
	assert(av);
	Item *item = av->getFirstItemWithShape(0x4ed, true);
	if (item)
		_credits = item->getQuality();

	_weaselDat = GameData::get_instance()->getWeaselDat(_level);
	if (!_weaselDat || _weaselDat->getNumItems() == 0)
		Close();
}

Gump *WeaselGump::playMovie(const Std::string &filename) {
	MovieGump *gump = MovieGump::CruMovieViewer(filename, 600, 450, nullptr, this, 0);
	if (!gump) {
		warning("Couldn't load flic %s", filename.c_str());
		return nullptr;
	}
	gump->CreateNotifier();
	return gump;
}

void WeaselGump::run() {
	ModalGump::run();
	// Don't do much while a movie is playing.
	if (_movie)
		return;
	_ui->UnhideGump();
	switch (_state) {
		case kWeaselStart:
			_state = kWeaselShowIntro;
			break;
		case kWeaselShowIntro: {
			if (_level == 2 && !_playedIntroMovie) {
				_movie = playMovie(FIRST_INTRO_MOVIE);
				_playedIntroMovie = true;
			} else {
				_movie = playMovie(_getRandomMovie(INTRO_MOVIES, ARRAYSIZE(INTRO_MOVIES)));
			}
			_state = kWeaselBrowsing;
			browsingMode(true);
			break;
		}
		case kWeaselCheckBuyMoreMovie:
			_movie = playMovie(_getRandomMovie(BUYMORE_MOVIES, ARRAYSIZE(BUYMORE_MOVIES)));
			_state = kWeaselCheckBuyMoreText;
			break;
		case kWeaselCheckBuyMoreText:
			checkBuyMore();
			break;
		case kWeaselClosing:
			Close();
			break;
		case kWeaselConfirmPurchaseMovie:
			_movie = playMovie(_getRandomMovie(CONFIRM_BUY_MOVIES, ARRAYSIZE(CONFIRM_BUY_MOVIES)));
			_state = kWeaselConfirmPurchaseText;
			break;
		case kWeaselConfirmPurchaseText:
			confirmPurchase();
			break;
		case kWeaselCancelledPurchaseMovie:
			browsingMode(true);
			_movie = playMovie(_getRandomMovie(CANCELLED_PURCHASE_MOVIES, ARRAYSIZE(CANCELLED_PURCHASE_MOVIES)));
			_state = kWeaselBrowsing;
			break;
		case kWeaselCompletedPurchase:
			_movie = playMovie(_getRandomMovie(COMPLETED_PURCHASE_MOVIES, ARRAYSIZE(COMPLETED_PURCHASE_MOVIES)));
			_state = kWeaselCheckBuyMoreText;
			break;
		case kWeaselInsufficientFunds:
			// TODO: how does it get to this situation?
			_movie = playMovie(_getRandomMovie(INSUFFICIENT_FUND_MOVIES, ARRAYSIZE(INSUFFICIENT_FUND_MOVIES)));
			break;
		case kWeaselBrowsing:
			_ui->UnhideGump();
		default:
			break;
	}
	if (_movie) {
		_ui->HideGump();
	}
}

void WeaselGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}

bool WeaselGump::OnKeyDown(int key, int mod) {
	if (Gump::OnKeyDown(key, mod)) return true;

	// TODO: support more keyboard input
	switch (key) {
		case Common::KEYCODE_LEFT:
			if (_state == kWeaselBrowsing)
				prevItem();
			break;
		case Common::KEYCODE_RIGHT:
			if (_state == kWeaselBrowsing)
				nextItem();
			break;
	}

	return true;
}

void WeaselGump::ChildNotify(Gump *child, uint32 message) {
	ButtonWidget *buttonWidget = dynamic_cast<ButtonWidget *>(child);
	MovieGump *movieGump = dynamic_cast<MovieGump *>(child);
	if (buttonWidget && message == ButtonWidget::BUTTON_CLICK) {
		onButtonClick(child->GetIndex());
	} else if (movieGump && message == Gump::GUMP_CLOSING) {
		// Movie has finished.
		_movie = nullptr;
	}
}

void WeaselGump::onButtonClick(int entry) {
	switch (entry) {
	case kBtnWeapons:
		_ammoMode = false;
		updateForAmmoMode();
		break;
	case kBtnAmmo:
		_ammoMode = true;
		updateForAmmoMode();
		break;
	case kBtnLeft:
		prevItem();
		break;
	case kBtnRight:
		nextItem();
		break;
	case kBtnBuy:
		buyItem();
		break;
	case kBtnExit:
		checkClose();
		break;
	case kBtnYes:
		if (_state == kWeaselConfirmPurchaseText)
			completePurchase();
		else if (_state == kWeaselCheckBuyMoreText)
			browsingMode(true);
		break;
	case kBtnNo:
		if (_state == kWeaselConfirmPurchaseText)
			abortPurchase();
		else if (_state == kWeaselCheckBuyMoreText)
			Close();
		break;
	case kBtnBlank:
	default:
		break;
	}
}

void WeaselGump::updateForAmmoMode() {
	Gump *ammobtn = _ui->FindGump(&FindByIndex<kBtnAmmo>);
	Gump *wpnbtn = _ui->FindGump(&FindByIndex<kBtnWeapons>);
	assert(ammobtn && wpnbtn);
	ammobtn->SetVisibility(!_ammoMode);
	wpnbtn->SetVisibility(_ammoMode);
	_curItem = 0;

	_weaselDat = GameData::get_instance()->getWeaselDat(_ammoMode ? 1 : _level);
	if (!_weaselDat || _weaselDat->getNumItems() == 0)
		Close();

	updateItemDisplay();
}


void WeaselGump::prevItem() {
	_curItem--;
	if (_curItem < 0)
		_curItem = _weaselDat->getNumItems() - 1;
	updateItemDisplay();
}

void WeaselGump::nextItem() {
	_curItem++;
	if (_curItem >= _weaselDat->getNumItems())
		_curItem = 0;
	updateItemDisplay();
}

void WeaselGump::buyItem() {
	if (_curItemCost < _credits) {
		_purchases.push_back(_curItemShape);
		_credits -= _curItemCost;
	} else {
		AudioProcess::get_instance()->playSFX(WEASEL_CANT_BUY_SFXNO, 0x80, 0, 0);
	}
	updateItemDisplay();
}

void WeaselGump::confirmPurchase() {
	static const char *confirm = "Are you sure you want to buy this?";
	setYesNoQuestion(confirm);
}

void WeaselGump::checkClose() {
	if (_purchases.size()) {
		_state = kWeaselConfirmPurchaseMovie;
	} else {
		Close();
	}
}

void WeaselGump::completePurchase() {
	assert(_state == kWeaselConfirmPurchaseText);
	MainActor *av = getMainActor();
	uint16 mapno = av->getMapNum();
	assert(av);
	Item *item = av->getFirstItemWithShape(0x4ed, true);
	if (item)
		item->setQuality(_credits);
	for (Std::vector<uint16>::const_iterator iter = _purchases.begin();
		 iter != _purchases.end(); iter++) {
		Item *newitem = ItemFactory::createItem(*iter, 0, 0, 0, 0, mapno, 0, true);
		av->addItemCru(newitem, false);
	}
	_state = kWeaselCompletedPurchase;
}

void WeaselGump::checkBuyMore() {
	static const char *buymore = "Do you want anything else?";
	setYesNoQuestion(buymore);
}

void WeaselGump::setYesNoQuestion(const Std::string &msg) {
	browsingMode(false);
	_closeIfExists(_ui->FindGump(&FindByIndex<kTxtQuestion>));
	TextWidget *textWidget = new TextWidget(30, 100, msg, true, WEASEL_FONT, 150);
	textWidget->InitGump(_ui);
	textWidget->SetIndex(kTxtQuestion);
}

void WeaselGump::browsingMode(bool browsing) {
	_ui->UnhideGump();

	updateForAmmoMode();
	updateItemDisplay();

	// Note: all these searches are not super effieient but it's
	// not a time-sensitive function and the search is relatively short
	Gump *yesbtn = _ui->FindGump(&FindByIndex<kBtnYes>);
	Gump *nobtn = _ui->FindGump(&FindByIndex<kBtnNo>);
	Gump *qtxt = _ui->FindGump(&FindByIndex<kTxtQuestion>);

	Gump *buybtn = _ui->FindGump(&FindByIndex<kBtnBuy>);
	Gump *wpnbtn = _ui->FindGump(&FindByIndex<kBtnWeapons>);
	Gump *ammobtn = _ui->FindGump(&FindByIndex<kBtnAmmo>);
	Gump *exitbtn = _ui->FindGump(&FindByIndex<kBtnExit>);
	Gump *blankbtn = _ui->FindGump(&FindByIndex<kBtnBlank>);
	Gump *leftbtn = _ui->FindGump(&FindByIndex<kBtnLeft>);
	Gump *rightbtn = _ui->FindGump(&FindByIndex<kBtnRight>);
	Gump *credtxt = _ui->FindGump(&FindByIndex<kTxtCredits>);
	Gump *nametxt = _ui->FindGump(&FindByIndex<kTxtItemName>);
	Gump *costtxt = _ui->FindGump(&FindByIndex<kTxtItemCost>);
	Gump *purchtxt = _ui->FindGump(&FindByIndex<kTxtItemPurch>);
	Gump *ownedtxt = _ui->FindGump(&FindByIndex<kTxtItemOwned>);
	Gump *icon = _ui->FindGump(&FindByIndex<kIconItem>);

	yesbtn->SetVisibility(!browsing);
	nobtn->SetVisibility(!browsing);
	if (qtxt)
		qtxt->SetVisibility(!browsing);

	buybtn->SetVisibility(browsing);
	wpnbtn->SetVisibility(browsing && _ammoMode);
	ammobtn->SetVisibility(browsing && !_ammoMode);
	exitbtn->SetVisibility(browsing);
	blankbtn->SetVisibility(browsing);
	leftbtn->SetVisibility(browsing);
	rightbtn->SetVisibility(browsing);
	credtxt->SetVisibility(browsing);
	nametxt->SetVisibility(browsing);
	costtxt->SetVisibility(browsing);
	purchtxt->SetVisibility(browsing);
	ownedtxt->SetVisibility(browsing);
	icon->SetVisibility(browsing);
}

void WeaselGump::abortPurchase() {
	assert(_state == kWeaselConfirmPurchaseText);
	_state = kWeaselCancelledPurchaseMovie;
	_purchases.clear();
}

int WeaselGump::purchasedCount(uint16 shape) const {
	int count = 0;
	for (Std::vector<uint16>::const_iterator iter = _purchases.begin();
		 iter != _purchases.end(); iter++) {
		 if (*iter == shape)
			 count++;
	}
	return count;
}

void WeaselGump::updateItemDisplay() {
	const Std::vector<WeaselDat::WeaselEntry> &items = _weaselDat->getItems();

	// should always have the item..
	assert(_curItem < (int)items.size());

	_curItemCost = items[_curItem]._cost;
	_curItemShape = items[_curItem]._shapeNo;

	const ShapeInfo *shapeinfo = GameData::get_instance()->getMainShapes()->getShapeInfo(_curItemShape);
	if (!shapeinfo || !shapeinfo->_weaponInfo) {
		warning("Weasel: no info for shape %d", _curItemShape);
		return;
	}
	const Shape *shape = GameData::get_instance()->getGumps()->getShape(shapeinfo->_weaponInfo->_displayGumpShape);

	_closeIfExists(_ui->FindGump(&FindByIndex<kTxtCredits>));
	_closeIfExists(_ui->FindGump(&FindByIndex<kTxtItemName>));
	_closeIfExists(_ui->FindGump(&FindByIndex<kTxtItemCost>));
	_closeIfExists(_ui->FindGump(&FindByIndex<kTxtItemPurch>));
	_closeIfExists(_ui->FindGump(&FindByIndex<kTxtItemOwned>));
	_closeIfExists(_ui->FindGump(&FindByIndex<kIconItem>));

	Std::string credstr = Std::string::format("Credits:%d", _credits);
	TextWidget *textWidget = new TextWidget(30, 57, credstr, true, WEASEL_FONT);
	textWidget->InitGump(_ui);
	textWidget->SetIndex(kTxtCredits);

	const ShapeFrame *frame = shape->getFrame(shapeinfo->_weaponInfo->_displayGumpFrame);
	Gump *icon = new Gump(105 - frame->_xoff, 120 - frame->_yoff, 200, 200);
	icon->SetShape(shape, shapeinfo->_weaponInfo->_displayGumpFrame);
	icon->UpdateDimsFromShape();
	icon->setRelativePosition(CENTER);
	icon->InitGump(_ui, false);
	icon->SetIndex(kIconItem);

	Std::string coststr = Std::string::format("Cost:%d", _curItemCost);
	Std::string purchstr = Std::string::format("Purchased:%02d", purchasedCount(_curItemShape));

	MainActor *av = getMainActor();
	const Item *item = av->getFirstItemWithShape(_curItemShape, true);
	int count = 0;
	if (item) {
		if (shapeinfo->_family == ShapeInfo::SF_CRUWEAPON) {
			count = 1;
		} else {
			count = item->getQuality();
		}
	}
	Std::string ownedstr = Std::string::format("Owned:%02d", count);

	TextWidget *nametxt = new TextWidget(27, 161, shapeinfo->_weaponInfo->_name, true, WEASEL_FONT);
	nametxt->InitGump(_ui, false);
	nametxt->SetIndex(kTxtItemName);
	TextWidget *costtxt = new TextWidget(27, 171, coststr, true, WEASEL_FONT);
	costtxt->InitGump(_ui, false);
	costtxt->SetIndex(kTxtItemCost);
	TextWidget *purchtxt = new TextWidget(27, 181, purchstr, true, WEASEL_FONT);
	purchtxt->InitGump(_ui, false);
	purchtxt->SetIndex(kTxtItemPurch);
	TextWidget *ownedtxt = new TextWidget(27, 191, ownedstr, true, WEASEL_FONT);
	ownedtxt->InitGump(_ui, false);
	ownedtxt->SetIndex(kTxtItemOwned);
}

bool WeaselGump::OnTextInput(int unicode) {
	if (Gump::OnTextInput(unicode)) return true;

	return true;
}

//static
uint32 WeaselGump::I_showWeaselGump(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(level);

	WeaselGump *gump = new WeaselGump(level);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);

	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
