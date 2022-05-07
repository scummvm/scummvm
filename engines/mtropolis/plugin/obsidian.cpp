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

#include "mtropolis/plugin/obsidian.h"
#include "mtropolis/plugins.h"

#include "mtropolis/miniscript.h"

namespace MTropolis {

namespace Obsidian {

bool MovementModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::MovementModifier &data) {
	return true;
}

Common::SharedPtr<Modifier> MovementModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new MovementModifier(*this));
}

bool RectShiftModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::RectShiftModifier &data) {
	return true;
}

Common::SharedPtr<Modifier> RectShiftModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new RectShiftModifier(*this));
}

TextWorkModifier::TextWorkModifier() : _firstChar(0), _lastChar(0) {
}

bool TextWorkModifier::load(const PlugInModifierLoaderContext &context, const Data::Obsidian::TextWorkModifier &data) {
	return true;
}

bool TextWorkModifier::readAttribute(MiniscriptThread *thread, DynamicValue &result, const Common::String &attrib) {
	if (attrib == "numchar") {
		result.setInt(_string.size());
		return true;
	} else if (attrib == "output") {
		int32 firstChar = _firstChar - 1;
		int32 len = _lastChar - _firstChar + 1;
		if (_firstChar < 0) {
			len += firstChar;
			firstChar = 0;
		}
		if (len <= 0 || static_cast<size_t>(firstChar) >= _string.size())
			result.setString("");
		else {
			const size_t availChars = _string.size() - firstChar;
			if (static_cast<size_t>(len) > availChars)
				len = availChars;
			result.setString(_string.substr(firstChar, len));
		}
		return true;
	} else if (attrib == "exists") {
		bool exists = (_string.find(_token) != Common::String::npos);
		result.setInt(exists ? 1 : 0);
		return true;
	} else if (attrib == "index") {
		size_t index = _string.find(_token);
		if (index == Common::String::npos)
			index = 0;
		else
			index++;

		result.setInt(index);
		return true;
	} else if (attrib == "numword") {
		thread->error("Not-yet-implemented TextWork attribute 'numword'");
		return false;
	}

	return Modifier::readAttribute(thread, result, attrib);
}

MiniscriptInstructionOutcome TextWorkModifier::writeRefAttribute(MiniscriptThread *thread, DynamicValueWriteProxy &result, const Common::String &attrib) {
	if (attrib == "string") {
		DynamicValueWriteStringHelper::create(&_string, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "firstchar") {
		DynamicValueWriteIntegerHelper<int32>::create(&_firstChar, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "lastchar") {
		DynamicValueWriteIntegerHelper<int32>::create(&_lastChar, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "token") {
		DynamicValueWriteStringHelper::create(&_token, result);
		return kMiniscriptInstructionOutcomeContinue;
	} else if (attrib == "firstword") {
		thread->error("Not-yet-implemented TextWork attrib 'firstword'");
		return kMiniscriptInstructionOutcomeFailed;
	} else if (attrib == "lastword") {
		thread->error("Not-yet-implemented TextWork attrib 'lastword'");
		return kMiniscriptInstructionOutcomeFailed;
	}

	return Modifier::writeRefAttribute(thread, result, attrib);
}


Common::SharedPtr<Modifier> TextWorkModifier::shallowClone() const {
	return Common::SharedPtr<Modifier>(new TextWorkModifier(*this));
}

ObsidianPlugIn::ObsidianPlugIn() : _movementModifierFactory(this), _rectShiftModifierFactory(this), _textWorkModifierFactory(this) {
}

void ObsidianPlugIn::registerModifiers(IPlugInModifierRegistrar *registrar) const {
	registrar->registerPlugInModifier("Movement", &_movementModifierFactory);
	registrar->registerPlugInModifier("rectshift", &_rectShiftModifierFactory);
	registrar->registerPlugInModifier("TextWork", &_textWorkModifierFactory);
}

} // End of namespace ObsidianPlugIn

namespace PlugIns {

Common::SharedPtr<PlugIn> createObsidian() {
	return Common::SharedPtr<PlugIn>(new Obsidian::ObsidianPlugIn());
}

} // End of namespace PlugIns

} // End of namespace MTropolis
