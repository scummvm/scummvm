/***************************************************************************
 sci_dos.h Copyright (C) 1999 Rink Springer


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

    Rink Springer [rink@springer.cx]

***************************************************************************/

#ifndef _SCI_DOS_H_
#define _SCI_DOS_H_

#include <stdarg.h>

#ifndef HAVE_DIRENT_H
#define HAVE_DIRENT_H
#endif

#define TRUE (!FALSE)
#define FALSE (0)

typedef signed char gint8;
typedef signed short gint16;
typedef signed long gint32;

typedef unsigned char guint8;
typedef unsigned short guint16;
typedef unsigned long guint32;

typedef char gchar;
typedef unsigned char guchar;
typedef int gint;
typedef unsigned int guint;
typedef long glong;
typedef unsigned long gulong;

typedef gint gboolean;

typedef void* gpointer;

#define g_new0(type, count) ((type*)g_malloc0((unsigned)sizeof(type) * (count)))
#define g_new(type, count) ((type*)sci_malloc((unsigned)sizeof(type) * (count)))

#define g_malloc(x) sci_malloc(x)
#define g_free(x) sci_free(x)
#define g_realloc(x,y) sci_realloc(x,y)

extern gpointer g_malloc0(guint32 size);

/* Name of package */
#define PACKAGE "freesci"

/* Version number of package */
#define VERSION "0.3.0"

/* directory separator */
#define G_DIR_SEPARATOR_S "/"

#define SSIZE_MAX 1024

#define g_get_current_time(x) gettimeofday(x,NULL)

#define g_strcasecmp(x,y) strcasecmp(x,y)
#define g_strncasecmp(x,y,z) strncasecmp(x,y,z)

extern gint g_vsnprintf(gchar*,gulong,gchar const*,va_list);
extern gpointer g_memdup (gpointer mem, guint byte_size);

#define DGFX_KEYUP    72
#define DGFX_KEYDOWN  80
#define DGFX_KEYLEFT  75
#define DGFX_KEYRIGHT 77
#define DGFX_KEYSTOP  76         /* numberic 5 */

#define DGFX_KEY_ESCAPE 1        /* escape */
#define DGFX_KEY_1     2         /* 1 */
#define DGFX_KEY_2     3         /* 2 */
#define DGFX_KEY_3     4         /* 3 */
#define DGFX_KEY_4     5         /* 4 */
#define DGFX_KEY_5     6         /* 5 */
#define DGFX_KEY_6     7         /* 6 */
#define DGFX_KEY_7     8         /* 7 */
#define DGFX_KEY_8     9         /* 8 */
#define DGFX_KEY_9     10        /* 9 */
#define DGFX_KEY_0     11        /* 0 */
#define DGFX_KEY_MINUS 12        /* -/_ */
#define DGFX_KEY_EQUAL 13        /* =/+ */
#define DGFX_KEY_BSPACE 14       /* backspace */
#define DGFX_KEY_TAB   15        /* enter */
#define DGFX_KEY_Q     16        /* q */
#define DGFX_KEY_W     17        /* w */
#define DGFX_KEY_E     18        /* e */
#define DGFX_KEY_R     19        /* r */
#define DGFX_KEY_T     20        /* t */
#define DGFX_KEY_Y     21        /* y */
#define DGFX_KEY_U     22        /* u */
#define DGFX_KEY_I     23        /* i */
#define DGFX_KEY_O     24        /* o */
#define DGFX_KEY_P     25        /* p */
#define DGFX_KEY_LBRACKET 26     /* [/{ */
#define DGFX_KEY_RBRACKET 27     /* ]/} */
#define DGFX_KEY_ENTER 28        /* enter */
#define DGFX_KEY_CTRL  29        /* control */
#define DGFX_KEY_A     30        /* a */
#define DGFX_KEY_S     31        /* s */
#define DGFX_KEY_D     32        /* d */
#define DGFX_KEY_F     33        /* f */
#define DGFX_KEY_G     34        /* g */
#define DGFX_KEY_H     35        /* h */
#define DGFX_KEY_J     36        /* j */
#define DGFX_KEY_K     37        /* k */
#define DGFX_KEY_L     38        /* l */
#define DGFX_KEY_COLON 39        /* ;/: */
#define DGFX_KEY_TILDE 41        /* tilde */
#define DGFX_KEY_LSHIFT 42       /* left shift */
#define DGFX_KEY_Z     44        /* z */
#define DGFX_KEY_X     45        /* x */
#define DGFX_KEY_C     46        /* c */
#define DGFX_KEY_V     47        /* v */
#define DGFX_KEY_B     48        /* b */
#define DGFX_KEY_N     49        /* n */
#define DGFX_KEY_M     50        /* m */
#define DGFX_KEY_COMMA 51        /* ,/< */
#define DGFX_KEY_DOT   52        /* ./> */
#define DGFX_KEY_SLASH 53        /* / / ? */
#define DGFX_KEY_RSHIFT 54       /* right shift */
#define DGFX_KEY_ALT   56        /* alt key */
#define DGFX_KEY_SPACE 57        /* space bar */
#define DGFX_KEY_F1    59        /* f1 */
#define DGFX_KEY_F2    60        /* f2 */
#define DGFX_KEY_F3    61        /* f3 */
#define DGFX_KEY_F4    62        /* f4 */
#define DGFX_KEY_F5    63        /* f5 */
#define DGFX_KEY_F6    64        /* f6 */
#define DGFX_KEY_F7    65        /* f7 */
#define DGFX_KEY_F8    66        /* f8 */
#define DGFX_KEY_F9    67        /* f9 */
#define DGFX_KEY_F10   68        /* f10 */

#define DGFX_KEY_HOME  71        /* home */
#define DGFX_KEY_PAGEUP 73       /* page up */
#define DGFX_KEY_END    79       /* end */
#define DGFX_KEY_PAGEDOWN 81     /* page down */
#define DGFX_KEY_INSERT 82       /* insert */
#define DGFX_KEY_DELETE 83       /* delete */

#define DGFX_KEY_KPSUB  74       /* keypad - */
#define DGFX_KEY_KPADD  78       /* keypad + */

#define DGFX_KEY_F11   87        /* f11 */
#define DGFX_KEY_F12   88        /* f12 (emergency exit) */

#endif /* _SCI_DOS_H_ */
