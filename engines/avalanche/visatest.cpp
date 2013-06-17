/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*#include "Crt.h"*/


char block;
word point;

array<1,2000,char> result;
word result_len;

void unskrimble()
{
    word fv;

 for( fv=1; fv <= 2000; fv ++) result[fv]=(char)((~(ord(result[fv])-fv)) % 256);
}

void visa_get_scroll(char block, word point)
{
 untyped_file indexfile,sezfile;
 word idx_offset,sez_offset;

 assign(indexfile,"avalot.idx"); assign(sezfile,"avalot.sez");

 reset(indexfile,1);
 seek(indexfile,(ord(upcase(block))-65)*2);
 blockread(indexfile,idx_offset,2);
 seek(indexfile,idx_offset+point*2);
 blockread(indexfile,sez_offset,2);
 close(indexfile);

 reset(sezfile,1);
 seek(sezfile,sez_offset);
 blockread(sezfile,result_len,2);
 blockread(sezfile,result,result_len);
 close(sezfile);
 unskrimble();
}

void access_get_scroll(char block, word point)
{
 string x;
 untyped_file f;

 str(point,x);
 x=string('S')+block+x+".RAW";
 assign(f,x);
 reset(f,1);
 result_len=filesize(f);
 blockread(f,result,result_len);
 close(f);
}

void display_it()
{
    word fv;

 for( fv=1; fv <= result_len; fv ++) output << result[fv];
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 do {
  output << NL;
  output << NL;
  output << "Block?"; input >> block >> NL;
  output << "Point?"; input >> point >> NL;

  output << "ACCESS reports (this one is always correct):" << NL;
  output << NL; 
  access_get_scroll(block,point);
  display_it();

  output << NL; output << NL;
  output << "VISA reports:" << NL;
  output << NL;
  visa_get_scroll(block,point);
  display_it();
 } while (!false);
 return EXIT_SUCCESS;
}