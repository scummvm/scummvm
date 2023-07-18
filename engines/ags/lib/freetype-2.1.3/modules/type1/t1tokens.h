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

/***************************************************************************/
/*                                                                         */
/*  t1tokens.h                                                             */
/*    Type 1 tokenizer (specification).                                    */
/*                                                                         */
/***************************************************************************/

#undef FT2_1_3_STRUCTURE
#define FT2_1_3_STRUCTURE PS_FontInfoRec

#undef T1CODE
#define T1CODE T1_FIELD_LOCATION_FONT_INFO

T1_FIELD_STRING("version", version)
T1_FIELD_STRING("Notice", notice)
T1_FIELD_STRING("FullName", full_name)
T1_FIELD_STRING("FamilyName", family_name)
T1_FIELD_STRING("Weight", weight)

T1_FIELD_NUM("ItalicAngle", italic_angle)
T1_FIELD_TYPE_BOOL("isFixedPitch", is_fixed_pitch)
T1_FIELD_NUM("UnderlinePosition", underline_position)
T1_FIELD_NUM("UnderlineThickness", underline_thickness)

#undef FT2_1_3_STRUCTURE
#define FT2_1_3_STRUCTURE PS_PrivateRec

#undef T1CODE
#define T1CODE T1_FIELD_LOCATION_PRIVATE

T1_FIELD_NUM("UniqueID", unique_id)
T1_FIELD_NUM("lenIV", lenIV)
T1_FIELD_NUM("LanguageGroup", language_group)
T1_FIELD_NUM("password", password)

T1_FIELD_FIXED("BlueScale", blue_scale)
T1_FIELD_NUM("BlueShift", blue_shift)
T1_FIELD_NUM("BlueFuzz", blue_fuzz)

T1_FIELD_NUM_TABLE("BlueValues", blue_values, 14)
T1_FIELD_NUM_TABLE("OtherBlues", other_blues, 10)
T1_FIELD_NUM_TABLE("FamilyBlues", family_blues, 14)
T1_FIELD_NUM_TABLE("FamilyOtherBlues", family_other_blues, 10)

T1_FIELD_NUM_TABLE2("StdHW", standard_width, 1)
T1_FIELD_NUM_TABLE2("StdVW", standard_height, 1)
T1_FIELD_NUM_TABLE2("MinFeature", min_feature, 2)

T1_FIELD_NUM_TABLE("StemSnapH", snap_widths, 12)
T1_FIELD_NUM_TABLE("StemSnapV", snap_heights, 12)

#undef FT2_1_3_STRUCTURE
#define FT2_1_3_STRUCTURE T1_FontRec

#undef T1CODE
#define T1CODE T1_FIELD_LOCATION_FONT_DICT

T1_FIELD_NUM("PaintType", paint_type)
T1_FIELD_NUM("FontType", font_type)
T1_FIELD_NUM("StrokeWidth", stroke_width)

#undef FT2_1_3_STRUCTURE
#define FT2_1_3_STRUCTURE FT_BBox

#undef T1CODE
#define T1CODE T1_FIELD_LOCATION_BBOX

T1_FIELD_BBOX("FontBBox", xMin)
