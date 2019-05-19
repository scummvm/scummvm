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

#include "glk/tads/os_banners.h"
#include "glk/tads/os_buffer.h"
#include "glk/tads/tads.h"
#include "glk/glk_api.h"
#include "glk/window_text_buffer.h"

namespace Glk {
namespace TADS {

struct os_banner_t;
struct banner_contents_t;
typedef os_banner_t *osbanid_t;
typedef banner_contents_t *contentid_t;

/* for tracking banner windows */
struct os_banner_t {
    uint id;                      /* unique identifier */
    uint valid;                   /* banner status */

    osbanid_t prev;                 /* previous sibling */
    osbanid_t next;                 /* next sibling */
    osbanid_t children;             /* child's descendents */
    osbanid_t parent;               /* child's parent */

    uint method;                  /* glk window method */
    uint size;                    /* glk window size */
    uint type;                    /* glk window type */
    uint status;                  /* glk status style */

    uint cheight;                 /* glk char height */
    uint cwidth;                  /* glk char width */

    uint fgcolor;                 /* foreground color */
    uint bgcolor;                 /* background color */
    uint fgcustom;                /* custom colors */
    uint bgcustom;
    uint bgtrans;

    contentid_t contents;           /* window contents */
    uint style;                   /* active Glk style value */
    uint newline;                 /* active newline */
    uint move, x, y;              /* active cursor position */

    winid_t win;                    /* glk window object */
};

/* for reprinting banner contents */
struct banner_contents_t {
    osbanid_t banner;               /* content owner */
    contentid_t next;               /* next set of contents */

    uint style;                   /* stored contents style */
    uint newline;                 /* stored newline */
    uint move, x, y;              /* stored cursor position */

    char *chars;
    uint len;
};

static osbanid_t os_banners = NULL;
static uint os_banner_count = 999;

extern winid_t mainwin;
extern winid_t statuswin;

extern uint mainfg;
extern uint mainbg;

extern uint statusfg;
extern uint statusbg;

void banner_contents_display(contentid_t contents);

/* Implementation-specific functions for managing banner windows */
/*
    os_banner_init();
    os_banner_insert();
    os_banner_styles_apply();
    os_banner_styles_reset();
    os_banners_close();
    os_banners_open();
    os_banners_redraw();
*/

osbanid_t os_banner_init() {
    osbanid_t instance;
    instance = (osbanid_t)malloc(sizeof(os_banner_t));
    if (!instance)
        return 0;

    instance->id = ++ os_banner_count;
    instance->valid = 1;

    instance->prev = 0;
    instance->next = 0;
    instance->children = 0;
    instance->parent = 0;

    instance->method = 0;
    instance->size = 0;
    instance->type = 0;
    instance->status = 0;

    instance->cheight = 0;
    instance->cwidth = 0;

    instance->contents = 0;
    instance->style = style_Normal;
    instance->newline = 0;
    instance->move = 0;
    instance->x = 0;
    instance->y = 0;

    instance->win = 0;

    return instance;
}

osbanid_t os_banner_insert(osbanid_t parent, uint operation, osbanid_t other,
                           uint method, uint size, uint type, uint status)
{
    if (!parent || !(parent->valid))
        return 0;

    if (operation == OS_BANNER_BEFORE || operation == OS_BANNER_AFTER)
        if (!other || !(other->valid) || !(other->parent == parent))
            operation = OS_BANNER_LAST;

    osbanid_t baby = os_banner_init();
    if (!baby)
        return 0;
    baby->parent = parent;

    if (!(parent->children))
    {
        parent->children = baby;
    }
    else
    {
        osbanid_t child = parent->children;

        switch (operation)
        {
            case OS_BANNER_FIRST:
                parent->children = baby;
                baby->next = child;
                child->prev = baby;
                break;

            case OS_BANNER_BEFORE:
                while (child != other && child->next)
                    child = child->next;

                if (child->prev)
                {
                    child->prev->next = baby;
                    baby->prev = child->prev;
                }
                else
                {
                    parent->children = baby;
                }

                baby->next = child;
                child->prev = baby;
                break;

            case OS_BANNER_LAST:
                while(child->next)
                    child = child->next;

                baby->prev = child;
                child->next = baby;
                break;

            case OS_BANNER_AFTER:
                while (child != other && child->next)
                    child = child->next;

                if (child->next)
                {
                    child->next->prev = baby;
                    baby->next = child->next;
                }

                baby->prev = child;
                child->next = baby;
                break;

            default: break;
        }
    }

    baby->method = method;
    baby->size = size;
    baby->type = type;
    baby->status = status;

    return baby;
}

void os_banner_styles_apply (osbanid_t banner)
{
    if (!banner || !(banner->valid))
        return;

    uint propval = banner->status ? 0 : (banner->type == wintype_TextGrid ? 0 : 1);
    uint bgcustom = banner->bgtrans ? banner->bgcolor : banner->bgcustom;

    /* font style: monospace for text grid and tab aligned buffers, else proportional */
    g_vm->glk_stylehint_set(banner->type, style_Alert, stylehint_Proportional, propval);
    g_vm->glk_stylehint_set(banner->type, style_Subheader, stylehint_Proportional, propval);
    g_vm->glk_stylehint_set(banner->type, style_Emphasized, stylehint_Proportional, propval);
    g_vm->glk_stylehint_set(banner->type, style_Normal, stylehint_Proportional, propval);
    g_vm->glk_stylehint_set(banner->type, style_User1, stylehint_Proportional, propval);
    g_vm->glk_stylehint_set(banner->type, style_User2, stylehint_Proportional, propval);

    /* foreground color: user1 reverse, user2 custom */
    g_vm->glk_stylehint_set(banner->type, style_Alert, stylehint_TextColor, banner->fgcolor);
    g_vm->glk_stylehint_set(banner->type, style_Subheader, stylehint_TextColor, banner->fgcolor);
    g_vm->glk_stylehint_set(banner->type, style_Emphasized, stylehint_TextColor, banner->fgcolor);
    g_vm->glk_stylehint_set(banner->type, style_Normal, stylehint_TextColor, banner->fgcolor);
    g_vm->glk_stylehint_set(banner->type, style_User1, stylehint_TextColor, banner->bgcolor);
    g_vm->glk_stylehint_set(banner->type, style_User2, stylehint_TextColor, banner->fgcustom);

    /* background color: user1 reverse, user2 custom */
    g_vm->glk_stylehint_set(banner->type, style_Alert, stylehint_BackColor, banner->bgcolor);
    g_vm->glk_stylehint_set(banner->type, style_Subheader, stylehint_BackColor, banner->bgcolor);
    g_vm->glk_stylehint_set(banner->type, style_Emphasized, stylehint_BackColor, banner->bgcolor);
    g_vm->glk_stylehint_set(banner->type, style_Normal, stylehint_BackColor, banner->bgcolor);
    g_vm->glk_stylehint_set(banner->type, style_User1, stylehint_BackColor, banner->fgcolor);
    g_vm->glk_stylehint_set(banner->type, style_User2, stylehint_BackColor, bgcustom);

}

void os_banner_styles_reset (void)
{
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Alert, stylehint_Proportional);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Subheader, stylehint_Proportional);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Emphasized, stylehint_Proportional);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Normal, stylehint_Proportional);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_User1, stylehint_Proportional);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_User2, stylehint_Proportional);

    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Alert, stylehint_TextColor);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Subheader, stylehint_TextColor);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Emphasized, stylehint_TextColor);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Normal, stylehint_TextColor);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_User1, stylehint_TextColor);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_User2, stylehint_TextColor);

    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Alert, stylehint_BackColor);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Subheader, stylehint_BackColor);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Emphasized, stylehint_BackColor);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_Normal, stylehint_BackColor);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_User1, stylehint_BackColor);
    g_vm->glk_stylehint_clear(wintype_AllTypes, style_User2, stylehint_BackColor);

#ifdef GARGLK
    /* reset our default colors with a superfluous hint */
    g_vm->glk_stylehint_set(wintype_AllTypes, style_Normal, stylehint_TextColor, mainfg);
    g_vm->glk_stylehint_set(wintype_AllTypes, style_Normal, stylehint_BackColor, mainbg);
#endif /* GARGLK */
}

void os_banners_close(osbanid_t banner)
{
    if (!banner)
        return;

    os_banners_close(banner->children);
    os_banners_close(banner->next);

    if (banner->win && (banner->win != mainwin))
    {
        g_vm->glk_window_close(banner->win, 0);
        banner->win = 0;
    }
}

void os_banners_open(osbanid_t banner)
{
    if (!banner)
        return;

    if (banner->valid)
    {
        if (banner->size && banner->parent && banner->parent->win)
        {
            os_banner_styles_apply(banner);
            banner->win = g_vm->glk_window_open(banner->parent->win, banner->method,
                                          banner->size, banner->type, banner->id);
            banner_contents_display(banner->contents);
        }
        os_banners_open(banner->children);
    }

    os_banners_open(banner->next);
}

void os_banners_redraw()
{
    if (!os_banners)
        return;

    os_banners_close(os_banners);
    os_banners_open(os_banners);
    os_banner_styles_reset();
}

/* Implementation-specific functions for managing banner contents */
/*
    banner_contents_init();
    banner_contents_insert();
    banner_contents_display();
    banner_contents_clear();
*/

contentid_t banner_contents_init(void)
{
    contentid_t instance;
    instance = (contentid_t)malloc(sizeof(banner_contents_t));
    if (!instance)
        return 0;

    instance->banner = 0;
    instance->next = 0;

    instance->style = style_Normal;
    instance->newline = 0;
    instance->move = 0;
    instance->x = 0;
    instance->y = 0;

    instance->chars = 0;
    instance->len = 0;

    return instance;
}

void banner_contents_insert(contentid_t contents, const char *txt, size_t len)
{
    if (!contents)
        return;

    contents->chars = (char *)malloc(sizeof(char) * (len + 1));
    if (!(contents->chars))
        return;

    memcpy(contents->chars, txt, len);

    contents->chars[len] = '\0';
    contents->len = len;
}

void banner_contents_display(contentid_t contents)
{
    if (!contents || !(contents->banner))
        return;

    winid_t win = contents->banner->win;
    uint len = contents->len;

    g_vm->glk_set_window(win);

    if (contents->newline)
    {
        char ch = '\n';
        os_put_buffer(&ch, 1);
    }
    
    if (len && (contents->chars[len-1] == '\n'))
    {
        len --;
        contents->banner->newline = 1;
    }
    else
    {
        contents->banner->newline = 0;
    }

    if (contents->move)
    {
        g_vm->glk_window_move_cursor(win, contents->x, contents->y);
        contents->banner->move = 0;
        contents->banner->x = 0;
        contents->banner->y = 0;
    }

    g_vm->glk_set_style(contents->style);
    os_put_buffer(contents->chars, len);
    g_vm->glk_set_window(mainwin);
    banner_contents_display(contents->next);
}

void banner_contents_clear(contentid_t contents)
{
    if (!contents)
        return;

    banner_contents_clear(contents->next);

    if (contents->chars)
        free(contents->chars);

    free(contents);
}

/* Banner API functions */

void *os_banner_create(void *parent, int where, void *other, int wintype,
                       int align, int siz, int siz_units,
                       unsigned long style)
{
    osbanid_t gparent = (osbanid_t)parent;
    osbanid_t gbanner;
    uint gwinmeth = 0;
    uint gwinsize = siz;
    uint gwintype = 0;
    uint gstatus = (style & OS_BANNER_STYLE_TAB_ALIGN);

    if (gparent && !(gparent->valid))
        return 0;

    if (!os_banners)
    {
        os_banners = os_banner_init();
        if (!os_banners)
            return 0;
        os_banners->win = mainwin;
    }

    if (!gparent)
        gparent = os_banners;

    switch (wintype)
    {
        case OS_BANNER_TYPE_TEXT: gwintype = wintype_TextBuffer; break;
        case OS_BANNER_TYPE_TEXTGRID: gwintype = wintype_TextGrid; break;
        default: gwintype = wintype_TextGrid; break;
    }

    switch (align)
    {
        case OS_BANNER_ALIGN_TOP: gwinmeth = winmethod_Above; break;
        case OS_BANNER_ALIGN_BOTTOM: gwinmeth = winmethod_Below; break;
        case OS_BANNER_ALIGN_LEFT: gwinmeth = winmethod_Left; break;
        case OS_BANNER_ALIGN_RIGHT: gwinmeth = winmethod_Right; break;
        default: gwinmeth = winmethod_Above; break;
    }

    switch (siz_units)
    {
        case OS_BANNER_SIZE_PCT: gwinmeth |= winmethod_Proportional; break;
        case OS_BANNER_SIZE_ABS: gwinmeth |= winmethod_Fixed; break;
        default: gwinmeth |= winmethod_Fixed; break;
    }

    gbanner = os_banner_insert(gparent, where, (osbanid_t)other, gwinmeth, gwinsize, gwintype, gstatus);

    if (gbanner)
    {
        gbanner->fgcolor = gstatus ? statusbg : mainfg;
        gbanner->bgcolor = gstatus ? statusfg : mainbg;
        gbanner->fgcustom = gbanner->fgcolor;
        gbanner->bgcustom = gbanner->bgcolor;
        gbanner->bgtrans = 1;
    }

    os_banners_redraw();

    return gbanner;
}

void os_banner_set_size(void *banner_handle, int siz, int siz_units, int is_advisory)
{
    osbanid_t banner = (osbanid_t)banner_handle;

    if (!banner || !banner->valid)
        return;

    uint gwinsize = siz;
    uint gwinmeth = 0;

    gwinmeth = banner->method &
        (winmethod_Above | winmethod_Below |
         winmethod_Left  | winmethod_Right);

    switch (siz_units)
    {
        case OS_BANNER_SIZE_PCT: gwinmeth |= winmethod_Proportional; break;
        case OS_BANNER_SIZE_ABS: gwinmeth |= winmethod_Fixed; break;
        default: gwinmeth |= winmethod_Fixed; break;
    }

    banner->method = gwinmeth;
    banner->size = gwinsize;

    os_banners_redraw();
}

void os_banner_size_to_contents(void *banner_handle)
{
    osbanid_t banner = (osbanid_t)banner_handle;

    if (!banner || !banner->valid || !banner->win)
        return;

#ifdef GARGLK
    if (banner->type == wintype_TextBuffer)
    {
		TextBufferWindow *win = dynamic_cast<TextBufferWindow *>(banner->win);
		assert(win);
        int size = win->_scrollMax;
        if (win->_numChars)
            size++;
        os_banner_set_size(banner, size, OS_BANNER_SIZE_ABS, 0);
    }
#endif /* GARGLK */
}

void os_banner_delete(void *banner_handle)
{
    osbanid_t banner = (osbanid_t)banner_handle;
    if (!banner || !(banner->valid))
        return;

    banner->valid = 0;
    os_banners_redraw();

    if (banner->parent && banner->parent->children == banner)
        banner->parent->children = banner->next;

    if (banner->next)
        banner->next->prev = banner->prev;

    if (banner->prev)
        banner->prev->next = banner->next;

    banner_contents_clear(banner->contents);

    free(banner);
}

void os_banner_orphan(void *banner_handle)
{
    os_banner_delete(banner_handle);
}

int os_banner_getinfo(void *banner_handle, os_banner_info_t *info)
{
    osbanid_t banner = (osbanid_t)banner_handle;
    if (!banner || !banner->valid || !banner->win)
        return 1;

    //winid_t win = banner->win;
    uint gwintype = banner->type;
    uint gwinmeth = banner->method;
    uint gstyletab = banner->status;

    if (gwinmeth & winmethod_Above)
        info->align = OS_BANNER_ALIGN_TOP;
    if (gwinmeth & winmethod_Below)
        info->align = OS_BANNER_ALIGN_BOTTOM;
    if (gwinmeth & winmethod_Left)
        info->align = OS_BANNER_ALIGN_LEFT;
    if (gwinmeth & winmethod_Right)
        info->align = OS_BANNER_ALIGN_RIGHT;

    info->style = gstyletab ? OS_BANNER_STYLE_TAB_ALIGN : 0;

    g_vm->glk_window_get_size(banner->win, &(banner->cwidth), &(banner->cheight));

    info->rows = banner->cheight;
    info->columns = banner->cwidth;

    info->pix_width = 0;
    info->pix_height = 0;

    info->os_line_wrap = gstyletab ? 0 : (gwintype == wintype_TextBuffer);

    return 1;
}

int os_banner_get_charwidth(void *banner_handle)
{
    osbanid_t banner = (osbanid_t)banner_handle;
    if (!banner || !banner->valid || !banner->win)
        return 0;

    g_vm->glk_window_get_size(banner->win, &(banner->cwidth), &(banner->cheight));

    return banner->cwidth;
}

int os_banner_get_charheight(void *banner_handle)
{
    osbanid_t banner = (osbanid_t)banner_handle;
    if (!banner || !banner->valid || !banner->win)
        return 0;

    g_vm->glk_window_get_size(banner->win, &(banner->cwidth), &(banner->cheight));

    return banner->cheight;
}

void os_banner_clear(void *banner_handle)
{
    osbanid_t banner = (osbanid_t)banner_handle;
    if (!banner || !banner->valid)
        return;

    if (banner->win)
    {
        winid_t win = banner->win;
        g_vm->glk_window_clear(win);
    }

    banner_contents_clear(banner->contents);
    banner->contents = 0;
    banner->newline = 0;
    banner->move = 0;
    banner->x = 0;
    banner->y = 0;
}

void os_banner_disp(void *banner_handle, const char *txt, size_t len)
{
    osbanid_t banner = (osbanid_t)banner_handle;
    if (!banner || !banner->valid || !banner->win)
        return;

    contentid_t update = banner_contents_init();
    if (!update)
        return;
    update->banner = banner;

    if (!(banner->contents))
    {
        banner->contents = update;
    }
    else
    {
        contentid_t contents = banner->contents;
        while (contents->next)
            contents = contents->next;
        contents->next = update;
    }

    update->style = banner->style;
    update->newline = banner->newline;
    update->move = banner->move;
    update->x = banner->x;
    update->y = banner->y;

    banner_contents_insert(update, txt, len);
    banner_contents_display(update);
}

void os_banner_goto(void *banner_handle, int row, int col)
{
    osbanid_t banner = (osbanid_t)banner_handle;
    if (!banner || !banner->valid || !banner->win)
        return;

    if (banner->type == wintype_TextGrid)
    {
        banner->move = 1;
        banner->x = col;
        banner->y = row;
    }
}

void os_banner_set_attr(void *banner_handle, int attr)
{
    osbanid_t banner = (osbanid_t)banner_handle;
    if (!banner || !banner->valid)
        return;

    if (attr & OS_ATTR_BOLD && attr & OS_ATTR_ITALIC)
        banner->style = style_Alert;
    else if (attr & OS_ATTR_BOLD)
        banner->style = style_Subheader;
    else if (attr & OS_ATTR_ITALIC)
        banner->style = style_Emphasized;
    else
        banner->style = style_Normal;
}

void os_banner_set_color(void *banner_handle, os_color_t fg, os_color_t bg)
{
    osbanid_t banner = (osbanid_t)banner_handle;
    if (!banner || !banner->valid)
        return;

    uint reversed = 0;
    uint normal = 0;
    uint transparent = 0;

    /* evaluate parameters */

    if (os_color_is_param(fg))
    {
        switch(fg)
        {
            case OS_COLOR_P_TEXTBG:
            case OS_COLOR_P_STATUSBG:
                reversed = 1;
                break;

            case OS_COLOR_P_TEXT:
            case OS_COLOR_P_STATUSLINE:
            case OS_COLOR_P_INPUT:
                normal = 1;
                break;

            default:
                break;
        }
    }

    if (os_color_is_param(bg))
    {
        switch (bg)
        {
            case OS_COLOR_P_TRANSPARENT:
                transparent = 1;
                break;

            default:
                break;
        }
    }

    /* choose a style */

    if (normal && transparent)
        banner->style = style_Normal;
    else if (reversed)
        banner->style = style_User1;
    else
        banner->style = style_User2;

    /* process our custom color */

    if (banner->style == style_User2)
    {
        /* store current settings */
        uint oldfg = banner->fgcustom;
        uint oldbg = banner->bgcustom;
        uint oldtr = banner->bgtrans;

        /* reset custom color parameters */
        banner->fgcustom = banner->fgcolor;
        banner->bgcustom = banner->bgcolor;
        banner->bgtrans = 1;

        if (!normal)
            banner->fgcustom = fg;

        if (!transparent)
        {
            banner->bgcustom = bg;
            banner->bgtrans = 0;
        }

        if (!(banner->fgcustom == oldfg
            && banner->bgcustom == oldbg
            && banner->bgtrans == oldtr))
            os_banners_redraw();
    }
}

void os_banner_set_screen_color(void *banner_handle, os_color_t color) {
    osbanid_t banner = (osbanid_t)banner_handle;
    if (!banner || !banner->valid)
        return;

    if (!(os_color_is_param(color)))
        banner->bgcolor = color;

    os_banners_redraw();
}

void os_banner_flush(void *banner_handle) {}
void os_banner_start_html(void *banner_handle) {}
void os_banner_end_html(void *banner_handle) {}

} // End of namespace TADS
} // End of namespace Glk
