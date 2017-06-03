#ifndef MSN_DEF_H
#define MSN_DEF_H

namespace Supernova {

struct obj_struct
{
    char *name,*description,id;
    int type;
    char click,click2,section;
    char exitroom,direction;
};

/* msn.c */
void overlay(int value);
void overlay_init(char *text);
void wait2(int time);
void set_color63(char x);
void fadeout2();
void fadein2();
void init_roomlist();
void anim_on();
void anim_off();
char *ltoa(long l);
void test_vga();
void test_mouse();
void setmode(char m);
void calculate_palette();
void palette();
void palette_5();
void palette_blend();
void fadeout();
void fadein();
void title_fadein();
void palette_off();
void screen_shake();
void crit_err();
void init_buffer();
void get_inf(int seg);
void move(int ziel_seg, int quell_seg, int chunks_16bit);
void disp_puffer(long adr, int ziel, int anz);
void load(char *name);
void getspace(int para);
void load_disk(char *name);
void error_loading(char *name);
void load_sound(char *name);
void load_sound_disk(char *name);
void image(char nr);
void image_remove(char nr);
char invert(char nr);
void show_current_section();
void text(char *t);
void textxy(int x, int y, char *t, char f);
int width(char *t);
void set_int();
void restore_int();
void wait1(int time);
void box(int x, int y, int b, int h, char f);
void hgr_save(int x, int y, int w, int h);
void hgr_restore(int x, int y, int w, int h);
void message(char *text);
void message_p(int pos, char *text);
void message_remove();
void init_menu();
void command_box(int bef, bool hell);
void show_menu();
void exits();
void mouse_reset();
void hourglass();
void arrow();
void mouse_on();
void mouse_off();
void install_event_handler();
void mouse_input();
void mouse_input2();
void mouse_input3();
void mouse_wait(int time);
void sentence(int nr, bool hell);
void sentence_remove(int sentence, int nr);
void sentence_add(int sentence, int nr);
int dialog(int num, char *rowlength[6], char **text[6], int nr);
void mouse_pos_dialog(int x, int y);
void say(char *t);
void reply(char *t, int aus1, int aus2);
int command_print();
void mouse_pos(int x, int y);
void maus_pos_weg();
void mouse_right();
void input();
void inventory_object(int nr, bool brightness);
void empty_object(int nr);
void inventory_arrow(int nr, bool brightness);
void show_inventory();
void take(struct obj_struct *obj);
void inventory_remove(struct obj_struct *obj);
void inventory_remove2(struct obj_struct *obj);
void scroll_up();
void scroll_down();
void inventory_seize();
void insert(char *text, char *character);
void edit(int x, int y, char *t, int length);
bool filecopy(int ziel, int quelle);
void error_temp();
bool save_game(int nr);
bool load_game(int nr);
void load_overlay();
void load_overlay_start();
void load_save(bool speichern, bool abbruch);
void text_up();
void text_down();
void list(char *name, bool bestell);
void help();
void settextspeed();
void altx();
bool room_execute();
char bios_key();
char number_read(char min, char max, char standard);
void do_setup();
void main(int argc, char *argv[]);
/* msn_mod.c */
void init_mod();
void rechne_note();
void set_pattern(int p);
void play_sb();
void play_pc();
void start_pc_mod();
void titel_schrift();
void nur_text();
/* msn_ovl1.c */
/* msn_r0.c */
long systime();
char *time_tostring(long z);
void save_time();
void load_time();
void load_door();
void door_sound();
int door_open();
int door_close();
void money_take(int amount);
bool combine(struct obj_struct *obj1, struct obj_struct *obj2, int o1, int o2);
void ok();
bool airless();
bool helmet_takeoff();
bool generic_specialcmd(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void shock();
void alarm();
void alarm_hearing();
void outro();
void dim(int color);
void room_brightness();
void turnon();
void turnoff();
void great(int nr);
void init_rooms();
void set_section(int room, int section, bool on);
/* msn_r1.c */
bool sb_corridor(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_hall(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_sleep(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void anim_sleep();
void once_sleep();
bool sb_cockpit(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void anim_cockpit();
bool sb_airlock(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_hold(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_landingmod(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_generator(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void open_locker(struct obj_struct *obj, struct obj_struct *lock, int section);
void close_locker(struct obj_struct *obj, struct obj_struct *lock, int section);
bool sb_cabin_r3(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_cabin_l3(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_cabin_l2(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_bathroom(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void once_cockpit();
void once_airlock();
void once_hold();
/* msn_r1_r.c */
/* msn_r2.c */
bool sb_rocks(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void once_rocks();
bool sb_meetup(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void once_meetup();
void anim_meetup();
bool sb_entrance(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void anim_entrance();
void anim_remaining();
void shipstart();
void nova_scroll();
void supernova();
bool sb_roger(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void anim_roger();
void once_roger();
bool sb_glider(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void anim_glider();
bool sb_meetup2(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void once_meetup2();
bool sb_meetup3(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
/* msn_r3.c */
bool sb_cell(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void anim_cell();
void once_cell();
bool sb_corridor4(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void once_corridor();
void once_corridor4();
bool sb_corridor5(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_corridor6(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_corridor8(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_corridor9(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void shot(int a, int b);
void guard_shot();
bool sb_guard(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void guard3_shot();
bool sb_guard3(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_bgang(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void once_bgang();
bool sb_office1(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_office2(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_office3(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_office4(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_office5(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void once_office5();
void walk(int a);
void guard_walk();
void telomat(int nr);
void search_start();
void search(int time);
void guard_noticed();
void busted(int i);
void guard_back();
bool sb_elevator(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
bool sb_station(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
void taxi();
bool sb_sign(int verb, struct obj_struct *obj1, struct obj_struct *obj2);
/* msn_s.c */
void talk(int a1, int a2, int time_remain, int pos, char *t);
void talkf(int a1, int a2, int dauer);
void talk2(int a1, int a2, int b1, int b2, int duration, int pos, char *t);
void title_image();
void sirene();
void intro();
/* msn_snd.c */
void play_sb_sample(int address);
void wait_soundblaster();
void call_driver();
void init_soundblaster();
void deinit_soundblaster();
void stop();
bool keypressed();
void pc_sound_on(int int8, int int9, char speed);
void pc_sound_off();
void play_pc_sample(int adr);
void init_sound_tab(bool mod);
void init_sound();
void deinit_sound();
void play_music(bool kurz);
void play(int adr);
void sound_on();
void sound(int counter);
void sound_off();


#define MAX_SECTION     40
#define MAX_SECTION_ZUS 50
#define MAX_CLICKFIELD_ZUS  80
#define MAX_OBJECT      25
#define MAX_CARRY      30

#define ROOMNUM0  1
#define ROOMNUM1 16
#define ROOMNUM2  9
#define ROOMNUM3 21

#define NIL   0

struct room
{
  char *file;
  bool shown[MAX_SECTION];
  bool (*specialcmd)();
  void (*animation)();
  void (*onEntrance)();
  struct obj_struct object[MAX_OBJECT];
};

/* object type */
#define TAKE            1
#define OPEN            2
#define OPENED          4
#define CLOSED          8
#define EXIT           16
#define PRESS          32
#define COMBINABLE     64
#define CARRIED       128
#define UNNECESSARY   256
#define WORN          512
#define TALK         1024
#define OCCUPIED     2048
#define CAUGHT       4096

enum {ACTION_WALK,
      ACTION_LOOK,
      ACTION_TAKE,
      ACTION_OPEN,
      ACTION_CLOSE,
      ACTION_PRESS,
      ACTION_PULL,
      ACTION_USE,
      ACTION_TALK,
      ACTION_GIVE};

enum {INTRO,CORRIDOR,HALL,SLEEP,COCKPIT,AIRLOCK,
      HOLD,LANDINGMODULE,GENERATOR,OUTSIDE,
      CABIN_R1,CABIN_R2,CABIN_R3,CABIN_L1,CABIN_L3,CABIN_L2,BATHROOM,

      ROCKS,CAVE,MEETUP,ENTRANCE,REST,ROGER,GLIDER,MEETUP2,MEETUP3,

      CELL,CORRIDOR1,CORRIDOR2,CORRIDOR3,CORRIDOR4,CORRIDOR5,CORRIDOR6,CORRIDOR7,CORRIDOR8,CORRIDOR9,
      BCORRIDOR,GUARD,GUARD3,OFFICE_L1,OFFICE_L2,OFFICE_R1,OFFICE_R2,OFFICE_L,
      ELEVATOR,STATION,SIGN};

enum {X,
      KEYCARD,KNIFE,WATCH,
      SOCKET,
      BUTTON,HATCH1,
      BUTTON1,BUTTON2,MANOMETER,SUIT,HELMET,LIFESUPPORT,
      SCRAP_LK,OUTERHATCH_TOP,GENERATOR_TOP,TERMINALSTRIP,LANDINGMOD_OUTERHATCH,
        HOLD_WIRE,
      LANDINGMOD_BUTTON,LANDINGMOD_SOCKET,LANDINGMOD_WIRE,LANDINGMOD_HATCH,LANDINGMOD_MONITOR,
        KEYBOARD,
      KEYCARD2,OUTERHATCH,GENERATOR_WIRE,TRAP,SHORT_WIRE,CLIP,
        VOLTMETER,LADDER,GENERATOR_ROPE,
      KITCHEN_HATCH,SLEEP_SLOT,
      MONITOR,INSTRUMENTS,
      COMPUTER,CABINS,CABIN,
      SLOT_K1,SLOT_K2,SLOT_K3,SLOT_K4,
      SHELF1,SHELF2,SHELF3,SHELF4,
      ROPE,BOOK,DISCMAN,CHESS,
      SLOT_KL1,SLOT_KL2,SLOT_KL3,SLOT_KL4,
      SHELF_L1,SHELF_L2,SHELF_L3,SHELF_L4,
      PISTOL,BOOK2,SPOOL,
      RECORD,TURNTABLE,TURNTABLE_BUTTON,WIRE,WIRE2,PLUG,
      PEN,
      BATHROOM_DOOR,BATHROOM_EXIT,SHOWER,TOILET,

      STONE,
      SPACESHIPS,SPACESHIP,STAR,DOOR,MEETUP_SIGN,
      PORTER,BATHROOM_BUTTON,BATHROOM_SIGN,KITCHEN_SIGN,CAR_SLOT,
        ARS_BATHROOM,COINS,SCHNUCK,EGG,PILL,PILL_HULL,STAIRCASE,
        MEETUP_EXIT,
      ROGER_W,WALLET,KEYCARD_R,CUP,
      GLIDER_BUTTON1,GLIDER_BUTTON2,GLIDER_BUTTON3,GLIDER_BUTTON4,GLIDER_SLOT,GLIDER_BUTTONS,
        GLIDER_DISPLAY,GLIDER_INSTRUMENTS,GLIDER_KEYCARD,
      UFO,

      CELL_BUTTON,CELL_TABLE,CELL_WIRE,TRAY,CELL_DOOR,MAGNET,
      NEWSPAPER,TABLE,
      PILLAR1,PILLAR2,DOOR1,DOOR2,DOOR3,DOOR4,
      GUARDIAN,LAMP,
      MASTERKEYCARD,PAINTING,MONEY,LOCKER,LETTER,
      JUNGLE,SLOT,STATION_SIGN,

      TICKETS};

}
#endif // MSN_DEF_H
