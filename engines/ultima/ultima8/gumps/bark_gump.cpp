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
#include "ultima/ultima8/gumps/bark_gump.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(BarkGump, ItemRelativeGump)

// TODO: Remove all the hacks

BarkGump::BarkGump() : ItemRelativeGump(), _counter(0), _textWidget(0),
		_speechShapeNum(0), _speechLength(0), _totalTextHeight(0) {
}

BarkGump::BarkGump(uint16 owner, const Std::string &msg, uint32 speechShapeNum) :
	ItemRelativeGump(0, 0, 100, 100, owner, FLAG_KEEP_VISIBLE, LAYER_ABOVE_NORMAL),
	_barked(msg), _counter(100), _speechShapeNum(speechShapeNum),
	_speechLength(0), _totalTextHeight(0), _textWidget(0) {
	SettingManager::get_instance()->get("textdelay", _textDelay);
}

BarkGump::~BarkGump(void) {
}

void BarkGump::InitGump(Gump *newparent, bool take_focus) {
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

	// This is a hack. We init the gump twice...
	ItemRelativeGump::InitGump(newparent, take_focus);

	// Create the TextWidget
	TextWidget *widget = new TextWidget(0, 0, _barked, true, fontnum, 194, 55);
	widget->InitGump(this);

	_textWidget = widget->getObjId();

	// see if we need to play speech
	AudioProcess *ap = AudioProcess::get_instance();
	_speechLength = 0;
	if (_speechShapeNum && ap) {
		if (ap->playSpeech(_barked, _speechShapeNum, _owner)) {
			_speechLength = ap->getSpeechLength(_barked, _speechShapeNum) / 33;
			if (_speechLength == 0) _speechLength = 1;

			// We're playing speech, so need to sync the text with the speech.
			// First we count the total height of all text blocks.
			Rect d;
			widget->GetDims(d);
			_totalTextHeight = d.h;
			while (widget->setupNextText()) {
				widget->GetDims(d);
				_totalTextHeight += d.h;
			}
			widget->rewind();
		}
	}

	// This is just a hack
	Rect d;
	widget->GetDims(d);
	if (_speechLength && _totalTextHeight) {
		_counter = (d.h * _speechLength) / _totalTextHeight;
	} else {
		_counter = d.h * _textDelay;
	}
	_dims.h = d.h;
	_dims.w = d.w;

	// Wait with ItemRelativeGump initialization until we calculated our size.
	ItemRelativeGump::InitGump(newparent, take_focus);
}

bool BarkGump::NextText() {
	TextWidget *widget = p_dynamic_cast<TextWidget *>(getGump(_textWidget));
	assert(widget);
	if (widget->setupNextText()) {
		// This is just a hack
		Rect d;
		widget->GetDims(d);
		if (_speechLength && _totalTextHeight) {
			_counter = (d.h * _speechLength) / _totalTextHeight;
		} else {
			_counter = d.h * _textDelay;
		}
		_dims.h = d.h;
		_dims.w = d.w;
		return true;
	}

	return false;
}

void BarkGump::run() {
	ItemRelativeGump::run();

	// Auto close
	if (!Kernel::get_instance()->isPaused()) {
		if (!--_counter) {
			// try next text
			bool done = !NextText();
			if (done) {
				bool speechplaying = false;
				if (_speechLength) {
					// waiting for speech to finish?
					AudioProcess *ap = AudioProcess::get_instance();
					if (ap)
						speechplaying = ap->isSpeechPlaying(_barked,
						                                    _speechShapeNum);
				}

				// if speech done too, close
				if (!speechplaying)
					Close();
				else
					_counter = _textDelay;
			}
		}
	}
}

Gump *BarkGump::OnMouseDown(int button, int32 mx, int32 my) {
	Gump *g = ItemRelativeGump::OnMouseDown(button, mx, my);
	if (g) return g;

	// Scroll to next text, if possible
	if (!NextText()) {
		if (_speechLength) {
			AudioProcess *ap = AudioProcess::get_instance();
			if (ap) ap->stopSpeech(_barked, _speechShapeNum, _owner);
		}
		Close();
	}
	return this;
}

void BarkGump::saveData(ODataSource *ods) {
	ItemRelativeGump::saveData(ods);

	ods->write4(static_cast<uint32>(_counter));
	ods->write2(_textWidget);
	ods->write4(_speechShapeNum);
	ods->write4(_speechLength);
	ods->write4(_totalTextHeight);
	ods->write4(static_cast<uint32>(_barked.size()));
	ods->write(_barked.c_str(), _barked.size());
}

bool BarkGump::loadData(IDataSource *ids, uint32 version) {
	if (!ItemRelativeGump::loadData(ids, version)) return false;

	_counter = static_cast<int32>(ids->read4());
	_textWidget = ids->read2();
	_speechShapeNum = ids->read4();
	_speechLength = ids->read4();
	_totalTextHeight = ids->read4();

	uint32 slen = ids->read4();
	if (slen > 0) {
		char *buf = new char[slen + 1];
		ids->read(buf, slen);
		buf[slen] = 0;
		_barked = buf;
		delete[] buf;
	} else {
		_barked = "";
	}


	TextWidget *widget = p_dynamic_cast<TextWidget *>(getGump(_textWidget));

	SettingManager::get_instance()->get("textdelay", _textDelay);

	// This is just a hack
	Rect d;
	widget->GetDims(d);
	_counter = d.h * _textDelay;
	_dims.h = d.h;
	_dims.w = d.w;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
