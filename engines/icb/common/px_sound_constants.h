/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

namespace ICB {

#define PITCH_MULT                                                                                                                                                                 \
	{                                                                                                                                                                          \
		128, 128, 128, 129, 129, 130, 130, 131, 131, 132, 132, 133, 133, 134, 134, 135, 135, 136, 136, 137, 137, 138, 138, 139, 139, 140, 140, 141, 141, 142, 142, 143,    \
		    143, 144, 144, 145, 145, 146, 146, 147, 147, 148, 148, 149, 150, 150, 151, 151, 152, 152, 153, 153, 154, 154, 155, 156, 156, 157, 157, 158, 158, 159, 160,     \
		    160, 161, 161, 162, 163, 163, 164, 164, 165, 165, 166, 167, 167, 168, 169, 169, 170, 170, 171, 172, 172, 173, 173, 174, 175, 175, 176, 177, 177, 178, 179,     \
		    179, 180, 181, 181, 182, 182, 183, 184, 184, 185, 186, 186, 187, 188, 189, 189, 190, 191, 191, 192, 193, 193, 194, 195, 195, 196, 197, 198, 198, 199, 200,     \
		    200, 201, 202, 203, 203, 204, 205, 206, 206, 207, 208, 209, 209, 210, 211, 212, 212, 213, 214, 215, 216, 216, 217, 218, 219, 219, 220, 221, 222, 223, 223,     \
		    224, 225, 226, 227, 228, 228, 229, 230, 231, 232, 233, 233, 234, 235, 236, 237, 238, 239, 239, 240, 241, 242, 243, 244, 245, 246, 246, 247, 248, 249, 250,     \
		    251, 252, 253, 254, 255                                                                                                                                        \
	}

#define PITCH_DIV                                                                                                                                                                  \
	{                                                                                                                                                                          \
		128, 127, 127, 126, 126, 125, 125, 124, 124, 123, 123, 123, 122, 122, 121, 121, 120, 120, 119, 119, 119, 118, 118, 117, 117, 116, 116, 116, 115, 115, 114, 114,    \
		    114, 113, 113, 112, 112, 111, 111, 111, 110, 110, 109, 109, 109, 108, 108, 108, 107, 107, 106, 106, 106, 105, 105, 104, 104, 104, 103, 103, 103, 102, 102,     \
		    101, 101, 101, 100, 100, 100, 99, 99, 99, 98, 98, 97, 97, 97, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 91, 90, 90, 90, 89, 89, 89,  \
		    88, 88, 88, 87, 87, 87, 86, 86, 86, 86, 85, 85, 85, 84, 84, 84, 83, 83, 83, 82, 82, 82, 82, 81, 81, 81, 80, 80, 80, 80, 79, 79, 79, 78, 78, 78, 78, 77, 77,    \
		    77, 76, 76, 76, 76, 75, 75, 75, 75, 74, 74, 74, 73, 73, 73, 73, 72, 72, 72, 72, 71, 71, 71, 71, 70, 70, 70, 70, 69, 69, 69, 69, 68, 68, 68, 68, 67, 67, 67,    \
		    67, 66, 66, 66, 66, 65, 65, 65, 65, 64, 64, 64, 64                                                                                                             \
	}

#define VOL_FUNCTION                                                                                                                                                               \
	{                                                                                                                                                                          \
		0, 11, 16, 19, 22, 25, 27, 29, 32, 33, 35, 37, 39, 40, 42, 43, 45, 46, 47, 49, 50, 51, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 64, 64, 65, 66, 67, 68, 69, 70, 71, \
		    72, 73, 74, 75, 75, 76, 77, 78, 79, 80, 80, 81, 82, 83, 83, 84, 85, 86, 86, 87, 88, 89, 89, 90, 91, 91, 92, 93, 93, 94, 95, 95, 96, 97, 97, 98, 99, 99, 100,   \
		    101, 101, 102, 103, 103, 104, 104, 105, 106, 106, 107, 107, 108, 109, 109, 110, 110, 111, 111, 112, 113, 113, 114, 114, 115, 115, 116, 117, 117, 118, 118,     \
		    119, 119, 120, 120, 121, 121, 122, 122, 123, 123, 124, 124, 125, 125, 126, 126, 127                                                                            \
	}

} // End of namespace ICB
