#include "ptoc.h"


/*#include "Crt.h"*/
/*#include "Dos.h"*/


matrix<1,7,1,3,byte> cols;
byte fv;

void adjust()
{
 registers r;
;
 {;
  ax=0x1012;
  bx=1;
  cx=2;
  es=seg(cols);
  dx=ofs(cols);

 }

 intr(0x10,r);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 textattr=0;
 clrscr;

 fillchar(cols,sizeof(cols),'\0');
 adjust();

 gotoxy(29,10); textattr=1; output << "Thorsoft of Letchworth";
 gotoxy(36,12); textattr=2; output << "presents";

 for( fv=1; fv <= 77; fv ++)
 {;
  delay(77);
  if (fv<64)  fillchar(cols[1],3,chr(fv));
  if (fv>14)  fillchar(cols[2],3,chr(fv-14));
  adjust();
 }

 delay(100);

 for( fv=63; fv >= 1; fv --)
 {;
  fillchar(cols,sizeof(cols),chr(fv));
  delay(37);
  adjust();
 }

return EXIT_SUCCESS;
}