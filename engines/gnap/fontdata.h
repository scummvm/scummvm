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

#ifndef GNAP_FONTDATA_H
#define GNAP_FONTDATA_H

namespace Gnap {

struct FONT_CHAR_INFO {
  const byte _width;              // width, in bits (or pixels), of the character
  const uint16 _offset;           // offset of the character's bitmap, in bytes, into the the FONT_INFO's data array

  FONT_CHAR_INFO(byte width, uint16 offset) : _width(width), _offset(offset) {}
};

/*
**  Font data for DejaVu Sans 9pt
*/

/* Character bitmaps for DejaVu Sans 9pt */
const byte _dejaVuSans9ptCharBitmaps[] = {
	/* @0 ' ' (5 pixels wide) */
	0x00, 0x00, /*                  */
	0x00, 0x00, /*                  */
	0x00, 0x00, /*                  */
	0x00, 0x00, /*                  */
	0x00, 0x00, /*                  */

	/* @10 '!' (1 pixels wide) */
	0x1B, 0xF0, /*    ## ######     */

	/* @12 '"' (3 pixels wide) */
	0x00, 0x70, /*          ###     */
	0x00, 0x00, /*                  */
	0x00, 0x70, /*          ###     */

	/* @18 '#' (8 pixels wide) */
	0x04, 0x00, /*      #           */
	0x14, 0x80, /*    # #  #        */
	0x0F, 0x80, /*     #####        */
	0x04, 0xE0, /*      #  ###      */
	0x1C, 0x80, /*    ###  #        */
	0x07, 0xC0, /*      #####       */
	0x04, 0xA0, /*      #  # #      */
	0x00, 0x80, /*         #        */

	/* @34 '$' (5 pixels wide) */
	0x09, 0xC0, /*     #  ###       */
	0x11, 0x20, /*    #   #  #      */
	0x7F, 0xF0, /*  ###########     */
	0x12, 0x20, /*    #  #   #      */
	0x0E, 0x40, /*     ###  #       */

	/* @44 '%' (10 pixels wide) */
	0x00, 0xE0, /*         ###      */
	0x01, 0x10, /*        #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x0C, 0xE0, /*     ##  ###      */
	0x03, 0x00, /*       ##         */
	0x01, 0x80, /*        ##        */
	0x0E, 0x60, /*     ###  ##      */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x00, /*    #   #         */
	0x0E, 0x00, /*     ###          */

	/* @64 '&' (8 pixels wide) */
	0x0E, 0x00, /*     ###          */
	0x19, 0xE0, /*    ##  ####      */
	0x10, 0x90, /*    #    #  #     */
	0x11, 0x10, /*    #   #   #     */
	0x12, 0x20, /*    #  #   #      */
	0x0C, 0x00, /*     ##           */
	0x14, 0x00, /*    # #           */
	0x13, 0x00, /*    #  ##         */

	/* @80 ''' (1 pixels wide) */
	0x00, 0x70, /*          ###     */

	/* @82 '(' (3 pixels wide) */
	0x07, 0xC0, /*      #####       */
	0x38, 0x38, /*   ###     ###    */
	0x20, 0x08, /*   #         #    */

	/* @88 ')' (3 pixels wide) */
	0x20, 0x08, /*   #         #    */
	0x38, 0x38, /*   ###     ###    */
	0x07, 0xC0, /*      #####       */

	/* @94 '*' (5 pixels wide) */
	0x01, 0x20, /*        #  #      */
	0x00, 0xC0, /*         ##       */
	0x03, 0xF0, /*       ######     */
	0x00, 0xC0, /*         ##       */
	0x01, 0x20, /*        #  #      */

	/* @104 '+' (7 pixels wide) */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */
	0x1F, 0xC0, /*    #######       */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */

	/* @118 ',' (1 pixels wide) */
	0x38, 0x00, /*   ###            */

	/* @120 '-' (3 pixels wide) */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */

	/* @126 '.' (1 pixels wide) */
	0x18, 0x00, /*    ##            */

	/* @128 '/' (4 pixels wide) */
	0x30, 0x00, /*   ##             */
	0x0E, 0x00, /*     ###          */
	0x01, 0xC0, /*        ###       */
	0x00, 0x30, /*           ##     */

	/* @136 '0' (6 pixels wide) */
	0x07, 0xC0, /*      #####       */
	0x18, 0x30, /*    ##     ##     */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x18, 0x30, /*    ##     ##     */
	0x07, 0xC0, /*      #####       */

	/* @148 '1' (5 pixels wide) */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x1F, 0xF0, /*    #########     */
	0x10, 0x00, /*    #             */
	0x10, 0x00, /*    #             */

	/* @158 '2' (6 pixels wide) */
	0x10, 0x20, /*    #      #      */
	0x18, 0x10, /*    ##      #     */
	0x14, 0x10, /*    # #     #     */
	0x12, 0x10, /*    #  #    #     */
	0x11, 0x30, /*    #   #  ##     */
	0x10, 0xE0, /*    #    ###      */

	/* @170 '3' (6 pixels wide) */
	0x08, 0x20, /*     #     #      */
	0x10, 0x10, /*    #       #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x0E, 0xE0, /*     ### ###      */

	/* @182 '4' (6 pixels wide) */
	0x06, 0x00, /*      ##          */
	0x05, 0x80, /*      # ##        */
	0x04, 0x40, /*      #   #       */
	0x04, 0x30, /*      #    ##     */
	0x1F, 0xF0, /*    #########     */
	0x04, 0x00, /*      #           */

	/* @194 '5' (6 pixels wide) */
	0x08, 0xF0, /*     #   ####     */
	0x10, 0x90, /*    #    #  #     */
	0x10, 0x90, /*    #    #  #     */
	0x10, 0x90, /*    #    #  #     */
	0x19, 0x90, /*    ##  ##  #     */
	0x0F, 0x00, /*     ####         */

	/* @206 '6' (6 pixels wide) */
	0x07, 0xC0, /*      #####       */
	0x19, 0x20, /*    ##  #  #      */
	0x10, 0x90, /*    #    #  #     */
	0x10, 0x90, /*    #    #  #     */
	0x19, 0x90, /*    ##  ##  #     */
	0x0F, 0x20, /*     ####  #      */

	/* @218 '7' (6 pixels wide) */
	0x00, 0x10, /*            #     */
	0x10, 0x10, /*    #       #     */
	0x0C, 0x10, /*     ##     #     */
	0x03, 0x10, /*       ##   #     */
	0x00, 0xD0, /*         ## #     */
	0x00, 0x30, /*           ##     */

	/* @230 '8' (6 pixels wide) */
	0x0E, 0xE0, /*     ### ###      */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x0E, 0xE0, /*     ### ###      */

	/* @242 '9' (6 pixels wide) */
	0x09, 0xE0, /*     #  ####      */
	0x13, 0x30, /*    #  ##  ##     */
	0x12, 0x10, /*    #  #    #     */
	0x12, 0x10, /*    #  #    #     */
	0x09, 0x30, /*     #  #  ##     */
	0x07, 0xC0, /*      #####       */

	/* @254 ':' (1 pixels wide) */
	0x19, 0x80, /*    ##  ##        */

	/* @256 ';' (1 pixels wide) */
	0x39, 0x80, /*   ###  ##        */

	/* @258 '<' (8 pixels wide) */
	0x03, 0x00, /*       ##         */
	0x03, 0x00, /*       ##         */
	0x03, 0x00, /*       ##         */
	0x04, 0x80, /*      #  #        */
	0x04, 0x80, /*      #  #        */
	0x04, 0x80, /*      #  #        */
	0x0C, 0xC0, /*     ##  ##       */
	0x08, 0x40, /*     #    #       */

	/* @274 '=' (8 pixels wide) */
	0x05, 0x00, /*      # #         */
	0x05, 0x00, /*      # #         */
	0x05, 0x00, /*      # #         */
	0x05, 0x00, /*      # #         */
	0x05, 0x00, /*      # #         */
	0x05, 0x00, /*      # #         */
	0x05, 0x00, /*      # #         */
	0x05, 0x00, /*      # #         */

	/* @290 '>' (8 pixels wide) */
	0x08, 0x40, /*     #    #       */
	0x0C, 0xC0, /*     ##  ##       */
	0x04, 0x80, /*      #  #        */
	0x04, 0x80, /*      #  #        */
	0x04, 0x80, /*      #  #        */
	0x03, 0x00, /*       ##         */
	0x03, 0x00, /*       ##         */
	0x03, 0x00, /*       ##         */

	/* @306 '?' (5 pixels wide) */
	0x00, 0x20, /*           #      */
	0x00, 0x10, /*            #     */
	0x1B, 0x10, /*    ## ##   #     */
	0x00, 0x90, /*         #  #     */
	0x00, 0x60, /*          ##      */

	/* @316 '@' (11 pixels wide) */
	0x0F, 0x80, /*     #####        */
	0x10, 0x40, /*    #     #       */
	0x20, 0x20, /*   #       #      */
	0x47, 0x10, /*  #   ###   #     */
	0x48, 0x90, /*  #  #   #  #     */
	0x48, 0x90, /*  #  #   #  #     */
	0x48, 0x90, /*  #  #   #  #     */
	0x4F, 0x90, /*  #  #####  #     */
	0x28, 0x20, /*   # #     #      */
	0x04, 0x60, /*      #   ##      */
	0x03, 0x80, /*       ###        */

	/* @338 'A' (8 pixels wide) */
	0x10, 0x00, /*    #             */
	0x0E, 0x00, /*     ###          */
	0x05, 0xC0, /*      # ###       */
	0x04, 0x30, /*      #    ##     */
	0x04, 0x30, /*      #    ##     */
	0x05, 0xC0, /*      # ###       */
	0x0E, 0x00, /*     ###          */
	0x10, 0x00, /*    #             */

	/* @354 'B' (6 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x0E, 0xE0, /*     ### ###      */

	/* @366 'C' (6 pixels wide) */
	0x07, 0xC0, /*      #####       */
	0x08, 0x20, /*     #     #      */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x08, 0x20, /*     #     #      */

	/* @378 'D' (7 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x08, 0x20, /*     #     #      */
	0x07, 0xC0, /*      #####       */

	/* @392 'E' (6 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */

	/* @404 'F' (5 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x01, 0x10, /*        #   #     */
	0x01, 0x10, /*        #   #     */
	0x01, 0x10, /*        #   #     */
	0x01, 0x10, /*        #   #     */

	/* @414 'G' (7 pixels wide) */
	0x07, 0xC0, /*      #####       */
	0x08, 0x20, /*     #     #      */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x0F, 0x20, /*     ####  #      */

	/* @428 'H' (7 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x01, 0x00, /*        #         */
	0x01, 0x00, /*        #         */
	0x01, 0x00, /*        #         */
	0x01, 0x00, /*        #         */
	0x01, 0x00, /*        #         */
	0x1F, 0xF0, /*    #########     */

	/* @442 'I' (1 pixels wide) */
	0x1F, 0xF0, /*    #########     */

	/* @444 'J' (3 pixels wide) */
	0x40, 0x00, /*  #               */
	0x40, 0x00, /*  #               */
	0x3F, 0xF0, /*   ##########     */

	/* @450 'K' (6 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x01, 0x00, /*        #         */
	0x02, 0x80, /*       # #        */
	0x04, 0x40, /*      #   #       */
	0x08, 0x20, /*     #     #      */
	0x10, 0x10, /*    #       #     */

	/* @462 'L' (5 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x10, 0x00, /*    #             */
	0x10, 0x00, /*    #             */
	0x10, 0x00, /*    #             */
	0x10, 0x00, /*    #             */

	/* @472 'M' (8 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x00, 0x60, /*          ##      */
	0x01, 0x80, /*        ##        */
	0x06, 0x00, /*      ##          */
	0x06, 0x00, /*      ##          */
	0x01, 0x80, /*        ##        */
	0x00, 0x60, /*          ##      */
	0x1F, 0xF0, /*    #########     */

	/* @488 'N' (7 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x00, 0x30, /*           ##     */
	0x00, 0xC0, /*         ##       */
	0x01, 0x00, /*        #         */
	0x06, 0x00, /*      ##          */
	0x18, 0x00, /*    ##            */
	0x1F, 0xF0, /*    #########     */

	/* @502 'O' (7 pixels wide) */
	0x07, 0xC0, /*      #####       */
	0x08, 0x20, /*     #     #      */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x08, 0x20, /*     #     #      */
	0x07, 0xC0, /*      #####       */

	/* @516 'P' (6 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x01, 0x10, /*        #   #     */
	0x01, 0x10, /*        #   #     */
	0x01, 0x10, /*        #   #     */
	0x01, 0x10, /*        #   #     */
	0x00, 0xE0, /*         ###      */

	/* @528 'Q' (7 pixels wide) */
	0x07, 0xC0, /*      #####       */
	0x08, 0x20, /*     #     #      */
	0x10, 0x10, /*    #       #     */
	0x10, 0x10, /*    #       #     */
	0x30, 0x10, /*   ##       #     */
	0x48, 0x20, /*  #  #     #      */
	0x07, 0xC0, /*      #####       */

	/* @542 'R' (7 pixels wide) */
	0x1F, 0xF0, /*    #########     */
	0x01, 0x10, /*        #   #     */
	0x01, 0x10, /*        #   #     */
	0x01, 0x10, /*        #   #     */
	0x03, 0x10, /*       ##   #     */
	0x0C, 0xE0, /*     ##  ###      */
	0x10, 0x00, /*    #             */

	/* @556 'S' (6 pixels wide) */
	0x08, 0xE0, /*     #   ###      */
	0x11, 0x90, /*    #   ##  #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x11, 0x10, /*    #   #   #     */
	0x0E, 0x20, /*     ###   #      */

	/* @568 'T' (7 pixels wide) */
	0x00, 0x10, /*            #     */
	0x00, 0x10, /*            #     */
	0x00, 0x10, /*            #     */
	0x1F, 0xF0, /*    #########     */
	0x00, 0x10, /*            #     */
	0x00, 0x10, /*            #     */
	0x00, 0x10, /*            #     */

	/* @582 'U' (7 pixels wide) */
	0x0F, 0xF0, /*     ########     */
	0x18, 0x00, /*    ##            */
	0x10, 0x00, /*    #             */
	0x10, 0x00, /*    #             */
	0x10, 0x00, /*    #             */
	0x18, 0x00, /*    ##            */
	0x0F, 0xF0, /*     ########     */

	/* @596 'V' (8 pixels wide) */
	0x00, 0x30, /*           ##     */
	0x01, 0xC0, /*        ###       */
	0x06, 0x00, /*      ##          */
	0x18, 0x00, /*    ##            */
	0x18, 0x00, /*    ##            */
	0x06, 0x00, /*      ##          */
	0x01, 0xC0, /*        ###       */
	0x00, 0x30, /*           ##     */

	/* @612 'W' (11 pixels wide) */
	0x00, 0x10, /*            #     */
	0x00, 0xE0, /*         ###      */
	0x07, 0x00, /*      ###         */
	0x18, 0x00, /*    ##            */
	0x07, 0x80, /*      ####        */
	0x00, 0x70, /*          ###     */
	0x07, 0x80, /*      ####        */
	0x18, 0x00, /*    ##            */
	0x07, 0x00, /*      ###         */
	0x00, 0xE0, /*         ###      */
	0x00, 0x10, /*            #     */

	/* @634 'X' (7 pixels wide) */
	0x10, 0x10, /*    #       #     */
	0x08, 0x30, /*     #     ##     */
	0x06, 0xC0, /*      ## ##       */
	0x01, 0x00, /*        #         */
	0x06, 0xC0, /*      ## ##       */
	0x08, 0x30, /*     #     ##     */
	0x10, 0x10, /*    #       #     */

	/* @648 'Y' (7 pixels wide) */
	0x00, 0x10, /*            #     */
	0x00, 0x60, /*          ##      */
	0x01, 0x80, /*        ##        */
	0x1E, 0x00, /*    ####          */
	0x01, 0x80, /*        ##        */
	0x00, 0x60, /*          ##      */
	0x00, 0x10, /*            #     */

	/* @662 'Z' (7 pixels wide) */
	0x18, 0x10, /*    ##      #     */
	0x14, 0x10, /*    # #     #     */
	0x12, 0x10, /*    #  #    #     */
	0x11, 0x10, /*    #   #   #     */
	0x10, 0x90, /*    #    #  #     */
	0x10, 0x50, /*    #     # #     */
	0x10, 0x30, /*    #      ##     */

	/* @676 '[' (2 pixels wide) */
	0x7F, 0xF0, /*  ###########     */
	0x40, 0x10, /*  #         #     */

	/* @680 '\' (4 pixels wide) */
	0x00, 0x30, /*           ##     */
	0x01, 0xC0, /*        ###       */
	0x0E, 0x00, /*     ###          */
	0x30, 0x00, /*   ##             */

	/* @688 ']' (2 pixels wide) */
	0x40, 0x10, /*  #         #     */
	0x7F, 0xF0, /*  ###########     */

	/* @692 '^' (6 pixels wide) */
	0x00, 0x40, /*          #       */
	0x00, 0x20, /*           #      */
	0x00, 0x10, /*            #     */
	0x00, 0x10, /*            #     */
	0x00, 0x20, /*           #      */
	0x00, 0x40, /*          #       */

	/* @704 '_' (6 pixels wide) */
	0x80, 0x00, /* #                */
	0x80, 0x00, /* #                */
	0x80, 0x00, /* #                */
	0x80, 0x00, /* #                */
	0x80, 0x00, /* #                */
	0x80, 0x00, /* #                */

	/* @716 '`' (2 pixels wide) */
	0x00, 0x08, /*             #    */
	0x00, 0x10, /*            #     */

	/* @720 'a' (6 pixels wide) */
	0x0C, 0x80, /*     ##  #        */
	0x12, 0x40, /*    #  #  #       */
	0x12, 0x40, /*    #  #  #       */
	0x12, 0x40, /*    #  #  #       */
	0x0A, 0x40, /*     # #  #       */
	0x1F, 0x80, /*    ######        */

	/* @732 'b' (6 pixels wide) */
	0x1F, 0xF8, /*    ##########    */
	0x18, 0xC0, /*    ##   ##       */
	0x10, 0x40, /*    #     #       */
	0x10, 0x40, /*    #     #       */
	0x18, 0xC0, /*    ##   ##       */
	0x0F, 0x80, /*     #####        */

	/* @744 'c' (5 pixels wide) */
	0x0F, 0x80, /*     #####        */
	0x18, 0xC0, /*    ##   ##       */
	0x10, 0x40, /*    #     #       */
	0x10, 0x40, /*    #     #       */
	0x08, 0x80, /*     #   #        */

	/* @754 'd' (6 pixels wide) */
	0x0F, 0x80, /*     #####        */
	0x18, 0xC0, /*    ##   ##       */
	0x10, 0x40, /*    #     #       */
	0x10, 0x40, /*    #     #       */
	0x18, 0xC0, /*    ##   ##       */
	0x1F, 0xF8, /*    ##########    */

	/* @766 'e' (6 pixels wide) */
	0x0F, 0x80, /*     #####        */
	0x0A, 0xC0, /*     # # ##       */
	0x12, 0x40, /*    #  #  #       */
	0x12, 0x40, /*    #  #  #       */
	0x12, 0xC0, /*    #  # ##       */
	0x0B, 0x80, /*     # ###        */

	/* @778 'f' (4 pixels wide) */
	0x00, 0x40, /*          #       */
	0x1F, 0xF0, /*    #########     */
	0x00, 0x48, /*          #  #    */
	0x00, 0x48, /*          #  #    */

	/* @786 'g' (6 pixels wide) */
	0x0F, 0x80, /*     #####        */
	0x58, 0xC0, /*  # ##   ##       */
	0x90, 0x40, /* #  #     #       */
	0x90, 0x40, /* #  #     #       */
	0xD8, 0xC0, /* ## ##   ##       */
	0x7F, 0xC0, /*  #########       */

	/* @798 'h' (6 pixels wide) */
	0x1F, 0xF8, /*    ##########    */
	0x00, 0x80, /*         #        */
	0x00, 0x40, /*          #       */
	0x00, 0x40, /*          #       */
	0x00, 0x40, /*          #       */
	0x1F, 0x80, /*    ######        */

	/* @810 'i' (1 pixels wide) */
	0x1F, 0xD0, /*    ####### #     */

	/* @812 'j' (2 pixels wide) */
	0x80, 0x00, /* #                */
	0xFF, 0xD0, /* ########## #     */

	/* @816 'k' (5 pixels wide) */
	0x1F, 0xF8, /*    ##########    */
	0x02, 0x00, /*       #          */
	0x05, 0x00, /*      # #         */
	0x08, 0x80, /*     #   #        */
	0x10, 0x40, /*    #     #       */

	/* @826 'l' (1 pixels wide) */
	0x1F, 0xF8, /*    ##########    */

	/* @828 'm' (9 pixels wide) */
	0x1F, 0xC0, /*    #######       */
	0x00, 0x40, /*          #       */
	0x00, 0x40, /*          #       */
	0x00, 0x40, /*          #       */
	0x1F, 0x80, /*    ######        */
	0x00, 0x40, /*          #       */
	0x00, 0x40, /*          #       */
	0x00, 0x40, /*          #       */
	0x1F, 0x80, /*    ######        */

	/* @846 'n' (6 pixels wide) */
	0x1F, 0xC0, /*    #######       */
	0x00, 0x80, /*         #        */
	0x00, 0x40, /*          #       */
	0x00, 0x40, /*          #       */
	0x00, 0x40, /*          #       */
	0x1F, 0x80, /*    ######        */

	/* @858 'o' (6 pixels wide) */
	0x0F, 0x80, /*     #####        */
	0x18, 0xC0, /*    ##   ##       */
	0x10, 0x40, /*    #     #       */
	0x10, 0x40, /*    #     #       */
	0x18, 0xC0, /*    ##   ##       */
	0x0F, 0x80, /*     #####        */

	/* @870 'p' (6 pixels wide) */
	0xFF, 0xC0, /* ##########       */
	0x18, 0xC0, /*    ##   ##       */
	0x10, 0x40, /*    #     #       */
	0x10, 0x40, /*    #     #       */
	0x18, 0xC0, /*    ##   ##       */
	0x0F, 0x80, /*     #####        */

	/* @882 'q' (6 pixels wide) */
	0x0F, 0x80, /*     #####        */
	0x18, 0xC0, /*    ##   ##       */
	0x10, 0x40, /*    #     #       */
	0x10, 0x40, /*    #     #       */
	0x18, 0xC0, /*    ##   ##       */
	0xFF, 0xC0, /* ##########       */

	/* @894 'r' (4 pixels wide) */
	0x1F, 0xC0, /*    #######       */
	0x00, 0x80, /*         #        */
	0x00, 0x40, /*          #       */
	0x00, 0x40, /*          #       */

	/* @902 's' (5 pixels wide) */
	0x09, 0x80, /*     #  ##        */
	0x12, 0x40, /*    #  #  #       */
	0x12, 0x40, /*    #  #  #       */
	0x12, 0x40, /*    #  #  #       */
	0x0C, 0x80, /*     ##  #        */

	/* @912 't' (4 pixels wide) */
	0x00, 0x40, /*          #       */
	0x1F, 0xF0, /*    #########     */
	0x10, 0x40, /*    #     #       */
	0x10, 0x40, /*    #     #       */

	/* @920 'u' (6 pixels wide) */
	0x0F, 0xC0, /*     ######       */
	0x10, 0x00, /*    #             */
	0x10, 0x00, /*    #             */
	0x10, 0x00, /*    #             */
	0x08, 0x00, /*     #            */
	0x1F, 0xC0, /*    #######       */

	/* @932 'v' (6 pixels wide) */
	0x00, 0xC0, /*         ##       */
	0x07, 0x00, /*      ###         */
	0x18, 0x00, /*    ##            */
	0x18, 0x00, /*    ##            */
	0x07, 0x00, /*      ###         */
	0x00, 0xC0, /*         ##       */

	/* @944 'w' (9 pixels wide) */
	0x00, 0xC0, /*         ##       */
	0x07, 0x00, /*      ###         */
	0x18, 0x00, /*    ##            */
	0x07, 0x00, /*      ###         */
	0x00, 0xC0, /*         ##       */
	0x07, 0x00, /*      ###         */
	0x18, 0x00, /*    ##            */
	0x07, 0x00, /*      ###         */
	0x00, 0xC0, /*         ##       */

	/* @962 'x' (6 pixels wide) */
	0x10, 0x40, /*    #     #       */
	0x0D, 0x80, /*     ## ##        */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */
	0x0D, 0x80, /*     ## ##        */
	0x10, 0x40, /*    #     #       */

	/* @974 'y' (6 pixels wide) */
	0x80, 0xC0, /* #       ##       */
	0x83, 0x00, /* #     ##         */
	0x4C, 0x00, /*  #  ##           */
	0x38, 0x00, /*   ###            */
	0x07, 0x00, /*      ###         */
	0x00, 0xC0, /*         ##       */

	/* @986 'z' (5 pixels wide) */
	0x18, 0x40, /*    ##    #       */
	0x14, 0x40, /*    # #   #       */
	0x12, 0x40, /*    #  #  #       */
	0x11, 0x40, /*    #   # #       */
	0x10, 0xC0, /*    #    ##       */

	/* @996 '{' (5 pixels wide) */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */
	0x7D, 0xF0, /*  ##### #####     */
	0x40, 0x10, /*  #         #     */
	0x40, 0x10, /*  #         #     */

	/* @1006 '|' (1 pixels wide) */
	0xFF, 0xF0, /* ############     */

	/* @1008 '}' (5 pixels wide) */
	0x40, 0x10, /*  #         #     */
	0x40, 0x10, /*  #         #     */
	0x7D, 0xF0, /*  ##### #####     */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */

	/* @1018 '~' (8 pixels wide) */
	0x02, 0x00, /*       #          */
	0x01, 0x00, /*        #         */
	0x01, 0x00, /*        #         */
	0x01, 0x00, /*        #         */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */
	0x02, 0x00, /*       #          */
	0x01, 0x00, /*        #         */
};

/* Character descriptors for DejaVu Sans 9pt */
/* { [Char width in bits], [Offset into dejaVuSans9ptCharBitmaps in bytes] } */
const FONT_CHAR_INFO _dejaVuSans9ptCharDescriptors[] = {
	FONT_CHAR_INFO(5, 0), 		/*   */
	FONT_CHAR_INFO(1, 10), 		/* ! */
	FONT_CHAR_INFO(3, 12), 		/* " */
	FONT_CHAR_INFO(8, 18), 		/* # */
	FONT_CHAR_INFO(5, 34), 		/* $ */
	FONT_CHAR_INFO(10, 44), 	/* % */
	FONT_CHAR_INFO(8, 64), 		/* & */
	FONT_CHAR_INFO(1, 80), 		/* ' */
	FONT_CHAR_INFO(3, 82), 		/* ( */
	FONT_CHAR_INFO(3, 88), 		/* ) */
	FONT_CHAR_INFO(5, 94), 		/* * */
	FONT_CHAR_INFO(7, 104), 	/* + */
	FONT_CHAR_INFO(1, 118), 	/* , */
	FONT_CHAR_INFO(3, 120), 	/* - */
	FONT_CHAR_INFO(1, 126), 	/* . */
	FONT_CHAR_INFO(4, 128), 	/* / */
	FONT_CHAR_INFO(6, 136), 	/* 0 */
	FONT_CHAR_INFO(5, 148), 	/* 1 */
	FONT_CHAR_INFO(6, 158), 	/* 2 */
	FONT_CHAR_INFO(6, 170), 	/* 3 */
	FONT_CHAR_INFO(6, 182), 	/* 4 */
	FONT_CHAR_INFO(6, 194), 	/* 5 */
	FONT_CHAR_INFO(6, 206), 	/* 6 */
	FONT_CHAR_INFO(6, 218), 	/* 7 */
	FONT_CHAR_INFO(6, 230), 	/* 8 */
	FONT_CHAR_INFO(6, 242), 	/* 9 */
	FONT_CHAR_INFO(1, 254), 	/* : */
	FONT_CHAR_INFO(1, 256), 	/* ; */
	FONT_CHAR_INFO(8, 258), 	/* < */
	FONT_CHAR_INFO(8, 274), 	/* = */
	FONT_CHAR_INFO(8, 290), 	/* > */
	FONT_CHAR_INFO(5, 306), 	/* ? */
	FONT_CHAR_INFO(11, 316), 	/* @ */
	FONT_CHAR_INFO(8, 338), 	/* A */
	FONT_CHAR_INFO(6, 354), 	/* B */
	FONT_CHAR_INFO(6, 366), 	/* C */
	FONT_CHAR_INFO(7, 378), 	/* D */
	FONT_CHAR_INFO(6, 392), 	/* E */
	FONT_CHAR_INFO(5, 404), 	/* F */
	FONT_CHAR_INFO(7, 414), 	/* G */
	FONT_CHAR_INFO(7, 428), 	/* H */
	FONT_CHAR_INFO(1, 442), 	/* I */
	FONT_CHAR_INFO(3, 444), 	/* J */
	FONT_CHAR_INFO(6, 450), 	/* K */
	FONT_CHAR_INFO(5, 462), 	/* L */
	FONT_CHAR_INFO(8, 472), 	/* M */
	FONT_CHAR_INFO(7, 488), 	/* N */
	FONT_CHAR_INFO(7, 502), 	/* O */
	FONT_CHAR_INFO(6, 516), 	/* P */
	FONT_CHAR_INFO(7, 528), 	/* Q */
	FONT_CHAR_INFO(7, 542), 	/* R */
	FONT_CHAR_INFO(6, 556), 	/* S */
	FONT_CHAR_INFO(7, 568), 	/* T */
	FONT_CHAR_INFO(7, 582), 	/* U */
	FONT_CHAR_INFO(8, 596), 	/* V */
	FONT_CHAR_INFO(11, 612), 	/* W */
	FONT_CHAR_INFO(7, 634), 	/* X */
	FONT_CHAR_INFO(7, 648), 	/* Y */
	FONT_CHAR_INFO(7, 662), 	/* Z */
	FONT_CHAR_INFO(2, 676), 	/* [ */
	FONT_CHAR_INFO(4, 680), 	/* \ */
	FONT_CHAR_INFO(2, 688), 	/* ] */
	FONT_CHAR_INFO(6, 692), 	/* ^ */
	FONT_CHAR_INFO(6, 704), 	/* _ */
	FONT_CHAR_INFO(2, 716), 	/* ` */
	FONT_CHAR_INFO(6, 720), 	/* a */
	FONT_CHAR_INFO(6, 732), 	/* b */
	FONT_CHAR_INFO(5, 744), 	/* c */
	FONT_CHAR_INFO(6, 754), 	/* d */
	FONT_CHAR_INFO(6, 766), 	/* e */
	FONT_CHAR_INFO(4, 778), 	/* f */
	FONT_CHAR_INFO(6, 786), 	/* g */
	FONT_CHAR_INFO(6, 798), 	/* h */
	FONT_CHAR_INFO(1, 810), 	/* i */
	FONT_CHAR_INFO(2, 812), 	/* j */
	FONT_CHAR_INFO(5, 816), 	/* k */
	FONT_CHAR_INFO(1, 826), 	/* l */
	FONT_CHAR_INFO(9, 828), 	/* m */
	FONT_CHAR_INFO(6, 846), 	/* n */
	FONT_CHAR_INFO(6, 858), 	/* o */
	FONT_CHAR_INFO(6, 870), 	/* p */
	FONT_CHAR_INFO(6, 882), 	/* q */
	FONT_CHAR_INFO(4, 894), 	/* r */
	FONT_CHAR_INFO(5, 902), 	/* s */
	FONT_CHAR_INFO(4, 912), 	/* t */
	FONT_CHAR_INFO(6, 920), 	/* u */
	FONT_CHAR_INFO(6, 932), 	/* v */
	FONT_CHAR_INFO(9, 944), 	/* w */
	FONT_CHAR_INFO(6, 962), 	/* x */
	FONT_CHAR_INFO(6, 974), 	/* y */
	FONT_CHAR_INFO(5, 986), 	/* z */
	FONT_CHAR_INFO(5, 996), 	/* { */
	FONT_CHAR_INFO(1, 1006), 	/* | */
	FONT_CHAR_INFO(5, 1008), 	/* ) */
	FONT_CHAR_INFO(8, 1018) 	/* ~ */
};

} // End of namespace Gnap

#endif // GNAP_RESOURCE_H
