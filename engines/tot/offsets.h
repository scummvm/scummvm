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
#include "common/scummsys.h"


namespace Tot {

	const int FONT_LITT_OFFSET_ES = 153584;
	const int FONT_EURO_OFFSET_ES = 159681;
	const int FONT_LITT_OFFSET_EN = 153010;
	const int FONT_EURO_OFFSET_EN = 159107;

    static const long menuOffsets_ES[8][2] = {
        {0,     16004},
        {16004, 24535},
        {40539, 24535},
        {65074, 26745},
        {91819, 16004},
        {107823, 24535},
        {132358, 14969},
        {147327, 7148},
    };

    static const long menuOffsets_EN[8][2] = {
        {0,     16004},
        {16004, 24535},
        {40539, 24535},
        {65074, 26745},
        {91819, 16004},
        {107823, 24535},
        {132358, 14417},
        {146775, 7148},
    };

    static const long flcOffsets[2][32] = {
        {
            0, //Logo
            837602, // Menu reveal
            // 2 - Intro vids
            888366, // Tv speaks
            908896,
            972458,  // opens car and gets to door
            1269108, // knocks on door
            1284784, // talks to door
            1299404, // doorman opens door
            1321354, // doorman speaks,
            1334608, // talks to doorman at door
            1349402, // doorman walks to car
            1463880, // doorman talks next to car

            1471630, // talks to doorman next to car
            1480982, // doorman opens car
            1500902, // doorman talks with car open
            1508036, // doorman leaves with car
            1701780, // main char speaks alone
            // 17 Sacrifice
            1712400, // character drinks from cup,
            1783054, // prayers
            1806212, // sacrifice
            // 20 Others
            2124896, // Use sharp scythe with statue,
            2216848, // change feather for scythe
            2209158, // Use ring with pedestal,
            2076280, // Use urn with altar
            2441804, // Sharpen scythe
            2361800, // Use rope with precipice
            2382552, // Trident with dent
            2464240, // Rock with mural
            2296092, // Press button of oven with the jar
            2231140, // Press button of oven with no jar
            // 30 Ending
            2481274, // Part 1
            2554766  // Part 2
        },
        {
            0, //Logo
            837602, // Menu reveal
            //Intro vids
            894900, // tv speaks
            915430,
            978992,  // opens car and gets to door
            1275642, // knocks door
            1291318, // talks to door
            1305938, // doorman opens door
            1327888, // doorman speaks
            1341142, // talks to doorman at door,
            1355936, // doorman walks to car
            1470414, // doorman talks next to car
            1478164, // talks to doorman next to car
            1487516, // doorman opens car
            1507436, // doorman talks with car open
            1514570, // doorman leaves with car
            1708314, // main char speaks alone
            // 17 Sacrifice
            1718934, // character drinks from cup
            1789588, // prayers
            1812746, // sacrifice
            // 20 others
            2399068, // Use sharp scythe with statue
            2491020, // change feather for scythe
            2483330, // Use ring with pedestal
            2350452, // Use urn with altar
            2715976, // Sharpen scythe
            2635972, // Use rope with precipice
            2656724, // Trident with dent
            2738412, // Rock with mural
            2570264, // Press button of oven with the jar
            2505312, // Press button of oven with no jar,
            // 30 Ending
            2082814, // Part 1
            2156306  // Part 2
        }
    };

} // End of namespace Tot
