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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

//
// simple sscanf replacement to match scummvm usage patterns
//

bool scanInt(const char** in, va_list* ap) {
  int* arg = va_arg(*ap, int*);
  char* end;
  long n = strtol(*in, &end, 0);

  if (end == *in) {
    return true;
  }

  *in = end;
  *arg = (int) n;
  return false;
}

bool scanUnsigned(const char** in, va_list* ap, int max) {
  unsigned* arg = va_arg(*ap, unsigned*);
  unsigned n = 0;
  int width = 0;
  while (isdigit(**in)) {
    n = (n * 10) + (**in - '0');
    (*in)++;
    width++;
  }

  *arg = n;
  return (width > max);
}

bool scanHex(const char** in, va_list* ap) {
  unsigned* arg = va_arg(*ap, unsigned*);
  char* end;
  long n = strtol(*in, &end, 16);
  if (end == *in) {
    return true;
  }

  *in = end;
  *arg = (unsigned) n;
  return false;
}

bool scanString(const char** in, va_list* ap) {
  char* arg = va_arg(*ap, char*);
  while (**in && **in != ' ' && **in != '\n' && **in != '\t') {
    *arg = **in;
    arg++;
    (*in)++;
  }
  *arg = '\0';
  (*in)++;
  return false;
}

bool scanUntil(const char** in, char c_end, va_list* ap) {
  char* arg = va_arg(*ap, char*);
  while (**in && **in != c_end) {
    *arg = **in;
    *arg++;
    (*in)++;
  }
  *arg = 0;
  (*in)++;
  return false;
}

int simple_sscanf(const char* input, const char* format, ...) {
  va_list ap;
  int result = 0;
  int maxWidth = -1;
  const char* next = input;

  va_start(ap, format);

  while (*format) {
    if (*format == '%') {
      format++;
      int max = 0;
      while (isdigit(*format)) {
        max = (max * 10) + (*format - '0');
        format++;
      }

      bool err = false;
      switch (*format++) {
      case 'd':
        err = scanInt(&next, &ap);
        break;
      case 'x':
        err = scanHex(&next, &ap);
        break;
      case 's':
        err = scanString(&next, &ap);
        break;
      case 'u':
        err = scanUnsigned(&next, &ap, max);
        break;
      case '[':
        // assume %[^c]
        if ('^' != *format) {
          err = true;
        }
        else {
          format++;
          if (*format && *(format+1) == ']') {
            err = scanUntil(&next, *format, &ap);
            format += 2;
          }
          else {
            err = true;
          }
        }
        break;
      default:
        err = true;
        break;
      }

      if (err) {
        break;
      }
      else {
        result++;
      }
    }
    else if (*format++ != *next++) {
      // match input
      break;
    }
  }
   
  va_end(ap);
  return result;
}

#if defined(TEST)
int main(int argc, char *pArgv[]) {
  int x,y,h;
  char buffer[100];
  unsigned u;
  strcpy(buffer, "hello");

  //  strcpy(buffer, "in the buffer something");
  if (simple_sscanf("CAT 123x10 0x100 FONT large 1 enough\n 123456.AUD", 
                    "CAT %dx%d %x FONT %[^\n] %08u.AUD", 
                    &x, &y, &h, &buffer, &u) != 5) {
    printf("Failed\n");
  }
  else {
    printf("Success %d %d %d %s %d\n", x, y, h, buffer, u);
  }
  return 0;
}
#endif
