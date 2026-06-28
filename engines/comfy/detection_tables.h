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

namespace Comfy {

const PlainGameDescriptor comfyGames[] = {
	{ "comfyland", "Comfy Land" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"comfyland",
		nullptr,
		{
			{"ENGLISH/COMFY.OBJ", 0, "7b4ce03e92a27c2271d3657d3c13084b", 221184},
			{"SETLANG/PICFILE.DAT", 0, "88dbbc127cd269b286f4d8c0adad0f1b", 29789},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Comfy
