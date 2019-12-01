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
#include "ultima8/gumps/bark_gump.h"
#include "ultima8/gumps/widgets/text_widget.h"
#include "ultima8/kernel/kernel.h"
#include "ultima8/audio/audio_process.h"
#include "ultima8/world/get_object.h"
#include "ultima8/conf/setting_manager.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(BarkGump, ItemRelativeGump)

// TODO: Remove all the hacks

BarkGump::BarkGump()
	: ItemRelativeGump() {

}

BarkGump::BarkGump(uint16 owner, std::string msg, uint32 speechshapenum_) :
	ItemRelativeGump(0, 0, 100, 100, owner,
	                 FLAG_KEEP_VISIBLE, LAYER_ABOVE_NORMAL),
	barked(msg), counter(100), speechshapenum(speechshapenum_),
	speechlength(0), totaltextheight(0) {
	SettingManager::get_instance()->get("textdelay", textdelay);
}

BarkGump::~BarkGump(void) {
}

void BarkGump::InitGump(Gump *newparent, bool take_focus) {
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

	// This is a hack. We init the gump twice...
	ItemRelativeGump::InitGump(newparent, take_focus);

	// Create the TextWidget
	TextWidget *widget = new TextWidget(0, 0, barked, true, fontnum, 194, 55);
	widget->InitGump(this);

	textwidget = widget->getObjId();

	// see if we need to play speech
	AudioProcess *ap = AudioProcess::get_instance();
	speechlength = 0;
	if (speechshapenum && ap) {
		if (ap->playSpeech(barked, speechshapenum, owner)) {
			speechlength = ap->getSpeechLength(barked, speechshapenum) / 33;
			if (speechlength == 0) speechlength = 1;

			// We're playing speech, so need to sync the text with the speech.
			// First we count the total height of all text blocks.
			Pentagram::Rect d;
			widget->GetDims(d);
			totaltextheight = d.h;
			while (widget->setupNextText()) {
				widget->GetDims(d);
				totaltextheight += d.h;
			}
			widget->rewind();
		}
	}

	// This is just a hack
	Pentagram::Rect d;
	widget->GetDims(d);
	if (speechlength && totaltextheight) {
		counter = (d.h * speechlength) / totaltextheight;
	} else {
		counter = d.h * textdelay;
	}
	dims.h = d.h;
	dims.w = d.w;

	// Wait with ItemRelativeGump initialization until we calculated our size.
	ItemRelativeGump::InitGump(newparent, take_focus);
}

bool BarkGump::NextText() {
	TextWidget *widget = p_dynamic_cast<TextWidget *>(getGump(textwidget));
	assert(widget);
	if (widget->setupNextText()) {
		// This is just a hack
		Pentagram::Rect d;
		widget->GetDims(d);
		if (speechlength && totaltextheight) {
			counter = (d.h * speechlength) / totaltextheight;
		} else {
			counter = d.h * textdelay;
		}
		dims.h = d.h;
		dims.w = d.w;
		return true;
	}

	return false;
}

void BarkGump::run() {
	ItemRelativeGump::run();

	// Auto close
	if (!Kernel::get_instance()->isPaused()) {
		if (!--counter) {
			// try next text
			bool done = !NextText();
			if (done) {
				bool speechplaying = false;
				if (speechlength) {
					// waiting for speech to finish?
					AudioProcess *ap = AudioProcess::get_instance();
					if (ap)
						speechplaying = ap->isSpeechPlaying(barked,
						                                    speechshapenum);
				}

				// if speech done too, close
				if (!speechplaying)
					Close();
				else
					counter = textdelay;
			}
		}
	}
}

Gump *BarkGump::OnMouseDown(int button, int mx, int my) {
	Gump *g = ItemRelativeGump::OnMouseDown(button, mx, my);
	if (g) return g;

	// Scroll to next text, if possible
	if (!NextText()) {
		if (speechlength) {
			AudioProcess *ap = AudioProcess::get_instance();
			if (ap) ap->stopSpeech(barked, speechshapenum, owner);
		}
		Close();
	}
	return this;
}

void BarkGump::saveData(ODataSource *ods) {
	ItemRelativeGump::saveData(ods);

	ods->write4(static_cast<uint32>(counter));
	ods->write2(textwidget);
	ods->write4(speechshapenum);
	ods->write4(speechlength);
	ods->write4(totaltextheight);
	ods->write4(static_cast<uint32>(barked.size()));
	ods->write(barked.c_str(), barked.size());
}

bool BarkGump::loadData(IDataSource *ids, uint32 version) {
	if (!ItemRelativeGump::loadData(ids, version)) return false;

	counter = static_cast<int32>(ids->read4());
	textwidget = ids->read2();
	speechshapenum = ids->read4();
	speechlength = ids->read4();
	totaltextheight = ids->read4();

	uint32 slen = ids->read4();
	if (slen > 0) {
		char *buf = new char[slen + 1];
		ids->read(buf, slen);
		buf[slen] = 0;
		barked = buf;
		delete[] buf;
	} else {
		barked = "";
	}


	TextWidget *widget = p_dynamic_cast<TextWidget *>(getGump(textwidget));

	SettingManager::get_instance()->get("textdelay", textdelay);

	// This is just a hack
	Pentagram::Rect d;
	widget->GetDims(d);
	counter = d.h * textdelay;
	dims.h = d.h;
	dims.w = d.w;

	return true;
}

} // End of namespace Ultima8
