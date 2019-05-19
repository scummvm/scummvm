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

/* TADS OS interface file type definitions
 *
 * Defines certain datatypes used in the TADS operating system interface
 */

#ifndef GLK_TADS_OS_BANNERS
#define GLK_TADS_OS_BANNERS

#include "common/scummsys.h"
#include "glk/tads/os_glk.h"

namespace Glk {
namespace TADS {

/* ------------------------------------------------------------------------ */
/*
 *   External Banner Interface.  This interface provides the ability to
 *   divide the display window into multiple sub-windows, each with its own
 *   independent contents.
 *   
 *   To determine where a new banner is displayed, we look at the banners as
 *   a tree, rooted at the "main window," the special banner that the system
 *   automatically creates initially for the main game text.  We start by
 *   allocating the entire display (or the entire application window, if
 *   we're running on a GUI system) to the main window.  We then traverse
 *   the tree, starting with the root window's children.  For each child
 *   window, we allocate space for the child out of the parent window's
 *   area, according to the child's alignment and size settings, and deduct
 *   this space from the parent window's size.  We then lay out the children
 *   of the child.
 *   
 *   For each banner window, we take its requested space out of the parent
 *   window's area by starting at the edge of the parent window rectangle as
 *   indicated by the banner's alignment, and taking the requested `width
 *   (for a left/right banner) or height (for a top/bottom banner), limiting
 *   to the available width/height in the parent window's space.  Give the
 *   banner the full extent of the parent's space in its other dimension (so
 *   a left/right banner gets the full height of the parent space, and a
 *   top/bottom banner gets the full width).
 *   
 *   Note that the layout proceeds exclusively down the tree (i.e., from the
 *   root to children to grandchildren, and so on).  It *appears* that a
 *   child affects its parent, because of the deduction step: a child
 *   acquires screen space by carving out a chunk of its parent.  The right
 *   way to think about this, though, is that the parent's full area is the
 *   union of the parent window and all of its children; when viewed this
 *   way, the parent's full area is fully determined the instant the parent
 *   is laid out, and never changes as its children are laid out.  Note in
 *   particular that a child can never make a parent larger; the only thing
 *   a child can do to a parent is carve out a chunk of the parent for
 *   itself, which doesn't affect the boundaries of the union of the parent
 *   plus its children.
 *   
 *   Note also that if the banner has a border, and the implementation
 *   actually draws borders, the border must be drawn for the *full* area of
 *   the banner, as defined above.  For example, suppose we have two
 *   borders: banner A is a child of the main window, is top-aligned, and
 *   has a border.  Banner B is a child of banner A, right-aligned, with no
 *   border.  Obviously, without considering banner B, banner A's space runs
 *   across the entire width of the main window, so its border (at the
 *   bottom of its area) runs across the entire width of the main window.
 *   Banner B carves out some space from A's right side for itself, so
 *   banner A's actual on-screen area runs from the left edge of the main
 *   window to banner B's left edge.  However, even though banner A itself
 *   no longer runs the full width of the main window, banner A's *full*
 *   area - that is, the union of banner A's on-screen area and all of its
 *   children's full areas - does still run the entire width of the main
 *   window, hence banner A's border must still run the full width of the
 *   main window.  The simple way of looking at this is that a banner's
 *   border is always to be drawn exactly the same way, regardless of
 *   whether or not the banner has children - simply draw the banner as it
 *   would be drawn if the banner had no children.
 *   
 *   Each time a banner is added or removed, we must recalculate the layout
 *   of the remaining banners and main text area.  The os_banner_xxx()
 *   implementation is responsible for this layout refiguring.
 *   
 *   The entire external banner window interface is optional, although the
 *   functions must at least be defined as dummies to avoid linker errors
 *   when building.  If a platform doesn't implement this feature,
 *   os_banner_create() should simply return null, and the other routines
 *   can do nothing.  
 */

/* 
 *   Create a banner window.  'info' gives the desired parameters for the new
 *   banner.
 *   
 *   Note that certain requested parameter settings might or might not be
 *   respected, depending on the capabilities of the platform and user
 *   preferences.  os_banner_getinfo() can be used after creation to
 *   determine which parameter settings are actually used in the new banner.
 *   
 *   'parent' gives the parent of this banner; this is the banner handle of
 *   another banner window, or null.  If 'parent' is null, then the new
 *   banner is a child of the main window, which the system creates
 *   automatically at startup and which contains the main input/output
 *   transcript.  The new banner's on-screen area is carved out of the
 *   parent's space, according to the alignment and size settings of the new
 *   window, so this determines how the window is laid out on the screen.
 *   
 *   'where' is OS_BANNER_FIRST to make the new window the first child of its
 *   parent; OS_BANNER_LAST to make it the last child of its parent;
 *   OS_BANNER_BEFORE to insert it immediately before the existing banner
 *   identified by handle in 'other'; or OS_BANNER_AFTER to insert
 *   immediately after 'other'.  When BEFORE or AFTER is used, 'other' must
 *   be another child of the same parent; if it is not, the routine should
 *   act as though 'where' were given as OS_BANNER_LAST.
 *   
 *   'other' is a banner handle for an existing banner window.  This is used
 *   to specify the relative position among children of the new banner's
 *   parent, if 'where' is either OS_BANNER_BEFORE or OS_BANNER_AFTER.  If
 *   'where' is OS_BANNER_FIRST or OS_BANNER_LAST, 'other' is ignored.
 *   
 *   'wintype' is the type of the window.  This is one of the
 *   OS_BANNER_TYPE_xxx codes indicating what kind of window is desired.
 *   
 *   'align' is the banner's alignment, given as an OS_BANNER_ALIGN_xxx
 *   value.  Top/bottom banners are horizontal: they run across the full
 *   width of the existing main text area.  Left/right banners are vertical:
 *   they run down the full height of the existing main text area.
 *   
 *   'siz' is the requested size of the new banner.  The meaning of 'siz'
 *   depends on the value of 'siz_units', which can be OS_BANNER_SIZE_PCT to
 *   set the size as a percentage of the REMAINING space, or
 *   OS_BANNER_SIZE_ABS to set an absolute size in the "natural" units of the
 *   window.  The natural units vary by window type: for text and text grid
 *   windows, this is in rows/columns of '0' characters in the default font
 *   for the window.  Note that when OS_BANNER_SIZE_ABS is used in a text or
 *   text grid window, the OS implementation MUST add the space needed for
 *   margins and borders when determining the actual pixel size of the
 *   window; in other words, the window should be large enough that it can
 *   actually display the given number or rows or columns.
 *   
 *   The size is interpreted as a width or height according to the window's
 *   orientation.  For a TOP or BOTTOM banner, the size is the height; for a
 *   LEFT or RIGHT banner, the size is the width.  A banner has only one
 *   dimension's size given, since the other dimension's size is determined
 *   automatically by the layout rules.
 *   
 *   Note that the window's size can be changed later using
 *   banner_size_to_contents() or banner_set_size().
 *   
 *   'style' is a combination of OS_BANNER_STYLE_xxx flags - see below.  The
 *   style flags give the REQUESTED style for the banner, which might or
 *   might not be respected, depending on the platform's capabilities, user
 *   preferences, and other factors.  os_banner_getinfo() can be used to
 *   determine which style flags are actually used.
 *   
 *   Returns the "handle" to the new banner window, which is an opaque value
 *   that is used in subsequent os_banner_xxx calls to operate on the window.
 *   Returns null if the window cannot be created.  An implementation is not
 *   required to support this functionality at all, and can subset it if it
 *   does support it (for example, an implementation could support only
 *   top/bottom-aligned banners, but not left/right-aligned), so callers must
 *   be prepared for this routine to return null.  
 */
void *os_banner_create(void *parent, int where, void *other, int wintype,
                       int align, int siz, int siz_units,
                       unsigned long style);


/*
 *   insertion positions 
 */
#define OS_BANNER_FIRST   1
#define OS_BANNER_LAST    2
#define OS_BANNER_BEFORE  3
#define OS_BANNER_AFTER   4

/*
 *   banner types 
 */

/* 
 *   Normal text stream window.  This is a text stream that behaves
 *   essentially like the main text window: text is displayed to this
 *   through os_banner_disp(), always in a stream-like fashion by adding new
 *   text to the end of any exiting text.
 *   
 *   Systems that use proportional fonts should usually simply use the same
 *   font they use by default in the main text window.  However, note that
 *   the OS_BANNER_STYLE_TAB_ALIGN style flag might imply that a fixed-pitch
 *   font should be used even when proportional fonts are available, because
 *   a fixed-pitch font will allow the calling code to rely on using spaces
 *   to align text within the window.  
 */
#define OS_BANNER_TYPE_TEXT       1

/* 
 *   "Text grid" window.  This type of window is similar to an normal text
 *   window (OS_BANNER_TYPE_TEXT), but is guaranteed to arrange its text in
 *   a regular grid of character cells, all of the same size.  This means
 *   that the output position can be moved to an arbitrary point within the
 *   window at any time, so the calling program can precisely control the
 *   layout of the text in the window.
 *   
 *   Because the output position can be moved to arbitrary positions in the
 *   window, it is possible to overwrite text previously displayed.  When
 *   this happens, the old text is completely obliterated by the new text,
 *   leaving no trace of the overwritten text.
 *   
 *   In order to guarantee that character cells are all the same size, this
 *   type of window does not allow any text attributes.  The implementation
 *   should simply ignore any attempts to change text attributes in this
 *   type of window.  However, colors can be used to the same degree they
 *   can be used in an ordinary text window.
 *   
 *   To guarantee the regular spacing of character cells, all
 *   implementations must use fixed-pitch fonts for these windows.  This
 *   applies even to platforms where proportional fonts are available.  
 */
#define OS_BANNER_TYPE_TEXTGRID   2


/* 
 *   banner alignment types 
 */
#define OS_BANNER_ALIGN_TOP       0
#define OS_BANNER_ALIGN_BOTTOM    1
#define OS_BANNER_ALIGN_LEFT      2
#define OS_BANNER_ALIGN_RIGHT     3

/*
 *   size units 
 */
#define OS_BANNER_SIZE_PCT  1
#define OS_BANNER_SIZE_ABS  2


/* 
 *   banner style flags 
 */

/* 
 *   The banner has a visible border; this indicates that a line is to be
 *   drawn to separate the banner from the adjacent window or windows
 *   "inside" the banner.  So, a top-aligned banner will have its border
 *   drawn along its bottom edge; a left-aligned banner will show a border
 *   along its right edge; and so forth.
 *   
 *   Note that character-mode platforms generally do NOT respect the border
 *   style, since doing so takes up too much screen space.  
 */
#define OS_BANNER_STYLE_BORDER     0x00000001

/*
 *   The banner has a vertical/horizontal scrollbar.  Character-mode
 *   platforms generally do not support scrollbars.  
 */
#define OS_BANNER_STYLE_VSCROLL    0x00000002
#define OS_BANNER_STYLE_HSCROLL    0x00000004

/* 
 *   Automatically scroll the banner vertically/horizontally whenever new
 *   text is displayed in the window.  In other words, whenever
 *   os_banner_disp() is called, scroll the window so that the text that the
 *   new cursor position after the new text is displayed is visible in the
 *   window.
 *   
 *   Note that this style is independent of the presence of scrollbars.
 *   Even if there are no scrollbars, we can still scroll the window's
 *   contents programmatically.
 *   
 *   Implementations can, if desired, keep an internal buffer of the
 *   window's contents, so that the contents can be recalled via the
 *   scrollbars if the text displayed in the banner exceeds the space
 *   available in the banner's window on the screen.  If the implementation
 *   does keep such a buffer, we recommend the following method for managing
 *   this buffer.  If the AUTO_VSCROLL flag is not set, then the banner's
 *   contents should be truncated at the bottom when the contents overflow
 *   the buffer; that is, once the banner's internal buffer is full, any new
 *   text that the calling program attempts to add to the banner should
 *   simply be discarded.  If the AUTO_VSCROLL flag is set, then the OLDEST
 *   text should be discarded instead, so that the most recent text is
 *   always retained.  
 */
#define OS_BANNER_STYLE_AUTO_VSCROLL 0x00000008
#define OS_BANNER_STYLE_AUTO_HSCROLL 0x00000010

/*
 *   Tab-based alignment is required/supported.  On creation, this is a hint
 *   to the implementation that is sometimes necessary to determine what
 *   kind of font to use in the new window, for non-HTML platforms.  If this
 *   flag is set on creation, the caller is indicating that it wants to use
 *   <TAB> tags to align text in the window.
 *   
 *   Character-mode implementations that use a single font with fixed pitch
 *   can simply ignore this.  These implementations ALWAYS have a working
 *   <TAB> capability, because the portable output formatter provides <TAB>
 *   interpretation for a fixed-pitch window.
 *   
 *   Full HTML TADS implementations can also ignore this.  HTML TADS
 *   implementations always have full <TAB> support via the HTML
 *   parser/renderer.
 *   
 *   Text-only implementations on GUI platforms (i.e., implementations that
 *   are not based on the HTML parser/renderer engine in HTML TADS, but
 *   which run on GUI platforms with proportionally-spaced text) should use
 *   this flag to determine the font to display.  If this flag is NOT set,
 *   then the caller doesn't care about <TAB>, and the implementation is
 *   free to use a proportionally-spaced font in the window if desired.
 *   
 *   When retrieving information on an existing banner, this flag indicates
 *   that <TAB> alignment is actually supported on the window.  
 */
#define OS_BANNER_STYLE_TAB_ALIGN 0x00000020

/*
 *   Use "MORE" mode in this window.  By default, a banner window should
 *   happily allow text to overflow the vertical limits of the window; the
 *   only special thing that should happen on overflow is that the window
 *   should be srolled down to show the latest text, if the auto-vscroll
 *   style is set.  With this flag, though, a banner window acts just like
 *   the main text window: when the window fills up vertically, we show a
 *   MORE prompt (using appropriate system conventions), and wait for the
 *   user to indicate that they're ready to see more text.  On most systems,
 *   the user acknowledges a MORE prompt by pressing a key or scrolling with
 *   the mouse, but it's up to the system implementor to decide what's
 *   appropriate for the system.
 *   
 *   Note that MORE mode in ANY banner window should generally override all
 *   other user input focus.  In other words, if the game in the main window
 *   would like to read a keystroke from the user, but one of the banner
 *   windows is pausing with a MORE prompt, any keyboard input should be
 *   directed to the banner paused at the MORE prompt, not to the main
 *   window; the main window should not receive any key events until the MORE
 *   prompt has been removed.
 *   
 *   This style requires the auto-vscroll style.  Implementations should
 *   assume auto-vscroll when this style is set.  This style can be ignored
 *   with text grid windows.  
 */
#define OS_BANNER_STYLE_MOREMODE  0x00000040

/*
 *   This banner is a horizontal/vertical "strut" for sizing purposes.  This
 *   means that the banner's content size is taken into account when figuring
 *   the content size of its *parent* banner.  If the banner has the same
 *   orientation as the parent, its content size is added to its parent's
 *   internal content size to determine the parent's overall content size.
 *   If the banner's orientation is orthogonal to the parent's, then the
 *   parent's overall content size is the larger of the parent's internal
 *   content size and this banner's content size.  
 */
#define OS_BANNER_STYLE_HSTRUT    0x00000080
#define OS_BANNER_STYLE_VSTRUT    0x00000100


/* 
 *   Delete a banner.  This removes the banner from the display, which
 *   requires recalculating the entire screen's layout to reallocate this
 *   banner's space to other windows.  When this routine returns, the banner
 *   handle is invalid and can no longer be used in any os_banner_xxx
 *   function calls.  
 *   
 *   If the banner has children, the children will no longer be displayed,
 *   but will remain valid in memory until deleted.  A child window's
 *   display area always comes out of its parent's space, so once the parent
 *   is gone, a child has no way to acquire any display space; resizing the
 *   child won't help, since it simply has no way to obtain any screen space
 *   once its parent has been deleted.  Even though the window's children
 *   will become invisible, their banner handles will remain valid; the
 *   caller is responsible for explicitly deleting the children even after
 *   deleting their parent.  
 */
void os_banner_delete(void *banner_handle);

/*
 *   "Orphan" a banner.  This tells the osifc implementation that the caller
 *   wishes to sever all of its ties with the banner (as part of program
 *   termination, for example), but that the calling program does not
 *   actually require that the banner's on-screen display be immediately
 *   removed.
 *   
 *   The osifc implementation can do one of two things:
 *   
 *   1.  Simply call os_banner_delete().  If the osifc implementation
 *   doesn't want to do anything extra with the banner, it can simply delete
 *   the banner, since the caller has no more use for it.
 *   
 *   2.  Take ownership of the banner.  If the osifc implementation wishes
 *   to continue displaying the final screen configuration after a program
 *   has terminated, it can simply take over the banner and leave it on the
 *   screen.  The osifc subsystem must eventually delete the banner itself
 *   if it takes this routine; for example, if the osifc subsystem allows
 *   another client program to be loaded into the same window after a
 *   previous program has terminated, it would want to delete any orphaned
 *   banners from the previous program when loading a new program.  
 */
void os_banner_orphan(void *banner_handle);

/*
 *   Banner information structure.  This is filled in by the system-specific
 *   implementation in os_banner_getinfo().  
 */
struct os_banner_info_t
{
    /* alignment */
    int align;

    /* style flags - these indicate the style flags actually in use */
    unsigned long style;

    /* 
     *   Actual on-screen size of the banner, in rows and columns.  If the
     *   banner is displayed in a proportional font or can display multiple
     *   fonts of different sizes, this is approximated by the number of "0"
     *   characters in the window's default font that will fit in the
     *   window's display area.  
     */
    int rows;
    int columns;

    /*
     *   Actual on-screen size of the banner in pixels.  This is meaningful
     *   only for full HTML interpreter; for text-only interpreters, these
     *   are always set to zero.
     *   
     *   Note that even if we're running on a GUI operating system, these
     *   aren't meaningful unless this is a full HTML interpreter.  Text-only
     *   interpreters should always set these to zero, even on GUI OS's.  
     */
    int pix_width;
    int pix_height;

    /* 
     *   OS line wrapping flag.  If this is set, the window uses OS-level
     *   line wrapping because the window uses a proportional font, so the
     *   caller does not need to (and should not) perform line breaking in
     *   text displayed in the window.
     *   
     *   Note that OS line wrapping is a PERMANENT feature of the window.
     *   Callers can note this information once and expect it to remain
     *   fixed through the window's lifetime.  
     */
    int os_line_wrap;
};
typedef struct os_banner_info_t os_banner_info_t;

/* 
 *   Get information on the banner - fills in the information structure with
 *   the banner's current settings.  Note that this should indicate the
 *   ACTUAL properties of the banner, not the requested properties; this
 *   allows callers to determine how the banner is actually displayed, which
 *   depends upon the platform's capabilities and user preferences.
 *   
 *   Returns true if the information was successfully obtained, false if
 *   not.  This can return false if the underlying OS window has already
 *   been closed by a user action, for example.  
 */
int os_banner_getinfo(void *banner_handle, os_banner_info_t *info);

/* 
 *   Get the character width/height of the banner, for layout purposes.  This
 *   gives the size of the banner in character cells.
 *   
 *   These are not meaningful when the underlying window uses a proportional
 *   font or varying fonts of different sizes.  When the size of text varies
 *   in the window, the OS layer is responsible for word-wrapping and other
 *   layout, in which case these simply return zero.
 *   
 *   Note that these routines might appear to be redundant with the 'rows'
 *   and 'columns' information returned from os_banner_getinfo(), but these
 *   have two important distinctions.  First, these routines return only the
 *   width and height information, so they can be implemented with less
 *   overhead than os_banner_getinfo(); this is important because formatters
 *   might need to call these routines frequently while formatting text.
 *   Second, these routines are not required to return an approximation for
 *   windows using proportional fonts, as os_banner_getinfo() does; these can
 *   simply return zero when a proportional font is in use.  
 */
int os_banner_get_charwidth(void *banner_handle);
int os_banner_get_charheight(void *banner_handle);

/* clear the contents of a banner */
void os_banner_clear(void *banner_handle);

/* 
 *   Display output on a banner.  Writes the output to the window on the
 *   display at the current output position.
 *   
 *   The following special characters should be recognized and handled:
 *   
 *   '\n' - newline; move output position to the start of the next line.
 *   
 *   '\r' - move output position to start of current line; subsequent text
 *   overwrites any text previously displayed on the current line.  It is
 *   permissible to delete the old text immediately on seeing the '\r',
 *   rather than waiting for additional text to actually overwrite it.
 *   
 *   All other characters should simply be displayed as ordinary printing
 *   text characters.  Note that tab characters should not be passed to this
 *   routine, but if they are, they can simply be treated as ordinary spaces
 *   if desired.  Other control characters (backspace, escape, etc) should
 *   never be passed to this routine; the implementation is free to ignore
 *   any control characters not listed above.
 *   
 *   If any text displayed here overflows the current boundaries of the
 *   window on the screen, the text MUST be "clipped" to the current window
 *   boundaries; in other words, anything this routine tries to display
 *   outside of the window's on-screen rectangle must not actually be shown
 *   on the screen.
 *   
 *   Text overflowing the display boundaries MUST also be retained in an
 *   internal buffer.  This internal buffer can be limited to the actual
 *   maximum display size of the terminal screen or application window, if
 *   desired.  It is necessary to retain clipped text, because this allows a
 *   window to be expanded to the size of its contents AFTER the contents
 *   have already been displayed.
 *   
 *   If the banner does its own line wrapping, it must indicate this via the
 *   os_line_wrap flag in the os_banner_getinfo() return data.  If the
 *   banner doesn't indicate this flag, then it must not do any line
 *   wrapping at all, even if the caller attempts to write text beyond the
 *   right edge of the window - any text overflowing the width of the window
 *   must simply be clipped.
 *   
 *   Text grid banners must ALWAYS clip - these banners should never perform
 *   any line wrapping.  
 */
void os_banner_disp(void *banner_handle, const char *txt, size_t len);

/*
 *   Set the text attributes in a banner, for subsequent text displays.
 *   'attr' is a (bitwise-OR'd) combination of OS_ATTR_xxx values. 
 */
void os_banner_set_attr(void *banner_handle, int attr);

/* 
 *   Set the text color in a banner, for subsequent text displays.  The 'fg'
 *   and 'bg' colors are given as RGB or parameterized colors; see the
 *   definition of os_color_t for details.
 *   
 *   If the underlying renderer is HTML-enabled, then this should not be
 *   used; the appropriate HTML code should simply be displayed to the
 *   banner instead.  
 */
void os_banner_set_color(void *banner_handle, os_color_t fg, os_color_t bg);

/* 
 *   Set the screen color in the banner - this is analogous to the screen
 *   color in the main text area.
 *   
 *   If the underlying renderer is HTML-enabled, then this should not be
 *   used; the HTML <BODY> tag should be used instead.  
 */
void os_banner_set_screen_color(void *banner_handle, os_color_t color);

/* flush output on a banner */
void os_banner_flush(void *banner_handle);

/*
 *   Set the banner's size.  The size has the same meaning as in
 *   os_banner_create().
 *   
 *   'is_advisory' indicates whether the sizing is required or advisory only.
 *   If this flag is false, then the size should be set as requested.  If
 *   this flag is true, it means that the caller intends to call
 *   os_banner_size_to_contents() at some point, and that the size being set
 *   now is for advisory purposes only.  Platforms that support
 *   size-to-contents may simply ignore advisory sizing requests, although
 *   they might want to ensure that they have sufficient off-screen buffer
 *   space to keep track of the requested size of display, so that the
 *   information the caller displays in preparation for calling
 *   size-to-contents will be retained.  Platforms that do not support
 *   size-to-contents should set the requested size even when 'is_advisory'
 *   is true.  
 */
void os_banner_set_size(void *banner_handle, int siz, int siz_units,
                        int is_advisory);

/* 
 *   Set the banner to the size of its current contents.  This can be used
 *   to set the banner's size after some text (or other material) has been
 *   displayed to the banner, so that the size can be set according to the
 *   banner's actual space requirements.
 *   
 *   This changes the banner's "requested size" to match the current size.
 *   Subsequent calls to os_banner_getinfo() will thus indicate a requested
 *   size according to the size set here.  
 */
void os_banner_size_to_contents(void *banner_handle);

/* 
 *   Turn HTML mode on/off in the banner window.  If the underlying renderer
 *   doesn't support HTML, these have no effect.  
 */
void os_banner_start_html(void *banner_handle);
void os_banner_end_html(void *banner_handle);

/*
 *   Set the output coordinates in a text grid window.  The grid window is
 *   arranged into character cells numbered from row zero, column zero for
 *   the upper left cell.  This function can only be used if the window was
 *   created with type OS_BANNER_TYPE_TEXTGRID; the request should simply be
 *   ignored by other window types.
 *   
 *   Moving the output position has no immediate effect on the display, and
 *   does not itself affect the "content size" for the purposes of
 *   os_banner_size_to_contents().  This simply sets the coordinates where
 *   any subsequent text is displayed.  
 */
void os_banner_goto(void *banner_handle, int row, int col);

} // End of namespace TADS
} // End of namespace Glk

#endif
