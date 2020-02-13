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

#ifndef GLK_TADS_TADS2_STRING_RESOURCES
#define GLK_TADS_TADS2_STRING_RESOURCES

namespace Glk {
namespace TADS {
namespace TADS2 {

/*
 *   Dialog buttons.  These provide the text for standard buttons in
 *   dialogs created with os_input_dialog().
 *   
 *   These labels can use "&" to indicate a shortcut letter, per the
 *   normal os_input_dialog() interface; for example, if the Yes button
 *   label is "&Yes", the button has the shortcut letter "Y".
 *   
 *   The text of these buttons may vary by system, since these should
 *   conform to local conventions where there are local conventions.  In
 *   addition, of course, these strings will vary by language.  
 */

/* OK and Cancel buttons */
#define RESID_BTN_OK       1
#define RESID_BTN_CANCEL   2

/* "Yes" and "No" buttons */
#define RESID_BTN_YES      3
#define RESID_BTN_NO       4

/*
 *   Reply strings for the yorn() built-in function.  These strings are
 *   regular expressions as matched by the regex.h functions.  For
 *   English, for example, the "yes" string would be "[Yy].*" and the "no"
 *   string would be "[Nn].*".  For German, it might be desirable to
 *   accept both "Ja" and "Yes", so the "Yes" string might be "[JjYy].*".
 *   
 *   It's not necessary in these patterns to consider leading spaces,
 *   since the yorn() function will skip any leading spaces before
 *   performing the pattern match.  
 */
#define RESID_YORN_YES     5
#define RESID_YORN_NO      6

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
