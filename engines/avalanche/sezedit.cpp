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

/*$M 65520,0,655360*/
/*#include "Crt.h"*/
#include "graph.h"


const char marker = '\33';
const char game[] = "Avalot";

typedef matrix<'\0','\377',0,15,byte> fonttype;

struct markertype {
              word length;
              longint offset;
              byte checksum;
};

struct sezheader {
             array<1,2,char> initials;     /* should be "TT" */
             word gamecode;
             word revision; /* as 3- or 4-digit code (eg v1.00 = 100) */
             longint chains; /* number of scroll chains */
             longint size; /* total size of all chains */
};

array<0,1999,char> buffer,was;
word bufpos,bufsize;
longint chainnum;
varying_string<40> nickname;
array<1,255,char> junk;
string temp;
char r;
fonttype font;
integer cpos;
sezheader sezhead;
untyped_file f,tempf;
text nicknames;

byte sumup()
{
    word fv; byte total;
byte sumup_result;
;
 total=0;
 for( fv=0; fv <= bufsize-1; fv ++)
 {;
  total += ord(was[fv]);
 }
 sumup_result=total;
return sumup_result;
}

string findname(longint which)
{
    string x; varying_string<10> q;
string findname_result;
;
 str(which,q);
 reset(nicknames);
 do { nicknames >> x >> NL; } while (!(x=="{go}"));
 do {
  nicknames >> x >> NL;
 } while (!(pos(q,x)>0));
 if (eof(nicknames)) 
 {;
  findname_result="noname";
 } else
 {;
  Delete(x,1,3); /* lose "{__" */
  Delete(x,pos(" ",x),255); /* lose all chars after first space */
  findname_result=x;
 }
 close(nicknames);
return findname_result;
}

void cursor()
{
    byte fv;
;
 for( fv=12; fv <= 15; fv ++)
  mem[0xa000*cpos+fv*80]=~(mem[0xa000*cpos+fv*80]);
}

void xy()
{;
 cpos=(bufpos / 80)*1280+(bufpos % 80);
}

void show()
{
    word fv,orig,y;
;
 for( fv=0; fv <= 1999; fv ++)
  if (buffer[fv]!=was[fv]) 
  {;
   orig=(fv / 80)*1280+(fv % 80);
   for( y=0; y <= 15; y ++)
    mem[0xa000*orig+y*80]=(byte)(font[buffer[fv]][y]);
  }
 xy();
 move(buffer,was,sizeof(buffer));
}

void sizeup()
{;
 do {
  switch (buffer[bufsize]) {
   case marker: return; break; /* we've found it OK! */
   case '\0': bufsize -= 1; break;
   default: bufsize += 1;
  }
 } while (!false);
}

void graphics()
{
 integer gd,gm;
 file<fonttype> fontfile;
;
 gd=3; gm=1; initgraph(gd,gm,"");
 assign(fontfile,"c:\\thomas\\lanche.fnt");
 reset(fontfile); fontfile >> font; close(fontfile);
}

void edit_it();

static void legit(char r)
{;     /* it's a legit keystroke */
 move(buffer[bufpos],buffer[bufpos+1],1999-bufpos);
 buffer[bufpos]=r;
 bufpos += 1; show();
}

void edit_it()
{
    byte fv;
;
 for( fv=1; fv <= bufsize; fv ++) buffer[fv] -= (byte)(177*bufsize);
 fillchar(was,sizeof(was),'\0'); show();
 do {
  cursor(); r=readkey(); cursor();
  switch (r) {
   case '\0': switch (readkey()) { /* extd keystrokes */
        case 'K': if (bufpos>0)     {; bufpos -= 1; xy(); } break; /* Left */
        case 'M': if (bufpos<bufsize)  {; bufpos += 1; xy(); } break; /* Right */
        case 'H': if (bufpos>80)    {; bufpos -= 80; xy(); } break; /* Up */
        case 'P': if (bufpos<(cardinal)bufsize-79)  {; bufpos += 80; xy(); } break; /* Down */
        case 'G': {; bufpos=0; xy(); } break; /* Home */
        case 'O': {; bufpos=bufsize; xy(); } break; /* End */
        case 'S': {; /* Del */
              move(buffer[bufpos+1],buffer[bufpos],1999-bufpos); show();
             }
             break;
        case 'A': legit('\357'); break; /* copyright symbol */
       }
       break;
   case '\10': if (bufpos>0)  {; /* backspace */
        bufpos -= 1;
        move(buffer[bufpos+1],buffer[bufpos],1999-bufpos);
        show();
       }
       break;
   case '\33': {;
         for( fv=1; fv <= bufsize; fv ++) buffer[fv] += (byte)(177*bufsize);
         restorecrtmode();
         return;
        }
        break; /* end of editing */
  default: legit(r);
  }
  sizeup();
 } while (!false);
}

void saveit(longint ourchain, word oldsize,word newsize)
{
 array<1,17777,byte> buffer;
 word numread,numwritten,total;
 char check;
 word fv;
 markertype m;
 integer sizechange; /* so's it can be +ve or -ve */
 longint wheresit; /* "offset" value for "ourchain" */
;
 assign(tempf,"sez.tmp");
 /* OK, here goes... */
 sezhead.size=sezhead.size-longint(oldsize)+longint(newsize); /* adjust our size */
 sizechange=newsize-oldsize;  /* +ve if bigger, -ve if smaller, 0 if same */
 textattr=27;

 rewrite(tempf,1); reset(f,1);
 seek(f,255+sizeof(sezhead)); /* bypass the junk & sezhead (they're in RAM) */

 blockwrite(tempf,junk,255); /* move the junk... */
 blockwrite(tempf,sezhead,sizeof(sezhead)); /* and the header to new files */

 /* Now to move the markers */
 output << "Moving markers..." << NL;
 for( fv=1; fv <= sezhead.chains; fv ++)
 {;
  output << fv << '\15';
  blockread(f,m,sizeof(m));
  if (fv==ourchain)   /* Right, this one's ours! */
            {;
   wheresit=m.offset; /* recorded for later */
   m.length=newsize-1;
   m.checksum=sumup();
  } else
   if (fv>ourchain) 
   {;     /* After ours- its offset will have to be changed... */
    m.offset=m.offset+longint(sizechange);
   }    /* Otherwise, before ours- leave it well alone */
  blockwrite(tempf,m,sizeof(m));
 }
 output << "Done OK." << NL;

 /* Right, that's the markers done... thank goodness... now it's just the
   chains themselves! */

 total=0;

 output << "Updating the chains..." << NL;
 while (total<=wheresit) 
 {;
  blockread(f,buffer,1,numread);
  blockwrite(tempf,buffer,numread,numwritten);
  output << '.';
  total += numwritten;
 }
 output << NL; output << "Updating our chain..." << NL;
 /* We're now in the right place (at last...) */
 for( fv=0; fv <= (bufsize-1); fv ++) was[fv] += 3+177*fv*(bufsize-1); /* scramble "was" */
 blockwrite(tempf,was,newsize); /* "was" contains what the values *were* */
 seek(f,filepos(f)+oldsize);
 output << NL; output << "Copying everything else..." << NL;
 while (! eof(f))    /* high-speed copy */
 {;
  blockread(f,buffer,17777,numread);
  blockwrite(tempf,buffer,numread);
  output << '.';
 }
 output << NL;

 /* Closedown */
 close(f); close(tempf);
 erase(f); rename(tempf,string(game)+".SEZ");
}

void particular()
{
    word origsize;
;
 bufsize=0;
 output << "Which one\? (? for a list)"; input >> chainnum >> NL;
 if ((chainnum<0) || (chainnum>sezhead.chains)) 
 {;
  output << "Don't be silly!" << NL; return;
 }

 origsize=0 /*bufsize*/; bufpos=0; cpos=0; nickname=findname(chainnum);
 fillchar(buffer,sizeof(buffer),'\0');
 buffer[0]=marker;
 do {
  textattr=30; output << NL;
  output << "SEZ EDITOR (c) 1992, Thomas Thurman." << NL;
  output << NL;
  output << "Editing scrollchain no." << chainnum << NL;
  output << "Your text is " << bufsize << " bytes long." << NL;
  output << NL;
  output << "Its nickname is \"" << nickname << "\"." << NL;
  output << NL;
  output << "Enter a command:" << NL;
  output << "  S) Save the text to disk," << NL;
  output << "  E) Edit this particular chain," << NL;
  output << "  N) change the Nickname," << NL;
  output << "  R) Revert text to the original," << NL;
  output << "Esc) Exit and do something else!" << NL;
  output << NL;
  output << "Your choice? ";
  r=upcase(readkey()); output << r << NL;
  switch (r) {
   case 'N': {;
         output << NL; output << "New nickname (max 40 chars, Enter to cancel)?";
         input >> temp >> NL;
         if (temp!="")  nickname=temp;
        }
        break;
   case 'E': {; setgraphmode(1); edit_it(); } break;
   case 'S': saveit(chainnum,origsize,bufsize); break;
   case '\33': return; break;
  }
 } while (!false);
}

void titles()
{
 const varying_string<7> title = "SEZedit";
 byte fv; char r;
;
 settextstyle(1,0,0);
 for( fv=7; fv >= 1; fv --)
 {;
  setcolor(fv*2);
  setusercharsize(8-fv,1,8-fv,1);
  outtextxy(fv*65,fv*40-30,title[fv-1]);
 }
 setusercharsize(17,7,1,1); setcolor(green);
 outtextxy(300,10,"Thorsoft");
 outtextxy(10,310,"Press any key...");
 do { r=readkey(); } while (!(! keypressed()));
 restorecrtmode(); textattr=30; clrscr;
}

void addone()     /* Adds a new, empty chain to the end of the list. */
{
 longint fv;
 markertype m;
 array<1,17777,byte> buffer;
 word numread;
;
 assign(tempf,"sez.tmp");
 textattr=27;
 rewrite(tempf,1); reset(f,1); /* f = AVALOT.SEZ (original file) */
 seek(f,255+sizeof(sezhead));
 sezhead.chains += 1; /* we're adding a new chain */

 blockwrite(tempf,junk,255); /* move the junk */
 blockwrite(tempf,sezhead,sizeof(sezhead)); /* move the header */

 /* Now to move the markers */
 output << "Moving markers..." << NL;
 for( fv=1; fv <= sezhead.chains-longint(1); fv ++) /* -1 because we've added 1 to it */
 {;
  output << fv << '\12';
  blockread(f,m,sizeof(m));
  blockwrite(tempf,m,sizeof(m));
 }
 output << "Done OK." << NL;
 /* Now we add a new marker onto the end! */
 {;
  m.offset=sezhead.size; /* right onto the end */
  m.length=0; /* it's empty */
 }
 blockwrite(tempf,m,sizeof(m)); /* write it out to disk */

 /* Copy everything else... including the #177 check char */
 output << "Copying everything else over..." << NL;
 while (! eof(f)) 
 {;
  blockread(f,buffer,17777,numread);
  blockwrite(tempf,buffer,numread);
  output << '.';
 }
 output << NL;
 close(f); close(tempf);
 erase(f); rename(tempf,string(game)+".SEZ");
}

void general()
{
    char r;
;
 do {
  textattr=31;
  output << NL; output << string(game)+".SEZ" << NL;
  output << NL;
  output << "No. of chains: " << sezhead.chains << NL;
  output << NL;
  output << "Choose one of these:" << NL;
  output << "  A) Add a new scrollchain" << NL;
  output << "  E) Edit one" << NL;
  output << "Esc) Exit" << NL;
  output << NL;
  output << "Your choice? ";
  r=upcase(readkey()); output << r << NL;
  switch (r) {
   case 'A': addone(); break;
   case 'E': particular(); break;
   case '\33': exit(0); break;
  }
 } while (!false);
}

void loadit()
{;
 reset(f,1);
 blockread(f,junk,255);
 blockread(f,sezhead,sizeof(sezhead));
 close(f);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 assign(f,string(game)+".SEZ");
 assign(tempf,"sez.tmp");
 assign(nicknames,string(game)+".NIK");
 loadit();
 graphics();
 titles();
 general();
return EXIT_SUCCESS;
}
