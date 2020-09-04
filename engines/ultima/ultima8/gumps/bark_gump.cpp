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

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(BarkGump)

// TODO: Remove all the hacks

BarkGump::BarkGump() : ItemRelativeGump(), _counter(0), _textWidget(0),
		_speechShapeNum(0), _speechLength(0), _totalTextHeight(0),
		_textDelay(20) {
}

BarkGump::BarkGump(uint16 owner, const Std::string &msg, uint32 speechShapeNum) :
	ItemRelativeGump(0, 0, 100, 100, owner, FLAG_KEEP_VISIBLE, LAYER_ABOVE_NORMAL),
	_barked(msg), _counter(100), _speechShapeNum(speechShapeNum),
	_speechLength(0), _totalTextHeight(0), _textWidget(0), _textDelay(20) {
	SettingManager::get_instance()->get("textdelay", _textDelay);
}

BarkGump::~BarkGump(void) {
}

int BarkGump::dialogFontForActor(uint16 actor) {
	// OK, this is a bit of a hack, but it's how it has to be
	if (actor == 1)
		return 6;
	if (actor > 256)
		return 8;
	switch (actor % 3) {
	case 1:
		return 5;
	case 2:
		return 7;
	default:
		return 0;
	}
}

void BarkGump::InitGump(Gump *newparent, bool take_focus) {
	int fontnum = dialogFontForActor(_owner);

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
			_totalTextHeight = d.height();
			while (widget->setupNextText()) {
				widget->GetDims(d);
				_totalTextHeight += d.height();
			}
			widget->rewind();
		}
	}

	// This is just a hack
	Rect d;
	widget->GetDims(d);
	if (_speechLength && _totalTextHeight) {
		_counter = (d.height() * _speechLength) / _totalTextHeight;
	} else {
		_counter = d.height() * _textDelay;
	}
	_dims.setHeight(d.height());
	_dims.setWidth(d.width());

	// Wait with ItemRelativeGump initialization until we calculated our size.
	ItemRelativeGump::InitGump(newparent, take_focus);
}

bool BarkGump::NextText() {
	TextWidget *widget = dynamic_cast<TextWidget *>(getGump(_textWidget));
	assert(widget);
	if (widget->setupNextText()) {
		// This is just a hack
		Rect d;
		widget->GetDims(d);
		if (_speechLength && _totalTextHeight) {
			_counter = (d.height() * _speechLength) / _totalTextHeight;
		} else {
			_counter = d.height() * _textDelay;
		}
		_dims.setHeight(d.height());
		_dims.setWidth(d.width());
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

Gump *BarkGump::onMouseDown(int button, int32 mx, int32 my) {
	Gump *g = ItemRelativeGump::onMouseDown(button, mx, my);
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

void BarkGump::saveData(Common::WriteStream *ws) {
	ItemRelativeGump::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_counter));
	ws->writeUint16LE(_textWidget);
	ws->writeUint32LE(_speechShapeNum);
	ws->writeUint32LE(_speechLength);
	ws->writeUint32LE(_totalTextHeight);
	ws->writeUint32LE(static_cast<uint32>(_barked.size()));
	ws->write(_barked.c_str(), _barked.size());
}

bool BarkGump::loadData(Common::ReadStream *rs, uint32 version) {
	if (!ItemRelativeGump::loadData(rs, version)) return false;

	_counter = static_cast<int32>(rs->readUint32LE());
	_textWidget = rs->readUint16LE();
	_speechShapeNum = rs->readUint32LE();
	_speechLength = rs->readUint32LE();
	_totalTextHeight = rs->readUint32LE();

	uint32 slen = rs->readUint32LE();
	if (slen > 0) {
		char *buf = new char[slen + 1];
		rs->read(buf, slen);
		buf[slen] = 0;
		_barked = buf;
		delete[] buf;
	} else {
		_barked = "";
	}

	TextWidget *widget = dynamic_cast<TextWidget *>(getGump(_textWidget));
	if (!widget)
		return false;

	SettingManager::get_instance()->get("textdelay", _textDelay);

	// This is just a hack
	Rect d;
	widget->GetDims(d);
	_counter = d.height() * _textDelay;
	_dims.setHeight(d.height());
	_dims.setWidth(d.width());

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
