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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/illusions.h"
#include "illusions/dictionary.h"
#include "illusions/resources/actorresource.h"
#include "illusions/resources/backgroundresource.h"
#include "illusions/resources/fontresource.h"
#include "illusions/resources/talkresource.h"

namespace Illusions {

void Dictionary::addActorType(uint32 id, ActorType *actorType) {
	_actorTypes.add(id, actorType);
}

void Dictionary::removeActorType(uint32 id) {
	_actorTypes.remove(id);
}

ActorType *Dictionary::findActorType(uint32 id) {
	return _actorTypes.find(id);
}

void Dictionary::addFont(uint32 id, FontResource *fontResource) {
	_fontResources.add(id, fontResource);
}

void Dictionary::removeFont(uint32 id) {
	_fontResources.remove(id);
}

FontResource *Dictionary::findFont(uint32 id) {
	return _fontResources.find(id);
}

void Dictionary::addSequence(uint32 id, Sequence *sequence) {
	_sequences.add(id, sequence);
}

void Dictionary::removeSequence(uint32 id) {
	_sequences.remove(id);
}

Sequence *Dictionary::findSequence(uint32 id) {
	return _sequences.find(id);
}

void Dictionary::addTalkEntry(uint32 id, TalkEntry *talkEntry) {
	_talkEntries.add(id, talkEntry);
}

void Dictionary::removeTalkEntry(uint32 id) {
	_talkEntries.remove(id);
}

TalkEntry *Dictionary::findTalkEntry(uint32 id) {
	return _talkEntries.find(id);
}

void Dictionary::setObjectControl(uint32 objectId, Control *control) {
	if (control)
		_controls.add(objectId, control);
	else
		_controls.remove(objectId);
}

Control *Dictionary::getObjectControl(uint32 objectId) {
	return _controls.find(objectId);
}

} // End of namespace Illusions
