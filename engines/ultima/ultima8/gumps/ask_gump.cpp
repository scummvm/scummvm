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
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AskGump, ItemRelativeGump)

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
	int fontnum;
	if (_owner == 1) fontnum = 6;
	else if (_owner > 256) fontnum = 8;
	else switch (_owner % 3) {
		case 1:
			fontnum = 5;
			break;

		case 2:
			fontnum = 7;
			break;

		default:
			fontnum = 0;
			break;
		}

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
			cd.h += child->getVlead();

		if (px + cd.w > 160 && px != 0) {
			py = _dims.h;
			px = 0;
			child->Move(px, py);
		}

		if (cd.w + px > _dims.w) _dims.w = cd.w + px;
		if (cd.h + py > _dims.h) _dims.h = cd.h + py;

		px += cd.w + 4;
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

void AskGump::saveData(ODataSource *ods) {
	ItemRelativeGump::saveData(ods);

	_answers->save(ods);
}

bool AskGump::loadData(IDataSource *ids, uint32 version) {
	if (!ItemRelativeGump::loadData(ids, version)) return false;

	_answers = new UCList(2);
	_answers->load(ids, version);

	// HACK ALERT
	int px = 0, py = 0;

	_dims.w = 0;
	_dims.h = 0;


	for (unsigned int i = 0; i < _answers->getSize(); ++i) {

		ButtonWidget *child = nullptr;

		Std::list<Gump *>::iterator it;
		for (it = _children.begin(); it != _children.end(); ++it) {
			if ((*it)->GetIndex() != (int)i) continue;
			child = p_dynamic_cast<ButtonWidget *>(*it);
			if (!child) continue;
		}

		if (!child) return false;

		Rect cd;
		child->GetDims(cd);

		if (px + cd.w > 160 && px != 0) {
			py = _dims.h;
			px = 0;
		}
		child->Move(px, py);

		if (cd.w + px > _dims.w) _dims.w = cd.w + px;
		if (cd.h + py > _dims.h) _dims.h = cd.h + py;

		px += cd.w + 4;
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
