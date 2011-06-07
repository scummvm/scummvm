#include	"general.h"
#include	<stdio.h>




void main (void)
{
  printf("This is%s the VGA video board\n", (IsVga()) ? "" : " not");
}