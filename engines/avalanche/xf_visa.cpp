/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */




/*#include "Crt.h"*/
/*#include "Dos.h"*/

/*$R+*/

const varying_string<9> used = "DNPQSTUXZ";

const varying_string<12> header = string("Avalot Sez:")+'\32';

untyped_file sez,infile;
searchrec s;

matrix<'A','Z',0,99,word> positions;
array<'A','Z',word> maxlen;

matrix<1,50,0,255,word> speak_positions;
array<1,50,word> speak_maxlen;

array<1,2000,char> data;
word data_length;

byte fv;

byte numeric_bit()
{
    varying_string<5> x; integer e; byte result;

 byte numeric_bit_result;
 x=copy(s.name,3,pos(".",s.name)-3);
 val(x,result,e);
 if (e!=0) 
 {
  output << "NUMERIC ERROR: " << s.name << '/' << x << NL;
  exit(255);
 }
 numeric_bit_result=result;
 return numeric_bit_result;
}

byte speak_left()
{
    string x; integer e; byte result;

 byte speak_left_result;
 x=copy(s.name,3,pos(".",s.name)-3);
 x=copy(x,1,pos("-",x)-1);
 val(x,result,e);
 if (e!=0) 
 {
  output << "NUMERIC ERROR (left): " << s.name << '/' << x << NL;
  exit(255);
 }
 speak_left_result=result;
 return speak_left_result;
}

byte speak_right()
{
    string x; integer e; byte result;

 byte speak_right_result;
 x=copy(s.name,3,pos(".",s.name)-3);
 x=copy(x,pos("-",x)+1,255);
 val(x,result,e);
 if (e!=0) 
 {
  output << "NUMERIC ERROR (right): " << s.name << '/' << x << NL;
  exit(255);
 }
 speak_right_result=result;
 return speak_right_result;
}

void write_out()
{
 array<'A','Z',word> points;
 array<1,50,word> speak_points;
 untyped_file outf;
 byte fv;

 fillchar(points,sizeof(points),'\0');
 fillchar(speak_points,sizeof(speak_points),'\0');

 assign(outf,"v:avalot.idx");
 rewrite(outf,1);
 blockwrite(outf,points,sizeof(points));

 for( fv=1; fv <= length(used); fv ++)
 {
  points[used[fv-1]]=filepos(outf);
  blockwrite(outf,positions[used[fv-1]],maxlen[used[fv-1]]*2+2);
 }

 seek(outf,0);
 blockwrite(outf,points,sizeof(points));

 close(outf);

 /* --- now the speech records --- */

 assign(outf,"v:converse.avd");
 rewrite(outf,1);
 blockwrite(outf,speak_points,sizeof(speak_points));

 for( fv=1; fv <= 15; fv ++)
 {
  speak_points[fv]=filepos(outf);

  blockwrite(outf,speak_positions[fv],speak_maxlen[fv]*2+2);
 }

 seek(outf,0);
 blockwrite(outf,speak_points,sizeof(speak_points));

 close(outf);
}

void skrimble()
{
    word fv;

 for( fv=1; fv <= 2000; fv ++) data[fv]=(char)((~(ord(data[fv]))+fv) % 256);
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 fillchar(positions,sizeof(positions),'\0');
 fillchar(maxlen,sizeof(maxlen),'\0');

 clrscr;

 assign(sez,"v:avalot.sez");
 rewrite(sez,1);
 blockwrite(sez,header[1],12);

 findfirst("s*.raw",anyfile,s);
 while (doserror==0) 
 {
  assign(infile,s.name);
  reset(infile,1);
  blockread(infile,data,2000,data_length);
  close(infile);

  clrscr;
  if (pos("-",s.name)==0) 
  {     /* Not a speech record. */
   output << s.name << format(numeric_bit(),10) << NL;

   positions[s.name[2]][numeric_bit()]=filepos(sez);
   if (maxlen[s.name[2]]<(unsigned char)numeric_bit())  maxlen[s.name[2]]=numeric_bit();

  } else
  {     /* A speech record. */
   output << s.name << format(speak_left(),10) << format(speak_right(),10) << " SR" << NL;

   speak_positions[speak_left()][speak_right()]=filepos(sez);
   if (speak_maxlen[speak_left()]<(unsigned char)speak_right()) 
       speak_maxlen[speak_left()]=speak_right();
  }

  skrimble();

  blockwrite(sez,data_length,2);
  blockwrite(sez,data,data_length);

  findnext(s);
  clreol;
 }

 close(sez);

 write_out(); 
 return EXIT_SUCCESS;
}