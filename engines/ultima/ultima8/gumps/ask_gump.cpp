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
#include "ultima/ultima8/gumps/ask_gump.h"
#include "ultima/ultima8/gumps/bark_gump.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/usecode/uc_machine.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AskGump)

AskGump::AskGump() : ItemRelativeGump(), _answers(0) {
}

AskGump::AskGump(uint16 owner, UCList *answers) :
	ItemRelativeGump(0, 0, 0, 0, owner, FLAG_KEEP_VISIBLE, LAYER_ABOVE_NORMAL),
	_answers(new UCList(2)) {
	_answers->copyStringList(*answers);
}

AskGump::~AskGump() {
	_answers->freeStrings();
	delete _answers;
}

// Init the gump, call after construction
void AskGump::InitGump(Gump *newparent, bool take_focus) {
	// OK, this is a bit of a hack, but it's how it has to be
	int fontnum = BarkGump::dialogFontForActor(_owner);

	int px = 0, py = 0;

	// This is a hack. We init the gump twice...
	ItemRelativeGump::InitGump(newparent, take_focus);

	for (unsigned int i = 0; i < _answers->getSize(); ++i) {
		Std::string str_answer = "@ ";
		str_answer += UCMachine::get_instance()->getString(_answers->getStringIndex(i));

		ButtonWidget *child = new ButtonWidget(px, py, str_answer,
		                                       true, fontnum);
		child->InitGump(this);
		child->SetIndex(i);

		Rect cd;
		child->GetDims(cd);
		if (i + 1 < _answers->getSize())
			cd.setHeight(cd.height() + child->getVlead());

		if (px + cd.width() > 160 && px != 0) {
			py = _dims.height();
			px = 0;
			child->Move(px, py);
		}

		if (cd.width() + px > _dims.width())
			_dims.setWidth(cd.width() + px);
		if (cd.height() + py > _dims.height())
			_dims.setHeight(cd.height() + py);

		px += cd.width() + 4;
	}

	// Wait with ItemRelativeGump initialization until we calculated our size.
	ItemRelativeGump::InitGump(newparent, take_focus);
}

void AskGump::ChildNotify(Gump *child, uint32 message) {
	if (message == ButtonWidget::BUTTON_CLICK) {
		uint16 s = _answers->getStringIndex(child->GetIndex());
		_processResult = s;

		// _answers' strings are going to be deleted, so make sure
		// the response string won't be deleted
		_answers->removeString(s, true); //!! assuming that answers doesn't
		//!! contain two identical strings
		Close();
	}
}

void AskGump::saveData(Common::WriteStream *ws) {
	ItemRelativeGump::saveData(ws);

	_answers->save(ws);
}

bool AskGump::loadData(Common::ReadStream *rs, uint32 version) {
	if (!ItemRelativeGump::loadData(rs, version)) return false;

	_answers = new UCList(2);
	_answers->load(rs, version);

	// HACK ALERT
	int px = 0, py = 0;

	_dims.setWidth(0);
	_dims.setHeight(0);


	for (unsigned int i = 0; i < _answers->getSize(); ++i) {

		ButtonWidget *child = nullptr;

		Std::list<Gump *>::iterator it;
		for (it = _children.begin(); it != _children.end(); ++it) {
			if ((*it)->GetIndex() != (int)i) continue;
			child = dynamic_cast<ButtonWidget *>(*it);
			if (!child) continue;
		}

		if (!child) return false;

		Rect cd;
		child->GetDims(cd);

		if (px + cd.width() > 160 && px != 0) {
			py = _dims.height();
			px = 0;
		}
		child->Move(px, py);

		if (cd.width() + px > _dims.width())
			_dims.setWidth(cd.width() + px);
		if (cd.height() + py > _dims.height())
			_dims.setHeight(cd.height() + py);

		px += cd.width() + 4;
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
