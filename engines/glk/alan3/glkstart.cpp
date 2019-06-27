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

#include "glk/alan3/glkstart.h"
#include "glk/alan3/alan_version.h"
#include "glk/alan3/alan3.h"
#include "glk/alan3/args.h"
#include "glk/alan3/main.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/resources.h"
#include "glk/alan3/utils.h"
#include "glk/streams.h"

namespace Glk {
namespace Alan3 {

const glkunix_argumentlist_t glkunix_arguments[] = {
    { "-l", glkunix_arg_NoValue, "-l: log player command and game output" },
    { "-c", glkunix_arg_NoValue, "-c: log player commands to a file" },
    { "-n", glkunix_arg_NoValue, "-n: no status line" },
    { "-i", glkunix_arg_NoValue, "-i: ignore version and checksum errors" },
    { "-d", glkunix_arg_NoValue, "-d: enter debug mode" },
    { "-t", glkunix_arg_NoValue, "-t [<n>]: trace game execution, higher <n> gives more trace" },
    { "-r", glkunix_arg_NoValue, "-r: refrain from printing timestamps and paging (making regression testing easier)" },
    { "", glkunix_arg_ValueFollows, "filename: The game file to load." },
    { NULL, glkunix_arg_End, NULL }
};

/* Resources */
static strid_t resourceFile;

/*----------------------------------------------------------------------*/
static void openGlkWindows() {
    glkMainWin = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
    if (glkMainWin == NULL) {
            printf("FATAL ERROR: Cannot open initial window");
			g_vm->glk_exit();
    }

    glkStatusWin = g_vm->glk_window_open(glkMainWin, winmethod_Above |
                                   winmethod_Fixed, 1, wintype_TextGrid, 0);
	g_vm->glk_set_window(glkStatusWin);
	g_vm->glk_set_style(style_Preformatted);
	g_vm->glk_set_window(glkMainWin);
}

/*----------------------------------------------------------------------*/
static void openResourceFile() {
#ifdef TODO
    char *originalFileName = strdup(adventureFileName);
	char *resourceFileName = originalFileName;
    char *extension = strrchr(resourceFileName, '.');
    frefid_t resourceFileRef;
//    giblorb_err_t ecode;

    strcpy(extension, ".a3r");

    resourceFileRef = winglk_fileref_create_by_name(fileusage_BinaryMode,
                                                    resourceFileName, 0, FALSE);

	if (glk_fileref_does_file_exist(resourceFileRef)) {
        resourceFile = glk_stream_open_file(resourceFileRef, filemode_Read, 0);
        ecode = giblorb_set_resource_map(resourceFile);
        (void)ecode;
    }
    free(originalFileName);
#endif
}


/*======================================================================*/
int glkunix_startup_code(glkunix_startup_t *data) {
    g_vm->glk_stylehint_set(wintype_AllTypes, style_Emphasized, stylehint_Weight, 0);
	g_vm->glk_stylehint_set(wintype_AllTypes, style_Emphasized, stylehint_Oblique, 1);
	g_vm->glk_stylehint_set(wintype_AllTypes, style_BlockQuote, stylehint_Indentation, 10);

    /* first, open a window for error output */
    openGlkWindows();

    /* now process the command line arguments */
    //args(data->argc, data->argv);

    if (adventureFileName == NULL || strcmp(adventureFileName, "") == 0) {
        printf("You should supply a game file to play.\n");
        usage("arun"); // TODO Find real programname from arguments
        terminate(1);
    }

    /* Open any possible blorb resource file */
    openResourceFile();

    return TRUE;
}

} // End of namespace Alan3
} // End of namespace Glk
