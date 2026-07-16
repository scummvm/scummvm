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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/milestones.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "ripper/detection.h"
#include "ripper/resources.h"

namespace Ripper {

Milestones::Milestones() {
	memset(_flags, 0, sizeof(_flags));
}

bool Milestones::initialize(ResourceManager &resources) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		resources.scripts().createReadStreamForMember("mileston.def"));
	if (!stream)
		return false;

	_labels.clear();
	_labels.resize(kFlagCount);
	uint labelCount = 0;
	while (!stream->eos()) {
		Common::String line = stream->readLine();
		line.trim();
		if (line.empty() || line[0] == '#')
			continue;

		uint cursor = 0;
		uint flag = 0;
		while (cursor < line.size() && line[cursor] >= '0' && line[cursor] <= '9') {
			flag = flag * 10 + line[cursor] - '0';
			++cursor;
		}
		if (cursor == 0 || cursor >= line.size() ||
			(line[cursor] != ' ' && line[cursor] != '\t') || flag >= kFlagCount) {
			warning("Ripper: invalid MILESTON.DEF line '%s'", line.c_str());
			return false;
		}
		while (cursor < line.size() && (line[cursor] == ' ' || line[cursor] == '\t'))
			++cursor;
		Common::String value = line.substr(cursor);
		value.trim();
		if (value.empty()) {
			warning("Ripper: empty MILESTON.DEF label for flag %u", flag);
			return false;
		}
		if (_labels[flag].empty())
			++labelCount;
		_labels[flag] = value;
	}

	debugC(1, kDebugMilestones,
		"Ripper: loaded milestone definitions labels=%u capacity=%u storageBytes=%u",
		labelCount, kFlagCount, kStorageByteCount);
	debugState("definition-load");
	return labelCount != 0;
}

bool Milestones::isSet(uint flag) const {
	return flag < kFlagCount && (_flags[flag / 8] & (1 << (flag % 8))) != 0;
}

void Milestones::setBit(uint flag, bool value) {
	const byte mask = 1 << (flag % 8);
	if (value)
		_flags[flag / 8] |= mask;
	else
		_flags[flag / 8] &= ~mask;
}

bool Milestones::set(uint flag, bool value, const char *source) {
	if (flag >= kFlagCount) {
		warning("Ripper: milestone flag %u is outside the %u-flag store", flag, kFlagCount);
		return false;
	}

	const bool changed = isSet(flag) != value;
	setBit(flag, value);
	debugC(changed ? 2 : 3, kDebugMilestones,
		"Ripper: milestone flag=%u label='%s' domain='%s' value=%d changed=%d source=%s",
		flag, label(flag).c_str(), domain(flag), value, changed, source ? source : "unknown");
	return true;
}

bool Milestones::syncGame(Common::Serializer &serializer) {
	if (serializer.isLoading())
		memset(_flags, 0, sizeof(_flags));

	uint setCount = 0;
	// The original game packs this store into 125 bytes. Ripper's existing
	// ScummVM save format writes one byte per flag, so retain that layout.
	for (uint flag = 0; flag < kFlagCount; ++flag) {
		byte value = isSet(flag) ? 1 : 0;
		serializer.syncAsByte(value);
		if (serializer.isLoading())
			setBit(flag, value != 0);
		if (value != 0)
			++setCount;
	}

	debugC(2, kDebugMilestones,
		"Ripper: %s milestone state flags=%u set=%u storageBytes=%u",
		serializer.isLoading() ? "restored" : "saved", kFlagCount, setCount,
		kStorageByteCount);
	debugState(serializer.isLoading() ? "save-restore" : "save-write");
	return !serializer.err();
}

void Milestones::debugState(const char *source) const {
	// LoadStartupKeyedTextTable at 0x1f169 enumerates every numeric key in
	// MILESTON.DEF. Keep the detailed ScummVM dump limited to those same
	// definitions rather than printing all 1,000 bits in the backing store.
	for (uint flag = 0; flag < _labels.size(); ++flag) {
		if (_labels[flag].empty())
			continue;

		debugC(3, kDebugMilestones,
			"Ripper: milestone state flag=%u label='%s' domain='%s' value=%d source=%s",
			flag, _labels[flag].c_str(), domain(flag), isSet(flag), source);
	}
}

const Common::String &Milestones::label(uint flag) const {
	static const Common::String empty;
	return flag < _labels.size() ? _labels[flag] : empty;
}

const char *Milestones::domain(uint flag) {
	if (flag >= kMilestoneCompletedPrologue && flag <= kMilestoneCompletedAct3)
		return "chapter";
	if (flag >= kMilestoneFirstTravelLocation && flag <= kMilestoneLastTravelLocation)
		return "travel";
	if (flag >= 50 && flag <= 58)
		return "inventory";
	if (flag >= kMilestoneFirstWacDatabaseEntry && flag < kMilestoneFirstWacDatabaseEntry + 30)
		return "wac-database";
	if (flag >= 200 && flag <= 231)
		return "puzzle";
	if (flag >= 250 && flag <= 266)
		return "journal";
	if (flag >= 300 && flag <= 399)
		return "story";
	if (flag >= 400 && flag <= 436)
		return "cyberspace";
	return "general";
}

} // End of namespace Ripper
