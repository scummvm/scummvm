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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

namespace tSage {

static const tSageGameDescription gameDescriptions[] = {

	// Ringworld English CD version
	{
		{
			"ring",
			"CD",
			AD_ENTRY1s("ring.rlb", "466f0e6492d9d0f34d35c5cd088de90f", 37847618),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_Ringworld,
		GF_CD
	},
	// Ringworld First Wave English CD version
	{
		{
			"ring",
			"CD",
			AD_ENTRY1s("ring.rlb", "0a25b4ee58d44a54425c0b47e5096bbc", 37847618),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_Ringworld,
		GF_CD
	},
	// Ringworld English Floppy version
	{
		{
			"ring",
			"Floppy",
			AD_ENTRY1s("ring.rlb", "61f78f68a56832ae95fe06748c403234", 8438770),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_Ringworld,
		GF_FLOPPY
	},

	// Blue Force
	{
		{
			"blueforce",
			"",
			AD_ENTRY1s("blue.rlb", "467da43c848cc0e800b547c59d84ccb1", 10032614),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_BlueForce,
		GF_FLOPPY
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace tSage
