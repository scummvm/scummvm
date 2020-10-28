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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/gumps/u8_save_gump.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/gumps/widgets/edit_widget.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/filesys/savegame.h"
#include "ultima/ultima8/gumps/paged_gump.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "common/savefile.h"
#include "common/translation.h"

namespace Ultima {
namespace Ultima8 {

static const int entryfont = 4;

DEFINE_RUNTIME_CLASSTYPE_CODE(U8SaveGump)

U8SaveGump::U8SaveGump(bool saveMode, int page)
	: Gump(0, 0, 5, 5), _save(saveMode), _page(page) {
}

U8SaveGump::~U8SaveGump() {
}


// gumps: 36/0-11: number 1-12
//        46/0: "Entry"

void U8SaveGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	_dims.setWidth(220);
	_dims.setHeight(170);

	FrameID entry_id(GameData::GUMPS, 46, 0);
	entry_id = _TL_SHP_(entry_id);

	const Shape *entryShape = GameData::get_instance()->getShape(entry_id);
	const ShapeFrame *sf = entryShape->getFrame(entry_id._frameNum);
	int entrywidth = sf->_width;
	int entryheight = sf->_height;

	if (_save)
		_editWidgets.resize(6); // constant!

	loadDescriptions();

	for (int i = 0; i < 6; ++i) {
		int index = _page * 6 + i;


		int xbase = 3;
		int yi = i;
		if (i >= 3) {
			xbase += _dims.width() / 2 + 9;
			yi -= 3;
		}

		Gump *gump = new Gump(xbase, 3 + 40 * yi, 1, 1);
		gump->SetShape(entry_id, true);
		gump->InitGump(this, false);

		int x = xbase + 2 + entrywidth;

		if (index >= 9) { // index 9 is labelled "10"
			FrameID entrynum1_id(GameData::GUMPS, 36, (index + 1) / 10 - 1);
			entrynum1_id = _TL_SHP_(entrynum1_id);
			entryShape = GameData::get_instance()->getShape(entrynum1_id);
			sf = entryShape->getFrame(entrynum1_id._frameNum);
			x += 1 + sf->_width;

			gump = new Gump(xbase + 2 + entrywidth, 3 + 40 * yi, 1, 1);
			gump->SetShape(entrynum1_id, true);
			gump->InitGump(this, false);
		}

		FrameID entrynum_id(GameData::GUMPS, 36, index % 10);
		entrynum_id = _TL_SHP_(entrynum_id);

		gump = new Gump(x, 3 + 40 * yi, 1, 1);
		gump->SetShape(entrynum_id, true);

		if (index % 10 == 9) {
			// HACK: There is no frame for '0', so we re-use part of the
			// frame for '10', cutting off the first 6 pixels.
			Rect rect;
			gump->GetDims(rect);
			rect.translate(6, 0);
			gump->SetDims(rect);
		}
		gump->InitGump(this, false);

		if (index == 0) {
			// special case for 'The Beginning...' _save
			Gump *widget = new TextWidget(xbase, entryheight + 4 + 40 * yi,
			                              _TL_("The Beginning..."),
			                              true, entryfont, 95);
			widget->InitGump(this, false);

		} else {

			if (_save) {
				EditWidget *ew = new EditWidget(xbase, entryheight + 4 + 40 * yi,
				                                _descriptions[i],
				                                true, entryfont,
				                                95, 38 - entryheight, 0, true);
				ew->SetIndex(i + 1);
				ew->InitGump(this, false);
				_editWidgets[i] = ew;
			} else {
				// load
				Gump *widget = new TextWidget(xbase, entryheight + 4 + 40 * yi,
				                              _descriptions[i], true, entryfont,
				                              95);
				widget->InitGump(this, false);
			}
		}

	}

	// remove focus from children (just in case)
	if (_focusChild) _focusChild->OnFocus(false);
	_focusChild = 0;
}

void U8SaveGump::Close(bool no_del) {
	Gump::Close(no_del);
}

void U8SaveGump::OnFocus(bool gain) {
	if (gain) {
		if (_save)
			Mouse::get_instance()->setMouseCursor(Mouse::MOUSE_QUILL);
		else
			Mouse::get_instance()->setMouseCursor(Mouse::MOUSE_MAGGLASS);
	}
}

Gump *U8SaveGump::onMouseDown(int button, int32 mx, int32 my) {
	// take all clicks
	return this;
}


void U8SaveGump::onMouseClick(int button, int32 mx, int32 my) {
	if (button != Shared::BUTTON_LEFT) return;

	ParentToGump(mx, my);

	int x;
	if (mx >= 3 && mx <= 100)
		x = 0;
	else if (mx >= _dims.width() / 2 + 10)
		x = 1;
	else
		return;

	int y;
	if (my >= 3 && my <= 40)
		y = 0;
	else if (my >= 43 && my <= 80)
		y = 1;
	else if (my >= 83 && my <= 120)
		y = 2;
	else
		return;

	int i = 3 * x + y;
	int index = 6 * _page + i + 1;

	if (_save && !_focusChild && _editWidgets[i]) {
		_editWidgets[i]->MakeFocus();
		PagedGump *p = dynamic_cast<PagedGump *>(_parent);
		if (p) p->enableButtons(false);
	}

	if (!_save) {

		// If our parent has a notifiy process, we'll put our result in it and wont actually load the game
		GumpNotifyProcess *p = _parent->GetNotifyProcess();
		if (p) {
			// Do nothing in this case
			if (index != 1 && _descriptions[i].empty()) return;

			_parent->SetResult(index);
			_parent->Close(); // close PagedGump (and us)
			return;
		}

		loadgame(index); // 'this' will be deleted here!
	}
}

void U8SaveGump::ChildNotify(Gump *child, uint32 message) {
	EditWidget *widget = dynamic_cast<EditWidget *>(child);
	if (widget && message == EditWidget::EDIT_ENTER) {
		// _save
		assert(_save);

		Std::string name = widget->getText();
		if (name.empty()) return;

		if (savegame(widget->GetIndex() + 6 * _page, name))
			_parent->Close(); // close PagedGump (and us)

		return;
	}

	if (widget && message == EditWidget::EDIT_ESCAPE) {
		// cancel edit
		assert(_save);

		// remove focus
		if (_focusChild) _focusChild->OnFocus(false);
		_focusChild = 0;

		PagedGump *p = dynamic_cast<PagedGump *>(_parent);
		if (p) p->enableButtons(true);

		widget->setText(_descriptions[widget->GetIndex() - 1]);

		return;
	}

}

bool U8SaveGump::OnKeyDown(int key, int mod) {
	if (Gump::OnKeyDown(key, mod)) return true;

	return false;
}

bool U8SaveGump::loadgame(int saveIndex) {
	if (saveIndex == 1) {
		Ultima8Engine::get_instance()->newGame();
		return true;
	} else {
		return Ultima8Engine::get_instance()->loadGameState(saveIndex).getCode() == Common::kNoError;
	}
}

bool U8SaveGump::savegame(int saveIndex, const Std::string &name) {
	pout << "Save " << saveIndex << ": \"" << name << "\"" << Std::endl;

	if (name.empty()) return false;

	Ultima8Engine::get_instance()->saveGame(saveIndex, name, true);
	return true;
}

void U8SaveGump::loadDescriptions() {
	_descriptions.resize( 6);

	for (int i = 0; i < 6; ++i) {
		int saveIndex = 6 * _page + i + 1;

		Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(
			Ultima8Engine::get_instance()->getSaveStateName(saveIndex));
		if (!saveFile)
			continue;

		const SavegameReader *sg = new SavegameReader(saveFile, true);
		SavegameReader::State state = sg->isValid();
		_descriptions[i] = "";

		// FIXME: move version checks elsewhere!!
		switch (state) {
		case SavegameReader::SAVE_CORRUPT:
			_descriptions[i] = Common::convertFromU32String(_("[corrupt]"));
			break;
		case SavegameReader::SAVE_OUT_OF_DATE:
			_descriptions[i] = Common::convertFromU32String(_("[outdated]"));
			break;
		case SavegameReader::SAVE_TOO_RECENT:
			_descriptions[i] = Common::convertFromU32String(_("[too modern]"));
			break;
		default:
			break;
		}

		if (state != SavegameReader::SAVE_VALID)
			_descriptions[i] += " ";
		_descriptions[i] += sg->getDescription();
		delete sg;
	}
}

//static
Gump *U8SaveGump::showLoadSaveGump(Gump *parent, bool save) {
	if (save) {
		// can't save if game over
		// FIXME: this check should probably be in Game or GUIApp
		const MainActor *av = getMainActor();
		if (!av || av->hasActorFlags(Actor::ACT_DEAD))
			return nullptr;
	}

	PagedGump *gump = new PagedGump(34, -38, 3, 35);
	gump->InitGump(parent);

	for (int page = 0; page < 16; ++page) {
		U8SaveGump *s = new U8SaveGump(save, page);
		s->InitGump(gump, false);
		gump->addPage(s);
	}


	gump->setRelativePosition(CENTER);

	return gump;
}

} // End of namespace Ultima8
} // End of namespace Ultima
