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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "macventure/prebuilt_dialogs.h"

namespace MacVenture {

const PrebuiltDialog g_prebuiltDialogs[kPrebuiltDialogCount] = {

	{/* kSaveAsDialog */
		{0, 146, 456, 254},
		{
			{kDEButton, "YES", kDASaveAs, 24, 68, 120, 22},
			{kDEButton, "NO", kDACloseDialog, 168, 68, 120, 22},
			{kDEButton, "CANCEL", kDACloseDialog, 312, 68, 120, 22},
			{kDEPlainText, "Save As...", kDANone, 100, 10, 340, 38},
			{kDETextInput, "", kDANone, 100, 30, 340, 20},
			{kDEEnd, "", kDANone, 0, 0, 0, 0}
		}
	},

	{ /* kSpeakDialog */
		{20, 92, 400, 200},
		{
			{kDEButton, "OK", kDASubmit, 10, 70, 50, 20},
			{kDEButton, "CANCEL", kDACloseDialog, 96, 70, 50, 20},
			{kDEPlainText, "What would you like to say?", kDANone, 10, 10, 400, 20},
			{kDETextInput, "", kDANone, 10, 25, 350, 40},
			{kDEEnd, "", kDANone, 0, 0, 0, 0}
		}
	},

	{ /* kWinGameDialog */
		{20, 100, 320, 200},
		{
			{kDEPlainText, "You Won!", kDANone, 20, 16, 280, 20},
			{kDEPlainText, "What do you want to do?", kDANone, 20, 30, 280, 20},
			{kDEButton, "New Game", kDANewGame, 20, 60, 70, 20},
			{kDEButton, "Load", kDALoadGame, 110, 60, 70, 20},
			{kDEButton, "Quit", kDAQuit, 200, 60, 70, 20},
			{kDEEnd, "", kDANone, 0, 0, 0, 0}
		}
	},

	{ /* kLoseGameDialog */
		{20, 100, 320, 200},
		{
			{kDEPlainText, "You Died", kDANone, 20, 16, 280, 20},
			{kDEPlainText, "What do you want to do?", kDANone, 20, 30, 280, 20},
			{kDEButton, "New Game", kDANewGame, 20, 60, 70, 20},
			{kDEButton, "Load", kDALoadGame, 110, 60, 70, 20},
			{kDEButton, "Quit", kDAQuit, 200, 60, 70, 20},
			{kDEEnd, "", kDANone, 0, 0, 0, 0}
		}
	}
};
} // End of namespace MacVenture
