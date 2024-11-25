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

#include "common/array.h"
#include "director/types.h"

namespace Director {

const char *const scriptTypes[] = {
	"ScoreScript",
	"CastScript",
	"MovieScript",
	"EventScript",
	"TestScript",
	"ScriptType5",
	"ScriptType6",
	"ParentScript",
};

const char *scriptType2str(ScriptType scr) {
	if (scr < 0)
		return "NoneScript";

	if (scr > kMaxScriptType) {
		warning("BUILDBOT: Unknown scriptType: %d", scr);
		return "<unknown>";
	}

	return scriptTypes[scr];
}

const char *const castTypes[] = {
	"empty",
	"bitmap",
	"filmLoop",
	"text",
	"palette",
	"picture",
	"sound",
	"button",
	"shape",
	"movie",
	"digitalVideo",
	"script",
	"RTE",
	"???",
	"transition",
};

const char *castType2str(CastType type) {
	if (type == kCastTypeAny)
		return "any";

	if (type <= kCastTransition)
		return castTypes[type];

	warning("BUILDBOT: Unknown castType: %d", type);
	return "<unknown>";
}

const char *const spriteType[] = {
	"Inactive",
	"Bitmap",
	"Rectangle",
	"RoundedRectangle",
	"Oval",
	"LineTopBottom",
	"LineBottomTop",
	"Text",
	"Button",
	"Checkbox",
	"RadioButton",
	"Pict",
	"OutlinedRectangle",
	"OutlinedRoundedRectangle",
	"OutlinedOval",
	"ThickLine",
	"CastMember",
	"FilmLoop",
	"DirMovie",
};

const char *spriteType2str(SpriteType type) {
	if (type >= kInactiveSprite && type <= kDirMovieSprite)
		return spriteType[type];

	warning("BUILDBOT: Unknown spriteType: %d", type);
	return "<unknown>";
}

const char *const inkType[] = {
	"Copy",
	"Transparent",
	"Reverse",
	"Ghost",
	"NotCopy",
	"NotTrans",
	"NotReverse",
	"NotGhost",
	"Matte",
	"Mask",

	"Blend", // 32
	"AddPin",
	"Add",
	"SubPin",
	"BackgndTrans",
	"Light",
	"Sub",
	"Dark"
};


const char *inkType2str(InkType type) {
	if (type <= kInkTypeMask)
		return inkType[type];

	if (type >= kInkTypeBlend && type <= kInkTypeDark)
		return inkType[type - 32 + 10];

	return "<unknown>";

}

} // End of namespace Director
