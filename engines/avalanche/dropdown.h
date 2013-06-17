/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */
#ifndef __dropdown_h__
#define __dropdown_h__


typedef void(*proc)();

class headtype {
public:
            varying_string<8> title;
            char trigger,alttrigger;
            byte position;
            integer xpos,xright;
            proc do_setup,do_choose;

            headtype* init
             (char trig,char alttrig, string name, byte p, proc dw,proc dc);
            void display();
            void highlight();
            boolean extdparse(char c);
};

struct optiontype {
              string title;
              char trigger;
              varying_string<9> shortcut;
              boolean valid;
};

class onemenu {
public:
           array<1,12,optiontype> oo;
           byte number;
           integer width,left;
           boolean firstlix;
           integer flx1,flx2,fly;
           byte oldy; /* used by Lightup */
           boolean menunow; /* Is there a menu now? */
           byte menunum; /* And if so, which is it? */
           byte choicenum; /* Your choice? */
           byte highlightnum;

           void start_afresh();
           void opt(string n, char tr, string key, boolean val);
           void display();
           void wipe();
           void lightup();
           void displayopt(byte y, boolean highlit);
           void movehighlight(shortint add);
           void select(byte n);
           void keystroke(char c);
           onemenu* init();
};

class menuset {
public:
           array<1,8,headtype> ddms;
           byte howmany;

           menuset* init();
           void create(char t, string n, char alttrig, proc dw,proc dc);
           void update();
           void extd(char c);
           void getcertain(byte fv);
           void getmenu(integer x);
};



#ifdef __dropdown_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN onemenu ddm_o;
EXTERN menuset ddm_m;

EXTERN varying_string<5> people;
#undef EXTERN
#define EXTERN extern



  void find_what_you_can_do_with_it();

  void parsekey(char r,char re);

  void menu_link();

  /* DDM menu-bar procs */

  void standard_bar();
  /*procedure map_bar;*/

#endif
