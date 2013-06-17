/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */


#define __fileunit_implementation__
               /* v:filer.pas - "avvyfiler" - in unit form. */

#include "fileunit.h"


#include "graph.h"
/*#include "Dos.h"*/
/*#include "Crt.h"*/
/*#include "Tommys.h"*/
#include "lucerna.h"
#include "pingo.h"
#include "Gyro.h"

/*$V-*/

struct windowtype {
              integer x1,y1,x2,y2;
              varying_string<20> title;
};

const integer border = 1; /* size of border on shadowboxes */

const array<1,4,string> buttons = {{"Okay","Wipe","Cancel","Info..."}};

const integer files = 3;
const integer dirs = 4;

const integer drlen = 15; /* no. of drives on one line */

const array<1,4,windowtype> threewins = 
 {{{155, 12, 630, 22, "Name"},
  {15, 25, 410, 70, "Drives"},
  {15, 83, 480, 179, "Files (*.ASG)"},
  {490, 83, 630, 179, "Subdirectories"}}};

const integer name_win = 1;
const integer drive_win = 2;
const integer file_win = 3;
const integer subdir_win = 4;

const integer filer_page = 3;

matrix<3,4,1,77,varying_string<12> > lists;
array<1,77,varying_string<40> > descs;
array<3,4,byte> nums,where,top,thumb_pos,thumb_len;
searchrec s;
boolean loading;
varying_string<26> drives;
pathstr current;
byte nowwin;

pathstr filename;
boolean filefound;

boolean cancelled;

void shadow(integer x1,integer y1,integer x2,integer y2, byte hc,byte sc)
{
    byte fv;

 for( fv=0; fv <= border; fv ++)
 {
  setfillstyle(1,hc);
  bar(x1+fv,y1+fv,x1+fv,y2-fv);
  bar(x1+fv,y1+fv,x2-fv,y1+fv);

  setfillstyle(1,sc);
  bar(x2-fv,y1+fv,x2-fv,y2-fv);
  bar(x1+fv,y2-fv,x2-fv,y2-fv);
 }
}

void shbox(integer x1,integer y1,integer x2,integer y2, string t)
{
      const integer fc = 7;

 shadow(x1,y1,x2,y2,15,8);
 setfillstyle(1,fc);
 bar(x1+border+1,y1+border+1,x2-border-1,y2-border-1);
 setcolor(1); x1=(x2-x1) / 2+x1; y1=(y2-y1) / 2+y1;
 outtextxy(x1,y1,t);
 if ((t[1]!='[') && (length(t)>1)) 
 {
  fillchar(t[2],length(t)-1,'\40'); t[1]='_';
  outtextxy(x1-1,y1+1,t);
 }
}

void show_drives()
{
    byte fv;

 settextjustify(1,1);
 for( fv=0; fv <= length(drives)-1; fv ++)
  shbox((fv % drlen)*25+25,(fv / drlen)*19+31,
   (fv % drlen)*25+40,45+(fv / drlen)*19,drives[fv+1]);
 setcolor(11);
 settextjustify(0,2);
}

char which_drive(integer x,integer y)
{
  char which_drive_result;
  x=(x-25) / 25; y=(y-32) / 19;

  which_drive_result= drives[1+x+y*drlen];
  return which_drive_result;
}

void box(integer x1,integer y1,integer x2,integer y2, string z)
{
 rectangle(x1,y1,x2,y2);
 outtextxy(x1+1,y1-10,z+':');
 outtextxy(x1,y1-9,"_");
}

string lowstr(string x)
{
    byte fv;

 string lowstr_result;
 for( fv=1; fv <= length(x); fv ++)
  if (set::of(range('A','Z'), eos).has(x[fv]))  x[fv] += 32;
 lowstr_result=x;
 return lowstr_result;
}

char lowchar(char x)
{
 char lowchar_result;
 if (set::of(range('A','Z'), eos).has(x))  x -= 32;
 lowchar_result=x;
 return lowchar_result;
}

void getcurrent()
{
 current=lowstr(fexpand("*.asg"));
}

void firstsetup()
{
 integer gd,gm;
 registers r;
 byte floppies;

 /* Now... find all drives that exist. */
 drives="";
 intr(0x11,r); floppies=(((cardinal)r.ax >> 6) & 0x3)+1; /* useful bit of code! */
 for( gm=1; gm <= floppies; gm ++) drives=drives+chr(64+gm);
 /* Winchesters, etc., can be found the easy way... */
 for( gd=3; gd <= 26; gd ++) /* C: to Z: */
  if (disksize(gd)>-1)  drives=drives+chr(64+gd);

 fillchar(where,sizeof(where),'\1');
 fillchar(top,sizeof(top),'\1');

 /* Set up mouse. */
 off_virtual();
 oncandopageswap=false;
 newpointer(2);
}

void draw_scroll_bar(byte which)
{
  setcolor(1);
  {
    windowtype& with = threewins[which]; 

    setfillstyle(1,7);
    bar(with.x2-7,with.y1+10,with.x2-1,with.y2-10);
    setfillstyle(1,3);
    bar(with.x2-7,with.y1+ 1,with.x2-1,with.y1+9);
    bar(with.x2-7,with.y2- 9,with.x2-1,with.y2-1);
    outtextxy(with.x2-7,with.y1+2,"\30");
    outtextxy(with.x2-7,with.y2-8,"\31");
  }
}

void setup()
{
 integer gd,gm;
 registers r;
 byte floppies;

 setactivepage(filer_page); setvisualpage(filer_page);
 setfillstyle(1,1); bar(2,2,637,197); shadow(0,0,639,199,15,7);

 settextstyle(0,0,0);
 settextjustify(1,1);
 for( gd=1; gd <= 2; gd ++)
  for( gm=0; gm <= 1; gm ++)
   shbox(420+gm*110,gd*25,520+gm*110,gd*25+20,buttons[gm*2+gd]);
 shbox(15,182,350,196,"Help... (press f1)");
 settextjustify(0,2); setcolor(11);
 setcolor(15); outtextxy(15,5,"The Avvy Filer...");
 setcolor(11); outtextxy(317,3,"Select a file to load.");
 outtextxy(357,185,"Copyright (c) 1993, Thomas Thurman.");

 /* Draw the boxes and names */
 setcolor(3);
 for( gm=1; gm <= 4; gm ++)
  { windowtype& with = threewins[gm]; 
   box(with.x1,with.y1,with.x2,with.y2,with.title);}

 /* Draw the scroll bars. */

 for( gm=3; gm <= 4; gm ++) draw_scroll_bar(gm);
}

void thumb(byte whichwin)
{
    word length,the_top;

  if (nums[whichwin]<9) 
  {
    length=76;
    the_top=0;
  } else
  {
    length=trunc(76*((real)(8)/nums[whichwin]));
    the_top=trunc(((real)(where[whichwin])/nums[whichwin])*(76-length));
  }

  the_top += 93;   /* Top of both the scrollbars. */

  setfillstyle(1,7);
    { windowtype& with = threewins[whichwin]; 
      bar(with.x2-6,thumb_pos[whichwin],with.x2-3,thumb_pos[whichwin]+length);}
  setfillstyle(1,1);
    { windowtype& with = threewins[whichwin]; 
      bar(with.x2-6,the_top,with.x2-3,the_top+length);}

  thumb_pos[whichwin]=the_top;
  thumb_len[whichwin]=length;
}

void quicksort(byte whichlist, integer lo, integer hi);


static void sort(integer l, integer r, byte& whichlist)
{
  integer i, j;
  varying_string<12> x, y;
  varying_string<40> d;

  i = l; j = r; x = lists[whichlist][(l+r) / 2];
  do {
    while (lists[whichlist][i] < x)  i = i + 1;
    while (x < lists[whichlist][j])  j = j - 1;
    if (i <= j) 
    {
      y = lists[whichlist][i];
      lists[whichlist][i] = lists[whichlist][j];
      lists[whichlist][j] = y;

      d = descs[i];
      descs[i] = descs[j];
      descs[j] = d;

      i = i + 1; j = j - 1;
    }
  } while (!(i > j));

/*  if j<1 then j:=1;
  if r<1 then r:=1;*/

  if (l < j)  sort(l, j, whichlist);
  if (i < r)  sort(i, r, whichlist);
}

void quicksort(byte whichlist, integer lo, integer hi)

{     /*QuickSort*/;
  sort(lo,hi, whichlist);
}

void scandir()
{
 pathstr nix;
 namestr name;
 untyped_file f;
 ednahead eh;
 array<1,4,char> dna_type;


 nums[files]=0;
 findfirst("*.asg",archive+hidden+readonly,s);

 while ((doserror==0) && (nums[files]<77)) 
 {
  fsplit(s.name,nix,name,nix);
  nums[files] += 1;
  lists[files][nums[files]]=lowstr(name);

  assign(f,s.name);
  reset(f,1);
  seek(f,11);
  blockread(f,dna_type,4);

  if (dna_type=="Avvy") 
  {     /* A DNA256 file. */
   descs[nums[files]]="* Saved by Avaricius!";
  } else
  {     /* EDNA-based files. */
   if (dna_type=="EDNA") 
   {
    seek(f,177);
    blockread(f,eh,sizeof(eh));

     if (eh.revision!=2) 
      descs[nums[files]]="* Unknown EDNA type!";
     else
     {

      if (eh.number!=2) 
       descs[nums[files]]=string("% Saved by ")+eh.shortname+'!';
      else
      {     /* Well... everything seems to have gone OK! */
       descs[nums[files]]=eh.desc;
      }
     }
   } else
    descs[nums[files]]="* Not an Avvy saved game!";
  }
  close(f);
  findnext(s);
 }
 nums[dirs]=0; findfirst("*.*",directory,s);
 while ((doserror==0) && (nums[dirs]<77)) 
 {
  if (((s.attr & directory)>0) && ((length(s.name))>1)) 
  {
   nums[dirs] += 1;
   lists[dirs][nums[dirs]]=lowstr(s.name);
  }
  findnext(s);
 }

 /* Now sort 'em! */

 if (nums[dirs ]!=0)  quicksort ( dirs,1,nums[ dirs]);
 if (nums[files]!=0)  quicksort (files,1,nums[files]);

 where[dirs]=1; where[files]=1;
   top[dirs]=1; top[files]=1;

 thumb_pos[3]=93; thumb_pos[4]=93;
}

void show_file(integer x,integer y, byte which)
{
 varying_string<58> z;

 fillchar(z[1],13,'\40');
 z=lists[files][which]+".asg";
 z[0]='\15'; z=z+descs[which];

 if (set::of('*','%', eos).has(descs[which][1]))  /* Can't load these! */
  setcolor(red);
 else
  setcolor(lightcyan);

 outtextxy(x,y,z);
}

void showfiles()
{
    byte fv;

 if (loading)  setcolor(11); else setcolor(3);
 if (nums[3]==0) 
 {
  outtextxy(22,86,"(None here!)");
  return;
 }
 for( fv=0; fv <= 8; fv ++)
  if (top[3]+fv<=nums[3]) 
   show_file(19,87+fv*10,top[3]+fv);

 draw_scroll_bar(files);
}

void showdirs()
{
    byte fv;

 setcolor(11);
 for( fv=0; fv <= 8; fv ++)
  if (top[4]+fv<=nums[4]) 
   outtextxy(497,87+fv*10,string('[')+lists[dirs][fv+top[4]]+']');
 draw_scroll_bar(dirs);
}

void show()
{
 byte fv;
 dirstr d; namestr n; extstr e;

 setfillstyle(1,1);
 for( fv=1; fv <= 4; fv ++)
  if (fv!=2) 
   { windowtype& with = threewins[fv]; 
    bar(with.x1+1,with.y1+1,with.x2-8,with.y2-1);}
 showfiles();
 showdirs();
 setcolor(7); outtextxy(159,14,current);
 for( fv=3; fv <= 4; fv ++) thumb(fv);
}

void blip()
{
 sound(177); delay(77); nosound;
}

void invert(integer x1,integer y1,integer x2,integer y2)
{
    pointer p,restore; word s;

 s=imagesize(x1,y1,x2,y2);
 mark(restore); getmem(p,s);
 getimage(x1,y1,x2,y2,p);
 putimage(x1,y1,p,notput);
 release(restore);
}

void changedrive(char drive)
{
    byte fv;

 fv=pos(drive,drives);
 if (fv==0)  { blip(); return; }
 off();
 fv -= 1;
 shadow((fv % drlen)*25+25,(fv / drlen)*19+31,
   (fv % drlen)*25+40,45+(fv / drlen)*19,8,7);
 chdir(string(drive)+':');
 getcurrent(); scandir(); show();
 shadow((fv % drlen)*25+25,(fv / drlen)*19+31,
   (fv % drlen)*25+40,45+(fv / drlen)*19,15,8);
 on();
}

void highlight(byte win,byte line)
{
 switch (win) {
  case 3: invert(16,75+line*10,470,85+line*10); break;
  case 4: invert(491,75+line*10,620,85+line*10); break;
 }
 thumb(win);
}

void repaint(byte whichwindow)
{
 setfillstyle(1,1);
 { windowtype& with = threewins[whichwindow]; 
  bar(with.x1+1,with.y1+1,with.x2-8,with.y2-1);}
 top[whichwindow]=where[whichwindow];
 switch (whichwindow) {
  case file_win: showfiles(); break;
  case subdir_win: showdirs(); break;
 }
 thumb(whichwindow);
}

void fileblit(word xpos,word xlen,word y1,word y2, shortint dir, word ylen)
{
    word fv; byte bit;

 for( bit=0; bit <= 3; bit ++)
 {
  fv=0;
  while (fv<ylen) 
  {
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   move(mem[0xac00*(y1+fv*dir)*80+xpos],mem[0xac00*(y2+fv*dir)*80+xpos],xlen);
   fv += 1;
  }
 }
 bit=getpixel(0,0);
}

void seekthrough(byte whichlist, byte& wherenow, char whatfor)
{
    byte startedat;

 startedat=wherenow;
 do {
  wherenow += 1;
  if (wherenow>nums[whichlist])  wherenow=1;
 } while (!((startedat==wherenow) || (lists[whichlist][wherenow][1]==whatfor)));
 off();
 repaint(whichlist);
 on();
}

void gotohome(byte whichlist)
{
 off();
 where[whichlist]=1;
 repaint(whichlist);
 highlight(whichlist,1);
 on();
}

void gotoend(byte whichlist)
{
 off();
 where[whichlist]=nums[whichlist];
 repaint(whichlist);
 highlight(whichlist,1);
 on();
}

void pageup(byte whichlist)
{
 off();
 if (where[whichlist]>9) 
 {
  where[whichlist] -= 9;
  repaint(whichlist);
  highlight(whichlist,1);
 } else gotohome(whichlist);
 on();
}

void pagedown(byte whichlist)
{
 off();
 if (where[whichlist]<nums[whichlist]-9) 
 {
  where[whichlist] += 9;
  repaint(whichlist);
  highlight(whichlist,1);
 } else gotoend(whichlist);
 on();
}

void subdirparse(string r);

static void movehl(byte which, shortint howmuch)
{
 off();
 highlight(4,where[4]-top[4]+1);
 if (((where[which]+howmuch)>0) && ((where[which]+howmuch)<=(unsigned char)nums[4]))
       where[which]=where[which]+howmuch;
 highlight(4,where[4]-top[4]+1);
 on();
}



static void change_dir()
{
 off(); dusk();
 /*$I-*/ chdir(lists[4][where[4]]);
 /*$I+*/
 if (ioresult!=0)  { dawn(); blip(); return; }
 where[4]=1; top[4]=1;
 getcurrent(); scandir(); show();
 highlight(4,1);
 dawn(); on();
}

void subdirparse(string r)

{
 switch (r[1]) {
  case creturn: change_dir(); break;

  case '\0': switch (r[2]) {
       case cup: if (where[4]-top[4]>0)  /* Up */
             movehl(4,-1); /* Within range */
            else if (top[4]>1) 
            {     /* Outside range- must scroll */
              off();
              highlight(4,1);
              top[4] -= 1; where[4] -= 1;
              fileblit(61,17,166,176,-1,80);
              setfillstyle(1,1); bar(490,85,622,95); setcolor(11);
              outtextxy(497,87,string('[')+lists[dirs][where[4]]+']');
              highlight(4,1);
              on();
            }
            break;
       case cdown:
            if (where[4]-top[4]<8)    /* Down */
             movehl(4,1);
            else if (top[4]+8<nums[4]) 
            {
              off();
              highlight(4,9);
              top[4] += 1; where[4] += 1;
              fileblit(60,17,97,87,1,80);
              setfillstyle(1,1); bar(490,165,622,175); setcolor(11);
              outtextxy(497,167,string('[')+lists[dirs][where[4]]+']');
              highlight(4,9);
              on();
            }
            break;
       case chome: gotohome(4); break;
       case cend: gotoend(4); break;
       case cpgup: pageup(4); break;
       case cpgdn: pagedown(4); break;
       case c_ao: change_dir(); break;
      }
      break;
   default: {
         off();
         seekthrough(4,where[4],lowchar(r[1]));
         highlight(4,1);
         on();
        }
 }
}

void fileinfo(byte which); 
void filer_help(); 
void wipe(byte which); 

void in_name_box(string x)
{
 off();
 setfillstyle(1,1); bar(156,13,629,21);
 setcolor(7); outtextxy(159,14,x);
 on();
}

void filesparse(string r);

static void movehl1(byte which, shortint howmuch)
{
 off();
 highlight(3,where[3]-top[3]+1);
 if (((where[which]+howmuch)>0) && ((where[which]+howmuch)<=(unsigned char)nums[3]))
       where[which]=where[which]+howmuch;
 highlight(3,where[3]-top[3]+1);
 on();
}



static boolean selected_file()
{
 boolean selected_file_result;
 if ((set::of('*','%', eos).has(descs[where[file_win]][1])) || (nums[3]==0)) 
 {
  blip();
  selected_file_result=false;
 } else
 {
  filename=lists[file_win][where[file_win]];
  filefound=true;
  selected_file_result=true;
 }
 return selected_file_result;
}

void filesparse(string r)

{
 switch (r[1]) {
  case creturn: if (selected_file())  return; break;
  case '\0': switch (r[2]) {
       case cup: if (where[3]-top[3]>0)  /* Up */
             movehl1(3,-1); /* Within range */
            else if (top[3]>1) 
            {     /* Outside range- must scroll */
              off();
              highlight(3,1);
              top[3] -= 1; where[3] -= 1;
              fileblit(1,58,166,176,-1,80);
              setfillstyle(1,1); bar( 15,85,472,95);
              show_file( 19,87,where[3]);
              highlight(3,1);
              on();
            }
            break;
       case cdown: if (where[3]-top[3]<8)  /* Down */
             movehl1(3,1);
            else if (top[3]+8<nums[3]) 
            {
              off();
              highlight(3,9);
              top[3] += 1; where[3] += 1;
              fileblit(1,58,97,87,1,80);
              setfillstyle(1,1); bar( 15,165,472,175);
              show_file( 19,167,where[3]);
              highlight(3,9);
              on();
            }
            break;
       case c_ai: fileinfo(where[3]); break; /* alt-I: information. */
       case c_aw: wipe(where[3]); break;     /* alt-W: wipe. */
       case chome: gotohome(3); break;
       case cend: gotoend(3); break;
       case cpgup: pageup(3); break;
       case cpgdn: pagedown(3); break;
       case c_ao: if (selected_file())  return; break;
      }
      break;
  default: {
        off();
        seekthrough(3,where[3],lowchar(r[1]));
        highlight(3,1);
        on();
       }
 }
 in_name_box(lists[files][where[3]]+".asg");
}

void entername(char r)
{
 switch (r) {
  case '\10': if (filename[0]!='\0')  filename[0] -= 1; break;
  case '\15': filefound=true; break;
  case '\40':; break; /* ignore spaces. */
  default:
  {
   if (length(filename)<55)  filename=filename+r;

   if (length(filename)==1) 
   {
    where[3]=1;
    seekthrough(3,where[3],filename[1]);
   }
  }
 }

 in_name_box(filename+".asg");

}

void changewin(byte i)
{
 off();
 switch (nowwin) {
  case 3:case 4: highlight(nowwin,where[nowwin]-top[nowwin]+1); break;
 }
 setcolor(3); { windowtype& with = threewins[nowwin];  box(with.x1,with.y1,with.x2,with.y2,with.title);}
 nowwin=i;
 if (nowwin<1)  nowwin=4; if (nowwin>4)  nowwin=1;
 switch (nowwin) {
  case 3:case 4: highlight(nowwin,where[nowwin]-top[nowwin]+1); break;
 }
 on();
}

void checkmouse();

static void relevant(string x)
{
  setcolor(14); off(); { windowtype& with = threewins[nowwin];  box(with.x1,with.y1,with.x2,with.y2,with.title);}
  on(); if (nowwin==3)  filesparse(x); else subdirparse(x);
}

void checkmouse()
{
    byte fv,new,waswin;

   check();
   if (mrelease>0) 
   {      /* Where did they click? */
     for( fv=1; fv <= 4; fv ++)
       { windowtype& with = threewins[fv]; 
         if (((cardinal)with.x1<=mx) && ((cardinal)with.x2>=mx) && ((cardinal)with.y1<=my) && ((cardinal)with.y2>=my)) 
         {
           waswin=nowwin;

           if (nowwin!=fv) 
           {
             changewin(fv);
             off();
             setcolor(14); { windowtype& with1 = threewins[nowwin];  box(with1.x1,with1.y1,with1.x2,with1.y2,with1.title);}
             on();
           }
           /* Now... individual windows should do their own checkclicking. */

           switch (fv) {
             /* 1: no effect. */
             case 2: changedrive(which_drive(mx,my)); break; /* Change drive w/mouse. */
             case 3:case 4: if (my<175)  { /* Click on highlight users. */
                    if (mx>(cardinal)threewins[nowwin].x2-9) 
                    {     /* Scroll bar. */
                      if (my<(cardinal)threewins[nowwin].y1+10) 
                        relevant(null+cup); /* scroll up */
                      else if (my>(cardinal)threewins[nowwin].y2-10) 
                        relevant(null+cdown); /* scroll down. */
                      else if (my<(unsigned char)thumb_pos[nowwin]) 
                        relevant(null+cpgup); /* above thumb-- page up. */
                      else if (my>(unsigned char)thumb_pos[nowwin]+thumb_len[nowwin]) 
                        relevant(null+cpgdn); /* above thumb-- page up. */
                      else {      /* On the thumb. */
                        blip();
                      }
                    } else
                    {
                      new=top[fv]+(my-75) / 10-1;

                      if ((new==where[fv]) && (nowwin==waswin)) 
                      {     /* Double-clicked, --> "OK" */
  /*                     filefound:=true;
                       filename:=lists[fv,new];*/
                       if (fv==3)  filesparse("\15"); else subdirparse("\15");
                      } else
                      {     /* Single-clicked, --> move highlight */
                        off();
                        highlight(fv,where[fv]-top[fv]+1);
                        if ((new>0) && (new<=nums[fv])) 
                           where[fv]=new;
                        highlight(fv,where[fv]-top[fv]+1);
                        on();
                      }
                    }
                  }
                  break;
           }

           return; /* Since they've clicked in a window... */
         }}

         /* Righto, they must have clicked on a button. Which? */

         switch (my) {
            case 25 ... 45: /* Top row: Okay, Cancel. */
                     switch (mx) {
                       case 420 ... 520: switch (nowwin) {
                                  case 1: entername('\15'); break;
                                  case 3: filesparse("\15"); break;
                                  case 4: subdirparse("\15"); break;
                                  default: blip();
                                 }
                                 break;
                       case 530 ... 630: cancelled=true; break;
                     }
                     break;
            case 50 ... 95: /* Bottom row: Wipe, Info. */
                     switch (mx) {
                       case 420 ... 520: if (nowwin==3)  wipe(where[3]); else blip(); break;
                       case 530 ... 630: if (nowwin==3)  fileinfo(where[3]); else blip(); break;
                     }
                     break;
           case 180 ... 200: filer_help(); break; /* The "help" button. */
         }
   }
}

string playaround()
{
    char r,r2;

 string playaround_result;
 filefound=false; dawn();

 do {
  setcolor(14); off(); { windowtype& with = threewins[nowwin];  box(with.x1,with.y1,with.x2,with.y2,with.title);} on();
  do { checkmouse(); } while (!(keypressed() || filefound || cancelled));
  if (! (filefound || cancelled)) 
  {
    r=readkey();
    switch (r) {
     case ctab: changewin(nowwin+1); break;
     case cescape: {
               playaround_result="";
               return playaround_result;
              }
              break;
     case '\0': { /* parse extd keystroke */
          r2=readkey();
          switch (r2) {
           case cs_tab: changewin(nowwin-1); break;
           case c_an: changewin(1); break;
           case c_ad: changewin(2); break;
           case c_af: changewin(3); break;
           case c_as: changewin(4); break;
           case c_ac: cancelled=false; break;
           case c_ah:case cf1: filer_help(); break;         /* alt-H: help. */
           default:
            switch (nowwin) {
             case 3: filesparse(string('\0')+r2); break;
             case 4: subdirparse(string('\0')+r2); break;
            }
           }
          }
          break;
    default:
     {     /* Pass keystroke to current window */
      switch (nowwin) {
       case 1: entername(r); break;
       case 2: changedrive(upcase(r)); break;
       case 4: subdirparse(r); break;
       case 3: filesparse(r); break;
       default: blip();
      }
     }
   }

  }

  if (filefound) 
  {
   dusk();
   playaround_result=filename;
   return playaround_result;
  }

  if (cancelled) 
  {
   dusk();
   playaround_result="";
   return playaround_result;
  }
 } while (!false);
 return playaround_result;
}

void drawup()
{
 off();
 loading=true;
 setup();
 show(); show_drives();
 on();
}

void little_cls()
{
 setfillstyle(1,1); bar(2,2,637,197); /* Interesting information coming up! */
}

void wait_for_keypress_or_mouse_click()
{
    char r;

  do { check(); } while (!((mrelease>0) || keypressed()));
  while (keypressed())  r=readkey();
}

void fileinfo(byte which);


static void display(integer y, string left,string right)
{
 y=17+y*12;
 settextjustify(2,1); setcolor(11); outtextxy(315,y,left);
 settextjustify(0,1); setcolor(15); outtextxy(325,y,right);
}

void fileinfo(byte which)
 /* This gives information on the file whose name is in lists[files,which]. */
{
 ednahead eh;
 untyped_file f;
 varying_string<4> os;



 /* Firstly, we must check whether or not it's an Avalot file. This is easily
   done, since the descriptions of all others always begin with a star. */

 if ((descs[which][1]=='*') || (nums[3]==0)) 
 {     /* it is. */
  blip(); /* Naaaarghh! */
  return;
 }

 /* Anyway... it wasn't. */

 assign(f,lists[files][which]+".asg");
 reset(f,1);
 seek(f,177);
 blockread(f,eh,sizeof(eh));
 close(f);

 /* We now hold its EDNA record. */

 off();
 little_cls(); /* Interesting information coming up! */

 {
  display(0,"File:",eh.fn);
  display(1,"Description:",eh.desc);
  display(2,"Saved by:", eh.game);
  display(3,"version:", eh.verstr);

  display(4,"under", eh.os);

  display(6,"Saved on:",strf(eh.d)+'-'+strf(eh.m)+'-'+strf(eh.y));

  display(9,"No. of times saved:",strf(eh.saves));

  display(11,"Money:",eh.money);
  display(12,"Score:",strf(eh.points));
 }

 settextjustify(1,1);
 shbox(400,177,600,195,"[Press any key...]");
 settextjustify(0,2); on();
 wait_for_keypress_or_mouse_click();

 off(); setfillstyle(1,1); bar(2,2,637,197);
 drawup();
 off(); highlight(3,where[3]-top[3]+1); on();
}

void filer_help()
 /* Just some general help... */

{
 off(); little_cls();

 setcolor(15);
 outtextxy(10, 10,"To change to a particular pane:");
 outtextxy(10, 50,"To choose a file:");
 outtextxy(10,100,"To change drives:");
 outtextxy(10,140,"Finally...");

 setcolor(14);
 outtextxy(20, 20,"Press Alt and the initial letter simultaneously.");
 outtextxy(20, 30,"(e.g. to change to the Name pane, press Alt-N.)");
 outtextxy(20, 60,"Either type its name in the Name pane or choose it");
 outtextxy(20, 70,"from the list in the Files pane. You may either use");
 outtextxy(20, 80,"a mouse or the keyboard to do this.");
 outtextxy(20,110,"Move into the Drives pane and press the letter of the");
 outtextxy(20,120,"drive you want.");
 outtextxy(20,150,"Either select OK to load the file, or Cancel to back out.");

 settextjustify(1,1);
 shbox(400,177,600,195,"[Press any key...]");
 settextjustify(0,2); on();
 wait_for_keypress_or_mouse_click();

 off(); setfillstyle(1,1); bar(2,2,637,197);
 drawup(); off();
 if (set::of(3,4, eos).has(nowwin))  highlight(nowwin,where[nowwin]-top[nowwin]+1);
 on();
}

void wipe(byte which)
 /* This wipes the file whose name is in lists[files,which]. */
{
 char r;
 untyped_file f;


 off(); little_cls();

 settextjustify(1,1);
 outtextxy(320,100,string("Are you sure you want to delete \"")+
                        lists[files][which]+".asg\"?");
 shbox(400,177,600,195,"[Y/N]");

 do {
  r=upcase(readkey());
  if (r=='Y') 
  {
   assign(f,lists[files][which]+".asg"); /*$I-*/ erase(f); /*$I+*/

   setcolor(14);
   if (ioresult==0) 
   {
     scandir();
     outtextxy(100,140,"Deleted.");
   } else
     outtextxy(100,140,"Not deleted (some problem...)");

   shbox(400,177,600,195,"[Press any key...]");
   on();
   wait_for_keypress_or_mouse_click(); off();
  }
 } while (!(set::of('Y','N', eos).has(r)));

 settextjustify(0,2); setcolor(14);
 setfillstyle(1,1); bar(2,2,637,197);
 drawup();
 off(); highlight(3,where[3]-top[3]+1); on();
}

string do_filer()
{
 pathstr p; byte groi; dirstr original_directory;

 string do_filer_result;
 getdir(0,original_directory);
 dusk();
 oncandopageswap=false; cancelled=false;
 copypage(3,1-cp); /* Store old screen. */ groi=getpixel(0,0);
 off();

 firstsetup();
 scandir();
 nowwin=1; getcurrent();
 firstsetup(); drawup();
 on();
 mousepage(filer_page);

 p=playaround();
 if (p!="")  p=fexpand(p+".ASG");
 do_filer_result=p;
 filename="";

 mousepage(cp);
 dusk(); off();
 oncandopageswap=true;
 copypage(1-cp,3); /* Restore old screen. */ groi=getpixel(0,0);
 on_virtual(); dawn(); fix_flashers();

 setvisualpage(cp);
 setactivepage(1-cp);
 chdir(original_directory);

 return do_filer_result;
}


