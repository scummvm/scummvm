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
 *
 *              Originally written by Syn9 in FreeBASIC with SDL
 *              http://syn9.thehideoutgames.com/index_backup.php
 *
 *            Ported to plain C for GCW-Zero handheld by Dmitry Smagin
 *                http://github.com/dmitrysmagin/griffon_legend
 *
 *
 *                 Programming/Graphics: Daniel "Syn9" Kennedy
 *                     Music/Sound effects: David Turner
 *
 *                   Beta testing and gameplay design help:
 *                    Deleter, Cha0s, Aether Fox, and Kiz
 *
 */

#include "griffon/config.h"
#include "common/config-manager.h"

namespace Griffon {

CONFIG config;

void config_load(CONFIG *config) {
	bool mute = false;
	config->music = config->effects = false;

	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	if (!mute) {
		config->music = !ConfMan.getBool("music_mute");
		config->effects = !ConfMan.getBool("sfx_mute");
	}

	config->musicvol = ConfMan.getInt("music_volume");
	config->effectsvol = ConfMan.getInt("sfx_volume");
}

void config_save(CONFIG *config) {
	ConfMan.setBool("mute", !(config->music || config->effectsvol));
	ConfMan.setBool("music_mute", !config->music);
	ConfMan.setBool("sfx_mute", !config->effects);
	ConfMan.setInt("music_volume", config->musicvol);
	ConfMan.setInt("sfx_volume", config->effectsvol);

	ConfMan.flushToDisk();
}

} // end of namespace Griffon
