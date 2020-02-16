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

#ifndef BACKENDS_KEYMAPPER_STANDARD_ACTIONS_H
#define BACKENDS_KEYMAPPER_STANDARD_ACTIONS_H

/**
 * @file
 * @brief A set of well known keymapper actions.
 *
 * The actions in this file are meant to be used by game engines
 * when defining their key mappings.
 * Backends can provide default key mappings for some of these actions
 * so users don't have to manually configure the action mappings for
 * the input devices.
 */

namespace Common {

extern const char *kStandardActionInteract;
extern const char *kStandardActionSkip;
extern const char *kStandardActionPause;
extern const char *kStandardActionMoveUp;
extern const char *kStandardActionMoveDown;
extern const char *kStandardActionMoveLeft;
extern const char *kStandardActionMoveRight;
extern const char *kStandardActionOpenMainMenu;
extern const char *kStandardActionLoad;
extern const char *kStandardActionSave;
extern const char *kStandardActionOpenSettings;

} //namespace Common

#endif // BACKENDS_KEYMAPPER_STANDARD_ACTIONS_H
