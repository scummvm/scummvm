
/***************************************************************************
 versions.h Copyright (C) 1999,2000,01 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CJR) [jameson@linuxgames.com]


***************************************************************************/
/* Versions management */

#ifndef _SCI_VERSIONS_H_
#define _SCI_VERSIONS_H_

#include <scitypes.h>

struct _state;

#define SCI_VERSION(_major_, _minor_, _patchlevel_) (((_major_)<<20) | ((_minor_)<<10) | _patchlevel_)
/* This allows version numbers to be compared directly */

#define SCI_VERSION_MAJOR(_version_) ((_version_) >> 20)
#define SCI_VERSION_MINOR(_version_) (((_version_) >> 10) & 0x3ff)
#define SCI_VERSION_PATCHLEVEL(_version_) ((_version_) & 0x3ff)
#define SCI_VERSION_IGNORE_PATCHLEVEL(_version_) ((_version) & ~0x3ff)

/* Version number guide:
** - Always use the version number of the first known version to have a special feature.
** - Don't assume that special feature changes are linked just because they appeared to change
**   simultaneously.
** - Put all the magic version numbers here, into THIS file.
** - "FTU" means "First To Use"
*/

#define SCI_VERSION_LAST_SCI0 SCI_VERSION(0,000,685)

#define SCI_VERSION_DEFAULT_SCI0 SCI_VERSION_LAST_SCI0
/* AFAIK this is the last published SCI0 version */
#define SCI_VERSION_DEFAULT_SCI01 SCI_VERSION(1,000,72)
/* The version used by my implementation of QfG2 */


#define SCI_VERSION_FTU_CENTERED_TEXT_AS_DEFAULT SCI_VERSION(0,000,629)
/* Last version known not to do this: 0.000.502 */

#define SCI_VERSION_FTU_NEW_GETTIME SCI_VERSION(0,000,629)
/* These versions of SCI has a different set of subfunctions in GetTime() */

#define SCI_VERSION_FTU_NEWER_DRAWPIC_PARAMETERS SCI_VERSION(0,000,502)
/* Last version known not to do this: 0.000.435
** Old SCI versions used to interpret the third DrawPic() parameter inversely,
** with the opposite default value (obviously)
*/

#define SCI_VERSION_FTU_PRIORITY_14_ZONES SCI_VERSION(0,000,502)
/* Last version known to do this: 0.000.490
 * Uses 14 zones from 42 to 190 instead of 15 zones from 42 to 200.
*/


#define SCI_VERSION_FTU_NEW_SCRIPT_HEADER SCI_VERSION(0,000,395)
/* Last version known not to do this: 0.000.343
** Old SCI versions used two word header for script blocks (first word equal
** to 0x82, meaning of the second one unknown). New SCI versions used one
** word header.
*/

#define SCI_VERSION_LTU_BASE_OB1 SCI_VERSION(0,000,256)
/* First version version known not to have this bug: ?
** When doing CanBeHere(), augment y offset by 1
*/

#define SCI_VERSION_FTU_2ND_ANGLES SCI_VERSION(0,000,395)
/* Last version known not to use this: ?
** Earlier versions assign 120 degrees to left & right , and 60 to up and down.
** Later versions use an even 90 degree distribution.
*/

#define SCI_VERSION_RESUME_SUSPENDED_SONG SCI_VERSION(0,000,490)
/* First version (PQ2-new) known to use the different song resumption 
   mechanism -- When a new song is initialized, we store its state and
   resume it when the new one finishes.  Older versions completely
   clobbered the old songs.
*/

#define SCI_VERSION_FTU_INVERSE_CANBEHERE SCI_VERSION(1,000,510)
/* FIXME: This shouldn't be a version number.
 * But it'll do for now.
 */

#define SCI_VERSION_FTU_LOFS_ABSOLUTE SCI_VERSION(1,000,200)
/* First version known to do this: ?
   In later versions (SCI1 and beyond), the argument of lofs[as]
   instructions is absolute rather than relative.
*/

#define SCI_VERSION_FTU_DISPLAY_COORDS_FUZZY SCI_VERSION(1,000,510)
/* First version known to do this: ?
   In later versions of SCI1 kDisplay(), if the text would not fit on
   the screen, the text is moved to the left and upwards until it
   fits.
*/

#define SCI_VERSION_FTU_DOSOUND_VARIANT_1 SCI_VERSION(1,000,000)
#define SCI_VERSION_FTU_DOSOUND_VARIANT_2 SCI_VERSION(1,000,510)


typedef int sci_version_t;

struct _state;

void
version_require_earlier_than(struct _state *s, sci_version_t version);
/* Function used in autodetection
** Parameters: (state_t *) s: state_t containing the version
**             (sci_version_t) version: The version that we're earlier than
*/

void
version_require_later_than(struct _state *s, sci_version_t version);
/* Function used in autodetection (read this function "version_require_later_than_or_equal_to")
** Parameters: (state_t *) s: state_t containing the version
**             (sci_version_t) version: The version that we're later than
*/

int
version_parse(char *vn, sci_version_t *result);
/* Parse a string containing an SCI version number
** Parameters: (char *) vn: The string to parse
** Returns   : (int) 0 on success, 1 on failure
**             (sci_version_t) *result: The resulting version number on success
*/

int
version_detect_from_executable(sci_version_t *result);
/* Try to detect version from Sierra executable in cwd
** Returns   : (int) 0 on success, 1 on failure
**             (sci_version_t) *result: The version number detected on success
*/

const char *
version_guess_from_hashcode(sci_version_t *result, int *res_version, guint32 *code);
/* Try to detect version from Sierra resource file(s) in cwd
** Returns   : (const char *) NULL on failure, the name of the associated game otherwise
**             (sci_version_t) *result: The version number detected on success
**             (int) *res_version: The resource version number detected on success
**             (guint32) *code: The resource hash  code
*/

#endif /* !_SCI_VERSIONS_H_ */
