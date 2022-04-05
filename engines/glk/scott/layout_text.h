#ifndef GLK_SCOTT_LAYOUTTEXT
#define GLK_SCOTT_LAYOUTTEXT

namespace Glk {
namespace Scott {

/* Breaks a null-terminated string up by inserting newlines,*/
/* moving words down to the next line when reaching the end of the line */
char *lineBreakText(char *source, int columns, int *rows, int *length);

} // End of namespace Scott
} // End of namespace Glk
#endif
