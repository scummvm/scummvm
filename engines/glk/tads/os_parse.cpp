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

#include "glk/tads/os_parse.h"
#include "glk/glk_types.h"

namespace Glk {
namespace TADS {

#ifdef GLK_MODULE_UNICODE

enum CHARMAPS { OS_UTF8, OS_CP1251, OS_CP1252, OS_MACROMAN, OS_UNKNOWN };

static uint os_charmap = OS_UTF8;

uint is_cyrillic(unsigned char ch)
{
    if (ch >= 0xBC)
        return 1;

    switch (ch)
    {
        case 0x80:
        case 0x81:
        case 0x83:
        case 0x8A:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F:
        case 0x90:
        case 0x9A:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA5:
        case 0xA8:
        case 0xAA:
        case 0xAF:
        case 0xB2:
        case 0xB3:
        case 0xB4:
        case 0xB8:
        case 0xBA:
            return 1;

        default:
            return 0;
    }
}

uint is_macroman (unsigned char ch)
{
    switch (ch)
    {
        /* trademarks */
        case 0xA8:
        case 0xAA:
            return 1;

        /* dashes and right quotes */
        case 0xD0:
        case 0xD1:
        case 0xD3:
        case 0xD5:
            return 1;

        /* accents */
        case 0x8E:
        case 0x8F:
        case 0x9A:
            return 1;

        default:
            return 0;
    }
}

uint is_cp1252 (unsigned char ch)
{
    switch (ch)
    {
        /* trademarks */
        case 0x99:
        case 0xAE:
            return 1;

        /* dashes and right quotes */
        case 0x92:
        case 0x94:
        case 0x96:
        case 0x97:
            return 1;

        /* accents */
        case 0xE8:
        case 0xE9:
        case 0xF6:
            return 1;

        default:
            return 0;
    }
}

uint identify_chars(const unsigned char *buf, uint buflen, uint32 *out, uint outlen) {
    uint pos = 0;
    uint val = 0;
    uint count_macroman = 0;
    uint count_cp1252 = 0;
    uint wordlen = 0;
    uint cyrilen = 0;
    uint charmap = OS_UNKNOWN;

    while (pos < buflen)
    {
        val = buf[pos++];

        count_macroman += is_macroman(val);
        count_cp1252 += is_cp1252(val);

        if (val != 0x20)
        {
            wordlen++;
            cyrilen += is_cyrillic(val);
        }
        else
        {
            if (wordlen == cyrilen)
            {
                charmap = OS_CP1251;
                break;
            }
            wordlen = 0;
            cyrilen = 0;
        }
    }

    if (charmap == OS_CP1251)
        os_charmap = OS_CP1251;
    else if (count_cp1252 >= count_macroman)
        os_charmap = OS_CP1252;
    else
        os_charmap = OS_MACROMAN;

    return os_parse_chars(buf, buflen, out, outlen);
}

uint parse_utf8(const unsigned char *buf, uint buflen, uint32 *out, uint outlen)
{
    uint pos = 0;
    uint outpos = 0;
    uint res;
    uint val0, val1, val2, val3;

    while (outpos < outlen)
    {
        if (pos >= buflen)
            break;

        val0 = buf[pos++];

        if (val0 < 0x80)
        {
            res = val0;
            out[outpos++] = res;
            continue;
        }

        if ((val0 & 0xe0) == 0xc0)
        {
            if (pos+1 > buflen)
            {
                //printf("incomplete two-byte character\n");
                return identify_chars(buf, buflen, out, outlen);
            }
            val1 = buf[pos++];
            if ((val1 & 0xc0) != 0x80)
            {
                //printf("malformed two-byte character\n");
                return identify_chars(buf, buflen, out, outlen);
            }
            res = (val0 & 0x1f) << 6;
            res |= (val1 & 0x3f);
            out[outpos++] = res;
            continue;
        }

        if ((val0 & 0xf0) == 0xe0)
        {
            if (pos+2 > buflen)
            {
                //printf("incomplete three-byte character\n");
                return identify_chars(buf, buflen, out, outlen);
            }
            val1 = buf[pos++];
            val2 = buf[pos++];
            if ((val1 & 0xc0) != 0x80)
            {
                //printf("malformed three-byte character\n");
                return identify_chars(buf, buflen, out, outlen);
            }
            if ((val2 & 0xc0) != 0x80)
            {
                //printf("malformed three-byte character\n");
                return identify_chars(buf, buflen, out, outlen);
            }
            res = (((val0 & 0xf)<<12)  & 0x0000f000);
            res |= (((val1 & 0x3f)<<6) & 0x00000fc0);
            res |= (((val2 & 0x3f))    & 0x0000003f);
            out[outpos++] = res;
            continue;
        }

        if ((val0 & 0xf0) == 0xf0)
        {
            if ((val0 & 0xf8) != 0xf0)
            {
                //printf("malformed four-byte character\n");
                return identify_chars(buf, buflen, out, outlen);
            }
            if (pos+3 > buflen)
            {
                //printf("incomplete four-byte character\n");
                return identify_chars(buf, buflen, out, outlen);
            }
            val1 = buf[pos++];
            val2 = buf[pos++];
            val3 = buf[pos++];
            if ((val1 & 0xc0) != 0x80)
            {
                //printf("malformed four-byte character\n");
                return identify_chars(buf, buflen, out, outlen);
            }
            if ((val2 & 0xc0) != 0x80)
            {
                //printf("malformed four-byte character\n");
                return identify_chars(buf, buflen, out, outlen);
            }
            if ((val3 & 0xc0) != 0x80)
            {
                //printf("malformed four-byte character\n");
                return identify_chars(buf, buflen, out, outlen);
            }
            res = (((val0 & 0x7)<<18)   & 0x1c0000);
            res |= (((val1 & 0x3f)<<12) & 0x03f000);
            res |= (((val2 & 0x3f)<<6)  & 0x000fc0);
            res |= (((val3 & 0x3f))     & 0x00003f);
            out[outpos++] = res;
            continue;
        }

        //printf("malformed character\n");
        return identify_chars(buf, buflen, out, outlen);
    }

    return outpos;
}

uint prepare_utf8(const uint32 *buf, uint buflen, unsigned char *out, uint outlen)
{
    uint i=0, k=0;

    /*convert UTF-32 to UTF-8 */
    while (i < buflen && k < outlen)
    {
        if ((buf[i] < 0x80))
        {
            out[k] = buf[i];
            k++;
        }
        else if ((buf[i] < 0x800) && (k < outlen - 1))
        {
            out[k  ] = (0xC0 | ((buf[i] & 0x7C0) >> 6));
            out[k+1] = (0x80 |  (buf[i] & 0x03F)      );
            k = k + 2;
        }
        else if ((buf[i] < 0x10000) && (k < outlen - 2))
        {
            out[k  ] = (0xE0 | ((buf[i] & 0xF000) >> 12));
            out[k+1] = (0x80 | ((buf[i] & 0x0FC0) >>  6));
            out[k+2] = (0x80 |  (buf[i] & 0x003F)       );
            k = k + 3;
        }
        else if ((buf[i] < 0x200000) && (k < outlen - 3))
        {
            out[k  ] = (0xF0 | ((buf[i] & 0x1C0000) >> 18));
            out[k+1] = (0x80 | ((buf[i] & 0x03F000) >> 12));
            out[k+2] = (0x80 | ((buf[i] & 0x000FC0) >>  6));
            out[k+3] = (0x80 |  (buf[i] & 0x00003F)       );
            k = k + 4;
        }
        else
        {
            out[k] = '?';
            k++;
        }
        i++;
    }

    return k;
}

/* http://unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1251.TXT */

static const uint CP1251ToUnicode[128] = {
    0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, 0x20AC, 0x2030,
    0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F, 0x0452, 0x2018, 0x2019, 0x201C,
    0x201D, 0x2022, 0x2013, 0x2014, 0x003F, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C,
    0x045B, 0x045F, 0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7,
    0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407, 0x00B0, 0x00B1,
    0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7, 0x0451, 0x2116, 0x0454, 0x00BB,
    0x0458, 0x0405, 0x0455, 0x0457, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415,
    0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429,
    0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F, 0x0430, 0x0431, 0x0432, 0x0433,
    0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D,
    0x043E, 0x043F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F

};

uint parse_cp1251(const unsigned char *buf, uint buflen, uint32 *out, uint outlen) {
    uint pos = 0;
    uint outpos = 0;
    uint res;

    while (outpos < outlen)
    {
        if (pos >= buflen)
            break;

        res = buf[pos++];

        if (res < 0x80)
            out[outpos++] = res;
        else
            out[outpos++] = CP1251ToUnicode[res - 0x80];
    }

    return outpos;
}

static uint prepare_cp1251(const uint32 *buf, uint buflen,
                      unsigned char *out, uint outlen)
{
    uint pos = 0;
    uint outpos = 0;
    uint res;

    while (outpos < outlen)
    {
        if (pos >= buflen)
            break;

        res = buf[pos++];

        if (res < 0x80)
        {
            out[outpos++] = res;
            continue;
        }

        switch (res)
        {
            case 0x0402: out[outpos++] = 0x80; break;
            case 0x0403: out[outpos++] = 0x81; break;
            case 0x201A: out[outpos++] = 0x82; break;
            case 0x0453: out[outpos++] = 0x83; break;
            case 0x201E: out[outpos++] = 0x84; break;
            case 0x2026: out[outpos++] = 0x85; break;
            case 0x2020: out[outpos++] = 0x86; break;
            case 0x2021: out[outpos++] = 0x87; break;
            case 0x20AC: out[outpos++] = 0x88; break;
            case 0x2030: out[outpos++] = 0x89; break;
            case 0x0409: out[outpos++] = 0x8A; break;
            case 0x2039: out[outpos++] = 0x8B; break;
            case 0x040A: out[outpos++] = 0x8C; break;
            case 0x040C: out[outpos++] = 0x8D; break;
            case 0x040B: out[outpos++] = 0x8E; break;
            case 0x040F: out[outpos++] = 0x8F; break;

            case 0x0452: out[outpos++] = 0x90; break;
            case 0x2018: out[outpos++] = 0x91; break;
            case 0x2019: out[outpos++] = 0x92; break;
            case 0x201C: out[outpos++] = 0x93; break;
            case 0x201D: out[outpos++] = 0x94; break;
            case 0x2022: out[outpos++] = 0x95; break;
            case 0x2013: out[outpos++] = 0x96; break;
            case 0x2014: out[outpos++] = 0x97; break;
            case 0x2122: out[outpos++] = 0x99; break;
            case 0x0459: out[outpos++] = 0x9A; break;
            case 0x203A: out[outpos++] = 0x9B; break;
            case 0x045A: out[outpos++] = 0x9C; break;
            case 0x045C: out[outpos++] = 0x9D; break;
            case 0x045B: out[outpos++] = 0x9E; break;
            case 0x045F: out[outpos++] = 0x9F; break;

            case 0x00A0: out[outpos++] = 0xA0; break;
            case 0x040E: out[outpos++] = 0xA1; break;
            case 0x045E: out[outpos++] = 0xA2; break;
            case 0x0408: out[outpos++] = 0xA3; break;
            case 0x00A4: out[outpos++] = 0xA4; break;
            case 0x0490: out[outpos++] = 0xA5; break;
            case 0x00A6: out[outpos++] = 0xA6; break;
            case 0x00A7: out[outpos++] = 0xA7; break;
            case 0x0401: out[outpos++] = 0xA8; break;
            case 0x00A9: out[outpos++] = 0xA9; break;
            case 0x0404: out[outpos++] = 0xAA; break;
            case 0x00AB: out[outpos++] = 0xAB; break;
            case 0x00AC: out[outpos++] = 0xAC; break;
            case 0x00AD: out[outpos++] = 0xAD; break;
            case 0x00AE: out[outpos++] = 0xAE; break;
            case 0x0407: out[outpos++] = 0xAF; break;

            case 0x00B0: out[outpos++] = 0xB0; break;
            case 0x00B1: out[outpos++] = 0xB1; break;
            case 0x0406: out[outpos++] = 0xB2; break;
            case 0x0456: out[outpos++] = 0xB3; break;
            case 0x0491: out[outpos++] = 0xB4; break;
            case 0x00B5: out[outpos++] = 0xB5; break;
            case 0x00B6: out[outpos++] = 0xB6; break;
            case 0x00B7: out[outpos++] = 0xB7; break;
            case 0x0451: out[outpos++] = 0xB8; break;
            case 0x2116: out[outpos++] = 0xB9; break;
            case 0x0454: out[outpos++] = 0xBA; break;
            case 0x00BB: out[outpos++] = 0xBB; break;
            case 0x0458: out[outpos++] = 0xBC; break;
            case 0x0405: out[outpos++] = 0xBD; break;
            case 0x0455: out[outpos++] = 0xBE; break;
            case 0x0457: out[outpos++] = 0xBF; break;

            case 0x0410: out[outpos++] = 0xC0; break;
            case 0x0411: out[outpos++] = 0xC1; break;
            case 0x0412: out[outpos++] = 0xC2; break;
            case 0x0413: out[outpos++] = 0xC3; break;
            case 0x0414: out[outpos++] = 0xC4; break;
            case 0x0415: out[outpos++] = 0xC5; break;
            case 0x0416: out[outpos++] = 0xC6; break;
            case 0x0417: out[outpos++] = 0xC7; break;
            case 0x0418: out[outpos++] = 0xC8; break;
            case 0x0419: out[outpos++] = 0xC9; break;
            case 0x041A: out[outpos++] = 0xCA; break;
            case 0x041B: out[outpos++] = 0xCB; break;
            case 0x041C: out[outpos++] = 0xCC; break;
            case 0x041D: out[outpos++] = 0xCD; break;
            case 0x041E: out[outpos++] = 0xCE; break;
            case 0x041F: out[outpos++] = 0xCF; break;

            case 0x0420: out[outpos++] = 0xD0; break;
            case 0x0421: out[outpos++] = 0xD1; break;
            case 0x0422: out[outpos++] = 0xD2; break;
            case 0x0423: out[outpos++] = 0xD3; break;
            case 0x0424: out[outpos++] = 0xD4; break;
            case 0x0425: out[outpos++] = 0xD5; break;
            case 0x0426: out[outpos++] = 0xD6; break;
            case 0x0427: out[outpos++] = 0xD7; break;
            case 0x0428: out[outpos++] = 0xD8; break;
            case 0x0429: out[outpos++] = 0xD9; break;
            case 0x042A: out[outpos++] = 0xDA; break;
            case 0x042B: out[outpos++] = 0xDB; break;
            case 0x042C: out[outpos++] = 0xDC; break;
            case 0x042D: out[outpos++] = 0xDD; break;
            case 0x042E: out[outpos++] = 0xDE; break;
            case 0x042F: out[outpos++] = 0xDF; break;

            case 0x0430: out[outpos++] = 0xE0; break;
            case 0x0431: out[outpos++] = 0xE1; break;
            case 0x0432: out[outpos++] = 0xE2; break;
            case 0x0433: out[outpos++] = 0xE3; break;
            case 0x0434: out[outpos++] = 0xE4; break;
            case 0x0435: out[outpos++] = 0xE5; break;
            case 0x0436: out[outpos++] = 0xE6; break;
            case 0x0437: out[outpos++] = 0xE7; break;
            case 0x0438: out[outpos++] = 0xE8; break;
            case 0x0439: out[outpos++] = 0xE9; break;
            case 0x043A: out[outpos++] = 0xEA; break;
            case 0x043B: out[outpos++] = 0xEB; break;
            case 0x043C: out[outpos++] = 0xEC; break;
            case 0x043D: out[outpos++] = 0xED; break;
            case 0x043E: out[outpos++] = 0xEE; break;
            case 0x043F: out[outpos++] = 0xEF; break;

            case 0x0440: out[outpos++] = 0xF0; break;
            case 0x0441: out[outpos++] = 0xF1; break;
            case 0x0442: out[outpos++] = 0xF2; break;
            case 0x0443: out[outpos++] = 0xF3; break;
            case 0x0444: out[outpos++] = 0xF4; break;
            case 0x0445: out[outpos++] = 0xF5; break;
            case 0x0446: out[outpos++] = 0xF6; break;
            case 0x0447: out[outpos++] = 0xF7; break;
            case 0x0448: out[outpos++] = 0xF8; break;
            case 0x0449: out[outpos++] = 0xF9; break;
            case 0x044A: out[outpos++] = 0xFA; break;
            case 0x044B: out[outpos++] = 0xFB; break;
            case 0x044C: out[outpos++] = 0xFC; break;
            case 0x044D: out[outpos++] = 0xFD; break;
            case 0x044E: out[outpos++] = 0xFE; break;
            case 0x044F: out[outpos++] = 0xFF; break;

            default:
                /* undefined */
                out[outpos++] = '?';
                break;
        }

        continue;
    }

    return outpos;
}

/* http://unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1252.TXT */

static const uint CP1252ToUnicode[128] = {
    0x20AC, 0x003F, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030,
    0x0160, 0x2039, 0x0152, 0x003F, 0x017D, 0x003F, 0x003F, 0x2018, 0x2019, 0x201C,
    0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x003F,
    0x017E, 0x0178, 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF, 0x00B0, 0x00B1,
    0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB,
    0x00BC, 0x00BD, 0x00BE, 0x00BF, 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5,
    0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9,
    0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 0x00E0, 0x00E1, 0x00E2, 0x00E3,
    0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED,
    0x00EE, 0x00EF, 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

uint parse_cp1252(const unsigned char *buf, uint buflen, uint32 *out, uint outlen) {
    uint pos = 0;
    uint outpos = 0;
    uint res;

    while (outpos < outlen)
    {
        if (pos >= buflen)
            break;

        res = buf[pos++];

        if (res < 0x80)
            out[outpos++] = res;
        else
            out[outpos++] = CP1252ToUnicode[res - 0x80];
    }

    return outpos;
}

static uint prepare_cp1252(const uint32 *buf, uint buflen,
                      unsigned char *out, uint outlen)
{
    uint pos = 0;
    uint outpos = 0;
    uint res;

    while (outpos < outlen)
    {
        if (pos >= buflen)
            break;

        res = buf[pos++];

        if (res < 0x80)
        {
            out[outpos++] = res;
            continue;
        }

        switch (res)
        {
            case 0x20AC: out[outpos++] = 0x80; break;
            case 0x201A: out[outpos++] = 0x82; break;
            case 0x0192: out[outpos++] = 0x83; break;
            case 0x201E: out[outpos++] = 0x84; break;
            case 0x2026: out[outpos++] = 0x85; break;
            case 0x2020: out[outpos++] = 0x86; break;
            case 0x2021: out[outpos++] = 0x87; break;
            case 0x02C6: out[outpos++] = 0x88; break;
            case 0x2030: out[outpos++] = 0x89; break;
            case 0x0160: out[outpos++] = 0x8A; break;
            case 0x2039: out[outpos++] = 0x8B; break;
            case 0x0152: out[outpos++] = 0x8C; break;
            case 0x017D: out[outpos++] = 0x8E; break;

            case 0x2018: out[outpos++] = 0x91; break;
            case 0x2019: out[outpos++] = 0x92; break;
            case 0x201C: out[outpos++] = 0x93; break;
            case 0x201D: out[outpos++] = 0x94; break;
            case 0x2022: out[outpos++] = 0x95; break;
            case 0x2013: out[outpos++] = 0x96; break;
            case 0x2014: out[outpos++] = 0x97; break;
            case 0x02DC: out[outpos++] = 0x98; break;
            case 0x2122: out[outpos++] = 0x99; break;
            case 0x0161: out[outpos++] = 0x9A; break;
            case 0x203A: out[outpos++] = 0x9B; break;
            case 0x0153: out[outpos++] = 0x9C; break;
            case 0x017E: out[outpos++] = 0x9E; break;
            case 0x0178: out[outpos++] = 0x9F; break;

            case 0x00A0: out[outpos++] = 0xA0; break;
            case 0x00A1: out[outpos++] = 0xA1; break;
            case 0x00A2: out[outpos++] = 0xA2; break;
            case 0x00A3: out[outpos++] = 0xA3; break;
            case 0x00A4: out[outpos++] = 0xA4; break;
            case 0x00A5: out[outpos++] = 0xA5; break;
            case 0x00A6: out[outpos++] = 0xA6; break;
            case 0x00A7: out[outpos++] = 0xA7; break;
            case 0x00A8: out[outpos++] = 0xA8; break;
            case 0x00A9: out[outpos++] = 0xA9; break;
            case 0x00AA: out[outpos++] = 0xAA; break;
            case 0x00AB: out[outpos++] = 0xAB; break;
            case 0x00AC: out[outpos++] = 0xAC; break;
            case 0x00AD: out[outpos++] = 0xAD; break;
            case 0x00AE: out[outpos++] = 0xAE; break;
            case 0x00AF: out[outpos++] = 0xAF; break;

            case 0x00B0: out[outpos++] = 0xB0; break;
            case 0x00B1: out[outpos++] = 0xB1; break;
            case 0x00B2: out[outpos++] = 0xB2; break;
            case 0x00B3: out[outpos++] = 0xB3; break;
            case 0x00B4: out[outpos++] = 0xB4; break;
            case 0x00B5: out[outpos++] = 0xB5; break;
            case 0x00B6: out[outpos++] = 0xB6; break;
            case 0x00B7: out[outpos++] = 0xB7; break;
            case 0x00B8: out[outpos++] = 0xB8; break;
            case 0x00B9: out[outpos++] = 0xB9; break;
            case 0x00BA: out[outpos++] = 0xBA; break;
            case 0x00BB: out[outpos++] = 0xBB; break;
            case 0x00BC: out[outpos++] = 0xBC; break;
            case 0x00BD: out[outpos++] = 0xBD; break;
            case 0x00BE: out[outpos++] = 0xBE; break;
            case 0x00BF: out[outpos++] = 0xBF; break;

            case 0x00C0: out[outpos++] = 0xC0; break;
            case 0x00C1: out[outpos++] = 0xC1; break;
            case 0x00C2: out[outpos++] = 0xC2; break;
            case 0x00C3: out[outpos++] = 0xC3; break;
            case 0x00C4: out[outpos++] = 0xC4; break;
            case 0x00C5: out[outpos++] = 0xC5; break;
            case 0x00C6: out[outpos++] = 0xC6; break;
            case 0x00C7: out[outpos++] = 0xC7; break;
            case 0x00C8: out[outpos++] = 0xC8; break;
            case 0x00C9: out[outpos++] = 0xC9; break;
            case 0x00CA: out[outpos++] = 0xCA; break;
            case 0x00CB: out[outpos++] = 0xCB; break;
            case 0x00CC: out[outpos++] = 0xCC; break;
            case 0x00CD: out[outpos++] = 0xCD; break;
            case 0x00CE: out[outpos++] = 0xCE; break;
            case 0x00CF: out[outpos++] = 0xCF; break;

            case 0x00D0: out[outpos++] = 0xD0; break;
            case 0x00D1: out[outpos++] = 0xD1; break;
            case 0x00D2: out[outpos++] = 0xD2; break;
            case 0x00D3: out[outpos++] = 0xD3; break;
            case 0x00D4: out[outpos++] = 0xD4; break;
            case 0x00D5: out[outpos++] = 0xD5; break;
            case 0x00D6: out[outpos++] = 0xD6; break;
            case 0x00D7: out[outpos++] = 0xD7; break;
            case 0x00D8: out[outpos++] = 0xD8; break;
            case 0x00D9: out[outpos++] = 0xD9; break;
            case 0x00DA: out[outpos++] = 0xDA; break;
            case 0x00DB: out[outpos++] = 0xDB; break;
            case 0x00DC: out[outpos++] = 0xDC; break;
            case 0x00DD: out[outpos++] = 0xDD; break;
            case 0x00DE: out[outpos++] = 0xDE; break;
            case 0x00DF: out[outpos++] = 0xDF; break;

            case 0x00E0: out[outpos++] = 0xE0; break;
            case 0x00E1: out[outpos++] = 0xE1; break;
            case 0x00E2: out[outpos++] = 0xE2; break;
            case 0x00E3: out[outpos++] = 0xE3; break;
            case 0x00E4: out[outpos++] = 0xE4; break;
            case 0x00E5: out[outpos++] = 0xE5; break;
            case 0x00E6: out[outpos++] = 0xE6; break;
            case 0x00E7: out[outpos++] = 0xE7; break;
            case 0x00E8: out[outpos++] = 0xE8; break;
            case 0x00E9: out[outpos++] = 0xE9; break;
            case 0x00EA: out[outpos++] = 0xEA; break;
            case 0x00EB: out[outpos++] = 0xEB; break;
            case 0x00EC: out[outpos++] = 0xEC; break;
            case 0x00ED: out[outpos++] = 0xED; break;
            case 0x00EE: out[outpos++] = 0xEE; break;
            case 0x00EF: out[outpos++] = 0xEF; break;

            case 0x00F0: out[outpos++] = 0xF0; break;
            case 0x00F1: out[outpos++] = 0xF1; break;
            case 0x00F2: out[outpos++] = 0xF2; break;
            case 0x00F3: out[outpos++] = 0xF3; break;
            case 0x00F4: out[outpos++] = 0xF4; break;
            case 0x00F5: out[outpos++] = 0xF5; break;
            case 0x00F6: out[outpos++] = 0xF6; break;
            case 0x00F7: out[outpos++] = 0xF7; break;
            case 0x00F8: out[outpos++] = 0xF8; break;
            case 0x00F9: out[outpos++] = 0xF9; break;
            case 0x00FA: out[outpos++] = 0xFA; break;
            case 0x00FB: out[outpos++] = 0xFB; break;
            case 0x00FC: out[outpos++] = 0xFC; break;
            case 0x00FD: out[outpos++] = 0xFD; break;
            case 0x00FE: out[outpos++] = 0xFE; break;
            case 0x00FF: out[outpos++] = 0xFF; break;

            default:
                /* undefined */
                out[outpos++] = '?';
                break;
        }

        continue;
    }

    return outpos;
}

/* http://unicode.org/Public/MAPPINGS/VENDORS/APPLE/ROMAN.TXT */

static const uint MacRomanToUnicode[128] = {
    0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1, 0x00E0, 0x00E2,
    0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8, 0x00EA, 0x00EB, 0x00ED, 0x00EC,
    0x00EE, 0x00EF, 0x00F1, 0x00F3, 0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9,
    0x00FB, 0x00FC, 0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF,
    0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8, 0x221E, 0x00B1,
    0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211, 0x220F, 0x03C0, 0x222B, 0x00AA,
    0x00BA, 0x03A9, 0x00E6, 0x00F8, 0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248,
    0x2206, 0x00AB, 0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA, 0x00FF, 0x0178,
    0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02, 0x2021, 0x00B7, 0x201A, 0x201E,
    0x2030, 0x00C2, 0x00CA, 0x00C1, 0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC,
    0x00D3, 0x00D4, 0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC,
    0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7
};

uint parse_mac(const unsigned char *buf, uint buflen, uint32 *out, uint outlen) {
    uint pos = 0;
    uint outpos = 0;
    uint res;

    while (outpos < outlen)
    {
        if (pos >= buflen)
            break;

        res = buf[pos++];

        if (res < 0x80)
            out[outpos++] = res;
        else
            out[outpos++] = MacRomanToUnicode[res - 0x80];
    }

    return outpos;
}

static uint prepare_mac(const uint32 *buf, uint buflen, unsigned char *out, uint outlen) {
    uint pos = 0;
    uint outpos = 0;
    uint res;

    while (outpos < outlen)
    {
        if (pos >= buflen)
            break;

        res = buf[pos++];

        if (res < 0x80)
        {
            out[outpos++] = res;
            continue;
        }

        switch (res)
        {
            case 0x00C4: out[outpos++] = 0x80; break;
            case 0x00C5: out[outpos++] = 0x81; break;
            case 0x00C7: out[outpos++] = 0x82; break;
            case 0x00C9: out[outpos++] = 0x83; break;
            case 0x00D1: out[outpos++] = 0x84; break;
            case 0x00D6: out[outpos++] = 0x85; break;
            case 0x00DC: out[outpos++] = 0x86; break;
            case 0x00E1: out[outpos++] = 0x87; break;
            case 0x00E0: out[outpos++] = 0x88; break;
            case 0x00E2: out[outpos++] = 0x89; break;
            case 0x00E4: out[outpos++] = 0x8A; break;
            case 0x00E3: out[outpos++] = 0x8B; break;
            case 0x00E5: out[outpos++] = 0x8C; break;
            case 0x00E7: out[outpos++] = 0x8D; break;
            case 0x00E9: out[outpos++] = 0x8E; break;
            case 0x00E8: out[outpos++] = 0x8F; break;

            case 0x00EA: out[outpos++] = 0x90; break;
            case 0x00EB: out[outpos++] = 0x91; break;
            case 0x00ED: out[outpos++] = 0x92; break;
            case 0x00EC: out[outpos++] = 0x93; break;
            case 0x00EE: out[outpos++] = 0x94; break;
            case 0x00EF: out[outpos++] = 0x95; break;
            case 0x00F1: out[outpos++] = 0x96; break;
            case 0x00F3: out[outpos++] = 0x97; break;
            case 0x00F2: out[outpos++] = 0x98; break;
            case 0x00F4: out[outpos++] = 0x99; break;
            case 0x00F6: out[outpos++] = 0x9A; break;
            case 0x00F5: out[outpos++] = 0x9B; break;
            case 0x00FA: out[outpos++] = 0x9C; break;
            case 0x00F9: out[outpos++] = 0x9D; break;
            case 0x00FB: out[outpos++] = 0x9E; break;
            case 0x00FC: out[outpos++] = 0x9F; break;

            case 0x2020: out[outpos++] = 0xA0; break;
            case 0x00B0: out[outpos++] = 0xA1; break;
            case 0x00A2: out[outpos++] = 0xA2; break;
            case 0x00A3: out[outpos++] = 0xA3; break;
            case 0x00A7: out[outpos++] = 0xA4; break;
            case 0x2022: out[outpos++] = 0xA5; break;
            case 0x00B6: out[outpos++] = 0xA6; break;
            case 0x00DF: out[outpos++] = 0xA7; break;
            case 0x00AE: out[outpos++] = 0xA8; break;
            case 0x00A9: out[outpos++] = 0xA9; break;
            case 0x2122: out[outpos++] = 0xAA; break;
            case 0x00B4: out[outpos++] = 0xAB; break;
            case 0x00A8: out[outpos++] = 0xAC; break;
            case 0x2260: out[outpos++] = 0xAD; break;
            case 0x00C6: out[outpos++] = 0xAE; break;
            case 0x00D8: out[outpos++] = 0xAF; break;

            case 0x221E: out[outpos++] = 0xB0; break;
            case 0x00B1: out[outpos++] = 0xB1; break;
            case 0x2264: out[outpos++] = 0xB2; break;
            case 0x2265: out[outpos++] = 0xB3; break;
            case 0x00A5: out[outpos++] = 0xB4; break;
            case 0x00B5: out[outpos++] = 0xB5; break;
            case 0x2202: out[outpos++] = 0xB6; break;
            case 0x2211: out[outpos++] = 0xB7; break;
            case 0x220F: out[outpos++] = 0xB8; break;
            case 0x03C0: out[outpos++] = 0xB9; break;
            case 0x222B: out[outpos++] = 0xBA; break;
            case 0x00AA: out[outpos++] = 0xBB; break;
            case 0x00BA: out[outpos++] = 0xBC; break;
            case 0x03A9: out[outpos++] = 0xBD; break;
            case 0x00E6: out[outpos++] = 0xBE; break;
            case 0x00F8: out[outpos++] = 0xBF; break;

            case 0x00BF: out[outpos++] = 0xC0; break;
            case 0x00A1: out[outpos++] = 0xC1; break;
            case 0x00AC: out[outpos++] = 0xC2; break;
            case 0x221A: out[outpos++] = 0xC3; break;
            case 0x0192: out[outpos++] = 0xC4; break;
            case 0x2248: out[outpos++] = 0xC5; break;
            case 0x2206: out[outpos++] = 0xC6; break;
            case 0x00AB: out[outpos++] = 0xC7; break;
            case 0x00BB: out[outpos++] = 0xC8; break;
            case 0x2026: out[outpos++] = 0xC9; break;
            case 0x00A0: out[outpos++] = 0xCA; break;
            case 0x00C0: out[outpos++] = 0xCB; break;
            case 0x00C3: out[outpos++] = 0xCC; break;
            case 0x00D5: out[outpos++] = 0xCD; break;
            case 0x0152: out[outpos++] = 0xCE; break;
            case 0x0153: out[outpos++] = 0xCF; break;

            case 0x2013: out[outpos++] = 0xD0; break;
            case 0x2014: out[outpos++] = 0xD1; break;
            case 0x201C: out[outpos++] = 0xD2; break;
            case 0x201D: out[outpos++] = 0xD3; break;
            case 0x2018: out[outpos++] = 0xD4; break;
            case 0x2019: out[outpos++] = 0xD5; break;
            case 0x00F7: out[outpos++] = 0xD6; break;
            case 0x25CA: out[outpos++] = 0xD7; break;
            case 0x00FF: out[outpos++] = 0xD8; break;
            case 0x0178: out[outpos++] = 0xD9; break;
            case 0x2044: out[outpos++] = 0xDA; break;
            case 0x20AC: out[outpos++] = 0xDB; break;
            case 0x2039: out[outpos++] = 0xDC; break;
            case 0x203A: out[outpos++] = 0xDD; break;
            case 0xFB01: out[outpos++] = 0xDE; break;
            case 0xFB02: out[outpos++] = 0xDF; break;

            case 0x2021: out[outpos++] = 0xE0; break;
            case 0x00B7: out[outpos++] = 0xE1; break;
            case 0x201A: out[outpos++] = 0xE2; break;
            case 0x201E: out[outpos++] = 0xE3; break;
            case 0x2030: out[outpos++] = 0xE4; break;
            case 0x00C2: out[outpos++] = 0xE5; break;
            case 0x00CA: out[outpos++] = 0xE6; break;
            case 0x00C1: out[outpos++] = 0xE7; break;
            case 0x00CB: out[outpos++] = 0xE8; break;
            case 0x00C8: out[outpos++] = 0xE9; break;
            case 0x00CD: out[outpos++] = 0xEA; break;
            case 0x00CE: out[outpos++] = 0xEB; break;
            case 0x00CF: out[outpos++] = 0xEC; break;
            case 0x00CC: out[outpos++] = 0xED; break;
            case 0x00D3: out[outpos++] = 0xEE; break;
            case 0x00D4: out[outpos++] = 0xEF; break;

            case 0xF8FF: out[outpos++] = 0xF0; break;
            case 0x00D2: out[outpos++] = 0xF1; break;
            case 0x00DA: out[outpos++] = 0xF2; break;
            case 0x00DB: out[outpos++] = 0xF3; break;
            case 0x00D9: out[outpos++] = 0xF4; break;
            case 0x0131: out[outpos++] = 0xF5; break;
            case 0x02C6: out[outpos++] = 0xF6; break;
            case 0x02DC: out[outpos++] = 0xF7; break;
            case 0x00AF: out[outpos++] = 0xF8; break;
            case 0x02D8: out[outpos++] = 0xF9; break;
            case 0x02D9: out[outpos++] = 0xFA; break;
            case 0x02DA: out[outpos++] = 0xFB; break;
            case 0x00B8: out[outpos++] = 0xFC; break;
            case 0x02DD: out[outpos++] = 0xFD; break;
            case 0x02DB: out[outpos++] = 0xFE; break;
            case 0x02C7: out[outpos++] = 0xFF; break;

            default:
                /* undefined */
                out[outpos++] = '?';
                break;
        }

        continue;
    }

    return outpos;
}

uint os_parse_chars(const unsigned char *buf, uint buflen, uint32 *out, uint outlen) {
    switch (os_charmap)
    {
        case OS_UTF8:
            return parse_utf8(buf, buflen, out, outlen);

        case OS_CP1251:
            return parse_cp1251(buf, buflen, out, outlen);

        case OS_CP1252:
            return parse_cp1252(buf, buflen, out, outlen);

        case OS_MACROMAN:
            return parse_mac(buf, buflen, out, outlen);

        default:
            return 0;
    }
}

uint os_prepare_chars(const uint32 *buf, uint buflen, unsigned char *out, uint outlen) {
    switch (os_charmap) {
        case OS_UTF8:
            return prepare_utf8(buf, buflen, out, outlen);

        case OS_CP1251:
            return prepare_cp1251(buf, buflen, out, outlen);

        case OS_CP1252:
            return prepare_cp1252(buf, buflen, out, outlen);

        case OS_MACROMAN:
            return prepare_mac(buf, buflen, out, outlen);

        default:
            return 0;
    }
}

#endif /* GLK_MODULE_UNICODE */

} // End of namespace TADS
} // End of namespace Glk
