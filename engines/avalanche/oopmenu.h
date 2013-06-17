#ifndef __oopmenu_h__
#define __oopmenu_h__


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
           void displayopt(byte y,byte b,byte f,byte d);
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
           void getmenu(integer x);
};



#ifdef __oopmenu_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN onemenu o;
EXTERN menuset m;
EXTERN string kbuffer;
#undef EXTERN
#define EXTERN extern


  void parsekey(char r,char re);

#endif
