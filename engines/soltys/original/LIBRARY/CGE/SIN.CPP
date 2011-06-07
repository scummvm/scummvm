#include	<math.h>
#include	<stdio.h>

#define	PI	((double)3.1415926)




int main (void)
{
  double x;
  int i;
  printf("int Sinus[] = {");
  for (i = 0; i < 360; i ++)
    {
      if (i % 10 == 0) printf("\n");
      printf("%6.0lf,", 32767*sin(((PI*2)*i)/360));
    }
  printf(" };\n");
  return 0;
}