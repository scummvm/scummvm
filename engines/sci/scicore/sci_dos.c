/***************************************************************************
 sci_dos.c Copyright (C) 1999 Rink Springer

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

#include <stdlib.h>
#include <sci_dos.h>
#include <string.h>

#define G_VA_COPY(ap1, ap2)     ((ap1) = (ap2))

gpointer
malloc0(guint32 size) {
    char* ptr;

    /* allocate the buffer, return NULL if no buffer */
    if((ptr= sci_malloc(size))==NULL) return NULL;

    /* clear it to zeros */
    memset(ptr,0,size);

    /* return the pointer */
    return ptr;
}

guint
g_printf_string_upper_bound (const gchar* format,
                 va_list      args)
{
  guint len = 1;

  while (*format)
    {
      gboolean long_int = FALSE;
      gboolean extra_long = FALSE;
      gchar c;

      c = *format++;

      if (c == '%')
    {
      gboolean done = FALSE;

      while (*format && !done)
        {
          switch (*format++)
        {
          gchar *string_arg;

        case '*':
          len += va_arg (args, int);
          break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          /* add specified format length, since it might exceed the
           * size we assume it to have.
           */
          format -= 1;
          len += strtol (format, (char**) &format, 10);
          break;
        case 'h':
          /* ignore short int flag, since all args have at least the
           * same size as an int
           */
          break;
        case 'l':
          if (long_int)
            extra_long = TRUE; /* linux specific */
          else
            long_int = TRUE;
          break;
        case 'q':
        case 'L':
          long_int = TRUE;
          extra_long = TRUE;
          break;
        case 's':
          string_arg = va_arg (args, char *);
          if (string_arg)
            len += strlen (string_arg);
          else
            {
              /* add enough padding to hold "(null)" identifier */
              len += 16;
            }
          done = TRUE;
          break;
        case 'd':
        case 'i':
        case 'o':
        case 'u':
        case 'x':
        case 'X':
            {
              if (long_int)
            (void) va_arg (args, long);
              else
            (void) va_arg (args, int);
            }
          len += extra_long ? 64 : 32;
          done = TRUE;
          break;
        case 'D':
        case 'O':
        case 'U':
          (void) va_arg (args, long);
          len += 32;
          done = TRUE;
          break;
        case 'e':
        case 'E':
        case 'f':
        case 'g':
            (void) va_arg (args, double);
          len += extra_long ? 64 : 32;
          done = TRUE;
          break;
        case 'c':
          (void) va_arg (args, int);
          len += 1;
          done = TRUE;
          break;
        case 'p':
        case 'n':
          (void) va_arg (args, void*);
          len += 32;
          done = TRUE;
          break;
        case '%':
          len += 1;
          done = TRUE;
          break;
        default:
          /* ignore unknow/invalid flags */
          break;
        }
        }
    }
      else
    len += 1;
    }

return len;
}

gchar*
g_strdup_vprintf (const gchar *format,
                  va_list      args1) {
  gchar *buffer;
  va_list args2;

  G_VA_COPY (args2, args1);

  buffer = g_new (gchar, g_printf_string_upper_bound (format, args1));

  vsprintf (buffer, format, args2);
  va_end (args2);

  return buffer;
}

gint
g_vsnprintf (gchar       *str,
             gulong       n,
             gchar const *fmt,
             va_list      args) {
  gchar* printed;


  printed = g_strdup_vprintf (fmt, args);
  strncpy (str, printed, n);
  str[n-1] = '\0';

  free (printed);

  return strlen (str);
}

gpointer
g_memdup (gpointer mem, guint byte_size) {
  gpointer new_mem;

  if (mem) {
    new_mem = sci_malloc (byte_size);
    memcpy (new_mem, mem, byte_size);
  } else {
    new_mem = NULL;
  }

  return new_mem;
}
