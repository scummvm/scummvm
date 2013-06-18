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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

namespace Avalanche {

const array<1, 36, char> letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
varying_string<30> name;
varying_string<5> number;

void alphanum();

static string z;

static byte p;


static void replace(char what, char whatwith) {
	p = pos(what, z);
	if (p > 0)  z[p] = whatwith;
}

void alphanum() {
	byte fv;

	z = "";
	for (fv = 1; fv <= length(name); fv ++)
		if (set::of(range('A', 'Z'), eos).has(name[fv]))
			z = z + '7' + name[fv];
		else
			z = z + upcase(name[fv]);
	replace(' ', '1');
	replace('.', '2');
	replace('-', '3');
	replace('\'', '4');
	replace('"', '5');
	replace('!', '6');
	replace(',', '9');
	replace('?', '0');

	for (fv = 1; fv <= length(number); fv ++)
		number[fv] = upcase(number[fv]);

	name = z + '8' + number;
}

void scramble() {
	byte fv, what;
	for (fv = 1; fv <= length(name); fv ++) {
		what = pos(name[fv], letters);
		what += 177;
		what += (fv + 1) * 3;
		name[fv] = letters[(what % 36) + 1 - 1];
	}
}

void checks() {
	byte fv, total;
	total = 177;
	for (fv = 1; fv <= length(name); fv ++)
		total += ord(name[fv]);
	name = string('T') + name + letters[total % 36 - 1];
}

void negate() {
	byte fv;
	name[1] = 'N';
	for (fv = 2; fv <= length(name); fv ++)
		name[fv] = letters[37 - pos(name[fv], letters) - 1];
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	output << "Registrant's name?";
	input >> name >> NL;
	output << "And number (eg, A1)?";
	input >> number >> NL;
	alphanum();
	output << "Name = " << name << NL;
	scramble();
	output << "Scrambled = " << name << NL;
	checks();
	output << "With checks = " << name << NL;
	negate();
	output << "Or, negated, = " << name << NL;
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.