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

#include "engines/nancy/util.h"

#include "engines/nancy/ui/ndui/common.h"

namespace Nancy {
namespace UI {
namespace NDUI {

// An event's value comes from this field instead, unless it holds this
static const int16 kNoIntValue = -4;

void Action::read(Common::SeekableReadStream &stream) {
	eventType = stream.readSint32LE();
	type = stream.readSint32LE();
	readFilename(stream, target);
	hasValue = stream.readByte();
	value = stream.readDoubleLE();

	int16 intValue = stream.readSint16LE();
	if (intValue != kNoIntValue) {
		value = intValue;
	}

	readFilename(stream, text);
}

double encodeRect(int32 left, int32 top, int32 right, int32 bottom) {
	if (left >= 0xFFFF || top >= 0xFFFF || right >= 0xFFFF || bottom >= 0xFFFF) {
		return -1.0;
	}

	const uint64 bits = (uint64)(uint16)left | ((uint64)(uint16)top << 16) |
		((uint64)(uint16)right << 32) | ((uint64)(uint16)bottom << 48);

	double value;
	memcpy(&value, &bits, sizeof(value));
	return value;
}

Common::Rect decodeRect(double value) {
	uint64 bits;
	memcpy(&bits, &value, sizeof(bits));

	Common::Rect rect;
	rect.left = (int16)(bits & 0xFFFF);
	rect.top = (int16)((bits >> 16) & 0xFFFF);
	rect.right = (int16)((bits >> 32) & 0xFFFF);
	rect.bottom = (int16)((bits >> 48) & 0xFFFF);
	return rect;
}

static void addStateChange(Common::Array<UIStateData::Change> &changes, const Common::String &name, int32 action) {
	UIStateData::Change change;
	change.name = name;
	change.action = action;
	changes.push_back(change);
}

void addStateChanges(Common::Array<UIStateData::Change> &changes, const Common::String &name, int32 initialState, bool visible, bool enabled) {
	if (visible != (initialState != kStartHidden)) {
		addStateChange(changes, name, visible ? kActionShow : kActionHide);
	}

	if (enabled != (initialState != kStartDisabled)) {
		addStateChange(changes, name, enabled ? kActionEnable : kActionDisable);
	}
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
