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

#ifndef GLK_TADS_TADS2_TEXT_IO
#define GLK_TADS_TADS2_TEXT_IO

/*
 * Text I/O interface
 *
 * Formatted text input and output interface definition
 */

#include "glk/tads/tads.h"
#include "glk/tads/tads2/error_handling.h"
#include "glk/tads/tads2/run.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* forward decls */
struct runcxdef;

/**
 * Text i/o context
 */
struct tiocxdef {
    errcxdef *tiocxerr;                           /* error handling context */
};

/**
 *   Initialize the output formatter subsystem.  This must be called once
 *   at startup. 
 */
void out_init();


/* redirect all tioxxx routines to TADS v1.x outxxx equivalents */
#define tioflushn(ctx, nl) outflushn(nl)
#define tioflush(ctx)      outflush()
#define tioblank(ctx)      outblank()
#define tioreset(ctx)      outreset()
#define tiogets(ctx, prompt, str, siz) getstring(prompt, str, siz)
#define tioputs(ctx, str)  outformat(str)
#define tioputslen(ctx, str, len) outformatlen(str, len)
#define tiocaps(ctx)       outcaps()
#define tionocaps(ctx)     outnocaps()
#define tioshow(ctx)       outshow()
#define tiohide(ctx)       outhide()
#define tioscore(ctx, s1, s2) os_score(s1, s2)
#define tiostrsc(ctx, s)   os_strsc(s)

/* set up format strings in output subsystem */
void tiosetfmt(tiocxdef *ctx, runcxdef *rctx, uchar *fmtbase,
               uint fmtlen);

/* tell tio subsystem the current actor */
void tiosetactor(tiocxdef *ctx, objnum actor);

/* get the current tio subsystem actor */
objnum tiogetactor(tiocxdef *ctx);

/* turn output capture on/off */
void tiocapture(tiocxdef *tioctx, mcmcxdef *memctx, int flag);

/* get the capture object handle */
mcmon tiogetcapture(tiocxdef *ctx);

/* get the amount of text captured */
uint tiocapturesize(tiocxdef *ctx);

/* format a length-prefixed (runtime-style) string to the display */
void outfmt(tiocxdef *ctx, uchar *txt);

/* format a null-terminated (C-style) string to the display */
int outformat(const char *s);

/* format a counted-length string, which may not be null-terminated */
int outformatlen(const char *s, uint len);

/* flush output, with specified newline mode */
void outflushn(int nl);

/* flush output */
void outflush(void);

/* reset output state */
void outreset(void);

/* 
 *   Get a string from the keyboard.  Returns non-zero if an error occurs
 *   (in particular, if no more input is available from the keyboard),
 *   zero on success.  
 */
int getstring(const char *prompt, char *buf, int bufl);

/* set capitalize-next-character mode on/off */
void outcaps(void);
void outnocaps(void);

/* open/close output log file */
int tiologopn(tiocxdef *ctx, char *fn);
int tiologcls(tiocxdef *ctx);

/*
 *   Write text explicitly to the log file.  This can be used to add
 *   special text (such as prompt text) that would normally be suppressed
 *   from the log file.  When more mode is turned off, we don't
 *   automatically copy text to the log file; any text that the caller
 *   knows should be in the log file during times when more mode is turned
 *   off can be explicitly added with this function.
 *   
 *   If nl is true, we'll add a newline at the end of this text.  The
 *   caller should not include any newlines in the text being displayed
 *   here.  
 */
void out_logfile_print(const char *txt, int nl);


/*
 *   Check output status.  Indicate whether output is currently hidden,
 *   and whether any hidden output has occurred. 
 */
void outstat(int *hidden, int *output_occurred);

/* hide/show output */
void outhide(void);
int outshow(void);

/* set the flag to indicate that output has occurred */
void outsethidden(void);

/* write a blank line */
void outblank(void);

/* start/end watchpoint evaluation */
void outwx(int flag);

/* Begin/end capturing */
void tiocapture(tiocxdef *tioctx, mcmcxdef *memctx, int flag);

/* clear all captured output */
void tioclrcapture(tiocxdef *tioctx);

/* 
 *   clear captured output back to a given point -- this can be used to
 *   remove captured output in an inner capture from an enclosing capture 
 */
void tiopopcapture(tiocxdef *tioctx, uint orig_size);

/* get the object handle of the captured output */
mcmon tiogetcapture(tiocxdef *ctx);

/* get the amount of text captured */
uint tiocapturesize(tiocxdef *ctx);

/* turn MORE mode on or off */
int setmore(int state);

/* explicitly activate the "MORE" prompt */
void out_more_prompt();

/*
 *   QA controller functions
 */
int qasopn(char *scrnam, int quiet);
void qasclose(void);
char *qasgets(char *buf, int bufl);

/*
 *   Set an HTML entity expansion.  This is called during initialization
 *   when we read a character mapping table that includes HTML entity
 *   expansions.  The HTML run-time uses its own expansion mechanism, so
 *   it will ignore this information.  The standard character-mode TADS
 *   run-time, however, uses this information to map HTML entities to the
 *   local character set. 
 */
void tio_set_html_expansion(unsigned int html_char_val,
                            const char *expansion, size_t expansion_len);

/* check for HTML mode - returns true if an "\H+" sequence is active */
int tio_is_html_mode();

/* set the user output filter function */
void out_set_filter(objnum filter_fn);

/* set the double-space mode */
void out_set_doublespace(int dbl);

/*
 *   Ask for a filename, using a system-defined dialog (via os_askfile) if
 *   possible.  Uses the same interface as os_askfile(), which we will
 *   call directly for graphical implementations.  We'll use formatted
 *   text for text-only implementations.  
 */
int tio_askfile(const char *prompt, char *reply, int replen, int prompt_type, os_filetype_t file_type);

/*
 *   Display a dialog, using a system-defined dialog (via os_input_dialog)
 *   if possible.  Uses the same interface as os_input_dialog(), which we
 *   will call directly for graphical implementations.  We'll use
 *   formatted text for text-only implementations.  
 */
int tio_input_dialog(int icon_id, const char *prompt, int standard_button_set,
                     const char **buttons, int button_count,
                     int default_index, int cancel_index);


} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
