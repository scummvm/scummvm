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

/* Detection tables for Croustibat. */

#ifndef GOB_DETECTION_TABLES_CROUSTI_H
#define GOB_DETECTION_TABLES_CROUSTI_H

// -- DOS VGA Floppy --

{
	{
		"crousti",
		"",
		AD_ENTRY1s("intro.stk", "63fd795818fa72c32b903bbd99e18ea1", 851926),
		PT_PRT,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeCrousti,
	kFeaturesAdLib,
	0, 0, 0
},
{
	{ // -- Fan Translation by BJNFNE --
		"crousti", 
		"",
		AD_ENTRY1s("intro.stk", "d71d49d89295b2f47182c0455a49bf6a", 864728),
		DE_DEU,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kGameTypeCrousti,
	kFeaturesAdLib,
	0, 0, 0
},

#endif // GOB_DETECTION_TABLES_CROUSTI_H
