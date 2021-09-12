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

#ifndef CHEWY_FEHLER_H
#define CHEWY_FEHLER_H

namespace Chewy {

#define MAX_MODULE 6
#define GERMAN 1
#define ENGL 2
extern int16 modul;
extern int16 fcode;

class fehler {
public:
	fehler();
	~fehler();

	void msg();
	void set_user_msg(const char *msg);
	Common::String get_user_msg() const { return user_msg; }
private:
	char err_txt[120];
	char err_dat[80];
	int16 language;
	const char *user_msg;
};

} // namespace Chewy

#endif
