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


#include "ultima8/misc/pent_include.h"
#include "ask_gump.h"
#include "button_widget.h"
#include "ultima8/usecode/UCList.h"
#include "ultima8/usecode/uc_machine.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"


// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AskGump, ItemRelativeGump);

AskGump::AskGump()
	: ItemRelativeGump(), answers(0) {

}

AskGump::AskGump(uint16 owner, UCList *answers_) :
	ItemRelativeGump(0, 0, 0, 0, owner, FLAG_KEEP_VISIBLE, LAYER_ABOVE_NORMAL),
	answers(new UCList(2)) {
	answers->copyStringList(*answers_);
}

AskGump::~AskGump() {
	answers->freeStrings();
	delete answers;
}

// Init the gump, call after construction
void AskGump::InitGump(Gump *newparent, bool take_focus) {
	// OK, this is a bit of a hack, but it's how it has to be
	int fontnum;
	if (owner == 1) fontnum = 6;
	else if (owner > 256) fontnum = 8;
	else switch (owner % 3) {
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

	for (unsigned int i = 0; i < answers->getSize(); ++i) {
		std::string str_answer = "@ ";
		str_answer += UCMachine::get_instance()->getString(answers->getStringIndex(i));

		ButtonWidget *child = new ButtonWidget(px, py, str_answer,
		                                       true, fontnum);
		child->InitGump(this);
		child->SetIndex(i);

		Pentagram::Rect cd;
		child->GetDims(cd);
		if (i + 1 < answers->getSize())
			cd.h += child->getVlead();

		if (px + cd.w > 160 && px != 0) {
			py = dims.h;
			px = 0;
			child->Move(px, py);
		}

		if (cd.w + px > dims.w) dims.w = cd.w + px;
		if (cd.h + py > dims.h) dims.h = cd.h + py;

		px += cd.w + 4;
	}

	// Wait with ItemRelativeGump initialization until we calculated our size.
	ItemRelativeGump::InitGump(newparent, take_focus);
}

void AskGump::ChildNotify(Gump *child, uint32 message) {
	if (message == ButtonWidget::BUTTON_CLICK) {
		uint16 s = answers->getStringIndex(child->GetIndex());
		process_result = s;

		// answers' strings are going to be deleted, so make sure
		// the response string won't be deleted
		answers->removeString(s, true); //!! assuming that answers doesn't
		//!! contain two identical strings
		Close();
	}
}

void AskGump::saveData(ODataSource *ods) {
	ItemRelativeGump::saveData(ods);

	answers->save(ods);
}

bool AskGump::loadData(IDataSource *ids, uint32 version) {
	if (!ItemRelativeGump::loadData(ids, version)) return false;

	answers = new UCList(2);
	answers->load(ids, version);

	// HACK ALERT
	int px = 0, py = 0;

	dims.w = 0;
	dims.h = 0;


	for (unsigned int i = 0; i < answers->getSize(); ++i) {

		ButtonWidget *child = 0;

		std::list<Gump *>::iterator it;
		for (it = children.begin(); it != children.end(); ++it) {
			if ((*it)->GetIndex() != (int)i) continue;
			child = p_dynamic_cast<ButtonWidget *>(*it);
			if (!child) continue;
		}

		if (!child) return false;

		Pentagram::Rect cd;
		child->GetDims(cd);

		if (px + cd.w > 160 && px != 0) {
			py = dims.h;
			px = 0;
		}
		child->Move(px, py);

		if (cd.w + px > dims.w) dims.w = cd.w + px;
		if (cd.h + py > dims.h) dims.h = cd.h + py;

		px += cd.w + 4;
	}

	return true;
}

// You should always use Protection
