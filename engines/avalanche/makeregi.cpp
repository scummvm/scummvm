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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* regname's name starts at $7D1 (2001). */

namespace Avalanche {

const array<1,56,char> padding = 
"For all the Etruscan armies were ranged beneath his eye";

text txi,txo;
string x,y;
word fv;
file<byte> f;
byte sum,n;
string name,number;

string chkname,chknum,regname,regnum;


   char decode1(char c)
   {
       byte b;

     char decode1_result;
     b=ord(c)-32;
     decode1_result=chr(( (b & 0xf) << 3) + ((cardinal)(b & 0x70) >> 4));
     return decode1_result;
   }

   char encode1(char c)
   {
       byte b;

     char encode1_result;
     b=ord(c);
     b=( (cardinal)(b & 0x78) >> 3) + ((b & 0x7) << 4);
     encode1_result=chr(b+32);
     return encode1_result;
   }

   char encode2(char c)
   {
     char encode2_result;
     encode2_result=chr((((ord(c) & 0xf) << 2)+0x43));
     return encode2_result;
   }

   string enc1(string x)
   {
       string y; byte fv;

     string enc1_result;
     y=x; for( fv=1; fv <= length(y); fv ++) y[fv]=encode1(y[fv]);
     enc1_result=y;
     return enc1_result;
   }

   string enc2(string x)
   {
       string y; byte fv;

     string enc2_result;
     y=x; for( fv=1; fv <= length(y); fv ++) y[fv]=encode2(y[fv]);
     enc2_result=y;
     return enc2_result;
   }

   boolean checker(string proper,string check)
   {
       byte fv; boolean ok;

     boolean checker_result;
     ok=true;
     for( fv=1; fv <= length(proper); fv ++)
       if ((ord(proper[fv]) & 0xf)!=((cardinal)(ord(check[fv])-0x43) >> 2))
              ok=false;

     checker_result=ok;
     return checker_result;
   }

  void unscramble()
  {
      byte namelen,numlen;


    namelen=107-ord(x[1]); numlen=107-ord(x[2]);

    regname=copy(x,3,namelen);
    regnum=copy(x,4+namelen,numlen);
    chkname=copy(x,4+namelen+numlen,namelen);
    chknum=copy(x,4+namelen+numlen+namelen,numlen);

    for( fv=1; fv <= namelen; fv ++) regname[fv]=decode1(regname[fv]);
    for( fv=1; fv <= numlen; fv ++) regnum[fv]=decode1(regnum[fv]);

    if ((! checker(regname,chkname)) || (! checker(regnum,chknum)))
          {
       output << "CHECK ERROR: " << regname << '/' << chkname << ';' << regnum << '/' << chknum << '.' << NL;
       exit(0);
     } else output << "--- Passed both checks. ---" << NL;
  }

int main(int argc, const char* argv[])
{

 pio_initialize(argc, argv);
 output << "Name? "; input >> name >> NL;
 output << "Number? "; input >> number >> NL;

 x=string(chr(107-ord(name[0])))+chr(107-ord(number[0]));


 x=x+enc1(name)+'J'+enc1(number)+enc2(name)+enc2(number);

 number=""; fv=1;
 while ((length(number)+length(x))<57) 
 {
   number=number+padding[fv-1]; fv=fv+1;
 }
 x=x+enc1(number);


 output << x << NL; output << NL;
 unscramble();

 assign(txi,"v:register.raw"); reset(txi);
 assign(txo,"a:register.dat"); rewrite(txo);

 for( fv=1; fv <= 53; fv ++)
 {
  txi >> y >> NL; txo << y << NL;
 }

 txi >> y >> NL; txo << x << NL;

 while (! eof(txi)) 
 {
  txi >> y >> NL; txo << y << NL;
 }

 close(txi); close(txo);
 return EXIT_SUCCESS;
}

} // End of namespace Avalanche.