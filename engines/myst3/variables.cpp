/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
#include "engines/myst3/variables.h"

namespace Myst3 {

Variables::Variables(Myst3Engine *vm):
	_vm(vm) {

#define VAR(var, x, unk) _descriptions.setVal(var, Description(var, #x, unk));

	VAR(61, LocationAge, false)
	VAR(62, LocationRoom, false)
	VAR(63, LocationNode, false)
	VAR(64, BookSavedAge, false)
	VAR(65, BookSavedRoom, false)
	VAR(66, BookSavedNode, false)

	VAR(115, SunspotIntensity, false)
	VAR(116, SunspotColor, false)
	VAR(117, SunspotRadius, false)

	VAR(142, MovieOverrideStartFrame, true)
	VAR(143, MovieOverrideEndFrame, true)
	VAR(144, MovieVolume1, true)
	VAR(145, MovieVolume2, true)
	VAR(146, MovieUnk146, true)
	VAR(147, MovieUnk147, true)
	VAR(148, MovieUnk148, true)
	VAR(149, MovieConditionBit, true)
	VAR(150, MoviePreloadToMemory, true)
	VAR(151, MovieScriptDriven, true)
	VAR(152, MovieNextFrameSetVar, true)
	VAR(153, MovieNextFrameGetVar, true)
	VAR(154, MovieStartFrameVar, true)
	VAR(155, MovieEndFrameVar, true)
	VAR(156, MovieForce2d, true)
	VAR(157, MovieVolumeVar, true)
	VAR(158, MovieSoundHeading, true)
	VAR(159, MoviePanningStrenght, true)
	VAR(160, MovieSynchronized, true)
	VAR(161, MovieUnk161, true)
	VAR(162, MovieUnk162, true)
	VAR(163, MovieOverrideCondition, true)
	VAR(164, MovieUVar, true)
	VAR(165, MovieVVar, true)
	VAR(166, MovieOverridePosition, true)
	VAR(167, MovieOverridePosU, true)
	VAR(168, MovieOverridePosV, true)
	VAR(169, MovieScale, true)
	VAR(170, MovieUnk170, true)
	VAR(171, MovieUnk171, true)
	VAR(172, MovieUnk172, true)
	VAR(173, MoviePlayingVar, true)

	VAR(178, MovieUnk178, true)

	VAR(189, LocationNextNode, false)
	VAR(190, LocationNextRoom, false)
	VAR(191, LocationNextAge, false)

	VAR(277, JournalAtrusState, false)
	VAR(279, JournalSaavedroState, false)
	VAR(280, JournalSaavedroClosed, false)
	VAR(281, JournalSaavedroOpen, false)
	VAR(282, JournalSaavedroLastPage, false)
	VAR(283, JournalSaavedroChapter, false)
	VAR(284, JournalSaavedroPageInChapter, false)
	VAR(480, BookStateTomahna, false)
	VAR(481, BookStateReleeshahn, false)

#undef VAR

	memset(&_vars, 0, sizeof(_vars));
	_vars[1] = 1;
}

Variables::~Variables() {
}

void Variables::checkRange(uint16 var) {
	if (var < 1 || var > 2047)
		error("Variable out of range %d", var);
}

uint32 Variables::get(uint16 var) {
	checkRange(var);
	return _vars[var];
}

void Variables::set(uint16 var, uint32 value) {
	checkRange(var);

	if (_descriptions.contains(var)) {
		const Description &d = _descriptions.getVal(var);
		if (d.unknown)
			warning("A script is writing to the unimplemented engine-mapped var %d (%s)", var, d.name);
	}

	_vars[var] = value;
}

bool Variables::evaluate(int16 condition) {
	uint16 unsignedCond = abs(condition);
	uint16 var = unsignedCond & 2047;
	int32 varValue = get(var);
	int32 targetValue = (unsignedCond >> 11) - 1;

	if (targetValue >= 0) {
		if (condition >= 0)
			return varValue == targetValue;
		else
			return varValue != targetValue;
	} else {
		if (condition >= 0)
			return varValue != 0;
		else
			return varValue == 0;
	}
}

uint32 Variables::valueOrVarValue(int16 value) {
	if (value < 0)
		return get(-value);

	return value;
}

uint32 Variables::engineGet(uint16 var) {
	if (!_descriptions.contains(var))
		error("The engine is trying to access an undescribed var (%d)", var);

	return _vars[var];
}

void Variables::engineSet(uint16 var, uint32 value) {
	if (!_descriptions.contains(var))
		error("The engine is trying to access an undescribed var (%d)", var);

	_vars[var] = value;
}

const Common::String Variables::describeVar(uint16 var) {
	if (_descriptions.contains(var)) {
		const Description &d = _descriptions.getVal(var);

		return Common::String::format("v%s", d.name);
	} else {
		return Common::String::format("v%d", var);
	}
}

const Common::String Variables::describeCondition(int16 condition) {
	uint16 unsignedCond = abs(condition);
	uint16 var = unsignedCond & 2047;
	int16 value = (unsignedCond >> 11) - 1;

	return Common::String::format("c[%s %s %d]",
			describeVar(var).c_str(),
			(condition >= 0 && value >= 0) || (condition < 0 && value < 0) ? "==" : "!=",
			value >= 0 ? value : 0);
}

} /* namespace Myst3 */
