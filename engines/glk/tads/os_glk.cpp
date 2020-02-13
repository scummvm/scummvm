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

#include "glk/tads/os_glk.h"
#include "glk/tads/tads.h"
#include "glk/tads/os_buffer.h"

namespace Glk {
namespace TADS {

static void redraw_windows(void);
static void os_status_redraw(void);
extern void os_banners_redraw(void);

static char lbuf[256], rbuf[256];
static int curwin = 0;
static int curattr = 0;

winid_t mainwin;
winid_t statuswin;

uint mainfg;
uint mainbg;

uint statusfg;
uint statusbg;

int G_os_pagelength;
int G_os_linewidth;
int G_os_moremode;
char G_os_gamename[OSFNMAX];

/* ------------------------------------------------------------------------ */

/* 
 *   Initialize.  This should be called during program startup to
 *   initialize the OS layer and check OS-specific command-line arguments.
 *   
 *   If 'prompt' and 'buf' are non-null, and there are no arguments on the
 *   given command line, the OS code can use the prompt to ask the user to
 *   supply a filename, then store the filename in 'buf' and set up
 *   argc/argv to give a one-argument command string.  (This mechanism for
 *   prompting for a filename is obsolescent, and is retained for
 *   compatibility with a small number of existing implementations only;
 *   new implementations should ignore this mechanism and leave the
 *   argc/argv values unchanged.)  
 */
int os_init(int *argc, char *argv[], const char *prompt,
            char *buf, int bufsiz)
{
    mainwin = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
    if (!mainwin)
        error("fatal: could not open window!\n");

    /* get default colors for main window */
    if (!g_vm->glk_style_measure(mainwin, style_Normal, stylehint_TextColor, &mainfg))
        mainfg = 0;

    if (!g_vm->glk_style_measure(mainwin, style_Normal, stylehint_BackColor, &mainbg))
        mainbg = 0;

    /* get default colors for status window */
    statuswin = g_vm->glk_window_open(mainwin,
            winmethod_Above | winmethod_Fixed, 1,
            wintype_TextGrid, 0);

    if (!g_vm->glk_style_measure(statuswin, style_Normal, stylehint_TextColor, &statusfg))
        statusfg = 0;

    if (!g_vm->glk_style_measure(statuswin, style_Normal, stylehint_BackColor, &statusbg))
        statusbg = 0;

    /* close statuswin; reopened on request */
    g_vm->glk_window_close(statuswin, 0);

    statuswin = nullptr;

    g_vm->glk_set_window(mainwin);

    strcpy(rbuf, "");

    return 0;
}

/*
 *   Uninitialize.  This is called prior to progam termination to reverse
 *   the effect of any changes made in os_init().  For example, if
 *   os_init() put the terminal in raw mode, this should restore the
 *   previous terminal mode.  This routine should not terminate the
 *   program (so don't call exit() here) - the caller might have more
 *   processing to perform after this routine returns.
 */
void os_uninit(void)
{
}

void os_term(int status) {
	g_vm->quitGame();
}

void os_instbrk(int install) {
	// No implementation
}

bool os_break() {
	return false;
}

void os_sleep_ms(long delay_in_milliseconds) {
	g_system->delayMillis(delay_in_milliseconds);
}

/* ------------------------------------------------------------------------ */
/*
 *   Get system information.  'code' is a SYSINFO_xxx code, which
 *   specifies what type of information to get.  The 'param' argument's
 *   meaning depends on which code is selected.  'result' is a pointer to
 *   an integer that is to be filled in with the result value.  If the
 *   code is not known, this function should return false.  If the code is
 *   known, the function should fill in *result and return true.
 */
int os_get_sysinfo(int code, void *param, long *result) {
    switch (code)
    {
        case SYSINFO_TEXT_HILITE:
            *result = 1;
            return true;
        case SYSINFO_BANNERS:
            *result = 1;
            return true;
        case SYSINFO_TEXT_COLORS:
            *result = SYSINFO_TXC_RGB;
            return true;

#ifdef USE_HTML
        case SYSINFO_INTERP_CLASS:
            *result = SYSINFO_ICLASS_HTML;
            return true;
        case SYSINFO_HTML:
            *result = 1;
            return true;
#else
        case SYSINFO_INTERP_CLASS:
            *result = SYSINFO_ICLASS_TEXTGUI;
            return true;
        case SYSINFO_HTML:
            *result = 0;
            return true;
#endif

        case SYSINFO_JPEG:
        case SYSINFO_PNG:
        case SYSINFO_WAV:
        case SYSINFO_MIDI:
        case SYSINFO_WAV_MIDI_OVL:
        case SYSINFO_WAV_OVL:
        case SYSINFO_PREF_IMAGES:
        case SYSINFO_PREF_SOUNDS:
        case SYSINFO_PREF_MUSIC:
        case SYSINFO_PREF_LINKS:
        case SYSINFO_MPEG:
        case SYSINFO_MPEG1:
        case SYSINFO_MPEG2:
        case SYSINFO_MPEG3:
        case SYSINFO_LINKS_HTTP:
        case SYSINFO_LINKS_FTP:
        case SYSINFO_LINKS_NEWS:
        case SYSINFO_LINKS_MAILTO:
        case SYSINFO_LINKS_TELNET:
        case SYSINFO_PNG_TRANS:
        case SYSINFO_PNG_ALPHA:
        case SYSINFO_OGG:
            *result = 0;
            return true;

        default:
            return false;
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Display routines.
 *   
 *   Our display model is a simple stdio-style character stream.
 *   
 *   In addition, we provide an optional "status line," which is a
 *   non-scrolling area where a line of text can be displayed.  If the status
 *   line is supported, text should only be displayed in this area when
 *   os_status() is used to enter status-line mode (mode 1); while in status
 *   line mode, text is written to the status line area, otherwise (mode 0)
 *   it's written to the normal main text area.  The status line is normally
 *   shown in a different color to set it off from the rest of the text.
 *   
 *   The OS layer can provide its own formatting (word wrapping in
 *   particular) if it wants, in which case it should also provide pagination
 *   using os_more_prompt().  
 */

/*
 *   Print a string on the console.  These routines come in two varieties:
 *   
 *   os_printz - write a NULL-TERMINATED string
 *.  os_print - write a COUNTED-LENGTH string, which may not end with a null
 *   
 *   These two routines are identical except that os_printz() takes a string
 *   which is terminated by a null byte, and os_print() instead takes an
 *   explicit length, and a string that may not end with a null byte.
 *   
 *   os_printz(str) may be implemented as simply os_print(str, strlen(str)).
 *   
 *   The string is written in one of three ways, depending on the status mode
 *   set by os_status():
 *   
 *   status mode == 0 -> write to main text window
 *.  status mode == 1 -> write to status line
 *.  anything else -> do not display the text at all
 *   
 *   Implementations are free to omit any status line support, in which case
 *   they should simply suppress all output when the status mode is anything
 *   other than zero.
 *   
 *   The following special characters must be recognized in the displayed
 *   text:
 *   
 *   '\n' - newline: end the current line and move the cursor to the start of
 *   the next line.  If the status line is supported, and the current status
 *   mode is 1 (i.e., displaying in the status line), then two special rules
 *   apply to newline handling: newlines preceding any other text should be
 *   ignored, and a newline following any other text should set the status
 *   mode to 2, so that all subsequent output is suppressed until the status
 *   mode is changed with an explicit call by the client program to
 *   os_status().
 *   
 *   '\r' - carriage return: end the current line and move the cursor back to
 *   the beginning of the current line.  Subsequent output is expected to
 *   overwrite the text previously on this same line.  The implementation
 *   may, if desired, IMMEDIATELY clear the previous text when the '\r' is
 *   written, rather than waiting for subsequent text to be displayed.
 *   
 *   All other characters may be assumed to be ordinary printing characters.
 *   The routine need not check for any other special characters.
 *   
 */

void os_printz(const char *str) {
    os_print(str, strlen(str));
}

void os_print(const char *str, size_t len) {
    if (curwin == 0 && str)
        os_put_buffer(str, len);

    if (curwin == 1)
    {
        const char *p;
        size_t      rem, max;

        /* The string requires some fiddling for the status window */
        for (p = str, rem = len ; rem != 0 && *p == '\n'; p++, --rem)
            ;
        if (rem != 0 && p[rem-1] == '\n')
            --rem;

        /* if that leaves anything, update the statusline */
        if (rem != 0)
        {
            max = sizeof(lbuf) - strlen(lbuf) - 1;
            strncat(lbuf, p, rem > max ? max : rem);
            os_status_redraw();
        }
    }
}


/* 
 *   Set the status line mode.  There are three possible settings:
 *   
 *   0 -> main text mode.  In this mode, all subsequent text written with
 *   os_print() and os_printz() is to be displayed to the main text area.
 *   This is the normal mode that should be in effect initially.  This mode
 *   stays in effect until an explicit call to os_status().
 *   
 *   1 -> statusline mode.  In this mode, text written with os_print() and
 *   os_printz() is written to the status line, which is usually rendered as
 *   a one-line area across the top of the terminal screen or application
 *   window.  In statusline mode, leading newlines ('\n' characters) are to
 *   be ignored, and any newline following any other character must change
 *   the mode to 2, as though os_status(2) had been called.
 *   
 *   2 -> suppress mode.  In this mode, all text written with os_print() and
 *   os_printz() must simply be ignored, and not displayed at all.  This mode
 *   stays in effect until an explicit call to os_status().  
 */

void os_status(int stat)
{
    curwin = stat;

    if (stat == 1)
    {
        if (statuswin == NULL)
        {
            g_vm->glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);
            statuswin = g_vm->glk_window_open(mainwin,
                                        winmethod_Above | winmethod_Fixed, 1,
                                        wintype_TextGrid, 0);
        }
        strcpy(lbuf, "");
    }
}

/* get the status line mode */
int os_get_status()
{
    return curwin;
}

/* 
 *   Set the score value.  This displays the given score and turn counts on
 *   the status line.  In most cases, these values are displayed at the right
 *   edge of the status line, in the format "score/turns", but the format is
 *   up to the implementation to determine.  In most cases, this can simply
 *   be implemented as follows:
 *   
 */
void os_score(int score, int turncount)
{
    char buf[40];
    sprintf(buf, "%d/%d", score, turncount);
    os_strsc(buf);
}

/* display a string in the score area in the status line */
void os_strsc(const char *p)
{
    snprintf(rbuf, sizeof rbuf, "%s", p);
    os_status_redraw();
}

static void os_status_redraw(void) {
    char fmt[32];
    char buf[256];
    uint wid;
    uint div;

    if (!statuswin)
        return;

    g_vm->glk_window_get_size(statuswin, &wid, NULL);
    div = wid - strlen(rbuf) - 3;

    sprintf(fmt, " %%%ds %%s ", - (int)div);
    sprintf(buf, fmt, lbuf, rbuf);

    g_vm->glk_window_clear(statuswin);
    g_vm->glk_set_window(statuswin);
    g_vm->glk_set_style(style_User1);
    os_put_buffer(buf, strlen(buf));
    g_vm->glk_set_window(mainwin);
}

static void redraw_windows(void)
{
    os_status_redraw();
    os_banners_redraw();
}

/* clear the screen */
void oscls(void)
{
    g_vm->glk_window_clear(mainwin);
}

/* ------------------------------------------------------------------------ */
/*
 *   Set text attributes.  Text subsequently displayed through os_print() and
 *   os_printz() are to be displayed with the given attributes.
 *   
 *   'attr' is a (bitwise-OR'd) combination of OS_ATTR_xxx values.  A value
 *   of zero indicates normal text, with no extra attributes.  
 */
void os_set_text_attr(int attr)
{
    curattr = attr;
    if (curattr & OS_ATTR_BOLD && curattr & OS_ATTR_ITALIC)
        g_vm->glk_set_style(style_Alert);
    else if (curattr & OS_ATTR_BOLD)
        g_vm->glk_set_style(style_Subheader);
    else if (curattr & OS_ATTR_ITALIC)
        g_vm->glk_set_style(style_Emphasized);
    else
        g_vm->glk_set_style(style_Normal);
}

/*
 *   Set the text foreground and background colors.  This sets the text
 *   color for subsequent os_printf() and os_vprintf() calls.
 *   
 *   The background color can be OS_COLOR_TRANSPARENT, in which case the
 *   background color is "inherited" from the current screen background.
 *   Note that if the platform is capable of keeping old text for
 *   "scrollback," then the transparency should be a permanent attribute of
 *   the character - in other words, it should not be mapped to the current
 *   screen color in the scrollback buffer, because doing so would keep the
 *   current screen color even if the screen color changes in the future. 
 *   
 *   Text color support is optional.  If the platform doesn't support text
 *   colors, this can simply do nothing.  If the platform supports text
 *   colors, but the requested color or attributes cannot be displayed, the
 *   implementation should use the best available approximation.  
 */
void os_set_text_color(os_color_t fg, os_color_t bg) {
}

/*
 *   Set the screen background color.  This sets the text color for the
 *   background of the screen.  If possible, this should immediately redraw
 *   the main text area with this background color.  The color is given as an
 *   OS_COLOR_xxx value.
 *   
 *   If the platform is capable of redisplaying the existing text, then any
 *   existing text that was originally displayed with 'transparent'
 *   background color should be redisplayed with the new screen background
 *   color.  In other words, the 'transparent' background color of previously
 *   drawn text should be a permanent attribute of the character - the color
 *   should not be mapped on display to the then-current background color,
 *   because doing so would lose the transparency and thus retain the old
 *   screen color on a screen color change.  
 */
void os_set_screen_color(os_color_t color)
{
}

/*
 *   Set the game title.  The output layer calls this routine when a game
 *   sets its title (via an HTML <title> tag, for example).  If it's
 *   convenient to do so, the OS layer can use this string to set a window
 *   caption, or whatever else makes sense on each system.  Most
 *   character-mode implementations will provide an empty implementation,
 *   since there's not usually any standard way to show the current
 *   application title on a character-mode display.  
 */
void os_set_title(const char *title)
{
#ifdef GARGLK
    g_vm->garglk_set_story_title(title);
#endif
}

/*
 *   Show the system-specific MORE prompt, and wait for the user to respond.
 *   Before returning, remove the MORE prompt from the screen.
 *   
 *   This routine is only used and only needs to be implemented when the OS
 *   layer takes responsibility for pagination; this will be the case on
 *   most systems that use proportionally-spaced (variable-pitch) fonts or
 *   variable-sized windows, since on such platforms the OS layer must do
 *   most of the formatting work, leaving the standard output layer unable
 *   to guess where pagination should occur.
 *   
 *   If the portable output formatter handles the MORE prompt, which is the
 *   usual case for character-mode or terminal-style implementations, this
 *   routine is not used and you don't need to provide an implementation.
 *   Note that HTML TADS provides an implementation of this routine, because
 *   the HTML renderer handles line breaking and thus must handle
 *   pagination.  
 */
void os_more_prompt()
{
    os_printz("\n[more]\n");
    os_waitc();
}

/* ------------------------------------------------------------------------ */
/*
 *   User Input Routines
 */

/*
 *   Ask the user for a filename, using a system-dependent dialog or other
 *   mechanism.  Returns one of the OS_AFE_xxx status codes (see below).
 *   
 *   prompt_type is the type of prompt to provide -- this is one of the
 *   OS_AFP_xxx codes (see below).  The OS implementation doesn't need to
 *   pay any attention to this parameter, but it can be used if desired to
 *   determine the type of dialog to present if the system provides
 *   different types of dialogs for different types of operations.
 *   
 *   file_type is one of the OSFTxxx codes for system file type.  The OS
 *   implementation is free to ignore this information, but can use it to
 *   filter the list of files displayed if desired; this can also be used
 *   to apply a default suffix on systems that use suffixes to indicate
 *   file type.  If OSFTUNK is specified, it means that no filtering
 *   should be performed, and no default suffix should be applied.  
 */
int os_askfile(const char *prompt, char *fname_buf, int fname_buf_len,
               int prompt_type, os_filetype_t file_type)
{
    frefid_t fileref;
    uint gprompt, gusage;

    if (prompt_type == OS_AFP_OPEN)
        gprompt = filemode_Read;
    else
        gprompt = filemode_ReadWrite;

    if (file_type == OSFTSAVE || file_type == OSFTT3SAV)
        gusage = fileusage_SavedGame;
    else if (file_type == OSFTLOG || file_type == OSFTTEXT)
        gusage = fileusage_Transcript;
    else
        gusage = fileusage_Data;

    fileref = g_vm->glk_fileref_create_by_prompt(gusage, (FileMode)gprompt, 0);
    if (fileref == NULL)
        return OS_AFE_CANCEL;

    strcpy(fname_buf, g_vm->garglk_fileref_get_name(fileref));

    g_vm->glk_fileref_destroy(fileref);

    return OS_AFE_SUCCESS;
}

/* 
 *   Read a string of input.  Fills in the buffer with a null-terminated
 *   string containing a line of text read from the standard input.  The
 *   returned string should NOT contain a trailing newline sequence.  On
 *   success, returns 'buf'; on failure, including end of file, returns a
 *   null pointer.  
 */
unsigned char *os_gets(unsigned char *buf, size_t buflen)
{
    event_t event;
	char *b = (char *)buf;

    os_get_buffer(b, buflen, 0);

    do
    {
        g_vm->glk_select(&event);
        if (event.type == evtype_Arrange)
            redraw_windows();
    }
    while (event.type != evtype_LineInput);

    return (unsigned char *)os_fill_buffer(b, event.val1);
}

/*
 *   Read a string of input with an optional timeout.  This behaves like
 *   os_gets(), in that it allows the user to edit a line of text (ideally
 *   using the same editing keys that os_gets() does), showing the line of
 *   text under construction during editing.  This routine differs from
 *   os_gets() in that it returns if the given timeout interval expires
 *   before the user presses Return (or the local equivalent).
 *   
 *   If the user presses Return before the timeout expires, we store the
 *   command line in the given buffer, just as os_gets() would, and we
 *   return OS_EVT_LINE.  We also update the display in the same manner that
 *   os_gets() would, by moving the cursor to a new line and scrolling the
 *   displayed text as needed.
 *   
 *   If a timeout occurs before the user presses Return, we store the
 *   command line so far in the given buffer, statically store the cursor
 *   position, insert mode, buffer text, and anything else relevant to the
 *   editing state, and we return OS_EVT_TIMEOUT.
 *   
 *   If the implementation does not support the timeout operation, this
 *   routine should simply return OS_EVT_NOTIMEOUT immediately when called;
 *   the routine should not allow the user to perform any editing if the
 *   timeout is not supported.  Callers must use the ordinary os_gets()
 *   routine, which has no timeout capabilities, if the timeout is not
 *   supported.
 *   
 *   When we return OS_EVT_TIMEOUT, the caller is responsible for doing one
 *   of two things.
 *   
 *   The first possibility is that the caller performs some work that
 *   doesn't require any display operations (in other words, the caller
 *   doesn't invoke os_printf, os_getc, or anything else that would update
 *   the display), and then calls os_gets_timeout() again.  In this case, we
 *   will use the editing state that we statically stored before we returned
 *   OS_EVT_TIMEOUT to continue editing where we left off.  This allows the
 *   caller to perform some computation in the middle of user command
 *   editing without interrupting the user - the extra computation is
 *   transparent to the user, because we act as though we were still in the
 *   midst of the original editing.
 *   
 *   The second possibility is that the caller wants to update the display.
 *   In this case, the caller must call os_gets_cancel() BEFORE making any
 *   display changes.  Then, the caller must do any post-input work of its
 *   own, such as updating the display mode (for example, closing HTML font
 *   tags that were opened at the start of the input).  The caller is now
 *   free to do any display work it wants.
 *   
 *   If we have information stored from a previous call that was interrupted
 *   by a timeout, and os_gets_cancel(true) was never called, we will resume
 *   editing where we left off when the cancelled call returned; this means
 *   that we'll restore the cursor position, insertion state, and anything
 *   else relevant.  Note that if os_gets_cancel(false) was called, we must
 *   re-display the command line under construction, but if os_gets_cancel()
 *   was never called, we will not have to make any changes to the display
 *   at all.
 *   
 *   Note that when resuming an interrupted editing session (interrupted via
 *   os_gets_cancel()), the caller must re-display the prompt prior to
 *   invoking this routine.
 *   
 *   Note that we can return OS_EVT_EOF in addition to the other codes
 *   mentioned above.  OS_EVT_EOF indicates that an error occurred reading,
 *   which usually indicates that the application is being terminated or
 *   that some hardware error occurred reading the keyboard.  
 *   
 *   If 'use_timeout' is false, the timeout should be ignored.  Without a
 *   timeout, the function behaves the same as os_gets(), except that it
 *   will resume editing of a previously-interrupted command line if
 *   appropriate.  (This difference is why the timeout is optional: a caller
 *   might not need a timeout, but might still want to resume a previous
 *   input that did time out, in which case the caller would invoke this
 *   routine with use_timeout==false.  The regular os_gets() would not
 *   satisfy this need, because it cannot resume an interrupted input.)  
 */
#if defined GLK_TIMERS && defined GLK_MODULE_LINE_ECHO
static char * timebuf = NULL;
static size_t timelen = 0;
#endif

int os_gets_timeout(unsigned char *buf, size_t bufl,
                    unsigned long timeout_in_milliseconds, int use_timeout)
{
#if defined GLK_TIMERS && defined GLK_MODULE_LINE_ECHO
    int timer = use_timeout ? timeout_in_milliseconds : 0;
    int timeout = 0;
    int initlen = 0;
    event_t event;

    /* restore saved buffer contents */
    if (timebuf)
    {
        assert(timelen && timelen <= bufl);
        memcpy(buf, timebuf, timelen);
        initlen = timelen - 1;
        buf[initlen] = 0;
        free(timebuf);
        timebuf = 0;
    }

    /* start timer and turn off line echo */
    if (timer)
    {
        g_vm->glk_request_timer_events(timer);
        g_vm->glk_set_echo_line_event(mainwin, 0);
    }

    os_get_buffer(buf, bufl, initlen);

    do
    {
        g_vm->glk_select(&event);
        if (event.type == evtype_Arrange)
            redraw_windows();
        else if (event.type == evtype_Timer && (timeout = 1))
            g_vm->glk_cancel_line_event(mainwin, &event);
    }
    while (event.type != evtype_LineInput);

    char *res = os_fill_buffer(buf, event.val1);

    /* stop timer and turn on line echo */
    if (timer)
    {
        g_vm->glk_request_timer_events(0);
        g_vm->glk_set_echo_line_event(mainwin, 1);
    }

    /* save or print buffer contents */
    if (res && timer)
    {
        if (timeout)
        {
            timelen = strlen(buf) + 1;
            timebuf = malloc(timelen);
            memcpy(timebuf, buf, timelen);
        }
        else
        {
            g_vm->glk_set_style(style_Input);
            os_print(buf, strlen(buf));
            os_print("\n", 1);
            g_vm->glk_set_style(style_Normal);
        }
    }

    return timeout ? OS_EVT_TIMEOUT : res ? OS_EVT_LINE : OS_EVT_EOF;
#else
    return OS_EVT_NOTIMEOUT;
#endif
}

/*
 *   Cancel an interrupted editing session.  This MUST be called if any
 *   output is to be displayed after a call to os_gets_timeout() returns
 *   OS_EVT_TIMEOUT.
 *   
 *   'reset' indicates whether or not we will forget the input state saved
 *   by os_gets_timeout() when it last returned.  If 'reset' is true, we'll
 *   clear the input state, so that the next call to os_gets_timeout() will
 *   start with an empty input buffer.  If 'reset' is false, we will retain
 *   the previous input state, if any; this means that the next call to
 *   os_gets_timeout() will re-display the same input buffer that was under
 *   construction when it last returned.
 *   
 *   This routine need not be called if os_gets_timeout() is to be called
 *   again with no other output operations between the previous
 *   os_gets_timeout() call and the next one.
 *   
 *   Note that this routine needs only a trivial implementation when
 *   os_gets_timeout() is not supported (i.e., the function always returns
 *   OS_EVT_NOTIMEOUT).  
 */
void os_gets_cancel(int reset)
{
#if defined GLK_TIMERS && defined GLK_MODULE_LINE_ECHO
    if (timebuf)
    {
        g_vm->glk_set_style(style_Input);
        os_print(timebuf, strlen(timebuf));
        os_print("\n", 1);
        g_vm->glk_set_style(style_Normal);

        if (reset)
        {
            free(timebuf);
            timebuf = 0;
        }
    }
#endif
}

/* 
 *   Read a character from the keyboard.  For extended keystrokes, this
 *   function returns zero, and then returns the CMD_xxx code for the
 *   extended keystroke on the next call.  For example, if the user
 *   presses the up-arrow key, the first call to os_getc() should return
 *   0, and the next call should return CMD_UP.  Refer to the CMD_xxx
 *   codes below.
 *   
 *   os_getc() should return a high-level, translated command code for
 *   command editing.  This means that, where a functional interpretation
 *   of a key and the raw key-cap interpretation both exist as CMD_xxx
 *   codes, the functional interpretation should be returned.  For
 *   example, on Unix, Ctrl-E is conventionally used in command editing to
 *   move to the end of the line, following Emacs key bindings.  Hence,
 *   os_getc() should return CMD_END for this keystroke, rather than
 *   (CMD_CTRL + 'E' - 'A'), because CMD_END is the high-level command
 *   code for the operation.
 *   
 *   The translation ability of this function allows for system-dependent
 *   key mappings to functional meanings.  
 */
static int glktotads(unsigned int key)
{
    if (key < 256)
        return key;
    switch (key)
    {
        case keycode_Up:
            return CMD_UP;
        case keycode_Down:
            return CMD_DOWN;
        case keycode_Left:
            return CMD_LEFT;
        case keycode_Right:
            return CMD_RIGHT;
        case keycode_PageUp:
            return CMD_PGUP;
        case keycode_PageDown:
            return CMD_PGDN;
        case keycode_Home:
            return CMD_HOME;
        case keycode_End:
            return CMD_END;
        case keycode_Func1:
            return CMD_F1;
        case keycode_Func2:
            return CMD_F2;
        case keycode_Func3:
            return CMD_F3;
        case keycode_Func4:
            return CMD_F4;
        case keycode_Func5:
            return CMD_F5;
        case keycode_Func6:
            return CMD_F6;
        case keycode_Func7:
            return CMD_F7;
        case keycode_Func8:
            return CMD_F8;
        case keycode_Func9:
            return CMD_F9;
        case keycode_Func10:
            return CMD_F10;
        default:
            return 0;
    }
}

static int bufchar = 0;
static int waitchar = 0;
static int timechar = 0;

static int getglkchar(void)
{
    event_t event;

    timechar = 0;

    g_vm->glk_request_char_event(mainwin);

    do
    {
        g_vm->glk_select(&event);
        if (event.type == evtype_Arrange)
            redraw_windows();
        else if (event.type == evtype_Timer)
            timechar = 1;
    }
    while (event.type != evtype_CharInput && event.type != evtype_Timer);

    g_vm->glk_cancel_char_event(mainwin);

    return timechar ? 0 : event.val1;
}

int os_getc(void)
{
    unsigned int c;

    if (bufchar)
    {
        c = bufchar;
        bufchar = 0;
        return c;
    }

    c = waitchar ? waitchar : getglkchar();
    waitchar = 0;

	if (c == keycode_Return)
		c = '\n';
	else if (c == keycode_Tab)
		c = '\t';
	else if (c == keycode_Escape)
		c = 27;

    if (c < 256)
        return c;

    bufchar = glktotads(c);

    return 0;
}

/*
 *   Read a character from the keyboard, following the same protocol as
 *   os_getc() for CMD_xxx codes (i.e., when an extended keystroke is
 *   encountered, os_getc_raw() returns zero, then returns the CMD_xxx code
 *   on the subsequent call).
 *   
 *   This function differs from os_getc() in that this function returns the
 *   low-level, untranslated key code whenever possible.  This means that,
 *   when a functional interpretation of a key and the raw key-cap
 *   interpretation both exist as CMD_xxx codes, this function returns the
 *   key-cap interpretation.  For the Unix Ctrl-E example in the comments
 *   describing os_getc() above, this function should return 5 (the ASCII
 *   code for Ctrl-E), because the CMD_CTRL interpretation is the low-level
 *   key code.
 *   
 *   This function should return all control keys using their ASCII control
 *   codes, whenever possible.  Similarly, this function should return ASCII
 *   27 for the Escape key, if possible.  
 *   
 *   For keys for which there is no portable ASCII representation, this
 *   should return the CMD_xxx sequence.  So, this function acts exactly the
 *   same as os_getc() for arrow keys, function keys, and other special keys
 *   that have no ASCII representation.  This function returns a
 *   non-translated version ONLY when an ASCII representation exists - in
 *   practice, this means that this function and os_getc() vary only for
 *   CTRL keys and Escape.
 */
int os_getc_raw(void)
{
    return os_getc();
}

/* wait for a character to become available from the keyboard */
void os_waitc(void)
{
    waitchar = getglkchar();
}

/*
 *   Get an input event.  The event types are shown above.  If use_timeout
 *   is false, this routine should simply wait until one of the events it
 *   recognizes occurs, then return the appropriate information on the
 *   event.  If use_timeout is true, this routine should return
 *   OS_EVT_TIMEOUT after the given number of milliseconds elapses if no
 *   event occurs first.
 *   
 *   This function is not obligated to obey the timeout.  If a timeout is
 *   specified and it is not possible to obey the timeout, the function
 *   should simply return OS_EVT_NOTIMEOUT.  The trivial implementation
 *   thus checks for a timeout, returns an error if specified, and
 *   otherwise simply waits for the user to press a key.  
 */
int os_get_event(unsigned long timeout_in_milliseconds, int use_timeout,
                 os_event_info_t *info)
{
#ifdef GLK_TIMERS
    /* start timer */
    int timer = use_timeout ? timeout_in_milliseconds : 0;
    if (timer)
        g_vm->glk_request_timer_events(timer);
#else
    /* we can't handle timeouts */
    if (use_timeout)
        return OS_EVT_NOTIMEOUT;
#endif

    /* get a key */
    info->key[0] = os_getc_raw();
    if (info->key[0] == 0 && timechar == 0)
        info->key[1] = os_getc_raw();

#ifdef GLK_TIMERS
    /* stop timer */
    if (timer)
        g_vm->glk_request_timer_events(0);
#endif

    /* return the event */
    return timechar ? OS_EVT_TIMEOUT : OS_EVT_KEY;
}

osfildef *os_exeseek(const char *argv0, const char *typ) {
	return nullptr;
}

int os_get_str_rsc(int id, char *buf, size_t buflen) {
	strcpy(buf, "");
	return 0;
}

void os_dbg_printf(const char *fmt, ...) {
	// No implementation, since we haven't set up a ScummVM debugger
}

void os_dbg_vprintf(const char *fmt, va_list args) {
	// No implementation, since we haven't set up a ScummVM debugger
}

int os_vasprintf(char **bufptr, const char *fmt, va_list ap) {
	Common::String s = Common::String::vformat(fmt, ap);

	*bufptr = (char *)malloc(s.size() + 1);
	strcpy(*bufptr, s.c_str());
	return s.size();
}

int os_paramfile(char *buf) {
	return false;
}

void os_rand(long *val) {
	*val = g_vm->getRandomNumber(0x7fffffff);
}

long os_get_sys_clock_ms() {
	return g_system->getMillis();
}

void os_xlat_html4(unsigned int html4_char, char *result, size_t result_len) {
	/* Return all standard Latin-1 characters as-is */
	if (html4_char <= 128 || (html4_char >= 160 && html4_char <= 255))
		result[0] = (unsigned char)html4_char;
	else {
		switch (html4_char) {
		case 130:                                      /* single back quote */
			result[0] = '`'; break;
		case 132:                                      /* double back quote */
			result[0] = '\"'; break;
		case 153:                                             /* trade mark */
			strcpy(result, "(tm)"); return;
		case 140:                                            /* OE ligature */
		case 338:                                            /* OE ligature */
			strcpy(result, "OE"); return;
		case 339:                                            /* oe ligature */
			strcpy(result, "oe"); return;
		case 159:                                                   /* Yuml */
			result[0] = (char)255; return;
		case 376:                                        /* Y with diaresis */
			result[0] = 'Y'; break;
		case 352:                                           /* S with caron */
			result[0] = 'S'; break;
		case 353:                                           /* s with caron */
			result[0] = 's'; break;
		case 150:                                                /* en dash */
		case 8211:                                               /* en dash */
			result[0] = '-'; break;
		case 151:                                                /* em dash */
		case 8212:                                               /* em dash */
			strcpy(result, "--"); return;
		case 145:                                      /* left single quote */
		case 8216:                                     /* left single quote */
			result[0] = '`'; break;
		case 146:                                     /* right single quote */
		case 8217:                                    /* right single quote */
		case 8218:                                    /* single low-9 quote */
			result[0] = '\''; break;
		case 147:                                      /* left double quote */
		case 148:                                     /* right double quote */
		case 8220:                                     /* left double quote */
		case 8221:                                    /* right double quote */
		case 8222:                                    /* double low-9 quote */
			result[0] = '\"'; break;
		case 8224:                                                /* dagger */
		case 8225:                                         /* double dagger */
		case 8240:                                        /* per mille sign */
			result[0] = ' '; break;
		case 139:                       /* single left-pointing angle quote */
		case 8249:                      /* single left-pointing angle quote */
			result[0] = '<'; break;
		case 155:                      /* single right-pointing angle quote */
		case 8250:                     /* single right-pointing angle quote */
			result[0] = '>'; break;
		case 8482:                                           /* small tilde */
			result[0] = '~'; break;

		default:
			/* unmappable character - return space */
			result[0] = (unsigned char)' ';
		}
	}
	result[1] = 0;
}

#ifndef os_tzset
void os_tzset() {}
#endif

void os_nonstop_mode(int flag) {}

void os_advise_load_charmap(const char *id, const char *ldesc, const char *sysinfo) {}

void os_gen_charmap_filename(char *filename, char *internal_id, char *argv0) {}

int os_input_dialog(int icon_id, const char *prompt, int standard_button_set,
	const char **buttons, int button_count, int default_index, int cancel_index) {
	// CUrrently unsupported
	return 0;
}

void os_flush() {
	g_vm->glk_tick();
}

char *os_strlwr(char *s) {
	for (char *p = s; *p; ++p)
		*p = tolower(*p);
	return s;
}

void os_expause() {
#ifdef USE_EXPAUSE
	os_printz("(Strike any key to exit...)");
	os_flush();
	os_waitc();
#endif /* USE_EXPAUSE */
}

void os_plain(void) {}

int memicmp(const char *s1, const char *s2, int len) {
	Common::String cs1(s1, len);
	Common::String cs2(s2, len);

	return cs1.compareToIgnoreCase(cs2);
}

} // End of namespace TADS
} // End of namespace Glk
