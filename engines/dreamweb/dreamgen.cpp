#include "dreamgen.h"

namespace dreamgen {

static inline void allocatebuffers(Context &context);
static inline void clearbuffers(Context &context);
static inline void clearpalette(Context &context);
static inline void readsetdata(Context &context);
static inline void loadpalfromiff(Context &context);
static inline void titles(Context &context);
static inline void credits(Context &context);
static inline void cls(Context &context);
static inline void decide(Context &context);
static inline void clearchanges(Context &context);
static inline void loadroom(Context &context);
static inline void clearsprites(Context &context);
static inline void initman(Context &context);
static inline void entrytexts(Context &context);
static inline void entryanims(Context &context);
static inline void initialinv(Context &context);
static inline void startup1(Context &context);
static inline void clearbeforeload(Context &context);
static inline void startup(Context &context);
static inline void worktoscreenm(Context &context);
static inline void screenupdate(Context &context);
static inline void showgun(Context &context);
static inline void fadescreendown(Context &context);
static inline void hangon(Context &context);
static inline void fadescreendowns(Context &context);
static inline void endgame(Context &context);
static inline void loadtemptext(Context &context);
static inline void monkspeaking(Context &context);
static inline void gettingshot(Context &context);
static inline void getridoftemptext(Context &context);
static inline void loadintroroom(Context &context);
static inline void fadescreenups(Context &context);
static inline void runendseq(Context &context);
static inline void atmospheres(Context &context);
static inline void spriteupdate(Context &context);
static inline void deleverything(Context &context);
static inline void printsprites(Context &context);
static inline void reelsonscreen(Context &context);
static inline void afterintroroom(Context &context);
static inline void usetimedtext(Context &context);
static inline void dumpmap(Context &context);
static inline void dumptimedtext(Context &context);
static inline void multidump(Context &context);
static inline void getundertimed(Context &context);
static inline void printdirect(Context &context);
static inline void putundertimed(Context &context);
static inline void multiput(Context &context);
static inline void getnumber(Context &context);
static inline void modifychar(Context &context);
static inline void printchar(Context &context);
static inline void showframe(Context &context);
static inline void kernchars(Context &context);
static inline void frameoutfx(Context &context);
static inline void frameoutnm(Context &context);
static inline void frameoutbh(Context &context);
static inline void frameoutv(Context &context);
static inline void getnextword(Context &context);
static inline void multiget(Context &context);
static inline void clearwork(Context &context);
static inline void findroominloc(Context &context);
static inline void drawfloor(Context &context);
static inline void worktoscreen(Context &context);
static inline void width160(Context &context);
static inline void eraseoldobs(Context &context);
static inline void drawflags(Context &context);
static inline void calcmapad(Context &context);
static inline void doblocks(Context &context);
static inline void showallobs(Context &context);
static inline void showallfree(Context &context);
static inline void showallex(Context &context);
static inline void paneltomap(Context &context);
static inline void initrain(Context &context);
static inline void splitintolines(Context &context);
static inline void getblockofpixel(Context &context);
static inline void checkone(Context &context);
static inline void getmapad(Context &context);
static inline void calcfrframe(Context &context);
static inline void finalframe(Context &context);
static inline void getxad(Context &context);
static inline void getyad(Context &context);
static inline void makebackob(Context &context);
static inline void makesprite(Context &context);
static inline void getdimension(Context &context);
static inline void addalong(Context &context);
static inline void addlength(Context &context);
static inline void reconstruct(Context &context);
static inline void updatepeople(Context &context);
static inline void watchreel(Context &context);
static inline void showrain(Context &context);
static inline void randomnum1(Context &context);
static inline void playchannel1(Context &context);
static inline void plotreel(Context &context);
static inline void checkforshake(Context &context);
static inline void autosetwalk(Context &context);
static inline void getroomspaths(Context &context);
static inline void checkdest(Context &context);
static inline void bresenhams(Context &context);
static inline void workoutframes(Context &context);
static inline void getreelstart(Context &context);
static inline void dealwithspecial(Context &context);
static inline void showreelframe(Context &context);
static inline void soundonreels(Context &context);
static inline void playchannel0(Context &context);
static inline void findsource(Context &context);
static inline void placesetobject(Context &context);
static inline void removesetobject(Context &context);
static inline void placefreeobject(Context &context);
static inline void removefreeobject(Context &context);
static inline void switchryanoff(Context &context);
static inline void switchryanon(Context &context);
static inline void movemap(Context &context);
static inline void getfreead(Context &context);
static inline void findormake(Context &context);
static inline void getsetad(Context &context);
static inline void undertextline(Context &context);
static inline void printasprite(Context &context);
static inline void maptopanel(Context &context);
static inline void cancelch0(Context &context);
static inline void clearstartpal(Context &context);
static inline void paltoendpal(Context &context);
static inline void loadroomssample(Context &context);
static inline void loadintotemp(Context &context);
static inline void showmonk(Context &context);
static inline void getridoftemp(Context &context);
static inline void standardload(Context &context);
static inline void readheader(Context &context);
static inline void twodigitnum(Context &context);
static inline void cancelch1(Context &context);
static inline void paltostartpal(Context &context);
static inline void clearendpal(Context &context);
static inline void greyscalesum(Context &context);
static inline void endpaltostart(Context &context);
static inline void createpanel2(Context &context);
static inline void fadescreenup(Context &context);
static inline void rollendcredits2(Context &context);
static inline void rollem(Context &context);
static inline void hangone(Context &context);
static inline void createpanel(Context &context);
static inline void newplace(Context &context);
static inline void mainscreen(Context &context);
static inline void animpointer(Context &context);
static inline void showpointer(Context &context);
static inline void readmouse1(Context &context);
static inline void dumppointer(Context &context);
static inline void dumptextline(Context &context);
static inline void delpointer(Context &context);
static inline void autolook(Context &context);
static inline void watchcount(Context &context);
static inline void zoom(Context &context);
static inline void readmouse2(Context &context);
static inline void dumpzoom(Context &context);
static inline void afternewroom(Context &context);
static inline void readmouse3(Context &context);
static inline void readmouse4(Context &context);
static inline void dumpwatch(Context &context);
static inline void findpathofpoint(Context &context);
static inline void findxyfrompath(Context &context);
static inline void showicon(Context &context);
static inline void getunderzoom(Context &context);
static inline void walkintoroom(Context &context);
static inline void reminders(Context &context);
static inline void isryanholding(Context &context);
static inline void findexobject(Context &context);
static inline void compare(Context &context);
static inline void setuptimeduse(Context &context);
static inline void getanyaddir(Context &context);
static inline void getexad(Context &context);
static inline void showpanel(Context &context);
static inline void showman(Context &context);
static inline void roomname(Context &context);
static inline void panelicons1(Context &context);
static inline void zoomicon(Context &context);
static inline void middlepanel(Context &context);
static inline void showwatch(Context &context);
static inline void showtime(Context &context);
static inline void printmessage(Context &context);
static inline void usecharset1(Context &context);
static inline void putunderzoom(Context &context);
static inline void crosshair(Context &context);
static inline void dolook(Context &context);
static inline void findnextcolon(Context &context);
static inline void printslow(Context &context);
static inline void hangonp(Context &context);
static inline void redrawmainscrn(Context &context);
static inline void readmouse(Context &context);
static inline void printboth(Context &context);
static inline void waitframes(Context &context);
static inline void dumpblink(Context &context);
static inline void showblink(Context &context);
static inline void getflagunderp(Context &context);
static inline void checkcoords(Context &context);
static inline void walkandexamine(Context &context);
static inline void finishedwalking(Context &context);
static inline void examineob(Context &context);
static inline void setwalk(Context &context);
static inline void examineobtext(Context &context);
static inline void blocknametext(Context &context);
static inline void personnametext(Context &context);
static inline void walktotext(Context &context);
static inline void commandwithob(Context &context);
static inline void deltextline(Context &context);
static inline void copyname(Context &context);
static inline void findobname(Context &context);
static inline void facerightway(Context &context);
static inline void showexit(Context &context);
static inline void obicons(Context &context);
static inline void obpicture(Context &context);
static inline void describeob(Context &context);
static inline void makemainscreen(Context &context);
static inline void getobtextstart(Context &context);
static inline void obsthatdothings(Context &context);
static inline void additionaltext(Context &context);
static inline void findpuztext(Context &context);
static inline void getlocation(Context &context);
static inline void setlocation(Context &context);
static inline void lookatcard(Context &context);
static inline void getridofreels(Context &context);
static inline void loadkeypad(Context &context);
static inline void hangonw(Context &context);
static inline void restorereels(Context &context);
static inline void putbackobstuff(Context &context);
static inline void getroomdata(Context &context);
static inline void allocateload(Context &context);
static inline void searchforsame(Context &context);
static inline void getanyad(Context &context);
static inline void selectlocation(Context &context);
static inline void readcitypic(Context &context);
static inline void showcity(Context &context);
static inline void readdesticon(Context &context);
static inline void loadtraveltext(Context &context);
static inline void showarrows(Context &context);
static inline void locationpic(Context &context);
static inline void getridoftemp2(Context &context);
static inline void getridoftemp3(Context &context);
static inline void getdestinfo(Context &context);
static inline void loadintotemp2(Context &context);
static inline void loadintotemp3(Context &context);
static inline void clearreels(Context &context);
static inline void clearrest(Context &context);
static inline void pickupob(Context &context);
static inline void transfertoex(Context &context);
static inline void emergencypurge(Context &context);
static inline void getexpos(Context &context);
static inline void transfermap(Context &context);
static inline void transferinv(Context &context);
static inline void transfertext(Context &context);
static inline void pickupconts(Context &context);
static inline void transfercontoex(Context &context);
static inline void purgeanitem(Context &context);
static inline void deleteexobject(Context &context);
static inline void deleteexframe(Context &context);
static inline void deleteextext(Context &context);
static inline void resetlocation(Context &context);
static inline void checkifpathison(Context &context);
static inline void turnpathon(Context &context);
static inline void turnanypathon(Context &context);
static inline void purgealocation(Context &context);
static inline void startloading(Context &context);
static inline void fillspace(Context &context);
static inline void sortoutmap(Context &context);
static inline void deletetaken(Context &context);
static inline void setallchanges(Context &context);
static inline void autoappear(Context &context);
static inline void dochange(Context &context);
static inline void loadsavebox(Context &context);
static inline void showdecisions(Context &context);
static inline void showopbox(Context &context);
static inline void realcredits(Context &context);
static inline void allpalette(Context &context);
static inline void dumpcurrent(Context &context);
static inline void biblequote(Context &context);
static inline void intro(Context &context);
static inline void runintroseq(Context &context);
static inline void trysoundalloc(Context &context);
static inline void allocatework(Context &context);
static inline void checkforemm(Context &context);
static inline void printcurs(Context &context);
static inline void delcurs(Context &context);
static inline void commandonly(Context &context);
static inline void talk(Context &context);
static inline void obname(Context &context);
static inline void pixelcheckset(Context &context);
static inline void isitdescribed(Context &context);
static inline void getreelframeax(Context &context);
static inline void blank(Context &context);
static inline void findfirstpath(Context &context);
static inline void checkifex(Context &context);
static inline void checkiffree(Context &context);
static inline void checkifperson(Context &context);
static inline void checkifset(Context &context);
static inline void identifyob(Context &context);
static inline void convnum(Context &context);
static inline void onedigit(Context &context);
static inline void parseblaster(Context &context);
static inline void volumeadjust(Context &context);
static inline void loopchannel0(Context &context);
static inline void createname(Context &context);
static inline void doload(Context &context);
static inline void showloadops(Context &context);
static inline void showslots(Context &context);
static inline void shownames(Context &context);
static inline void namestoold(Context &context);
static inline void storeit(Context &context);
static inline void makeheader(Context &context);
static inline void oldtonames(Context &context);
static inline void showsaveops(Context &context);
static inline void readkey(Context &context);
static inline void getnamepos(Context &context);
static inline void selectslot(Context &context);
static inline void restoreall(Context &context);
static inline void checkinput(Context &context);
static inline void showdiscops(Context &context);
static inline void getback1(Context &context);
static inline void getridofall(Context &context);
static inline void showmainops(Context &context);
static inline void dosaveload(Context &context);
static inline void findtext1(Context &context);
static inline void usetempcharset(Context &context);
static inline void showdiary(Context &context);
static inline void showdiarypage(Context &context);
static inline void loadtempcharset(Context &context);
static inline void showdiarykeys(Context &context);
static inline void dumpdiarykeys(Context &context);
static inline void getridoftempcharset(Context &context);
static inline void nextsymbol(Context &context);
static inline void showsymbol(Context &context);
static inline void updatesymboltop(Context &context);
static inline void updatesymbolbot(Context &context);
static inline void dumpsymbol(Context &context);
static inline void turnanypathoff(Context &context);
static inline void folderexit(Context &context);
static inline void showleftpage(Context &context);
static inline void showrightpage(Context &context);
static inline void showfolder(Context &context);
static inline void folderhints(Context &context);
static inline void loadfolder(Context &context);
static inline void loadmenu(Context &context);
static inline void getundermenu(Context &context);
static inline void putundermenu(Context &context);
static inline void showmenu(Context &context);
static inline void dumpmenu(Context &context);
static inline void singlekey(Context &context);
static inline void buttonpress(Context &context);
static inline void showouterpad(Context &context);
static inline void showkeypad(Context &context);
static inline void dumpkeypad(Context &context);
static inline void addtopresslist(Context &context);
static inline void isitright(Context &context);
static inline void checkinside(Context &context);
static inline void showfirstuse(Context &context);
static inline void withwhat(Context &context);
static inline void showpuztext(Context &context);
static inline void issetobonmap(Context &context);
static inline void showseconduse(Context &context);
static inline void removeobfrominv(Context &context);
static inline void turnpathoff(Context &context);
static inline void findinvpos(Context &context);
static inline void useroutine(Context &context);
static inline void printmessage2(Context &context);
static inline void fillryan(Context &context);
static inline void findsetobject(Context &context);
static inline void usetext(Context &context);
static inline void moneypoke(Context &context);
static inline void nextcolon(Context &context);
static inline void entercode(Context &context);
static inline void makeworn(Context &context);
static inline void nothelderror(Context &context);
static inline void triggermessage(Context &context);
static inline void monprint(Context &context);
static inline void showcurrentfile(Context &context);
static inline void printlogo(Context &context);
static inline void printundermon(Context &context);
static inline void randomaccess(Context &context);
static inline void locklighton(Context &context);
static inline void locklightoff(Context &context);
static inline void makecaps(Context &context);
static inline void monmessage(Context &context);
static inline void scrollmonitor(Context &context);
static inline void searchforstring(Context &context);
static inline void getkeyandlogo(Context &context);
static inline void monitorlogo(Context &context);
static inline void parser(Context &context);
static inline void neterror(Context &context);
static inline void processtrigger(Context &context);
static inline void input(Context &context);
static inline void dirfile(Context &context);
static inline void searchforfiles(Context &context);
static inline void dircom(Context &context);
static inline void signon(Context &context);
static inline void read(Context &context);
static inline void showkeys(Context &context);
static inline void delchar(Context &context);
static inline void accesslighton(Context &context);
static inline void accesslightoff(Context &context);
static inline void powerlighton(Context &context);
static inline void powerlightoff(Context &context);
static inline void lookininterface(Context &context);
static inline void loadpersonal(Context &context);
static inline void loadnews(Context &context);
static inline void loadcart(Context &context);
static inline void printoutermon(Context &context);
static inline void initialmoncols(Context &context);
static inline void turnonpower(Context &context);
static inline void fadeupyellows(Context &context);
static inline void fadeupmonfirst(Context &context);
static inline void hangoncurs(Context &context);
static inline void execcommand(Context &context);
static inline void getundercentre(Context &context);
static inline void putundercentre(Context &context);
static inline void convicons(Context &context);
static inline void starttalk(Context &context);
static inline void hangonpq(Context &context);
static inline void redes(Context &context);
static inline void dosometalk(Context &context);
static inline void getpersontext(Context &context);
static inline void getpersframe(Context &context);
static inline void findopenpos(Context &context);
static inline void reexfromopen(Context &context);
static inline void geteitherad(Context &context);
static inline void fillopen(Context &context);
static inline void useopened(Context &context);
static inline void getopenedsize(Context &context);
static inline void errormessage3(Context &context);
static inline void errormessage2(Context &context);
static inline void examicon(Context &context);
static inline void outofopen(Context &context);
static inline void swapwithopen(Context &context);
static inline void isitworn(Context &context);
static inline void wornerror(Context &context);
static inline void errormessage1(Context &context);
static inline void checkobjectsize(Context &context);
static inline void openinv(Context &context);
static inline void openob(Context &context);
static inline void droperror(Context &context);
static inline void cantdrop(Context &context);
static inline void reexfrominv(Context &context);
static inline void intoinv(Context &context);
static inline void outofinv(Context &context);
static inline void swapwithinv(Context &context);
static inline void findallopen(Context &context);
static inline void obtoinv(Context &context);
static inline void findallryan(Context &context);
static inline void showryanpage(Context &context);
static inline void lockmon(Context &context);
static inline void fadecalculation(Context &context);
static inline void dodoor(Context &context);
static inline void liftnoise(Context &context);
static inline void widedoor(Context &context);
static inline void random(Context &context);
static inline void lockeddoorway(Context &context);
static inline void liftsprite(Context &context);
static inline void doorway(Context &context);
static inline void constant(Context &context);
static inline void steady(Context &context);
static inline void adjustleft(Context &context);
static inline void adjustright(Context &context);
static inline void adjustdown(Context &context);
static inline void adjustup(Context &context);
static inline void aboutturn(Context &context);
static inline void checkforexit(Context &context);
static inline void walking(Context &context);
static inline void showgamereel(Context &context);
static inline void checkspeed(Context &context);
static inline void addtopeoplelist(Context &context);
static inline void setuptimedtemp(Context &context);
static inline void madmantext(Context &context);
static inline void madmode(Context &context);
static inline void priesttext(Context &context);
static inline void fadescreenuphalf(Context &context);
static inline void textforend(Context &context);
static inline void fadescreendownhalf(Context &context);
static inline void rollendcredits(Context &context);
static inline void textformonk(Context &context);
static inline void monks2text(Context &context);
static inline void intro2text(Context &context);
static inline void intro3text(Context &context);
static inline void intro1text(Context &context);

static inline void alleybarksound(Context & context) {
	context.ax = context.data.word(context.bx+3);
	context._sub(context.ax, 1);
	context._cmp(context.ax, 0);
	if (!context.flags.z()) goto nobark;
	context.push(context.bx);
	context.push(context.es);
	context.al = 14;
	playchannel1(context);
	context.es = context.pop();
	context.bx = context.pop();
	context.ax = 1000;
nobark:
	context.data.word(context.bx+3) = context.ax;
	return;
}

static inline void intromusic(Context & context) {
	return;
}

static inline void foghornsound(Context & context) {
	randomnumber(context);
	context._cmp(context.al, 198);
	if (!context.flags.z()) goto nofog;
	context.al = 13;
	playchannel1(context);
nofog:
	return;
}

static inline void receptionist(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto gotrecep;
	context._cmp(context.data.byte(52), 1);
	if (!context.flags.z()) goto notsetcard;
	context._add(context.data.byte(52), 1);
	context.data.byte(context.bx+7) = 1;
	context.data.word(context.bx+3) = 64;
notsetcard:
	context._cmp(context.data.word(context.bx+3), 58);
	if (!context.flags.z()) goto notdes1;
	randomnumber(context);
	context._cmp(context.al, 30);
	if (context.flags.c()) goto notdes2;
	context.data.word(context.bx+3) = 55;
	goto gotrecep;
notdes1:
	context._cmp(context.data.word(context.bx+3), 60);
	if (!context.flags.z()) goto notdes2;
	randomnumber(context);
	context._cmp(context.al, 240);
	if (context.flags.c()) goto gotrecep;
	context.data.word(context.bx+3) = 53;
	goto gotrecep;
notdes2:
	context._cmp(context.data.word(context.bx+3), 88);
	if (!context.flags.z()) goto notendcard;
	context.data.word(context.bx+3) = 53;
	goto gotrecep;
notendcard:
	context._add(context.data.word(context.bx+3), 1);
gotrecep:
	showgamereel(context);
	addtopeoplelist(context);
	context.al = context.data.byte(context.bx+7);
	context._and(context.al, 128);
	if (context.flags.z()) goto nottalkedrecep;
	context.data.byte(51) = 1;
nottalkedrecep:
	return;
}

static inline void smokebloke(Context & context) {
	context._cmp(context.data.byte(41), 0);
	if (!context.flags.z()) goto notspokento;
	context.al = context.data.byte(context.bx+7);
	context._and(context.al, 128);
	if (context.flags.z()) goto notspokento;
	context.push(context.es);
	context.push(context.bx);
	context.al = 5;
	setlocation(context);
	context.bx = context.pop();
	context.es = context.pop();
notspokento:
	checkspeed(context);
	if (!context.flags.z()) goto gotsmokeb;
	context._cmp(context.data.word(context.bx+3), 100);
	if (!context.flags.z()) goto notsmokeb1;
	randomnumber(context);
	context._cmp(context.al, 30);
	if (context.flags.c()) goto notsmokeb2;
	context.data.word(context.bx+3) = 96;
	goto gotsmokeb;
notsmokeb1:
	context._cmp(context.data.word(context.bx+3), 117);
	if (!context.flags.z()) goto notsmokeb2;
	context.data.word(context.bx+3) = 96;
	goto gotsmokeb;
notsmokeb2:
	context._add(context.data.word(context.bx+3), 1);
gotsmokeb:
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void attendant(Context & context) {
	showgamereel(context);
	addtopeoplelist(context);
	context.al = context.data.byte(context.bx+7);
	context._and(context.al, 128);
	if (context.flags.z()) goto nottalked;
	context.data.byte(48) = 1;
nottalked:
	return;
}

static inline void manasleep(Context & context) {
	context.al = context.data.byte(context.bx+7);
	context._and(context.al, 127);
	context.data.byte(context.bx+7) = context.al;
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void eden(Context & context) {
	context._cmp(context.data.byte(42), 0);
	if (!context.flags.z()) goto notinbed;
	showgamereel(context);
	addtopeoplelist(context);
notinbed:
	return;
}

static inline void edeninbath(Context & context) {
	context._cmp(context.data.byte(42), 0);
	if (context.flags.z()) goto notinbed;
	context._cmp(context.data.byte(43), 0);
	if (!context.flags.z()) goto notinbath;
	showgamereel(context);
	addtopeoplelist(context);
notinbath:
	return;
/*continuing to unbounded code: notinbed from eden:5-6*/
notinbed:
	return;
	return;
}

static inline void malefan(Context & context) {
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void femalefan(Context & context) {
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void louis(Context & context) {
	context._cmp(context.data.byte(41), 0);
	if (!context.flags.z()) goto notlouis1;
	showgamereel(context);
	addtopeoplelist(context);
notlouis1:
	return;
}

static inline void louischair(Context & context) {
	context._cmp(context.data.byte(41), 0);
	if (context.flags.z()) goto notlouis2;
	checkspeed(context);
	if (!context.flags.z()) goto notlouisanim;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 191);
	if (context.flags.z()) goto restartlouis;
	context._cmp(context.ax, 185);
	if (context.flags.z()) goto randomlouis;
	context.data.word(context.bx+3) = context.ax;
	goto notlouisanim;
randomlouis:
	context.data.word(context.bx+3) = context.ax;
	randomnumber(context);
	context._cmp(context.al, 245);
	if (!context.flags.c()) goto notlouisanim;
restartlouis:
	context.ax = 182;
	context.data.word(context.bx+3) = context.ax;
notlouisanim:
	showgamereel(context);
	addtopeoplelist(context);
notlouis2:
	return;
}

static inline void manasleep2(Context & context) {
	context.al = context.data.byte(context.bx+7);
	context._and(context.al, 127);
	context.data.byte(context.bx+7) = context.al;
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void mansatstill(Context & context) {
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void tattooman(Context & context) {
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void drinker(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto gotdrinker;
	context._add(context.data.word(context.bx+3), 1);
	context._cmp(context.data.word(context.bx+3), 115);
	if (!context.flags.z()) goto notdrinker1;
	context.data.word(context.bx+3) = 105;
	goto gotdrinker;
notdrinker1:
	context._cmp(context.data.word(context.bx+3), 106);
	if (!context.flags.z()) goto gotdrinker;
	randomnumber(context);
	context._cmp(context.al, 3);
	if (context.flags.c()) goto gotdrinker;
	context.data.word(context.bx+3) = 105;
gotdrinker:
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void bartender(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto gotsmoket;
	context._cmp(context.data.word(context.bx+3), 86);
	if (!context.flags.z()) goto notsmoket1;
	randomnumber(context);
	context._cmp(context.al, 18);
	if (context.flags.c()) goto notsmoket2;
	context.data.word(context.bx+3) = 81;
	goto gotsmoket;
notsmoket1:
	context._cmp(context.data.word(context.bx+3), 103);
	if (!context.flags.z()) goto notsmoket2;
	context.data.word(context.bx+3) = 81;
	goto gotsmoket;
notsmoket2:
	context._add(context.data.word(context.bx+3), 1);
gotsmoket:
	showgamereel(context);
	context._cmp(context.data.byte(46), 1);
	if (!context.flags.z()) goto notgotgun;
	context.data.byte(context.bx+7) = 9;
notgotgun:
	addtopeoplelist(context);
	return;
}

static inline void othersmoker(Context & context) {
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void barwoman(Context & context) {
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void interviewer(Context & context) {
	context._cmp(context.data.word(23), 68);
	if (!context.flags.z()) goto notgeneralstart;
	context._add(context.data.word(context.bx+3), 1);
notgeneralstart:
	context._cmp(context.data.word(context.bx+3), 250);
	if (context.flags.z()) goto talking;
	checkspeed(context);
	if (!context.flags.z()) goto talking;
	context._cmp(context.data.word(context.bx+3), 259);
	if (context.flags.z()) goto talking;
	context._add(context.data.word(context.bx+3), 1);
talking:
	showgamereel(context);
	return;
}

static inline void soldier1(Context & context) {
	context._cmp(context.data.word(context.bx+3), 0);
	if (context.flags.z()) goto soldierwait;
	context.data.word(21) = 10;
	context._cmp(context.data.word(context.bx+3), 30);
	if (!context.flags.z()) goto notaftersshot;
	context._add(context.data.byte(64), 1);
	context._cmp(context.data.byte(64), 40);
	if (!context.flags.z()) goto gotsoldframe;
	context.data.byte(56) = 2;
	goto gotsoldframe;
notaftersshot:
	checkspeed(context);
	if (!context.flags.z()) goto gotsoldframe;
	context._add(context.data.word(context.bx+3), 1);
	goto gotsoldframe;
soldierwait:
	context._cmp(context.data.byte(65), 1);
	if (!context.flags.z()) goto gotsoldframe;
	context.data.word(21) = 10;
	context._cmp(context.data.byte(475), 2);
	if (!context.flags.z()) goto gotsoldframe;
	context._cmp(context.data.byte(133), 4);
	if (!context.flags.z()) goto gotsoldframe;
	context._add(context.data.word(context.bx+3), 1);
	context.data.byte(65) = -1;
	context.data.byte(64) = 0;
gotsoldframe:
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void rockstar(Context & context) {
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 303);
	if (context.flags.z()) goto rockcombatend;
	context._cmp(context.ax, 118);
	if (context.flags.z()) goto rockcombatend;
	checkspeed(context);
	if (!context.flags.z()) goto rockspeed;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 118);
	if (!context.flags.z()) goto notbeforedead;
	context.data.byte(56) = 2;
	goto gotrockframe;
notbeforedead:
	context._cmp(context.ax, 79);
	if (!context.flags.z()) goto gotrockframe;
	context._sub(context.ax, 1);
	context._cmp(context.data.byte(65), 1);
	if (!context.flags.z()) goto notgunonrock;
	context.data.byte(65) = -1;
	context.ax = 123;
	goto gotrockframe;
notgunonrock:
	context._add(context.data.byte(64), 1);
	context._cmp(context.data.byte(64), 40);
	if (!context.flags.z()) goto gotrockframe;
	context.data.byte(64) = 0;
	context.ax = 79;
gotrockframe:
	context.data.word(context.bx+3) = context.ax;
rockspeed:
	showgamereel(context);
	context._cmp(context.data.word(context.bx+3), 78);
	if (!context.flags.z()) goto notalkrock;
	addtopeoplelist(context);
	context.data.byte(234) = 2;
	context.data.word(21) = 0;
	return;
notalkrock:
	context.data.word(21) = 2;
	context.data.byte(234) = 0;
	context.al = context.data.byte(149);
	context.data.byte(context.bx+2) = context.al;
	return;
rockcombatend:
	context.data.byte(188) = 45;
	showgamereel(context);
	return;
}

static inline void helicopter(Context & context) {
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 203);
	if (context.flags.z()) goto heliwon;
	checkspeed(context);
	if (!context.flags.z()) goto helispeed;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 53);
	if (!context.flags.z()) goto notbeforehdead;
	context._add(context.data.byte(64), 1);
	context._cmp(context.data.byte(64), 8);
	if (context.flags.c()) goto waitabit;
	context.data.byte(56) = 2;
waitabit:
	context.ax = 49;
	goto gotheliframe;
notbeforehdead:
	context._cmp(context.ax, 9);
	if (!context.flags.z()) goto gotheliframe;
	context._sub(context.ax, 1);
	context._cmp(context.data.byte(65), 1);
	if (!context.flags.z()) goto notgunonheli;
	context.data.byte(65) = -1;
	context.ax = 55;
	goto gotheliframe;
notgunonheli:
	context.ax = 5;
	context._add(context.data.byte(64), 1);
	context._cmp(context.data.byte(64), 20);
	if (!context.flags.z()) goto gotheliframe;
	context.data.byte(64) = 0;
	context.ax = 9;
gotheliframe:
	context.data.word(context.bx+3) = context.ax;
helispeed:
	showgamereel(context);
	context.al = context.data.byte(148);
	context.data.byte(context.bx+1) = context.al;
helicombatend:
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 9);
	if (!context.flags.c()) goto notwaitingheli;
	context._cmp(context.data.byte(64), 7);
	if (context.flags.c()) goto notwaitingheli;
	context.data.byte(234) = 2;
	context.data.word(21) = 0;
	return;
notwaitingheli:
	context.data.byte(234) = 0;
	context.data.word(21) = 2;
	return;
heliwon:
	context.data.byte(234) = 0;
	return;
}

static inline void mugger(Context & context) {
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 138);
	if (context.flags.z()) goto endmugger1;
	context._cmp(context.ax, 176);
	if (context.flags.z()) goto endmugger2;
	context._cmp(context.ax, 2);
	if (!context.flags.z()) goto havesetwatch;
	context.data.word(21) = 175*2;
havesetwatch:
	checkspeed(context);
	if (!context.flags.z()) goto notmugger;
	context._add(context.data.word(context.bx+3), 1);
notmugger:
	showgamereel(context);
	context.al = context.data.byte(148);
	context.data.byte(context.bx+1) = context.al;
	return;
endmugger1:
	context.push(context.es);
	context.push(context.bx);
	createpanel2(context);
	showicon(context);
	context.al = 41;
	findpuztext(context);
	context.di = 33+20;
	context.bx = 104;
	context.dl = 241;
	context.ah = 0;
	printdirect(context);
	worktoscreen(context);
	context.cx = 300;
	hangon(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.push(context.es);
	context.push(context.bx);
	context.data.word(context.bx+3) = 140;
	context.data.byte(475) = 2;
	context.data.byte(477) = 2;
	findxyfrompath(context);
	context.data.byte(187) = 1;
	context.al = 'W';
	context.ah = 'E';
	context.cl = 'T';
	context.ch = 'A';
	findexobject(context);
	context.data.byte(99) = context.al;
	context.data.byte(102) = 4;
	removeobfrominv(context);
	context.al = 'W';
	context.ah = 'E';
	context.cl = 'T';
	context.ch = 'B';
	findexobject(context);
	context.data.byte(99) = context.al;
	context.data.byte(102) = 4;
	removeobfrominv(context);
	makemainscreen(context);
	context.al = 48;
	context.bl = 68-32;
	context.bh = 54+64;
	context.cx = 70;
	context.dx = 10;
	setuptimeduse(context);
	context.data.byte(45) = 1;
	context.bx = context.pop();
	context.es = context.pop();
	return;
endmugger2:
	return;
}

static inline void aide(Context & context) {
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void businessman(Context & context) {
	context.data.byte(234) = 0;
	context.data.word(21) = 2;
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 2);
	if (!context.flags.z()) goto notfirstbiz;
	context.push(context.ax);
	context.push(context.bx);
	context.push(context.es);
	context.al = 49;
	context.cx = 30;
	context.dx = 1;
	context.bl = 68;
	context.bh = 174;
	setuptimeduse(context);
	context.es = context.pop();
	context.bx = context.pop();
	context.ax = context.pop();
notfirstbiz:
	context._cmp(context.ax, 95);
	if (context.flags.z()) goto buscombatwonend;
	context._cmp(context.ax, 49);
	if (context.flags.z()) goto buscombatend;
	checkspeed(context);
	if (!context.flags.z()) goto busspeed;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 48);
	if (!context.flags.z()) goto notbeforedeadb;
	context.data.byte(56) = 2;
	goto gotbusframe;
notbeforedeadb:
	context._cmp(context.ax, 15);
	if (!context.flags.z()) goto buscombatwon;
	context._sub(context.ax, 1);
	context._cmp(context.data.byte(65), 3);
	if (!context.flags.z()) goto notshieldonbus;
	context.data.byte(65) = -1;
	context.data.byte(64) = 0;
	context.ax = 51;
	goto gotbusframe;
notshieldonbus:
	context._add(context.data.byte(64), 1);
	context._cmp(context.data.byte(64), 20);
	if (!context.flags.z()) goto gotbusframe;
	context.data.byte(64) = 0;
	context.ax = 15;
	goto gotbusframe;
buscombatwon:
	context._cmp(context.ax, 91);
	if (!context.flags.z()) goto gotbusframe;
	context.push(context.bx);
	context.push(context.es);
	context.al = 0;
	turnpathon(context);
	context.al = 1;
	turnpathon(context);
	context.al = 2;
	turnpathon(context);
	context.al = 3;
	turnpathoff(context);
	context.data.byte(475) = 5;
	context.data.byte(477) = 5;
	findxyfrompath(context);
	context.data.byte(187) = 1;
	context.es = context.pop();
	context.bx = context.pop();
	context.ax = 92;
	goto gotbusframe;
gotbusframe:
	context.data.word(context.bx+3) = context.ax;
busspeed:
	showgamereel(context);
	context.al = context.data.byte(149);
	context.data.byte(context.bx+2) = context.al;
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 14);
	if (!context.flags.z()) goto buscombatend;
	context.data.word(21) = 0;
	context.data.byte(234) = 2;
	return;
buscombatend:
	return;
buscombatwonend:
	context.data.byte(234) = 0;
	context.data.word(21) = 0;
	return;
}

static inline void poolguard(Context & context) {
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 214);
	if (context.flags.z()) goto combatover2;
	context._cmp(context.ax, 258);
	if (context.flags.z()) goto combatover2;
	context._cmp(context.ax, 185);
	if (context.flags.z()) goto combatover1;
	context._cmp(context.ax, 0);
	if (!context.flags.z()) goto notfirstpool;
	context.al = 0;
	turnpathon(context);
notfirstpool:
	checkspeed(context);
	if (!context.flags.z()) goto guardspeed;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 122);
	if (!context.flags.z()) goto notendguard1;
	context._sub(context.ax, 1);
	context._cmp(context.data.byte(65), 2);
	if (!context.flags.z()) goto notaxeonpool;
	context.data.byte(65) = -1;
	context.ax = 122;
	goto gotguardframe;
notaxeonpool:
	context._add(context.data.byte(64), 1);
	context._cmp(context.data.byte(64), 40);
	if (!context.flags.z()) goto gotguardframe;
	context.data.byte(64) = 0;
	context.ax = 195;
	goto gotguardframe;
notendguard1:
	context._cmp(context.ax, 147);
	if (!context.flags.z()) goto gotguardframe;
	context._sub(context.ax, 1);
	context._cmp(context.data.byte(65), 1);
	if (!context.flags.z()) goto notgunonpool;
	context.data.byte(65) = -1;
	context.ax = 147;
	goto gotguardframe;
notgunonpool:
	context._add(context.data.byte(64), 1);
	context._cmp(context.data.byte(64), 40);
	if (!context.flags.z()) goto gotguardframe;
	context.data.byte(64) = 0;
	context.ax = 220;
gotguardframe:
	context.data.word(context.bx+3) = context.ax;
guardspeed:
	showgamereel(context);
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 121);
	if (context.flags.z()) goto iswaitingpool;
	context._cmp(context.ax, 146);
	if (context.flags.z()) goto iswaitingpool;
	context.data.byte(234) = 0;
	context.data.word(21) = 2;
	return;
iswaitingpool:
	context.data.byte(234) = 2;
	context.data.word(21) = 0;
	return;
combatover1:
	context.data.word(21) = 0;
	context.data.byte(234) = 0;
	context.al = 0;
	turnpathon(context);
	context.al = 1;
	turnpathoff(context);
	return;
combatover2:
	showgamereel(context);
	context.data.word(21) = 2;
	context.data.byte(234) = 0;
	context._add(context.data.byte(64), 1);
	context._cmp(context.data.byte(64), 100);
	if (context.flags.c()) goto doneover2;
	context.data.word(21) = 0;
	context.data.byte(56) = 2;
doneover2:
	return;
}

static inline void security(Context & context) {
	context._cmp(context.data.word(context.bx+3), 32);
	if (context.flags.z()) goto securwait;
	context._cmp(context.data.word(context.bx+3), 69);
	if (!context.flags.z()) goto notaftersec;
	return;
notaftersec:
	context.data.word(21) = 10;
	checkspeed(context);
	if (!context.flags.z()) goto gotsecurframe;
	context._add(context.data.word(context.bx+3), 1);
	goto gotsecurframe;
securwait:
	context._cmp(context.data.byte(65), 1);
	if (!context.flags.z()) goto gotsecurframe;
	context.data.word(21) = 10;
	context._cmp(context.data.byte(475), 9);
	if (!context.flags.z()) goto gotsecurframe;
	context._cmp(context.data.byte(133), 0);
	if (!context.flags.z()) goto gotsecurframe;
	context.data.byte(65) = -1;
	context._add(context.data.word(context.bx+3), 1);
gotsecurframe:
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void heavy(Context & context) {
	context.al = context.data.byte(context.bx+7);
	context._and(context.al, 127);
	context.data.byte(context.bx+7) = context.al;
	context._cmp(context.data.word(context.bx+3), 43);
	if (context.flags.z()) goto heavywait;
	context.data.word(21) = 10;
	context._cmp(context.data.word(context.bx+3), 70);
	if (!context.flags.z()) goto notafterhshot;
	context._add(context.data.byte(64), 1);
	context._cmp(context.data.byte(64), 80);
	if (!context.flags.z()) goto gotheavyframe;
	context.data.byte(56) = 2;
	goto gotheavyframe;
notafterhshot:
	checkspeed(context);
	if (!context.flags.z()) goto gotheavyframe;
	context._add(context.data.word(context.bx+3), 1);
	goto gotheavyframe;
heavywait:
	context._cmp(context.data.byte(65), 1);
	if (!context.flags.z()) goto gotheavyframe;
	context._cmp(context.data.byte(475), 5);
	if (!context.flags.z()) goto gotheavyframe;
	context._cmp(context.data.byte(133), 4);
	if (!context.flags.z()) goto gotheavyframe;
	context.data.byte(65) = -1;
	context._add(context.data.word(context.bx+3), 1);
	context.data.byte(64) = 0;
gotheavyframe:
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void bossman(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto notboss;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 4);
	if (context.flags.z()) goto firstdes;
	context._cmp(context.ax, 20);
	if (context.flags.z()) goto secdes;
	context._cmp(context.ax, 41);
	if (!context.flags.z()) goto gotallboss;
	context.ax = 0;
	context._add(context.data.byte(46), 1);
	context.data.byte(context.bx+7) = 10;
	goto gotallboss;
firstdes:
	context._cmp(context.data.byte(46), 1);
	if (context.flags.z()) goto gotallboss;
	context.push(context.ax);
	randomnumber(context);
	context.cl = context.al;
	context.ax = context.pop();
	context._cmp(context.cl, 10);
	if (context.flags.c()) goto gotallboss;
	context.ax = 0;
	goto gotallboss;
secdes:
	context._cmp(context.data.byte(46), 1);
	if (context.flags.z()) goto gotallboss;
	context.ax = 0;
gotallboss:
	context.data.word(context.bx+3) = context.ax;
notboss:
	showgamereel(context);
	addtopeoplelist(context);
	context.al = context.data.byte(context.bx+7);
	context._and(context.al, 128);
	if (context.flags.z()) goto nottalkedboss;
	context.data.byte(50) = 1;
nottalkedboss:
	return;
}

static inline void gamer(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto gamerfin;
gameragain:
	randomnum1(context);
	context._and(context.al, 7);
	context._cmp(context.al, 5);
	if (!context.flags.c()) goto gameragain;
	context._add(context.al, 20);
	context._cmp(context.al, context.data.byte(context.bx+3));
	if (context.flags.z()) goto gameragain;
	context.ah = 0;
	context.data.word(context.bx+3) = context.ax;
gamerfin:
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void sparkydrip(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto cantdrip;
	context.al = 14;
	context.ah = 0;
	playchannel0(context);
cantdrip:
	return;
}

static inline void carparkdrip(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto cantdrip2;
	context.al = 14;
	playchannel1(context);
cantdrip2:
	return;
}

static inline void keeper(Context & context) {
	context._cmp(context.data.byte(54), 0);
	if (!context.flags.z()) goto notwaiting;
	context._cmp(context.data.word(23), 190);
	if (context.flags.c()) goto waiting;
	context._add(context.data.byte(54), 1);
	context.ah = context.data.byte(context.bx+7);
	context._and(context.ah, 127);
	context._cmp(context.ah, context.data.byte(66));
	if (context.flags.z()) goto notdiff;
	context.al = context.data.byte(66);
	context.data.byte(context.bx+7) = context.al;
notdiff:
	return;
notwaiting:
	addtopeoplelist(context);
	showgamereel(context);
waiting:
	return;
}

static inline void candles1(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto candle1;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 44);
	if (!context.flags.z()) goto notendcandle1;
	context.ax = 39;
notendcandle1:
	context.data.word(context.bx+3) = context.ax;
candle1:
	showgamereel(context);
	return;
}

static inline void smallcandle(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto smallcandlef;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 37);
	if (!context.flags.z()) goto notendsmallcandle;
	context.ax = 25;
notendsmallcandle:
	context.data.word(context.bx+3) = context.ax;
smallcandlef:
	showgamereel(context);
	return;
}

static inline void intromagic1(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto introm1fin;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 145);
	if (!context.flags.z()) goto gotintrom1;
	context.ax = 121;
gotintrom1:
	context.data.word(context.bx+3) = context.ax;
	context._cmp(context.ax, 121);
	if (!context.flags.z()) goto introm1fin;
	context._add(context.data.byte(139), 1);
	context.push(context.es);
	context.push(context.bx);
	intro1text(context);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.data.byte(139), 8);
	if (!context.flags.z()) goto introm1fin;
	context._add(context.data.byte(149), 10);
	context.data.byte(186) = 1;
introm1fin:
	showgamereel(context);
	return;
}

static inline void candles(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto candlesfin;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 167);
	if (!context.flags.z()) goto gotcandles;
	context.ax = 162;
gotcandles:
	context.data.word(context.bx+3) = context.ax;
candlesfin:
	showgamereel(context);
	return;
}

static inline void candles2(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto candles2fin;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 238);
	if (!context.flags.z()) goto gotcandles2;
	context.ax = 233;
gotcandles2:
	context.data.word(context.bx+3) = context.ax;
candles2fin:
	showgamereel(context);
	return;
}

static inline void gates(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto gatesfin;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 116);
	if (!context.flags.z()) goto notbang;
	context.push(context.ax);
	context.push(context.bx);
	context.push(context.es);
	context.al = 17;
	playchannel1(context);
	context.es = context.pop();
	context.bx = context.pop();
	context.ax = context.pop();
notbang:
	context._cmp(context.ax, 110);
	if (context.flags.c()) goto slowgates;
	context.data.byte(context.bx+5) = 2;
slowgates:
	context._cmp(context.ax, 120);
	if (!context.flags.z()) goto gotgates;
	context.data.byte(103) = 1;
	context.ax = 119;
gotgates:
	context.data.word(context.bx+3) = context.ax;
	context.push(context.es);
	context.push(context.bx);
	intro3text(context);
	context.bx = context.pop();
	context.es = context.pop();
gatesfin:
	showgamereel(context);
	return;
}

static inline void intromagic2(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto introm2fin;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 216);
	if (!context.flags.z()) goto gotintrom2;
	context.ax = 192;
gotintrom2:
	context.data.word(context.bx+3) = context.ax;
introm2fin:
	showgamereel(context);
	return;
}

static inline void intromagic3(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto introm3fin;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 218);
	if (!context.flags.z()) goto gotintrom3;
	context.data.byte(103) = 1;
gotintrom3:
	context.data.word(context.bx+3) = context.ax;
introm3fin:
	showgamereel(context);
	context.al = context.data.byte(148);
	context.data.byte(context.bx+1) = context.al;
	return;
}

static inline void intromonks1(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto intromonk1fin;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 80);
	if (!context.flags.z()) goto notendmonk1;
	context._add(context.data.byte(149), 10);
	context.data.byte(186) = 1;
	showgamereel(context);
	return;
notendmonk1:
	context._cmp(context.ax, 30);
	if (!context.flags.z()) goto gotintromonk1;
	context._sub(context.data.byte(149), 10);
	context.data.byte(186) = 1;
	context.ax = 51;
gotintromonk1:
	context.data.word(context.bx+3) = context.ax;
	context._cmp(context.ax, 5);
	if (context.flags.z()) goto waitstep;
	context._cmp(context.ax, 15);
	if (context.flags.z()) goto waitstep;
	context._cmp(context.ax, 25);
	if (context.flags.z()) goto waitstep;
	context._cmp(context.ax, 61);
	if (context.flags.z()) goto waitstep;
	context._cmp(context.ax, 71);
	if (context.flags.z()) goto waitstep;
	goto intromonk1fin;
waitstep:
	context.push(context.es);
	context.push(context.bx);
	intro2text(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.data.byte(context.bx+6) = -20;
intromonk1fin:
	showgamereel(context);
	context.al = context.data.byte(149);
	context.data.byte(context.bx+2) = context.al;
	return;
}

static inline void intromonks2(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto intromonk2fin;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 87);
	if (!context.flags.z()) goto nottalk1;
	context._add(context.data.byte(139), 1);
	context.push(context.es);
	context.push(context.bx);
	monks2text(context);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.data.byte(139), 19);
	if (!context.flags.z()) goto notlasttalk1;
	context.ax = 87;
	goto gotintromonk2;
notlasttalk1:
	context.ax = 74;
	goto gotintromonk2;
nottalk1:
	context._cmp(context.ax, 110);
	if (!context.flags.z()) goto notraisearm;
	context._add(context.data.byte(139), 1);
	context.push(context.es);
	context.push(context.bx);
	monks2text(context);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.data.byte(139), 35);
	if (!context.flags.z()) goto notlastraise;
	context.ax = 111;
	goto gotintromonk2;
notlastraise:
	context.ax = 98;
	goto gotintromonk2;
notraisearm:
	context._cmp(context.ax, 176);
	if (!context.flags.z()) goto notendmonk2;
	context.data.byte(103) = 1;
	goto gotintromonk2;
notendmonk2:
	context._cmp(context.ax, 125);
	if (!context.flags.z()) goto gotintromonk2;
	context.ax = 140;
gotintromonk2:
	context.data.word(context.bx+3) = context.ax;
intromonk2fin:
	showgamereel(context);
	return;
}

static inline void handclap(Context & context) {
	return;
}

static inline void monks2text(Context & context) {
	context._cmp(context.data.byte(139), 1);
	if (!context.flags.z()) goto notmonk2text1;
	context.al = 8;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text1:
	context._cmp(context.data.byte(139), 4);
	if (!context.flags.z()) goto notmonk2text2;
	context.al = 9;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text2:
	context._cmp(context.data.byte(139), 7);
	if (!context.flags.z()) goto notmonk2text3;
	context.al = 10;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text3:
	context._cmp(context.data.byte(139), 10);
	if (!context.flags.z()) goto notmonk2text4;
	context.al = 11;
	context.bl = 0;
	context.bh = 105;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text4:
	context._cmp(context.data.byte(139), 13);
	if (!context.flags.z()) goto notmonk2text5;
	context.al = 12;
	context.bl = 0;
	context.bh = 120;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text5:
	context._cmp(context.data.byte(139), 16);
	if (!context.flags.z()) goto notmonk2text6;
	context.al = 13;
	context.bl = 0;
	context.bh = 135;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text6:
	context._cmp(context.data.byte(139), 19);
	if (!context.flags.z()) goto notmonk2text7;
	context.al = 14;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	context.dx = 1;
	context.ah = 82;
	{ setuptimedtemp(context); return; };
notmonk2text7:
	context._cmp(context.data.byte(139), 22);
	if (!context.flags.z()) goto notmonk2text8;
	context.al = 15;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text8:
	context._cmp(context.data.byte(139), 25);
	if (!context.flags.z()) goto notmonk2text9;
	context.al = 16;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text9:
	context._cmp(context.data.byte(139), 28);
	if (!context.flags.z()) goto notmonk2text10;
	context.al = 17;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text10:
	context._cmp(context.data.byte(139), 31);
	if (!context.flags.z()) goto notmonk2text11;
	context.al = 18;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text11:
	return;
gotmonks2text:
	context.dx = 1;
	context.cx = 120;
	context.ah = 82;
	setuptimedtemp(context);
	return;
}

static inline void intro1text(Context & context) {
	context._cmp(context.data.byte(139), 2);
	if (!context.flags.z()) goto notintro1text1;
	context.al = 40;
	context.bl = 34;
	context.bh = 130;
	context.cx = 90;
	goto gotintro1text;
notintro1text1:
	context._cmp(context.data.byte(139), 4);
	if (!context.flags.z()) goto notintro1text2;
	context.al = 41;
	context.bl = 34;
	context.bh = 130;
	context.cx = 90;
	goto gotintro1text;
notintro1text2:
	context._cmp(context.data.byte(139), 6);
	if (!context.flags.z()) goto notintro1text3;
	context.al = 42;
	context.bl = 34;
	context.bh = 130;
	context.cx = 90;
	goto gotintro1text;
notintro1text3:
	return;
gotintro1text:
	context.dx = 1;
	context.ah = 82;
oktalk2:
	setuptimedtemp(context);
	return;
}

static inline void intro2text(Context & context) {
	context._cmp(context.ax, 5);
	if (!context.flags.z()) goto notintro2text1;
	context.al = 43;
	context.bl = 34;
	context.bh = 40;
	context.cx = 90;
	goto gotintro2text;
notintro2text1:
	context._cmp(context.ax, 15);
	if (!context.flags.z()) goto notintro2text2;
	context.al = 44;
	context.bl = 34;
	context.bh = 40;
	context.cx = 90;
	goto gotintro2text;
notintro2text2:
	return;
gotintro2text:
	context.dx = 1;
	context.ah = 82;
	setuptimedtemp(context);
	return;
}

static inline void intro3text(Context & context) {
	context._cmp(context.ax, 107);
	if (!context.flags.z()) goto notintro3text1;
	context.al = 45;
	context.bl = 36;
	context.bh = 56;
	context.cx = 100;
	goto gotintro3text;
notintro3text1:
	context._cmp(context.ax, 109);
	if (!context.flags.z()) goto notintro3text2;
	context.al = 46;
	context.bl = 36;
	context.bh = 56;
	context.cx = 100;
	goto gotintro3text;
notintro3text2:
	return;
gotintro3text:
	context.dx = 1;
	context.ah = 82;
	setuptimedtemp(context);
	return;
}

static inline void monkandryan(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto notmonkryan;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 83);
	if (!context.flags.z()) goto gotmonkryan;
	context._add(context.data.byte(139), 1);
	context.push(context.es);
	context.push(context.bx);
	textformonk(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.ax = 77;
	context._cmp(context.data.byte(139), 57);
	if (!context.flags.z()) goto gotmonkryan;
	context.data.byte(103) = 1;
	return;
gotmonkryan:
	context.data.word(context.bx+3) = context.ax;
notmonkryan:
	showgamereel(context);
	return;
}

static inline void endgameseq(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto notendseq;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 51);
	if (!context.flags.z()) goto gotendseq;
	context._cmp(context.data.byte(139), 140);
	if (context.flags.z()) goto gotendseq;
	context._add(context.data.byte(139), 1);
	context.push(context.es);
	context.push(context.bx);
	textforend(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.ax = 50;
gotendseq:
	context.data.word(context.bx+3) = context.ax;
	context._cmp(context.ax, 134);
	if (!context.flags.z()) goto notfadedown;
	context.push(context.es);
	context.push(context.bx);
	context.push(context.ax);
	fadescreendownhalf(context);
	context.ax = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	goto notendseq;
notfadedown:
	context._cmp(context.ax, 324);
	if (!context.flags.z()) goto notfadeend;
	context.push(context.es);
	context.push(context.bx);
	context.push(context.ax);
	fadescreendowns(context);
	context.data.byte(387) = 7;
	context.data.byte(388) = 1;
	context.ax = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
notfadeend:
	context._cmp(context.ax, 340);
	if (!context.flags.z()) goto notendseq;
	context.data.byte(103) = 1;
notendseq:
	showgamereel(context);
	context.al = context.data.byte(149);
	context.data.byte(context.bx+2) = context.al;
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 145);
	if (!context.flags.z()) goto notendcreds;
	context.data.word(context.bx+3) = 146;
	rollendcredits(context);
notendcreds:
	return;
}

static inline void rollendcredits(Context & context) {
	context.al = 16;
	context.ah = 255;
	playchannel0(context);
	context.data.byte(386) = 7;
	context.data.byte(387) = 0;
	context.data.byte(388) = -1;
	context.cl = 160;
	context.ch = 160;
	context.di = 75;
	context.bx = 20;
	context.ds = context.data.word(402);
	context.si = 0;
	multiget(context);
	context.es = context.data.word(466);
	context.si = 3*2;
	context.ax = context.data.word(context.si);
	context.si = context.ax;
	context._add(context.si, 66*2);
	context.cx = 254;
endcredits1:
	context.push(context.cx);
	context.bx = 10;
	context.cx = context.data.word(77);
endcredits2:
	context.push(context.cx);
	context.push(context.si);
	context.push(context.di);
	context.push(context.es);
	context.push(context.bx);
	vsync(context);
	context.cl = 160;
	context.ch = 160;
	context.di = 75;
	context.bx = 20;
	context.ds = context.data.word(402);
	context.si = 0;
	multiput(context);
	vsync(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.di = context.pop();
	context.si = context.pop();
	context.push(context.si);
	context.push(context.di);
	context.push(context.es);
	context.push(context.bx);
	context.cx = 18;
onelot:
	context.push(context.cx);
	context.di = 75;
	context.dx = 161;
	context.ax = 0;
	printdirect(context);
	context._add(context.bx, context.data.word(77));
	context.cx = context.pop();
	if (--context.cx) goto onelot;
	vsync(context);
	context.cl = 160;
	context.ch = 160;
	context.di = 75;
	context.bx = 20;
	multidump(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.di = context.pop();
	context.si = context.pop();
	context.cx = context.pop();
	context._sub(context.bx, 1);
	if (--context.cx) goto endcredits2;
	context.cx = context.pop();
looknext:
	context.al = context.data.byte(context.si);
	context._add(context.si, 1);
	context._cmp(context.al, ':');
	if (context.flags.z()) goto gotnext;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto gotnext;
	goto looknext;
gotnext:
	if (--context.cx) goto endcredits1;
	context.cx = 100;
	hangon(context);
	paneltomap(context);
	fadescreenuphalf(context);
	return;
}

static inline void priest(Context & context) {
	context._cmp(context.data.word(context.bx+3), 8);
	if (context.flags.z()) goto priestspoken;
	context.data.byte(234) = 0;
	context.data.word(21) = 2;
	checkspeed(context);
	if (!context.flags.z()) goto priestwait;
	context._add(context.data.word(context.bx+3), 1);
	context.push(context.es);
	context.push(context.bx);
	priesttext(context);
	context.bx = context.pop();
	context.es = context.pop();
priestwait:
	return;
priestspoken:
	return;
}

static inline void madmanstelly(Context & context) {
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 307);
	if (!context.flags.z()) goto notendtelly;
	context.ax = 300;
notendtelly:
	context.data.word(context.bx+3) = context.ax;
	showgamereel(context);
	return;
}

static inline void madman(Context & context) {
	context.data.word(21) = 2;
	checkspeed(context);
	if (!context.flags.z()) goto nomadspeed;
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 364);
	if (!context.flags.c()) goto ryansded;
	context._cmp(context.ax, 10);
	if (!context.flags.z()) goto notfirstmad;
	context.push(context.es);
	context.push(context.bx);
	context.push(context.ax);
	context.dx = 2260;
	loadtemptext(context);
	context.ax = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	context.data.byte(64) = -1;
	context.data.byte(69) = 0;
notfirstmad:
	context._add(context.ax, 1);
	context._cmp(context.ax, 294);
	if (context.flags.z()) goto madmanspoken;
	context._cmp(context.ax, 66);
	if (!context.flags.z()) goto nomadspeak;
	context._add(context.data.byte(64), 1);
	context.push(context.es);
	context.push(context.bx);
	madmantext(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.ax = 53;
	context._cmp(context.data.byte(64), 62);
	if (context.flags.c()) goto nomadspeak;
	context._cmp(context.data.byte(64), 68);
	if (context.flags.z()) goto killryan;
	context._cmp(context.data.byte(65), 8);
	if (!context.flags.z()) goto nomadspeak;
	context.data.byte(64) = 70;
	context.data.byte(65) = -1;
	context.data.byte(53) = 1;
	context.ax = 67;
	goto nomadspeak;
killryan:
	context.ax = 310;
nomadspeak:
	context.data.word(context.bx+3) = context.ax;
nomadspeed:
	showgamereel(context);
	context.al = context.data.byte(148);
	context.data.byte(context.bx+1) = context.al;
	madmode(context);
	return;
madmanspoken:
	context._cmp(context.data.byte(391), 1);
	if (context.flags.z()) goto alreadywon;
	context.data.byte(391) = 1;
	context.push(context.es);
	context.push(context.bx);
	getridoftemptext(context);
	context.bx = context.pop();
	context.es = context.pop();
alreadywon:
	return;
ryansded:
	context.data.byte(56) = 2;
	showgamereel(context);
	return;
}

static inline void madmantext(Context & context) {
	context._cmp(context.data.byte(64), 61);
	if (!context.flags.c()) goto nomadtext;
	context.al = context.data.byte(64);
	context._and(context.al, 3);
	if (!context.flags.z()) goto nomadtext;
	context.al = context.data.byte(64);
	context._shr(context.al, 1);
	context._shr(context.al, 1);
	context._add(context.al, 47);
	context.bl = 72;
	context.bh = 80;
	context.cx = 90;
	context.dx = 1;
	context.ah = 82;
	setuptimedtemp(context);
nomadtext:
	return;
}

static inline void madmode(Context & context) {
	context.data.word(21) = 2;
	context.data.byte(234) = 0;
	context._cmp(context.data.byte(64), 63);
	if (context.flags.c()) goto iswatchmad;
	context._cmp(context.data.byte(64), 68);
	if (!context.flags.c()) goto iswatchmad;
	context.data.byte(234) = 2;
iswatchmad:
	return;
}

static inline void priesttext(Context & context) {
	context._cmp(context.data.word(context.bx+3), 2);
	if (context.flags.c()) goto nopriesttext;
	context._cmp(context.data.word(context.bx+3), 7);
	if (!context.flags.c()) goto nopriesttext;
	context.al = context.data.byte(context.bx+3);
	context._and(context.al, 1);
	if (!context.flags.z()) goto nopriesttext;
	context.al = context.data.byte(context.bx+3);
	context._shr(context.al, 1);
	context._add(context.al, 50);
	context.bl = 72;
	context.bh = 80;
	context.cx = 54;
	context.dx = 1;
	setuptimeduse(context);
nopriesttext:
	return;
}

static inline void textforend(Context & context) {
	context._cmp(context.data.byte(139), 20);
	if (!context.flags.z()) goto notendtext1;
	context.al = 0;
	context.bl = 34;
	context.bh = 20;
	context.cx = 60;
	goto gotendtext;
notendtext1:
	context._cmp(context.data.byte(139), 65);
	if (!context.flags.z()) goto notendtext2;
	context.al = 1;
	context.bl = 34;
	context.bh = 20;
	context.cx = 60;
	goto gotendtext;
notendtext2:
	context._cmp(context.data.byte(139), 110);
	if (!context.flags.z()) goto notendtext3;
	context.al = 2;
	context.bl = 34;
	context.bh = 20;
	context.cx = 60;
	goto gotendtext;
notendtext3:
	return;
gotendtext:
	context.dx = 1;
	context.ah = 83;
	setuptimedtemp(context);
	return;
}

static inline void textformonk(Context & context) {
	context._cmp(context.data.byte(139), 1);
	if (!context.flags.z()) goto notmonktext1;
	context.al = 19;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext1:
	context._cmp(context.data.byte(139), 5);
	if (!context.flags.z()) goto notmonktext2;
	context.al = 20;
	context.bl = 68;
	context.bh = 38;
	context.cx = 120;
	goto gotmonktext;
notmonktext2:
	context._cmp(context.data.byte(139), 9);
	if (!context.flags.z()) goto notmonktext3;
	context.al = 21;
	context.bl = 48;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext3:
	context._cmp(context.data.byte(139), 13);
	if (!context.flags.z()) goto notmonktext4;
	context.al = 22;
	context.bl = 68;
	context.bh = 38;
	context.cx = 120;
	goto gotmonktext;
notmonktext4:
	context._cmp(context.data.byte(139), 17);
	if (!context.flags.z()) goto notmonktext5;
	context.al = 23;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext5:
	context._cmp(context.data.byte(139), 21);
	if (!context.flags.z()) goto notmonktext6;
	context.al = 24;
	context.bl = 68;
	context.bh = 38;
	context.cx = 120;
	goto gotmonktext;
notmonktext6:
	context._cmp(context.data.byte(139), 25);
	if (!context.flags.z()) goto notmonktext7;
	context.al = 25;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext7:
	context._cmp(context.data.byte(139), 29);
	if (!context.flags.z()) goto notmonktext8;
	context.al = 26;
	context.bl = 68;
	context.bh = 38;
	context.cx = 120;
	goto gotmonktext;
notmonktext8:
	context._cmp(context.data.byte(139), 33);
	if (!context.flags.z()) goto notmonktext9;
	context.al = 27;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext9:
	context._cmp(context.data.byte(139), 37);
	if (!context.flags.z()) goto notmonktext10;
	context.al = 28;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext10:
	context._cmp(context.data.byte(139), 41);
	if (!context.flags.z()) goto notmonktext11;
	context.al = 29;
	context.bl = 68;
	context.bh = 38;
	context.cx = 120;
	goto gotmonktext;
notmonktext11:
	context._cmp(context.data.byte(139), 45);
	if (!context.flags.z()) goto notmonktext12;
	context.al = 30;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext12:
	context._cmp(context.data.byte(139), 49);
	if (!context.flags.z()) goto notmonktext13;
	context.al = 31;
	context.bl = 68;
	context.bh = 154;
	context.cx = 220;
	goto gotmonktext;
notmonktext13:
	context._cmp(context.data.byte(139), 53);
	if (!context.flags.z()) goto notendtitles;
	fadescreendowns(context);
notendtitles:
	return;
gotmonktext:
	context.dx = 1;
	context.ah = 82;
oktalk:
	setuptimedtemp(context);
	return;
}

static inline void drunk(Context & context) {
	context._cmp(context.data.byte(42), 0);
	if (!context.flags.z()) goto trampgone;
	context.al = context.data.byte(context.bx+7);
	context._and(context.al, 127);
	context.data.byte(context.bx+7) = context.al;
	showgamereel(context);
	addtopeoplelist(context);
trampgone:
	return;
}

static inline void advisor(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto noadvisor;
	goto noadvisor;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 123);
	if (!context.flags.z()) goto notendadvis;
	context.ax = 106;
	goto gotadvframe;
notendadvis:
	context._cmp(context.ax, 108);
	if (!context.flags.z()) goto gotadvframe;
	context.push(context.ax);
	randomnumber(context);
	context.cl = context.al;
	context.ax = context.pop();
	context._cmp(context.cl, 3);
	if (context.flags.c()) goto gotadvframe;
	context.ax = 106;
gotadvframe:
	context.data.word(context.bx+3) = context.ax;
noadvisor:
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void copper(Context & context) {
	checkspeed(context);
	if (!context.flags.z()) goto nocopper;
	context.ax = context.data.word(context.bx+3);
	context._add(context.ax, 1);
	context._cmp(context.ax, 94);
	if (!context.flags.z()) goto notendcopper;
	context.ax = 64;
	goto gotcopframe;
notendcopper:
	context._cmp(context.ax, 81);
	if (context.flags.z()) goto mightwait;
	context._cmp(context.ax, 66);
	if (!context.flags.z()) goto gotcopframe;
mightwait:
	context.push(context.ax);
	randomnumber(context);
	context.cl = context.al;
	context.ax = context.pop();
	context._cmp(context.cl, 7);
	if (context.flags.c()) goto gotcopframe;
	context._sub(context.ax, 1);
gotcopframe:
	context.data.word(context.bx+3) = context.ax;
nocopper:
	showgamereel(context);
	addtopeoplelist(context);
	return;
}

static inline void sparky(Context & context) {
	context._cmp(context.data.word(16), 0);
	if (context.flags.z()) goto animsparky;
	context.data.byte(context.bx+7) = 3;
	goto animsparky;
animsparky:
	checkspeed(context);
	if (!context.flags.z()) goto finishsparky;
	context._cmp(context.data.word(context.bx+3), 34);
	if (!context.flags.z()) goto notsparky1;
	randomnumber(context);
	context._cmp(context.al, 30);
	if (context.flags.c()) goto dosparky;
	context.data.word(context.bx+3) = 27;
	goto finishsparky;
notsparky1:
	context._cmp(context.data.word(context.bx+3), 48);
	if (!context.flags.z()) goto dosparky;
	context.data.word(context.bx+3) = 27;
	goto finishsparky;
dosparky:
	context._add(context.data.word(context.bx+3), 1);
finishsparky:
	showgamereel(context);
	addtopeoplelist(context);
	context.al = context.data.byte(context.bx+7);
	context._and(context.al, 128);
	if (context.flags.z()) goto nottalkedsparky;
	context.data.byte(49) = 1;
nottalkedsparky:
	return;
}

static inline void train(Context & context) {
	return;
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 21);
	if (!context.flags.c()) goto notrainyet;
	context._add(context.ax, 1);
	goto gottrainframe;
notrainyet:
	randomnumber(context);
	context._cmp(context.al, 253);
	if (context.flags.c()) goto notrainatall;
	context._cmp(context.data.byte(475), 5);
	if (!context.flags.z()) goto notrainatall;
	context._cmp(context.data.byte(477), 5);
	if (!context.flags.z()) goto notrainatall;
	context.ax = 5;
gottrainframe:
	context.data.word(context.bx+3) = context.ax;
	showgamereel(context);
notrainatall:
	return;
}

static inline void addtopeoplelist(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context.push(context.bx);
	context.cl = context.data.byte(context.bx+7);
	context.ax = context.data.word(context.bx+3);
	context.bx = context.data.word(18);
	context.es = context.data.word(412);
	context.data.word(context.bx) = context.ax;
	context.ax = context.pop();
	context.data.word(context.bx+2) = context.ax;
	context.data.byte(context.bx+4) = context.cl;
	context.bx = context.pop();
	context.es = context.pop();
	context._add(context.data.word(18), 5);
	return;
}

static inline void showgamereel(Context & context) {
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.ax, 512);
	if (!context.flags.c()) goto noshow;
	context.data.word(239) = context.ax;
	context.push(context.es);
	context.push(context.bx);
	plotreel(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.ax = context.data.word(239);
	context.data.word(context.bx+3) = context.ax;
noshow:
	return;
}

static inline void checkspeed(Context & context) {
	context._cmp(context.data.byte(65), -1);
	if (!context.flags.z()) goto forcenext;
	context._add(context.data.byte(context.bx+6), 1);
	context.al = context.data.byte(context.bx+6);
	context._cmp(context.al, context.data.byte(context.bx+5));
	if (!context.flags.z()) goto notspeed;
	context.al = 0;
	context.data.byte(context.bx+6) = context.al;
	context._cmp(context.al, context.al);
notspeed:
	return;
forcenext:
	context._cmp(context.al, context.al);
	return;
}

static inline void delsprite(Context & context) {
	context.di = context.bx;
	context.cx = 32;
	context.al = 255;
	while(--context.cx) 	context._stosb();
	return;
}

static inline void mainman(Context & context) {
	context._cmp(context.data.byte(187), 1);
	if (!context.flags.z()) goto notinnewroom;
	context.data.byte(187) = 0;
	context.al = context.data.byte(151);
	context.ah = context.data.byte(152);
	context.data.word(context.bx+10) = context.ax;
	context.data.byte(context.bx+29) = 0;
	goto executewalk;
notinnewroom:
	context._sub(context.data.byte(context.bx+22), 1);
	context._cmp(context.data.byte(context.bx+22), -1);
	if (context.flags.z()) goto executewalk;
	return;
executewalk:
	context.data.byte(context.bx+22) = 0;
	context.al = context.data.byte(135);
	context._cmp(context.al, context.data.byte(133));
	if (context.flags.z()) goto facingok;
	aboutturn(context);
	goto notwalk;
facingok:
	context._cmp(context.data.byte(136), 0);
	if (context.flags.z()) goto alreadyturned;
	context._cmp(context.data.byte(492), 254);
	if (!context.flags.z()) goto alreadyturned;
	context.data.byte(473) = 1;
	context.al = context.data.byte(133);
	context._cmp(context.al, context.data.byte(134));
	if (!context.flags.z()) goto alreadyturned;
	checkforexit(context);
alreadyturned:
	context.data.byte(136) = 0;
	context._cmp(context.data.byte(492), 254);
	if (!context.flags.z()) goto walkman;
	context.data.byte(context.bx+29) = 0;
	goto notwalk;
walkman:
	context.al = context.data.byte(context.bx+29);
	context._add(context.al, 1);
	context._cmp(context.al, 11);
	if (!context.flags.z()) goto notanimend1;
	context.al = 1;
notanimend1:
	context.data.byte(context.bx+29) = context.al;
	walking(context);
	context._cmp(context.data.byte(492), 254);
	if (context.flags.z()) goto afterwalk;
	context.al = context.data.byte(133);
	context._and(context.al, 1);
	if (context.flags.z()) goto isdouble;
	context.al = context.data.byte(context.bx+29);
	context._cmp(context.al, 2);
	if (context.flags.z()) goto afterwalk;
	context._cmp(context.al, 7);
	if (context.flags.z()) goto afterwalk;
isdouble:
	walking(context);
afterwalk:
	context._cmp(context.data.byte(492), 254);
	if (!context.flags.z()) goto notwalk;
	context.al = context.data.byte(135);
	context._cmp(context.al, context.data.byte(133));
	if (!context.flags.z()) goto notwalk;
	context.data.byte(473) = 1;
	context.al = context.data.byte(133);
	context._cmp(context.al, context.data.byte(134));
	if (!context.flags.z()) goto notwalk;
	checkforexit(context);
notwalk:
	context.al = context.data.byte(133);
	context.ah = 0;
	context.di = 1105;
	context._add(context.di, context.ax);
	context.al = context.data.byte(context.di);
	context._add(context.al, context.data.byte(context.bx+29));
	context.data.byte(context.bx+15) = context.al;
	context.ax = context.data.word(context.bx+10);
	context.data.byte(151) = context.al;
	context.data.byte(152) = context.ah;
	return;
}

static inline void aboutturn(Context & context) {
	context._cmp(context.data.byte(136), 1);
	if (context.flags.z()) goto incdir;
	context._cmp(context.data.byte(136), -1);
	if (context.flags.z()) goto decdir;
	context.al = context.data.byte(133);
	context._sub(context.al, context.data.byte(135));
	if (!context.flags.c()) goto higher;
	context._neg(context.al);
	context._cmp(context.al, 4);
	if (!context.flags.c()) goto decdir;
	goto incdir;
higher:
	context._cmp(context.al, 4);
	if (!context.flags.c()) goto incdir;
	goto decdir;
incdir:
	context.data.byte(136) = 1;
	context.al = context.data.byte(133);
	context._add(context.al, 1);
	context._and(context.al, 7);
	context.data.byte(133) = context.al;
	context.data.byte(context.bx+29) = 0;
	return;
decdir:
	context.data.byte(136) = -1;
	context.al = context.data.byte(133);
	context._sub(context.al, 1);
	context._and(context.al, 7);
	context.data.byte(133) = context.al;
	context.data.byte(context.bx+29) = 0;
	return;
}

static inline void walking(Context & context) {
	context._cmp(context.data.byte(493), 0);
	if (context.flags.z()) goto normalwalk;
	context.al = context.data.byte(492);
	context._sub(context.al, 1);
	context.data.byte(492) = context.al;
	context._cmp(context.al, 200);
	if (!context.flags.c()) goto endofline;
	goto continuewalk;
normalwalk:
	context.al = context.data.byte(492);
	context._add(context.al, 1);
	context.data.byte(492) = context.al;
	context._cmp(context.al, context.data.byte(494));
	if (!context.flags.c()) goto endofline;
continuewalk:
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.push(context.es);
	context.push(context.bx);
	context.dx = context.ds;
	context.es = context.dx;
	context.bx = 8344;
	context._add(context.bx, context.ax);
	context.ax = context.data.word(context.bx);
	context.bx = context.pop();
	context.es = context.pop();
stillline:
	context.data.word(context.bx+10) = context.ax;
	return;
endofline:
	context.data.byte(492) = 254;
	context.al = context.data.byte(478);
	context.data.byte(475) = context.al;
	context._cmp(context.al, context.data.byte(477));
	if (context.flags.z()) goto finishedwalk;
	context.al = context.data.byte(477);
	context.data.byte(478) = context.al;
	context.push(context.es);
	context.push(context.bx);
	autosetwalk(context);
	context.bx = context.pop();
	context.es = context.pop();
	return;
finishedwalk:
	facerightway(context);
	return;
}

static inline void checkforexit(Context & context) {
	context.cl = context.data.byte(151);
	context._add(context.cl, 12);
	context.ch = context.data.byte(152);
	context._add(context.ch, 12);
	checkone(context);
	context.data.byte(153) = context.cl;
	context.data.byte(154) = context.ch;
	context.data.byte(155) = context.dl;
	context.data.byte(156) = context.dh;
	context.al = context.data.byte(153);
	context._test(context.al, 64);
	if (context.flags.z()) goto notnewdirect;
	context.al = context.data.byte(154);
	context.data.byte(189) = context.al;
	return;
notnewdirect:
	context._test(context.al, 32);
	if (context.flags.z()) goto notleave;
	context.push(context.es);
	context.push(context.bx);
	context._cmp(context.data.byte(184), 2);
	if (!context.flags.z()) goto notlouis;
	context.bl = 0;
	context.push(context.bx);
	context.al = 'W';
	context.ah = 'E';
	context.cl = 'T';
	context.ch = 'A';
	isryanholding(context);
	context.bx = context.pop();
	if (context.flags.z()) goto noshoe1;
	context._add(context.bl, 1);
noshoe1:
	context.push(context.bx);
	context.al = 'W';
	context.ah = 'E';
	context.cl = 'T';
	context.ch = 'B';
	isryanholding(context);
	context.bx = context.pop();
	if (context.flags.z()) goto noshoe2;
	context._add(context.bl, 1);
noshoe2:
	context._cmp(context.bl, 2);
	if (context.flags.z()) goto notlouis;
	context.al = 42;
	context._cmp(context.bl, 0);
	if (context.flags.z()) goto notravmessage;
	context._add(context.al, 1);
notravmessage:
	context.cx = 80;
	context.dx = 10;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	context.al = context.data.byte(133);
	context._add(context.al, 4);
	context._and(context.al, 7);
	context.data.byte(135) = context.al;
	context.bx = context.pop();
	context.es = context.pop();
	return;
notlouis:
	context.bx = context.pop();
	context.es = context.pop();
	context.data.byte(60) = 1;
	return;
notleave:
	context._test(context.al, 4);
	if (context.flags.z()) goto notaleft;
	adjustleft(context);
	return;
notaleft:
	context._test(context.al, 2);
	if (context.flags.z()) goto notaright;
	adjustright(context);
	return;
notaright:
	context._test(context.al, 8);
	if (context.flags.z()) goto notadown;
	adjustdown(context);
	return;
notadown:
	context._test(context.al, 16);
	if (context.flags.z()) goto notanup;
	adjustup(context);
	return;
notanup:
	return;
}

static inline void adjustdown(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context._add(context.data.byte(149), 10);
	context.al = context.data.byte(154);
	context.cl = 16;
	context._mul(context.cl);
	context.data.byte(context.bx+11) = context.al;
	context.data.byte(186) = 1;
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void adjustup(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context._sub(context.data.byte(149), 10);
	context.al = context.data.byte(154);
	context.cl = 16;
	context._mul(context.cl);
	context.data.byte(context.bx+11) = context.al;
	context.data.byte(186) = 1;
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void adjustleft(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context.data.byte(153) = 0;
	context._sub(context.data.byte(148), 11);
	context.al = context.data.byte(154);
	context.cl = 16;
	context._mul(context.cl);
	context.data.byte(context.bx+10) = context.al;
	context.data.byte(186) = 1;
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void adjustright(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context._add(context.data.byte(148), 11);
	context.al = context.data.byte(154);
	context.cl = 16;
	context._mul(context.cl);
	context._sub(context.al, 2);
	context.data.byte(context.bx+10) = context.al;
	context.data.byte(186) = 1;
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void backobject(Context & context) {
	context.ds = context.data.word(428);
	context.di = context.data.word(context.bx+20);
	context.al = context.data.byte(context.bx+18);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto _tmp48z;
	context._sub(context.al, 1);
	context.data.byte(context.bx+18) = context.al;
	goto finishback;
_tmp48z:
	context.al = context.data.byte(context.di+7);
	context.data.byte(context.bx+18) = context.al;
	context.al = context.data.byte(context.di+8);
	context._cmp(context.al, 6);
	if (!context.flags.z()) goto notwidedoor;
	widedoor(context);
	goto finishback;
notwidedoor:
	context._cmp(context.al, 5);
	if (!context.flags.z()) goto notrandom;
	random(context);
	goto finishback;
notrandom:
	context._cmp(context.al, 4);
	if (!context.flags.z()) goto notlockdoor;
	lockeddoorway(context);
	goto finishback;
notlockdoor:
	context._cmp(context.al, 3);
	if (!context.flags.z()) goto notlift;
	liftsprite(context);
	goto finishback;
notlift:
	context._cmp(context.al, 2);
	if (!context.flags.z()) goto notdoor;
	doorway(context);
	goto finishback;
notdoor:
	context._cmp(context.al, 1);
	if (!context.flags.z()) goto steadyob;
	constant(context);
	goto finishback;
steadyob:
	steady(context);
finishback:
	return;
}

static inline void liftsprite(Context & context) {
	context.al = context.data.byte(35);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto liftclosed;
	context._cmp(context.al, 1);
	if (context.flags.z()) goto liftopen;
	context._cmp(context.al, 3);
	if (context.flags.z()) goto openlift;
	context.al = context.data.byte(context.bx+19);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto finishclose;
	context._sub(context.al, 1);
	context._cmp(context.al, 11);
	if (!context.flags.z()) goto pokelift;
	context.push(context.ax);
	context.al = 3;
	liftnoise(context);
	context.ax = context.pop();
	goto pokelift;
finishclose:
	context.data.byte(35) = 0;
	return;
openlift:
	context.al = context.data.byte(context.bx+19);
	context._cmp(context.al, 12);
	if (context.flags.z()) goto endoflist;
	context._add(context.al, 1);
	context._cmp(context.al, 1);
	if (!context.flags.z()) goto pokelift;
	context.push(context.ax);
	context.al = 2;
	liftnoise(context);
	context.ax = context.pop();
pokelift:
	context.data.byte(context.bx+19) = context.al;
	context.ah = 0;
	context.push(context.di);
	context._add(context.di, context.ax);
	context.al = context.data.byte(context.di+18);
	context.di = context.pop();
	context.data.byte(context.bx+15) = context.al;
	context.data.byte(context.di+17) = context.al;
	return;
endoflist:
	context.data.byte(35) = 1;
	return;
liftopen:
	context.al = context.data.byte(36);
	context.push(context.es);
	context.push(context.bx);
	turnpathon(context);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.data.byte(40), 0);
	if (context.flags.z()) goto nocountclose;
	context._sub(context.data.byte(40), 1);
	context._cmp(context.data.byte(40), 0);
	if (!context.flags.z()) goto nocountclose;
	context.data.byte(35) = 2;
nocountclose:
	context.al = 12;
	goto pokelift;
liftclosed:
	context.al = context.data.byte(36);
	context.push(context.es);
	context.push(context.bx);
	turnpathoff(context);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.data.byte(39), 0);
	if (context.flags.z()) goto nocountopen;
	context._sub(context.data.byte(39), 1);
	context._cmp(context.data.byte(39), 0);
	if (!context.flags.z()) goto nocountopen;
	context.data.byte(35) = 3;
nocountopen:
	context.al = 0;
	goto pokelift;
}

static inline void liftnoise(Context & context) {
	context._cmp(context.data.byte(184), 5);
	if (context.flags.z()) goto hissnoise;
	context._cmp(context.data.byte(184), 21);
	if (context.flags.z()) goto hissnoise;
	playchannel1(context);
	return;
hissnoise:
	context.al = 13;
	playchannel1(context);
	return;
}

static inline void random(Context & context) {
	randomnum1(context);
	context.push(context.di);
	context._and(context.ax, 7);
	context._add(context.di, 18);
	context._add(context.di, context.ax);
	context.al = context.data.byte(context.di);
	context.di = context.pop();
	context.data.byte(context.bx+15) = context.al;
	return;
}

static inline void steady(Context & context) {
	context.al = context.data.byte(context.di+18);
	context.data.byte(context.di+17) = context.al;
	context.data.byte(context.bx+15) = context.al;
	return;
}

static inline void constant(Context & context) {
	context._add(context.data.byte(context.bx+19), 1);
	context.cl = context.data.byte(context.bx+19);
	context.ch = 0;
	context._add(context.di, context.cx);
	context._cmp(context.data.byte(context.di+18), 255);
	if (!context.flags.z()) goto gotconst;
	context._sub(context.di, context.cx);
	context.cx = 0;
	context.data.byte(context.bx+19) = context.cl;
gotconst:
	context.al = context.data.byte(context.di+18);
	context._sub(context.di, context.cx);
	context.data.byte(context.bx+15) = context.al;
	context.data.byte(context.di+17) = context.al;
	return;
}

static inline void doorway(Context & context) {
	context.data.byte(193) = -24;
	context.data.byte(194) = 10;
	context.data.byte(195) = -30;
	context.data.byte(196) = 10;
	dodoor(context);
	return;
}

static inline void widedoor(Context & context) {
	context.data.byte(193) = -24;
	context.data.byte(194) = 24;
	context.data.byte(195) = -30;
	context.data.byte(196) = 24;
	dodoor(context);
	return;
}

static inline void dodoor(Context & context) {
	context.al = context.data.byte(151);
	context.ah = context.data.byte(152);
	context.cl = context.data.byte(context.bx+10);
	context.ch = context.data.byte(context.bx+11);
	context._cmp(context.al, context.cl);
	if (!context.flags.c()) goto rtofdoor;
	context._sub(context.al, context.cl);
	context._cmp(context.al, context.data.byte(193));
	if (!context.flags.c()) goto upordown;
	goto shutdoor;
rtofdoor:
	context._sub(context.al, context.cl);
	context._cmp(context.al, context.data.byte(194));
	if (!context.flags.c()) goto shutdoor;
upordown:
	context._cmp(context.ah, context.ch);
	if (!context.flags.c()) goto botofdoor;
	context._sub(context.ah, context.ch);
	context._cmp(context.ah, context.data.byte(195));
	if (context.flags.c()) goto shutdoor;
	goto opendoor;
botofdoor:
	context._sub(context.ah, context.ch);
	context._cmp(context.ah, context.data.byte(196));
	if (!context.flags.c()) goto shutdoor;
opendoor:
	context.cl = context.data.byte(context.bx+19);
	context._cmp(context.data.byte(61), 1);
	if (!context.flags.z()) goto notthrough;
	context._cmp(context.cl, 0);
	if (!context.flags.z()) goto notthrough;
	context.cl = 6;
notthrough:
	context._add(context.cl, 1);
	context._cmp(context.cl, 1);
	if (!context.flags.z()) goto notdoorsound2;
	context.al = 0;
	context._cmp(context.data.byte(184), 5);
	if (!context.flags.z()) goto nothoteldoor2;
	context.al = 13;
nothoteldoor2:
	playchannel1(context);
notdoorsound2:
	context.ch = 0;
	context.push(context.di);
	context._add(context.di, context.cx);
	context.al = context.data.byte(context.di+18);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto atlast1;
	context._sub(context.di, 1);
	context._sub(context.cl, 1);
atlast1:
	context.data.byte(context.bx+19) = context.cl;
	context.al = context.data.byte(context.di+18);
	context.di = context.pop();
	context.data.byte(context.bx+15) = context.al;
	context.data.byte(context.di+17) = context.al;
	context.data.byte(61) = 1;
	return;
shutdoor:
	context.cl = context.data.byte(context.bx+19);
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto notdoorsound1;
	context.al = 1;
	context._cmp(context.data.byte(184), 5);
	if (!context.flags.z()) goto nothoteldoor1;
	context.al = 13;
nothoteldoor1:
	playchannel1(context);
notdoorsound1:
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto atlast2;
	context._sub(context.cl, 1);
	context.data.byte(context.bx+19) = context.cl;
atlast2:
	context.ch = 0;
	context.push(context.di);
	context._add(context.di, context.cx);
	context.al = context.data.byte(context.di+18);
	context.di = context.pop();
	context.data.byte(context.bx+15) = context.al;
	context.data.byte(context.di+17) = context.al;
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto notnearly;
	context.data.byte(61) = 0;
notnearly:
	return;
}

static inline void lockeddoorway(Context & context) {
	context.al = context.data.byte(151);
	context.ah = context.data.byte(152);
	context.cl = context.data.byte(context.bx+10);
	context.ch = context.data.byte(context.bx+11);
	context._cmp(context.al, context.cl);
	if (!context.flags.c()) goto rtofdoor2;
	context._sub(context.al, context.cl);
	context._cmp(context.al, -24);
	if (!context.flags.c()) goto upordown2;
	goto shutdoor2;
rtofdoor2:
	context._sub(context.al, context.cl);
	context._cmp(context.al, 10);
	if (!context.flags.c()) goto shutdoor2;
upordown2:
	context._cmp(context.ah, context.ch);
	if (!context.flags.c()) goto botofdoor2;
	context._sub(context.ah, context.ch);
	context._cmp(context.ah, -30);
	if (context.flags.c()) goto shutdoor2;
	goto opendoor2;
botofdoor2:
	context._sub(context.ah, context.ch);
	context._cmp(context.ah, 12);
	if (!context.flags.c()) goto shutdoor2;
opendoor2:
	context._cmp(context.data.byte(61), 1);
	if (context.flags.z()) goto mustbeopen;
	context._cmp(context.data.byte(37), 1);
	if (context.flags.z()) goto shutdoor;
mustbeopen:
	context.cl = context.data.byte(context.bx+19);
	context._cmp(context.cl, 1);
	if (!context.flags.z()) goto notdoorsound4;
	context.al = 0;
	playchannel1(context);
notdoorsound4:
	context._cmp(context.cl, 6);
	if (!context.flags.z()) goto noturnonyet;
	context.al = context.data.byte(38);
	context.push(context.es);
	context.push(context.bx);
	turnpathon(context);
	context.bx = context.pop();
	context.es = context.pop();
noturnonyet:
	context.cl = context.data.byte(context.bx+19);
	context._cmp(context.data.byte(61), 1);
	if (!context.flags.z()) goto notthrough2;
	context._cmp(context.cl, 0);
	if (!context.flags.z()) goto notthrough2;
	context.cl = 6;
notthrough2:
	context._add(context.cl, 1);
	context.ch = 0;
	context.push(context.di);
	context._add(context.di, context.cx);
	context.al = context.data.byte(context.di+18);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto atlast3;
	context._sub(context.di, 1);
	context._sub(context.cl, 1);
atlast3:
	context.data.byte(context.bx+19) = context.cl;
	context.al = context.data.byte(context.di+18);
	context.di = context.pop();
	context.data.byte(context.bx+15) = context.al;
	context.data.byte(context.di+17) = context.al;
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto justshutting;
	context.data.byte(61) = 1;
justshutting:
	return;
shutdoor2:
	context.cl = context.data.byte(context.bx+19);
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto notdoorsound3;
	context.al = 1;
	playchannel1(context);
notdoorsound3:
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto atlast4;
	context._sub(context.cl, 1);
	context.data.byte(context.bx+19) = context.cl;
atlast4:
	context.ch = 0;
	context.data.byte(61) = 0;
	context.push(context.di);
	context._add(context.di, context.cx);
	context.al = context.data.byte(context.di+18);
	context.di = context.pop();
	context.data.byte(context.bx+15) = context.al;
	context.data.byte(context.di+17) = context.al;
	context._cmp(context.cl, 0);
	if (!context.flags.z()) goto notlocky;
	context.al = context.data.byte(38);
	context.push(context.es);
	context.push(context.bx);
	turnpathoff(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.data.byte(37) = 1;
notlocky:
	return;
/*continuing to unbounded code: shutdoor from dodoor:60-87*/
shutdoor:
	context.cl = context.data.byte(context.bx+19);
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto notdoorsound1;
	context.al = 1;
	context._cmp(context.data.byte(184), 5);
	if (!context.flags.z()) goto nothoteldoor1;
	context.al = 13;
nothoteldoor1:
	playchannel1(context);
notdoorsound1:
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto atlast2;
	context._sub(context.cl, 1);
	context.data.byte(context.bx+19) = context.cl;
atlast2:
	context.ch = 0;
	context.push(context.di);
	context._add(context.di, context.cx);
	context.al = context.data.byte(context.di+18);
	context.di = context.pop();
	context.data.byte(context.bx+15) = context.al;
	context.data.byte(context.di+17) = context.al;
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto notnearly;
	context.data.byte(61) = 0;
notnearly:
	return;
	return;
}

static inline void getreelframeax(Context & context) {
	context.push(context.ds);
	context.data.word(160) = context.ax;
	findsource(context);
	context.es = context.ds;
	context.ds = context.pop();
	context.ax = context.data.word(160);
	context._sub(context.ax, context.data.word(244));
	context._add(context.ax, context.ax);
	context.cx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.cx);
	context.bx = context.ax;
	return;
}

static inline void dumpeverything(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40);
dumpevery1:
	context.ax = context.data.word(context.bx);
	context.cx = context.data.word(context.bx+2);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto finishevery1;
	context._cmp(context.ax, context.data.word(context.bx+(40*5)));
	if (!context.flags.z()) goto notskip1;
	context._cmp(context.cx, context.data.word(context.bx+(40*5)+2));
	if (context.flags.z()) goto skip1;
notskip1:
	context.push(context.bx);
	context.push(context.es);
	context.push(context.ds);
	context.bl = context.ah;
	context.bh = 0;
	context.ah = 0;
	context.di = context.ax;
	context._add(context.di, context.data.word(117));
	context._add(context.bx, context.data.word(119));
	multidump(context);
	context.ds = context.pop();
	context.es = context.pop();
	context.bx = context.pop();
skip1:
	context._add(context.bx, 5);
	goto dumpevery1;
finishevery1:
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(40*5);
dumpevery2:
	context.ax = context.data.word(context.bx);
	context.cx = context.data.word(context.bx+2);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto finishevery2;
	context.push(context.bx);
	context.push(context.es);
	context.push(context.ds);
	context.bl = context.ah;
	context.bh = 0;
	context.ah = 0;
	context.di = context.ax;
	context._add(context.di, context.data.word(117));
	context._add(context.bx, context.data.word(119));
	multidump(context);
	context.ds = context.pop();
	context.es = context.pop();
	context.bx = context.pop();
	context._add(context.bx, 5);
	goto dumpevery2;
finishevery2:
	return;
}

static inline void readabyte(Context & context) {
	context._cmp(context.si, 30000);
	if (!context.flags.z()) goto notendblock;
	context.push(context.bx);
	context.push(context.es);
	context.push(context.di);
	context.push(context.ds);
	context.push(context.si);
	readoneblock(context);
	context.si = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	context.es = context.pop();
	context.bx = context.pop();
	context.si = 0;
notendblock:
	context._lodsb();
	return;
}

static inline void printundermon(Context & context) {
	context.si = (320*43)+76;
	context.di = context.si;
	context.es = context.data.word(400);
	context._add(context.si, 8*320);
	context.dx = 0x0a000;
	context.ds = context.dx;
	context.cx = 104;
scrollmonloop1:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.si);
	context.cx = 170;
scrollmonloop2:
	context._lodsb();
	context._cmp(context.al, 231);
	if (!context.flags.c()) goto dontplace;
placeit:
	context._stosb();
	if (--context.cx) goto scrollmonloop2;
	goto finmonscroll;
dontplace:
	context._add(context.di, 1);
	if (--context.cx) goto scrollmonloop2;
finmonscroll:
	context.si = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.si, 320);
	context._add(context.di, 320);
	if (--context.cx) goto scrollmonloop1;
	return;
}

static inline void pixelcheckset(Context & context) {
	context.push(context.ax);
	context._sub(context.al, context.data.byte(context.bx));
	context._sub(context.ah, context.data.byte(context.bx+1));
	context.push(context.es);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.ax);
	context.al = context.data.byte(context.bx+4);
	getsetad(context);
	context.al = context.data.byte(context.bx+17);
	context.es = context.data.word(444);
	context.bx = 0;
	context.ah = 0;
	context.cx = 6;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.ax = context.pop();
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context.cl = context.data.byte(context.bx);
	context.ch = 0;
	context._mul(context.cx);
	context.cx = context.pop();
	context.ch = 0;
	context._add(context.ax, context.cx);
	context._add(context.ax, context.data.word(context.bx+2));
	context.bx = context.ax;
	context._add(context.bx, 0+2080);
	context.al = context.data.byte(context.bx);
	context.dl = context.al;
	context.cx = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	context.ax = context.pop();
	context._cmp(context.dl, 0);
	return;
}

static inline void delthisone(Context & context) {
	context.push(context.ax);
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context._add(context.ax, context.data.word(119));
	context.bx = 320;
	context._mul(context.bx);
	context.bx = context.pop();
	context.bh = 0;
	context._add(context.bx, context.data.word(117));
	context._add(context.ax, context.bx);
	context.di = context.ax;
	context.ax = context.pop();
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context.bx = 22*8;
	context._mul(context.bx);
	context.bx = context.pop();
	context.bh = 0;
	context._add(context.ax, context.bx);
	context.si = context.ax;
	context.es = context.data.word(400);
	context.ds = context.data.word(402);
	context.dl = context.cl;
	context.dh = 0;
	context.ax = 320;
	context._sub(context.ax, context.dx);
	context._neg(context.dx);
	context._add(context.dx, 22*8);
deloneloop:
	context.push(context.cx);
	context.ch = 0;
	while(--context.cx) 	context._movsb();
 	context.cx = context.pop();
	context._add(context.di, context.ax);
	context._add(context.si, context.dx);
	context._sub(context.ch, 1);
	if (!context.flags.z()) goto deloneloop;
	return;
}

static inline void dofade(Context & context) {
	context._cmp(context.data.byte(344), 0);
	if (context.flags.z()) goto finishfade;
	context.cl = context.data.byte(345);
	context.ch = 0;
	context.al = context.data.byte(343);
	context.ah = 0;
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3);
	context._add(context.si, context.ax);
	context._add(context.si, context.ax);
	context._add(context.si, context.ax);
	showgroup(context);
	context.al = context.data.byte(345);
	context._add(context.al, context.data.byte(343));
	context.data.byte(343) = context.al;
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto finishfade;
	fadecalculation(context);
finishfade:
	return;
}

static inline void fadetowhite(Context & context) {
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768;
	context.cx = 768;
	context.al = 63;
	while(--context.cx) 	context._stosb();
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768;
	context.al = 0;
	context._stosb();
	context._stosb();
	context._stosb();
	paltostartpal(context);
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 128;
	return;
}

static inline void fadefromwhite(Context & context) {
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3);
	context.cx = 768;
	context.al = 63;
	while(--context.cx) 	context._stosb();
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3);
	context.al = 0;
	context._stosb();
	context._stosb();
	context._stosb();
	paltoendpal(context);
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 128;
	return;
}

static inline void fadescreendownhalf(Context & context) {
	paltostartpal(context);
	paltoendpal(context);
	context.cx = 768;
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768;
halfend:
	context.al = context.data.byte(context.bx);
	context._shr(context.al, 1);
	context.data.byte(context.bx) = context.al;
	context._add(context.bx, 1);
	if (--context.cx) goto halfend;
	context.ds = context.data.word(412);
	context.es = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+(56*3);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+(56*3);
	context.cx = 3*5;
	while(--context.cx) 	context._movsb();
 	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+(77*3);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+(77*3);
	context.cx = 3*2;
	while(--context.cx) 	context._movsb();
 	context.data.byte(344) = 1;
	context.data.byte(346) = 31;
	context.data.byte(343) = 0;
	context.data.byte(345) = 32;
	return;
}

static inline void fadescreenuphalf(Context & context) {
	endpaltostart(context);
	paltoendpal(context);
	context.data.byte(344) = 1;
	context.data.byte(346) = 31;
	context.data.byte(343) = 0;
	context.data.byte(345) = 32;
	return;
}

static inline void fadecalculation(Context & context) {
	context._cmp(context.data.byte(346), 0);
	if (context.flags.z()) goto nomorefading;
	context.bl = context.data.byte(346);
	context.es = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768;
	context.cx = 768;
fadecolloop:
	context.al = context.data.byte(context.si);
	context.ah = context.data.byte(context.di);
	context._cmp(context.al, context.ah);
	if (context.flags.z()) goto gotthere;
	if (context.flags.c()) goto lesscolour;
	context._sub(context.data.byte(context.si), 1);
	goto gotthere;
lesscolour:
	context._cmp(context.bl, context.ah);
	if (context.flags.z()) goto withit;
	if (!context.flags.c()) goto gotthere;
withit:
	context._add(context.data.byte(context.si), 1);
gotthere:
	context._add(context.si, 1);
	context._add(context.di, 1);
	if (--context.cx) goto fadecolloop;
	context._sub(context.data.byte(346), 1);
	return;
nomorefading:
	context.data.byte(344) = 0;
	return;
}

static inline void startpaltoend(Context & context) {
	context.es = context.data.word(412);
	context.ds = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768;
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3);
	context.cx = 768/2;
	while(--context.cx) 	context._movsw();
 	return;
}

static inline void fadedownmon(Context & context) {
	paltostartpal(context);
	paltoendpal(context);
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+(231*3);
	context.cx = 3*8;
	context.ax = 0;
	while(--context.cx) 	context._stosb();
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+(246*3);
	context._stosb();
	context._stosw();
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 128;
	context.cx = 64;
	hangon(context);
	return;
}

static inline void fadeupmon(Context & context) {
	paltostartpal(context);
	paltoendpal(context);
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+(231*3);
	context.cx = 3*8;
	context.ax = 0;
	while(--context.cx) 	context._stosb();
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+(246*3);
	context._stosb();
	context._stosw();
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 128;
	context.cx = 128;
	hangon(context);
	return;
}

static inline void fadeupmonfirst(Context & context) {
	paltostartpal(context);
	paltoendpal(context);
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+(231*3);
	context.cx = 3*8;
	context.ax = 0;
	while(--context.cx) 	context._stosb();
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+(246*3);
	context._stosb();
	context._stosw();
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 128;
	context.cx = 64;
	hangon(context);
	context.al = 26;
	playchannel1(context);
	context.cx = 64;
	hangon(context);
	return;
}

static inline void fadeupyellows(Context & context) {
	paltoendpal(context);
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+(231*3);
	context.cx = 3*8;
	context.ax = 0;
	while(--context.cx) 	context._stosb();
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+(246*3);
	context._stosb();
	context._stosw();
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 128;
	context.cx = 128;
	hangon(context);
	return;
}

static inline void initialmoncols(Context & context) {
	paltostartpal(context);
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+(230*3);
	context.cx = 3*9;
	context.ax = 0;
	while(--context.cx) 	context._stosb();
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+(246*3);
	context._stosb();
	context._stosw();
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+(230*3);
	context.al = 230;
	context.cx = 18;
	showgroup(context);
	return;
}

static inline void monprint(Context & context) {
	context.data.byte(72) = 1;
	context.si = context.bx;
	context.dl = 166;
	context.di = context.data.word(318);
	context.bx = context.data.word(320);
	context.ds = context.data.word(406);
printloop8:
	context.push(context.bx);
	context.push(context.di);
	context.push(context.dx);
	getnumber(context);
	context.ch = 0;
printloop7:
	context.al = context.data.byte(context.si);
	context._add(context.si, 1);
	context._cmp(context.al, ':');
	if (context.flags.z()) goto finishmon2;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto finishmon;
	context._cmp(context.al, 34);
	if (context.flags.z()) goto finishmon;
	context._cmp(context.al, '=');
	if (context.flags.z()) goto finishmon;
	context._cmp(context.al, '%');
	if (!context.flags.z()) goto nottrigger;
	context.ah = context.data.byte(context.si);
	context._add(context.si, 1);
	context._add(context.si, 1);
	goto finishmon;
nottrigger:
	context.push(context.cx);
	context.push(context.es);
	modifychar(context);
	printchar(context);
	context.data.word(312) = context.di;
	context.data.word(314) = context.bx;
	context.data.word(138) = 1;
	printcurs(context);
	vsync(context);
	context.push(context.si);
	context.push(context.dx);
	context.push(context.ds);
	context.push(context.es);
	context.push(context.bx);
	context.push(context.di);
	lockmon(context);
	context.di = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	context.ds = context.pop();
	context.dx = context.pop();
	context.si = context.pop();
	delcurs(context);
	context.es = context.pop();
	context.cx = context.pop();
	if (--context.cx) goto printloop7;
finishmon2:
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	scrollmonitor(context);
	context.data.word(312) = context.di;
	goto printloop8;
finishmon:
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context._cmp(context.al, '%');
	if (!context.flags.z()) goto nottrigger2;
	context.data.byte(55) = context.ah;
nottrigger2:
	context.data.word(312) = context.di;
	scrollmonitor(context);
	context.bx = context.si;
	context.data.byte(72) = 0;
	return;
}

static inline void fillryan(Context & context) {
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32;
	findallryan(context);
	context.si = 0+(228*13)+32;
	context.al = context.data.byte(19);
	context.ah = 0;
	context.cx = 20;
	context._mul(context.cx);
	context._add(context.si, context.ax);
	context.di = 80;
	context.bx = 58;
	context.cx = 2;
ryanloop2:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.bx);
	context.cx = 5;
ryanloop1:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.bx);
	context.ax = context.data.word(context.si);
	context._add(context.si, 2);
	context.push(context.si);
	context.push(context.es);
	obtoinv(context);
	context.es = context.pop();
	context.si = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.di, 44);
	if (--context.cx) goto ryanloop1;
	context.bx = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 44);
	if (--context.cx) goto ryanloop2;
	showryanpage(context);
	return;
}

static inline void fillopen(Context & context) {
	deltextline(context);
	getopenedsize(context);
	context._cmp(context.ah, 4);
	if (context.flags.c()) goto lessthanapage;
	context.ah = 4;
lessthanapage:
	context.al = 1;
	context.push(context.ax);
	context.es = context.data.word(412);
	context.di = 0+(228*13);
	findallopen(context);
	context.si = 0+(228*13);
	context.di = 80;
	context.bx = 58+96;
	context.cx = context.pop();
openloop1:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.bx);
	context.ax = context.data.word(context.si);
	context._add(context.si, 2);
	context.push(context.si);
	context.push(context.es);
	context._cmp(context.ch, context.cl);
	if (context.flags.c()) goto nextopenslot;
	obtoinv(context);
nextopenslot:
	context.es = context.pop();
	context.si = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.di, 44);
	context._add(context.cl, 1);
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto openloop1;
	undertextline(context);
	return;
}

static inline void findallryan(Context & context) {
	context.push(context.di);
	context.cx = 30;
	context.ax = 0x0ffff;
	while(--context.cx) 	context._stosw();
	context.di = context.pop();
	context.cl = 4;
	context.ds = context.data.word(398);
	context.bx = 0+2080+30000;
	context.ch = 0;
findryanloop:
	context._cmp(context.data.byte(context.bx+2), context.cl);
	if (!context.flags.z()) goto notinryaninv;
	context._cmp(context.data.byte(context.bx+3), 255);
	if (!context.flags.z()) goto notinryaninv;
	context.al = context.data.byte(context.bx+4);
	context.ah = 0;
	context.push(context.di);
	context._add(context.di, context.ax);
	context._add(context.di, context.ax);
	context.al = context.ch;
	context.ah = 4;
	context._stosw();
	context.di = context.pop();
notinryaninv:
	context._add(context.bx, 16);
	context._add(context.ch, 1);
	context._cmp(context.ch, 114);
	if (!context.flags.z()) goto findryanloop;
	return;
}

static inline void findallopen(Context & context) {
	context.push(context.di);
	context.cx = 16;
	context.ax = 0x0ffff;
	while(--context.cx) 	context._stosw();
	context.di = context.pop();
	context.cl = context.data.byte(110);
	context.dl = context.data.byte(111);
	context.ds = context.data.word(398);
	context.bx = 0+2080+30000;
	context.ch = 0;
findopen1:
	context._cmp(context.data.byte(context.bx+3), context.cl);
	if (!context.flags.z()) goto findopen2;
	context._cmp(context.data.byte(context.bx+2), context.dl);
	if (!context.flags.z()) goto findopen2;
	context._cmp(context.data.byte(111), 4);
	if (context.flags.z()) goto noloccheck;
	context.al = context.data.byte(context.bx+5);
	context._cmp(context.al, context.data.byte(184));
	if (!context.flags.z()) goto findopen2;
noloccheck:
	context.al = context.data.byte(context.bx+4);
	context.ah = 0;
	context.push(context.di);
	context._add(context.di, context.ax);
	context._add(context.di, context.ax);
	context.al = context.ch;
	context.ah = 4;
	context._stosw();
	context.di = context.pop();
findopen2:
	context._add(context.bx, 16);
	context._add(context.ch, 1);
	context._cmp(context.ch, 114);
	if (!context.flags.z()) goto findopen1;
	context.cl = context.data.byte(110);
	context.dl = context.data.byte(111);
	context.push(context.dx);
	context.ds = context.data.word(426);
	context.dx = context.pop();
	context.bx = 0;
	context.ch = 0;
findopen1a:
	context._cmp(context.data.byte(context.bx+3), context.cl);
	if (!context.flags.z()) goto findopen2a;
	context._cmp(context.data.byte(context.bx+2), context.dl);
	if (!context.flags.z()) goto findopen2a;
	context.al = context.data.byte(context.bx+4);
	context.ah = 0;
	context.push(context.di);
	context._add(context.di, context.ax);
	context._add(context.di, context.ax);
	context.al = context.ch;
	context.ah = 2;
	context._stosw();
	context.di = context.pop();
findopen2a:
	context._add(context.bx, 16);
	context._add(context.ch, 1);
	context._cmp(context.ch, 80);
	if (!context.flags.z()) goto findopen1a;
	return;
}

static inline void obtoinv(Context & context) {
	context.push(context.bx);
	context.push(context.es);
	context.push(context.si);
	context.push(context.ax);
	context.push(context.ax);
	context.push(context.di);
	context.push(context.bx);
	context.ds = context.data.word(408);
	context._sub(context.di, 2);
	context._sub(context.bx, 1);
	context.al = 10;
	context.ah = 0;
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.ax = context.pop();
	context._cmp(context.al, 255);
	if (context.flags.z()) goto finishfill;
	context.push(context.bx);
	context.push(context.di);
	context.push(context.ax);
	context.ds = context.data.word(398);
	context._cmp(context.ah, 4);
	if (context.flags.z()) goto isanextra;
	context.ds = context.data.word(446);
isanextra:
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	context._add(context.al, 1);
	context.ah = 128;
	context._add(context.bx, 19);
	context._add(context.di, 18);
	showframe(context);
	context.ax = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.push(context.bx);
	getanyaddir(context);
	isitworn(context);
	context.bx = context.pop();
	if (!context.flags.z()) goto finishfill;
	context.ds = context.data.word(408);
	context._sub(context.di, 3);
	context._sub(context.bx, 2);
	context.al = 7;
	context.ah = 0;
	showframe(context);
finishfill:
	context.ax = context.pop();
	context.si = context.pop();
	context.es = context.pop();
	context.bx = context.pop();
	return;
}

static inline void isitworn(Context & context) {
	context.al = context.data.byte(context.bx+12);
	context._cmp(context.al, 'W'-'A');
	if (!context.flags.z()) goto notworn;
	context.al = context.data.byte(context.bx+13);
	context._cmp(context.al, 'E'-'A');
notworn:
	return;
}

static inline void makeworn(Context & context) {
	context.data.byte(context.bx+12) = 'W'-'A';
	context.data.byte(context.bx+13) = 'E'-'A';
	return;
}

static inline void getbackfromob(Context & context) {
	context._cmp(context.data.byte(106), 1);
	if (!context.flags.z()) goto notheldob;
	blank(context);
	return;
notheldob:
	getback1(context);
	return;
}

static inline void incryanpage(Context & context) {
	context._cmp(context.data.byte(100), 222);
	if (context.flags.z()) goto alreadyincryan;
	context.data.byte(100) = 222;
	context.al = 31;
	commandonly(context);
alreadyincryan:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto noincryan;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doincryan;
noincryan:
	return;
doincryan:
	context.ax = context.data.word(198);
	context._sub(context.ax, 80+167);
	context.data.byte(19) = -1;
findnewpage:
	context._add(context.data.byte(19), 1);
	context._sub(context.ax, 18);
	if (!context.flags.c()) goto findnewpage;
	delpointer(context);
	fillryan(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void openinv(Context & context) {
	context.data.byte(104) = 1;
	context.al = 61;
	context.di = 80;
	context.bx = 58-10;
	context.dl = 240;
	printmessage(context);
	fillryan(context);
	context.data.byte(100) = 255;
	return;
}

static inline void showryanpage(Context & context) {
	context.ds = context.data.word(408);
	context.di = 80+167;
	context.bx = 58-12;
	context.al = 12;
	context.ah = 0;
	showframe(context);
	context.al = 13;
	context._add(context.al, context.data.byte(19));
	context.push(context.ax);
	context.al = context.data.byte(19);
	context.ah = 0;
	context.cx = 18;
	context._mul(context.cx);
	context.ds = context.data.word(408);
	context.di = 80+167;
	context._add(context.di, context.ax);
	context.bx = 58-12;
	context.ax = context.pop();
	context.ah = 0;
	showframe(context);
	return;
}

static inline void openob(Context & context) {
	context.al = context.data.byte(110);
	context.ah = context.data.byte(111);
	context.di = 5674;
	copyname(context);
	context.di = 80;
	context.bx = 58+86;
	context.al = 62;
	context.dl = 240;
	printmessage(context);
	context.di = context.data.word(84);
	context._add(context.di, 5);
	context.bx = 58+86;
	context.es = context.cs;
	context.si = 5674;
	context.dl = 220;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	fillopen(context);
	getopenedsize(context);
	context.al = context.ah;
	context.ah = 0;
	context.cx = 44;
	context._mul(context.cx);
	context._add(context.ax, 80);
	context.bx = 2588;
	context.data.word(context.bx) = context.ax;
	return;
}

static inline void examicon(Context & context) {
	context.ds = context.data.word(410);
	context.di = 254;
	context.bx = 5;
	context.al = 3;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void inventory(Context & context) {
	context._cmp(context.data.byte(56), 1);
	if (context.flags.z()) goto iswatchinv;
	context._cmp(context.data.word(21), 0);
	if (context.flags.z()) goto notwatchinv;
iswatchinv:
	blank(context);
	return;
notwatchinv:
	context._cmp(context.data.byte(100), 239);
	if (context.flags.z()) goto alreadyopinv;
	context.data.byte(100) = 239;
	context.al = 32;
	commandonly(context);
alreadyopinv:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto cantopinv;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doopeninv;
cantopinv:
	return;
doopeninv:
	context.data.word(328) = 0;
	context.data.byte(234) = 0;
	context.data.byte(237) = 0;
	animpointer(context);
	createpanel(context);
	showpanel(context);
	examicon(context);
	showman(context);
	showexit(context);
	undertextline(context);
	context.data.byte(106) = 0;
	context.data.byte(104) = 2;
	openinv(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	context.data.byte(110) = 255;
	goto waitexam;
/*continuing to unbounded code: examineagain from examineob:3-62*/
examineagain:
	context.data.byte(237) = 0;
	context.data.byte(108) = 0;
	context.data.byte(110) = 255;
	context.data.byte(111) = 255;
	context.data.byte(104) = 0;
	context.al = context.data.byte(100);
	context.data.byte(102) = context.al;
	context.data.byte(89) = 0;
	context.data.byte(231) = 0;
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	obicons(context);
	obpicture(context);
	describeob(context);
	undertextline(context);
	context.data.byte(100) = 255;
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
waitexam:
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	delpointer(context);
	context.data.byte(103) = 0;
	context.bx = 2494;
	context._cmp(context.data.byte(104), 0);
	if (context.flags.z()) goto notuseinv;
	context.bx = 2556;
	context._cmp(context.data.byte(104), 1);
	if (context.flags.z()) goto notuseinv;
	context.bx = 2618;
notuseinv:
	checkcoords(context);
	context._cmp(context.data.byte(108), 0);
	if (context.flags.z()) goto norex;
	goto examineagain;
norex:
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto waitexam;
	context.data.byte(106) = 0;
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) goto iswatching;
	context._cmp(context.data.byte(188), 255);
	if (!context.flags.z()) goto justgetback;
iswatching:
	makemainscreen(context);
	context.data.byte(104) = 0;
	context.data.byte(110) = 255;
	return;
justgetback:
	context.data.byte(104) = 0;
	context.data.byte(110) = 255;
	return;
	return;
}

static inline void setpickup(Context & context) {
	context._cmp(context.data.byte(102), 1);
	if (context.flags.z()) goto cantpick;
	context._cmp(context.data.byte(102), 3);
	if (context.flags.z()) goto cantpick;
	getanyad(context);
	context.al = context.data.byte(context.bx+2);
	context._cmp(context.al, 4);
	if (!context.flags.z()) goto canpick;
cantpick:
	blank(context);
	return;
canpick:
	context._cmp(context.data.byte(100), 209);
	if (context.flags.z()) goto alreadysp;
	context.data.byte(100) = 209;
	context.bl = context.data.byte(99);
	context.bh = context.data.byte(102);
	context.al = 33;
	commandwithob(context);
alreadysp:
	context.ax = context.data.word(202);
	context._cmp(context.ax, 1);
	if (!context.flags.z()) goto nosetpick;
	context._cmp(context.ax, context.data.word(212));
	if (!context.flags.z()) goto dosetpick;
nosetpick:
	return;
dosetpick:
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	examicon(context);
	context.data.byte(106) = 1;
	context.data.byte(104) = 2;
	context._cmp(context.data.byte(102), 4);
	if (context.flags.z()) goto pickupexob;
	context.al = context.data.byte(99);
	context.data.byte(89) = context.al;
	context.data.byte(110) = 255;
	transfertoex(context);
	context.data.byte(89) = context.al;
	context.data.byte(102) = 4;
	geteitherad(context);
	context.data.byte(context.bx+2) = 20;
	context.data.byte(context.bx+3) = 255;
	openinv(context);
	worktoscreenm(context);
	return;
pickupexob:
	context.al = context.data.byte(99);
	context.data.byte(89) = context.al;
	context.data.byte(110) = 255;
	openinv(context);
	worktoscreenm(context);
	return;
}

static inline void examinventory(Context & context) {
	context._cmp(context.data.byte(100), 249);
	if (context.flags.z()) goto alreadyexinv;
	context.data.byte(100) = 249;
	context.al = 32;
	commandonly(context);
alreadyexinv:
	context.ax = context.data.word(202);
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doexinv;
	return;
doexinv:
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	examicon(context);
	context.data.byte(106) = 0;
	context.data.byte(104) = 2;
	openinv(context);
	worktoscreenm(context);
	return;
}

static inline void reexfrominv(Context & context) {
	findinvpos(context);
	context.ax = context.data.word(context.bx);
	context.data.byte(100) = context.ah;
	context.data.byte(99) = context.al;
	context.data.byte(108) = 1;
	context.data.byte(234) = 0;
	return;
}

static inline void reexfromopen(Context & context) {
	return;
	findopenpos(context);
	context.ax = context.data.word(context.bx);
	context.data.byte(100) = context.ah;
	context.data.byte(99) = context.al;
	context.data.byte(108) = 1;
	context.data.byte(234) = 0;
	return;
}

static inline void swapwithinv(Context & context) {
	context.al = context.data.byte(89);
	context.ah = context.data.byte(102);
	context._cmp(context.ax, context.data.word(94));
	if (!context.flags.z()) goto difsub7;
	context._cmp(context.data.byte(100), 243);
	if (context.flags.z()) goto alreadyswap1;
	context.data.byte(100) = 243;
difsub7:
	context.data.word(94) = context.ax;
	context.bx = context.ax;
	context.al = 34;
	commandwithob(context);
alreadyswap1:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto cantswap1;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doswap1;
cantswap1:
	return;
doswap1:
	context.ah = context.data.byte(102);
	context.al = context.data.byte(89);
	context.push(context.ax);
	findinvpos(context);
	context.ax = context.data.word(context.bx);
	context.data.byte(89) = context.al;
	context.data.byte(102) = context.ah;
	geteitherad(context);
	context.data.byte(context.bx+2) = 20;
	context.data.byte(context.bx+3) = 255;
	context.bl = context.data.byte(89);
	context.bh = context.data.byte(102);
	context.ax = context.pop();
	context.data.byte(102) = context.ah;
	context.data.byte(89) = context.al;
	context.push(context.bx);
	findinvpos(context);
	delpointer(context);
	context.al = context.data.byte(89);
	geteitherad(context);
	context.data.byte(context.bx+2) = 4;
	context.data.byte(context.bx+3) = 255;
	context.al = context.data.byte(107);
	context.data.byte(context.bx+4) = context.al;
	context.ax = context.pop();
	context.data.byte(102) = context.ah;
	context.data.byte(89) = context.al;
	fillryan(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void swapwithopen(Context & context) {
	context.al = context.data.byte(89);
	context.ah = context.data.byte(102);
	context._cmp(context.ax, context.data.word(94));
	if (!context.flags.z()) goto difsub8;
	context._cmp(context.data.byte(100), 242);
	if (context.flags.z()) goto alreadyswap2;
	context.data.byte(100) = 242;
difsub8:
	context.data.word(94) = context.ax;
	context.bx = context.ax;
	context.al = 34;
	commandwithob(context);
alreadyswap2:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto cantswap2;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doswap2;
cantswap2:
	return;
doswap2:
	geteitherad(context);
	isitworn(context);
	if (!context.flags.z()) goto notwornswap;
	wornerror(context);
	return;
notwornswap:
	delpointer(context);
	context.al = context.data.byte(89);
	context._cmp(context.al, context.data.byte(110));
	if (!context.flags.z()) goto isntsame2;
	context.al = context.data.byte(102);
	context._cmp(context.al, context.data.byte(111));
	if (!context.flags.z()) goto isntsame2;
	errormessage1(context);
	return;
isntsame2:
	checkobjectsize(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto sizeok2;
	return;
sizeok2:
	context.ah = context.data.byte(102);
	context.al = context.data.byte(89);
	context.push(context.ax);
	findopenpos(context);
	context.ax = context.data.word(context.bx);
	context.data.byte(89) = context.al;
	context.data.byte(102) = context.ah;
	context._cmp(context.ah, 4);
	if (!context.flags.z()) goto makeswapex;
	geteitherad(context);
	context.data.byte(context.bx+2) = 20;
	context.data.byte(context.bx+3) = 255;
	goto actuallyswap;
makeswapex:
	transfertoex(context);
	context.data.byte(89) = context.al;
	context.data.byte(102) = 4;
	geteitherad(context);
	context.data.byte(context.bx+2) = 20;
	context.data.byte(context.bx+3) = 255;
actuallyswap:
	context.bl = context.data.byte(89);
	context.bh = context.data.byte(102);
	context.ax = context.pop();
	context.data.byte(102) = context.ah;
	context.data.byte(89) = context.al;
	context.push(context.bx);
	findopenpos(context);
	geteitherad(context);
	context.al = context.data.byte(111);
	context.data.byte(context.bx+2) = context.al;
	context.al = context.data.byte(110);
	context.data.byte(context.bx+3) = context.al;
	context.al = context.data.byte(107);
	context.data.byte(context.bx+4) = context.al;
	context.al = context.data.byte(184);
	context.data.byte(context.bx+5) = context.al;
	context.ax = context.pop();
	context.data.byte(102) = context.ah;
	context.data.byte(89) = context.al;
	fillopen(context);
	fillryan(context);
	undertextline(context);
	readmouse(context);
	useopened(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void intoinv(Context & context) {
	context._cmp(context.data.byte(106), 0);
	if (!context.flags.z()) goto notout;
	outofinv(context);
	return;
notout:
	findinvpos(context);
	context.ax = context.data.word(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto canplace1;
	swapwithinv(context);
	return;
canplace1:
	context.al = context.data.byte(89);
	context.ah = context.data.byte(102);
	context._cmp(context.ax, context.data.word(94));
	if (!context.flags.z()) goto difsub1;
	context._cmp(context.data.byte(100), 220);
	if (context.flags.z()) goto alreadyplce;
	context.data.byte(100) = 220;
difsub1:
	context.data.word(94) = context.ax;
	context.bx = context.ax;
	context.al = 35;
	commandwithob(context);
alreadyplce:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notletgo2;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doplace;
notletgo2:
	return;
doplace:
	delpointer(context);
	context.al = context.data.byte(89);
	getexad(context);
	context.data.byte(context.bx+2) = 4;
	context.data.byte(context.bx+3) = 255;
	context.al = context.data.byte(107);
	context.data.byte(context.bx+4) = context.al;
	context.data.byte(106) = 0;
	fillryan(context);
	readmouse(context);
	showpointer(context);
	outofinv(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void outofinv(Context & context) {
	findinvpos(context);
	context.ax = context.data.word(context.bx);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto canpick2;
	blank(context);
	return;
canpick2:
	context.bx = context.data.word(202);
	context._cmp(context.bx, 2);
	if (!context.flags.z()) goto canpick2a;
	reexfrominv(context);
	return;
canpick2a:
	context._cmp(context.ax, context.data.word(94));
	if (!context.flags.z()) goto difsub3;
	context._cmp(context.data.byte(100), 221);
	if (context.flags.z()) goto alreadygrab;
	context.data.byte(100) = 221;
difsub3:
	context.data.word(94) = context.ax;
	context.bx = context.ax;
	context.al = 36;
	commandwithob(context);
alreadygrab:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notletgo;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dograb;
notletgo:
	return;
dograb:
	delpointer(context);
	context.data.byte(106) = 1;
	findinvpos(context);
	context.ax = context.data.word(context.bx);
	context.data.byte(89) = context.al;
	context.data.byte(102) = context.ah;
	getexad(context);
	context.data.byte(context.bx+2) = 20;
	context.data.byte(context.bx+3) = 255;
	fillryan(context);
	readmouse(context);
	showpointer(context);
	intoinv(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void geteitherad(Context & context) {
	context._cmp(context.data.byte(102), 4);
	if (context.flags.z()) goto isinexlist;
	context.al = context.data.byte(89);
	getfreead(context);
	return;
isinexlist:
	context.al = context.data.byte(89);
	getexad(context);
	return;
}

static inline void getopenedsize(Context & context) {
	context._cmp(context.data.byte(111), 4);
	if (context.flags.z()) goto isex2;
	context._cmp(context.data.byte(111), 2);
	if (context.flags.z()) goto isfree2;
	context.al = context.data.byte(110);
	getsetad(context);
	context.ax = context.data.word(context.bx+3);
	return;
isfree2:
	context.al = context.data.byte(110);
	getfreead(context);
	context.ax = context.data.word(context.bx+7);
	return;
isex2:
	context.al = context.data.byte(110);
	getexad(context);
	context.ax = context.data.word(context.bx+7);
	return;
}

static inline void findinvpos(Context & context) {
	context.cx = context.data.word(198);
	context._sub(context.cx, 80);
	context.bx = -1;
findinv1:
	context._add(context.bx, 1);
	context._sub(context.cx, 44);
	if (!context.flags.c()) goto findinv1;
	context.cx = context.data.word(200);
	context._sub(context.cx, 58);
	context._sub(context.bx, 5);
findinv2:
	context._add(context.bx, 5);
	context._sub(context.cx, 44);
	if (!context.flags.c()) goto findinv2;
	context.al = context.data.byte(19);
	context.ah = 0;
	context.cx = 10;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.al = context.bl;
	context.data.byte(107) = context.al;
	context._add(context.bx, context.bx);
	context.es = context.data.word(412);
	context._add(context.bx, 0+(228*13)+32);
	return;
}

static inline void findopenpos(Context & context) {
	context.cx = context.data.word(198);
	context._sub(context.cx, 80);
	context.bx = -1;
findopenp1:
	context._add(context.bx, 1);
	context._sub(context.cx, 44);
	if (!context.flags.c()) goto findopenp1;
	context.al = context.bl;
	context.data.byte(107) = context.al;
	context._add(context.bx, context.bx);
	context.es = context.data.word(412);
	context._add(context.bx, 0+(228*13));
	return;
}

static inline void dropobject(Context & context) {
	context._cmp(context.data.byte(100), 223);
	if (context.flags.z()) goto alreadydrop;
	context.data.byte(100) = 223;
	context._cmp(context.data.byte(106), 0);
	if (context.flags.z()) { blank(context); return; };
	context.bl = context.data.byte(89);
	context.bh = context.data.byte(102);
	context.al = 37;
	commandwithob(context);
alreadydrop:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto nodrop;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dodrop;
nodrop:
	return;
dodrop:
	geteitherad(context);
	isitworn(context);
	if (!context.flags.z()) goto nowornerror;
	wornerror(context);
	return;
nowornerror:
	context._cmp(context.data.byte(184), 47);
	if (context.flags.z()) goto nodrop2;
	context.cl = context.data.byte(151);
	context._add(context.cl, 12);
	context.ch = context.data.byte(152);
	context._add(context.ch, 12);
	checkone(context);
	context._cmp(context.cl, 2);
	if (context.flags.c()) goto nodroperror;
nodrop2:
	droperror(context);
	return;
nodroperror:
	context._cmp(context.data.byte(128), 64);
	if (!context.flags.z()) goto notinlift;
	context._cmp(context.data.byte(129), 64);
	if (!context.flags.z()) goto notinlift;
	droperror(context);
	return;
notinlift:
	context.al = context.data.byte(89);
	context.ah = 4;
	context.cl = 'G';
	context.ch = 'U';
	context.dl = 'N';
	context.dh = 'A';
	compare(context);
	if (context.flags.z()) { cantdrop(context); return; };
	context.al = context.data.byte(89);
	context.ah = 4;
	context.cl = 'S';
	context.ch = 'H';
	context.dl = 'L';
	context.dh = 'D';
	compare(context);
	if (context.flags.z()) { cantdrop(context); return; };
	context.data.byte(102) = 4;
	context.al = context.data.byte(89);
	getexad(context);
	context.data.byte(context.bx+2) = 0;
	context.al = context.data.byte(151);
	context._add(context.al, 4);
	context.cl = 4;
	context._shr(context.al, context.cl);
	context._add(context.al, context.data.byte(148));
	context.ah = context.data.byte(152);
	context._add(context.ah, 8);
	context.cl = 4;
	context._shr(context.ah, context.cl);
	context._add(context.ah, context.data.byte(149));
	context.data.byte(context.bx+3) = context.al;
	context.data.byte(context.bx+5) = context.ah;
	context.al = context.data.byte(151);
	context._add(context.al, 4);
	context._and(context.al, 15);
	context.ah = context.data.byte(152);
	context._add(context.ah, 8);
	context._and(context.ah, 15);
	context.data.byte(context.bx+4) = context.al;
	context.data.byte(context.bx+6) = context.ah;
	context.data.byte(106) = 0;
	context.al = context.data.byte(184);
	context.data.byte(context.bx) = context.al;
	return;
}

static inline void droperror(Context & context) {
	context.data.byte(100) = 255;
	delpointer(context);
	context.di = 76;
	context.bx = 21;
	context.al = 56;
	context.dl = 240;
	printmessage(context);
	worktoscreenm(context);
	context.cx = 50;
	hangonp(context);
	showpanel(context);
	showman(context);
	examicon(context);
	context.data.byte(100) = 255;
	worktoscreenm(context);
	return;
}

static inline void cantdrop(Context & context) {
	context.data.byte(100) = 255;
	delpointer(context);
	context.di = 76;
	context.bx = 21;
	context.al = 24;
	context.dl = 240;
	printmessage(context);
	worktoscreenm(context);
	context.cx = 50;
	hangonp(context);
	showpanel(context);
	showman(context);
	examicon(context);
	context.data.byte(100) = 255;
	worktoscreenm(context);
	return;
}

static inline void wornerror(Context & context) {
	context.data.byte(100) = 255;
	delpointer(context);
	context.di = 76;
	context.bx = 21;
	context.al = 57;
	context.dl = 240;
	printmessage(context);
	worktoscreenm(context);
	context.cx = 50;
	hangonp(context);
	showpanel(context);
	showman(context);
	examicon(context);
	context.data.byte(100) = 255;
	worktoscreenm(context);
	return;
}

static inline void removeobfrominv(Context & context) {
	context._cmp(context.data.byte(99), 100);
	if (context.flags.z()) goto obnotexist;
	getanyad(context);
	context.di = context.bx;
	context.cl = context.data.byte(99);
	context.ch = 0;
	deleteexobject(context);
obnotexist:
	return;
}

static inline void selectopenob(Context & context) {
	context.al = context.data.byte(99);
	getanyad(context);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto canopenit1;
	blank(context);
	return;
canopenit1:
	context._cmp(context.data.byte(100), 224);
	if (context.flags.z()) goto alreadyopob;
	context.data.byte(100) = 224;
	context.bl = context.data.byte(99);
	context.bh = context.data.byte(102);
	context.al = 38;
	commandwithob(context);
alreadyopob:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto noopenob;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doopenob;
noopenob:
	return;
doopenob:
	context.al = context.data.byte(99);
	context.data.byte(110) = context.al;
	context.al = context.data.byte(102);
	context.data.byte(111) = context.al;
	createpanel(context);
	showpanel(context);
	showman(context);
	examicon(context);
	showexit(context);
	openinv(context);
	openob(context);
	undertextline(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void useopened(Context & context) {
	context._cmp(context.data.byte(110), 255);
	if (context.flags.z()) goto cannotuseopen;
	context._cmp(context.data.byte(106), 0);
	if (!context.flags.z()) goto notout2;
	outofopen(context);
	return;
notout2:
	findopenpos(context);
	context.ax = context.data.word(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto canplace3;
	swapwithopen(context);
cannotuseopen:
	return;
canplace3:
	context._cmp(context.data.byte(106), 1);
	if (context.flags.z()) goto intoopen;
	blank(context);
	return;
intoopen:
	context.al = context.data.byte(89);
	context.ah = context.data.byte(102);
	context._cmp(context.ax, context.data.word(94));
	if (!context.flags.z()) goto difsub2;
	context._cmp(context.data.byte(100), 227);
	if (context.flags.z()) goto alreadyplc2;
	context.data.byte(100) = 227;
difsub2:
	context.data.word(94) = context.ax;
	context.bx = context.ax;
	context.al = 35;
	commandwithob(context);
alreadyplc2:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notletgo3;
	context._cmp(context.ax, 1);
	if (context.flags.z()) goto doplace2;
notletgo3:
	return;
doplace2:
	geteitherad(context);
	isitworn(context);
	if (!context.flags.z()) goto notworntoopen;
	wornerror(context);
	return;
notworntoopen:
	delpointer(context);
	context.al = context.data.byte(89);
	context._cmp(context.al, context.data.byte(110));
	if (!context.flags.z()) goto isntsame;
	context.al = context.data.byte(102);
	context._cmp(context.al, context.data.byte(111));
	if (!context.flags.z()) goto isntsame;
	errormessage1(context);
	return;
isntsame:
	checkobjectsize(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto sizeok1;
	return;
sizeok1:
	context.data.byte(106) = 0;
	context.al = context.data.byte(89);
	geteitherad(context);
	context.al = context.data.byte(111);
	context.data.byte(context.bx+2) = context.al;
	context.al = context.data.byte(110);
	context.data.byte(context.bx+3) = context.al;
	context.al = context.data.byte(107);
	context.data.byte(context.bx+4) = context.al;
	context.al = context.data.byte(184);
	context.data.byte(context.bx+5) = context.al;
	fillopen(context);
	undertextline(context);
	readmouse(context);
	useopened(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void errormessage1(Context & context) {
	delpointer(context);
	context.di = 76;
	context.bx = 21;
	context.al = 58;
	context.dl = 240;
	printmessage(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	context.cx = 50;
	hangonp(context);
	showpanel(context);
	showman(context);
	examicon(context);
	readmouse(context);
	useopened(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void errormessage2(Context & context) {
	context.data.byte(100) = 255;
	delpointer(context);
	context.di = 76;
	context.bx = 21;
	context.al = 59;
	context.dl = 240;
	printmessage(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	context.cx = 50;
	hangonp(context);
	showpanel(context);
	showman(context);
	examicon(context);
	readmouse(context);
	useopened(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void errormessage3(Context & context) {
	delpointer(context);
	context.di = 76;
	context.bx = 21;
	context.al = 60;
	context.dl = 240;
	printmessage(context);
	worktoscreenm(context);
	context.cx = 50;
	hangonp(context);
	showpanel(context);
	showman(context);
	examicon(context);
	readmouse(context);
	useopened(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void checkobjectsize(Context & context) {
	getopenedsize(context);
	context.push(context.ax);
	context.al = context.data.byte(89);
	geteitherad(context);
	context.al = context.data.byte(context.bx+9);
	context.cx = context.pop();
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto notunsized;
	context.al = 6;
notunsized:
	context._cmp(context.al, 100);
	if (!context.flags.c()) goto specialcase;
	context._cmp(context.cl, 100);
	if (context.flags.c()) goto isntspecial;
	errormessage3(context);
	goto sizewrong;
isntspecial:
	context._cmp(context.cl, context.al);
	if (!context.flags.c()) goto sizeok;
specialcase:
	context._sub(context.al, 100);
	context._cmp(context.cl, 100);
	if (!context.flags.c()) goto bothspecial;
	context._cmp(context.cl, context.al);
	if (!context.flags.c()) goto sizeok;
	errormessage2(context);
	goto sizewrong;
bothspecial:
	context._sub(context.cl, 100);
	context._cmp(context.al, context.cl);
	if (context.flags.z()) goto sizeok;
	errormessage3(context);
sizewrong:
	context.al = 1;
	return;
sizeok:
	context.al = 0;
	return;
}

static inline void outofopen(Context & context) {
	context._cmp(context.data.byte(110), 255);
	if (context.flags.z()) goto cantuseopen;
	findopenpos(context);
	context.ax = context.data.word(context.bx);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto canpick4;
cantuseopen:
	blank(context);
	return;
canpick4:
	context._cmp(context.ax, context.data.word(94));
	if (!context.flags.z()) goto difsub4;
	context._cmp(context.data.byte(100), 228);
	if (context.flags.z()) goto alreadygrb;
	context.data.byte(100) = 228;
difsub4:
	context.data.word(94) = context.ax;
	context.bx = context.ax;
	context.al = 36;
	commandwithob(context);
alreadygrb:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notletgo4;
	context._cmp(context.ax, 1);
	if (context.flags.z()) goto dogrb;
	context._cmp(context.ax, 2);
	if (!context.flags.z()) goto notletgo4;
	reexfromopen(context);
notletgo4:
	return;
dogrb:
	delpointer(context);
	context.data.byte(106) = 1;
	findopenpos(context);
	context.ax = context.data.word(context.bx);
	context.data.byte(89) = context.al;
	context.data.byte(102) = context.ah;
	context._cmp(context.ah, 4);
	if (!context.flags.z()) goto makeintoex;
	geteitherad(context);
	context.data.byte(context.bx+2) = 20;
	context.data.byte(context.bx+3) = 255;
	goto actuallyout;
makeintoex:
	transfertoex(context);
	context.data.byte(89) = context.al;
	context.data.byte(102) = 4;
	geteitherad(context);
	context.data.byte(context.bx+2) = 20;
	context.data.byte(context.bx+3) = 255;
actuallyout:
	fillopen(context);
	undertextline(context);
	readmouse(context);
	useopened(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void blockget(Context & context) {
	context.ah = context.al;
	context.al = 0;
	context.ds = context.data.word(416);
	context.si = 0+192;
	context._add(context.si, context.ax);
	return;
}

static inline void adjustlen(Context & context) {
	context.ah = context.al;
	context._add(context.al, context.ch);
	context._cmp(context.al, 100);
	if (context.flags.c()) goto over242;
	context.al = 224;
	context._sub(context.al, context.ch);
	context.ch = context.al;
over242:
	return;
}

static inline void look(Context & context) {
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(234), 2);
	if (context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(100), 241);
	if (context.flags.z()) goto alreadylook;
	context.data.byte(100) = 241;
	context.al = 25;
	commandonly(context);
alreadylook:
	context._cmp(context.data.word(202), 1);
	if (!context.flags.z()) goto nolook;
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto nolook;
	dolook(context);
nolook:
	return;
}

static inline void getback1(Context & context) {
	context._cmp(context.data.byte(106), 0);
	if (context.flags.z()) goto notgotobject;
	blank(context);
	return;
notgotobject:
	context._cmp(context.data.byte(100), 202);
	if (context.flags.z()) goto alreadyget;
	context.data.byte(100) = 202;
	context.al = 26;
	commandonly(context);
alreadyget:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto nogetback;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dogetback;
nogetback:
	return;
dogetback:
	context.data.byte(103) = 1;
	context.data.byte(106) = 0;
	return;
}

static inline void talk(Context & context) {
	context.data.byte(246) = 0;
	context.data.byte(237) = 0;
	context.al = context.data.byte(99);
	context.data.byte(247) = context.al;
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	undertextline(context);
	convicons(context);
	starttalk(context);
	context.data.byte(100) = 255;
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
waittalk:
	delpointer(context);
	readmouse(context);
	animpointer(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.data.byte(103) = 0;
	context.bx = 2660;
	checkcoords(context);
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto waittalk;
finishtalk:
	context.bx = context.data.word(249);
	context.es = context.cs;
	context._cmp(context.data.byte(246), 4);
	if (context.flags.c()) goto notnexttalk;
	context.al = context.data.byte(context.bx+7);
	context._or(context.al, 128);
	context.data.byte(context.bx+7) = context.al;
notnexttalk:
	redrawmainscrn(context);
	worktoscreenm(context);
	context._cmp(context.data.byte(383), 1);
	if (!context.flags.z()) goto nospeech;
	cancelch1(context);
	context.data.byte(388) = -1;
	context.data.byte(387) = 0;
nospeech:
	return;
}

static inline void convicons(Context & context) {
	context.al = context.data.byte(247);
	context._and(context.al, 127);
	getpersframe(context);
	context.di = 234;
	context.bx = 2;
	context.data.word(160) = context.ax;
	findsource(context);
	context.ax = context.data.word(160);
	context._sub(context.ax, context.data.word(244));
	context.ah = 0;
	showframe(context);
	return;
}

static inline void getpersframe(Context & context) {
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.es = context.data.word(448);
	context._add(context.bx, 0);
	context.ax = context.data.word(context.bx);
	return;
}

static inline void starttalk(Context & context) {
	context.data.byte(245) = 0;
	context.al = context.data.byte(247);
	context._and(context.al, 127);
	getpersontext(context);
	context.data.word(71) = 91+91;
	context.di = 66;
	context.bx = 64;
	context.dl = 241;
	context.al = 0;
	context.ah = 79;
	printdirect(context);
	context.data.word(71) = 0;
	context.di = 66;
	context.bx = 80;
	context.dl = 241;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
nospeech1:
	return;
}

static inline void getpersontext(Context & context) {
	context.ah = 0;
	context.cx = 64*2;
	context._mul(context.cx);
	context.si = context.ax;
	context.es = context.data.word(448);
	context._add(context.si, 0+24);
	context.cx = 0+24+(1026*2);
	context.ax = context.data.word(context.si);
	context._add(context.ax, context.cx);
	context.si = context.ax;
	return;
}

static inline void moretalk(Context & context) {
	context._cmp(context.data.byte(245), 0);
	if (context.flags.z()) goto canmore;
	redes(context);
	return;
canmore:
	context._cmp(context.data.byte(100), 215);
	if (context.flags.z()) goto alreadymore;
	context.data.byte(100) = 215;
	context.al = 49;
	commandonly(context);
alreadymore:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto nomore;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto domoretalk;
nomore:
	return;
domoretalk:
	context.data.byte(245) = 2;
	context.data.byte(246) = 4;
	context._cmp(context.data.byte(247), 100);
	if (context.flags.c()) goto notsecondpart;
	context.data.byte(246) = 48;
notsecondpart:
	dosometalk(context);
	return;
}

static inline void dosometalk(Context & context) {
watchtalk:
	context.al = context.data.byte(246);
	context.al = context.data.byte(247);
	context._and(context.al, 127);
	context.ah = 0;
	context.cx = 64;
	context._mul(context.cx);
	context.cx = context.ax;
	context.al = context.data.byte(246);
	context.ah = 0;
	context._add(context.ax, context.cx);
	context._add(context.ax, context.ax);
	context.si = context.ax;
	context.es = context.data.word(448);
	context._add(context.si, 0+24);
	context.cx = 0+24+(1026*2);
	context.ax = context.data.word(context.si);
	context._add(context.ax, context.cx);
	context.si = context.ax;
	context._cmp(context.data.byte(context.si), 0);
	if (context.flags.z()) goto endwatchtalk;
	context.push(context.es);
	context.push(context.si);
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	convicons(context);
	context.si = context.pop();
	context.es = context.pop();
	context.di = 164;
	context.bx = 64;
	context.dl = 144;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.data.byte(234) = 3;
	worktoscreenm(context);
	context.cx = 180;
	hangonpq(context);
	context._add(context.data.byte(246), 1);
	context.al = context.data.byte(246);
	context.al = context.data.byte(247);
	context._and(context.al, 127);
	context.ah = 0;
	context.cx = 64;
	context._mul(context.cx);
	context.cx = context.ax;
	context.al = context.data.byte(246);
	context.ah = 0;
	context._add(context.ax, context.cx);
	context._add(context.ax, context.ax);
	context.si = context.ax;
	context.es = context.data.word(448);
	context._add(context.si, 0+24);
	context.cx = 0+24+(1026*2);
	context.ax = context.data.word(context.si);
	context._add(context.ax, context.cx);
	context.si = context.ax;
	context._cmp(context.data.byte(context.si), 0);
	if (context.flags.z()) goto endwatchtalk;
	context._cmp(context.data.byte(context.si), ':');
	if (context.flags.z()) goto skiptalk;
	context._cmp(context.data.byte(context.si), 32);
	if (context.flags.z()) goto skiptalk;
	context.push(context.es);
	context.push(context.si);
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	convicons(context);
	context.si = context.pop();
	context.es = context.pop();
	context.di = 48;
	context.bx = 128;
	context.dl = 144;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.data.byte(234) = 3;
	worktoscreenm(context);
	context.cx = 180;
	hangonpq(context);
skiptalk:
	context._add(context.data.byte(246), 1);
	goto watchtalk;
endwatchtalk:
	context.data.byte(234) = 0;
	return;
}

static inline void hangonpq(Context & context) {
	context.data.byte(103) = 0;
	context.bx = 0;
hangloopq:
	context.push(context.cx);
	context.push(context.bx);
	delpointer(context);
	readmouse(context);
	animpointer(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.bx = 2692;
	checkcoords(context);
	context.bx = context.pop();
	context.cx = context.pop();
	context._cmp(context.data.byte(103), 1);
	if (context.flags.z()) goto quitconv;
	context._cmp(context.data.byte(383), 1);
	if (!context.flags.z()) goto notspeaking;
	context._cmp(context.data.byte(515), 255);
	if (!context.flags.z()) goto notspeaking;
	context._add(context.bx, 1);
	context._cmp(context.bx, 40);
	if (context.flags.z()) goto finishconv;
notspeaking:
	context._cmp(context.data.word(202), 0);
	if (context.flags.z()) goto hangloopq;
	context._cmp(context.data.word(212), 0);
	if (!context.flags.z()) goto hangloopq;
finishconv:
	delpointer(context);
	context.data.byte(234) = 0;
	return;
quitconv:
	delpointer(context);
	context.data.byte(234) = 0;
	context.ax = context.pop();
	cancelch1(context);
	return;
}

static inline void redes(Context & context) {
	context._cmp(context.data.byte(515), 255);
	if (!context.flags.z()) goto cantredes;
	context._cmp(context.data.byte(245), 2);
	if (context.flags.z()) goto canredes;
cantredes:
	blank(context);
	return;
canredes:
	context._cmp(context.data.byte(100), 217);
	if (context.flags.z()) goto alreadyreds;
	context.data.byte(100) = 217;
	context.al = 50;
	commandonly(context);
alreadyreds:
	context.ax = context.data.word(202);
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doredes;
	return;
doredes:
	delpointer(context);
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	convicons(context);
	starttalk(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void lookatplace(Context & context) {
	context._cmp(context.data.byte(100), 224);
	if (context.flags.z()) goto alreadyinfo;
	context.data.byte(100) = 224;
	context.al = 27;
	commandonly(context);
alreadyinfo:
	context.ax = context.data.word(202);
	context._and(context.ax, 1);
	if (context.flags.z()) goto noinfo;
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto noinfo;
	context.bl = context.data.byte(183);
	context._cmp(context.bl, 15);
	if (!context.flags.c()) goto noinfo;
	context.push(context.bx);
	delpointer(context);
	deltextline(context);
	getundercentre(context);
	context.ds = context.data.word(462);
	context.al = 0;
	context.ah = 0;
	context.di = 60;
	context.bx = 72;
	showframe(context);
	context.al = 4;
	context.ah = 0;
	context.di = 60;
	context.bx = 72+55;
	showframe(context);
	context.al = 4;
	context.ah = 0;
	context.di = 60;
	context.bx = 72+55+21;
	showframe(context);
	context.bx = context.pop();
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(456);
	context.si = context.data.word(context.bx);
	context._add(context.si, 66*2);
	findnextcolon(context);
	context.di = 63;
	context.bx = 84+4;
	context.dl = 191;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	worktoscreenm(context);
	context.cx = 500;
	hangonp(context);
afterinfo:
	context.data.byte(234) = 0;
	context.data.byte(231) = 0;
	putundercentre(context);
	worktoscreenm(context);
noinfo:
	return;
}

static inline void getundercentre(Context & context) {
	context.di = 58;
	context.bx = 72;
	context.ds = context.data.word(402);
	context.si = 0;
	context.cl = 254;
	context.ch = 110;
	multiget(context);
	return;
}

static inline void putundercentre(Context & context) {
	context.di = 58;
	context.bx = 72;
	context.ds = context.data.word(402);
	context.si = 0;
	context.cl = 254;
	context.ch = 110;
	multiput(context);
	return;
}

static inline void nextdest(Context & context) {
duok:
	context._cmp(context.data.byte(100), 218);
	if (context.flags.z()) goto alreadydu;
	context.data.byte(100) = 218;
	context.al = 28;
	commandonly(context);
alreadydu:
	context.ax = context.data.word(202);
	context._and(context.ax, 1);
	if (context.flags.z()) goto nodu;
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto nodu;
searchdestup:
	context._add(context.data.byte(183), 1);
	context._cmp(context.data.byte(183), 15);
	if (!context.flags.z()) goto notlastdest;
	context.data.byte(183) = 0;
notlastdest:
	getdestinfo(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto searchdestup;
	context.data.byte(109) = 1;
	deltextline(context);
	delpointer(context);
	showpanel(context);
	showman(context);
	showarrows(context);
	locationpic(context);
	undertextline(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
nodu:
	return;
}

static inline void lastdest(Context & context) {
ddok:
	context._cmp(context.data.byte(100), 219);
	if (context.flags.z()) goto alreadydd;
	context.data.byte(100) = 219;
	context.al = 29;
	commandonly(context);
alreadydd:
	context.ax = context.data.word(202);
	context._and(context.ax, 1);
	if (context.flags.z()) goto nodd;
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto nodd;
searchdestdown:
	context._sub(context.data.byte(183), 1);
	context._cmp(context.data.byte(183), -1);
	if (!context.flags.z()) goto notfirstdest;
	context.data.byte(183) = 15;
notfirstdest:
	getdestinfo(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto searchdestdown;
	context.data.byte(109) = 1;
	deltextline(context);
	delpointer(context);
	showpanel(context);
	showman(context);
	showarrows(context);
	locationpic(context);
	undertextline(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
nodd:
	return;
}

static inline void destselect(Context & context) {
	context._cmp(context.data.byte(100), 222);
	if (context.flags.z()) goto alreadytrav;
	context.data.byte(100) = 222;
	context.al = 30;
	commandonly(context);
alreadytrav:
	context.ax = context.data.word(202);
	context._and(context.ax, 1);
	if (context.flags.z()) goto notrav;
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notrav;
	getdestinfo(context);
	context.al = context.data.byte(183);
	context.data.byte(188) = context.al;
notrav:
	return;
}

static inline void usemon(Context & context) {
	context.data.byte(55) = 0;
	context.es = context.cs;
	context.di = 2895+1;
	context.cx = 12;
	context.al = 32;
	while(--context.cx) 	context._stosb();
	context.es = context.cs;
	context.di = 2883+1;
	context.cx = 12;
	context.al = 32;
	while(--context.cx) 	context._stosb();
	context.es = context.cs;
	context.di = 2807;
	context.data.byte(context.di) = 1;
	context._add(context.di, 26);
	context.cx = 3;
keyloop:
	context.data.byte(context.di) = 0;
	context._add(context.di, 26);
	if (--context.cx) goto keyloop;
	createpanel(context);
	showpanel(context);
	showicon(context);
	drawfloor(context);
	getridofall(context);
	context.dx = 1987;
	loadintotemp(context);
	loadpersonal(context);
	loadnews(context);
	loadcart(context);
	context.dx = 1883;
	loadtempcharset(context);
	printoutermon(context);
	initialmoncols(context);
	printlogo(context);
	worktoscreen(context);
	turnonpower(context);
	fadeupyellows(context);
	fadeupmonfirst(context);
	context.data.word(318) = 76;
	context.data.word(320) = 141;
	context.al = 1;
	monmessage(context);
	context.cx = 120;
	hangoncurs(context);
	context.al = 2;
	monmessage(context);
	context.cx = 60;
	randomaccess(context);
	context.al = 3;
	monmessage(context);
	context.cx = 100;
	hangoncurs(context);
	printlogo(context);
	scrollmonitor(context);
	context.data.word(394) = 0;
	context.data.word(396) = 0;
moreinput:
	context.di = context.data.word(318);
	context.bx = context.data.word(320);
	context.push(context.di);
	context.push(context.bx);
	input(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.data.word(318) = context.di;
	context.data.word(320) = context.bx;
	execcommand(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto moreinput;
endmon:
	getridoftemp(context);
	getridoftempcharset(context);
	context.es = context.data.word(466);
	deallocatemem(context);
	context.es = context.data.word(468);
	deallocatemem(context);
	context.es = context.data.word(470);
	deallocatemem(context);
	context.data.byte(103) = 1;
	context.al = 26;
	playchannel1(context);
	context.data.byte(131) = 0;
	restoreall(context);
	redrawmainscrn(context);
	worktoscreenm(context);
	return;
}

static inline void printoutermon(Context & context) {
	context.di = 40;
	context.bx = 32;
	context.ds = context.data.word(458);
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.di = 264;
	context.bx = 32;
	context.ds = context.data.word(458);
	context.al = 2;
	context.ah = 0;
	showframe(context);
	context.di = 40;
	context.bx = 12;
	context.ds = context.data.word(458);
	context.al = 3;
	context.ah = 0;
	showframe(context);
	context.di = 40;
	context.bx = 164;
	context.ds = context.data.word(458);
	context.al = 4;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void loadpersonal(Context & context) {
	context.al = context.data.byte(9);
	context.dx = 2065;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundpersonal;
	context._cmp(context.al, 42);
	if (context.flags.z()) goto foundpersonal;
	context.dx = 2078;
	context._cmp(context.al, 2);
	if (context.flags.z()) goto foundpersonal;
foundpersonal:
	openfile(context);
	readheader(context);
	context.bx = context.data.word(context.di);
	context.push(context.bx);
	context.cl = 4;
	context._shr(context.bx, context.cl);
	allocatemem(context);
	context.data.word(466) = context.ax;
	context.ds = context.ax;
	context.cx = context.pop();
	context.dx = 0;
	readfromfile(context);
	closefile(context);
	return;
}

static inline void loadnews(Context & context) {
	context.al = context.data.byte(34);
	context.dx = 2091;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundnews;
	context.dx = 2104;
	context._cmp(context.al, 1);
	if (context.flags.z()) goto foundnews;
	context.dx = 2117;
	context._cmp(context.al, 2);
	if (context.flags.z()) goto foundnews;
	context.dx = 2130;
foundnews:
	openfile(context);
	readheader(context);
	context.bx = context.data.word(context.di);
	context.push(context.bx);
	context.cl = 4;
	context._shr(context.bx, context.cl);
	allocatemem(context);
	context.data.word(468) = context.ax;
	context.ds = context.ax;
	context.cx = context.pop();
	context.dx = 0;
	readfromfile(context);
	closefile(context);
	return;
}

static inline void loadcart(Context & context) {
	lookininterface(context);
	context.dx = 2143;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto gotcart;
	context.dx = 2156;
	context._cmp(context.al, 1);
	if (context.flags.z()) goto gotcart;
	context.dx = 2169;
	context._cmp(context.al, 2);
	if (context.flags.z()) goto gotcart;
	context.dx = 2182;
	context._cmp(context.al, 3);
	if (context.flags.z()) goto gotcart;
	context.dx = 2195;
gotcart:
	openfile(context);
	readheader(context);
	context.bx = context.data.word(context.di);
	context.push(context.bx);
	context.cl = 4;
	context._shr(context.bx, context.cl);
	allocatemem(context);
	context.data.word(470) = context.ax;
	context.ds = context.ax;
	context.cx = context.pop();
	context.dx = 0;
	readfromfile(context);
	closefile(context);
	return;
}

static inline void lookininterface(Context & context) {
	context.al = 'I';
	context.ah = 'N';
	context.cl = 'T';
	context.ch = 'F';
	findsetobject(context);
	context.ah = 1;
	checkinside(context);
	context._cmp(context.cl, 114);
	if (context.flags.z()) goto emptyinterface;
	context.al = context.data.byte(context.bx+15);
	context._add(context.al, 1);
	return;
emptyinterface:
	context.al = 0;
	return;
}

static inline void turnonpower(Context & context) {
	context.cx = 3;
powerloop:
	context.push(context.cx);
	powerlighton(context);
	context.cx = 30;
	hangon(context);
	powerlightoff(context);
	context.cx = 30;
	hangon(context);
	context.cx = context.pop();
	if (--context.cx) goto powerloop;
	powerlighton(context);
	return;
}

static inline void randomaccess(Context & context) {
accessloop:
	context.push(context.cx);
	vsync(context);
	vsync(context);
	randomnum1(context);
	context._and(context.al, 15);
	context._cmp(context.al, 10);
	if (context.flags.c()) goto off;
	accesslighton(context);
	goto chosenaccess;
off:
	accesslightoff(context);
chosenaccess:
	context.cx = context.pop();
	if (--context.cx) goto accessloop;
	accesslightoff(context);
	return;
}

static inline void powerlighton(Context & context) {
	context.di = 257+4;
	context.bx = 182;
	context.ds = context.data.word(458);
	context.al = 6;
	context.ah = 0;
	context.push(context.di);
	context.push(context.bx);
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.cl = 12;
	context.ch = 8;
	multidump(context);
	return;
}

static inline void powerlightoff(Context & context) {
	context.di = 257+4;
	context.bx = 182;
	context.ds = context.data.word(458);
	context.al = 5;
	context.ah = 0;
	context.push(context.di);
	context.push(context.bx);
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.cl = 12;
	context.ch = 8;
	multidump(context);
	return;
}

static inline void accesslighton(Context & context) {
	context.di = 74;
	context.bx = 182;
	context.ds = context.data.word(458);
	context.al = 8;
	context.ah = 0;
	context.push(context.di);
	context.push(context.bx);
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.cl = 12;
	context.ch = 8;
	multidump(context);
	return;
}

static inline void accesslightoff(Context & context) {
	context.di = 74;
	context.bx = 182;
	context.ds = context.data.word(458);
	context.al = 7;
	context.ah = 0;
	context.push(context.di);
	context.push(context.bx);
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.cl = 12;
	context.ch = 8;
	multidump(context);
	return;
}

static inline void locklighton(Context & context) {
	context.di = 56;
	context.bx = 182;
	context.ds = context.data.word(458);
	context.al = 10;
	context.ah = 0;
	context.push(context.di);
	context.push(context.bx);
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.cl = 12;
	context.ch = 8;
	multidump(context);
	return;
}

static inline void locklightoff(Context & context) {
	context.di = 56;
	context.bx = 182;
	context.ds = context.data.word(458);
	context.al = 9;
	context.ah = 0;
	context.push(context.di);
	context.push(context.bx);
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.cl = 12;
	context.ch = 8;
	multidump(context);
	return;
}

static inline void input(Context & context) {
	context.es = context.cs;
	context.di = 7944;
	context.cx = 64;
	context.al = 0;
	while(--context.cx) 	context._stosb();
	context.data.word(316) = 0;
	context.al = '>';
	context.di = context.data.word(318);
	context.bx = context.data.word(320);
	context.ds = context.data.word(406);
	context.ah = 0;
	printchar(context);
	context.di = context.data.word(318);
	context.bx = context.data.word(320);
	context.cl = 6;
	context.ch = 8;
	multidump(context);
	context._add(context.data.word(318), 6);
	context.ax = context.data.word(318);
	context.data.word(312) = context.ax;
	context.ax = context.data.word(320);
	context.data.word(314) = context.ax;
waitkey:
	printcurs(context);
	vsync(context);
	delcurs(context);
	readkey(context);
	context.al = context.data.byte(142);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto waitkey;
	context._cmp(context.al, 13);
	if (context.flags.z()) goto endofinput;
	context._cmp(context.al, 8);
	if (!context.flags.z()) goto notdel;
	context._cmp(context.data.word(316), 0);
	if (context.flags.z()) goto waitkey;
	delchar(context);
	goto waitkey;
notdel:
	context._cmp(context.data.word(316), 28);
	if (context.flags.z()) goto waitkey;
	context._cmp(context.data.byte(142), 32);
	if (!context.flags.z()) goto notleadingspace;
	context._cmp(context.data.word(316), 0);
	if (context.flags.z()) goto waitkey;
notleadingspace:
	makecaps(context);
	context.es = context.cs;
	context.si = context.data.word(316);
	context._add(context.si, context.si);
	context._add(context.si, 7944);
	context.data.byte(context.si) = context.al;
	context._cmp(context.al, 'Z'+1);
	if (!context.flags.c()) goto waitkey;
	context.push(context.ax);
	context.push(context.es);
	context.push(context.si);
	context.di = context.data.word(318);
	context.bx = context.data.word(320);
	context.ds = context.data.word(402);
	context.ax = context.data.word(316);
	context._xchg(context.al, context.ah);
	context.si = context.ax;
	context.cl = 8;
	context.ch = 8;
	multiget(context);
	context.si = context.pop();
	context.es = context.pop();
	context.ax = context.pop();
	context.push(context.es);
	context.push(context.si);
	context.di = context.data.word(318);
	context.bx = context.data.word(320);
	context.ds = context.data.word(406);
	context.ah = 0;
	printchar(context);
	context.si = context.pop();
	context.es = context.pop();
	context.data.byte(context.si+1) = context.cl;
	context.ch = 0;
	context._add(context.data.word(318), context.cx);
	context._add(context.data.word(316), 1);
	context._add(context.data.word(312), context.cx);
	goto waitkey;
endofinput:
	return;
}

static inline void makecaps(Context & context) {
	context._cmp(context.al, 'a');
	if (context.flags.c()) goto notupperc;
	context._sub(context.al, 32);
notupperc:
	return;
}

static inline void delchar(Context & context) {
	context._sub(context.data.word(316), 1);
	context.si = context.data.word(316);
	context._add(context.si, context.si);
	context.es = context.cs;
	context._add(context.si, 7944);
	context.data.byte(context.si) = 0;
	context.al = context.data.byte(context.si+1);
	context.ah = 0;
	context._sub(context.data.word(318), context.ax);
	context._sub(context.data.word(312), context.ax);
	context.di = context.data.word(318);
	context.bx = context.data.word(320);
	context.ds = context.data.word(402);
	context.ax = context.data.word(316);
	context._xchg(context.al, context.ah);
	context.si = context.ax;
	context.cl = 8;
	context.ch = 8;
	multiput(context);
	context.di = context.data.word(318);
	context.bx = context.data.word(320);
	context.cl = context.al;
	context.ch = 8;
	multidump(context);
	return;
}

static inline void execcommand(Context & context) {
	context.es = context.cs;
	context.bx = 2776;
	context.ds = context.cs;
	context.si = 7944;
	context.al = context.data.byte(context.si);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto notblankinp;
	scrollmonitor(context);
	return;
notblankinp:
	context.cl = 0;
comloop:
	context.push(context.bx);
	context.push(context.si);
comloop2:
	context.al = context.data.byte(context.si);
	context._add(context.si, 2);
	context.ah = context.data.byte(context.bx);
	context._add(context.bx, 1);
	context._cmp(context.ah, 32);
	if (context.flags.z()) goto foundcom;
	context._cmp(context.al, context.ah);
	if (context.flags.z()) goto comloop2;
	context.si = context.pop();
	context.bx = context.pop();
	context._add(context.bx, 10);
	context._add(context.cl, 1);
	context._cmp(context.cl, 6);
	if (!context.flags.z()) goto comloop;
	neterror(context);
	context.al = 0;
	return;
foundcom:
	context.si = context.pop();
	context.bx = context.pop();
	context._cmp(context.cl, 1);
	if (context.flags.z()) goto testcom;
	context._cmp(context.cl, 2);
	if (context.flags.z()) goto directory;
	context._cmp(context.cl, 3);
	if (context.flags.z()) goto accesscom;
	context._cmp(context.cl, 4);
	if (context.flags.z()) goto signoncom;
	context._cmp(context.cl, 5);
	if (context.flags.z()) goto keyscom;
	goto quitcom;
directory:
	dircom(context);
	context.al = 0;
	return;
signoncom:
	signon(context);
	context.al = 0;
	return;
accesscom:
	read(context);
	context.al = 0;
	return;
keyscom:
	showkeys(context);
	context.al = 0;
	return;
testcom:
	context.al = 6;
	monmessage(context);
	context.al = 0;
	return;
quitcom:
	context.al = 1;
	return;
}

static inline void neterror(Context & context) {
	context.al = 5;
	monmessage(context);
	scrollmonitor(context);
	return;
}

static inline void dircom(Context & context) {
	context.cx = 30;
	randomaccess(context);
	parser(context);
	context._cmp(context.data.byte(context.di+1), 0);
	if (context.flags.z()) goto dirroot;
	dirfile(context);
	return;
dirroot:
	context.data.byte(269) = 0;
	context.ds = context.cs;
	context.si = 2885;
	context._add(context.si, 1);
	context.es = context.cs;
	context.di = 2895;
	context._add(context.di, 1);
	context.cx = 12;
	while(--context.cx) 	context._movsb();
 	monitorlogo(context);
	scrollmonitor(context);
	context.al = 9;
	monmessage(context);
	context.es = context.data.word(466);
	searchforfiles(context);
	context.es = context.data.word(468);
	searchforfiles(context);
	context.es = context.data.word(470);
	searchforfiles(context);
	scrollmonitor(context);
	return;
}

static inline void searchforfiles(Context & context) {
	context.bx = 66*2;
directloop1:
	context.al = context.data.byte(context.bx);
	context._add(context.bx, 1);
	context._cmp(context.al, '*');
	if (context.flags.z()) goto endofdir;
	context._cmp(context.al, 34);
	if (!context.flags.z()) goto directloop1;
	monprint(context);
	goto directloop1;
endofdir:
	return;
}

static inline void signon(Context & context) {
	parser(context);
	context._add(context.di, 1);
	context.ds = context.cs;
	context.si = 2807;
	context.cx = 4;
signonloop:
	context.push(context.cx);
	context.push(context.si);
	context.push(context.di);
	context._add(context.si, 14);
	context.cx = 11;
signonloop2:
	context._lodsb();
	context._cmp(context.al, 32);
	if (context.flags.z()) goto foundsign;
	makecaps(context);
	context.ah = context.data.byte(context.di);
	context._add(context.di, 1);
	context._cmp(context.al, context.ah);
	if (!context.flags.z()) goto nomatch;
	if (--context.cx) goto signonloop2;
nomatch:
	context.di = context.pop();
	context.si = context.pop();
	context.cx = context.pop();
	context._add(context.si, 26);
	if (--context.cx) goto signonloop;
	context.al = 13;
	monmessage(context);
	return;
foundsign:
	context.di = context.pop();
	context.si = context.pop();
	context.cx = context.pop();
	context.bx = context.si;
	context.es = context.ds;
	context._cmp(context.data.byte(context.bx), 0);
	if (context.flags.z()) goto notyetassigned;
	context.al = 17;
	monmessage(context);
	return;
notyetassigned:
	context.push(context.es);
	context.push(context.bx);
	scrollmonitor(context);
	context.al = 15;
	monmessage(context);
	context.di = context.data.word(318);
	context.bx = context.data.word(320);
	context.push(context.di);
	context.push(context.bx);
	input(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.data.word(318) = context.di;
	context.data.word(320) = context.bx;
	context.bx = context.pop();
	context.es = context.pop();
	context.push(context.es);
	context.push(context.bx);
	context._add(context.bx, 2);
	context.ds = context.cs;
	context.si = 7944;
checkpass:
	context._lodsw();
	context.ah = context.data.byte(context.bx);
	context._add(context.bx, 1);
	context._cmp(context.ah, 32);
	if (context.flags.z()) goto passpassed;
	context._cmp(context.al, context.ah);
	if (context.flags.z()) goto checkpass;
passerror:
	context.bx = context.pop();
	context.es = context.pop();
	scrollmonitor(context);
	context.al = 16;
	monmessage(context);
	return;
passpassed:
	context.al = 14;
	monmessage(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.push(context.es);
	context.push(context.bx);
	context._add(context.bx, 14);
	monprint(context);
	scrollmonitor(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.data.byte(context.bx) = 1;
	return;
}

static inline void showkeys(Context & context) {
	context.cx = 10;
	randomaccess(context);
	scrollmonitor(context);
	context.al = 18;
	monmessage(context);
	context.es = context.cs;
	context.bx = 2807;
	context.cx = 4;
keysloop:
	context.push(context.cx);
	context.push(context.bx);
	context._cmp(context.data.byte(context.bx), 0);
	if (context.flags.z()) goto notheld;
	context._add(context.bx, 14);
	monprint(context);
notheld:
	context.bx = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 26);
	if (--context.cx) goto keysloop;
	scrollmonitor(context);
	return;
}

static inline void read(Context & context) {
	context.cx = 40;
	randomaccess(context);
	parser(context);
	context._cmp(context.data.byte(context.di+1), 0);
	if (!context.flags.z()) goto okcom;
	neterror(context);
	return;
okcom:
	context.es = context.cs;
	context.di = 2895;
	context.ax = context.data.word(466);
	context.data.word(324) = context.ax;
	context.ds = context.ax;
	context.si = 66*2;
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile2;
	context.ax = context.data.word(468);
	context.data.word(324) = context.ax;
	context.ds = context.ax;
	context.si = 66*2;
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile2;
	context.ax = context.data.word(470);
	context.data.word(324) = context.ax;
	context.ds = context.ax;
	context.si = 66*2;
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile2;
	context.al = 7;
	monmessage(context);
	return;
foundfile2:
	getkeyandlogo(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto keyok1;
	return;
keyok1:
	context.es = context.cs;
	context.di = 2883;
	context.ds = context.data.word(324);
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto findtopictext;
	context.al = context.data.byte(270);
	context.data.byte(269) = context.al;
	context.al = 11;
	monmessage(context);
	return;
findtopictext:
	context._add(context.bx, 1);
	context.push(context.es);
	context.push(context.bx);
	monitorlogo(context);
	scrollmonitor(context);
	context.bx = context.pop();
	context.es = context.pop();
moretopic:
	monprint(context);
	context.al = context.data.byte(context.bx);
	context._cmp(context.al, 34);
	if (context.flags.z()) goto endoftopic;
	context._cmp(context.al, '=');
	if (context.flags.z()) goto endoftopic;
	context._cmp(context.al, '*');
	if (context.flags.z()) goto endoftopic;
	context.push(context.es);
	context.push(context.bx);
	processtrigger(context);
	context.cx = 24;
	randomaccess(context);
	context.bx = context.pop();
	context.es = context.pop();
	goto moretopic;
endoftopic:
	scrollmonitor(context);
	return;
}

static inline void dirfile(Context & context) {
	context.al = 34;
	context.data.byte(context.di) = context.al;
	context.push(context.es);
	context.push(context.di);
	context.ds = context.data.word(466);
	context.si = 66*2;
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile;
	context.di = context.pop();
	context.es = context.pop();
	context.push(context.es);
	context.push(context.di);
	context.ds = context.data.word(468);
	context.si = 66*2;
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile;
	context.di = context.pop();
	context.es = context.pop();
	context.push(context.es);
	context.push(context.di);
	context.ds = context.data.word(470);
	context.si = 66*2;
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile;
	context.di = context.pop();
	context.es = context.pop();
	context.al = 7;
	monmessage(context);
	return;
foundfile:
	context.ax = context.pop();
	context.ax = context.pop();
	getkeyandlogo(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto keyok2;
	return;
keyok2:
	context.push(context.es);
	context.push(context.bx);
	context.ds = context.cs;
	context.si = 2883+1;
	context.es = context.cs;
	context.di = 2895+1;
	context.cx = 12;
	while(--context.cx) 	context._movsb();
 	monitorlogo(context);
	scrollmonitor(context);
	context.al = 10;
	monmessage(context);
	context.bx = context.pop();
	context.es = context.pop();
directloop2:
	context.al = context.data.byte(context.bx);
	context._add(context.bx, 1);
	context._cmp(context.al, 34);
	if (context.flags.z()) goto endofdir2;
	context._cmp(context.al, '*');
	if (context.flags.z()) goto endofdir2;
	context._cmp(context.al, '=');
	if (!context.flags.z()) goto directloop2;
	monprint(context);
	goto directloop2;
endofdir2:
	scrollmonitor(context);
	return;
}

static inline void getkeyandlogo(Context & context) {
	context._add(context.bx, 1);
	context.al = context.data.byte(context.bx);
	context._sub(context.al, 48);
	context.data.byte(271) = context.al;
	context._add(context.bx, 2);
	context.al = context.data.byte(context.bx);
	context._sub(context.al, 48);
	context.data.byte(276) = context.al;
	context._add(context.bx, 1);
	context.push(context.es);
	context.push(context.bx);
	context.al = context.data.byte(276);
	context.ah = 0;
	context.cx = 26;
	context._mul(context.cx);
	context.es = context.cs;
	context.bx = 2807;
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.bx);
	context._cmp(context.al, 1);
	if (context.flags.z()) goto keyok;
	context.push(context.bx);
	context.push(context.es);
	context.al = 12;
	monmessage(context);
	context.es = context.pop();
	context.bx = context.pop();
	context._add(context.bx, 14);
	monprint(context);
	scrollmonitor(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.al = 1;
	return;
keyok:
	context.bx = context.pop();
	context.es = context.pop();
	context.al = context.data.byte(271);
	context.data.byte(269) = context.al;
	context.al = 0;
	return;
}

static inline void searchforstring(Context & context) {
	context.dl = context.data.byte(context.di);
	context.cx = context.di;
restartlook:
	context.di = context.cx;
	context.bx = context.si;
	context.dh = 0;
keeplooking:
	context._lodsb();
	makecaps(context);
	context._cmp(context.al, '*');
	if (context.flags.z()) goto notfound;
	context._cmp(context.dl, '=');
	if (!context.flags.z()) goto nofindingtopic;
	context._cmp(context.al, 34);
	if (context.flags.z()) goto notfound;
nofindingtopic:
	context.ah = context.data.byte(context.di);
	context._cmp(context.al, context.dl);
	if (!context.flags.z()) goto notbracket;
	context._add(context.dh, 1);
	context._cmp(context.dh, 2);
	if (context.flags.z()) goto complete;
notbracket:
	context._cmp(context.al, context.ah);
	if (!context.flags.z()) goto restartlook;
	context._add(context.di, 1);
	goto keeplooking;
complete:
	context.es = context.ds;
	context.al = 0;
	context.bx = context.si;
	return;
notfound:
	context.al = 1;
	return;
}

static inline void parser(Context & context) {
	context.es = context.cs;
	context.di = 2883;
	context.cx = 13;
	context.al = 0;
	while(--context.cx) 	context._stosb();
	context.di = 2883;
	context.al = '=';
	context._stosb();
	context.ds = context.cs;
	context.si = 7944;
notspace1:
	context._lodsw();
	context._cmp(context.al, 32);
	if (context.flags.z()) goto stillspace1;
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto notspace1;
	goto finishpars;
stillspace1:
	context._lodsw();
	context._cmp(context.al, 32);
	if (context.flags.z()) goto stillspace1;
copyin1:
	context._stosb();
	context._lodsw();
	context._cmp(context.al, 0);
	if (context.flags.z()) goto finishpars;
	context._cmp(context.al, 32);
	if (!context.flags.z()) goto copyin1;
finishpars:
	context.di = 2883;
	return;
}

static inline void scrollmonitor(Context & context) {
	context.push(context.ax);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.dx);
	context.push(context.di);
	context.push(context.si);
	context.push(context.es);
	context.push(context.ds);
	printlogo(context);
	context.di = context.data.word(318);
	context.bx = context.data.word(320);
	printundermon(context);
	context.ax = context.data.word(320);
	worktoscreen(context);
	context.al = 25;
	playchannel1(context);
	context.ds = context.pop();
	context.es = context.pop();
	context.si = context.pop();
	context.di = context.pop();
	context.dx = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ax = context.pop();
	return;
}

static inline void lockmon(Context & context) {
	context._cmp(context.data.byte(392), 57);
	if (!context.flags.z()) goto notlock;
	locklighton(context);
lockloop:
	context._cmp(context.data.byte(392), 57);
	if (context.flags.z()) goto lockloop;
	locklightoff(context);
notlock:
	return;
}

static inline void monitorlogo(Context & context) {
	context.al = context.data.byte(269);
	context._cmp(context.al, context.data.byte(270));
	if (context.flags.z()) goto notnewlogo;
	context.data.byte(270) = context.al;
	printlogo(context);
	printundermon(context);
	worktoscreen(context);
	printlogo(context);
	printlogo(context);
	context.al = 26;
	playchannel1(context);
	context.cx = 20;
	randomaccess(context);
	return;
notnewlogo:
	printlogo(context);
	return;
}

static inline void printlogo(Context & context) {
	context.di = 56;
	context.bx = 32;
	context.ds = context.data.word(458);
	context.al = 0;
	context.ah = 0;
	showframe(context);
	showcurrentfile(context);
	return;
}

static inline void showcurrentfile(Context & context) {
	context.di = 178;
	context.bx = 37;
	context.si = 2895+1;
curfileloop:
	context.al = context.data.byte(context.si);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto finishfile;
	context._add(context.si, 1);
	context.push(context.si);
	modifychar(context);
	context.ds = context.data.word(406);
	context.ah = 0;
	printchar(context);
	context.si = context.pop();
	goto curfileloop;
finishfile:
	return;
}

static inline void monmessage(Context & context) {
	context.es = context.data.word(466);
	context.bx = 66*2;
	context.cl = context.al;
	context.ch = 0;
monmessageloop:
	context.al = context.data.byte(context.bx);
	context._add(context.bx, 1);
	context._cmp(context.al, '+');
	if (!context.flags.z()) goto monmessageloop;
	if (--context.cx) goto monmessageloop;
	monprint(context);
	return;
}

static inline void processtrigger(Context & context) {
	context._cmp(context.data.byte(55), '1');
	if (!context.flags.z()) goto notfirsttrigger;
	context.al = 8;
	setlocation(context);
	context.al = 45;
	triggermessage(context);
	return;
notfirsttrigger:
	context._cmp(context.data.byte(55), '2');
	if (!context.flags.z()) goto notsecondtrigger;
	context.al = 9;
	setlocation(context);
	context.al = 55;
	triggermessage(context);
	return;
notsecondtrigger:
	context._cmp(context.data.byte(55), '3');
	if (!context.flags.z()) goto notthirdtrigger;
	context.al = 2;
	setlocation(context);
	context.al = 59;
	triggermessage(context);
notthirdtrigger:
	return;
}

static inline void triggermessage(Context & context) {
	context.push(context.ax);
	context.di = 174;
	context.bx = 153;
	context.cl = 200;
	context.ch = 63;
	context.ds = context.data.word(402);
	context.si = 0;
	multiget(context);
	context.ax = context.pop();
	findpuztext(context);
	context.di = 174;
	context.bx = 156;
	context.dl = 141;
	context.ah = 16;
	printdirect(context);
	context.cx = 140;
	hangon(context);
	worktoscreen(context);
	context.cx = 340;
	hangon(context);
	context.di = 174;
	context.bx = 153;
	context.cl = 200;
	context.ch = 63;
	context.ds = context.data.word(402);
	context.si = 0;
	multiput(context);
	worktoscreen(context);
	context.data.byte(55) = 0;
	return;
}

static inline void printcurs(Context & context) {
	context.push(context.si);
	context.push(context.di);
	context.push(context.ds);
	context.push(context.dx);
	context.push(context.bx);
	context.push(context.es);
	context.di = context.data.word(312);
	context.bx = context.data.word(314);
	context.cl = 6;
	context.ch = 8;
	context._sub(context.bx, 3);
	context.ch = 11;
	context.ds = context.data.word(412);
	context.si = 0;
	context.push(context.di);
	context.push(context.bx);
	multiget(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.push(context.bx);
	context.push(context.di);
	context._add(context.data.word(138), 1);
	context.ax = context.data.word(138);
	context._and(context.al, 16);
	if (!context.flags.z()) goto flashcurs;
	context.al = '/';
	context._sub(context.al, 32);
	context.ah = 0;
	context.ds = context.data.word(406);
	showframe(context);
flashcurs:
	context.di = context.pop();
	context.bx = context.pop();
	context._sub(context.di, 6);
	context.cl = 12;
	context.ch = 11;
	multidump(context);
	context.es = context.pop();
	context.bx = context.pop();
	context.dx = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	context.si = context.pop();
	return;
}

static inline void delcurs(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context.push(context.di);
	context.push(context.ds);
	context.push(context.dx);
	context.push(context.si);
	context.di = context.data.word(312);
	context.bx = context.data.word(314);
	context.cl = 6;
	context.ch = 8;
	context._sub(context.bx, 3);
	context.ch = 11;
	context.push(context.di);
	context.push(context.bx);
	context.push(context.cx);
	context.ds = context.data.word(412);
	context.si = 0;
	multiput(context);
	context.cx = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	multidump(context);
finishcurdel:
	context.si = context.pop();
	context.dx = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void useobject(Context & context) {
	context.data.byte(95) = 255;
	context._cmp(context.data.byte(100), 229);
	if (context.flags.z()) goto alreadyuse;
	context.data.byte(100) = 229;
	context.bl = context.data.byte(99);
	context.bh = context.data.byte(102);
	context.al = 51;
	commandwithob(context);
alreadyuse:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto nouse;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto douse;
nouse:
	return;
douse:
	useroutine(context);
	return;
}

static inline void useroutine(Context & context) {
	context._cmp(context.data.byte(184), 50);
	if (context.flags.c()) goto nodream7;
	context._cmp(context.data.byte(232), 0);
	if (!context.flags.z()) goto powerok;
	return;
powerok:
	context.data.byte(232) = 0;
nodream7:
	getanyad(context);
	context.dx = context.ds;
	context.ds = context.dx;
	context.si = 2895;
checkuselist:
	context.push(context.si);
	context._lodsb();
	context._sub(context.al, 'A');
	context._cmp(context.al, context.data.byte(context.bx+12));
	if (!context.flags.z()) goto failed;
	context._lodsb();
	context._sub(context.al, 'A');
	context._cmp(context.al, context.data.byte(context.bx+13));
	if (!context.flags.z()) goto failed;
	context._lodsb();
	context._sub(context.al, 'A');
	context._cmp(context.al, context.data.byte(context.bx+14));
	if (!context.flags.z()) goto failed;
	context._lodsb();
	context._sub(context.al, 'A');
	context._cmp(context.al, context.data.byte(context.bx+15));
	if (!context.flags.z()) goto failed;
	context._lodsw();
	context.si = context.pop();
	__dispatch_call(context, context.ax);
	return;
failed:
	context.si = context.pop();
	context._add(context.si, 6);
	context._cmp(context.data.byte(context.si), 140);
	if (!context.flags.z()) goto checkuselist;
	delpointer(context);
	getobtextstart(context);
	findnextcolon(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto cantuse2;
	findnextcolon(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto cantuse2;
	context.al = context.data.byte(context.si);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto cantuse2;
	usetext(context);
	context.cx = 400;
	hangonp(context);
	putbackobstuff(context);
	return;
cantuse2:
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	obicons(context);
	context.di = 33;
	context.bx = 100;
	context.al = 63;
	context.dl = 241;
	printmessage(context);
	worktoscreenm(context);
	context.cx = 50;
	hangonp(context);
	putbackobstuff(context);
	context.data.byte(100) = 255;
	return;
}

static inline void wheelsound(Context & context) {
	context.al = 17;
	playchannel1(context);
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void runtap(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto tapwith;
	withwhat(context);
	return;
tapwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto fillcupfromtap;
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'F';
	compare(context);
	if (context.flags.z()) goto cupfromtapfull;
	context.cx = 300;
	context.al = 56;
	showpuztext(context);
	putbackobstuff(context);
	return;
fillcupfromtap:
	context.al = context.data.byte(95);
	getexad(context);
	context.data.byte(context.bx+15) = 'F'-'A';
	context.al = 8;
	playchannel1(context);
	context.cx = 300;
	context.al = 57;
	showpuztext(context);
	putbackobstuff(context);
	return;
cupfromtapfull:
	context.cx = 300;
	context.al = 58;
	showpuztext(context);
	putbackobstuff(context);
	return;
}

static inline void playguitar(Context & context) {
	context.al = 14;
	playchannel1(context);
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void hotelcontrol(Context & context) {
	context._cmp(context.data.byte(184), 21);
	if (!context.flags.z()) goto notrightcont;
	context._cmp(context.data.byte(148), 33);
	if (!context.flags.z()) goto notrightcont;
	showfirstuse(context);
	putbackobstuff(context);
	return;
notrightcont:
	showseconduse(context);
	putbackobstuff(context);
	return;
}

static inline void hotelbell(Context & context) {
	context.al = 12;
	playchannel1(context);
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void opentomb(Context & context) {
	context._add(context.data.byte(2), 1);
	showfirstuse(context);
	context.data.word(21) = 35*2;
	context.data.word(23) = 1;
	context.data.word(25) = 33;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void usetrainer(Context & context) {
	getanyad(context);
	context._cmp(context.data.byte(context.bx+2), 4);
	if (!context.flags.z()) goto notheldtrainer;
	context._add(context.data.byte(2), 1);
	makeworn(context);
	showseconduse(context);
	putbackobstuff(context);
	return;
notheldtrainer:
	nothelderror(context);
	return;
}

static inline void nothelderror(Context & context) {
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	obicons(context);
	context.di = 64;
	context.bx = 100;
	context.al = 63;
	context.ah = 1;
	context.dl = 201;
	printmessage2(context);
	worktoscreenm(context);
	context.cx = 50;
	hangonp(context);
	putbackobstuff(context);
	return;
}

static inline void usepipe(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto pipewith;
	withwhat(context);
	return;
pipewith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto fillcup;
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'F';
	compare(context);
	if (context.flags.z()) goto alreadyfull;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
fillcup:
	context.cx = 300;
	context.al = 36;
	showpuztext(context);
	putbackobstuff(context);
	context.al = context.data.byte(95);
	getexad(context);
	context.data.byte(context.bx+15) = 'F'-'A';
	return;
alreadyfull:
	context.cx = 300;
	context.al = 35;
	showpuztext(context);
	putbackobstuff(context);
	return;
}

static inline void usefullcart(Context & context) {
	context._add(context.data.byte(2), 1);
	context.al = 2;
	context.ah = context.data.byte(185);
	context._add(context.ah, 6);
	turnanypathon(context);
	context.data.byte(475) = 4;
	context.data.byte(133) = 4;
	context.data.byte(135) = 4;
	context.data.byte(477) = 4;
	findxyfrompath(context);
	context.data.byte(187) = 1;
	showfirstuse(context);
	context.data.word(21) = 72*2;
	context.data.word(23) = 58;
	context.data.word(25) = 142;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void useplinth(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto plinthwith;
	withwhat(context);
	return;
plinthwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'D';
	context.ch = 'K';
	context.dl = 'E';
	context.dh = 'Y';
	compare(context);
	if (context.flags.z()) goto isrightkey;
	showfirstuse(context);
	putbackobstuff(context);
	return;
isrightkey:
	context._add(context.data.byte(2), 1);
	showseconduse(context);
	context.data.word(21) = 220;
	context.data.word(23) = 0;
	context.data.word(25) = 104;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	context.al = context.data.byte(67);
	context.data.byte(188) = context.al;
	return;
}

static inline void chewy(Context & context) {
	showfirstuse(context);
	getanyad(context);
	context.data.byte(context.bx+2) = 255;
	context.data.byte(103) = 1;
	return;
}

static inline void useladder(Context & context) {
	showfirstuse(context);
	context._sub(context.data.byte(148), 11);
	findroominloc(context);
	context.data.byte(133) = 6;
	context.data.byte(135) = 6;
	context.data.byte(475) = 0;
	context.data.byte(478) = 0;
	context.data.byte(477) = 0;
	findxyfrompath(context);
	context.data.byte(187) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void useladderb(Context & context) {
	showfirstuse(context);
	context._add(context.data.byte(148), 11);
	findroominloc(context);
	context.data.byte(133) = 2;
	context.data.byte(135) = 2;
	context.data.byte(475) = 1;
	context.data.byte(478) = 1;
	context.data.byte(477) = 1;
	findxyfrompath(context);
	context.data.byte(187) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void slabdoora(Context & context) {
	showfirstuse(context);
	context.data.byte(103) = 1;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.word(23) = 13;
	context._cmp(context.data.byte(66), 3);
	if (!context.flags.z()) goto slabawrong;
	context._add(context.data.byte(2), 1);
	context.data.word(21) = 60;
	context.data.word(25) = 42;
	context.data.byte(188) = 47;
	return;
slabawrong:
	context.data.word(21) = 40;
	context.data.word(25) = 34;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	return;
}

static inline void slabdoorb(Context & context) {
	context._cmp(context.data.byte(66), 1);
	if (!context.flags.z()) goto slabbwrong;
	context.al = 'S';
	context.ah = 'H';
	context.cl = 'L';
	context.ch = 'D';
	isryanholding(context);
	if (!context.flags.z()) goto gotcrystal;
	context.al = 44;
	context.cx = 200;
	showpuztext(context);
	putbackobstuff(context);
	return;
gotcrystal:
	showfirstuse(context);
	context._add(context.data.byte(2), 1);
	context.data.byte(103) = 1;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.word(23) = 44;
	context.data.word(21) = 60;
	context.data.word(25) = 71;
	context.data.byte(188) = 47;
	return;
slabbwrong:
	showfirstuse(context);
	context.data.byte(103) = 1;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.word(23) = 44;
	context.data.word(21) = 40;
	context.data.word(25) = 63;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	return;
}

static inline void slabdoord(Context & context) {
	showfirstuse(context);
	context.data.byte(103) = 1;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.word(23) = 75;
	context._cmp(context.data.byte(66), 0);
	if (!context.flags.z()) goto slabcwrong;
	context._add(context.data.byte(2), 1);
	context.data.word(21) = 60;
	context.data.word(25) = 102;
	context.data.byte(188) = 47;
	return;
slabcwrong:
	context.data.word(21) = 40;
	context.data.word(25) = 94;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	return;
}

static inline void slabdoorc(Context & context) {
	showfirstuse(context);
	context.data.byte(103) = 1;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.word(23) = 108;
	context._cmp(context.data.byte(66), 4);
	if (!context.flags.z()) goto slabdwrong;
	context._add(context.data.byte(2), 1);
	context.data.word(21) = 60;
	context.data.word(25) = 135;
	context.data.byte(188) = 47;
	return;
slabdwrong:
	context.data.word(21) = 40;
	context.data.word(25) = 127;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	return;
}

static inline void slabdoore(Context & context) {
	showfirstuse(context);
	context.data.byte(103) = 1;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.word(23) = 141;
	context._cmp(context.data.byte(66), 5);
	if (!context.flags.z()) goto slabewrong;
	context._add(context.data.byte(2), 1);
	context.data.word(21) = 60;
	context.data.word(25) = 168;
	context.data.byte(188) = 47;
	return;
slabewrong:
	context.data.word(21) = 40;
	context.data.word(25) = 160;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	return;
}

static inline void slabdoorf(Context & context) {
	showfirstuse(context);
	context.data.byte(103) = 1;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.word(23) = 171;
	context._cmp(context.data.byte(66), 2);
	if (!context.flags.z()) goto slabfwrong;
	context._add(context.data.byte(2), 1);
	context.data.word(21) = 60;
	context.data.word(25) = 197;
	context.data.byte(188) = 47;
	return;
slabfwrong:
	context.data.word(21) = 40;
	context.data.word(25) = 189;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	return;
}

static inline void useslab(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto slabwith;
	withwhat(context);
	return;
slabwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'J';
	context.ch = 'E';
	context.dl = 'W';
	context.dh = 'L';
	compare(context);
	if (context.flags.z()) goto nextslab;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
nextslab:
	context.al = context.data.byte(95);
	getexad(context);
	context.data.byte(context.bx+2) = 0;
	context.al = context.data.byte(99);
	context.push(context.ax);
	removesetobject(context);
	context.ax = context.pop();
	context._add(context.al, 1);
	context.push(context.ax);
	placesetobject(context);
	context.ax = context.pop();
	context._cmp(context.al, 54);
	if (!context.flags.z()) goto notlastslab;
	context.al = 0;
	turnpathon(context);
	context.data.word(21) = 22;
	context.data.word(23) = 35;
	context.data.word(25) = 48;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
notlastslab:
	context._add(context.data.byte(2), 1);
	showfirstuse(context);
	context.data.byte(103) = 1;
	return;
}

static inline void usecart(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto cartwith;
	withwhat(context);
	return;
cartwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'R';
	context.ch = 'O';
	context.dl = 'C';
	context.dh = 'K';
	compare(context);
	if (context.flags.z()) goto nextcart;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
nextcart:
	context.al = context.data.byte(95);
	getexad(context);
	context.data.byte(context.bx+2) = 0;
	context.al = context.data.byte(99);
	context.push(context.ax);
	removesetobject(context);
	context.ax = context.pop();
	context._add(context.al, 1);
	placesetobject(context);
	context._add(context.data.byte(2), 1);
	context.al = 17;
	playchannel1(context);
	showfirstuse(context);
	context.data.byte(103) = 1;
	return;
}

static inline void useclearbox(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto clearboxwith;
	withwhat(context);
	return;
clearboxwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'R';
	context.ch = 'A';
	context.dl = 'I';
	context.dh = 'L';
	compare(context);
	if (context.flags.z()) goto openbox;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
openbox:
	context._add(context.data.byte(2), 1);
	showfirstuse(context);
	context.data.word(21) = 80;
	context.data.word(23) = 67;
	context.data.word(25) = 105;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void usecoveredbox(Context & context) {
	context._add(context.data.byte(2), 1);
	showfirstuse(context);
	context.data.word(21) = 50;
	context.data.word(23) = 41;
	context.data.word(25) = 66;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void userailing(Context & context) {
	showfirstuse(context);
	context.data.word(21) = 80;
	context.data.word(23) = 0;
	context.data.word(25) = 30;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	context.data.byte(56) = 4;
	return;
}

static inline void useopenbox(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto openboxwith;
	withwhat(context);
	return;
openboxwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'F';
	compare(context);
	if (context.flags.z()) goto destoryopenbox;
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto openboxwrong;
	showfirstuse(context);
	return;
destoryopenbox:
	context._add(context.data.byte(2), 1);
	context.cx = 300;
	context.al = 37;
	showpuztext(context);
	context.al = context.data.byte(95);
	getexad(context);
	context.data.byte(context.bx+15) = 'E'-'A';
	context.data.word(21) = 140;
	context.data.word(23) = 105;
	context.data.word(25) = 181;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.al = 4;
	turnpathon(context);
	context.data.byte(103) = 1;
	return;
openboxwrong:
	context.cx = 300;
	context.al = 38;
	showpuztext(context);
	putbackobstuff(context);
	return;
}

static inline void wearwatch(Context & context) {
	context._cmp(context.data.byte(3), 1);
	if (context.flags.z()) goto wearingwatch;
	showfirstuse(context);
	context.data.byte(3) = 1;
	context.data.byte(103) = 1;
	getanyad(context);
	makeworn(context);
	return;
wearingwatch:
	showseconduse(context);
	putbackobstuff(context);
	return;
}

static inline void wearshades(Context & context) {
	context._cmp(context.data.byte(4), 1);
	if (context.flags.z()) goto wearingshades;
	context.data.byte(4) = 1;
	showfirstuse(context);
	context.data.byte(103) = 1;
	getanyad(context);
	makeworn(context);
	return;
wearingshades:
	showseconduse(context);
	putbackobstuff(context);
	return;
}

static inline void sitdowninbar(Context & context) {
	context._cmp(context.data.byte(32), -1);
	if (!context.flags.z()) goto satdown;
	showfirstuse(context);
	context.data.word(21) = 50;
	context.data.word(23) = 55;
	context.data.word(25) = 71;
	context.data.word(29) = 73;
	context.data.word(31) = 83;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	return;
satdown:
	showseconduse(context);
	putbackobstuff(context);
	return;
}

static inline void usechurchhole(Context & context) {
	showfirstuse(context);
	context.data.byte(103) = 1;
	context.data.word(21) = 28;
	context.data.word(23) = 13;
	context.data.word(25) = 26;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	return;
}

static inline void usehole(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto holewith;
	withwhat(context);
	return;
holewith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'H';
	context.ch = 'N';
	context.dl = 'D';
	context.dh = 'A';
	compare(context);
	if (context.flags.z()) goto righthand;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
righthand:
	showfirstuse(context);
	context.al = 86;
	removesetobject(context);
	context.al = context.data.byte(95);
	getexad(context);
	context.data.byte(context.bx+2) = 255;
	context.data.byte(47) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void usealtar(Context & context) {
	context.al = 'C';
	context.ah = 'N';
	context.cl = 'D';
	context.ch = 'A';
	findexobject(context);
	context._cmp(context.al, 114);
	if (context.flags.z()) goto thingsonaltar;
	context.al = 'C';
	context.ah = 'N';
	context.cl = 'D';
	context.ch = 'B';
	findexobject(context);
	context._cmp(context.al, 114);
	if (context.flags.z()) goto thingsonaltar;
	context._cmp(context.data.byte(47), 1);
	if (context.flags.z()) goto movealtar;
	context.cx = 300;
	context.al = 23;
	showpuztext(context);
	context.data.byte(103) = 1;
	return;
movealtar:
	context._add(context.data.byte(2), 1);
	showseconduse(context);
	context.data.word(21) = 160;
	context.data.word(23) = 81;
	context.data.word(25) = 174;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.al = 47;
	context.bl = 52;
	context.bh = 76;
	context.cx = 32;
	context.dx = 98;
	setuptimeduse(context);
	context.data.byte(103) = 1;
	return;
thingsonaltar:
	showfirstuse(context);
	context.data.byte(103) = 1;
	return;
}

static inline void opentvdoor(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto tvdoorwith;
	withwhat(context);
	return;
tvdoorwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'U';
	context.ch = 'L';
	context.dl = 'O';
	context.dh = 'K';
	compare(context);
	if (context.flags.z()) goto keyontv;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
keyontv:
	showfirstuse(context);
	context.data.byte(37) = 0;
	context.data.byte(103) = 1;
	return;
}

static inline void usedryer(Context & context) {
	context.al = 12;
	playchannel1(context);
	showfirstuse(context);
	context.data.byte(103) = 1;
	return;
}

static inline void openlouis(Context & context) {
	context.al = 5;
	context.ah = 2;
	context.cl = 3;
	context.ch = 8;
	entercode(context);
	context.data.byte(103) = 1;
	return;
}

static inline void nextcolon(Context & context) {
lookcolon:
	context.al = context.data.byte(context.si);
	context._add(context.si, 1);
	context._cmp(context.al, ':');
	if (!context.flags.z()) goto lookcolon;
	return;
}

static inline void openyourneighbour(Context & context) {
	context.al = 255;
	context.ah = 255;
	context.cl = 255;
	context.ch = 255;
	entercode(context);
	context.data.byte(103) = 1;
	return;
}

static inline void usewindow(Context & context) {
	context._cmp(context.data.byte(475), 6);
	if (!context.flags.z()) goto notonbalc;
	context._add(context.data.byte(2), 1);
	showfirstuse(context);
	context.data.byte(188) = 29;
	context.data.byte(103) = 1;
	return;
notonbalc:
	showseconduse(context);
	putbackobstuff(context);
	return;
}

static inline void usebalcony(Context & context) {
	showfirstuse(context);
	context.al = 6;
	turnpathon(context);
	context.al = 0;
	turnpathoff(context);
	context.al = 1;
	turnpathoff(context);
	context.al = 2;
	turnpathoff(context);
	context.al = 3;
	turnpathoff(context);
	context.al = 4;
	turnpathoff(context);
	context.al = 5;
	turnpathoff(context);
	context._add(context.data.byte(2), 1);
	context.data.byte(475) = 6;
	context.data.byte(478) = 6;
	context.data.byte(477) = 6;
	findxyfrompath(context);
	switchryanoff(context);
	context.data.byte(187) = 1;
	context.data.word(21) = 30*2;
	context.data.word(23) = 183;
	context.data.word(25) = 212;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void openryan(Context & context) {
	context.al = 5;
	context.ah = 1;
	context.cl = 0;
	context.ch = 6;
	entercode(context);
	context.data.byte(103) = 1;
	return;
}

static inline void openpoolboss(Context & context) {
	context.al = 5;
	context.ah = 2;
	context.cl = 2;
	context.ch = 2;
	entercode(context);
	context.data.byte(103) = 1;
	return;
}

static inline void openeden(Context & context) {
	context.al = 2;
	context.ah = 8;
	context.cl = 6;
	context.ch = 5;
	entercode(context);
	context.data.byte(103) = 1;
	return;
}

static inline void opensarters(Context & context) {
	context.al = 7;
	context.ah = 8;
	context.cl = 3;
	context.ch = 3;
	entercode(context);
	context.data.byte(103) = 1;
	return;
}

static inline void isitright(Context & context) {
	context.bx = context.ds;
	context.es = context.bx;
	context.bx = 8350;
	context._cmp(context.data.byte(context.bx+0), context.al);
	if (!context.flags.z()) goto notright;
	context._cmp(context.data.byte(context.bx+1), context.ah);
	if (!context.flags.z()) goto notright;
	context._cmp(context.data.byte(context.bx+2), context.cl);
	if (!context.flags.z()) goto notright;
	context._cmp(context.data.byte(context.bx+3), context.ch);
notright:
	return;
}

static inline void drawitall(Context & context) {
	createpanel(context);
	drawfloor(context);
	printsprites(context);
	showicon(context);
	return;
}

static inline void openhoteldoor(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto hoteldoorwith;
	withwhat(context);
	return;
hoteldoorwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'K';
	context.ch = 'E';
	context.dl = 'Y';
	context.dh = 'A';
	compare(context);
	if (context.flags.z()) goto keyonhotel1;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
keyonhotel1:
	context.al = 16;
	playchannel1(context);
	showfirstuse(context);
	context.data.byte(37) = 0;
	context.data.byte(103) = 1;
	return;
}

static inline void openhoteldoor2(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto hoteldoorwith2;
	withwhat(context);
	return;
hoteldoorwith2:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'K';
	context.ch = 'E';
	context.dl = 'Y';
	context.dh = 'A';
	compare(context);
	if (context.flags.z()) goto keyonhotel2;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
keyonhotel2:
	context.al = 16;
	playchannel1(context);
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void grafittidoor(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto grafwith;
	withwhat(context);
	return;
grafwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'A';
	context.ch = 'P';
	context.dl = 'E';
	context.dh = 'N';
	compare(context);
	if (context.flags.z()) goto dograf;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
dograf:
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void trapdoor(Context & context) {
	context._add(context.data.byte(2), 1);
	showfirstuse(context);
	switchryanoff(context);
	context.data.word(21) = 20*2;
	context.data.word(23) = 181;
	context.data.word(25) = 197;
	context.data.byte(188) = 26;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void callhotellift(Context & context) {
	context.al = 12;
	playchannel1(context);
	showfirstuse(context);
	context.data.byte(39) = 8;
	context.data.byte(103) = 1;
	context.data.byte(478) = 5;
	context.data.byte(477) = 5;
	autosetwalk(context);
	context.al = 4;
	turnpathon(context);
	return;
}

static inline void calledenslift(Context & context) {
	showfirstuse(context);
	context.data.byte(39) = 8;
	context.data.byte(103) = 1;
	context.al = 2;
	turnpathon(context);
	return;
}

static inline void calledensdlift(Context & context) {
	context._cmp(context.data.byte(35), 1);
	if (context.flags.z()) goto edensdhere;
	showfirstuse(context);
	context.data.byte(39) = 8;
	context.data.byte(103) = 1;
	context.al = 2;
	turnpathon(context);
	return;
edensdhere:
	showseconduse(context);
	putbackobstuff(context);
	return;
}

static inline void usepoolreader(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto poolwith;
	withwhat(context);
	return;
poolwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'M';
	context.ch = 'E';
	context.dl = 'M';
	context.dh = 'B';
	compare(context);
	if (context.flags.z()) goto openpool;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
openpool:
	context._cmp(context.data.byte(48), 1);
	if (context.flags.z()) goto canopenpool;
	showseconduse(context);
	putbackobstuff(context);
	return;
canopenpool:
	context.al = 17;
	playchannel1(context);
	showfirstuse(context);
	context.data.byte(39) = 6;
	context.data.byte(103) = 1;
	return;
}

static inline void uselighter(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto gotlighterwith;
	withwhat(context);
	return;
gotlighterwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'S';
	context.ch = 'M';
	context.dl = 'K';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto cigarette;
	showfirstuse(context);
	putbackobstuff(context);
	return;
cigarette:
	context.cx = 300;
	context.al = 9;
	showpuztext(context);
	context.al = context.data.byte(95);
	getexad(context);
	context.data.byte(context.bx+2) = 255;
	context.data.byte(103) = 1;
	return;
}

static inline void showseconduse(Context & context) {
	getobtextstart(context);
	nextcolon(context);
	nextcolon(context);
	nextcolon(context);
	usetext(context);
	context.cx = 400;
	hangonp(context);
	return;
}

static inline void usecardreader1(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto gotreader1with;
	withwhat(context);
	return;
gotreader1with:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'C';
	context.ch = 'S';
	context.dl = 'H';
	context.dh = 'R';
	compare(context);
	if (context.flags.z()) goto correctcard;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
correctcard:
	context._cmp(context.data.byte(49), 0);
	if (context.flags.z()) goto notyet;
	context._cmp(context.data.word(16), 0);
	if (context.flags.z()) goto getscash;
	context.cx = 300;
	context.al = 17;
	showpuztext(context);
	putbackobstuff(context);
	return;
getscash:
	context.al = 16;
	playchannel1(context);
	context.cx = 300;
	context.al = 18;
	showpuztext(context);
	context._add(context.data.byte(2), 1);
	context.data.word(16) = 12432;
	context.data.byte(103) = 1;
	return;
notyet:
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void usecardreader2(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto gotreader2with;
	withwhat(context);
	return;
gotreader2with:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'C';
	context.ch = 'S';
	context.dl = 'H';
	context.dh = 'R';
	compare(context);
	if (context.flags.z()) goto correctcard2;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
correctcard2:
	context._cmp(context.data.byte(50), 0);
	if (context.flags.z()) goto notyetboss;
	context._cmp(context.data.word(16), 0);
	if (context.flags.z()) goto nocash;
	context._cmp(context.data.byte(46), 2);
	if (context.flags.z()) goto alreadygotnew;
	context.al = 18;
	playchannel1(context);
	context.cx = 300;
	context.al = 19;
	showpuztext(context);
	context.al = 94;
	placesetobject(context);
	context.data.byte(46) = 1;
	context._sub(context.data.word(16), 2000);
	context._add(context.data.byte(2), 1);
	context.data.byte(103) = 1;
	return;
nocash:
	context.cx = 300;
	context.al = 20;
	showpuztext(context);
	putbackobstuff(context);
	return;
alreadygotnew:
	context.cx = 300;
	context.al = 22;
	showpuztext(context);
	putbackobstuff(context);
	return;
notyetboss:
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void usecardreader3(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto gotreader3with;
	withwhat(context);
	return;
gotreader3with:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'C';
	context.ch = 'S';
	context.dl = 'H';
	context.dh = 'R';
	compare(context);
	if (context.flags.z()) goto rightcard;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
rightcard:
	context._cmp(context.data.byte(51), 0);
	if (context.flags.z()) goto notyetrecep;
	context._cmp(context.data.byte(52), 0);
	if (!context.flags.z()) goto alreadyusedit;
	context.al = 16;
	playchannel1(context);
	context.cx = 300;
	context.al = 25;
	showpuztext(context);
	context._add(context.data.byte(2), 1);
	context._sub(context.data.word(16), 8300);
	context.data.byte(52) = 1;
	context.data.byte(103) = 1;
	return;
alreadyusedit:
	context.cx = 300;
	context.al = 26;
	showpuztext(context);
	putbackobstuff(context);
	return;
notyetrecep:
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void usecashcard(Context & context) {
	getridofreels(context);
	loadkeypad(context);
	createpanel(context);
	showpanel(context);
	showexit(context);
	showman(context);
	context.di = 114;
	context.bx = 120-3;
	context.ds = context.data.word(458);
	context.al = 39;
	context.ah = 0;
	showframe(context);
	context.ax = context.data.word(16);
	moneypoke(context);
	getobtextstart(context);
	nextcolon(context);
	nextcolon(context);
	context.di = 36;
	context.bx = 98;
	context.dl = 241;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.di = 160;
	context.bx = 155;
	context.es = context.cs;
	context.si = 3385;
	context.data.word(71) = 91*2+75;
	context.al = 0;
	context.ah = 0;
	context.dl = 240;
	printdirect(context);
	context.di = 187;
	context.bx = 155;
	context.es = context.cs;
	context.si = 3390;
	context.data.word(71) = 91*2+85;
	context.al = 0;
	context.ah = 0;
	context.dl = 240;
	printdirect(context);
	context.data.word(71) = 0;
	worktoscreenm(context);
	context.cx = 400;
	hangonp(context);
	getridoftemp(context);
	restorereels(context);
	putbackobstuff(context);
	return;
}

static inline void moneypoke(Context & context) {
	context.bx = 3385;
	context.cl = 48-1;
numberpoke0:
	context._add(context.cl, 1);
	context._sub(context.ax, 10000);
	if (!context.flags.c()) goto numberpoke0;
	context._add(context.ax, 10000);
	context.data.byte(context.bx) = context.cl;
	context._add(context.bx, 1);
	context.cl = 48-1;
numberpoke1:
	context._add(context.cl, 1);
	context._sub(context.ax, 1000);
	if (!context.flags.c()) goto numberpoke1;
	context._add(context.ax, 1000);
	context.data.byte(context.bx) = context.cl;
	context._add(context.bx, 1);
	context.cl = 48-1;
numberpoke2:
	context._add(context.cl, 1);
	context._sub(context.ax, 100);
	if (!context.flags.c()) goto numberpoke2;
	context._add(context.ax, 100);
	context.data.byte(context.bx) = context.cl;
	context._add(context.bx, 1);
	context.cl = 48-1;
numberpoke3:
	context._add(context.cl, 1);
	context._sub(context.ax, 10);
	if (!context.flags.c()) goto numberpoke3;
	context._add(context.ax, 10);
	context.data.byte(context.bx) = context.cl;
	context.bx = 3390;
	context._add(context.al, 48);
	context.data.byte(context.bx) = context.al;
	return;
}

static inline void usecontrol(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto gotcontrolwith;
	withwhat(context);
	return;
gotcontrolwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'K';
	context.ch = 'E';
	context.dl = 'Y';
	context.dh = 'A';
	compare(context);
	if (context.flags.z()) goto rightkey;
	context._cmp(context.data.byte(184), 21);
	if (!context.flags.z()) goto balls;
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'K';
	context.ch = 'N';
	context.dl = 'F';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto jimmycontrols;
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'A';
	context.ch = 'X';
	context.dl = 'E';
	context.dh = 'D';
	compare(context);
	if (context.flags.z()) goto axeoncontrols;
balls:
	showfirstuse(context);
	putbackobstuff(context);
	return;
rightkey:
	context.al = 16;
	playchannel1(context);
	context._cmp(context.data.byte(9), 21);
	if (context.flags.z()) goto goingdown;
	context.cx = 300;
	context.al = 0;
	showpuztext(context);
	context.data.byte(188) = 21;
	context.data.byte(40) = 8;
	context.data.byte(39) = 0;
	context.data.word(21) = 80;
	context.data.byte(103) = 1;
	return;
goingdown:
	context.cx = 300;
	context.al = 3;
	showpuztext(context);
	context.data.byte(188) = 30;
	context.data.byte(40) = 8;
	context.data.byte(39) = 0;
	context.data.word(21) = 80;
	context.data.byte(103) = 1;
	return;
jimmycontrols:
	context.al = 50;
	placesetobject(context);
	context.al = 51;
	placesetobject(context);
	context.al = 26;
	placesetobject(context);
	context.al = 30;
	placesetobject(context);
	context.al = 16;
	removesetobject(context);
	context.al = 17;
	removesetobject(context);
	context.al = 14;
	playchannel1(context);
	context.cx = 300;
	context.al = 10;
	showpuztext(context);
	context._add(context.data.byte(2), 1);
	context.data.byte(103) = 1;
	return;
axeoncontrols:
	context.cx = 300;
	context.al = 16;
	showpuztext(context);
	context._add(context.data.byte(2), 1);
	putbackobstuff(context);
	return;
}

static inline void usehatch(Context & context) {
	showfirstuse(context);
	context.data.byte(188) = 40;
	context.data.byte(103) = 1;
	return;
}

static inline void usewire(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto gotwirewith;
	withwhat(context);
	return;
gotwirewith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'K';
	context.ch = 'N';
	context.dl = 'F';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto wireknife;
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'A';
	context.ch = 'X';
	context.dl = 'E';
	context.dh = 'D';
	compare(context);
	if (context.flags.z()) goto wireaxe;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
wireaxe:
	context.cx = 300;
	context.al = 16;
	showpuztext(context);
	putbackobstuff(context);
	return;
wireknife:
	context.al = 51;
	removesetobject(context);
	context.al = 52;
	placesetobject(context);
	context.cx = 300;
	context.al = 11;
	showpuztext(context);
	context._add(context.data.byte(2), 1);
	context.data.byte(103) = 1;
	return;
}

static inline void usehandle(Context & context) {
	context.al = 'C';
	context.ah = 'U';
	context.cl = 'T';
	context.ch = 'W';
	findsetobject(context);
	context.al = context.data.byte(context.bx+58);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto havecutwire;
	context.cx = 300;
	context.al = 12;
	showpuztext(context);
	context.data.byte(103) = 1;
	return;
havecutwire:
	context.cx = 300;
	context.al = 13;
	showpuztext(context);
	context.data.byte(188) = 22;
	context.data.byte(103) = 1;
	return;
}

static inline void useelevator1(Context & context) {
	showfirstuse(context);
	selectlocation(context);
	context.data.byte(103) = 1;
	return;
}

static inline void showfirstuse(Context & context) {
	getobtextstart(context);
	findnextcolon(context);
	findnextcolon(context);
	usetext(context);
	context.cx = 400;
	hangonp(context);
	return;
}

static inline void useelevator3(Context & context) {
	showfirstuse(context);
	context.data.byte(40) = 20;
	context.data.byte(188) = 34;
	context.data.word(23) = 46;
	context.data.word(25) = 63;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.word(21) = 80;
	context.data.byte(103) = 1;
	return;
}

static inline void useelevator4(Context & context) {
	showfirstuse(context);
	context.data.word(23) = 0;
	context.data.word(25) = 11;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(40) = 20;
	context.data.word(21) = 80;
	context.data.byte(103) = 1;
	context.data.byte(188) = 24;
	return;
}

static inline void useelevator2(Context & context) {
	context._cmp(context.data.byte(9), 23);
	if (context.flags.z()) goto inpoolhall;
	showfirstuse(context);
	context.data.byte(188) = 23;
	context.data.byte(40) = 20;
	context.data.byte(39) = 0;
	context.data.word(21) = 80;
	context.data.byte(103) = 1;
	return;
inpoolhall:
	showfirstuse(context);
	context.data.byte(188) = 31;
	context.data.byte(40) = 20;
	context.data.byte(39) = 0;
	context.data.word(21) = 80;
	context.data.byte(103) = 1;
	return;
}

static inline void useelevator5(Context & context) {
	context.al = 4;
	placesetobject(context);
	context.al = 0;
	removesetobject(context);
	context.data.byte(188) = 20;
	context.data.word(21) = 80;
	context.data.byte(35) = 1;
	context.data.byte(40) = 8;
	context.data.byte(103) = 1;
	return;
}

static inline void usekey(Context & context) {
	context._cmp(context.data.byte(9), 5);
	if (context.flags.z()) goto usekey1;
	context._cmp(context.data.byte(9), 30);
	if (context.flags.z()) goto usekey1;
	context._cmp(context.data.byte(9), 21);
	if (context.flags.z()) goto usekey2;
	context.cx = 200;
	context.al = 1;
	showpuztext(context);
	putbackobstuff(context);
	return;
usekey1:
	context._cmp(context.data.byte(148), 22);
	if (!context.flags.z()) goto wrongroom1;
	context._cmp(context.data.byte(149), 10);
	if (!context.flags.z()) goto wrongroom1;
	context.cx = 300;
	context.al = 0;
	showpuztext(context);
	context.data.byte(40) = 100;
	context.data.byte(103) = 1;
	return;
usekey2:
	context._cmp(context.data.byte(148), 11);
	if (!context.flags.z()) goto wrongroom1;
	context._cmp(context.data.byte(149), 10);
	if (!context.flags.z()) goto wrongroom1;
	context.cx = 300;
	context.al = 3;
	showpuztext(context);
	context.data.byte(188) = 30;
	context.al = 2;
	fadescreendown(context);
	showfirstuse(context);
	putbackobstuff(context);
	return;
wrongroom1:
	context.cx = 200;
	context.al = 2;
	showpuztext(context);
	putbackobstuff(context);
	return;
}

static inline void usestereo(Context & context) {
	context._cmp(context.data.byte(9), 0);
	if (context.flags.z()) goto stereook;
	context.cx = 400;
	context.al = 4;
	showpuztext(context);
	putbackobstuff(context);
	return;
stereook:
	context._cmp(context.data.byte(148), 11);
	if (!context.flags.z()) goto stereonotok;
	context._cmp(context.data.byte(149), 0);
	if (context.flags.z()) goto stereook2;
stereonotok:
	context.cx = 400;
	context.al = 5;
	showpuztext(context);
	putbackobstuff(context);
	return;
stereook2:
	context.al = 'C';
	context.ah = 'D';
	context.cl = 'P';
	context.ch = 'L';
	findsetobject(context);
	context.ah = 1;
	checkinside(context);
	context._cmp(context.cl, 114);
	if (!context.flags.z()) goto cdinside;
	context.al = 6;
	context.cx = 400;
	showpuztext(context);
	putbackobstuff(context);
	getanyad(context);
	context.al = 255;
	context.data.byte(context.bx+10) = context.al;
	return;
cdinside:
	getanyad(context);
	context.al = context.data.byte(context.bx+10);
	context._xor(context.al, 1);
	context.data.byte(context.bx+10) = context.al;
	context._cmp(context.al, 255);
	if (context.flags.z()) goto stereoon;
	context.al = 7;
	context.cx = 400;
	showpuztext(context);
	putbackobstuff(context);
	return;
stereoon:
	context.al = 8;
	context.cx = 400;
	showpuztext(context);
	putbackobstuff(context);
	return;
}

static inline void usecooker(Context & context) {
	context.al = context.data.byte(99);
	context.ah = context.data.byte(102);
	checkinside(context);
	context._cmp(context.cl, 114);
	if (!context.flags.z()) goto foodinside;
	showfirstuse(context);
	putbackobstuff(context);
	return;
foodinside:
	showseconduse(context);
	putbackobstuff(context);
	return;
}

static inline void useaxe(Context & context) {
	context._cmp(context.data.byte(184), 22);
	if (!context.flags.z()) goto notinpool;
	context._cmp(context.data.byte(149), 10);
	if (context.flags.z()) goto axeondoor;
	showseconduse(context);
	context._add(context.data.byte(2), 1);
	context.data.byte(65) = 2;
	context.data.byte(103) = 1;
	removeobfrominv(context);
	return;
notinpool:
	showfirstuse(context);
	return;
/*continuing to unbounded code: axeondoor from useelvdoor:19-30*/
axeondoor:
	context.al = 15;
	context.cx = 300;
	showpuztext(context);
	context._add(context.data.byte(2), 1);
	context.data.word(21) = 46*2;
	context.data.word(23) = 31;
	context.data.word(25) = 77;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	return;
	return;
}

static inline void useelvdoor(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto gotdoorwith;
	withwhat(context);
	return;
gotdoorwith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'A';
	context.ch = 'X';
	context.dl = 'E';
	context.dh = 'D';
	compare(context);
	if (context.flags.z()) goto axeondoor;
	context.al = 14;
	context.cx = 300;
	showpuztext(context);
	putbackobstuff(context);
	return;
axeondoor:
	context.al = 15;
	context.cx = 300;
	showpuztext(context);
	context._add(context.data.byte(2), 1);
	context.data.word(21) = 46*2;
	context.data.word(23) = 31;
	context.data.word(25) = 77;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void withwhat(Context & context) {
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	context.al = context.data.byte(99);
	context.ah = context.data.byte(102);
	context.es = context.cs;
	context.di = 5674;
	copyname(context);
	context.di = 100;
	context.bx = 21;
	context.dl = 200;
	context.al = 63;
	context.ah = 2;
	printmessage2(context);
	context.di = context.data.word(84);
	context._add(context.di, 5);
	context.bx = 21;
	context.es = context.cs;
	context.si = 5674;
	context.dl = 220;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.di = context.data.word(84);
	context._add(context.di, 5);
	context.bx = 21;
	context.dl = 200;
	context.al = 63;
	context.ah = 3;
	printmessage2(context);
	fillryan(context);
	context.data.byte(100) = 255;
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	context.data.byte(104) = 2;
	return;
}

static inline void selectob(Context & context) {
	findinvpos(context);
	context.ax = context.data.word(context.bx);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto canselectob;
	blank(context);
	return;
canselectob:
	context.data.byte(95) = context.al;
	context.data.byte(96) = context.ah;
	context._cmp(context.ax, context.data.word(94));
	if (!context.flags.z()) goto diffsub3;
	context._cmp(context.data.byte(100), 221);
	if (context.flags.z()) goto alreadyselob;
	context.data.byte(100) = 221;
diffsub3:
	context.data.word(94) = context.ax;
	context.bx = context.ax;
	context.al = 0;
	commandwithob(context);
alreadyselob:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notselob;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doselob;
notselob:
	return;
doselob:
	delpointer(context);
	context.data.byte(104) = 0;
	useroutine(context);
	return;
}

static inline void findsetobject(Context & context) {
	context._sub(context.al, 'A');
	context._sub(context.ah, 'A');
	context._sub(context.cl, 'A');
	context._sub(context.ch, 'A');
	context.es = context.data.word(428);
	context.bx = 0;
	context.dl = 0;
findsetloop:
	context._cmp(context.al, context.data.byte(context.bx+12));
	if (!context.flags.z()) goto nofind;
	context._cmp(context.ah, context.data.byte(context.bx+13));
	if (!context.flags.z()) goto nofind;
	context._cmp(context.cl, context.data.byte(context.bx+14));
	if (!context.flags.z()) goto nofind;
	context._cmp(context.ch, context.data.byte(context.bx+15));
	if (!context.flags.z()) goto nofind;
	context.al = context.dl;
	return;
nofind:
	context._add(context.bx, 64);
	context._add(context.dl, 1);
	context._cmp(context.dl, 128);
	if (!context.flags.z()) goto findsetloop;
	context.al = context.dl;
	return;
}

static inline void checkinside(Context & context) {
	context.es = context.data.word(398);
	context.bx = 0+2080+30000;
	context.cl = 0;
insideloop:
	context._cmp(context.al, context.data.byte(context.bx+3));
	if (!context.flags.z()) goto notfoundinside;
	context._cmp(context.ah, context.data.byte(context.bx+2));
	if (!context.flags.z()) goto notfoundinside;
	return;
notfoundinside:
	context._add(context.bx, 16);
	context._add(context.cl, 1);
	context._cmp(context.cl, 114);
	if (!context.flags.z()) goto insideloop;
	return;
}

static inline void usetext(Context & context) {
	context.push(context.es);
	context.push(context.si);
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	obicons(context);
	context.si = context.pop();
	context.es = context.pop();
	context.di = 36;
	context.bx = 104;
	context.dl = 241;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	worktoscreenm(context);
	return;
}

static inline void showpuztext(Context & context) {
	context.push(context.cx);
	findpuztext(context);
	context.push(context.es);
	context.push(context.si);
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	obicons(context);
	context.si = context.pop();
	context.es = context.pop();
	context.di = 36;
	context.bx = 104;
	context.dl = 241;
	context.ah = 0;
	printdirect(context);
	worktoscreenm(context);
	context.cx = context.pop();
	hangonp(context);
	return;
}

static inline void issetobonmap(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	getsetad(context);
	context.al = context.data.byte(context.bx+58);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.al, 0);
	return;
}

static inline void setuptimedtemp(Context & context) {
	context._cmp(context.data.word(328), 0);
	if (!context.flags.z()) goto cantsetup2;
	context.data.byte(335) = context.bh;
	context.data.byte(336) = context.bl;
	context.data.word(330) = context.cx;
	context._add(context.dx, context.cx);
	context.data.word(328) = context.dx;
	context.bl = context.al;
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(466);
	context.cx = 66*2;
	context.ax = context.data.word(context.bx);
	context._add(context.ax, context.cx);
	context.bx = context.ax;
	context.data.word(332) = context.es;
	context.data.word(334) = context.bx;
cantsetup2:
	return;
}

static inline void edenscdplayer(Context & context) {
	showfirstuse(context);
	context.data.word(21) = 18*2;
	context.data.word(23) = 25;
	context.data.word(25) = 42;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	return;
}

static inline void usewall(Context & context) {
	showfirstuse(context);
	context._cmp(context.data.byte(475), 3);
	if (context.flags.z()) goto gobackover;
	context.data.word(21) = 30*2;
	context.data.word(23) = 2;
	context.data.word(25) = 31;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	context.al = 3;
	turnpathon(context);
	context.al = 4;
	turnpathon(context);
	context.al = 0;
	turnpathoff(context);
	context.al = 1;
	turnpathoff(context);
	context.al = 2;
	turnpathoff(context);
	context.al = 5;
	turnpathoff(context);
	context.data.byte(475) = 3;
	context.data.byte(477) = 3;
	findxyfrompath(context);
	context.data.byte(187) = 1;
	switchryanoff(context);
	return;
gobackover:
	context.data.word(21) = 30*2;
	context.data.word(23) = 34;
	context.data.word(25) = 60;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	context.al = 3;
	turnpathoff(context);
	context.al = 4;
	turnpathoff(context);
	context.al = 0;
	turnpathon(context);
	context.al = 1;
	turnpathon(context);
	context.al = 2;
	turnpathon(context);
	context.al = 5;
	turnpathon(context);
	context.data.byte(475) = 5;
	context.data.byte(477) = 5;
	findxyfrompath(context);
	context.data.byte(187) = 1;
	switchryanoff(context);
	return;
}

static inline void usechurchgate(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto gatewith;
	withwhat(context);
	return;
gatewith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'T';
	context.dh = 'T';
	compare(context);
	if (context.flags.z()) goto cutgate;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
cutgate:
	showfirstuse(context);
	context.data.word(21) = 64*2;
	context.data.word(23) = 4;
	context.data.word(25) = 70;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	context._add(context.data.byte(2), 1);
	context.al = 3;
	turnpathon(context);
	context._cmp(context.data.byte(44), 0);
	if (context.flags.z()) goto notopenchurch;
	context.al = 2;
	turnpathon(context);
notopenchurch:
	return;
}

static inline void usegun(Context & context) {
	context._cmp(context.data.byte(102), 4);
	if (context.flags.z()) goto istakengun;
	showseconduse(context);
	putbackobstuff(context);
	return;
istakengun:
	context._cmp(context.data.byte(184), 22);
	if (!context.flags.z()) goto notinpoolroom;
	context.cx = 300;
	context.al = 34;
	showpuztext(context);
	context.data.byte(65) = 1;
	context.data.byte(64) = 39;
	context.data.byte(103) = 1;
	context._add(context.data.byte(2), 1);
	return;
notinpoolroom:
	context._cmp(context.data.byte(184), 25);
	if (!context.flags.z()) goto nothelicopter;
	context.cx = 300;
	context.al = 34;
	showpuztext(context);
	context.data.byte(65) = 1;
	context.data.byte(64) = 19;
	context.data.byte(103) = 1;
	context.data.byte(66) = 2;
	context.data.byte(67) = 38;
	context.data.byte(43) = 1;
	context._add(context.data.byte(2), 1);
	return;
nothelicopter:
	context._cmp(context.data.byte(184), 27);
	if (!context.flags.z()) goto notinrockroom;
	context.cx = 300;
	context.al = 46;
	showpuztext(context);
	context.data.byte(234) = 2;
	context.data.byte(41) = 1;
	context.data.byte(65) = 1;
	context.data.byte(34) = 1;
	context.data.byte(103) = 1;
	context.data.byte(67) = 32;
	context.data.byte(66) = 0;
	context._add(context.data.byte(2), 1);
	return;
notinrockroom:
	context._cmp(context.data.byte(184), 8);
	if (!context.flags.z()) goto notbystudio;
	context._cmp(context.data.byte(148), 22);
	if (!context.flags.z()) goto notbystudio;
	context._cmp(context.data.byte(149), 40);
	if (!context.flags.z()) goto notbystudio;
	context.al = 92;
	issetobonmap(context);
	if (context.flags.z()) goto notbystudio;
	context._cmp(context.data.byte(475), 9);
	if (context.flags.z()) goto notbystudio;
	context.data.byte(478) = 9;
	context.data.byte(477) = 9;
	autosetwalk(context);
	context.data.byte(65) = 1;
	context.data.byte(103) = 1;
	context._add(context.data.byte(2), 1);
	return;
notbystudio:
	context._cmp(context.data.byte(184), 6);
	if (!context.flags.z()) goto notsarters;
	context._cmp(context.data.byte(148), 11);
	if (!context.flags.z()) goto notsarters;
	context._cmp(context.data.byte(149), 20);
	if (!context.flags.z()) goto notsarters;
	context.al = 5;
	issetobonmap(context);
	if (!context.flags.z()) goto notsarters;
	context.data.byte(478) = 1;
	context.data.byte(477) = 1;
	autosetwalk(context);
	context.al = 5;
	removesetobject(context);
	context.al = 6;
	placesetobject(context);
	context.al = 1;
	context.ah = context.data.byte(185);
	context._sub(context.ah, 1);
	turnanypathon(context);
	context.data.byte(35) = 1;
	context.data.word(21) = 40*2;
	context.data.word(23) = 4;
	context.data.word(25) = 43;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	context._add(context.data.byte(2), 1);
	return;
notsarters:
	context._cmp(context.data.byte(184), 29);
	if (!context.flags.z()) goto notaide;
	context.data.byte(103) = 1;
	context.al = 13;
	resetlocation(context);
	context.al = 12;
	setlocation(context);
	context.data.byte(183) = 12;
	context.data.byte(478) = 2;
	context.data.byte(477) = 2;
	autosetwalk(context);
	context.data.word(21) = 164*2;
	context.data.word(23) = 3;
	context.data.word(25) = 164;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(44) = 1;
	context.data.byte(66) = 3;
	context.data.byte(67) = 33;
	context._add(context.data.byte(2), 1);
	return;
notaide:
	context._cmp(context.data.byte(184), 23);
	if (!context.flags.z()) goto notwithboss;
	context._cmp(context.data.byte(148), 0);
	if (!context.flags.z()) goto notwithboss;
	context._cmp(context.data.byte(149), 50);
	if (!context.flags.z()) goto notwithboss;
	context._cmp(context.data.byte(475), 5);
	if (context.flags.z()) goto pathokboss;
	context.data.byte(478) = 5;
	context.data.byte(477) = 5;
	autosetwalk(context);
pathokboss:
	context.data.byte(65) = 1;
	context.data.byte(103) = 1;
	return;
notwithboss:
	context._cmp(context.data.byte(184), 8);
	if (!context.flags.z()) goto nottvsoldier;
	context._cmp(context.data.byte(148), 11);
	if (!context.flags.z()) goto nottvsoldier;
	context._cmp(context.data.byte(149), 10);
	if (!context.flags.z()) goto nottvsoldier;
	context._cmp(context.data.byte(475), 2);
	if (context.flags.z()) goto pathoktv;
	context.data.byte(478) = 2;
	context.data.byte(477) = 2;
	autosetwalk(context);
pathoktv:
	context.data.byte(65) = 1;
	context.data.byte(103) = 1;
	return;
nottvsoldier:
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void useshield(Context & context) {
	context._cmp(context.data.byte(184), 20);
	if (!context.flags.z()) goto notinsartroom;
	context._cmp(context.data.byte(64), 0);
	if (context.flags.z()) goto notinsartroom;
	context.data.byte(65) = 3;
	showseconduse(context);
	context.data.byte(103) = 1;
	context._add(context.data.byte(2), 1);
	removeobfrominv(context);
	return;
notinsartroom:
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void usebuttona(Context & context) {
	context.al = 95;
	issetobonmap(context);
	if (context.flags.z()) goto donethisbit;
	showfirstuse(context);
	context.al = 0;
	context.ah = context.data.byte(185);
	context._sub(context.ah, 1);
	turnanypathon(context);
	context.al = 9;
	removesetobject(context);
	context.al = 95;
	placesetobject(context);
	context.data.word(21) = 15*2;
	context.data.word(23) = 71;
	context.data.word(25) = 85;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(103) = 1;
	context._add(context.data.byte(2), 1);
	return;
donethisbit:
	showseconduse(context);
	putbackobstuff(context);
	return;
}

static inline void useplate(Context & context) {
	context._cmp(context.data.byte(95), 255);
	if (!context.flags.z()) goto platewith;
	withwhat(context);
	return;
platewith:
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'S';
	context.ch = 'C';
	context.dl = 'R';
	context.dh = 'W';
	compare(context);
	if (context.flags.z()) goto unscrewplate;
	context.al = context.data.byte(95);
	context.ah = context.data.byte(96);
	context.cl = 'K';
	context.ch = 'N';
	context.dl = 'F';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto triedknife;
	context.cx = 300;
	context.al = 14;
	showpuztext(context);
	putbackobstuff(context);
	return;
unscrewplate:
	context.al = 20;
	playchannel1(context);
	showfirstuse(context);
	context.al = 28;
	placesetobject(context);
	context.al = 24;
	placesetobject(context);
	context.al = 25;
	removesetobject(context);
	context.al = 0;
	placefreeobject(context);
	context._add(context.data.byte(2), 1);
	context.data.byte(103) = 1;
	return;
triedknife:
	context.cx = 300;
	context.al = 54;
	showpuztext(context);
	putbackobstuff(context);
	return;
}

static inline void usewinch(Context & context) {
	context.al = 40;
	context.ah = 1;
	checkinside(context);
	context._cmp(context.cl, 114);
	if (context.flags.z()) goto nowinch;
	context.al = context.cl;
	context.ah = 4;
	context.cl = 'F';
	context.ch = 'U';
	context.dl = 'S';
	context.dh = 'E';
	compare(context);
	if (!context.flags.z()) goto nowinch;
	context.data.word(21) = 217*2;
	context.data.word(23) = 0;
	context.data.word(25) = 217;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	context.data.byte(183) = 1;
	context.data.byte(188) = 45;
	context.data.byte(66) = 1;
	context.data.byte(67) = 44;
	context.data.byte(42) = 1;
	context.data.byte(34) = 2;
	context.data.byte(103) = 1;
	context._add(context.data.byte(2), 1);
	return;
nowinch:
	showfirstuse(context);
	putbackobstuff(context);
	return;
}

static inline void entercode(Context & context) {
	context.data.word(284) = context.ax;
	context.data.word(286) = context.cx;
	getridofreels(context);
	loadkeypad(context);
	createpanel(context);
	showicon(context);
	showouterpad(context);
	showkeypad(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	context.data.word(280) = 0;
	context.data.byte(103) = 0;
keypadloop:
	delpointer(context);
	readmouse(context);
	showkeypad(context);
	showpointer(context);
	context._cmp(context.data.byte(282), 0);
	if (context.flags.z()) goto nopresses;
	context._sub(context.data.byte(282), 1);
	goto afterpress;
nopresses:
	context.data.byte(278) = 255;
	context.data.byte(281) = 255;
	vsync(context);
afterpress:
	dumppointer(context);
	dumpkeypad(context);
	dumptextline(context);
	context.bx = 3393;
	checkcoords(context);
	context._cmp(context.data.byte(103), 1);
	if (context.flags.z()) goto numberright;
	context._cmp(context.data.byte(287), 1);
	if (!context.flags.z()) goto notendkey;
	context._cmp(context.data.byte(37), 0);
	if (context.flags.z()) goto numberright;
	goto keypadloop;
notendkey:
	context._cmp(context.data.byte(282), 40);
	if (!context.flags.z()) goto keypadloop;
	addtopresslist(context);
	context._cmp(context.data.byte(278), 11);
	if (!context.flags.z()) goto keypadloop;
	context.ax = context.data.word(284);
	context.cx = context.data.word(286);
	isitright(context);
	if (!context.flags.z()) goto incorrect;
	context.data.byte(37) = 0;
	context.al = 11;
	playchannel1(context);
	context.data.byte(287) = 120;
	context.data.word(280) = 0;
	goto keypadloop;
incorrect:
	context.al = 11;
	playchannel1(context);
	context.data.byte(287) = 120;
	context.data.word(280) = 0;
	goto keypadloop;
numberright:
	context.data.byte(131) = 0;
	getridoftemp(context);
	restorereels(context);
	redrawmainscrn(context);
	worktoscreenm(context);
	return;
}

static inline void quitkey(Context & context) {
	context._cmp(context.data.byte(100), 222);
	if (context.flags.z()) goto alreadyqk;
	context.data.byte(100) = 222;
	context.al = 4;
	commandonly(context);
alreadyqk:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notqk;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doqk;
notqk:
	return;
doqk:
	context.data.byte(103) = 1;
	return;
}

static inline void addtopresslist(Context & context) {
	context._cmp(context.data.word(280), 5);
	if (context.flags.z()) goto nomorekeys;
	context.al = context.data.byte(278);
	context._cmp(context.al, 10);
	if (!context.flags.z()) goto not10;
	context.al = 0;
not10:
	context.bx = context.data.word(280);
	context.dx = context.ds;
	context.es = context.dx;
	context._add(context.bx, 8350);
	context.data.byte(context.bx) = context.al;
	context._add(context.data.word(280), 1);
nomorekeys:
	return;
}

static inline void buttonone(Context & context) {
	context.cl = 1;
	buttonpress(context);
	return;
}

static inline void buttontwo(Context & context) {
	context.cl = 2;
	buttonpress(context);
	return;
}

static inline void buttonthree(Context & context) {
	context.cl = 3;
	buttonpress(context);
	return;
}

static inline void buttonfour(Context & context) {
	context.cl = 4;
	buttonpress(context);
	return;
}

static inline void buttonfive(Context & context) {
	context.cl = 5;
	buttonpress(context);
	return;
}

static inline void buttonsix(Context & context) {
	context.cl = 6;
	buttonpress(context);
	return;
}

static inline void buttonseven(Context & context) {
	context.cl = 7;
	buttonpress(context);
	return;
}

static inline void buttoneight(Context & context) {
	context.cl = 8;
	buttonpress(context);
	return;
}

static inline void buttonnine(Context & context) {
	context.cl = 9;
	buttonpress(context);
	return;
}

static inline void buttonnought(Context & context) {
	context.cl = 10;
	buttonpress(context);
	return;
}

static inline void buttonenter(Context & context) {
	context.cl = 11;
	buttonpress(context);
	return;
}

static inline void buttonpress(Context & context) {
	context.ch = context.cl;
	context._add(context.ch, 100);
	context._cmp(context.data.byte(100), context.ch);
	if (context.flags.z()) goto alreadyb;
	context.data.byte(100) = context.ch;
	context.al = context.cl;
	context._add(context.al, 4);
	context.push(context.cx);
	commandonly(context);
	context.cx = context.pop();
alreadyb:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notb;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dob;
notb:
	return;
dob:
	context.data.byte(278) = context.cl;
	context._add(context.cl, 21);
	context.data.byte(281) = context.cl;
	context.data.byte(282) = 40;
	context._cmp(context.cl, 32);
	if (context.flags.z()) goto nonoise;
	context.al = 10;
	playchannel1(context);
nonoise:
	return;
}

static inline void showouterpad(Context & context) {
	context.di = 36+112-3;
	context.bx = 72-4;
	context.ds = context.data.word(458);
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.di = 36+112+74;
	context.bx = 72+76;
	context.ds = context.data.word(458);
	context.al = 37;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void showkeypad(Context & context) {
	context.al = 22;
	context.di = 36+112+9;
	context.bx = 72+5;
	singlekey(context);
	context.al = 23;
	context.di = 36+112+31;
	context.bx = 72+5;
	singlekey(context);
	context.al = 24;
	context.di = 36+112+53;
	context.bx = 72+5;
	singlekey(context);
	context.al = 25;
	context.di = 36+112+9;
	context.bx = 72+23;
	singlekey(context);
	context.al = 26;
	context.di = 36+112+31;
	context.bx = 72+23;
	singlekey(context);
	context.al = 27;
	context.di = 36+112+53;
	context.bx = 72+23;
	singlekey(context);
	context.al = 28;
	context.di = 36+112+9;
	context.bx = 72+41;
	singlekey(context);
	context.al = 29;
	context.di = 36+112+31;
	context.bx = 72+41;
	singlekey(context);
	context.al = 30;
	context.di = 36+112+53;
	context.bx = 72+41;
	singlekey(context);
	context.al = 31;
	context.di = 36+112+9;
	context.bx = 72+59;
	singlekey(context);
	context.al = 32;
	context.di = 36+112+31;
	context.bx = 72+59;
	singlekey(context);
	context._cmp(context.data.byte(287), 0);
	if (context.flags.z()) goto notenter;
	context._sub(context.data.byte(287), 1);
	context.al = 36;
	context.bx = 72-1+63;
	context._cmp(context.data.byte(37), 0);
	if (!context.flags.z()) goto changelight;
	context.al = 41;
	context.bx = 72+4+63;
changelight:
	context._cmp(context.data.byte(287), 60);
	if (context.flags.c()) goto gotlight;
	context._cmp(context.data.byte(287), 100);
	if (!context.flags.c()) goto gotlight;
	context._sub(context.al, 1);
gotlight:
	context.ds = context.data.word(458);
	context.ah = 0;
	context.di = 36+112+60;
	showframe(context);
notenter:
	return;
}

static inline void singlekey(Context & context) {
	context._cmp(context.data.byte(281), context.al);
	if (!context.flags.z()) goto gotkey;
	context._add(context.al, 11);
	context._cmp(context.data.byte(282), 8);
	if (!context.flags.c()) goto gotkey;
	context._sub(context.al, 11);
gotkey:
	context.ds = context.data.word(458);
	context._sub(context.al, 20);
	context.ah = 0;
	showframe(context);
	return;
}

static inline void dumpkeypad(Context & context) {
	context.di = 36+112-3;
	context.bx = 72-4;
	context.cl = 120;
	context.ch = 90;
	multidump(context);
	return;
}

static inline void usemenu(Context & context) {
	getridofreels(context);
	loadmenu(context);
	createpanel(context);
	showpanel(context);
	showicon(context);
	context.data.byte(62) = 0;
	drawfloor(context);
	printsprites(context);
	context.al = 4;
	context.ah = 0;
	context.di = 80+40-48;
	context.bx = 60-4;
	context.ds = context.data.word(460);
	showframe(context);
	getundermenu(context);
	context.al = 5;
	context.ah = 0;
	context.di = 80+40+54;
	context.bx = 60+72;
	context.ds = context.data.word(460);
	showframe(context);
	worktoscreenm(context);
	context.data.byte(103) = 0;
menuloop:
	delpointer(context);
	putundermenu(context);
	showmenu(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumpmenu(context);
	dumptextline(context);
	context.bx = 3525;
	checkcoords(context);
	context._cmp(context.data.byte(103), 1);
	if (!context.flags.z()) goto menuloop;
	context.data.byte(131) = 0;
	redrawmainscrn(context);
	getridoftemp(context);
	getridoftemp2(context);
	restorereels(context);
	worktoscreenm(context);
	return;
menulist:
	return;
}

static inline void dumpmenu(Context & context) {
	context.di = 80+40;
	context.bx = 60;
	context.cl = 48;
	context.ch = 48;
	multidump(context);
	return;
}

static inline void getundermenu(Context & context) {
	context.di = 80+40;
	context.bx = 60;
	context.cl = 48;
	context.ch = 48;
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4);
	multiget(context);
	return;
}

static inline void putundermenu(Context & context) {
	context.di = 80+40;
	context.bx = 60;
	context.cl = 48;
	context.ch = 48;
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4);
	multiput(context);
	return;
}

static inline void showoutermenu(Context & context) {
	context.al = 40;
	context.ah = 0;
	context.di = 80+40-34;
	context.bx = 60-40;
	context.ds = context.data.word(458);
	showframe(context);
	context.al = 41;
	context.ah = 0;
	context.di = 80+40+64-34;
	context.bx = 60-40;
	context.ds = context.data.word(458);
	showframe(context);
	context.al = 42;
	context.ah = 0;
	context.di = 80+40-26;
	context.bx = 60+57-40;
	context.ds = context.data.word(458);
	showframe(context);
	context.al = 43;
	context.ah = 0;
	context.di = 80+40+64-26;
	context.bx = 60+57-40;
	context.ds = context.data.word(458);
	showframe(context);
	return;
}

static inline void showmenu(Context & context) {
	context._add(context.data.byte(290), 1);
	context._cmp(context.data.byte(290), 37*2);
	if (!context.flags.z()) goto menuframeok;
	context.data.byte(290) = 0;
menuframeok:
	context.al = context.data.byte(290);
	context._shr(context.al, 1);
	context.ah = 0;
	context.di = 80+40;
	context.bx = 60;
	context.ds = context.data.word(458);
	showframe(context);
	return;
}

static inline void loadmenu(Context & context) {
	context.dx = 1845;
	loadintotemp(context);
	context.dx = 2000;
	loadintotemp2(context);
	return;
}

static inline void viewfolder(Context & context) {
	context.data.byte(131) = 1;
	getridofall(context);
	loadfolder(context);
	context.data.byte(288) = 0;
	showfolder(context);
	worktoscreenm(context);
	context.data.byte(103) = 0;
folderloop:
	delpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.bx = 3547;
	checkcoords(context);
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto folderloop;
	context.data.byte(131) = 0;
	getridoftemp(context);
	getridoftemp2(context);
	getridoftemp3(context);
	getridoftempcharset(context);
	restoreall(context);
	redrawmainscrn(context);
	worktoscreenm(context);
	return;
}

static inline void nextfolder(Context & context) {
	context._cmp(context.data.byte(288), 12);
	if (!context.flags.z()) goto cannextf;
	blank(context);
	return;
cannextf:
	context._cmp(context.data.byte(100), 201);
	if (context.flags.z()) goto alreadynextf;
	context.data.byte(100) = 201;
	context.al = 16;
	commandonly(context);
alreadynextf:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notnextf;
	context._cmp(context.ax, 1);
	if (context.flags.z()) goto donextf;
notnextf:
	return;
donextf:
	context._add(context.data.byte(288), 1);
	folderhints(context);
	delpointer(context);
	showfolder(context);
	context.data.word(202) = 0;
	context.bx = 3547;
	checkcoords(context);
	worktoscreenm(context);
	return;
}

static inline void folderhints(Context & context) {
	context._cmp(context.data.byte(288), 5);
	if (!context.flags.z()) goto notaideadd;
	context._cmp(context.data.byte(44), 1);
	if (context.flags.z()) goto notaideadd;
	context.al = 13;
	getlocation(context);
	context._cmp(context.al, 1);
	if (context.flags.z()) goto notaideadd;
	context.al = 13;
	setlocation(context);
	showfolder(context);
	context.al = 30;
	findtext1(context);
	context.di = 0;
	context.bx = 86;
	context.dl = 141;
	context.ah = 16;
	printdirect(context);
	worktoscreenm(context);
	context.cx = 200;
	hangonp(context);
	return;
notaideadd:
	context._cmp(context.data.byte(288), 9);
	if (!context.flags.z()) goto notaristoadd;
	context.al = 7;
	getlocation(context);
	context._cmp(context.al, 1);
	if (context.flags.z()) goto notaristoadd;
	context.al = 7;
	setlocation(context);
	showfolder(context);
	context.al = 31;
	findtext1(context);
	context.di = 0;
	context.bx = 86;
	context.dl = 141;
	context.ah = 16;
	printdirect(context);
	worktoscreenm(context);
	context.cx = 200;
	hangonp(context);
notaristoadd:
	return;
}

static inline void lastfolder(Context & context) {
	context._cmp(context.data.byte(288), 0);
	if (!context.flags.z()) goto canlastf;
	blank(context);
	return;
canlastf:
	context._cmp(context.data.byte(100), 202);
	if (context.flags.z()) goto alreadylastf;
	context.data.byte(100) = 202;
	context.al = 17;
	commandonly(context);
alreadylastf:
	context._cmp(context.data.byte(288), 0);
	if (context.flags.z()) goto notlastf;
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notlastf;
	context._cmp(context.ax, 1);
	if (context.flags.z()) goto dolastf;
notlastf:
	return;
dolastf:
	context._sub(context.data.byte(288), 1);
	delpointer(context);
	showfolder(context);
	context.data.word(202) = 0;
	context.bx = 3547;
	checkcoords(context);
	worktoscreenm(context);
	return;
}

static inline void loadfolder(Context & context) {
	context.dx = 2312;
	loadintotemp(context);
	context.dx = 2325;
	loadintotemp2(context);
	context.dx = 2338;
	loadintotemp3(context);
	context.dx = 1896;
	loadtempcharset(context);
	context.dx = 2208;
	loadtemptext(context);
	return;
}

static inline void showfolder(Context & context) {
	context.data.byte(100) = 255;
	context._cmp(context.data.byte(288), 0);
	if (context.flags.z()) goto closedfolder;
	usetempcharset(context);
	createpanel2(context);
	context.ds = context.data.word(458);
	context.di = 0;
	context.bx = 0;
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 143;
	context.bx = 0;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 0;
	context.bx = 92;
	context.al = 2;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 143;
	context.bx = 92;
	context.al = 3;
	context.ah = 0;
	showframe(context);
	folderexit(context);
	context._cmp(context.data.byte(288), 1);
	if (context.flags.z()) goto noleftpage;
	showleftpage(context);
noleftpage:
	context._cmp(context.data.byte(288), 12);
	if (context.flags.z()) goto norightpage;
	showrightpage(context);
norightpage:
	usecharset1(context);
	undertextline(context);
	return;
closedfolder:
	createpanel2(context);
	context.ds = context.data.word(462);
	context.di = 143-28;
	context.bx = 0;
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(462);
	context.di = 143-28;
	context.bx = 92;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	folderexit(context);
	undertextline(context);
	return;
}

static inline void folderexit(Context & context) {
	context.ds = context.data.word(460);
	context.di = 296;
	context.bx = 178;
	context.al = 6;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void showleftpage(Context & context) {
	context.ds = context.data.word(460);
	context.di = 0;
	context.bx = 12;
	context.al = 3;
	context.ah = 0;
	showframe(context);
	context.bx = 12+5;
	context.cx = 9;
leftpageloop:
	context.push(context.cx);
	context.push(context.bx);
	context.ds = context.data.word(460);
	context.di = 0;
	context.al = 4;
	context.ah = 0;
	showframe(context);
	context.bx = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 16);
	if (--context.cx) goto leftpageloop;
	context.ds = context.data.word(460);
	context.di = 0;
	context.al = 5;
	context.ah = 0;
	showframe(context);
	context.data.word(77) = 8;
	context.data.word(71) = 91;
	context.data.byte(72) = 1;
	context.bl = context.data.byte(288);
	context._sub(context.bl, 1);
	context._sub(context.bl, 1);
	context._add(context.bl, context.bl);
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(466);
	context.si = context.data.word(context.bx);
	context._add(context.si, 66*2);
	context.di = 2;
	context.bx = 48;
	context.dl = 140;
	context.cx = 2;
twolotsleft:
	context.push(context.cx);
contleftpage:
	printdirect(context);
	context._add(context.bx, context.data.word(77));
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto contleftpage;
	context.cx = context.pop();
	if (--context.cx) goto twolotsleft;
	context.data.byte(72) = 0;
	context.data.word(71) = 0;
	context.data.word(77) = 10;
	context.es = context.data.word(400);
	context.ds = context.data.word(400);
	context.di = (48*320)+2;
	context.si = (48*320)+2+130;
	context.cx = 120;
flipfolder:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.si);
	context.cx = 65;
flipfolderline:
	context.al = context.data.byte(context.di);
	context.ah = context.data.byte(context.si);
	context.data.byte(context.di) = context.ah;
	context.data.byte(context.si) = context.al;
	context._sub(context.si, 1);
	context._add(context.di, 1);
	if (--context.cx) goto flipfolderline;
	context.si = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.si, 320);
	context._add(context.di, 320);
	if (--context.cx) goto flipfolder;
	return;
}

static inline void showrightpage(Context & context) {
	context.ds = context.data.word(460);
	context.di = 143;
	context.bx = 12;
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.bx = 12+37;
	context.cx = 7;
rightpageloop:
	context.push(context.cx);
	context.push(context.bx);
	context.ds = context.data.word(460);
	context.di = 143;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.bx = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 16);
	if (--context.cx) goto rightpageloop;
	context.ds = context.data.word(460);
	context.di = 143;
	context.al = 2;
	context.ah = 0;
	showframe(context);
	context.data.word(77) = 8;
	context.data.byte(72) = 1;
	context.bl = context.data.byte(288);
	context._sub(context.bl, 1);
	context._add(context.bl, context.bl);
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(466);
	context.si = context.data.word(context.bx);
	context._add(context.si, 66*2);
	context.di = 152;
	context.bx = 48;
	context.dl = 140;
	context.cx = 2;
twolotsright:
	context.push(context.cx);
contrightpage:
	printdirect(context);
	context._add(context.bx, context.data.word(77));
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto contrightpage;
	context.cx = context.pop();
	if (--context.cx) goto twolotsright;
	context.data.byte(72) = 0;
	context.data.word(77) = 10;
	return;
}

static inline void entersymbol(Context & context) {
	context.data.byte(131) = 1;
	getridofreels(context);
	context.dx = 2351;
	loadintotemp(context);
	context.data.byte(291) = 24;
	context.data.byte(293) = 0;
	context.data.byte(294) = 24;
	context.data.byte(296) = 0;
	redrawmainscrn(context);
	showsymbol(context);
	undertextline(context);
	worktoscreenm(context);
	context.data.byte(103) = 0;
symbolloop:
	delpointer(context);
	updatesymboltop(context);
	updatesymbolbot(context);
	showsymbol(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	dumpsymbol(context);
	context.bx = 3589;
	checkcoords(context);
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto symbolloop;
	context._cmp(context.data.byte(295), 3);
	if (!context.flags.z()) goto symbolwrong;
	context._cmp(context.data.byte(292), 5);
	if (!context.flags.z()) goto symbolwrong;
	context.al = 43;
	removesetobject(context);
	context.al = 46;
	placesetobject(context);
	context.ah = context.data.byte(185);
	context._add(context.ah, 12);
	context.al = 0;
	turnanypathon(context);
	context.data.byte(131) = 0;
	redrawmainscrn(context);
	getridoftemp(context);
	restorereels(context);
	worktoscreenm(context);
	context.al = 13;
	playchannel1(context);
	return;
symbolwrong:
	context.al = 46;
	removesetobject(context);
	context.al = 43;
	placesetobject(context);
	context.ah = context.data.byte(185);
	context._add(context.ah, 12);
	context.al = 0;
	turnanypathoff(context);
	context.data.byte(131) = 0;
	redrawmainscrn(context);
	getridoftemp(context);
	restorereels(context);
	worktoscreenm(context);
	return;
}

static inline void quitsymbol(Context & context) {
	context._cmp(context.data.byte(291), 24);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(294), 24);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(100), 222);
	if (context.flags.z()) goto alreadyqs;
	context.data.byte(100) = 222;
	context.al = 18;
	commandonly(context);
alreadyqs:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notqs;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doqs;
notqs:
	return;
doqs:
	context.data.byte(103) = 1;
	return;
}

static inline void settopleft(Context & context) {
	context._cmp(context.data.byte(293), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(100), 210);
	if (context.flags.z()) goto alreadytopl;
	context.data.byte(100) = 210;
	context.al = 19;
	commandonly(context);
alreadytopl:
	context._cmp(context.data.word(202), 0);
	if (context.flags.z()) goto notopleft;
	context.data.byte(293) = -1;
notopleft:
	return;
}

static inline void settopright(Context & context) {
	context._cmp(context.data.byte(293), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(100), 211);
	if (context.flags.z()) goto alreadytopr;
	context.data.byte(100) = 211;
	context.al = 20;
	commandonly(context);
alreadytopr:
	context._cmp(context.data.word(202), 0);
	if (context.flags.z()) goto notopright;
	context.data.byte(293) = 1;
notopright:
	return;
}

static inline void setbotleft(Context & context) {
	context._cmp(context.data.byte(296), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(100), 212);
	if (context.flags.z()) goto alreadybotl;
	context.data.byte(100) = 212;
	context.al = 21;
	commandonly(context);
alreadybotl:
	context._cmp(context.data.word(202), 0);
	if (context.flags.z()) goto nobotleft;
	context.data.byte(296) = -1;
nobotleft:
	return;
}

static inline void setbotright(Context & context) {
	context._cmp(context.data.byte(296), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(100), 213);
	if (context.flags.z()) goto alreadybotr;
	context.data.byte(100) = 213;
	context.al = 22;
	commandonly(context);
alreadybotr:
	context._cmp(context.data.word(202), 0);
	if (context.flags.z()) goto nobotright;
	context.data.byte(296) = 1;
nobotright:
	return;
}

static inline void dumpsymbol(Context & context) {
	context.data.byte(109) = 0;
	context.di = 64;
	context.bx = 56+20;
	context.cl = 104;
	context.ch = 60;
	multidump(context);
	return;
}

static inline void showsymbol(Context & context) {
	context.al = 12;
	context.ah = 0;
	context.di = 64;
	context.bx = 56;
	context.ds = context.data.word(458);
	showframe(context);
	context.al = context.data.byte(291);
	context.ah = 0;
	context.di = context.ax;
	context._add(context.di, 64-44);
	context.al = context.data.byte(292);
	context.bx = 56+20;
	context.ds = context.data.word(458);
	context.ah = 32;
	context.push(context.ax);
	context.push(context.di);
	context.push(context.bx);
	context.push(context.ds);
	showframe(context);
	context.ds = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	context.ax = context.pop();
	nextsymbol(context);
	context._add(context.di, 49);
	context.push(context.ax);
	context.push(context.di);
	context.push(context.bx);
	context.push(context.ds);
	showframe(context);
	context.ds = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	context.ax = context.pop();
	nextsymbol(context);
	context._add(context.di, 49);
	showframe(context);
	context.al = context.data.byte(294);
	context.ah = 0;
	context.di = context.ax;
	context._add(context.di, 64-44);
	context.al = context.data.byte(295);
	context._add(context.al, 6);
	context.bx = 56+49;
	context.ds = context.data.word(458);
	context.ah = 32;
	context.push(context.ax);
	context.push(context.di);
	context.push(context.bx);
	context.push(context.ds);
	showframe(context);
	context.ds = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	context.ax = context.pop();
	nextsymbol(context);
	context._add(context.di, 49);
	context.push(context.ax);
	context.push(context.di);
	context.push(context.bx);
	context.push(context.ds);
	showframe(context);
	context.ds = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	context.ax = context.pop();
	nextsymbol(context);
	context._add(context.di, 49);
	showframe(context);
	return;
}

static inline void nextsymbol(Context & context) {
	context._add(context.al, 1);
	context._cmp(context.al, 6);
	if (context.flags.z()) goto topwrap;
	context._cmp(context.al, 12);
	if (context.flags.z()) goto botwrap;
	return;
topwrap:
	context.al = 0;
	return;
botwrap:
	context.al = 6;
	return;
}

static inline void updatesymboltop(Context & context) {
	context._cmp(context.data.byte(293), 0);
	if (context.flags.z()) goto topfinished;
	context._cmp(context.data.byte(293), -1);
	if (context.flags.z()) goto backwards;
	context._add(context.data.byte(291), 1);
	context._cmp(context.data.byte(291), 49);
	if (!context.flags.z()) goto notwrapfor;
	context.data.byte(291) = 0;
	context._sub(context.data.byte(292), 1);
	context._cmp(context.data.byte(292), -1);
	if (!context.flags.z()) goto topfinished;
	context.data.byte(292) = 5;
	return;
notwrapfor:
	context._cmp(context.data.byte(291), 24);
	if (!context.flags.z()) goto topfinished;
	context.data.byte(293) = 0;
	return;
backwards:
	context._sub(context.data.byte(291), 1);
	context._cmp(context.data.byte(291), -1);
	if (!context.flags.z()) goto notwrapback;
	context.data.byte(291) = 48;
	context._add(context.data.byte(292), 1);
	context._cmp(context.data.byte(292), 6);
	if (!context.flags.z()) goto topfinished;
	context.data.byte(292) = 0;
	return;
notwrapback:
	context._cmp(context.data.byte(291), 24);
	if (!context.flags.z()) goto topfinished;
	context.data.byte(293) = 0;
topfinished:
	return;
}

static inline void updatesymbolbot(Context & context) {
	context._cmp(context.data.byte(296), 0);
	if (context.flags.z()) goto botfinished;
	context._cmp(context.data.byte(296), -1);
	if (context.flags.z()) goto backwardsbot;
	context._add(context.data.byte(294), 1);
	context._cmp(context.data.byte(294), 49);
	if (!context.flags.z()) goto notwrapforb;
	context.data.byte(294) = 0;
	context._sub(context.data.byte(295), 1);
	context._cmp(context.data.byte(295), -1);
	if (!context.flags.z()) goto botfinished;
	context.data.byte(295) = 5;
	return;
notwrapforb:
	context._cmp(context.data.byte(294), 24);
	if (!context.flags.z()) goto botfinished;
	context.data.byte(296) = 0;
	return;
backwardsbot:
	context._sub(context.data.byte(294), 1);
	context._cmp(context.data.byte(294), -1);
	if (!context.flags.z()) goto notwrapbackb;
	context.data.byte(294) = 48;
	context._add(context.data.byte(295), 1);
	context._cmp(context.data.byte(295), 6);
	if (!context.flags.z()) goto botfinished;
	context.data.byte(295) = 0;
	return;
notwrapbackb:
	context._cmp(context.data.byte(294), 24);
	if (!context.flags.z()) goto botfinished;
	context.data.byte(296) = 0;
botfinished:
	return;
}

static inline void dumpsymbox(Context & context) {
	context._cmp(context.data.word(303), -1);
	if (context.flags.z()) goto nodumpsym;
	context.di = context.data.word(303);
	context.bx = context.data.word(305);
	context.cl = 30;
	context.ch = 77;
	multidump(context);
	context.data.word(303) = -1;
nodumpsym:
	return;
}

static inline void usediary(Context & context) {
	getridofreels(context);
	context.dx = 2052;
	loadintotemp(context);
	context.dx = 2221;
	loadtemptext(context);
	context.dx = 1896;
	loadtempcharset(context);
	createpanel(context);
	showicon(context);
	showdiary(context);
	undertextline(context);
	showdiarypage(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	context.data.byte(103) = 0;
diaryloop:
	delpointer(context);
	readmouse(context);
	showdiarykeys(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumpdiarykeys(context);
	dumptextline(context);
	context.bx = 3651;
	checkcoords(context);
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto diaryloop;
	getridoftemp(context);
	getridoftemptext(context);
	getridoftempcharset(context);
	restorereels(context);
	context.data.byte(131) = 0;
	redrawmainscrn(context);
	worktoscreenm(context);
	return;
diarylist:
	return;
}

static inline void showdiary(Context & context) {
	context.al = 1;
	context.ah = 0;
	context.di = 68+24;
	context.bx = 48+12+37;
	context.ds = context.data.word(458);
	showframe(context);
	context.al = 2;
	context.ah = 0;
	context.di = 68+24+176;
	context.bx = 48+12+108;
	context.ds = context.data.word(458);
	showframe(context);
	return;
}

static inline void showdiarykeys(Context & context) {
	context._cmp(context.data.byte(282), 0);
	if (context.flags.z()) goto nokeyatall;
	context._sub(context.data.byte(282), 1);
	context._cmp(context.data.byte(282), 0);
	if (context.flags.z()) goto nokeyatall;
	context._cmp(context.data.byte(278), 'N');
	if (!context.flags.z()) goto nokeyn;
	context.al = 3;
	context._cmp(context.data.byte(282), 1);
	if (context.flags.z()) goto gotkeyn;
	context.al = 4;
gotkeyn:
	context.ah = 0;
	context.di = 68+24+94;
	context.bx = 48+12+97;
	context.ds = context.data.word(458);
	showframe(context);
	context._cmp(context.data.byte(282), 1);
	if (!context.flags.z()) goto notshown;
	showdiarypage(context);
notshown:
	return;
nokeyn:
	context.al = 5;
	context._cmp(context.data.byte(282), 1);
	if (context.flags.z()) goto gotkeyp;
	context.al = 6;
gotkeyp:
	context.ah = 0;
	context.di = 68+24+151;
	context.bx = 48+12+71;
	context.ds = context.data.word(458);
	showframe(context);
	context._cmp(context.data.byte(282), 1);
	if (!context.flags.z()) goto notshowp;
	showdiarypage(context);
notshowp:
	return;
nokeyatall:
	return;
}

static inline void dumpdiarykeys(Context & context) {
	context._cmp(context.data.byte(282), 1);
	if (!context.flags.z()) goto notdumpdiary;
	context._cmp(context.data.byte(43), 1);
	if (context.flags.z()) goto notsartadd;
	context._cmp(context.data.byte(289), 5);
	if (!context.flags.z()) goto notsartadd;
	context._cmp(context.data.byte(289), 5);
	if (!context.flags.z()) goto notsartadd;
	context.al = 6;
	getlocation(context);
	context._cmp(context.al, 1);
	if (context.flags.z()) goto notsartadd;
	context.al = 6;
	setlocation(context);
	delpointer(context);
	context.al = 12;
	findtext1(context);
	context.di = 70;
	context.bx = 106;
	context.dl = 241;
	context.ah = 16;
	printdirect(context);
	worktoscreenm(context);
	context.cx = 200;
	hangonp(context);
	createpanel(context);
	showicon(context);
	showdiary(context);
	showdiarypage(context);
	worktoscreenm(context);
	showpointer(context);
	return;
notsartadd:
	context.di = 68+24+48;
	context.bx = 48+12+15;
	context.cl = 200;
	context.ch = 16;
	multidump(context);
notdumpdiary:
	context.di = 68+24+94;
	context.bx = 48+12+97;
	context.cl = 16;
	context.ch = 16;
	multidump(context);
	context.di = 68+24+151;
	context.bx = 48+12+71;
	context.cl = 16;
	context.ch = 16;
	multidump(context);
	return;
}

static inline void diarykeyp(Context & context) {
	context._cmp(context.data.byte(100), 214);
	if (context.flags.z()) goto alreadykeyp;
	context.data.byte(100) = 214;
	context.al = 23;
	commandonly(context);
alreadykeyp:
	context._cmp(context.data.word(202), 0);
	if (context.flags.z()) goto notkeyp;
	context.ax = context.data.word(212);
	context._cmp(context.ax, context.data.word(202));
	if (context.flags.z()) goto notkeyp;
	context._cmp(context.data.byte(282), 0);
	if (!context.flags.z()) goto notkeyp;
	context.al = 16;
	playchannel1(context);
	context.data.byte(282) = 12;
	context.data.byte(278) = 'P';
	context._sub(context.data.byte(289), 1);
	context._cmp(context.data.byte(289), -1);
	if (!context.flags.z()) goto notkeyp;
	context.data.byte(289) = 11;
notkeyp:
	return;
}

static inline void diarykeyn(Context & context) {
	context._cmp(context.data.byte(100), 213);
	if (context.flags.z()) goto alreadykeyn;
	context.data.byte(100) = 213;
	context.al = 23;
	commandonly(context);
alreadykeyn:
	context._cmp(context.data.word(202), 0);
	if (context.flags.z()) goto notkeyn;
	context.ax = context.data.word(212);
	context._cmp(context.ax, context.data.word(202));
	if (context.flags.z()) goto notkeyn;
	context._cmp(context.data.byte(282), 0);
	if (!context.flags.z()) goto notkeyn;
	context.al = 16;
	playchannel1(context);
	context.data.byte(282) = 12;
	context.data.byte(278) = 'N';
	context._add(context.data.byte(289), 1);
	context._cmp(context.data.byte(289), 12);
	if (!context.flags.z()) goto notkeyn;
	context.data.byte(289) = 0;
notkeyn:
	return;
}

static inline void showdiarypage(Context & context) {
	context.al = 0;
	context.ah = 0;
	context.di = 68+24;
	context.bx = 48+12;
	context.ds = context.data.word(458);
	showframe(context);
	context.al = context.data.byte(289);
	findtext1(context);
	context.data.byte(72) = 1;
	usetempcharset(context);
	context.di = 68+24+48;
	context.bx = 48+12+16;
	context.dl = 240;
	context.ah = 16;
	context.data.word(71) = 91+91;
	printdirect(context);
	context.di = 68+24+129;
	context.bx = 48+12+16;
	context.dl = 240;
	context.ah = 16;
	printdirect(context);
	context.di = 68+24+48;
	context.bx = 48+12+23;
	context.dl = 240;
	context.ah = 16;
	printdirect(context);
	context.data.byte(72) = 0;
	context.data.word(71) = 0;
	usecharset1(context);
	return;
}

static inline void findtext1(Context & context) {
	context.ah = 0;
	context.si = context.ax;
	context._add(context.si, context.si);
	context.es = context.data.word(466);
	context.ax = context.data.word(context.si);
	context._add(context.ax, 66*2);
	context.si = context.ax;
	return;
}

static inline void zoomonoff(Context & context) {
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(234), 2);
	if (context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(100), 222);
	if (context.flags.z()) goto alreadyonoff;
	context.data.byte(100) = 222;
	context.al = 39;
	commandonly(context);
alreadyonoff:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto nozoomonoff;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dozoomonoff;
nozoomonoff:
	return;
dozoomonoff:
	context.al = context.data.byte(8);
	context._xor(context.al, 1);
	context.data.byte(8) = context.al;
	createpanel(context);
	context.data.byte(62) = 0;
	drawfloor(context);
	printsprites(context);
	reelsonscreen(context);
	showicon(context);
	getunderzoom(context);
	undertextline(context);
	context.al = 39;
	commandonly(context);
	readmouse(context);
	worktoscreenm(context);
	return;
}

static inline void saveload(Context & context) {
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(234), 2);
	if (context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(100), 253);
	if (context.flags.z()) goto alreadyops;
	context.data.byte(100) = 253;
	context.al = 43;
	commandonly(context);
alreadyops:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto noops;
	context._and(context.ax, 1);
	if (context.flags.z()) goto noops;
	dosaveload(context);
noops:
	return;
}

static inline void dosaveload(Context & context) {
	context.data.byte(231) = 0;
	context.data.word(79) = 70;
	context.data.word(81) = 182-8;
	context.data.byte(82) = 181;
	context.data.byte(131) = 1;
	clearwork(context);
	createpanel2(context);
	undertextline(context);
	getridofall(context);
	loadsavebox(context);
	showopbox(context);
	showmainops(context);
	worktoscreen(context);
	goto donefirstops;
restartops:
	showopbox(context);
	showmainops(context);
	worktoscreenm(context);
donefirstops:
	context.data.byte(103) = 0;
waitops:
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	delpointer(context);
	context.bx = 3693;
	checkcoords(context);
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto waitops;
	context._cmp(context.data.byte(103), 2);
	if (context.flags.z()) goto restartops;
	context.data.word(79) = 13;
	context.data.word(81) = 182;
	context.data.byte(82) = 240;
	context._cmp(context.data.byte(103), 4);
	if (context.flags.z()) goto justret;
	getridoftemp(context);
	restoreall(context);
	redrawmainscrn(context);
	worktoscreenm(context);
	context.data.byte(100) = 200;
justret:
	context.data.byte(131) = 0;
	return;
}

static inline void getbackfromops(Context & context) {
	context._cmp(context.data.byte(56), 2);
	if (context.flags.z()) goto opsblock1;
	getback1(context);
	return;
opsblock1:
	blank(context);
	return;
}

static inline void showmainops(Context & context) {
	context.ds = context.data.word(458);
	context.di = 60+10;
	context.bx = 52+10;
	context.al = 8;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 60+59;
	context.bx = 52+30;
	context.al = 7;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 60+128+4;
	context.bx = 52+12;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void showdiscops(Context & context) {
	context.ds = context.data.word(458);
	context.di = 60+128+4;
	context.bx = 52+12;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 60+10;
	context.bx = 52+10;
	context.al = 9;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 60+59;
	context.bx = 52+30;
	context.al = 10;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 60+176+2;
	context.bx = 52+60-4;
	context.al = 5;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void loadgame(Context & context) {
	context._cmp(context.data.byte(100), 246);
	if (context.flags.z()) goto alreadyload;
	context.data.byte(100) = 246;
	context.al = 41;
	commandonly(context);
alreadyload:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto noload;
	context._cmp(context.ax, 1);
	if (context.flags.z()) goto doload;
noload:
	return;
doload:
	context.data.byte(340) = 1;
	showopbox(context);
	showloadops(context);
	context.data.byte(341) = 0;
	showslots(context);
	shownames(context);
	context.data.byte(231) = 0;
	worktoscreenm(context);
	namestoold(context);
	context.data.byte(103) = 0;
loadops:
	delpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.bx = 3735;
	checkcoords(context);
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto loadops;
	context._cmp(context.data.byte(103), 2);
	if (context.flags.z()) goto quitloaded;
	getridoftemp(context);
	context.dx = context.ds;
	context.es = context.dx;
	context.bx = 7782;
	startloading(context);
	loadroomssample(context);
	context.data.byte(74) = 1;
	context.data.byte(188) = 255;
	clearsprites(context);
	initman(context);
	initrain(context);
	context.data.word(79) = 13;
	context.data.word(81) = 182;
	context.data.byte(82) = 240;
	startup(context);
	worktoscreen(context);
	context.data.byte(103) = 4;
quitloaded:
	return;
}

static inline void getbacktoops(Context & context) {
	context._cmp(context.data.byte(100), 201);
	if (context.flags.z()) goto alreadygetops;
	context.data.byte(100) = 201;
	context.al = 42;
	commandonly(context);
alreadygetops:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto nogetbackops;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dogetbackops;
nogetbackops:
	return;
dogetbackops:
	oldtonames(context);
	context.data.byte(103) = 2;
	return;
}

static inline void discops(Context & context) {
	context._cmp(context.data.byte(100), 249);
	if (context.flags.z()) goto alreadydiscops;
	context.data.byte(100) = 249;
	context.al = 43;
	commandonly(context);
alreadydiscops:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto nodiscops;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dodiscops;
nodiscops:
	return;
dodiscops:
	scanfornames(context);
	context.data.byte(340) = 2;
	showopbox(context);
	showdiscops(context);
	context.data.byte(341) = 0;
	worktoscreenm(context);
	context.data.byte(103) = 0;
discopsloop:
	delpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.bx = 3777;
	checkcoords(context);
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto discopsloop;
	return;
}

static inline void savegame(Context & context) {
	context._cmp(context.data.byte(56), 2);
	if (!context.flags.z()) goto cansaveok;
	blank(context);
	return;
cansaveok:
	context._cmp(context.data.byte(100), 247);
	if (context.flags.z()) goto alreadysave;
	context.data.byte(100) = 247;
	context.al = 44;
	commandonly(context);
alreadysave:
	context.ax = context.data.word(202);
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dosave;
	return;
dosave:
	context.data.byte(340) = 2;
	showopbox(context);
	showsaveops(context);
	context.data.byte(341) = 0;
	showslots(context);
	shownames(context);
	worktoscreenm(context);
	namestoold(context);
	context.data.word(394) = 0;
	context.data.word(396) = 0;
	context.data.byte(103) = 0;
saveops:
	delpointer(context);
	checkinput(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.bx = 3819;
	checkcoords(context);
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto saveops;
	return;
}

static inline void actualsave(Context & context) {
	context._cmp(context.data.byte(100), 222);
	if (context.flags.z()) goto alreadyactsave;
	context.data.byte(100) = 222;
	context.al = 44;
	commandonly(context);
alreadyactsave:
	context.ax = context.data.word(202);
	context._and(context.ax, 1);
	if (context.flags.z()) goto noactsave;
	context.dx = context.ds;
	context.ds = context.dx;
	context.si = 8367;
	context.al = context.data.byte(341);
	context.ah = 0;
	context.cx = 17;
	context._mul(context.cx);
	context._add(context.si, context.ax);
	context._add(context.si, 1);
	context._cmp(context.data.byte(context.si), 0);
	if (context.flags.z()) goto noactsave;
	context.al = context.data.byte(9);
	context.ah = 0;
	context.cx = 32;
	context._mul(context.cx);
	context.ds = context.cs;
	context.si = 5971;
	context._add(context.si, context.ax);
	context.di = 7782;
	context.bx = context.di;
	context.es = context.cs;
	context.cx = 16;
	while(--context.cx) 	context._movsw();
 	context.al = context.data.byte(531);
	context.data.byte(context.bx+13) = context.al;
	context.al = context.data.byte(148);
	context.data.byte(context.bx+15) = context.al;
	context.al = context.data.byte(149);
	context.data.byte(context.bx+16) = context.al;
	context.al = context.data.byte(35);
	context.data.byte(context.bx+20) = context.al;
	context.al = context.data.byte(475);
	context.data.byte(context.bx+21) = context.al;
	context.al = context.data.byte(133);
	context.data.byte(context.bx+22) = context.al;
	context.al = 255;
	context.data.byte(context.bx+27) = context.al;
	saveposition(context);
	getridoftemp(context);
	restoreall(context);
	context.data.word(79) = 13;
	context.data.word(81) = 182;
	context.data.byte(82) = 240;
	redrawmainscrn(context);
	worktoscreenm(context);
	context.data.byte(103) = 4;
noactsave:
	return;
}

static inline void actualload(Context & context) {
	context._cmp(context.data.byte(100), 221);
	if (context.flags.z()) goto alreadyactload;
	context.data.byte(100) = 221;
	context.al = 41;
	commandonly(context);
alreadyactload:
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto notactload;
	context._cmp(context.ax, 1);
	if (!context.flags.z()) goto notactload;
	context.dx = context.ds;
	context.ds = context.dx;
	context.si = 8367;
	context.al = context.data.byte(341);
	context.ah = 0;
	context.cx = 17;
	context._mul(context.cx);
	context._add(context.si, context.ax);
	context._add(context.si, 1);
	context._cmp(context.data.byte(context.si), 0);
	if (context.flags.z()) goto notactload;
	loadposition(context);
	context.data.byte(103) = 1;
notactload:
	return;
}

static inline void selectslot2(Context & context) {
	context._cmp(context.data.word(202), 0);
	if (context.flags.z()) goto noselslot2;
	context.data.byte(340) = 2;
noselslot2:
	selectslot(context);
	return;
}

static inline void checkinput(Context & context) {
	context._cmp(context.data.byte(340), 3);
	if (context.flags.z()) goto nokeypress;
	readkey(context);
	context.al = context.data.byte(142);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto nokeypress;
	context._cmp(context.al, 13);
	if (!context.flags.z()) goto notret;
	context.data.byte(340) = 3;
	goto afterkey;
notret:
	context._cmp(context.al, 8);
	if (!context.flags.z()) goto nodel2;
	context._cmp(context.data.byte(342), 0);
	if (context.flags.z()) goto nokeypress;
	getnamepos(context);
	context._sub(context.data.byte(342), 1);
	context.data.byte(context.bx) = 0;
	context.data.byte(context.bx+1) = 1;
	goto afterkey;
nodel2:
spacepress:
	context._cmp(context.data.byte(342), 14);
	if (context.flags.z()) goto nokeypress;
	getnamepos(context);
	context._add(context.data.byte(342), 1);
	context.al = context.data.byte(142);
	context.data.byte(context.bx+1) = context.al;
	context.data.byte(context.bx+2) = 0;
	context.data.byte(context.bx+3) = 1;
	goto afterkey;
nokeypress:
	return;
afterkey:
	showopbox(context);
	shownames(context);
	showslots(context);
	showsaveops(context);
	worktoscreenm(context);
	return;
}

static inline void getnamepos(Context & context) {
	context.al = context.data.byte(341);
	context.ah = 0;
	context.cx = 17;
	context._mul(context.cx);
	context.dx = context.ds;
	context.es = context.dx;
	context.bx = 8367;
	context._add(context.bx, context.ax);
	context.al = context.data.byte(342);
	context.ah = 0;
	context._add(context.bx, context.ax);
	return;
}

static inline void showloadops(Context & context) {
	context.ds = context.data.word(458);
	context.di = 60+128+4;
	context.bx = 52+12;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 60+176+2;
	context.bx = 52+60-4;
	context.al = 5;
	context.ah = 0;
	showframe(context);
	context.di = 60+104;
	context.bx = 52+14;
	context.al = 55;
	context.dl = 101;
	printmessage(context);
	return;
}

static inline void showsaveops(Context & context) {
	context.ds = context.data.word(458);
	context.di = 60+128+4;
	context.bx = 52+12;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 60+176+2;
	context.bx = 52+60-4;
	context.al = 5;
	context.ah = 0;
	showframe(context);
	context.di = 60+104;
	context.bx = 52+14;
	context.al = 54;
	context.dl = 101;
	printmessage(context);
	return;
}

static inline void selectslot(Context & context) {
	context._cmp(context.data.byte(100), 244);
	if (context.flags.z()) goto alreadysel;
	context.data.byte(100) = 244;
	context.al = 45;
	commandonly(context);
alreadysel:
	context.ax = context.data.word(202);
	context._cmp(context.ax, 1);
	if (!context.flags.z()) goto noselslot;
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto noselslot;
	context._cmp(context.data.byte(340), 3);
	if (!context.flags.z()) goto notnocurs;
	context._sub(context.data.byte(340), 1);
notnocurs:
	oldtonames(context);
	context.ax = context.data.word(200);
	context._sub(context.ax, 52+4);
	context.cl = -1;
getslotnum:
	context._add(context.cl, 1);
	context._sub(context.ax, 11);
	if (!context.flags.c()) goto getslotnum;
	context.data.byte(341) = context.cl;
	delpointer(context);
	showopbox(context);
	showslots(context);
	shownames(context);
	context._cmp(context.data.byte(340), 1);
	if (context.flags.z()) goto isloadmode;
	showsaveops(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
isloadmode:
	showloadops(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
noselslot:
	return;
}

static inline void showslots(Context & context) {
	context.di = 60+7;
	context.bx = 52+8;
	context.al = 2;
	context.ds = context.data.word(458);
	context.ah = 0;
	showframe(context);
	context.di = 60+10;
	context.bx = 52+11;
	context.cl = 0;
slotloop:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.bx);
	context._cmp(context.cl, context.data.byte(341));
	if (!context.flags.z()) goto nomatchslot;
	context.al = 3;
	context.ds = context.data.word(458);
	context.ah = 0;
	showframe(context);
nomatchslot:
	context.bx = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 10);
	context._add(context.cl, 1);
	context._cmp(context.cl, 7);
	if (!context.flags.z()) goto slotloop;
	return;
}

static inline void shownames(Context & context) {
	context.dx = context.ds;
	context.es = context.dx;
	context.si = 8367+1;
	context.di = 60+21;
	context.bx = 52+10;
	context.cl = 0;
shownameloop:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.es);
	context.push(context.bx);
	context.push(context.si);
	context.al = 4;
	context._cmp(context.cl, context.data.byte(341));
	if (!context.flags.z()) goto nomatchslot2;
	context._cmp(context.data.byte(340), 2);
	if (!context.flags.z()) goto loadmode;
	context.dx = context.si;
	context.cx = 15;
	context._add(context.si, 15);
zerostill:
	context._sub(context.si, 1);
	context._sub(context.cl, 1);
	context._cmp(context.data.byte(context.si), 1);
	if (!context.flags.z()) goto foundcharacter;
	goto zerostill;
foundcharacter:
	context.data.byte(342) = context.cl;
	context.data.byte(context.si) = '/';
	context.data.byte(context.si+1) = 0;
	context.push(context.si);
	context.si = context.dx;
	context.dl = 200;
	context.ah = 0;
	printdirect(context);
	context.si = context.pop();
	context.data.byte(context.si) = 0;
	context.data.byte(context.si+1) = 1;
	goto afterprintname;
loadmode:
	context.al = 0;
	context.dl = 200;
	context.ah = 0;
	context.data.word(71) = 91;
	printdirect(context);
	context.data.word(71) = 0;
	goto afterprintname;
nomatchslot2:
	context.dl = 200;
	context.ah = 0;
	printdirect(context);
afterprintname:
	context.si = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.si, 17);
	context._add(context.bx, 10);
	context._add(context.cl, 1);
	context._cmp(context.cl, 7);
	if (!context.flags.z()) goto shownameloop;
	return;
}

static inline void namestoold(Context & context) {
	context.ds = context.cs;
	context.si = 8367;
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5);
	context.es = context.data.word(412);
	context.cx = 17*4;
	while(--context.cx) 	context._movsb();
 	return;
}

static inline void oldtonames(Context & context) {
	context.es = context.cs;
	context.di = 8367;
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5);
	context.ds = context.data.word(412);
	context.cx = 17*4;
	while(--context.cx) 	context._movsb();
 	return;
}

static inline void makeheader(Context & context) {
	context.dx = context.ds;
	context.es = context.dx;
	context.di = 5952;
	context.ax = 17;
	storeit(context);
	context.ax = 68-context.data.byte(1);
	storeit(context);
	context.ax = 0+2080+30000+(16*114)+((114+2)*2)+18000;
	storeit(context);
	context.ax = 250*4;
	storeit(context);
	context.ax = 48;
	storeit(context);
	context.ax = 991-context.data.byte(537);
	storeit(context);
	return;
}

static inline void storeit(Context & context) {
	context._cmp(context.ax, 0);
	if (!context.flags.z()) goto isntblank;
	context._add(context.ax, 1);
isntblank:
	context._stosw();
	return;
}

static inline void findlen(Context & context) {
	context._sub(context.bx, 1);
	context._add(context.bx, context.ax);
nextone:
	context._cmp(context.cl, context.data.byte(context.bx));
	if (!context.flags.z()) goto foundlen;
	context._sub(context.bx, 1);
	context._sub(context.ax, 1);
	context._cmp(context.ax, 0);
	if (!context.flags.z()) goto nextone;
foundlen:
	return;
}

static inline void newgame(Context & context) {
	context._cmp(context.data.byte(100), 251);
	if (context.flags.z()) goto alreadynewgame;
	context.data.byte(100) = 251;
	context.al = 47;
	commandonly(context);
alreadynewgame:
	context.ax = context.data.word(202);
	context._cmp(context.ax, 1);
	if (!context.flags.z()) goto nonewgame;
	context.data.byte(103) = 3;
nonewgame:
	return;
}

static inline void doload(Context & context) {
	context.data.byte(340) = 1;
	showopbox(context);
	showloadops(context);
	context.data.byte(341) = 0;
	showslots(context);
	shownames(context);
	context.data.byte(231) = 0;
	worktoscreenm(context);
	namestoold(context);
	context.data.byte(103) = 0;
loadops:
	delpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.bx = 3735;
	checkcoords(context);
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto loadops;
	context._cmp(context.data.byte(103), 2);
	if (context.flags.z()) goto quitloaded;
	getridoftemp(context);
	context.dx = context.ds;
	context.es = context.dx;
	context.bx = 7782;
	startloading(context);
	loadroomssample(context);
	context.data.byte(74) = 1;
	context.data.byte(188) = 255;
	clearsprites(context);
	initman(context);
	initrain(context);
	context.data.word(79) = 13;
	context.data.word(81) = 182;
	context.data.byte(82) = 240;
	startup(context);
	worktoscreen(context);
	context.data.byte(103) = 4;
quitloaded:
	return;
}

static inline void loadold(Context & context) {
	context._cmp(context.data.byte(100), 252);
	if (context.flags.z()) goto alreadyloadold;
	context.data.byte(100) = 252;
	context.al = 48;
	commandonly(context);
alreadyloadold:
	context.ax = context.data.word(202);
	context._and(context.ax, 1);
	if (context.flags.z()) goto noloadold;
	doload(context);
	context._cmp(context.data.byte(103), 4);
	if (context.flags.z()) goto noloadold;
	showdecisions(context);
	worktoscreenm(context);
	context.data.byte(103) = 0;
noloadold:
	return;
}

static inline void createname(Context & context) {
	context.push(context.ax);
	context.di = 4932;
	context.data.byte(context.di+0) = context.dl;
	context.data.byte(context.di+3) = context.cl;
	context.al = context.dh;
	context.ah = '0'-1;
findten:
	context._add(context.ah, 1);
	context._sub(context.al, 10);
	if (!context.flags.c()) goto findten;
	context.data.byte(context.di+1) = context.ah;
	context._add(context.al, 10+'0');
	context.data.byte(context.di+2) = context.al;
	context.ax = context.pop();
	context.cl = '0'-1;
thousandsc:
	context._add(context.cl, 1);
	context._sub(context.ax, 1000);
	if (!context.flags.c()) goto thousandsc;
	context._add(context.ax, 1000);
	context.data.byte(context.di+4) = context.cl;
	context.cl = '0'-1;
hundredsc:
	context._add(context.cl, 1);
	context._sub(context.ax, 100);
	if (!context.flags.c()) goto hundredsc;
	context._add(context.ax, 100);
	context.data.byte(context.di+5) = context.cl;
	context.cl = '0'-1;
tensc:
	context._add(context.cl, 1);
	context._sub(context.ax, 10);
	if (!context.flags.c()) goto tensc;
	context._add(context.ax, 10);
	context.data.byte(context.di+6) = context.cl;
	context._add(context.al, '0');
	context.data.byte(context.di+7) = context.al;
	return;
}

static inline void makenextblock(Context & context) {
	volumeadjust(context);
	loopchannel0(context);
	context._cmp(context.data.word(521), 0);
	if (context.flags.z()) goto mightbeonlych0;
	context._cmp(context.data.word(506), 0);
	if (context.flags.z()) goto mightbeonlych1;
	context._sub(context.data.word(506), 1);
	context._sub(context.data.word(521), 1);
	bothchannels(context);
	return;
mightbeonlych1:
	context.data.byte(507) = 255;
	context._cmp(context.data.word(521), 0);
	if (context.flags.z()) goto notch1only;
	context._sub(context.data.word(521), 1);
	channel1only(context);
notch1only:
	return;
mightbeonlych0:
	context.data.byte(515) = 255;
	context._cmp(context.data.word(506), 0);
	if (context.flags.z()) goto notch0only;
	context._sub(context.data.word(506), 1);
	channel0only(context);
	return;
notch0only:
	context.es = context.data.word(353);
	context.di = context.data.word(525);
	context.cx = 1024;
	context.ax = 0x7f7f;
	while(--context.cx) 	context._stosw();
	context._and(context.di, 16384-1);
	context.data.word(525) = context.di;
	return;
}

static inline void volumeadjust(Context & context) {
	context.al = context.data.byte(388);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto volok;
	context.al = context.data.byte(386);
	context._cmp(context.al, context.data.byte(387));
	if (context.flags.z()) goto volfinish;
	context._add(context.data.byte(389), 64);
	if (!context.flags.z()) goto volok;
	context.al = context.data.byte(386);
	context._add(context.al, context.data.byte(388));
	context.data.byte(386) = context.al;
	return;
volfinish:
	context.data.byte(388) = 0;
volok:
	return;
}

static inline void loopchannel0(Context & context) {
	context._cmp(context.data.word(506), 0);
	if (!context.flags.z()) goto notloop;
	context._cmp(context.data.byte(508), 0);
	if (context.flags.z()) goto notloop;
	context._cmp(context.data.byte(508), 255);
	if (context.flags.z()) goto endlessloop;
	context._sub(context.data.byte(508), 1);
endlessloop:
	context.ax = context.data.word(510);
	context.data.word(502) = context.ax;
	context.ax = context.data.word(512);
	context.data.word(504) = context.ax;
	context.ax = context.data.word(506);
	context._add(context.ax, context.data.word(514));
	context.data.word(506) = context.ax;
	return;
notloop:
	return;
}

static inline void channel0tran(Context & context) {
	context._cmp(context.data.byte(386), 0);
	if (!context.flags.z()) goto lowvolumetran;
	context.cx = 1024;
	while(--context.cx) 	context._movsw();
 	return;
lowvolumetran:
	context.cx = 1024;
	context.bh = context.data.byte(386);
	context.bl = 0;
	context._add(context.bx, 16384-256);
volloop:
	context._lodsw();
	context.bl = context.al;
	context.al = context.data.byte(context.bx);
	context.bl = context.ah;
	context.ah = context.data.byte(context.bx);
	context._stosw();
	if (--context.cx) goto volloop;
	return;
}

static inline void domix(Context & context) {
	context._cmp(context.data.byte(386), 0);
	if (!context.flags.z()) goto lowvolumemix;
slow:
	context._lodsb();
	context.ah = context.data.byte(context.bx);
	context._add(context.bx, 1);
	context._cmp(context.al, context.dh);
	if (!context.flags.c()) goto toplot;
botlot:
	context._cmp(context.ah, context.dh);
	if (!context.flags.c()) goto nodistort;
	context._add(context.al, context.ah);
	if (context.flags.s()) goto botok;
	context._xor(context.al, context.al);
	context._stosb();
	if (--context.cx) goto slow;
	goto doneit;
botok:
	context._xor(context.al, context.dh);
	context._stosb();
	if (--context.cx) goto slow;
	goto doneit;
toplot:
	context._cmp(context.ah, context.dh);
	if (context.flags.c()) goto nodistort;
	context._add(context.al, context.ah);
	if (!context.flags.s()) goto topok;
	context.al = context.dl;
	context._stosb();
	if (--context.cx) goto slow;
	goto doneit;
topok:
	context._xor(context.al, context.dh);
	context._stosb();
	if (--context.cx) goto slow;
	goto doneit;
nodistort:
	context._add(context.al, context.ah);
	context._xor(context.al, context.dh);
	context._stosb();
	if (--context.cx) goto slow;
	goto doneit;
lowvolumemix:
	context._lodsb();
	context.push(context.bx);
	context.bh = context.data.byte(386);
	context._add(context.bh, 63);
	context.bl = context.al;
	context.al = context.data.byte(context.bx);
	context.bx = context.pop();
	context.ah = context.data.byte(context.bx);
	context._add(context.bx, 1);
	context._cmp(context.al, context.dh);
	if (!context.flags.c()) goto toplotv;
botlotv:
	context._cmp(context.ah, context.dh);
	if (!context.flags.c()) goto nodistortv;
	context._add(context.al, context.ah);
	if (context.flags.s()) goto botokv;
	context._xor(context.al, context.al);
	context._stosb();
	if (--context.cx) goto lowvolumemix;
	goto doneit;
botokv:
	context._xor(context.al, context.dh);
	context._stosb();
	if (--context.cx) goto lowvolumemix;
	goto doneit;
toplotv:
	context._cmp(context.ah, context.dh);
	if (context.flags.c()) goto nodistortv;
	context._add(context.al, context.ah);
	if (!context.flags.s()) goto topokv;
	context.al = context.dl;
	context._stosb();
	if (--context.cx) goto lowvolumemix;
	goto doneit;
topokv:
	context._xor(context.al, context.dh);
	context._stosb();
	if (--context.cx) goto lowvolumemix;
	goto doneit;
nodistortv:
	context._add(context.al, context.ah);
	context._xor(context.al, context.dh);
	context._stosb();
	if (--context.cx) goto lowvolumemix;
doneit:
	return;
}

static inline void seecommandtail(Context & context) {
	context.data.word(372) = 0x220;
	context.data.byte(378) = 5;
	context.data.byte(379) = 1;
	context.data.byte(73) = 0;
	context.bx = 2;
	context.ax = context.data.word(context.bx);
	context.dx = context.es;
	context._sub(context.ax, context.dx);
	context.data.word(534) = context.ax;
	context.bx = 0x02c;
	context.ax = context.data.word(context.bx);
	context.push(context.es);
	context.push(context.bx);
	context.es = context.ax;
	context.bx = 0;
findblaster:
	context.ax = context.data.word(context.bx);
	context._cmp(context.ax, 0);
	if (context.flags.z()) goto endofenvironment;
	context._cmp(context.al, 'B');
	if (!context.flags.z()) goto notblast;
	context._cmp(context.ah, 'L');
	if (!context.flags.z()) goto notblast;
	context._cmp(context.data.byte(context.bx+2), 'A');
	if (!context.flags.z()) goto notblast;
	context._cmp(context.data.byte(context.bx+3), 'S');
	if (!context.flags.z()) goto notblast;
	context._cmp(context.data.byte(context.bx+4), 'T');
	if (!context.flags.z()) goto notblast;
	context._cmp(context.data.byte(context.bx+5), 'E');
	if (!context.flags.z()) goto notblast;
	context._cmp(context.data.byte(context.bx+6), 'R');
	if (!context.flags.z()) goto notblast;
	context._add(context.bx, 7);
	parseblaster(context);
	goto endofenvironment;
notblast:
	context._add(context.bx, 1);
	goto findblaster;
endofenvironment:
	context.bx = context.pop();
	context.es = context.pop();
	context.bx = 0x81;
	parseblaster(context);
	return;
}

static inline void parseblaster(Context & context) {
lookattail:
	context.al = context.data.byte(context.bx);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto endtail;
	context._cmp(context.al, 13);
	if (context.flags.z()) goto endtail;
	context._cmp(context.al, 'i');
	if (context.flags.z()) goto issoundint;
	context._cmp(context.al, 'I');
	if (context.flags.z()) goto issoundint;
	context._cmp(context.al, 'b');
	if (context.flags.z()) goto isbright;
	context._cmp(context.al, 'B');
	if (context.flags.z()) goto isbright;
	context._cmp(context.al, 'a');
	if (context.flags.z()) goto isbaseadd;
	context._cmp(context.al, 'A');
	if (context.flags.z()) goto isbaseadd;
	context._cmp(context.al, 'n');
	if (context.flags.z()) goto isnosound;
	context._cmp(context.al, 'N');
	if (context.flags.z()) goto isnosound;
	context._cmp(context.al, 'd');
	if (context.flags.z()) goto isdma;
	context._cmp(context.al, 'D');
	if (context.flags.z()) goto isdma;
	context._add(context.bx, 1);
	if (--context.cx) goto lookattail;
	return;
issoundint:
	context.al = context.data.byte(context.bx+1);
	context._sub(context.al, '0');
	context.data.byte(378) = context.al;
	context._add(context.bx, 1);
	goto lookattail;
isdma:
	context.al = context.data.byte(context.bx+1);
	context._sub(context.al, '0');
	context.data.byte(379) = context.al;
	context._add(context.bx, 1);
	goto lookattail;
isbaseadd:
	context.push(context.cx);
	context.al = context.data.byte(context.bx+2);
	context._sub(context.al, '0');
	context.ah = 0;
	context.cl = 4;
	context._shl(context.ax, context.cl);
	context._add(context.ax, 0x200);
	context.data.word(372) = context.ax;
	context.cx = context.pop();
	context._add(context.bx, 1);
	goto lookattail;
isbright:
	context.data.byte(73) = 1;
	context._add(context.bx, 1);
	goto lookattail;
isnosound:
	context.data.byte(378) = 255;
	context._add(context.bx, 1);
	goto lookattail;
endtail:
	return;
}

static inline void showbyte(Context & context) {
	context.dl = context.al;
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	onedigit(context);
	context.data.byte(context.di) = context.dl;
	context.dl = context.al;
	context._and(context.dl, 15);
	onedigit(context);
	context.data.byte(context.di+1) = context.dl;
	context._add(context.di, 3);
	return;
}

static inline void onedigit(Context & context) {
	context._cmp(context.dl, 10);
	if (!context.flags.c()) goto morethan10;
	context._add(context.dl, '0');
	return;
morethan10:
	context._sub(context.dl, 10);
	context._add(context.dl, 'A');
	return;
}

static inline void showword(Context & context) {
	context.ch = 0;
	context.bx = 10000;
	context.cl = 47;
word1:
	context._add(context.cl, 1);
	context._sub(context.ax, context.bx);
	if (!context.flags.c()) goto word1;
	context._add(context.ax, context.bx);
	convnum(context);
	context.data.byte(context.di) = context.cl;
	context.bx = 1000;
	context.cl = 47;
word2:
	context._add(context.cl, 1);
	context._sub(context.ax, context.bx);
	if (!context.flags.c()) goto word2;
	context._add(context.ax, context.bx);
	convnum(context);
	context.data.byte(context.di+1) = context.cl;
	context.bx = 100;
	context.cl = 47;
word3:
	context._add(context.cl, 1);
	context._sub(context.ax, context.bx);
	if (!context.flags.c()) goto word3;
	context._add(context.ax, context.bx);
	convnum(context);
	context.data.byte(context.di+2) = context.cl;
	context.bx = 10;
	context.cl = 47;
word4:
	context._add(context.cl, 1);
	context._sub(context.ax, context.bx);
	if (!context.flags.c()) goto word4;
	context._add(context.ax, context.bx);
	convnum(context);
	context.data.byte(context.di+3) = context.cl;
	context._add(context.al, 48);
	context.cl = context.al;
	convnum(context);
	context.data.byte(context.di+4) = context.cl;
	return;
}

static inline void convnum(Context & context) {
	context._cmp(context.ch, 0);
	if (!context.flags.z()) goto noconvnum;
	context._cmp(context.cl, '0');
	if (!context.flags.z()) goto notzeronum;
	context.cl = 32;
	goto noconvnum;
notzeronum:
	context.ch = 1;
noconvnum:
	return;
}

static inline void madmanrun(Context & context) {
	context._cmp(context.data.byte(9), 14);
	if (!context.flags.z()) { identifyob(context); return; };
	context._cmp(context.data.byte(148), 22);
	if (!context.flags.z()) { identifyob(context); return; };
	context._cmp(context.data.byte(234), 2);
	if (!context.flags.z()) { identifyob(context); return; };
	context._cmp(context.data.byte(53), 0);
	if (!context.flags.z()) { identifyob(context); return; };
	context._cmp(context.data.byte(100), 211);
	if (context.flags.z()) goto alreadyrun;
	context.data.byte(100) = 211;
	context.al = 52;
	commandonly(context);
alreadyrun:
	context._cmp(context.data.word(202), 1);
	if (!context.flags.z()) goto norun;
	context.ax = context.data.word(202);
	context._cmp(context.ax, context.data.word(212));
	if (context.flags.z()) goto norun;
	context.data.byte(65) = 8;
norun:
	return;
}

static inline void identifyob(Context & context) {
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) { blank(context); return; };
	context.ax = context.data.word(198);
	context._sub(context.ax, context.data.word(117));
	context._cmp(context.ax, 22*8);
	if (context.flags.c()) goto notover1;
	blank(context);
	return;
notover1:
	context.bx = context.data.word(200);
	context._sub(context.bx, context.data.word(119));
	context._cmp(context.bx, 20*8);
	if (context.flags.c()) goto notover2;
	blank(context);
	return;
notover2:
	context.data.byte(237) = 1;
	context.ah = context.bl;
	context.push(context.ax);
	findpathofpoint(context);
	context.data.byte(474) = context.dl;
	context.ax = context.pop();
	context.push(context.ax);
	findfirstpath(context);
	context.data.byte(476) = context.al;
	context.ax = context.pop();
	checkifex(context);
	if (!context.flags.z()) goto finishidentify;
	checkiffree(context);
	if (!context.flags.z()) goto finishidentify;
	checkifperson(context);
	if (!context.flags.z()) goto finishidentify;
	checkifset(context);
	if (!context.flags.z()) goto finishidentify;
	context.ax = context.data.word(198);
	context._sub(context.ax, context.data.word(117));
	context.cl = context.al;
	context.ax = context.data.word(200);
	context._sub(context.ax, context.data.word(119));
	context.ch = context.al;
	checkone(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto nothingund;
	context._cmp(context.data.byte(56), 1);
	if (context.flags.z()) goto nothingund;
	context.ah = 3;
	obname(context);
finishidentify:
	return;
nothingund:
	blank(context);
	return;
}

static inline void checkifperson(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5);
	context.cx = 12;
identifyreel:
	context.push(context.cx);
	context._cmp(context.data.byte(context.bx+4), 255);
	if (context.flags.z()) goto notareelid;
	context.push(context.es);
	context.push(context.bx);
	context.push(context.ax);
	context.ax = context.data.word(context.bx+0);
	context.data.word(239) = context.ax;
	getreelstart(context);
	context._cmp(context.data.word(context.si+2), 0x0ffff);
	if (!context.flags.z()) goto notblankpers;
	context._add(context.si, 5);
notblankpers:
	context.cx = context.data.word(context.si+2);
	context.ax = context.data.word(context.si+0);
	context.push(context.cx);
	getreelframeax(context);
	context.cx = context.pop();
	context._add(context.cl, context.data.byte(context.bx+4));
	context._add(context.ch, context.data.byte(context.bx+5));
	context.dx = context.cx;
	context._add(context.dl, context.data.byte(context.bx+0));
	context._add(context.dh, context.data.byte(context.bx+1));
	context.ax = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.al, context.cl);
	if (context.flags.c()) goto notareelid;
	context._cmp(context.ah, context.ch);
	if (context.flags.c()) goto notareelid;
	context._cmp(context.al, context.dl);
	if (!context.flags.c()) goto notareelid;
	context._cmp(context.ah, context.dh);
	if (!context.flags.c()) goto notareelid;
	context.cx = context.pop();
	context.ax = context.data.word(context.bx+2);
	context.data.word(249) = context.ax;
	context.al = context.data.byte(context.bx+4);
	context.ah = 5;
	obname(context);
	context.al = 0;
	context._cmp(context.al, 1);
	return;
notareelid:
	context.cx = context.pop();
	context._add(context.bx, 5);
	context._sub(context.cx, 1);
	if (!context.flags.z()) goto identifyreel;
	return;
}

static inline void checkifset(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(127*5);
	context.cx = 127;
identifyset:
	context._cmp(context.data.byte(context.bx+4), 255);
	if (context.flags.z()) goto notasetid;
	context._cmp(context.al, context.data.byte(context.bx));
	if (context.flags.c()) goto notasetid;
	context._cmp(context.al, context.data.byte(context.bx+2));
	if (!context.flags.c()) goto notasetid;
	context._cmp(context.ah, context.data.byte(context.bx+1));
	if (context.flags.c()) goto notasetid;
	context._cmp(context.ah, context.data.byte(context.bx+3));
	if (!context.flags.c()) goto notasetid;
	pixelcheckset(context);
	if (context.flags.z()) goto notasetid;
	isitdescribed(context);
	if (context.flags.z()) goto notasetid;
	context.al = context.data.byte(context.bx+4);
	context.ah = 1;
	obname(context);
	context.al = 0;
	context._cmp(context.al, 1);
	return;
notasetid:
	context._sub(context.bx, 5);
	context._sub(context.cx, 1);
	context._cmp(context.cx, -1);
	if (!context.flags.z()) goto identifyset;
	return;
}

static inline void checkifex(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(99*5);
	context.cx = 99;
identifyex:
	context._cmp(context.data.byte(context.bx+4), 255);
	if (context.flags.z()) goto notanexid;
	context._cmp(context.al, context.data.byte(context.bx));
	if (context.flags.c()) goto notanexid;
	context._cmp(context.al, context.data.byte(context.bx+2));
	if (!context.flags.c()) goto notanexid;
	context._cmp(context.ah, context.data.byte(context.bx+1));
	if (context.flags.c()) goto notanexid;
	context._cmp(context.ah, context.data.byte(context.bx+3));
	if (!context.flags.c()) goto notanexid;
	context.al = context.data.byte(context.bx+4);
	context.ah = 4;
	obname(context);
	context.al = 1;
	context._cmp(context.al, 0);
	return;
notanexid:
	context._sub(context.bx, 5);
	context._sub(context.cx, 1);
	context._cmp(context.cx, -1);
	if (!context.flags.z()) goto identifyex;
	return;
}

static inline void checkiffree(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(79*5);
	context.cx = 79;
identifyfree:
	context._cmp(context.data.byte(context.bx+4), 255);
	if (context.flags.z()) goto notafreeid;
	context._cmp(context.al, context.data.byte(context.bx));
	if (context.flags.c()) goto notafreeid;
	context._cmp(context.al, context.data.byte(context.bx+2));
	if (!context.flags.c()) goto notafreeid;
	context._cmp(context.ah, context.data.byte(context.bx+1));
	if (context.flags.c()) goto notafreeid;
	context._cmp(context.ah, context.data.byte(context.bx+3));
	if (!context.flags.c()) goto notafreeid;
	context.al = context.data.byte(context.bx+4);
	context.ah = 2;
	obname(context);
	context.al = 0;
	context._cmp(context.al, 1);
	return;
notafreeid:
	context._sub(context.bx, 5);
	context._sub(context.cx, 1);
	context._cmp(context.cx, -1);
	if (!context.flags.z()) goto identifyfree;
	return;
}

static inline void isitdescribed(Context & context) {
	context.push(context.ax);
	context.push(context.cx);
	context.push(context.es);
	context.push(context.bx);
	context.al = context.data.byte(context.bx+4);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.es = context.data.word(440);
	context._add(context.bx, 0);
	context.ax = context.data.word(context.bx);
	context._add(context.ax, 0+(130*2));
	context.bx = context.ax;
	context.dl = context.data.byte(context.bx);
	context.bx = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context.ax = context.pop();
	context._cmp(context.dl, 0);
	return;
}

static inline void findfirstpath(Context & context) {
	context.push(context.ax);
	context.bx = 0;
	context.es = context.data.word(450);
	context.al = context.data.byte(185);
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.cx = context.pop();
	context.dl = 0;
fpathloop:
	context.ax = context.data.word(context.bx+2);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto nofirst;
	context._cmp(context.cl, context.al);
	if (context.flags.c()) goto nofirst;
	context._cmp(context.ch, context.ah);
	if (context.flags.c()) goto nofirst;
	context.ax = context.data.word(context.bx+4);
	context._cmp(context.cl, context.al);
	if (!context.flags.c()) goto nofirst;
	context._cmp(context.ch, context.ah);
	if (!context.flags.c()) goto nofirst;
	goto gotfirst;
nofirst:
	context._add(context.bx, 8);
	context._add(context.dl, 1);
	context._cmp(context.dl, 12);
	if (!context.flags.z()) goto fpathloop;
	context.al = 0;
	return;
gotfirst:
	context.al = context.data.byte(context.bx+6);
	return;
}

static inline void turnpathoff(Context & context) {
	context.push(context.ax);
	context.push(context.ax);
	context.cl = 0;
	context.ch = context.data.byte(185);
	context._add(context.ch, 100);
	findormake(context);
	context.ax = context.pop();
	getroomspaths(context);
	context.ax = context.pop();
	context._cmp(context.al, 255);
	if (context.flags.z()) goto nopathoff;
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = 0;
	context.data.byte(context.bx+6) = context.al;
nopathoff:
	return;
}

static inline void turnanypathoff(Context & context) {
	context.push(context.ax);
	context.push(context.ax);
	context.cl = 0;
	context.ch = context.ah;
	context._add(context.ch, 100);
	findormake(context);
	context.ax = context.pop();
	context.al = context.ah;
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context.es = context.data.word(450);
	context.bx = 0;
	context._add(context.bx, context.ax);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = 0;
	context.data.byte(context.bx+6) = context.al;
	return;
}

static inline void obname(Context & context) {
	context._cmp(context.data.byte(473), 0);
	if (context.flags.z()) goto notnewpath;
	context.data.byte(473) = 0;
	goto diff;
notnewpath:
	context._cmp(context.ah, context.data.byte(100));
	if (context.flags.z()) goto notdiffob;
	goto diff;
notdiffob:
	context._cmp(context.al, context.data.byte(99));
	if (!context.flags.z()) goto diff;
	context._cmp(context.data.byte(306), 1);
	if (context.flags.z()) goto walkandexamine;
	context._cmp(context.data.word(202), 0);
	if (context.flags.z()) goto noobselect;
	context._cmp(context.data.byte(100), 3);
	if (!context.flags.z()) goto isntblock;
	context._cmp(context.data.byte(153), 2);
	if (context.flags.c()) goto noobselect;
isntblock:
	context.bl = context.data.byte(475);
	context._cmp(context.bl, context.data.byte(474));
	if (!context.flags.z()) goto wantstowalk;
	context._cmp(context.data.byte(100), 3);
	if (context.flags.z()) goto wantstowalk;
	finishedwalking(context);
	if (!context.flags.z()) goto noobselect;
	context._cmp(context.data.byte(100), 5);
	if (context.flags.z()) goto wantstotalk;
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) goto noobselect;
	examineob(context);
	return;
wantstotalk:
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) goto noobselect;
	talk(context);
	return;
walkandexamine:
	finishedwalking(context);
	if (!context.flags.z()) goto noobselect;
	context.al = context.data.byte(307);
	context.data.byte(100) = context.al;
	context.al = context.data.byte(308);
	context.data.byte(99) = context.al;
	context.data.byte(306) = 0;
	context._cmp(context.data.byte(100), 5);
	if (context.flags.z()) goto noobselect;
	examineob(context);
	return;
wantstowalk:
	setwalk(context);
	context.data.byte(473) = 1;
noobselect:
	return;
diff:
	context.data.byte(99) = context.al;
	context.data.byte(100) = context.ah;
diff2:
	context._cmp(context.data.byte(492), 254);
	if (!context.flags.z()) goto middleofwalk;
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) goto middleofwalk;
	context.al = context.data.byte(133);
	context._cmp(context.al, context.data.byte(135));
	if (!context.flags.z()) goto middleofwalk;
	context._cmp(context.data.byte(100), 3);
	if (!context.flags.z()) goto notblock;
	context.bl = context.data.byte(475);
	context._cmp(context.bl, context.data.byte(474));
	if (!context.flags.z()) goto dontcheck;
	context.cl = context.data.byte(151);
	context._add(context.cl, 12);
	context.ch = context.data.byte(152);
	context._add(context.ch, 12);
	checkone(context);
	context._cmp(context.cl, 2);
	if (context.flags.c()) goto isblock;
dontcheck:
	getflagunderp(context);
	context._cmp(context.data.byte(153), 2);
	if (context.flags.c()) goto isblock;
	context._cmp(context.data.byte(153), 128);
	if (!context.flags.c()) goto isblock;
	goto toofaraway;
notblock:
	context.bl = context.data.byte(475);
	context._cmp(context.bl, context.data.byte(474));
	if (!context.flags.z()) goto toofaraway;
	context._cmp(context.data.byte(100), 3);
	if (context.flags.z()) goto isblock;
	context._cmp(context.data.byte(100), 5);
	if (context.flags.z()) goto isaperson;
	examineobtext(context);
	return;
middleofwalk:
	blocknametext(context);
	return;
isblock:
	blocknametext(context);
	return;
isaperson:
	personnametext(context);
	return;
toofaraway:
	walktotext(context);
	return;
}

static inline void commandonly(Context & context) {
	context.push(context.ax);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.dx);
	context.push(context.es);
	context.push(context.ds);
	context.push(context.si);
	context.push(context.di);
	deltextline(context);
	context.di = context.pop();
	context.si = context.pop();
	context.ds = context.pop();
	context.es = context.pop();
	context.dx = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.es = context.data.word(452);
	context.ax = context.data.word(context.bx);
	context._add(context.ax, 66*2);
	context.si = context.ax;
	context.di = context.data.word(79);
	context.bx = context.data.word(81);
	context.dl = context.data.byte(82);
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.data.byte(109) = 1;
	return;
}

static inline void printmessage2(Context & context) {
	context.push(context.dx);
	context.push(context.bx);
	context.push(context.di);
	context.push(context.ax);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.es = context.data.word(452);
	context.ax = context.data.word(context.bx);
	context._add(context.ax, 66*2);
	context.si = context.ax;
	context.ax = context.pop();
searchmess:
	context.push(context.ax);
	findnextcolon(context);
	context.ax = context.pop();
	context._sub(context.ah, 1);
	if (!context.flags.z()) goto searchmess;
	context.di = context.pop();
	context.bx = context.pop();
	context.dx = context.pop();
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	return;
}

static inline void usetempcharset(Context & context) {
	context.ax = context.data.word(406);
	context.data.word(268) = context.ax;
	return;
}

static inline void blank(Context & context) {
	context._cmp(context.data.byte(100), 199);
	if (context.flags.z()) goto alreadyblnk;
	context.data.byte(100) = 199;
	context.al = 0;
	commandonly(context);
alreadyblnk:
	return;
}

static inline void allpointer(Context & context) {
	readmouse(context);
	showpointer(context);
	dumppointer(context);
	return;
}

static inline void hangoncurs(Context & context) {
monloop1:
	context.push(context.cx);
	printcurs(context);
	vsync(context);
	delcurs(context);
	context.cx = context.pop();
	if (--context.cx) goto monloop1;
	return;
}

static inline void readkey(Context & context) {
	context.bx = context.data.word(396);
	context._cmp(context.bx, context.data.word(394));
	if (context.flags.z()) goto nokey;
	context._add(context.bx, 1);
	context._and(context.bx, 15);
	context.data.word(396) = context.bx;
	context.di = 5715;
	context._add(context.di, context.bx);
	context.al = context.data.byte(context.di);
	context.data.byte(142) = context.al;
	return;
nokey:
	context.data.byte(142) = 0;
	return;
}

static inline void convertkey(Context & context) {
	context._and(context.al, 127);
	context.ah = 0;
	context.di = 5731;
	context._add(context.di, context.ax);
	context.al = context.data.byte(context.di);
	return;
}

static inline void randomnum2(Context & context) {
	context.push(context.ds);
	context.push(context.es);
	context.push(context.di);
	context.push(context.bx);
	context.push(context.ax);
	randomnumber(context);
	context.cl = context.al;
	context.ax = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	context.es = context.pop();
	context.ds = context.pop();
	return;
}

static inline void loadtempcharset(Context & context) {
	standardload(context);
	context.data.word(406) = context.ax;
	return;
}

static inline void getridofall(Context & context) {
	context.es = context.data.word(416);
	deallocatemem(context);
	context.es = context.data.word(444);
	deallocatemem(context);
	context.es = context.data.word(430);
	deallocatemem(context);
	context.es = context.data.word(432);
	deallocatemem(context);
	context.es = context.data.word(434);
	deallocatemem(context);
	context.es = context.data.word(450);
	deallocatemem(context);
	context.es = context.data.word(448);
	deallocatemem(context);
	context.es = context.data.word(440);
	deallocatemem(context);
	context.es = context.data.word(442);
	deallocatemem(context);
	context.es = context.data.word(436);
	deallocatemem(context);
	context.es = context.data.word(446);
	deallocatemem(context);
	context.es = context.data.word(438);
	deallocatemem(context);
	return;
}

static inline void restoreall(Context & context) {
	context.al = context.data.byte(9);
	getroomdata(context);
	context.dx = context.bx;
	openfile(context);
	readheader(context);
	allocateload(context);
	context.ds = context.ax;
	context.data.word(416) = context.ax;
	context.dx = 0;
	loadseg(context);
	context.ds = context.data.word(400);
	context.dx = 0;
	context.cx = 132*66;
	context.al = 0;
	fillspace(context);
	loadseg(context);
	sortoutmap(context);
	allocateload(context);
	context.data.word(444) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	dontloadseg(context);
	allocateload(context);
	context.data.word(430) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(432) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(434) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(450) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(448) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(440) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(442) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(436) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(446) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	dontloadseg(context);
	allocateload(context);
	context.data.word(438) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	closefile(context);
	setallchanges(context);
	return;
}

static inline void disablepath(Context & context) {
	context.push(context.cx);
	context._xchg(context.al, context.ah);
	context.cx = -6;
looky2:
	context._add(context.cx, 6);
	context._sub(context.al, 10);
	if (!context.flags.c()) goto looky2;
	context.al = context.ah;
	context._sub(context.cx, 1);
lookx2:
	context._add(context.cx, 1);
	context._sub(context.al, 11);
	if (!context.flags.c()) goto lookx2;
	context.al = context.cl;
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context.es = context.data.word(450);
	context.bx = 0;
	context._add(context.bx, context.ax);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = 0;
	context.data.byte(context.bx+6) = context.al;
	return;
}

static inline void getridoftempcharset(Context & context) {
	context.es = context.data.word(406);
	deallocatemem(context);
	return;
}

static inline void getridoftempsp(Context & context) {
	context.es = context.data.word(464);
	deallocatemem(context);
	return;
}

static inline void checkforemm(Context & context) {
	return;
}

static inline void trysoundalloc(Context & context) {
	context._cmp(context.data.byte(358), 1);
	if (context.flags.z()) goto gotsoundbuff;
	context._add(context.data.byte(357), 1);
	context.bx = (16384+2048)/16;
	allocatemem(context);
	context.data.word(353) = context.ax;
	context.push(context.ax);
	context.al = context.ah;
	context.cl = 4;
	context._shr(context.al, context.cl);
	context.data.byte(356) = context.al;
	context.ax = context.pop();
	context.cl = 4;
	context._shl(context.ax, context.cl);
	context.data.word(355) = context.ax;
	context._cmp(context.ax, 0x0b7ff);
	if (!context.flags.c()) goto soundfail;
	context.es = context.data.word(353);
	context.di = 0;
	context.cx = 16384/2;
	context.ax = 0x7f7f;
	while(--context.cx) 	context._stosw();
	context.data.byte(358) = 1;
	return;
soundfail:
	context.es = context.data.word(353);
	deallocatemem(context);
gotsoundbuff:
	return;
}

static inline void allocatework(Context & context) {
	context.bx = 0x1000;
	allocatemem(context);
	context.data.word(400) = context.ax;
	return;
}

static inline void allocatebuffers(Context & context) {
	context.bx = 0+2080+30000+(16*114)+((114+2)*2)+18000/16;
	allocatemem(context);
	context.data.word(398) = context.ax;
	trysoundalloc(context);
	context.bx = 0+(66*60)/16;
	allocatemem(context);
	context.data.word(418) = context.ax;
	trysoundalloc(context);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+991-context.data.byte(537)+68-context.data.byte(1)/16;
	allocatemem(context);
	context.data.word(412) = context.ax;
	trysoundalloc(context);
	context.bx = 16*80/16;
	allocatemem(context);
	context.data.word(426) = context.ax;
	trysoundalloc(context);
	context.bx = 64*128/16;
	allocatemem(context);
	context.data.word(428) = context.ax;
	trysoundalloc(context);
	context.bx = 22*8*20*8/16;
	allocatemem(context);
	context.data.word(402) = context.ax;
	allocatework(context);
	context.bx = 2048/16;
	allocatemem(context);
	context.data.word(420) = context.ax;
	context.bx = 2048/16;
	allocatemem(context);
	context.data.word(422) = context.ax;
	return;
}

static inline void clearbuffers(Context & context) {
	context.es = context.data.word(412);
	context.cx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+991-context.data.byte(537)+68-context.data.byte(1)/2;
	context.ax = 0;
	context.di = 0;
	while(--context.cx) 	context._stosw();
	context.es = context.data.word(398);
	context.cx = 0+2080+30000+(16*114)+((114+2)*2)+18000/2;
	context.ax = 0x0ffff;
	context.di = 0;
	while(--context.cx) 	context._stosw();
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64);
	context.ds = context.cs;
	context.si = 537;
	context.cx = 991-context.data.byte(537);
	while(--context.cx) 	context._movsb();
 	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+991-context.data.byte(537);
	context.ds = context.cs;
	context.si = 1;
	context.cx = 68-context.data.byte(1);
	while(--context.cx) 	context._movsb();
 	clearchanges(context);
	return;
}

static inline void clearpalette(Context & context) {
	context.data.byte(344) = 0;
	clearstartpal(context);
	dumpcurrent(context);
	return;
}

static inline void readsetdata(Context & context) {
	context.dx = 1870;
	standardload(context);
	context.data.word(404) = context.ax;
	context.dx = 1935;
	standardload(context);
	context.data.word(408) = context.ax;
	context.dx = 1948;
	standardload(context);
	context.data.word(410) = context.ax;
	context.dx = 1832;
	standardload(context);
	context.data.word(414) = context.ax;
	context.dx = 2234;
	standardload(context);
	context.data.word(454) = context.ax;
	context.dx = 2286;
	standardload(context);
	context.data.word(452) = context.ax;
	context.ax = context.data.word(404);
	context.data.word(268) = context.ax;
	context._cmp(context.data.byte(378), 255);
	if (context.flags.z()) goto novolumeload;
	context.dx = 2299;
	openfile(context);
	context.cx = 2048-256;
	context.ds = context.data.word(353);
	context.dx = 16384;
	readfromfile(context);
	closefile(context);
novolumeload:
	return;
}

static inline void loadpalfromiff(Context & context) {
	context.dx = 2494;
	openfile(context);
	context.cx = 2000;
	context.ds = context.data.word(402);
	context.dx = 0;
	readfromfile(context);
	closefile(context);
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768;
	context.ds = context.data.word(402);
	context.si = 0x30;
	context.cx = 768;
palloop:
	context._lodsb();
	context._shr(context.al, 1);
	context._shr(context.al, 1);
	context._cmp(context.data.byte(73), 1);
	if (!context.flags.z()) goto nought;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto nought;
	context.ah = context.al;
	context._shr(context.ah, 1);
	context._add(context.al, context.ah);
	context._shr(context.ah, 1);
	context._add(context.al, context.ah);
	context._cmp(context.al, 64);
	if (context.flags.c()) goto nought;
	context.al = 63;
nought:
	context._stosb();
	if (--context.cx) goto palloop;
	return;
}

static inline void biblequote(Context & context) {
	mode640x480(context);
	context.dx = 2390;
	showpcx(context);
	fadescreenups(context);
	context.cx = 80;
	hangone(context);
	context.cx = 560;
	hangone(context);
	fadescreendowns(context);
	context.cx = 200;
	hangone(context);
	cancelch0(context);
	return;
}

static inline void runintroseq(Context & context) {
	context.data.byte(103) = 0;
moreintroseq:
	vsync(context);
	context._cmp(context.data.byte(392), 1);
	if (context.flags.z()) goto earlyendrun;
	spriteupdate(context);
	vsync(context);
	context._cmp(context.data.byte(392), 1);
	if (context.flags.z()) goto earlyendrun;
	deleverything(context);
	printsprites(context);
	reelsonscreen(context);
	afterintroroom(context);
	usetimedtext(context);
	vsync(context);
	context._cmp(context.data.byte(392), 1);
	if (context.flags.z()) goto earlyendrun;
	dumpmap(context);
	dumptimedtext(context);
	vsync(context);
	context._cmp(context.data.byte(392), 1);
	if (context.flags.z()) goto earlyendrun;
	context._cmp(context.data.byte(103), 1);
	if (!context.flags.z()) goto moreintroseq;
	return;
earlyendrun:
	context.ax = context.pop();
	context.ax = context.pop();
	getridoftemptext(context);
	clearbeforeload(context);
	return;
}

static inline void intro(Context & context) {
	context.dx = 2260;
	loadtemptext(context);
	loadpalfromiff(context);
	setmode(context);
	context.data.byte(188) = 50;
	clearpalette(context);
	loadintroroom(context);
	context.data.byte(386) = 7;
	context.data.byte(388) = -1;
	context.data.byte(387) = 0;
	context.al = 12;
	context.ah = 255;
	playchannel0(context);
	fadescreenups(context);
	runintroseq(context);
	clearbeforeload(context);
	context.data.byte(188) = 52;
	loadintroroom(context);
	runintroseq(context);
	clearbeforeload(context);
	context.data.byte(188) = 53;
	loadintroroom(context);
	runintroseq(context);
	clearbeforeload(context);
	allpalette(context);
	context.data.byte(188) = 54;
	loadintroroom(context);
	runintroseq(context);
	getridoftemptext(context);
	clearbeforeload(context);
	return;
}

static inline void titles(Context & context) {
	clearpalette(context);
	biblequote(context);
	intro(context);
	return;
}

static inline void dumpcurrent(Context & context) {
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3);
	context.ds = context.data.word(412);
	vsync(context);
	context.al = 0;
	context.cx = 128;
	showgroup(context);
	vsync(context);
	context.al = 128;
	context.cx = 128;
	showgroup(context);
	return;
}

static inline void allpalette(Context & context) {
	context.es = context.data.word(412);
	context.ds = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768;
	context.cx = 768/2;
	while(--context.cx) 	context._movsw();
 	dumpcurrent(context);
	return;
}

static inline void realcredits(Context & context) {
	context.data.byte(531) = 33;
	loadroomssample(context);
	context.data.byte(386) = 0;
	mode640x480(context);
	context.cx = 35;
	hangon(context);
	context.dx = 2403;
	showpcx(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.cx = 2;
	hangone(context);
	allpalette(context);
	context.cx = 80;
	hangone(context);
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	context.dx = 2416;
	showpcx(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.cx = 2;
	hangone(context);
	allpalette(context);
	context.cx = 80;
	hangone(context);
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	context.dx = 2429;
	showpcx(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.cx = 2;
	hangone(context);
	allpalette(context);
	context.cx = 80;
	hangone(context);
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	context.dx = 2442;
	showpcx(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.cx = 2;
	hangone(context);
	allpalette(context);
	context.cx = 80;
	hangone(context);
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	context.dx = 2455;
	showpcx(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.cx = 2;
	hangone(context);
	allpalette(context);
	context.cx = 80;
	hangone(context);
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	context.dx = 2468;
	showpcx(context);
	fadescreenups(context);
	context.cx = 60;
	hangone(context);
	context.al = 13;
	context.ah = 0;
	playchannel0(context);
	context.cx = 350;
	hangone(context);
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	return;
}

static inline void credits(Context & context) {
	clearpalette(context);
	realcredits(context);
	return;
}

static inline void cls(Context & context) {
	context.ax = 0x0a000;
	context.es = context.ax;
	context.di = 0;
	context.cx = 0x7fff;
	context.ax = 0;
	while(--context.cx) 	context._stosw();
	return;
}

static inline void loadsavebox(Context & context) {
	context.dx = 1974;
	loadintotemp(context);
	return;
}

static inline void showopbox(Context & context) {
	context.ds = context.data.word(458);
	context.di = 60;
	context.bx = 52;
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 60;
	context.bx = 52+55;
	context.al = 4;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void showdecisions(Context & context) {
	createpanel2(context);
	showopbox(context);
	context.ds = context.data.word(458);
	context.di = 60+17;
	context.bx = 52+13;
	context.al = 6;
	context.ah = 0;
	showframe(context);
	undertextline(context);
	return;
}

static inline void decide(Context & context) {
	setmode(context);
	loadpalfromiff(context);
	clearpalette(context);
	context.data.byte(234) = 0;
	context.data.word(21) = 0;
	context.data.byte(231) = 0;
	context.data.word(79) = 70;
	context.data.word(81) = 182-8;
	context.data.byte(82) = 181;
	context.data.byte(131) = 1;
	loadsavebox(context);
	showdecisions(context);
	worktoscreen(context);
	fadescreenup(context);
	context.data.byte(103) = 0;
waitdecide:
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	delpointer(context);
	context.bx = 4884;
	checkcoords(context);
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto waitdecide;
	context._cmp(context.data.byte(103), 4);
	if (context.flags.z()) goto hasloadedroom;
	getridoftemp(context);
hasloadedroom:
	context.data.word(79) = 13;
	context.data.word(81) = 182;
	context.data.byte(82) = 240;
	return;
}

static inline void clearchanges(Context & context) {
	context.es = context.data.word(412);
	context.cx = 250*2;
	context.ax = 0x0ffff;
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80);
	while(--context.cx) 	context._stosw();
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64);
	context.es = context.cs;
	context.di = 537;
	context.cx = 991-context.data.byte(537);
	while(--context.cx) 	context._movsb();
 	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30)+(6*64)+991-context.data.byte(537);
	context.es = context.cs;
	context.di = 1;
	context.cx = 68-context.data.byte(1);
	while(--context.cx) 	context._movsb();
 	context.data.byte(10) = 0;
	context.data.word(12) = 0;
	context.data.word(14) = 0;
	context.es = context.data.word(398);
	context.cx = 0+2080+30000+(16*114)+((114+2)*2)+18000/2;
	context.ax = 0x0ffff;
	context.di = 0;
	while(--context.cx) 	context._stosw();
	context.es = context.cs;
	context.di = 7798;
	context.al = 1;
	context._stosb();
	context._stosb();
	context.al = 0;
	context._stosb();
	context.al = 1;
	context._stosb();
	context.ax = 0;
	context.cx = 6;
	while(--context.cx) 	context._stosw();
	return;
}

static inline void fillspace(Context & context) {
	context.push(context.es);
	context.push(context.ds);
	context.push(context.dx);
	context.push(context.di);
	context.push(context.bx);
	context.di = context.dx;
	context.es = context.ds;
	while(--context.cx) 	context._stosb();
	context.bx = context.pop();
	context.di = context.pop();
	context.dx = context.pop();
	context.ds = context.pop();
	context.es = context.pop();
	return;
}

static inline void sortoutmap(Context & context) {
	context.push(context.es);
	context.push(context.di);
	context.ds = context.data.word(400);
	context.si = 0;
	context.es = context.data.word(418);
	context.di = 0;
	context.cx = 60;
blimey:
	context.push(context.cx);
	context.push(context.si);
	context.cx = 66;
	while(--context.cx) 	context._movsb();
 	context.si = context.pop();
	context.cx = context.pop();
	context._add(context.si, 132);
	if (--context.cx) goto blimey;
	context.di = context.pop();
	context.es = context.pop();
	return;
}

static inline void deletetaken(Context & context) {
	context.es = context.data.word(426);
	context.ah = context.data.byte(184);
	context.ds = context.data.word(398);
	context.si = 0+2080+30000;
	context.cx = 114;
takenloop:
	context.al = context.data.byte(context.si+11);
	context._cmp(context.al, context.ah);
	if (!context.flags.z()) goto notinhere;
	context.bl = context.data.byte(context.si+1);
	context.bh = 0;
	context._add(context.bx, context.bx);
	context._add(context.bx, context.bx);
	context._add(context.bx, context.bx);
	context._add(context.bx, context.bx);
	context.data.byte(context.bx+2) = 254;
notinhere:
	context._add(context.si, 16);
	if (--context.cx) goto takenloop;
	return;
}

static inline void dochange(Context & context) {
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto object;
	context._cmp(context.ch, 1);
	if (context.flags.z()) goto freeobject;
path:
	context.push(context.cx);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context.push(context.ax);
	context.al = context.ch;
	context._sub(context.al, 100);
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context.bx = context.pop();
	context._add(context.bx, context.ax);
	context._add(context.bx, 0);
	context.es = context.data.word(450);
	context.cx = context.pop();
	context.data.byte(context.bx+6) = context.cl;
nopath:
	return;
object:
	context.push(context.cx);
	getsetad(context);
	context.cx = context.pop();
	context.data.byte(context.bx+58) = context.cl;
	return;
freeobject:
	context.push(context.cx);
	getfreead(context);
	context.cx = context.pop();
	context._cmp(context.data.byte(context.bx+2), 255);
	if (!context.flags.z()) goto beenpickedup;
	context.data.byte(context.bx+2) = context.cl;
beenpickedup:
	return;
}

static inline void setallchanges(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80);
setallloop:
	context.ax = context.data.word(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto endsetloop;
	context.cx = context.data.word(context.bx+2);
	context._add(context.bx, 4);
	context._cmp(context.ah, context.data.byte(184));
	if (!context.flags.z()) goto setallloop;
	context.push(context.es);
	context.push(context.bx);
	dochange(context);
	context.bx = context.pop();
	context.es = context.pop();
	goto setallloop;
endsetloop:
	return;
}

static inline void autoappear(Context & context) {
	context._cmp(context.data.byte(9), 32);
	if (!context.flags.z()) goto notinalley;
	context.al = 5;
	resetlocation(context);
	context.al = 10;
	setlocation(context);
	context.data.byte(183) = 10;
	return;
notinalley:
	context._cmp(context.data.byte(184), 24);
	if (!context.flags.z()) goto notinedens;
	context._cmp(context.data.byte(42), 1);
	if (!context.flags.z()) goto edenspart2;
	context._add(context.data.byte(42), 1);
	context.al = 44;
	placesetobject(context);
	context.al = 18;
	placesetobject(context);
	context.al = 93;
	placesetobject(context);
	context.al = 92;
	removesetobject(context);
	context.al = 55;
	removesetobject(context);
	context.al = 75;
	removesetobject(context);
	context.al = 84;
	removesetobject(context);
	context.al = 85;
	removesetobject(context);
	return;
edenspart2:
	context._cmp(context.data.byte(43), 1);
	if (!context.flags.z()) goto notedens2;
	context.al = 44;
	removesetobject(context);
	context.al = 93;
	removesetobject(context);
	context.al = 55;
	placesetobject(context);
	context._add(context.data.byte(43), 1);
notedens2:
	return;
notinedens:
	context._cmp(context.data.byte(184), 25);
	if (!context.flags.z()) goto notonsartroof;
	context.data.byte(34) = 3;
	context.al = 6;
	resetlocation(context);
	context.al = 11;
	setlocation(context);
	context.data.byte(183) = 11;
	return;
notonsartroof:
	context._cmp(context.data.byte(184), 2);
	if (!context.flags.z()) goto notinlouiss;
	context._cmp(context.data.byte(41), 0);
	if (context.flags.z()) goto notinlouiss;
	context.al = 23;
	placesetobject(context);
notinlouiss:
	return;
}

static inline void startloading(Context & context) {
	context.data.byte(64) = 0;
	context.al = context.data.byte(context.bx+13);
	context.data.byte(531) = context.al;
	context.al = context.data.byte(context.bx+15);
	context.data.byte(148) = context.al;
	context.al = context.data.byte(context.bx+16);
	context.data.byte(149) = context.al;
	context.al = context.data.byte(context.bx+20);
	context.data.byte(35) = context.al;
	context.al = context.data.byte(context.bx+21);
	context.data.byte(475) = context.al;
	context.data.byte(478) = context.al;
	context.data.byte(477) = context.al;
	context.al = context.data.byte(context.bx+22);
	context.data.byte(133) = context.al;
	context.data.byte(135) = context.al;
	context.al = context.data.byte(context.bx+23);
	context.data.byte(39) = context.al;
	context.al = context.data.byte(context.bx+24);
	context.data.byte(36) = context.al;
	context.al = context.data.byte(context.bx+25);
	context.data.byte(38) = context.al;
	context.data.byte(65) = -1;
	context.al = context.data.byte(context.bx+27);
	context.push(context.ax);
	context.al = context.data.byte(context.bx+31);
	context.ah = context.data.byte(184);
	context.data.byte(184) = context.al;
	context.dx = context.bx;
	openfile(context);
	readheader(context);
	allocateload(context);
	context.ds = context.ax;
	context.data.word(416) = context.ax;
	context.dx = 0;
	loadseg(context);
	context.ds = context.data.word(400);
	context.dx = 0;
	context.cx = 132*66;
	context.al = 0;
	fillspace(context);
	loadseg(context);
	sortoutmap(context);
	allocateload(context);
	context.data.word(444) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	context.ds = context.data.word(428);
	context.dx = 0;
	context.cx = 64*128;
	context.al = 255;
	fillspace(context);
	loadseg(context);
	allocateload(context);
	context.data.word(430) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(432) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(434) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(450) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(448) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(440) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(442) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(436) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(446) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	context.ds = context.data.word(426);
	context.dx = 0;
	context.cx = 16*80;
	context.al = 255;
	fillspace(context);
	loadseg(context);
	allocateload(context);
	context.data.word(438) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	closefile(context);
	findroominloc(context);
	deletetaken(context);
	setallchanges(context);
	autoappear(context);
	context.al = context.data.byte(188);
	getroomdata(context);
	context.data.byte(65) = -1;
	context.data.byte(56) = 0;
	context.data.word(98) = 160;
	context.data.byte(188) = 255;
	context.data.byte(492) = 254;
	context.ax = context.pop();
	context._cmp(context.al, 255);
	if (context.flags.z()) goto dontwalkin;
	context.data.byte(475) = context.al;
	context.push(context.bx);
	autosetwalk(context);
	context.bx = context.pop();
dontwalkin:
	findxyfrompath(context);
	return;
}

static inline void loadroom(Context & context) {
	context.data.byte(74) = 1;
	context.data.word(328) = 0;
	context.data.word(138) = 0;
	context.data.word(121) = 104;
	context.data.word(123) = 38;
	context.data.word(79) = 13;
	context.data.word(81) = 182;
	context.data.byte(82) = 240;
	context.al = context.data.byte(188);
	context.data.byte(9) = context.al;
	getroomdata(context);
	startloading(context);
	loadroomssample(context);
	switchryanon(context);
	drawflags(context);
	getdimension(context);
	return;
}

static inline void clearsprites(Context & context) {
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768;
	context.al = 255;
	context.cx = 32*16;
	while(--context.cx) 	context._stosb();
	return;
}

static inline void initman(Context & context) {
	context.al = context.data.byte(151);
	context.ah = context.data.byte(152);
	context.si = context.ax;
	context.cx = 49464;
	context.dx = context.data.word(414);
	context.di = 0;
	makesprite(context);
	context.data.byte(context.bx+23) = 4;
	context.data.byte(context.bx+22) = 0;
	context.data.byte(context.bx+29) = 0;
	return;
}

static inline void entrytexts(Context & context) {
	context._cmp(context.data.byte(9), 21);
	if (!context.flags.z()) goto notloc15;
	context.al = 28;
	context.cx = 60;
	context.dx = 11;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	return;
notloc15:
	context._cmp(context.data.byte(9), 30);
	if (!context.flags.z()) goto notloc43;
	context.al = 27;
	context.cx = 60;
	context.dx = 11;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	return;
notloc43:
	context._cmp(context.data.byte(9), 23);
	if (!context.flags.z()) goto notloc23;
	context.al = 29;
	context.cx = 60;
	context.dx = 11;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	return;
notloc23:
	context._cmp(context.data.byte(9), 31);
	if (!context.flags.z()) goto notloc44;
	context.al = 30;
	context.cx = 60;
	context.dx = 11;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	return;
notloc44:
	context._cmp(context.data.byte(9), 20);
	if (!context.flags.z()) goto notsarters2;
	context.al = 31;
	context.cx = 60;
	context.dx = 11;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	return;
notsarters2:
	context._cmp(context.data.byte(9), 24);
	if (!context.flags.z()) goto notedenlob;
	context.al = 32;
	context.cx = 60;
	context.dx = 3;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	return;
notedenlob:
	context._cmp(context.data.byte(9), 34);
	if (!context.flags.z()) goto noteden2;
	context.al = 33;
	context.cx = 60;
	context.dx = 3;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	return;
noteden2:
	return;
}

static inline void purgealocation(Context & context) {
	context.push(context.ax);
	context.es = context.data.word(398);
	context.di = 0+2080+30000;
	context.bx = context.pop();
	context.cx = 0;
purgeloc:
	context._cmp(context.bl, context.data.byte(context.di+0));
	if (!context.flags.z()) goto dontpurge;
	context._cmp(context.data.byte(context.di+2), 0);
	if (!context.flags.z()) goto dontpurge;
	context.push(context.di);
	context.push(context.es);
	context.push(context.bx);
	context.push(context.cx);
	deleteexobject(context);
	context.cx = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	context.di = context.pop();
dontpurge:
	context._add(context.di, 16);
	context._add(context.cx, 1);
	context._cmp(context.cx, 114);
	if (!context.flags.z()) goto purgeloc;
	return;
}

static inline void resetlocation(Context & context) {
	context.push(context.ax);
	context._cmp(context.al, 5);
	if (!context.flags.z()) goto notdelhotel;
	purgealocation(context);
	context.al = 21;
	purgealocation(context);
	context.al = 22;
	purgealocation(context);
	context.al = 27;
	purgealocation(context);
	goto clearedlocations;
notdelhotel:
	context._cmp(context.al, 8);
	if (!context.flags.z()) goto notdeltvstud;
	purgealocation(context);
	context.al = 28;
	purgealocation(context);
	goto clearedlocations;
notdeltvstud:
	context._cmp(context.al, 6);
	if (!context.flags.z()) goto notdelsarters;
	purgealocation(context);
	context.al = 20;
	purgealocation(context);
	context.al = 25;
	purgealocation(context);
	goto clearedlocations;
notdelsarters:
	context._cmp(context.al, 13);
	if (!context.flags.z()) goto notdelboathouse;
	purgealocation(context);
	context.al = 29;
	purgealocation(context);
	goto clearedlocations;
notdelboathouse:
clearedlocations:
	context.ax = context.pop();
	context.ah = 0;
	context.bx = context.ax;
	context.dx = context.ds;
	context.es = context.dx;
	context._add(context.bx, 7798);
	context.data.byte(context.bx) = 0;
	return;
}

static inline void checkifpathison(Context & context) {
	context.push(context.ax);
	getroomspaths(context);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.bx+6);
	context._cmp(context.al, 255);
	return;
}

static inline void turnpathon(Context & context) {
	context.push(context.ax);
	context.push(context.ax);
	context.cl = 255;
	context.ch = context.data.byte(185);
	context._add(context.ch, 100);
	findormake(context);
	context.ax = context.pop();
	getroomspaths(context);
	context.ax = context.pop();
	context._cmp(context.al, 255);
	if (context.flags.z()) goto nopathon;
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = 255;
	context.data.byte(context.bx+6) = context.al;
nopathon:
	return;
}

static inline void turnanypathon(Context & context) {
	context.push(context.ax);
	context.push(context.ax);
	context.cl = 255;
	context.ch = context.ah;
	context._add(context.ch, 100);
	findormake(context);
	context.ax = context.pop();
	context.al = context.ah;
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context.es = context.data.word(450);
	context.bx = 0;
	context._add(context.bx, context.ax);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = 255;
	context.data.byte(context.bx+6) = context.al;
	return;
}

static inline void entryanims(Context & context) {
	context.data.word(23) = -1;
	context.data.byte(32) = -1;
	context._cmp(context.data.byte(9), 33);
	if (!context.flags.z()) goto notinthebeach;
	switchryanoff(context);
	context.data.word(21) = 76*2;
	context.data.word(23) = 0;
	context.data.word(25) = 76;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	return;
notinthebeach:
	context._cmp(context.data.byte(9), 44);
	if (!context.flags.z()) goto notsparkys;
	context.al = 8;
	resetlocation(context);
	context.data.word(21) = 50*2;
	context.data.word(23) = 247;
	context.data.word(25) = 297;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	switchryanoff(context);
	return;
notsparkys:
	context._cmp(context.data.byte(9), 22);
	if (!context.flags.z()) goto notinthelift;
	context.data.word(21) = 31*2;
	context.data.word(23) = 0;
	context.data.word(25) = 30;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	switchryanoff(context);
	return;
notinthelift:
	context._cmp(context.data.byte(9), 26);
	if (!context.flags.z()) goto notunderchurch;
	context.data.byte(292) = 2;
	context.data.byte(295) = 1;
	return;
notunderchurch:
	context._cmp(context.data.byte(9), 45);
	if (!context.flags.z()) goto notenterdream;
	context.data.byte(54) = 0;
	context.data.word(21) = 296;
	context.data.word(23) = 45;
	context.data.word(25) = 198;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	switchryanoff(context);
	return;
notenterdream:
	context._cmp(context.data.byte(184), 46);
	if (!context.flags.z()) goto notcrystal;
	context._cmp(context.data.byte(43), 1);
	if (!context.flags.z()) goto notcrystal;
	context.al = 0;
	removefreeobject(context);
	return;
notcrystal:
	context._cmp(context.data.byte(9), 9);
	if (!context.flags.z()) goto nottopchurch;
	context.al = 2;
	checkifpathison(context);
	if (context.flags.z()) goto nottopchurch;
	context._cmp(context.data.byte(44), 0);
	if (context.flags.z()) goto nottopchurch;
	context.al = 3;
	checkifpathison(context);
	if (!context.flags.z()) goto makedoorsopen;
	context.al = 2;
	turnpathon(context);
makedoorsopen:
	context.al = 4;
	removesetobject(context);
	context.al = 5;
	placesetobject(context);
	return;
nottopchurch:
	context._cmp(context.data.byte(9), 47);
	if (!context.flags.z()) goto notdreamcentre;
	context.al = 4;
	placesetobject(context);
	context.al = 5;
	placesetobject(context);
	return;
notdreamcentre:
	context._cmp(context.data.byte(9), 38);
	if (!context.flags.z()) goto notcarpark;
	context.data.word(21) = 57*2;
	context.data.word(23) = 4;
	context.data.word(25) = 57;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	switchryanoff(context);
	return;
notcarpark:
	context._cmp(context.data.byte(9), 32);
	if (!context.flags.z()) goto notalley;
	context.data.word(21) = 66*2;
	context.data.word(23) = 0;
	context.data.word(25) = 66;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	switchryanoff(context);
	return;
notalley:
	context._cmp(context.data.byte(9), 24);
	if (!context.flags.z()) goto notedensagain;
	context.al = 2;
	context.ah = context.data.byte(185);
	context._sub(context.ah, 1);
	turnanypathon(context);
notedensagain:
	return;
}

static inline void deleteexframe(Context & context) {
	context.di = 0;
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.di, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.di, context.ax);
	context.al = context.data.byte(context.di);
	context.ah = 0;
	context.cl = context.data.byte(context.di+1);
	context.ch = 0;
	context._mul(context.cx);
	context.si = context.data.word(context.di+2);
	context.push(context.si);
	context._add(context.si, 0+2080);
	context.cx = 30000;
	context._sub(context.cx, context.data.word(context.di+2));
	context.di = context.si;
	context._add(context.si, context.ax);
	context.push(context.ax);
	context.ds = context.es;
	while(--context.cx) 	context._movsb();
 	context.bx = context.pop();
	context._sub(context.data.word(12), context.bx);
	context.si = context.pop();
	context.cx = 114*3;
	context.di = 0;
shuffleadsdown:
	context.ax = context.data.word(context.di+2);
	context._cmp(context.ax, context.si);
	if (context.flags.c()) goto beforethisone;
	context._sub(context.ax, context.bx);
beforethisone:
	context.data.word(context.di+2) = context.ax;
	context._add(context.di, 6);
	if (--context.cx) goto shuffleadsdown;
	return;
}

static inline void deleteextext(Context & context) {
	context.di = 0+2080+30000+(16*114);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.di, context.ax);
	context.ax = context.data.word(context.di);
	context.si = context.ax;
	context.di = context.ax;
	context._add(context.si, 0+2080+30000+(16*114)+((114+2)*2));
	context._add(context.di, 0+2080+30000+(16*114)+((114+2)*2));
	context.ax = 0;
findlenextext:
	context.cl = context.data.byte(context.si);
	context._add(context.ax, 1);
	context._add(context.si, 1);
	context._cmp(context.cl, 0);
	if (!context.flags.z()) goto findlenextext;
	context.cx = 18000;
	context.bx = context.si;
	context._sub(context.bx, 0+2080+30000+(16*114)+((114+2)*2));
	context.push(context.bx);
	context.push(context.ax);
	context._sub(context.cx, context.bx);
	while(--context.cx) 	context._movsb();
 	context.bx = context.pop();
	context._sub(context.data.word(14), context.bx);
	context.si = context.pop();
	context.cx = 114;
	context.di = 0+2080+30000+(16*114);
shuffletextads:
	context.ax = context.data.word(context.di);
	context._cmp(context.ax, context.si);
	if (context.flags.c()) goto beforethistext;
	context._sub(context.ax, context.bx);
beforethistext:
	context.data.word(context.di) = context.ax;
	context._add(context.di, 2);
	if (--context.cx) goto shuffletextads;
	return;
}

static inline void deleteexobject(Context & context) {
	context.push(context.cx);
	context.push(context.cx);
	context.push(context.cx);
	context.push(context.cx);
	context.al = 255;
	context.cx = 16;
	while(--context.cx) 	context._stosb();
	context.ax = context.pop();
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	deleteexframe(context);
	context.ax = context.pop();
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	context._add(context.al, 1);
	deleteexframe(context);
	context.ax = context.pop();
	deleteextext(context);
	context.bx = context.pop();
	context.bh = context.bl;
	context.bl = 4;
	context.di = 0+2080+30000;
	context.cx = 0;
deleteconts:
	context._cmp(context.data.word(context.di+2), context.bx);
	if (!context.flags.z()) goto notinsideex;
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.di);
	deleteexobject(context);
	context.di = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
notinsideex:
	context._add(context.di, 16);
	context._add(context.cx, 1);
	context._cmp(context.cx, 114);
	if (!context.flags.z()) goto deleteconts;
	return;
}

static inline void purgeanitem(Context & context) {
	context.es = context.data.word(398);
	context.di = 0+2080+30000;
	context.bl = context.data.byte(184);
	context.cx = 0;
lookforpurge:
	context.al = context.data.byte(context.di+2);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto cantpurge;
	context._cmp(context.data.byte(context.di+12), 2);
	if (context.flags.z()) goto iscup;
	context._cmp(context.data.byte(context.di+12), 255);
	if (!context.flags.z()) goto cantpurge;
iscup:
	context._cmp(context.data.byte(context.di+11), context.bl);
	if (context.flags.z()) goto cantpurge;
	deleteexobject(context);
	return;
cantpurge:
	context._add(context.di, 16);
	context._add(context.cx, 1);
	context._cmp(context.cx, 114);
	if (!context.flags.z()) goto lookforpurge;
	context.di = 0+2080+30000;
	context.bl = context.data.byte(184);
	context.cx = 0;
lookforpurge2:
	context.al = context.data.byte(context.di+2);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto cantpurge2;
	context._cmp(context.data.byte(context.di+12), 255);
	if (!context.flags.z()) goto cantpurge2;
	deleteexobject(context);
	return;
cantpurge2:
	context._add(context.di, 16);
	context._add(context.cx, 1);
	context._cmp(context.cx, 114);
	if (!context.flags.z()) goto lookforpurge2;
	return;
}

static inline void emergencypurge(Context & context) {
checkpurgeagain:
	context.ax = context.data.word(12);
	context._add(context.ax, 4000);
	context._cmp(context.ax, 30000);
	if (context.flags.c()) goto notnearframeend;
	purgeanitem(context);
	goto checkpurgeagain;
notnearframeend:
	context.ax = context.data.word(14);
	context._add(context.ax, 400);
	context._cmp(context.ax, 18000);
	if (context.flags.c()) goto notneartextend;
	purgeanitem(context);
	goto checkpurgeagain;
notneartextend:
	return;
}

static inline void getexpos(Context & context) {
	context.es = context.data.word(398);
	context.al = 0;
	context.di = 0+2080+30000;
tryanotherex:
	context._cmp(context.data.byte(context.di+2), 255);
	if (context.flags.z()) goto foundnewex;
	context._add(context.di, 16);
	context._add(context.al, 1);
	context._cmp(context.al, 114);
	if (!context.flags.z()) goto tryanotherex;
foundnewex:
	context.data.byte(10) = context.al;
	return;
}

static inline void transfermap(Context & context) {
	context.di = context.data.word(12);
	context.push(context.di);
	context.al = context.data.byte(10);
	context.ah = 0;
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.bx);
	context.cx = 6;
	context._mul(context.cx);
	context.es = context.data.word(398);
	context.bx = 0;
	context._add(context.bx, context.ax);
	context._add(context.di, 0+2080);
	context.push(context.bx);
	context.al = context.data.byte(90);
	context.ah = 0;
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.bx);
	context.cx = 6;
	context._mul(context.cx);
	context.ds = context.data.word(446);
	context.bx = 0;
	context._add(context.bx, context.ax);
	context.si = 0+2080;
	context.al = context.data.byte(context.bx);
	context.ah = 0;
	context.cl = context.data.byte(context.bx+1);
	context.ch = 0;
	context._add(context.si, context.data.word(context.bx+2));
	context.dx = context.data.word(context.bx+4);
	context.bx = context.pop();
	context.data.byte(context.bx+0) = context.al;
	context.data.byte(context.bx+1) = context.cl;
	context.data.word(context.bx+4) = context.dx;
	context._mul(context.cx);
	context.cx = context.ax;
	context.push(context.cx);
	while(--context.cx) 	context._movsb();
 	context.cx = context.pop();
	context.ax = context.pop();
	context.data.word(context.bx+2) = context.ax;
	context._add(context.data.word(12), context.cx);
	return;
}

static inline void transferinv(Context & context) {
	context.di = context.data.word(12);
	context.push(context.di);
	context.al = context.data.byte(10);
	context.ah = 0;
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.bx);
	context._add(context.ax, 1);
	context.cx = 6;
	context._mul(context.cx);
	context.es = context.data.word(398);
	context.bx = 0;
	context._add(context.bx, context.ax);
	context._add(context.di, 0+2080);
	context.push(context.bx);
	context.al = context.data.byte(90);
	context.ah = 0;
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.bx);
	context._add(context.ax, 1);
	context.cx = 6;
	context._mul(context.cx);
	context.ds = context.data.word(446);
	context.bx = 0;
	context._add(context.bx, context.ax);
	context.si = 0+2080;
	context.al = context.data.byte(context.bx);
	context.ah = 0;
	context.cl = context.data.byte(context.bx+1);
	context.ch = 0;
	context._add(context.si, context.data.word(context.bx+2));
	context.dx = context.data.word(context.bx+4);
	context.bx = context.pop();
	context.data.byte(context.bx+0) = context.al;
	context.data.byte(context.bx+1) = context.cl;
	context.data.word(context.bx+4) = context.dx;
	context._mul(context.cx);
	context.cx = context.ax;
	context.push(context.cx);
	while(--context.cx) 	context._movsb();
 	context.cx = context.pop();
	context.ax = context.pop();
	context.data.word(context.bx+2) = context.ax;
	context._add(context.data.word(12), context.cx);
	return;
}

static inline void transfertext(Context & context) {
	context.es = context.data.word(398);
	context.al = context.data.byte(10);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = 0+2080+30000+(16*114);
	context._add(context.bx, context.ax);
	context.di = context.data.word(14);
	context.data.word(context.bx) = context.di;
	context._add(context.di, 0+2080+30000+(16*114)+((114+2)*2));
	context.al = context.data.byte(90);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.ds = context.data.word(438);
	context.bx = 0;
	context._add(context.bx, context.ax);
	context.si = 0+(82*2);
	context.ax = context.data.word(context.bx);
	context._add(context.si, context.ax);
moretext:
	context._lodsb();
	context._stosb();
	context._add(context.data.word(14), 1);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto moretext;
	return;
}

static inline void transfercontoex(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context.push(context.dx);
	context.push(context.es);
	context.push(context.bx);
	getexpos(context);
	context.si = context.pop();
	context.ds = context.pop();
	context.push(context.di);
	context.cx = 8;
	while(--context.cx) 	context._movsw();
 	context.di = context.pop();
	context.dx = context.pop();
	context.al = context.data.byte(184);
	context.data.byte(context.di) = context.al;
	context.data.byte(context.di+11) = context.al;
	context.al = context.data.byte(90);
	context.data.byte(context.di+1) = context.al;
	context.data.byte(context.di+3) = context.dl;
	context.data.byte(context.di+2) = 4;
	transfermap(context);
	transferinv(context);
	transfertext(context);
	context.si = context.pop();
	context.ds = context.pop();
	context.data.byte(context.si+2) = 255;
	return;
}

static inline void pickupconts(Context & context) {
	context.al = context.data.byte(context.si+7);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto notopenable;
	context.al = context.data.byte(89);
	context.ah = context.data.byte(102);
	context.dl = context.data.byte(10);
	context.es = context.data.word(426);
	context.bx = 0;
	context.cx = 0;
pickupcontloop:
	context.push(context.cx);
	context.push(context.es);
	context.push(context.bx);
	context.push(context.dx);
	context.push(context.ax);
	context._cmp(context.data.byte(context.bx+2), context.ah);
	if (!context.flags.z()) goto notinsidethis;
	context._cmp(context.data.byte(context.bx+3), context.al);
	if (!context.flags.z()) goto notinsidethis;
	context.data.byte(90) = context.cl;
	transfercontoex(context);
notinsidethis:
	context.ax = context.pop();
	context.dx = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 16);
	context._add(context.cx, 1);
	context._cmp(context.cx, 80);
	if (!context.flags.z()) goto pickupcontloop;
notopenable:
	return;
}

static inline void transfertoex(Context & context) {
	emergencypurge(context);
	getexpos(context);
	context.al = context.data.byte(10);
	context.push(context.ax);
	context.push(context.di);
	context.al = context.data.byte(89);
	context.ah = 0;
	context.bx = 16;
	context._mul(context.bx);
	context.ds = context.data.word(426);
	context.si = context.ax;
	context.cx = 8;
	while(--context.cx) 	context._movsw();
 	context.di = context.pop();
	context.al = context.data.byte(184);
	context.data.byte(context.di) = context.al;
	context.data.byte(context.di+11) = context.al;
	context.al = context.data.byte(89);
	context.data.byte(context.di+1) = context.al;
	context.data.byte(context.di+2) = 4;
	context.data.byte(context.di+3) = 255;
	context.al = context.data.byte(107);
	context.data.byte(context.di+4) = context.al;
	context.al = context.data.byte(89);
	context.data.byte(90) = context.al;
	transfermap(context);
	transferinv(context);
	transfertext(context);
	context.al = context.data.byte(89);
	context.ah = 0;
	context.bx = 16;
	context._mul(context.bx);
	context.ds = context.data.word(426);
	context.si = context.ax;
	context.data.byte(context.si+2) = 254;
	pickupconts(context);
	context.ax = context.pop();
	return;
}

static inline void pickupob(Context & context) {
	context.data.byte(107) = context.ah;
	context.data.byte(102) = 2;
	context.data.byte(89) = context.al;
	context.data.byte(99) = context.al;
	getanyad(context);
	transfertoex(context);
	return;
}

static inline void initialinv(Context & context) {
	context._cmp(context.data.byte(184), 24);
	if (context.flags.z()) goto isedens;
	return;
isedens:
	context.al = 11;
	context.ah = 5;
	pickupob(context);
	context.al = 12;
	context.ah = 6;
	pickupob(context);
	context.al = 13;
	context.ah = 7;
	pickupob(context);
	context.al = 14;
	context.ah = 8;
	pickupob(context);
	context.al = 18;
	context.al = 18;
	context.ah = 0;
	pickupob(context);
	context.al = 19;
	context.ah = 1;
	pickupob(context);
	context.al = 20;
	context.ah = 9;
	pickupob(context);
	context.al = 16;
	context.ah = 2;
	pickupob(context);
	context.data.byte(32) = 1;
	context.data.word(29) = 0;
	context.data.word(31) = 6;
	context.data.byte(27) = 1;
	context.data.byte(26) = 1;
	switchryanoff(context);
	return;
}

static inline void startup1(Context & context) {
	clearpalette(context);
	context.data.byte(61) = 0;
	context.data.byte(142) = '0';
	context.data.byte(105) = 0;
	createpanel(context);
	context.data.byte(62) = 1;
	drawfloor(context);
	showicon(context);
	getunderzoom(context);
	spriteupdate(context);
	printsprites(context);
	undertextline(context);
	reelsonscreen(context);
	atmospheres(context);
	worktoscreen(context);
	fadescreenup(context);
	return;
}

static inline void clearreels(Context & context) {
	context.es = context.data.word(430);
	deallocatemem(context);
	context.es = context.data.word(432);
	deallocatemem(context);
	context.es = context.data.word(434);
	deallocatemem(context);
	return;
}

static inline void clearrest(Context & context) {
	context.es = context.data.word(418);
	context.cx = 66*60/2;
	context.ax = 0;
	context.di = 0;
	while(--context.cx) 	context._stosw();
	context.es = context.data.word(416);
	deallocatemem(context);
	context.es = context.data.word(444);
	deallocatemem(context);
	context.es = context.data.word(450);
	deallocatemem(context);
	context.es = context.data.word(448);
	deallocatemem(context);
	context.es = context.data.word(440);
	deallocatemem(context);
	context.es = context.data.word(442);
	deallocatemem(context);
	context.es = context.data.word(436);
	deallocatemem(context);
	context.es = context.data.word(446);
	deallocatemem(context);
	context.es = context.data.word(438);
	deallocatemem(context);
	return;
}

static inline void clearbeforeload(Context & context) {
	context._cmp(context.data.byte(74), 1);
	if (!context.flags.z()) goto noclear;
	clearreels(context);
	clearrest(context);
	context.data.byte(74) = 0;
noclear:
	return;
}

static inline void startup(Context & context) {
	context.data.byte(142) = 0;
	context.data.byte(105) = 0;
	createpanel(context);
	context.data.byte(62) = 1;
	drawfloor(context);
	showicon(context);
	getunderzoom(context);
	spriteupdate(context);
	printsprites(context);
	undertextline(context);
	reelsonscreen(context);
	atmospheres(context);
	return;
}

static inline void worktoscreenm(Context & context) {
	animpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void readcitypic(Context & context) {
	context.dx = 2013;
	loadintotemp(context);
	return;
}

static inline void showcity(Context & context) {
	clearwork(context);
	context.ds = context.data.word(458);
	context.di = 57;
	context.bx = 32;
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(458);
	context.di = 120+57;
	context.bx = 32;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void loadintotemp2(Context & context) {
	context.ds = context.cs;
	standardload(context);
	context.data.word(460) = context.ax;
	return;
}

static inline void loadintotemp3(Context & context) {
	context.ds = context.cs;
	standardload(context);
	context.data.word(462) = context.ax;
	return;
}

static inline void readdesticon(Context & context) {
	context.dx = 2026;
	loadintotemp(context);
	context.dx = 2039;
	loadintotemp2(context);
	context.dx = 1974;
	loadintotemp3(context);
	return;
}

static inline void loadtraveltext(Context & context) {
	context.dx = 2247;
	standardload(context);
	context.data.word(456) = context.ax;
	return;
}

static inline void showarrows(Context & context) {
	context.di = 116-12;
	context.bx = 16;
	context.ds = context.data.word(458);
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.di = 226+12;
	context.bx = 16;
	context.ds = context.data.word(458);
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.di = 280;
	context.bx = 14;
	context.ds = context.data.word(458);
	context.al = 2;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void getdestinfo(Context & context) {
	context.al = context.data.byte(183);
	context.ah = 0;
	context.push(context.ax);
	context.dx = context.ds;
	context.es = context.dx;
	context.si = 7798;
	context._add(context.si, context.ax);
	context.cl = context.data.byte(context.si);
	context.ax = context.pop();
	context.push(context.cx);
	context.dx = context.ds;
	context.es = context.dx;
	context.si = 7813;
	context._add(context.si, context.ax);
	context.ax = context.pop();
	return;
}

static inline void locationpic(Context & context) {
	getdestinfo(context);
	context.al = context.data.byte(context.si);
	context.push(context.es);
	context.push(context.si);
	context.di = 0;
	context._cmp(context.al, 6);
	if (!context.flags.c()) goto secondlot;
	context.ds = context.data.word(458);
	context._add(context.al, 4);
	goto gotgraphic;
secondlot:
	context._sub(context.al, 6);
	context.ds = context.data.word(460);
gotgraphic:
	context._add(context.di, 104);
	context.bx = 138+14;
	context.ah = 0;
	showframe(context);
	context.si = context.pop();
	context.es = context.pop();
	context.al = context.data.byte(183);
	context._cmp(context.al, context.data.byte(184));
	if (!context.flags.z()) goto notinthisone;
	context.al = 3;
	context.di = 104;
	context.bx = 140+14;
	context.ds = context.data.word(458);
	context.ah = 0;
	showframe(context);
notinthisone:
	context.bl = context.data.byte(183);
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(456);
	context.si = context.data.word(context.bx);
	context._add(context.si, 66*2);
	context.di = 50;
	context.bx = 20;
	context.dl = 241;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	return;
}

static inline void getridoftemp2(Context & context) {
	context.es = context.data.word(460);
	deallocatemem(context);
	return;
}

static inline void getridoftemp3(Context & context) {
	context.es = context.data.word(462);
	deallocatemem(context);
	return;
}

static inline void selectlocation(Context & context) {
	context.data.byte(237) = 0;
	clearbeforeload(context);
	context.data.byte(103) = 0;
	context.data.byte(231) = 22;
	readcitypic(context);
	showcity(context);
	getridoftemp(context);
	readdesticon(context);
	loadtraveltext(context);
	showpanel(context);
	showman(context);
	showarrows(context);
	showexit(context);
	locationpic(context);
	undertextline(context);
	context.data.byte(100) = 255;
	readmouse(context);
	context.data.byte(231) = 0;
	showpointer(context);
	worktoscreen(context);
	context.al = 9;
	context.ah = 255;
	playchannel0(context);
	context.data.byte(188) = 255;
select:
	delpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context._cmp(context.data.byte(103), 1);
	if (context.flags.z()) goto quittravel;
	context.bx = 2714;
	checkcoords(context);
	context._cmp(context.data.byte(188), 255);
	if (context.flags.z()) goto select;
	context.al = context.data.byte(188);
	context._cmp(context.al, context.data.byte(9));
	if (context.flags.z()) goto quittravel;
	getridoftemp(context);
	getridoftemp2(context);
	getridoftemp3(context);
	context.es = context.data.word(456);
	deallocatemem(context);
	return;
quittravel:
	context.al = context.data.byte(184);
	context.data.byte(188) = context.al;
	context.data.byte(103) = 0;
	getridoftemp(context);
	getridoftemp2(context);
	getridoftemp3(context);
	context.es = context.data.word(456);
	deallocatemem(context);
	return;
}

static inline void newplace(Context & context) {
	context._cmp(context.data.byte(60), 1);
	if (context.flags.z()) goto istravel;
	context._cmp(context.data.byte(189), -1);
	if (!context.flags.z()) goto isautoloc;
	return;
isautoloc:
	context.al = context.data.byte(189);
	context.data.byte(188) = context.al;
	context.data.byte(189) = -1;
	return;
istravel:
	context.data.byte(60) = 0;
	selectlocation(context);
	return;
}

static inline void checkcoords(Context & context) {
loop048:
	context.ax = context.data.word(context.bx);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto nonefound;
	context.push(context.bx);
	context._cmp(context.data.word(198), context.ax);
	if (context.flags.l()) goto over045;
	context.ax = context.data.word(context.bx+2);
	context._cmp(context.data.word(198), context.ax);
	if (!context.flags.ge()) goto over045;
	context.ax = context.data.word(context.bx+4);
	context._cmp(context.data.word(200), context.ax);
	if (context.flags.l()) goto over045;
	context.ax = context.data.word(context.bx+6);
	context._cmp(context.data.word(200), context.ax);
	if (!context.flags.ge()) goto over045;
	context.ax = context.data.word(context.bx+8);
	__dispatch_call(context, context.ax);
finished:
	context.ax = context.pop();
	return;
over045:
	context.bx = context.pop();
	context._add(context.bx, 10);
	goto loop048;
nonefound:
	return;
}

static inline void finishedwalking(Context & context) {
	context._cmp(context.data.byte(492), 254);
	if (!context.flags.z()) goto iswalking;
	context.al = context.data.byte(133);
	context._cmp(context.al, context.data.byte(135));
iswalking:
	return;
}

static inline void showexit(Context & context) {
	context.ds = context.data.word(408);
	context.di = 274;
	context.bx = 154;
	context.al = 11;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void getanyad(Context & context) {
	context._cmp(context.data.byte(102), 4);
	if (context.flags.z()) goto isex;
	context._cmp(context.data.byte(102), 2);
	if (context.flags.z()) goto isfree;
	context.al = context.data.byte(99);
	getsetad(context);
	context.ax = context.data.word(context.bx+4);
	return;
isfree:
	context.al = context.data.byte(99);
	getfreead(context);
	context.ax = context.data.word(context.bx+7);
	return;
isex:
	context.al = context.data.byte(99);
	getexad(context);
	context.ax = context.data.word(context.bx+7);
	return;
}

static inline void obicons(Context & context) {
	context.al = context.data.byte(99);
	getanyad(context);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto cantopenit;
	context.ds = context.data.word(410);
	context.di = 210;
	context.bx = 1;
	context.al = 4;
	context.ah = 0;
	showframe(context);
cantopenit:
	context.ds = context.data.word(410);
	context.di = 260;
	context.bx = 1;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void obpicture(Context & context) {
	context.al = context.data.byte(99);
	context.ah = context.data.byte(102);
	context._cmp(context.ah, 1);
	if (context.flags.z()) goto setframe;
	context._cmp(context.ah, 4);
	if (context.flags.z()) goto exframe;
	context.ds = context.data.word(446);
	context.di = 160;
	context.bx = 68;
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	context._add(context.al, 1);
	context.ah = 128;
	showframe(context);
	return;
setframe:
	return;
exframe:
	context.ds = context.data.word(398);
	context.di = 160;
	context.bx = 68;
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	context._add(context.al, 1);
	context.ah = 128;
	showframe(context);
	return;
}

static inline void searchforsame(Context & context) {
	context.si = context.cx;
searchagain:
	context._add(context.si, 1);
	context.al = context.data.byte(context.bx);
search:
	context._cmp(context.data.byte(context.si), context.al);
	if (context.flags.z()) goto gotstartletter;
	context._add(context.cx, 1);
	context._add(context.si, 1);
	context._cmp(context.si, 8000);
	if (context.flags.c()) goto search;
	context.si = context.bx;
	context.ax = context.pop();
	return;
gotstartletter:
	context.push(context.bx);
	context.push(context.si);
keepchecking:
	context._add(context.si, 1);
	context._add(context.bx, 1);
	context.al = context.data.byte(context.bx);
	context.ah = context.data.byte(context.si);
	context._cmp(context.al, ':');
	if (context.flags.z()) goto foundmatch;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundmatch;
	context._cmp(context.al, context.ah);
	if (context.flags.z()) goto keepchecking;
	context.si = context.pop();
	context.bx = context.pop();
	goto searchagain;
foundmatch:
	context.si = context.pop();
	context.bx = context.pop();
	return;
}

static inline void getobtextstart(Context & context) {
	context.es = context.data.word(438);
	context.si = 0;
	context.cx = 0+(82*2);
	context._cmp(context.data.byte(102), 2);
	if (context.flags.z()) goto describe;
	context.es = context.data.word(440);
	context.si = 0;
	context.cx = 0+(130*2);
	context._cmp(context.data.byte(102), 1);
	if (context.flags.z()) goto describe;
	context.es = context.data.word(398);
	context.si = 0+2080+30000+(16*114);
	context.cx = 0+2080+30000+(16*114)+((114+2)*2);
describe:
	context.al = context.data.byte(99);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.si, context.ax);
	context.ax = context.data.word(context.si);
	context._add(context.ax, context.cx);
	context.si = context.ax;
	context.bx = context.ax;
tryagain:
	context.push(context.si);
	findnextcolon(context);
	context.al = context.data.byte(context.si);
	context.cx = context.si;
	context.si = context.pop();
	context._cmp(context.data.byte(102), 1);
	if (!context.flags.z()) goto cantmakeoneup;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto findsometext;
	context._cmp(context.al, ':');
	if (context.flags.z()) goto findsometext;
cantmakeoneup:
	return;
findsometext:
	searchforsame(context);
	goto tryagain;
	return;
}

static inline void getlocation(Context & context) {
	context.ah = 0;
	context.bx = context.ax;
	context.dx = context.ds;
	context.es = context.dx;
	context._add(context.bx, 7798);
	context.al = context.data.byte(context.bx);
	return;
}

static inline void setlocation(Context & context) {
	context.ah = 0;
	context.bx = context.ax;
	context.dx = context.ds;
	context.es = context.dx;
	context._add(context.bx, 7798);
	context.data.byte(context.bx) = 1;
	return;
}

static inline void getridofreels(Context & context) {
	context._cmp(context.data.byte(74), 0);
	if (context.flags.z()) goto dontgetrid;
	context.es = context.data.word(430);
	deallocatemem(context);
	context.es = context.data.word(432);
	deallocatemem(context);
	context.es = context.data.word(434);
	deallocatemem(context);
dontgetrid:
	return;
}

static inline void loadkeypad(Context & context) {
	context.dx = 1961;
	loadintotemp(context);
	return;
}

static inline void hangonw(Context & context) {
hangloopw:
	context.push(context.cx);
	delpointer(context);
	readmouse(context);
	animpointer(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	context.cx = context.pop();
	if (--context.cx) goto hangloopw;
	return;
}

static inline void getroomdata(Context & context) {
	context.ah = 0;
	context.cx = 32;
	context._mul(context.cx);
	context.bx = 5971;
	context._add(context.bx, context.ax);
	return;
}

static inline void allocateload(Context & context) {
	context.push(context.es);
	context.push(context.di);
	context.bx = context.data.word(context.di);
	context.cl = 4;
	context._shr(context.bx, context.cl);
	allocatemem(context);
	context.di = context.pop();
	context.es = context.pop();
	return;
}

static inline void restorereels(Context & context) {
	context._cmp(context.data.byte(74), 0);
	if (context.flags.z()) goto dontrestore;
	context.al = context.data.byte(184);
	getroomdata(context);
	context.dx = context.bx;
	openfile(context);
	readheader(context);
	dontloadseg(context);
	dontloadseg(context);
	dontloadseg(context);
	dontloadseg(context);
	allocateload(context);
	context.data.word(430) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(432) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(434) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	closefile(context);
dontrestore:
	return;
}

static inline void putbackobstuff(Context & context) {
	createpanel(context);
	showpanel(context);
	showman(context);
	obicons(context);
	showexit(context);
	obpicture(context);
	describeob(context);
	undertextline(context);
	context.data.byte(100) = 255;
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	return;
}

static inline void lookatcard(Context & context) {
	context.data.byte(131) = 1;
	getridofreels(context);
	loadkeypad(context);
	createpanel2(context);
	context.di = 160;
	context.bx = 80;
	context.ds = context.data.word(458);
	context.al = 42;
	context.ah = 128;
	showframe(context);
	getobtextstart(context);
	findnextcolon(context);
	findnextcolon(context);
	findnextcolon(context);
	context.di = 36;
	context.bx = 124;
	context.dl = 241;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.push(context.es);
	context.push(context.si);
	worktoscreenm(context);
	context.cx = 280;
	hangonw(context);
	createpanel2(context);
	context.di = 160;
	context.bx = 80;
	context.ds = context.data.word(458);
	context.al = 42;
	context.ah = 128;
	showframe(context);
	context.si = context.pop();
	context.es = context.pop();
	context.di = 36;
	context.bx = 130;
	context.dl = 241;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	worktoscreenm(context);
	context.cx = 200;
	hangonw(context);
	context.data.byte(131) = 0;
	getridoftemp(context);
	restorereels(context);
	putbackobstuff(context);
	return;
}

static inline void obsthatdothings(Context & context) {
	context.al = context.data.byte(99);
	context.ah = context.data.byte(102);
	context.cl = 'M';
	context.ch = 'E';
	context.dl = 'M';
	context.dh = 'B';
	compare(context);
	if (!context.flags.z()) goto notlouiscard;
	context.al = 4;
	getlocation(context);
	context._cmp(context.al, 1);
	if (context.flags.z()) goto seencard;
	context.al = 4;
	setlocation(context);
	lookatcard(context);
seencard:
	return;
notlouiscard:
	return;
}

static inline void findpuztext(Context & context) {
	context.ah = 0;
	context.si = context.ax;
	context._add(context.si, context.si);
	context.es = context.data.word(454);
	context.ax = context.data.word(context.si);
	context._add(context.ax, 66*2);
	context.si = context.ax;
	return;
}

static inline void additionaltext(Context & context) {
	context._add(context.bx, 10);
	context.push(context.bx);
	context.al = context.data.byte(99);
	context.ah = context.data.byte(102);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto emptycup;
	context.al = context.data.byte(99);
	context.ah = context.data.byte(102);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'F';
	compare(context);
	if (context.flags.z()) goto fullcup;
	context.bx = context.pop();
	return;
emptycup:
	context.al = 40;
	findpuztext(context);
	context.bx = context.pop();
	context.di = 36;
	context.dl = 241;
	context.ah = 0;
	printdirect(context);
	return;
fullcup:
	context.al = 39;
	findpuztext(context);
	context.bx = context.pop();
	context.di = 36;
	context.dl = 241;
	context.ah = 0;
	printdirect(context);
	return;
}

static inline void describeob(Context & context) {
	getobtextstart(context);
	context.di = 33;
	context.bx = 92;
	context._cmp(context.data.byte(102), 1);
	if (!context.flags.z()) goto notsetd;
	context.bx = 82;
notsetd:
	context.dl = 241;
	context.ah = 16;
	context.data.word(71) = 91+91;
	printdirect(context);
	context.data.word(71) = 0;
	context.di = 36;
	context.bx = 104;
	context._cmp(context.data.byte(102), 1);
	if (!context.flags.z()) goto notsetd2;
	context.bx = 94;
notsetd2:
	context.dl = 241;
	context.ah = 0;
	printdirect(context);
	context.push(context.bx);
	obsthatdothings(context);
	context.bx = context.pop();
	additionaltext(context);
	return;
}

static inline void makemainscreen(Context & context) {
	createpanel(context);
	context.data.byte(62) = 1;
	drawfloor(context);
	spriteupdate(context);
	printsprites(context);
	reelsonscreen(context);
	showicon(context);
	getunderzoom(context);
	undertextline(context);
	context.data.byte(100) = 255;
	animpointer(context);
	worktoscreenm(context);
	context.data.byte(100) = 200;
	context.data.byte(131) = 0;
	return;
}

static inline void examineob(Context & context) {
	context.data.byte(234) = 0;
	context.data.word(328) = 0;
examineagain:
	context.data.byte(237) = 0;
	context.data.byte(108) = 0;
	context.data.byte(110) = 255;
	context.data.byte(111) = 255;
	context.data.byte(104) = 0;
	context.al = context.data.byte(100);
	context.data.byte(102) = context.al;
	context.data.byte(89) = 0;
	context.data.byte(231) = 0;
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	obicons(context);
	obpicture(context);
	describeob(context);
	undertextline(context);
	context.data.byte(100) = 255;
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
waitexam:
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	delpointer(context);
	context.data.byte(103) = 0;
	context.bx = 2494;
	context._cmp(context.data.byte(104), 0);
	if (context.flags.z()) goto notuseinv;
	context.bx = 2556;
	context._cmp(context.data.byte(104), 1);
	if (context.flags.z()) goto notuseinv;
	context.bx = 2618;
notuseinv:
	checkcoords(context);
	context._cmp(context.data.byte(108), 0);
	if (context.flags.z()) goto norex;
	goto examineagain;
norex:
	context._cmp(context.data.byte(103), 0);
	if (context.flags.z()) goto waitexam;
	context.data.byte(106) = 0;
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) goto iswatching;
	context._cmp(context.data.byte(188), 255);
	if (!context.flags.z()) goto justgetback;
iswatching:
	makemainscreen(context);
	context.data.byte(104) = 0;
	context.data.byte(110) = 255;
	return;
justgetback:
	context.data.byte(104) = 0;
	context.data.byte(110) = 255;
	return;
}

static inline void facerightway(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	getroomspaths(context);
	context.al = context.data.byte(475);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.bx+7);
	context.data.byte(135) = context.al;
	context.data.byte(134) = context.al;
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void setwalk(Context & context) {
	context._cmp(context.data.byte(492), 254);
	if (!context.flags.z()) goto alreadywalking;
	context.al = context.data.byte(474);
	context._cmp(context.al, context.data.byte(475));
	if (context.flags.z()) goto cantwalk2;
	context._cmp(context.data.byte(32), 1);
	if (context.flags.z()) goto holdingreel;
	context._cmp(context.data.byte(32), 2);
	if (context.flags.z()) goto cantwalk;
	context.data.byte(478) = context.al;
	context.data.byte(477) = context.al;
	context._cmp(context.data.word(202), 2);
	if (!context.flags.z()) goto notwalkandexam;
	context._cmp(context.data.byte(100), 3);
	if (context.flags.z()) goto notwalkandexam;
	context.data.byte(306) = 1;
	context.al = context.data.byte(100);
	context.data.byte(307) = context.al;
	context.al = context.data.byte(99);
	context.data.byte(308) = context.al;
notwalkandexam:
	autosetwalk(context);
cantwalk:
	return;
cantwalk2:
	facerightway(context);
	return;
alreadywalking:
	context.al = context.data.byte(474);
	context.data.byte(477) = context.al;
	return;
holdingreel:
	context.data.byte(33) = context.al;
	context.data.byte(32) = 2;
	return;
}

static inline void examineobtext(Context & context) {
	context.bl = context.data.byte(99);
	context.bh = context.data.byte(100);
	context.al = 1;
	commandwithob(context);
	return;
}

static inline void blocknametext(Context & context) {
	context.bl = context.data.byte(99);
	context.bh = context.data.byte(100);
	context.al = 0;
	commandwithob(context);
	return;
}

static inline void personnametext(Context & context) {
	context.bl = context.data.byte(99);
	context._and(context.bl, 127);
	context.bh = context.data.byte(100);
	context.al = 2;
	commandwithob(context);
	return;
}

static inline void deltextline(Context & context) {
	context.di = context.data.word(79);
	context.bx = context.data.word(81);
	context._sub(context.bx, 3);
	context.ds = context.data.word(412);
	context.si = 0;
	context.cl = 228;
	context.ch = 13;
	multiput(context);
	return;
}

static inline void findobname(Context & context) {
	context.push(context.ax);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.ax = context.pop();
	context._cmp(context.ah, 5);
	if (!context.flags.z()) goto notpersonname;
	context.push(context.ax);
	context._and(context.al, 127);
	context.ah = 0;
	context.bx = 64*2;
	context._mul(context.bx);
	context.si = context.ax;
	context.ds = context.data.word(448);
	context._add(context.si, 0+24);
	context.cx = 0+24+(1026*2);
	context.ax = context.data.word(context.si);
	context._add(context.ax, context.cx);
	context.si = context.ax;
	context.ax = context.pop();
	return;
notpersonname:
	context._cmp(context.ah, 4);
	if (!context.flags.z()) goto notextraname;
	context.ds = context.data.word(398);
	context._add(context.bx, 0+2080+30000+(16*114));
	context.ax = context.data.word(context.bx);
	context._add(context.ax, 0+2080+30000+(16*114)+((114+2)*2));
	context.si = context.ax;
	return;
notextraname:
	context._cmp(context.ah, 2);
	if (!context.flags.z()) goto notfreename;
	context.ds = context.data.word(438);
	context._add(context.bx, 0);
	context.ax = context.data.word(context.bx);
	context._add(context.ax, 0+(82*2));
	context.si = context.ax;
	return;
notfreename:
	context._cmp(context.ah, 1);
	if (!context.flags.z()) goto notsetname;
	context.ds = context.data.word(440);
	context._add(context.bx, 0);
	context.ax = context.data.word(context.bx);
	context._add(context.ax, 0+(130*2));
	context.si = context.ax;
	return;
notsetname:
	context.ds = context.data.word(442);
	context._add(context.bx, 0);
	context.ax = context.data.word(context.bx);
	context._add(context.ax, 0+(98*2));
	context.si = context.ax;
	return;
}

static inline void copyname(Context & context) {
	context.push(context.di);
	findobname(context);
	context.di = context.pop();
	context.es = context.cs;
copytext:
	context.cx = 28;
make:
	context._lodsb();
	context._cmp(context.al, ':');
	if (context.flags.z()) goto finishmakename;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto finishmakename;
	context._stosb();
	if (--context.cx) goto make;
finishmakename:
	context._add(context.cx, 1);
	context.al = 0;
	context._stosb();
	return;
	context.al = 255;
	while(--context.cx) 	context._stosb();
	return;
}

static inline void commandwithob(Context & context) {
	context.push(context.ax);
	context.push(context.ax);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.dx);
	context.push(context.es);
	context.push(context.ds);
	context.push(context.si);
	context.push(context.di);
	deltextline(context);
	context.di = context.pop();
	context.si = context.pop();
	context.ds = context.pop();
	context.es = context.pop();
	context.dx = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ax = context.pop();
	context.push(context.bx);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.es = context.data.word(452);
	context.ax = context.data.word(context.bx);
	context._add(context.ax, 66*2);
	context.si = context.ax;
	context.di = context.data.word(79);
	context.bx = context.data.word(81);
	context.dl = context.data.byte(82);
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.ax = context.pop();
	context.di = 5674;
	copyname(context);
	context.ax = context.pop();
	context.di = context.data.word(84);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto noadd;
	context._add(context.di, 5);
noadd:
	context.bx = context.data.word(81);
	context.es = context.cs;
	context.si = 5674;
	context.dl = context.data.byte(82);
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.data.byte(109) = 1;
	return;
}

static inline void walktotext(Context & context) {
	context.bl = context.data.byte(99);
	context.bh = context.data.byte(100);
	context.al = 3;
	commandwithob(context);
	return;
}

static inline void walkandexamine(Context & context) {
	finishedwalking(context);
	if (!context.flags.z()) goto noobselect;
	context.al = context.data.byte(307);
	context.data.byte(100) = context.al;
	context.al = context.data.byte(308);
	context.data.byte(99) = context.al;
	context.data.byte(306) = 0;
	context._cmp(context.data.byte(100), 5);
	if (context.flags.z()) goto noobselect;
	examineob(context);
	return;
wantstowalk:
	setwalk(context);
	context.data.byte(473) = 1;
noobselect:
	return;
diff:
	context.data.byte(99) = context.al;
	context.data.byte(100) = context.ah;
diff2:
	context._cmp(context.data.byte(492), 254);
	if (!context.flags.z()) goto middleofwalk;
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) goto middleofwalk;
	context.al = context.data.byte(133);
	context._cmp(context.al, context.data.byte(135));
	if (!context.flags.z()) goto middleofwalk;
	context._cmp(context.data.byte(100), 3);
	if (!context.flags.z()) goto notblock;
	context.bl = context.data.byte(475);
	context._cmp(context.bl, context.data.byte(474));
	if (!context.flags.z()) goto dontcheck;
	context.cl = context.data.byte(151);
	context._add(context.cl, 12);
	context.ch = context.data.byte(152);
	context._add(context.ch, 12);
	checkone(context);
	context._cmp(context.cl, 2);
	if (context.flags.c()) goto isblock;
dontcheck:
	getflagunderp(context);
	context._cmp(context.data.byte(153), 2);
	if (context.flags.c()) goto isblock;
	context._cmp(context.data.byte(153), 128);
	if (!context.flags.c()) goto isblock;
	goto toofaraway;
notblock:
	context.bl = context.data.byte(475);
	context._cmp(context.bl, context.data.byte(474));
	if (!context.flags.z()) goto toofaraway;
	context._cmp(context.data.byte(100), 3);
	if (context.flags.z()) goto isblock;
	context._cmp(context.data.byte(100), 5);
	if (context.flags.z()) goto isaperson;
	examineobtext(context);
	return;
middleofwalk:
	blocknametext(context);
	return;
isblock:
	blocknametext(context);
	return;
isaperson:
	personnametext(context);
	return;
toofaraway:
	walktotext(context);
	return;
}

static inline void mainscreen(Context & context) {
	context.data.byte(237) = 0;
	context.bx = 4949;
	context._cmp(context.data.byte(3), 1);
	if (context.flags.z()) goto checkmain;
	context.bx = 5011;
checkmain:
	checkcoords(context);
	context._cmp(context.data.byte(306), 0);
	if (context.flags.z()) goto finishmain;
	walkandexamine(context);
finishmain:
	return;
}

static inline void getflagunderp(Context & context) {
	context.cx = context.data.word(198);
	context._sub(context.cx, context.data.word(117));
	context.ax = context.data.word(200);
	context._sub(context.ax, context.data.word(119));
	context.ch = context.al;
	checkone(context);
	context.data.byte(153) = context.cl;
	context.data.byte(154) = context.ch;
	return;
}

static inline void animpointer(Context & context) {
	context._cmp(context.data.byte(234), 2);
	if (context.flags.z()) goto combathand;
	context._cmp(context.data.byte(234), 3);
	if (context.flags.z()) goto mousehand;
	context._cmp(context.data.word(21), 0);
	if (context.flags.z()) goto notwatchpoint;
	context.data.byte(231) = 11;
	return;
notwatchpoint:
	context.data.byte(231) = 0;
	context._cmp(context.data.byte(237), 0);
	if (context.flags.z()) goto gothand;
	context._cmp(context.data.byte(476), 0);
	if (context.flags.z()) goto gothand;
arrow:
	getflagunderp(context);
	context._cmp(context.cl, 2);
	if (context.flags.c()) goto gothand;
	context._cmp(context.cl, 128);
	if (!context.flags.c()) goto gothand;
	context.data.byte(231) = 3;
	context._test(context.cl, 4);
	if (!context.flags.z()) goto gothand;
	context.data.byte(231) = 4;
	context._test(context.cl, 16);
	if (!context.flags.z()) goto gothand;
	context.data.byte(231) = 5;
	context._test(context.cl, 2);
	if (!context.flags.z()) goto gothand;
	context.data.byte(231) = 6;
	context._test(context.cl, 8);
	if (!context.flags.z()) goto gothand;
	context.data.byte(231) = 8;
gothand:
	return;
mousehand:
	context._cmp(context.data.byte(235), 0);
	if (context.flags.z()) goto rightspeed3;
	context._sub(context.data.byte(235), 1);
	goto finflashmouse;
rightspeed3:
	context.data.byte(235) = 5;
	context._add(context.data.byte(236), 1);
	context._cmp(context.data.byte(236), 16);
	if (!context.flags.z()) goto finflashmouse;
	context.data.byte(236) = 0;
finflashmouse:
	context.al = context.data.byte(236);
	context.ah = 0;
	context.bx = 5698;
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.bx);
	context.data.byte(231) = context.al;
	return;
combathand:
	context.data.byte(231) = 0;
	context._cmp(context.data.byte(184), 14);
	if (!context.flags.z()) goto notarrow;
	context._cmp(context.data.byte(100), 211);
	if (!context.flags.z()) goto notarrow;
	context.data.byte(231) = 5;
notarrow:
	return;
}

static inline void showblink(Context & context) {
	context._cmp(context.data.byte(131), 1);
	if (context.flags.z()) goto finblink1;
	context._add(context.data.byte(472), 1);
	context._cmp(context.data.byte(4), 0);
	if (!context.flags.z()) goto finblink1;
	context._cmp(context.data.byte(184), 50);
	if (!context.flags.c()) goto eyesshut;
	context.al = context.data.byte(472);
	context._cmp(context.al, 3);
	if (!context.flags.z()) goto finblink1;
	context.data.byte(472) = 0;
	context.al = context.data.byte(471);
	context._add(context.al, 1);
	context.data.byte(471) = context.al;
	context._cmp(context.al, 6);
	if (context.flags.c()) goto nomorethan6;
	context.al = 6;
nomorethan6:
	context.ah = 0;
	context.bx = 5691;
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.bx);
	context.ds = context.data.word(408);
	context.di = 44;
	context.bx = 32;
	context.ah = 0;
	showframe(context);
finblink1:
	return;
eyesshut:
	return;
}

static inline void showpointer(Context & context) {
	showblink(context);
	context.di = context.data.word(198);
	context.data.word(220) = context.di;
	context.bx = context.data.word(200);
	context.data.word(222) = context.bx;
	context._cmp(context.data.byte(106), 1);
	if (context.flags.z()) goto itsanobject;
	context.push(context.bx);
	context.push(context.di);
	context.ds = context.data.word(408);
	context.al = context.data.byte(231);
	context._add(context.al, 20);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.si = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.si, context.ax);
	context.cx = context.data.word(context.si);
	context._cmp(context.cl, 12);
	if (!context.flags.c()) goto notsmallx;
	context.cl = 12;
notsmallx:
	context._cmp(context.ch, 12);
	if (!context.flags.c()) goto notsmally;
	context.ch = 12;
notsmally:
	context.data.byte(227) = context.cl;
	context.data.byte(228) = context.ch;
	context.push(context.ds);
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60;
	multiget(context);
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.push(context.di);
	context.push(context.bx);
	context.al = context.data.byte(231);
	context._add(context.al, 20);
	context.ah = 0;
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	return;
itsanobject:
	context.al = context.data.byte(89);
	context.ds = context.data.word(398);
	context._cmp(context.data.byte(102), 4);
	if (context.flags.z()) goto itsfrominv;
	context.ds = context.data.word(446);
itsfrominv:
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	context._add(context.al, 1);
	context.ah = 0;
	context.push(context.ax);
	context._add(context.ax, context.ax);
	context.si = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.si, context.ax);
	context.ax = 2080;
	context.cx = context.data.word(context.si);
	context._cmp(context.cl, 12);
	if (!context.flags.c()) goto notsmallx2;
	context.cl = 12;
notsmallx2:
	context._cmp(context.ch, 12);
	if (!context.flags.c()) goto notsmally2;
	context.ch = 12;
notsmally2:
	context.data.byte(227) = context.cl;
	context.data.byte(228) = context.ch;
	context.ax = context.pop();
	context.push(context.di);
	context.push(context.bx);
	context.push(context.ax);
	context.push(context.bx);
	context.push(context.di);
	context.push(context.ds);
	context.al = context.cl;
	context.ah = 0;
	context._shr(context.ax, 1);
	context._sub(context.data.word(220), context.ax);
	context._sub(context.di, context.ax);
	context.al = context.ch;
	context._shr(context.ax, 1);
	context._sub(context.data.word(222), context.ax);
	context._sub(context.bx, context.ax);
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60;
	multiget(context);
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.ax = context.pop();
	context.ah = 128;
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.ds = context.data.word(408);
	context.al = 3;
	context.ah = 128;
	showframe(context);
	return;
}

static inline void readmouse1(Context & context) {
	context.ax = context.data.word(198);
	context.data.word(214) = context.ax;
	context.ax = context.data.word(200);
	context.data.word(216) = context.ax;
	mousecall(context);
	context.data.word(198) = context.cx;
	context.data.word(200) = context.dx;
	context.data.word(204) = context.bx;
	return;
}

static inline void dumpblink(Context & context) {
	context._cmp(context.data.byte(4), 0);
	if (!context.flags.z()) goto nodumpeye;
	context._cmp(context.data.byte(472), 0);
	if (!context.flags.z()) goto nodumpeye;
	context.al = context.data.byte(471);
	context._cmp(context.al, 6);
	if (!context.flags.c()) goto nodumpeye;
	context.push(context.ds);
	context.di = 44;
	context.bx = 32;
	context.cl = 16;
	context.ch = 12;
	multidump(context);
	context.ds = context.pop();
nodumpeye:
	return;
}

static inline void dumppointer(Context & context) {
	dumpblink(context);
	context.cl = context.data.byte(229);
	context.ch = context.data.byte(230);
	context.di = context.data.word(224);
	context.bx = context.data.word(226);
	multidump(context);
	context.bx = context.data.word(222);
	context.di = context.data.word(220);
	context._cmp(context.di, context.data.word(224));
	if (!context.flags.z()) goto difffound;
	context._cmp(context.bx, context.data.word(226));
	if (context.flags.z()) goto notboth;
difffound:
	context.cl = context.data.byte(227);
	context.ch = context.data.byte(228);
	multidump(context);
notboth:
	return;
}

static inline void dumptextline(Context & context) {
	context._cmp(context.data.byte(109), 1);
	if (!context.flags.z()) goto nodumptextline;
	context.data.byte(109) = 0;
	context.di = context.data.word(79);
	context.bx = context.data.word(81);
	context._sub(context.bx, 3);
	context.cl = 228;
	context.ch = 13;
	multidump(context);
nodumptextline:
	return;
}

static inline void delpointer(Context & context) {
	context.ax = context.data.word(220);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto nevershown;
	context.data.word(224) = context.ax;
	context.ax = context.data.word(222);
	context.data.word(226) = context.ax;
	context.cl = context.data.byte(227);
	context.data.byte(229) = context.cl;
	context.ch = context.data.byte(228);
	context.data.byte(230) = context.ch;
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60;
	context.di = context.data.word(224);
	context.bx = context.data.word(226);
	multiput(context);
nevershown:
	return;
}

static inline void findnextcolon(Context & context) {
isntcolon:
	context.al = context.data.byte(context.si);
	context._add(context.si, 1);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto endofcolon;
	context._cmp(context.al, ':');
	if (!context.flags.z()) goto isntcolon;
endofcolon:
	return;
}

static inline void printboth(Context & context) {
	context.push(context.ax);
	context.push(context.cx);
	context.push(context.bx);
	context.push(context.di);
	printchar(context);
	context.ax = context.pop();
	context.push(context.di);
	context.di = context.ax;
	multidump(context);
	context.di = context.pop();
	context.bx = context.pop();
	context.cx = context.pop();
	context.ax = context.pop();
	return;
}

static inline void waitframes(Context & context) {
	context.push(context.di);
	context.push(context.bx);
	context.push(context.es);
	context.push(context.si);
	context.push(context.ds);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	delpointer(context);
	context.ax = context.data.word(202);
	context.ds = context.pop();
	context.si = context.pop();
	context.es = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	return;
}

static inline void printslow(Context & context) {
	context.data.byte(231) = 1;
	context.data.byte(234) = 3;
	context.ds = context.data.word(404);
printloopslow6:
	context.push(context.bx);
	context.push(context.di);
	context.push(context.dx);
	getnumber(context);
	context.ch = 0;
printloopslow5:
	context.push(context.cx);
	context.push(context.si);
	context.push(context.es);
	context.ax = context.data.word(context.si);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.es);
	context.push(context.si);
	context.push(context.ds);
	modifychar(context);
	printboth(context);
	context.ds = context.pop();
	context.si = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ax = context.data.word(context.si+1);
	context._add(context.si, 1);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto finishslow;
	context._cmp(context.al, ':');
	if (context.flags.z()) goto finishslow;
	context._cmp(context.cl, 1);
	if (context.flags.z()) goto afterslow;
	context.push(context.di);
	context.push(context.ds);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.es);
	context.push(context.si);
	modifychar(context);
	context.data.word(71) = 91;
	printboth(context);
	context.data.word(71) = 0;
	context.si = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	waitframes(context);
	context._cmp(context.ax, 0);
	if (context.flags.z()) goto keepgoing;
	context._cmp(context.ax, context.data.word(212));
	if (!context.flags.z()) goto finishslow2;
keepgoing:
	waitframes(context);
noslow:
	context._cmp(context.ax, 0);
	if (context.flags.z()) goto afterslow;
	context._cmp(context.ax, context.data.word(212));
	if (!context.flags.z()) goto finishslow2;
afterslow:
	context.es = context.pop();
	context.si = context.pop();
	context.cx = context.pop();
	context._add(context.si, 1);
	if (--context.cx) goto printloopslow5;
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context._add(context.bx, 10);
	goto printloopslow6;
finishslow:
	context.cx = context.pop();
	context.si = context.pop();
	context.es = context.pop();
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.al = 0;
	return;
finishslow2:
	context.cx = context.pop();
	context.si = context.pop();
	context.es = context.pop();
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.al = 1;
	return;
}

static inline void hangonp(Context & context) {
	context.push(context.cx);
	context._add(context.cx, context.cx);
	context.ax = context.pop();
	context._add(context.cx, context.ax);
	context.data.word(138) = 0;
	context.al = context.data.byte(231);
	context.ah = context.data.byte(106);
	context.push(context.ax);
	context.data.byte(234) = 3;
	context.data.byte(106) = 0;
	context.push(context.cx);
	context.data.byte(100) = 255;
	readmouse(context);
	animpointer(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	context.cx = context.pop();
hangloop:
	context.push(context.cx);
	delpointer(context);
	readmouse(context);
	animpointer(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	context.cx = context.pop();
	context.ax = context.data.word(202);
	context._cmp(context.ax, 0);
	if (context.flags.z()) goto notpressed;
	context._cmp(context.ax, context.data.word(212));
	if (!context.flags.z()) goto getoutofit;
notpressed:
	if (--context.cx) goto hangloop;
getoutofit:
	delpointer(context);
	context.ax = context.pop();
	context.data.byte(231) = context.al;
	context.data.byte(106) = context.ah;
	context.data.byte(234) = 0;
	return;
}

static inline void readmouse(Context & context) {
	context.ax = context.data.word(202);
	context.data.word(212) = context.ax;
	context.ax = context.data.word(198);
	context.data.word(214) = context.ax;
	context.ax = context.data.word(200);
	context.data.word(216) = context.ax;
	mousecall(context);
	context.data.word(198) = context.cx;
	context.data.word(200) = context.dx;
	context.data.word(202) = context.bx;
	return;
}

static inline void redrawmainscrn(Context & context) {
	context.data.word(328) = 0;
	createpanel(context);
	context.data.byte(62) = 0;
	drawfloor(context);
	printsprites(context);
	reelsonscreen(context);
	showicon(context);
	getunderzoom(context);
	undertextline(context);
	readmouse(context);
	context.data.byte(100) = 255;
	return;
}

static inline void dolook(Context & context) {
	createpanel(context);
	showicon(context);
	undertextline(context);
	worktoscreenm(context);
	context.data.byte(100) = 255;
	dumptextline(context);
	context.bl = context.data.byte(185);
	context._and(context.bl, 31);
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(436);
	context._add(context.bx, 0);
	context.si = context.data.word(context.bx);
	context._add(context.si, 0+(38*2));
	findnextcolon(context);
	context.di = 66;
	context._cmp(context.data.byte(184), 50);
	if (context.flags.c()) goto notdream3;
	context.di = 40;
notdream3:
	context.bx = 80;
	context.dl = 241;
	printslow(context);
	context._cmp(context.al, 1);
	if (context.flags.z()) goto afterlook;
	context.cx = 400;
	hangonp(context);
afterlook:
	context.data.byte(234) = 0;
	context.data.byte(100) = 0;
	redrawmainscrn(context);
	worktoscreenm(context);
	return;
}

static inline void autolook(Context & context) {
	context.ax = context.data.word(198);
	context._cmp(context.ax, context.data.word(214));
	if (!context.flags.z()) goto diffmouse;
	context.ax = context.data.word(200);
	context._cmp(context.ax, context.data.word(216));
	if (!context.flags.z()) goto diffmouse;
	context._sub(context.data.word(98), 1);
	context._cmp(context.data.word(98), 0);
	if (!context.flags.z()) goto noautolook;
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) goto noautolook;
	dolook(context);
noautolook:
	return;
diffmouse:
	context.data.word(98) = 1000;
	return;
}

static inline void watchcount(Context & context) {
	context._cmp(context.data.byte(3), 0);
	if (context.flags.z()) goto nowatchworn;
	context._add(context.data.byte(146), 1);
	context._cmp(context.data.byte(146), 9);
	if (context.flags.z()) goto flashdots;
	context._cmp(context.data.byte(146), 18);
	if (context.flags.z()) goto uptime;
nowatchworn:
	return;
flashdots:
	context.ax = 91*3+21;
	context.di = 268+4;
	context.bx = 21;
	context.ds = context.data.word(404);
	showframe(context);
	goto finishwatch;
uptime:
	context.data.byte(146) = 0;
	context._add(context.data.byte(5), 1);
	context._cmp(context.data.byte(5), 60);
	if (!context.flags.z()) goto finishtime;
	context.data.byte(5) = 0;
	context._add(context.data.byte(6), 1);
	context._cmp(context.data.byte(6), 60);
	if (!context.flags.z()) goto finishtime;
	context.data.byte(6) = 0;
	context._add(context.data.byte(7), 1);
	context._cmp(context.data.byte(7), 24);
	if (!context.flags.z()) goto finishtime;
	context.data.byte(7) = 0;
finishtime:
	showtime(context);
finishwatch:
	context.data.byte(266) = 1;
	return;
}

static inline void putunderzoom(Context & context) {
	context.di = 8+5;
	context.bx = 132+4;
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5);
	context.cl = 46;
	context.ch = 40;
	multiput(context);
	return;
}

static inline void crosshair(Context & context) {
	context._cmp(context.data.byte(100), 3);
	if (context.flags.z()) goto nocross;
	context._cmp(context.data.byte(100), 10);
	if (!context.flags.c()) goto nocross;
	context.es = context.data.word(400);
	context.ds = context.data.word(408);
	context.di = 8+24;
	context.bx = 132+19;
	context.al = 9;
	context.ah = 0;
	showframe(context);
	return;
nocross:
	context.es = context.data.word(400);
	context.ds = context.data.word(408);
	context.di = 8+24;
	context.bx = 132+19;
	context.al = 29;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void zoom(Context & context) {
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) goto inwatching;
	context._cmp(context.data.byte(8), 1);
	if (context.flags.z()) goto zoomswitch;
inwatching:
	return;
zoomswitch:
	context._cmp(context.data.byte(100), 199);
	if (context.flags.c()) goto zoomit;
cantzoom:
	putunderzoom(context);
	return;
zoomit:
	context.ax = context.data.word(222);
	context._sub(context.ax, 9);
	context.cx = 320;
	context._mul(context.cx);
	context._add(context.ax, context.data.word(220));
	context._sub(context.ax, 11);
	context.si = context.ax;
	context.ax = 132+4;
	context.cx = 320;
	context._mul(context.cx);
	context._add(context.ax, 8+5);
	context.di = context.ax;
	context.es = context.data.word(400);
	context.ds = context.data.word(400);
	context.cx = 20;
zoomloop:
	context.push(context.cx);
	context.cx = 23;
zoomloop2:
	context._lodsb();
	context.ah = context.al;
	context._stosw();
	context.data.word(context.di+320-2) = context.ax;
	if (--context.cx) goto zoomloop2;
	context._add(context.si, 320-23);
	context._add(context.di, 320-46+320);
	context.cx = context.pop();
	if (--context.cx) goto zoomloop;
	crosshair(context);
	context.data.byte(75) = 1;
	return;
}

static inline void readmouse2(Context & context) {
	context.ax = context.data.word(198);
	context.data.word(214) = context.ax;
	context.ax = context.data.word(200);
	context.data.word(216) = context.ax;
	mousecall(context);
	context.data.word(198) = context.cx;
	context.data.word(200) = context.dx;
	context.data.word(206) = context.bx;
	return;
}

static inline void dumpzoom(Context & context) {
	context._cmp(context.data.byte(8), 1);
	if (!context.flags.z()) goto notzoomon;
	context.di = 8+5;
	context.bx = 132+4;
	context.cl = 46;
	context.ch = 40;
	multidump(context);
notzoomon:
	return;
}

static inline void findpathofpoint(Context & context) {
	context.push(context.ax);
	context.bx = 0;
	context.es = context.data.word(450);
	context.al = context.data.byte(185);
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.cx = context.pop();
	context.dl = 0;
pathloop:
	context.al = context.data.byte(context.bx+6);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto flunkedit;
	context.ax = context.data.word(context.bx+2);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto flunkedit;
	context._cmp(context.cl, context.al);
	if (context.flags.c()) goto flunkedit;
	context._cmp(context.ch, context.ah);
	if (context.flags.c()) goto flunkedit;
	context.ax = context.data.word(context.bx+4);
	context._cmp(context.cl, context.al);
	if (!context.flags.c()) goto flunkedit;
	context._cmp(context.ch, context.ah);
	if (!context.flags.c()) goto flunkedit;
	goto gotvalidpath;
flunkedit:
	context._add(context.bx, 8);
	context._add(context.dl, 1);
	context._cmp(context.dl, 12);
	if (!context.flags.z()) goto pathloop;
	context.dl = 255;
gotvalidpath:
	return;
}

static inline void findxyfrompath(Context & context) {
	getroomspaths(context);
	context.al = context.data.byte(475);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.ax = context.data.word(context.bx);
	context._sub(context.al, 12);
	context._sub(context.ah, 12);
	context.data.byte(151) = context.al;
	context.data.byte(152) = context.ah;
	return;
}

static inline void showpanel(Context & context) {
	context.ds = context.data.word(408);
	context.di = 72;
	context.bx = 0;
	context.al = 19;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(408);
	context.di = 192;
	context.bx = 0;
	context.al = 19;
	context.ah = 0;
	showframe(context);
	return;
}

static inline void showman(Context & context) {
	context.ds = context.data.word(408);
	context.di = 0;
	context.bx = 0;
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(408);
	context.di = 0;
	context.bx = 114;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context._cmp(context.data.byte(4), 0);
	if (context.flags.z()) goto notverycool;
	context.ds = context.data.word(408);
	context.di = 28;
	context.bx = 25;
	context.al = 2;
	context.ah = 0;
	showframe(context);
notverycool:
	return;
}

static inline void printmessage(Context & context) {
	context.push(context.dx);
	context.push(context.bx);
	context.push(context.di);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.es = context.data.word(452);
	context.ax = context.data.word(context.bx);
	context._add(context.ax, 66*2);
	context.si = context.ax;
	context.di = context.pop();
	context.bx = context.pop();
	context.dx = context.pop();
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	return;
}

static inline void usecharset1(Context & context) {
	context.ax = context.data.word(404);
	context.data.word(268) = context.ax;
	return;
}

static inline void roomname(Context & context) {
	context.di = 88;
	context.bx = 18;
	context.al = 53;
	context.dl = 240;
	printmessage(context);
	context.bl = context.data.byte(185);
	context._cmp(context.bl, 32);
	if (context.flags.c()) goto notover32;
	context._sub(context.bl, 32);
notover32:
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(436);
	context._add(context.bx, 0);
	context.ax = context.data.word(context.bx);
	context._add(context.ax, 0+(38*2));
	context.si = context.ax;
	context.data.word(77) = 7;
	context.di = 88;
	context.bx = 25;
	context.dl = 120;
	context._cmp(context.data.byte(3), 1);
	if (context.flags.z()) goto gotpl;
	context.dl = 160;
gotpl:
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.data.word(77) = 10;
	usecharset1(context);
	return;
}

static inline void showtime(Context & context) {
	context._cmp(context.data.byte(3), 0);
	if (context.flags.z()) goto nowatch;
	context.al = context.data.byte(5);
	context.cl = 0;
	twodigitnum(context);
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context._add(context.ax, 91*3+10);
	context.ds = context.data.word(404);
	context.di = 282+5;
	context.bx = 21;
	showframe(context);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, 91*3+10);
	context.ds = context.data.word(404);
	context.di = 282+9;
	context.bx = 21;
	showframe(context);
	context.al = context.data.byte(6);
	context.cl = 0;
	twodigitnum(context);
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context._add(context.ax, 91*3);
	context.ds = context.data.word(404);
	context.di = 270+5;
	context.bx = 21;
	showframe(context);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, 91*3);
	context.ds = context.data.word(404);
	context.di = 270+11;
	context.bx = 21;
	showframe(context);
	context.al = context.data.byte(7);
	context.cl = 0;
	twodigitnum(context);
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context._add(context.ax, 91*3);
	context.ds = context.data.word(404);
	context.di = 256+5;
	context.bx = 21;
	showframe(context);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, 91*3);
	context.ds = context.data.word(404);
	context.di = 256+11;
	context.bx = 21;
	showframe(context);
	context.ax = 91*3+20;
	context.ds = context.data.word(404);
	context.di = 267+5;
	context.bx = 21;
	showframe(context);
nowatch:
	return;
}

static inline void showwatch(Context & context) {
	context._cmp(context.data.byte(3), 0);
	if (context.flags.z()) goto nowristwatch;
	context.ds = context.data.word(408);
	context.di = 250;
	context.bx = 1;
	context.al = 6;
	context.ah = 0;
	showframe(context);
	showtime(context);
nowristwatch:
	return;
}

static inline void panelicons1(Context & context) {
	context.di = 0;
	context._cmp(context.data.byte(3), 1);
	if (context.flags.z()) goto watchison;
	context.di = 48;
watchison:
	context.push(context.di);
	context.ds = context.data.word(410);
	context._add(context.di, 204);
	context.bx = 4;
	context.al = 2;
	context.ah = 0;
	showframe(context);
	context.di = context.pop();
	context.push(context.di);
	context._cmp(context.data.byte(8), 1);
	if (context.flags.z()) goto zoomisoff;
	context.ds = context.data.word(408);
	context._add(context.di, 228);
	context.bx = 8;
	context.al = 5;
	context.ah = 0;
	showframe(context);
zoomisoff:
	context.di = context.pop();
	showwatch(context);
	return;
}

static inline void zoomicon(Context & context) {
	context._cmp(context.data.byte(8), 0);
	if (context.flags.z()) goto nozoom1;
	context.ds = context.data.word(408);
	context.di = 8;
	context.bx = 132-1;
	context.al = 8;
	context.ah = 0;
	showframe(context);
nozoom1:
	return;
}

static inline void middlepanel(Context & context) {
	context.ds = context.data.word(464);
	context.di = 72+47+20;
	context.bx = 0;
	context.al = 48;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(464);
	context.di = 72+19;
	context.bx = 21;
	context.al = 47;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(464);
	context.di = 160+23;
	context.bx = 0;
	context.al = 48;
	context.ah = 4;
	showframe(context);
	context.ds = context.data.word(464);
	context.di = 160+71;
	context.bx = 21;
	context.al = 47;
	context.ah = 4;
	showframe(context);
	return;
}

static inline void showicon(Context & context) {
	context._cmp(context.data.byte(184), 50);
	if (!context.flags.c()) goto isdream1;
	showpanel(context);
	showman(context);
	roomname(context);
	panelicons1(context);
	zoomicon(context);
	return;
isdream1:
	context.ds = context.data.word(464);
	context.di = 72;
	context.bx = 2;
	context.al = 45;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(464);
	context.di = 72+47;
	context.bx = 2;
	context.al = 46;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(464);
	context.di = 69-10;
	context.bx = 21;
	context.al = 49;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(464);
	context.di = 160+88;
	context.bx = 2;
	context.al = 45;
	context.ah = 4;
	showframe(context);
	context.ds = context.data.word(464);
	context.di = 160+43;
	context.bx = 2;
	context.al = 46;
	context.ah = 4;
	showframe(context);
	context.ds = context.data.word(464);
	context.di = 160+101;
	context.bx = 21;
	context.al = 49;
	context.ah = 4;
	showframe(context);
	middlepanel(context);
	return;
}

static inline void getunderzoom(Context & context) {
	context.di = 8+5;
	context.bx = 132+4;
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5);
	context.cl = 46;
	context.ch = 40;
	multiget(context);
	return;
}

static inline void walkintoroom(Context & context) {
	context._cmp(context.data.byte(9), 14);
	if (!context.flags.z()) goto notlair;
	context._cmp(context.data.byte(148), 22);
	if (!context.flags.z()) goto notlair;
	context.data.byte(478) = 1;
	context.data.byte(477) = 1;
	autosetwalk(context);
notlair:
	return;
}

static inline void isryanholding(Context & context) {
	context._sub(context.al, 'A');
	context._sub(context.ah, 'A');
	context._sub(context.cl, 'A');
	context._sub(context.ch, 'A');
	context.es = context.data.word(398);
	context.bx = 0+2080+30000;
	context.dl = 0;
searchinv:
	context._cmp(context.data.byte(context.bx+2), 4);
	if (!context.flags.z()) goto nofindininv;
	context._cmp(context.al, context.data.byte(context.bx+12));
	if (!context.flags.z()) goto nofindininv;
	context._cmp(context.ah, context.data.byte(context.bx+13));
	if (!context.flags.z()) goto nofindininv;
	context._cmp(context.cl, context.data.byte(context.bx+14));
	if (!context.flags.z()) goto nofindininv;
	context._cmp(context.ch, context.data.byte(context.bx+15));
	if (!context.flags.z()) goto nofindininv;
	context.al = context.dl;
	context._cmp(context.al, 114);
	return;
nofindininv:
	context._add(context.bx, 16);
	context._add(context.dl, 1);
	context._cmp(context.dl, 114);
	if (!context.flags.z()) goto searchinv;
	context.al = context.dl;
	context._cmp(context.al, 114);
	return;
}

static inline void findexobject(Context & context) {
	context._sub(context.al, 'A');
	context._sub(context.ah, 'A');
	context._sub(context.cl, 'A');
	context._sub(context.ch, 'A');
	context.es = context.data.word(398);
	context.bx = 0+2080+30000;
	context.dl = 0;
findexloop:
	context._cmp(context.al, context.data.byte(context.bx+12));
	if (!context.flags.z()) goto nofindex;
	context._cmp(context.ah, context.data.byte(context.bx+13));
	if (!context.flags.z()) goto nofindex;
	context._cmp(context.cl, context.data.byte(context.bx+14));
	if (!context.flags.z()) goto nofindex;
	context._cmp(context.ch, context.data.byte(context.bx+15));
	if (!context.flags.z()) goto nofindex;
	context.al = context.dl;
	return;
nofindex:
	context._add(context.bx, 16);
	context._add(context.dl, 1);
	context._cmp(context.dl, 114);
	if (!context.flags.z()) goto findexloop;
	context.al = context.dl;
	return;
}

static inline void getexad(Context & context) {
	context.ah = 0;
	context.bx = 16;
	context._mul(context.bx);
	context.bx = context.ax;
	context.es = context.data.word(398);
	context._add(context.bx, 0+2080+30000);
	return;
}

static inline void getanyaddir(Context & context) {
	context._cmp(context.ah, 4);
	if (context.flags.z()) goto isex3;
	context._cmp(context.ah, 2);
	if (context.flags.z()) goto isfree3;
	getsetad(context);
	return;
isfree3:
	getfreead(context);
	return;
isex3:
	getexad(context);
	return;
}

static inline void compare(Context & context) {
	context._sub(context.dl, 'A');
	context._sub(context.dh, 'A');
	context._sub(context.cl, 'A');
	context._sub(context.ch, 'A');
	context.push(context.cx);
	context.push(context.dx);
	getanyaddir(context);
	context.dx = context.pop();
	context.cx = context.pop();
	context._cmp(context.data.word(context.bx+12), context.cx);
	if (!context.flags.z()) goto comparefin;
	context._cmp(context.data.word(context.bx+14), context.dx);
comparefin:
	return;
}

static inline void setuptimeduse(Context & context) {
	context._cmp(context.data.word(328), 0);
	if (!context.flags.z()) goto cantsetup;
	context.data.byte(335) = context.bh;
	context.data.byte(336) = context.bl;
	context.data.word(330) = context.cx;
	context._add(context.dx, context.cx);
	context.data.word(328) = context.dx;
	context.bl = context.al;
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(454);
	context.cx = 66*2;
	context.ax = context.data.word(context.bx);
	context._add(context.ax, context.cx);
	context.bx = context.ax;
	context.data.word(332) = context.es;
	context.data.word(334) = context.bx;
cantsetup:
	return;
}

static inline void reminders(Context & context) {
	context._cmp(context.data.byte(184), 24);
	if (!context.flags.z()) goto notinedenslift;
	context._cmp(context.data.byte(148), 44);
	if (!context.flags.z()) goto notinedenslift;
	context._cmp(context.data.byte(2), 0);
	if (!context.flags.z()) goto notfirst;
	context.al = 'D';
	context.ah = 'K';
	context.cl = 'E';
	context.ch = 'Y';
	isryanholding(context);
	if (context.flags.z()) goto forgotone;
	context.al = 'C';
	context.ah = 'S';
	context.cl = 'H';
	context.ch = 'R';
	findexobject(context);
	context._cmp(context.al, 114);
	if (context.flags.z()) goto forgotone;
	context.ax = context.data.word(context.bx+2);
	context._cmp(context.al, 4);
	if (!context.flags.z()) goto forgotone;
	context._cmp(context.ah, 255);
	if (context.flags.z()) goto havegotcard;
	context.cl = 'P';
	context.ch = 'U';
	context.dl = 'R';
	context.dh = 'S';
	context._xchg(context.al, context.ah);
	compare(context);
	if (!context.flags.z()) goto forgotone;
havegotcard:
	context._add(context.data.byte(2), 1);
notfirst:
	return;
forgotone:
	context.al = 50;
	context.bl = 54;
	context.bh = 70;
	context.cx = 48;
	context.dx = 8;
	setuptimeduse(context);
	return;
notinedenslift:
	return;
}

static inline void afternewroom(Context & context) {
	context._cmp(context.data.byte(186), 0);
	if (context.flags.z()) goto notnew;
	context.data.word(328) = 0;
	createpanel(context);
	context.data.byte(100) = 0;
	findroominloc(context);
	context._cmp(context.data.byte(63), 1);
	if (context.flags.z()) goto ryansoff;
	context.al = context.data.byte(151);
	context._add(context.al, 12);
	context.ah = context.data.byte(152);
	context._add(context.ah, 12);
	findpathofpoint(context);
	context.data.byte(475) = context.dl;
	findxyfrompath(context);
	context.data.byte(187) = 1;
ryansoff:
	context.data.byte(62) = 1;
	drawfloor(context);
	context.data.word(98) = 160;
	context.data.byte(186) = 0;
	showicon(context);
	spriteupdate(context);
	printsprites(context);
	undertextline(context);
	reelsonscreen(context);
	mainscreen(context);
	getunderzoom(context);
	zoom(context);
	worktoscreenm(context);
	walkintoroom(context);
	reminders(context);
	atmospheres(context);
notnew:
	return;
}

static inline void readmouse3(Context & context) {
	context.ax = context.data.word(198);
	context.data.word(214) = context.ax;
	context.ax = context.data.word(200);
	context.data.word(216) = context.ax;
	mousecall(context);
	context.data.word(198) = context.cx;
	context.data.word(200) = context.dx;
	context.data.word(208) = context.bx;
	return;
}

static inline void readmouse4(Context & context) {
	context.ax = context.data.word(202);
	context.data.word(212) = context.ax;
	context.ax = context.data.word(198);
	context.data.word(214) = context.ax;
	context.ax = context.data.word(200);
	context.data.word(216) = context.ax;
	mousecall(context);
	context.data.word(198) = context.cx;
	context.data.word(200) = context.dx;
	context.ax = context.data.word(204);
	context._or(context.ax, context.data.word(206));
	context._or(context.ax, context.data.word(208));
	context._or(context.bx, context.ax);
	context.data.word(202) = context.bx;
	return;
}

static inline void dumpwatch(Context & context) {
	context._cmp(context.data.byte(266), 1);
	if (!context.flags.z()) goto nodumpwatch;
	context.di = 256;
	context.bx = 21;
	context.cl = 40;
	context.ch = 12;
	multidump(context);
	context.data.byte(266) = 0;
nodumpwatch:
	return;
}

static inline void screenupdate(Context & context) {
	newplace(context);
	mainscreen(context);
	animpointer(context);
	showpointer(context);
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) goto iswatchingmode;
	context._cmp(context.data.byte(188), 255);
	if (!context.flags.z()) goto finishearly;
iswatchingmode:
	vsync(context);
	readmouse1(context);
	dumppointer(context);
	dumptextline(context);
	delpointer(context);
	autolook(context);
	spriteupdate(context);
	watchcount(context);
	zoom(context);
	showpointer(context);
	context._cmp(context.data.byte(391), 0);
	if (!context.flags.z()) goto finishearly;
	vsync(context);
	readmouse2(context);
	dumppointer(context);
	dumpzoom(context);
	delpointer(context);
	deleverything(context);
	printsprites(context);
	reelsonscreen(context);
	afternewroom(context);
	showpointer(context);
	vsync(context);
	readmouse3(context);
	dumppointer(context);
	dumpmap(context);
	dumptimedtext(context);
	delpointer(context);
	showpointer(context);
	vsync(context);
	readmouse4(context);
	dumppointer(context);
	dumpwatch(context);
	delpointer(context);
finishearly:
	return;
}

static inline void greyscalesum(Context & context) {
	context.es = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768;
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768;
	context.cx = 256;
greysumloop1:
	context.push(context.cx);
	context.bx = 0;
	context.al = context.data.byte(context.si);
	context.ah = 0;
	context.cx = 20;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.si+1);
	context.ah = 0;
	context.cx = 59;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.si+2);
	context.ah = 0;
	context.cx = 11;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.al = -1;
greysumloop2:
	context._add(context.al, 1);
	context._sub(context.bx, 100);
	if (!context.flags.c()) goto greysumloop2;
	context.bl = context.al;
	context.al = context.bl;
	context.ah = context.data.byte(348);
	context._cmp(context.al, 0);
	context._add(context.al, context.ah);
noaddr:
	context._stosb();
	context.ah = context.data.byte(347);
	context.al = context.bl;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto noaddg;
	context._add(context.al, context.ah);
noaddg:
	context._stosb();
	context.ah = context.data.byte(349);
	context.al = context.bl;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto noaddb;
	context._add(context.al, context.ah);
noaddb:
	context._stosb();
	context._add(context.si, 3);
	context.cx = context.pop();
	if (--context.cx) goto greysumloop1;
	return;
}

static inline void endpaltostart(Context & context) {
	context.es = context.data.word(412);
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768;
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3);
	context.cx = 768/2;
	while(--context.cx) 	context._movsw();
 	return;
}

static inline void createpanel(Context & context) {
	context.di = 0;
	context.bx = 8;
	context.ds = context.data.word(410);
	context.al = 0;
	context.ah = 2;
	showframe(context);
	context.di = 160;
	context.bx = 8;
	context.ds = context.data.word(410);
	context.al = 0;
	context.ah = 2;
	showframe(context);
	context.di = 0;
	context.bx = 104;
	context.ds = context.data.word(410);
	context.al = 0;
	context.ah = 2;
	showframe(context);
	context.di = 160;
	context.bx = 104;
	context.ds = context.data.word(410);
	context.al = 0;
	context.ah = 2;
	showframe(context);
	return;
}

static inline void createpanel2(Context & context) {
	createpanel(context);
	context.di = 0;
	context.bx = 0;
	context.ds = context.data.word(410);
	context.al = 5;
	context.ah = 2;
	showframe(context);
	context.di = 160;
	context.bx = 0;
	context.ds = context.data.word(410);
	context.al = 5;
	context.ah = 2;
	showframe(context);
	return;
}

static inline void fadescreenup(Context & context) {
	clearstartpal(context);
	paltoendpal(context);
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 128;
	return;
}

static inline void hangone(Context & context) {
hangonloope:
	context.push(context.cx);
	vsync(context);
	context.cx = context.pop();
	context._cmp(context.data.byte(392), 1);
	if (context.flags.z()) goto hangonearly;
	if (--context.cx) goto hangonloope;
	return;
hangonearly:
	context.ax = context.pop();
	context.ax = context.pop();
	return;
}

static inline void rollem(Context & context) {
	context.cl = 160;
	context.ch = 160;
	context.di = 25;
	context.bx = 20;
	context.ds = context.data.word(402);
	context.si = 0;
	multiget(context);
	context.es = context.data.word(466);
	context.si = 49*2;
	context.ax = context.data.word(context.si);
	context.si = context.ax;
	context._add(context.si, 66*2);
	context.cx = 80;
endcredits21:
	context.push(context.cx);
	context.bx = 10;
	context.cx = context.data.word(77);
endcredits22:
	context.push(context.cx);
	context.push(context.si);
	context.push(context.di);
	context.push(context.es);
	context.push(context.bx);
	vsync(context);
	context.cl = 160;
	context.ch = 160;
	context.di = 25;
	context.bx = 20;
	context.ds = context.data.word(402);
	context.si = 0;
	multiput(context);
	vsync(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.di = context.pop();
	context.si = context.pop();
	context.push(context.si);
	context.push(context.di);
	context.push(context.es);
	context.push(context.bx);
	context.cx = 18;
onelot2:
	context.push(context.cx);
	context.di = 25;
	context.dx = 161;
	context.ax = 0;
	printdirect(context);
	context._add(context.bx, context.data.word(77));
	context.cx = context.pop();
	if (--context.cx) goto onelot2;
	vsync(context);
	context.cl = 160;
	context.ch = 160;
	context.di = 25;
	context.bx = 20;
	multidump(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.di = context.pop();
	context.si = context.pop();
	context.cx = context.pop();
	context._cmp(context.data.byte(392), 1);
	if (context.flags.z()) goto endearly2;
	context._sub(context.bx, 1);
	if (--context.cx) goto endcredits22;
	context.cx = context.pop();
looknext2:
	context.al = context.data.byte(context.si);
	context._add(context.si, 1);
	context._cmp(context.al, ':');
	if (context.flags.z()) goto gotnext2;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto gotnext2;
	goto looknext2;
gotnext2:
	context._cmp(context.data.byte(392), 1);
	if (context.flags.z()) goto endearly;
	if (--context.cx) goto endcredits21;
	context.cx = 120;
	hangone(context);
	return;
endearly2:
	context.cx = context.pop();
endearly:
	return;
}

static inline void rollendcredits2(Context & context) {
	rollem(context);
	return;
}

static inline void showgun(Context & context) {
	context.data.byte(348) = 0;
	context.data.byte(347) = 0;
	context.data.byte(349) = 0;
	paltostartpal(context);
	paltoendpal(context);
	greyscalesum(context);
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 128;
	context.cx = 130;
	hangon(context);
	endpaltostart(context);
	clearendpal(context);
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 128;
	context.cx = 200;
	hangon(context);
	context.data.byte(531) = 34;
	loadroomssample(context);
	context.data.byte(386) = 0;
	context.dx = 2364;
	loadintotemp(context);
	createpanel2(context);
	context.ds = context.data.word(458);
	context.al = 0;
	context.ah = 0;
	context.di = 100;
	context.bx = 4;
	showframe(context);
	context.ds = context.data.word(458);
	context.al = 1;
	context.ah = 0;
	context.di = 158;
	context.bx = 106;
	showframe(context);
	worktoscreen(context);
	getridoftemp(context);
	fadescreenup(context);
	context.cx = 160;
	hangon(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.dx = 2273;
	loadtemptext(context);
	rollendcredits2(context);
	getridoftemptext(context);
	return;
}

static inline void fadescreendown(Context & context) {
	paltostartpal(context);
	clearendpal(context);
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 128;
	return;
}

static inline void hangon(Context & context) {
hangonloop:
	context.push(context.cx);
	vsync(context);
	context.cx = context.pop();
	if (--context.cx) goto hangonloop;
	return;
}

static inline void paltostartpal(Context & context) {
	context.es = context.data.word(412);
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768;
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3);
	context.cx = 768/2;
	while(--context.cx) 	context._movsw();
 	return;
}

static inline void clearendpal(Context & context) {
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768;
	context.cx = 768;
	context.al = 0;
	while(--context.cx) 	context._stosb();
	return;
}

static inline void fadescreendowns(Context & context) {
	paltostartpal(context);
	clearendpal(context);
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 64;
	return;
}

static inline void loadtemptext(Context & context) {
	standardload(context);
	context.data.word(466) = context.ax;
	return;
}

static inline void twodigitnum(Context & context) {
	context.ah = context.cl;
	context._sub(context.ah, 1);
numloop1:
	context._add(context.ah, 1);
	context._sub(context.al, 10);
	if (!context.flags.c()) goto numloop1;
	context._add(context.al, 10);
	context._add(context.al, context.cl);
	return;
}

static inline void cancelch1(Context & context) {
	context.data.word(521) = 0;
	context.data.byte(515) = 255;
	return;
}

static inline void loadroomssample(Context & context) {
	context.al = context.data.byte(531);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto loadedalready;
	context._cmp(context.al, context.data.byte(530));
	if (context.flags.z()) goto loadedalready;
	context.data.byte(530) = context.al;
	context.al = context.data.byte(530);
	context.cl = '0';
	twodigitnum(context);
	context.di = 1909;
	context._xchg(context.al, context.ah);
	context.data.word(context.di+10) = context.ax;
	context.dx = context.di;
	loadsecondsample(context);
loadedalready:
	return;
}

static inline void readheader(Context & context) {
	context.ds = context.cs;
	context.dx = 5881;
	context.cx = 5958-context.data.byte(5881);
	readfromfile(context);
	context.es = context.cs;
	context.di = 5952;
	return;
}

static inline void standardload(Context & context) {
	openfile(context);
	readheader(context);
	context.bx = context.data.word(context.di);
	context.push(context.bx);
	context.cl = 4;
	context._shr(context.bx, context.cl);
	allocatemem(context);
	context.ds = context.ax;
	context.cx = context.pop();
	context.push(context.ax);
	context.dx = 0;
	readfromfile(context);
	closefile(context);
	context.ax = context.pop();
	return;
}

static inline void loadintotemp(Context & context) {
	context.ds = context.cs;
	standardload(context);
	context.data.word(458) = context.ax;
	return;
}

static inline void showmonk(Context & context) {
	context.al = 0;
	context.ah = 128;
	context.di = 160;
	context.bx = 72;
	context.ds = context.data.word(458);
	showframe(context);
	return;
}

static inline void getridoftemp(Context & context) {
	context.es = context.data.word(458);
	deallocatemem(context);
	return;
}

static inline void monkspeaking(Context & context) {
	context.data.byte(531) = 35;
	loadroomssample(context);
	context.dx = 2377;
	loadintotemp(context);
	clearwork(context);
	showmonk(context);
	worktoscreen(context);
	context.data.byte(386) = 7;
	context.data.byte(388) = -1;
	context.data.byte(387) = 0;
	context.al = 12;
	context.ah = 255;
	playchannel0(context);
	fadescreenups(context);
	context.cx = 300;
	hangon(context);
	context.al = 40;
nextmonkspeak:
	context.push(context.ax);
	context.ah = 0;
	context.si = context.ax;
	context._add(context.si, context.si);
	context.es = context.data.word(466);
	context.ax = context.data.word(context.si);
	context._add(context.ax, 66*2);
	context.si = context.ax;
nextbit:
	context.di = 36;
	context.bx = 140;
	context.dl = 239;
	printdirect(context);
	context.push(context.ax);
	context.push(context.si);
	context.push(context.es);
	worktoscreen(context);
	clearwork(context);
	showmonk(context);
	context.cx = 240;
	hangon(context);
	context.es = context.pop();
	context.si = context.pop();
	context.ax = context.pop();
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto nextbit;
	context.ax = context.pop();
	context._add(context.al, 1);
	context._cmp(context.al, 44);
	if (!context.flags.z()) goto nextmonkspeak;
	context.data.byte(388) = 1;
	context.data.byte(387) = 7;
	fadescreendowns(context);
	context.cx = 300;
	hangon(context);
	getridoftemp(context);
	return;
}

static inline void loadintroroom(Context & context) {
	context.data.byte(139) = 0;
	context.data.byte(9) = 255;
	loadroom(context);
	context.data.word(121) = 72;
	context.data.word(123) = 16;
	clearsprites(context);
	context.data.byte(61) = 0;
	context.data.byte(142) = '0';
	context.data.byte(105) = 0;
	clearwork(context);
	context.data.byte(62) = 1;
	drawfloor(context);
	reelsonscreen(context);
	spriteupdate(context);
	printsprites(context);
	worktoscreen(context);
	return;
}

static inline void clearstartpal(Context & context) {
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3);
	context.cx = 256;
wholeloop1:
	context.ax = 0;
	context._stosw();
	context.al = 0;
	context._stosb();
	if (--context.cx) goto wholeloop1;
	return;
}

static inline void paltoendpal(Context & context) {
	context.es = context.data.word(412);
	context.ds = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768;
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768;
	context.cx = 768/2;
	while(--context.cx) 	context._movsw();
 	return;
}

static inline void fadescreenups(Context & context) {
	clearstartpal(context);
	paltoendpal(context);
	context.data.byte(344) = 1;
	context.data.byte(346) = 63;
	context.data.byte(343) = 0;
	context.data.byte(345) = 64;
	return;
}

static inline void playchannel0(Context & context) {
	context._cmp(context.data.byte(378), 255);
	if (context.flags.z()) goto dontbother4;
	context.push(context.es);
	context.push(context.ds);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.di);
	context.push(context.si);
	context.data.byte(507) = context.al;
	context.es = context.data.word(420);
	context._cmp(context.al, 12);
	if (context.flags.c()) goto notsecondbank;
	context.es = context.data.word(422);
	context._sub(context.al, 12);
notsecondbank:
	context.data.byte(508) = context.ah;
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.bx);
	context.ah = 0;
	context.data.word(502) = context.ax;
	context.ax = context.data.word(context.bx+1);
	context.data.word(504) = context.ax;
	context.ax = context.data.word(context.bx+3);
	context.data.word(506) = context.ax;
	context._cmp(context.data.byte(508), 0);
	if (context.flags.z()) goto nosetloop;
	context.ax = context.data.word(502);
	context.data.word(510) = context.ax;
	context.ax = context.data.word(504);
	context.data.word(512) = context.ax;
	context.ax = context.data.word(506);
	context.data.word(514) = context.ax;
nosetloop:
	context.si = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ds = context.pop();
	context.es = context.pop();
dontbother4:
	return;
}

static inline void cancelch0(Context & context) {
	context.data.byte(508) = 0;
	context.data.word(506) = 0;
	context.data.byte(507) = 255;
	return;
}

static inline void atmospheres(Context & context) {
	context.cl = context.data.byte(148);
	context.ch = context.data.byte(149);
	context.bx = 5073;
nextatmos:
	context.al = context.data.byte(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto nomoreatmos;
	context._cmp(context.al, context.data.byte(184));
	if (!context.flags.z()) goto wrongatmos;
	context.ax = context.data.word(context.bx+1);
	context._cmp(context.ax, context.cx);
	if (!context.flags.z()) goto wrongatmos;
	context.ax = context.data.word(context.bx+3);
	context._cmp(context.al, context.data.byte(507));
	if (context.flags.z()) goto playingalready;
	context._cmp(context.data.byte(9), 45);
	if (!context.flags.z()) goto notweb;
	context._cmp(context.data.word(23), 45);
	if (context.flags.z()) goto wrongatmos;
notweb:
	playchannel0(context);
	context._cmp(context.data.byte(184), 2);
	context._cmp(context.data.byte(149), 0);
	if (context.flags.z()) goto fullvol;
	if (!context.flags.z()) goto notlouisvol;
	context._cmp(context.data.byte(149), 10);
	if (!context.flags.z()) goto notlouisvol;
	context._cmp(context.data.byte(148), 22);
	if (!context.flags.z()) goto notlouisvol;
	context.data.byte(386) = 5;
notlouisvol:
playingalready:
	context._cmp(context.data.byte(184), 2);
	if (!context.flags.z()) goto notlouisvol2;
	context._cmp(context.data.byte(148), 22);
	if (context.flags.z()) goto louisvol;
	context._cmp(context.data.byte(148), 11);
	if (!context.flags.z()) goto notlouisvol2;
fullvol:
	context.data.byte(386) = 0;
notlouisvol2:
	return;
louisvol:
	context.data.byte(386) = 5;
	return;
wrongatmos:
	context._add(context.bx, 5);
	goto nextatmos;
nomoreatmos:
	cancelch0(context);
	return;
}

static inline void spriteupdate(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768;
	context.al = context.data.byte(63);
	context.data.byte(context.bx+31) = context.al;
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768;
	context.cx = 16;
_tmp18:
	context.push(context.cx);
	context.push(context.bx);
	context.ax = context.data.word(context.bx);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto _tmp18a;
	context.push(context.es);
	context.push(context.ds);
	context.cx = context.data.word(context.bx+2);
	context.data.word(context.bx+24) = context.cx;
	__dispatch_call(context, context.ax);
	context.ds = context.pop();
	context.es = context.pop();
_tmp18a:
	context.bx = context.pop();
	context.cx = context.pop();
	context._cmp(context.data.byte(186), 1);
	if (context.flags.z()) goto _tmp18b;
	context._add(context.bx, 32);
	if (--context.cx) goto _tmp18;
_tmp18b:
	return;
}

static inline void maptopanel(Context & context) {
	context.di = context.data.word(125);
	context._add(context.di, context.data.word(117));
	context.bx = context.data.word(127);
	context._add(context.bx, context.data.word(119));
	context.ds = context.data.word(402);
	context.si = 0;
	context.cl = context.data.byte(128);
	context.ch = context.data.byte(129);
	multiput(context);
	return;
}

static inline void deleverything(Context & context) {
	context.al = context.data.byte(129);
	context.ah = 0;
	context._add(context.ax, context.data.word(123));
	context._cmp(context.ax, 182);
	if (!context.flags.c()) goto bigroom;
	maptopanel(context);
	return;
bigroom:
	context._sub(context.data.byte(129), 8);
	maptopanel(context);
	context._add(context.data.byte(129), 8);
	return;
}

static inline void printasprite(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context.si = context.bx;
	context.ds = context.data.word(context.si+6);
	context.al = context.data.byte(context.si+11);
	context.ah = 0;
	context._cmp(context.al, 220);
	if (context.flags.c()) goto notnegative1;
	context.ah = 255;
notnegative1:
	context.bx = context.ax;
	context._add(context.bx, context.data.word(119));
	context.al = context.data.byte(context.si+10);
	context.ah = 0;
	context._cmp(context.al, 220);
	if (context.flags.c()) goto notnegative2;
	context.ah = 255;
notnegative2:
	context.di = context.ax;
	context._add(context.di, context.data.word(117));
	context.al = context.data.byte(context.si+15);
	context.ah = 0;
	context._cmp(context.data.byte(context.si+30), 0);
	if (context.flags.z()) goto steadyframe;
	context.ah = 8;
steadyframe:
	context._cmp(context.data.byte(182), 6);
	if (!context.flags.z()) goto notquickp;
notquickp:
	showframe(context);
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void printsprites(Context & context) {
	context.es = context.data.word(412);
	context.cx = 0;
priorityloop:
	context.push(context.cx);
	context.data.byte(182) = context.cl;
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768;
	context.cx = 16;
prtspriteloop:
	context.push(context.cx);
	context.push(context.bx);
	context.ax = context.data.word(context.bx);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto skipsprite;
	context.al = context.data.byte(182);
	context._cmp(context.al, context.data.byte(context.bx+23));
	if (!context.flags.z()) goto skipsprite;
	context._cmp(context.data.byte(context.bx+31), 1);
	if (context.flags.z()) goto skipsprite;
	printasprite(context);
skipsprite:
	context.bx = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 32);
	if (--context.cx) goto prtspriteloop;
	context.cx = context.pop();
	context._add(context.cx, 1);
	context._cmp(context.cx, 7);
	if (!context.flags.z()) goto priorityloop;
	return;
}

static inline void undertextline(Context & context) {
	context.di = context.data.word(79);
	context.bx = context.data.word(81);
	context._sub(context.bx, 3);
	context.ds = context.data.word(412);
	context.si = 0;
	context.cl = 228;
	context.ch = 13;
	multiget(context);
	return;
}

static inline void reconstruct(Context & context) {
	context._cmp(context.data.byte(130), 0);
	if (context.flags.z()) goto noneedtorecon;
	context.data.byte(62) = 1;
	drawfloor(context);
	spriteupdate(context);
	printsprites(context);
	context._cmp(context.data.byte(184), 20);
	if (!context.flags.z()) goto notfudge;
	undertextline(context);
notfudge:
	context.data.byte(130) = 0;
noneedtorecon:
	return;
}

static inline void updatepeople(Context & context) {
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5);
	context.data.word(18) = context.di;
	context.cx = 12*5;
	context.al = 255;
	while(--context.cx) 	context._stosb();
	context._add(context.data.word(138), 1);
	context.es = context.cs;
	context.bx = 537;
	context.di = 1003;
updateloop:
	context.al = context.data.byte(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto endupdate;
	context._cmp(context.al, context.data.byte(184));
	if (!context.flags.z()) goto notinthisroom;
	context.cx = context.data.word(context.bx+1);
	context._cmp(context.cl, context.data.byte(148));
	if (!context.flags.z()) goto notinthisroom;
	context._cmp(context.ch, context.data.byte(149));
	if (!context.flags.z()) goto notinthisroom;
	context.push(context.di);
	context.ax = context.data.word(context.di);
	__dispatch_call(context, context.ax);
	context.di = context.pop();
notinthisroom:
	context._add(context.bx, 8);
	context._add(context.di, 2);
	goto updateloop;
endupdate:
	return;
}

static inline void getreelstart(Context & context) {
	context.ax = context.data.word(239);
	context.cx = 40;
	context._mul(context.cx);
	context.es = context.data.word(450);
	context.si = context.ax;
	context._add(context.si, 0+(36*144));
	return;
}

static inline void placesetobject(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context.cl = 0;
	context.ch = 0;
	findormake(context);
	getsetad(context);
	context.data.byte(context.bx+58) = 0;
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void getsetad(Context & context) {
	context.ah = 0;
	context.bx = 64;
	context._mul(context.bx);
	context.bx = context.ax;
	context.es = context.data.word(428);
	return;
}

static inline void removesetobject(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context.cl = 255;
	context.ch = 0;
	findormake(context);
	getsetad(context);
	context.data.byte(context.bx+58) = 255;
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void findormake(Context & context) {
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80);
	context.push(context.ax);
	context.es = context.data.word(412);
	context.ah = context.data.byte(184);
changeloop:
	context._cmp(context.data.byte(context.bx), 255);
	if (context.flags.z()) goto haventfound;
	context._cmp(context.ax, context.data.word(context.bx));
	if (!context.flags.z()) goto nofoundchange;
	context._cmp(context.ch, context.data.byte(context.bx+3));
	if (context.flags.z()) goto foundchange;
nofoundchange:
	context._add(context.bx, 4);
	goto changeloop;
foundchange:
	context.ax = context.pop();
	context.data.byte(context.bx+2) = context.cl;
	return;
haventfound:
	context.data.word(context.bx) = context.ax;
	context.data.word(context.bx+2) = context.cx;
	context.ax = context.pop();
	return;
}

static inline void placefreeobject(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	context.cl = 0;
	context.ch = 1;
	findormake(context);
	getfreead(context);
	context.data.byte(context.bx+2) = 0;
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void getfreead(Context & context) {
	context.ah = 0;
	context.cl = 4;
	context._shl(context.ax, context.cl);
	context.bx = context.ax;
	context.es = context.data.word(426);
	return;
}

static inline void removefreeobject(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	getfreead(context);
	context.data.byte(context.bx+2) = 255;
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void switchryanoff(Context & context) {
	context.data.byte(63) = 1;
	return;
}

static inline void switchryanon(Context & context) {
	context.data.byte(63) = 255;
	return;
}

static inline void movemap(Context & context) {
	context._cmp(context.ah, 32);
	if (!context.flags.z()) goto notmapup2;
	context._sub(context.data.byte(149), 20);
	context.data.byte(186) = 1;
	return;
notmapup2:
	context._cmp(context.ah, 16);
	if (!context.flags.z()) goto notmapupspec;
	context._sub(context.data.byte(149), 10);
	context.data.byte(186) = 1;
	return;
notmapupspec:
	context._cmp(context.ah, 8);
	if (!context.flags.z()) goto notmapdownspec;
	context._add(context.data.byte(149), 10);
	context.data.byte(186) = 1;
	return;
notmapdownspec:
	context._cmp(context.ah, 2);
	if (!context.flags.z()) goto notmaprightspec;
	context._add(context.data.byte(148), 11);
	context.data.byte(186) = 1;
	return;
notmaprightspec:
	context._sub(context.data.byte(148), 11);
	context.data.byte(186) = 1;
	return;
}

static inline void dealwithspecial(Context & context) {
	context._sub(context.al, 220);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto notplset;
	context.al = context.ah;
	placesetobject(context);
	context.data.byte(130) = 1;
	return;
notplset:
	context._cmp(context.al, 1);
	if (!context.flags.z()) goto notremset;
	context.al = context.ah;
	removesetobject(context);
	context.data.byte(130) = 1;
	return;
notremset:
	context._cmp(context.al, 2);
	if (!context.flags.z()) goto notplfree;
	context.al = context.ah;
	placefreeobject(context);
	context.data.byte(130) = 1;
	return;
notplfree:
	context._cmp(context.al, 3);
	if (!context.flags.z()) goto notremfree;
	context.al = context.ah;
	removefreeobject(context);
	context.data.byte(130) = 1;
	return;
notremfree:
	context._cmp(context.al, 4);
	if (!context.flags.z()) goto notryanoff;
	switchryanoff(context);
	return;
notryanoff:
	context._cmp(context.al, 5);
	if (!context.flags.z()) goto notryanon;
	context.data.byte(135) = context.ah;
	context.data.byte(133) = context.ah;
	switchryanon(context);
	return;
notryanon:
	context._cmp(context.al, 6);
	if (!context.flags.z()) goto notchangeloc;
	context.data.byte(188) = context.ah;
	return;
notchangeloc:
	movemap(context);
	return;
}

static inline void findsource(Context & context) {
	context.ax = context.data.word(160);
	context._cmp(context.ax, 160);
	if (!context.flags.c()) goto over1000;
	context.ds = context.data.word(430);
	context.data.word(244) = 0;
	return;
over1000:
	context._cmp(context.ax, 320);
	if (!context.flags.c()) goto over1001;
	context.ds = context.data.word(432);
	context.data.word(244) = 160;
	return;
over1001:
	context.ds = context.data.word(434);
	context.data.word(244) = 320;
	return;
}

static inline void showreelframe(Context & context) {
	context.al = context.data.byte(context.si+2);
	context.ah = 0;
	context.di = context.ax;
	context._add(context.di, context.data.word(117));
	context.al = context.data.byte(context.si+3);
	context.bx = context.ax;
	context._add(context.bx, context.data.word(119));
	context.ax = context.data.word(context.si);
	context.data.word(160) = context.ax;
	findsource(context);
	context.ax = context.data.word(160);
	context._sub(context.ax, context.data.word(244));
	context.ah = 8;
	showframe(context);
	return;
}

static inline void soundonreels(Context & context) {
	context.bl = context.data.byte(184);
	context._add(context.bl, context.bl);
	context._xor(context.bh, context.bh);
	context._add(context.bx, 1254);
	context.si = context.data.word(context.bx);
reelsoundloop:
	context.al = context.data.byte(context.si);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto endreelsound;
	context.ax = context.data.word(context.si+1);
	context._cmp(context.ax, context.data.word(239));
	if (!context.flags.z()) goto skipreelsound;
	context._cmp(context.ax, context.data.word(351));
	if (context.flags.z()) goto skipreelsound;
	context.data.word(351) = context.ax;
	context.al = context.data.byte(context.si);
	context._cmp(context.al, 64);
	if (context.flags.c()) { playchannel1(context); return; };
	context._cmp(context.al, 128);
	if (context.flags.c()) goto channel0once;
	context._and(context.al, 63);
	context.ah = 255;
	{ playchannel0(context); return; };
channel0once:
	context._and(context.al, 63);
	context.ah = 0;
	{ playchannel0(context); return; };
skipreelsound:
	context._add(context.si, 3);
	goto reelsoundloop;
endreelsound:
	context.ax = context.data.word(351);
	context._cmp(context.ax, context.data.word(239));
	if (context.flags.z()) goto nochange2;
	context.data.word(351) = -1;
nochange2:
	return;
}

static inline void plotreel(Context & context) {
	getreelstart(context);
retryreel:
	context.push(context.es);
	context.push(context.si);
	context.ax = context.data.word(context.si+2);
	context._cmp(context.al, 220);
	if (context.flags.c()) goto normalreel;
	context._cmp(context.al, 255);
	if (context.flags.z()) goto normalreel;
	dealwithspecial(context);
	context._add(context.data.word(239), 1);
	context.si = context.pop();
	context.es = context.pop();
	context._add(context.si, 40);
	goto retryreel;
normalreel:
	context.cx = 8;
plotloop:
	context.push(context.cx);
	context.push(context.es);
	context.push(context.si);
	context.ax = context.data.word(context.si);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto notplot;
	showreelframe(context);
notplot:
	context.si = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context._add(context.si, 5);
	if (--context.cx) goto plotloop;
	soundonreels(context);
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void checkforshake(Context & context) {
	context._cmp(context.data.byte(184), 26);
	if (!context.flags.z()) goto notstartshake;
	context._cmp(context.ax, 104);
	if (!context.flags.z()) goto notstartshake;
	context.data.byte(68) = -1;
notstartshake:
	return;
}

static inline void getroomspaths(Context & context) {
	context.al = context.data.byte(185);
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context.es = context.data.word(450);
	context.bx = 0;
	context._add(context.bx, context.ax);
	return;
}

static inline void checkdest(Context & context) {
	context.push(context.bx);
	context._add(context.bx, 12*8);
	context.ah = context.data.byte(475);
	context.cl = 4;
	context._shl(context.ah, context.cl);
	context.al = context.data.byte(478);
	context.cl = 24;
	context.ch = context.data.byte(478);
checkdestloop:
	context.dh = context.data.byte(context.bx);
	context._and(context.dh, 0b11110000);
	context.dl = context.data.byte(context.bx);
	context._and(context.dl, 0b00001111);
	context._cmp(context.ax, context.dx);
	if (!context.flags.z()) goto nextcheck;
	context.al = context.data.byte(context.bx+1);
	context._and(context.al, 15);
	context.data.byte(478) = context.al;
	context.bx = context.pop();
	return;
nextcheck:
	context.dl = context.data.byte(context.bx);
	context._and(context.dl, 0b11110000);
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	context.dh = context.data.byte(context.bx);
	context._and(context.dh, 0b00001111);
	context._shl(context.dh, 1);
	context._shl(context.dh, 1);
	context._shl(context.dh, 1);
	context._shl(context.dh, 1);
	context._cmp(context.ax, context.dx);
	if (!context.flags.z()) goto nextcheck2;
	context.ch = context.data.byte(context.bx+1);
	context._and(context.ch, 15);
nextcheck2:
	context._add(context.bx, 2);
	context._sub(context.cl, 1);
	if (!context.flags.z()) goto checkdestloop;
	context.data.byte(478) = context.ch;
	context.bx = context.pop();
	return;
}

static inline void workoutframes(Context & context) {
	context.bx = context.data.word(480);
	context._add(context.bx, 32);
	context.ax = context.data.word(484);
	context._add(context.ax, 32);
	context._sub(context.bx, context.ax);
	if (!context.flags.c()) goto notneg1;
	context._neg(context.bx);
notneg1:
	context.cx = context.data.word(482);
	context._add(context.cx, 32);
	context.ax = context.data.word(486);
	context._add(context.ax, 32);
	context._sub(context.cx, context.ax);
	if (!context.flags.c()) goto notneg2;
	context._neg(context.cx);
notneg2:
	context._cmp(context.bx, context.cx);
	if (!context.flags.c()) goto tendstohoriz;
	context.dl = 2;
	context.ax = context.cx;
	context._shr(context.ax, 1);
	context._cmp(context.bx, context.ax);
	if (context.flags.c()) goto gotquad;
	context.dl = 1;
	goto gotquad;
tendstohoriz:
	context.dl = 0;
	context.ax = context.bx;
	context._shr(context.ax, 1);
	context._cmp(context.cx, context.ax);
	if (context.flags.c()) goto gotquad;
	context.dl = 1;
	goto gotquad;
gotquad:
	context.bx = context.data.word(480);
	context._add(context.bx, 32);
	context.ax = context.data.word(484);
	context._add(context.ax, 32);
	context._sub(context.bx, context.ax);
	if (context.flags.c()) goto isinright;
isinleft:
	context.cx = context.data.word(482);
	context._add(context.cx, 32);
	context.ax = context.data.word(486);
	context._add(context.ax, 32);
	context._sub(context.cx, context.ax);
	if (!context.flags.c()) goto topleft;
	context._cmp(context.dl, 1);
	if (context.flags.z()) goto noswap1;
	context._xor(context.dl, 2);
noswap1:
	context._add(context.dl, 4);
	goto success;
topleft:
	context._add(context.dl, 6);
	goto success;
isinright:
	context.cx = context.data.word(482);
	context._add(context.cx, 32);
	context.ax = context.data.word(486);
	context._add(context.ax, 32);
	context._sub(context.cx, context.ax);
	if (!context.flags.c()) goto botright;
	context._add(context.dl, 2);
	goto success;
botright:
	context._cmp(context.dl, 1);
	if (context.flags.z()) goto noswap2;
	context._xor(context.dl, 2);
noswap2:
success:
	context._and(context.dl, 7);
	context.data.byte(135) = context.dl;
	context.data.byte(136) = 0;
	return;
}

static inline void bresenhams(Context & context) {
	workoutframes(context);
	context.dx = context.ds;
	context.es = context.dx;
	context.di = 8344;
	context.si = 1;
	context.data.byte(493) = 0;
	context.cx = context.data.word(484);
	context._sub(context.cx, context.data.word(480));
	if (context.flags.z()) goto vertline;
	if (!context.flags.s()) goto line1;
	context._neg(context.cx);
	context.bx = context.data.word(484);
	context._xchg(context.bx, context.data.word(480));
	context.data.word(484) = context.bx;
	context.bx = context.data.word(486);
	context._xchg(context.bx, context.data.word(482));
	context.data.word(486) = context.bx;
	context.data.byte(493) = 1;
line1:
	context.bx = context.data.word(486);
	context._sub(context.bx, context.data.word(482));
	if (context.flags.z()) goto horizline;
	if (!context.flags.s()) goto line3;
	context._neg(context.bx);
	context._neg(context.si);
line3:
	context.push(context.si);
	context.data.byte(491) = 0;
	context._cmp(context.bx, context.cx);
	if (context.flags.le()) goto line4;
	context.data.byte(491) = 1;
	context._xchg(context.bx, context.cx);
line4:
	context._shl(context.bx, 1);
	context.data.word(488) = context.bx;
	context._sub(context.bx, context.cx);
	context.si = context.bx;
	context._sub(context.bx, context.cx);
	context.data.word(490) = context.bx;
	context.ax = context.data.word(480);
	context.bx = context.data.word(482);
	context.ah = context.bl;
	context._add(context.cx, 1);
	context.bx = context.pop();
	context._cmp(context.data.byte(491), 1);
	if (context.flags.z()) goto hislope;
	goto loslope;
vertline:
	context.ax = context.data.word(482);
	context.bx = context.data.word(486);
	context.cx = context.bx;
	context._sub(context.cx, context.ax);
	if (!context.flags.ge()) goto line31;
	context._neg(context.cx);
	context.ax = context.bx;
	context.data.byte(493) = 1;
line31:
	context._add(context.cx, 1);
	context.bx = context.data.word(480);
	context._xchg(context.ax, context.bx);
	context.ah = context.bl;
	context.bx = context.si;
line32:
	context._stosw();
	context._add(context.ah, context.bl);
	if (--context.cx) goto line32;
	goto lineexit;
horizline:
	context.ax = context.data.word(480);
	context.bx = context.data.word(482);
	context.ah = context.bl;
	context._add(context.cx, 1);
horizloop:
	context._stosw();
	context._add(context.al, 1);
	if (--context.cx) goto horizloop;
	goto lineexit;
loslope:
loloop:
	context._stosw();
	context._add(context.al, 1);
	context._or(context.si, context.si);
	if (!context.flags.s()) goto line12;
	context._add(context.si, context.data.word(488));
	if (--context.cx) goto loloop;
	goto lineexit;
line12:
	context._add(context.si, context.data.word(490));
	context._add(context.ah, context.bl);
	if (--context.cx) goto loloop;
	goto lineexit;
hislope:
hiloop:
	context._stosw();
	context._add(context.ah, context.bl);
	context._or(context.si, context.si);
	if (!context.flags.s()) goto line23;
	context._add(context.si, context.data.word(488));
	if (--context.cx) goto hiloop;
	goto lineexit;
line23:
	context._add(context.si, context.data.word(490));
	context._add(context.al, 1);
	if (--context.cx) goto hiloop;
lineexit:
	context._sub(context.di, 8344);
	context.ax = context.di;
	context._shr(context.ax, 1);
	context.data.byte(494) = context.al;
	return;
}

static inline void autosetwalk(Context & context) {
	context.al = context.data.byte(475);
	context._cmp(context.data.byte(477), context.al);
	if (!context.flags.z()) goto notsamealready;
	return;
notsamealready:
	getroomspaths(context);
	checkdest(context);
	context.push(context.bx);
	context.al = context.data.byte(475);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.bx);
	context.ah = 0;
	context._sub(context.ax, 12);
	context.data.word(480) = context.ax;
	context.al = context.data.byte(context.bx+1);
	context.ah = 0;
	context._sub(context.ax, 12);
	context.data.word(482) = context.ax;
	context.bx = context.pop();
	context.al = context.data.byte(478);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.bx);
	context.ah = 0;
	context._sub(context.ax, 12);
	context.data.word(484) = context.ax;
	context.al = context.data.byte(context.bx+1);
	context.ah = 0;
	context._sub(context.ax, 12);
	context.data.word(486) = context.ax;
	bresenhams(context);
	context._cmp(context.data.byte(493), 0);
	if (context.flags.z()) goto normalline;
	context.al = context.data.byte(494);
	context._sub(context.al, 1);
	context.data.byte(492) = context.al;
	context.data.byte(493) = 1;
	return;
normalline:
	context.data.byte(492) = 0;
	return;
}

static inline void watchreel(Context & context) {
	context._cmp(context.data.word(23), -1);
	if (context.flags.z()) goto notplayingreel;
	context.al = context.data.byte(475);
	context._cmp(context.al, context.data.byte(477));
	if (!context.flags.z()) goto waitstopwalk;
	context.al = context.data.byte(135);
	context._cmp(context.al, context.data.byte(133));
	if (context.flags.z()) goto notwatchpath;
waitstopwalk:
	return;
notwatchpath:
	context._sub(context.data.byte(26), 1);
	context._cmp(context.data.byte(26), -1);
	if (!context.flags.z()) goto showwatchreel;
	context.al = context.data.byte(27);
	context.data.byte(26) = context.al;
	context.ax = context.data.word(23);
	context._cmp(context.ax, context.data.word(25));
	if (!context.flags.z()) goto ismorereel;
	context._cmp(context.data.word(21), 0);
	if (!context.flags.z()) goto showwatchreel;
	context.data.word(23) = -1;
	context.data.byte(32) = -1;
	context._cmp(context.data.word(29), -1);
	if (context.flags.z()) goto nomorereel;
	context.data.byte(32) = 1;
	goto notplayingreel;
ismorereel:
	context._add(context.data.word(23), 1);
showwatchreel:
	context.ax = context.data.word(23);
	context.data.word(239) = context.ax;
	plotreel(context);
	context.ax = context.data.word(239);
	context.data.word(23) = context.ax;
	checkforshake(context);
nomorereel:
	return;
notplayingreel:
	context._cmp(context.data.byte(32), 1);
	if (!context.flags.z()) goto notholdingreel;
	context.ax = context.data.word(29);
	context.data.word(239) = context.ax;
	plotreel(context);
	return;
notholdingreel:
	context._cmp(context.data.byte(32), 2);
	if (!context.flags.z()) goto notreleasehold;
	context._sub(context.data.byte(26), 1);
	context._cmp(context.data.byte(26), -1);
	if (!context.flags.z()) goto notlastspeed2;
	context.al = context.data.byte(27);
	context.data.byte(26) = context.al;
	context._add(context.data.word(29), 1);
notlastspeed2:
	context.ax = context.data.word(29);
	context._cmp(context.ax, context.data.word(31));
	if (!context.flags.z()) goto ismorereel2;
	context.data.word(29) = -1;
	context.data.byte(32) = -1;
	context.al = context.data.byte(33);
	context.data.byte(478) = context.al;
	context.data.byte(477) = context.al;
	autosetwalk(context);
	return;
ismorereel2:
	context.ax = context.data.word(29);
	context.data.word(239) = context.ax;
	plotreel(context);
	return;
notreleasehold:
	return;
}

static inline void randomnum1(Context & context) {
	context.push(context.ds);
	context.push(context.es);
	context.push(context.di);
	context.push(context.bx);
	context.push(context.cx);
	randomnumber(context);
	context.cx = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	context.es = context.pop();
	context.ds = context.pop();
	return;
}

static inline void playchannel1(Context & context) {
	context._cmp(context.data.byte(378), 255);
	if (context.flags.z()) goto dontbother5;
	context._cmp(context.data.byte(515), 7);
	if (context.flags.z()) goto dontbother5;
	context.push(context.es);
	context.push(context.ds);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.di);
	context.push(context.si);
	context.data.byte(515) = context.al;
	context.es = context.data.word(420);
	context._cmp(context.al, 12);
	if (context.flags.c()) goto notsecondbank1;
	context.es = context.data.word(422);
	context._sub(context.al, 12);
notsecondbank1:
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.data.byte(context.bx);
	context.ah = 0;
	context.data.word(517) = context.ax;
	context.ax = context.data.word(context.bx+1);
	context.data.word(519) = context.ax;
	context.ax = context.data.word(context.bx+3);
	context.data.word(521) = context.ax;
	context.si = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ds = context.pop();
	context.es = context.pop();
dontbother5:
	return;
}

static inline void showrain(Context & context) {
	context.ds = context.data.word(414);
	context.si = 6*58;
	context.ax = context.data.word(context.si+2);
	context.si = context.ax;
	context._add(context.si, 2080);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30);
	context.es = context.data.word(412);
	context._cmp(context.data.byte(context.bx), 255);
	if (context.flags.z()) goto nothunder;
morerain:
	context.es = context.data.word(412);
	context._cmp(context.data.byte(context.bx), 255);
	if (context.flags.z()) goto finishrain;
	context.al = context.data.byte(context.bx+1);
	context.ah = 0;
	context._add(context.ax, context.data.word(119));
	context._add(context.ax, context.data.word(127));
	context.cx = 320;
	context._mul(context.cx);
	context.cl = context.data.byte(context.bx);
	context.ch = 0;
	context._add(context.ax, context.cx);
	context._add(context.ax, context.data.word(117));
	context._add(context.ax, context.data.word(125));
	context.di = context.ax;
	context.cl = context.data.byte(context.bx+2);
	context.ch = 0;
	context.ax = context.data.word(context.bx+3);
	context.dl = context.data.byte(context.bx+5);
	context.dh = 0;
	context._sub(context.ax, context.dx);
	context._and(context.ax, 511);
	context.data.word(context.bx+3) = context.ax;
	context._add(context.bx, 6);
	context.push(context.si);
	context._add(context.si, context.ax);
	context.es = context.data.word(400);
	context.ah = 0;
	context.dx = 320-2;
rainloop:
	context._lodsb();
	context._cmp(context.al, context.ah);
	if (context.flags.z()) goto noplot;
	context._stosb();
	context._add(context.di, context.dx);
	if (--context.cx) goto rainloop;
	context.si = context.pop();
	goto morerain;
noplot:
	context._add(context.di, 320-1);
	if (--context.cx) goto rainloop;
	context.si = context.pop();
	goto morerain;
finishrain:
	context._cmp(context.data.word(521), 0);
	if (!context.flags.z()) goto nothunder;
	context._cmp(context.data.byte(184), 2);
	if (!context.flags.z()) goto notlouisthund;
	context._cmp(context.data.byte(45), 1);
	if (!context.flags.z()) goto nothunder;
notlouisthund:
	context._cmp(context.data.byte(184), 55);
	if (context.flags.z()) goto nothunder;
	randomnum1(context);
	context._cmp(context.al, 1);
	if (!context.flags.c()) goto nothunder;
	context.al = 7;
	context._cmp(context.data.byte(507), 6);
	if (context.flags.z()) goto isthunder1;
	context.al = 4;
isthunder1:
	playchannel1(context);
nothunder:
	return;
}

static inline void reelsonscreen(Context & context) {
	reconstruct(context);
	updatepeople(context);
	watchreel(context);
	showrain(context);
	usetimedtext(context);
	return;
}

static inline void clearwork(Context & context) {
	context.ax = 0x0;
	context.es = context.data.word(400);
	context.di = 0;
	context.cx = (200*320)/64;
clearloop:
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	if (--context.cx) goto clearloop;
	return;
}

static inline void findroominloc(Context & context) {
	context.al = context.data.byte(149);
	context.cx = -6;
looky:
	context._add(context.cx, 6);
	context._sub(context.al, 10);
	if (!context.flags.c()) goto looky;
	context.al = context.data.byte(148);
	context._sub(context.cx, 1);
lookx:
	context._add(context.cx, 1);
	context._sub(context.al, 11);
	if (!context.flags.c()) goto lookx;
	context.data.byte(185) = context.cl;
	return;
}

static inline void eraseoldobs(Context & context) {
	context._cmp(context.data.byte(62), 0);
	if (context.flags.z()) goto donterase;
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768;
	context.cx = 16;
oberase:
	context.push(context.cx);
	context.push(context.bx);
	context.ax = context.data.word(context.bx+20);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto notthisob;
	context.di = context.bx;
	context.al = 255;
	context.cx = 32;
	while(--context.cx) 	context._stosb();
notthisob:
	context.bx = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 32);
	if (--context.cx) goto oberase;
donterase:
	return;
}

static inline void drawflags(Context & context) {
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32);
	context.al = context.data.byte(149);
	context.ah = 0;
	context.cx = 66;
	context._mul(context.cx);
	context.bl = context.data.byte(148);
	context.bh = 0;
	context._add(context.ax, context.bx);
	context.si = 0;
	context._add(context.si, context.ax);
	context.cx = 10;
_tmp28:
	context.push(context.cx);
	context.cx = 11;
_tmp28a:
	context.ds = context.data.word(418);
	context._lodsb();
	context.ds = context.data.word(416);
	context.push(context.si);
	context.push(context.ax);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.si = 0;
	context._add(context.si, context.ax);
	context._movsw();
 	context.ax = context.pop();
	context._stosb();
	context.si = context.pop();
	if (--context.cx) goto _tmp28a;
	context._add(context.si, 66-11);
	context.cx = context.pop();
	if (--context.cx) goto _tmp28;
	return;
}

static inline void addalong(Context & context) {
	context.ah = 11;
addloop:
	context._cmp(context.data.byte(context.bx), 0);
	if (!context.flags.z()) goto gotalong;
	context._add(context.bx, 3);
	context._sub(context.ah, 1);
	if (!context.flags.z()) goto addloop;
	context.al = 0;
	return;
gotalong:
	context.al = 1;
	return;
}

static inline void addlength(Context & context) {
	context.ah = 10;
addloop2:
	context._cmp(context.data.byte(context.bx), 0);
	if (!context.flags.z()) goto gotlength;
	context._add(context.bx, 3*11);
	context._sub(context.ah, 1);
	if (!context.flags.z()) goto addloop2;
	context.al = 0;
	return;
gotlength:
	context.al = 1;
	return;
}

static inline void getdimension(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32);
	context.ch = 0;
dimloop1:
	addalong(context);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto finishdim1;
	context._add(context.ch, 1);
	goto dimloop1;
finishdim1:
	context.bx = 0+(228*13)+32+60+(32*32);
	context.cl = 0;
dimloop2:
	context.push(context.bx);
	addlength(context);
	context.bx = context.pop();
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto finishdim2;
	context._add(context.cl, 1);
	context._add(context.bx, 3);
	goto dimloop2;
finishdim2:
	context.bx = 0+(228*13)+32+60+(32*32)+(11*3*9);
	context.dh = 10;
dimloop3:
	context.push(context.bx);
	addalong(context);
	context.bx = context.pop();
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto finishdim3;
	context._sub(context.dh, 1);
	context._sub(context.bx, 11*3);
	goto dimloop3;
finishdim3:
	context.bx = 0+(228*13)+32+60+(32*32)+(3*10);
	context.dl = 11;
dimloop4:
	context.push(context.bx);
	addlength(context);
	context.bx = context.pop();
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto finishdim4;
	context._sub(context.dl, 1);
	context._sub(context.bx, 3);
	goto dimloop4;
finishdim4:
	context.al = context.cl;
	context.ah = 0;
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context.data.word(125) = context.ax;
	context.al = context.ch;
	context.ah = 0;
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context.data.word(127) = context.ax;
	context._sub(context.dl, context.cl);
	context._sub(context.dh, context.ch);
	context.al = context.dl;
	context.ah = 0;
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context.data.byte(128) = context.al;
	context.al = context.dh;
	context.ah = 0;
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context.data.byte(129) = context.al;
	return;
}

static inline void calcmapad(Context & context) {
	getdimension(context);
	context.push(context.cx);
	context.push(context.dx);
	context.al = 11;
	context._sub(context.al, context.dl);
	context._sub(context.al, context.cl);
	context._sub(context.al, context.cl);
	context.ax.cbw();
	context.bx = 8;
	context._mul(context.bx);
	context._add(context.ax, context.data.word(121));
	context.data.word(117) = context.ax;
	context.dx = context.pop();
	context.cx = context.pop();
	context.al = 10;
	context._sub(context.al, context.dh);
	context._sub(context.al, context.ch);
	context._sub(context.al, context.ch);
	context.ax.cbw();
	context.bx = 8;
	context._mul(context.bx);
	context._add(context.ax, context.data.word(123));
	context.data.word(119) = context.ax;
	return;
}

static inline void doblocks(Context & context) {
	context.es = context.data.word(400);
	context.ax = context.data.word(119);
	context.cx = 320;
	context._mul(context.cx);
	context.di = context.data.word(117);
	context._add(context.di, context.ax);
	context.al = context.data.byte(149);
	context.ah = 0;
	context.bx = 66;
	context._mul(context.bx);
	context.bl = context.data.byte(148);
	context.bh = 0;
	context._add(context.ax, context.bx);
	context.si = 0;
	context._add(context.si, context.ax);
	context.cx = 10;
loop120:
	context.push(context.di);
	context.push(context.cx);
	context.cx = 11;
loop124:
	context.push(context.cx);
	context.push(context.di);
	context.ds = context.data.word(418);
	context._lodsb();
	context.ds = context.data.word(416);
	context.push(context.si);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto zeroblock;
	context.ah = context.al;
	context.al = 0;
	context.si = 0+192;
	context._add(context.si, context.ax);
	context.bh = 14;
	context.bh = 4;
firstbitofblock:
	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._add(context.di, 320-16);
	context._sub(context.bh, 1);
	if (!context.flags.z()) goto firstbitofblock;
	context.bh = 12;
loop125:
	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context.ax = 0x0dfdf;
	context._stosw();
	context._stosw();
	context._add(context.di, 320-20);
	context._sub(context.bh, 1);
	if (!context.flags.z()) goto loop125;
	context._add(context.di, 4);
	context.ax = 0x0dfdf;
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._add(context.di, 320-16);
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._add(context.di, 320-16);
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._add(context.di, 320-16);
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
zeroblock:
	context.si = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.di, 16);
	if (--context.cx) goto loop124;
	context._add(context.si, 66-11);
	context.cx = context.pop();
	context.di = context.pop();
	context._add(context.di, 320*16);
	if (--context.cx) goto loop120;
	return;
}

static inline void makesprite(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768;
_tmp17:
	context._cmp(context.data.byte(context.bx+15), 255);
	if (context.flags.z()) goto _tmp17a;
	context._add(context.bx, 32);
	goto _tmp17;
_tmp17a:
	context.data.word(context.bx) = context.cx;
	context.data.word(context.bx+10) = context.si;
	context.data.word(context.bx+6) = context.dx;
	context.data.word(context.bx+8) = context.di;
	context.data.word(context.bx+2) = 0x0ffff;
	context.data.byte(context.bx+15) = 0;
	context.data.byte(context.bx+18) = 0;
	return;
}

static inline void makebackob(Context & context) {
	context._cmp(context.data.byte(62), 0);
	if (context.flags.z()) goto nomake;
	context.al = context.data.byte(context.si+5);
	context.ah = context.data.byte(context.si+8);
	context.push(context.si);
	context.push(context.ax);
	context.push(context.si);
	context.ax = context.data.word(168);
	context.bx = context.data.word(170);
	context.ah = context.bl;
	context.si = context.ax;
	context.cx = 49520;
	context.dx = context.data.word(444);
	context.di = 0;
	makesprite(context);
	context.ax = context.pop();
	context.data.word(context.bx+20) = context.ax;
	context.ax = context.pop();
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto usedpriority;
	context.al = 0;
usedpriority:
	context.data.byte(context.bx+23) = context.al;
	context.data.byte(context.bx+30) = context.ah;
	context.data.byte(context.bx+16) = 0;
	context.data.byte(context.bx+18) = 0;
	context.data.byte(context.bx+19) = 0;
	context.si = context.pop();
nomake:
	return;
}

static inline void showallobs(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32);
	context.data.word(18) = context.bx;
	context.di = context.bx;
	context.cx = 128*5;
	context.al = 255;
	while(--context.cx) 	context._stosb();
	context.es = context.data.word(444);
	context.data.word(166) = context.es;
	context.ax = 0;
	context.data.word(164) = context.ax;
	context.ax = 0+2080;
	context.data.word(162) = context.ax;
	context.data.byte(181) = 0;
	context.ds = context.data.word(428);
	context.si = 0;
	context.cx = 128;
showobsloop:
	context.push(context.cx);
	context.push(context.si);
	context.push(context.si);
	context._add(context.si, 58);
	context.es = context.data.word(428);
	getmapad(context);
	context.si = context.pop();
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto blankframe;
	context.al = context.data.byte(context.si+18);
	context.ah = 0;
	context.data.word(160) = context.ax;
	context._cmp(context.al, 255);
	if (context.flags.z()) goto blankframe;
	context.push(context.es);
	context.push(context.si);
	calcfrframe(context);
	finalframe(context);
	context.si = context.pop();
	context.es = context.pop();
	context.al = context.data.byte(context.si+18);
	context.data.byte(context.si+17) = context.al;
	context._cmp(context.data.byte(context.si+8), 0);
	if (!context.flags.z()) goto animating;
	context._cmp(context.data.byte(context.si+5), 5);
	if (context.flags.z()) goto animating;
	context._cmp(context.data.byte(context.si+5), 6);
	if (context.flags.z()) goto animating;
	context.ax = context.data.word(160);
	context.ah = 0;
	context._add(context.di, context.data.word(117));
	context._add(context.bx, context.data.word(119));
	showframe(context);
	goto drawnsetob;
animating:
	makebackob(context);
drawnsetob:
	context.si = context.data.word(18);
	context.es = context.data.word(412);
	context.al = context.data.byte(179);
	context.ah = context.data.byte(180);
	context.data.word(context.si) = context.ax;
	context.cx = context.ax;
	context.ax = context.data.word(176);
	context._add(context.al, context.cl);
	context._add(context.ah, context.ch);
	context.data.word(context.si+2) = context.ax;
	context.al = context.data.byte(181);
	context.data.byte(context.si+4) = context.al;
	context._add(context.si, 5);
	context.data.word(18) = context.si;
blankframe:
	context._add(context.data.byte(181), 1);
	context.si = context.pop();
	context.cx = context.pop();
	context._add(context.si, 64);
	context._sub(context.cx, 1);
	if (context.flags.z()) goto finishedsetobs;
	goto showobsloop;
finishedsetobs:
	return;
}

static inline void showallfree(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5);
	context.data.word(18) = context.bx;
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5);
	context.cx = 80*5;
	context.al = 255;
	while(--context.cx) 	context._stosb();
	context.es = context.data.word(446);
	context.data.word(166) = context.es;
	context.ax = 0;
	context.data.word(164) = context.ax;
	context.ax = 0+2080;
	context.data.word(162) = context.ax;
	context.al = 0;
	context.data.byte(158) = context.al;
	context.ds = context.data.word(426);
	context.si = 2;
	context.cx = 0;
loop127:
	context.push(context.cx);
	context.push(context.si);
	context.push(context.si);
	context.es = context.data.word(426);
	getmapad(context);
	context.si = context.pop();
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto over138;
	context.al = context.data.byte(158);
	context.ah = 0;
	context.dx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.dx);
	context.data.word(160) = context.ax;
	context.push(context.es);
	context.push(context.si);
	calcfrframe(context);
	context.es = context.data.word(402);
	context.ds = context.data.word(166);
	finalframe(context);
	context.si = context.pop();
	context.es = context.pop();
	context._cmp(context.cx, 0);
	if (context.flags.z()) goto over138;
	context.ax = context.data.word(160);
	context.ah = 0;
	context._add(context.di, context.data.word(117));
	context._add(context.bx, context.data.word(119));
	showframe(context);
	context.si = context.data.word(18);
	context.es = context.data.word(412);
	context.al = context.data.byte(179);
	context.ah = context.data.byte(180);
	context.data.word(context.si) = context.ax;
	context.cx = context.ax;
	context.ax = context.data.word(176);
	context._add(context.al, context.cl);
	context._add(context.ah, context.ch);
	context.data.word(context.si+2) = context.ax;
	context.ax = context.pop();
	context.cx = context.pop();
	context.push(context.cx);
	context.push(context.ax);
	context.data.byte(context.si+4) = context.cl;
	context._add(context.si, 5);
	context.data.word(18) = context.si;
over138:
	context._add(context.data.byte(158), 1);
	context.si = context.pop();
	context.cx = context.pop();
	context._add(context.si, 16);
	context._add(context.cx, 1);
	context._cmp(context.cx, 80);
	if (context.flags.z()) goto finfree;
	goto loop127;
finfree:
	return;
}

static inline void getxad(Context & context) {
	context.cl = context.data.byte(context.si);
	context._add(context.si, 1);
	context.al = context.data.byte(context.si);
	context._add(context.si, 1);
	context.ah = context.data.byte(context.si);
	context._add(context.si, 1);
	context._cmp(context.cl, 0);
	if (!context.flags.z()) goto over148;
	context._sub(context.al, context.data.byte(148));
	if (context.flags.c()) goto over148;
	context._cmp(context.al, 11);
	if (!context.flags.c()) goto over148;
	context.cl = 4;
	context._shl(context.al, context.cl);
	context._or(context.al, context.ah);
	context.ah = 0;
	context.ch = 1;
	return;
over148:
	context.ch = 0;
	return;
}

static inline void getyad(Context & context) {
	context.al = context.data.byte(context.si);
	context._add(context.si, 1);
	context.ah = context.data.byte(context.si);
	context._add(context.si, 1);
	context._sub(context.al, context.data.byte(149));
	if (context.flags.c()) goto over147;
	context._cmp(context.al, 10);
	if (!context.flags.c()) goto over147;
	context.cl = 4;
	context._shl(context.al, context.cl);
	context._or(context.al, context.ah);
	context.ah = 0;
	context.ch = 1;
	return;
over147:
	context.ch = 0;
	return;
}

static inline void getmapad(Context & context) {
	getxad(context);
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto over146;
	context.data.word(168) = context.ax;
	getyad(context);
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto over146;
	context.data.word(170) = context.ax;
	context.ch = 1;
over146:
	return;
}

static inline void calcfrframe(Context & context) {
	context.dx = context.data.word(166);
	context.ax = context.data.word(162);
	context.push(context.ax);
	context.cx = context.data.word(164);
	context.ax = context.data.word(160);
	context.ds = context.dx;
	context.bx = 6;
	context._mul(context.bx);
	context._add(context.ax, context.cx);
	context.bx = context.ax;
	context.cx = context.data.word(context.bx);
	context.ax = context.data.word(context.bx+2);
	context.dx = context.data.word(context.bx+4);
	context.bx = context.pop();
	context.push(context.dx);
	context._add(context.ax, context.bx);
	context.data.word(178) = context.ax;
	context.data.word(176) = context.cx;
	context.ax = context.pop();
	context.push(context.ax);
	context.ah = 0;
	context.data.word(172) = context.ax;
	context.ax = context.pop();
	context.al = context.ah;
	context.ah = 0;
	context.data.word(174) = context.ax;
	return;
nullframe:
	context.ax = context.pop();
	context.cx = 0;
	context.data.word(176) = context.cx;
	return;
}

static inline void finalframe(Context & context) {
	context.ax = context.data.word(170);
	context._add(context.ax, context.data.word(174));
	context.bx = context.data.word(168);
	context._add(context.bx, context.data.word(172));
	context.data.byte(179) = context.bl;
	context.data.byte(180) = context.al;
	context.di = context.data.word(168);
	context.bx = context.data.word(170);
	return;
}

static inline void showallex(Context & context) {
	context.es = context.data.word(412);
	context.bx = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5);
	context.data.word(18) = context.bx;
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5);
	context.cx = 100*5;
	context.al = 255;
	while(--context.cx) 	context._stosb();
	context.es = context.data.word(398);
	context.data.word(166) = context.es;
	context.ax = 0;
	context.data.word(164) = context.ax;
	context.ax = 0+2080;
	context.data.word(162) = context.ax;
	context.data.byte(157) = 0;
	context.si = 0+2080+30000+2;
	context.cx = 0;
exloop:
	context.push(context.cx);
	context.push(context.si);
	context.es = context.data.word(398);
	context.push(context.si);
	context.ch = 0;
	context._cmp(context.data.byte(context.si), 255);
	if (context.flags.z()) goto notinroom;
	context.al = context.data.byte(context.si-2);
	context._cmp(context.al, context.data.byte(184));
	if (!context.flags.z()) goto notinroom;
	getmapad(context);
notinroom:
	context.si = context.pop();
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto blankex;
	context.al = context.data.byte(157);
	context.ah = 0;
	context.dx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.dx);
	context.data.word(160) = context.ax;
	context.push(context.es);
	context.push(context.si);
	calcfrframe(context);
	context.es = context.data.word(402);
	context.ds = context.data.word(166);
	finalframe(context);
	context.si = context.pop();
	context.es = context.pop();
	context._cmp(context.cx, 0);
	if (context.flags.z()) goto blankex;
	context.ax = context.data.word(160);
	context.ah = 0;
	context._add(context.di, context.data.word(117));
	context._add(context.bx, context.data.word(119));
	showframe(context);
	context.si = context.data.word(18);
	context.es = context.data.word(412);
	context.al = context.data.byte(179);
	context.ah = context.data.byte(180);
	context.data.word(context.si) = context.ax;
	context.cx = context.ax;
	context.ax = context.data.word(176);
	context._add(context.al, context.cl);
	context._add(context.ah, context.ch);
	context.data.word(context.si+2) = context.ax;
	context.ax = context.pop();
	context.cx = context.pop();
	context.push(context.cx);
	context.push(context.ax);
	context.data.byte(context.si+4) = context.cl;
	context._add(context.si, 5);
	context.data.word(18) = context.si;
blankex:
	context._add(context.data.byte(157), 1);
	context.si = context.pop();
	context.cx = context.pop();
	context._add(context.si, 16);
	context._add(context.cx, 1);
	context._cmp(context.cx, 100);
	if (context.flags.z()) goto finex;
	goto exloop;
finex:
	return;
}

static inline void paneltomap(Context & context) {
	context.di = context.data.word(125);
	context._add(context.di, context.data.word(117));
	context.bx = context.data.word(127);
	context._add(context.bx, context.data.word(119));
	context.ds = context.data.word(402);
	context.si = 0;
	context.cl = context.data.byte(128);
	context.ch = context.data.byte(129);
	multiget(context);
	return;
}

static inline void checkone(Context & context) {
	context.push(context.cx);
	context.al = context.ch;
	context.ah = 0;
	context.cl = 4;
	context._shr(context.ax, context.cl);
	context.dl = context.al;
	context.cx = context.pop();
	context.al = context.cl;
	context.ah = 0;
	context.cl = 4;
	context._shr(context.ax, context.cl);
	context.ah = context.dl;
	context.push(context.ax);
	context.ch = 0;
	context.cl = context.al;
	context.push(context.cx);
	context.al = context.ah;
	context.ah = 0;
	context.cx = 11;
	context._mul(context.cx);
	context.cx = context.pop();
	context._add(context.ax, context.cx);
	context.cx = 3;
	context._mul(context.cx);
	context.si = context.ax;
	context.ds = context.data.word(412);
	context._add(context.si, 0+(228*13)+32+60+(32*32));
	context._lodsw();
	context.cx = context.ax;
	context._lodsb();
	context.dx = context.pop();
	return;
}

static inline void getblockofpixel(Context & context) {
	context.push(context.cx);
	context.push(context.es);
	context.push(context.di);
	context.ax = context.data.word(125);
	context._add(context.cl, context.al);
	context.ax = context.data.word(127);
	context._add(context.ch, context.al);
	checkone(context);
	context._and(context.cl, 1);
	if (!context.flags.z()) goto failrain;
	context.di = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	return;
failrain:
	context.di = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context.al = 0;
	return;
}

static inline void splitintolines(Context & context) {
lookforlinestart:
	getblockofpixel(context);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto foundlinestart;
	context._sub(context.cl, 1);
	context._add(context.ch, 1);
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto endofthisline;
	context._cmp(context.ch, context.data.byte(129));
	if (!context.flags.c()) goto endofthisline;
	goto lookforlinestart;
foundlinestart:
	context.data.word(context.di) = context.cx;
	context.bh = 1;
lookforlineend:
	getblockofpixel(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundlineend;
	context._sub(context.cl, 1);
	context._add(context.ch, 1);
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto foundlineend;
	context._cmp(context.ch, context.data.byte(129));
	if (!context.flags.c()) goto foundlineend;
	context._add(context.bh, 1);
	goto lookforlineend;
foundlineend:
	context.push(context.cx);
	context.data.byte(context.di+2) = context.bh;
	randomnumber(context);
	context.data.byte(context.di+3) = context.al;
	randomnumber(context);
	context.data.byte(context.di+4) = context.al;
	randomnumber(context);
	context._and(context.al, 3);
	context._add(context.al, 4);
	context.data.byte(context.di+5) = context.al;
	context._add(context.di, 6);
	context.cx = context.pop();
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto endofthisline;
	context._cmp(context.ch, context.data.byte(129));
	if (!context.flags.c()) goto endofthisline;
	goto lookforlinestart;
endofthisline:
	return;
}

static inline void initrain(Context & context) {
	context.es = context.data.word(412);
	context.di = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*30);
	context.bx = 1113;
checkmorerain:
	context.al = context.data.byte(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto finishinitrain;
	context._cmp(context.al, context.data.byte(184));
	if (!context.flags.z()) goto checkrain;
	context.al = context.data.byte(context.bx+1);
	context._cmp(context.al, context.data.byte(148));
	if (!context.flags.z()) goto checkrain;
	context.al = context.data.byte(context.bx+2);
	context._cmp(context.al, context.data.byte(149));
	if (!context.flags.z()) goto checkrain;
	context.al = context.data.byte(context.bx+3);
	context.data.byte(132) = context.al;
	goto dorain;
checkrain:
	context._add(context.bx, 4);
	goto checkmorerain;
dorain:
	context.cx = 4;
initraintop:
	randomnumber(context);
	context._and(context.al, 31);
	context._add(context.al, 3);
	context._cmp(context.al, context.data.byte(132));
	if (!context.flags.c()) goto initraintop;
	context._add(context.cl, context.al);
	context._cmp(context.cl, context.data.byte(128));
	if (!context.flags.c()) goto initrainside;
	context.push(context.cx);
	splitintolines(context);
	context.cx = context.pop();
	goto initraintop;
initrainside:
	context.cl = context.data.byte(128);
	context._sub(context.cl, 1);
initrainside2:
	randomnumber(context);
	context._and(context.al, 31);
	context._add(context.al, 3);
	context._cmp(context.al, context.data.byte(132));
	if (!context.flags.c()) goto initrainside2;
	context._add(context.ch, context.al);
	context._cmp(context.ch, context.data.byte(129));
	if (!context.flags.c()) goto finishinitrain;
	context.push(context.cx);
	splitintolines(context);
	context.cx = context.pop();
	goto initrainside2;
finishinitrain:
	context.al = 255;
	context._stosb();
	return;
}

static inline void drawfloor(Context & context) {
	context.push(context.es);
	context.push(context.bx);
	eraseoldobs(context);
	drawflags(context);
	calcmapad(context);
	doblocks(context);
	showallobs(context);
	showallfree(context);
	showallex(context);
	paneltomap(context);
	initrain(context);
	context.data.byte(62) = 0;
	context.bx = context.pop();
	context.es = context.pop();
	return;
}

static inline void width160(Context & context) {
	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 width128:
	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 width110:
	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 width88:
	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 width80:
	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 	context._movsw();
 width63:
	context._movsw();
 width62:
	context._movsw();
 width61:
	context._movsw();
 width60:
	context._movsw();
 width59:
	context._movsw();
 width58:
	context._movsw();
 width57:
	context._movsw();
 width56:
	context._movsw();
 width55:
	context._movsw();
 width54:
	context._movsw();
 width53:
	context._movsw();
 width52:
	context._movsw();
 width51:
	context._movsw();
 width50:
	context._movsw();
 width49:
	context._movsw();
 width48:
	context._movsw();
 width47:
	context._movsw();
 width46:
	context._movsw();
 width45:
	context._movsw();
 width44:
	context._movsw();
 width43:
	context._movsw();
 width42:
	context._movsw();
 width41:
	context._movsw();
 width40:
	context._movsw();
 width39:
	context._movsw();
 width38:
	context._movsw();
 width37:
	context._movsw();
 width36:
	context._movsw();
 width35:
	context._movsw();
 width34:
	context._movsw();
 width33:
	context._movsw();
 width32:
	context._movsw();
 width31:
	context._movsw();
 width30:
	context._movsw();
 width29:
	context._movsw();
 width28:
	context._movsw();
 width27:
	context._movsw();
 width26:
	context._movsw();
 width25:
	context._movsw();
 width24:
	context._movsw();
 width23:
	context._movsw();
 width22:
	context._movsw();
 width21:
	context._movsw();
 width20:
	context._movsw();
 width19:
	context._movsw();
 width18:
	context._movsw();
 width17:
	context._movsw();
 width16:
	context._movsw();
 width15:
	context._movsw();
 width14:
	context._movsw();
 width13:
	context._movsw();
 width12:
	context._movsw();
 width11:
	context._movsw();
 width10:
	context._movsw();
 width9:
	context._movsw();
 width8:
	context._movsw();
 width7:
	context._movsw();
 width6:
	context._movsw();
 width5:
	context._movsw();
 width4:
	context._movsw();
 width3:
	context._movsw();
 width2:
	context._movsw();
 width1:
	context._movsw();
 width0:
	return;
}

static inline void worktoscreen(Context & context) {
	vsync(context);
	context.si = 0;
	context.di = 0;
	context.cx = 25;
	context.ds = context.data.word(400);
	context.dx = 0x0a000;
	context.es = context.dx;
dumpallloop:
	width160(context);
	width160(context);
	width160(context);
	width160(context);
	width160(context);
	width160(context);
	width160(context);
	width160(context);
	if (--context.cx) goto dumpallloop;
	return;
}

static inline void afterintroroom(Context & context) {
	context._cmp(context.data.byte(186), 0);
	if (context.flags.z()) goto notnewintro;
	clearwork(context);
	findroominloc(context);
	context.data.byte(62) = 1;
	drawfloor(context);
	reelsonscreen(context);
	spriteupdate(context);
	printsprites(context);
	worktoscreen(context);
	context.data.byte(186) = 0;
notnewintro:
	return;
}

static inline void multiget(Context & context) {
	context.ax = context.bx;
	context.bx = 320;
	context._mul(context.bx);
	context._add(context.di, context.ax);
	context.es = context.data.word(400);
	context.es = context.ds;
	context.ds = context.es;
	context._xchg(context.di, context.si);
	context.al = context.cl;
	context.ah = 0;
	context.dx = 320;
	context._sub(context.dx, context.ax);
	context.al = context.cl;
	context._and(context.al, 1);
	if (!context.flags.z()) goto oddwidth2;
	context.bl = context.cl;
	context.bh = 0;
	context.ax = 1819;
	context._shr(context.bx, 1);
	context._sub(context.ax, context.bx);
	context.cl = context.ch;
	context.ch = 0;
multiloop3:
	__dispatch_call(context, context.ax);
	context._add(context.si, context.dx);
	if (--context.cx) goto multiloop3;
	return;
oddwidth2:
	context.bl = context.cl;
	context.bh = 0;
	context._shr(context.bx, 1);
	context.ax = 1819;
	context._sub(context.ax, context.bx);
	context.cl = context.ch;
	context.ch = 0;
multiloop4:
	__dispatch_call(context, context.ax);
	context._movsb();
 	context._add(context.si, context.dx);
	if (--context.cx) goto multiloop4;
	return;
}

static inline void getundertimed(Context & context) {
	context.al = context.data.byte(335);
	context._sub(context.al, 3);
	context.ah = 0;
	context.bx = context.ax;
	context.al = context.data.byte(336);
	context.ah = 0;
	context.di = context.ax;
	context.ch = 30;
	context.cl = 240;
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4);
	multiget(context);
	return;
}

static inline void getnextword(Context & context) {
	context.bx = 0;
getloop:
	context.ax = context.data.word(context.di);
	context._add(context.di, 1);
	context._add(context.bh, 1);
	context._cmp(context.al, ':');
	if (context.flags.z()) goto endall;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto endall;
	context._cmp(context.al, 32);
	if (context.flags.z()) goto endword;
	modifychar(context);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto getloop;
	context.push(context.ax);
	context._sub(context.al, 32);
	context.ah = 0;
	context._add(context.ax, context.data.word(71));
	context._add(context.ax, context.ax);
	context.si = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.si, context.ax);
	context.cl = context.data.byte(context.si+0);
	context.ax = context.pop();
	kernchars(context);
	context._add(context.bl, context.cl);
	goto getloop;
endword:
	context._add(context.bl, 6);
	context.al = 0;
	return;
endall:
	context._add(context.bl, 6);
	context.al = 1;
	return;
}

static inline void getnumber(Context & context) {
	context.cx = 0;
	context.push(context.si);
	context.push(context.bx);
	context.push(context.di);
	context.push(context.ds);
	context.push(context.es);
	context.di = context.si;
wordloop:
	context.push(context.cx);
	context.push(context.dx);
	getnextword(context);
	context.dx = context.pop();
	context.cx = context.pop();
	context._cmp(context.al, 1);
	if (context.flags.z()) goto endoftext;
	context.al = context.cl;
	context.ah = 0;
	context.push(context.bx);
	context.bh = 0;
	context._add(context.ax, context.bx);
	context.bx = context.pop();
	context._sub(context.ax, 10);
	context.dh = 0;
	context._cmp(context.ax, context.dx);
	if (!context.flags.c()) goto gotoverend;
	context._add(context.cl, context.bl);
	context._add(context.ch, context.bh);
	goto wordloop;
gotoverend:
	context.al = context.dl;
	context._and(context.al, 1);
	if (context.flags.z()) goto notcentre;
	context.push(context.cx);
	context.al = context.dl;
	context._and(context.al, 0b11111110);
	context.ah = 0;
	context.ch = 0;
	context._sub(context.ax, context.cx);
	context._add(context.ax, 20);
	context._shr(context.ax, 1);
	context.cx = context.pop();
	context.es = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.si = context.pop();
	context._add(context.di, context.ax);
	context.cl = context.ch;
	return;
notcentre:
	context.es = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.si = context.pop();
	context.cl = context.ch;
	return;
endoftext:
	context.al = context.cl;
	context.ah = 0;
	context.push(context.bx);
	context.bh = 0;
	context._add(context.ax, context.bx);
	context.bx = context.pop();
	context._sub(context.ax, 10);
	context.dh = 0;
	context._cmp(context.ax, context.dx);
	if (!context.flags.c()) goto gotoverend2;
	context._add(context.cl, context.bl);
	context._add(context.ch, context.bh);
gotoverend2:
	context.al = context.dl;
	context._and(context.al, 1);
	if (context.flags.z()) goto notcent2;
	context.push(context.cx);
	context.al = context.dl;
	context._and(context.al, 0b11111110);
	context._add(context.al, 2);
	context.ah = 0;
	context.ch = 0;
	context._add(context.ax, 20);
	context._sub(context.ax, context.cx);
	context._shr(context.ax, 1);
	context.cx = context.pop();
	context.es = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.si = context.pop();
	context._add(context.di, context.ax);
	context.cl = context.ch;
	return;
notcent2:
	context.es = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.si = context.pop();
	context.cl = context.ch;
	return;
}

static inline void modifychar(Context & context) {
	context._cmp(context.al, 128);
	if (context.flags.c()) goto nomod;
	context._cmp(context.al, 160);
	if (!context.flags.z()) goto not160;
	context.al = 'Z'+1;
	return;
not160:
	context._cmp(context.al, 130);
	if (!context.flags.z()) goto not130;
	context.al = 'Z'+2;
	return;
not130:
	context._cmp(context.al, 161);
	if (!context.flags.z()) goto not161;
	context.al = 'Z'+3;
	return;
not161:
	context._cmp(context.al, 162);
	if (!context.flags.z()) goto not162;
	context.al = 'Z'+4;
	return;
not162:
	context._cmp(context.al, 163);
	if (!context.flags.z()) goto not163;
	context.al = 'Z'+5;
	return;
not163:
	context._cmp(context.al, 164);
	if (!context.flags.z()) goto not164;
	context.al = 'Z'+6;
	return;
not164:
	context._cmp(context.al, 165);
	if (!context.flags.z()) goto not165;
	context.al = ','-1;
	return;
not165:
	context._cmp(context.al, 168);
	if (!context.flags.z()) goto not168;
	context.al = 'A'-1;
	return;
not168:
	context._cmp(context.al, 173);
	if (!context.flags.z()) goto not173;
	context.al = 'A'-4;
	return;
not173:
	context._cmp(context.al, 129);
	if (!context.flags.z()) goto not129;
	context.al = 'A'-5;
not129:
	return;
nomod:
	return;
}

static inline void frameoutfx(Context & context) {
	context.push(context.dx);
	context.ax = context.bx;
	context.bx = context.dx;
	context._mul(context.bx);
	context._add(context.di, context.ax);
	context.dx = context.pop();
	context.push(context.cx);
	context.ch = 0;
	context._add(context.dx, context.cx);
	context.cx = context.pop();
frameloopfx1:
	context.push(context.cx);
	context.ch = 0;
frameloopfx2:
	context._lodsb();
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto backtosolidfx;
backtootherfx:
	context._sub(context.di, 1);
	if (--context.cx) goto frameloopfx2;
	context.cx = context.pop();
	context._add(context.di, context.dx);
	context._sub(context.ch, 1);
	if (!context.flags.z()) goto frameloopfx1;
	return;
frameloopfx3:
	context._lodsb();
	context._cmp(context.al, 0);
	if (context.flags.z()) goto backtootherfx;
backtosolidfx:
	context.data.byte(context.di) = context.al;
	context._sub(context.di, 1);
	if (--context.cx) goto frameloopfx3;
	context.cx = context.pop();
	context._add(context.di, context.dx);
	context._sub(context.ch, 1);
	if (!context.flags.z()) goto frameloopfx1;
	return;
}

static inline void frameoutnm(Context & context) {
	context.push(context.dx);
	context.ax = context.bx;
	context.bx = context.dx;
	context._mul(context.bx);
	context._add(context.di, context.ax);
	context.dx = context.pop();
	context.push(context.cx);
	context.ch = 0;
	context._sub(context.dx, context.cx);
	context.cx = context.pop();
	context.al = context.cl;
	context._and(context.al, 1);
	if (!context.flags.z()) goto oddwidthframe;
	context.bl = context.cl;
	context.bh = 0;
	context.ax = 1819;
	context._shr(context.bx, 1);
	context._sub(context.ax, context.bx);
	context.cl = context.ch;
	context.ch = 0;
nmloop1:
	__dispatch_call(context, context.ax);
	context._add(context.di, context.dx);
	if (--context.cx) goto nmloop1;
	return;
oddwidthframe:
	context.bl = context.cl;
	context.bh = 0;
	context._shr(context.bx, 1);
	context.ax = 1819;
	context._sub(context.ax, context.bx);
	context.cl = context.ch;
	context.ch = 0;
nmloop2:
	__dispatch_call(context, context.ax);
	context._movsb();
 	context._add(context.di, context.dx);
	if (--context.cx) goto nmloop2;
	return;
}

static inline void frameoutbh(Context & context) {
	context.push(context.dx);
	context.ax = context.bx;
	context.bx = context.dx;
	context._mul(context.bx);
	context._add(context.di, context.ax);
	context.dx = context.pop();
	context.push(context.cx);
	context.ch = 0;
	context._sub(context.dx, context.cx);
	context.cx = context.pop();
bhloop2:
	context.push(context.cx);
	context.ch = 0;
	context.ah = 255;
bhloop1:
	context._cmp(context.data.byte(context.di), context.ah);
	if (!context.flags.z()) goto nofill;
	context._movsb();
 	if (--context.cx) goto bhloop1;
	goto nextline;
nofill:
	context._add(context.di, 1);
	context._add(context.si, 1);
	if (--context.cx) goto bhloop1;
nextline:
	context._add(context.di, context.dx);
	context.cx = context.pop();
	context._sub(context.ch, 1);
	if (!context.flags.z()) goto bhloop2;
	return;
}

static inline void frameoutv(Context & context) {
	context.push(context.dx);
	context.ax = context.bx;
	context.bx = context.dx;
	context._mul(context.bx);
	context._add(context.di, context.ax);
	context.dx = context.pop();
	context.push(context.cx);
	context.ch = 0;
	context._sub(context.dx, context.cx);
	context.cx = context.pop();
frameloop1:
	context.push(context.cx);
	context.ch = 0;
frameloop2:
	context._lodsb();
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto backtosolid;
backtoother:
	context._add(context.di, 1);
	if (--context.cx) goto frameloop2;
	context.cx = context.pop();
	context._add(context.di, context.dx);
	context._sub(context.ch, 1);
	if (!context.flags.z()) goto frameloop1;
	return;
frameloop3:
	context._lodsb();
	context._cmp(context.al, 0);
	if (context.flags.z()) goto backtoother;
backtosolid:
	context._stosb();
	if (--context.cx) goto frameloop3;
	context.cx = context.pop();
	context._add(context.di, context.dx);
	context._sub(context.ch, 1);
	if (!context.flags.z()) goto frameloop1;
	return;
}

static inline void showframe(Context & context) {
	context.push(context.dx);
	context.push(context.ax);
	context.cx = context.ax;
	context._and(context.cx, 511);
	context._add(context.cx, context.cx);
	context.si = context.cx;
	context._add(context.cx, context.cx);
	context._add(context.si, context.cx);
	context._cmp(context.data.word(context.si), 0);
	if (!context.flags.z()) goto notblankshow;
	context.cx = 0;
	return;
notblankshow:
	context._test(context.ah, 128);
	if (!context.flags.z()) goto skipoffsets;
	context.al = context.data.byte(context.si+4);
	context.ah = 0;
	context._add(context.di, context.ax);
	context.al = context.data.byte(context.si+5);
	context.ah = 0;
	context._add(context.bx, context.ax);
skipoffsets:
	context.cx = context.data.word(context.si+0);
	context.ax = context.data.word(context.si+2);
	context._add(context.ax, 2080);
	context.si = context.ax;
	context.ax = context.pop();
	context.dx = context.pop();
	context._cmp(context.ah, 0);
	if (context.flags.z()) goto noeffects;
	context._test(context.ah, 128);
	if (context.flags.z()) goto notcentred;
	context.push(context.ax);
	context.al = context.cl;
	context.ah = 0;
	context._shr(context.ax, 1);
	context._sub(context.di, context.ax);
	context.al = context.ch;
	context.ah = 0;
	context._shr(context.ax, 1);
	context._sub(context.bx, context.ax);
	context.ax = context.pop();
notcentred:
	context._test(context.ah, 64);
	if (context.flags.z()) goto notdiffdest;
	context.push(context.cx);
	frameoutfx(context);
	context.cx = context.pop();
	return;
notdiffdest:
	context._test(context.ah, 8);
	if (context.flags.z()) goto notprintlist;
	context.push(context.ax);
	context.ax = context.di;
	context._sub(context.ax, context.data.word(117));
	context.push(context.bx);
	context._sub(context.bx, context.data.word(119));
	context.ah = context.bl;
	context.bx = context.pop();
	context.ax = context.pop();
notprintlist:
	context._test(context.ah, 4);
	if (context.flags.z()) goto notflippedx;
	context.dx = 320;
	context.es = context.data.word(400);
	context.push(context.cx);
	frameoutfx(context);
	context.cx = context.pop();
	return;
notflippedx:
	context._test(context.ah, 2);
	if (context.flags.z()) goto notnomask;
	context.dx = 320;
	context.es = context.data.word(400);
	context.push(context.cx);
	frameoutnm(context);
	context.cx = context.pop();
	return;
notnomask:
	context._test(context.ah, 32);
	if (context.flags.z()) goto noeffects;
	context.dx = 320;
	context.es = context.data.word(400);
	context.push(context.cx);
	frameoutbh(context);
	context.cx = context.pop();
	return;
noeffects:
	context.dx = 320;
	context.es = context.data.word(400);
	context.push(context.cx);
	frameoutv(context);
	context.cx = context.pop();
	return;
}

static inline void kernchars(Context & context) {
	context._cmp(context.al, 'a');
	if (context.flags.z()) goto iskern;
	context._cmp(context.al, 'u');
	if (context.flags.z()) goto iskern;
	return;
iskern:
	context._cmp(context.ah, 'n');
	if (context.flags.z()) goto kernit;
	context._cmp(context.ah, 't');
	if (context.flags.z()) goto kernit;
	context._cmp(context.ah, 'r');
	if (context.flags.z()) goto kernit;
	context._cmp(context.ah, 'i');
	if (context.flags.z()) goto kernit;
	context._cmp(context.ah, 'l');
	if (context.flags.z()) goto kernit;
	return;
kernit:
	context._sub(context.cl, 1);
	return;
}

static inline void printchar(Context & context) {
	context._cmp(context.al, 255);
	if (context.flags.z()) goto ignoreit;
	context.push(context.si);
	context.push(context.bx);
	context.push(context.di);
	context._sub(context.bx, 3);
	context.push(context.ax);
	context._sub(context.al, 32);
	context.ah = 0;
	context._add(context.ax, context.data.word(71));
	showframe(context);
	context.ax = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.si = context.pop();
	context._cmp(context.data.byte(72), 0);
	if (!context.flags.z()) goto nokern;
	kernchars(context);
nokern:
	context.push(context.cx);
	context.ch = 0;
	context._add(context.di, context.cx);
	context.cx = context.pop();
ignoreit:
	return;
}

static inline void printdirect(Context & context) {
	context.data.word(84) = context.di;
	context.ds = context.data.word(268);
printloop6:
	context.push(context.bx);
	context.push(context.di);
	context.push(context.dx);
	getnumber(context);
	context.ch = 0;
printloop5:
	context.ax = context.data.word(context.si);
	context._add(context.si, 1);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto finishdirct;
	context._cmp(context.al, ':');
	if (context.flags.z()) goto finishdirct;
	context.push(context.cx);
	context.push(context.es);
	modifychar(context);
	printchar(context);
	context.data.word(84) = context.di;
	context.es = context.pop();
	context.cx = context.pop();
	if (--context.cx) goto printloop5;
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context._add(context.bx, context.data.word(77));
	goto printloop6;
finishdirct:
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	return;
}

static inline void multiput(Context & context) {
	context.ax = context.bx;
	context.bx = 320;
	context._mul(context.bx);
	context._add(context.di, context.ax);
	context.es = context.data.word(400);
	context.al = context.cl;
	context.ah = 0;
	context.dx = 320;
	context._sub(context.dx, context.ax);
	context.al = context.cl;
	context._and(context.al, 1);
	if (!context.flags.z()) goto oddwidth3;
	context.bl = context.cl;
	context.bh = 0;
	context._shr(context.bx, 1);
	context.ax = 1819;
	context._sub(context.ax, context.bx);
	context.cl = context.ch;
	context.ch = 0;
multiloop5:
	__dispatch_call(context, context.ax);
	context._add(context.di, context.dx);
	if (--context.cx) goto multiloop5;
	return;
oddwidth3:
	context.bl = context.cl;
	context.bh = 0;
	context._shr(context.bx, 1);
	context.ax = 1819;
	context._sub(context.ax, context.bx);
	context.cl = context.ch;
	context.ch = 0;
multiloop6:
	__dispatch_call(context, context.ax);
	context._movsb();
 	context._add(context.di, context.dx);
	if (--context.cx) goto multiloop6;
	return;
}

static inline void putundertimed(Context & context) {
	context.al = context.data.byte(335);
	context._sub(context.al, 3);
	context.ah = 0;
	context.bx = context.ax;
	context.al = context.data.byte(336);
	context.ah = 0;
	context.di = context.ax;
	context.ch = 30;
	context.cl = 240;
	context.ds = context.data.word(412);
	context.si = 0+(228*13)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4);
	multiput(context);
	return;
}

static inline void usetimedtext(Context & context) {
	context._cmp(context.data.word(328), 0);
	if (context.flags.z()) goto notext;
	context._sub(context.data.word(328), 1);
	context._cmp(context.data.word(328), 0);
	if (context.flags.z()) goto deltimedtext;
	context.ax = context.data.word(328);
	context._cmp(context.ax, context.data.word(330));
	if (context.flags.z()) goto firsttimed;
	if (!context.flags.c()) goto notext;
	goto notfirsttimed;
firsttimed:
	getundertimed(context);
notfirsttimed:
	context.bl = context.data.byte(335);
	context.bh = 0;
	context.al = context.data.byte(336);
	context.ah = 0;
	context.di = context.ax;
	context.es = context.data.word(332);
	context.si = context.data.word(334);
	context.dl = 237;
	context.ah = 0;
	printdirect(context);
	context.data.byte(337) = 1;
notext:
	return;
deltimedtext:
	putundertimed(context);
	context.data.byte(337) = 1;
	return;
}

static inline void dumpmap(Context & context) {
	context.di = context.data.word(125);
	context._add(context.di, context.data.word(117));
	context.bx = context.data.word(127);
	context._add(context.bx, context.data.word(119));
	context.cl = context.data.byte(128);
	context.ch = context.data.byte(129);
	multidump(context);
	return;
}

static inline void multidump(Context & context) {
	context.dx = 0x0a000;
	context.es = context.dx;
	context.ds = context.data.word(400);
	context.ax = context.bx;
	context.bx = 320;
	context._mul(context.bx);
	context._add(context.di, context.ax);
	context.dx = 320;
	context.si = context.di;
	context.al = context.cl;
	context._and(context.al, 1);
	if (!context.flags.z()) goto oddwidth;
	context.bl = context.cl;
	context.bh = 0;
	context._shr(context.bx, 1);
	context.ax = 1819;
	context._sub(context.ax, context.bx);
	context.bl = context.cl;
	context.bh = 0;
	context._neg(context.bx);
	context._add(context.bx, context.dx);
	context.cl = context.ch;
	context.ch = 0;
multiloop1:
	__dispatch_call(context, context.ax);
	context._add(context.di, context.bx);
	context._add(context.si, context.bx);
	if (--context.cx) goto multiloop1;
	return;
oddwidth:
	context.bl = context.cl;
	context.bh = 0;
	context._shr(context.bx, 1);
	context.ax = 1819;
	context._sub(context.ax, context.bx);
	context.bl = context.cl;
	context.bh = 0;
	context._neg(context.bx);
	context._add(context.bx, 320);
	context.cl = context.ch;
	context.ch = 0;
multiloop2:
	__dispatch_call(context, context.ax);
	context._movsb();
 	context._add(context.di, context.bx);
	context._add(context.si, context.bx);
	if (--context.cx) goto multiloop2;
	return;
}

static inline void dumptimedtext(Context & context) {
	context._cmp(context.data.byte(337), 1);
	if (!context.flags.z()) goto nodumptimed;
	context.al = context.data.byte(335);
	context._sub(context.al, 3);
	context.ah = 0;
	context.bx = context.ax;
	context.al = context.data.byte(336);
	context.ah = 0;
	context.di = context.ax;
	context.cl = 240;
	context.ch = 30;
	multidump(context);
	context.data.byte(337) = 0;
nodumptimed:
	return;
}

static inline void runendseq(Context & context) {
	atmospheres(context);
	context.data.byte(103) = 0;
moreendseq:
	vsync(context);
	spriteupdate(context);
	vsync(context);
	deleverything(context);
	printsprites(context);
	reelsonscreen(context);
	afterintroroom(context);
	usetimedtext(context);
	vsync(context);
	dumpmap(context);
	dumptimedtext(context);
	vsync(context);
	context._cmp(context.data.byte(103), 1);
	if (!context.flags.z()) goto moreendseq;
	return;
}

static inline void gettingshot(Context & context) {
	context.data.byte(188) = 55;
	clearpalette(context);
	loadintroroom(context);
	fadescreenups(context);
	context.data.byte(387) = 0;
	context.data.byte(388) = -1;
	runendseq(context);
	clearbeforeload(context);
	return;
}

static inline void getridoftemptext(Context & context) {
	context.es = context.data.word(466);
	deallocatemem(context);
	return;
}

static inline void endgame(Context & context) {
	context.dx = 2273;
	loadtemptext(context);
	monkspeaking(context);
	gettingshot(context);
	getridoftemptext(context);
	context.data.byte(387) = 7;
	context.data.byte(388) = 1;
	context.cx = 200;
	hangon(context);
	return;
}


Data::Data() {
	static const uint8 src[] = {
		0, 0, 0, 0, 0, 30, 19, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 0, 255, 255, 255, 255, 255, 
		0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 0, 0, 0, 255, 0, 
		255, 0, 0, 48, 0, 0, 0, 0, 0, 0, 0, 10, 0, 13, 0, 182, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 104, 0, 38, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 32, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 1, 255, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 255, 0, 0, 0, 0, 1, 44, 0, 20, 0, 2, 0, 1, 1, 55, 
		0, 0, 0, 50, 20, 0, 24, 22, 0, 74, 0, 1, 0, 0, 24, 33, 
		10, 75, 0, 1, 0, 1, 1, 44, 0, 27, 0, 2, 0, 2, 1, 44, 
		0, 96, 0, 3, 0, 4, 1, 44, 0, 118, 0, 2, 0, 5, 1, 44, 
		10, 0, 0, 2, 0, 0, 5, 22, 20, 53, 0, 3, 0, 0, 5, 22, 
		20, 40, 0, 1, 0, 2, 5, 22, 20, 50, 0, 1, 0, 3, 2, 11, 
		10, 192, 0, 1, 0, 0, 2, 11, 10, 182, 0, 2, 0, 1, 8, 11, 
		10, 0, 0, 2, 0, 1, 23, 0, 50, 0, 0, 3, 0, 0, 28, 11, 
		20, 250, 0, 4, 0, 0, 23, 0, 50, 43, 0, 2, 0, 8, 23, 11, 
		40, 130, 0, 2, 0, 1, 23, 22, 40, 122, 0, 2, 0, 2, 23, 22, 
		40, 105, 0, 2, 0, 3, 23, 22, 40, 81, 0, 2, 0, 4, 23, 11, 
		40, 135, 0, 2, 0, 5, 23, 22, 40, 145, 0, 2, 0, 6, 4, 22, 
		30, 0, 0, 2, 0, 0, 45, 22, 30, 200, 0, 0, 0, 20, 45, 22, 
		30, 39, 0, 2, 0, 0, 45, 22, 30, 25, 0, 2, 0, 0, 8, 22, 
		40, 32, 0, 2, 0, 0, 7, 11, 20, 64, 0, 2, 0, 0, 22, 22, 
		20, 82, 0, 2, 0, 0, 27, 11, 30, 0, 0, 2, 0, 0, 20, 0, 
		30, 0, 0, 2, 0, 0, 14, 33, 40, 21, 0, 1, 0, 0, 29, 11, 
		10, 0, 0, 1, 0, 0, 2, 22, 0, 2, 0, 2, 0, 0, 25, 0, 
		50, 4, 0, 2, 0, 0, 50, 22, 30, 121, 0, 2, 0, 0, 50, 22, 
		30, 0, 0, 20, 0, 0, 52, 22, 30, 192, 0, 2, 0, 0, 52, 22, 
		30, 233, 0, 2, 0, 0, 50, 22, 40, 104, 0, 55, 0, 0, 53, 33, 
		0, 99, 0, 2, 0, 0, 50, 22, 40, 0, 0, 3, 0, 0, 50, 22, 
		30, 162, 0, 2, 0, 0, 52, 22, 30, 57, 0, 2, 0, 0, 52, 22, 
		30, 0, 0, 2, 0, 0, 54, 0, 0, 72, 0, 3, 0, 0, 55, 44, 
		0, 0, 0, 2, 0, 0, 19, 0, 0, 0, 0, 28, 0, 0, 14, 22, 
		0, 2, 0, 2, 0, 0, 14, 22, 0, 44, 1, 1, 0, 0, 10, 22, 
		30, 174, 0, 0, 0, 0, 12, 22, 20, 0, 0, 1, 0, 0, 11, 11, 
		20, 0, 0, 50, 20, 0, 11, 11, 30, 0, 0, 50, 20, 0, 11, 22, 
		20, 0, 0, 50, 20, 0, 14, 33, 40, 0, 0, 50, 20, 0, 255, 124, 
		192, 128, 192, 28, 192, 32, 192, 0, 193, 16, 192, 24, 192, 244, 192, 12, 
		192, 36, 192, 40, 192, 44, 192, 48, 192, 84, 192, 120, 192, 80, 192, 116, 
		192, 52, 192, 56, 192, 64, 192, 68, 192, 72, 192, 60, 192, 20, 192, 136, 
		192, 140, 192, 144, 192, 112, 192, 252, 192, 108, 192, 88, 192, 104, 192, 4, 
		193, 100, 192, 96, 192, 92, 192, 148, 192, 4, 192, 164, 192, 156, 192, 160, 
		192, 168, 192, 172, 192, 152, 192, 176, 192, 180, 192, 200, 192, 204, 192, 212, 
		192, 220, 192, 216, 192, 0, 192, 8, 192, 132, 192, 132, 192, 132, 192, 132, 
		192, 0, 60, 33, 71, 11, 82, 22, 93, 1, 44, 10, 16, 4, 11, 30, 
		14, 4, 22, 30, 14, 3, 33, 10, 14, 10, 33, 30, 14, 10, 22, 30, 
		24, 9, 22, 10, 14, 2, 33, 0, 14, 2, 22, 0, 14, 6, 11, 30, 
		14, 7, 11, 20, 18, 7, 0, 20, 18, 7, 0, 30, 18, 55, 44, 0, 
		14, 5, 22, 30, 14, 8, 0, 10, 18, 8, 11, 10, 18, 8, 22, 10, 
		18, 8, 33, 10, 18, 8, 33, 20, 18, 8, 33, 30, 18, 8, 33, 40, 
		18, 8, 22, 40, 18, 8, 11, 40, 18, 21, 44, 20, 18, 255, 47, 5, 
		48, 5, 52, 5, 47, 5, 47, 5, 47, 5, 71, 5, 47, 5, 78, 5, 
		94, 5, 101, 5, 105, 5, 109, 5, 113, 5, 126, 5, 47, 5, 47, 5, 
		47, 5, 47, 5, 47, 5, 160, 5, 47, 5, 182, 5, 213, 5, 47, 5, 
		226, 5, 248, 5, 14, 6, 39, 6, 58, 6, 47, 5, 47, 5, 47, 5, 
		47, 5, 47, 5, 47, 5, 47, 5, 47, 5, 47, 5, 47, 5, 47, 5, 
		47, 5, 47, 5, 47, 5, 47, 5, 74, 6, 81, 6, 118, 6, 47, 5, 
		47, 5, 47, 5, 47, 5, 131, 6, 135, 6, 47, 5, 142, 6, 255, 15, 
		1, 1, 255, 12, 5, 0, 13, 21, 0, 15, 35, 0, 17, 50, 0, 18, 
		103, 0, 19, 108, 0, 255, 18, 19, 0, 19, 23, 0, 255, 12, 51, 0, 
		13, 53, 0, 14, 14, 0, 15, 20, 0, 0, 78, 0, 255, 12, 119, 0, 
		12, 145, 0, 255, 13, 16, 0, 255, 13, 20, 0, 255, 14, 16, 0, 255, 
		15, 4, 0, 16, 8, 0, 17, 134, 0, 18, 153, 0, 255, 13, 108, 0, 
		15, 70, 1, 15, 75, 1, 15, 80, 1, 15, 86, 1, 15, 92, 1, 15, 
		98, 1, 18, 159, 0, 18, 178, 0, 147, 217, 0, 84, 228, 0, 255, 13, 
		20, 0, 13, 21, 0, 15, 34, 0, 13, 52, 0, 13, 55, 0, 25, 57, 
		0, 21, 73, 0, 255, 13, 196, 0, 13, 234, 0, 13, 156, 0, 14, 129, 
		0, 13, 124, 0, 15, 162, 0, 15, 200, 0, 15, 239, 0, 17, 99, 0, 
		12, 52, 0, 255, 15, 56, 0, 16, 64, 0, 19, 22, 0, 20, 33, 0, 
		255, 20, 11, 0, 20, 15, 0, 15, 28, 0, 13, 80, 0, 21, 82, 0, 
		147, 87, 0, 87, 128, 0, 255, 12, 13, 0, 14, 39, 0, 12, 67, 0, 
		12, 75, 0, 12, 83, 0, 12, 91, 0, 15, 102, 0, 255, 22, 36, 0, 
		13, 125, 0, 18, 88, 0, 15, 107, 0, 14, 127, 0, 14, 154, 0, 147, 
		170, 0, 87, 232, 0, 255, 21, 16, 0, 21, 72, 0, 21, 205, 0, 22, 
		63, 0, 151, 99, 0, 88, 158, 0, 255, 13, 21, 0, 14, 24, 0, 147, 
		50, 0, 87, 75, 0, 24, 128, 0, 255, 83, 46, 0, 16, 167, 0, 255, 
		16, 19, 0, 14, 36, 0, 16, 50, 0, 14, 65, 0, 16, 81, 0, 14, 
		96, 0, 16, 114, 0, 14, 129, 0, 16, 147, 0, 14, 162, 0, 16, 177, 
		0, 14, 191, 0, 255, 13, 48, 0, 14, 41, 0, 15, 78, 0, 16, 92, 
		0, 255, 16, 115, 0, 255, 21, 103, 0, 20, 199, 0, 255, 17, 53, 0, 
		17, 54, 0, 17, 55, 0, 17, 56, 0, 17, 57, 0, 17, 58, 0, 17, 
		59, 0, 17, 61, 0, 17, 63, 0, 17, 64, 0, 17, 65, 0, 255, 156, 
		154, 159, 154, 156, 158, 160, 155, 157, 153, 159, 158, 156, 154, 159, 154, 156, 
		158, 160, 155, 157, 153, 159, 158, 156, 154, 159, 154, 156, 158, 160, 155, 157, 
		153, 159, 158, 156, 154, 159, 154, 156, 158, 160, 155, 157, 153, 159, 158, 156, 
		154, 159, 154, 156, 158, 160, 155, 157, 153, 159, 158, 156, 154, 159, 154, 156, 
		158, 160, 155, 157, 153, 159, 158, 156, 154, 159, 154, 156, 158, 160, 155, 157, 
		153, 159, 158, 156, 154, 159, 154, 156, 158, 160, 155, 157, 153, 159, 158, 156, 
		154, 159, 154, 156, 158, 160, 155, 157, 153, 159, 156, 68, 82, 69, 65, 77, 
		87, 69, 66, 46, 83, 48, 48, 0, 68, 82, 69, 65, 77, 87, 69, 66, 
		46, 83, 48, 50, 0, 73, 78, 83, 84, 65, 76, 76, 46, 68, 65, 84, 
		0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 67, 48, 48, 0, 68, 82, 
		69, 65, 77, 87, 69, 66, 46, 67, 48, 49, 0, 68, 82, 69, 65, 77, 
		87, 69, 66, 46, 67, 48, 50, 0, 68, 82, 69, 65, 77, 87, 69, 66, 
		46, 86, 48, 48, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 86, 57, 
		57, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 71, 48, 48, 0, 68, 
		82, 69, 65, 77, 87, 69, 66, 46, 71, 48, 49, 0, 68, 82, 69, 65, 
		77, 87, 69, 66, 46, 71, 48, 50, 0, 68, 82, 69, 65, 77, 87, 69, 
		66, 46, 71, 48, 56, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 71, 
		48, 51, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 71, 48, 55, 0, 
		68, 82, 69, 65, 77, 87, 69, 66, 46, 71, 48, 52, 0, 68, 82, 69, 
		65, 77, 87, 69, 66, 46, 71, 48, 53, 0, 68, 82, 69, 65, 77, 87, 
		69, 66, 46, 71, 48, 54, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 
		71, 49, 52, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 84, 48, 49, 
		0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 84, 48, 50, 0, 68, 82, 
		69, 65, 77, 87, 69, 66, 46, 84, 49, 48, 0, 68, 82, 69, 65, 77, 
		87, 69, 66, 46, 84, 49, 49, 0, 68, 82, 69, 65, 77, 87, 69, 66, 
		46, 84, 49, 50, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 84, 49, 
		51, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 84, 50, 48, 0, 68, 
		82, 69, 65, 77, 87, 69, 66, 46, 84, 50, 49, 0, 68, 82, 69, 65, 
		77, 87, 69, 66, 46, 84, 50, 50, 0, 68, 82, 69, 65, 77, 87, 69, 
		66, 46, 84, 50, 51, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 84, 
		50, 52, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 84, 53, 48, 0, 
		68, 82, 69, 65, 77, 87, 69, 66, 46, 84, 53, 49, 0, 68, 82, 69, 
		65, 77, 87, 69, 66, 46, 84, 56, 48, 0, 68, 82, 69, 65, 77, 87, 
		69, 66, 46, 84, 56, 49, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 
		84, 56, 50, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 84, 56, 51, 
		0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 84, 56, 52, 0, 68, 82, 
		69, 65, 77, 87, 69, 66, 46, 86, 79, 76, 0, 68, 82, 69, 65, 77, 
		87, 69, 66, 46, 71, 48, 57, 0, 68, 82, 69, 65, 77, 87, 69, 66, 
		46, 71, 49, 48, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 71, 49, 
		49, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 71, 49, 50, 0, 68, 
		82, 69, 65, 77, 87, 69, 66, 46, 71, 49, 51, 0, 68, 82, 69, 65, 
		77, 87, 69, 66, 46, 71, 49, 53, 0, 68, 82, 69, 65, 77, 87, 69, 
		66, 46, 73, 48, 48, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 73, 
		48, 49, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 73, 48, 50, 0, 
		68, 82, 69, 65, 77, 87, 69, 66, 46, 73, 48, 51, 0, 68, 82, 69, 
		65, 77, 87, 69, 66, 46, 73, 48, 52, 0, 68, 82, 69, 65, 77, 87, 
		69, 66, 46, 73, 48, 53, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 
		73, 48, 54, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 73, 48, 55, 
		0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 80, 65, 76, 0, 17, 1, 
		64, 1, 157, 0, 198, 0, 72, 195, 4, 1, 44, 1, 0, 0, 44, 0, 
		132, 197, 210, 0, 254, 0, 0, 0, 44, 0, 224, 195, 144, 0, 176, 0, 
		64, 0, 96, 0, 132, 195, 0, 0, 50, 0, 50, 0, 200, 0, 136, 195, 
		0, 0, 64, 1, 0, 0, 200, 0, 156, 202, 255, 255, 17, 1, 64, 1, 
		157, 0, 198, 0, 72, 195, 255, 0, 38, 1, 0, 0, 24, 0, 204, 195, 
		247, 0, 45, 1, 40, 0, 56, 0, 76, 195, 80, 0, 0, 1, 158, 0, 
		202, 0, 228, 195, 80, 0, 44, 1, 58, 0, 146, 0, 156, 195, 0, 0, 
		64, 1, 0, 0, 200, 0, 156, 202, 255, 255, 17, 1, 64, 1, 157, 0, 
		198, 0, 72, 195, 247, 0, 45, 1, 40, 0, 56, 0, 76, 195, 80, 0, 
		44, 1, 58, 0, 146, 0, 192, 198, 0, 0, 64, 1, 0, 0, 200, 0, 
		156, 202, 255, 255, 17, 1, 64, 1, 157, 0, 198, 0, 128, 196, 240, 0, 
		34, 1, 2, 0, 44, 0, 152, 196, 0, 0, 64, 1, 0, 0, 200, 0, 
		156, 202, 255, 255, 17, 1, 64, 1, 157, 0, 198, 0, 128, 196, 0, 0, 
		64, 1, 0, 0, 200, 0, 156, 202, 255, 255, 238, 0, 2, 1, 4, 0, 
		44, 0, 204, 196, 104, 0, 124, 0, 4, 0, 44, 0, 208, 196, 24, 1, 
		52, 1, 4, 0, 44, 0, 180, 196, 104, 0, 216, 0, 138, 0, 192, 0, 
		212, 196, 17, 1, 64, 1, 157, 0, 198, 0, 128, 196, 0, 0, 64, 1, 
		0, 0, 200, 0, 156, 202, 255, 255, 69, 88, 73, 84, 32, 72, 69, 76, 
		80, 32, 76, 73, 83, 84, 32, 82, 69, 65, 68, 32, 76, 79, 71, 79, 
		78, 32, 75, 69, 89, 83, 32, 1, 0, 80, 85, 66, 76, 73, 67, 32, 
		80, 85, 66, 76, 73, 67, 32, 0, 0, 0, 66, 76, 65, 67, 75, 68, 
		82, 65, 71, 79, 78, 32, 82, 89, 65, 78, 32, 0, 0, 0, 72, 69, 
		78, 68, 82, 73, 88, 32, 76, 79, 85, 73, 83, 32, 0, 0, 0, 83, 
		69, 80, 84, 73, 77, 85, 83, 32, 66, 69, 67, 75, 69, 84, 84, 32, 
		0, 255, 255, 32, 0, 34, 82, 79, 79, 84, 32, 0, 34, 32, 0, 78, 
		69, 84, 87, 236, 196, 69, 76, 86, 65, 144, 198, 69, 76, 86, 66, 160, 
		198, 69, 76, 86, 67, 152, 198, 69, 76, 86, 69, 156, 198, 69, 76, 86, 
		70, 164, 198, 67, 71, 65, 84, 52, 199, 82, 69, 77, 79, 172, 198, 66, 
		85, 84, 65, 64, 199, 67, 66, 79, 88, 72, 199, 76, 73, 84, 69, 96, 
		198, 80, 76, 65, 84, 68, 199, 76, 73, 70, 84, 128, 198, 87, 73, 82, 
		69, 136, 198, 72, 78, 68, 76, 140, 198, 72, 65, 67, 72, 132, 198, 68, 
		79, 79, 82, 184, 198, 67, 83, 72, 82, 116, 198, 71, 85, 78, 65, 56, 
		199, 67, 82, 65, 65, 104, 198, 67, 82, 66, 66, 108, 198, 67, 82, 67, 
		67, 112, 198, 83, 69, 65, 84, 252, 197, 77, 69, 78, 85, 156, 199, 67, 
		79, 79, 75, 176, 198, 69, 76, 67, 65, 80, 198, 69, 68, 67, 65, 84, 
		198, 68, 68, 67, 65, 88, 198, 65, 76, 84, 82, 8, 198, 76, 79, 75, 
		65, 64, 198, 76, 79, 75, 66, 68, 198, 69, 78, 84, 65, 20, 198, 69, 
		78, 84, 66, 40, 198, 69, 78, 84, 69, 44, 198, 69, 78, 84, 67, 28, 
		198, 69, 78, 84, 68, 48, 198, 69, 78, 84, 72, 52, 198, 87, 87, 65, 
		84, 244, 197, 80, 79, 79, 76, 92, 198, 87, 83, 72, 68, 248, 197, 71, 
		82, 65, 70, 72, 198, 84, 82, 65, 80, 76, 198, 67, 68, 80, 69, 44, 
		199, 68, 76, 79, 75, 12, 198, 72, 79, 76, 69, 4, 198, 68, 82, 89, 
		82, 16, 198, 72, 79, 76, 89, 0, 198, 87, 65, 76, 76, 48, 199, 66, 
		79, 79, 75, 12, 200, 65, 88, 69, 68, 180, 198, 83, 72, 76, 68, 60, 
		199, 66, 67, 78, 89, 236, 197, 76, 73, 68, 67, 232, 197, 76, 73, 68, 
		85, 228, 197, 76, 73, 68, 79, 240, 197, 80, 73, 80, 69, 172, 197, 66, 
		65, 76, 67, 36, 198, 87, 73, 78, 68, 32, 198, 80, 65, 80, 82, 184, 
		199, 85, 87, 84, 65, 164, 197, 85, 87, 84, 66, 164, 197, 83, 84, 65, 
		84, 220, 199, 84, 76, 73, 68, 160, 197, 83, 76, 65, 66, 220, 197, 67, 
		65, 82, 84, 224, 197, 70, 67, 65, 82, 176, 197, 83, 76, 66, 65, 196, 
		197, 83, 76, 66, 66, 200, 197, 83, 76, 66, 67, 208, 197, 83, 76, 66, 
		68, 204, 197, 83, 76, 66, 69, 212, 197, 83, 76, 66, 70, 216, 197, 80, 
		76, 73, 78, 180, 197, 76, 65, 68, 68, 188, 197, 76, 65, 68, 66, 192, 
		197, 71, 85, 77, 65, 184, 197, 83, 81, 69, 69, 140, 197, 84, 65, 80, 
		80, 144, 197, 71, 85, 73, 84, 148, 197, 67, 79, 78, 84, 152, 197, 66, 
		69, 76, 76, 156, 197, 140, 140, 140, 140, 48, 48, 48, 48, 0, 48, 48, 
		0, 157, 0, 178, 0, 81, 0, 94, 0, 92, 199, 179, 0, 200, 0, 81, 
		0, 94, 0, 96, 199, 201, 0, 222, 0, 81, 0, 94, 0, 100, 199, 157, 
		0, 178, 0, 95, 0, 112, 0, 104, 199, 179, 0, 200, 0, 95, 0, 112, 
		0, 108, 199, 201, 0, 222, 0, 95, 0, 112, 0, 112, 199, 157, 0, 178, 
		0, 113, 0, 130, 0, 116, 199, 179, 0, 200, 0, 113, 0, 130, 0, 120, 
		199, 201, 0, 222, 0, 113, 0, 130, 0, 124, 199, 157, 0, 178, 0, 131, 
		0, 145, 0, 128, 199, 179, 0, 222, 0, 131, 0, 145, 0, 132, 199, 220, 
		0, 234, 0, 152, 0, 166, 0, 84, 199, 0, 0, 64, 1, 0, 0, 200, 
		0, 156, 202, 255, 255, 174, 0, 188, 0, 132, 0, 148, 0, 84, 199, 0, 
		0, 64, 1, 0, 0, 200, 0, 156, 202, 255, 255, 24, 1, 64, 1, 160, 
		0, 200, 0, 84, 199, 143, 0, 44, 1, 6, 0, 194, 0, 188, 199, 0, 
		0, 143, 0, 6, 0, 194, 0, 196, 199, 0, 0, 64, 1, 0, 0, 200, 
		0, 156, 202, 255, 255, 104, 0, 128, 0, 58, 0, 72, 0, 224, 199, 64, 
		0, 116, 0, 76, 0, 106, 0, 228, 199, 116, 0, 168, 0, 76, 0, 106, 
		0, 232, 199, 64, 0, 116, 0, 106, 0, 136, 0, 236, 199, 116, 0, 168, 
		0, 106, 0, 136, 0, 240, 199, 0, 0, 64, 1, 0, 0, 200, 0, 156, 
		202, 255, 255, 186, 0, 202, 0, 157, 0, 173, 0, 32, 200, 243, 0, 3, 
		1, 131, 0, 147, 0, 28, 200, 12, 1, 28, 1, 168, 0, 184, 0, 84, 
		199, 0, 0, 64, 1, 0, 0, 200, 0, 156, 202, 255, 255, 119, 0, 174, 
		0, 82, 0, 128, 0, 56, 200, 70, 0, 137, 0, 62, 0, 111, 0, 132, 
		200, 188, 0, 250, 0, 68, 0, 152, 0, 80, 200, 0, 0, 64, 1, 0, 
		0, 200, 0, 156, 202, 255, 255, 236, 0, 252, 0, 112, 0, 128, 0, 76, 
		200, 188, 0, 250, 0, 64, 0, 152, 0, 92, 200, 62, 0, 152, 0, 56, 
		0, 133, 0, 120, 200, 0, 0, 64, 1, 0, 0, 200, 0, 156, 202, 255, 
		255, 119, 0, 174, 0, 82, 0, 128, 0, 72, 200, 70, 0, 139, 0, 62, 
		0, 111, 0, 84, 200, 236, 0, 252, 0, 112, 0, 128, 0, 76, 200, 0, 
		0, 64, 1, 0, 0, 200, 0, 156, 202, 255, 255, 236, 0, 252, 0, 112, 
		0, 128, 0, 76, 200, 188, 0, 250, 0, 64, 0, 152, 0, 88, 200, 62, 
		0, 152, 0, 56, 0, 133, 0, 120, 200, 0, 0, 64, 1, 0, 0, 200, 
		0, 156, 202, 255, 255, 13, 10, 13, 10, 68, 114, 101, 97, 109, 119, 101, 
		98, 32, 104, 97, 115, 32, 97, 110, 32, 69, 114, 114, 111, 114, 58, 13, 
		10, 85, 110, 97, 98, 108, 101, 32, 116, 111, 32, 97, 108, 108, 111, 99, 
		97, 116, 101, 32, 69, 120, 112, 97, 110, 100, 101, 100, 32, 77, 101, 109, 
		111, 114, 121, 46, 13, 10, 13, 10, 36, 13, 10, 13, 10, 68, 114, 101, 
		97, 109, 119, 101, 98, 32, 104, 97, 115, 32, 97, 110, 32, 69, 114, 114, 
		111, 114, 58, 13, 10, 83, 111, 117, 110, 100, 32, 66, 108, 97, 115, 116, 
		101, 114, 32, 99, 97, 114, 100, 32, 110, 111, 116, 32, 102, 111, 117, 110, 
		100, 32, 97, 116, 32, 97, 100, 100, 114, 101, 115, 115, 32, 50, 50, 48, 
		32, 72, 101, 120, 46, 13, 10, 13, 10, 36, 13, 10, 13, 10, 68, 114, 
		101, 97, 109, 119, 101, 98, 32, 104, 97, 115, 32, 97, 110, 32, 69, 114, 
		114, 111, 114, 58, 13, 10, 79, 117, 116, 32, 111, 102, 32, 66, 97, 115, 
		101, 32, 77, 101, 109, 111, 114, 121, 46, 13, 10, 13, 10, 36, 13, 10, 
		13, 10, 68, 114, 101, 97, 109, 119, 101, 98, 32, 104, 97, 115, 32, 97, 
		110, 32, 69, 114, 114, 111, 114, 58, 13, 10, 77, 101, 109, 111, 114, 121, 
		32, 68, 101, 97, 108, 108, 111, 99, 97, 116, 105, 111, 110, 32, 112, 114, 
		111, 98, 108, 101, 109, 46, 13, 10, 13, 10, 36, 13, 10, 13, 10, 68, 
		114, 101, 97, 109, 119, 101, 98, 32, 104, 97, 115, 32, 97, 110, 32, 69, 
		114, 114, 111, 114, 58, 13, 10, 65, 116, 32, 108, 101, 97, 115, 116, 32, 
		53, 57, 48, 75, 32, 111, 102, 32, 98, 97, 115, 101, 32, 109, 101, 109, 
		111, 114, 121, 32, 105, 115, 32, 114, 101, 113, 117, 105, 114, 101, 100, 46, 
		13, 10, 13, 10, 36, 13, 10, 13, 10, 68, 114, 101, 97, 109, 119, 101, 
		98, 32, 104, 97, 115, 32, 97, 110, 32, 69, 114, 114, 111, 114, 58, 13, 
		10, 83, 111, 117, 110, 100, 32, 66, 108, 97, 115, 116, 101, 114, 32, 110, 
		111, 116, 32, 102, 111, 117, 110, 100, 32, 111, 110, 32, 105, 110, 116, 101, 
		114, 117, 112, 116, 32, 48, 13, 10, 13, 10, 36, 13, 10, 13, 10, 68, 
		114, 101, 97, 109, 119, 101, 98, 32, 104, 97, 115, 32, 97, 110, 32, 69, 
		114, 114, 111, 114, 58, 13, 10, 85, 110, 97, 98, 108, 101, 32, 116, 111, 
		32, 115, 101, 108, 101, 99, 116, 32, 69, 77, 77, 32, 112, 97, 103, 101, 
		46, 13, 10, 13, 10, 36, 13, 10, 13, 10, 68, 114, 101, 97, 109, 119, 
		101, 98, 32, 104, 97, 115, 32, 97, 110, 32, 69, 114, 114, 111, 114, 58, 
		13, 10, 70, 105, 108, 101, 32, 110, 111, 116, 32, 102, 111, 117, 110, 100, 
		46, 99, 13, 10, 13, 10, 36, 68, 114, 101, 97, 109, 119, 101, 98, 32, 
		108, 111, 111, 107, 115, 32, 102, 111, 114, 32, 83, 111, 117, 110, 100, 32, 
		66, 108, 97, 115, 116, 101, 114, 32, 105, 110, 102, 111, 114, 109, 97, 116, 
		105, 111, 110, 32, 105, 110, 13, 10, 116, 104, 101, 32, 66, 76, 65, 83, 
		84, 69, 82, 32, 101, 110, 118, 105, 114, 111, 110, 109, 101, 110, 116, 32, 
		118, 97, 114, 105, 97, 98, 108, 101, 32, 40, 105, 110, 32, 121, 111, 117, 
		114, 32, 65, 85, 84, 79, 69, 88, 69, 67, 46, 66, 65, 84, 41, 13, 
		10, 13, 10, 73, 102, 32, 116, 104, 105, 115, 32, 105, 115, 32, 110, 111, 
		116, 32, 102, 111, 117, 110, 100, 32, 116, 104, 101, 110, 32, 73, 82, 81, 
		32, 55, 44, 32, 68, 77, 65, 32, 99, 104, 97, 110, 110, 101, 108, 32, 
		49, 32, 97, 110, 100, 32, 98, 97, 115, 101, 13, 10, 97, 100, 100, 114, 
		101, 115, 115, 32, 50, 50, 48, 104, 32, 97, 114, 101, 32, 97, 115, 115, 
		117, 109, 101, 100, 46, 13, 10, 13, 10, 84, 111, 32, 97, 108, 116, 101, 
		114, 32, 97, 110, 121, 32, 111, 114, 32, 97, 108, 108, 32, 111, 102, 32, 
		116, 104, 101, 115, 101, 32, 115, 101, 116, 116, 105, 110, 103, 115, 32, 121, 
		111, 117, 32, 99, 97, 110, 32, 115, 112, 101, 99, 105, 102, 121, 32, 116, 
		104, 101, 109, 13, 10, 111, 110, 32, 116, 104, 101, 32, 99, 111, 109, 109, 
		97, 110, 100, 32, 108, 105, 110, 101, 46, 32, 70, 111, 114, 32, 101, 120, 
		97, 109, 112, 108, 101, 58, 13, 10, 13, 10, 84, 121, 112, 101, 32, 68, 
		82, 69, 65, 77, 87, 69, 66, 32, 73, 55, 32, 65, 50, 50, 48, 32, 
		68, 49, 32, 116, 111, 32, 114, 117, 110, 32, 68, 114, 101, 97, 109, 119, 
		101, 98, 32, 111, 110, 32, 73, 82, 81, 32, 55, 44, 32, 68, 77, 65, 
		13, 10, 32, 99, 104, 97, 110, 110, 101, 108, 32, 49, 32, 97, 110, 100, 
		32, 98, 97, 115, 101, 32, 97, 100, 100, 114, 101, 115, 115, 32, 50, 50, 
		48, 104, 13, 10, 32, 68, 82, 69, 65, 77, 87, 69, 66, 32, 73, 53, 
		32, 116, 111, 32, 114, 117, 110, 32, 68, 114, 101, 97, 109, 119, 101, 98, 
		32, 111, 110, 32, 73, 82, 81, 32, 53, 32, 97, 110, 100, 13, 10, 32, 
		100, 101, 102, 97, 117, 108, 116, 32, 97, 100, 100, 114, 101, 115, 115, 32, 
		111, 102, 32, 50, 50, 48, 104, 44, 32, 68, 77, 65, 32, 49, 13, 10, 
		13, 10, 36, 13, 10, 13, 10, 84, 114, 121, 32, 116, 104, 101, 32, 68, 
		114, 101, 97, 109, 119, 101, 98, 32, 67, 68, 32, 105, 110, 32, 121, 111, 
		117, 114, 32, 115, 116, 101, 114, 101, 111, 46, 46, 46, 46, 13, 10, 13, 
		10, 13, 10, 36, 129, 0, 184, 0, 82, 0, 128, 0, 188, 200, 80, 0, 
		147, 0, 62, 0, 111, 0, 132, 200, 183, 0, 250, 0, 62, 0, 111, 0, 
		192, 200, 0, 0, 64, 1, 0, 0, 200, 0, 156, 202, 255, 255, 83, 80, 
		69, 69, 67, 72, 82, 50, 52, 67, 48, 48, 48, 53, 46, 82, 65, 87, 
		0, 135, 131, 129, 130, 44, 0, 70, 0, 32, 0, 46, 0, 116, 196, 0, 
		0, 50, 0, 0, 0, 180, 0, 128, 195, 226, 0, 244, 0, 10, 0, 26, 
		0, 44, 200, 226, 0, 244, 0, 26, 0, 40, 0, 48, 200, 240, 0, 4, 
		1, 100, 0, 124, 0, 200, 201, 0, 0, 64, 1, 0, 0, 200, 0, 208, 
		201, 255, 255, 44, 0, 70, 0, 32, 0, 46, 0, 116, 196, 0, 0, 50, 
		0, 0, 0, 180, 0, 128, 195, 18, 1, 36, 1, 10, 0, 26, 0, 44, 
		200, 18, 1, 36, 1, 26, 0, 40, 0, 48, 200, 240, 0, 4, 1, 100, 
		0, 124, 0, 200, 201, 0, 0, 64, 1, 0, 0, 200, 0, 208, 201, 255, 
		255, 0, 33, 10, 15, 255, 0, 22, 10, 15, 255, 0, 22, 0, 15, 255, 
		0, 11, 0, 15, 255, 0, 11, 10, 15, 255, 0, 0, 10, 15, 255, 1, 
		44, 10, 6, 255, 1, 44, 0, 13, 255, 2, 33, 0, 6, 255, 2, 22, 
		0, 5, 255, 2, 22, 10, 16, 255, 2, 11, 10, 16, 255, 3, 44, 0, 
		15, 255, 3, 33, 10, 6, 255, 3, 33, 0, 5, 255, 4, 11, 30, 6, 
		255, 4, 22, 30, 5, 255, 4, 22, 20, 13, 255, 10, 33, 30, 6, 255, 
		10, 22, 30, 6, 255, 9, 22, 10, 6, 255, 9, 22, 20, 16, 255, 9, 
		22, 30, 16, 255, 9, 22, 40, 16, 255, 9, 22, 50, 16, 255, 6, 11, 
		30, 6, 255, 6, 0, 10, 15, 255, 6, 0, 20, 15, 255, 6, 11, 20, 
		15, 255, 6, 22, 20, 15, 255, 7, 11, 20, 6, 255, 7, 0, 20, 6, 
		255, 7, 0, 30, 6, 255, 55, 44, 0, 5, 255, 55, 44, 10, 5, 255, 
		5, 22, 30, 6, 255, 5, 22, 20, 15, 255, 5, 22, 10, 15, 255, 24, 
		22, 0, 15, 255, 24, 33, 0, 15, 255, 24, 44, 0, 15, 255, 24, 33, 
		10, 15, 255, 8, 0, 10, 6, 255, 8, 11, 10, 6, 255, 8, 22, 10, 
		6, 255, 8, 33, 10, 6, 255, 8, 33, 20, 6, 255, 8, 33, 30, 6, 
		255, 8, 33, 40, 6, 255, 8, 22, 40, 6, 255, 8, 11, 40, 6, 255, 
		11, 11, 20, 12, 255, 11, 11, 30, 12, 255, 11, 22, 20, 12, 255, 11, 
		22, 30, 12, 255, 12, 22, 20, 12, 255, 13, 22, 20, 12, 255, 13, 33, 
		20, 12, 255, 14, 44, 20, 12, 255, 14, 33, 0, 12, 255, 14, 33, 10, 
		12, 255, 14, 33, 20, 12, 255, 14, 33, 30, 12, 255, 14, 33, 40, 12, 
		255, 14, 22, 0, 16, 255, 19, 0, 0, 12, 255, 20, 0, 20, 16, 255, 
		20, 0, 30, 16, 255, 20, 11, 30, 16, 255, 20, 0, 40, 16, 255, 20, 
		11, 40, 16, 255, 21, 11, 10, 15, 255, 21, 11, 20, 15, 255, 21, 0, 
		20, 15, 255, 21, 22, 20, 15, 255, 21, 33, 20, 15, 255, 21, 44, 20, 
		15, 255, 21, 44, 10, 15, 255, 22, 22, 10, 16, 255, 22, 22, 20, 16, 
		255, 23, 22, 30, 13, 255, 23, 22, 40, 13, 255, 23, 33, 40, 13, 255, 
		23, 11, 40, 13, 255, 23, 0, 40, 13, 255, 23, 0, 50, 13, 255, 25, 
		11, 40, 16, 255, 25, 11, 50, 16, 255, 25, 0, 50, 16, 255, 27, 11, 
		20, 16, 255, 27, 11, 30, 16, 255, 29, 11, 10, 16, 255, 45, 22, 30, 
		12, 255, 45, 22, 40, 12, 255, 45, 22, 50, 12, 255, 46, 22, 40, 12, 
		255, 46, 11, 50, 12, 255, 46, 22, 50, 12, 255, 46, 33, 50, 12, 255, 
		47, 0, 0, 12, 255, 26, 22, 20, 16, 255, 26, 33, 10, 16, 255, 26, 
		33, 20, 16, 255, 26, 33, 30, 16, 255, 26, 44, 30, 16, 255, 26, 22, 
		30, 16, 255, 26, 11, 30, 16, 255, 26, 11, 20, 16, 255, 26, 0, 20, 
		16, 255, 26, 11, 40, 16, 255, 26, 0, 40, 16, 255, 26, 22, 40, 16, 
		255, 26, 11, 50, 16, 255, 28, 0, 30, 15, 255, 28, 0, 20, 15, 255, 
		28, 0, 40, 15, 255, 28, 11, 30, 15, 255, 28, 11, 20, 15, 255, 28, 
		22, 30, 15, 255, 28, 22, 20, 15, 255, 255, 79, 66, 74, 69, 67, 84, 
		32, 78, 65, 77, 69, 32, 79, 78, 69, 32, 0, 16, 18, 18, 17, 16, 
		16, 16, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 
		2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 
		0, 8, 0, 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 0, 0, 13, 
		0, 65, 83, 68, 70, 71, 72, 74, 75, 76, 0, 0, 0, 0, 0, 90, 
		88, 67, 86, 66, 78, 77, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 68, 82, 69, 65, 77, 87, 69, 66, 32, 68, 
		65, 84, 65, 32, 70, 73, 76, 69, 32, 67, 79, 80, 89, 82, 73, 71, 
		72, 84, 32, 49, 57, 57, 50, 32, 67, 82, 69, 65, 84, 73, 86, 69, 
		32, 82, 69, 65, 76, 73, 84, 89, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 48, 0, 5, 255, 33, 10, 255, 255, 255, 0, 1, 6, 2, 255, 3, 
		255, 255, 255, 255, 255, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 49, 0, 1, 255, 44, 10, 255, 255, 255, 0, 7, 2, 255, 255, 255, 
		255, 6, 255, 255, 255, 1, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 50, 0, 2, 255, 33, 0, 255, 255, 255, 0, 1, 0, 255, 255, 1, 
		255, 3, 255, 255, 255, 2, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 51, 0, 5, 255, 33, 10, 255, 255, 255, 0, 2, 2, 0, 2, 4, 
		255, 0, 255, 255, 255, 3, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 52, 0, 23, 255, 11, 30, 255, 255, 255, 0, 1, 4, 0, 5, 255, 
		255, 3, 255, 255, 255, 4, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 53, 0, 5, 255, 22, 30, 255, 255, 255, 0, 1, 2, 0, 4, 255, 
		255, 3, 255, 255, 255, 5, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 54, 0, 5, 255, 11, 30, 255, 255, 255, 0, 1, 0, 0, 1, 2, 
		255, 0, 255, 255, 255, 6, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 55, 0, 255, 255, 0, 20, 255, 255, 255, 0, 2, 2, 255, 255, 255, 
		255, 0, 255, 255, 255, 7, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 56, 0, 8, 255, 0, 10, 255, 255, 255, 0, 1, 2, 255, 255, 255, 
		255, 0, 11, 40, 0, 8, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 57, 0, 9, 255, 22, 10, 255, 255, 255, 0, 4, 6, 255, 255, 255, 
		255, 0, 255, 255, 255, 9, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		49, 48, 0, 10, 255, 33, 30, 255, 255, 255, 0, 2, 0, 255, 255, 2, 
		2, 4, 22, 30, 255, 10, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		49, 49, 0, 11, 255, 11, 20, 255, 255, 255, 0, 0, 4, 255, 255, 255, 
		255, 255, 255, 255, 255, 11, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		49, 50, 0, 12, 255, 22, 20, 255, 255, 255, 0, 1, 4, 255, 255, 255, 
		255, 255, 255, 255, 255, 12, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		49, 51, 0, 12, 255, 22, 20, 255, 255, 255, 0, 1, 4, 255, 255, 255, 
		255, 255, 255, 255, 255, 13, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		49, 52, 0, 14, 255, 44, 20, 255, 255, 255, 0, 0, 6, 255, 255, 255, 
		255, 255, 255, 255, 255, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		49, 57, 0, 19, 255, 0, 0, 255, 255, 255, 0, 0, 4, 255, 255, 255, 
		255, 255, 255, 255, 255, 19, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 48, 0, 22, 255, 0, 20, 255, 255, 255, 0, 1, 4, 2, 15, 255, 
		255, 255, 255, 255, 255, 20, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 49, 0, 5, 255, 11, 10, 255, 255, 255, 0, 1, 4, 2, 15, 1, 
		255, 255, 255, 255, 255, 21, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 50, 0, 22, 255, 22, 10, 255, 255, 255, 0, 0, 4, 255, 255, 1, 
		255, 255, 255, 255, 255, 22, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 51, 0, 23, 255, 22, 30, 255, 255, 255, 0, 1, 4, 2, 15, 3, 
		255, 255, 255, 255, 255, 23, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 52, 0, 5, 255, 44, 0, 255, 255, 255, 0, 1, 6, 2, 15, 255, 
		255, 255, 255, 255, 255, 24, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 53, 0, 22, 255, 11, 40, 255, 255, 255, 0, 1, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 25, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 54, 0, 9, 255, 22, 20, 255, 255, 255, 0, 4, 2, 255, 255, 255, 
		255, 255, 255, 255, 255, 26, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 55, 0, 22, 255, 11, 20, 255, 255, 255, 0, 0, 6, 255, 255, 255, 
		255, 255, 255, 255, 255, 27, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 56, 0, 5, 255, 11, 30, 255, 255, 255, 0, 0, 0, 255, 255, 2, 
		255, 255, 255, 255, 255, 28, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 57, 0, 22, 255, 11, 10, 255, 255, 255, 0, 0, 2, 255, 255, 255, 
		255, 255, 255, 255, 255, 29, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 53, 0, 5, 255, 22, 10, 255, 255, 255, 0, 1, 4, 1, 15, 255, 
		255, 255, 255, 255, 255, 5, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 52, 0, 23, 255, 22, 20, 255, 255, 255, 0, 1, 4, 2, 15, 255, 
		255, 255, 255, 255, 255, 4, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		49, 48, 0, 10, 255, 22, 30, 255, 255, 255, 0, 3, 6, 255, 255, 255, 
		255, 255, 255, 255, 255, 10, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		49, 50, 0, 12, 255, 22, 20, 255, 255, 255, 0, 0, 2, 255, 255, 255, 
		255, 255, 255, 255, 255, 12, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 51, 0, 5, 255, 44, 0, 255, 255, 255, 0, 1, 6, 2, 255, 4, 
		255, 255, 255, 255, 255, 3, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 52, 0, 5, 255, 22, 0, 255, 255, 255, 0, 3, 6, 0, 255, 255, 
		255, 255, 33, 0, 3, 24, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 50, 0, 22, 255, 22, 20, 255, 255, 255, 0, 1, 4, 255, 255, 255, 
		255, 255, 255, 255, 255, 22, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 50, 0, 22, 255, 22, 20, 255, 255, 255, 0, 0, 2, 255, 255, 255, 
		255, 255, 255, 255, 255, 22, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		49, 49, 0, 11, 255, 22, 30, 255, 255, 255, 0, 0, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 11, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 56, 0, 5, 255, 11, 20, 255, 255, 255, 0, 0, 6, 255, 255, 2, 
		255, 255, 255, 255, 255, 28, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 49, 0, 5, 255, 11, 10, 255, 255, 255, 0, 1, 4, 2, 15, 1, 
		255, 255, 255, 255, 255, 21, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		50, 54, 0, 9, 255, 0, 40, 255, 255, 255, 0, 0, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 26, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		49, 57, 0, 19, 255, 0, 0, 255, 255, 255, 0, 2, 2, 255, 255, 255, 
		255, 255, 255, 255, 255, 19, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 56, 0, 8, 255, 11, 40, 255, 255, 255, 0, 0, 4, 255, 255, 255, 
		255, 255, 255, 255, 255, 8, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		48, 49, 0, 1, 255, 44, 10, 255, 255, 255, 0, 3, 6, 255, 255, 255, 
		255, 255, 255, 255, 255, 1, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		52, 53, 0, 35, 255, 22, 30, 255, 255, 255, 0, 0, 6, 255, 255, 255, 
		255, 255, 255, 255, 255, 45, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		52, 54, 0, 35, 255, 22, 40, 255, 255, 255, 0, 0, 4, 255, 255, 255, 
		255, 255, 255, 255, 255, 46, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		52, 55, 0, 35, 255, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 47, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		52, 53, 0, 35, 255, 22, 30, 255, 255, 255, 0, 4, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 45, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		52, 54, 0, 35, 255, 22, 50, 255, 255, 255, 0, 0, 4, 255, 255, 255, 
		255, 255, 255, 255, 255, 46, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		53, 48, 0, 35, 255, 22, 30, 255, 255, 255, 0, 0, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 50, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		53, 49, 0, 35, 255, 11, 30, 255, 255, 255, 0, 0, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 51, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		53, 50, 0, 35, 255, 22, 30, 255, 255, 255, 0, 0, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 52, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		53, 51, 0, 35, 255, 33, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 53, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		53, 52, 0, 35, 255, 0, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 54, 68, 82, 69, 65, 77, 87, 69, 66, 46, 82, 
		53, 53, 0, 14, 255, 44, 0, 255, 255, 255, 0, 0, 0, 255, 255, 255, 
		255, 255, 255, 255, 255, 55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 5, 0, 3, 2, 4, 1, 10, 9, 8, 6, 
		11, 4, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 2, 0, 
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		2, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 2, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 1, 2, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 1, 1, 2, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 1, 1, 1, 2, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		1, 1, 1, 1, 1, 68, 82, 69, 65, 77, 87, 69, 66, 46, 68, 48, 
		48, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 68, 48, 49, 0, 68, 
		82, 69, 65, 77, 87, 69, 66, 46, 68, 48, 50, 0, 68, 82, 69, 65, 
		77, 87, 69, 66, 46, 68, 48, 51, 0, 68, 82, 69, 65, 77, 87, 69, 
		66, 46, 68, 48, 52, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 68, 
		48, 53, 0, 68, 82, 69, 65, 77, 87, 69, 66, 46, 68, 48, 54, 0, 
		68, 82, 69, 65, 77, 87, 69, 66, 46, 68, 69, 77, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
data.assign(src, src + sizeof(src));
	};


void __dispatch_call(Context &context, unsigned addr) {
	switch(addr) {
		case 0xc000: alleybarksound(context); break;
		case 0xc004: intromusic(context); break;
		case 0xc008: foghornsound(context); break;
		case 0xc00c: receptionist(context); break;
		case 0xc010: smokebloke(context); break;
		case 0xc014: attendant(context); break;
		case 0xc018: manasleep(context); break;
		case 0xc01c: eden(context); break;
		case 0xc020: edeninbath(context); break;
		case 0xc024: malefan(context); break;
		case 0xc028: femalefan(context); break;
		case 0xc02c: louis(context); break;
		case 0xc030: louischair(context); break;
		case 0xc034: manasleep2(context); break;
		case 0xc038: mansatstill(context); break;
		case 0xc03c: tattooman(context); break;
		case 0xc040: drinker(context); break;
		case 0xc044: bartender(context); break;
		case 0xc048: othersmoker(context); break;
		case 0xc04c: barwoman(context); break;
		case 0xc050: interviewer(context); break;
		case 0xc054: soldier1(context); break;
		case 0xc058: rockstar(context); break;
		case 0xc05c: helicopter(context); break;
		case 0xc060: mugger(context); break;
		case 0xc064: aide(context); break;
		case 0xc068: businessman(context); break;
		case 0xc06c: poolguard(context); break;
		case 0xc070: security(context); break;
		case 0xc074: heavy(context); break;
		case 0xc078: bossman(context); break;
		case 0xc07c: gamer(context); break;
		case 0xc080: sparkydrip(context); break;
		case 0xc084: carparkdrip(context); break;
		case 0xc088: keeper(context); break;
		case 0xc08c: candles1(context); break;
		case 0xc090: smallcandle(context); break;
		case 0xc094: intromagic1(context); break;
		case 0xc098: candles(context); break;
		case 0xc09c: candles2(context); break;
		case 0xc0a0: gates(context); break;
		case 0xc0a4: intromagic2(context); break;
		case 0xc0a8: intromagic3(context); break;
		case 0xc0ac: intromonks1(context); break;
		case 0xc0b0: intromonks2(context); break;
		case 0xc0b4: handclap(context); break;
		case 0xc0b8: monks2text(context); break;
		case 0xc0bc: intro1text(context); break;
		case 0xc0c0: intro2text(context); break;
		case 0xc0c4: intro3text(context); break;
		case 0xc0c8: monkandryan(context); break;
		case 0xc0cc: endgameseq(context); break;
		case 0xc0d0: rollendcredits(context); break;
		case 0xc0d4: priest(context); break;
		case 0xc0d8: madmanstelly(context); break;
		case 0xc0dc: madman(context); break;
		case 0xc0e0: madmantext(context); break;
		case 0xc0e4: madmode(context); break;
		case 0xc0e8: priesttext(context); break;
		case 0xc0ec: textforend(context); break;
		case 0xc0f0: textformonk(context); break;
		case 0xc0f4: drunk(context); break;
		case 0xc0f8: advisor(context); break;
		case 0xc0fc: copper(context); break;
		case 0xc100: sparky(context); break;
		case 0xc104: train(context); break;
		case 0xc108: addtopeoplelist(context); break;
		case 0xc10c: showgamereel(context); break;
		case 0xc110: checkspeed(context); break;
		case 0xc114: clearsprites(context); break;
		case 0xc118: makesprite(context); break;
		case 0xc11c: delsprite(context); break;
		case 0xc120: spriteupdate(context); break;
		case 0xc124: printsprites(context); break;
		case 0xc128: printasprite(context); break;
		case 0xc12c: checkone(context); break;
		case 0xc130: findsource(context); break;
		case 0xc134: initman(context); break;
		case 0xc138: mainman(context); break;
		case 0xc13c: aboutturn(context); break;
		case 0xc140: walking(context); break;
		case 0xc144: facerightway(context); break;
		case 0xc148: checkforexit(context); break;
		case 0xc14c: adjustdown(context); break;
		case 0xc150: adjustup(context); break;
		case 0xc154: adjustleft(context); break;
		case 0xc158: adjustright(context); break;
		case 0xc15c: reminders(context); break;
		case 0xc160: initrain(context); break;
		case 0xc164: splitintolines(context); break;
		case 0xc168: getblockofpixel(context); break;
		case 0xc16c: showrain(context); break;
		case 0xc170: backobject(context); break;
		case 0xc174: liftsprite(context); break;
		case 0xc178: liftnoise(context); break;
		case 0xc17c: random(context); break;
		case 0xc180: steady(context); break;
		case 0xc184: constant(context); break;
		case 0xc188: doorway(context); break;
		case 0xc18c: widedoor(context); break;
		case 0xc190: dodoor(context); break;
		case 0xc194: lockeddoorway(context); break;
		case 0xc198: updatepeople(context); break;
		case 0xc19c: getreelframeax(context); break;
		case 0xc1a0: reelsonscreen(context); break;
		case 0xc1a4: plotreel(context); break;
		case 0xc1a8: soundonreels(context); break;
		case 0xc1ac: reconstruct(context); break;
		case 0xc1b0: dealwithspecial(context); break;
		case 0xc1b4: movemap(context); break;
		case 0xc1b8: getreelstart(context); break;
		case 0xc1bc: showreelframe(context); break;
		case 0xc1c0: deleverything(context); break;
		case 0xc1c4: dumpeverything(context); break;
		case 0xc1c8: allocatework(context); break;
		case 0xc1cc: showpcx(context); break;
		case 0xc1d0: readabyte(context); break;
		case 0xc1d4: readoneblock(context); break;
		case 0xc1d8: loadpalfromiff(context); break;
		case 0xc1dc: setmode(context); break;
		case 0xc1e0: cls(context); break;
		case 0xc1e4: printundermon(context); break;
		case 0xc1e8: worktoscreen(context); break;
		case 0xc1ec: paneltomap(context); break;
		case 0xc1f0: maptopanel(context); break;
		case 0xc1f4: dumpmap(context); break;
		case 0xc1f8: pixelcheckset(context); break;
		case 0xc1fc: createpanel(context); break;
		case 0xc200: createpanel2(context); break;
		case 0xc204: clearwork(context); break;
		case 0xc208: vsync(context); break;
		case 0xc20c: doshake(context); break;
		case 0xc210: zoom(context); break;
		case 0xc214: delthisone(context); break;
		case 0xc218: multiget(context); break;
		case 0xc21c: multiput(context); break;
		case 0xc220: multidump(context); break;
		case 0xc224: width160(context); break;
		case 0xc228: doblocks(context); break;
		case 0xc22c: showframe(context); break;
		case 0xc230: frameoutv(context); break;
		case 0xc234: frameoutnm(context); break;
		case 0xc238: frameoutbh(context); break;
		case 0xc23c: frameoutfx(context); break;
		case 0xc240: transferinv(context); break;
		case 0xc244: transfermap(context); break;
		case 0xc248: fadedos(context); break;
		case 0xc24c: dofade(context); break;
		case 0xc250: clearendpal(context); break;
		case 0xc254: clearpalette(context); break;
		case 0xc258: fadescreenup(context); break;
		case 0xc25c: fadetowhite(context); break;
		case 0xc260: fadefromwhite(context); break;
		case 0xc264: fadescreenups(context); break;
		case 0xc268: fadescreendownhalf(context); break;
		case 0xc26c: fadescreenuphalf(context); break;
		case 0xc270: fadescreendown(context); break;
		case 0xc274: fadescreendowns(context); break;
		case 0xc278: clearstartpal(context); break;
		case 0xc27c: showgun(context); break;
		case 0xc280: rollendcredits2(context); break;
		case 0xc284: rollem(context); break;
		case 0xc288: fadecalculation(context); break;
		case 0xc28c: greyscalesum(context); break;
		case 0xc290: showgroup(context); break;
		case 0xc294: paltostartpal(context); break;
		case 0xc298: endpaltostart(context); break;
		case 0xc29c: startpaltoend(context); break;
		case 0xc2a0: paltoendpal(context); break;
		case 0xc2a4: allpalette(context); break;
		case 0xc2a8: dumpcurrent(context); break;
		case 0xc2ac: fadedownmon(context); break;
		case 0xc2b0: fadeupmon(context); break;
		case 0xc2b4: fadeupmonfirst(context); break;
		case 0xc2b8: fadeupyellows(context); break;
		case 0xc2bc: initialmoncols(context); break;
		case 0xc2c0: titles(context); break;
		case 0xc2c4: endgame(context); break;
		case 0xc2c8: monkspeaking(context); break;
		case 0xc2cc: showmonk(context); break;
		case 0xc2d0: gettingshot(context); break;
		case 0xc2d4: credits(context); break;
		case 0xc2d8: biblequote(context); break;
		case 0xc2dc: hangone(context); break;
		case 0xc2e0: intro(context); break;
		case 0xc2e4: runintroseq(context); break;
		case 0xc2e8: runendseq(context); break;
		case 0xc2ec: loadintroroom(context); break;
		case 0xc2f0: mode640x480(context); break;
		case 0xc2f4: set16colpalette(context); break;
		case 0xc2f8: realcredits(context); break;
		case 0xc2fc: printchar(context); break;
		case 0xc300: kernchars(context); break;
		case 0xc304: printslow(context); break;
		case 0xc308: waitframes(context); break;
		case 0xc30c: printboth(context); break;
		case 0xc310: printdirect(context); break;
		case 0xc314: monprint(context); break;
		case 0xc318: getnumber(context); break;
		case 0xc31c: getnextword(context); break;
		case 0xc320: modifychar(context); break;
		case 0xc324: fillryan(context); break;
		case 0xc328: fillopen(context); break;
		case 0xc32c: findallryan(context); break;
		case 0xc330: findallopen(context); break;
		case 0xc334: obtoinv(context); break;
		case 0xc338: isitworn(context); break;
		case 0xc33c: makeworn(context); break;
		case 0xc340: examineob(context); break;
		case 0xc344: makemainscreen(context); break;
		case 0xc348: getbackfromob(context); break;
		case 0xc34c: incryanpage(context); break;
		case 0xc350: openinv(context); break;
		case 0xc354: showryanpage(context); break;
		case 0xc358: openob(context); break;
		case 0xc35c: obicons(context); break;
		case 0xc360: examicon(context); break;
		case 0xc364: obpicture(context); break;
		case 0xc368: describeob(context); break;
		case 0xc36c: additionaltext(context); break;
		case 0xc370: obsthatdothings(context); break;
		case 0xc374: getobtextstart(context); break;
		case 0xc378: searchforsame(context); break;
		case 0xc37c: findnextcolon(context); break;
		case 0xc380: inventory(context); break;
		case 0xc384: setpickup(context); break;
		case 0xc388: examinventory(context); break;
		case 0xc38c: reexfrominv(context); break;
		case 0xc390: reexfromopen(context); break;
		case 0xc394: swapwithinv(context); break;
		case 0xc398: swapwithopen(context); break;
		case 0xc39c: intoinv(context); break;
		case 0xc3a0: deletetaken(context); break;
		case 0xc3a4: outofinv(context); break;
		case 0xc3a8: getfreead(context); break;
		case 0xc3ac: getexad(context); break;
		case 0xc3b0: geteitherad(context); break;
		case 0xc3b4: getanyad(context); break;
		case 0xc3b8: getanyaddir(context); break;
		case 0xc3bc: getopenedsize(context); break;
		case 0xc3c0: getsetad(context); break;
		case 0xc3c4: findinvpos(context); break;
		case 0xc3c8: findopenpos(context); break;
		case 0xc3cc: dropobject(context); break;
		case 0xc3d0: droperror(context); break;
		case 0xc3d4: cantdrop(context); break;
		case 0xc3d8: wornerror(context); break;
		case 0xc3dc: removeobfrominv(context); break;
		case 0xc3e0: selectopenob(context); break;
		case 0xc3e4: useopened(context); break;
		case 0xc3e8: errormessage1(context); break;
		case 0xc3ec: errormessage2(context); break;
		case 0xc3f0: errormessage3(context); break;
		case 0xc3f4: checkobjectsize(context); break;
		case 0xc3f8: outofopen(context); break;
		case 0xc3fc: transfertoex(context); break;
		case 0xc400: pickupconts(context); break;
		case 0xc404: transfercontoex(context); break;
		case 0xc408: transfertext(context); break;
		case 0xc40c: getexpos(context); break;
		case 0xc410: purgealocation(context); break;
		case 0xc414: emergencypurge(context); break;
		case 0xc418: purgeanitem(context); break;
		case 0xc41c: deleteexobject(context); break;
		case 0xc420: deleteexframe(context); break;
		case 0xc424: deleteextext(context); break;
		case 0xc428: blockget(context); break;
		case 0xc42c: drawfloor(context); break;
		case 0xc430: calcmapad(context); break;
		case 0xc434: getdimension(context); break;
		case 0xc438: addalong(context); break;
		case 0xc43c: addlength(context); break;
		case 0xc440: drawflags(context); break;
		case 0xc444: eraseoldobs(context); break;
		case 0xc448: showallobs(context); break;
		case 0xc44c: makebackob(context); break;
		case 0xc450: showallfree(context); break;
		case 0xc454: showallex(context); break;
		case 0xc458: calcfrframe(context); break;
		case 0xc45c: finalframe(context); break;
		case 0xc460: adjustlen(context); break;
		case 0xc464: getmapad(context); break;
		case 0xc468: getxad(context); break;
		case 0xc46c: getyad(context); break;
		case 0xc470: autolook(context); break;
		case 0xc474: look(context); break;
		case 0xc478: dolook(context); break;
		case 0xc47c: redrawmainscrn(context); break;
		case 0xc480: getback1(context); break;
		case 0xc484: talk(context); break;
		case 0xc488: convicons(context); break;
		case 0xc48c: getpersframe(context); break;
		case 0xc490: starttalk(context); break;
		case 0xc494: getpersontext(context); break;
		case 0xc498: moretalk(context); break;
		case 0xc49c: dosometalk(context); break;
		case 0xc4a0: hangonpq(context); break;
		case 0xc4a4: redes(context); break;
		case 0xc4a8: newplace(context); break;
		case 0xc4ac: selectlocation(context); break;
		case 0xc4b0: showcity(context); break;
		case 0xc4b4: lookatplace(context); break;
		case 0xc4b8: getundercentre(context); break;
		case 0xc4bc: putundercentre(context); break;
		case 0xc4c0: locationpic(context); break;
		case 0xc4c4: getdestinfo(context); break;
		case 0xc4c8: showarrows(context); break;
		case 0xc4cc: nextdest(context); break;
		case 0xc4d0: lastdest(context); break;
		case 0xc4d4: destselect(context); break;
		case 0xc4d8: getlocation(context); break;
		case 0xc4dc: setlocation(context); break;
		case 0xc4e0: resetlocation(context); break;
		case 0xc4e4: readdesticon(context); break;
		case 0xc4e8: readcitypic(context); break;
		case 0xc4ec: usemon(context); break;
		case 0xc4f0: printoutermon(context); break;
		case 0xc4f4: loadpersonal(context); break;
		case 0xc4f8: loadnews(context); break;
		case 0xc4fc: loadcart(context); break;
		case 0xc500: lookininterface(context); break;
		case 0xc504: turnonpower(context); break;
		case 0xc508: randomaccess(context); break;
		case 0xc50c: powerlighton(context); break;
		case 0xc510: powerlightoff(context); break;
		case 0xc514: accesslighton(context); break;
		case 0xc518: accesslightoff(context); break;
		case 0xc51c: locklighton(context); break;
		case 0xc520: locklightoff(context); break;
		case 0xc524: input(context); break;
		case 0xc528: makecaps(context); break;
		case 0xc52c: delchar(context); break;
		case 0xc530: execcommand(context); break;
		case 0xc534: neterror(context); break;
		case 0xc538: dircom(context); break;
		case 0xc53c: searchforfiles(context); break;
		case 0xc540: signon(context); break;
		case 0xc544: showkeys(context); break;
		case 0xc548: read(context); break;
		case 0xc54c: dirfile(context); break;
		case 0xc550: getkeyandlogo(context); break;
		case 0xc554: searchforstring(context); break;
		case 0xc558: parser(context); break;
		case 0xc55c: scrollmonitor(context); break;
		case 0xc560: lockmon(context); break;
		case 0xc564: monitorlogo(context); break;
		case 0xc568: printlogo(context); break;
		case 0xc56c: showcurrentfile(context); break;
		case 0xc570: monmessage(context); break;
		case 0xc574: processtrigger(context); break;
		case 0xc578: triggermessage(context); break;
		case 0xc57c: printcurs(context); break;
		case 0xc580: delcurs(context); break;
		case 0xc584: useobject(context); break;
		case 0xc588: useroutine(context); break;
		case 0xc58c: wheelsound(context); break;
		case 0xc590: runtap(context); break;
		case 0xc594: playguitar(context); break;
		case 0xc598: hotelcontrol(context); break;
		case 0xc59c: hotelbell(context); break;
		case 0xc5a0: opentomb(context); break;
		case 0xc5a4: usetrainer(context); break;
		case 0xc5a8: nothelderror(context); break;
		case 0xc5ac: usepipe(context); break;
		case 0xc5b0: usefullcart(context); break;
		case 0xc5b4: useplinth(context); break;
		case 0xc5b8: chewy(context); break;
		case 0xc5bc: useladder(context); break;
		case 0xc5c0: useladderb(context); break;
		case 0xc5c4: slabdoora(context); break;
		case 0xc5c8: slabdoorb(context); break;
		case 0xc5cc: slabdoord(context); break;
		case 0xc5d0: slabdoorc(context); break;
		case 0xc5d4: slabdoore(context); break;
		case 0xc5d8: slabdoorf(context); break;
		case 0xc5dc: useslab(context); break;
		case 0xc5e0: usecart(context); break;
		case 0xc5e4: useclearbox(context); break;
		case 0xc5e8: usecoveredbox(context); break;
		case 0xc5ec: userailing(context); break;
		case 0xc5f0: useopenbox(context); break;
		case 0xc5f4: wearwatch(context); break;
		case 0xc5f8: wearshades(context); break;
		case 0xc5fc: sitdowninbar(context); break;
		case 0xc600: usechurchhole(context); break;
		case 0xc604: usehole(context); break;
		case 0xc608: usealtar(context); break;
		case 0xc60c: opentvdoor(context); break;
		case 0xc610: usedryer(context); break;
		case 0xc614: openlouis(context); break;
		case 0xc618: nextcolon(context); break;
		case 0xc61c: openyourneighbour(context); break;
		case 0xc620: usewindow(context); break;
		case 0xc624: usebalcony(context); break;
		case 0xc628: openryan(context); break;
		case 0xc62c: openpoolboss(context); break;
		case 0xc630: openeden(context); break;
		case 0xc634: opensarters(context); break;
		case 0xc638: isitright(context); break;
		case 0xc63c: drawitall(context); break;
		case 0xc640: openhoteldoor(context); break;
		case 0xc644: openhoteldoor2(context); break;
		case 0xc648: grafittidoor(context); break;
		case 0xc64c: trapdoor(context); break;
		case 0xc650: callhotellift(context); break;
		case 0xc654: calledenslift(context); break;
		case 0xc658: calledensdlift(context); break;
		case 0xc65c: usepoolreader(context); break;
		case 0xc660: uselighter(context); break;
		case 0xc664: showseconduse(context); break;
		case 0xc668: usecardreader1(context); break;
		case 0xc66c: usecardreader2(context); break;
		case 0xc670: usecardreader3(context); break;
		case 0xc674: usecashcard(context); break;
		case 0xc678: lookatcard(context); break;
		case 0xc67c: moneypoke(context); break;
		case 0xc680: usecontrol(context); break;
		case 0xc684: usehatch(context); break;
		case 0xc688: usewire(context); break;
		case 0xc68c: usehandle(context); break;
		case 0xc690: useelevator1(context); break;
		case 0xc694: showfirstuse(context); break;
		case 0xc698: useelevator3(context); break;
		case 0xc69c: useelevator4(context); break;
		case 0xc6a0: useelevator2(context); break;
		case 0xc6a4: useelevator5(context); break;
		case 0xc6a8: usekey(context); break;
		case 0xc6ac: usestereo(context); break;
		case 0xc6b0: usecooker(context); break;
		case 0xc6b4: useaxe(context); break;
		case 0xc6b8: useelvdoor(context); break;
		case 0xc6bc: withwhat(context); break;
		case 0xc6c0: selectob(context); break;
		case 0xc6c4: compare(context); break;
		case 0xc6c8: findsetobject(context); break;
		case 0xc6cc: findexobject(context); break;
		case 0xc6d0: isryanholding(context); break;
		case 0xc6d4: checkinside(context); break;
		case 0xc6d8: usetext(context); break;
		case 0xc6dc: putbackobstuff(context); break;
		case 0xc6e0: showpuztext(context); break;
		case 0xc6e4: findpuztext(context); break;
		case 0xc6e8: placesetobject(context); break;
		case 0xc6ec: removesetobject(context); break;
		case 0xc6f0: issetobonmap(context); break;
		case 0xc6f4: placefreeobject(context); break;
		case 0xc6f8: removefreeobject(context); break;
		case 0xc6fc: findormake(context); break;
		case 0xc700: switchryanon(context); break;
		case 0xc704: switchryanoff(context); break;
		case 0xc708: setallchanges(context); break;
		case 0xc70c: dochange(context); break;
		case 0xc710: autoappear(context); break;
		case 0xc714: getundertimed(context); break;
		case 0xc718: putundertimed(context); break;
		case 0xc71c: dumptimedtext(context); break;
		case 0xc720: setuptimeduse(context); break;
		case 0xc724: setuptimedtemp(context); break;
		case 0xc728: usetimedtext(context); break;
		case 0xc72c: edenscdplayer(context); break;
		case 0xc730: usewall(context); break;
		case 0xc734: usechurchgate(context); break;
		case 0xc738: usegun(context); break;
		case 0xc73c: useshield(context); break;
		case 0xc740: usebuttona(context); break;
		case 0xc744: useplate(context); break;
		case 0xc748: usewinch(context); break;
		case 0xc74c: entercode(context); break;
		case 0xc750: loadkeypad(context); break;
		case 0xc754: quitkey(context); break;
		case 0xc758: addtopresslist(context); break;
		case 0xc75c: buttonone(context); break;
		case 0xc760: buttontwo(context); break;
		case 0xc764: buttonthree(context); break;
		case 0xc768: buttonfour(context); break;
		case 0xc76c: buttonfive(context); break;
		case 0xc770: buttonsix(context); break;
		case 0xc774: buttonseven(context); break;
		case 0xc778: buttoneight(context); break;
		case 0xc77c: buttonnine(context); break;
		case 0xc780: buttonnought(context); break;
		case 0xc784: buttonenter(context); break;
		case 0xc788: buttonpress(context); break;
		case 0xc78c: showouterpad(context); break;
		case 0xc790: showkeypad(context); break;
		case 0xc794: singlekey(context); break;
		case 0xc798: dumpkeypad(context); break;
		case 0xc79c: usemenu(context); break;
		case 0xc7a0: dumpmenu(context); break;
		case 0xc7a4: getundermenu(context); break;
		case 0xc7a8: putundermenu(context); break;
		case 0xc7ac: showoutermenu(context); break;
		case 0xc7b0: showmenu(context); break;
		case 0xc7b4: loadmenu(context); break;
		case 0xc7b8: viewfolder(context); break;
		case 0xc7bc: nextfolder(context); break;
		case 0xc7c0: folderhints(context); break;
		case 0xc7c4: lastfolder(context); break;
		case 0xc7c8: loadfolder(context); break;
		case 0xc7cc: showfolder(context); break;
		case 0xc7d0: folderexit(context); break;
		case 0xc7d4: showleftpage(context); break;
		case 0xc7d8: showrightpage(context); break;
		case 0xc7dc: entersymbol(context); break;
		case 0xc7e0: quitsymbol(context); break;
		case 0xc7e4: settopleft(context); break;
		case 0xc7e8: settopright(context); break;
		case 0xc7ec: setbotleft(context); break;
		case 0xc7f0: setbotright(context); break;
		case 0xc7f4: dumpsymbol(context); break;
		case 0xc7f8: showsymbol(context); break;
		case 0xc7fc: nextsymbol(context); break;
		case 0xc800: updatesymboltop(context); break;
		case 0xc804: updatesymbolbot(context); break;
		case 0xc808: dumpsymbox(context); break;
		case 0xc80c: usediary(context); break;
		case 0xc810: showdiary(context); break;
		case 0xc814: showdiarykeys(context); break;
		case 0xc818: dumpdiarykeys(context); break;
		case 0xc81c: diarykeyp(context); break;
		case 0xc820: diarykeyn(context); break;
		case 0xc824: showdiarypage(context); break;
		case 0xc828: findtext1(context); break;
		case 0xc82c: zoomonoff(context); break;
		case 0xc830: saveload(context); break;
		case 0xc834: dosaveload(context); break;
		case 0xc838: getbackfromops(context); break;
		case 0xc83c: showmainops(context); break;
		case 0xc840: showdiscops(context); break;
		case 0xc844: loadsavebox(context); break;
		case 0xc848: loadgame(context); break;
		case 0xc84c: getbacktoops(context); break;
		case 0xc850: discops(context); break;
		case 0xc854: savegame(context); break;
		case 0xc858: actualsave(context); break;
		case 0xc85c: actualload(context); break;
		case 0xc860: selectslot2(context); break;
		case 0xc864: checkinput(context); break;
		case 0xc868: getnamepos(context); break;
		case 0xc86c: showopbox(context); break;
		case 0xc870: showloadops(context); break;
		case 0xc874: showsaveops(context); break;
		case 0xc878: selectslot(context); break;
		case 0xc87c: showslots(context); break;
		case 0xc880: shownames(context); break;
		case 0xc884: dosreturn(context); break;
		case 0xc888: error(context); break;
		case 0xc88c: namestoold(context); break;
		case 0xc890: oldtonames(context); break;
		case 0xc894: saveposition(context); break;
		case 0xc898: loadposition(context); break;
		case 0xc89c: loadseg(context); break;
		case 0xc8a0: makeheader(context); break;
		case 0xc8a4: storeit(context); break;
		case 0xc8a8: saveseg(context); break;
		case 0xc8ac: findlen(context); break;
		case 0xc8b0: scanfornames(context); break;
		case 0xc8b4: decide(context); break;
		case 0xc8b8: showdecisions(context); break;
		case 0xc8bc: newgame(context); break;
		case 0xc8c0: loadold(context); break;
		case 0xc8c4: loadspeech(context); break;
		case 0xc8c8: createname(context); break;
		case 0xc8cc: loadsample(context); break;
		case 0xc8d0: loadsecondsample(context); break;
		case 0xc8d4: soundstartup(context); break;
		case 0xc8d8: trysoundalloc(context); break;
		case 0xc8dc: setsoundoff(context); break;
		case 0xc8e0: checksoundint(context); break;
		case 0xc8e4: enablesoundint(context); break;
		case 0xc8e8: disablesoundint(context); break;
		case 0xc8ec: interupttest(context); break;
		case 0xc8f0: soundend(context); break;
		case 0xc8f4: out22c(context); break;
		case 0xc8f8: playchannel0(context); break;
		case 0xc8fc: playchannel1(context); break;
		case 0xc900: makenextblock(context); break;
		case 0xc904: volumeadjust(context); break;
		case 0xc908: loopchannel0(context); break;
		case 0xc90c: cancelch0(context); break;
		case 0xc910: cancelch1(context); break;
		case 0xc914: channel0only(context); break;
		case 0xc918: channel1only(context); break;
		case 0xc91c: channel0tran(context); break;
		case 0xc920: bothchannels(context); break;
		case 0xc924: saveems(context); break;
		case 0xc928: restoreems(context); break;
		case 0xc92c: domix(context); break;
		case 0xc930: dmaend(context); break;
		case 0xc934: startdmablock(context); break;
		case 0xc938: setuppit(context); break;
		case 0xc93c: getridofpit(context); break;
		case 0xc940: pitinterupt(context); break;
		case 0xc944: dreamweb(context); break;
		case 0xc948: entrytexts(context); break;
		case 0xc94c: entryanims(context); break;
		case 0xc950: initialinv(context); break;
		case 0xc954: pickupob(context); break;
		case 0xc958: setupemm(context); break;
		case 0xc95c: removeemm(context); break;
		case 0xc960: checkforemm(context); break;
		case 0xc964: checkbasemem(context); break;
		case 0xc968: allocatebuffers(context); break;
		case 0xc96c: clearbuffers(context); break;
		case 0xc970: clearchanges(context); break;
		case 0xc974: clearbeforeload(context); break;
		case 0xc978: clearreels(context); break;
		case 0xc97c: clearrest(context); break;
		case 0xc980: deallocatemem(context); break;
		case 0xc984: allocatemem(context); break;
		case 0xc988: seecommandtail(context); break;
		case 0xc98c: parseblaster(context); break;
		case 0xc990: startup(context); break;
		case 0xc994: startup1(context); break;
		case 0xc998: screenupdate(context); break;
		case 0xc99c: watchreel(context); break;
		case 0xc9a0: checkforshake(context); break;
		case 0xc9a4: watchcount(context); break;
		case 0xc9a8: showtime(context); break;
		case 0xc9ac: dumpwatch(context); break;
		case 0xc9b0: showbyte(context); break;
		case 0xc9b4: onedigit(context); break;
		case 0xc9b8: twodigitnum(context); break;
		case 0xc9bc: showword(context); break;
		case 0xc9c0: convnum(context); break;
		case 0xc9c4: mainscreen(context); break;
		case 0xc9c8: madmanrun(context); break;
		case 0xc9cc: checkcoords(context); break;
		case 0xc9d0: identifyob(context); break;
		case 0xc9d4: checkifperson(context); break;
		case 0xc9d8: checkifset(context); break;
		case 0xc9dc: checkifex(context); break;
		case 0xc9e0: checkiffree(context); break;
		case 0xc9e4: isitdescribed(context); break;
		case 0xc9e8: findpathofpoint(context); break;
		case 0xc9ec: findfirstpath(context); break;
		case 0xc9f0: turnpathon(context); break;
		case 0xc9f4: turnpathoff(context); break;
		case 0xc9f8: turnanypathon(context); break;
		case 0xc9fc: turnanypathoff(context); break;
		case 0xca00: checkifpathison(context); break;
		case 0xca04: afternewroom(context); break;
		case 0xca08: atmospheres(context); break;
		case 0xca0c: walkintoroom(context); break;
		case 0xca10: afterintroroom(context); break;
		case 0xca14: obname(context); break;
		case 0xca18: finishedwalking(context); break;
		case 0xca1c: examineobtext(context); break;
		case 0xca20: commandwithob(context); break;
		case 0xca24: commandonly(context); break;
		case 0xca28: printmessage(context); break;
		case 0xca2c: printmessage2(context); break;
		case 0xca30: blocknametext(context); break;
		case 0xca34: personnametext(context); break;
		case 0xca38: walktotext(context); break;
		case 0xca3c: getflagunderp(context); break;
		case 0xca40: setwalk(context); break;
		case 0xca44: autosetwalk(context); break;
		case 0xca48: checkdest(context); break;
		case 0xca4c: bresenhams(context); break;
		case 0xca50: workoutframes(context); break;
		case 0xca54: getroomspaths(context); break;
		case 0xca58: copyname(context); break;
		case 0xca5c: findobname(context); break;
		case 0xca60: showicon(context); break;
		case 0xca64: middlepanel(context); break;
		case 0xca68: showman(context); break;
		case 0xca6c: showpanel(context); break;
		case 0xca70: roomname(context); break;
		case 0xca74: usecharset1(context); break;
		case 0xca78: usetempcharset(context); break;
		case 0xca7c: showexit(context); break;
		case 0xca80: panelicons1(context); break;
		case 0xca84: showwatch(context); break;
		case 0xca88: gettime(context); break;
		case 0xca8c: zoomicon(context); break;
		case 0xca90: showblink(context); break;
		case 0xca94: dumpblink(context); break;
		case 0xca98: worktoscreenm(context); break;
		case 0xca9c: blank(context); break;
		case 0xcaa0: allpointer(context); break;
		case 0xcaa4: hangonp(context); break;
		case 0xcaa8: hangonw(context); break;
		case 0xcaac: hangoncurs(context); break;
		case 0xcab0: getunderzoom(context); break;
		case 0xcab4: dumpzoom(context); break;
		case 0xcab8: putunderzoom(context); break;
		case 0xcabc: crosshair(context); break;
		case 0xcac0: showpointer(context); break;
		case 0xcac4: delpointer(context); break;
		case 0xcac8: dumppointer(context); break;
		case 0xcacc: undertextline(context); break;
		case 0xcad0: deltextline(context); break;
		case 0xcad4: dumptextline(context); break;
		case 0xcad8: animpointer(context); break;
		case 0xcadc: setmouse(context); break;
		case 0xcae0: readmouse(context); break;
		case 0xcae4: mousecall(context); break;
		case 0xcae8: readmouse1(context); break;
		case 0xcaec: readmouse2(context); break;
		case 0xcaf0: readmouse3(context); break;
		case 0xcaf4: readmouse4(context); break;
		case 0xcaf8: readkey(context); break;
		case 0xcafc: convertkey(context); break;
		case 0xcb00: randomnum1(context); break;
		case 0xcb04: randomnum2(context); break;
		case 0xcb0c: hangon(context); break;
		case 0xcb10: loadtraveltext(context); break;
		case 0xcb14: loadintotemp(context); break;
		case 0xcb18: loadintotemp2(context); break;
		case 0xcb1c: loadintotemp3(context); break;
		case 0xcb20: loadtempcharset(context); break;
		case 0xcb24: standardload(context); break;
		case 0xcb28: loadtemptext(context); break;
		case 0xcb2c: loadroom(context); break;
		case 0xcb30: loadroomssample(context); break;
		case 0xcb34: getridofreels(context); break;
		case 0xcb38: getridofall(context); break;
		case 0xcb3c: restorereels(context); break;
		case 0xcb40: restoreall(context); break;
		case 0xcb44: sortoutmap(context); break;
		case 0xcb48: startloading(context); break;
		case 0xcb4c: disablepath(context); break;
		case 0xcb50: findxyfrompath(context); break;
		case 0xcb54: findroominloc(context); break;
		case 0xcb58: getroomdata(context); break;
		case 0xcb5c: readheader(context); break;
		case 0xcb60: dontloadseg(context); break;
		case 0xcb64: allocateload(context); break;
		case 0xcb68: fillspace(context); break;
		case 0xcb6c: getridoftemp(context); break;
		case 0xcb70: getridoftemptext(context); break;
		case 0xcb74: getridoftemp2(context); break;
		case 0xcb78: getridoftemp3(context); break;
		case 0xcb7c: getridoftempcharset(context); break;
		case 0xcb80: getridoftempsp(context); break;
		case 0xcb84: readsetdata(context); break;
		case 0xcb88: createfile(context); break;
		case 0xcb8c: openfile(context); break;
		case 0xcb90: openfilenocheck(context); break;
		case 0xcb94: openforsave(context); break;
		case 0xcb98: closefile(context); break;
		case 0xcb9c: readfromfile(context); break;
		case 0xcba0: setkeyboardint(context); break;
		case 0xcba4: resetkeyboard(context); break;
		case 0xcba8: keyboardread(context); break;
		case 0xcbac: walkandexamine(context); break;
		case 0xcbb0: doload(context); break;
		case 0xcbb4: generalerror(context); break;

	}
}

} /*namespace*/
