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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/font.h"
#include "mads/screen.h"
#include "mads/messages.h"
#include "mads/scene_data.h"

namespace MADS {

RandomMessages::RandomMessages() {
	reserve(RANDOM_MESSAGE_SIZE);
	_randomSpacing = 0;
	_color = -1;
	_duration = 0;
	_scrollRate = -1;
}

void RandomMessages::reset() {
	for (uint i = 0; i < size(); ++i) {
		(*this)[i]._handle = -1;
		(*this)[i]._quoteId = -1;
	}
}


KernelMessages::KernelMessages(MADSEngine *vm)
	: _vm(vm) {
	for (int i = 0; i < KERNEL_MESSAGES_SIZE; ++i) {
		KernelMessage rec;
		_entries.push_back(rec);
	}

	_talkFont = _vm->_font->getFont(FONT_CONVERSATION);
}

KernelMessages::~KernelMessages() {
}

void KernelMessages::clear() {
	Scene &scene = _vm->_game->_scene;

	for (uint i = 0; i < _entries.size(); ++i)
		_entries[i]._flags = 0;

	_talkFont = _vm->_font->getFont(FONT_CONVERSATION);
	scene._textSpacing = -1;
}

int KernelMessages::add(const Common::Point &pt, uint fontColor, uint8 flags,
		int endTrigger, uint32 timeout, const Common::String &msg) {
	Scene &scene = _vm->_game->_scene;

	// Find a free slot
	uint idx = 0;
	while ((idx < _entries.size()) && ((_entries[idx]._flags & KMSG_ACTIVE) != 0))
		++idx;
	if (idx == _entries.size()) {
		if (endTrigger == 0)
			return -1;

		error("KernelMessages overflow");
	}

	KernelMessage &rec = _entries[idx];
	rec._msg = msg;
	rec._flags = flags | KMSG_ACTIVE;
	rec._color1 = fontColor & 0xff;
	rec._color2 = fontColor >> 8;
	rec._position = pt;
	rec._textDisplayIndex = -1;
	rec._timeout = timeout;
	rec._frameTimer = scene._frameStartTime;
	rec._trigger = endTrigger;
	rec._abortMode = _vm->_game->_triggerSetupMode;

	rec._actionDetails = scene._action._activeAction;

	if (flags & KMSG_PLAYER_TIMEOUT)
		rec._frameTimer = _vm->_game->_player._ticksAmount +
			_vm->_game->_player._priorTimer;

	return idx;
}

int KernelMessages::addQuote(int quoteId, int endTrigger, uint32 timeout) {
	Common::String quoteStr = _vm->_game->getQuote(quoteId);
	return add(Common::Point(), 0x1110, KMSG_PLAYER_TIMEOUT | KMSG_CENTER_ALIGN,
		endTrigger, timeout, quoteStr);
}

void KernelMessages::scrollMessage(int msgIndex, int numTicks, bool quoted) {
	if (msgIndex < 0)
		return;

	_entries[msgIndex]._flags |= quoted ? (KMSG_SCROLL | KMSG_QUOTED) : KMSG_SCROLL;
	_entries[msgIndex]._msgOffset = 0;
	_entries[msgIndex]._numTicks = numTicks;
	_entries[msgIndex]._frameTimer2 = _vm->_game->_priorFrameTimer;

	Common::String msg = _entries[msgIndex]._msg;

	if (_entries[msgIndex]._flags & KMSG_PLAYER_TIMEOUT)
		_entries[msgIndex]._frameTimer2 = _vm->_game->_player._ticksAmount +
		_vm->_game->_player._priorTimer;

	_entries[msgIndex]._frameTimer = _entries[msgIndex]._frameTimer2;
}

void KernelMessages::setSeqIndex(int msgIndex, int seqIndex) {
	if (msgIndex >= 0) {
		_entries[msgIndex]._flags |= KMSG_SEQ_ENTRY;
		_entries[msgIndex]._sequenceIndex = seqIndex;
	}
}

void KernelMessages::remove(int msgIndex) {
	KernelMessage &rec = _entries[msgIndex];
	Scene &scene = _vm->_game->_scene;

	if (rec._flags & KMSG_ACTIVE) {
		if (rec._flags & KMSG_SCROLL) {
			// WORKAROUND: Code here no longer needed in ScummVM
		}

		if (rec._textDisplayIndex >= 0)
			scene._textDisplay.expire(rec._textDisplayIndex);

		rec._flags &= ~KMSG_ACTIVE;
	}
}

void KernelMessages::reset() {
	for (uint i = 0; i < _entries.size(); ++i)
		remove(i);

	_randomMessages.clear();
}

void KernelMessages::update() {
	uint32 currentTimer = _vm->_game->_scene._frameStartTime;

	for (uint i = 0; i < _entries.size() && !_vm->_game->_trigger; ++i) {
		if (_vm->_game->_trigger)
			break;

		KernelMessage &msg = _entries[i];
		if (((msg._flags & KMSG_ACTIVE) != 0) && (currentTimer >= msg._frameTimer))
			processText(i);
	}
}

void KernelMessages::processText(int msgIndex) {
	Scene &scene = _vm->_game->_scene;
	KernelMessage &msg = _entries[msgIndex];
	uint32 currentTimer = scene._frameStartTime;
	bool flag = false;

	if ((msg._flags & KMSG_EXPIRE) != 0) {
		scene._textDisplay.expire(msg._textDisplayIndex);
		msg._flags &= ~KMSG_ACTIVE;
		return;
	}

	if ((msg._flags & KMSG_SCROLL) == 0) {
		msg._timeout -= 3;
	}

	if (msg._flags & KMSG_SEQ_ENTRY) {
		SequenceEntry &seqEntry = scene._sequences[msg._sequenceIndex];
		if (seqEntry._doneFlag || !seqEntry._active)
			msg._timeout = 0;
	}

	if (msg._flags & KMSG_ANIM) {
		warning("TODO: Implement animated text");
	}

	if ((msg._timeout <= 0) && (_vm->_game->_trigger == 0)) {
		msg._flags |= KMSG_EXPIRE;
		if (msg._trigger != 0) {
			_vm->_game->_trigger = msg._trigger;
			_vm->_game->_triggerMode = msg._abortMode;

			if (_vm->_game->_triggerMode != SEQUENCE_TRIGGER_DAEMON) {
				scene._action._activeAction = msg._actionDetails;
			}
		}
	}

	msg._frameTimer = currentTimer + 3;
	int x1 = 0, y1 = 0;

	if (msg._flags & KMSG_SEQ_ENTRY) {
		SequenceEntry &seqEntry = scene._sequences[msg._sequenceIndex];
		if (!seqEntry._nonFixed) {
			SpriteAsset &spriteSet = *scene._sprites[seqEntry._spritesIndex];
			MSprite *frame = spriteSet.getFrame(seqEntry._frameIndex - 1);
			x1 = frame->getBounds().left;
			y1 = frame->getBounds().top;
		} else {
			x1 = seqEntry._position.x;
			y1 = seqEntry._position.y;
		}
	}

	Player &player = _vm->_game->_player;
	if (msg._flags & KMSG_PLAYER_TIMEOUT) {
		if (player._beenVisible) {
			SpriteAsset &asset = *_vm->_game->_scene._sprites[player._spritesStart + player._spritesIdx];
			MSprite *frame = asset.getFrame(player._frameNumber - 1);

			int yAmount = player._currentScale * player._centerOfGravity / 100;
			x1 = player._playerPos.x;
			y1 = (frame->h * player._currentScale / -100) + yAmount +
				player._playerPos.y - 15;
		} else {
			x1 = 160;
			y1 = 78;
		}
	}

	x1 += msg._position.x;
	y1 += msg._position.y;

	Common::String displayMsg = msg._msg;

	if ((msg._flags & KMSG_SCROLL) && (msg._frameTimer >= currentTimer)) {
		++msg._msgOffset;

		if (msg._msgOffset >= msg._msg.size()) {
			// End of message
			msg._flags &= ~KMSG_SCROLL;
		} else {
			displayMsg = Common::String(msg._msg.c_str(), msg._msg.c_str() + msg._msgOffset);
		}

		msg._frameTimer = msg._frameTimer2 = currentTimer + msg._numTicks;
		flag = true;
	}

	int strWidth = _talkFont->getWidth(displayMsg, scene._textSpacing);

	if (msg._flags & (KMSG_RIGHT_ALIGN | KMSG_CENTER_ALIGN)) {
		x1 -= (msg._flags & KMSG_CENTER_ALIGN) ? strWidth / 2 : strWidth;
	}

	// Make sure text appears entirely on-screen
	int x2 = x1 + strWidth;
	if (x2 > MADS_SCREEN_WIDTH)
		x1 -= x2 - MADS_SCREEN_WIDTH;
	if (x1 > (MADS_SCREEN_WIDTH - 1))
		x1 = MADS_SCREEN_WIDTH - 1;
	if (x1 < 0)
		x1 = 0;

	if (y1 >(MADS_SCENE_HEIGHT - 1))
		y1 = MADS_SCENE_HEIGHT - 1;
	if (y1 < 0)
		y1 = 0;

	if (msg._textDisplayIndex >= 0) {
		TextDisplay &textEntry = scene._textDisplay[msg._textDisplayIndex];

		if (flag || (textEntry._bounds.left != x1) || (textEntry._bounds.top != y1)) {
			// Mark the associated text entry as deleted, so it can be re-created
			scene._textDisplay.expire(msg._textDisplayIndex);
			msg._textDisplayIndex = -1;
		}
	}

	if (msg._textDisplayIndex < 0) {
		// Need to create a new text display entry for this message
		int idx = scene._textDisplay.add(x1, y1, msg._color1 | (msg._color2 << 8),
			scene._textSpacing, displayMsg, _talkFont);
		if (idx >= 0)
			msg._textDisplayIndex = idx;
	}
}

void KernelMessages::delay(uint32 priorFrameTime, uint32 currentTime) {
	for (uint i = 0; i < _entries.size(); ++i) {
		_entries[i]._timeout += currentTime - priorFrameTime;
	}
}

void KernelMessages::setQuoted(int msgIndex, int numTicks, bool quoted) {
	if (msgIndex >= 0) {
		KernelMessage &msg = _entries[msgIndex];

		msg._flags |= KMSG_SCROLL;
		if (quoted)
			msg._flags |= KMSG_QUOTED;

		msg._msgOffset = 0;
		msg._numTicks = numTicks;
		msg._frameTimer2 = _vm->_game->_scene._frameStartTime;

		if (msg._flags & KMSG_PLAYER_TIMEOUT) {
			msg._frameTimer2 = _vm->_game->_player._priorTimer +
				_vm->_game->_player._ticksAmount;
		}

		msg._frameTimer = msg._frameTimer2;
	}
}

#define RANDOM_MESSAGE_TRIGGER 240

void KernelMessages::randomServer() {
	if ((_vm->_game->_trigger >= RANDOM_MESSAGE_TRIGGER) &&
		(_vm->_game->_trigger <  (RANDOM_MESSAGE_TRIGGER + (int)_randomMessages.size()))) {
		_randomMessages[_vm->_game->_trigger - RANDOM_MESSAGE_TRIGGER]._handle = -1;
		_randomMessages[_vm->_game->_trigger - RANDOM_MESSAGE_TRIGGER]._quoteId = -1;
	}
}

int KernelMessages::checkRandom() {
	int total = 0;

	for (uint i = 0; i < _randomMessages.size(); ++i) {
		if (_randomMessages[i]._handle >= 0)
			++total;
	}

	return total;
}

bool KernelMessages::generateRandom(int major, int minor) {
	bool generatedMessage = false;

	// Scan through the random messages array
	for (uint msgCtr = 0; msgCtr < _randomMessages.size(); msgCtr++) {
		// Find currently active random messages
		if (_randomMessages[msgCtr]._handle < 0) {
			// Check whether there's any existing 'scrolling in' message
			bool bad = false;
			for (uint scanCtr = 0; scanCtr < _randomMessages.size(); ++scanCtr) {
				if (_randomMessages[scanCtr]._handle >= 0) {
					if (_entries[_randomMessages[scanCtr]._handle]._flags & KMSG_SCROLL) {
						bad = true;
						break;
					}
				}
			}

			// Do a random check for a new message to appear
			if (_vm->getRandomNumber(major) <= minor && !bad) {
				int quoteId;

				// Pick a random quote to display from the available list
				do {
					int quoteIdx = _vm->getRandomNumber(_randomQuotes.size() - 1);
					quoteId = _randomQuotes[quoteIdx];

					// Ensure the quote isn't already in use
					bad = false;
					for (uint scanCtr = 0; scanCtr < _randomMessages.size(); ++scanCtr) {
						if (quoteId == _randomMessages[scanCtr]._quoteId) {
							bad = true;
							break;
						}
					}
				} while (bad);

				// Store the quote Id to be used
				_randomMessages[msgCtr]._quoteId = quoteId;

				// Position the message at a random position
				Common::Point textPos;
				textPos.x = _vm->getRandomNumber(_randomMessages._bounds.left,
					_randomMessages._bounds.right);

				// Figure out Y position, making sure not to be overlapping with
				// any other on-screen message
				int abortCounter = 0;

				do {
					// Ensure we don't get stuck in an infinite loop if too many messages
					// are alrady on-screen
					if (abortCounter++ > 100) goto done;
					bad = false;

					// Set potential new Y position
					textPos.y = _vm->getRandomNumber(_randomMessages._bounds.top,
						_randomMessages._bounds.bottom);

					// Ensure it doesn't overlap an existing on-screen message
					for (uint msgCtr2 = 0; msgCtr2 < _randomMessages.size(); ++msgCtr2) {
						if (_randomMessages[msgCtr2]._handle >= 0) {
							int lastY = _entries[_randomMessages[msgCtr2]._handle]._position.y;

							if ((textPos.y >= (lastY - _randomMessages._randomSpacing)) &&
								(textPos.y <= (lastY + _randomMessages._randomSpacing))) {
								bad = true;
							}
						}
					}
				} while (bad);

				// Add the message
				_randomMessages[msgCtr]._handle = add(textPos, _randomMessages._color, 0,
					RANDOM_MESSAGE_TRIGGER + msgCtr, _randomMessages._duration,
					_vm->_game->getQuote(_randomMessages[msgCtr]._quoteId));

				if (_randomMessages._scrollRate > 0) {
					if (_randomMessages[msgCtr]._handle >= 0) {
						setQuoted(_randomMessages[msgCtr]._handle,
							_randomMessages._scrollRate, true);
					}
				}

				generatedMessage = true;
				break;
			}
		}
	}

done:
	return generatedMessage;
}

void KernelMessages::initRandomMessages(int maxSimultaneousMessages,
		const Common::Rect &bounds, int minYSpacing, int scrollRate,
		int color, int duration, int quoteId, ...) {
	// Reset the random messages list
	_randomMessages.clear();
	_randomMessages.resize(maxSimultaneousMessages);

	// Store passed parameters
	_randomMessages._bounds = bounds;
	_randomMessages._randomSpacing = minYSpacing;
	_randomMessages._scrollRate = scrollRate;
	_randomMessages._color = color;
	_randomMessages._duration = duration;

	// Store the variable length random quote list
	va_list va;
	va_start(va, quoteId);
	_randomQuotes.clear();

	while (quoteId > 0) {
		_randomQuotes.push_back(quoteId);
		assert(_randomQuotes.size() < 100);
		quoteId = va_arg(va, int);
	}

	va_end(va);
}

void KernelMessages::setAnim(int msgId, int seqId, int val3 = 0) {
	if (msgId < 0)
		return;

	_entries[msgId]._flags |= KMSG_ANIM;
	_entries[msgId]._sequenceIndex = seqId;

	warning("TODO: KernelMessages::setAnim, unused parameter");
}


/*------------------------------------------------------------------------*/

TextDisplay::TextDisplay() {
	_active = false;
	_expire = 0;
	_spacing = 0;
	_color1 = 0;
	_color2 = 0;
	_font = nullptr;
}

/*------------------------------------------------------------------------*/

TextDisplayList::TextDisplayList(MADSEngine *vm) : _vm(vm) {
	for (int i = 0; i < TEXT_DISPLAY_SIZE; ++i) {
		TextDisplay rec;
		rec._active = false;
		rec._expire = 0;
		push_back(rec);
	}
}

void TextDisplayList::reset() {
	for (int i = 0; i < TEXT_DISPLAY_SIZE; ++i)
		(*this)[i]._active = false;
}

int TextDisplayList::add(int xp, int yp, uint fontColor, int charSpacing,
		const Common::String &msg, Font *font) {
	int usedSlot = -1;

	for (int idx = 0; idx < TEXT_DISPLAY_SIZE; ++idx) {
		TextDisplay &td = (*this)[idx];
		if (!td._active) {
			usedSlot = idx;

			td._bounds.left = xp;
			td._bounds.top = yp;
			td._font = font;
			td._msg = msg;
			td._bounds.setWidth(font->getWidth(msg, charSpacing));
			td._bounds.setHeight(font->getHeight());
			td._color1 = fontColor & 0xff;
			td._color2 = fontColor >> 8;
			td._spacing = charSpacing;
			td._expire = 1;
			td._active = true;
			break;
		}
	}

	return usedSlot;
}

void TextDisplayList::setDirtyAreas() {
	Scene &scene = _vm->_game->_scene;

	for (uint idx = 0, dirtyIdx = SPRITE_SLOTS_MAX_SIZE; dirtyIdx < size(); ++idx, ++dirtyIdx) {
		if (((*this)[idx]._expire >= 0) || !(*this)[idx]._active) {
			scene._dirtyAreas[dirtyIdx]._active = false;
		} else {
			scene._dirtyAreas[dirtyIdx]._textActive = true;
			scene._dirtyAreas[dirtyIdx].setTextDisplay(&(*this)[idx]);
		}
	}
}

void TextDisplayList::setDirtyAreas2() {
	Scene &scene = _vm->_game->_scene;

	for (uint idx = 0, dirtyIdx = SPRITE_SLOTS_MAX_SIZE; idx < size(); ++idx, ++dirtyIdx) {
		if ((*this)[idx]._active && ((*this)[idx]._expire >= 0)) {
			scene._dirtyAreas[dirtyIdx].setTextDisplay(&(*this)[idx]);
			scene._dirtyAreas[dirtyIdx]._textActive = ((*this)[idx]._expire <= 0) ? 0 : 1;
		}
	}
}

void TextDisplayList::draw(BaseSurface *s) {
	for (uint idx = 0; idx < size(); ++idx) {
		TextDisplay &td = (*this)[idx];
		if (td._active && (td._expire >= 0)) {
			Common::Point destPos(td._bounds.left, td._bounds.top);
			td._font->setColors(0xFF, td._color1, td._color2, 0);
			td._font->writeString(s, td._msg, destPos, td._spacing, td._bounds.width());
		}
	}
}

void TextDisplayList::cleanUp() {
	for (uint idx = 0; idx < size(); ++idx) {
		if ((*this)[idx]._expire < 0) {
			(*this)[idx]._active = false;
			(*this)[idx]._expire = 0;
		}
	}
}

void TextDisplayList::expire(int idx) {
	(*this)[idx]._expire = -1;
}

} // End of namespace MADS
