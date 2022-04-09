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

#include "mtropolis/miniscript.h"
#include "mtropolis/modifiers.h"
#include "mtropolis/modifier_factory.h"

#include "common/memstream.h"

namespace MTropolis {

static MessageFlags translateMessengerFlags(uint32 messengerFlags) {
	MessageFlags messageFlags;
	messageFlags.relay = ((messengerFlags & 0x20000000) == 0);
	messageFlags.cascade = ((messengerFlags & 0x40000000) == 0);
	messageFlags.immediate = ((messengerFlags & 0x80000000) == 0);
	return messageFlags;
}

bool BehaviorModifier::load(ModifierLoaderContext &context, const Data::BehaviorModifier &data) {
	if (data.numChildren > 0) {
		ChildLoaderContext loaderContext;
		loaderContext.containerUnion.modifierContainer = this;
		loaderContext.type = ChildLoaderContext::kTypeModifierList;
		loaderContext.remainingCount = data.numChildren;

		context.childLoaderStack->contexts.push_back(loaderContext);
	}

	if (!_enableWhen.load(data.enableWhen) || !_disableWhen.load(data.disableWhen))
		return false;

	_guid = data.guid;
	_name = data.name;

	return true;
}

const Common::Array<Common::SharedPtr<Modifier> > &BehaviorModifier::getModifiers() const {
	return _children;
}

void BehaviorModifier::appendModifier(const Common::SharedPtr<Modifier> &modifier) {
	_children.push_back(modifier);
}


// Miniscript modifier
bool MiniscriptModifier::load(ModifierLoaderContext &context, const Data::MiniscriptModifier &data) {
	if (!_enableWhen.load(data.enableWhen))
		return false;

	_program = MiniscriptParser::parse(data.program);
	if (!_program)
		return false;

	return true;
}

bool MessengerModifier::load(ModifierLoaderContext &context, const Data::MessengerModifier &data) {
	_guid = data.modHeader.guid;
	_name = data.modHeader.name;

	if (!_when.load(data.when) || !_send.load(data.send))
		return false;

	_messageFlags = translateMessengerFlags(data.messageFlags);
	_messageWithType = static_cast<MessageWithType>(data.with);
	_messageDestination = data.destination;

	return true;
}

bool IfMessengerModifier::load(ModifierLoaderContext &context, const Data::IfMessengerModifier &data) {
	_guid = data.modHeader.guid;
	_name = data.modHeader.name;

	if (!_when.load(data.when) || !_send.load(data.send))
		return false;

	_messageFlags = translateMessengerFlags(data.messageFlags);
	_messageWithType = static_cast<MessageWithType>(data.with);
	_messageDestination = data.destination;

	_program = MiniscriptParser::parse(data.program);
	if (!_program)
		return false;

	return true;
}

bool BooleanVariableModifier::load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data) {
	_guid = data.modHeader.guid;
	_name = data.modHeader.name;

	_value = (data.value != 0);

	return true;
}

bool PointVariableModifier::load(ModifierLoaderContext &context, const Data::PointVariableModifier &data) {
	_guid = data.modHeader.guid;
	_name = data.modHeader.name;

	_value.x = data.value.x;
	_value.y = data.value.y;

	return true;
}

} // End of namespace MTropolis
