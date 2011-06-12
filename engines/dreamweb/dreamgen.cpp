#include "dreamgen.h"

namespace dreamgen {

void checkbasemem(Context &context);
void allocatebuffers(Context &context);
void clearbuffers(Context &context);
void clearpalette(Context &context);
void readsetdata(Context &context);
void scanfornames(Context &context);
void loadpalfromiff(Context &context);
void titles(Context &context);
void credits(Context &context);
void decide(Context &context);
void clearchanges(Context &context);
void loadroom(Context &context);
void clearsprites(Context &context);
void initman(Context &context);
void entrytexts(Context &context);
void entryanims(Context &context);
void initialinv(Context &context);
void startup1(Context &context);
void clearbeforeload(Context &context);
void startup(Context &context);
void worktoscreenm(Context &context);
void screenupdate(Context &context);
void showgun(Context &context);
void fadescreendown(Context &context);
void hangon(Context &context);
void fadescreendowns(Context &context);
void endgame(Context &context);
void standardload(Context &context);
void getroomspaths(Context &context);
void readheader(Context &context);
void allocateload(Context &context);
void fillspace(Context &context);
void sortoutmap(Context &context);
void findroominloc(Context &context);
void deletetaken(Context &context);
void setallchanges(Context &context);
void autoappear(Context &context);
void getroomdata(Context &context);
void autosetwalk(Context &context);
void findxyfrompath(Context &context);
void twodigitnum(Context &context);
void startloading(Context &context);
void loadroomssample(Context &context);
void switchryanon(Context &context);
void drawflags(Context &context);
void getdimension(Context &context);
void getflagunderp(Context &context);
void dumpblink(Context &context);
void showblink(Context &context);
void showframe(Context &context);
void printcurs(Context &context);
void delcurs(Context &context);
void delpointer(Context &context);
void readmouse(Context &context);
void animpointer(Context &context);
void showpointer(Context &context);
void dumppointer(Context &context);
void commandonly(Context &context);
void showtime(Context &context);
void showwatch(Context &context);
void printmessage(Context &context);
void printdirect(Context &context);
void usecharset1(Context &context);
void showpanel(Context &context);
void showman(Context &context);
void roomname(Context &context);
void panelicons1(Context &context);
void zoomicon(Context &context);
void middlepanel(Context &context);
void findobname(Context &context);
void workoutframes(Context &context);
void checkdest(Context &context);
void bresenhams(Context &context);
void facerightway(Context &context);
void checkone(Context &context);
void commandwithob(Context &context);
void findnextcolon(Context &context);
void deltextline(Context &context);
void copyname(Context &context);
void finishedwalking(Context &context);
void examineob(Context &context);
void talk(Context &context);
void setwalk(Context &context);
void examineobtext(Context &context);
void blocknametext(Context &context);
void personnametext(Context &context);
void walktotext(Context &context);
void clearwork(Context &context);
void drawfloor(Context &context);
void reelsonscreen(Context &context);
void spriteupdate(Context &context);
void printsprites(Context &context);
void playchannel0(Context &context);
void cancelch0(Context &context);
void createpanel(Context &context);
void findpathofpoint(Context &context);
void showicon(Context &context);
void undertextline(Context &context);
void mainscreen(Context &context);
void getunderzoom(Context &context);
void zoom(Context &context);
void walkintoroom(Context &context);
void reminders(Context &context);
void atmospheres(Context &context);
void findormake(Context &context);
void obname(Context &context);
void pixelcheckset(Context &context);
void isitdescribed(Context &context);
void getreelstart(Context &context);
void getreelframeax(Context &context);
void blank(Context &context);
void findfirstpath(Context &context);
void checkifex(Context &context);
void checkiffree(Context &context);
void checkifperson(Context &context);
void checkifset(Context &context);
void identifyob(Context &context);
void checkcoords(Context &context);
void walkandexamine(Context &context);
void convnum(Context &context);
void onedigit(Context &context);
void plotreel(Context &context);
void checkforshake(Context &context);
void newplace(Context &context);
void readmouse1(Context &context);
void dumptextline(Context &context);
void autolook(Context &context);
void watchcount(Context &context);
void readmouse2(Context &context);
void dumpzoom(Context &context);
void deleverything(Context &context);
void afternewroom(Context &context);
void readmouse3(Context &context);
void dumpmap(Context &context);
void dumptimedtext(Context &context);
void readmouse4(Context &context);
void dumpwatch(Context &context);
void fadescreenup(Context &context);
void clearreels(Context &context);
void clearrest(Context &context);
void trysoundalloc(Context &context);
void allocatework(Context &context);
void checkforemm(Context &context);
void getanyad(Context &context);
void transfertoex(Context &context);
void pickupob(Context &context);
void switchryanoff(Context &context);
void resetlocation(Context &context);
void removefreeobject(Context &context);
void checkifpathison(Context &context);
void turnpathon(Context &context);
void removesetobject(Context &context);
void placesetobject(Context &context);
void turnanypathon(Context &context);
void setuptimeduse(Context &context);
void volumeadjust(Context &context);
void loopchannel0(Context &context);
void cancelch1(Context &context);
void createname(Context &context);
void doload(Context &context);
void showdecisions(Context &context);
void showopbox(Context &context);
void showloadops(Context &context);
void showslots(Context &context);
void shownames(Context &context);
void namestoold(Context &context);
void getridoftemp(Context &context);
void initrain(Context &context);
void createpanel2(Context &context);
void loadsavebox(Context &context);
void storeit(Context &context);
void makeheader(Context &context);
void oldtonames(Context &context);
void showsaveops(Context &context);
void readkey(Context &context);
void getnamepos(Context &context);
void selectslot(Context &context);
void loadposition(Context &context);
void saveposition(Context &context);
void restoreall(Context &context);
void redrawmainscrn(Context &context);
void checkinput(Context &context);
void showdiscops(Context &context);
void loadintotemp(Context &context);
void getback1(Context &context);
void getridofall(Context &context);
void showmainops(Context &context);
void dosaveload(Context &context);
void findtext1(Context &context);
void usetempcharset(Context &context);
void playchannel1(Context &context);
void getlocation(Context &context);
void setlocation(Context &context);
void hangonp(Context &context);
void showdiary(Context &context);
void showdiarypage(Context &context);
void getridofreels(Context &context);
void loadtemptext(Context &context);
void loadtempcharset(Context &context);
void showdiarykeys(Context &context);
void dumpdiarykeys(Context &context);
void getridoftemptext(Context &context);
void getridoftempcharset(Context &context);
void restorereels(Context &context);
void nextsymbol(Context &context);
void showsymbol(Context &context);
void updatesymboltop(Context &context);
void updatesymbolbot(Context &context);
void dumpsymbol(Context &context);
void turnanypathoff(Context &context);
void folderexit(Context &context);
void showleftpage(Context &context);
void showrightpage(Context &context);
void loadintotemp2(Context &context);
void loadintotemp3(Context &context);
void showfolder(Context &context);
void folderhints(Context &context);
void loadfolder(Context &context);
void getridoftemp2(Context &context);
void getridoftemp3(Context &context);
void loadmenu(Context &context);
void getundermenu(Context &context);
void putundermenu(Context &context);
void showmenu(Context &context);
void dumpmenu(Context &context);
void singlekey(Context &context);
void buttonpress(Context &context);
void loadkeypad(Context &context);
void showouterpad(Context &context);
void showkeypad(Context &context);
void dumpkeypad(Context &context);
void addtopresslist(Context &context);
void isitright(Context &context);
void checkinside(Context &context);
void compare(Context &context);
void showfirstuse(Context &context);
void putbackobstuff(Context &context);
void withwhat(Context &context);
void showpuztext(Context &context);
void placefreeobject(Context &context);
void issetobonmap(Context &context);
void showseconduse(Context &context);
void removeobfrominv(Context &context);
void turnpathoff(Context &context);
void getundertimed(Context &context);
void putundertimed(Context &context);
void getsetad(Context &context);
void getfreead(Context &context);
void dochange(Context &context);
void findpuztext(Context &context);
void showexit(Context &context);
void obicons(Context &context);
void obpicture(Context &context);
void describeob(Context &context);
void getanyaddir(Context &context);
void findinvpos(Context &context);
void useroutine(Context &context);
void printmessage2(Context &context);
void fillryan(Context &context);
void findsetobject(Context &context);
void getobtextstart(Context &context);
void usetext(Context &context);
void selectlocation(Context &context);
void hangonw(Context &context);
void moneypoke(Context &context);
void nextcolon(Context &context);
void getexad(Context &context);
void entercode(Context &context);
void findexobject(Context &context);
void makeworn(Context &context);
void isryanholding(Context &context);
void nothelderror(Context &context);
void triggermessage(Context &context);
void monprint(Context &context);
void printchar(Context &context);
void showcurrentfile(Context &context);
void printlogo(Context &context);
void randomaccess(Context &context);
void locklighton(Context &context);
void locklightoff(Context &context);
void makecaps(Context &context);
void monmessage(Context &context);
void scrollmonitor(Context &context);
void searchforstring(Context &context);
void getkeyandlogo(Context &context);
void monitorlogo(Context &context);
void parser(Context &context);
void neterror(Context &context);
void processtrigger(Context &context);
void input(Context &context);
void dirfile(Context &context);
void searchforfiles(Context &context);
void dircom(Context &context);
void signon(Context &context);
void read(Context &context);
void showkeys(Context &context);
void delchar(Context &context);
void randomnum1(Context &context);
void accesslighton(Context &context);
void accesslightoff(Context &context);
void powerlighton(Context &context);
void powerlightoff(Context &context);
void lookininterface(Context &context);
void loadpersonal(Context &context);
void loadnews(Context &context);
void loadcart(Context &context);
void printoutermon(Context &context);
void initialmoncols(Context &context);
void turnonpower(Context &context);
void fadeupyellows(Context &context);
void fadeupmonfirst(Context &context);
void hangoncurs(Context &context);
void execcommand(Context &context);
void purgealocation(Context &context);
void getdestinfo(Context &context);
void showarrows(Context &context);
void locationpic(Context &context);
void getundercentre(Context &context);
void putundercentre(Context &context);
void readcitypic(Context &context);
void showcity(Context &context);
void readdesticon(Context &context);
void loadtraveltext(Context &context);
void convicons(Context &context);
void starttalk(Context &context);
void hangonpq(Context &context);
void redes(Context &context);
void dosometalk(Context &context);
void getpersontext(Context &context);
void getpersframe(Context &context);
void findsource(Context &context);
void printslow(Context &context);
void dolook(Context &context);
void getxad(Context &context);
void getyad(Context &context);
void getmapad(Context &context);
void calcfrframe(Context &context);
void finalframe(Context &context);
void makesprite(Context &context);
void makebackob(Context &context);
void addalong(Context &context);
void addlength(Context &context);
void eraseoldobs(Context &context);
void calcmapad(Context &context);
void doblocks(Context &context);
void showallobs(Context &context);
void showallfree(Context &context);
void showallex(Context &context);
void paneltomap(Context &context);
void deleteexframe(Context &context);
void deleteextext(Context &context);
void deleteexobject(Context &context);
void purgeanitem(Context &context);
void getexpos(Context &context);
void transfermap(Context &context);
void transferinv(Context &context);
void transfertext(Context &context);
void transfercontoex(Context &context);
void emergencypurge(Context &context);
void pickupconts(Context &context);
void findopenpos(Context &context);
void reexfromopen(Context &context);
void geteitherad(Context &context);
void fillopen(Context &context);
void useopened(Context &context);
void getopenedsize(Context &context);
void errormessage3(Context &context);
void errormessage2(Context &context);
void examicon(Context &context);
void outofopen(Context &context);
void swapwithopen(Context &context);
void isitworn(Context &context);
void wornerror(Context &context);
void errormessage1(Context &context);
void checkobjectsize(Context &context);
void openinv(Context &context);
void openob(Context &context);
void droperror(Context &context);
void cantdrop(Context &context);
void reexfrominv(Context &context);
void intoinv(Context &context);
void outofinv(Context &context);
void swapwithinv(Context &context);
void makemainscreen(Context &context);
void searchforsame(Context &context);
void lookatcard(Context &context);
void obsthatdothings(Context &context);
void additionaltext(Context &context);
void findallopen(Context &context);
void obtoinv(Context &context);
void findallryan(Context &context);
void showryanpage(Context &context);
void kernchars(Context &context);
void getnextword(Context &context);
void getnumber(Context &context);
void lockmon(Context &context);
void printboth(Context &context);
void waitframes(Context &context);
void hangone(Context &context);
void allpalette(Context &context);
void fadescreenups(Context &context);
void afterintroroom(Context &context);
void usetimedtext(Context &context);
void loadintroroom(Context &context);
void runintroseq(Context &context);
void realcredits(Context &context);
void runendseq(Context &context);
void showmonk(Context &context);
void monkspeaking(Context &context);
void gettingshot(Context &context);
void biblequote(Context &context);
void intro(Context &context);
void paltostartpal(Context &context);
void paltoendpal(Context &context);
void dumpcurrent(Context &context);
void rollem(Context &context);
void greyscalesum(Context &context);
void endpaltostart(Context &context);
void clearendpal(Context &context);
void rollendcredits2(Context &context);
void clearstartpal(Context &context);
void fadecalculation(Context &context);
void frameoutfx(Context &context);
void frameoutbh(Context &context);
void frameoutv(Context &context);
void putunderzoom(Context &context);
void crosshair(Context &context);
void maptopanel(Context &context);
void movemap(Context &context);
void dealwithspecial(Context &context);
void showreelframe(Context &context);
void soundonreels(Context &context);
void reconstruct(Context &context);
void updatepeople(Context &context);
void watchreel(Context &context);
void showrain(Context &context);
void dodoor(Context &context);
void liftnoise(Context &context);
void widedoor(Context &context);
void random(Context &context);
void lockeddoorway(Context &context);
void liftsprite(Context &context);
void doorway(Context &context);
void constant(Context &context);
void steady(Context &context);
void getblockofpixel(Context &context);
void splitintolines(Context &context);
void adjustleft(Context &context);
void adjustright(Context &context);
void adjustdown(Context &context);
void adjustup(Context &context);
void aboutturn(Context &context);
void checkforexit(Context &context);
void walking(Context &context);
void printasprite(Context &context);
void showgamereel(Context &context);
void checkspeed(Context &context);
void addtopeoplelist(Context &context);
void setuptimedtemp(Context &context);
void madmantext(Context &context);
void madmode(Context &context);
void priesttext(Context &context);
void fadescreenuphalf(Context &context);
void textforend(Context &context);
void fadescreendownhalf(Context &context);
void rollendcredits(Context &context);
void textformonk(Context &context);
void monks2text(Context &context);
void intro2text(Context &context);
void intro3text(Context &context);
void intro1text(Context &context);

void alleybarksound(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.es.word(context.bx+3);
	context._dec(context.ax);
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
	context.es.word(context.bx+3) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void intromusic(Context & context) {
	uint stack_depth = context.stack.size();
	{assert(stack_depth == context.stack.size()); return; }
}

void foghornsound(Context & context) {
	uint stack_depth = context.stack.size();
	randomnumber(context);
	context._cmp(context.al, 198);
	if (!context.flags.z()) goto nofog;
	context.al = 13;
	playchannel1(context);
nofog:
	{assert(stack_depth == context.stack.size()); return; }
}

void receptionist(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto gotrecep;
	context._cmp(context.data.byte(kCardpassflag), 1);
	if (!context.flags.z()) goto notsetcard;
	context._inc(context.data.byte(kCardpassflag));
	context.es.byte(context.bx+7) = 1;
	context.es.word(context.bx+3) = 64;
notsetcard:
	context._cmp(context.es.word(context.bx+3), 58);
	if (!context.flags.z()) goto notdes1;
	randomnumber(context);
	context._cmp(context.al, 30);
	if (context.flags.c()) goto notdes2;
	context.es.word(context.bx+3) = 55;
	goto gotrecep;
notdes1:
	context._cmp(context.es.word(context.bx+3), 60);
	if (!context.flags.z()) goto notdes2;
	randomnumber(context);
	context._cmp(context.al, 240);
	if (context.flags.c()) goto gotrecep;
	context.es.word(context.bx+3) = 53;
	goto gotrecep;
notdes2:
	context._cmp(context.es.word(context.bx+3), 88);
	if (!context.flags.z()) goto notendcard;
	context.es.word(context.bx+3) = 53;
	goto gotrecep;
notendcard:
	context._inc(context.es.word(context.bx+3));
gotrecep:
	showgamereel(context);
	addtopeoplelist(context);
	context.al = context.es.byte(context.bx+7);
	context._and(context.al, 128);
	if (context.flags.z()) goto nottalkedrecep;
	context.data.byte(kTalkedtorecep) = 1;
nottalkedrecep:
	{assert(stack_depth == context.stack.size()); return; }
}

void smokebloke(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kRockstardead), 0);
	if (!context.flags.z()) goto notspokento;
	context.al = context.es.byte(context.bx+7);
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
	context._cmp(context.es.word(context.bx+3), 100);
	if (!context.flags.z()) goto notsmokeb1;
	randomnumber(context);
	context._cmp(context.al, 30);
	if (context.flags.c()) goto notsmokeb2;
	context.es.word(context.bx+3) = 96;
	goto gotsmokeb;
notsmokeb1:
	context._cmp(context.es.word(context.bx+3), 117);
	if (!context.flags.z()) goto notsmokeb2;
	context.es.word(context.bx+3) = 96;
	goto gotsmokeb;
notsmokeb2:
	context._inc(context.es.word(context.bx+3));
gotsmokeb:
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void attendant(Context & context) {
	uint stack_depth = context.stack.size();
	showgamereel(context);
	addtopeoplelist(context);
	context.al = context.es.byte(context.bx+7);
	context._and(context.al, 128);
	if (context.flags.z()) goto nottalked;
	context.data.byte(kTalkedtoattendant) = 1;
nottalked:
	{assert(stack_depth == context.stack.size()); return; }
}

void manasleep(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.es.byte(context.bx+7);
	context._and(context.al, 127);
	context.es.byte(context.bx+7) = context.al;
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void eden(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kGeneraldead), 0);
	if (!context.flags.z()) goto notinbed;
	showgamereel(context);
	addtopeoplelist(context);
notinbed:
	{assert(stack_depth == context.stack.size()); return; }
}

void edeninbath(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kGeneraldead), 0);
	if (context.flags.z()) goto notinbed;
	context._cmp(context.data.byte(kSartaindead), 0);
	if (!context.flags.z()) goto notinbath;
	showgamereel(context);
	addtopeoplelist(context);
notinbath:
	{assert(stack_depth == context.stack.size()); return; }
/*continuing to unbounded code: notinbed from eden:5-6*/
notinbed:
	{assert(stack_depth == context.stack.size()); return; }
	{assert(stack_depth == context.stack.size()); return; }
}

void malefan(Context & context) {
	uint stack_depth = context.stack.size();
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void femalefan(Context & context) {
	uint stack_depth = context.stack.size();
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void louis(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kRockstardead), 0);
	if (!context.flags.z()) goto notlouis1;
	showgamereel(context);
	addtopeoplelist(context);
notlouis1:
	{assert(stack_depth == context.stack.size()); return; }
}

void louischair(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kRockstardead), 0);
	if (context.flags.z()) goto notlouis2;
	checkspeed(context);
	if (!context.flags.z()) goto notlouisanim;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 191);
	if (context.flags.z()) goto restartlouis;
	context._cmp(context.ax, 185);
	if (context.flags.z()) goto randomlouis;
	context.es.word(context.bx+3) = context.ax;
	goto notlouisanim;
randomlouis:
	context.es.word(context.bx+3) = context.ax;
	randomnumber(context);
	context._cmp(context.al, 245);
	if (!context.flags.c()) goto notlouisanim;
restartlouis:
	context.ax = 182;
	context.es.word(context.bx+3) = context.ax;
notlouisanim:
	showgamereel(context);
	addtopeoplelist(context);
notlouis2:
	{assert(stack_depth == context.stack.size()); return; }
}

void manasleep2(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.es.byte(context.bx+7);
	context._and(context.al, 127);
	context.es.byte(context.bx+7) = context.al;
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void mansatstill(Context & context) {
	uint stack_depth = context.stack.size();
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void tattooman(Context & context) {
	uint stack_depth = context.stack.size();
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void drinker(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto gotdrinker;
	context._inc(context.es.word(context.bx+3));
	context._cmp(context.es.word(context.bx+3), 115);
	if (!context.flags.z()) goto notdrinker1;
	context.es.word(context.bx+3) = 105;
	goto gotdrinker;
notdrinker1:
	context._cmp(context.es.word(context.bx+3), 106);
	if (!context.flags.z()) goto gotdrinker;
	randomnumber(context);
	context._cmp(context.al, 3);
	if (context.flags.c()) goto gotdrinker;
	context.es.word(context.bx+3) = 105;
gotdrinker:
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void bartender(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto gotsmoket;
	context._cmp(context.es.word(context.bx+3), 86);
	if (!context.flags.z()) goto notsmoket1;
	randomnumber(context);
	context._cmp(context.al, 18);
	if (context.flags.c()) goto notsmoket2;
	context.es.word(context.bx+3) = 81;
	goto gotsmoket;
notsmoket1:
	context._cmp(context.es.word(context.bx+3), 103);
	if (!context.flags.z()) goto notsmoket2;
	context.es.word(context.bx+3) = 81;
	goto gotsmoket;
notsmoket2:
	context._inc(context.es.word(context.bx+3));
gotsmoket:
	showgamereel(context);
	context._cmp(context.data.byte(kGunpassflag), 1);
	if (!context.flags.z()) goto notgotgun;
	context.es.byte(context.bx+7) = 9;
notgotgun:
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void othersmoker(Context & context) {
	uint stack_depth = context.stack.size();
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void barwoman(Context & context) {
	uint stack_depth = context.stack.size();
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void interviewer(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kReeltowatch), 68);
	if (!context.flags.z()) goto notgeneralstart;
	context._inc(context.es.word(context.bx+3));
notgeneralstart:
	context._cmp(context.es.word(context.bx+3), 250);
	if (context.flags.z()) goto talking;
	checkspeed(context);
	if (!context.flags.z()) goto talking;
	context._cmp(context.es.word(context.bx+3), 259);
	if (context.flags.z()) goto talking;
	context._inc(context.es.word(context.bx+3));
talking:
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void soldier1(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.es.word(context.bx+3), 0);
	if (context.flags.z()) goto soldierwait;
	context.data.word(kWatchingtime) = 10;
	context._cmp(context.es.word(context.bx+3), 30);
	if (!context.flags.z()) goto notaftersshot;
	context._inc(context.data.byte(kCombatcount));
	context._cmp(context.data.byte(kCombatcount), 40);
	if (!context.flags.z()) goto gotsoldframe;
	context.data.byte(kMandead) = 2;
	goto gotsoldframe;
notaftersshot:
	checkspeed(context);
	if (!context.flags.z()) goto gotsoldframe;
	context._inc(context.es.word(context.bx+3));
	goto gotsoldframe;
soldierwait:
	context._cmp(context.data.byte(kLastweapon), 1);
	if (!context.flags.z()) goto gotsoldframe;
	context.data.word(kWatchingtime) = 10;
	context._cmp(context.data.byte(kManspath), 2);
	if (!context.flags.z()) goto gotsoldframe;
	context._cmp(context.data.byte(kFacing), 4);
	if (!context.flags.z()) goto gotsoldframe;
	context._inc(context.es.word(context.bx+3));
	context.data.byte(kLastweapon) = -1;
	context.data.byte(kCombatcount) = 0;
gotsoldframe:
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void rockstar(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.es.word(context.bx+3);
	context._cmp(context.ax, 303);
	if (context.flags.z()) goto rockcombatend;
	context._cmp(context.ax, 118);
	if (context.flags.z()) goto rockcombatend;
	checkspeed(context);
	if (!context.flags.z()) goto rockspeed;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 118);
	if (!context.flags.z()) goto notbeforedead;
	context.data.byte(kMandead) = 2;
	goto gotrockframe;
notbeforedead:
	context._cmp(context.ax, 79);
	if (!context.flags.z()) goto gotrockframe;
	context._dec(context.ax);
	context._cmp(context.data.byte(kLastweapon), 1);
	if (!context.flags.z()) goto notgunonrock;
	context.data.byte(kLastweapon) = -1;
	context.ax = 123;
	goto gotrockframe;
notgunonrock:
	context._inc(context.data.byte(kCombatcount));
	context._cmp(context.data.byte(kCombatcount), 40);
	if (!context.flags.z()) goto gotrockframe;
	context.data.byte(kCombatcount) = 0;
	context.ax = 79;
gotrockframe:
	context.es.word(context.bx+3) = context.ax;
rockspeed:
	showgamereel(context);
	context._cmp(context.es.word(context.bx+3), 78);
	if (!context.flags.z()) goto notalkrock;
	addtopeoplelist(context);
	context.data.byte(kPointermode) = 2;
	context.data.word(kWatchingtime) = 0;
	{assert(stack_depth == context.stack.size()); return; }
notalkrock:
	context.data.word(kWatchingtime) = 2;
	context.data.byte(kPointermode) = 0;
	context.al = context.data.byte(kMapy);
	context.es.byte(context.bx+2) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
rockcombatend:
	context.data.byte(kNewlocation) = 45;
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void helicopter(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.es.word(context.bx+3);
	context._cmp(context.ax, 203);
	if (context.flags.z()) goto heliwon;
	checkspeed(context);
	if (!context.flags.z()) goto helispeed;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 53);
	if (!context.flags.z()) goto notbeforehdead;
	context._inc(context.data.byte(kCombatcount));
	context._cmp(context.data.byte(kCombatcount), 8);
	if (context.flags.c()) goto waitabit;
	context.data.byte(kMandead) = 2;
waitabit:
	context.ax = 49;
	goto gotheliframe;
notbeforehdead:
	context._cmp(context.ax, 9);
	if (!context.flags.z()) goto gotheliframe;
	context._dec(context.ax);
	context._cmp(context.data.byte(kLastweapon), 1);
	if (!context.flags.z()) goto notgunonheli;
	context.data.byte(kLastweapon) = -1;
	context.ax = 55;
	goto gotheliframe;
notgunonheli:
	context.ax = 5;
	context._inc(context.data.byte(kCombatcount));
	context._cmp(context.data.byte(kCombatcount), 20);
	if (!context.flags.z()) goto gotheliframe;
	context.data.byte(kCombatcount) = 0;
	context.ax = 9;
gotheliframe:
	context.es.word(context.bx+3) = context.ax;
helispeed:
	showgamereel(context);
	context.al = context.data.byte(kMapx);
	context.es.byte(context.bx+1) = context.al;
helicombatend:
	context.ax = context.es.word(context.bx+3);
	context._cmp(context.ax, 9);
	if (!context.flags.c()) goto notwaitingheli;
	context._cmp(context.data.byte(kCombatcount), 7);
	if (context.flags.c()) goto notwaitingheli;
	context.data.byte(kPointermode) = 2;
	context.data.word(kWatchingtime) = 0;
	{assert(stack_depth == context.stack.size()); return; }
notwaitingheli:
	context.data.byte(kPointermode) = 0;
	context.data.word(kWatchingtime) = 2;
	{assert(stack_depth == context.stack.size()); return; }
heliwon:
	context.data.byte(kPointermode) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void mugger(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.es.word(context.bx+3);
	context._cmp(context.ax, 138);
	if (context.flags.z()) goto endmugger1;
	context._cmp(context.ax, 176);
	if (context.flags.z()) goto endmugger2;
	context._cmp(context.ax, 2);
	if (!context.flags.z()) goto havesetwatch;
	context.data.word(kWatchingtime) = 175*2;
havesetwatch:
	checkspeed(context);
	if (!context.flags.z()) goto notmugger;
	context._inc(context.es.word(context.bx+3));
notmugger:
	showgamereel(context);
	context.al = context.data.byte(kMapx);
	context.es.byte(context.bx+1) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
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
	context.es.word(context.bx+3) = 140;
	context.data.byte(kManspath) = 2;
	context.data.byte(kFinaldest) = 2;
	findxyfrompath(context);
	context.data.byte(kResetmanxy) = 1;
	context.al = 'W';
	context.ah = 'E';
	context.cl = 'T';
	context.ch = 'A';
	findexobject(context);
	context.data.byte(kCommand) = context.al;
	context.data.byte(kObjecttype) = 4;
	removeobfrominv(context);
	context.al = 'W';
	context.ah = 'E';
	context.cl = 'T';
	context.ch = 'B';
	findexobject(context);
	context.data.byte(kCommand) = context.al;
	context.data.byte(kObjecttype) = 4;
	removeobfrominv(context);
	makemainscreen(context);
	context.al = 48;
	context.bl = 68-32;
	context.bh = 54+64;
	context.cx = 70;
	context.dx = 10;
	setuptimeduse(context);
	context.data.byte(kBeenmugged) = 1;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
endmugger2:
	{assert(stack_depth == context.stack.size()); return; }
}

void aide(Context & context) {
	uint stack_depth = context.stack.size();
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void businessman(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kPointermode) = 0;
	context.data.word(kWatchingtime) = 2;
	context.ax = context.es.word(context.bx+3);
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
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 48);
	if (!context.flags.z()) goto notbeforedeadb;
	context.data.byte(kMandead) = 2;
	goto gotbusframe;
notbeforedeadb:
	context._cmp(context.ax, 15);
	if (!context.flags.z()) goto buscombatwon;
	context._dec(context.ax);
	context._cmp(context.data.byte(kLastweapon), 3);
	if (!context.flags.z()) goto notshieldonbus;
	context.data.byte(kLastweapon) = -1;
	context.data.byte(kCombatcount) = 0;
	context.ax = 51;
	goto gotbusframe;
notshieldonbus:
	context._inc(context.data.byte(kCombatcount));
	context._cmp(context.data.byte(kCombatcount), 20);
	if (!context.flags.z()) goto gotbusframe;
	context.data.byte(kCombatcount) = 0;
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
	context.data.byte(kManspath) = 5;
	context.data.byte(kFinaldest) = 5;
	findxyfrompath(context);
	context.data.byte(kResetmanxy) = 1;
	context.es = context.pop();
	context.bx = context.pop();
	context.ax = 92;
	goto gotbusframe;
gotbusframe:
	context.es.word(context.bx+3) = context.ax;
busspeed:
	showgamereel(context);
	context.al = context.data.byte(kMapy);
	context.es.byte(context.bx+2) = context.al;
	context.ax = context.es.word(context.bx+3);
	context._cmp(context.ax, 14);
	if (!context.flags.z()) goto buscombatend;
	context.data.word(kWatchingtime) = 0;
	context.data.byte(kPointermode) = 2;
	{assert(stack_depth == context.stack.size()); return; }
buscombatend:
	{assert(stack_depth == context.stack.size()); return; }
buscombatwonend:
	context.data.byte(kPointermode) = 0;
	context.data.word(kWatchingtime) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void poolguard(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.es.word(context.bx+3);
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
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 122);
	if (!context.flags.z()) goto notendguard1;
	context._dec(context.ax);
	context._cmp(context.data.byte(kLastweapon), 2);
	if (!context.flags.z()) goto notaxeonpool;
	context.data.byte(kLastweapon) = -1;
	context.ax = 122;
	goto gotguardframe;
notaxeonpool:
	context._inc(context.data.byte(kCombatcount));
	context._cmp(context.data.byte(kCombatcount), 40);
	if (!context.flags.z()) goto gotguardframe;
	context.data.byte(kCombatcount) = 0;
	context.ax = 195;
	goto gotguardframe;
notendguard1:
	context._cmp(context.ax, 147);
	if (!context.flags.z()) goto gotguardframe;
	context._dec(context.ax);
	context._cmp(context.data.byte(kLastweapon), 1);
	if (!context.flags.z()) goto notgunonpool;
	context.data.byte(kLastweapon) = -1;
	context.ax = 147;
	goto gotguardframe;
notgunonpool:
	context._inc(context.data.byte(kCombatcount));
	context._cmp(context.data.byte(kCombatcount), 40);
	if (!context.flags.z()) goto gotguardframe;
	context.data.byte(kCombatcount) = 0;
	context.ax = 220;
gotguardframe:
	context.es.word(context.bx+3) = context.ax;
guardspeed:
	showgamereel(context);
	context.ax = context.es.word(context.bx+3);
	context._cmp(context.ax, 121);
	if (context.flags.z()) goto iswaitingpool;
	context._cmp(context.ax, 146);
	if (context.flags.z()) goto iswaitingpool;
	context.data.byte(kPointermode) = 0;
	context.data.word(kWatchingtime) = 2;
	{assert(stack_depth == context.stack.size()); return; }
iswaitingpool:
	context.data.byte(kPointermode) = 2;
	context.data.word(kWatchingtime) = 0;
	{assert(stack_depth == context.stack.size()); return; }
combatover1:
	context.data.word(kWatchingtime) = 0;
	context.data.byte(kPointermode) = 0;
	context.al = 0;
	turnpathon(context);
	context.al = 1;
	turnpathoff(context);
	{assert(stack_depth == context.stack.size()); return; }
combatover2:
	showgamereel(context);
	context.data.word(kWatchingtime) = 2;
	context.data.byte(kPointermode) = 0;
	context._inc(context.data.byte(kCombatcount));
	context._cmp(context.data.byte(kCombatcount), 100);
	if (context.flags.c()) goto doneover2;
	context.data.word(kWatchingtime) = 0;
	context.data.byte(kMandead) = 2;
doneover2:
	{assert(stack_depth == context.stack.size()); return; }
}

void security(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.es.word(context.bx+3), 32);
	if (context.flags.z()) goto securwait;
	context._cmp(context.es.word(context.bx+3), 69);
	if (!context.flags.z()) goto notaftersec;
	{assert(stack_depth == context.stack.size()); return; }
notaftersec:
	context.data.word(kWatchingtime) = 10;
	checkspeed(context);
	if (!context.flags.z()) goto gotsecurframe;
	context._inc(context.es.word(context.bx+3));
	goto gotsecurframe;
securwait:
	context._cmp(context.data.byte(kLastweapon), 1);
	if (!context.flags.z()) goto gotsecurframe;
	context.data.word(kWatchingtime) = 10;
	context._cmp(context.data.byte(kManspath), 9);
	if (!context.flags.z()) goto gotsecurframe;
	context._cmp(context.data.byte(kFacing), 0);
	if (!context.flags.z()) goto gotsecurframe;
	context.data.byte(kLastweapon) = -1;
	context._inc(context.es.word(context.bx+3));
gotsecurframe:
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void heavy(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.es.byte(context.bx+7);
	context._and(context.al, 127);
	context.es.byte(context.bx+7) = context.al;
	context._cmp(context.es.word(context.bx+3), 43);
	if (context.flags.z()) goto heavywait;
	context.data.word(kWatchingtime) = 10;
	context._cmp(context.es.word(context.bx+3), 70);
	if (!context.flags.z()) goto notafterhshot;
	context._inc(context.data.byte(kCombatcount));
	context._cmp(context.data.byte(kCombatcount), 80);
	if (!context.flags.z()) goto gotheavyframe;
	context.data.byte(kMandead) = 2;
	goto gotheavyframe;
notafterhshot:
	checkspeed(context);
	if (!context.flags.z()) goto gotheavyframe;
	context._inc(context.es.word(context.bx+3));
	goto gotheavyframe;
heavywait:
	context._cmp(context.data.byte(kLastweapon), 1);
	if (!context.flags.z()) goto gotheavyframe;
	context._cmp(context.data.byte(kManspath), 5);
	if (!context.flags.z()) goto gotheavyframe;
	context._cmp(context.data.byte(kFacing), 4);
	if (!context.flags.z()) goto gotheavyframe;
	context.data.byte(kLastweapon) = -1;
	context._inc(context.es.word(context.bx+3));
	context.data.byte(kCombatcount) = 0;
gotheavyframe:
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void bossman(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto notboss;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 4);
	if (context.flags.z()) goto firstdes;
	context._cmp(context.ax, 20);
	if (context.flags.z()) goto secdes;
	context._cmp(context.ax, 41);
	if (!context.flags.z()) goto gotallboss;
	context.ax = 0;
	context._inc(context.data.byte(kGunpassflag));
	context.es.byte(context.bx+7) = 10;
	goto gotallboss;
firstdes:
	context._cmp(context.data.byte(kGunpassflag), 1);
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
	context._cmp(context.data.byte(kGunpassflag), 1);
	if (context.flags.z()) goto gotallboss;
	context.ax = 0;
gotallboss:
	context.es.word(context.bx+3) = context.ax;
notboss:
	showgamereel(context);
	addtopeoplelist(context);
	context.al = context.es.byte(context.bx+7);
	context._and(context.al, 128);
	if (context.flags.z()) goto nottalkedboss;
	context.data.byte(kTalkedtoboss) = 1;
nottalkedboss:
	{assert(stack_depth == context.stack.size()); return; }
}

void gamer(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto gamerfin;
gameragain:
	randomnum1(context);
	context._and(context.al, 7);
	context._cmp(context.al, 5);
	if (!context.flags.c()) goto gameragain;
	context._add(context.al, 20);
	context._cmp(context.al, context.es.byte(context.bx+3));
	if (context.flags.z()) goto gameragain;
	context.ah = 0;
	context.es.word(context.bx+3) = context.ax;
gamerfin:
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void sparkydrip(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto cantdrip;
	context.al = 14;
	context.ah = 0;
	playchannel0(context);
cantdrip:
	{assert(stack_depth == context.stack.size()); return; }
}

void carparkdrip(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto cantdrip2;
	context.al = 14;
	playchannel1(context);
cantdrip2:
	{assert(stack_depth == context.stack.size()); return; }
}

void keeper(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kKeeperflag), 0);
	if (!context.flags.z()) goto notwaiting;
	context._cmp(context.data.word(kReeltowatch), 190);
	if (context.flags.c()) goto waiting;
	context._inc(context.data.byte(kKeeperflag));
	context.ah = context.es.byte(context.bx+7);
	context._and(context.ah, 127);
	context._cmp(context.ah, context.data.byte(kDreamnumber));
	if (context.flags.z()) goto notdiff;
	context.al = context.data.byte(kDreamnumber);
	context.es.byte(context.bx+7) = context.al;
notdiff:
	{assert(stack_depth == context.stack.size()); return; }
notwaiting:
	addtopeoplelist(context);
	showgamereel(context);
waiting:
	{assert(stack_depth == context.stack.size()); return; }
}

void candles1(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto candle1;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 44);
	if (!context.flags.z()) goto notendcandle1;
	context.ax = 39;
notendcandle1:
	context.es.word(context.bx+3) = context.ax;
candle1:
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void smallcandle(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto smallcandlef;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 37);
	if (!context.flags.z()) goto notendsmallcandle;
	context.ax = 25;
notendsmallcandle:
	context.es.word(context.bx+3) = context.ax;
smallcandlef:
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void intromagic1(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto introm1fin;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 145);
	if (!context.flags.z()) goto gotintrom1;
	context.ax = 121;
gotintrom1:
	context.es.word(context.bx+3) = context.ax;
	context._cmp(context.ax, 121);
	if (!context.flags.z()) goto introm1fin;
	context._inc(context.data.byte(kIntrocount));
	context.push(context.es);
	context.push(context.bx);
	intro1text(context);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.data.byte(kIntrocount), 8);
	if (!context.flags.z()) goto introm1fin;
	context._add(context.data.byte(kMapy), 10);
	context.data.byte(kNowinnewroom) = 1;
introm1fin:
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void candles(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto candlesfin;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 167);
	if (!context.flags.z()) goto gotcandles;
	context.ax = 162;
gotcandles:
	context.es.word(context.bx+3) = context.ax;
candlesfin:
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void candles2(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto candles2fin;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 238);
	if (!context.flags.z()) goto gotcandles2;
	context.ax = 233;
gotcandles2:
	context.es.word(context.bx+3) = context.ax;
candles2fin:
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void gates(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto gatesfin;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
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
	context.es.byte(context.bx+5) = 2;
slowgates:
	context._cmp(context.ax, 120);
	if (!context.flags.z()) goto gotgates;
	context.data.byte(kGetback) = 1;
	context.ax = 119;
gotgates:
	context.es.word(context.bx+3) = context.ax;
	context.push(context.es);
	context.push(context.bx);
	intro3text(context);
	context.bx = context.pop();
	context.es = context.pop();
gatesfin:
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void intromagic2(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto introm2fin;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 216);
	if (!context.flags.z()) goto gotintrom2;
	context.ax = 192;
gotintrom2:
	context.es.word(context.bx+3) = context.ax;
introm2fin:
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void intromagic3(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto introm3fin;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 218);
	if (!context.flags.z()) goto gotintrom3;
	context.data.byte(kGetback) = 1;
gotintrom3:
	context.es.word(context.bx+3) = context.ax;
introm3fin:
	showgamereel(context);
	context.al = context.data.byte(kMapx);
	context.es.byte(context.bx+1) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void intromonks1(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto intromonk1fin;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 80);
	if (!context.flags.z()) goto notendmonk1;
	context._add(context.data.byte(kMapy), 10);
	context.data.byte(kNowinnewroom) = 1;
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
notendmonk1:
	context._cmp(context.ax, 30);
	if (!context.flags.z()) goto gotintromonk1;
	context._sub(context.data.byte(kMapy), 10);
	context.data.byte(kNowinnewroom) = 1;
	context.ax = 51;
gotintromonk1:
	context.es.word(context.bx+3) = context.ax;
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
	context.es.byte(context.bx+6) = -20;
intromonk1fin:
	showgamereel(context);
	context.al = context.data.byte(kMapy);
	context.es.byte(context.bx+2) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void intromonks2(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto intromonk2fin;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 87);
	if (!context.flags.z()) goto nottalk1;
	context._inc(context.data.byte(kIntrocount));
	context.push(context.es);
	context.push(context.bx);
	monks2text(context);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.data.byte(kIntrocount), 19);
	if (!context.flags.z()) goto notlasttalk1;
	context.ax = 87;
	goto gotintromonk2;
notlasttalk1:
	context.ax = 74;
	goto gotintromonk2;
nottalk1:
	context._cmp(context.ax, 110);
	if (!context.flags.z()) goto notraisearm;
	context._inc(context.data.byte(kIntrocount));
	context.push(context.es);
	context.push(context.bx);
	monks2text(context);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.data.byte(kIntrocount), 35);
	if (!context.flags.z()) goto notlastraise;
	context.ax = 111;
	goto gotintromonk2;
notlastraise:
	context.ax = 98;
	goto gotintromonk2;
notraisearm:
	context._cmp(context.ax, 176);
	if (!context.flags.z()) goto notendmonk2;
	context.data.byte(kGetback) = 1;
	goto gotintromonk2;
notendmonk2:
	context._cmp(context.ax, 125);
	if (!context.flags.z()) goto gotintromonk2;
	context.ax = 140;
gotintromonk2:
	context.es.word(context.bx+3) = context.ax;
intromonk2fin:
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void handclap(Context & context) {
	uint stack_depth = context.stack.size();
	{assert(stack_depth == context.stack.size()); return; }
}

void monks2text(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kIntrocount), 1);
	if (!context.flags.z()) goto notmonk2text1;
	context.al = 8;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text1:
	context._cmp(context.data.byte(kIntrocount), 4);
	if (!context.flags.z()) goto notmonk2text2;
	context.al = 9;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text2:
	context._cmp(context.data.byte(kIntrocount), 7);
	if (!context.flags.z()) goto notmonk2text3;
	context.al = 10;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text3:
	context._cmp(context.data.byte(kIntrocount), 10);
	if (!context.flags.z()) goto notmonk2text4;
	context.al = 11;
	context.bl = 0;
	context.bh = 105;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text4:
	context._cmp(context.data.byte(kIntrocount), 13);
	if (!context.flags.z()) goto notmonk2text5;
	context.al = 12;
	context.bl = 0;
	context.bh = 120;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text5:
	context._cmp(context.data.byte(kIntrocount), 16);
	if (!context.flags.z()) goto notmonk2text6;
	context.al = 13;
	context.bl = 0;
	context.bh = 135;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text6:
	context._cmp(context.data.byte(kIntrocount), 19);
	if (!context.flags.z()) goto notmonk2text7;
	context.al = 14;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	context.dx = 1;
	context.ah = 82;
	{ setuptimedtemp(context); return; };
notmonk2text7:
	context._cmp(context.data.byte(kIntrocount), 22);
	if (!context.flags.z()) goto notmonk2text8;
	context.al = 15;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text8:
	context._cmp(context.data.byte(kIntrocount), 25);
	if (!context.flags.z()) goto notmonk2text9;
	context.al = 16;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text9:
	context._cmp(context.data.byte(kIntrocount), 28);
	if (!context.flags.z()) goto notmonk2text10;
	context.al = 17;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text10:
	context._cmp(context.data.byte(kIntrocount), 31);
	if (!context.flags.z()) goto notmonk2text11;
	context.al = 18;
	context.bl = 36;
	context.bh = 160;
	context.cx = 100;
	goto gotmonks2text;
notmonk2text11:
	{assert(stack_depth == context.stack.size()); return; }
gotmonks2text:
	context.dx = 1;
	context.cx = 120;
	context.ah = 82;
	setuptimedtemp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void intro1text(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kIntrocount), 2);
	if (!context.flags.z()) goto notintro1text1;
	context.al = 40;
	context.bl = 34;
	context.bh = 130;
	context.cx = 90;
	goto gotintro1text;
notintro1text1:
	context._cmp(context.data.byte(kIntrocount), 4);
	if (!context.flags.z()) goto notintro1text2;
	context.al = 41;
	context.bl = 34;
	context.bh = 130;
	context.cx = 90;
	goto gotintro1text;
notintro1text2:
	context._cmp(context.data.byte(kIntrocount), 6);
	if (!context.flags.z()) goto notintro1text3;
	context.al = 42;
	context.bl = 34;
	context.bh = 130;
	context.cx = 90;
	goto gotintro1text;
notintro1text3:
	{assert(stack_depth == context.stack.size()); return; }
gotintro1text:
	context.dx = 1;
	context.ah = 82;
oktalk2:
	setuptimedtemp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void intro2text(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
gotintro2text:
	context.dx = 1;
	context.ah = 82;
	setuptimedtemp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void intro3text(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
gotintro3text:
	context.dx = 1;
	context.ah = 82;
	setuptimedtemp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void monkandryan(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto notmonkryan;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 83);
	if (!context.flags.z()) goto gotmonkryan;
	context._inc(context.data.byte(kIntrocount));
	context.push(context.es);
	context.push(context.bx);
	textformonk(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.ax = 77;
	context._cmp(context.data.byte(kIntrocount), 57);
	if (!context.flags.z()) goto gotmonkryan;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
gotmonkryan:
	context.es.word(context.bx+3) = context.ax;
notmonkryan:
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void endgameseq(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto notendseq;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 51);
	if (!context.flags.z()) goto gotendseq;
	context._cmp(context.data.byte(kIntrocount), 140);
	if (context.flags.z()) goto gotendseq;
	context._inc(context.data.byte(kIntrocount));
	context.push(context.es);
	context.push(context.bx);
	textforend(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.ax = 50;
gotendseq:
	context.es.word(context.bx+3) = context.ax;
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
	context.data.byte(kVolumeto) = 7;
	context.data.byte(kVolumedirection) = 1;
	context.ax = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
notfadeend:
	context._cmp(context.ax, 340);
	if (!context.flags.z()) goto notendseq;
	context.data.byte(kGetback) = 1;
notendseq:
	showgamereel(context);
	context.al = context.data.byte(kMapy);
	context.es.byte(context.bx+2) = context.al;
	context.ax = context.es.word(context.bx+3);
	context._cmp(context.ax, 145);
	if (!context.flags.z()) goto notendcreds;
	context.es.word(context.bx+3) = 146;
	rollendcredits(context);
notendcreds:
	{assert(stack_depth == context.stack.size()); return; }
}

void rollendcredits(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 16;
	context.ah = 255;
	playchannel0(context);
	context.data.byte(kVolume) = 7;
	context.data.byte(kVolumeto) = 0;
	context.data.byte(kVolumedirection) = -1;
	context.cl = 160;
	context.ch = 160;
	context.di = 75;
	context.bx = 20;
	context.ds = context.data.word(kMapstore);
	context.si = 0;
	multiget(context);
	context.es = context.data.word(kTextfile1);
	context.si = 3*2;
	context.ax = context.es.word(context.si);
	context.si = context.ax;
	context._add(context.si, (66*2));
	context.cx = 254;
endcredits1:
	context.push(context.cx);
	context.bx = 10;
	context.cx = context.data.word(kLinespacing);
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
	context.ds = context.data.word(kMapstore);
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
	context._add(context.bx, context.data.word(kLinespacing));
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
	context._dec(context.bx);
	if (--context.cx) goto endcredits2;
	context.cx = context.pop();
looknext:
	context.al = context.es.byte(context.si);
	context._inc(context.si);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void priest(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.es.word(context.bx+3), 8);
	if (context.flags.z()) goto priestspoken;
	context.data.byte(kPointermode) = 0;
	context.data.word(kWatchingtime) = 2;
	checkspeed(context);
	if (!context.flags.z()) goto priestwait;
	context._inc(context.es.word(context.bx+3));
	context.push(context.es);
	context.push(context.bx);
	priesttext(context);
	context.bx = context.pop();
	context.es = context.pop();
priestwait:
	{assert(stack_depth == context.stack.size()); return; }
priestspoken:
	{assert(stack_depth == context.stack.size()); return; }
}

void madmanstelly(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
	context._cmp(context.ax, 307);
	if (!context.flags.z()) goto notendtelly;
	context.ax = 300;
notendtelly:
	context.es.word(context.bx+3) = context.ax;
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void madman(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.word(kWatchingtime) = 2;
	checkspeed(context);
	if (!context.flags.z()) goto nomadspeed;
	context.ax = context.es.word(context.bx+3);
	context._cmp(context.ax, 364);
	if (!context.flags.c()) goto ryansded;
	context._cmp(context.ax, 10);
	if (!context.flags.z()) goto notfirstmad;
	context.push(context.es);
	context.push(context.bx);
	context.push(context.ax);
	context.dx = 2247;
	loadtemptext(context);
	context.ax = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	context.data.byte(kCombatcount) = -1;
	context.data.byte(kSpeechcount) = 0;
notfirstmad:
	context._inc(context.ax);
	context._cmp(context.ax, 294);
	if (context.flags.z()) goto madmanspoken;
	context._cmp(context.ax, 66);
	if (!context.flags.z()) goto nomadspeak;
	context._inc(context.data.byte(kCombatcount));
	context.push(context.es);
	context.push(context.bx);
	madmantext(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.ax = 53;
	context._cmp(context.data.byte(kCombatcount), 62);
	if (context.flags.c()) goto nomadspeak;
	context._cmp(context.data.byte(kCombatcount), 68);
	if (context.flags.z()) goto killryan;
	context._cmp(context.data.byte(kLastweapon), 8);
	if (!context.flags.z()) goto nomadspeak;
	context.data.byte(kCombatcount) = 70;
	context.data.byte(kLastweapon) = -1;
	context.data.byte(kMadmanflag) = 1;
	context.ax = 67;
	goto nomadspeak;
killryan:
	context.ax = 310;
nomadspeak:
	context.es.word(context.bx+3) = context.ax;
nomadspeed:
	showgamereel(context);
	context.al = context.data.byte(kMapx);
	context.es.byte(context.bx+1) = context.al;
	madmode(context);
	{assert(stack_depth == context.stack.size()); return; }
madmanspoken:
	context._cmp(context.data.byte(kWongame), 1);
	if (context.flags.z()) goto alreadywon;
	context.data.byte(kWongame) = 1;
	context.push(context.es);
	context.push(context.bx);
	getridoftemptext(context);
	context.bx = context.pop();
	context.es = context.pop();
alreadywon:
	{assert(stack_depth == context.stack.size()); return; }
ryansded:
	context.data.byte(kMandead) = 2;
	showgamereel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void madmantext(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCombatcount), 61);
	if (!context.flags.c()) goto nomadtext;
	context.al = context.data.byte(kCombatcount);
	context._and(context.al, 3);
	if (!context.flags.z()) goto nomadtext;
	context.al = context.data.byte(kCombatcount);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void madmode(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.word(kWatchingtime) = 2;
	context.data.byte(kPointermode) = 0;
	context._cmp(context.data.byte(kCombatcount), 63);
	if (context.flags.c()) goto iswatchmad;
	context._cmp(context.data.byte(kCombatcount), 68);
	if (!context.flags.c()) goto iswatchmad;
	context.data.byte(kPointermode) = 2;
iswatchmad:
	{assert(stack_depth == context.stack.size()); return; }
}

void priesttext(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.es.word(context.bx+3), 2);
	if (context.flags.c()) goto nopriesttext;
	context._cmp(context.es.word(context.bx+3), 7);
	if (!context.flags.c()) goto nopriesttext;
	context.al = context.es.byte(context.bx+3);
	context._and(context.al, 1);
	if (!context.flags.z()) goto nopriesttext;
	context.al = context.es.byte(context.bx+3);
	context._shr(context.al, 1);
	context._add(context.al, 50);
	context.bl = 72;
	context.bh = 80;
	context.cx = 54;
	context.dx = 1;
	setuptimeduse(context);
nopriesttext:
	{assert(stack_depth == context.stack.size()); return; }
}

void textforend(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kIntrocount), 20);
	if (!context.flags.z()) goto notendtext1;
	context.al = 0;
	context.bl = 34;
	context.bh = 20;
	context.cx = 60;
	goto gotendtext;
notendtext1:
	context._cmp(context.data.byte(kIntrocount), 65);
	if (!context.flags.z()) goto notendtext2;
	context.al = 1;
	context.bl = 34;
	context.bh = 20;
	context.cx = 60;
	goto gotendtext;
notendtext2:
	context._cmp(context.data.byte(kIntrocount), 110);
	if (!context.flags.z()) goto notendtext3;
	context.al = 2;
	context.bl = 34;
	context.bh = 20;
	context.cx = 60;
	goto gotendtext;
notendtext3:
	{assert(stack_depth == context.stack.size()); return; }
gotendtext:
	context.dx = 1;
	context.ah = 83;
	setuptimedtemp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void textformonk(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kIntrocount), 1);
	if (!context.flags.z()) goto notmonktext1;
	context.al = 19;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext1:
	context._cmp(context.data.byte(kIntrocount), 5);
	if (!context.flags.z()) goto notmonktext2;
	context.al = 20;
	context.bl = 68;
	context.bh = 38;
	context.cx = 120;
	goto gotmonktext;
notmonktext2:
	context._cmp(context.data.byte(kIntrocount), 9);
	if (!context.flags.z()) goto notmonktext3;
	context.al = 21;
	context.bl = 48;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext3:
	context._cmp(context.data.byte(kIntrocount), 13);
	if (!context.flags.z()) goto notmonktext4;
	context.al = 22;
	context.bl = 68;
	context.bh = 38;
	context.cx = 120;
	goto gotmonktext;
notmonktext4:
	context._cmp(context.data.byte(kIntrocount), 17);
	if (!context.flags.z()) goto notmonktext5;
	context.al = 23;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext5:
	context._cmp(context.data.byte(kIntrocount), 21);
	if (!context.flags.z()) goto notmonktext6;
	context.al = 24;
	context.bl = 68;
	context.bh = 38;
	context.cx = 120;
	goto gotmonktext;
notmonktext6:
	context._cmp(context.data.byte(kIntrocount), 25);
	if (!context.flags.z()) goto notmonktext7;
	context.al = 25;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext7:
	context._cmp(context.data.byte(kIntrocount), 29);
	if (!context.flags.z()) goto notmonktext8;
	context.al = 26;
	context.bl = 68;
	context.bh = 38;
	context.cx = 120;
	goto gotmonktext;
notmonktext8:
	context._cmp(context.data.byte(kIntrocount), 33);
	if (!context.flags.z()) goto notmonktext9;
	context.al = 27;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext9:
	context._cmp(context.data.byte(kIntrocount), 37);
	if (!context.flags.z()) goto notmonktext10;
	context.al = 28;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext10:
	context._cmp(context.data.byte(kIntrocount), 41);
	if (!context.flags.z()) goto notmonktext11;
	context.al = 29;
	context.bl = 68;
	context.bh = 38;
	context.cx = 120;
	goto gotmonktext;
notmonktext11:
	context._cmp(context.data.byte(kIntrocount), 45);
	if (!context.flags.z()) goto notmonktext12;
	context.al = 30;
	context.bl = 68;
	context.bh = 154;
	context.cx = 120;
	goto gotmonktext;
notmonktext12:
	context._cmp(context.data.byte(kIntrocount), 49);
	if (!context.flags.z()) goto notmonktext13;
	context.al = 31;
	context.bl = 68;
	context.bh = 154;
	context.cx = 220;
	goto gotmonktext;
notmonktext13:
	context._cmp(context.data.byte(kIntrocount), 53);
	if (!context.flags.z()) goto notendtitles;
	fadescreendowns(context);
notendtitles:
	{assert(stack_depth == context.stack.size()); return; }
gotmonktext:
	context.dx = 1;
	context.ah = 82;
oktalk:
	setuptimedtemp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void drunk(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kGeneraldead), 0);
	if (!context.flags.z()) goto trampgone;
	context.al = context.es.byte(context.bx+7);
	context._and(context.al, 127);
	context.es.byte(context.bx+7) = context.al;
	showgamereel(context);
	addtopeoplelist(context);
trampgone:
	{assert(stack_depth == context.stack.size()); return; }
}

void advisor(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto noadvisor;
	goto noadvisor;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
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
	context.es.word(context.bx+3) = context.ax;
noadvisor:
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void copper(Context & context) {
	uint stack_depth = context.stack.size();
	checkspeed(context);
	if (!context.flags.z()) goto nocopper;
	context.ax = context.es.word(context.bx+3);
	context._inc(context.ax);
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
	context._dec(context.ax);
gotcopframe:
	context.es.word(context.bx+3) = context.ax;
nocopper:
	showgamereel(context);
	addtopeoplelist(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void sparky(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kCard1money), 0);
	if (context.flags.z()) goto animsparky;
	context.es.byte(context.bx+7) = 3;
	goto animsparky;
animsparky:
	checkspeed(context);
	if (!context.flags.z()) goto finishsparky;
	context._cmp(context.es.word(context.bx+3), 34);
	if (!context.flags.z()) goto notsparky1;
	randomnumber(context);
	context._cmp(context.al, 30);
	if (context.flags.c()) goto dosparky;
	context.es.word(context.bx+3) = 27;
	goto finishsparky;
notsparky1:
	context._cmp(context.es.word(context.bx+3), 48);
	if (!context.flags.z()) goto dosparky;
	context.es.word(context.bx+3) = 27;
	goto finishsparky;
dosparky:
	context._inc(context.es.word(context.bx+3));
finishsparky:
	showgamereel(context);
	addtopeoplelist(context);
	context.al = context.es.byte(context.bx+7);
	context._and(context.al, 128);
	if (context.flags.z()) goto nottalkedsparky;
	context.data.byte(kTalkedtosparky) = 1;
nottalkedsparky:
	{assert(stack_depth == context.stack.size()); return; }
}

void train(Context & context) {
	uint stack_depth = context.stack.size();
	{assert(stack_depth == context.stack.size()); return; }
	context.ax = context.es.word(context.bx+3);
	context._cmp(context.ax, 21);
	if (!context.flags.c()) goto notrainyet;
	context._inc(context.ax);
	goto gottrainframe;
notrainyet:
	randomnumber(context);
	context._cmp(context.al, 253);
	if (context.flags.c()) goto notrainatall;
	context._cmp(context.data.byte(kManspath), 5);
	if (!context.flags.z()) goto notrainatall;
	context._cmp(context.data.byte(kFinaldest), 5);
	if (!context.flags.z()) goto notrainatall;
	context.ax = 5;
gottrainframe:
	context.es.word(context.bx+3) = context.ax;
	showgamereel(context);
notrainatall:
	{assert(stack_depth == context.stack.size()); return; }
}

void addtopeoplelist(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	context.push(context.bx);
	context.cl = context.es.byte(context.bx+7);
	context.ax = context.es.word(context.bx+3);
	context.bx = context.data.word(kListpos);
	context.es = context.data.word(kBuffers);
	context.es.word(context.bx) = context.ax;
	context.ax = context.pop();
	context.es.word(context.bx+2) = context.ax;
	context.es.byte(context.bx+4) = context.cl;
	context.bx = context.pop();
	context.es = context.pop();
	context._add(context.data.word(kListpos), 5);
	{assert(stack_depth == context.stack.size()); return; }
}

void showgamereel(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.es.word(context.bx+3);
	context._cmp(context.ax, 512);
	if (!context.flags.c()) goto noshow;
	context.data.word(kReelpointer) = context.ax;
	context.push(context.es);
	context.push(context.bx);
	plotreel(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.ax = context.data.word(kReelpointer);
	context.es.word(context.bx+3) = context.ax;
noshow:
	{assert(stack_depth == context.stack.size()); return; }
}

void checkspeed(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLastweapon), -1);
	if (!context.flags.z()) goto forcenext;
	context._inc(context.es.byte(context.bx+6));
	context.al = context.es.byte(context.bx+6);
	context._cmp(context.al, context.es.byte(context.bx+5));
	if (!context.flags.z()) goto notspeed;
	context.al = 0;
	context.es.byte(context.bx+6) = context.al;
	context._cmp(context.al, context.al);
notspeed:
	{assert(stack_depth == context.stack.size()); return; }
forcenext:
	context._cmp(context.al, context.al);
	{assert(stack_depth == context.stack.size()); return; }
}

void clearsprites(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768);
	context.al = 255;
	context.cx = (32)*16;
	while(context.cx--) 	context._stosb();
	{assert(stack_depth == context.stack.size()); return; }
}

void makesprite(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768);
_tmp17:
	context._cmp(context.es.byte(context.bx+15), 255);
	if (context.flags.z()) goto _tmp17a;
	context._add(context.bx, (32));
	goto _tmp17;
_tmp17a:
	context.es.word(context.bx) = context.cx;
	context.es.word(context.bx+10) = context.si;
	context.es.word(context.bx+6) = context.dx;
	context.es.word(context.bx+8) = context.di;
	context.es.word(context.bx+2) = 0x0ffff;
	context.es.byte(context.bx+15) = 0;
	context.es.byte(context.bx+18) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void delsprite(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = context.bx;
	context.cx = (32);
	context.al = 255;
	while(context.cx--) 	context._stosb();
	{assert(stack_depth == context.stack.size()); return; }
}

void spriteupdate(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768);
	context.al = context.data.byte(kRyanon);
	context.es.byte(context.bx+31) = context.al;
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768);
	context.cx = 16;
_tmp18:
	context.push(context.cx);
	context.push(context.bx);
	context.ax = context.es.word(context.bx);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto _tmp18a;
	context.push(context.es);
	context.push(context.ds);
	context.cx = context.es.word(context.bx+2);
	context.es.word(context.bx+24) = context.cx;
	__dispatch_call(context, context.ax);
	context.ds = context.pop();
	context.es = context.pop();
_tmp18a:
	context.bx = context.pop();
	context.cx = context.pop();
	context._cmp(context.data.byte(kNowinnewroom), 1);
	if (context.flags.z()) goto _tmp18b;
	context._add(context.bx, (32));
	if (--context.cx) goto _tmp18;
_tmp18b:
	{assert(stack_depth == context.stack.size()); return; }
}

void printsprites(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.cx = 0;
priorityloop:
	context.push(context.cx);
	context.data.byte(kPriority) = context.cl;
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768);
	context.cx = 16;
prtspriteloop:
	context.push(context.cx);
	context.push(context.bx);
	context.ax = context.es.word(context.bx);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto skipsprite;
	context.al = context.data.byte(kPriority);
	context._cmp(context.al, context.es.byte(context.bx+23));
	if (!context.flags.z()) goto skipsprite;
	context._cmp(context.es.byte(context.bx+31), 1);
	if (context.flags.z()) goto skipsprite;
	printasprite(context);
skipsprite:
	context.bx = context.pop();
	context.cx = context.pop();
	context._add(context.bx, (32));
	if (--context.cx) goto prtspriteloop;
	context.cx = context.pop();
	context._inc(context.cx);
	context._cmp(context.cx, 7);
	if (!context.flags.z()) goto priorityloop;
	{assert(stack_depth == context.stack.size()); return; }
}

void printasprite(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	context.si = context.bx;
	context.ds = context.es.word(context.si+6);
	context.al = context.es.byte(context.si+11);
	context.ah = 0;
	context._cmp(context.al, 220);
	if (context.flags.c()) goto notnegative1;
	context.ah = 255;
notnegative1:
	context.bx = context.ax;
	context._add(context.bx, context.data.word(kMapady));
	context.al = context.es.byte(context.si+10);
	context.ah = 0;
	context._cmp(context.al, 220);
	if (context.flags.c()) goto notnegative2;
	context.ah = 255;
notnegative2:
	context.di = context.ax;
	context._add(context.di, context.data.word(kMapadx));
	context.al = context.es.byte(context.si+15);
	context.ah = 0;
	context._cmp(context.es.byte(context.si+30), 0);
	if (context.flags.z()) goto steadyframe;
	context.ah = 8;
steadyframe:
	context._cmp(context.data.byte(kPriority), 6);
	if (!context.flags.z()) goto notquickp;
notquickp:
	showframe(context);
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void checkone(Context & context) {
	uint stack_depth = context.stack.size();
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
	context.ds = context.data.word(kBuffers);
	context._add(context.si, (0+(180*10)+32+60+(32*32)));
	context._lodsw();
	context.cx = context.ax;
	context._lodsb();
	context.dx = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void findsource(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kCurrentframe);
	context._cmp(context.ax, 160);
	if (!context.flags.c()) goto over1000;
	context.ds = context.data.word(kReel1);
	context.data.word(kTakeoff) = 0;
	{assert(stack_depth == context.stack.size()); return; }
over1000:
	context._cmp(context.ax, 320);
	if (!context.flags.c()) goto over1001;
	context.ds = context.data.word(kReel2);
	context.data.word(kTakeoff) = 160;
	{assert(stack_depth == context.stack.size()); return; }
over1001:
	context.ds = context.data.word(kReel3);
	context.data.word(kTakeoff) = 320;
	{assert(stack_depth == context.stack.size()); return; }
}

void initman(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kRyanx);
	context.ah = context.data.byte(kRyany);
	context.si = context.ax;
	context.cx = 49464;
	context.dx = context.data.word(kMainsprites);
	context.di = 0;
	makesprite(context);
	context.es.byte(context.bx+23) = 4;
	context.es.byte(context.bx+22) = 0;
	context.es.byte(context.bx+29) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void mainman(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kResetmanxy), 1);
	if (!context.flags.z()) goto notinnewroom;
	context.data.byte(kResetmanxy) = 0;
	context.al = context.data.byte(kRyanx);
	context.ah = context.data.byte(kRyany);
	context.es.word(context.bx+10) = context.ax;
	context.es.byte(context.bx+29) = 0;
	goto executewalk;
notinnewroom:
	context._dec(context.es.byte(context.bx+22));
	context._cmp(context.es.byte(context.bx+22), -1);
	if (context.flags.z()) goto executewalk;
	{assert(stack_depth == context.stack.size()); return; }
executewalk:
	context.es.byte(context.bx+22) = 0;
	context.al = context.data.byte(kTurntoface);
	context._cmp(context.al, context.data.byte(kFacing));
	if (context.flags.z()) goto facingok;
	aboutturn(context);
	goto notwalk;
facingok:
	context._cmp(context.data.byte(kTurndirection), 0);
	if (context.flags.z()) goto alreadyturned;
	context._cmp(context.data.byte(kLinepointer), 254);
	if (!context.flags.z()) goto alreadyturned;
	context.data.byte(kReasseschanges) = 1;
	context.al = context.data.byte(kFacing);
	context._cmp(context.al, context.data.byte(kLeavedirection));
	if (!context.flags.z()) goto alreadyturned;
	checkforexit(context);
alreadyturned:
	context.data.byte(kTurndirection) = 0;
	context._cmp(context.data.byte(kLinepointer), 254);
	if (!context.flags.z()) goto walkman;
	context.es.byte(context.bx+29) = 0;
	goto notwalk;
walkman:
	context.al = context.es.byte(context.bx+29);
	context._inc(context.al);
	context._cmp(context.al, 11);
	if (!context.flags.z()) goto notanimend1;
	context.al = 1;
notanimend1:
	context.es.byte(context.bx+29) = context.al;
	walking(context);
	context._cmp(context.data.byte(kLinepointer), 254);
	if (context.flags.z()) goto afterwalk;
	context.al = context.data.byte(kFacing);
	context._and(context.al, 1);
	if (context.flags.z()) goto isdouble;
	context.al = context.es.byte(context.bx+29);
	context._cmp(context.al, 2);
	if (context.flags.z()) goto afterwalk;
	context._cmp(context.al, 7);
	if (context.flags.z()) goto afterwalk;
isdouble:
	walking(context);
afterwalk:
	context._cmp(context.data.byte(kLinepointer), 254);
	if (!context.flags.z()) goto notwalk;
	context.al = context.data.byte(kTurntoface);
	context._cmp(context.al, context.data.byte(kFacing));
	if (!context.flags.z()) goto notwalk;
	context.data.byte(kReasseschanges) = 1;
	context.al = context.data.byte(kFacing);
	context._cmp(context.al, context.data.byte(kLeavedirection));
	if (!context.flags.z()) goto notwalk;
	checkforexit(context);
notwalk:
	context.al = context.data.byte(kFacing);
	context.ah = 0;
	context.di = 1105;
	context._add(context.di, context.ax);
	context.al = context.cs.byte(context.di);
	context._add(context.al, context.es.byte(context.bx+29));
	context.es.byte(context.bx+15) = context.al;
	context.ax = context.es.word(context.bx+10);
	context.data.byte(kRyanx) = context.al;
	context.data.byte(kRyany) = context.ah;
	{assert(stack_depth == context.stack.size()); return; }
}

void aboutturn(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kTurndirection), 1);
	if (context.flags.z()) goto incdir;
	context._cmp(context.data.byte(kTurndirection), -1);
	if (context.flags.z()) goto decdir;
	context.al = context.data.byte(kFacing);
	context._sub(context.al, context.data.byte(kTurntoface));
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
	context.data.byte(kTurndirection) = 1;
	context.al = context.data.byte(kFacing);
	context._inc(context.al);
	context._and(context.al, 7);
	context.data.byte(kFacing) = context.al;
	context.es.byte(context.bx+29) = 0;
	{assert(stack_depth == context.stack.size()); return; }
decdir:
	context.data.byte(kTurndirection) = -1;
	context.al = context.data.byte(kFacing);
	context._dec(context.al);
	context._and(context.al, 7);
	context.data.byte(kFacing) = context.al;
	context.es.byte(context.bx+29) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void walking(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLinedirection), 0);
	if (context.flags.z()) goto normalwalk;
	context.al = context.data.byte(kLinepointer);
	context._dec(context.al);
	context.data.byte(kLinepointer) = context.al;
	context._cmp(context.al, 200);
	if (!context.flags.c()) goto endofline;
	goto continuewalk;
normalwalk:
	context.al = context.data.byte(kLinepointer);
	context._inc(context.al);
	context.data.byte(kLinepointer) = context.al;
	context._cmp(context.al, context.data.byte(kLinelength));
	if (!context.flags.c()) goto endofline;
continuewalk:
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.push(context.es);
	context.push(context.bx);
	context.dx = context.data;
	context.es = context.dx;
	context.bx = 7944;
	context._add(context.bx, context.ax);
	context.ax = context.es.word(context.bx);
	context.bx = context.pop();
	context.es = context.pop();
stillline:
	context.es.word(context.bx+10) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
endofline:
	context.data.byte(kLinepointer) = 254;
	context.al = context.data.byte(kDestination);
	context.data.byte(kManspath) = context.al;
	context._cmp(context.al, context.data.byte(kFinaldest));
	if (context.flags.z()) goto finishedwalk;
	context.al = context.data.byte(kFinaldest);
	context.data.byte(kDestination) = context.al;
	context.push(context.es);
	context.push(context.bx);
	autosetwalk(context);
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
finishedwalk:
	facerightway(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void facerightway(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	getroomspaths(context);
	context.al = context.data.byte(kManspath);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.es.byte(context.bx+7);
	context.data.byte(kTurntoface) = context.al;
	context.data.byte(kLeavedirection) = context.al;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void checkforexit(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = context.data.byte(kRyanx);
	context._add(context.cl, 12);
	context.ch = context.data.byte(kRyany);
	context._add(context.ch, 12);
	checkone(context);
	context.data.byte(kLastflag) = context.cl;
	context.data.byte(kLastflagex) = context.ch;
	context.data.byte(kFlagx) = context.dl;
	context.data.byte(kFlagy) = context.dh;
	context.al = context.data.byte(kLastflag);
	context._test(context.al, 64);
	if (context.flags.z()) goto notnewdirect;
	context.al = context.data.byte(kLastflagex);
	context.data.byte(kAutolocation) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
notnewdirect:
	context._test(context.al, 32);
	if (context.flags.z()) goto notleave;
	context.push(context.es);
	context.push(context.bx);
	context._cmp(context.data.byte(kReallocation), 2);
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
	context._inc(context.bl);
noshoe1:
	context.push(context.bx);
	context.al = 'W';
	context.ah = 'E';
	context.cl = 'T';
	context.ch = 'B';
	isryanholding(context);
	context.bx = context.pop();
	if (context.flags.z()) goto noshoe2;
	context._inc(context.bl);
noshoe2:
	context._cmp(context.bl, 2);
	if (context.flags.z()) goto notlouis;
	context.al = 42;
	context._cmp(context.bl, 0);
	if (context.flags.z()) goto notravmessage;
	context._inc(context.al);
notravmessage:
	context.cx = 80;
	context.dx = 10;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	context.al = context.data.byte(kFacing);
	context._add(context.al, 4);
	context._and(context.al, 7);
	context.data.byte(kTurntoface) = context.al;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
notlouis:
	context.bx = context.pop();
	context.es = context.pop();
	context.data.byte(kNeedtotravel) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notleave:
	context._test(context.al, 4);
	if (context.flags.z()) goto notaleft;
	adjustleft(context);
	{assert(stack_depth == context.stack.size()); return; }
notaleft:
	context._test(context.al, 2);
	if (context.flags.z()) goto notaright;
	adjustright(context);
	{assert(stack_depth == context.stack.size()); return; }
notaright:
	context._test(context.al, 8);
	if (context.flags.z()) goto notadown;
	adjustdown(context);
	{assert(stack_depth == context.stack.size()); return; }
notadown:
	context._test(context.al, 16);
	if (context.flags.z()) goto notanup;
	adjustup(context);
	{assert(stack_depth == context.stack.size()); return; }
notanup:
	{assert(stack_depth == context.stack.size()); return; }
}

void adjustdown(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	context._add(context.data.byte(kMapy), 10);
	context.al = context.data.byte(kLastflagex);
	context.cl = 16;
	context._mul(context.cl);
	context.es.byte(context.bx+11) = context.al;
	context.data.byte(kNowinnewroom) = 1;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void adjustup(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	context._sub(context.data.byte(kMapy), 10);
	context.al = context.data.byte(kLastflagex);
	context.cl = 16;
	context._mul(context.cl);
	context.es.byte(context.bx+11) = context.al;
	context.data.byte(kNowinnewroom) = 1;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void adjustleft(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	context.data.byte(kLastflag) = 0;
	context._sub(context.data.byte(kMapx), 11);
	context.al = context.data.byte(kLastflagex);
	context.cl = 16;
	context._mul(context.cl);
	context.es.byte(context.bx+10) = context.al;
	context.data.byte(kNowinnewroom) = 1;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void adjustright(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	context._add(context.data.byte(kMapx), 11);
	context.al = context.data.byte(kLastflagex);
	context.cl = 16;
	context._mul(context.cl);
	context._sub(context.al, 2);
	context.es.byte(context.bx+10) = context.al;
	context.data.byte(kNowinnewroom) = 1;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void reminders(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kReallocation), 24);
	if (!context.flags.z()) goto notinedenslift;
	context._cmp(context.data.byte(kMapx), 44);
	if (!context.flags.z()) goto notinedenslift;
	context._cmp(context.data.byte(kProgresspoints), 0);
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
	context._cmp(context.al, (114));
	if (context.flags.z()) goto forgotone;
	context.ax = context.es.word(context.bx+2);
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
	context._inc(context.data.byte(kProgresspoints));
notfirst:
	{assert(stack_depth == context.stack.size()); return; }
forgotone:
	context.al = 50;
	context.bl = 54;
	context.bh = 70;
	context.cx = 48;
	context.dx = 8;
	setuptimeduse(context);
	{assert(stack_depth == context.stack.size()); return; }
notinedenslift:
	{assert(stack_depth == context.stack.size()); return; }
}

void initrain(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*24));
	context.bx = 1113;
checkmorerain:
	context.al = context.cs.byte(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto finishinitrain;
	context._cmp(context.al, context.data.byte(kReallocation));
	if (!context.flags.z()) goto checkrain;
	context.al = context.cs.byte(context.bx+1);
	context._cmp(context.al, context.data.byte(kMapx));
	if (!context.flags.z()) goto checkrain;
	context.al = context.cs.byte(context.bx+2);
	context._cmp(context.al, context.data.byte(kMapy));
	if (!context.flags.z()) goto checkrain;
	context.al = context.cs.byte(context.bx+3);
	context.data.byte(kRainspace) = context.al;
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
	context._cmp(context.al, context.data.byte(kRainspace));
	if (!context.flags.c()) goto initraintop;
	context._add(context.cl, context.al);
	context._cmp(context.cl, context.data.byte(kMapxsize));
	if (!context.flags.c()) goto initrainside;
	context.push(context.cx);
	splitintolines(context);
	context.cx = context.pop();
	goto initraintop;
initrainside:
	context.cl = context.data.byte(kMapxsize);
	context._dec(context.cl);
initrainside2:
	randomnumber(context);
	context._and(context.al, 31);
	context._add(context.al, 3);
	context._cmp(context.al, context.data.byte(kRainspace));
	if (!context.flags.c()) goto initrainside2;
	context._add(context.ch, context.al);
	context._cmp(context.ch, context.data.byte(kMapysize));
	if (!context.flags.c()) goto finishinitrain;
	context.push(context.cx);
	splitintolines(context);
	context.cx = context.pop();
	goto initrainside2;
finishinitrain:
	context.al = 255;
	context._stosb();
	{assert(stack_depth == context.stack.size()); return; }
}

void splitintolines(Context & context) {
	uint stack_depth = context.stack.size();
lookforlinestart:
	getblockofpixel(context);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto foundlinestart;
	context._dec(context.cl);
	context._inc(context.ch);
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto endofthisline;
	context._cmp(context.ch, context.data.byte(kMapysize));
	if (!context.flags.c()) goto endofthisline;
	goto lookforlinestart;
foundlinestart:
	context.es.word(context.di) = context.cx;
	context.bh = 1;
lookforlineend:
	getblockofpixel(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundlineend;
	context._dec(context.cl);
	context._inc(context.ch);
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto foundlineend;
	context._cmp(context.ch, context.data.byte(kMapysize));
	if (!context.flags.c()) goto foundlineend;
	context._inc(context.bh);
	goto lookforlineend;
foundlineend:
	context.push(context.cx);
	context.es.byte(context.di+2) = context.bh;
	randomnumber(context);
	context.es.byte(context.di+3) = context.al;
	randomnumber(context);
	context.es.byte(context.di+4) = context.al;
	randomnumber(context);
	context._and(context.al, 3);
	context._add(context.al, 4);
	context.es.byte(context.di+5) = context.al;
	context._add(context.di, 6);
	context.cx = context.pop();
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto endofthisline;
	context._cmp(context.ch, context.data.byte(kMapysize));
	if (!context.flags.c()) goto endofthisline;
	goto lookforlinestart;
endofthisline:
	{assert(stack_depth == context.stack.size()); return; }
}

void getblockofpixel(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.cx);
	context.push(context.es);
	context.push(context.di);
	context.ax = context.data.word(kMapxstart);
	context._add(context.cl, context.al);
	context.ax = context.data.word(kMapystart);
	context._add(context.ch, context.al);
	checkone(context);
	context._and(context.cl, 1);
	if (!context.flags.z()) goto failrain;
	context.di = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
failrain:
	context.di = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void showrain(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kMainsprites);
	context.si = 6*58;
	context.ax = context.ds.word(context.si+2);
	context.si = context.ax;
	context._add(context.si, 2080);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*24));
	context.es = context.data.word(kBuffers);
	context._cmp(context.es.byte(context.bx), 255);
	if (context.flags.z()) goto nothunder;
morerain:
	context.es = context.data.word(kBuffers);
	context._cmp(context.es.byte(context.bx), 255);
	if (context.flags.z()) goto finishrain;
	context.al = context.es.byte(context.bx+1);
	context.ah = 0;
	context._add(context.ax, context.data.word(kMapady));
	context._add(context.ax, context.data.word(kMapystart));
	context.cx = 320;
	context._mul(context.cx);
	context.cl = context.es.byte(context.bx);
	context.ch = 0;
	context._add(context.ax, context.cx);
	context._add(context.ax, context.data.word(kMapadx));
	context._add(context.ax, context.data.word(kMapxstart));
	context.di = context.ax;
	context.cl = context.es.byte(context.bx+2);
	context.ch = 0;
	context.ax = context.es.word(context.bx+3);
	context.dl = context.es.byte(context.bx+5);
	context.dh = 0;
	context._sub(context.ax, context.dx);
	context._and(context.ax, 511);
	context.es.word(context.bx+3) = context.ax;
	context._add(context.bx, 6);
	context.push(context.si);
	context._add(context.si, context.ax);
	context.es = context.data.word(kWorkspace);
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
	context._cmp(context.data.word(kCh1blockstocopy), 0);
	if (!context.flags.z()) goto nothunder;
	context._cmp(context.data.byte(kReallocation), 2);
	if (!context.flags.z()) goto notlouisthund;
	context._cmp(context.data.byte(kBeenmugged), 1);
	if (!context.flags.z()) goto nothunder;
notlouisthund:
	context._cmp(context.data.byte(kReallocation), 55);
	if (context.flags.z()) goto nothunder;
	randomnum1(context);
	context._cmp(context.al, 1);
	if (!context.flags.c()) goto nothunder;
	context.al = 7;
	context._cmp(context.data.byte(kCh0playing), 6);
	if (context.flags.z()) goto isthunder1;
	context.al = 4;
isthunder1:
	playchannel1(context);
nothunder:
	{assert(stack_depth == context.stack.size()); return; }
}

void backobject(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kSetdat);
	context.di = context.es.word(context.bx+20);
	context.al = context.es.byte(context.bx+18);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto _tmp48z;
	context._dec(context.al);
	context.es.byte(context.bx+18) = context.al;
	goto finishback;
_tmp48z:
	context.al = context.ds.byte(context.di+7);
	context.es.byte(context.bx+18) = context.al;
	context.al = context.ds.byte(context.di+8);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void liftsprite(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kLiftflag);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto liftclosed;
	context._cmp(context.al, 1);
	if (context.flags.z()) goto liftopen;
	context._cmp(context.al, 3);
	if (context.flags.z()) goto openlift;
	context.al = context.es.byte(context.bx+19);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto finishclose;
	context._dec(context.al);
	context._cmp(context.al, 11);
	if (!context.flags.z()) goto pokelift;
	context.push(context.ax);
	context.al = 3;
	liftnoise(context);
	context.ax = context.pop();
	goto pokelift;
finishclose:
	context.data.byte(kLiftflag) = 0;
	{assert(stack_depth == context.stack.size()); return; }
openlift:
	context.al = context.es.byte(context.bx+19);
	context._cmp(context.al, 12);
	if (context.flags.z()) goto endoflist;
	context._inc(context.al);
	context._cmp(context.al, 1);
	if (!context.flags.z()) goto pokelift;
	context.push(context.ax);
	context.al = 2;
	liftnoise(context);
	context.ax = context.pop();
pokelift:
	context.es.byte(context.bx+19) = context.al;
	context.ah = 0;
	context.push(context.di);
	context._add(context.di, context.ax);
	context.al = context.ds.byte(context.di+18);
	context.di = context.pop();
	context.es.byte(context.bx+15) = context.al;
	context.ds.byte(context.di+17) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
endoflist:
	context.data.byte(kLiftflag) = 1;
	{assert(stack_depth == context.stack.size()); return; }
liftopen:
	context.al = context.data.byte(kLiftpath);
	context.push(context.es);
	context.push(context.bx);
	turnpathon(context);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.data.byte(kCounttoclose), 0);
	if (context.flags.z()) goto nocountclose;
	context._dec(context.data.byte(kCounttoclose));
	context._cmp(context.data.byte(kCounttoclose), 0);
	if (!context.flags.z()) goto nocountclose;
	context.data.byte(kLiftflag) = 2;
nocountclose:
	context.al = 12;
	goto pokelift;
liftclosed:
	context.al = context.data.byte(kLiftpath);
	context.push(context.es);
	context.push(context.bx);
	turnpathoff(context);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.data.byte(kCounttoopen), 0);
	if (context.flags.z()) goto nocountopen;
	context._dec(context.data.byte(kCounttoopen));
	context._cmp(context.data.byte(kCounttoopen), 0);
	if (!context.flags.z()) goto nocountopen;
	context.data.byte(kLiftflag) = 3;
nocountopen:
	context.al = 0;
	goto pokelift;
}

void liftnoise(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kReallocation), 5);
	if (context.flags.z()) goto hissnoise;
	context._cmp(context.data.byte(kReallocation), 21);
	if (context.flags.z()) goto hissnoise;
	playchannel1(context);
	{assert(stack_depth == context.stack.size()); return; }
hissnoise:
	context.al = 13;
	playchannel1(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void random(Context & context) {
	uint stack_depth = context.stack.size();
	randomnum1(context);
	context.push(context.di);
	context._and(context.ax, 7);
	context._add(context.di, 18);
	context._add(context.di, context.ax);
	context.al = context.ds.byte(context.di);
	context.di = context.pop();
	context.es.byte(context.bx+15) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void steady(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.ds.byte(context.di+18);
	context.ds.byte(context.di+17) = context.al;
	context.es.byte(context.bx+15) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void constant(Context & context) {
	uint stack_depth = context.stack.size();
	context._inc(context.es.byte(context.bx+19));
	context.cl = context.es.byte(context.bx+19);
	context.ch = 0;
	context._add(context.di, context.cx);
	context._cmp(context.ds.byte(context.di+18), 255);
	if (!context.flags.z()) goto gotconst;
	context._sub(context.di, context.cx);
	context.cx = 0;
	context.es.byte(context.bx+19) = context.cl;
gotconst:
	context.al = context.ds.byte(context.di+18);
	context._sub(context.di, context.cx);
	context.es.byte(context.bx+15) = context.al;
	context.ds.byte(context.di+17) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void doorway(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kDoorcheck1) = -24;
	context.data.byte(kDoorcheck2) = 10;
	context.data.byte(kDoorcheck3) = -30;
	context.data.byte(kDoorcheck4) = 10;
	dodoor(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void widedoor(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kDoorcheck1) = -24;
	context.data.byte(kDoorcheck2) = 24;
	context.data.byte(kDoorcheck3) = -30;
	context.data.byte(kDoorcheck4) = 24;
	dodoor(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void dodoor(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kRyanx);
	context.ah = context.data.byte(kRyany);
	context.cl = context.es.byte(context.bx+10);
	context.ch = context.es.byte(context.bx+11);
	context._cmp(context.al, context.cl);
	if (!context.flags.c()) goto rtofdoor;
	context._sub(context.al, context.cl);
	context._cmp(context.al, context.data.byte(kDoorcheck1));
	if (!context.flags.c()) goto upordown;
	goto shutdoor;
rtofdoor:
	context._sub(context.al, context.cl);
	context._cmp(context.al, context.data.byte(kDoorcheck2));
	if (!context.flags.c()) goto shutdoor;
upordown:
	context._cmp(context.ah, context.ch);
	if (!context.flags.c()) goto botofdoor;
	context._sub(context.ah, context.ch);
	context._cmp(context.ah, context.data.byte(kDoorcheck3));
	if (context.flags.c()) goto shutdoor;
	goto opendoor;
botofdoor:
	context._sub(context.ah, context.ch);
	context._cmp(context.ah, context.data.byte(kDoorcheck4));
	if (!context.flags.c()) goto shutdoor;
opendoor:
	context.cl = context.es.byte(context.bx+19);
	context._cmp(context.data.byte(kThroughdoor), 1);
	if (!context.flags.z()) goto notthrough;
	context._cmp(context.cl, 0);
	if (!context.flags.z()) goto notthrough;
	context.cl = 6;
notthrough:
	context._inc(context.cl);
	context._cmp(context.cl, 1);
	if (!context.flags.z()) goto notdoorsound2;
	context.al = 0;
	context._cmp(context.data.byte(kReallocation), 5);
	if (!context.flags.z()) goto nothoteldoor2;
	context.al = 13;
nothoteldoor2:
	playchannel1(context);
notdoorsound2:
	context.ch = 0;
	context.push(context.di);
	context._add(context.di, context.cx);
	context.al = context.ds.byte(context.di+18);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto atlast1;
	context._dec(context.di);
	context._dec(context.cl);
atlast1:
	context.es.byte(context.bx+19) = context.cl;
	context.al = context.ds.byte(context.di+18);
	context.di = context.pop();
	context.es.byte(context.bx+15) = context.al;
	context.ds.byte(context.di+17) = context.al;
	context.data.byte(kThroughdoor) = 1;
	{assert(stack_depth == context.stack.size()); return; }
shutdoor:
	context.cl = context.es.byte(context.bx+19);
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto notdoorsound1;
	context.al = 1;
	context._cmp(context.data.byte(kReallocation), 5);
	if (!context.flags.z()) goto nothoteldoor1;
	context.al = 13;
nothoteldoor1:
	playchannel1(context);
notdoorsound1:
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto atlast2;
	context._dec(context.cl);
	context.es.byte(context.bx+19) = context.cl;
atlast2:
	context.ch = 0;
	context.push(context.di);
	context._add(context.di, context.cx);
	context.al = context.ds.byte(context.di+18);
	context.di = context.pop();
	context.es.byte(context.bx+15) = context.al;
	context.ds.byte(context.di+17) = context.al;
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto notnearly;
	context.data.byte(kThroughdoor) = 0;
notnearly:
	{assert(stack_depth == context.stack.size()); return; }
}

void lockeddoorway(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kRyanx);
	context.ah = context.data.byte(kRyany);
	context.cl = context.es.byte(context.bx+10);
	context.ch = context.es.byte(context.bx+11);
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
	context._cmp(context.data.byte(kThroughdoor), 1);
	if (context.flags.z()) goto mustbeopen;
	context._cmp(context.data.byte(kLockstatus), 1);
	if (context.flags.z()) goto shutdoor;
mustbeopen:
	context.cl = context.es.byte(context.bx+19);
	context._cmp(context.cl, 1);
	if (!context.flags.z()) goto notdoorsound4;
	context.al = 0;
	playchannel1(context);
notdoorsound4:
	context._cmp(context.cl, 6);
	if (!context.flags.z()) goto noturnonyet;
	context.al = context.data.byte(kDoorpath);
	context.push(context.es);
	context.push(context.bx);
	turnpathon(context);
	context.bx = context.pop();
	context.es = context.pop();
noturnonyet:
	context.cl = context.es.byte(context.bx+19);
	context._cmp(context.data.byte(kThroughdoor), 1);
	if (!context.flags.z()) goto notthrough2;
	context._cmp(context.cl, 0);
	if (!context.flags.z()) goto notthrough2;
	context.cl = 6;
notthrough2:
	context._inc(context.cl);
	context.ch = 0;
	context.push(context.di);
	context._add(context.di, context.cx);
	context.al = context.ds.byte(context.di+18);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto atlast3;
	context._dec(context.di);
	context._dec(context.cl);
atlast3:
	context.es.byte(context.bx+19) = context.cl;
	context.al = context.ds.byte(context.di+18);
	context.di = context.pop();
	context.es.byte(context.bx+15) = context.al;
	context.ds.byte(context.di+17) = context.al;
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto justshutting;
	context.data.byte(kThroughdoor) = 1;
justshutting:
	{assert(stack_depth == context.stack.size()); return; }
shutdoor2:
	context.cl = context.es.byte(context.bx+19);
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto notdoorsound3;
	context.al = 1;
	playchannel1(context);
notdoorsound3:
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto atlast4;
	context._dec(context.cl);
	context.es.byte(context.bx+19) = context.cl;
atlast4:
	context.ch = 0;
	context.data.byte(kThroughdoor) = 0;
	context.push(context.di);
	context._add(context.di, context.cx);
	context.al = context.ds.byte(context.di+18);
	context.di = context.pop();
	context.es.byte(context.bx+15) = context.al;
	context.ds.byte(context.di+17) = context.al;
	context._cmp(context.cl, 0);
	if (!context.flags.z()) goto notlocky;
	context.al = context.data.byte(kDoorpath);
	context.push(context.es);
	context.push(context.bx);
	turnpathoff(context);
	context.bx = context.pop();
	context.es = context.pop();
	context.data.byte(kLockstatus) = 1;
notlocky:
	{assert(stack_depth == context.stack.size()); return; }
/*continuing to unbounded code: shutdoor from dodoor:60-87*/
shutdoor:
	context.cl = context.es.byte(context.bx+19);
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto notdoorsound1;
	context.al = 1;
	context._cmp(context.data.byte(kReallocation), 5);
	if (!context.flags.z()) goto nothoteldoor1;
	context.al = 13;
nothoteldoor1:
	playchannel1(context);
notdoorsound1:
	context._cmp(context.cl, 0);
	if (context.flags.z()) goto atlast2;
	context._dec(context.cl);
	context.es.byte(context.bx+19) = context.cl;
atlast2:
	context.ch = 0;
	context.push(context.di);
	context._add(context.di, context.cx);
	context.al = context.ds.byte(context.di+18);
	context.di = context.pop();
	context.es.byte(context.bx+15) = context.al;
	context.ds.byte(context.di+17) = context.al;
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto notnearly;
	context.data.byte(kThroughdoor) = 0;
notnearly:
	{assert(stack_depth == context.stack.size()); return; }
	{assert(stack_depth == context.stack.size()); return; }
}

void updatepeople(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5));
	context.data.word(kListpos) = context.di;
	context.cx = 12*5;
	context.al = 255;
	while(context.cx--) 	context._stosb();
	context._inc(context.data.word(kMaintimer));
	context.es = context.cs;
	context.bx = 534;
	context.di = 991;
updateloop:
	context.al = context.es.byte(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto endupdate;
	context._cmp(context.al, context.data.byte(kReallocation));
	if (!context.flags.z()) goto notinthisroom;
	context.cx = context.es.word(context.bx+1);
	context._cmp(context.cl, context.data.byte(kMapx));
	if (!context.flags.z()) goto notinthisroom;
	context._cmp(context.ch, context.data.byte(kMapy));
	if (!context.flags.z()) goto notinthisroom;
	context.push(context.di);
	context.ax = context.cs.word(context.di);
	__dispatch_call(context, context.ax);
	context.di = context.pop();
notinthisroom:
	context._add(context.bx, 8);
	context._add(context.di, 2);
	goto updateloop;
endupdate:
	{assert(stack_depth == context.stack.size()); return; }
}

void getreelframeax(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ds);
	context.data.word(kCurrentframe) = context.ax;
	findsource(context);
	context.es = context.ds;
	context.ds = context.pop();
	context.ax = context.data.word(kCurrentframe);
	context._sub(context.ax, context.data.word(kTakeoff));
	context._add(context.ax, context.ax);
	context.cx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.cx);
	context.bx = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void reelsonscreen(Context & context) {
	uint stack_depth = context.stack.size();
	reconstruct(context);
	updatepeople(context);
	watchreel(context);
	showrain(context);
	usetimedtext(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void plotreel(Context & context) {
	uint stack_depth = context.stack.size();
	getreelstart(context);
retryreel:
	context.push(context.es);
	context.push(context.si);
	context.ax = context.es.word(context.si+2);
	context._cmp(context.al, 220);
	if (context.flags.c()) goto normalreel;
	context._cmp(context.al, 255);
	if (context.flags.z()) goto normalreel;
	dealwithspecial(context);
	context._inc(context.data.word(kReelpointer));
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
	context.ax = context.es.word(context.si);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void soundonreels(Context & context) {
	uint stack_depth = context.stack.size();
	context.bl = context.data.byte(kReallocation);
	context._add(context.bl, context.bl);
	context._xor(context.bh, context.bh);
	context._add(context.bx, 1214);
	context.si = context.cs.word(context.bx);
reelsoundloop:
	context.al = context.cs.byte(context.si);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto endreelsound;
	context.ax = context.cs.word(context.si+1);
	context._cmp(context.ax, context.data.word(kReelpointer));
	if (!context.flags.z()) goto skipreelsound;
	context._cmp(context.ax, context.data.word(kLastsoundreel));
	if (context.flags.z()) goto skipreelsound;
	context.data.word(kLastsoundreel) = context.ax;
	context.al = context.cs.byte(context.si);
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
	context.ax = context.data.word(kLastsoundreel);
	context._cmp(context.ax, context.data.word(kReelpointer));
	if (context.flags.z()) goto nochange2;
	context.data.word(kLastsoundreel) = -1;
nochange2:
	{assert(stack_depth == context.stack.size()); return; }
}

void reconstruct(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kHavedoneobs), 0);
	if (context.flags.z()) goto noneedtorecon;
	context.data.byte(kNewobs) = 1;
	drawfloor(context);
	spriteupdate(context);
	printsprites(context);
	context.data.byte(kHavedoneobs) = 0;
noneedtorecon:
	{assert(stack_depth == context.stack.size()); return; }
}

void dealwithspecial(Context & context) {
	uint stack_depth = context.stack.size();
	context._sub(context.al, 220);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto notplset;
	context.al = context.ah;
	placesetobject(context);
	context.data.byte(kHavedoneobs) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notplset:
	context._cmp(context.al, 1);
	if (!context.flags.z()) goto notremset;
	context.al = context.ah;
	removesetobject(context);
	context.data.byte(kHavedoneobs) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notremset:
	context._cmp(context.al, 2);
	if (!context.flags.z()) goto notplfree;
	context.al = context.ah;
	placefreeobject(context);
	context.data.byte(kHavedoneobs) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notplfree:
	context._cmp(context.al, 3);
	if (!context.flags.z()) goto notremfree;
	context.al = context.ah;
	removefreeobject(context);
	context.data.byte(kHavedoneobs) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notremfree:
	context._cmp(context.al, 4);
	if (!context.flags.z()) goto notryanoff;
	switchryanoff(context);
	{assert(stack_depth == context.stack.size()); return; }
notryanoff:
	context._cmp(context.al, 5);
	if (!context.flags.z()) goto notryanon;
	context.data.byte(kTurntoface) = context.ah;
	context.data.byte(kFacing) = context.ah;
	switchryanon(context);
	{assert(stack_depth == context.stack.size()); return; }
notryanon:
	context._cmp(context.al, 6);
	if (!context.flags.z()) goto notchangeloc;
	context.data.byte(kNewlocation) = context.ah;
	{assert(stack_depth == context.stack.size()); return; }
notchangeloc:
	movemap(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void movemap(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.ah, 32);
	if (!context.flags.z()) goto notmapup2;
	context._sub(context.data.byte(kMapy), 20);
	context.data.byte(kNowinnewroom) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notmapup2:
	context._cmp(context.ah, 16);
	if (!context.flags.z()) goto notmapupspec;
	context._sub(context.data.byte(kMapy), 10);
	context.data.byte(kNowinnewroom) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notmapupspec:
	context._cmp(context.ah, 8);
	if (!context.flags.z()) goto notmapdownspec;
	context._add(context.data.byte(kMapy), 10);
	context.data.byte(kNowinnewroom) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notmapdownspec:
	context._cmp(context.ah, 2);
	if (!context.flags.z()) goto notmaprightspec;
	context._add(context.data.byte(kMapx), 11);
	context.data.byte(kNowinnewroom) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notmaprightspec:
	context._sub(context.data.byte(kMapx), 11);
	context.data.byte(kNowinnewroom) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void getreelstart(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kReelpointer);
	context.cx = 40;
	context._mul(context.cx);
	context.es = context.data.word(kReels);
	context.si = context.ax;
	context._add(context.si, (0+(36*144)));
	{assert(stack_depth == context.stack.size()); return; }
}

void showreelframe(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.es.byte(context.si+2);
	context.ah = 0;
	context.di = context.ax;
	context._add(context.di, context.data.word(kMapadx));
	context.al = context.es.byte(context.si+3);
	context.bx = context.ax;
	context._add(context.bx, context.data.word(kMapady));
	context.ax = context.es.word(context.si);
	context.data.word(kCurrentframe) = context.ax;
	findsource(context);
	context.ax = context.data.word(kCurrentframe);
	context._sub(context.ax, context.data.word(kTakeoff));
	context.ah = 8;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void deleverything(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kMapysize);
	context.ah = 0;
	context._add(context.ax, context.data.word(kMapoffsety));
	context._cmp(context.ax, 182);
	if (!context.flags.c()) goto bigroom;
	maptopanel(context);
	{assert(stack_depth == context.stack.size()); return; }
bigroom:
	context._sub(context.data.byte(kMapysize), 8);
	maptopanel(context);
	context._add(context.data.byte(kMapysize), 8);
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpeverything(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40));
dumpevery1:
	context.ax = context.es.word(context.bx);
	context.cx = context.es.word(context.bx+2);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto finishevery1;
	context._cmp(context.ax, context.es.word(context.bx+(40*5)));
	if (!context.flags.z()) goto notskip1;
	context._cmp(context.cx, context.es.word(context.bx+(40*5)+2));
	if (context.flags.z()) goto skip1;
notskip1:
	context.push(context.bx);
	context.push(context.es);
	context.push(context.ds);
	context.bl = context.ah;
	context.bh = 0;
	context.ah = 0;
	context.di = context.ax;
	context._add(context.di, context.data.word(kMapadx));
	context._add(context.bx, context.data.word(kMapady));
	multidump(context);
	context.ds = context.pop();
	context.es = context.pop();
	context.bx = context.pop();
skip1:
	context._add(context.bx, 5);
	goto dumpevery1;
finishevery1:
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40))+(40*5);
dumpevery2:
	context.ax = context.es.word(context.bx);
	context.cx = context.es.word(context.bx+2);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto finishevery2;
	context.push(context.bx);
	context.push(context.es);
	context.push(context.ds);
	context.bl = context.ah;
	context.bh = 0;
	context.ah = 0;
	context.di = context.ax;
	context._add(context.di, context.data.word(kMapadx));
	context._add(context.bx, context.data.word(kMapady));
	multidump(context);
	context.ds = context.pop();
	context.es = context.pop();
	context.bx = context.pop();
	context._add(context.bx, 5);
	goto dumpevery2;
finishevery2:
	{assert(stack_depth == context.stack.size()); return; }
}

void allocatework(Context & context) {
	uint stack_depth = context.stack.size();
	context.bx = 0x1000;
	allocatemem(context);
	context.data.word(kWorkspace) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void readabyte(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void loadpalfromiff(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 2481;
	openfile(context);
	context.cx = 2000;
	context.ds = context.data.word(kMapstore);
	context.dx = 0;
	readfromfile(context);
	closefile(context);
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768);
	context.ds = context.data.word(kMapstore);
	context.si = 0x30;
	context.cx = 768;
palloop:
	context._lodsb();
	context._shr(context.al, 1);
	context._shr(context.al, 1);
	context._cmp(context.data.byte(kBrightness), 1);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void paneltomap(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = context.data.word(kMapxstart);
	context._add(context.di, context.data.word(kMapadx));
	context.bx = context.data.word(kMapystart);
	context._add(context.bx, context.data.word(kMapady));
	context.ds = context.data.word(kMapstore);
	context.si = 0;
	context.cl = context.data.byte(kMapxsize);
	context.ch = context.data.byte(kMapysize);
	multiget(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void maptopanel(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = context.data.word(kMapxstart);
	context._add(context.di, context.data.word(kMapadx));
	context.bx = context.data.word(kMapystart);
	context._add(context.bx, context.data.word(kMapady));
	context.ds = context.data.word(kMapstore);
	context.si = 0;
	context.cl = context.data.byte(kMapxsize);
	context.ch = context.data.byte(kMapysize);
	multiput(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpmap(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = context.data.word(kMapxstart);
	context._add(context.di, context.data.word(kMapadx));
	context.bx = context.data.word(kMapystart);
	context._add(context.bx, context.data.word(kMapady));
	context.cl = context.data.byte(kMapxsize);
	context.ch = context.data.byte(kMapysize);
	multidump(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void pixelcheckset(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context._sub(context.al, context.es.byte(context.bx));
	context._sub(context.ah, context.es.byte(context.bx+1));
	context.push(context.es);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.ax);
	context.al = context.es.byte(context.bx+4);
	getsetad(context);
	context.al = context.es.byte(context.bx+17);
	context.es = context.data.word(kSetframes);
	context.bx = (0);
	context.ah = 0;
	context.cx = 6;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.ax = context.pop();
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context.cl = context.es.byte(context.bx);
	context.ch = 0;
	context._mul(context.cx);
	context.cx = context.pop();
	context.ch = 0;
	context._add(context.ax, context.cx);
	context._add(context.ax, context.es.word(context.bx+2));
	context.bx = context.ax;
	context._add(context.bx, (0+2080));
	context.al = context.es.byte(context.bx);
	context.dl = context.al;
	context.cx = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	context.ax = context.pop();
	context._cmp(context.dl, 0);
	{assert(stack_depth == context.stack.size()); return; }
}

void createpanel(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 0;
	context.bx = 8;
	context.ds = context.data.word(kIcons2);
	context.al = 0;
	context.ah = 2;
	showframe(context);
	context.di = 160;
	context.bx = 8;
	context.ds = context.data.word(kIcons2);
	context.al = 0;
	context.ah = 2;
	showframe(context);
	context.di = 0;
	context.bx = 104;
	context.ds = context.data.word(kIcons2);
	context.al = 0;
	context.ah = 2;
	showframe(context);
	context.di = 160;
	context.bx = 104;
	context.ds = context.data.word(kIcons2);
	context.al = 0;
	context.ah = 2;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void createpanel2(Context & context) {
	uint stack_depth = context.stack.size();
	createpanel(context);
	context.di = 0;
	context.bx = 0;
	context.ds = context.data.word(kIcons2);
	context.al = 5;
	context.ah = 2;
	showframe(context);
	context.di = 160;
	context.bx = 0;
	context.ds = context.data.word(kIcons2);
	context.al = 5;
	context.ah = 2;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void clearwork(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = 0x0;
	context.es = context.data.word(kWorkspace);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void zoom(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) goto inwatching;
	context._cmp(context.data.byte(kZoomon), 1);
	if (context.flags.z()) goto zoomswitch;
inwatching:
	{assert(stack_depth == context.stack.size()); return; }
zoomswitch:
	context._cmp(context.data.byte(kCommandtype), 199);
	if (context.flags.c()) goto zoomit;
cantzoom:
	putunderzoom(context);
	{assert(stack_depth == context.stack.size()); return; }
zoomit:
	context.ax = context.data.word(kOldpointery);
	context._sub(context.ax, 9);
	context.cx = (320);
	context._mul(context.cx);
	context._add(context.ax, context.data.word(kOldpointerx));
	context._sub(context.ax, 11);
	context.si = context.ax;
	context.ax = (132)+4;
	context.cx = (320);
	context._mul(context.cx);
	context._add(context.ax, (8)+5);
	context.di = context.ax;
	context.es = context.data.word(kWorkspace);
	context.ds = context.data.word(kWorkspace);
	context.cx = 20;
zoomloop:
	context.push(context.cx);
	context.cx = 23;
zoomloop2:
	context._lodsb();
	context.ah = context.al;
	context._stosw();
	context.es.word(context.di+(320)-2) = context.ax;
	if (--context.cx) goto zoomloop2;
	context._add(context.si, (320)-23);
	context._add(context.di, (320)-46+(320));
	context.cx = context.pop();
	if (--context.cx) goto zoomloop;
	crosshair(context);
	context.data.byte(kDidzoom) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void delthisone(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context._add(context.ax, context.data.word(kMapady));
	context.bx = (320);
	context._mul(context.bx);
	context.bx = context.pop();
	context.bh = 0;
	context._add(context.bx, context.data.word(kMapadx));
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
	context.es = context.data.word(kWorkspace);
	context.ds = context.data.word(kMapstore);
	context.dl = context.cl;
	context.dh = 0;
	context.ax = (320);
	context._sub(context.ax, context.dx);
	context._neg(context.dx);
	context._add(context.dx, 22*8);
deloneloop:
	context.push(context.cx);
	context.ch = 0;
	while(context.cx--) 	context._movsb();
 	context.cx = context.pop();
	context._add(context.di, context.ax);
	context._add(context.si, context.dx);
	context._dec(context.ch);
	if (!context.flags.z()) goto deloneloop;
	{assert(stack_depth == context.stack.size()); return; }
}

void width160(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void doblocks(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kWorkspace);
	context.ax = context.data.word(kMapady);
	context.cx = (320);
	context._mul(context.cx);
	context.di = context.data.word(kMapadx);
	context._add(context.di, context.ax);
	context.al = context.data.byte(kMapy);
	context.ah = 0;
	context.bx = (66);
	context._mul(context.bx);
	context.bl = context.data.byte(kMapx);
	context.bh = 0;
	context._add(context.ax, context.bx);
	context.si = (0);
	context._add(context.si, context.ax);
	context.cx = 10;
loop120:
	context.push(context.di);
	context.push(context.cx);
	context.cx = 11;
loop124:
	context.push(context.cx);
	context.push(context.di);
	context.ds = context.data.word(kMapdata);
	context._lodsb();
	context.ds = context.data.word(kBackdrop);
	context.push(context.si);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto zeroblock;
	context.ah = context.al;
	context.al = 0;
	context.si = (0+192);
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
 	context._add(context.di, (320)-16);
	context._dec(context.bh);
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
	context._add(context.di, (320)-20);
	context._dec(context.bh);
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
	context._add(context.di, (320)-16);
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._add(context.di, (320)-16);
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._stosw();
	context._add(context.di, (320)-16);
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
	context._add(context.si, (66)-11);
	context.cx = context.pop();
	context.di = context.pop();
	context._add(context.di, (320)*16);
	if (--context.cx) goto loop120;
	{assert(stack_depth == context.stack.size()); return; }
}

void showframe(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.dx);
	context.push(context.ax);
	context.cx = context.ax;
	context._and(context.cx, 511);
	context._add(context.cx, context.cx);
	context.si = context.cx;
	context._add(context.cx, context.cx);
	context._add(context.si, context.cx);
	context._cmp(context.ds.word(context.si), 0);
	if (!context.flags.z()) goto notblankshow;
	context.ax = context.pop();
	context.dx = context.pop();
	context.cx = 0;
	{assert(stack_depth == context.stack.size()); return; }
notblankshow:
	context._test(context.ah, 128);
	if (!context.flags.z()) goto skipoffsets;
	context.al = context.ds.byte(context.si+4);
	context.ah = 0;
	context._add(context.di, context.ax);
	context.al = context.ds.byte(context.si+5);
	context.ah = 0;
	context._add(context.bx, context.ax);
skipoffsets:
	context.cx = context.ds.word(context.si+0);
	context.ax = context.ds.word(context.si+2);
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
	{assert(stack_depth == context.stack.size()); return; }
notdiffdest:
	context._test(context.ah, 8);
	if (context.flags.z()) goto notprintlist;
	context.push(context.ax);
	context.ax = context.di;
	context._sub(context.ax, context.data.word(kMapadx));
	context.push(context.bx);
	context._sub(context.bx, context.data.word(kMapady));
	context.ah = context.bl;
	context.bx = context.pop();
	context.ax = context.pop();
notprintlist:
	context._test(context.ah, 4);
	if (context.flags.z()) goto notflippedx;
	context.dx = (320);
	context.es = context.data.word(kWorkspace);
	context.push(context.cx);
	frameoutfx(context);
	context.cx = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
notflippedx:
	context._test(context.ah, 2);
	if (context.flags.z()) goto notnomask;
	context.dx = (320);
	context.es = context.data.word(kWorkspace);
	context.push(context.cx);
	frameoutnm(context);
	context.cx = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
notnomask:
	context._test(context.ah, 32);
	if (context.flags.z()) goto noeffects;
	context.dx = (320);
	context.es = context.data.word(kWorkspace);
	context.push(context.cx);
	frameoutbh(context);
	context.cx = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
noeffects:
	context.dx = (320);
	context.es = context.data.word(kWorkspace);
	context.push(context.cx);
	frameoutv(context);
	context.cx = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void frameoutv(Context & context) {
	uint stack_depth = context.stack.size();
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
	context._inc(context.di);
	if (--context.cx) goto frameloop2;
	context.cx = context.pop();
	context._add(context.di, context.dx);
	context._dec(context.ch);
	if (!context.flags.z()) goto frameloop1;
	{assert(stack_depth == context.stack.size()); return; }
frameloop3:
	context._lodsb();
	context._cmp(context.al, 0);
	if (context.flags.z()) goto backtoother;
backtosolid:
	context._stosb();
	if (--context.cx) goto frameloop3;
	context.cx = context.pop();
	context._add(context.di, context.dx);
	context._dec(context.ch);
	if (!context.flags.z()) goto frameloop1;
	{assert(stack_depth == context.stack.size()); return; }
}

void frameoutbh(Context & context) {
	uint stack_depth = context.stack.size();
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
	context._cmp(context.es.byte(context.di), context.ah);
	if (!context.flags.z()) goto nofill;
	context._movsb();
 	if (--context.cx) goto bhloop1;
	goto nextline;
nofill:
	context._inc(context.di);
	context._inc(context.si);
	if (--context.cx) goto bhloop1;
nextline:
	context._add(context.di, context.dx);
	context.cx = context.pop();
	context._dec(context.ch);
	if (!context.flags.z()) goto bhloop2;
	{assert(stack_depth == context.stack.size()); return; }
}

void frameoutfx(Context & context) {
	uint stack_depth = context.stack.size();
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
	context._dec(context.di);
	if (--context.cx) goto frameloopfx2;
	context.cx = context.pop();
	context._add(context.di, context.dx);
	context._dec(context.ch);
	if (!context.flags.z()) goto frameloopfx1;
	{assert(stack_depth == context.stack.size()); return; }
frameloopfx3:
	context._lodsb();
	context._cmp(context.al, 0);
	if (context.flags.z()) goto backtootherfx;
backtosolidfx:
	context.es.byte(context.di) = context.al;
	context._dec(context.di);
	if (--context.cx) goto frameloopfx3;
	context.cx = context.pop();
	context._add(context.di, context.dx);
	context._dec(context.ch);
	if (!context.flags.z()) goto frameloopfx1;
	{assert(stack_depth == context.stack.size()); return; }
}

void transferinv(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = context.data.word(kExframepos);
	context.push(context.di);
	context.al = context.data.byte(kExpos);
	context.ah = 0;
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.bx);
	context._inc(context.ax);
	context.cx = 6;
	context._mul(context.cx);
	context.es = context.data.word(kExtras);
	context.bx = (0);
	context._add(context.bx, context.ax);
	context._add(context.di, (0+2080));
	context.push(context.bx);
	context.al = context.data.byte(kItemtotran);
	context.ah = 0;
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.bx);
	context._inc(context.ax);
	context.cx = 6;
	context._mul(context.cx);
	context.ds = context.data.word(kFreeframes);
	context.bx = (0);
	context._add(context.bx, context.ax);
	context.si = (0+2080);
	context.al = context.ds.byte(context.bx);
	context.ah = 0;
	context.cl = context.ds.byte(context.bx+1);
	context.ch = 0;
	context._add(context.si, context.ds.word(context.bx+2));
	context.dx = context.ds.word(context.bx+4);
	context.bx = context.pop();
	context.es.byte(context.bx+0) = context.al;
	context.es.byte(context.bx+1) = context.cl;
	context.es.word(context.bx+4) = context.dx;
	context._mul(context.cx);
	context.cx = context.ax;
	context.push(context.cx);
	while(context.cx--) 	context._movsb();
 	context.cx = context.pop();
	context.ax = context.pop();
	context.es.word(context.bx+2) = context.ax;
	context._add(context.data.word(kExframepos), context.cx);
	{assert(stack_depth == context.stack.size()); return; }
}

void transfermap(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = context.data.word(kExframepos);
	context.push(context.di);
	context.al = context.data.byte(kExpos);
	context.ah = 0;
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.bx);
	context.cx = 6;
	context._mul(context.cx);
	context.es = context.data.word(kExtras);
	context.bx = (0);
	context._add(context.bx, context.ax);
	context._add(context.di, (0+2080));
	context.push(context.bx);
	context.al = context.data.byte(kItemtotran);
	context.ah = 0;
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.bx);
	context.cx = 6;
	context._mul(context.cx);
	context.ds = context.data.word(kFreeframes);
	context.bx = (0);
	context._add(context.bx, context.ax);
	context.si = (0+2080);
	context.al = context.ds.byte(context.bx);
	context.ah = 0;
	context.cl = context.ds.byte(context.bx+1);
	context.ch = 0;
	context._add(context.si, context.ds.word(context.bx+2));
	context.dx = context.ds.word(context.bx+4);
	context.bx = context.pop();
	context.es.byte(context.bx+0) = context.al;
	context.es.byte(context.bx+1) = context.cl;
	context.es.word(context.bx+4) = context.dx;
	context._mul(context.cx);
	context.cx = context.ax;
	context.push(context.cx);
	while(context.cx--) 	context._movsb();
 	context.cx = context.pop();
	context.ax = context.pop();
	context.es.word(context.bx+2) = context.ax;
	context._add(context.data.word(kExframepos), context.cx);
	{assert(stack_depth == context.stack.size()); return; }
}

void dofade(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kFadedirection), 0);
	if (context.flags.z()) goto finishfade;
	context.cl = context.data.byte(kNumtofade);
	context.ch = 0;
	context.al = context.data.byte(kColourpos);
	context.ah = 0;
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3));
	context._add(context.si, context.ax);
	context._add(context.si, context.ax);
	context._add(context.si, context.ax);
	showgroup(context);
	context.al = context.data.byte(kNumtofade);
	context._add(context.al, context.data.byte(kColourpos));
	context.data.byte(kColourpos) = context.al;
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto finishfade;
	fadecalculation(context);
finishfade:
	{assert(stack_depth == context.stack.size()); return; }
}

void clearendpal(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768);
	context.cx = 768;
	context.al = 0;
	while(context.cx--) 	context._stosb();
	{assert(stack_depth == context.stack.size()); return; }
}

void clearpalette(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kFadedirection) = 0;
	clearstartpal(context);
	dumpcurrent(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void fadescreenup(Context & context) {
	uint stack_depth = context.stack.size();
	clearstartpal(context);
	paltoendpal(context);
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 128;
	{assert(stack_depth == context.stack.size()); return; }
}

void fadetowhite(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768);
	context.cx = 768;
	context.al = 63;
	while(context.cx--) 	context._stosb();
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768);
	context.al = 0;
	context._stosb();
	context._stosb();
	context._stosb();
	paltostartpal(context);
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 128;
	{assert(stack_depth == context.stack.size()); return; }
}

void fadefromwhite(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3));
	context.cx = 768;
	context.al = 63;
	while(context.cx--) 	context._stosb();
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3));
	context.al = 0;
	context._stosb();
	context._stosb();
	context._stosb();
	paltoendpal(context);
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 128;
	{assert(stack_depth == context.stack.size()); return; }
}

void fadescreenups(Context & context) {
	uint stack_depth = context.stack.size();
	clearstartpal(context);
	paltoendpal(context);
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 64;
	{assert(stack_depth == context.stack.size()); return; }
}

void fadescreendownhalf(Context & context) {
	uint stack_depth = context.stack.size();
	paltostartpal(context);
	paltoendpal(context);
	context.cx = 768;
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768);
halfend:
	context.al = context.es.byte(context.bx);
	context._shr(context.al, 1);
	context.es.byte(context.bx) = context.al;
	context._inc(context.bx);
	if (--context.cx) goto halfend;
	context.ds = context.data.word(kBuffers);
	context.es = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3))+(56*3);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768)+(56*3);
	context.cx = 3*5;
	while(context.cx--) 	context._movsb();
 	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3))+(77*3);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768)+(77*3);
	context.cx = 3*2;
	while(context.cx--) 	context._movsb();
 	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 31;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 32;
	{assert(stack_depth == context.stack.size()); return; }
}

void fadescreenuphalf(Context & context) {
	uint stack_depth = context.stack.size();
	endpaltostart(context);
	paltoendpal(context);
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 31;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 32;
	{assert(stack_depth == context.stack.size()); return; }
}

void fadescreendown(Context & context) {
	uint stack_depth = context.stack.size();
	paltostartpal(context);
	clearendpal(context);
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 128;
	{assert(stack_depth == context.stack.size()); return; }
}

void fadescreendowns(Context & context) {
	uint stack_depth = context.stack.size();
	paltostartpal(context);
	clearendpal(context);
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 64;
	{assert(stack_depth == context.stack.size()); return; }
}

void clearstartpal(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3));
	context.cx = 256;
wholeloop1:
	context.ax = 0;
	context._stosw();
	context.al = 0;
	context._stosb();
	if (--context.cx) goto wholeloop1;
	{assert(stack_depth == context.stack.size()); return; }
}

void showgun(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kAddtored) = 0;
	context.data.byte(kAddtogreen) = 0;
	context.data.byte(kAddtoblue) = 0;
	paltostartpal(context);
	paltoendpal(context);
	greyscalesum(context);
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 128;
	context.cx = 130;
	hangon(context);
	endpaltostart(context);
	clearendpal(context);
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 128;
	context.cx = 200;
	hangon(context);
	context.data.byte(kRoomssample) = 34;
	loadroomssample(context);
	context.data.byte(kVolume) = 0;
	context.dx = 2351;
	loadintotemp(context);
	createpanel2(context);
	context.ds = context.data.word(kTempgraphics);
	context.al = 0;
	context.ah = 0;
	context.di = 100;
	context.bx = 4;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
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
	context.dx = 2260;
	loadtemptext(context);
	rollendcredits2(context);
	getridoftemptext(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void rollendcredits2(Context & context) {
	uint stack_depth = context.stack.size();
	rollem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void rollem(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 160;
	context.ch = 160;
	context.di = 25;
	context.bx = 20;
	context.ds = context.data.word(kMapstore);
	context.si = 0;
	multiget(context);
	context.es = context.data.word(kTextfile1);
	context.si = 49*2;
	context.ax = context.es.word(context.si);
	context.si = context.ax;
	context._add(context.si, (66*2));
	context.cx = 80;
endcredits21:
	context.push(context.cx);
	context.bx = 10;
	context.cx = context.data.word(kLinespacing);
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
	context.ds = context.data.word(kMapstore);
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
	context._add(context.bx, context.data.word(kLinespacing));
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
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto endearly2;
	context._dec(context.bx);
	if (--context.cx) goto endcredits22;
	context.cx = context.pop();
looknext2:
	context.al = context.es.byte(context.si);
	context._inc(context.si);
	context._cmp(context.al, ':');
	if (context.flags.z()) goto gotnext2;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto gotnext2;
	goto looknext2;
gotnext2:
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto endearly;
	if (--context.cx) goto endcredits21;
	context.cx = 120;
	hangone(context);
	{assert(stack_depth == context.stack.size()); return; }
endearly2:
	context.cx = context.pop();
endearly:
	{assert(stack_depth == context.stack.size()); return; }
}

void fadecalculation(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kFadecount), 0);
	if (context.flags.z()) goto nomorefading;
	context.bl = context.data.byte(kFadecount);
	context.es = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3));
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768);
	context.cx = 768;
fadecolloop:
	context.al = context.es.byte(context.si);
	context.ah = context.es.byte(context.di);
	context._cmp(context.al, context.ah);
	if (context.flags.z()) goto gotthere;
	if (context.flags.c()) goto lesscolour;
	context._dec(context.es.byte(context.si));
	goto gotthere;
lesscolour:
	context._cmp(context.bl, context.ah);
	if (context.flags.z()) goto withit;
	if (!context.flags.c()) goto gotthere;
withit:
	context._inc(context.es.byte(context.si));
gotthere:
	context._inc(context.si);
	context._inc(context.di);
	if (--context.cx) goto fadecolloop;
	context._dec(context.data.byte(kFadecount));
	{assert(stack_depth == context.stack.size()); return; }
nomorefading:
	context.data.byte(kFadedirection) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void greyscalesum(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768);
	context.cx = 256;
greysumloop1:
	context.push(context.cx);
	context.bx = 0;
	context.al = context.es.byte(context.si);
	context.ah = 0;
	context.cx = 20;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.al = context.es.byte(context.si+1);
	context.ah = 0;
	context.cx = 59;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.al = context.es.byte(context.si+2);
	context.ah = 0;
	context.cx = 11;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.al = -1;
greysumloop2:
	context._inc(context.al);
	context._sub(context.bx, 100);
	if (!context.flags.c()) goto greysumloop2;
	context.bl = context.al;
	context.al = context.bl;
	context.ah = context.data.byte(kAddtored);
	context._cmp(context.al, 0);
	context._add(context.al, context.ah);
noaddr:
	context._stosb();
	context.ah = context.data.byte(kAddtogreen);
	context.al = context.bl;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto noaddg;
	context._add(context.al, context.ah);
noaddg:
	context._stosb();
	context.ah = context.data.byte(kAddtoblue);
	context.al = context.bl;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto noaddb;
	context._add(context.al, context.ah);
noaddb:
	context._stosb();
	context._add(context.si, 3);
	context.cx = context.pop();
	if (--context.cx) goto greysumloop1;
	{assert(stack_depth == context.stack.size()); return; }
}

void paltostartpal(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3));
	context.cx = 768/2;
	while(context.cx--) 	context._movsw();
 	{assert(stack_depth == context.stack.size()); return; }
}

void endpaltostart(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3));
	context.cx = 768/2;
	while(context.cx--) 	context._movsw();
 	{assert(stack_depth == context.stack.size()); return; }
}

void startpaltoend(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.ds = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3));
	context.cx = 768/2;
	while(context.cx--) 	context._movsw();
 	{assert(stack_depth == context.stack.size()); return; }
}

void paltoendpal(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.ds = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768);
	context.cx = 768/2;
	while(context.cx--) 	context._movsw();
 	{assert(stack_depth == context.stack.size()); return; }
}

void allpalette(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.ds = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3));
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768);
	context.cx = 768/2;
	while(context.cx--) 	context._movsw();
 	dumpcurrent(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpcurrent(Context & context) {
	uint stack_depth = context.stack.size();
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3));
	context.ds = context.data.word(kBuffers);
	vsync(context);
	context.al = 0;
	context.cx = 128;
	showgroup(context);
	vsync(context);
	context.al = 128;
	context.cx = 128;
	showgroup(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void fadedownmon(Context & context) {
	uint stack_depth = context.stack.size();
	paltostartpal(context);
	paltoendpal(context);
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768)+(231*3);
	context.cx = 3*8;
	context.ax = 0;
	while(context.cx--) 	context._stosb();
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768)+(246*3);
	context._stosb();
	context._stosw();
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 128;
	context.cx = 64;
	hangon(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void fadeupmon(Context & context) {
	uint stack_depth = context.stack.size();
	paltostartpal(context);
	paltoendpal(context);
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3))+(231*3);
	context.cx = 3*8;
	context.ax = 0;
	while(context.cx--) 	context._stosb();
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3))+(246*3);
	context._stosb();
	context._stosw();
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 128;
	context.cx = 128;
	hangon(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void fadeupmonfirst(Context & context) {
	uint stack_depth = context.stack.size();
	paltostartpal(context);
	paltoendpal(context);
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3))+(231*3);
	context.cx = 3*8;
	context.ax = 0;
	while(context.cx--) 	context._stosb();
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3))+(246*3);
	context._stosb();
	context._stosw();
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 128;
	context.cx = 64;
	hangon(context);
	context.al = 26;
	playchannel1(context);
	context.cx = 64;
	hangon(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void fadeupyellows(Context & context) {
	uint stack_depth = context.stack.size();
	paltoendpal(context);
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768)+(231*3);
	context.cx = 3*8;
	context.ax = 0;
	while(context.cx--) 	context._stosb();
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768)+(246*3);
	context._stosb();
	context._stosw();
	context.data.byte(kFadedirection) = 1;
	context.data.byte(kFadecount) = 63;
	context.data.byte(kColourpos) = 0;
	context.data.byte(kNumtofade) = 128;
	context.cx = 128;
	hangon(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void initialmoncols(Context & context) {
	uint stack_depth = context.stack.size();
	paltostartpal(context);
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3))+(230*3);
	context.cx = 3*9;
	context.ax = 0;
	while(context.cx--) 	context._stosb();
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3))+(246*3);
	context._stosb();
	context._stosw();
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3))+(230*3);
	context.al = 230;
	context.cx = 18;
	showgroup(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void titles(Context & context) {
	uint stack_depth = context.stack.size();
	clearpalette(context);
	biblequote(context);
	intro(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void endgame(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 2260;
	loadtemptext(context);
	monkspeaking(context);
	gettingshot(context);
	getridoftemptext(context);
	context.data.byte(kVolumeto) = 7;
	context.data.byte(kVolumedirection) = 1;
	context.cx = 200;
	hangon(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void monkspeaking(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kRoomssample) = 35;
	loadroomssample(context);
	context.dx = 2364;
	loadintotemp(context);
	clearwork(context);
	showmonk(context);
	worktoscreen(context);
	context.data.byte(kVolume) = 7;
	context.data.byte(kVolumedirection) = -1;
	context.data.byte(kVolumeto) = 0;
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
	context.es = context.data.word(kTextfile1);
	context.ax = context.es.word(context.si);
	context._add(context.ax, (66*2));
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
	context._inc(context.al);
	context._cmp(context.al, 44);
	if (!context.flags.z()) goto nextmonkspeak;
	context.data.byte(kVolumedirection) = 1;
	context.data.byte(kVolumeto) = 7;
	fadescreendowns(context);
	context.cx = 300;
	hangon(context);
	getridoftemp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showmonk(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 0;
	context.ah = 128;
	context.di = 160;
	context.bx = 72;
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void gettingshot(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kNewlocation) = 55;
	clearpalette(context);
	loadintroroom(context);
	fadescreenups(context);
	context.data.byte(kVolumeto) = 0;
	context.data.byte(kVolumedirection) = -1;
	runendseq(context);
	clearbeforeload(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void credits(Context & context) {
	uint stack_depth = context.stack.size();
	clearpalette(context);
	realcredits(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void biblequote(Context & context) {
	uint stack_depth = context.stack.size();
	mode640x480(context);
	context.dx = 2377;
	showpcx(context);
	fadescreenups(context);
	context.cx = 80;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto biblequotearly;
	context.cx = 560;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto biblequotearly;
	fadescreendowns(context);
	context.cx = 200;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto biblequotearly;
	cancelch0(context);
biblequotearly:
	{assert(stack_depth == context.stack.size()); return; }
}

void hangone(Context & context) {
	uint stack_depth = context.stack.size();
hangonloope:
	context.push(context.cx);
	vsync(context);
	context.cx = context.pop();
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto hangonearly;
	if (--context.cx) goto hangonloope;
hangonearly:
	{assert(stack_depth == context.stack.size()); return; }
}

void intro(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 2247;
	loadtemptext(context);
	loadpalfromiff(context);
	setmode(context);
	context.data.byte(kNewlocation) = 50;
	clearpalette(context);
	loadintroroom(context);
	context.data.byte(kVolume) = 7;
	context.data.byte(kVolumedirection) = -1;
	context.data.byte(kVolumeto) = 0;
	context.al = 12;
	context.ah = 255;
	playchannel0(context);
	fadescreenups(context);
	runintroseq(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto introearly;
	clearbeforeload(context);
	context.data.byte(kNewlocation) = 52;
	loadintroroom(context);
	runintroseq(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto introearly;
	clearbeforeload(context);
	context.data.byte(kNewlocation) = 53;
	loadintroroom(context);
	runintroseq(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto introearly;
	clearbeforeload(context);
	allpalette(context);
	context.data.byte(kNewlocation) = 54;
	loadintroroom(context);
	runintroseq(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto introearly;
	getridoftemptext(context);
	clearbeforeload(context);
introearly:
	{assert(stack_depth == context.stack.size()); return; }
}

void runintroseq(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kGetback) = 0;
moreintroseq:
	vsync(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto earlyendrun;
	spriteupdate(context);
	vsync(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto earlyendrun;
	deleverything(context);
	printsprites(context);
	reelsonscreen(context);
	afterintroroom(context);
	usetimedtext(context);
	vsync(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto earlyendrun;
	dumpmap(context);
	dumptimedtext(context);
	vsync(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto earlyendrun;
	context._cmp(context.data.byte(kGetback), 1);
	if (!context.flags.z()) goto moreintroseq;
	{assert(stack_depth == context.stack.size()); return; }
earlyendrun:
	getridoftemptext(context);
	clearbeforeload(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void runendseq(Context & context) {
	uint stack_depth = context.stack.size();
	atmospheres(context);
	context.data.byte(kGetback) = 0;
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
	context._cmp(context.data.byte(kGetback), 1);
	if (!context.flags.z()) goto moreendseq;
	{assert(stack_depth == context.stack.size()); return; }
}

void loadintroroom(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kIntrocount) = 0;
	context.data.byte(kLocation) = 255;
	loadroom(context);
	context.data.word(kMapoffsetx) = 72;
	context.data.word(kMapoffsety) = 16;
	clearsprites(context);
	context.data.byte(kThroughdoor) = 0;
	context.data.byte(kCurrentkey) = '0';
	context.data.byte(kMainmode) = 0;
	clearwork(context);
	context.data.byte(kNewobs) = 1;
	drawfloor(context);
	reelsonscreen(context);
	spriteupdate(context);
	printsprites(context);
	worktoscreen(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void realcredits(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kRoomssample) = 33;
	loadroomssample(context);
	context.data.byte(kVolume) = 0;
	mode640x480(context);
	context.cx = 35;
	hangon(context);
	context.dx = 2390;
	showpcx(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.cx = 2;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	allpalette(context);
	context.cx = 80;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	context.dx = 2403;
	showpcx(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.cx = 2;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	allpalette(context);
	context.cx = 80;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	context.dx = 2416;
	showpcx(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.cx = 2;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	allpalette(context);
	context.cx = 80;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	context.dx = 2429;
	showpcx(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.cx = 2;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	allpalette(context);
	context.cx = 80;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	context.dx = 2442;
	showpcx(context);
	context.al = 12;
	context.ah = 0;
	playchannel0(context);
	context.cx = 2;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	allpalette(context);
	context.cx = 80;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	context.dx = 2455;
	showpcx(context);
	fadescreenups(context);
	context.cx = 60;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	context.al = 13;
	context.ah = 0;
	playchannel0(context);
	context.cx = 350;
	hangone(context);
	context._cmp(context.data.byte(kLasthardkey), 1);
	if (context.flags.z()) goto realcreditsearly;
	fadescreendowns(context);
	context.cx = 256;
	hangone(context);
realcreditsearly:
	{assert(stack_depth == context.stack.size()); return; }
}

void printchar(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.al, 255);
	if (context.flags.z()) goto ignoreit;
	context.push(context.si);
	context.push(context.bx);
	context.push(context.di);
	context.push(context.ax);
	context._sub(context.al, 32);
	context.ah = 0;
	context._add(context.ax, context.data.word(kCharshift));
	showframe(context);
	context.ax = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.si = context.pop();
	context._cmp(context.data.byte(kKerning), 0);
	if (!context.flags.z()) goto nokern;
	kernchars(context);
nokern:
	context.push(context.cx);
	context.ch = 0;
	context._add(context.di, context.cx);
	context.cx = context.pop();
ignoreit:
	{assert(stack_depth == context.stack.size()); return; }
}

void kernchars(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.al, 'a');
	if (context.flags.z()) goto iskern;
	context._cmp(context.al, 'u');
	if (context.flags.z()) goto iskern;
	{assert(stack_depth == context.stack.size()); return; }
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
	{assert(stack_depth == context.stack.size()); return; }
kernit:
	context._dec(context.cl);
	{assert(stack_depth == context.stack.size()); return; }
}

void printslow(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kPointerframe) = 1;
	context.data.byte(kPointermode) = 3;
	context.ds = context.data.word(kCharset1);
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
	context.ax = context.es.word(context.si);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.es);
	context.push(context.si);
	context.push(context.ds);
	printboth(context);
	context.ds = context.pop();
	context.si = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ax = context.es.word(context.si+1);
	context._inc(context.si);
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
	context.data.word(kCharshift) = 91;
	printboth(context);
	context.data.word(kCharshift) = 0;
	context.si = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	waitframes(context);
	context._cmp(context.ax, 0);
	if (context.flags.z()) goto keepgoing;
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (!context.flags.z()) goto finishslow2;
keepgoing:
	waitframes(context);
noslow:
	context._cmp(context.ax, 0);
	if (context.flags.z()) goto afterslow;
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (!context.flags.z()) goto finishslow2;
afterslow:
	context.es = context.pop();
	context.si = context.pop();
	context.cx = context.pop();
	context._inc(context.si);
	if (--context.cx) goto printloopslow5;
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context._add(context.bx, 10);
	goto printloopslow6;
finishslow:
	context.es = context.pop();
	context.si = context.pop();
	context.cx = context.pop();
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
finishslow2:
	context.es = context.pop();
	context.si = context.pop();
	context.cx = context.pop();
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.al = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void waitframes(Context & context) {
	uint stack_depth = context.stack.size();
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
	context.ax = context.data.word(kMousebutton);
	context.ds = context.pop();
	context.si = context.pop();
	context.es = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void printboth(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void printdirect(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.word(kLastxpos) = context.di;
	context.ds = context.data.word(kCurrentset);
printloop6:
	context.push(context.bx);
	context.push(context.di);
	context.push(context.dx);
	getnumber(context);
	context.ch = 0;
printloop5:
	context.ax = context.es.word(context.si);
	context._inc(context.si);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto finishdirct;
	context._cmp(context.al, ':');
	if (context.flags.z()) goto finishdirct;
	context.push(context.cx);
	context.push(context.es);
	printchar(context);
	context.data.word(kLastxpos) = context.di;
	context.es = context.pop();
	context.cx = context.pop();
	if (--context.cx) goto printloop5;
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context._add(context.bx, context.data.word(kLinespacing));
	goto printloop6;
finishdirct:
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void monprint(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kKerning) = 1;
	context.si = context.bx;
	context.dl = 166;
	context.di = context.data.word(kMonadx);
	context.bx = context.data.word(kMonady);
	context.ds = context.data.word(kTempcharset);
printloop8:
	context.push(context.bx);
	context.push(context.di);
	context.push(context.dx);
	getnumber(context);
	context.ch = 0;
printloop7:
	context.al = context.es.byte(context.si);
	context._inc(context.si);
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
	context.ah = context.es.byte(context.si);
	context._inc(context.si);
	context._inc(context.si);
	goto finishmon;
nottrigger:
	context.push(context.cx);
	context.push(context.es);
	printchar(context);
	context.data.word(kCurslocx) = context.di;
	context.data.word(kCurslocy) = context.bx;
	context.data.word(kMaintimer) = 1;
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
	context.data.word(kCurslocx) = context.di;
	goto printloop8;
finishmon:
	context.dx = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context._cmp(context.al, '%');
	if (!context.flags.z()) goto nottrigger2;
	context.data.byte(kLasttrigger) = context.ah;
nottrigger2:
	context.data.word(kCurslocx) = context.di;
	scrollmonitor(context);
	context.bx = context.si;
	context.data.byte(kKerning) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void getnumber(Context & context) {
	uint stack_depth = context.stack.size();
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
	context._and(context.al, 0xfe);
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
	{assert(stack_depth == context.stack.size()); return; }
notcentre:
	context.es = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.si = context.pop();
	context.cl = context.ch;
	{assert(stack_depth == context.stack.size()); return; }
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
	context._and(context.al, 0xfe);
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
	{assert(stack_depth == context.stack.size()); return; }
notcent2:
	context.es = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.si = context.pop();
	context.cl = context.ch;
	{assert(stack_depth == context.stack.size()); return; }
}

void getnextword(Context & context) {
	uint stack_depth = context.stack.size();
	context.bx = 0;
getloop:
	context.ax = context.es.word(context.di);
	context._inc(context.di);
	context._inc(context.bh);
	context._cmp(context.al, ':');
	if (context.flags.z()) goto endall;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto endall;
	context._cmp(context.al, 32);
	if (context.flags.z()) goto endword;
	context._cmp(context.al, 255);
	if (context.flags.z()) goto getloop;
	context.push(context.ax);
	context._sub(context.al, 32);
	context.ah = 0;
	context._add(context.ax, context.data.word(kCharshift));
	context._add(context.ax, context.ax);
	context.si = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.si, context.ax);
	context.cl = context.ds.byte(context.si+0);
	context.ax = context.pop();
	kernchars(context);
	context._add(context.bl, context.cl);
	goto getloop;
endword:
	context._add(context.bl, 6);
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
endall:
	context._add(context.bl, 6);
	context.al = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void fillryan(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32);
	findallryan(context);
	context.si = (0+(180*10)+32);
	context.al = context.data.byte(kRyanpage);
	context.ah = 0;
	context.cx = 20;
	context._mul(context.cx);
	context._add(context.si, context.ax);
	context.di = (80);
	context.bx = (58);
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
	context.ax = context.es.word(context.si);
	context._add(context.si, 2);
	context.push(context.si);
	context.push(context.es);
	obtoinv(context);
	context.es = context.pop();
	context.si = context.pop();
	context.bx = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.di, (44));
	if (--context.cx) goto ryanloop1;
	context.bx = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.bx, (44));
	if (--context.cx) goto ryanloop2;
	showryanpage(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void fillopen(Context & context) {
	uint stack_depth = context.stack.size();
	deltextline(context);
	getopenedsize(context);
	context._cmp(context.ah, 4);
	if (context.flags.c()) goto lessthanapage;
	context.ah = 4;
lessthanapage:
	context.al = 1;
	context.push(context.ax);
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10));
	findallopen(context);
	context.si = (0+(180*10));
	context.di = (80);
	context.bx = (58)+96;
	context.cx = context.pop();
openloop1:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.bx);
	context.ax = context.es.word(context.si);
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
	context._add(context.di, (44));
	context._inc(context.cl);
	context._cmp(context.cl, 5);
	if (!context.flags.z()) goto openloop1;
	undertextline(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void findallryan(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.di);
	context.cx = 30;
	context.ax = 0x0ffff;
	while(context.cx--) 	context._stosw();
	context.di = context.pop();
	context.cl = 4;
	context.ds = context.data.word(kExtras);
	context.bx = (0+2080+30000);
	context.ch = 0;
findryanloop:
	context._cmp(context.ds.byte(context.bx+2), context.cl);
	if (!context.flags.z()) goto notinryaninv;
	context._cmp(context.ds.byte(context.bx+3), 255);
	if (!context.flags.z()) goto notinryaninv;
	context.al = context.ds.byte(context.bx+4);
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
	context._inc(context.ch);
	context._cmp(context.ch, (114));
	if (!context.flags.z()) goto findryanloop;
	{assert(stack_depth == context.stack.size()); return; }
}

void findallopen(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.di);
	context.cx = 16;
	context.ax = 0x0ffff;
	while(context.cx--) 	context._stosw();
	context.di = context.pop();
	context.cl = context.data.byte(kOpenedob);
	context.dl = context.data.byte(kOpenedtype);
	context.ds = context.data.word(kExtras);
	context.bx = (0+2080+30000);
	context.ch = 0;
findopen1:
	context._cmp(context.ds.byte(context.bx+3), context.cl);
	if (!context.flags.z()) goto findopen2;
	context._cmp(context.ds.byte(context.bx+2), context.dl);
	if (!context.flags.z()) goto findopen2;
	context._cmp(context.data.byte(kOpenedtype), 4);
	if (context.flags.z()) goto noloccheck;
	context.al = context.ds.byte(context.bx+5);
	context._cmp(context.al, context.data.byte(kReallocation));
	if (!context.flags.z()) goto findopen2;
noloccheck:
	context.al = context.ds.byte(context.bx+4);
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
	context._inc(context.ch);
	context._cmp(context.ch, (114));
	if (!context.flags.z()) goto findopen1;
	context.cl = context.data.byte(kOpenedob);
	context.dl = context.data.byte(kOpenedtype);
	context.push(context.dx);
	context.ds = context.data.word(kFreedat);
	context.dx = context.pop();
	context.bx = 0;
	context.ch = 0;
findopen1a:
	context._cmp(context.ds.byte(context.bx+3), context.cl);
	if (!context.flags.z()) goto findopen2a;
	context._cmp(context.ds.byte(context.bx+2), context.dl);
	if (!context.flags.z()) goto findopen2a;
	context.al = context.ds.byte(context.bx+4);
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
	context._inc(context.ch);
	context._cmp(context.ch, 80);
	if (!context.flags.z()) goto findopen1a;
	{assert(stack_depth == context.stack.size()); return; }
}

void obtoinv(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.bx);
	context.push(context.es);
	context.push(context.si);
	context.push(context.ax);
	context.push(context.ax);
	context.push(context.di);
	context.push(context.bx);
	context.ds = context.data.word(kIcons1);
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
	context.ds = context.data.word(kExtras);
	context._cmp(context.ah, 4);
	if (context.flags.z()) goto isanextra;
	context.ds = context.data.word(kFreeframes);
isanextra:
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	context._inc(context.al);
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
	context.ds = context.data.word(kIcons1);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void isitworn(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.es.byte(context.bx+12);
	context._cmp(context.al, 'W'-'A');
	if (!context.flags.z()) goto notworn;
	context.al = context.es.byte(context.bx+13);
	context._cmp(context.al, 'E'-'A');
notworn:
	{assert(stack_depth == context.stack.size()); return; }
}

void makeworn(Context & context) {
	uint stack_depth = context.stack.size();
	context.es.byte(context.bx+12) = 'W'-'A';
	context.es.byte(context.bx+13) = 'E'-'A';
	{assert(stack_depth == context.stack.size()); return; }
}

void examineob(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kPointermode) = 0;
	context.data.word(kTimecount) = 0;
examineagain:
	context.data.byte(kInmaparea) = 0;
	context.data.byte(kExamagain) = 0;
	context.data.byte(kOpenedob) = 255;
	context.data.byte(kOpenedtype) = 255;
	context.data.byte(kInvopen) = 0;
	context.al = context.data.byte(kCommandtype);
	context.data.byte(kObjecttype) = context.al;
	context.data.byte(kItemframe) = 0;
	context.data.byte(kPointerframe) = 0;
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	obicons(context);
	obpicture(context);
	describeob(context);
	undertextline(context);
	context.data.byte(kCommandtype) = 255;
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
	context.data.byte(kGetback) = 0;
	context.bx = 2494;
	context._cmp(context.data.byte(kInvopen), 0);
	if (context.flags.z()) goto notuseinv;
	context.bx = 2556;
	context._cmp(context.data.byte(kInvopen), 1);
	if (context.flags.z()) goto notuseinv;
	context.bx = 2618;
notuseinv:
	checkcoords(context);
	context._cmp(context.data.byte(kExamagain), 0);
	if (context.flags.z()) goto norex;
	goto examineagain;
norex:
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto waitexam;
	context.data.byte(kPickup) = 0;
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) goto iswatching;
	context._cmp(context.data.byte(kNewlocation), 255);
	if (!context.flags.z()) goto justgetback;
iswatching:
	makemainscreen(context);
	context.data.byte(kInvopen) = 0;
	context.data.byte(kOpenedob) = 255;
	{assert(stack_depth == context.stack.size()); return; }
justgetback:
	context.data.byte(kInvopen) = 0;
	context.data.byte(kOpenedob) = 255;
	{assert(stack_depth == context.stack.size()); return; }
}

void makemainscreen(Context & context) {
	uint stack_depth = context.stack.size();
	createpanel(context);
	context.data.byte(kNewobs) = 1;
	drawfloor(context);
	spriteupdate(context);
	printsprites(context);
	reelsonscreen(context);
	showicon(context);
	getunderzoom(context);
	undertextline(context);
	context.data.byte(kCommandtype) = 255;
	animpointer(context);
	worktoscreenm(context);
	context.data.byte(kCommandtype) = 200;
	context.data.byte(kManisoffscreen) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void getbackfromob(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kPickup), 1);
	if (!context.flags.z()) goto notheldob;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
notheldob:
	getback1(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void incryanpage(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 222);
	if (context.flags.z()) goto alreadyincryan;
	context.data.byte(kCommandtype) = 222;
	context.al = 31;
	commandonly(context);
alreadyincryan:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto noincryan;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doincryan;
noincryan:
	{assert(stack_depth == context.stack.size()); return; }
doincryan:
	context.ax = context.data.word(kMousex);
	context._sub(context.ax, (80)+167);
	context.data.byte(kRyanpage) = -1;
findnewpage:
	context._inc(context.data.byte(kRyanpage));
	context._sub(context.ax, 18);
	if (!context.flags.c()) goto findnewpage;
	delpointer(context);
	fillryan(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void openinv(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kInvopen) = 1;
	context.al = 61;
	context.di = (80);
	context.bx = (58)-10;
	context.dl = 240;
	printmessage(context);
	fillryan(context);
	context.data.byte(kCommandtype) = 255;
	{assert(stack_depth == context.stack.size()); return; }
}

void showryanpage(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kIcons1);
	context.di = (80)+167;
	context.bx = (58)-12;
	context.al = 12;
	context.ah = 0;
	showframe(context);
	context.al = 13;
	context._add(context.al, context.data.byte(kRyanpage));
	context.push(context.ax);
	context.al = context.data.byte(kRyanpage);
	context.ah = 0;
	context.cx = 18;
	context._mul(context.cx);
	context.ds = context.data.word(kIcons1);
	context.di = (80)+167;
	context._add(context.di, context.ax);
	context.bx = (58)-12;
	context.ax = context.pop();
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void openob(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kOpenedob);
	context.ah = context.data.byte(kOpenedtype);
	context.di = 5674;
	copyname(context);
	context.di = (80);
	context.bx = (58)+86;
	context.al = 62;
	context.dl = 240;
	printmessage(context);
	context.di = context.data.word(kLastxpos);
	context._add(context.di, 5);
	context.bx = (58)+86;
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
	context.cx = (44);
	context._mul(context.cx);
	context._add(context.ax, (80));
	context.bx = 2588;
	context.cs.word(context.bx) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void obicons(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kCommand);
	getanyad(context);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto cantopenit;
	context.ds = context.data.word(kIcons2);
	context.di = 210;
	context.bx = 1;
	context.al = 4;
	context.ah = 0;
	showframe(context);
cantopenit:
	context.ds = context.data.word(kIcons2);
	context.di = 260;
	context.bx = 1;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void examicon(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kIcons2);
	context.di = 254;
	context.bx = 5;
	context.al = 3;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void obpicture(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kCommand);
	context.ah = context.data.byte(kObjecttype);
	context._cmp(context.ah, 1);
	if (context.flags.z()) goto setframe;
	context._cmp(context.ah, 4);
	if (context.flags.z()) goto exframe;
	context.ds = context.data.word(kFreeframes);
	context.di = 160;
	context.bx = 68;
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	context._inc(context.al);
	context.ah = 128;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
setframe:
	{assert(stack_depth == context.stack.size()); return; }
exframe:
	context.ds = context.data.word(kExtras);
	context.di = 160;
	context.bx = 68;
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	context._inc(context.al);
	context.ah = 128;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void describeob(Context & context) {
	uint stack_depth = context.stack.size();
	getobtextstart(context);
	context.di = 33;
	context.bx = 92;
	context.dl = 241;
	context.ah = 16;
	context.data.word(kCharshift) = 91+91;
	printdirect(context);
	context.data.word(kCharshift) = 0;
	context.di = 36;
	context.bx = 104;
	context.dl = 241;
	context.ah = 0;
	printdirect(context);
	context.push(context.bx);
	obsthatdothings(context);
	context.bx = context.pop();
	additionaltext(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void additionaltext(Context & context) {
	uint stack_depth = context.stack.size();
	context._add(context.bx, 10);
	context.push(context.bx);
	context.al = context.data.byte(kCommand);
	context.ah = context.data.byte(kObjecttype);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto emptycup;
	context.al = context.data.byte(kCommand);
	context.ah = context.data.byte(kObjecttype);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'F';
	compare(context);
	if (context.flags.z()) goto fullcup;
	context.bx = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
emptycup:
	context.al = 40;
	findpuztext(context);
	context.bx = context.pop();
	context.di = 36;
	context.dl = 241;
	context.ah = 0;
	printdirect(context);
	{assert(stack_depth == context.stack.size()); return; }
fullcup:
	context.al = 39;
	findpuztext(context);
	context.bx = context.pop();
	context.di = 36;
	context.dl = 241;
	context.ah = 0;
	printdirect(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void obsthatdothings(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kCommand);
	context.ah = context.data.byte(kObjecttype);
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
	{assert(stack_depth == context.stack.size()); return; }
notlouiscard:
	{assert(stack_depth == context.stack.size()); return; }
}

void getobtextstart(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kFreedesc);
	context.si = (0);
	context.cx = (0+(82*2));
	context._cmp(context.data.byte(kObjecttype), 2);
	if (context.flags.z()) goto describe;
	context.es = context.data.word(kSetdesc);
	context.si = (0);
	context.cx = (0+(130*2));
	context._cmp(context.data.byte(kObjecttype), 1);
	if (context.flags.z()) goto describe;
	context.es = context.data.word(kExtras);
	context.si = (0+2080+30000+(16*114));
	context.cx = (0+2080+30000+(16*114)+((114+2)*2));
describe:
	context.al = context.data.byte(kCommand);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.si, context.ax);
	context.ax = context.es.word(context.si);
	context._add(context.ax, context.cx);
	context.si = context.ax;
	context.bx = context.ax;
tryagain:
	context.push(context.si);
	findnextcolon(context);
	context.al = context.es.byte(context.si);
	context.cx = context.si;
	context.si = context.pop();
	context._cmp(context.data.byte(kObjecttype), 1);
	if (!context.flags.z()) goto cantmakeoneup;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto findsometext;
	context._cmp(context.al, ':');
	if (context.flags.z()) goto findsometext;
cantmakeoneup:
	{assert(stack_depth == context.stack.size()); return; }
findsometext:
	searchforsame(context);
	goto tryagain;
	{assert(stack_depth == context.stack.size()); return; }
}

void searchforsame(Context & context) {
	uint stack_depth = context.stack.size();
	context.si = context.cx;
searchagain:
	context._inc(context.si);
	context.al = context.es.byte(context.bx);
search:
	context._cmp(context.es.byte(context.si), context.al);
	if (context.flags.z()) goto gotstartletter;
	context._inc(context.cx);
	context._inc(context.si);
	context._cmp(context.si, 8000);
	if (context.flags.c()) goto search;
	context.si = context.bx;
	context.ax = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
gotstartletter:
	context.push(context.bx);
	context.push(context.si);
keepchecking:
	context._inc(context.si);
	context._inc(context.bx);
	context.al = context.es.byte(context.bx);
	context.ah = context.es.byte(context.si);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void findnextcolon(Context & context) {
	uint stack_depth = context.stack.size();
isntcolon:
	context.al = context.es.byte(context.si);
	context._inc(context.si);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto endofcolon;
	context._cmp(context.al, ':');
	if (!context.flags.z()) goto isntcolon;
endofcolon:
	{assert(stack_depth == context.stack.size()); return; }
}

void inventory(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kMandead), 1);
	if (context.flags.z()) goto iswatchinv;
	context._cmp(context.data.word(kWatchingtime), 0);
	if (context.flags.z()) goto notwatchinv;
iswatchinv:
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
notwatchinv:
	context._cmp(context.data.byte(kCommandtype), 239);
	if (context.flags.z()) goto alreadyopinv;
	context.data.byte(kCommandtype) = 239;
	context.al = 32;
	commandonly(context);
alreadyopinv:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto cantopinv;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doopeninv;
cantopinv:
	{assert(stack_depth == context.stack.size()); return; }
doopeninv:
	context.data.word(kTimecount) = 0;
	context.data.byte(kPointermode) = 0;
	context.data.byte(kInmaparea) = 0;
	animpointer(context);
	createpanel(context);
	showpanel(context);
	examicon(context);
	showman(context);
	showexit(context);
	undertextline(context);
	context.data.byte(kPickup) = 0;
	context.data.byte(kInvopen) = 2;
	openinv(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	context.data.byte(kOpenedob) = 255;
	goto waitexam;
/*continuing to unbounded code: examineagain from examineob:3-66*/
examineagain:
	context.data.byte(kInmaparea) = 0;
	context.data.byte(kExamagain) = 0;
	context.data.byte(kOpenedob) = 255;
	context.data.byte(kOpenedtype) = 255;
	context.data.byte(kInvopen) = 0;
	context.al = context.data.byte(kCommandtype);
	context.data.byte(kObjecttype) = context.al;
	context.data.byte(kItemframe) = 0;
	context.data.byte(kPointerframe) = 0;
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	obicons(context);
	obpicture(context);
	describeob(context);
	undertextline(context);
	context.data.byte(kCommandtype) = 255;
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
	context.data.byte(kGetback) = 0;
	context.bx = 2494;
	context._cmp(context.data.byte(kInvopen), 0);
	if (context.flags.z()) goto notuseinv;
	context.bx = 2556;
	context._cmp(context.data.byte(kInvopen), 1);
	if (context.flags.z()) goto notuseinv;
	context.bx = 2618;
notuseinv:
	checkcoords(context);
	context._cmp(context.data.byte(kExamagain), 0);
	if (context.flags.z()) goto norex;
	goto examineagain;
norex:
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto waitexam;
	context.data.byte(kPickup) = 0;
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) goto iswatching;
	context._cmp(context.data.byte(kNewlocation), 255);
	if (!context.flags.z()) goto justgetback;
iswatching:
	makemainscreen(context);
	context.data.byte(kInvopen) = 0;
	context.data.byte(kOpenedob) = 255;
	{assert(stack_depth == context.stack.size()); return; }
justgetback:
	context.data.byte(kInvopen) = 0;
	context.data.byte(kOpenedob) = 255;
	{assert(stack_depth == context.stack.size()); return; }
examlist:
invlist1:
openchangesize:
withlist1:
	{assert(stack_depth == context.stack.size()); return; }
}

void setpickup(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kObjecttype), 1);
	if (context.flags.z()) goto cantpick;
	context._cmp(context.data.byte(kObjecttype), 3);
	if (context.flags.z()) goto cantpick;
	getanyad(context);
	context.al = context.es.byte(context.bx+2);
	context._cmp(context.al, 4);
	if (!context.flags.z()) goto canpick;
cantpick:
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
canpick:
	context._cmp(context.data.byte(kCommandtype), 209);
	if (context.flags.z()) goto alreadysp;
	context.data.byte(kCommandtype) = 209;
	context.bl = context.data.byte(kCommand);
	context.bh = context.data.byte(kObjecttype);
	context.al = 33;
	commandwithob(context);
alreadysp:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, 1);
	if (!context.flags.z()) goto nosetpick;
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (!context.flags.z()) goto dosetpick;
nosetpick:
	{assert(stack_depth == context.stack.size()); return; }
dosetpick:
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	examicon(context);
	context.data.byte(kPickup) = 1;
	context.data.byte(kInvopen) = 2;
	context._cmp(context.data.byte(kObjecttype), 4);
	if (context.flags.z()) goto pickupexob;
	context.al = context.data.byte(kCommand);
	context.data.byte(kItemframe) = context.al;
	context.data.byte(kOpenedob) = 255;
	transfertoex(context);
	context.data.byte(kItemframe) = context.al;
	context.data.byte(kObjecttype) = 4;
	geteitherad(context);
	context.es.byte(context.bx+2) = 20;
	context.es.byte(context.bx+3) = 255;
	openinv(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
pickupexob:
	context.al = context.data.byte(kCommand);
	context.data.byte(kItemframe) = context.al;
	context.data.byte(kOpenedob) = 255;
	openinv(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void examinventory(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 249);
	if (context.flags.z()) goto alreadyexinv;
	context.data.byte(kCommandtype) = 249;
	context.al = 32;
	commandonly(context);
alreadyexinv:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doexinv;
	{assert(stack_depth == context.stack.size()); return; }
doexinv:
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	examicon(context);
	context.data.byte(kPickup) = 0;
	context.data.byte(kInvopen) = 2;
	openinv(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void reexfrominv(Context & context) {
	uint stack_depth = context.stack.size();
	findinvpos(context);
	context.ax = context.es.word(context.bx);
	context.data.byte(kCommandtype) = context.ah;
	context.data.byte(kCommand) = context.al;
	context.data.byte(kExamagain) = 1;
	context.data.byte(kPointermode) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void reexfromopen(Context & context) {
	uint stack_depth = context.stack.size();
	{assert(stack_depth == context.stack.size()); return; }
	findopenpos(context);
	context.ax = context.es.word(context.bx);
	context.data.byte(kCommandtype) = context.ah;
	context.data.byte(kCommand) = context.al;
	context.data.byte(kExamagain) = 1;
	context.data.byte(kPointermode) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void swapwithinv(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kItemframe);
	context.ah = context.data.byte(kObjecttype);
	context._cmp(context.ax, context.data.word(kOldsubject));
	if (!context.flags.z()) goto difsub7;
	context._cmp(context.data.byte(kCommandtype), 243);
	if (context.flags.z()) goto alreadyswap1;
	context.data.byte(kCommandtype) = 243;
difsub7:
	context.data.word(kOldsubject) = context.ax;
	context.bx = context.ax;
	context.al = 34;
	commandwithob(context);
alreadyswap1:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto cantswap1;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doswap1;
cantswap1:
	{assert(stack_depth == context.stack.size()); return; }
doswap1:
	context.ah = context.data.byte(kObjecttype);
	context.al = context.data.byte(kItemframe);
	context.push(context.ax);
	findinvpos(context);
	context.ax = context.es.word(context.bx);
	context.data.byte(kItemframe) = context.al;
	context.data.byte(kObjecttype) = context.ah;
	geteitherad(context);
	context.es.byte(context.bx+2) = 20;
	context.es.byte(context.bx+3) = 255;
	context.bl = context.data.byte(kItemframe);
	context.bh = context.data.byte(kObjecttype);
	context.ax = context.pop();
	context.data.byte(kObjecttype) = context.ah;
	context.data.byte(kItemframe) = context.al;
	context.push(context.bx);
	findinvpos(context);
	delpointer(context);
	context.al = context.data.byte(kItemframe);
	geteitherad(context);
	context.es.byte(context.bx+2) = 4;
	context.es.byte(context.bx+3) = 255;
	context.al = context.data.byte(kLastinvpos);
	context.es.byte(context.bx+4) = context.al;
	context.ax = context.pop();
	context.data.byte(kObjecttype) = context.ah;
	context.data.byte(kItemframe) = context.al;
	fillryan(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void swapwithopen(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kItemframe);
	context.ah = context.data.byte(kObjecttype);
	context._cmp(context.ax, context.data.word(kOldsubject));
	if (!context.flags.z()) goto difsub8;
	context._cmp(context.data.byte(kCommandtype), 242);
	if (context.flags.z()) goto alreadyswap2;
	context.data.byte(kCommandtype) = 242;
difsub8:
	context.data.word(kOldsubject) = context.ax;
	context.bx = context.ax;
	context.al = 34;
	commandwithob(context);
alreadyswap2:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto cantswap2;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doswap2;
cantswap2:
	{assert(stack_depth == context.stack.size()); return; }
doswap2:
	geteitherad(context);
	isitworn(context);
	if (!context.flags.z()) goto notwornswap;
	wornerror(context);
	{assert(stack_depth == context.stack.size()); return; }
notwornswap:
	delpointer(context);
	context.al = context.data.byte(kItemframe);
	context._cmp(context.al, context.data.byte(kOpenedob));
	if (!context.flags.z()) goto isntsame2;
	context.al = context.data.byte(kObjecttype);
	context._cmp(context.al, context.data.byte(kOpenedtype));
	if (!context.flags.z()) goto isntsame2;
	errormessage1(context);
	{assert(stack_depth == context.stack.size()); return; }
isntsame2:
	checkobjectsize(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto sizeok2;
	{assert(stack_depth == context.stack.size()); return; }
sizeok2:
	context.ah = context.data.byte(kObjecttype);
	context.al = context.data.byte(kItemframe);
	context.push(context.ax);
	findopenpos(context);
	context.ax = context.es.word(context.bx);
	context.data.byte(kItemframe) = context.al;
	context.data.byte(kObjecttype) = context.ah;
	context._cmp(context.ah, 4);
	if (!context.flags.z()) goto makeswapex;
	geteitherad(context);
	context.es.byte(context.bx+2) = 20;
	context.es.byte(context.bx+3) = 255;
	goto actuallyswap;
makeswapex:
	transfertoex(context);
	context.data.byte(kItemframe) = context.al;
	context.data.byte(kObjecttype) = 4;
	geteitherad(context);
	context.es.byte(context.bx+2) = 20;
	context.es.byte(context.bx+3) = 255;
actuallyswap:
	context.bl = context.data.byte(kItemframe);
	context.bh = context.data.byte(kObjecttype);
	context.ax = context.pop();
	context.data.byte(kObjecttype) = context.ah;
	context.data.byte(kItemframe) = context.al;
	context.push(context.bx);
	findopenpos(context);
	geteitherad(context);
	context.al = context.data.byte(kOpenedtype);
	context.es.byte(context.bx+2) = context.al;
	context.al = context.data.byte(kOpenedob);
	context.es.byte(context.bx+3) = context.al;
	context.al = context.data.byte(kLastinvpos);
	context.es.byte(context.bx+4) = context.al;
	context.al = context.data.byte(kReallocation);
	context.es.byte(context.bx+5) = context.al;
	context.ax = context.pop();
	context.data.byte(kObjecttype) = context.ah;
	context.data.byte(kItemframe) = context.al;
	fillopen(context);
	fillryan(context);
	undertextline(context);
	readmouse(context);
	useopened(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void intoinv(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kPickup), 0);
	if (!context.flags.z()) goto notout;
	outofinv(context);
	{assert(stack_depth == context.stack.size()); return; }
notout:
	findinvpos(context);
	context.ax = context.es.word(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto canplace1;
	swapwithinv(context);
	{assert(stack_depth == context.stack.size()); return; }
canplace1:
	context.al = context.data.byte(kItemframe);
	context.ah = context.data.byte(kObjecttype);
	context._cmp(context.ax, context.data.word(kOldsubject));
	if (!context.flags.z()) goto difsub1;
	context._cmp(context.data.byte(kCommandtype), 220);
	if (context.flags.z()) goto alreadyplce;
	context.data.byte(kCommandtype) = 220;
difsub1:
	context.data.word(kOldsubject) = context.ax;
	context.bx = context.ax;
	context.al = 35;
	commandwithob(context);
alreadyplce:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notletgo2;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doplace;
notletgo2:
	{assert(stack_depth == context.stack.size()); return; }
doplace:
	delpointer(context);
	context.al = context.data.byte(kItemframe);
	getexad(context);
	context.es.byte(context.bx+2) = 4;
	context.es.byte(context.bx+3) = 255;
	context.al = context.data.byte(kLastinvpos);
	context.es.byte(context.bx+4) = context.al;
	context.data.byte(kPickup) = 0;
	fillryan(context);
	readmouse(context);
	showpointer(context);
	outofinv(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void deletetaken(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kFreedat);
	context.ah = context.data.byte(kReallocation);
	context.ds = context.data.word(kExtras);
	context.si = (0+2080+30000);
	context.cx = (114);
takenloop:
	context.al = context.ds.byte(context.si+11);
	context._cmp(context.al, context.ah);
	if (!context.flags.z()) goto notinhere;
	context.bl = context.ds.byte(context.si+1);
	context.bh = 0;
	context._add(context.bx, context.bx);
	context._add(context.bx, context.bx);
	context._add(context.bx, context.bx);
	context._add(context.bx, context.bx);
	context.es.byte(context.bx+2) = 254;
notinhere:
	context._add(context.si, 16);
	if (--context.cx) goto takenloop;
	{assert(stack_depth == context.stack.size()); return; }
}

void outofinv(Context & context) {
	uint stack_depth = context.stack.size();
	findinvpos(context);
	context.ax = context.es.word(context.bx);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto canpick2;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
canpick2:
	context.bx = context.data.word(kMousebutton);
	context._cmp(context.bx, 2);
	if (!context.flags.z()) goto canpick2a;
	reexfrominv(context);
	{assert(stack_depth == context.stack.size()); return; }
canpick2a:
	context._cmp(context.ax, context.data.word(kOldsubject));
	if (!context.flags.z()) goto difsub3;
	context._cmp(context.data.byte(kCommandtype), 221);
	if (context.flags.z()) goto alreadygrab;
	context.data.byte(kCommandtype) = 221;
difsub3:
	context.data.word(kOldsubject) = context.ax;
	context.bx = context.ax;
	context.al = 36;
	commandwithob(context);
alreadygrab:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notletgo;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dograb;
notletgo:
	{assert(stack_depth == context.stack.size()); return; }
dograb:
	delpointer(context);
	context.data.byte(kPickup) = 1;
	findinvpos(context);
	context.ax = context.es.word(context.bx);
	context.data.byte(kItemframe) = context.al;
	context.data.byte(kObjecttype) = context.ah;
	getexad(context);
	context.es.byte(context.bx+2) = 20;
	context.es.byte(context.bx+3) = 255;
	fillryan(context);
	readmouse(context);
	showpointer(context);
	intoinv(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getfreead(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 0;
	context.cl = 4;
	context._shl(context.ax, context.cl);
	context.bx = context.ax;
	context.es = context.data.word(kFreedat);
	{assert(stack_depth == context.stack.size()); return; }
}

void getexad(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 0;
	context.bx = 16;
	context._mul(context.bx);
	context.bx = context.ax;
	context.es = context.data.word(kExtras);
	context._add(context.bx, (0+2080+30000));
	{assert(stack_depth == context.stack.size()); return; }
}

void geteitherad(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kObjecttype), 4);
	if (context.flags.z()) goto isinexlist;
	context.al = context.data.byte(kItemframe);
	getfreead(context);
	{assert(stack_depth == context.stack.size()); return; }
isinexlist:
	context.al = context.data.byte(kItemframe);
	getexad(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getanyad(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kObjecttype), 4);
	if (context.flags.z()) goto isex;
	context._cmp(context.data.byte(kObjecttype), 2);
	if (context.flags.z()) goto isfree;
	context.al = context.data.byte(kCommand);
	getsetad(context);
	context.ax = context.es.word(context.bx+4);
	{assert(stack_depth == context.stack.size()); return; }
isfree:
	context.al = context.data.byte(kCommand);
	getfreead(context);
	context.ax = context.es.word(context.bx+7);
	{assert(stack_depth == context.stack.size()); return; }
isex:
	context.al = context.data.byte(kCommand);
	getexad(context);
	context.ax = context.es.word(context.bx+7);
	{assert(stack_depth == context.stack.size()); return; }
}

void getanyaddir(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.ah, 4);
	if (context.flags.z()) goto isex3;
	context._cmp(context.ah, 2);
	if (context.flags.z()) goto isfree3;
	getsetad(context);
	{assert(stack_depth == context.stack.size()); return; }
isfree3:
	getfreead(context);
	{assert(stack_depth == context.stack.size()); return; }
isex3:
	getexad(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getopenedsize(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kOpenedtype), 4);
	if (context.flags.z()) goto isex2;
	context._cmp(context.data.byte(kOpenedtype), 2);
	if (context.flags.z()) goto isfree2;
	context.al = context.data.byte(kOpenedob);
	getsetad(context);
	context.ax = context.es.word(context.bx+3);
	{assert(stack_depth == context.stack.size()); return; }
isfree2:
	context.al = context.data.byte(kOpenedob);
	getfreead(context);
	context.ax = context.es.word(context.bx+7);
	{assert(stack_depth == context.stack.size()); return; }
isex2:
	context.al = context.data.byte(kOpenedob);
	getexad(context);
	context.ax = context.es.word(context.bx+7);
	{assert(stack_depth == context.stack.size()); return; }
}

void getsetad(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 0;
	context.bx = 64;
	context._mul(context.bx);
	context.bx = context.ax;
	context.es = context.data.word(kSetdat);
	{assert(stack_depth == context.stack.size()); return; }
}

void findinvpos(Context & context) {
	uint stack_depth = context.stack.size();
	context.cx = context.data.word(kMousex);
	context._sub(context.cx, (80));
	context.bx = -1;
findinv1:
	context._inc(context.bx);
	context._sub(context.cx, (44));
	if (!context.flags.c()) goto findinv1;
	context.cx = context.data.word(kMousey);
	context._sub(context.cx, (58));
	context._sub(context.bx, 5);
findinv2:
	context._add(context.bx, 5);
	context._sub(context.cx, (44));
	if (!context.flags.c()) goto findinv2;
	context.al = context.data.byte(kRyanpage);
	context.ah = 0;
	context.cx = 10;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.al = context.bl;
	context.data.byte(kLastinvpos) = context.al;
	context._add(context.bx, context.bx);
	context.es = context.data.word(kBuffers);
	context._add(context.bx, (0+(180*10)+32));
	{assert(stack_depth == context.stack.size()); return; }
}

void findopenpos(Context & context) {
	uint stack_depth = context.stack.size();
	context.cx = context.data.word(kMousex);
	context._sub(context.cx, (80));
	context.bx = -1;
findopenp1:
	context._inc(context.bx);
	context._sub(context.cx, (44));
	if (!context.flags.c()) goto findopenp1;
	context.al = context.bl;
	context.data.byte(kLastinvpos) = context.al;
	context._add(context.bx, context.bx);
	context.es = context.data.word(kBuffers);
	context._add(context.bx, (0+(180*10)));
	{assert(stack_depth == context.stack.size()); return; }
}

void dropobject(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 223);
	if (context.flags.z()) goto alreadydrop;
	context.data.byte(kCommandtype) = 223;
	context._cmp(context.data.byte(kPickup), 0);
	if (context.flags.z()) { blank(context); return; };
	context.bl = context.data.byte(kItemframe);
	context.bh = context.data.byte(kObjecttype);
	context.al = 37;
	commandwithob(context);
alreadydrop:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto nodrop;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dodrop;
nodrop:
	{assert(stack_depth == context.stack.size()); return; }
dodrop:
	geteitherad(context);
	isitworn(context);
	if (!context.flags.z()) goto nowornerror;
	wornerror(context);
	{assert(stack_depth == context.stack.size()); return; }
nowornerror:
	context._cmp(context.data.byte(kReallocation), 47);
	if (context.flags.z()) goto nodrop2;
	context.cl = context.data.byte(kRyanx);
	context._add(context.cl, 12);
	context.ch = context.data.byte(kRyany);
	context._add(context.ch, 12);
	checkone(context);
	context._cmp(context.cl, 2);
	if (context.flags.c()) goto nodroperror;
nodrop2:
	droperror(context);
	{assert(stack_depth == context.stack.size()); return; }
nodroperror:
	context._cmp(context.data.byte(kMapxsize), 64);
	if (!context.flags.z()) goto notinlift;
	context._cmp(context.data.byte(kMapysize), 64);
	if (!context.flags.z()) goto notinlift;
	droperror(context);
	{assert(stack_depth == context.stack.size()); return; }
notinlift:
	context.al = context.data.byte(kItemframe);
	context.ah = 4;
	context.cl = 'G';
	context.ch = 'U';
	context.dl = 'N';
	context.dh = 'A';
	compare(context);
	if (context.flags.z()) { cantdrop(context); return; };
	context.al = context.data.byte(kItemframe);
	context.ah = 4;
	context.cl = 'S';
	context.ch = 'H';
	context.dl = 'L';
	context.dh = 'D';
	compare(context);
	if (context.flags.z()) { cantdrop(context); return; };
	context.data.byte(kObjecttype) = 4;
	context.al = context.data.byte(kItemframe);
	getexad(context);
	context.es.byte(context.bx+2) = 0;
	context.al = context.data.byte(kRyanx);
	context._add(context.al, 4);
	context.cl = 4;
	context._shr(context.al, context.cl);
	context._add(context.al, context.data.byte(kMapx));
	context.ah = context.data.byte(kRyany);
	context._add(context.ah, 8);
	context.cl = 4;
	context._shr(context.ah, context.cl);
	context._add(context.ah, context.data.byte(kMapy));
	context.es.byte(context.bx+3) = context.al;
	context.es.byte(context.bx+5) = context.ah;
	context.al = context.data.byte(kRyanx);
	context._add(context.al, 4);
	context._and(context.al, 15);
	context.ah = context.data.byte(kRyany);
	context._add(context.ah, 8);
	context._and(context.ah, 15);
	context.es.byte(context.bx+4) = context.al;
	context.es.byte(context.bx+6) = context.ah;
	context.data.byte(kPickup) = 0;
	context.al = context.data.byte(kReallocation);
	context.es.byte(context.bx) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void droperror(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kCommandtype) = 255;
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
	context.data.byte(kCommandtype) = 255;
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void cantdrop(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kCommandtype) = 255;
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
	context.data.byte(kCommandtype) = 255;
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void wornerror(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kCommandtype) = 255;
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
	context.data.byte(kCommandtype) = 255;
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void removeobfrominv(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommand), 100);
	if (context.flags.z()) goto obnotexist;
	getanyad(context);
	context.di = context.bx;
	context.cl = context.data.byte(kCommand);
	context.ch = 0;
	deleteexobject(context);
obnotexist:
	{assert(stack_depth == context.stack.size()); return; }
}

void selectopenob(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kCommand);
	getanyad(context);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto canopenit1;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
canopenit1:
	context._cmp(context.data.byte(kCommandtype), 224);
	if (context.flags.z()) goto alreadyopob;
	context.data.byte(kCommandtype) = 224;
	context.bl = context.data.byte(kCommand);
	context.bh = context.data.byte(kObjecttype);
	context.al = 38;
	commandwithob(context);
alreadyopob:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto noopenob;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doopenob;
noopenob:
	{assert(stack_depth == context.stack.size()); return; }
doopenob:
	context.al = context.data.byte(kCommand);
	context.data.byte(kOpenedob) = context.al;
	context.al = context.data.byte(kObjecttype);
	context.data.byte(kOpenedtype) = context.al;
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
	{assert(stack_depth == context.stack.size()); return; }
}

void useopened(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kOpenedob), 255);
	if (context.flags.z()) goto cannotuseopen;
	context._cmp(context.data.byte(kPickup), 0);
	if (!context.flags.z()) goto notout2;
	outofopen(context);
	{assert(stack_depth == context.stack.size()); return; }
notout2:
	findopenpos(context);
	context.ax = context.es.word(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto canplace3;
	swapwithopen(context);
cannotuseopen:
	{assert(stack_depth == context.stack.size()); return; }
canplace3:
	context._cmp(context.data.byte(kPickup), 1);
	if (context.flags.z()) goto intoopen;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
intoopen:
	context.al = context.data.byte(kItemframe);
	context.ah = context.data.byte(kObjecttype);
	context._cmp(context.ax, context.data.word(kOldsubject));
	if (!context.flags.z()) goto difsub2;
	context._cmp(context.data.byte(kCommandtype), 227);
	if (context.flags.z()) goto alreadyplc2;
	context.data.byte(kCommandtype) = 227;
difsub2:
	context.data.word(kOldsubject) = context.ax;
	context.bx = context.ax;
	context.al = 35;
	commandwithob(context);
alreadyplc2:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notletgo3;
	context._cmp(context.ax, 1);
	if (context.flags.z()) goto doplace2;
notletgo3:
	{assert(stack_depth == context.stack.size()); return; }
doplace2:
	geteitherad(context);
	isitworn(context);
	if (!context.flags.z()) goto notworntoopen;
	wornerror(context);
	{assert(stack_depth == context.stack.size()); return; }
notworntoopen:
	delpointer(context);
	context.al = context.data.byte(kItemframe);
	context._cmp(context.al, context.data.byte(kOpenedob));
	if (!context.flags.z()) goto isntsame;
	context.al = context.data.byte(kObjecttype);
	context._cmp(context.al, context.data.byte(kOpenedtype));
	if (!context.flags.z()) goto isntsame;
	errormessage1(context);
	{assert(stack_depth == context.stack.size()); return; }
isntsame:
	checkobjectsize(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto sizeok1;
	{assert(stack_depth == context.stack.size()); return; }
sizeok1:
	context.data.byte(kPickup) = 0;
	context.al = context.data.byte(kItemframe);
	geteitherad(context);
	context.al = context.data.byte(kOpenedtype);
	context.es.byte(context.bx+2) = context.al;
	context.al = context.data.byte(kOpenedob);
	context.es.byte(context.bx+3) = context.al;
	context.al = context.data.byte(kLastinvpos);
	context.es.byte(context.bx+4) = context.al;
	context.al = context.data.byte(kReallocation);
	context.es.byte(context.bx+5) = context.al;
	fillopen(context);
	undertextline(context);
	readmouse(context);
	useopened(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void errormessage1(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void errormessage2(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kCommandtype) = 255;
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
	{assert(stack_depth == context.stack.size()); return; }
}

void errormessage3(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void checkobjectsize(Context & context) {
	uint stack_depth = context.stack.size();
	getopenedsize(context);
	context.push(context.ax);
	context.al = context.data.byte(kItemframe);
	geteitherad(context);
	context.al = context.es.byte(context.bx+9);
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
	{assert(stack_depth == context.stack.size()); return; }
sizeok:
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void outofopen(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kOpenedob), 255);
	if (context.flags.z()) goto cantuseopen;
	findopenpos(context);
	context.ax = context.es.word(context.bx);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto canpick4;
cantuseopen:
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
canpick4:
	context._cmp(context.ax, context.data.word(kOldsubject));
	if (!context.flags.z()) goto difsub4;
	context._cmp(context.data.byte(kCommandtype), 228);
	if (context.flags.z()) goto alreadygrb;
	context.data.byte(kCommandtype) = 228;
difsub4:
	context.data.word(kOldsubject) = context.ax;
	context.bx = context.ax;
	context.al = 36;
	commandwithob(context);
alreadygrb:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notletgo4;
	context._cmp(context.ax, 1);
	if (context.flags.z()) goto dogrb;
	context._cmp(context.ax, 2);
	if (!context.flags.z()) goto notletgo4;
	reexfromopen(context);
notletgo4:
	{assert(stack_depth == context.stack.size()); return; }
dogrb:
	delpointer(context);
	context.data.byte(kPickup) = 1;
	findopenpos(context);
	context.ax = context.es.word(context.bx);
	context.data.byte(kItemframe) = context.al;
	context.data.byte(kObjecttype) = context.ah;
	context._cmp(context.ah, 4);
	if (!context.flags.z()) goto makeintoex;
	geteitherad(context);
	context.es.byte(context.bx+2) = 20;
	context.es.byte(context.bx+3) = 255;
	goto actuallyout;
makeintoex:
	transfertoex(context);
	context.data.byte(kItemframe) = context.al;
	context.data.byte(kObjecttype) = 4;
	geteitherad(context);
	context.es.byte(context.bx+2) = 20;
	context.es.byte(context.bx+3) = 255;
actuallyout:
	fillopen(context);
	undertextline(context);
	readmouse(context);
	useopened(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void transfertoex(Context & context) {
	uint stack_depth = context.stack.size();
	emergencypurge(context);
	getexpos(context);
	context.al = context.data.byte(kExpos);
	context.push(context.ax);
	context.push(context.di);
	context.al = context.data.byte(kItemframe);
	context.ah = 0;
	context.bx = 16;
	context._mul(context.bx);
	context.ds = context.data.word(kFreedat);
	context.si = context.ax;
	context.cx = 8;
	while(context.cx--) 	context._movsw();
 	context.di = context.pop();
	context.al = context.data.byte(kReallocation);
	context.es.byte(context.di) = context.al;
	context.es.byte(context.di+11) = context.al;
	context.al = context.data.byte(kItemframe);
	context.es.byte(context.di+1) = context.al;
	context.es.byte(context.di+2) = 4;
	context.es.byte(context.di+3) = 255;
	context.al = context.data.byte(kLastinvpos);
	context.es.byte(context.di+4) = context.al;
	context.al = context.data.byte(kItemframe);
	context.data.byte(kItemtotran) = context.al;
	transfermap(context);
	transferinv(context);
	transfertext(context);
	context.al = context.data.byte(kItemframe);
	context.ah = 0;
	context.bx = 16;
	context._mul(context.bx);
	context.ds = context.data.word(kFreedat);
	context.si = context.ax;
	context.ds.byte(context.si+2) = 254;
	pickupconts(context);
	context.ax = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void pickupconts(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.ds.byte(context.si+7);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto notopenable;
	context.al = context.data.byte(kItemframe);
	context.ah = context.data.byte(kObjecttype);
	context.dl = context.data.byte(kExpos);
	context.es = context.data.word(kFreedat);
	context.bx = 0;
	context.cx = 0;
pickupcontloop:
	context.push(context.cx);
	context.push(context.es);
	context.push(context.bx);
	context.push(context.dx);
	context.push(context.ax);
	context._cmp(context.es.byte(context.bx+2), context.ah);
	if (!context.flags.z()) goto notinsidethis;
	context._cmp(context.es.byte(context.bx+3), context.al);
	if (!context.flags.z()) goto notinsidethis;
	context.data.byte(kItemtotran) = context.cl;
	transfercontoex(context);
notinsidethis:
	context.ax = context.pop();
	context.dx = context.pop();
	context.bx = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 16);
	context._inc(context.cx);
	context._cmp(context.cx, 80);
	if (!context.flags.z()) goto pickupcontloop;
notopenable:
	{assert(stack_depth == context.stack.size()); return; }
}

void transfercontoex(Context & context) {
	uint stack_depth = context.stack.size();
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
	while(context.cx--) 	context._movsw();
 	context.di = context.pop();
	context.dx = context.pop();
	context.al = context.data.byte(kReallocation);
	context.es.byte(context.di) = context.al;
	context.es.byte(context.di+11) = context.al;
	context.al = context.data.byte(kItemtotran);
	context.es.byte(context.di+1) = context.al;
	context.es.byte(context.di+3) = context.dl;
	context.es.byte(context.di+2) = 4;
	transfermap(context);
	transferinv(context);
	transfertext(context);
	context.si = context.pop();
	context.ds = context.pop();
	context.ds.byte(context.si+2) = 255;
	{assert(stack_depth == context.stack.size()); return; }
}

void transfertext(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kExtras);
	context.al = context.data.byte(kExpos);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = (0+2080+30000+(16*114));
	context._add(context.bx, context.ax);
	context.di = context.data.word(kExtextpos);
	context.es.word(context.bx) = context.di;
	context._add(context.di, (0+2080+30000+(16*114)+((114+2)*2)));
	context.al = context.data.byte(kItemtotran);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.ds = context.data.word(kFreedesc);
	context.bx = (0);
	context._add(context.bx, context.ax);
	context.si = (0+(82*2));
	context.ax = context.ds.word(context.bx);
	context._add(context.si, context.ax);
moretext:
	context._lodsb();
	context._stosb();
	context._inc(context.data.word(kExtextpos));
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto moretext;
	{assert(stack_depth == context.stack.size()); return; }
}

void getexpos(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kExtras);
	context.al = 0;
	context.di = (0+2080+30000);
tryanotherex:
	context._cmp(context.es.byte(context.di+2), 255);
	if (context.flags.z()) goto foundnewex;
	context._add(context.di, 16);
	context._inc(context.al);
	context._cmp(context.al, (114));
	if (!context.flags.z()) goto tryanotherex;
foundnewex:
	context.data.byte(kExpos) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void purgealocation(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context.es = context.data.word(kExtras);
	context.di = (0+2080+30000);
	context.bx = context.pop();
	context.cx = 0;
purgeloc:
	context._cmp(context.bl, context.es.byte(context.di+0));
	if (!context.flags.z()) goto dontpurge;
	context._cmp(context.es.byte(context.di+2), 0);
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
	context._inc(context.cx);
	context._cmp(context.cx, (114));
	if (!context.flags.z()) goto purgeloc;
	{assert(stack_depth == context.stack.size()); return; }
}

void emergencypurge(Context & context) {
	uint stack_depth = context.stack.size();
checkpurgeagain:
	context.ax = context.data.word(kExframepos);
	context._add(context.ax, 4000);
	context._cmp(context.ax, (30000));
	if (context.flags.c()) goto notnearframeend;
	purgeanitem(context);
	goto checkpurgeagain;
notnearframeend:
	context.ax = context.data.word(kExtextpos);
	context._add(context.ax, 400);
	context._cmp(context.ax, (18000));
	if (context.flags.c()) goto notneartextend;
	purgeanitem(context);
	goto checkpurgeagain;
notneartextend:
	{assert(stack_depth == context.stack.size()); return; }
}

void purgeanitem(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kExtras);
	context.di = (0+2080+30000);
	context.bl = context.data.byte(kReallocation);
	context.cx = 0;
lookforpurge:
	context.al = context.es.byte(context.di+2);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto cantpurge;
	context._cmp(context.es.byte(context.di+12), 2);
	if (context.flags.z()) goto iscup;
	context._cmp(context.es.byte(context.di+12), 255);
	if (!context.flags.z()) goto cantpurge;
iscup:
	context._cmp(context.es.byte(context.di+11), context.bl);
	if (context.flags.z()) goto cantpurge;
	deleteexobject(context);
	{assert(stack_depth == context.stack.size()); return; }
cantpurge:
	context._add(context.di, 16);
	context._inc(context.cx);
	context._cmp(context.cx, (114));
	if (!context.flags.z()) goto lookforpurge;
	context.di = (0+2080+30000);
	context.bl = context.data.byte(kReallocation);
	context.cx = 0;
lookforpurge2:
	context.al = context.es.byte(context.di+2);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto cantpurge2;
	context._cmp(context.es.byte(context.di+12), 255);
	if (!context.flags.z()) goto cantpurge2;
	deleteexobject(context);
	{assert(stack_depth == context.stack.size()); return; }
cantpurge2:
	context._add(context.di, 16);
	context._inc(context.cx);
	context._cmp(context.cx, (114));
	if (!context.flags.z()) goto lookforpurge2;
	{assert(stack_depth == context.stack.size()); return; }
}

void deleteexobject(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.cx);
	context.push(context.cx);
	context.push(context.cx);
	context.push(context.cx);
	context.al = 255;
	context.cx = 16;
	while(context.cx--) 	context._stosb();
	context.ax = context.pop();
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	deleteexframe(context);
	context.ax = context.pop();
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	context._inc(context.al);
	deleteexframe(context);
	context.ax = context.pop();
	deleteextext(context);
	context.bx = context.pop();
	context.bh = context.bl;
	context.bl = 4;
	context.di = (0+2080+30000);
	context.cx = 0;
deleteconts:
	context._cmp(context.es.word(context.di+2), context.bx);
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
	context._inc(context.cx);
	context._cmp(context.cx, (114));
	if (!context.flags.z()) goto deleteconts;
	{assert(stack_depth == context.stack.size()); return; }
}

void deleteexframe(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = (0);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.di, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.di, context.ax);
	context.al = context.es.byte(context.di);
	context.ah = 0;
	context.cl = context.es.byte(context.di+1);
	context.ch = 0;
	context._mul(context.cx);
	context.si = context.es.word(context.di+2);
	context.push(context.si);
	context._add(context.si, (0+2080));
	context.cx = (30000);
	context._sub(context.cx, context.es.word(context.di+2));
	context.di = context.si;
	context._add(context.si, context.ax);
	context.push(context.ax);
	context.ds = context.es;
	while(context.cx--) 	context._movsb();
 	context.bx = context.pop();
	context._sub(context.data.word(kExframepos), context.bx);
	context.si = context.pop();
	context.cx = (114)*3;
	context.di = (0);
shuffleadsdown:
	context.ax = context.es.word(context.di+2);
	context._cmp(context.ax, context.si);
	if (context.flags.c()) goto beforethisone;
	context._sub(context.ax, context.bx);
beforethisone:
	context.es.word(context.di+2) = context.ax;
	context._add(context.di, 6);
	if (--context.cx) goto shuffleadsdown;
	{assert(stack_depth == context.stack.size()); return; }
}

void deleteextext(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = (0+2080+30000+(16*114));
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.di, context.ax);
	context.ax = context.es.word(context.di);
	context.si = context.ax;
	context.di = context.ax;
	context._add(context.si, (0+2080+30000+(16*114)+((114+2)*2)));
	context._add(context.di, (0+2080+30000+(16*114)+((114+2)*2)));
	context.ax = 0;
findlenextext:
	context.cl = context.es.byte(context.si);
	context._inc(context.ax);
	context._inc(context.si);
	context._cmp(context.cl, 0);
	if (!context.flags.z()) goto findlenextext;
	context.cx = (18000);
	context.bx = context.si;
	context._sub(context.bx, (0+2080+30000+(16*114)+((114+2)*2)));
	context.push(context.bx);
	context.push(context.ax);
	context._sub(context.cx, context.bx);
	while(context.cx--) 	context._movsb();
 	context.bx = context.pop();
	context._sub(context.data.word(kExtextpos), context.bx);
	context.si = context.pop();
	context.cx = (114);
	context.di = (0+2080+30000+(16*114));
shuffletextads:
	context.ax = context.es.word(context.di);
	context._cmp(context.ax, context.si);
	if (context.flags.c()) goto beforethistext;
	context._sub(context.ax, context.bx);
beforethistext:
	context.es.word(context.di) = context.ax;
	context._add(context.di, 2);
	if (--context.cx) goto shuffletextads;
	{assert(stack_depth == context.stack.size()); return; }
}

void blockget(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = context.al;
	context.al = 0;
	context.ds = context.data.word(kBackdrop);
	context.si = (0+192);
	context._add(context.si, context.ax);
	{assert(stack_depth == context.stack.size()); return; }
}

void drawfloor(Context & context) {
	uint stack_depth = context.stack.size();
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
	context.data.byte(kNewobs) = 0;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void calcmapad(Context & context) {
	uint stack_depth = context.stack.size();
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
	context._add(context.ax, context.data.word(kMapoffsetx));
	context.data.word(kMapadx) = context.ax;
	context.dx = context.pop();
	context.cx = context.pop();
	context.al = 10;
	context._sub(context.al, context.dh);
	context._sub(context.al, context.ch);
	context._sub(context.al, context.ch);
	context.ax.cbw();
	context.bx = 8;
	context._mul(context.bx);
	context._add(context.ax, context.data.word(kMapoffsety));
	context.data.word(kMapady) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void getdimension(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32));
	context.ch = 0;
dimloop1:
	addalong(context);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto finishdim1;
	context._inc(context.ch);
	goto dimloop1;
finishdim1:
	context.bx = (0+(180*10)+32+60+(32*32));
	context.cl = 0;
dimloop2:
	context.push(context.bx);
	addlength(context);
	context.bx = context.pop();
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto finishdim2;
	context._inc(context.cl);
	context._add(context.bx, 3);
	goto dimloop2;
finishdim2:
	context.bx = (0+(180*10)+32+60+(32*32))+(11*3*9);
	context.dh = 10;
dimloop3:
	context.push(context.bx);
	addalong(context);
	context.bx = context.pop();
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto finishdim3;
	context._dec(context.dh);
	context._sub(context.bx, 11*3);
	goto dimloop3;
finishdim3:
	context.bx = (0+(180*10)+32+60+(32*32))+(3*10);
	context.dl = 11;
dimloop4:
	context.push(context.bx);
	addlength(context);
	context.bx = context.pop();
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto finishdim4;
	context._dec(context.dl);
	context._sub(context.bx, 3);
	goto dimloop4;
finishdim4:
	context.al = context.cl;
	context.ah = 0;
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context.data.word(kMapxstart) = context.ax;
	context.al = context.ch;
	context.ah = 0;
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context.data.word(kMapystart) = context.ax;
	context._sub(context.dl, context.cl);
	context._sub(context.dh, context.ch);
	context.al = context.dl;
	context.ah = 0;
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context.data.byte(kMapxsize) = context.al;
	context.al = context.dh;
	context.ah = 0;
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context._shl(context.ax, 1);
	context.data.byte(kMapysize) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void addalong(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 11;
addloop:
	context._cmp(context.es.byte(context.bx), 0);
	if (!context.flags.z()) goto gotalong;
	context._add(context.bx, 3);
	context._dec(context.ah);
	if (!context.flags.z()) goto addloop;
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
gotalong:
	context.al = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void addlength(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 10;
addloop2:
	context._cmp(context.es.byte(context.bx), 0);
	if (!context.flags.z()) goto gotlength;
	context._add(context.bx, 3*11);
	context._dec(context.ah);
	if (!context.flags.z()) goto addloop2;
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
gotlength:
	context.al = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void drawflags(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32));
	context.al = context.data.byte(kMapy);
	context.ah = 0;
	context.cx = (66);
	context._mul(context.cx);
	context.bl = context.data.byte(kMapx);
	context.bh = 0;
	context._add(context.ax, context.bx);
	context.si = (0);
	context._add(context.si, context.ax);
	context.cx = 10;
_tmp28:
	context.push(context.cx);
	context.cx = 11;
_tmp28a:
	context.ds = context.data.word(kMapdata);
	context._lodsb();
	context.ds = context.data.word(kBackdrop);
	context.push(context.si);
	context.push(context.ax);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.si = (0);
	context._add(context.si, context.ax);
	context._movsw();
 	context.ax = context.pop();
	context._stosb();
	context.si = context.pop();
	if (--context.cx) goto _tmp28a;
	context._add(context.si, (66)-11);
	context.cx = context.pop();
	if (--context.cx) goto _tmp28;
	{assert(stack_depth == context.stack.size()); return; }
}

void eraseoldobs(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kNewobs), 0);
	if (context.flags.z()) goto donterase;
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768);
	context.cx = 16;
oberase:
	context.push(context.cx);
	context.push(context.bx);
	context.ax = context.es.word(context.bx+20);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto notthisob;
	context.di = context.bx;
	context.al = 255;
	context.cx = (32);
	while(context.cx--) 	context._stosb();
notthisob:
	context.bx = context.pop();
	context.cx = context.pop();
	context._add(context.bx, (32));
	if (--context.cx) goto oberase;
donterase:
	{assert(stack_depth == context.stack.size()); return; }
}

void showallobs(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32));
	context.data.word(kListpos) = context.bx;
	context.di = context.bx;
	context.cx = 128*5;
	context.al = 255;
	while(context.cx--) 	context._stosb();
	context.es = context.data.word(kSetframes);
	context.data.word(kFrsegment) = context.es;
	context.ax = (0);
	context.data.word(kDataad) = context.ax;
	context.ax = (0+2080);
	context.data.word(kFramesad) = context.ax;
	context.data.byte(kCurrentob) = 0;
	context.ds = context.data.word(kSetdat);
	context.si = 0;
	context.cx = 128;
showobsloop:
	context.push(context.cx);
	context.push(context.si);
	context.push(context.si);
	context._add(context.si, 58);
	context.es = context.data.word(kSetdat);
	getmapad(context);
	context.si = context.pop();
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto blankframe;
	context.al = context.es.byte(context.si+18);
	context.ah = 0;
	context.data.word(kCurrentframe) = context.ax;
	context._cmp(context.al, 255);
	if (context.flags.z()) goto blankframe;
	context.push(context.es);
	context.push(context.si);
	calcfrframe(context);
	finalframe(context);
	context.si = context.pop();
	context.es = context.pop();
	context.al = context.es.byte(context.si+18);
	context.es.byte(context.si+17) = context.al;
	context._cmp(context.es.byte(context.si+8), 0);
	if (!context.flags.z()) goto animating;
	context._cmp(context.es.byte(context.si+5), 5);
	if (context.flags.z()) goto animating;
	context._cmp(context.es.byte(context.si+5), 6);
	if (context.flags.z()) goto animating;
	context.ax = context.data.word(kCurrentframe);
	context.ah = 0;
	context._add(context.di, context.data.word(kMapadx));
	context._add(context.bx, context.data.word(kMapady));
	showframe(context);
	goto drawnsetob;
animating:
	makebackob(context);
drawnsetob:
	context.si = context.data.word(kListpos);
	context.es = context.data.word(kBuffers);
	context.al = context.data.byte(kSavex);
	context.ah = context.data.byte(kSavey);
	context.es.word(context.si) = context.ax;
	context.cx = context.ax;
	context.ax = context.data.word(kSavesize);
	context._add(context.al, context.cl);
	context._add(context.ah, context.ch);
	context.es.word(context.si+2) = context.ax;
	context.al = context.data.byte(kCurrentob);
	context.es.byte(context.si+4) = context.al;
	context._add(context.si, 5);
	context.data.word(kListpos) = context.si;
blankframe:
	context._inc(context.data.byte(kCurrentob));
	context.si = context.pop();
	context.cx = context.pop();
	context._add(context.si, 64);
	context._dec(context.cx);
	if (context.flags.z()) goto finishedsetobs;
	goto showobsloop;
finishedsetobs:
	{assert(stack_depth == context.stack.size()); return; }
}

void makebackob(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kNewobs), 0);
	if (context.flags.z()) goto nomake;
	context.al = context.es.byte(context.si+5);
	context.ah = context.es.byte(context.si+8);
	context.push(context.si);
	context.push(context.ax);
	context.push(context.si);
	context.ax = context.data.word(kObjectx);
	context.bx = context.data.word(kObjecty);
	context.ah = context.bl;
	context.si = context.ax;
	context.cx = 49520;
	context.dx = context.data.word(kSetframes);
	context.di = (0);
	makesprite(context);
	context.ax = context.pop();
	context.es.word(context.bx+20) = context.ax;
	context.ax = context.pop();
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto usedpriority;
	context.al = 0;
usedpriority:
	context.es.byte(context.bx+23) = context.al;
	context.es.byte(context.bx+30) = context.ah;
	context.es.byte(context.bx+16) = 0;
	context.es.byte(context.bx+18) = 0;
	context.es.byte(context.bx+19) = 0;
	context.si = context.pop();
nomake:
	{assert(stack_depth == context.stack.size()); return; }
}

void showallfree(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5));
	context.data.word(kListpos) = context.bx;
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5));
	context.cx = 80*5;
	context.al = 255;
	while(context.cx--) 	context._stosb();
	context.es = context.data.word(kFreeframes);
	context.data.word(kFrsegment) = context.es;
	context.ax = (0);
	context.data.word(kDataad) = context.ax;
	context.ax = (0+2080);
	context.data.word(kFramesad) = context.ax;
	context.al = 0;
	context.data.byte(kCurrentfree) = context.al;
	context.ds = context.data.word(kFreedat);
	context.si = 2;
	context.cx = 0;
loop127:
	context.push(context.cx);
	context.push(context.si);
	context.push(context.si);
	context.es = context.data.word(kFreedat);
	getmapad(context);
	context.si = context.pop();
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto over138;
	context.al = context.data.byte(kCurrentfree);
	context.ah = 0;
	context.dx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.dx);
	context.data.word(kCurrentframe) = context.ax;
	context.push(context.es);
	context.push(context.si);
	calcfrframe(context);
	context.es = context.data.word(kMapstore);
	context.ds = context.data.word(kFrsegment);
	finalframe(context);
	context.si = context.pop();
	context.es = context.pop();
	context._cmp(context.cx, 0);
	if (context.flags.z()) goto over138;
	context.ax = context.data.word(kCurrentframe);
	context.ah = 0;
	context._add(context.di, context.data.word(kMapadx));
	context._add(context.bx, context.data.word(kMapady));
	showframe(context);
	context.si = context.data.word(kListpos);
	context.es = context.data.word(kBuffers);
	context.al = context.data.byte(kSavex);
	context.ah = context.data.byte(kSavey);
	context.es.word(context.si) = context.ax;
	context.cx = context.ax;
	context.ax = context.data.word(kSavesize);
	context._add(context.al, context.cl);
	context._add(context.ah, context.ch);
	context.es.word(context.si+2) = context.ax;
	context.ax = context.pop();
	context.cx = context.pop();
	context.push(context.cx);
	context.push(context.ax);
	context.es.byte(context.si+4) = context.cl;
	context._add(context.si, 5);
	context.data.word(kListpos) = context.si;
over138:
	context._inc(context.data.byte(kCurrentfree));
	context.si = context.pop();
	context.cx = context.pop();
	context._add(context.si, 16);
	context._inc(context.cx);
	context._cmp(context.cx, 80);
	if (context.flags.z()) goto finfree;
	goto loop127;
finfree:
	{assert(stack_depth == context.stack.size()); return; }
}

void showallex(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5));
	context.data.word(kListpos) = context.bx;
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5));
	context.cx = 100*5;
	context.al = 255;
	while(context.cx--) 	context._stosb();
	context.es = context.data.word(kExtras);
	context.data.word(kFrsegment) = context.es;
	context.ax = (0);
	context.data.word(kDataad) = context.ax;
	context.ax = (0+2080);
	context.data.word(kFramesad) = context.ax;
	context.data.byte(kCurrentex) = 0;
	context.si = (0+2080+30000)+2;
	context.cx = 0;
exloop:
	context.push(context.cx);
	context.push(context.si);
	context.es = context.data.word(kExtras);
	context.push(context.si);
	context.ch = 0;
	context._cmp(context.es.byte(context.si), 255);
	if (context.flags.z()) goto notinroom;
	context.al = context.es.byte(context.si-2);
	context._cmp(context.al, context.data.byte(kReallocation));
	if (!context.flags.z()) goto notinroom;
	getmapad(context);
notinroom:
	context.si = context.pop();
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto blankex;
	context.al = context.data.byte(kCurrentex);
	context.ah = 0;
	context.dx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.dx);
	context.data.word(kCurrentframe) = context.ax;
	context.push(context.es);
	context.push(context.si);
	calcfrframe(context);
	context.es = context.data.word(kMapstore);
	context.ds = context.data.word(kFrsegment);
	finalframe(context);
	context.si = context.pop();
	context.es = context.pop();
	context._cmp(context.cx, 0);
	if (context.flags.z()) goto blankex;
	context.ax = context.data.word(kCurrentframe);
	context.ah = 0;
	context._add(context.di, context.data.word(kMapadx));
	context._add(context.bx, context.data.word(kMapady));
	showframe(context);
	context.si = context.data.word(kListpos);
	context.es = context.data.word(kBuffers);
	context.al = context.data.byte(kSavex);
	context.ah = context.data.byte(kSavey);
	context.es.word(context.si) = context.ax;
	context.cx = context.ax;
	context.ax = context.data.word(kSavesize);
	context._add(context.al, context.cl);
	context._add(context.ah, context.ch);
	context.es.word(context.si+2) = context.ax;
	context.ax = context.pop();
	context.cx = context.pop();
	context.push(context.cx);
	context.push(context.ax);
	context.es.byte(context.si+4) = context.cl;
	context._add(context.si, 5);
	context.data.word(kListpos) = context.si;
blankex:
	context._inc(context.data.byte(kCurrentex));
	context.si = context.pop();
	context.cx = context.pop();
	context._add(context.si, 16);
	context._inc(context.cx);
	context._cmp(context.cx, 100);
	if (context.flags.z()) goto finex;
	goto exloop;
finex:
	{assert(stack_depth == context.stack.size()); return; }
}

void calcfrframe(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = context.data.word(kFrsegment);
	context.ax = context.data.word(kFramesad);
	context.push(context.ax);
	context.cx = context.data.word(kDataad);
	context.ax = context.data.word(kCurrentframe);
	context.ds = context.dx;
	context.bx = 6;
	context._mul(context.bx);
	context._add(context.ax, context.cx);
	context.bx = context.ax;
	context.cx = context.ds.word(context.bx);
	context.ax = context.ds.word(context.bx+2);
	context.dx = context.ds.word(context.bx+4);
	context.bx = context.pop();
	context.push(context.dx);
	context._add(context.ax, context.bx);
	context.data.word(kSavesource) = context.ax;
	context.data.word(kSavesize) = context.cx;
	context.ax = context.pop();
	context.push(context.ax);
	context.ah = 0;
	context.data.word(kOffsetx) = context.ax;
	context.ax = context.pop();
	context.al = context.ah;
	context.ah = 0;
	context.data.word(kOffsety) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
nullframe:
	context.ax = context.pop();
	context.cx = 0;
	context.data.word(kSavesize) = context.cx;
	{assert(stack_depth == context.stack.size()); return; }
}

void finalframe(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kObjecty);
	context._add(context.ax, context.data.word(kOffsety));
	context.bx = context.data.word(kObjectx);
	context._add(context.bx, context.data.word(kOffsetx));
	context.data.byte(kSavex) = context.bl;
	context.data.byte(kSavey) = context.al;
	context.di = context.data.word(kObjectx);
	context.bx = context.data.word(kObjecty);
	{assert(stack_depth == context.stack.size()); return; }
}

void adjustlen(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = context.al;
	context._add(context.al, context.ch);
	context._cmp(context.al, 100);
	if (context.flags.c()) goto over242;
	context.al = 224;
	context._sub(context.al, context.ch);
	context.ch = context.al;
over242:
	{assert(stack_depth == context.stack.size()); return; }
}

void getmapad(Context & context) {
	uint stack_depth = context.stack.size();
	getxad(context);
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto over146;
	context.data.word(kObjectx) = context.ax;
	getyad(context);
	context._cmp(context.ch, 0);
	if (context.flags.z()) goto over146;
	context.data.word(kObjecty) = context.ax;
	context.ch = 1;
over146:
	{assert(stack_depth == context.stack.size()); return; }
}

void getxad(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = context.es.byte(context.si);
	context._inc(context.si);
	context.al = context.es.byte(context.si);
	context._inc(context.si);
	context.ah = context.es.byte(context.si);
	context._inc(context.si);
	context._cmp(context.cl, 0);
	if (!context.flags.z()) goto over148;
	context._sub(context.al, context.data.byte(kMapx));
	if (context.flags.c()) goto over148;
	context._cmp(context.al, 11);
	if (!context.flags.c()) goto over148;
	context.cl = 4;
	context._shl(context.al, context.cl);
	context._or(context.al, context.ah);
	context.ah = 0;
	context.ch = 1;
	{assert(stack_depth == context.stack.size()); return; }
over148:
	context.ch = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void getyad(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.es.byte(context.si);
	context._inc(context.si);
	context.ah = context.es.byte(context.si);
	context._inc(context.si);
	context._sub(context.al, context.data.byte(kMapy));
	if (context.flags.c()) goto over147;
	context._cmp(context.al, 10);
	if (!context.flags.c()) goto over147;
	context.cl = 4;
	context._shl(context.al, context.cl);
	context._or(context.al, context.ah);
	context.ah = 0;
	context.ch = 1;
	{assert(stack_depth == context.stack.size()); return; }
over147:
	context.ch = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void autolook(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kMousex);
	context._cmp(context.ax, context.data.word(kOldx));
	if (!context.flags.z()) goto diffmouse;
	context.ax = context.data.word(kMousey);
	context._cmp(context.ax, context.data.word(kOldy));
	if (!context.flags.z()) goto diffmouse;
	context._dec(context.data.word(kLookcounter));
	context._cmp(context.data.word(kLookcounter), 0);
	if (!context.flags.z()) goto noautolook;
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) goto noautolook;
	dolook(context);
noautolook:
	{assert(stack_depth == context.stack.size()); return; }
diffmouse:
	context.data.word(kLookcounter) = 1000;
	{assert(stack_depth == context.stack.size()); return; }
}

void look(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kPointermode), 2);
	if (context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kCommandtype), 241);
	if (context.flags.z()) goto alreadylook;
	context.data.byte(kCommandtype) = 241;
	context.al = 25;
	commandonly(context);
alreadylook:
	context._cmp(context.data.word(kMousebutton), 1);
	if (!context.flags.z()) goto nolook;
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto nolook;
	dolook(context);
nolook:
	{assert(stack_depth == context.stack.size()); return; }
}

void dolook(Context & context) {
	uint stack_depth = context.stack.size();
	createpanel(context);
	showicon(context);
	undertextline(context);
	worktoscreenm(context);
	context.data.byte(kCommandtype) = 255;
	dumptextline(context);
	context.bl = context.data.byte(kRoomnum);
	context._and(context.bl, 31);
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(kRoomdesc);
	context._add(context.bx, (0));
	context.si = context.es.word(context.bx);
	context._add(context.si, (0+(38*2)));
	findnextcolon(context);
	context.di = 66;
	context._cmp(context.data.byte(kReallocation), 50);
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
	context.data.byte(kPointermode) = 0;
	context.data.byte(kCommandtype) = 0;
	redrawmainscrn(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void redrawmainscrn(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.word(kTimecount) = 0;
	createpanel(context);
	context.data.byte(kNewobs) = 0;
	drawfloor(context);
	printsprites(context);
	reelsonscreen(context);
	showicon(context);
	getunderzoom(context);
	undertextline(context);
	readmouse(context);
	context.data.byte(kCommandtype) = 255;
	{assert(stack_depth == context.stack.size()); return; }
}

void getback1(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kPickup), 0);
	if (context.flags.z()) goto notgotobject;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
notgotobject:
	context._cmp(context.data.byte(kCommandtype), 202);
	if (context.flags.z()) goto alreadyget;
	context.data.byte(kCommandtype) = 202;
	context.al = 26;
	commandonly(context);
alreadyget:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto nogetback;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dogetback;
nogetback:
	{assert(stack_depth == context.stack.size()); return; }
dogetback:
	context.data.byte(kGetback) = 1;
	context.data.byte(kPickup) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void talk(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kTalkpos) = 0;
	context.data.byte(kInmaparea) = 0;
	context.al = context.data.byte(kCommand);
	context.data.byte(kCharacter) = context.al;
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	undertextline(context);
	convicons(context);
	starttalk(context);
	context.data.byte(kCommandtype) = 255;
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
	context.data.byte(kGetback) = 0;
	context.bx = 2660;
	checkcoords(context);
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto waittalk;
finishtalk:
	context.bx = context.data.word(kPersondata);
	context.es = context.cs;
	context._cmp(context.data.byte(kTalkpos), 4);
	if (context.flags.c()) goto notnexttalk;
	context.al = context.es.byte(context.bx+7);
	context._or(context.al, 128);
	context.es.byte(context.bx+7) = context.al;
notnexttalk:
	redrawmainscrn(context);
	worktoscreenm(context);
	context._cmp(context.data.byte(kSpeechloaded), 1);
	if (!context.flags.z()) goto nospeech;
	cancelch1(context);
	context.data.byte(kVolumedirection) = -1;
	context.data.byte(kVolumeto) = 0;
nospeech:
	{assert(stack_depth == context.stack.size()); return; }
}

void convicons(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kCharacter);
	context._and(context.al, 127);
	getpersframe(context);
	context.di = 234;
	context.bx = 2;
	context.data.word(kCurrentframe) = context.ax;
	findsource(context);
	context.ax = context.data.word(kCurrentframe);
	context._sub(context.ax, context.data.word(kTakeoff));
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getpersframe(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.es = context.data.word(kPeople);
	context._add(context.bx, (0));
	context.ax = context.es.word(context.bx);
	{assert(stack_depth == context.stack.size()); return; }
}

void starttalk(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kTalkmode) = 0;
	context.al = context.data.byte(kCharacter);
	context._and(context.al, 127);
	getpersontext(context);
	context.data.word(kCharshift) = 91+91;
	context.di = 66;
	context.bx = 64;
	context.dl = 241;
	context.al = 0;
	context.ah = 79;
	printdirect(context);
	context.data.word(kCharshift) = 0;
	context.di = 66;
	context.bx = 80;
	context.dl = 241;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
nospeech1:
	{assert(stack_depth == context.stack.size()); return; }
}

void getpersontext(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 0;
	context.cx = 64*2;
	context._mul(context.cx);
	context.si = context.ax;
	context.es = context.data.word(kPeople);
	context._add(context.si, (0+24));
	context.cx = (0+24+(1026*2));
	context.ax = context.es.word(context.si);
	context._add(context.ax, context.cx);
	context.si = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void moretalk(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kTalkmode), 0);
	if (context.flags.z()) goto canmore;
	redes(context);
	{assert(stack_depth == context.stack.size()); return; }
canmore:
	context._cmp(context.data.byte(kCommandtype), 215);
	if (context.flags.z()) goto alreadymore;
	context.data.byte(kCommandtype) = 215;
	context.al = 49;
	commandonly(context);
alreadymore:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto nomore;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto domoretalk;
nomore:
	{assert(stack_depth == context.stack.size()); return; }
domoretalk:
	context.data.byte(kTalkmode) = 2;
	context.data.byte(kTalkpos) = 4;
	context._cmp(context.data.byte(kCharacter), 100);
	if (context.flags.c()) goto notsecondpart;
	context.data.byte(kTalkpos) = 48;
notsecondpart:
	dosometalk(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void dosometalk(Context & context) {
	uint stack_depth = context.stack.size();
watchtalk:
	context.al = context.data.byte(kTalkpos);
	context.al = context.data.byte(kCharacter);
	context._and(context.al, 127);
	context.ah = 0;
	context.cx = 64;
	context._mul(context.cx);
	context.cx = context.ax;
	context.al = context.data.byte(kTalkpos);
	context.ah = 0;
	context._add(context.ax, context.cx);
	context._add(context.ax, context.ax);
	context.si = context.ax;
	context.es = context.data.word(kPeople);
	context._add(context.si, (0+24));
	context.cx = (0+24+(1026*2));
	context.ax = context.es.word(context.si);
	context._add(context.ax, context.cx);
	context.si = context.ax;
	context._cmp(context.es.byte(context.si), 0);
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
	context.data.byte(kPointermode) = 3;
	worktoscreenm(context);
	context.cx = 180;
	hangonpq(context);
	context._inc(context.data.byte(kTalkpos));
	context.al = context.data.byte(kTalkpos);
	context.al = context.data.byte(kCharacter);
	context._and(context.al, 127);
	context.ah = 0;
	context.cx = 64;
	context._mul(context.cx);
	context.cx = context.ax;
	context.al = context.data.byte(kTalkpos);
	context.ah = 0;
	context._add(context.ax, context.cx);
	context._add(context.ax, context.ax);
	context.si = context.ax;
	context.es = context.data.word(kPeople);
	context._add(context.si, (0+24));
	context.cx = (0+24+(1026*2));
	context.ax = context.es.word(context.si);
	context._add(context.ax, context.cx);
	context.si = context.ax;
	context._cmp(context.es.byte(context.si), 0);
	if (context.flags.z()) goto endwatchtalk;
	context._cmp(context.es.byte(context.si), ':');
	if (context.flags.z()) goto skiptalk;
	context._cmp(context.es.byte(context.si), 32);
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
	context.data.byte(kPointermode) = 3;
	worktoscreenm(context);
	context.cx = 180;
	hangonpq(context);
skiptalk:
	context._inc(context.data.byte(kTalkpos));
	goto watchtalk;
endwatchtalk:
	context.data.byte(kPointermode) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void hangonpq(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kGetback) = 0;
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
	context._cmp(context.data.byte(kGetback), 1);
	if (context.flags.z()) goto quitconv;
	context._cmp(context.data.byte(kSpeechloaded), 1);
	if (!context.flags.z()) goto notspeaking;
	context._cmp(context.data.byte(kCh1playing), 255);
	if (!context.flags.z()) goto notspeaking;
	context._inc(context.bx);
	context._cmp(context.bx, 40);
	if (context.flags.z()) goto finishconv;
notspeaking:
	context._cmp(context.data.word(kMousebutton), 0);
	if (context.flags.z()) goto hangloopq;
	context._cmp(context.data.word(kOldbutton), 0);
	if (!context.flags.z()) goto hangloopq;
finishconv:
	delpointer(context);
	context.data.byte(kPointermode) = 0;
	{assert(stack_depth == context.stack.size()); return; }
quitconv:
	delpointer(context);
	context.data.byte(kPointermode) = 0;
	context.ax = context.pop();
	cancelch1(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void redes(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCh1playing), 255);
	if (!context.flags.z()) goto cantredes;
	context._cmp(context.data.byte(kTalkmode), 2);
	if (context.flags.z()) goto canredes;
cantredes:
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
canredes:
	context._cmp(context.data.byte(kCommandtype), 217);
	if (context.flags.z()) goto alreadyreds;
	context.data.byte(kCommandtype) = 217;
	context.al = 50;
	commandonly(context);
alreadyreds:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doredes;
	{assert(stack_depth == context.stack.size()); return; }
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
	{assert(stack_depth == context.stack.size()); return; }
}

void newplace(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kNeedtotravel), 1);
	if (context.flags.z()) goto istravel;
	context._cmp(context.data.byte(kAutolocation), -1);
	if (!context.flags.z()) goto isautoloc;
	{assert(stack_depth == context.stack.size()); return; }
isautoloc:
	context.al = context.data.byte(kAutolocation);
	context.data.byte(kNewlocation) = context.al;
	context.data.byte(kAutolocation) = -1;
	{assert(stack_depth == context.stack.size()); return; }
istravel:
	context.data.byte(kNeedtotravel) = 0;
	selectlocation(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void selectlocation(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kInmaparea) = 0;
	clearbeforeload(context);
	context.data.byte(kGetback) = 0;
	context.data.byte(kPointerframe) = 22;
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
	context.data.byte(kCommandtype) = 255;
	readmouse(context);
	context.data.byte(kPointerframe) = 0;
	showpointer(context);
	worktoscreen(context);
	context.al = 9;
	context.ah = 255;
	playchannel0(context);
	context.data.byte(kNewlocation) = 255;
select:
	delpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context._cmp(context.data.byte(kGetback), 1);
	if (context.flags.z()) goto quittravel;
	context.bx = 2714;
	checkcoords(context);
	context._cmp(context.data.byte(kNewlocation), 255);
	if (context.flags.z()) goto select;
	context.al = context.data.byte(kNewlocation);
	context._cmp(context.al, context.data.byte(kLocation));
	if (context.flags.z()) goto quittravel;
	getridoftemp(context);
	getridoftemp2(context);
	getridoftemp3(context);
	context.es = context.data.word(kTraveltext);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
quittravel:
	context.al = context.data.byte(kReallocation);
	context.data.byte(kNewlocation) = context.al;
	context.data.byte(kGetback) = 0;
	getridoftemp(context);
	getridoftemp2(context);
	getridoftemp3(context);
	context.es = context.data.word(kTraveltext);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showcity(Context & context) {
	uint stack_depth = context.stack.size();
	clearwork(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = 57;
	context.bx = 32;
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = 120+57;
	context.bx = 32;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void lookatplace(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 224);
	if (context.flags.z()) goto alreadyinfo;
	context.data.byte(kCommandtype) = 224;
	context.al = 27;
	commandonly(context);
alreadyinfo:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (context.flags.z()) goto noinfo;
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto noinfo;
	context.bl = context.data.byte(kDestpos);
	context._cmp(context.bl, 15);
	if (!context.flags.c()) goto noinfo;
	context.push(context.bx);
	delpointer(context);
	deltextline(context);
	getundercentre(context);
	context.ds = context.data.word(kTempgraphics3);
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
	context.bx = context.pop();
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(kTraveltext);
	context.si = context.es.word(context.bx);
	context._add(context.si, (66*2));
	findnextcolon(context);
	context.di = 63;
	context.bx = 84;
	context.dl = 191;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	worktoscreenm(context);
	context.cx = 500;
	hangonp(context);
afterinfo:
	context.data.byte(kPointermode) = 0;
	context.data.byte(kPointerframe) = 0;
	putundercentre(context);
	worktoscreenm(context);
noinfo:
	{assert(stack_depth == context.stack.size()); return; }
}

void getundercentre(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 58;
	context.bx = 72;
	context.ds = context.data.word(kMapstore);
	context.si = 0;
	context.cl = 254;
	context.ch = 110;
	multiget(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void putundercentre(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 58;
	context.bx = 72;
	context.ds = context.data.word(kMapstore);
	context.si = 0;
	context.cl = 254;
	context.ch = 110;
	multiput(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void locationpic(Context & context) {
	uint stack_depth = context.stack.size();
	getdestinfo(context);
	context.al = context.es.byte(context.si);
	context.push(context.es);
	context.push(context.si);
	context.di = 0;
	context._cmp(context.al, 6);
	if (!context.flags.c()) goto secondlot;
	context.ds = context.data.word(kTempgraphics);
	context._add(context.al, 4);
	goto gotgraphic;
secondlot:
	context._sub(context.al, 6);
	context.ds = context.data.word(kTempgraphics2);
gotgraphic:
	context._add(context.di, 104);
	context.bx = 138+14;
	context.ah = 0;
	showframe(context);
	context.si = context.pop();
	context.es = context.pop();
	context.al = context.data.byte(kDestpos);
	context._cmp(context.al, context.data.byte(kReallocation));
	if (!context.flags.z()) goto notinthisone;
	context.al = 3;
	context.di = 104;
	context.bx = 140+14;
	context.ds = context.data.word(kTempgraphics);
	context.ah = 0;
	showframe(context);
notinthisone:
	context.bl = context.data.byte(kDestpos);
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(kTraveltext);
	context.si = context.es.word(context.bx);
	context._add(context.si, (66*2));
	context.di = 50;
	context.bx = 20;
	context.dl = 241;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getdestinfo(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kDestpos);
	context.ah = 0;
	context.push(context.ax);
	context.dx = context.data;
	context.es = context.dx;
	context.si = 7782;
	context._add(context.si, context.ax);
	context.cl = context.es.byte(context.si);
	context.ax = context.pop();
	context.push(context.cx);
	context.dx = context.data;
	context.es = context.dx;
	context.si = 7798;
	context._add(context.si, context.ax);
	context.ax = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void showarrows(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 116-12;
	context.bx = 16;
	context.ds = context.data.word(kTempgraphics);
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.di = 226+12;
	context.bx = 16;
	context.ds = context.data.word(kTempgraphics);
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.di = 280;
	context.bx = 14;
	context.ds = context.data.word(kTempgraphics);
	context.al = 2;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void nextdest(Context & context) {
	uint stack_depth = context.stack.size();
duok:
	context._cmp(context.data.byte(kCommandtype), 218);
	if (context.flags.z()) goto alreadydu;
	context.data.byte(kCommandtype) = 218;
	context.al = 28;
	commandonly(context);
alreadydu:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (context.flags.z()) goto nodu;
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto nodu;
searchdestup:
	context._inc(context.data.byte(kDestpos));
	context._cmp(context.data.byte(kDestpos), 15);
	if (!context.flags.z()) goto notlastdest;
	context.data.byte(kDestpos) = 0;
notlastdest:
	getdestinfo(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto searchdestup;
	context.data.byte(kNewtextline) = 1;
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
	{assert(stack_depth == context.stack.size()); return; }
}

void lastdest(Context & context) {
	uint stack_depth = context.stack.size();
ddok:
	context._cmp(context.data.byte(kCommandtype), 219);
	if (context.flags.z()) goto alreadydd;
	context.data.byte(kCommandtype) = 219;
	context.al = 29;
	commandonly(context);
alreadydd:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (context.flags.z()) goto nodd;
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto nodd;
searchdestdown:
	context._dec(context.data.byte(kDestpos));
	context._cmp(context.data.byte(kDestpos), -1);
	if (!context.flags.z()) goto notfirstdest;
	context.data.byte(kDestpos) = 15;
notfirstdest:
	getdestinfo(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto searchdestdown;
	context.data.byte(kNewtextline) = 1;
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
	{assert(stack_depth == context.stack.size()); return; }
}

void destselect(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 222);
	if (context.flags.z()) goto alreadytrav;
	context.data.byte(kCommandtype) = 222;
	context.al = 30;
	commandonly(context);
alreadytrav:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (context.flags.z()) goto notrav;
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notrav;
	getdestinfo(context);
	context.al = context.data.byte(kDestpos);
	context.data.byte(kNewlocation) = context.al;
notrav:
	{assert(stack_depth == context.stack.size()); return; }
}

void getlocation(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 0;
	context.bx = context.ax;
	context.dx = context.data;
	context.es = context.dx;
	context._add(context.bx, 7782);
	context.al = context.es.byte(context.bx);
	{assert(stack_depth == context.stack.size()); return; }
}

void setlocation(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 0;
	context.bx = context.ax;
	context.dx = context.data;
	context.es = context.dx;
	context._add(context.bx, 7782);
	context.es.byte(context.bx) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void resetlocation(Context & context) {
	uint stack_depth = context.stack.size();
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
	context.dx = context.data;
	context.es = context.dx;
	context._add(context.bx, 7782);
	context.es.byte(context.bx) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void readdesticon(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 2013;
	loadintotemp(context);
	context.dx = 2026;
	loadintotemp2(context);
	context.dx = 1961;
	loadintotemp3(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void readcitypic(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 2000;
	loadintotemp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usemon(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kLasttrigger) = 0;
	context.es = context.cs;
	context.di = 2892+1;
	context.cx = 12;
	context.al = 32;
	while(context.cx--) 	context._stosb();
	context.es = context.cs;
	context.di = 2883+1;
	context.cx = 12;
	context.al = 32;
	while(context.cx--) 	context._stosb();
	context.es = context.cs;
	context.di = 2807;
	context.es.byte(context.di) = 1;
	context._add(context.di, 26);
	context.cx = 3;
keyloop:
	context.es.byte(context.di) = 0;
	context._add(context.di, 26);
	if (--context.cx) goto keyloop;
	createpanel(context);
	showpanel(context);
	showicon(context);
	drawfloor(context);
	getridofall(context);
	context.dx = 1974;
	loadintotemp(context);
	loadpersonal(context);
	loadnews(context);
	loadcart(context);
	context.dx = 1870;
	loadtempcharset(context);
	printoutermon(context);
	initialmoncols(context);
	printlogo(context);
	worktoscreen(context);
	turnonpower(context);
	fadeupyellows(context);
	fadeupmonfirst(context);
	context.data.word(kMonadx) = 76;
	context.data.word(kMonady) = 141;
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
	context.data.word(kBufferin) = 0;
	context.data.word(kBufferout) = 0;
moreinput:
	context.di = context.data.word(kMonadx);
	context.bx = context.data.word(kMonady);
	context.push(context.di);
	context.push(context.bx);
	input(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.data.word(kMonadx) = context.di;
	context.data.word(kMonady) = context.bx;
	execcommand(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto moreinput;
endmon:
	getridoftemp(context);
	getridoftempcharset(context);
	context.es = context.data.word(kTextfile1);
	deallocatemem(context);
	context.es = context.data.word(kTextfile2);
	deallocatemem(context);
	context.es = context.data.word(kTextfile3);
	deallocatemem(context);
	context.data.byte(kGetback) = 1;
	context.al = 26;
	playchannel1(context);
	context.data.byte(kManisoffscreen) = 0;
	restoreall(context);
	redrawmainscrn(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void printoutermon(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 40;
	context.bx = 32;
	context.ds = context.data.word(kTempgraphics);
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.di = 264;
	context.bx = 32;
	context.ds = context.data.word(kTempgraphics);
	context.al = 2;
	context.ah = 0;
	showframe(context);
	context.di = 40;
	context.bx = 12;
	context.ds = context.data.word(kTempgraphics);
	context.al = 3;
	context.ah = 0;
	showframe(context);
	context.di = 40;
	context.bx = 164;
	context.ds = context.data.word(kTempgraphics);
	context.al = 4;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void loadpersonal(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kLocation);
	context.dx = 2052;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundpersonal;
	context._cmp(context.al, 42);
	if (context.flags.z()) goto foundpersonal;
	context.dx = 2065;
	context._cmp(context.al, 2);
	if (context.flags.z()) goto foundpersonal;
foundpersonal:
	openfile(context);
	readheader(context);
	context.bx = context.es.word(context.di);
	context.push(context.bx);
	context.cl = 4;
	context._shr(context.bx, context.cl);
	allocatemem(context);
	context.data.word(kTextfile1) = context.ax;
	context.ds = context.ax;
	context.cx = context.pop();
	context.dx = 0;
	readfromfile(context);
	closefile(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void loadnews(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kNewsitem);
	context.dx = 2078;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundnews;
	context.dx = 2091;
	context._cmp(context.al, 1);
	if (context.flags.z()) goto foundnews;
	context.dx = 2104;
	context._cmp(context.al, 2);
	if (context.flags.z()) goto foundnews;
	context.dx = 2117;
foundnews:
	openfile(context);
	readheader(context);
	context.bx = context.es.word(context.di);
	context.push(context.bx);
	context.cl = 4;
	context._shr(context.bx, context.cl);
	allocatemem(context);
	context.data.word(kTextfile2) = context.ax;
	context.ds = context.ax;
	context.cx = context.pop();
	context.dx = 0;
	readfromfile(context);
	closefile(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void loadcart(Context & context) {
	uint stack_depth = context.stack.size();
	lookininterface(context);
	context.dx = 2130;
	context._cmp(context.al, 0);
	if (context.flags.z()) goto gotcart;
	context.dx = 2143;
	context._cmp(context.al, 1);
	if (context.flags.z()) goto gotcart;
	context.dx = 2156;
	context._cmp(context.al, 2);
	if (context.flags.z()) goto gotcart;
	context.dx = 2169;
	context._cmp(context.al, 3);
	if (context.flags.z()) goto gotcart;
	context.dx = 2182;
gotcart:
	openfile(context);
	readheader(context);
	context.bx = context.es.word(context.di);
	context.push(context.bx);
	context.cl = 4;
	context._shr(context.bx, context.cl);
	allocatemem(context);
	context.data.word(kTextfile3) = context.ax;
	context.ds = context.ax;
	context.cx = context.pop();
	context.dx = 0;
	readfromfile(context);
	closefile(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void lookininterface(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 'I';
	context.ah = 'N';
	context.cl = 'T';
	context.ch = 'F';
	findsetobject(context);
	context.ah = 1;
	checkinside(context);
	context._cmp(context.cl, (114));
	if (context.flags.z()) goto emptyinterface;
	context.al = context.es.byte(context.bx+15);
	context._inc(context.al);
	{assert(stack_depth == context.stack.size()); return; }
emptyinterface:
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void turnonpower(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void randomaccess(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void powerlighton(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 257+4;
	context.bx = 182;
	context.ds = context.data.word(kTempgraphics);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void powerlightoff(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 257+4;
	context.bx = 182;
	context.ds = context.data.word(kTempgraphics);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void accesslighton(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 74;
	context.bx = 182;
	context.ds = context.data.word(kTempgraphics);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void accesslightoff(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 74;
	context.bx = 182;
	context.ds = context.data.word(kTempgraphics);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void locklighton(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 56;
	context.bx = 182;
	context.ds = context.data.word(kTempgraphics);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void locklightoff(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 56;
	context.bx = 182;
	context.ds = context.data.word(kTempgraphics);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void input(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.cs;
	context.di = 7816;
	context.cx = 64;
	context.al = 0;
	while(context.cx--) 	context._stosb();
	context.data.word(kCurpos) = 0;
	context.al = '>';
	context.di = context.data.word(kMonadx);
	context.bx = context.data.word(kMonady);
	context.ds = context.data.word(kTempcharset);
	context.ah = 0;
	printchar(context);
	context.di = context.data.word(kMonadx);
	context.bx = context.data.word(kMonady);
	context.cl = 6;
	context.ch = 8;
	multidump(context);
	context._add(context.data.word(kMonadx), 6);
	context.ax = context.data.word(kMonadx);
	context.data.word(kCurslocx) = context.ax;
	context.ax = context.data.word(kMonady);
	context.data.word(kCurslocy) = context.ax;
waitkey:
	printcurs(context);
	vsync(context);
	delcurs(context);
	readkey(context);
	context.al = context.data.byte(kCurrentkey);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto waitkey;
	context._cmp(context.al, 13);
	if (context.flags.z()) goto endofinput;
	context._cmp(context.al, 8);
	if (!context.flags.z()) goto notdel;
	context._cmp(context.data.word(kCurpos), 0);
	if (context.flags.z()) goto waitkey;
	delchar(context);
	goto waitkey;
notdel:
	context._cmp(context.data.word(kCurpos), 28);
	if (context.flags.z()) goto waitkey;
	context._cmp(context.data.byte(kCurrentkey), 32);
	if (!context.flags.z()) goto notleadingspace;
	context._cmp(context.data.word(kCurpos), 0);
	if (context.flags.z()) goto waitkey;
notleadingspace:
	makecaps(context);
	context.es = context.cs;
	context.si = context.data.word(kCurpos);
	context._add(context.si, context.si);
	context._add(context.si, 7816);
	context.es.byte(context.si) = context.al;
	context._cmp(context.al, 'Z'+1);
	if (!context.flags.c()) goto waitkey;
	context.push(context.ax);
	context.push(context.es);
	context.push(context.si);
	context.di = context.data.word(kMonadx);
	context.bx = context.data.word(kMonady);
	context.ds = context.data.word(kMapstore);
	context.ax = context.data.word(kCurpos);
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
	context.di = context.data.word(kMonadx);
	context.bx = context.data.word(kMonady);
	context.ds = context.data.word(kTempcharset);
	context.ah = 0;
	printchar(context);
	context.si = context.pop();
	context.es = context.pop();
	context.es.byte(context.si+1) = context.cl;
	context.ch = 0;
	context._add(context.data.word(kMonadx), context.cx);
	context._inc(context.data.word(kCurpos));
	context._add(context.data.word(kCurslocx), context.cx);
	goto waitkey;
endofinput:
	{assert(stack_depth == context.stack.size()); return; }
}

void makecaps(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.al, 'a');
	if (context.flags.c()) goto notupperc;
	context._sub(context.al, 32);
notupperc:
	{assert(stack_depth == context.stack.size()); return; }
}

void delchar(Context & context) {
	uint stack_depth = context.stack.size();
	context._dec(context.data.word(kCurpos));
	context.si = context.data.word(kCurpos);
	context._add(context.si, context.si);
	context.es = context.cs;
	context._add(context.si, 7816);
	context.es.byte(context.si) = 0;
	context.al = context.es.byte(context.si+1);
	context.ah = 0;
	context._sub(context.data.word(kMonadx), context.ax);
	context._sub(context.data.word(kCurslocx), context.ax);
	context.di = context.data.word(kMonadx);
	context.bx = context.data.word(kMonady);
	context.ds = context.data.word(kMapstore);
	context.ax = context.data.word(kCurpos);
	context._xchg(context.al, context.ah);
	context.si = context.ax;
	context.cl = 8;
	context.ch = 8;
	multiput(context);
	context.di = context.data.word(kMonadx);
	context.bx = context.data.word(kMonady);
	context.cl = context.al;
	context.ch = 8;
	multidump(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void execcommand(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.cs;
	context.bx = 2776;
	context.ds = context.cs;
	context.si = 7816;
	context.al = context.ds.byte(context.si);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto notblankinp;
	scrollmonitor(context);
	{assert(stack_depth == context.stack.size()); return; }
notblankinp:
	context.cl = 0;
comloop:
	context.push(context.bx);
	context.push(context.si);
comloop2:
	context.al = context.ds.byte(context.si);
	context._add(context.si, 2);
	context.ah = context.es.byte(context.bx);
	context._inc(context.bx);
	context._cmp(context.ah, 32);
	if (context.flags.z()) goto foundcom;
	context._cmp(context.al, context.ah);
	if (context.flags.z()) goto comloop2;
	context.si = context.pop();
	context.bx = context.pop();
	context._add(context.bx, 10);
	context._inc(context.cl);
	context._cmp(context.cl, 6);
	if (!context.flags.z()) goto comloop;
	neterror(context);
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
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
	{assert(stack_depth == context.stack.size()); return; }
signoncom:
	signon(context);
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
accesscom:
	read(context);
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
keyscom:
	showkeys(context);
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
testcom:
	context.al = 6;
	monmessage(context);
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
quitcom:
	context.al = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void neterror(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 5;
	monmessage(context);
	scrollmonitor(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void dircom(Context & context) {
	uint stack_depth = context.stack.size();
	context.cx = 30;
	randomaccess(context);
	parser(context);
	context._cmp(context.es.byte(context.di+1), 0);
	if (context.flags.z()) goto dirroot;
	dirfile(context);
	{assert(stack_depth == context.stack.size()); return; }
dirroot:
	context.data.byte(kLogonum) = 0;
	context.ds = context.cs;
	context.si = 2885;
	context._inc(context.si);
	context.es = context.cs;
	context.di = 2892;
	context._inc(context.di);
	context.cx = 12;
	while(context.cx--) 	context._movsb();
 	monitorlogo(context);
	scrollmonitor(context);
	context.al = 9;
	monmessage(context);
	context.es = context.data.word(kTextfile1);
	searchforfiles(context);
	context.es = context.data.word(kTextfile2);
	searchforfiles(context);
	context.es = context.data.word(kTextfile3);
	searchforfiles(context);
	scrollmonitor(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void searchforfiles(Context & context) {
	uint stack_depth = context.stack.size();
	context.bx = (66*2);
directloop1:
	context.al = context.es.byte(context.bx);
	context._inc(context.bx);
	context._cmp(context.al, '*');
	if (context.flags.z()) goto endofdir;
	context._cmp(context.al, 34);
	if (!context.flags.z()) goto directloop1;
	monprint(context);
	goto directloop1;
endofdir:
	{assert(stack_depth == context.stack.size()); return; }
}

void signon(Context & context) {
	uint stack_depth = context.stack.size();
	parser(context);
	context._inc(context.di);
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
	context.ah = context.es.byte(context.di);
	context._inc(context.di);
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
	{assert(stack_depth == context.stack.size()); return; }
foundsign:
	context.di = context.pop();
	context.si = context.pop();
	context.cx = context.pop();
	context.bx = context.si;
	context.es = context.ds;
	context._cmp(context.es.byte(context.bx), 0);
	if (context.flags.z()) goto notyetassigned;
	context.al = 17;
	monmessage(context);
	{assert(stack_depth == context.stack.size()); return; }
notyetassigned:
	context.push(context.es);
	context.push(context.bx);
	scrollmonitor(context);
	context.al = 15;
	monmessage(context);
	context.di = context.data.word(kMonadx);
	context.bx = context.data.word(kMonady);
	context.push(context.di);
	context.push(context.bx);
	input(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.data.word(kMonadx) = context.di;
	context.data.word(kMonady) = context.bx;
	context.bx = context.pop();
	context.es = context.pop();
	context.push(context.es);
	context.push(context.bx);
	context._add(context.bx, 2);
	context.ds = context.cs;
	context.si = 7816;
checkpass:
	context._lodsw();
	context.ah = context.es.byte(context.bx);
	context._inc(context.bx);
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
	{assert(stack_depth == context.stack.size()); return; }
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
	context.es.byte(context.bx) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void showkeys(Context & context) {
	uint stack_depth = context.stack.size();
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
	context._cmp(context.es.byte(context.bx), 0);
	if (context.flags.z()) goto notheld;
	context._add(context.bx, 14);
	monprint(context);
notheld:
	context.bx = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 26);
	if (--context.cx) goto keysloop;
	scrollmonitor(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void read(Context & context) {
	uint stack_depth = context.stack.size();
	context.cx = 40;
	randomaccess(context);
	parser(context);
	context._cmp(context.es.byte(context.di+1), 0);
	if (!context.flags.z()) goto okcom;
	neterror(context);
	{assert(stack_depth == context.stack.size()); return; }
okcom:
	context.es = context.cs;
	context.di = 2892;
	context.ax = context.data.word(kTextfile1);
	context.data.word(kMonsource) = context.ax;
	context.ds = context.ax;
	context.si = (66*2);
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile2;
	context.ax = context.data.word(kTextfile2);
	context.data.word(kMonsource) = context.ax;
	context.ds = context.ax;
	context.si = (66*2);
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile2;
	context.ax = context.data.word(kTextfile3);
	context.data.word(kMonsource) = context.ax;
	context.ds = context.ax;
	context.si = (66*2);
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile2;
	context.al = 7;
	monmessage(context);
	{assert(stack_depth == context.stack.size()); return; }
foundfile2:
	getkeyandlogo(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto keyok1;
	{assert(stack_depth == context.stack.size()); return; }
keyok1:
	context.es = context.cs;
	context.di = 2883;
	context.ds = context.data.word(kMonsource);
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto findtopictext;
	context.al = context.data.byte(kOldlogonum);
	context.data.byte(kLogonum) = context.al;
	context.al = 11;
	monmessage(context);
	{assert(stack_depth == context.stack.size()); return; }
findtopictext:
	context._inc(context.bx);
	context.push(context.es);
	context.push(context.bx);
	monitorlogo(context);
	scrollmonitor(context);
	context.bx = context.pop();
	context.es = context.pop();
moretopic:
	monprint(context);
	context.al = context.es.byte(context.bx);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void dirfile(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 34;
	context.es.byte(context.di) = context.al;
	context.push(context.es);
	context.push(context.di);
	context.ds = context.data.word(kTextfile1);
	context.si = (66*2);
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile;
	context.di = context.pop();
	context.es = context.pop();
	context.push(context.es);
	context.push(context.di);
	context.ds = context.data.word(kTextfile2);
	context.si = (66*2);
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile;
	context.di = context.pop();
	context.es = context.pop();
	context.push(context.es);
	context.push(context.di);
	context.ds = context.data.word(kTextfile3);
	context.si = (66*2);
	searchforstring(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto foundfile;
	context.di = context.pop();
	context.es = context.pop();
	context.al = 7;
	monmessage(context);
	{assert(stack_depth == context.stack.size()); return; }
foundfile:
	context.ax = context.pop();
	context.ax = context.pop();
	getkeyandlogo(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto keyok2;
	{assert(stack_depth == context.stack.size()); return; }
keyok2:
	context.push(context.es);
	context.push(context.bx);
	context.ds = context.cs;
	context.si = 2883+1;
	context.es = context.cs;
	context.di = 2892+1;
	context.cx = 12;
	while(context.cx--) 	context._movsb();
 	monitorlogo(context);
	scrollmonitor(context);
	context.al = 10;
	monmessage(context);
	context.bx = context.pop();
	context.es = context.pop();
directloop2:
	context.al = context.es.byte(context.bx);
	context._inc(context.bx);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void getkeyandlogo(Context & context) {
	uint stack_depth = context.stack.size();
	context._inc(context.bx);
	context.al = context.es.byte(context.bx);
	context._sub(context.al, 48);
	context.data.byte(kNewlogonum) = context.al;
	context._add(context.bx, 2);
	context.al = context.es.byte(context.bx);
	context._sub(context.al, 48);
	context.data.byte(kKeynum) = context.al;
	context._inc(context.bx);
	context.push(context.es);
	context.push(context.bx);
	context.al = context.data.byte(kKeynum);
	context.ah = 0;
	context.cx = 26;
	context._mul(context.cx);
	context.es = context.cs;
	context.bx = 2807;
	context._add(context.bx, context.ax);
	context.al = context.es.byte(context.bx);
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
	{assert(stack_depth == context.stack.size()); return; }
keyok:
	context.bx = context.pop();
	context.es = context.pop();
	context.al = context.data.byte(kNewlogonum);
	context.data.byte(kLogonum) = context.al;
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void searchforstring(Context & context) {
	uint stack_depth = context.stack.size();
	context.dl = context.es.byte(context.di);
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
	context.ah = context.es.byte(context.di);
	context._cmp(context.al, context.dl);
	if (!context.flags.z()) goto notbracket;
	context._inc(context.dh);
	context._cmp(context.dh, 2);
	if (context.flags.z()) goto complete;
notbracket:
	context._cmp(context.al, context.ah);
	if (!context.flags.z()) goto restartlook;
	context._inc(context.di);
	goto keeplooking;
complete:
	context.es = context.ds;
	context.al = 0;
	context.bx = context.si;
	{assert(stack_depth == context.stack.size()); return; }
notfound:
	context.al = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void parser(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.cs;
	context.di = 2883;
	context.cx = 13;
	context.al = 0;
	while(context.cx--) 	context._stosb();
	context.di = 2883;
	context.al = '=';
	context._stosb();
	context.ds = context.cs;
	context.si = 7816;
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
	{assert(stack_depth == context.stack.size()); return; }
}

void scrollmonitor(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.dx);
	context.push(context.di);
	context.push(context.si);
	context.push(context.es);
	context.push(context.ds);
	printlogo(context);
	context.di = context.data.word(kMonadx);
	context.bx = context.data.word(kMonady);
	printundermon(context);
	context.ax = context.data.word(kMonady);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void lockmon(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLasthardkey), 57);
	if (!context.flags.z()) goto notlock;
	locklighton(context);
lockloop:
	context._cmp(context.data.byte(kLasthardkey), 57);
	if (context.flags.z()) goto lockloop;
	locklightoff(context);
notlock:
	{assert(stack_depth == context.stack.size()); return; }
}

void monitorlogo(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kLogonum);
	context._cmp(context.al, context.data.byte(kOldlogonum));
	if (context.flags.z()) goto notnewlogo;
	context.data.byte(kOldlogonum) = context.al;
	printlogo(context);
	printundermon(context);
	worktoscreen(context);
	printlogo(context);
	printlogo(context);
	context.al = 26;
	playchannel1(context);
	context.cx = 20;
	randomaccess(context);
	{assert(stack_depth == context.stack.size()); return; }
notnewlogo:
	printlogo(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void printlogo(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 56;
	context.bx = 32;
	context.ds = context.data.word(kTempgraphics);
	context.al = 0;
	context.ah = 0;
	showframe(context);
	showcurrentfile(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showcurrentfile(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 178;
	context.bx = 37;
	context.si = 2892+1;
curfileloop:
	context.al = context.cs.byte(context.si);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto finishfile;
	context._inc(context.si);
	context.push(context.si);
	context.ds = context.data.word(kTempcharset);
	context.ah = 0;
	printchar(context);
	context.si = context.pop();
	goto curfileloop;
finishfile:
	{assert(stack_depth == context.stack.size()); return; }
}

void monmessage(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kTextfile1);
	context.bx = (66*2);
	context.cl = context.al;
	context.ch = 0;
monmessageloop:
	context.al = context.es.byte(context.bx);
	context._inc(context.bx);
	context._cmp(context.al, '+');
	if (!context.flags.z()) goto monmessageloop;
	if (--context.cx) goto monmessageloop;
	monprint(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void processtrigger(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLasttrigger), '1');
	if (!context.flags.z()) goto notfirsttrigger;
	context.al = 8;
	setlocation(context);
	context.al = 45;
	triggermessage(context);
	{assert(stack_depth == context.stack.size()); return; }
notfirsttrigger:
	context._cmp(context.data.byte(kLasttrigger), '2');
	if (!context.flags.z()) goto notsecondtrigger;
	context.al = 9;
	setlocation(context);
	context.al = 55;
	triggermessage(context);
	{assert(stack_depth == context.stack.size()); return; }
notsecondtrigger:
	context._cmp(context.data.byte(kLasttrigger), '3');
	if (!context.flags.z()) goto notthirdtrigger;
	context.al = 2;
	setlocation(context);
	context.al = 59;
	triggermessage(context);
notthirdtrigger:
	{assert(stack_depth == context.stack.size()); return; }
}

void triggermessage(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context.di = 174;
	context.bx = 153;
	context.cl = 200;
	context.ch = 63;
	context.ds = context.data.word(kMapstore);
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
	context.ds = context.data.word(kMapstore);
	context.si = 0;
	multiput(context);
	worktoscreen(context);
	context.data.byte(kLasttrigger) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void printcurs(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.si);
	context.push(context.di);
	context.push(context.ds);
	context.push(context.dx);
	context.push(context.bx);
	context.push(context.es);
	context.di = context.data.word(kCurslocx);
	context.bx = context.data.word(kCurslocy);
	context.cl = 6;
	context.ch = 8;
	context.ds = context.data.word(kBuffers);
	context.si = (0);
	context.push(context.di);
	context.push(context.bx);
	multiget(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.push(context.bx);
	context.push(context.di);
	context._inc(context.data.word(kMaintimer));
	context.ax = context.data.word(kMaintimer);
	context._and(context.al, 16);
	if (!context.flags.z()) goto flashcurs;
	context.al = '/';
	context._sub(context.al, 32);
	context.ah = 0;
	context.ds = context.data.word(kTempcharset);
	showframe(context);
flashcurs:
	context.di = context.pop();
	context.bx = context.pop();
	context._sub(context.di, 6);
	context.cl = 12;
	context.ch = 8;
	multidump(context);
	context.es = context.pop();
	context.bx = context.pop();
	context.dx = context.pop();
	context.ds = context.pop();
	context.di = context.pop();
	context.si = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void delcurs(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	context.push(context.di);
	context.push(context.ds);
	context.push(context.dx);
	context.push(context.si);
	context.di = context.data.word(kCurslocx);
	context.bx = context.data.word(kCurslocy);
	context.cl = 6;
	context.ch = 8;
	context.push(context.di);
	context.push(context.bx);
	context.push(context.cx);
	context.ds = context.data.word(kBuffers);
	context.si = (0);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void useobject(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kWithobject) = 255;
	context._cmp(context.data.byte(kCommandtype), 229);
	if (context.flags.z()) goto alreadyuse;
	context.data.byte(kCommandtype) = 229;
	context.bl = context.data.byte(kCommand);
	context.bh = context.data.byte(kObjecttype);
	context.al = 51;
	commandwithob(context);
alreadyuse:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto nouse;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto douse;
nouse:
	{assert(stack_depth == context.stack.size()); return; }
douse:
	useroutine(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void useroutine(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kReallocation), 50);
	if (context.flags.c()) goto nodream7;
	context._cmp(context.data.byte(kPointerpower), 0);
	if (!context.flags.z()) goto powerok;
	{assert(stack_depth == context.stack.size()); return; }
powerok:
	context.data.byte(kPointerpower) = 0;
nodream7:
	getanyad(context);
	context.dx = context.data;
	context.ds = context.dx;
	context.si = 2895;
checkuselist:
	context.push(context.si);
	context._lodsb();
	context._sub(context.al, 'A');
	context._cmp(context.al, context.es.byte(context.bx+12));
	if (!context.flags.z()) goto failed;
	context._lodsb();
	context._sub(context.al, 'A');
	context._cmp(context.al, context.es.byte(context.bx+13));
	if (!context.flags.z()) goto failed;
	context._lodsb();
	context._sub(context.al, 'A');
	context._cmp(context.al, context.es.byte(context.bx+14));
	if (!context.flags.z()) goto failed;
	context._lodsb();
	context._sub(context.al, 'A');
	context._cmp(context.al, context.es.byte(context.bx+15));
	if (!context.flags.z()) goto failed;
	context._lodsw();
	context.si = context.pop();
	__dispatch_call(context, context.ax);
	{assert(stack_depth == context.stack.size()); return; }
failed:
	context.si = context.pop();
	context._add(context.si, 6);
	context._cmp(context.ds.byte(context.si), 140);
	if (!context.flags.z()) goto checkuselist;
	delpointer(context);
	getobtextstart(context);
	findnextcolon(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto cantuse2;
	findnextcolon(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto cantuse2;
	context.al = context.es.byte(context.si);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto cantuse2;
	usetext(context);
	context.cx = 400;
	hangonp(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
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
	context.data.byte(kCommandtype) = 255;
	{assert(stack_depth == context.stack.size()); return; }
}

void wheelsound(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 17;
	playchannel1(context);
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void runtap(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto tapwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
tapwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto fillcupfromtap;
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
fillcupfromtap:
	context.al = context.data.byte(kWithobject);
	getexad(context);
	context.es.byte(context.bx+15) = 'F'-'A';
	context.al = 8;
	playchannel1(context);
	context.cx = 300;
	context.al = 57;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
cupfromtapfull:
	context.cx = 300;
	context.al = 58;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void playguitar(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 14;
	playchannel1(context);
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void hotelcontrol(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kReallocation), 21);
	if (!context.flags.z()) goto notrightcont;
	context._cmp(context.data.byte(kMapx), 33);
	if (!context.flags.z()) goto notrightcont;
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
notrightcont:
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void hotelbell(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 12;
	playchannel1(context);
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void opentomb(Context & context) {
	uint stack_depth = context.stack.size();
	context._inc(context.data.byte(kProgresspoints));
	showfirstuse(context);
	context.data.word(kWatchingtime) = 35*2;
	context.data.word(kReeltowatch) = 1;
	context.data.word(kEndwatchreel) = 33;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usetrainer(Context & context) {
	uint stack_depth = context.stack.size();
	getanyad(context);
	context._cmp(context.es.byte(context.bx+2), 4);
	if (!context.flags.z()) goto notheldtrainer;
	context._inc(context.data.byte(kProgresspoints));
	makeworn(context);
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
notheldtrainer:
	nothelderror(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void nothelderror(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void usepipe(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto pipewith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
pipewith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto fillcup;
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
fillcup:
	context.cx = 300;
	context.al = 36;
	showpuztext(context);
	putbackobstuff(context);
	context.al = context.data.byte(kWithobject);
	getexad(context);
	context.es.byte(context.bx+15) = 'F'-'A';
	{assert(stack_depth == context.stack.size()); return; }
alreadyfull:
	context.cx = 300;
	context.al = 35;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usefullcart(Context & context) {
	uint stack_depth = context.stack.size();
	context._inc(context.data.byte(kProgresspoints));
	context.al = 2;
	context.ah = context.data.byte(kRoomnum);
	context._add(context.ah, 6);
	turnanypathon(context);
	context.data.byte(kManspath) = 4;
	context.data.byte(kFacing) = 4;
	context.data.byte(kTurntoface) = 4;
	context.data.byte(kFinaldest) = 4;
	findxyfrompath(context);
	context.data.byte(kResetmanxy) = 1;
	showfirstuse(context);
	context.data.word(kWatchingtime) = 72*2;
	context.data.word(kReeltowatch) = 58;
	context.data.word(kEndwatchreel) = 142;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void useplinth(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto plinthwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
plinthwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'D';
	context.ch = 'K';
	context.dl = 'E';
	context.dh = 'Y';
	compare(context);
	if (context.flags.z()) goto isrightkey;
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
isrightkey:
	context._inc(context.data.byte(kProgresspoints));
	showseconduse(context);
	context.data.word(kWatchingtime) = 220;
	context.data.word(kReeltowatch) = 0;
	context.data.word(kEndwatchreel) = 104;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	context.al = context.data.byte(kRoomafterdream);
	context.data.byte(kNewlocation) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void chewy(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	getanyad(context);
	context.es.byte(context.bx+2) = 255;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void useladder(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context._sub(context.data.byte(kMapx), 11);
	findroominloc(context);
	context.data.byte(kFacing) = 6;
	context.data.byte(kTurntoface) = 6;
	context.data.byte(kManspath) = 0;
	context.data.byte(kDestination) = 0;
	context.data.byte(kFinaldest) = 0;
	findxyfrompath(context);
	context.data.byte(kResetmanxy) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void useladderb(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context._add(context.data.byte(kMapx), 11);
	findroominloc(context);
	context.data.byte(kFacing) = 2;
	context.data.byte(kTurntoface) = 2;
	context.data.byte(kManspath) = 1;
	context.data.byte(kDestination) = 1;
	context.data.byte(kFinaldest) = 1;
	findxyfrompath(context);
	context.data.byte(kResetmanxy) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void slabdoora(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.word(kReeltowatch) = 13;
	context._cmp(context.data.byte(kDreamnumber), 3);
	if (!context.flags.z()) goto slabawrong;
	context._inc(context.data.byte(kProgresspoints));
	context.data.word(kWatchingtime) = 60;
	context.data.word(kEndwatchreel) = 42;
	context.data.byte(kNewlocation) = 47;
	{assert(stack_depth == context.stack.size()); return; }
slabawrong:
	context.data.word(kWatchingtime) = 40;
	context.data.word(kEndwatchreel) = 34;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void slabdoorb(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kDreamnumber), 1);
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
	{assert(stack_depth == context.stack.size()); return; }
gotcrystal:
	showfirstuse(context);
	context._inc(context.data.byte(kProgresspoints));
	context.data.byte(kGetback) = 1;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.word(kReeltowatch) = 44;
	context.data.word(kWatchingtime) = 60;
	context.data.word(kEndwatchreel) = 71;
	context.data.byte(kNewlocation) = 47;
	{assert(stack_depth == context.stack.size()); return; }
slabbwrong:
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.word(kReeltowatch) = 44;
	context.data.word(kWatchingtime) = 40;
	context.data.word(kEndwatchreel) = 63;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void slabdoord(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.word(kReeltowatch) = 75;
	context._cmp(context.data.byte(kDreamnumber), 0);
	if (!context.flags.z()) goto slabcwrong;
	context._inc(context.data.byte(kProgresspoints));
	context.data.word(kWatchingtime) = 60;
	context.data.word(kEndwatchreel) = 102;
	context.data.byte(kNewlocation) = 47;
	{assert(stack_depth == context.stack.size()); return; }
slabcwrong:
	context.data.word(kWatchingtime) = 40;
	context.data.word(kEndwatchreel) = 94;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void slabdoorc(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.word(kReeltowatch) = 108;
	context._cmp(context.data.byte(kDreamnumber), 4);
	if (!context.flags.z()) goto slabdwrong;
	context._inc(context.data.byte(kProgresspoints));
	context.data.word(kWatchingtime) = 60;
	context.data.word(kEndwatchreel) = 135;
	context.data.byte(kNewlocation) = 47;
	{assert(stack_depth == context.stack.size()); return; }
slabdwrong:
	context.data.word(kWatchingtime) = 40;
	context.data.word(kEndwatchreel) = 127;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void slabdoore(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.word(kReeltowatch) = 141;
	context._cmp(context.data.byte(kDreamnumber), 5);
	if (!context.flags.z()) goto slabewrong;
	context._inc(context.data.byte(kProgresspoints));
	context.data.word(kWatchingtime) = 60;
	context.data.word(kEndwatchreel) = 168;
	context.data.byte(kNewlocation) = 47;
	{assert(stack_depth == context.stack.size()); return; }
slabewrong:
	context.data.word(kWatchingtime) = 40;
	context.data.word(kEndwatchreel) = 160;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void slabdoorf(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.word(kReeltowatch) = 171;
	context._cmp(context.data.byte(kDreamnumber), 2);
	if (!context.flags.z()) goto slabfwrong;
	context._inc(context.data.byte(kProgresspoints));
	context.data.word(kWatchingtime) = 60;
	context.data.word(kEndwatchreel) = 197;
	context.data.byte(kNewlocation) = 47;
	{assert(stack_depth == context.stack.size()); return; }
slabfwrong:
	context.data.word(kWatchingtime) = 40;
	context.data.word(kEndwatchreel) = 189;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void useslab(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto slabwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
slabwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
nextslab:
	context.al = context.data.byte(kWithobject);
	getexad(context);
	context.es.byte(context.bx+2) = 0;
	context.al = context.data.byte(kCommand);
	context.push(context.ax);
	removesetobject(context);
	context.ax = context.pop();
	context._inc(context.al);
	context.push(context.ax);
	placesetobject(context);
	context.ax = context.pop();
	context._cmp(context.al, 54);
	if (!context.flags.z()) goto notlastslab;
	context.al = 0;
	turnpathon(context);
	context.data.word(kWatchingtime) = 22;
	context.data.word(kReeltowatch) = 35;
	context.data.word(kEndwatchreel) = 48;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
notlastslab:
	context._inc(context.data.byte(kProgresspoints));
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usecart(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto cartwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
cartwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
nextcart:
	context.al = context.data.byte(kWithobject);
	getexad(context);
	context.es.byte(context.bx+2) = 0;
	context.al = context.data.byte(kCommand);
	context.push(context.ax);
	removesetobject(context);
	context.ax = context.pop();
	context._inc(context.al);
	placesetobject(context);
	context._inc(context.data.byte(kProgresspoints));
	context.al = 17;
	playchannel1(context);
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void useclearbox(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto clearboxwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
clearboxwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
openbox:
	context._inc(context.data.byte(kProgresspoints));
	showfirstuse(context);
	context.data.word(kWatchingtime) = 80;
	context.data.word(kReeltowatch) = 67;
	context.data.word(kEndwatchreel) = 105;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usecoveredbox(Context & context) {
	uint stack_depth = context.stack.size();
	context._inc(context.data.byte(kProgresspoints));
	showfirstuse(context);
	context.data.word(kWatchingtime) = 50;
	context.data.word(kReeltowatch) = 41;
	context.data.word(kEndwatchreel) = 66;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void userailing(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.word(kWatchingtime) = 80;
	context.data.word(kReeltowatch) = 0;
	context.data.word(kEndwatchreel) = 30;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	context.data.byte(kMandead) = 4;
	{assert(stack_depth == context.stack.size()); return; }
}

void useopenbox(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto openboxwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
openboxwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'F';
	compare(context);
	if (context.flags.z()) goto destoryopenbox;
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'C';
	context.ch = 'U';
	context.dl = 'P';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto openboxwrong;
	showfirstuse(context);
	{assert(stack_depth == context.stack.size()); return; }
destoryopenbox:
	context._inc(context.data.byte(kProgresspoints));
	context.cx = 300;
	context.al = 37;
	showpuztext(context);
	context.al = context.data.byte(kWithobject);
	getexad(context);
	context.es.byte(context.bx+15) = 'E'-'A';
	context.data.word(kWatchingtime) = 140;
	context.data.word(kReeltowatch) = 105;
	context.data.word(kEndwatchreel) = 181;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.al = 4;
	turnpathon(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
openboxwrong:
	context.cx = 300;
	context.al = 38;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void wearwatch(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWatchon), 1);
	if (context.flags.z()) goto wearingwatch;
	showfirstuse(context);
	context.data.byte(kWatchon) = 1;
	context.data.byte(kGetback) = 1;
	getanyad(context);
	makeworn(context);
	{assert(stack_depth == context.stack.size()); return; }
wearingwatch:
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void wearshades(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kShadeson), 1);
	if (context.flags.z()) goto wearingshades;
	context.data.byte(kShadeson) = 1;
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	getanyad(context);
	makeworn(context);
	{assert(stack_depth == context.stack.size()); return; }
wearingshades:
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void sitdowninbar(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWatchmode), -1);
	if (!context.flags.z()) goto satdown;
	showfirstuse(context);
	context.data.word(kWatchingtime) = 50;
	context.data.word(kReeltowatch) = 55;
	context.data.word(kEndwatchreel) = 71;
	context.data.word(kReeltohold) = 73;
	context.data.word(kEndofholdreel) = 83;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
satdown:
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usechurchhole(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	context.data.word(kWatchingtime) = 28;
	context.data.word(kReeltowatch) = 13;
	context.data.word(kEndwatchreel) = 26;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usehole(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto holewith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
holewith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
righthand:
	showfirstuse(context);
	context.al = 86;
	removesetobject(context);
	context.al = context.data.byte(kWithobject);
	getexad(context);
	context.es.byte(context.bx+2) = 255;
	context.data.byte(kCanmovealtar) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usealtar(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 'C';
	context.ah = 'N';
	context.cl = 'D';
	context.ch = 'A';
	findexobject(context);
	context._cmp(context.al, (114));
	if (context.flags.z()) goto thingsonaltar;
	context.al = 'C';
	context.ah = 'N';
	context.cl = 'D';
	context.ch = 'B';
	findexobject(context);
	context._cmp(context.al, (114));
	if (context.flags.z()) goto thingsonaltar;
	context._cmp(context.data.byte(kCanmovealtar), 1);
	if (context.flags.z()) goto movealtar;
	context.cx = 300;
	context.al = 23;
	showpuztext(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
movealtar:
	context._inc(context.data.byte(kProgresspoints));
	showseconduse(context);
	context.data.word(kWatchingtime) = 160;
	context.data.word(kReeltowatch) = 81;
	context.data.word(kEndwatchreel) = 174;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.al = 47;
	context.bl = 52;
	context.bh = 76;
	context.cx = 32;
	context.dx = 98;
	setuptimeduse(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
thingsonaltar:
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void opentvdoor(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto tvdoorwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
tvdoorwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
keyontv:
	showfirstuse(context);
	context.data.byte(kLockstatus) = 0;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usedryer(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 12;
	playchannel1(context);
	showfirstuse(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void openlouis(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 5;
	context.ah = 2;
	context.cl = 3;
	context.ch = 8;
	entercode(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void nextcolon(Context & context) {
	uint stack_depth = context.stack.size();
lookcolon:
	context.al = context.es.byte(context.si);
	context._inc(context.si);
	context._cmp(context.al, ':');
	if (!context.flags.z()) goto lookcolon;
	{assert(stack_depth == context.stack.size()); return; }
}

void openyourneighbour(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 255;
	context.ah = 255;
	context.cl = 255;
	context.ch = 255;
	entercode(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usewindow(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kManspath), 6);
	if (!context.flags.z()) goto notonbalc;
	context._inc(context.data.byte(kProgresspoints));
	showfirstuse(context);
	context.data.byte(kNewlocation) = 29;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notonbalc:
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usebalcony(Context & context) {
	uint stack_depth = context.stack.size();
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
	context._inc(context.data.byte(kProgresspoints));
	context.data.byte(kManspath) = 6;
	context.data.byte(kDestination) = 6;
	context.data.byte(kFinaldest) = 6;
	findxyfrompath(context);
	switchryanoff(context);
	context.data.byte(kResetmanxy) = 1;
	context.data.word(kWatchingtime) = 30*2;
	context.data.word(kReeltowatch) = 183;
	context.data.word(kEndwatchreel) = 212;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void openryan(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 5;
	context.ah = 1;
	context.cl = 0;
	context.ch = 6;
	entercode(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void openpoolboss(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 5;
	context.ah = 2;
	context.cl = 2;
	context.ch = 2;
	entercode(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void openeden(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 2;
	context.ah = 8;
	context.cl = 6;
	context.ch = 5;
	entercode(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void opensarters(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 7;
	context.ah = 8;
	context.cl = 3;
	context.ch = 3;
	entercode(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void isitright(Context & context) {
	uint stack_depth = context.stack.size();
	context.bx = context.data;
	context.es = context.bx;
	context.bx = 8344;
	context._cmp(context.es.byte(context.bx+0), context.al);
	if (!context.flags.z()) goto notright;
	context._cmp(context.es.byte(context.bx+1), context.ah);
	if (!context.flags.z()) goto notright;
	context._cmp(context.es.byte(context.bx+2), context.cl);
	if (!context.flags.z()) goto notright;
	context._cmp(context.es.byte(context.bx+3), context.ch);
notright:
	{assert(stack_depth == context.stack.size()); return; }
}

void drawitall(Context & context) {
	uint stack_depth = context.stack.size();
	createpanel(context);
	drawfloor(context);
	printsprites(context);
	showicon(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void openhoteldoor(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto hoteldoorwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
hoteldoorwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
keyonhotel1:
	context.al = 16;
	playchannel1(context);
	showfirstuse(context);
	context.data.byte(kLockstatus) = 0;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void openhoteldoor2(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto hoteldoorwith2;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
hoteldoorwith2:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
keyonhotel2:
	context.al = 16;
	playchannel1(context);
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void grafittidoor(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto grafwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
grafwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
dograf:
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void trapdoor(Context & context) {
	uint stack_depth = context.stack.size();
	context._inc(context.data.byte(kProgresspoints));
	showfirstuse(context);
	switchryanoff(context);
	context.data.word(kWatchingtime) = 20*2;
	context.data.word(kReeltowatch) = 181;
	context.data.word(kEndwatchreel) = 197;
	context.data.byte(kNewlocation) = 26;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void callhotellift(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 12;
	playchannel1(context);
	showfirstuse(context);
	context.data.byte(kCounttoopen) = 8;
	context.data.byte(kGetback) = 1;
	context.data.byte(kDestination) = 5;
	context.data.byte(kFinaldest) = 5;
	autosetwalk(context);
	context.al = 4;
	turnpathon(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void calledenslift(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.byte(kCounttoopen) = 8;
	context.data.byte(kGetback) = 1;
	context.al = 2;
	turnpathon(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void calledensdlift(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLiftflag), 1);
	if (context.flags.z()) goto edensdhere;
	showfirstuse(context);
	context.data.byte(kCounttoopen) = 8;
	context.data.byte(kGetback) = 1;
	context.al = 2;
	turnpathon(context);
	{assert(stack_depth == context.stack.size()); return; }
edensdhere:
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usepoolreader(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto poolwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
poolwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
openpool:
	context._cmp(context.data.byte(kTalkedtoattendant), 1);
	if (context.flags.z()) goto canopenpool;
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
canopenpool:
	context.al = 17;
	playchannel1(context);
	showfirstuse(context);
	context.data.byte(kCounttoopen) = 6;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void uselighter(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto gotlighterwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
gotlighterwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'S';
	context.ch = 'M';
	context.dl = 'K';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto cigarette;
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
cigarette:
	context.cx = 300;
	context.al = 9;
	showpuztext(context);
	context.al = context.data.byte(kWithobject);
	getexad(context);
	context.es.byte(context.bx+2) = 255;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void showseconduse(Context & context) {
	uint stack_depth = context.stack.size();
	getobtextstart(context);
	nextcolon(context);
	nextcolon(context);
	nextcolon(context);
	usetext(context);
	context.cx = 400;
	hangonp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usecardreader1(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto gotreader1with;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
gotreader1with:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
correctcard:
	context._cmp(context.data.byte(kTalkedtosparky), 0);
	if (context.flags.z()) goto notyet;
	context._cmp(context.data.word(kCard1money), 0);
	if (context.flags.z()) goto getscash;
	context.cx = 300;
	context.al = 17;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
getscash:
	context.al = 16;
	playchannel1(context);
	context.cx = 300;
	context.al = 18;
	showpuztext(context);
	context._inc(context.data.byte(kProgresspoints));
	context.data.word(kCard1money) = 12432;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notyet:
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usecardreader2(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto gotreader2with;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
gotreader2with:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
correctcard2:
	context._cmp(context.data.byte(kTalkedtoboss), 0);
	if (context.flags.z()) goto notyetboss;
	context._cmp(context.data.word(kCard1money), 0);
	if (context.flags.z()) goto nocash;
	context._cmp(context.data.byte(kGunpassflag), 2);
	if (context.flags.z()) goto alreadygotnew;
	context.al = 18;
	playchannel1(context);
	context.cx = 300;
	context.al = 19;
	showpuztext(context);
	context.al = 94;
	placesetobject(context);
	context.data.byte(kGunpassflag) = 1;
	context._sub(context.data.word(kCard1money), 2000);
	context._inc(context.data.byte(kProgresspoints));
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
nocash:
	context.cx = 300;
	context.al = 20;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
alreadygotnew:
	context.cx = 300;
	context.al = 22;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
notyetboss:
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usecardreader3(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto gotreader3with;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
gotreader3with:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
rightcard:
	context._cmp(context.data.byte(kTalkedtorecep), 0);
	if (context.flags.z()) goto notyetrecep;
	context._cmp(context.data.byte(kCardpassflag), 0);
	if (!context.flags.z()) goto alreadyusedit;
	context.al = 16;
	playchannel1(context);
	context.cx = 300;
	context.al = 25;
	showpuztext(context);
	context._inc(context.data.byte(kProgresspoints));
	context._sub(context.data.word(kCard1money), 8300);
	context.data.byte(kCardpassflag) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
alreadyusedit:
	context.cx = 300;
	context.al = 26;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
notyetrecep:
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usecashcard(Context & context) {
	uint stack_depth = context.stack.size();
	getridofreels(context);
	loadkeypad(context);
	createpanel(context);
	showpanel(context);
	showexit(context);
	showman(context);
	context.di = 114;
	context.bx = 120;
	context.ds = context.data.word(kTempgraphics);
	context.al = 39;
	context.ah = 0;
	showframe(context);
	context.ax = context.data.word(kCard1money);
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
	context.data.word(kCharshift) = 91*2+75;
	context.al = 0;
	context.ah = 0;
	context.dl = 240;
	printdirect(context);
	context.di = 187;
	context.bx = 155;
	context.es = context.cs;
	context.si = 3390;
	context.data.word(kCharshift) = 91*2+85;
	context.al = 0;
	context.ah = 0;
	context.dl = 240;
	printdirect(context);
	context.data.word(kCharshift) = 0;
	worktoscreenm(context);
	context.cx = 400;
	hangonp(context);
	getridoftemp(context);
	restorereels(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void lookatcard(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kManisoffscreen) = 1;
	getridofreels(context);
	loadkeypad(context);
	createpanel2(context);
	context.di = 160;
	context.bx = 80;
	context.ds = context.data.word(kTempgraphics);
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
	context.ds = context.data.word(kTempgraphics);
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
	context.data.byte(kManisoffscreen) = 0;
	getridoftemp(context);
	restorereels(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void moneypoke(Context & context) {
	uint stack_depth = context.stack.size();
	context.bx = 3385;
	context.cl = 48-1;
numberpoke0:
	context._inc(context.cl);
	context._sub(context.ax, 10000);
	if (!context.flags.c()) goto numberpoke0;
	context._add(context.ax, 10000);
	context.cs.byte(context.bx) = context.cl;
	context._inc(context.bx);
	context.cl = 48-1;
numberpoke1:
	context._inc(context.cl);
	context._sub(context.ax, 1000);
	if (!context.flags.c()) goto numberpoke1;
	context._add(context.ax, 1000);
	context.cs.byte(context.bx) = context.cl;
	context._inc(context.bx);
	context.cl = 48-1;
numberpoke2:
	context._inc(context.cl);
	context._sub(context.ax, 100);
	if (!context.flags.c()) goto numberpoke2;
	context._add(context.ax, 100);
	context.cs.byte(context.bx) = context.cl;
	context._inc(context.bx);
	context.cl = 48-1;
numberpoke3:
	context._inc(context.cl);
	context._sub(context.ax, 10);
	if (!context.flags.c()) goto numberpoke3;
	context._add(context.ax, 10);
	context.cs.byte(context.bx) = context.cl;
	context.bx = 3390;
	context._add(context.al, 48);
	context.cs.byte(context.bx) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void usecontrol(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto gotcontrolwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
gotcontrolwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'K';
	context.ch = 'E';
	context.dl = 'Y';
	context.dh = 'A';
	compare(context);
	if (context.flags.z()) goto rightkey;
	context._cmp(context.data.byte(kReallocation), 21);
	if (!context.flags.z()) goto balls;
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'K';
	context.ch = 'N';
	context.dl = 'F';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto jimmycontrols;
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'A';
	context.ch = 'X';
	context.dl = 'E';
	context.dh = 'D';
	compare(context);
	if (context.flags.z()) goto axeoncontrols;
balls:
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
rightkey:
	context.al = 16;
	playchannel1(context);
	context._cmp(context.data.byte(kLocation), 21);
	if (context.flags.z()) goto goingdown;
	context.cx = 300;
	context.al = 0;
	showpuztext(context);
	context.data.byte(kNewlocation) = 21;
	context.data.byte(kCounttoclose) = 8;
	context.data.byte(kCounttoopen) = 0;
	context.data.word(kWatchingtime) = 80;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
goingdown:
	context.cx = 300;
	context.al = 3;
	showpuztext(context);
	context.data.byte(kNewlocation) = 30;
	context.data.byte(kCounttoclose) = 8;
	context.data.byte(kCounttoopen) = 0;
	context.data.word(kWatchingtime) = 80;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
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
	context._inc(context.data.byte(kProgresspoints));
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
axeoncontrols:
	context.cx = 300;
	context.al = 16;
	showpuztext(context);
	context._inc(context.data.byte(kProgresspoints));
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usehatch(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.byte(kNewlocation) = 40;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usewire(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto gotwirewith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
gotwirewith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'K';
	context.ch = 'N';
	context.dl = 'F';
	context.dh = 'E';
	compare(context);
	if (context.flags.z()) goto wireknife;
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
wireaxe:
	context.cx = 300;
	context.al = 16;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
wireknife:
	context.al = 51;
	removesetobject(context);
	context.al = 52;
	placesetobject(context);
	context.cx = 300;
	context.al = 11;
	showpuztext(context);
	context._inc(context.data.byte(kProgresspoints));
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usehandle(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 'C';
	context.ah = 'U';
	context.cl = 'T';
	context.ch = 'W';
	findsetobject(context);
	context.al = context.es.byte(context.bx+58);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto havecutwire;
	context.cx = 300;
	context.al = 12;
	showpuztext(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
havecutwire:
	context.cx = 300;
	context.al = 13;
	showpuztext(context);
	context.data.byte(kNewlocation) = 22;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void useelevator1(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	selectlocation(context);
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void showfirstuse(Context & context) {
	uint stack_depth = context.stack.size();
	getobtextstart(context);
	findnextcolon(context);
	findnextcolon(context);
	usetext(context);
	context.cx = 400;
	hangonp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void useelevator3(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.byte(kCounttoclose) = 20;
	context.data.byte(kNewlocation) = 34;
	context.data.word(kReeltowatch) = 46;
	context.data.word(kEndwatchreel) = 63;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.word(kWatchingtime) = 80;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void useelevator4(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.word(kReeltowatch) = 0;
	context.data.word(kEndwatchreel) = 11;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kCounttoclose) = 20;
	context.data.word(kWatchingtime) = 80;
	context.data.byte(kGetback) = 1;
	context.data.byte(kNewlocation) = 24;
	{assert(stack_depth == context.stack.size()); return; }
}

void useelevator2(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLocation), 23);
	if (context.flags.z()) goto inpoolhall;
	showfirstuse(context);
	context.data.byte(kNewlocation) = 23;
	context.data.byte(kCounttoclose) = 20;
	context.data.byte(kCounttoopen) = 0;
	context.data.word(kWatchingtime) = 80;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
inpoolhall:
	showfirstuse(context);
	context.data.byte(kNewlocation) = 31;
	context.data.byte(kCounttoclose) = 20;
	context.data.byte(kCounttoopen) = 0;
	context.data.word(kWatchingtime) = 80;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void useelevator5(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 4;
	placesetobject(context);
	context.al = 0;
	removesetobject(context);
	context.data.byte(kNewlocation) = 20;
	context.data.word(kWatchingtime) = 80;
	context.data.byte(kLiftflag) = 1;
	context.data.byte(kCounttoclose) = 8;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usekey(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLocation), 5);
	if (context.flags.z()) goto usekey1;
	context._cmp(context.data.byte(kLocation), 30);
	if (context.flags.z()) goto usekey1;
	context._cmp(context.data.byte(kLocation), 21);
	if (context.flags.z()) goto usekey2;
	context.cx = 200;
	context.al = 1;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
usekey1:
	context._cmp(context.data.byte(kMapx), 22);
	if (!context.flags.z()) goto wrongroom1;
	context._cmp(context.data.byte(kMapy), 10);
	if (!context.flags.z()) goto wrongroom1;
	context.cx = 300;
	context.al = 0;
	showpuztext(context);
	context.data.byte(kCounttoclose) = 100;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
usekey2:
	context._cmp(context.data.byte(kMapx), 11);
	if (!context.flags.z()) goto wrongroom1;
	context._cmp(context.data.byte(kMapy), 10);
	if (!context.flags.z()) goto wrongroom1;
	context.cx = 300;
	context.al = 3;
	showpuztext(context);
	context.data.byte(kNewlocation) = 30;
	context.al = 2;
	fadescreendown(context);
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
wrongroom1:
	context.cx = 200;
	context.al = 2;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usestereo(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLocation), 0);
	if (context.flags.z()) goto stereook;
	context.cx = 400;
	context.al = 4;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
stereook:
	context._cmp(context.data.byte(kMapx), 11);
	if (!context.flags.z()) goto stereonotok;
	context._cmp(context.data.byte(kMapy), 0);
	if (context.flags.z()) goto stereook2;
stereonotok:
	context.cx = 400;
	context.al = 5;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
stereook2:
	context.al = 'C';
	context.ah = 'D';
	context.cl = 'P';
	context.ch = 'L';
	findsetobject(context);
	context.ah = 1;
	checkinside(context);
	context._cmp(context.cl, (114));
	if (!context.flags.z()) goto cdinside;
	context.al = 6;
	context.cx = 400;
	showpuztext(context);
	putbackobstuff(context);
	getanyad(context);
	context.al = 255;
	context.es.byte(context.bx+10) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
cdinside:
	getanyad(context);
	context.al = context.es.byte(context.bx+10);
	context._xor(context.al, 1);
	context.es.byte(context.bx+10) = context.al;
	context._cmp(context.al, 255);
	if (context.flags.z()) goto stereoon;
	context.al = 7;
	context.cx = 400;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
stereoon:
	context.al = 8;
	context.cx = 400;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usecooker(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kCommand);
	context.ah = context.data.byte(kObjecttype);
	checkinside(context);
	context._cmp(context.cl, (114));
	if (!context.flags.z()) goto foodinside;
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
foodinside:
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void useaxe(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kReallocation), 22);
	if (!context.flags.z()) goto notinpool;
	context._cmp(context.data.byte(kMapy), 10);
	if (context.flags.z()) goto axeondoor;
	showseconduse(context);
	context._inc(context.data.byte(kProgresspoints));
	context.data.byte(kLastweapon) = 2;
	context.data.byte(kGetback) = 1;
	removeobfrominv(context);
	{assert(stack_depth == context.stack.size()); return; }
notinpool:
	showfirstuse(context);
	{assert(stack_depth == context.stack.size()); return; }
/*continuing to unbounded code: axeondoor from useelvdoor:19-30*/
axeondoor:
	context.al = 15;
	context.cx = 300;
	showpuztext(context);
	context._inc(context.data.byte(kProgresspoints));
	context.data.word(kWatchingtime) = 46*2;
	context.data.word(kReeltowatch) = 31;
	context.data.word(kEndwatchreel) = 77;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
	{assert(stack_depth == context.stack.size()); return; }
}

void useelvdoor(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto gotdoorwith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
gotdoorwith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
axeondoor:
	context.al = 15;
	context.cx = 300;
	showpuztext(context);
	context._inc(context.data.byte(kProgresspoints));
	context.data.word(kWatchingtime) = 46*2;
	context.data.word(kReeltowatch) = 31;
	context.data.word(kEndwatchreel) = 77;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void withwhat(Context & context) {
	uint stack_depth = context.stack.size();
	createpanel(context);
	showpanel(context);
	showman(context);
	showexit(context);
	context.al = context.data.byte(kCommand);
	context.ah = context.data.byte(kObjecttype);
	context.es = context.cs;
	context.di = 5674;
	copyname(context);
	context.di = 100;
	context.bx = 21;
	context.dl = 200;
	context.al = 63;
	context.ah = 2;
	printmessage2(context);
	context.di = context.data.word(kLastxpos);
	context._add(context.di, 5);
	context.bx = 21;
	context.es = context.cs;
	context.si = 5674;
	context.dl = 220;
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.di = context.data.word(kLastxpos);
	context._add(context.di, 5);
	context.bx = 21;
	context.dl = 200;
	context.al = 63;
	context.ah = 3;
	printmessage2(context);
	fillryan(context);
	context.data.byte(kCommandtype) = 255;
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	context.data.byte(kInvopen) = 2;
	{assert(stack_depth == context.stack.size()); return; }
}

void selectob(Context & context) {
	uint stack_depth = context.stack.size();
	findinvpos(context);
	context.ax = context.es.word(context.bx);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto canselectob;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
canselectob:
	context.data.byte(kWithobject) = context.al;
	context.data.byte(kWithtype) = context.ah;
	context._cmp(context.ax, context.data.word(kOldsubject));
	if (!context.flags.z()) goto diffsub3;
	context._cmp(context.data.byte(kCommandtype), 221);
	if (context.flags.z()) goto alreadyselob;
	context.data.byte(kCommandtype) = 221;
diffsub3:
	context.data.word(kOldsubject) = context.ax;
	context.bx = context.ax;
	context.al = 0;
	commandwithob(context);
alreadyselob:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notselob;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doselob;
notselob:
	{assert(stack_depth == context.stack.size()); return; }
doselob:
	delpointer(context);
	context.data.byte(kInvopen) = 0;
	useroutine(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void compare(Context & context) {
	uint stack_depth = context.stack.size();
	context._sub(context.dl, 'A');
	context._sub(context.dh, 'A');
	context._sub(context.cl, 'A');
	context._sub(context.ch, 'A');
	context.push(context.cx);
	context.push(context.dx);
	getanyaddir(context);
	context.dx = context.pop();
	context.cx = context.pop();
	context._cmp(context.es.word(context.bx+12), context.cx);
	if (!context.flags.z()) goto comparefin;
	context._cmp(context.es.word(context.bx+14), context.dx);
comparefin:
	{assert(stack_depth == context.stack.size()); return; }
}

void findsetobject(Context & context) {
	uint stack_depth = context.stack.size();
	context._sub(context.al, 'A');
	context._sub(context.ah, 'A');
	context._sub(context.cl, 'A');
	context._sub(context.ch, 'A');
	context.es = context.data.word(kSetdat);
	context.bx = 0;
	context.dl = 0;
findsetloop:
	context._cmp(context.al, context.es.byte(context.bx+12));
	if (!context.flags.z()) goto nofind;
	context._cmp(context.ah, context.es.byte(context.bx+13));
	if (!context.flags.z()) goto nofind;
	context._cmp(context.cl, context.es.byte(context.bx+14));
	if (!context.flags.z()) goto nofind;
	context._cmp(context.ch, context.es.byte(context.bx+15));
	if (!context.flags.z()) goto nofind;
	context.al = context.dl;
	{assert(stack_depth == context.stack.size()); return; }
nofind:
	context._add(context.bx, 64);
	context._inc(context.dl);
	context._cmp(context.dl, 128);
	if (!context.flags.z()) goto findsetloop;
	context.al = context.dl;
	{assert(stack_depth == context.stack.size()); return; }
}

void findexobject(Context & context) {
	uint stack_depth = context.stack.size();
	context._sub(context.al, 'A');
	context._sub(context.ah, 'A');
	context._sub(context.cl, 'A');
	context._sub(context.ch, 'A');
	context.es = context.data.word(kExtras);
	context.bx = (0+2080+30000);
	context.dl = 0;
findexloop:
	context._cmp(context.al, context.es.byte(context.bx+12));
	if (!context.flags.z()) goto nofindex;
	context._cmp(context.ah, context.es.byte(context.bx+13));
	if (!context.flags.z()) goto nofindex;
	context._cmp(context.cl, context.es.byte(context.bx+14));
	if (!context.flags.z()) goto nofindex;
	context._cmp(context.ch, context.es.byte(context.bx+15));
	if (!context.flags.z()) goto nofindex;
	context.al = context.dl;
	{assert(stack_depth == context.stack.size()); return; }
nofindex:
	context._add(context.bx, 16);
	context._inc(context.dl);
	context._cmp(context.dl, (114));
	if (!context.flags.z()) goto findexloop;
	context.al = context.dl;
	{assert(stack_depth == context.stack.size()); return; }
}

void isryanholding(Context & context) {
	uint stack_depth = context.stack.size();
	context._sub(context.al, 'A');
	context._sub(context.ah, 'A');
	context._sub(context.cl, 'A');
	context._sub(context.ch, 'A');
	context.es = context.data.word(kExtras);
	context.bx = (0+2080+30000);
	context.dl = 0;
searchinv:
	context._cmp(context.es.byte(context.bx+2), 4);
	if (!context.flags.z()) goto nofindininv;
	context._cmp(context.al, context.es.byte(context.bx+12));
	if (!context.flags.z()) goto nofindininv;
	context._cmp(context.ah, context.es.byte(context.bx+13));
	if (!context.flags.z()) goto nofindininv;
	context._cmp(context.cl, context.es.byte(context.bx+14));
	if (!context.flags.z()) goto nofindininv;
	context._cmp(context.ch, context.es.byte(context.bx+15));
	if (!context.flags.z()) goto nofindininv;
	context.al = context.dl;
	context._cmp(context.al, (114));
	{assert(stack_depth == context.stack.size()); return; }
nofindininv:
	context._add(context.bx, 16);
	context._inc(context.dl);
	context._cmp(context.dl, (114));
	if (!context.flags.z()) goto searchinv;
	context.al = context.dl;
	context._cmp(context.al, (114));
	{assert(stack_depth == context.stack.size()); return; }
}

void checkinside(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kExtras);
	context.bx = (0+2080+30000);
	context.cl = 0;
insideloop:
	context._cmp(context.al, context.es.byte(context.bx+3));
	if (!context.flags.z()) goto notfoundinside;
	context._cmp(context.ah, context.es.byte(context.bx+2));
	if (!context.flags.z()) goto notfoundinside;
	{assert(stack_depth == context.stack.size()); return; }
notfoundinside:
	context._add(context.bx, 16);
	context._inc(context.cl);
	context._cmp(context.cl, (114));
	if (!context.flags.z()) goto insideloop;
	{assert(stack_depth == context.stack.size()); return; }
}

void usetext(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void putbackobstuff(Context & context) {
	uint stack_depth = context.stack.size();
	createpanel(context);
	showpanel(context);
	showman(context);
	obicons(context);
	showexit(context);
	obpicture(context);
	describeob(context);
	undertextline(context);
	context.data.byte(kCommandtype) = 255;
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showpuztext(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void findpuztext(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 0;
	context.si = context.ax;
	context._add(context.si, context.si);
	context.es = context.data.word(kPuzzletext);
	context.ax = context.es.word(context.si);
	context._add(context.ax, (66*2));
	context.si = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void placesetobject(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	context.cl = 0;
	context.ch = 0;
	findormake(context);
	getsetad(context);
	context.es.byte(context.bx+58) = 0;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void removesetobject(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	context.cl = 255;
	context.ch = 0;
	findormake(context);
	getsetad(context);
	context.es.byte(context.bx+58) = 255;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void issetobonmap(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	getsetad(context);
	context.al = context.es.byte(context.bx+58);
	context.bx = context.pop();
	context.es = context.pop();
	context._cmp(context.al, 0);
	{assert(stack_depth == context.stack.size()); return; }
}

void placefreeobject(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	context.cl = 0;
	context.ch = 1;
	findormake(context);
	getfreead(context);
	context.es.byte(context.bx+2) = 0;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void removefreeobject(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.bx);
	getfreead(context);
	context.es.byte(context.bx+2) = 255;
	context.bx = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void findormake(Context & context) {
	uint stack_depth = context.stack.size();
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
	context.push(context.ax);
	context.es = context.data.word(kBuffers);
	context.ah = context.data.byte(kReallocation);
changeloop:
	context._cmp(context.es.byte(context.bx), 255);
	if (context.flags.z()) goto haventfound;
	context._cmp(context.ax, context.es.word(context.bx));
	if (!context.flags.z()) goto nofoundchange;
	context._cmp(context.ch, context.es.byte(context.bx+3));
	if (context.flags.z()) goto foundchange;
nofoundchange:
	context._add(context.bx, 4);
	goto changeloop;
foundchange:
	context.ax = context.pop();
	context.es.byte(context.bx+2) = context.cl;
	{assert(stack_depth == context.stack.size()); return; }
haventfound:
	context.es.word(context.bx) = context.ax;
	context.es.word(context.bx+2) = context.cx;
	context.ax = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void switchryanon(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kRyanon) = 255;
	{assert(stack_depth == context.stack.size()); return; }
}

void switchryanoff(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kRyanon) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void setallchanges(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
setallloop:
	context.ax = context.es.word(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto endsetloop;
	context.cx = context.es.word(context.bx+2);
	context._add(context.bx, 4);
	context._cmp(context.ah, context.data.byte(kReallocation));
	if (!context.flags.z()) goto setallloop;
	context.push(context.es);
	context.push(context.bx);
	dochange(context);
	context.bx = context.pop();
	context.es = context.pop();
	goto setallloop;
endsetloop:
	{assert(stack_depth == context.stack.size()); return; }
}

void dochange(Context & context) {
	uint stack_depth = context.stack.size();
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
	context._add(context.bx, (0));
	context.es = context.data.word(kReels);
	context.cx = context.pop();
	context.es.byte(context.bx+6) = context.cl;
nopath:
	{assert(stack_depth == context.stack.size()); return; }
object:
	context.push(context.cx);
	getsetad(context);
	context.cx = context.pop();
	context.es.byte(context.bx+58) = context.cl;
	{assert(stack_depth == context.stack.size()); return; }
freeobject:
	context.push(context.cx);
	getfreead(context);
	context.cx = context.pop();
	context._cmp(context.es.byte(context.bx+2), 255);
	if (!context.flags.z()) goto beenpickedup;
	context.es.byte(context.bx+2) = context.cl;
beenpickedup:
	{assert(stack_depth == context.stack.size()); return; }
}

void autoappear(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLocation), 32);
	if (!context.flags.z()) goto notinalley;
	context.al = 5;
	resetlocation(context);
	context.al = 10;
	setlocation(context);
	context.data.byte(kDestpos) = 10;
	{assert(stack_depth == context.stack.size()); return; }
notinalley:
	context._cmp(context.data.byte(kReallocation), 24);
	if (!context.flags.z()) goto notinedens;
	context._cmp(context.data.byte(kGeneraldead), 1);
	if (!context.flags.z()) goto edenspart2;
	context._inc(context.data.byte(kGeneraldead));
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
	{assert(stack_depth == context.stack.size()); return; }
edenspart2:
	context._cmp(context.data.byte(kSartaindead), 1);
	if (!context.flags.z()) goto notedens2;
	context.al = 44;
	removesetobject(context);
	context.al = 93;
	removesetobject(context);
	context.al = 55;
	placesetobject(context);
	context._inc(context.data.byte(kSartaindead));
notedens2:
	{assert(stack_depth == context.stack.size()); return; }
notinedens:
	context._cmp(context.data.byte(kReallocation), 25);
	if (!context.flags.z()) goto notonsartroof;
	context.data.byte(kNewsitem) = 3;
	context.al = 6;
	resetlocation(context);
	context.al = 11;
	setlocation(context);
	context.data.byte(kDestpos) = 11;
	{assert(stack_depth == context.stack.size()); return; }
notonsartroof:
	context._cmp(context.data.byte(kReallocation), 2);
	if (!context.flags.z()) goto notinlouiss;
	context._cmp(context.data.byte(kRockstardead), 0);
	if (context.flags.z()) goto notinlouiss;
	context.al = 23;
	placesetobject(context);
notinlouiss:
	{assert(stack_depth == context.stack.size()); return; }
}

void getundertimed(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kTimedy);
	context.ah = 0;
	context.bx = context.ax;
	context.al = context.data.byte(kTimedx);
	context.ah = 0;
	context.di = context.ax;
	context.ch = (24);
	context.cl = 240;
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4));
	multiget(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void putundertimed(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kTimedy);
	context.ah = 0;
	context.bx = context.ax;
	context.al = context.data.byte(kTimedx);
	context.ah = 0;
	context.di = context.ax;
	context.ch = (24);
	context.cl = 240;
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4));
	multiput(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void dumptimedtext(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kNeedtodumptimed), 1);
	if (!context.flags.z()) goto nodumptimed;
	context.al = context.data.byte(kTimedy);
	context.ah = 0;
	context.bx = context.ax;
	context.al = context.data.byte(kTimedx);
	context.ah = 0;
	context.di = context.ax;
	context.cl = 240;
	context.ch = (24);
	multidump(context);
	context.data.byte(kNeedtodumptimed) = 0;
nodumptimed:
	{assert(stack_depth == context.stack.size()); return; }
}

void setuptimeduse(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kTimecount), 0);
	if (!context.flags.z()) goto cantsetup;
	context.data.byte(kTimedy) = context.bh;
	context.data.byte(kTimedx) = context.bl;
	context.data.word(kCounttotimed) = context.cx;
	context._add(context.dx, context.cx);
	context.data.word(kTimecount) = context.dx;
	context.bl = context.al;
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(kPuzzletext);
	context.cx = (66*2);
	context.ax = context.es.word(context.bx);
	context._add(context.ax, context.cx);
	context.bx = context.ax;
	context.data.word(kTimedseg) = context.es;
	context.data.word(kTimedoffset) = context.bx;
cantsetup:
	{assert(stack_depth == context.stack.size()); return; }
}

void setuptimedtemp(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kTimecount), 0);
	if (!context.flags.z()) goto cantsetup2;
	context.data.byte(kTimedy) = context.bh;
	context.data.byte(kTimedx) = context.bl;
	context.data.word(kCounttotimed) = context.cx;
	context._add(context.dx, context.cx);
	context.data.word(kTimecount) = context.dx;
	context.bl = context.al;
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(kTextfile1);
	context.cx = (66*2);
	context.ax = context.es.word(context.bx);
	context._add(context.ax, context.cx);
	context.bx = context.ax;
	context.data.word(kTimedseg) = context.es;
	context.data.word(kTimedoffset) = context.bx;
cantsetup2:
	{assert(stack_depth == context.stack.size()); return; }
}

void usetimedtext(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kTimecount), 0);
	if (context.flags.z()) goto notext;
	context._dec(context.data.word(kTimecount));
	context._cmp(context.data.word(kTimecount), 0);
	if (context.flags.z()) goto deltimedtext;
	context.ax = context.data.word(kTimecount);
	context._cmp(context.ax, context.data.word(kCounttotimed));
	if (context.flags.z()) goto firsttimed;
	if (!context.flags.c()) goto notext;
	goto notfirsttimed;
firsttimed:
	getundertimed(context);
notfirsttimed:
	context.bl = context.data.byte(kTimedy);
	context.bh = 0;
	context.al = context.data.byte(kTimedx);
	context.ah = 0;
	context.di = context.ax;
	context.es = context.data.word(kTimedseg);
	context.si = context.data.word(kTimedoffset);
	context.dl = 237;
	context.ah = 0;
	printdirect(context);
	context.data.byte(kNeedtodumptimed) = 1;
notext:
	{assert(stack_depth == context.stack.size()); return; }
deltimedtext:
	putundertimed(context);
	context.data.byte(kNeedtodumptimed) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void edenscdplayer(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context.data.word(kWatchingtime) = 18*2;
	context.data.word(kReeltowatch) = 25;
	context.data.word(kEndwatchreel) = 42;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void usewall(Context & context) {
	uint stack_depth = context.stack.size();
	showfirstuse(context);
	context._cmp(context.data.byte(kManspath), 3);
	if (context.flags.z()) goto gobackover;
	context.data.word(kWatchingtime) = 30*2;
	context.data.word(kReeltowatch) = 2;
	context.data.word(kEndwatchreel) = 31;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
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
	context.data.byte(kManspath) = 3;
	context.data.byte(kFinaldest) = 3;
	findxyfrompath(context);
	context.data.byte(kResetmanxy) = 1;
	switchryanoff(context);
	{assert(stack_depth == context.stack.size()); return; }
gobackover:
	context.data.word(kWatchingtime) = 30*2;
	context.data.word(kReeltowatch) = 34;
	context.data.word(kEndwatchreel) = 60;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
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
	context.data.byte(kManspath) = 5;
	context.data.byte(kFinaldest) = 5;
	findxyfrompath(context);
	context.data.byte(kResetmanxy) = 1;
	switchryanoff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usechurchgate(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto gatewith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
gatewith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
cutgate:
	showfirstuse(context);
	context.data.word(kWatchingtime) = 64*2;
	context.data.word(kReeltowatch) = 4;
	context.data.word(kEndwatchreel) = 70;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	context._inc(context.data.byte(kProgresspoints));
	context.al = 3;
	turnpathon(context);
	context._cmp(context.data.byte(kAidedead), 0);
	if (context.flags.z()) goto notopenchurch;
	context.al = 2;
	turnpathon(context);
notopenchurch:
	{assert(stack_depth == context.stack.size()); return; }
}

void usegun(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kObjecttype), 4);
	if (context.flags.z()) goto istakengun;
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
istakengun:
	context._cmp(context.data.byte(kReallocation), 22);
	if (!context.flags.z()) goto notinpoolroom;
	context.cx = 300;
	context.al = 34;
	showpuztext(context);
	context.data.byte(kLastweapon) = 1;
	context.data.byte(kCombatcount) = 39;
	context.data.byte(kGetback) = 1;
	context._inc(context.data.byte(kProgresspoints));
	{assert(stack_depth == context.stack.size()); return; }
notinpoolroom:
	context._cmp(context.data.byte(kReallocation), 25);
	if (!context.flags.z()) goto nothelicopter;
	context.cx = 300;
	context.al = 34;
	showpuztext(context);
	context.data.byte(kLastweapon) = 1;
	context.data.byte(kCombatcount) = 19;
	context.data.byte(kGetback) = 1;
	context.data.byte(kDreamnumber) = 2;
	context.data.byte(kRoomafterdream) = 38;
	context.data.byte(kSartaindead) = 1;
	context._inc(context.data.byte(kProgresspoints));
	{assert(stack_depth == context.stack.size()); return; }
nothelicopter:
	context._cmp(context.data.byte(kReallocation), 27);
	if (!context.flags.z()) goto notinrockroom;
	context.cx = 300;
	context.al = 46;
	showpuztext(context);
	context.data.byte(kPointermode) = 2;
	context.data.byte(kRockstardead) = 1;
	context.data.byte(kLastweapon) = 1;
	context.data.byte(kNewsitem) = 1;
	context.data.byte(kGetback) = 1;
	context.data.byte(kRoomafterdream) = 32;
	context.data.byte(kDreamnumber) = 0;
	context._inc(context.data.byte(kProgresspoints));
	{assert(stack_depth == context.stack.size()); return; }
notinrockroom:
	context._cmp(context.data.byte(kReallocation), 8);
	if (!context.flags.z()) goto notbystudio;
	context._cmp(context.data.byte(kMapx), 22);
	if (!context.flags.z()) goto notbystudio;
	context._cmp(context.data.byte(kMapy), 40);
	if (!context.flags.z()) goto notbystudio;
	context.al = 92;
	issetobonmap(context);
	if (context.flags.z()) goto notbystudio;
	context._cmp(context.data.byte(kManspath), 9);
	if (context.flags.z()) goto notbystudio;
	context.data.byte(kDestination) = 9;
	context.data.byte(kFinaldest) = 9;
	autosetwalk(context);
	context.data.byte(kLastweapon) = 1;
	context.data.byte(kGetback) = 1;
	context._inc(context.data.byte(kProgresspoints));
	{assert(stack_depth == context.stack.size()); return; }
notbystudio:
	context._cmp(context.data.byte(kReallocation), 6);
	if (!context.flags.z()) goto notsarters;
	context._cmp(context.data.byte(kMapx), 11);
	if (!context.flags.z()) goto notsarters;
	context._cmp(context.data.byte(kMapy), 20);
	if (!context.flags.z()) goto notsarters;
	context.al = 5;
	issetobonmap(context);
	if (!context.flags.z()) goto notsarters;
	context.data.byte(kDestination) = 1;
	context.data.byte(kFinaldest) = 1;
	autosetwalk(context);
	context.al = 5;
	removesetobject(context);
	context.al = 6;
	placesetobject(context);
	context.al = 1;
	context.ah = context.data.byte(kRoomnum);
	context._dec(context.ah);
	turnanypathon(context);
	context.data.byte(kLiftflag) = 1;
	context.data.word(kWatchingtime) = 40*2;
	context.data.word(kReeltowatch) = 4;
	context.data.word(kEndwatchreel) = 43;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	context._inc(context.data.byte(kProgresspoints));
	{assert(stack_depth == context.stack.size()); return; }
notsarters:
	context._cmp(context.data.byte(kReallocation), 29);
	if (!context.flags.z()) goto notaide;
	context.data.byte(kGetback) = 1;
	context.al = 13;
	resetlocation(context);
	context.al = 12;
	setlocation(context);
	context.data.byte(kDestpos) = 12;
	context.data.byte(kDestination) = 2;
	context.data.byte(kFinaldest) = 2;
	autosetwalk(context);
	context.data.word(kWatchingtime) = 164*2;
	context.data.word(kReeltowatch) = 3;
	context.data.word(kEndwatchreel) = 164;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kAidedead) = 1;
	context.data.byte(kDreamnumber) = 3;
	context.data.byte(kRoomafterdream) = 33;
	context._inc(context.data.byte(kProgresspoints));
	{assert(stack_depth == context.stack.size()); return; }
notaide:
	context._cmp(context.data.byte(kReallocation), 23);
	if (!context.flags.z()) goto notwithboss;
	context._cmp(context.data.byte(kMapx), 0);
	if (!context.flags.z()) goto notwithboss;
	context._cmp(context.data.byte(kMapy), 50);
	if (!context.flags.z()) goto notwithboss;
	context._cmp(context.data.byte(kManspath), 5);
	if (context.flags.z()) goto pathokboss;
	context.data.byte(kDestination) = 5;
	context.data.byte(kFinaldest) = 5;
	autosetwalk(context);
pathokboss:
	context.data.byte(kLastweapon) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notwithboss:
	context._cmp(context.data.byte(kReallocation), 8);
	if (!context.flags.z()) goto nottvsoldier;
	context._cmp(context.data.byte(kMapx), 11);
	if (!context.flags.z()) goto nottvsoldier;
	context._cmp(context.data.byte(kMapy), 10);
	if (!context.flags.z()) goto nottvsoldier;
	context._cmp(context.data.byte(kManspath), 2);
	if (context.flags.z()) goto pathoktv;
	context.data.byte(kDestination) = 2;
	context.data.byte(kFinaldest) = 2;
	autosetwalk(context);
pathoktv:
	context.data.byte(kLastweapon) = 1;
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
nottvsoldier:
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void useshield(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kReallocation), 20);
	if (!context.flags.z()) goto notinsartroom;
	context._cmp(context.data.byte(kCombatcount), 0);
	if (context.flags.z()) goto notinsartroom;
	context.data.byte(kLastweapon) = 3;
	showseconduse(context);
	context.data.byte(kGetback) = 1;
	context._inc(context.data.byte(kProgresspoints));
	removeobfrominv(context);
	{assert(stack_depth == context.stack.size()); return; }
notinsartroom:
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usebuttona(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 95;
	issetobonmap(context);
	if (context.flags.z()) goto donethisbit;
	showfirstuse(context);
	context.al = 0;
	context.ah = context.data.byte(kRoomnum);
	context._dec(context.ah);
	turnanypathon(context);
	context.al = 9;
	removesetobject(context);
	context.al = 95;
	placesetobject(context);
	context.data.word(kWatchingtime) = 15*2;
	context.data.word(kReeltowatch) = 71;
	context.data.word(kEndwatchreel) = 85;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kGetback) = 1;
	context._inc(context.data.byte(kProgresspoints));
	{assert(stack_depth == context.stack.size()); return; }
donethisbit:
	showseconduse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void useplate(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWithobject), 255);
	if (!context.flags.z()) goto platewith;
	withwhat(context);
	{assert(stack_depth == context.stack.size()); return; }
platewith:
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
	context.cl = 'S';
	context.ch = 'C';
	context.dl = 'R';
	context.dh = 'W';
	compare(context);
	if (context.flags.z()) goto unscrewplate;
	context.al = context.data.byte(kWithobject);
	context.ah = context.data.byte(kWithtype);
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
	{assert(stack_depth == context.stack.size()); return; }
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
	context._inc(context.data.byte(kProgresspoints));
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
triedknife:
	context.cx = 300;
	context.al = 54;
	showpuztext(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usewinch(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 40;
	context.ah = 1;
	checkinside(context);
	context._cmp(context.cl, (114));
	if (context.flags.z()) goto nowinch;
	context.al = context.cl;
	context.ah = 4;
	context.cl = 'F';
	context.ch = 'U';
	context.dl = 'S';
	context.dh = 'E';
	compare(context);
	if (!context.flags.z()) goto nowinch;
	context.data.word(kWatchingtime) = 217*2;
	context.data.word(kReeltowatch) = 0;
	context.data.word(kEndwatchreel) = 217;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	context.data.byte(kDestpos) = 1;
	context.data.byte(kNewlocation) = 45;
	context.data.byte(kDreamnumber) = 1;
	context.data.byte(kRoomafterdream) = 44;
	context.data.byte(kGeneraldead) = 1;
	context.data.byte(kNewsitem) = 2;
	context.data.byte(kGetback) = 1;
	context._inc(context.data.byte(kProgresspoints));
	{assert(stack_depth == context.stack.size()); return; }
nowinch:
	showfirstuse(context);
	putbackobstuff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void entercode(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.word(kKeypadax) = context.ax;
	context.data.word(kKeypadcx) = context.cx;
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
	context.data.word(kPresspointer) = 0;
	context.data.byte(kGetback) = 0;
keypadloop:
	delpointer(context);
	readmouse(context);
	showkeypad(context);
	showpointer(context);
	context._cmp(context.data.byte(kPresscount), 0);
	if (context.flags.z()) goto nopresses;
	context._dec(context.data.byte(kPresscount));
	goto afterpress;
nopresses:
	context.data.byte(kPressed) = 255;
	context.data.byte(kGraphicpress) = 255;
	vsync(context);
afterpress:
	dumppointer(context);
	dumpkeypad(context);
	dumptextline(context);
	context.bx = 3393;
	checkcoords(context);
	context._cmp(context.data.byte(kGetback), 1);
	if (context.flags.z()) goto numberright;
	context._cmp(context.data.byte(kLightcount), 1);
	if (!context.flags.z()) goto notendkey;
	context._cmp(context.data.byte(kLockstatus), 0);
	if (context.flags.z()) goto numberright;
	goto keypadloop;
notendkey:
	context._cmp(context.data.byte(kPresscount), 40);
	if (!context.flags.z()) goto keypadloop;
	addtopresslist(context);
	context._cmp(context.data.byte(kPressed), 11);
	if (!context.flags.z()) goto keypadloop;
	context.ax = context.data.word(kKeypadax);
	context.cx = context.data.word(kKeypadcx);
	isitright(context);
	if (!context.flags.z()) goto incorrect;
	context.data.byte(kLockstatus) = 0;
	context.al = 11;
	playchannel1(context);
	context.data.byte(kLightcount) = 120;
	context.data.word(kPresspointer) = 0;
	goto keypadloop;
incorrect:
	context.al = 11;
	playchannel1(context);
	context.data.byte(kLightcount) = 120;
	context.data.word(kPresspointer) = 0;
	goto keypadloop;
numberright:
	context.data.byte(kManisoffscreen) = 0;
	getridoftemp(context);
	restorereels(context);
	redrawmainscrn(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void loadkeypad(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 1948;
	loadintotemp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void quitkey(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 222);
	if (context.flags.z()) goto alreadyqk;
	context.data.byte(kCommandtype) = 222;
	context.al = 4;
	commandonly(context);
alreadyqk:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notqk;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doqk;
notqk:
	{assert(stack_depth == context.stack.size()); return; }
doqk:
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void addtopresslist(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kPresspointer), 5);
	if (context.flags.z()) goto nomorekeys;
	context.al = context.data.byte(kPressed);
	context._cmp(context.al, 10);
	if (!context.flags.z()) goto not10;
	context.al = 0;
not10:
	context.bx = context.data.word(kPresspointer);
	context.dx = context.data;
	context.es = context.dx;
	context._add(context.bx, 8344);
	context.es.byte(context.bx) = context.al;
	context._inc(context.data.word(kPresspointer));
nomorekeys:
	{assert(stack_depth == context.stack.size()); return; }
}

void buttonone(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 1;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttontwo(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 2;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttonthree(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 3;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttonfour(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 4;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttonfive(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 5;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttonsix(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 6;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttonseven(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 7;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttoneight(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 8;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttonnine(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 9;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttonnought(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 10;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttonenter(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = 11;
	buttonpress(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void buttonpress(Context & context) {
	uint stack_depth = context.stack.size();
	context.ch = context.cl;
	context._add(context.ch, 100);
	context._cmp(context.data.byte(kCommandtype), context.ch);
	if (context.flags.z()) goto alreadyb;
	context.data.byte(kCommandtype) = context.ch;
	context.al = context.cl;
	context._add(context.al, 4);
	context.push(context.cx);
	commandonly(context);
	context.cx = context.pop();
alreadyb:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notb;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dob;
notb:
	{assert(stack_depth == context.stack.size()); return; }
dob:
	context.data.byte(kPressed) = context.cl;
	context._add(context.cl, 21);
	context.data.byte(kGraphicpress) = context.cl;
	context.data.byte(kPresscount) = 40;
	context._cmp(context.cl, 32);
	if (context.flags.z()) goto nonoise;
	context.al = 10;
	playchannel1(context);
nonoise:
	{assert(stack_depth == context.stack.size()); return; }
}

void showouterpad(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = (36+112)-3;
	context.bx = (72)-4;
	context.ds = context.data.word(kTempgraphics);
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.di = (36+112)+74;
	context.bx = (72)+76;
	context.ds = context.data.word(kTempgraphics);
	context.al = 37;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showkeypad(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 22;
	context.di = (36+112)+9;
	context.bx = (72)+5;
	singlekey(context);
	context.al = 23;
	context.di = (36+112)+31;
	context.bx = (72)+5;
	singlekey(context);
	context.al = 24;
	context.di = (36+112)+53;
	context.bx = (72)+5;
	singlekey(context);
	context.al = 25;
	context.di = (36+112)+9;
	context.bx = (72)+23;
	singlekey(context);
	context.al = 26;
	context.di = (36+112)+31;
	context.bx = (72)+23;
	singlekey(context);
	context.al = 27;
	context.di = (36+112)+53;
	context.bx = (72)+23;
	singlekey(context);
	context.al = 28;
	context.di = (36+112)+9;
	context.bx = (72)+41;
	singlekey(context);
	context.al = 29;
	context.di = (36+112)+31;
	context.bx = (72)+41;
	singlekey(context);
	context.al = 30;
	context.di = (36+112)+53;
	context.bx = (72)+41;
	singlekey(context);
	context.al = 31;
	context.di = (36+112)+9;
	context.bx = (72)+59;
	singlekey(context);
	context.al = 32;
	context.di = (36+112)+31;
	context.bx = (72)+59;
	singlekey(context);
	context._cmp(context.data.byte(kLightcount), 0);
	if (context.flags.z()) goto notenter;
	context._dec(context.data.byte(kLightcount));
	context.al = 36;
	context.bx = (72)-1+63;
	context._cmp(context.data.byte(kLockstatus), 0);
	if (!context.flags.z()) goto changelight;
	context.al = 41;
	context.bx = (72)+4+63;
changelight:
	context._cmp(context.data.byte(kLightcount), 60);
	if (context.flags.c()) goto gotlight;
	context._cmp(context.data.byte(kLightcount), 100);
	if (!context.flags.c()) goto gotlight;
	context._dec(context.al);
gotlight:
	context.ds = context.data.word(kTempgraphics);
	context.ah = 0;
	context.di = (36+112)+60;
	showframe(context);
notenter:
	{assert(stack_depth == context.stack.size()); return; }
}

void singlekey(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kGraphicpress), context.al);
	if (!context.flags.z()) goto gotkey;
	context._add(context.al, 11);
	context._cmp(context.data.byte(kPresscount), 8);
	if (!context.flags.c()) goto gotkey;
	context._sub(context.al, 11);
gotkey:
	context.ds = context.data.word(kTempgraphics);
	context._sub(context.al, 20);
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpkeypad(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = (36+112)-3;
	context.bx = (72)-4;
	context.cl = 120;
	context.ch = 90;
	multidump(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usemenu(Context & context) {
	uint stack_depth = context.stack.size();
	getridofreels(context);
	loadmenu(context);
	createpanel(context);
	showpanel(context);
	showicon(context);
	context.data.byte(kNewobs) = 0;
	drawfloor(context);
	printsprites(context);
	context.al = 4;
	context.ah = 0;
	context.di = (80+40)-48;
	context.bx = (60)-4;
	context.ds = context.data.word(kTempgraphics2);
	showframe(context);
	getundermenu(context);
	context.al = 5;
	context.ah = 0;
	context.di = (80+40)+54;
	context.bx = (60)+72;
	context.ds = context.data.word(kTempgraphics2);
	showframe(context);
	worktoscreenm(context);
	context.data.byte(kGetback) = 0;
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
	context._cmp(context.data.byte(kGetback), 1);
	if (!context.flags.z()) goto menuloop;
	context.data.byte(kManisoffscreen) = 0;
	redrawmainscrn(context);
	getridoftemp(context);
	getridoftemp2(context);
	restorereels(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
menulist:
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpmenu(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = (80+40);
	context.bx = (60);
	context.cl = 48;
	context.ch = 48;
	multidump(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getundermenu(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = (80+40);
	context.bx = (60);
	context.cl = 48;
	context.ch = 48;
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4));
	multiget(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void putundermenu(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = (80+40);
	context.bx = (60);
	context.cl = 48;
	context.ch = 48;
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4));
	multiput(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showoutermenu(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 40;
	context.ah = 0;
	context.di = (80+40)-34;
	context.bx = (60)-40;
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	context.al = 41;
	context.ah = 0;
	context.di = (80+40)+64-34;
	context.bx = (60)-40;
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	context.al = 42;
	context.ah = 0;
	context.di = (80+40)-26;
	context.bx = (60)+57-40;
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	context.al = 43;
	context.ah = 0;
	context.di = (80+40)+64-26;
	context.bx = (60)+57-40;
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showmenu(Context & context) {
	uint stack_depth = context.stack.size();
	context._inc(context.data.byte(kMenucount));
	context._cmp(context.data.byte(kMenucount), 37*2);
	if (!context.flags.z()) goto menuframeok;
	context.data.byte(kMenucount) = 0;
menuframeok:
	context.al = context.data.byte(kMenucount);
	context._shr(context.al, 1);
	context.ah = 0;
	context.di = (80+40);
	context.bx = (60);
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void loadmenu(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 1832;
	loadintotemp(context);
	context.dx = 1987;
	loadintotemp2(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void viewfolder(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kManisoffscreen) = 1;
	getridofall(context);
	loadfolder(context);
	context.data.byte(kFolderpage) = 0;
	showfolder(context);
	worktoscreenm(context);
	context.data.byte(kGetback) = 0;
folderloop:
	delpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.bx = 3547;
	checkcoords(context);
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto folderloop;
	context.data.byte(kManisoffscreen) = 0;
	getridoftemp(context);
	getridoftemp2(context);
	getridoftemp3(context);
	getridoftempcharset(context);
	restoreall(context);
	redrawmainscrn(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void nextfolder(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kFolderpage), 12);
	if (!context.flags.z()) goto cannextf;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
cannextf:
	context._cmp(context.data.byte(kCommandtype), 201);
	if (context.flags.z()) goto alreadynextf;
	context.data.byte(kCommandtype) = 201;
	context.al = 16;
	commandonly(context);
alreadynextf:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notnextf;
	context._cmp(context.ax, 1);
	if (context.flags.z()) goto donextf;
notnextf:
	{assert(stack_depth == context.stack.size()); return; }
donextf:
	context._inc(context.data.byte(kFolderpage));
	folderhints(context);
	delpointer(context);
	showfolder(context);
	context.data.word(kMousebutton) = 0;
	context.bx = 3547;
	checkcoords(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void folderhints(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kFolderpage), 5);
	if (!context.flags.z()) goto notaideadd;
	context._cmp(context.data.byte(kAidedead), 1);
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
	{assert(stack_depth == context.stack.size()); return; }
notaideadd:
	context._cmp(context.data.byte(kFolderpage), 9);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void lastfolder(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kFolderpage), 0);
	if (!context.flags.z()) goto canlastf;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
canlastf:
	context._cmp(context.data.byte(kCommandtype), 202);
	if (context.flags.z()) goto alreadylastf;
	context.data.byte(kCommandtype) = 202;
	context.al = 17;
	commandonly(context);
alreadylastf:
	context._cmp(context.data.byte(kFolderpage), 0);
	if (context.flags.z()) goto notlastf;
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notlastf;
	context._cmp(context.ax, 1);
	if (context.flags.z()) goto dolastf;
notlastf:
	{assert(stack_depth == context.stack.size()); return; }
dolastf:
	context._dec(context.data.byte(kFolderpage));
	delpointer(context);
	showfolder(context);
	context.data.word(kMousebutton) = 0;
	context.bx = 3547;
	checkcoords(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void loadfolder(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 2299;
	loadintotemp(context);
	context.dx = 2312;
	loadintotemp2(context);
	context.dx = 2325;
	loadintotemp3(context);
	context.dx = 1883;
	loadtempcharset(context);
	context.dx = 2195;
	loadtemptext(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showfolder(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kCommandtype) = 255;
	context._cmp(context.data.byte(kFolderpage), 0);
	if (context.flags.z()) goto closedfolder;
	usetempcharset(context);
	createpanel2(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = 0;
	context.bx = 0;
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = 143;
	context.bx = 0;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = 0;
	context.bx = 92;
	context.al = 2;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = 143;
	context.bx = 92;
	context.al = 3;
	context.ah = 0;
	showframe(context);
	folderexit(context);
	context._cmp(context.data.byte(kFolderpage), 1);
	if (context.flags.z()) goto noleftpage;
	showleftpage(context);
noleftpage:
	context._cmp(context.data.byte(kFolderpage), 12);
	if (context.flags.z()) goto norightpage;
	showrightpage(context);
norightpage:
	usecharset1(context);
	undertextline(context);
	{assert(stack_depth == context.stack.size()); return; }
closedfolder:
	createpanel2(context);
	context.ds = context.data.word(kTempgraphics3);
	context.di = 143-28;
	context.bx = 0;
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics3);
	context.di = 143-28;
	context.bx = 92;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	folderexit(context);
	undertextline(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void folderexit(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kTempgraphics2);
	context.di = 296;
	context.bx = 178;
	context.al = 6;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showleftpage(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kTempgraphics2);
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
	context.ds = context.data.word(kTempgraphics2);
	context.di = 0;
	context.al = 4;
	context.ah = 0;
	showframe(context);
	context.bx = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 16);
	if (--context.cx) goto leftpageloop;
	context.ds = context.data.word(kTempgraphics2);
	context.di = 0;
	context.al = 5;
	context.ah = 0;
	showframe(context);
	context.data.word(kLinespacing) = 8;
	context.data.word(kCharshift) = 91;
	context.data.byte(kKerning) = 1;
	context.bl = context.data.byte(kFolderpage);
	context._dec(context.bl);
	context._dec(context.bl);
	context._add(context.bl, context.bl);
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(kTextfile1);
	context.si = context.es.word(context.bx);
	context._add(context.si, 66*2);
	context.di = 2;
	context.bx = 48;
	context.dl = 140;
	context.cx = 2;
twolotsleft:
	context.push(context.cx);
contleftpage:
	printdirect(context);
	context._add(context.bx, context.data.word(kLinespacing));
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto contleftpage;
	context.cx = context.pop();
	if (--context.cx) goto twolotsleft;
	context.data.byte(kKerning) = 0;
	context.data.word(kCharshift) = 0;
	context.data.word(kLinespacing) = 10;
	context.es = context.data.word(kWorkspace);
	context.ds = context.data.word(kWorkspace);
	context.di = (48*320)+2;
	context.si = (48*320)+2+130;
	context.cx = 120;
flipfolder:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.si);
	context.cx = 65;
flipfolderline:
	context.al = context.es.byte(context.di);
	context.ah = context.es.byte(context.si);
	context.es.byte(context.di) = context.ah;
	context.es.byte(context.si) = context.al;
	context._dec(context.si);
	context._inc(context.di);
	if (--context.cx) goto flipfolderline;
	context.si = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.si, 320);
	context._add(context.di, 320);
	if (--context.cx) goto flipfolder;
	{assert(stack_depth == context.stack.size()); return; }
}

void showrightpage(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kTempgraphics2);
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
	context.ds = context.data.word(kTempgraphics2);
	context.di = 143;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.bx = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 16);
	if (--context.cx) goto rightpageloop;
	context.ds = context.data.word(kTempgraphics2);
	context.di = 143;
	context.al = 2;
	context.ah = 0;
	showframe(context);
	context.data.word(kLinespacing) = 8;
	context.data.byte(kKerning) = 1;
	context.bl = context.data.byte(kFolderpage);
	context._dec(context.bl);
	context._add(context.bl, context.bl);
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(kTextfile1);
	context.si = context.es.word(context.bx);
	context._add(context.si, 66*2);
	context.di = 152;
	context.bx = 48;
	context.dl = 140;
	context.cx = 2;
twolotsright:
	context.push(context.cx);
contrightpage:
	printdirect(context);
	context._add(context.bx, context.data.word(kLinespacing));
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto contrightpage;
	context.cx = context.pop();
	if (--context.cx) goto twolotsright;
	context.data.byte(kKerning) = 0;
	context.data.word(kLinespacing) = 10;
	{assert(stack_depth == context.stack.size()); return; }
}

void entersymbol(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kManisoffscreen) = 1;
	getridofreels(context);
	context.dx = 2338;
	loadintotemp(context);
	context.data.byte(kSymboltopx) = 24;
	context.data.byte(kSymboltopdir) = 0;
	context.data.byte(kSymbolbotx) = 24;
	context.data.byte(kSymbolbotdir) = 0;
	redrawmainscrn(context);
	showsymbol(context);
	undertextline(context);
	worktoscreenm(context);
	context.data.byte(kGetback) = 0;
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
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto symbolloop;
	context._cmp(context.data.byte(kSymbolbotnum), 3);
	if (!context.flags.z()) goto symbolwrong;
	context._cmp(context.data.byte(kSymboltopnum), 5);
	if (!context.flags.z()) goto symbolwrong;
	context.al = 43;
	removesetobject(context);
	context.al = 46;
	placesetobject(context);
	context.ah = context.data.byte(kRoomnum);
	context._add(context.ah, 12);
	context.al = 0;
	turnanypathon(context);
	context.data.byte(kManisoffscreen) = 0;
	redrawmainscrn(context);
	getridoftemp(context);
	restorereels(context);
	worktoscreenm(context);
	context.al = 13;
	playchannel1(context);
	{assert(stack_depth == context.stack.size()); return; }
symbolwrong:
	context.al = 46;
	removesetobject(context);
	context.al = 43;
	placesetobject(context);
	context.ah = context.data.byte(kRoomnum);
	context._add(context.ah, 12);
	context.al = 0;
	turnanypathoff(context);
	context.data.byte(kManisoffscreen) = 0;
	redrawmainscrn(context);
	getridoftemp(context);
	restorereels(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void quitsymbol(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kSymboltopx), 24);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kSymbolbotx), 24);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kCommandtype), 222);
	if (context.flags.z()) goto alreadyqs;
	context.data.byte(kCommandtype) = 222;
	context.al = 18;
	commandonly(context);
alreadyqs:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notqs;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto doqs;
notqs:
	{assert(stack_depth == context.stack.size()); return; }
doqs:
	context.data.byte(kGetback) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void settopleft(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kSymboltopdir), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kCommandtype), 210);
	if (context.flags.z()) goto alreadytopl;
	context.data.byte(kCommandtype) = 210;
	context.al = 19;
	commandonly(context);
alreadytopl:
	context._cmp(context.data.word(kMousebutton), 0);
	if (context.flags.z()) goto notopleft;
	context.data.byte(kSymboltopdir) = -1;
notopleft:
	{assert(stack_depth == context.stack.size()); return; }
}

void settopright(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kSymboltopdir), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kCommandtype), 211);
	if (context.flags.z()) goto alreadytopr;
	context.data.byte(kCommandtype) = 211;
	context.al = 20;
	commandonly(context);
alreadytopr:
	context._cmp(context.data.word(kMousebutton), 0);
	if (context.flags.z()) goto notopright;
	context.data.byte(kSymboltopdir) = 1;
notopright:
	{assert(stack_depth == context.stack.size()); return; }
}

void setbotleft(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kSymbolbotdir), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kCommandtype), 212);
	if (context.flags.z()) goto alreadybotl;
	context.data.byte(kCommandtype) = 212;
	context.al = 21;
	commandonly(context);
alreadybotl:
	context._cmp(context.data.word(kMousebutton), 0);
	if (context.flags.z()) goto nobotleft;
	context.data.byte(kSymbolbotdir) = -1;
nobotleft:
	{assert(stack_depth == context.stack.size()); return; }
}

void setbotright(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kSymbolbotdir), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kCommandtype), 213);
	if (context.flags.z()) goto alreadybotr;
	context.data.byte(kCommandtype) = 213;
	context.al = 22;
	commandonly(context);
alreadybotr:
	context._cmp(context.data.word(kMousebutton), 0);
	if (context.flags.z()) goto nobotright;
	context.data.byte(kSymbolbotdir) = 1;
nobotright:
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpsymbol(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kNewtextline) = 0;
	context.di = (64);
	context.bx = (56)+20;
	context.cl = 104;
	context.ch = 60;
	multidump(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showsymbol(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 12;
	context.ah = 0;
	context.di = (64);
	context.bx = (56);
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	context.al = context.data.byte(kSymboltopx);
	context.ah = 0;
	context.di = context.ax;
	context._add(context.di, (64)-44);
	context.al = context.data.byte(kSymboltopnum);
	context.bx = (56)+20;
	context.ds = context.data.word(kTempgraphics);
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
	context.al = context.data.byte(kSymbolbotx);
	context.ah = 0;
	context.di = context.ax;
	context._add(context.di, (64)-44);
	context.al = context.data.byte(kSymbolbotnum);
	context._add(context.al, 6);
	context.bx = (56)+49;
	context.ds = context.data.word(kTempgraphics);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void nextsymbol(Context & context) {
	uint stack_depth = context.stack.size();
	context._inc(context.al);
	context._cmp(context.al, 6);
	if (context.flags.z()) goto topwrap;
	context._cmp(context.al, 12);
	if (context.flags.z()) goto botwrap;
	{assert(stack_depth == context.stack.size()); return; }
topwrap:
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
botwrap:
	context.al = 6;
	{assert(stack_depth == context.stack.size()); return; }
}

void updatesymboltop(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kSymboltopdir), 0);
	if (context.flags.z()) goto topfinished;
	context._cmp(context.data.byte(kSymboltopdir), -1);
	if (context.flags.z()) goto backwards;
	context._inc(context.data.byte(kSymboltopx));
	context._cmp(context.data.byte(kSymboltopx), 49);
	if (!context.flags.z()) goto notwrapfor;
	context.data.byte(kSymboltopx) = 0;
	context._dec(context.data.byte(kSymboltopnum));
	context._cmp(context.data.byte(kSymboltopnum), -1);
	if (!context.flags.z()) goto topfinished;
	context.data.byte(kSymboltopnum) = 5;
	{assert(stack_depth == context.stack.size()); return; }
notwrapfor:
	context._cmp(context.data.byte(kSymboltopx), 24);
	if (!context.flags.z()) goto topfinished;
	context.data.byte(kSymboltopdir) = 0;
	{assert(stack_depth == context.stack.size()); return; }
backwards:
	context._dec(context.data.byte(kSymboltopx));
	context._cmp(context.data.byte(kSymboltopx), -1);
	if (!context.flags.z()) goto notwrapback;
	context.data.byte(kSymboltopx) = 48;
	context._inc(context.data.byte(kSymboltopnum));
	context._cmp(context.data.byte(kSymboltopnum), 6);
	if (!context.flags.z()) goto topfinished;
	context.data.byte(kSymboltopnum) = 0;
	{assert(stack_depth == context.stack.size()); return; }
notwrapback:
	context._cmp(context.data.byte(kSymboltopx), 24);
	if (!context.flags.z()) goto topfinished;
	context.data.byte(kSymboltopdir) = 0;
topfinished:
	{assert(stack_depth == context.stack.size()); return; }
}

void updatesymbolbot(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kSymbolbotdir), 0);
	if (context.flags.z()) goto botfinished;
	context._cmp(context.data.byte(kSymbolbotdir), -1);
	if (context.flags.z()) goto backwardsbot;
	context._inc(context.data.byte(kSymbolbotx));
	context._cmp(context.data.byte(kSymbolbotx), 49);
	if (!context.flags.z()) goto notwrapforb;
	context.data.byte(kSymbolbotx) = 0;
	context._dec(context.data.byte(kSymbolbotnum));
	context._cmp(context.data.byte(kSymbolbotnum), -1);
	if (!context.flags.z()) goto botfinished;
	context.data.byte(kSymbolbotnum) = 5;
	{assert(stack_depth == context.stack.size()); return; }
notwrapforb:
	context._cmp(context.data.byte(kSymbolbotx), 24);
	if (!context.flags.z()) goto botfinished;
	context.data.byte(kSymbolbotdir) = 0;
	{assert(stack_depth == context.stack.size()); return; }
backwardsbot:
	context._dec(context.data.byte(kSymbolbotx));
	context._cmp(context.data.byte(kSymbolbotx), -1);
	if (!context.flags.z()) goto notwrapbackb;
	context.data.byte(kSymbolbotx) = 48;
	context._inc(context.data.byte(kSymbolbotnum));
	context._cmp(context.data.byte(kSymbolbotnum), 6);
	if (!context.flags.z()) goto botfinished;
	context.data.byte(kSymbolbotnum) = 0;
	{assert(stack_depth == context.stack.size()); return; }
notwrapbackb:
	context._cmp(context.data.byte(kSymbolbotx), 24);
	if (!context.flags.z()) goto botfinished;
	context.data.byte(kSymbolbotdir) = 0;
botfinished:
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpsymbox(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kDumpx), -1);
	if (context.flags.z()) goto nodumpsym;
	context.di = context.data.word(kDumpx);
	context.bx = context.data.word(kDumpy);
	context.cl = 30;
	context.ch = 77;
	multidump(context);
	context.data.word(kDumpx) = -1;
nodumpsym:
	{assert(stack_depth == context.stack.size()); return; }
}

void usediary(Context & context) {
	uint stack_depth = context.stack.size();
	getridofreels(context);
	context.dx = 2039;
	loadintotemp(context);
	context.dx = 2208;
	loadtemptext(context);
	context.dx = 1883;
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
	context.data.byte(kGetback) = 0;
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
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto diaryloop;
	getridoftemp(context);
	getridoftemptext(context);
	getridoftempcharset(context);
	restorereels(context);
	context.data.byte(kManisoffscreen) = 0;
	redrawmainscrn(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
diarylist:
	{assert(stack_depth == context.stack.size()); return; }
}

void showdiary(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 1;
	context.ah = 0;
	context.di = (68+24);
	context.bx = (48+12)+37;
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	context.al = 2;
	context.ah = 0;
	context.di = (68+24)+176;
	context.bx = (48+12)+108;
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showdiarykeys(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kPresscount), 0);
	if (context.flags.z()) goto nokeyatall;
	context._dec(context.data.byte(kPresscount));
	context._cmp(context.data.byte(kPresscount), 0);
	if (context.flags.z()) goto nokeyatall;
	context._cmp(context.data.byte(kPressed), 'N');
	if (!context.flags.z()) goto nokeyn;
	context.al = 3;
	context._cmp(context.data.byte(kPresscount), 1);
	if (context.flags.z()) goto gotkeyn;
	context.al = 4;
gotkeyn:
	context.ah = 0;
	context.di = (68+24)+94;
	context.bx = (48+12)+97;
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	context._cmp(context.data.byte(kPresscount), 1);
	if (!context.flags.z()) goto notshown;
	showdiarypage(context);
notshown:
	{assert(stack_depth == context.stack.size()); return; }
nokeyn:
	context.al = 5;
	context._cmp(context.data.byte(kPresscount), 1);
	if (context.flags.z()) goto gotkeyp;
	context.al = 6;
gotkeyp:
	context.ah = 0;
	context.di = (68+24)+151;
	context.bx = (48+12)+71;
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	context._cmp(context.data.byte(kPresscount), 1);
	if (!context.flags.z()) goto notshowp;
	showdiarypage(context);
notshowp:
	{assert(stack_depth == context.stack.size()); return; }
nokeyatall:
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpdiarykeys(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kPresscount), 1);
	if (!context.flags.z()) goto notdumpdiary;
	context._cmp(context.data.byte(kSartaindead), 1);
	if (context.flags.z()) goto notsartadd;
	context._cmp(context.data.byte(kDiarypage), 5);
	if (!context.flags.z()) goto notsartadd;
	context._cmp(context.data.byte(kDiarypage), 5);
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
	{assert(stack_depth == context.stack.size()); return; }
notsartadd:
	context.di = (68+24)+48;
	context.bx = (48+12)+15;
	context.cl = 200;
	context.ch = 16;
	multidump(context);
notdumpdiary:
	context.di = (68+24)+94;
	context.bx = (48+12)+97;
	context.cl = 16;
	context.ch = 16;
	multidump(context);
	context.di = (68+24)+151;
	context.bx = (48+12)+71;
	context.cl = 16;
	context.ch = 16;
	multidump(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void diarykeyp(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 214);
	if (context.flags.z()) goto alreadykeyp;
	context.data.byte(kCommandtype) = 214;
	context.al = 23;
	commandonly(context);
alreadykeyp:
	context._cmp(context.data.word(kMousebutton), 0);
	if (context.flags.z()) goto notkeyp;
	context.ax = context.data.word(kOldbutton);
	context._cmp(context.ax, context.data.word(kMousebutton));
	if (context.flags.z()) goto notkeyp;
	context._cmp(context.data.byte(kPresscount), 0);
	if (!context.flags.z()) goto notkeyp;
	context.al = 16;
	playchannel1(context);
	context.data.byte(kPresscount) = 12;
	context.data.byte(kPressed) = 'P';
	context._dec(context.data.byte(kDiarypage));
	context._cmp(context.data.byte(kDiarypage), -1);
	if (!context.flags.z()) goto notkeyp;
	context.data.byte(kDiarypage) = 11;
notkeyp:
	{assert(stack_depth == context.stack.size()); return; }
}

void diarykeyn(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 213);
	if (context.flags.z()) goto alreadykeyn;
	context.data.byte(kCommandtype) = 213;
	context.al = 23;
	commandonly(context);
alreadykeyn:
	context._cmp(context.data.word(kMousebutton), 0);
	if (context.flags.z()) goto notkeyn;
	context.ax = context.data.word(kOldbutton);
	context._cmp(context.ax, context.data.word(kMousebutton));
	if (context.flags.z()) goto notkeyn;
	context._cmp(context.data.byte(kPresscount), 0);
	if (!context.flags.z()) goto notkeyn;
	context.al = 16;
	playchannel1(context);
	context.data.byte(kPresscount) = 12;
	context.data.byte(kPressed) = 'N';
	context._inc(context.data.byte(kDiarypage));
	context._cmp(context.data.byte(kDiarypage), 12);
	if (!context.flags.z()) goto notkeyn;
	context.data.byte(kDiarypage) = 0;
notkeyn:
	{assert(stack_depth == context.stack.size()); return; }
}

void showdiarypage(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = 0;
	context.ah = 0;
	context.di = (68+24);
	context.bx = (48+12);
	context.ds = context.data.word(kTempgraphics);
	showframe(context);
	context.al = context.data.byte(kDiarypage);
	findtext1(context);
	context.data.byte(kKerning) = 1;
	usetempcharset(context);
	context.di = (68+24)+48;
	context.bx = (48+12)+16;
	context.dl = 240;
	context.ah = 16;
	context.data.word(kCharshift) = 91+91;
	printdirect(context);
	context.di = (68+24)+129;
	context.bx = (48+12)+16;
	context.dl = 240;
	context.ah = 16;
	printdirect(context);
	context.di = (68+24)+48;
	context.bx = (48+12)+23;
	context.dl = 240;
	context.ah = 16;
	printdirect(context);
	context.data.byte(kKerning) = 0;
	context.data.word(kCharshift) = 0;
	usecharset1(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void findtext1(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 0;
	context.si = context.ax;
	context._add(context.si, context.si);
	context.es = context.data.word(kTextfile1);
	context.ax = context.es.word(context.si);
	context._add(context.ax, (66*2));
	context.si = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void zoomonoff(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kPointermode), 2);
	if (context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kCommandtype), 222);
	if (context.flags.z()) goto alreadyonoff;
	context.data.byte(kCommandtype) = 222;
	context.al = 39;
	commandonly(context);
alreadyonoff:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto nozoomonoff;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dozoomonoff;
nozoomonoff:
	{assert(stack_depth == context.stack.size()); return; }
dozoomonoff:
	context.al = context.data.byte(kZoomon);
	context._xor(context.al, 1);
	context.data.byte(kZoomon) = context.al;
	createpanel(context);
	context.data.byte(kNewobs) = 0;
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
	{assert(stack_depth == context.stack.size()); return; }
}

void saveload(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kPointermode), 2);
	if (context.flags.z()) { blank(context); return; };
	context._cmp(context.data.byte(kCommandtype), 253);
	if (context.flags.z()) goto alreadyops;
	context.data.byte(kCommandtype) = 253;
	context.al = 43;
	commandonly(context);
alreadyops:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto noops;
	context._and(context.ax, 1);
	if (context.flags.z()) goto noops;
	dosaveload(context);
noops:
	{assert(stack_depth == context.stack.size()); return; }
}

void dosaveload(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kPointerframe) = 0;
	context.data.word(kTextaddressx) = 70;
	context.data.word(kTextaddressy) = 182-8;
	context.data.byte(kTextlen) = 181;
	context.data.byte(kManisoffscreen) = 1;
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
	context.data.byte(kGetback) = 0;
waitops:
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	delpointer(context);
	context.bx = 3693;
	checkcoords(context);
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto waitops;
	context._cmp(context.data.byte(kGetback), 2);
	if (context.flags.z()) goto restartops;
	context.data.word(kTextaddressx) = 13;
	context.data.word(kTextaddressy) = 182;
	context.data.byte(kTextlen) = 240;
	context._cmp(context.data.byte(kGetback), 4);
	if (context.flags.z()) goto justret;
	getridoftemp(context);
	restoreall(context);
	redrawmainscrn(context);
	worktoscreenm(context);
	context.data.byte(kCommandtype) = 200;
justret:
	context.data.byte(kManisoffscreen) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void getbackfromops(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kMandead), 2);
	if (context.flags.z()) goto opsblock1;
	getback1(context);
	{assert(stack_depth == context.stack.size()); return; }
opsblock1:
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showmainops(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+10;
	context.bx = (52)+10;
	context.al = 8;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+59;
	context.bx = (52)+30;
	context.al = 7;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+128+4;
	context.bx = (52)+12;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showdiscops(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+128+4;
	context.bx = (52)+12;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+10;
	context.bx = (52)+10;
	context.al = 9;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+59;
	context.bx = (52)+30;
	context.al = 10;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+176+2;
	context.bx = (52)+60-4;
	context.al = 5;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void loadsavebox(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 1961;
	loadintotemp(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void loadgame(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 246);
	if (context.flags.z()) goto alreadyload;
	context.data.byte(kCommandtype) = 246;
	context.al = 41;
	commandonly(context);
alreadyload:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto noload;
	context._cmp(context.ax, 1);
	if (context.flags.z()) goto doload;
noload:
	{assert(stack_depth == context.stack.size()); return; }
doload:
	context.data.byte(kLoadingorsave) = 1;
	showopbox(context);
	showloadops(context);
	context.data.byte(kCurrentslot) = 0;
	showslots(context);
	shownames(context);
	context.data.byte(kPointerframe) = 0;
	worktoscreenm(context);
	namestoold(context);
	context.data.byte(kGetback) = 0;
loadops:
	delpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.bx = 3735;
	checkcoords(context);
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto loadops;
	context._cmp(context.data.byte(kGetback), 2);
	if (context.flags.z()) goto quitloaded;
	getridoftemp(context);
	context.dx = context.data;
	context.es = context.dx;
	context.bx = 7750;
	startloading(context);
	loadroomssample(context);
	context.data.byte(kRoomloaded) = 1;
	context.data.byte(kNewlocation) = 255;
	clearsprites(context);
	initman(context);
	initrain(context);
	context.data.word(kTextaddressx) = 13;
	context.data.word(kTextaddressy) = 182;
	context.data.byte(kTextlen) = 240;
	startup(context);
	worktoscreen(context);
	context.data.byte(kGetback) = 4;
quitloaded:
	{assert(stack_depth == context.stack.size()); return; }
}

void getbacktoops(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 201);
	if (context.flags.z()) goto alreadygetops;
	context.data.byte(kCommandtype) = 201;
	context.al = 42;
	commandonly(context);
alreadygetops:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto nogetbackops;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dogetbackops;
nogetbackops:
	{assert(stack_depth == context.stack.size()); return; }
dogetbackops:
	oldtonames(context);
	context.data.byte(kGetback) = 2;
	{assert(stack_depth == context.stack.size()); return; }
}

void discops(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 249);
	if (context.flags.z()) goto alreadydiscops;
	context.data.byte(kCommandtype) = 249;
	context.al = 43;
	commandonly(context);
alreadydiscops:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto nodiscops;
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dodiscops;
nodiscops:
	{assert(stack_depth == context.stack.size()); return; }
dodiscops:
	scanfornames(context);
	context.data.byte(kLoadingorsave) = 2;
	showopbox(context);
	showdiscops(context);
	context.data.byte(kCurrentslot) = 0;
	worktoscreenm(context);
	context.data.byte(kGetback) = 0;
discopsloop:
	delpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.bx = 3777;
	checkcoords(context);
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto discopsloop;
	{assert(stack_depth == context.stack.size()); return; }
}

void savegame(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kMandead), 2);
	if (!context.flags.z()) goto cansaveok;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
cansaveok:
	context._cmp(context.data.byte(kCommandtype), 247);
	if (context.flags.z()) goto alreadysave;
	context.data.byte(kCommandtype) = 247;
	context.al = 44;
	commandonly(context);
alreadysave:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (!context.flags.z()) goto dosave;
	{assert(stack_depth == context.stack.size()); return; }
dosave:
	context.data.byte(kLoadingorsave) = 2;
	showopbox(context);
	showsaveops(context);
	context.data.byte(kCurrentslot) = 0;
	showslots(context);
	shownames(context);
	worktoscreenm(context);
	namestoold(context);
	context.data.word(kBufferin) = 0;
	context.data.word(kBufferout) = 0;
	context.data.byte(kGetback) = 0;
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
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto saveops;
	{assert(stack_depth == context.stack.size()); return; }
}

void actualsave(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 222);
	if (context.flags.z()) goto alreadyactsave;
	context.data.byte(kCommandtype) = 222;
	context.al = 44;
	commandonly(context);
alreadyactsave:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (context.flags.z()) goto noactsave;
	context.dx = context.data;
	context.ds = context.dx;
	context.si = 8350;
	context.al = context.data.byte(kCurrentslot);
	context.ah = 0;
	context.cx = 17;
	context._mul(context.cx);
	context._add(context.si, context.ax);
	context._inc(context.si);
	context._cmp(context.ds.byte(context.si), 0);
	if (context.flags.z()) goto noactsave;
	context.al = context.data.byte(kLocation);
	context.ah = 0;
	context.cx = 32;
	context._mul(context.cx);
	context.ds = context.cs;
	context.si = 5958;
	context._add(context.si, context.ax);
	context.di = 7750;
	context.bx = context.di;
	context.es = context.cs;
	context.cx = 16;
	while(context.cx--) 	context._movsw();
 	context.al = context.data.byte(kRoomssample);
	context.es.byte(context.bx+13) = context.al;
	context.al = context.data.byte(kMapx);
	context.es.byte(context.bx+15) = context.al;
	context.al = context.data.byte(kMapy);
	context.es.byte(context.bx+16) = context.al;
	context.al = context.data.byte(kLiftflag);
	context.es.byte(context.bx+20) = context.al;
	context.al = context.data.byte(kManspath);
	context.es.byte(context.bx+21) = context.al;
	context.al = context.data.byte(kFacing);
	context.es.byte(context.bx+22) = context.al;
	context.al = 255;
	context.es.byte(context.bx+27) = context.al;
	saveposition(context);
	getridoftemp(context);
	restoreall(context);
	context.data.word(kTextaddressx) = 13;
	context.data.word(kTextaddressy) = 182;
	context.data.byte(kTextlen) = 240;
	redrawmainscrn(context);
	worktoscreenm(context);
	context.data.byte(kGetback) = 4;
noactsave:
	{assert(stack_depth == context.stack.size()); return; }
}

void actualload(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 221);
	if (context.flags.z()) goto alreadyactload;
	context.data.byte(kCommandtype) = 221;
	context.al = 41;
	commandonly(context);
alreadyactload:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto notactload;
	context._cmp(context.ax, 1);
	if (!context.flags.z()) goto notactload;
	context.dx = context.data;
	context.ds = context.dx;
	context.si = 8350;
	context.al = context.data.byte(kCurrentslot);
	context.ah = 0;
	context.cx = 17;
	context._mul(context.cx);
	context._add(context.si, context.ax);
	context._inc(context.si);
	context._cmp(context.ds.byte(context.si), 0);
	if (context.flags.z()) goto notactload;
	loadposition(context);
	context.data.byte(kGetback) = 1;
notactload:
	{assert(stack_depth == context.stack.size()); return; }
}

void selectslot2(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kMousebutton), 0);
	if (context.flags.z()) goto noselslot2;
	context.data.byte(kLoadingorsave) = 2;
noselslot2:
	selectslot(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void checkinput(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLoadingorsave), 3);
	if (context.flags.z()) goto nokeypress;
	readkey(context);
	context.al = context.data.byte(kCurrentkey);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto nokeypress;
	context._cmp(context.al, 13);
	if (!context.flags.z()) goto notret;
	context.data.byte(kLoadingorsave) = 3;
	goto afterkey;
notret:
	context._cmp(context.al, 8);
	if (!context.flags.z()) goto nodel2;
	context._cmp(context.data.byte(kCursorpos), 0);
	if (context.flags.z()) goto nokeypress;
	getnamepos(context);
	context._dec(context.data.byte(kCursorpos));
	context.es.byte(context.bx) = 0;
	context.es.byte(context.bx+1) = 1;
	goto afterkey;
nodel2:
spacepress:
	context._cmp(context.data.byte(kCursorpos), 14);
	if (context.flags.z()) goto nokeypress;
	getnamepos(context);
	context._inc(context.data.byte(kCursorpos));
	context.al = context.data.byte(kCurrentkey);
	context.es.byte(context.bx+1) = context.al;
	context.es.byte(context.bx+2) = 0;
	context.es.byte(context.bx+3) = 1;
	goto afterkey;
nokeypress:
	{assert(stack_depth == context.stack.size()); return; }
afterkey:
	showopbox(context);
	shownames(context);
	showslots(context);
	showsaveops(context);
	worktoscreenm(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getnamepos(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kCurrentslot);
	context.ah = 0;
	context.cx = 17;
	context._mul(context.cx);
	context.dx = context.data;
	context.es = context.dx;
	context.bx = 8350;
	context._add(context.bx, context.ax);
	context.al = context.data.byte(kCursorpos);
	context.ah = 0;
	context._add(context.bx, context.ax);
	{assert(stack_depth == context.stack.size()); return; }
}

void showopbox(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kTempgraphics);
	context.di = (60);
	context.bx = (52);
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = (60);
	context.bx = (52)+55;
	context.al = 4;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showloadops(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+128+4;
	context.bx = (52)+12;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+176+2;
	context.bx = (52)+60-4;
	context.al = 5;
	context.ah = 0;
	showframe(context);
	context.di = (60)+104;
	context.bx = (52)+14;
	context.al = 55;
	context.dl = 101;
	printmessage(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showsaveops(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+128+4;
	context.bx = (52)+12;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+176+2;
	context.bx = (52)+60-4;
	context.al = 5;
	context.ah = 0;
	showframe(context);
	context.di = (60)+104;
	context.bx = (52)+14;
	context.al = 54;
	context.dl = 101;
	printmessage(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void selectslot(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 244);
	if (context.flags.z()) goto alreadysel;
	context.data.byte(kCommandtype) = 244;
	context.al = 45;
	commandonly(context);
alreadysel:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, 1);
	if (!context.flags.z()) goto noselslot;
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto noselslot;
	context._cmp(context.data.byte(kLoadingorsave), 3);
	if (!context.flags.z()) goto notnocurs;
	context._dec(context.data.byte(kLoadingorsave));
notnocurs:
	oldtonames(context);
	context.ax = context.data.word(kMousey);
	context._sub(context.ax, (52)+4);
	context.cl = -1;
getslotnum:
	context._inc(context.cl);
	context._sub(context.ax, 11);
	if (!context.flags.c()) goto getslotnum;
	context.data.byte(kCurrentslot) = context.cl;
	delpointer(context);
	showopbox(context);
	showslots(context);
	shownames(context);
	context._cmp(context.data.byte(kLoadingorsave), 1);
	if (context.flags.z()) goto isloadmode;
	showsaveops(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
isloadmode:
	showloadops(context);
	readmouse(context);
	showpointer(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
noselslot:
	{assert(stack_depth == context.stack.size()); return; }
}

void showslots(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = (60)+7;
	context.bx = (52)+8;
	context.al = 2;
	context.ds = context.data.word(kTempgraphics);
	context.ah = 0;
	showframe(context);
	context.di = (60)+10;
	context.bx = (52)+11;
	context.cl = 0;
slotloop:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.bx);
	context._cmp(context.cl, context.data.byte(kCurrentslot));
	if (!context.flags.z()) goto nomatchslot;
	context.al = 3;
	context.ds = context.data.word(kTempgraphics);
	context.ah = 0;
	showframe(context);
nomatchslot:
	context.bx = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context._add(context.bx, 10);
	context._inc(context.cl);
	context._cmp(context.cl, 7);
	if (!context.flags.z()) goto slotloop;
	{assert(stack_depth == context.stack.size()); return; }
}

void shownames(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = context.data;
	context.es = context.dx;
	context.si = 8350+1;
	context.di = (60)+21;
	context.bx = (52)+10;
	context.cl = 0;
shownameloop:
	context.push(context.cx);
	context.push(context.di);
	context.push(context.es);
	context.push(context.bx);
	context.push(context.si);
	context.al = 4;
	context._cmp(context.cl, context.data.byte(kCurrentslot));
	if (!context.flags.z()) goto nomatchslot2;
	context._cmp(context.data.byte(kLoadingorsave), 2);
	if (!context.flags.z()) goto loadmode;
	context.dx = context.si;
	context.cx = 15;
	context._add(context.si, 15);
zerostill:
	context._dec(context.si);
	context._dec(context.cl);
	context._cmp(context.es.byte(context.si), 1);
	if (!context.flags.z()) goto foundcharacter;
	goto zerostill;
foundcharacter:
	context.data.byte(kCursorpos) = context.cl;
	context.es.byte(context.si) = '/';
	context.es.byte(context.si+1) = 0;
	context.push(context.si);
	context.si = context.dx;
	context.dl = 200;
	context.ah = 0;
	printdirect(context);
	context.si = context.pop();
	context.es.byte(context.si) = 0;
	context.es.byte(context.si+1) = 1;
	goto afterprintname;
loadmode:
	context.al = 0;
	context.dl = 200;
	context.ah = 0;
	context.data.word(kCharshift) = 91;
	printdirect(context);
	context.data.word(kCharshift) = 0;
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
	context._inc(context.cl);
	context._cmp(context.cl, 7);
	if (!context.flags.z()) goto shownameloop;
	{assert(stack_depth == context.stack.size()); return; }
}

void namestoold(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.cs;
	context.si = 8350;
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
	context.es = context.data.word(kBuffers);
	context.cx = 17*4;
	while(context.cx--) 	context._movsb();
 	{assert(stack_depth == context.stack.size()); return; }
}

void oldtonames(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.cs;
	context.di = 8350;
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
	context.ds = context.data.word(kBuffers);
	context.cx = 17*4;
	while(context.cx--) 	context._movsb();
 	{assert(stack_depth == context.stack.size()); return; }
}

void saveposition(Context & context) {
	uint stack_depth = context.stack.size();
	makeheader(context);
	context.al = context.data.byte(kCurrentslot);
	context.ah = 0;
	context.push(context.ax);
	context.cx = 13;
	context._mul(context.cx);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 8469;
	context._add(context.dx, context.ax);
	openforsave(context);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 5862;
	context.cx = (5958-5862);
	savefilewrite(context);
	context.dx = context.data;
	context.es = context.dx;
	context.di = 5912;
	context.ax = context.pop();
	context.cx = 17;
	context._mul(context.cx);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 8350;
	context._add(context.dx, context.ax);
	saveseg(context);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 0;
	saveseg(context);
	context.ds = context.data.word(kExtras);
	context.dx = (0);
	saveseg(context);
	context.ds = context.data.word(kBuffers);
	context.dx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
	saveseg(context);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 7750;
	saveseg(context);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 534;
	saveseg(context);
fquit:
	closefile(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void loadposition(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.word(kTimecount) = 0;
	clearchanges(context);
	context.al = context.data.byte(kCurrentslot);
	context.ah = 0;
	context.push(context.ax);
	context.cx = 13;
	context._mul(context.cx);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 8469;
	context._add(context.dx, context.ax);
	openfile(context);
	context.ds = context.cs;
	context.dx = 5862;
	context.cx = (5958-5862);
	savefileread(context);
	context.es = context.cs;
	context.di = 5912;
	context.ax = context.pop();
	context.cx = 17;
	context._mul(context.cx);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 8350;
	context._add(context.dx, context.ax);
	loadseg(context);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 0;
	loadseg(context);
	context.ds = context.data.word(kExtras);
	context.dx = (0);
	loadseg(context);
	context.ds = context.data.word(kBuffers);
	context.dx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
	loadseg(context);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 7750;
	loadseg(context);
	context.ds = context.cs;
	context.dx = 534;
	loadseg(context);
	closefile(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void makeheader(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = context.data;
	context.es = context.dx;
	context.di = 5912;
	context.ax = 17;
	storeit(context);
	context.ax = (68-0);
	storeit(context);
	context.ax = (0+2080+30000+(16*114)+((114+2)*2)+18000);
	storeit(context);
	context.ax = (250)*4;
	storeit(context);
	context.ax = 48;
	storeit(context);
	context.ax = (991-534);
	storeit(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void storeit(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.ax, 0);
	if (!context.flags.z()) goto isntblank;
	context._inc(context.ax);
isntblank:
	context._stosw();
	{assert(stack_depth == context.stack.size()); return; }
}

void findlen(Context & context) {
	uint stack_depth = context.stack.size();
	context._dec(context.bx);
	context._add(context.bx, context.ax);
nextone:
	context._cmp(context.cl, context.ds.byte(context.bx));
	if (!context.flags.z()) goto foundlen;
	context._dec(context.bx);
	context._dec(context.ax);
	context._cmp(context.ax, 0);
	if (!context.flags.z()) goto nextone;
foundlen:
	{assert(stack_depth == context.stack.size()); return; }
}

void scanfornames(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = context.data;
	context.es = context.dx;
	context.di = 8350;
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 8469;
	context.cx = 7;
scanloop:
	context.push(context.es);
	context.push(context.ds);
	context.push(context.di);
	context.push(context.dx);
	context.push(context.cx);
	openfilenocheck(context);
	if (context.flags.c()) goto notexist;
	context.cx = context.pop();
	context._inc(context.ch);
	context.push(context.cx);
	context.push(context.di);
	context.push(context.es);
	context.dx = context.data;
	context.ds = context.dx;
	context.dx = 5862;
	context.cx = (5958-5862);
	savefileread(context);
	context.dx = context.data;
	context.es = context.dx;
	context.di = 5912;
	context.ds = context.pop();
	context.dx = context.pop();
	loadseg(context);
	context.bx = context.data.word(kHandle);
	closefile(context);
notexist:
	context.cx = context.pop();
	context.dx = context.pop();
	context.di = context.pop();
	context.ds = context.pop();
	context.es = context.pop();
	context._add(context.dx, 13);
	context._add(context.di, 17);
	context._dec(context.cl);
	if (!context.flags.z()) goto scanloop;
	context.al = context.ch;
	{assert(stack_depth == context.stack.size()); return; }
}

void decide(Context & context) {
	uint stack_depth = context.stack.size();
	setmode(context);
	loadpalfromiff(context);
	clearpalette(context);
	context.data.byte(kPointermode) = 0;
	context.data.word(kWatchingtime) = 0;
	context.data.byte(kPointerframe) = 0;
	context.data.word(kTextaddressx) = 70;
	context.data.word(kTextaddressy) = 182-8;
	context.data.byte(kTextlen) = 181;
	context.data.byte(kManisoffscreen) = 1;
	loadsavebox(context);
	showdecisions(context);
	worktoscreen(context);
	fadescreenup(context);
	context.data.byte(kGetback) = 0;
waitdecide:
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	delpointer(context);
	context.bx = 4884;
	checkcoords(context);
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto waitdecide;
	context._cmp(context.data.byte(kGetback), 4);
	if (context.flags.z()) goto hasloadedroom;
	getridoftemp(context);
hasloadedroom:
	context.data.word(kTextaddressx) = 13;
	context.data.word(kTextaddressy) = 182;
	context.data.byte(kTextlen) = 240;
	{assert(stack_depth == context.stack.size()); return; }
}

void showdecisions(Context & context) {
	uint stack_depth = context.stack.size();
	createpanel2(context);
	showopbox(context);
	context.ds = context.data.word(kTempgraphics);
	context.di = (60)+17;
	context.bx = (52)+13;
	context.al = 6;
	context.ah = 0;
	showframe(context);
	undertextline(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void newgame(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 251);
	if (context.flags.z()) goto alreadynewgame;
	context.data.byte(kCommandtype) = 251;
	context.al = 47;
	commandonly(context);
alreadynewgame:
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, 1);
	if (!context.flags.z()) goto nonewgame;
	context.data.byte(kGetback) = 3;
nonewgame:
	{assert(stack_depth == context.stack.size()); return; }
}

void doload(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kLoadingorsave) = 1;
	showopbox(context);
	showloadops(context);
	context.data.byte(kCurrentslot) = 0;
	showslots(context);
	shownames(context);
	context.data.byte(kPointerframe) = 0;
	worktoscreenm(context);
	namestoold(context);
	context.data.byte(kGetback) = 0;
loadops:
	delpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	dumppointer(context);
	dumptextline(context);
	context.bx = 3735;
	checkcoords(context);
	context._cmp(context.data.byte(kGetback), 0);
	if (context.flags.z()) goto loadops;
	context._cmp(context.data.byte(kGetback), 2);
	if (context.flags.z()) goto quitloaded;
	getridoftemp(context);
	context.dx = context.data;
	context.es = context.dx;
	context.bx = 7750;
	startloading(context);
	loadroomssample(context);
	context.data.byte(kRoomloaded) = 1;
	context.data.byte(kNewlocation) = 255;
	clearsprites(context);
	initman(context);
	initrain(context);
	context.data.word(kTextaddressx) = 13;
	context.data.word(kTextaddressy) = 182;
	context.data.byte(kTextlen) = 240;
	startup(context);
	worktoscreen(context);
	context.data.byte(kGetback) = 4;
quitloaded:
	{assert(stack_depth == context.stack.size()); return; }
}

void loadold(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 252);
	if (context.flags.z()) goto alreadyloadold;
	context.data.byte(kCommandtype) = 252;
	context.al = 48;
	commandonly(context);
alreadyloadold:
	context.ax = context.data.word(kMousebutton);
	context._and(context.ax, 1);
	if (context.flags.z()) goto noloadold;
	doload(context);
	context._cmp(context.data.byte(kGetback), 4);
	if (context.flags.z()) goto noloadold;
	showdecisions(context);
	worktoscreenm(context);
	context.data.byte(kGetback) = 0;
noloadold:
	{assert(stack_depth == context.stack.size()); return; }
}

void createname(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context.di = 4932;
	context.cs.byte(context.di+0) = context.dl;
	context.cs.byte(context.di+3) = context.cl;
	context.al = context.dh;
	context.ah = '0'-1;
findten:
	context._inc(context.ah);
	context._sub(context.al, 10);
	if (!context.flags.c()) goto findten;
	context.cs.byte(context.di+1) = context.ah;
	context._add(context.al, 10+'0');
	context.cs.byte(context.di+2) = context.al;
	context.ax = context.pop();
	context.cl = '0'-1;
thousandsc:
	context._inc(context.cl);
	context._sub(context.ax, 1000);
	if (!context.flags.c()) goto thousandsc;
	context._add(context.ax, 1000);
	context.cs.byte(context.di+4) = context.cl;
	context.cl = '0'-1;
hundredsc:
	context._inc(context.cl);
	context._sub(context.ax, 100);
	if (!context.flags.c()) goto hundredsc;
	context._add(context.ax, 100);
	context.cs.byte(context.di+5) = context.cl;
	context.cl = '0'-1;
tensc:
	context._inc(context.cl);
	context._sub(context.ax, 10);
	if (!context.flags.c()) goto tensc;
	context._add(context.ax, 10);
	context.cs.byte(context.di+6) = context.cl;
	context._add(context.al, '0');
	context.cs.byte(context.di+7) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void trysoundalloc(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kNeedsoundbuff), 1);
	if (context.flags.z()) goto gotsoundbuff;
	context._inc(context.data.byte(kSoundtimes));
	context.bx = (16384+2048)/16;
	allocatemem(context);
	context.data.word(kSoundbuffer) = context.ax;
	context.push(context.ax);
	context.al = context.ah;
	context.cl = 4;
	context._shr(context.al, context.cl);
	context.data.byte(kSoundbufferpage) = context.al;
	context.ax = context.pop();
	context.cl = 4;
	context._shl(context.ax, context.cl);
	context.data.word(kSoundbufferad) = context.ax;
	context._cmp(context.ax, 0x0b7ff);
	if (!context.flags.c()) goto soundfail;
	context.es = context.data.word(kSoundbuffer);
	context.di = 0;
	context.cx = 16384/2;
	context.ax = 0x7f7f;
	while(context.cx--) 	context._stosw();
	context.data.byte(kNeedsoundbuff) = 1;
	{assert(stack_depth == context.stack.size()); return; }
soundfail:
	context.es = context.data.word(kSoundbuffer);
	deallocatemem(context);
gotsoundbuff:
	{assert(stack_depth == context.stack.size()); return; }
}

void playchannel0(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kSoundint), 255);
	if (context.flags.z()) goto dontbother4;
	context.push(context.es);
	context.push(context.ds);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.di);
	context.push(context.si);
	context.data.byte(kCh0playing) = context.al;
	context.es = context.data.word(kSounddata);
	context._cmp(context.al, 12);
	if (context.flags.c()) goto notsecondbank;
	context.es = context.data.word(kSounddata2);
	context._sub(context.al, 12);
notsecondbank:
	context.data.byte(kCh0repeat) = context.ah;
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.es.byte(context.bx);
	context.ah = 0;
	context.data.word(kCh0emmpage) = context.ax;
	context.ax = context.es.word(context.bx+1);
	context.data.word(kCh0offset) = context.ax;
	context.ax = context.es.word(context.bx+3);
	context.data.word(kCh0blockstocopy) = context.ax;
	context._cmp(context.data.byte(kCh0repeat), 0);
	if (context.flags.z()) goto nosetloop;
	context.ax = context.data.word(kCh0emmpage);
	context.data.word(kCh0oldemmpage) = context.ax;
	context.ax = context.data.word(kCh0offset);
	context.data.word(kCh0oldoffset) = context.ax;
	context.ax = context.data.word(kCh0blockstocopy);
	context.data.word(kCh0oldblockstocopy) = context.ax;
nosetloop:
	context.si = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ds = context.pop();
	context.es = context.pop();
dontbother4:
	{assert(stack_depth == context.stack.size()); return; }
}

void playchannel1(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kSoundint), 255);
	if (context.flags.z()) goto dontbother5;
	context._cmp(context.data.byte(kCh1playing), 7);
	if (context.flags.z()) goto dontbother5;
	context.push(context.es);
	context.push(context.ds);
	context.push(context.bx);
	context.push(context.cx);
	context.push(context.di);
	context.push(context.si);
	context.data.byte(kCh1playing) = context.al;
	context.es = context.data.word(kSounddata);
	context._cmp(context.al, 12);
	if (context.flags.c()) goto notsecondbank1;
	context.es = context.data.word(kSounddata2);
	context._sub(context.al, 12);
notsecondbank1:
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.es.byte(context.bx);
	context.ah = 0;
	context.data.word(kCh1emmpage) = context.ax;
	context.ax = context.es.word(context.bx+1);
	context.data.word(kCh1offset) = context.ax;
	context.ax = context.es.word(context.bx+3);
	context.data.word(kCh1blockstocopy) = context.ax;
	context.si = context.pop();
	context.di = context.pop();
	context.cx = context.pop();
	context.bx = context.pop();
	context.ds = context.pop();
	context.es = context.pop();
dontbother5:
	{assert(stack_depth == context.stack.size()); return; }
}

void makenextblock(Context & context) {
	uint stack_depth = context.stack.size();
	volumeadjust(context);
	loopchannel0(context);
	context._cmp(context.data.word(kCh1blockstocopy), 0);
	if (context.flags.z()) goto mightbeonlych0;
	context._cmp(context.data.word(kCh0blockstocopy), 0);
	if (context.flags.z()) goto mightbeonlych1;
	context._dec(context.data.word(kCh0blockstocopy));
	context._dec(context.data.word(kCh1blockstocopy));
	bothchannels(context);
	{assert(stack_depth == context.stack.size()); return; }
mightbeonlych1:
	context.data.byte(kCh0playing) = 255;
	context._cmp(context.data.word(kCh1blockstocopy), 0);
	if (context.flags.z()) goto notch1only;
	context._dec(context.data.word(kCh1blockstocopy));
	channel1only(context);
notch1only:
	{assert(stack_depth == context.stack.size()); return; }
mightbeonlych0:
	context.data.byte(kCh1playing) = 255;
	context._cmp(context.data.word(kCh0blockstocopy), 0);
	if (context.flags.z()) goto notch0only;
	context._dec(context.data.word(kCh0blockstocopy));
	channel0only(context);
	{assert(stack_depth == context.stack.size()); return; }
notch0only:
	context.es = context.data.word(kSoundbuffer);
	context.di = context.data.word(kSoundbufferwrite);
	context.cx = 1024;
	context.ax = 0x7f7f;
	while(context.cx--) 	context._stosw();
	context._and(context.di, 16384-1);
	context.data.word(kSoundbufferwrite) = context.di;
	{assert(stack_depth == context.stack.size()); return; }
}

void volumeadjust(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kVolumedirection);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto volok;
	context.al = context.data.byte(kVolume);
	context._cmp(context.al, context.data.byte(kVolumeto));
	if (context.flags.z()) goto volfinish;
	context._add(context.data.byte(kVolumecount), 64);
	if (!context.flags.z()) goto volok;
	context.al = context.data.byte(kVolume);
	context._add(context.al, context.data.byte(kVolumedirection));
	context.data.byte(kVolume) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
volfinish:
	context.data.byte(kVolumedirection) = 0;
volok:
	{assert(stack_depth == context.stack.size()); return; }
}

void loopchannel0(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kCh0blockstocopy), 0);
	if (!context.flags.z()) goto notloop;
	context._cmp(context.data.byte(kCh0repeat), 0);
	if (context.flags.z()) goto notloop;
	context._cmp(context.data.byte(kCh0repeat), 255);
	if (context.flags.z()) goto endlessloop;
	context._dec(context.data.byte(kCh0repeat));
endlessloop:
	context.ax = context.data.word(kCh0oldemmpage);
	context.data.word(kCh0emmpage) = context.ax;
	context.ax = context.data.word(kCh0oldoffset);
	context.data.word(kCh0offset) = context.ax;
	context.ax = context.data.word(kCh0blockstocopy);
	context._add(context.ax, context.data.word(kCh0oldblockstocopy));
	context.data.word(kCh0blockstocopy) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
notloop:
	{assert(stack_depth == context.stack.size()); return; }
}

void cancelch0(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kCh0repeat) = 0;
	context.data.word(kCh0blockstocopy) = 0;
	context.data.byte(kCh0playing) = 255;
	{assert(stack_depth == context.stack.size()); return; }
}

void cancelch1(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.word(kCh1blockstocopy) = 0;
	context.data.byte(kCh1playing) = 255;
	{assert(stack_depth == context.stack.size()); return; }
}

void channel0tran(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kVolume), 0);
	if (!context.flags.z()) goto lowvolumetran;
	context.cx = 1024;
	while(context.cx--) 	context._movsw();
 	{assert(stack_depth == context.stack.size()); return; }
lowvolumetran:
	context.cx = 1024;
	context.bh = context.data.byte(kVolume);
	context.bl = 0;
	context._add(context.bx, 16384-256);
volloop:
	context._lodsw();
	context.bl = context.al;
	context.al = context.es.byte(context.bx);
	context.bl = context.ah;
	context.ah = context.es.byte(context.bx);
	context._stosw();
	if (--context.cx) goto volloop;
	{assert(stack_depth == context.stack.size()); return; }
}

void domix(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kVolume), 0);
	if (!context.flags.z()) goto lowvolumemix;
slow:
	context._lodsb();
	context.ah = context.ds.byte(context.bx);
	context._inc(context.bx);
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
	context.bh = context.data.byte(kVolume);
	context._add(context.bh, 63);
	context.bl = context.al;
	context.al = context.es.byte(context.bx);
	context.bx = context.pop();
	context.ah = context.ds.byte(context.bx);
	context._inc(context.bx);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void entrytexts(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLocation), 21);
	if (!context.flags.z()) goto notloc15;
	context.al = 28;
	context.cx = 60;
	context.dx = 11;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	{assert(stack_depth == context.stack.size()); return; }
notloc15:
	context._cmp(context.data.byte(kLocation), 30);
	if (!context.flags.z()) goto notloc43;
	context.al = 27;
	context.cx = 60;
	context.dx = 11;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	{assert(stack_depth == context.stack.size()); return; }
notloc43:
	context._cmp(context.data.byte(kLocation), 23);
	if (!context.flags.z()) goto notloc23;
	context.al = 29;
	context.cx = 60;
	context.dx = 11;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	{assert(stack_depth == context.stack.size()); return; }
notloc23:
	context._cmp(context.data.byte(kLocation), 31);
	if (!context.flags.z()) goto notloc44;
	context.al = 30;
	context.cx = 60;
	context.dx = 11;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	{assert(stack_depth == context.stack.size()); return; }
notloc44:
	context._cmp(context.data.byte(kLocation), 20);
	if (!context.flags.z()) goto notsarters2;
	context.al = 31;
	context.cx = 60;
	context.dx = 11;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	{assert(stack_depth == context.stack.size()); return; }
notsarters2:
	context._cmp(context.data.byte(kLocation), 24);
	if (!context.flags.z()) goto notedenlob;
	context.al = 32;
	context.cx = 60;
	context.dx = 3;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	{assert(stack_depth == context.stack.size()); return; }
notedenlob:
	context._cmp(context.data.byte(kLocation), 34);
	if (!context.flags.z()) goto noteden2;
	context.al = 33;
	context.cx = 60;
	context.dx = 3;
	context.bl = 68;
	context.bh = 64;
	setuptimeduse(context);
	{assert(stack_depth == context.stack.size()); return; }
noteden2:
	{assert(stack_depth == context.stack.size()); return; }
}

void entryanims(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.word(kReeltowatch) = -1;
	context.data.byte(kWatchmode) = -1;
	context._cmp(context.data.byte(kLocation), 33);
	if (!context.flags.z()) goto notinthebeach;
	switchryanoff(context);
	context.data.word(kWatchingtime) = 76*2;
	context.data.word(kReeltowatch) = 0;
	context.data.word(kEndwatchreel) = 76;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notinthebeach:
	context._cmp(context.data.byte(kLocation), 44);
	if (!context.flags.z()) goto notsparkys;
	context.al = 8;
	resetlocation(context);
	context.data.word(kWatchingtime) = 50*2;
	context.data.word(kReeltowatch) = 247;
	context.data.word(kEndwatchreel) = 297;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	switchryanoff(context);
	{assert(stack_depth == context.stack.size()); return; }
notsparkys:
	context._cmp(context.data.byte(kLocation), 22);
	if (!context.flags.z()) goto notinthelift;
	context.data.word(kWatchingtime) = 31*2;
	context.data.word(kReeltowatch) = 0;
	context.data.word(kEndwatchreel) = 30;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	switchryanoff(context);
	{assert(stack_depth == context.stack.size()); return; }
notinthelift:
	context._cmp(context.data.byte(kLocation), 26);
	if (!context.flags.z()) goto notunderchurch;
	context.data.byte(kSymboltopnum) = 2;
	context.data.byte(kSymbolbotnum) = 1;
	{assert(stack_depth == context.stack.size()); return; }
notunderchurch:
	context._cmp(context.data.byte(kLocation), 45);
	if (!context.flags.z()) goto notenterdream;
	context.data.byte(kKeeperflag) = 0;
	context.data.word(kWatchingtime) = 296;
	context.data.word(kReeltowatch) = 45;
	context.data.word(kEndwatchreel) = 198;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	switchryanoff(context);
	{assert(stack_depth == context.stack.size()); return; }
notenterdream:
	context._cmp(context.data.byte(kReallocation), 46);
	if (!context.flags.z()) goto notcrystal;
	context._cmp(context.data.byte(kSartaindead), 1);
	if (!context.flags.z()) goto notcrystal;
	context.al = 0;
	removefreeobject(context);
	{assert(stack_depth == context.stack.size()); return; }
notcrystal:
	context._cmp(context.data.byte(kLocation), 9);
	if (!context.flags.z()) goto nottopchurch;
	context.al = 2;
	checkifpathison(context);
	if (context.flags.z()) goto nottopchurch;
	context._cmp(context.data.byte(kAidedead), 0);
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
	{assert(stack_depth == context.stack.size()); return; }
nottopchurch:
	context._cmp(context.data.byte(kLocation), 47);
	if (!context.flags.z()) goto notdreamcentre;
	context.al = 4;
	placesetobject(context);
	context.al = 5;
	placesetobject(context);
	{assert(stack_depth == context.stack.size()); return; }
notdreamcentre:
	context._cmp(context.data.byte(kLocation), 38);
	if (!context.flags.z()) goto notcarpark;
	context.data.word(kWatchingtime) = 57*2;
	context.data.word(kReeltowatch) = 4;
	context.data.word(kEndwatchreel) = 57;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	switchryanoff(context);
	{assert(stack_depth == context.stack.size()); return; }
notcarpark:
	context._cmp(context.data.byte(kLocation), 32);
	if (!context.flags.z()) goto notalley;
	context.data.word(kWatchingtime) = 66*2;
	context.data.word(kReeltowatch) = 0;
	context.data.word(kEndwatchreel) = 66;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	switchryanoff(context);
	{assert(stack_depth == context.stack.size()); return; }
notalley:
	context._cmp(context.data.byte(kLocation), 24);
	if (!context.flags.z()) goto notedensagain;
	context.al = 2;
	context.ah = context.data.byte(kRoomnum);
	context._dec(context.ah);
	turnanypathon(context);
notedensagain:
	{assert(stack_depth == context.stack.size()); return; }
}

void initialinv(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kReallocation), 24);
	if (context.flags.z()) goto isedens;
	{assert(stack_depth == context.stack.size()); return; }
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
	context.data.byte(kWatchmode) = 1;
	context.data.word(kReeltohold) = 0;
	context.data.word(kEndofholdreel) = 6;
	context.data.byte(kWatchspeed) = 1;
	context.data.byte(kSpeedcount) = 1;
	switchryanoff(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void pickupob(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kLastinvpos) = context.ah;
	context.data.byte(kObjecttype) = 2;
	context.data.byte(kItemframe) = context.al;
	context.data.byte(kCommand) = context.al;
	getanyad(context);
	transfertoex(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void checkforemm(Context & context) {
	uint stack_depth = context.stack.size();
	{assert(stack_depth == context.stack.size()); return; }
}

void checkbasemem(Context & context) {
	uint stack_depth = context.stack.size();
	context.bx = context.data.word(kHowmuchalloc);
	context._cmp(context.bx, 0x9360);
	if (!context.flags.c()) goto enoughmem;
	context.data.byte(kGameerror) = 5;
	{ quickquit(context); return; };
enoughmem:
	{assert(stack_depth == context.stack.size()); return; }
}

void allocatebuffers(Context & context) {
	uint stack_depth = context.stack.size();
	context.bx = (0+2080+30000+(16*114)+((114+2)*2)+18000)/16;
	allocatemem(context);
	context.data.word(kExtras) = context.ax;
	trysoundalloc(context);
	context.bx = (0+(66*60))/16;
	allocatemem(context);
	context.data.word(kMapdata) = context.ax;
	trysoundalloc(context);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*24)+(6*64)+991-534+68-0)/16;
	allocatemem(context);
	context.data.word(kBuffers) = context.ax;
	trysoundalloc(context);
	context.bx = (16*80)/16;
	allocatemem(context);
	context.data.word(kFreedat) = context.ax;
	trysoundalloc(context);
	context.bx = (64*128)/16;
	allocatemem(context);
	context.data.word(kSetdat) = context.ax;
	trysoundalloc(context);
	context.bx = (22*8*20*8)/16;
	allocatemem(context);
	context.data.word(kMapstore) = context.ax;
	allocatework(context);
	context.bx = 2048/16;
	allocatemem(context);
	context.data.word(kSounddata) = context.ax;
	context.bx = 2048/16;
	allocatemem(context);
	context.data.word(kSounddata2) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void clearbuffers(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.cx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*24)+(6*64)+991-534+68-0)/2;
	context.ax = 0;
	context.di = 0;
	while(context.cx--) 	context._stosw();
	context.es = context.data.word(kExtras);
	context.cx = (0+2080+30000+(16*114)+((114+2)*2)+18000)/2;
	context.ax = 0x0ffff;
	context.di = 0;
	while(context.cx--) 	context._stosw();
	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*24)+(6*64));
	context.ds = context.cs;
	context.si = 534;
	context.cx = (991-534);
	while(context.cx--) 	context._movsb();
 	context.es = context.data.word(kBuffers);
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*24)+(6*64)+991-534);
	context.ds = context.cs;
	context.si = 0;
	context.cx = (68-0);
	while(context.cx--) 	context._movsb();
 	clearchanges(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void clearchanges(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.cx = (250)*2;
	context.ax = 0x0ffff;
	context.di = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80));
	while(context.cx--) 	context._stosw();
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*24)+(6*64));
	context.es = context.cs;
	context.di = 534;
	context.cx = (991-534);
	while(context.cx--) 	context._movsb();
 	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5)+(46*40)+(5*80)+(250*4)+(256*24)+(6*64)+991-534);
	context.es = context.cs;
	context.di = 0;
	context.cx = (68-0);
	while(context.cx--) 	context._movsb();
 	context.data.byte(kExpos) = 0;
	context.data.word(kExframepos) = 0;
	context.data.word(kExtextpos) = 0;
	context.es = context.data.word(kExtras);
	context.cx = (0+2080+30000+(16*114)+((114+2)*2)+18000)/2;
	context.ax = 0x0ffff;
	context.di = 0;
	while(context.cx--) 	context._stosw();
	context.es = context.cs;
	context.di = 7782;
	context.al = 1;
	context._stosb();
	context._stosb();
	context.al = 0;
	context._stosb();
	context.al = 1;
	context._stosb();
	context.ax = 0;
	context.cx = 6;
	while(context.cx--) 	context._stosw();
	{assert(stack_depth == context.stack.size()); return; }
}

void clearbeforeload(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kRoomloaded), 1);
	if (!context.flags.z()) goto noclear;
	clearreels(context);
	clearrest(context);
	context.data.byte(kRoomloaded) = 0;
noclear:
	{assert(stack_depth == context.stack.size()); return; }
}

void clearreels(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kReel1);
	deallocatemem(context);
	context.es = context.data.word(kReel2);
	deallocatemem(context);
	context.es = context.data.word(kReel3);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void clearrest(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kMapdata);
	context.cx = (66*60)/2;
	context.ax = 0;
	context.di = (0);
	while(context.cx--) 	context._stosw();
	context.es = context.data.word(kBackdrop);
	deallocatemem(context);
	context.es = context.data.word(kSetframes);
	deallocatemem(context);
	context.es = context.data.word(kReels);
	deallocatemem(context);
	context.es = context.data.word(kPeople);
	deallocatemem(context);
	context.es = context.data.word(kSetdesc);
	deallocatemem(context);
	context.es = context.data.word(kBlockdesc);
	deallocatemem(context);
	context.es = context.data.word(kRoomdesc);
	deallocatemem(context);
	context.es = context.data.word(kFreeframes);
	deallocatemem(context);
	context.es = context.data.word(kFreedesc);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void parseblaster(Context & context) {
	uint stack_depth = context.stack.size();
lookattail:
	context.al = context.es.byte(context.bx);
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
	context._inc(context.bx);
	if (--context.cx) goto lookattail;
	{assert(stack_depth == context.stack.size()); return; }
issoundint:
	context.al = context.es.byte(context.bx+1);
	context._sub(context.al, '0');
	context.data.byte(kSoundint) = context.al;
	context._inc(context.bx);
	goto lookattail;
isdma:
	context.al = context.es.byte(context.bx+1);
	context._sub(context.al, '0');
	context.data.byte(kSounddmachannel) = context.al;
	context._inc(context.bx);
	goto lookattail;
isbaseadd:
	context.push(context.cx);
	context.al = context.es.byte(context.bx+2);
	context._sub(context.al, '0');
	context.ah = 0;
	context.cl = 4;
	context._shl(context.ax, context.cl);
	context._add(context.ax, 0x200);
	context.data.word(kSoundbaseadd) = context.ax;
	context.cx = context.pop();
	context._inc(context.bx);
	goto lookattail;
isbright:
	context.data.byte(kBrightness) = 1;
	context._inc(context.bx);
	goto lookattail;
isnosound:
	context.data.byte(kSoundint) = 255;
	context._inc(context.bx);
	goto lookattail;
endtail:
	{assert(stack_depth == context.stack.size()); return; }
}

void startup(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kCurrentkey) = 0;
	context.data.byte(kMainmode) = 0;
	createpanel(context);
	context.data.byte(kNewobs) = 1;
	drawfloor(context);
	showicon(context);
	getunderzoom(context);
	spriteupdate(context);
	printsprites(context);
	undertextline(context);
	reelsonscreen(context);
	atmospheres(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void startup1(Context & context) {
	uint stack_depth = context.stack.size();
	clearpalette(context);
	context.data.byte(kThroughdoor) = 0;
	context.data.byte(kCurrentkey) = '0';
	context.data.byte(kMainmode) = 0;
	createpanel(context);
	context.data.byte(kNewobs) = 1;
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
	{assert(stack_depth == context.stack.size()); return; }
}

void screenupdate(Context & context) {
	uint stack_depth = context.stack.size();
	newplace(context);
	mainscreen(context);
	animpointer(context);
	showpointer(context);
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) goto iswatchingmode;
	context._cmp(context.data.byte(kNewlocation), 255);
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
	context._cmp(context.data.byte(kWongame), 0);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void watchreel(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kReeltowatch), -1);
	if (context.flags.z()) goto notplayingreel;
	context.al = context.data.byte(kManspath);
	context._cmp(context.al, context.data.byte(kFinaldest));
	if (!context.flags.z()) goto waitstopwalk;
	context.al = context.data.byte(kTurntoface);
	context._cmp(context.al, context.data.byte(kFacing));
	if (context.flags.z()) goto notwatchpath;
waitstopwalk:
	{assert(stack_depth == context.stack.size()); return; }
notwatchpath:
	context._dec(context.data.byte(kSpeedcount));
	context._cmp(context.data.byte(kSpeedcount), -1);
	if (!context.flags.z()) goto showwatchreel;
	context.al = context.data.byte(kWatchspeed);
	context.data.byte(kSpeedcount) = context.al;
	context.ax = context.data.word(kReeltowatch);
	context._cmp(context.ax, context.data.word(kEndwatchreel));
	if (!context.flags.z()) goto ismorereel;
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) goto showwatchreel;
	context.data.word(kReeltowatch) = -1;
	context.data.byte(kWatchmode) = -1;
	context._cmp(context.data.word(kReeltohold), -1);
	if (context.flags.z()) goto nomorereel;
	context.data.byte(kWatchmode) = 1;
	goto notplayingreel;
ismorereel:
	context._inc(context.data.word(kReeltowatch));
showwatchreel:
	context.ax = context.data.word(kReeltowatch);
	context.data.word(kReelpointer) = context.ax;
	plotreel(context);
	context.ax = context.data.word(kReelpointer);
	context.data.word(kReeltowatch) = context.ax;
	checkforshake(context);
nomorereel:
	{assert(stack_depth == context.stack.size()); return; }
notplayingreel:
	context._cmp(context.data.byte(kWatchmode), 1);
	if (!context.flags.z()) goto notholdingreel;
	context.ax = context.data.word(kReeltohold);
	context.data.word(kReelpointer) = context.ax;
	plotreel(context);
	{assert(stack_depth == context.stack.size()); return; }
notholdingreel:
	context._cmp(context.data.byte(kWatchmode), 2);
	if (!context.flags.z()) goto notreleasehold;
	context._dec(context.data.byte(kSpeedcount));
	context._cmp(context.data.byte(kSpeedcount), -1);
	if (!context.flags.z()) goto notlastspeed2;
	context.al = context.data.byte(kWatchspeed);
	context.data.byte(kSpeedcount) = context.al;
	context._inc(context.data.word(kReeltohold));
notlastspeed2:
	context.ax = context.data.word(kReeltohold);
	context._cmp(context.ax, context.data.word(kEndofholdreel));
	if (!context.flags.z()) goto ismorereel2;
	context.data.word(kReeltohold) = -1;
	context.data.byte(kWatchmode) = -1;
	context.al = context.data.byte(kDestafterhold);
	context.data.byte(kDestination) = context.al;
	context.data.byte(kFinaldest) = context.al;
	autosetwalk(context);
	{assert(stack_depth == context.stack.size()); return; }
ismorereel2:
	context.ax = context.data.word(kReeltohold);
	context.data.word(kReelpointer) = context.ax;
	plotreel(context);
	{assert(stack_depth == context.stack.size()); return; }
notreleasehold:
	{assert(stack_depth == context.stack.size()); return; }
}

void checkforshake(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kReallocation), 26);
	if (!context.flags.z()) goto notstartshake;
	context._cmp(context.ax, 104);
	if (!context.flags.z()) goto notstartshake;
	context.data.byte(kShakecounter) = -1;
notstartshake:
	{assert(stack_depth == context.stack.size()); return; }
}

void watchcount(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWatchon), 0);
	if (context.flags.z()) goto nowatchworn;
	context._inc(context.data.byte(kTimercount));
	context._cmp(context.data.byte(kTimercount), 9);
	if (context.flags.z()) goto flashdots;
	context._cmp(context.data.byte(kTimercount), 18);
	if (context.flags.z()) goto uptime;
nowatchworn:
	{assert(stack_depth == context.stack.size()); return; }
flashdots:
	context.ax = 91*3+21;
	context.di = 268+4;
	context.bx = 21;
	context.ds = context.data.word(kCharset1);
	showframe(context);
	goto finishwatch;
uptime:
	context.data.byte(kTimercount) = 0;
	context._add(context.data.byte(kSecondcount), 1);
	context._cmp(context.data.byte(kSecondcount), 60);
	if (!context.flags.z()) goto finishtime;
	context.data.byte(kSecondcount) = 0;
	context._inc(context.data.byte(kMinutecount));
	context._cmp(context.data.byte(kMinutecount), 60);
	if (!context.flags.z()) goto finishtime;
	context.data.byte(kMinutecount) = 0;
	context._inc(context.data.byte(kHourcount));
	context._cmp(context.data.byte(kHourcount), 24);
	if (!context.flags.z()) goto finishtime;
	context.data.byte(kHourcount) = 0;
finishtime:
	showtime(context);
finishwatch:
	context.data.byte(kWatchdump) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void showtime(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWatchon), 0);
	if (context.flags.z()) goto nowatch;
	context.al = context.data.byte(kSecondcount);
	context.cl = 0;
	twodigitnum(context);
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context._add(context.ax, 91*3+10);
	context.ds = context.data.word(kCharset1);
	context.di = 282+5;
	context.bx = 21;
	showframe(context);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, 91*3+10);
	context.ds = context.data.word(kCharset1);
	context.di = 282+9;
	context.bx = 21;
	showframe(context);
	context.al = context.data.byte(kMinutecount);
	context.cl = 0;
	twodigitnum(context);
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context._add(context.ax, 91*3);
	context.ds = context.data.word(kCharset1);
	context.di = 270+5;
	context.bx = 21;
	showframe(context);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, 91*3);
	context.ds = context.data.word(kCharset1);
	context.di = 270+11;
	context.bx = 21;
	showframe(context);
	context.al = context.data.byte(kHourcount);
	context.cl = 0;
	twodigitnum(context);
	context.push(context.ax);
	context.al = context.ah;
	context.ah = 0;
	context._add(context.ax, 91*3);
	context.ds = context.data.word(kCharset1);
	context.di = 256+5;
	context.bx = 21;
	showframe(context);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, 91*3);
	context.ds = context.data.word(kCharset1);
	context.di = 256+11;
	context.bx = 21;
	showframe(context);
	context.ax = 91*3+20;
	context.ds = context.data.word(kCharset1);
	context.di = 267+5;
	context.bx = 21;
	showframe(context);
nowatch:
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpwatch(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWatchdump), 1);
	if (!context.flags.z()) goto nodumpwatch;
	context.di = 256;
	context.bx = 21;
	context.cl = 40;
	context.ch = 12;
	multidump(context);
	context.data.byte(kWatchdump) = 0;
nodumpwatch:
	{assert(stack_depth == context.stack.size()); return; }
}

void showbyte(Context & context) {
	uint stack_depth = context.stack.size();
	context.dl = context.al;
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	onedigit(context);
	context.es.byte(context.di) = context.dl;
	context.dl = context.al;
	context._and(context.dl, 15);
	onedigit(context);
	context.es.byte(context.di+1) = context.dl;
	context._add(context.di, 3);
	{assert(stack_depth == context.stack.size()); return; }
}

void onedigit(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.dl, 10);
	if (!context.flags.c()) goto morethan10;
	context._add(context.dl, '0');
	{assert(stack_depth == context.stack.size()); return; }
morethan10:
	context._sub(context.dl, 10);
	context._add(context.dl, 'A');
	{assert(stack_depth == context.stack.size()); return; }
}

void twodigitnum(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = context.cl;
	context._dec(context.ah);
numloop1:
	context._inc(context.ah);
	context._sub(context.al, 10);
	if (!context.flags.c()) goto numloop1;
	context._add(context.al, 10);
	context._add(context.al, context.cl);
	{assert(stack_depth == context.stack.size()); return; }
}

void showword(Context & context) {
	uint stack_depth = context.stack.size();
	context.ch = 0;
	context.bx = 10000;
	context.cl = 47;
word1:
	context._inc(context.cl);
	context._sub(context.ax, context.bx);
	if (!context.flags.c()) goto word1;
	context._add(context.ax, context.bx);
	convnum(context);
	context.cs.byte(context.di) = context.cl;
	context.bx = 1000;
	context.cl = 47;
word2:
	context._inc(context.cl);
	context._sub(context.ax, context.bx);
	if (!context.flags.c()) goto word2;
	context._add(context.ax, context.bx);
	convnum(context);
	context.cs.byte(context.di+1) = context.cl;
	context.bx = 100;
	context.cl = 47;
word3:
	context._inc(context.cl);
	context._sub(context.ax, context.bx);
	if (!context.flags.c()) goto word3;
	context._add(context.ax, context.bx);
	convnum(context);
	context.cs.byte(context.di+2) = context.cl;
	context.bx = 10;
	context.cl = 47;
word4:
	context._inc(context.cl);
	context._sub(context.ax, context.bx);
	if (!context.flags.c()) goto word4;
	context._add(context.ax, context.bx);
	convnum(context);
	context.cs.byte(context.di+3) = context.cl;
	context._add(context.al, 48);
	context.cl = context.al;
	convnum(context);
	context.cs.byte(context.di+4) = context.cl;
	{assert(stack_depth == context.stack.size()); return; }
}

void convnum(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.ch, 0);
	if (!context.flags.z()) goto noconvnum;
	context._cmp(context.cl, '0');
	if (!context.flags.z()) goto notzeronum;
	context.cl = 32;
	goto noconvnum;
notzeronum:
	context.ch = 1;
noconvnum:
	{assert(stack_depth == context.stack.size()); return; }
}

void walkandexamine(Context & context) {
	uint stack_depth = context.stack.size();
	finishedwalking(context);
	if (!context.flags.z()) goto noobselect;
	context.al = context.data.byte(kWalkexamtype);
	context.data.byte(kCommandtype) = context.al;
	context.al = context.data.byte(kWalkexamnum);
	context.data.byte(kCommand) = context.al;
	context.data.byte(kWalkandexam) = 0;
	context._cmp(context.data.byte(kCommandtype), 5);
	if (context.flags.z()) goto noobselect;
	examineob(context);
	{assert(stack_depth == context.stack.size()); return; }
wantstowalk:
	setwalk(context);
	context.data.byte(kReasseschanges) = 1;
noobselect:
	{assert(stack_depth == context.stack.size()); return; }
diff:
	context.data.byte(kCommand) = context.al;
	context.data.byte(kCommandtype) = context.ah;
diff2:
	context._cmp(context.data.byte(kLinepointer), 254);
	if (!context.flags.z()) goto middleofwalk;
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) goto middleofwalk;
	context.al = context.data.byte(kFacing);
	context._cmp(context.al, context.data.byte(kTurntoface));
	if (!context.flags.z()) goto middleofwalk;
	context._cmp(context.data.byte(kCommandtype), 3);
	if (!context.flags.z()) goto notblock;
	context.bl = context.data.byte(kManspath);
	context._cmp(context.bl, context.data.byte(kPointerspath));
	if (!context.flags.z()) goto dontcheck;
	context.cl = context.data.byte(kRyanx);
	context._add(context.cl, 12);
	context.ch = context.data.byte(kRyany);
	context._add(context.ch, 12);
	checkone(context);
	context._cmp(context.cl, 2);
	if (context.flags.c()) goto isblock;
dontcheck:
	getflagunderp(context);
	context._cmp(context.data.byte(kLastflag), 2);
	if (context.flags.c()) goto isblock;
	context._cmp(context.data.byte(kLastflag), 128);
	if (!context.flags.c()) goto isblock;
	goto toofaraway;
notblock:
	context.bl = context.data.byte(kManspath);
	context._cmp(context.bl, context.data.byte(kPointerspath));
	if (!context.flags.z()) goto toofaraway;
	context._cmp(context.data.byte(kCommandtype), 3);
	if (context.flags.z()) goto isblock;
	context._cmp(context.data.byte(kCommandtype), 5);
	if (context.flags.z()) goto isaperson;
	examineobtext(context);
	{assert(stack_depth == context.stack.size()); return; }
middleofwalk:
	blocknametext(context);
	{assert(stack_depth == context.stack.size()); return; }
isblock:
	blocknametext(context);
	{assert(stack_depth == context.stack.size()); return; }
isaperson:
	personnametext(context);
	{assert(stack_depth == context.stack.size()); return; }
toofaraway:
	walktotext(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void mainscreen(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kInmaparea) = 0;
	context.bx = 4949;
	context._cmp(context.data.byte(kWatchon), 1);
	if (context.flags.z()) goto checkmain;
	context.bx = 5011;
checkmain:
	checkcoords(context);
	context._cmp(context.data.byte(kWalkandexam), 0);
	if (context.flags.z()) goto finishmain;
	walkandexamine(context);
finishmain:
	{assert(stack_depth == context.stack.size()); return; }
}

void madmanrun(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLocation), 14);
	if (!context.flags.z()) { identifyob(context); return; };
	context._cmp(context.data.byte(kMapx), 22);
	if (!context.flags.z()) { identifyob(context); return; };
	context._cmp(context.data.byte(kPointermode), 2);
	if (!context.flags.z()) { identifyob(context); return; };
	context._cmp(context.data.byte(kMadmanflag), 0);
	if (!context.flags.z()) { identifyob(context); return; };
	context._cmp(context.data.byte(kCommandtype), 211);
	if (context.flags.z()) goto alreadyrun;
	context.data.byte(kCommandtype) = 211;
	context.al = 52;
	commandonly(context);
alreadyrun:
	context._cmp(context.data.word(kMousebutton), 1);
	if (!context.flags.z()) goto norun;
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (context.flags.z()) goto norun;
	context.data.byte(kLastweapon) = 8;
norun:
	{assert(stack_depth == context.stack.size()); return; }
}

void checkcoords(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kNewlocation), 255);
	if (context.flags.z()) goto loop048;
	{assert(stack_depth == context.stack.size()); return; }
loop048:
	context.ax = context.cs.word(context.bx);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto nonefound;
	context.push(context.bx);
	context._cmp(context.data.word(kMousex), context.ax);
	if (context.flags.l()) goto over045;
	context.ax = context.cs.word(context.bx+2);
	context._cmp(context.data.word(kMousex), context.ax);
	if (!context.flags.l()) goto over045;
	context.ax = context.cs.word(context.bx+4);
	context._cmp(context.data.word(kMousey), context.ax);
	if (context.flags.l()) goto over045;
	context.ax = context.cs.word(context.bx+6);
	context._cmp(context.data.word(kMousey), context.ax);
	if (!context.flags.l()) goto over045;
	context.ax = context.cs.word(context.bx+8);
	__dispatch_call(context, context.ax);
finished:
	context.ax = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
over045:
	context.bx = context.pop();
	context._add(context.bx, 10);
	goto loop048;
nonefound:
	{assert(stack_depth == context.stack.size()); return; }
}

void identifyob(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) { blank(context); return; };
	context.ax = context.data.word(kMousex);
	context._sub(context.ax, context.data.word(kMapadx));
	context._cmp(context.ax, 22*8);
	if (context.flags.c()) goto notover1;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
notover1:
	context.bx = context.data.word(kMousey);
	context._sub(context.bx, context.data.word(kMapady));
	context._cmp(context.bx, 20*8);
	if (context.flags.c()) goto notover2;
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
notover2:
	context.data.byte(kInmaparea) = 1;
	context.ah = context.bl;
	context.push(context.ax);
	findpathofpoint(context);
	context.data.byte(kPointerspath) = context.dl;
	context.ax = context.pop();
	context.push(context.ax);
	findfirstpath(context);
	context.data.byte(kPointerfirstpath) = context.al;
	context.ax = context.pop();
	checkifex(context);
	if (!context.flags.z()) goto finishidentify;
	checkiffree(context);
	if (!context.flags.z()) goto finishidentify;
	checkifperson(context);
	if (!context.flags.z()) goto finishidentify;
	checkifset(context);
	if (!context.flags.z()) goto finishidentify;
	context.ax = context.data.word(kMousex);
	context._sub(context.ax, context.data.word(kMapadx));
	context.cl = context.al;
	context.ax = context.data.word(kMousey);
	context._sub(context.ax, context.data.word(kMapady));
	context.ch = context.al;
	checkone(context);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto nothingund;
	context._cmp(context.data.byte(kMandead), 1);
	if (context.flags.z()) goto nothingund;
	context.ah = 3;
	obname(context);
finishidentify:
	{assert(stack_depth == context.stack.size()); return; }
nothingund:
	blank(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void checkifperson(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5));
	context.cx = 12;
identifyreel:
	context.push(context.cx);
	context._cmp(context.es.byte(context.bx+4), 255);
	if (context.flags.z()) goto notareelid;
	context.push(context.es);
	context.push(context.bx);
	context.push(context.ax);
	context.ax = context.es.word(context.bx+0);
	context.data.word(kReelpointer) = context.ax;
	getreelstart(context);
	context._cmp(context.es.word(context.si+2), 0x0ffff);
	if (!context.flags.z()) goto notblankpers;
	context._add(context.si, 5);
notblankpers:
	context.cx = context.es.word(context.si+2);
	context.ax = context.es.word(context.si+0);
	context.push(context.cx);
	getreelframeax(context);
	context.cx = context.pop();
	context._add(context.cl, context.es.byte(context.bx+4));
	context._add(context.ch, context.es.byte(context.bx+5));
	context.dx = context.cx;
	context._add(context.dl, context.es.byte(context.bx+0));
	context._add(context.dh, context.es.byte(context.bx+1));
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
	context.ax = context.es.word(context.bx+2);
	context.data.word(kPersondata) = context.ax;
	context.al = context.es.byte(context.bx+4);
	context.ah = 5;
	obname(context);
	context.al = 0;
	context._cmp(context.al, 1);
	{assert(stack_depth == context.stack.size()); return; }
notareelid:
	context.cx = context.pop();
	context._add(context.bx, 5);
	context._dec(context.cx);
	if (!context.flags.z()) goto identifyreel;
	{assert(stack_depth == context.stack.size()); return; }
}

void checkifset(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32))+(127*5);
	context.cx = 127;
identifyset:
	context._cmp(context.es.byte(context.bx+4), 255);
	if (context.flags.z()) goto notasetid;
	context._cmp(context.al, context.es.byte(context.bx));
	if (context.flags.c()) goto notasetid;
	context._cmp(context.al, context.es.byte(context.bx+2));
	if (!context.flags.c()) goto notasetid;
	context._cmp(context.ah, context.es.byte(context.bx+1));
	if (context.flags.c()) goto notasetid;
	context._cmp(context.ah, context.es.byte(context.bx+3));
	if (!context.flags.c()) goto notasetid;
	pixelcheckset(context);
	if (context.flags.z()) goto notasetid;
	isitdescribed(context);
	if (context.flags.z()) goto notasetid;
	context.al = context.es.byte(context.bx+4);
	context.ah = 1;
	obname(context);
	context.al = 0;
	context._cmp(context.al, 1);
	{assert(stack_depth == context.stack.size()); return; }
notasetid:
	context._sub(context.bx, 5);
	context._dec(context.cx);
	context._cmp(context.cx, -1);
	if (!context.flags.z()) goto identifyset;
	{assert(stack_depth == context.stack.size()); return; }
}

void checkifex(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5))+(99*5);
	context.cx = 99;
identifyex:
	context._cmp(context.es.byte(context.bx+4), 255);
	if (context.flags.z()) goto notanexid;
	context._cmp(context.al, context.es.byte(context.bx));
	if (context.flags.c()) goto notanexid;
	context._cmp(context.al, context.es.byte(context.bx+2));
	if (!context.flags.c()) goto notanexid;
	context._cmp(context.ah, context.es.byte(context.bx+1));
	if (context.flags.c()) goto notanexid;
	context._cmp(context.ah, context.es.byte(context.bx+3));
	if (!context.flags.c()) goto notanexid;
	context.al = context.es.byte(context.bx+4);
	context.ah = 4;
	obname(context);
	context.al = 1;
	context._cmp(context.al, 0);
	{assert(stack_depth == context.stack.size()); return; }
notanexid:
	context._sub(context.bx, 5);
	context._dec(context.cx);
	context._cmp(context.cx, -1);
	if (!context.flags.z()) goto identifyex;
	{assert(stack_depth == context.stack.size()); return; }
}

void checkiffree(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBuffers);
	context.bx = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5))+(79*5);
	context.cx = 79;
identifyfree:
	context._cmp(context.es.byte(context.bx+4), 255);
	if (context.flags.z()) goto notafreeid;
	context._cmp(context.al, context.es.byte(context.bx));
	if (context.flags.c()) goto notafreeid;
	context._cmp(context.al, context.es.byte(context.bx+2));
	if (!context.flags.c()) goto notafreeid;
	context._cmp(context.ah, context.es.byte(context.bx+1));
	if (context.flags.c()) goto notafreeid;
	context._cmp(context.ah, context.es.byte(context.bx+3));
	if (!context.flags.c()) goto notafreeid;
	context.al = context.es.byte(context.bx+4);
	context.ah = 2;
	obname(context);
	context.al = 0;
	context._cmp(context.al, 1);
	{assert(stack_depth == context.stack.size()); return; }
notafreeid:
	context._sub(context.bx, 5);
	context._dec(context.cx);
	context._cmp(context.cx, -1);
	if (!context.flags.z()) goto identifyfree;
	{assert(stack_depth == context.stack.size()); return; }
}

void isitdescribed(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context.push(context.cx);
	context.push(context.es);
	context.push(context.bx);
	context.al = context.es.byte(context.bx+4);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.es = context.data.word(kSetdesc);
	context._add(context.bx, (0));
	context.ax = context.es.word(context.bx);
	context._add(context.ax, (0+(130*2)));
	context.bx = context.ax;
	context.dl = context.es.byte(context.bx);
	context.bx = context.pop();
	context.es = context.pop();
	context.cx = context.pop();
	context.ax = context.pop();
	context._cmp(context.dl, 0);
	{assert(stack_depth == context.stack.size()); return; }
}

void findpathofpoint(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context.bx = (0);
	context.es = context.data.word(kReels);
	context.al = context.data.byte(kRoomnum);
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.cx = context.pop();
	context.dl = 0;
pathloop:
	context.al = context.es.byte(context.bx+6);
	context._cmp(context.al, 255);
	if (!context.flags.z()) goto flunkedit;
	context.ax = context.es.word(context.bx+2);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto flunkedit;
	context._cmp(context.cl, context.al);
	if (context.flags.c()) goto flunkedit;
	context._cmp(context.ch, context.ah);
	if (context.flags.c()) goto flunkedit;
	context.ax = context.es.word(context.bx+4);
	context._cmp(context.cl, context.al);
	if (!context.flags.c()) goto flunkedit;
	context._cmp(context.ch, context.ah);
	if (!context.flags.c()) goto flunkedit;
	goto gotvalidpath;
flunkedit:
	context._add(context.bx, 8);
	context._inc(context.dl);
	context._cmp(context.dl, 12);
	if (!context.flags.z()) goto pathloop;
	context.dl = 255;
gotvalidpath:
	{assert(stack_depth == context.stack.size()); return; }
}

void findfirstpath(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context.bx = (0);
	context.es = context.data.word(kReels);
	context.al = context.data.byte(kRoomnum);
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context._add(context.bx, context.ax);
	context.cx = context.pop();
	context.dl = 0;
fpathloop:
	context.ax = context.es.word(context.bx+2);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto nofirst;
	context._cmp(context.cl, context.al);
	if (context.flags.c()) goto nofirst;
	context._cmp(context.ch, context.ah);
	if (context.flags.c()) goto nofirst;
	context.ax = context.es.word(context.bx+4);
	context._cmp(context.cl, context.al);
	if (!context.flags.c()) goto nofirst;
	context._cmp(context.ch, context.ah);
	if (!context.flags.c()) goto nofirst;
	goto gotfirst;
nofirst:
	context._add(context.bx, 8);
	context._inc(context.dl);
	context._cmp(context.dl, 12);
	if (!context.flags.z()) goto fpathloop;
	context.al = 0;
	{assert(stack_depth == context.stack.size()); return; }
gotfirst:
	context.al = context.es.byte(context.bx+6);
	{assert(stack_depth == context.stack.size()); return; }
}

void turnpathon(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context.push(context.ax);
	context.cl = 255;
	context.ch = context.data.byte(kRoomnum);
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
	context.es.byte(context.bx+6) = context.al;
nopathon:
	{assert(stack_depth == context.stack.size()); return; }
}

void turnpathoff(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	context.push(context.ax);
	context.cl = 0;
	context.ch = context.data.byte(kRoomnum);
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
	context.es.byte(context.bx+6) = context.al;
nopathoff:
	{assert(stack_depth == context.stack.size()); return; }
}

void turnanypathon(Context & context) {
	uint stack_depth = context.stack.size();
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
	context.es = context.data.word(kReels);
	context.bx = (0);
	context._add(context.bx, context.ax);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = 255;
	context.es.byte(context.bx+6) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void turnanypathoff(Context & context) {
	uint stack_depth = context.stack.size();
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
	context.es = context.data.word(kReels);
	context.bx = (0);
	context._add(context.bx, context.ax);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = 0;
	context.es.byte(context.bx+6) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void checkifpathison(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.ax);
	getroomspaths(context);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.es.byte(context.bx+6);
	context._cmp(context.al, 255);
	{assert(stack_depth == context.stack.size()); return; }
}

void afternewroom(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kNowinnewroom), 0);
	if (context.flags.z()) goto notnew;
	context.data.word(kTimecount) = 0;
	createpanel(context);
	context.data.byte(kCommandtype) = 0;
	findroominloc(context);
	context._cmp(context.data.byte(kRyanon), 1);
	if (context.flags.z()) goto ryansoff;
	context.al = context.data.byte(kRyanx);
	context._add(context.al, 12);
	context.ah = context.data.byte(kRyany);
	context._add(context.ah, 12);
	findpathofpoint(context);
	context.data.byte(kManspath) = context.dl;
	findxyfrompath(context);
	context.data.byte(kResetmanxy) = 1;
ryansoff:
	context.data.byte(kNewobs) = 1;
	drawfloor(context);
	context.data.word(kLookcounter) = 160;
	context.data.byte(kNowinnewroom) = 0;
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
	{assert(stack_depth == context.stack.size()); return; }
}

void atmospheres(Context & context) {
	uint stack_depth = context.stack.size();
	context.cl = context.data.byte(kMapx);
	context.ch = context.data.byte(kMapy);
	context.bx = 5073;
nextatmos:
	context.al = context.cs.byte(context.bx);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto nomoreatmos;
	context._cmp(context.al, context.data.byte(kReallocation));
	if (!context.flags.z()) goto wrongatmos;
	context.ax = context.cs.word(context.bx+1);
	context._cmp(context.ax, context.cx);
	if (!context.flags.z()) goto wrongatmos;
	context.ax = context.cs.word(context.bx+3);
	context._cmp(context.al, context.data.byte(kCh0playing));
	if (context.flags.z()) goto playingalready;
	context._cmp(context.data.byte(kLocation), 45);
	if (!context.flags.z()) goto notweb;
	context._cmp(context.data.word(kReeltowatch), 45);
	if (context.flags.z()) goto wrongatmos;
notweb:
	playchannel0(context);
	context._cmp(context.data.byte(kReallocation), 2);
	context._cmp(context.data.byte(kMapy), 0);
	if (context.flags.z()) goto fullvol;
	if (!context.flags.z()) goto notlouisvol;
	context._cmp(context.data.byte(kMapy), 10);
	if (!context.flags.z()) goto notlouisvol;
	context._cmp(context.data.byte(kMapx), 22);
	if (!context.flags.z()) goto notlouisvol;
	context.data.byte(kVolume) = 5;
notlouisvol:
playingalready:
	context._cmp(context.data.byte(kReallocation), 2);
	if (!context.flags.z()) goto notlouisvol2;
	context._cmp(context.data.byte(kMapx), 22);
	if (context.flags.z()) goto louisvol;
	context._cmp(context.data.byte(kMapx), 11);
	if (!context.flags.z()) goto notlouisvol2;
fullvol:
	context.data.byte(kVolume) = 0;
notlouisvol2:
	{assert(stack_depth == context.stack.size()); return; }
louisvol:
	context.data.byte(kVolume) = 5;
	{assert(stack_depth == context.stack.size()); return; }
wrongatmos:
	context._add(context.bx, 5);
	goto nextatmos;
nomoreatmos:
	cancelch0(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void walkintoroom(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLocation), 14);
	if (!context.flags.z()) goto notlair;
	context._cmp(context.data.byte(kMapx), 22);
	if (!context.flags.z()) goto notlair;
	context.data.byte(kDestination) = 1;
	context.data.byte(kFinaldest) = 1;
	autosetwalk(context);
notlair:
	{assert(stack_depth == context.stack.size()); return; }
}

void afterintroroom(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kNowinnewroom), 0);
	if (context.flags.z()) goto notnewintro;
	clearwork(context);
	findroominloc(context);
	context.data.byte(kNewobs) = 1;
	drawfloor(context);
	reelsonscreen(context);
	spriteupdate(context);
	printsprites(context);
	worktoscreen(context);
	context.data.byte(kNowinnewroom) = 0;
notnewintro:
	{assert(stack_depth == context.stack.size()); return; }
}

void obname(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kReasseschanges), 0);
	if (context.flags.z()) goto notnewpath;
	context.data.byte(kReasseschanges) = 0;
	goto diff;
notnewpath:
	context._cmp(context.ah, context.data.byte(kCommandtype));
	if (context.flags.z()) goto notdiffob;
	goto diff;
notdiffob:
	context._cmp(context.al, context.data.byte(kCommand));
	if (!context.flags.z()) goto diff;
	context._cmp(context.data.byte(kWalkandexam), 1);
	if (context.flags.z()) goto walkandexamine;
	context._cmp(context.data.word(kMousebutton), 0);
	if (context.flags.z()) goto noobselect;
	context._cmp(context.data.byte(kCommandtype), 3);
	if (!context.flags.z()) goto isntblock;
	context._cmp(context.data.byte(kLastflag), 2);
	if (context.flags.c()) goto noobselect;
isntblock:
	context.bl = context.data.byte(kManspath);
	context._cmp(context.bl, context.data.byte(kPointerspath));
	if (!context.flags.z()) goto wantstowalk;
	context._cmp(context.data.byte(kCommandtype), 3);
	if (context.flags.z()) goto wantstowalk;
	finishedwalking(context);
	if (!context.flags.z()) goto noobselect;
	context._cmp(context.data.byte(kCommandtype), 5);
	if (context.flags.z()) goto wantstotalk;
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) goto noobselect;
	examineob(context);
	{assert(stack_depth == context.stack.size()); return; }
wantstotalk:
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) goto noobselect;
	talk(context);
	{assert(stack_depth == context.stack.size()); return; }
walkandexamine:
	finishedwalking(context);
	if (!context.flags.z()) goto noobselect;
	context.al = context.data.byte(kWalkexamtype);
	context.data.byte(kCommandtype) = context.al;
	context.al = context.data.byte(kWalkexamnum);
	context.data.byte(kCommand) = context.al;
	context.data.byte(kWalkandexam) = 0;
	context._cmp(context.data.byte(kCommandtype), 5);
	if (context.flags.z()) goto noobselect;
	examineob(context);
	{assert(stack_depth == context.stack.size()); return; }
wantstowalk:
	setwalk(context);
	context.data.byte(kReasseschanges) = 1;
noobselect:
	{assert(stack_depth == context.stack.size()); return; }
diff:
	context.data.byte(kCommand) = context.al;
	context.data.byte(kCommandtype) = context.ah;
diff2:
	context._cmp(context.data.byte(kLinepointer), 254);
	if (!context.flags.z()) goto middleofwalk;
	context._cmp(context.data.word(kWatchingtime), 0);
	if (!context.flags.z()) goto middleofwalk;
	context.al = context.data.byte(kFacing);
	context._cmp(context.al, context.data.byte(kTurntoface));
	if (!context.flags.z()) goto middleofwalk;
	context._cmp(context.data.byte(kCommandtype), 3);
	if (!context.flags.z()) goto notblock;
	context.bl = context.data.byte(kManspath);
	context._cmp(context.bl, context.data.byte(kPointerspath));
	if (!context.flags.z()) goto dontcheck;
	context.cl = context.data.byte(kRyanx);
	context._add(context.cl, 12);
	context.ch = context.data.byte(kRyany);
	context._add(context.ch, 12);
	checkone(context);
	context._cmp(context.cl, 2);
	if (context.flags.c()) goto isblock;
dontcheck:
	getflagunderp(context);
	context._cmp(context.data.byte(kLastflag), 2);
	if (context.flags.c()) goto isblock;
	context._cmp(context.data.byte(kLastflag), 128);
	if (!context.flags.c()) goto isblock;
	goto toofaraway;
notblock:
	context.bl = context.data.byte(kManspath);
	context._cmp(context.bl, context.data.byte(kPointerspath));
	if (!context.flags.z()) goto toofaraway;
	context._cmp(context.data.byte(kCommandtype), 3);
	if (context.flags.z()) goto isblock;
	context._cmp(context.data.byte(kCommandtype), 5);
	if (context.flags.z()) goto isaperson;
	examineobtext(context);
	{assert(stack_depth == context.stack.size()); return; }
middleofwalk:
	blocknametext(context);
	{assert(stack_depth == context.stack.size()); return; }
isblock:
	blocknametext(context);
	{assert(stack_depth == context.stack.size()); return; }
isaperson:
	personnametext(context);
	{assert(stack_depth == context.stack.size()); return; }
toofaraway:
	walktotext(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void finishedwalking(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLinepointer), 254);
	if (!context.flags.z()) goto iswalking;
	context.al = context.data.byte(kFacing);
	context._cmp(context.al, context.data.byte(kTurntoface));
iswalking:
	{assert(stack_depth == context.stack.size()); return; }
}

void examineobtext(Context & context) {
	uint stack_depth = context.stack.size();
	context.bl = context.data.byte(kCommand);
	context.bh = context.data.byte(kCommandtype);
	context.al = 1;
	commandwithob(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void commandwithob(Context & context) {
	uint stack_depth = context.stack.size();
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
	context.es = context.data.word(kCommandtext);
	context.ax = context.es.word(context.bx);
	context._add(context.ax, (66*2));
	context.si = context.ax;
	context.di = context.data.word(kTextaddressx);
	context.bx = context.data.word(kTextaddressy);
	context.dl = context.data.byte(kTextlen);
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.ax = context.pop();
	context.di = 5674;
	copyname(context);
	context.ax = context.pop();
	context.di = context.data.word(kLastxpos);
	context._cmp(context.al, 0);
	if (context.flags.z()) goto noadd;
	context._add(context.di, 5);
noadd:
	context.bx = context.data.word(kTextaddressy);
	context.es = context.cs;
	context.si = 5674;
	context.dl = context.data.byte(kTextlen);
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.data.byte(kNewtextline) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void commandonly(Context & context) {
	uint stack_depth = context.stack.size();
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
	context.es = context.data.word(kCommandtext);
	context.ax = context.es.word(context.bx);
	context._add(context.ax, (66*2));
	context.si = context.ax;
	context.di = context.data.word(kTextaddressx);
	context.bx = context.data.word(kTextaddressy);
	context.dl = context.data.byte(kTextlen);
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.data.byte(kNewtextline) = 1;
	{assert(stack_depth == context.stack.size()); return; }
}

void printmessage(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.dx);
	context.push(context.bx);
	context.push(context.di);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.es = context.data.word(kCommandtext);
	context.ax = context.es.word(context.bx);
	context._add(context.ax, (66*2));
	context.si = context.ax;
	context.di = context.pop();
	context.bx = context.pop();
	context.dx = context.pop();
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void printmessage2(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.dx);
	context.push(context.bx);
	context.push(context.di);
	context.push(context.ax);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.bx = context.ax;
	context.es = context.data.word(kCommandtext);
	context.ax = context.es.word(context.bx);
	context._add(context.ax, (66*2));
	context.si = context.ax;
	context.ax = context.pop();
searchmess:
	context.push(context.ax);
	findnextcolon(context);
	context.ax = context.pop();
	context._dec(context.ah);
	if (!context.flags.z()) goto searchmess;
	context.di = context.pop();
	context.bx = context.pop();
	context.dx = context.pop();
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void blocknametext(Context & context) {
	uint stack_depth = context.stack.size();
	context.bl = context.data.byte(kCommand);
	context.bh = context.data.byte(kCommandtype);
	context.al = 0;
	commandwithob(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void personnametext(Context & context) {
	uint stack_depth = context.stack.size();
	context.bl = context.data.byte(kCommand);
	context._and(context.bl, 127);
	context.bh = context.data.byte(kCommandtype);
	context.al = 2;
	commandwithob(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void walktotext(Context & context) {
	uint stack_depth = context.stack.size();
	context.bl = context.data.byte(kCommand);
	context.bh = context.data.byte(kCommandtype);
	context.al = 3;
	commandwithob(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getflagunderp(Context & context) {
	uint stack_depth = context.stack.size();
	context.cx = context.data.word(kMousex);
	context._sub(context.cx, context.data.word(kMapadx));
	context.ax = context.data.word(kMousey);
	context._sub(context.ax, context.data.word(kMapady));
	context.ch = context.al;
	checkone(context);
	context.data.byte(kLastflag) = context.cl;
	context.data.byte(kLastflagex) = context.ch;
	{assert(stack_depth == context.stack.size()); return; }
}

void setwalk(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kLinepointer), 254);
	if (!context.flags.z()) goto alreadywalking;
	context.al = context.data.byte(kPointerspath);
	context._cmp(context.al, context.data.byte(kManspath));
	if (context.flags.z()) goto cantwalk2;
	context._cmp(context.data.byte(kWatchmode), 1);
	if (context.flags.z()) goto holdingreel;
	context._cmp(context.data.byte(kWatchmode), 2);
	if (context.flags.z()) goto cantwalk;
	context.data.byte(kDestination) = context.al;
	context.data.byte(kFinaldest) = context.al;
	context._cmp(context.data.word(kMousebutton), 2);
	if (!context.flags.z()) goto notwalkandexam;
	context._cmp(context.data.byte(kCommandtype), 3);
	if (context.flags.z()) goto notwalkandexam;
	context.data.byte(kWalkandexam) = 1;
	context.al = context.data.byte(kCommandtype);
	context.data.byte(kWalkexamtype) = context.al;
	context.al = context.data.byte(kCommand);
	context.data.byte(kWalkexamnum) = context.al;
notwalkandexam:
	autosetwalk(context);
cantwalk:
	{assert(stack_depth == context.stack.size()); return; }
cantwalk2:
	facerightway(context);
	{assert(stack_depth == context.stack.size()); return; }
alreadywalking:
	context.al = context.data.byte(kPointerspath);
	context.data.byte(kFinaldest) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
holdingreel:
	context.data.byte(kDestafterhold) = context.al;
	context.data.byte(kWatchmode) = 2;
	{assert(stack_depth == context.stack.size()); return; }
}

void autosetwalk(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kManspath);
	context._cmp(context.data.byte(kFinaldest), context.al);
	if (!context.flags.z()) goto notsamealready;
	{assert(stack_depth == context.stack.size()); return; }
notsamealready:
	getroomspaths(context);
	checkdest(context);
	context.push(context.bx);
	context.al = context.data.byte(kManspath);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.es.byte(context.bx);
	context.ah = 0;
	context._sub(context.ax, 12);
	context.data.word(kLinestartx) = context.ax;
	context.al = context.es.byte(context.bx+1);
	context.ah = 0;
	context._sub(context.ax, 12);
	context.data.word(kLinestarty) = context.ax;
	context.bx = context.pop();
	context.al = context.data.byte(kDestination);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = context.es.byte(context.bx);
	context.ah = 0;
	context._sub(context.ax, 12);
	context.data.word(kLineendx) = context.ax;
	context.al = context.es.byte(context.bx+1);
	context.ah = 0;
	context._sub(context.ax, 12);
	context.data.word(kLineendy) = context.ax;
	bresenhams(context);
	context._cmp(context.data.byte(kLinedirection), 0);
	if (context.flags.z()) goto normalline;
	context.al = context.data.byte(kLinelength);
	context._dec(context.al);
	context.data.byte(kLinepointer) = context.al;
	context.data.byte(kLinedirection) = 1;
	{assert(stack_depth == context.stack.size()); return; }
normalline:
	context.data.byte(kLinepointer) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void checkdest(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.bx);
	context._add(context.bx, 12*8);
	context.ah = context.data.byte(kManspath);
	context.cl = 4;
	context._shl(context.ah, context.cl);
	context.al = context.data.byte(kDestination);
	context.cl = 24;
	context.ch = context.data.byte(kDestination);
checkdestloop:
	context.dh = context.es.byte(context.bx);
	context._and(context.dh, 0xf0);
	context.dl = context.es.byte(context.bx);
	context._and(context.dl, 0xf);
	context._cmp(context.ax, context.dx);
	if (!context.flags.z()) goto nextcheck;
	context.al = context.es.byte(context.bx+1);
	context._and(context.al, 15);
	context.data.byte(kDestination) = context.al;
	context.bx = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
nextcheck:
	context.dl = context.es.byte(context.bx);
	context._and(context.dl, 0xf0);
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	context._shr(context.dl, 1);
	context.dh = context.es.byte(context.bx);
	context._and(context.dh, 0xf);
	context._shl(context.dh, 1);
	context._shl(context.dh, 1);
	context._shl(context.dh, 1);
	context._shl(context.dh, 1);
	context._cmp(context.ax, context.dx);
	if (!context.flags.z()) goto nextcheck2;
	context.ch = context.es.byte(context.bx+1);
	context._and(context.ch, 15);
nextcheck2:
	context._add(context.bx, 2);
	context._dec(context.cl);
	if (!context.flags.z()) goto checkdestloop;
	context.data.byte(kDestination) = context.ch;
	context.bx = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void bresenhams(Context & context) {
	uint stack_depth = context.stack.size();
	workoutframes(context);
	context.dx = context.data;
	context.es = context.dx;
	context.di = 7944;
	context.si = 1;
	context.data.byte(kLinedirection) = 0;
	context.cx = context.data.word(kLineendx);
	context._sub(context.cx, context.data.word(kLinestartx));
	if (context.flags.z()) goto vertline;
	if (!context.flags.s()) goto line1;
	context._neg(context.cx);
	context.bx = context.data.word(kLineendx);
	context._xchg(context.bx, context.data.word(kLinestartx));
	context.data.word(kLineendx) = context.bx;
	context.bx = context.data.word(kLineendy);
	context._xchg(context.bx, context.data.word(kLinestarty));
	context.data.word(kLineendy) = context.bx;
	context.data.byte(kLinedirection) = 1;
line1:
	context.bx = context.data.word(kLineendy);
	context._sub(context.bx, context.data.word(kLinestarty));
	if (context.flags.z()) goto horizline;
	if (!context.flags.s()) goto line3;
	context._neg(context.bx);
	context._neg(context.si);
line3:
	context.push(context.si);
	context.data.byte(kLineroutine) = 0;
	context._cmp(context.bx, context.cx);
	if (context.flags.le()) goto line4;
	context.data.byte(kLineroutine) = 1;
	context._xchg(context.bx, context.cx);
line4:
	context._shl(context.bx, 1);
	context.data.word(kIncrement1) = context.bx;
	context._sub(context.bx, context.cx);
	context.si = context.bx;
	context._sub(context.bx, context.cx);
	context.data.word(kIncrement2) = context.bx;
	context.ax = context.data.word(kLinestartx);
	context.bx = context.data.word(kLinestarty);
	context.ah = context.bl;
	context._inc(context.cx);
	context.bx = context.pop();
	context._cmp(context.data.byte(kLineroutine), 1);
	if (context.flags.z()) goto hislope;
	goto loslope;
vertline:
	context.ax = context.data.word(kLinestarty);
	context.bx = context.data.word(kLineendy);
	context.cx = context.bx;
	context._sub(context.cx, context.ax);
	if (!context.flags.l()) goto line31;
	context._neg(context.cx);
	context.ax = context.bx;
	context.data.byte(kLinedirection) = 1;
line31:
	context._inc(context.cx);
	context.bx = context.data.word(kLinestartx);
	context._xchg(context.ax, context.bx);
	context.ah = context.bl;
	context.bx = context.si;
line32:
	context._stosw();
	context._add(context.ah, context.bl);
	if (--context.cx) goto line32;
	goto lineexit;
horizline:
	context.ax = context.data.word(kLinestartx);
	context.bx = context.data.word(kLinestarty);
	context.ah = context.bl;
	context._inc(context.cx);
horizloop:
	context._stosw();
	context._inc(context.al);
	if (--context.cx) goto horizloop;
	goto lineexit;
loslope:
loloop:
	context._stosw();
	context._inc(context.al);
	context._or(context.si, context.si);
	if (!context.flags.s()) goto line12;
	context._add(context.si, context.data.word(kIncrement1));
	if (--context.cx) goto loloop;
	goto lineexit;
line12:
	context._add(context.si, context.data.word(kIncrement2));
	context._add(context.ah, context.bl);
	if (--context.cx) goto loloop;
	goto lineexit;
hislope:
hiloop:
	context._stosw();
	context._add(context.ah, context.bl);
	context._or(context.si, context.si);
	if (!context.flags.s()) goto line23;
	context._add(context.si, context.data.word(kIncrement1));
	if (--context.cx) goto hiloop;
	goto lineexit;
line23:
	context._add(context.si, context.data.word(kIncrement2));
	context._inc(context.al);
	if (--context.cx) goto hiloop;
lineexit:
	context._sub(context.di, 7944);
	context.ax = context.di;
	context._shr(context.ax, 1);
	context.data.byte(kLinelength) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void workoutframes(Context & context) {
	uint stack_depth = context.stack.size();
	context.bx = context.data.word(kLinestartx);
	context._add(context.bx, 32);
	context.ax = context.data.word(kLineendx);
	context._add(context.ax, 32);
	context._sub(context.bx, context.ax);
	if (!context.flags.c()) goto notneg1;
	context._neg(context.bx);
notneg1:
	context.cx = context.data.word(kLinestarty);
	context._add(context.cx, 32);
	context.ax = context.data.word(kLineendy);
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
	context.bx = context.data.word(kLinestartx);
	context._add(context.bx, 32);
	context.ax = context.data.word(kLineendx);
	context._add(context.ax, 32);
	context._sub(context.bx, context.ax);
	if (context.flags.c()) goto isinright;
isinleft:
	context.cx = context.data.word(kLinestarty);
	context._add(context.cx, 32);
	context.ax = context.data.word(kLineendy);
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
	context.cx = context.data.word(kLinestarty);
	context._add(context.cx, 32);
	context.ax = context.data.word(kLineendy);
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
	context.data.byte(kTurntoface) = context.dl;
	context.data.byte(kTurndirection) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void getroomspaths(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kRoomnum);
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context.es = context.data.word(kReels);
	context.bx = (0);
	context._add(context.bx, context.ax);
	{assert(stack_depth == context.stack.size()); return; }
}

void copyname(Context & context) {
	uint stack_depth = context.stack.size();
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
	context._inc(context.cx);
	context.al = 0;
	context._stosb();
	{assert(stack_depth == context.stack.size()); return; }
	context.al = 255;
	while(context.cx--) 	context._stosb();
	{assert(stack_depth == context.stack.size()); return; }
}

void findobname(Context & context) {
	uint stack_depth = context.stack.size();
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
	context.ds = context.data.word(kPeople);
	context._add(context.si, (0+24));
	context.cx = (0+24+(1026*2));
	context.ax = context.ds.word(context.si);
	context._add(context.ax, context.cx);
	context.si = context.ax;
	context.ax = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
notpersonname:
	context._cmp(context.ah, 4);
	if (!context.flags.z()) goto notextraname;
	context.ds = context.data.word(kExtras);
	context._add(context.bx, (0+2080+30000+(16*114)));
	context.ax = context.ds.word(context.bx);
	context._add(context.ax, (0+2080+30000+(16*114)+((114+2)*2)));
	context.si = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
notextraname:
	context._cmp(context.ah, 2);
	if (!context.flags.z()) goto notfreename;
	context.ds = context.data.word(kFreedesc);
	context._add(context.bx, (0));
	context.ax = context.ds.word(context.bx);
	context._add(context.ax, (0+(82*2)));
	context.si = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
notfreename:
	context._cmp(context.ah, 1);
	if (!context.flags.z()) goto notsetname;
	context.ds = context.data.word(kSetdesc);
	context._add(context.bx, (0));
	context.ax = context.ds.word(context.bx);
	context._add(context.ax, (0+(130*2)));
	context.si = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
notsetname:
	context.ds = context.data.word(kBlockdesc);
	context._add(context.bx, (0));
	context.ax = context.ds.word(context.bx);
	context._add(context.ax, (0+(98*2)));
	context.si = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void showicon(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kReallocation), 50);
	if (!context.flags.c()) goto isdream1;
	showpanel(context);
	showman(context);
	roomname(context);
	panelicons1(context);
	zoomicon(context);
	{assert(stack_depth == context.stack.size()); return; }
isdream1:
	context.ds = context.data.word(kTempsprites);
	context.di = 72;
	context.bx = 2;
	context.al = 45;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempsprites);
	context.di = 72+47;
	context.bx = 2;
	context.al = 46;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempsprites);
	context.di = 69-10;
	context.bx = 21;
	context.al = 49;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempsprites);
	context.di = 160+88;
	context.bx = 2;
	context.al = 45;
	context.ah = 4;
	showframe(context);
	context.ds = context.data.word(kTempsprites);
	context.di = 160+43;
	context.bx = 2;
	context.al = 46;
	context.ah = 4;
	showframe(context);
	context.ds = context.data.word(kTempsprites);
	context.di = 160+101;
	context.bx = 21;
	context.al = 49;
	context.ah = 4;
	showframe(context);
	middlepanel(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void middlepanel(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kTempsprites);
	context.di = 72+47+20;
	context.bx = 0;
	context.al = 48;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempsprites);
	context.di = 72+19;
	context.bx = 21;
	context.al = 47;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kTempsprites);
	context.di = 160+23;
	context.bx = 0;
	context.al = 48;
	context.ah = 4;
	showframe(context);
	context.ds = context.data.word(kTempsprites);
	context.di = 160+71;
	context.bx = 21;
	context.al = 47;
	context.ah = 4;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showman(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kIcons1);
	context.di = 0;
	context.bx = 0;
	context.al = 0;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kIcons1);
	context.di = 0;
	context.bx = 114;
	context.al = 1;
	context.ah = 0;
	showframe(context);
	context._cmp(context.data.byte(kShadeson), 0);
	if (context.flags.z()) goto notverycool;
	context.ds = context.data.word(kIcons1);
	context.di = 28;
	context.bx = 25;
	context.al = 2;
	context.ah = 0;
	showframe(context);
notverycool:
	{assert(stack_depth == context.stack.size()); return; }
}

void showpanel(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kIcons1);
	context.di = 72;
	context.bx = 0;
	context.al = 19;
	context.ah = 0;
	showframe(context);
	context.ds = context.data.word(kIcons1);
	context.di = 192;
	context.bx = 0;
	context.al = 19;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void roomname(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 88;
	context.bx = 18;
	context.al = 53;
	context.dl = 240;
	printmessage(context);
	context.bl = context.data.byte(kRoomnum);
	context._cmp(context.bl, 32);
	if (context.flags.c()) goto notover32;
	context._sub(context.bl, 32);
notover32:
	context.bh = 0;
	context._add(context.bx, context.bx);
	context.es = context.data.word(kRoomdesc);
	context._add(context.bx, (0));
	context.ax = context.es.word(context.bx);
	context._add(context.ax, (0+(38*2)));
	context.si = context.ax;
	context.data.word(kLinespacing) = 7;
	context.di = 88;
	context.bx = 25;
	context.dl = 120;
	context._cmp(context.data.byte(kWatchon), 1);
	if (context.flags.z()) goto gotpl;
	context.dl = 160;
gotpl:
	context.al = 0;
	context.ah = 0;
	printdirect(context);
	context.data.word(kLinespacing) = 10;
	usecharset1(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void usecharset1(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kCharset1);
	context.data.word(kCurrentset) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void usetempcharset(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kTempcharset);
	context.data.word(kCurrentset) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void showexit(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.data.word(kIcons1);
	context.di = 274;
	context.bx = 154;
	context.al = 11;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void panelicons1(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = 0;
	context._cmp(context.data.byte(kWatchon), 1);
	if (context.flags.z()) goto watchison;
	context.di = 48;
watchison:
	context.push(context.di);
	context.ds = context.data.word(kIcons2);
	context._add(context.di, 204);
	context.bx = 4;
	context.al = 2;
	context.ah = 0;
	showframe(context);
	context.di = context.pop();
	context.push(context.di);
	context._cmp(context.data.byte(kZoomon), 1);
	if (context.flags.z()) goto zoomisoff;
	context.ds = context.data.word(kIcons1);
	context._add(context.di, 228);
	context.bx = 8;
	context.al = 5;
	context.ah = 0;
	showframe(context);
zoomisoff:
	context.di = context.pop();
	showwatch(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showwatch(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kWatchon), 0);
	if (context.flags.z()) goto nowristwatch;
	context.ds = context.data.word(kIcons1);
	context.di = 250;
	context.bx = 1;
	context.al = 6;
	context.ah = 0;
	showframe(context);
	showtime(context);
nowristwatch:
	{assert(stack_depth == context.stack.size()); return; }
}

void zoomicon(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kZoomon), 0);
	if (context.flags.z()) goto nozoom1;
	context.ds = context.data.word(kIcons1);
	context.di = (8);
	context.bx = (132)-1;
	context.al = 8;
	context.ah = 0;
	showframe(context);
nozoom1:
	{assert(stack_depth == context.stack.size()); return; }
}

void showblink(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kManisoffscreen), 1);
	if (context.flags.z()) goto finblink1;
	context._inc(context.data.byte(kBlinkcount));
	context._cmp(context.data.byte(kShadeson), 0);
	if (!context.flags.z()) goto finblink1;
	context._cmp(context.data.byte(kReallocation), 50);
	if (!context.flags.c()) goto eyesshut;
	context.al = context.data.byte(kBlinkcount);
	context._cmp(context.al, 3);
	if (!context.flags.z()) goto finblink1;
	context.data.byte(kBlinkcount) = 0;
	context.al = context.data.byte(kBlinkframe);
	context._inc(context.al);
	context.data.byte(kBlinkframe) = context.al;
	context._cmp(context.al, 6);
	if (context.flags.c()) goto nomorethan6;
	context.al = 6;
nomorethan6:
	context.ah = 0;
	context.bx = 5691;
	context._add(context.bx, context.ax);
	context.al = context.cs.byte(context.bx);
	context.ds = context.data.word(kIcons1);
	context.di = 44;
	context.bx = 32;
	context.ah = 0;
	showframe(context);
finblink1:
	{assert(stack_depth == context.stack.size()); return; }
eyesshut:
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpblink(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kShadeson), 0);
	if (!context.flags.z()) goto nodumpeye;
	context._cmp(context.data.byte(kBlinkcount), 0);
	if (!context.flags.z()) goto nodumpeye;
	context.al = context.data.byte(kBlinkframe);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void worktoscreenm(Context & context) {
	uint stack_depth = context.stack.size();
	animpointer(context);
	readmouse(context);
	showpointer(context);
	vsync(context);
	worktoscreen(context);
	delpointer(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void blank(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 199);
	if (context.flags.z()) goto alreadyblnk;
	context.data.byte(kCommandtype) = 199;
	context.al = 0;
	commandonly(context);
alreadyblnk:
	{assert(stack_depth == context.stack.size()); return; }
}

void allpointer(Context & context) {
	uint stack_depth = context.stack.size();
	readmouse(context);
	showpointer(context);
	dumppointer(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void hangonp(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.cx);
	context._add(context.cx, context.cx);
	context.ax = context.pop();
	context._add(context.cx, context.ax);
	context.data.word(kMaintimer) = 0;
	context.al = context.data.byte(kPointerframe);
	context.ah = context.data.byte(kPickup);
	context.push(context.ax);
	context.data.byte(kPointermode) = 3;
	context.data.byte(kPickup) = 0;
	context.push(context.cx);
	context.data.byte(kCommandtype) = 255;
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
	context.ax = context.data.word(kMousebutton);
	context._cmp(context.ax, 0);
	if (context.flags.z()) goto notpressed;
	context._cmp(context.ax, context.data.word(kOldbutton));
	if (!context.flags.z()) goto getoutofit;
notpressed:
	if (--context.cx) goto hangloop;
getoutofit:
	delpointer(context);
	context.ax = context.pop();
	context.data.byte(kPointerframe) = context.al;
	context.data.byte(kPickup) = context.ah;
	context.data.byte(kPointermode) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void hangonw(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void hangoncurs(Context & context) {
	uint stack_depth = context.stack.size();
monloop1:
	context.push(context.cx);
	printcurs(context);
	vsync(context);
	delcurs(context);
	context.cx = context.pop();
	if (--context.cx) goto monloop1;
	{assert(stack_depth == context.stack.size()); return; }
}

void getunderzoom(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = (8)+5;
	context.bx = (132)+4;
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
	context.cl = 46;
	context.ch = 40;
	multiget(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void dumpzoom(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kZoomon), 1);
	if (!context.flags.z()) goto notzoomon;
	context.di = (8)+5;
	context.bx = (132)+4;
	context.cl = 46;
	context.ch = 40;
	multidump(context);
notzoomon:
	{assert(stack_depth == context.stack.size()); return; }
}

void putunderzoom(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = (8)+5;
	context.bx = (132)+4;
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60+(32*32)+(11*10*3)+768+768+768+(32*32)+(128*5)+(80*5)+(100*5)+(12*5));
	context.cl = 46;
	context.ch = 40;
	multiput(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void crosshair(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kCommandtype), 3);
	if (context.flags.z()) goto nocross;
	context._cmp(context.data.byte(kCommandtype), 10);
	if (!context.flags.c()) goto nocross;
	context.es = context.data.word(kWorkspace);
	context.ds = context.data.word(kIcons1);
	context.di = (8)+24;
	context.bx = (132)+19;
	context.al = 9;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
nocross:
	context.es = context.data.word(kWorkspace);
	context.ds = context.data.word(kIcons1);
	context.di = (8)+24;
	context.bx = (132)+19;
	context.al = 29;
	context.ah = 0;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void showpointer(Context & context) {
	uint stack_depth = context.stack.size();
	showblink(context);
	context.di = context.data.word(kMousex);
	context.data.word(kOldpointerx) = context.di;
	context.bx = context.data.word(kMousey);
	context.data.word(kOldpointery) = context.bx;
	context._cmp(context.data.byte(kPickup), 1);
	if (context.flags.z()) goto itsanobject;
	context.push(context.bx);
	context.push(context.di);
	context.ds = context.data.word(kIcons1);
	context.al = context.data.byte(kPointerframe);
	context._add(context.al, 20);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context.si = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.si, context.ax);
	context.cx = context.ds.word(context.si);
	context._cmp(context.cl, 12);
	if (!context.flags.c()) goto notsmallx;
	context.cl = 12;
notsmallx:
	context._cmp(context.ch, 12);
	if (!context.flags.c()) goto notsmally;
	context.ch = 12;
notsmally:
	context.data.byte(kPointerxs) = context.cl;
	context.data.byte(kPointerys) = context.ch;
	context.push(context.ds);
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60);
	multiget(context);
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.push(context.di);
	context.push(context.bx);
	context.al = context.data.byte(kPointerframe);
	context._add(context.al, 20);
	context.ah = 0;
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
itsanobject:
	context.al = context.data.byte(kItemframe);
	context.ds = context.data.word(kExtras);
	context._cmp(context.data.byte(kObjecttype), 4);
	if (context.flags.z()) goto itsfrominv;
	context.ds = context.data.word(kFreeframes);
itsfrominv:
	context.cl = context.al;
	context._add(context.al, context.al);
	context._add(context.al, context.cl);
	context._inc(context.al);
	context.ah = 0;
	context.push(context.ax);
	context._add(context.ax, context.ax);
	context.si = context.ax;
	context._add(context.ax, context.ax);
	context._add(context.si, context.ax);
	context.ax = 2080;
	context.cx = context.ds.word(context.si);
	context._cmp(context.cl, 12);
	if (!context.flags.c()) goto notsmallx2;
	context.cl = 12;
notsmallx2:
	context._cmp(context.ch, 12);
	if (!context.flags.c()) goto notsmally2;
	context.ch = 12;
notsmally2:
	context.data.byte(kPointerxs) = context.cl;
	context.data.byte(kPointerys) = context.ch;
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
	context._sub(context.data.word(kOldpointerx), context.ax);
	context._sub(context.di, context.ax);
	context.al = context.ch;
	context._shr(context.ax, 1);
	context._sub(context.data.word(kOldpointery), context.ax);
	context._sub(context.bx, context.ax);
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60);
	multiget(context);
	context.ds = context.pop();
	context.di = context.pop();
	context.bx = context.pop();
	context.ax = context.pop();
	context.ah = 128;
	showframe(context);
	context.bx = context.pop();
	context.di = context.pop();
	context.ds = context.data.word(kIcons1);
	context.al = 3;
	context.ah = 128;
	showframe(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void delpointer(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kOldpointerx);
	context._cmp(context.ax, 0x0ffff);
	if (context.flags.z()) goto nevershown;
	context.data.word(kDelherex) = context.ax;
	context.ax = context.data.word(kOldpointery);
	context.data.word(kDelherey) = context.ax;
	context.cl = context.data.byte(kPointerxs);
	context.data.byte(kDelxs) = context.cl;
	context.ch = context.data.byte(kPointerys);
	context.data.byte(kDelys) = context.ch;
	context.ds = context.data.word(kBuffers);
	context.si = (0+(180*10)+32+60);
	context.di = context.data.word(kDelherex);
	context.bx = context.data.word(kDelherey);
	multiput(context);
nevershown:
	{assert(stack_depth == context.stack.size()); return; }
}

void dumppointer(Context & context) {
	uint stack_depth = context.stack.size();
	dumpblink(context);
	context.cl = context.data.byte(kDelxs);
	context.ch = context.data.byte(kDelys);
	context.di = context.data.word(kDelherex);
	context.bx = context.data.word(kDelherey);
	multidump(context);
	context.bx = context.data.word(kOldpointery);
	context.di = context.data.word(kOldpointerx);
	context._cmp(context.di, context.data.word(kDelherex));
	if (!context.flags.z()) goto difffound;
	context._cmp(context.bx, context.data.word(kDelherey));
	if (context.flags.z()) goto notboth;
difffound:
	context.cl = context.data.byte(kPointerxs);
	context.ch = context.data.byte(kPointerys);
	multidump(context);
notboth:
	{assert(stack_depth == context.stack.size()); return; }
}

void undertextline(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = context.data.word(kTextaddressx);
	context.bx = context.data.word(kTextaddressy);
	context.ds = context.data.word(kBuffers);
	context.si = (0);
	context.cl = (180);
	context.ch = (10);
	multiget(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void deltextline(Context & context) {
	uint stack_depth = context.stack.size();
	context.di = context.data.word(kTextaddressx);
	context.bx = context.data.word(kTextaddressy);
	context.ds = context.data.word(kBuffers);
	context.si = (0);
	context.cl = (180);
	context.ch = (10);
	multiput(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void dumptextline(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kNewtextline), 1);
	if (!context.flags.z()) goto nodumptextline;
	context.data.byte(kNewtextline) = 0;
	context.di = context.data.word(kTextaddressx);
	context.bx = context.data.word(kTextaddressy);
	context.cl = (180);
	context.ch = (10);
	multidump(context);
nodumptextline:
	{assert(stack_depth == context.stack.size()); return; }
}

void animpointer(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kPointermode), 2);
	if (context.flags.z()) goto combathand;
	context._cmp(context.data.byte(kPointermode), 3);
	if (context.flags.z()) goto mousehand;
	context._cmp(context.data.word(kWatchingtime), 0);
	if (context.flags.z()) goto notwatchpoint;
	context.data.byte(kPointerframe) = 11;
	{assert(stack_depth == context.stack.size()); return; }
notwatchpoint:
	context.data.byte(kPointerframe) = 0;
	context._cmp(context.data.byte(kInmaparea), 0);
	if (context.flags.z()) goto gothand;
	context._cmp(context.data.byte(kPointerfirstpath), 0);
	if (context.flags.z()) goto gothand;
arrow:
	getflagunderp(context);
	context._cmp(context.cl, 2);
	if (context.flags.c()) goto gothand;
	context._cmp(context.cl, 128);
	if (!context.flags.c()) goto gothand;
	context.data.byte(kPointerframe) = 3;
	context._test(context.cl, 4);
	if (!context.flags.z()) goto gothand;
	context.data.byte(kPointerframe) = 4;
	context._test(context.cl, 16);
	if (!context.flags.z()) goto gothand;
	context.data.byte(kPointerframe) = 5;
	context._test(context.cl, 2);
	if (!context.flags.z()) goto gothand;
	context.data.byte(kPointerframe) = 6;
	context._test(context.cl, 8);
	if (!context.flags.z()) goto gothand;
	context.data.byte(kPointerframe) = 8;
gothand:
	{assert(stack_depth == context.stack.size()); return; }
mousehand:
	context._cmp(context.data.byte(kPointerspeed), 0);
	if (context.flags.z()) goto rightspeed3;
	context._dec(context.data.byte(kPointerspeed));
	goto finflashmouse;
rightspeed3:
	context.data.byte(kPointerspeed) = 5;
	context._inc(context.data.byte(kPointercount));
	context._cmp(context.data.byte(kPointercount), 16);
	if (!context.flags.z()) goto finflashmouse;
	context.data.byte(kPointercount) = 0;
finflashmouse:
	context.al = context.data.byte(kPointercount);
	context.ah = 0;
	context.bx = 5698;
	context._add(context.bx, context.ax);
	context.al = context.cs.byte(context.bx);
	context.data.byte(kPointerframe) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
combathand:
	context.data.byte(kPointerframe) = 0;
	context._cmp(context.data.byte(kReallocation), 14);
	if (!context.flags.z()) goto notarrow;
	context._cmp(context.data.byte(kCommandtype), 211);
	if (!context.flags.z()) goto notarrow;
	context.data.byte(kPointerframe) = 5;
notarrow:
	{assert(stack_depth == context.stack.size()); return; }
}

void readmouse(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kMousebutton);
	context.data.word(kOldbutton) = context.ax;
	context.ax = context.data.word(kMousex);
	context.data.word(kOldx) = context.ax;
	context.ax = context.data.word(kMousey);
	context.data.word(kOldy) = context.ax;
	mousecall(context);
	context.data.word(kMousex) = context.cx;
	context.data.word(kMousey) = context.dx;
	context.data.word(kMousebutton) = context.bx;
	{assert(stack_depth == context.stack.size()); return; }
}

void readmouse1(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kMousex);
	context.data.word(kOldx) = context.ax;
	context.ax = context.data.word(kMousey);
	context.data.word(kOldy) = context.ax;
	mousecall(context);
	context.data.word(kMousex) = context.cx;
	context.data.word(kMousey) = context.dx;
	context.data.word(kMousebutton1) = context.bx;
	{assert(stack_depth == context.stack.size()); return; }
}

void readmouse2(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kMousex);
	context.data.word(kOldx) = context.ax;
	context.ax = context.data.word(kMousey);
	context.data.word(kOldy) = context.ax;
	mousecall(context);
	context.data.word(kMousex) = context.cx;
	context.data.word(kMousey) = context.dx;
	context.data.word(kMousebutton2) = context.bx;
	{assert(stack_depth == context.stack.size()); return; }
}

void readmouse3(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kMousex);
	context.data.word(kOldx) = context.ax;
	context.ax = context.data.word(kMousey);
	context.data.word(kOldy) = context.ax;
	mousecall(context);
	context.data.word(kMousex) = context.cx;
	context.data.word(kMousey) = context.dx;
	context.data.word(kMousebutton3) = context.bx;
	{assert(stack_depth == context.stack.size()); return; }
}

void readmouse4(Context & context) {
	uint stack_depth = context.stack.size();
	context.ax = context.data.word(kMousebutton);
	context.data.word(kOldbutton) = context.ax;
	context.ax = context.data.word(kMousex);
	context.data.word(kOldx) = context.ax;
	context.ax = context.data.word(kMousey);
	context.data.word(kOldy) = context.ax;
	mousecall(context);
	context.data.word(kMousex) = context.cx;
	context.data.word(kMousey) = context.dx;
	context.ax = context.data.word(kMousebutton1);
	context._or(context.ax, context.data.word(kMousebutton2));
	context._or(context.ax, context.data.word(kMousebutton3));
	context._or(context.bx, context.ax);
	context.data.word(kMousebutton) = context.bx;
	{assert(stack_depth == context.stack.size()); return; }
}

void readkey(Context & context) {
	uint stack_depth = context.stack.size();
	context.bx = context.data.word(kBufferout);
	context._cmp(context.bx, context.data.word(kBufferin));
	if (context.flags.z()) goto nokey;
	context._inc(context.bx);
	context._and(context.bx, 15);
	context.data.word(kBufferout) = context.bx;
	context.di = 5715;
	context._add(context.di, context.bx);
	context.al = context.cs.byte(context.di);
	context.data.byte(kCurrentkey) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
nokey:
	context.data.byte(kCurrentkey) = 0;
	{assert(stack_depth == context.stack.size()); return; }
}

void convertkey(Context & context) {
	uint stack_depth = context.stack.size();
	context._and(context.al, 127);
	context.ah = 0;
	context.di = 5731;
	context._add(context.di, context.ax);
	context.al = context.cs.byte(context.di);
	{assert(stack_depth == context.stack.size()); return; }
}

void randomnum1(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void randomnum2(Context & context) {
	uint stack_depth = context.stack.size();
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
	{assert(stack_depth == context.stack.size()); return; }
}

void hangon(Context & context) {
	uint stack_depth = context.stack.size();
hangonloop:
	context.push(context.cx);
	vsync(context);
	context.cx = context.pop();
	if (--context.cx) goto hangonloop;
	{assert(stack_depth == context.stack.size()); return; }
}

void loadtraveltext(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 2234;
	standardload(context);
	context.data.word(kTraveltext) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void loadintotemp(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.cs;
	standardload(context);
	context.data.word(kTempgraphics) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void loadintotemp2(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.cs;
	standardload(context);
	context.data.word(kTempgraphics2) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void loadintotemp3(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.cs;
	standardload(context);
	context.data.word(kTempgraphics3) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void loadtempcharset(Context & context) {
	uint stack_depth = context.stack.size();
	standardload(context);
	context.data.word(kTempcharset) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void standardload(Context & context) {
	uint stack_depth = context.stack.size();
	openfile(context);
	readheader(context);
	context.bx = context.es.word(context.di);
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
	{assert(stack_depth == context.stack.size()); return; }
}

void loadtemptext(Context & context) {
	uint stack_depth = context.stack.size();
	standardload(context);
	context.data.word(kTextfile1) = context.ax;
	{assert(stack_depth == context.stack.size()); return; }
}

void loadroom(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kRoomloaded) = 1;
	context.data.word(kTimecount) = 0;
	context.data.word(kMaintimer) = 0;
	context.data.word(kMapoffsetx) = 104;
	context.data.word(kMapoffsety) = 38;
	context.data.word(kTextaddressx) = 13;
	context.data.word(kTextaddressy) = 182;
	context.data.byte(kTextlen) = 240;
	context.al = context.data.byte(kNewlocation);
	context.data.byte(kLocation) = context.al;
	getroomdata(context);
	startloading(context);
	loadroomssample(context);
	switchryanon(context);
	drawflags(context);
	getdimension(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void loadroomssample(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kRoomssample);
	context._cmp(context.al, 255);
	if (context.flags.z()) goto loadedalready;
	context._cmp(context.al, context.data.byte(kCurrentsample));
	if (context.flags.z()) goto loadedalready;
	context.data.byte(kCurrentsample) = context.al;
	context.al = context.data.byte(kCurrentsample);
	context.cl = '0';
	twodigitnum(context);
	context.di = 1896;
	context._xchg(context.al, context.ah);
	context.cs.word(context.di+10) = context.ax;
	context.dx = context.di;
	loadsecondsample(context);
loadedalready:
	{assert(stack_depth == context.stack.size()); return; }
}

void getridofreels(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kRoomloaded), 0);
	if (context.flags.z()) goto dontgetrid;
	context.es = context.data.word(kReel1);
	deallocatemem(context);
	context.es = context.data.word(kReel2);
	deallocatemem(context);
	context.es = context.data.word(kReel3);
	deallocatemem(context);
dontgetrid:
	{assert(stack_depth == context.stack.size()); return; }
}

void getridofall(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kBackdrop);
	deallocatemem(context);
	context.es = context.data.word(kSetframes);
	deallocatemem(context);
	context.es = context.data.word(kReel1);
	deallocatemem(context);
	context.es = context.data.word(kReel2);
	deallocatemem(context);
	context.es = context.data.word(kReel3);
	deallocatemem(context);
	context.es = context.data.word(kReels);
	deallocatemem(context);
	context.es = context.data.word(kPeople);
	deallocatemem(context);
	context.es = context.data.word(kSetdesc);
	deallocatemem(context);
	context.es = context.data.word(kBlockdesc);
	deallocatemem(context);
	context.es = context.data.word(kRoomdesc);
	deallocatemem(context);
	context.es = context.data.word(kFreeframes);
	deallocatemem(context);
	context.es = context.data.word(kFreedesc);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void restorereels(Context & context) {
	uint stack_depth = context.stack.size();
	context._cmp(context.data.byte(kRoomloaded), 0);
	if (context.flags.z()) goto dontrestore;
	context.al = context.data.byte(kReallocation);
	getroomdata(context);
	context.dx = context.bx;
	openfile(context);
	readheader(context);
	dontloadseg(context);
	dontloadseg(context);
	dontloadseg(context);
	dontloadseg(context);
	allocateload(context);
	context.data.word(kReel1) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kReel2) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kReel3) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	closefile(context);
dontrestore:
	{assert(stack_depth == context.stack.size()); return; }
}

void restoreall(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kLocation);
	getroomdata(context);
	context.dx = context.bx;
	openfile(context);
	readheader(context);
	allocateload(context);
	context.ds = context.ax;
	context.data.word(kBackdrop) = context.ax;
	context.dx = (0);
	loadseg(context);
	context.ds = context.data.word(kWorkspace);
	context.dx = (0);
	context.cx = 132*66;
	context.al = 0;
	fillspace(context);
	loadseg(context);
	sortoutmap(context);
	allocateload(context);
	context.data.word(kSetframes) = context.ax;
	context.ds = context.ax;
	context.dx = (0);
	loadseg(context);
	dontloadseg(context);
	allocateload(context);
	context.data.word(kReel1) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kReel2) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kReel3) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kReels) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kPeople) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kSetdesc) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kBlockdesc) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kRoomdesc) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kFreeframes) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	dontloadseg(context);
	allocateload(context);
	context.data.word(kFreedesc) = context.ax;
	context.ds = context.ax;
	context.dx = (0);
	loadseg(context);
	closefile(context);
	setallchanges(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void sortoutmap(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.di);
	context.ds = context.data.word(kWorkspace);
	context.si = 0;
	context.es = context.data.word(kMapdata);
	context.di = 0;
	context.cx = (60);
blimey:
	context.push(context.cx);
	context.push(context.si);
	context.cx = (66);
	while(context.cx--) 	context._movsb();
 	context.si = context.pop();
	context.cx = context.pop();
	context._add(context.si, 132);
	if (--context.cx) goto blimey;
	context.di = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void startloading(Context & context) {
	uint stack_depth = context.stack.size();
	context.data.byte(kCombatcount) = 0;
	context.al = context.cs.byte(context.bx+13);
	context.data.byte(kRoomssample) = context.al;
	context.al = context.cs.byte(context.bx+15);
	context.data.byte(kMapx) = context.al;
	context.al = context.cs.byte(context.bx+16);
	context.data.byte(kMapy) = context.al;
	context.al = context.cs.byte(context.bx+20);
	context.data.byte(kLiftflag) = context.al;
	context.al = context.cs.byte(context.bx+21);
	context.data.byte(kManspath) = context.al;
	context.data.byte(kDestination) = context.al;
	context.data.byte(kFinaldest) = context.al;
	context.al = context.cs.byte(context.bx+22);
	context.data.byte(kFacing) = context.al;
	context.data.byte(kTurntoface) = context.al;
	context.al = context.cs.byte(context.bx+23);
	context.data.byte(kCounttoopen) = context.al;
	context.al = context.cs.byte(context.bx+24);
	context.data.byte(kLiftpath) = context.al;
	context.al = context.cs.byte(context.bx+25);
	context.data.byte(kDoorpath) = context.al;
	context.data.byte(kLastweapon) = -1;
	context.al = context.cs.byte(context.bx+27);
	context.push(context.ax);
	context.al = context.cs.byte(context.bx+31);
	context.ah = context.data.byte(kReallocation);
	context.data.byte(kReallocation) = context.al;
	context.dx = context.bx;
	openfile(context);
	readheader(context);
	allocateload(context);
	context.ds = context.ax;
	context.data.word(kBackdrop) = context.ax;
	context.dx = (0);
	loadseg(context);
	context.ds = context.data.word(kWorkspace);
	context.dx = (0);
	context.cx = 132*66;
	context.al = 0;
	fillspace(context);
	loadseg(context);
	sortoutmap(context);
	allocateload(context);
	context.data.word(kSetframes) = context.ax;
	context.ds = context.ax;
	context.dx = (0);
	loadseg(context);
	context.ds = context.data.word(kSetdat);
	context.dx = 0;
	context.cx = (64*128);
	context.al = 255;
	fillspace(context);
	loadseg(context);
	allocateload(context);
	context.data.word(kReel1) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kReel2) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kReel3) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kReels) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kPeople) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kSetdesc) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kBlockdesc) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kRoomdesc) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	allocateload(context);
	context.data.word(kFreeframes) = context.ax;
	context.ds = context.ax;
	context.dx = 0;
	loadseg(context);
	context.ds = context.data.word(kFreedat);
	context.dx = 0;
	context.cx = (16*80);
	context.al = 255;
	fillspace(context);
	loadseg(context);
	allocateload(context);
	context.data.word(kFreedesc) = context.ax;
	context.ds = context.ax;
	context.dx = (0);
	loadseg(context);
	closefile(context);
	findroominloc(context);
	deletetaken(context);
	setallchanges(context);
	autoappear(context);
	context.al = context.data.byte(kNewlocation);
	getroomdata(context);
	context.data.byte(kLastweapon) = -1;
	context.data.byte(kMandead) = 0;
	context.data.word(kLookcounter) = 160;
	context.data.byte(kNewlocation) = 255;
	context.data.byte(kLinepointer) = 254;
	context.ax = context.pop();
	context._cmp(context.al, 255);
	if (context.flags.z()) goto dontwalkin;
	context.data.byte(kManspath) = context.al;
	context.push(context.bx);
	autosetwalk(context);
	context.bx = context.pop();
dontwalkin:
	findxyfrompath(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void disablepath(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.cx);
	context._xchg(context.al, context.ah);
	context.cx = -6;
looky2:
	context._add(context.cx, 6);
	context._sub(context.al, 10);
	if (!context.flags.c()) goto looky2;
	context.al = context.ah;
	context._dec(context.cx);
lookx2:
	context._inc(context.cx);
	context._sub(context.al, 11);
	if (!context.flags.c()) goto lookx2;
	context.al = context.cl;
	context.ah = 0;
	context.cx = 144;
	context._mul(context.cx);
	context.es = context.data.word(kReels);
	context.bx = (0);
	context._add(context.bx, context.ax);
	context.ax = context.pop();
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.al = 0;
	context.es.byte(context.bx+6) = context.al;
	{assert(stack_depth == context.stack.size()); return; }
}

void findxyfrompath(Context & context) {
	uint stack_depth = context.stack.size();
	getroomspaths(context);
	context.al = context.data.byte(kManspath);
	context.ah = 0;
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.ax, context.ax);
	context._add(context.bx, context.ax);
	context.ax = context.es.word(context.bx);
	context._sub(context.al, 12);
	context._sub(context.ah, 12);
	context.data.byte(kRyanx) = context.al;
	context.data.byte(kRyany) = context.ah;
	{assert(stack_depth == context.stack.size()); return; }
}

void findroominloc(Context & context) {
	uint stack_depth = context.stack.size();
	context.al = context.data.byte(kMapy);
	context.cx = -6;
looky:
	context._add(context.cx, 6);
	context._sub(context.al, 10);
	if (!context.flags.c()) goto looky;
	context.al = context.data.byte(kMapx);
	context._dec(context.cx);
lookx:
	context._inc(context.cx);
	context._sub(context.al, 11);
	if (!context.flags.c()) goto lookx;
	context.data.byte(kRoomnum) = context.cl;
	{assert(stack_depth == context.stack.size()); return; }
}

void getroomdata(Context & context) {
	uint stack_depth = context.stack.size();
	context.ah = 0;
	context.cx = 32;
	context._mul(context.cx);
	context.bx = 5958;
	context._add(context.bx, context.ax);
	{assert(stack_depth == context.stack.size()); return; }
}

void readheader(Context & context) {
	uint stack_depth = context.stack.size();
	context.ds = context.cs;
	context.dx = 5862;
	context.cx = (5958-5862);
	readfromfile(context);
	context.es = context.cs;
	context.di = 5912;
	{assert(stack_depth == context.stack.size()); return; }
}

void allocateload(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.di);
	context.bx = context.es.word(context.di);
	context.cl = 4;
	context._shr(context.bx, context.cl);
	allocatemem(context);
	context.di = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void fillspace(Context & context) {
	uint stack_depth = context.stack.size();
	context.push(context.es);
	context.push(context.ds);
	context.push(context.dx);
	context.push(context.di);
	context.push(context.bx);
	context.di = context.dx;
	context.es = context.ds;
	while(context.cx--) 	context._stosb();
	context.bx = context.pop();
	context.di = context.pop();
	context.dx = context.pop();
	context.ds = context.pop();
	context.es = context.pop();
	{assert(stack_depth == context.stack.size()); return; }
}

void getridoftemp(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kTempgraphics);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getridoftemptext(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kTextfile1);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getridoftemp2(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kTempgraphics2);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getridoftemp3(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kTempgraphics3);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getridoftempcharset(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kTempcharset);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void getridoftempsp(Context & context) {
	uint stack_depth = context.stack.size();
	context.es = context.data.word(kTempsprites);
	deallocatemem(context);
	{assert(stack_depth == context.stack.size()); return; }
}

void readsetdata(Context & context) {
	uint stack_depth = context.stack.size();
	context.dx = 1857;
	standardload(context);
	context.data.word(kCharset1) = context.ax;
	context.dx = 1922;
	standardload(context);
	context.data.word(kIcons1) = context.ax;
	context.dx = 1935;
	standardload(context);
	context.data.word(kIcons2) = context.ax;
	context.dx = 1819;
	standardload(context);
	context.data.word(kMainsprites) = context.ax;
	context.dx = 2221;
	standardload(context);
	context.data.word(kPuzzletext) = context.ax;
	context.dx = 2273;
	standardload(context);
	context.data.word(kCommandtext) = context.ax;
	context.ax = context.data.word(kCharset1);
	context.data.word(kCurrentset) = context.ax;
	context._cmp(context.data.byte(kSoundint), 255);
	if (context.flags.z()) goto novolumeload;
	context.dx = 2286;
	openfile(context);
	context.cx = 2048-256;
	context.ds = context.data.word(kSoundbuffer);
	context.dx = 16384;
	readfromfile(context);
	closefile(context);
novolumeload:
	{assert(stack_depth == context.stack.size()); return; }
}

void dreamweb(Context & context) {
	uint stack_depth = context.stack.size();
	seecommandtail(context);
	checkbasemem(context);
	soundstartup(context);
	setkeyboardint(context);
	setupemm(context);
	allocatebuffers(context);
	setmouse(context);
	fadedos(context);
	gettime(context);
	clearbuffers(context);
	clearpalette(context);
	set16colpalette(context);
	readsetdata(context);
	context.data.byte(kWongame) = 0;
	context.dx = 1909;
	loadsample(context);
	setsoundoff(context);
	scanfornames(context);
	context._cmp(context.al, 0);
	if (!context.flags.z()) goto dodecisions;
	setmode(context);
	loadpalfromiff(context);
	titles(context);
	credits(context);
	goto playgame;
dodecisions:
	cls(context);
	setmode(context);
	decide(context);
	context._cmp(context.data.byte(kGetback), 4);
	if (context.flags.z()) goto mainloop;
	titles(context);
	credits(context);
playgame:
	clearchanges(context);
	setmode(context);
	loadpalfromiff(context);
	context.data.byte(kLocation) = 255;
	context.data.byte(kRoomafterdream) = 1;
	context.data.byte(kNewlocation) = 35;
	context.data.byte(kVolume) = 7;
	loadroom(context);
	clearsprites(context);
	initman(context);
	entrytexts(context);
	entryanims(context);
	context.data.byte(kDestpos) = 3;
	initialinv(context);
	context.data.byte(kLastflag) = 32;
	startup1(context);
	context.data.byte(kVolumeto) = 0;
	context.data.byte(kVolumedirection) = -1;
	context.data.byte(kCommandtype) = 255;
	goto mainloop;
loadnew:
	clearbeforeload(context);
	loadroom(context);
	clearsprites(context);
	initman(context);
	entrytexts(context);
	entryanims(context);
	context.data.byte(kNewlocation) = 255;
	startup(context);
	context.data.byte(kCommandtype) = 255;
	worktoscreenm(context);
	goto mainloop;
alreadyloaded:
	context.data.byte(kNewlocation) = 255;
	clearsprites(context);
	initman(context);
	startup(context);
	context.data.byte(kCommandtype) = 255;
mainloop:
	screenupdate(context);
	context._cmp(context.data.byte(kWongame), 0);
	if (!context.flags.z()) goto endofgame;
	context._cmp(context.data.byte(kMandead), 1);
	if (context.flags.z()) goto gameover;
	context._cmp(context.data.byte(kMandead), 2);
	if (context.flags.z()) goto gameover;
	context._cmp(context.data.word(kWatchingtime), 0);
	if (context.flags.z()) goto notwatching;
	context.al = context.data.byte(kFinaldest);
	context._cmp(context.al, context.data.byte(kManspath));
	if (!context.flags.z()) goto mainloop;
	context._dec(context.data.word(kWatchingtime));
	if (!context.flags.z()) goto mainloop;
notwatching:
	context._cmp(context.data.byte(kMandead), 4);
	if (context.flags.z()) goto gameover;
	context._cmp(context.data.byte(kNewlocation), 255);
	if (!context.flags.z()) goto loadnew;
	goto mainloop;
gameover:
	clearbeforeload(context);
	showgun(context);
	fadescreendown(context);
	context.cx = 100;
	hangon(context);
	goto dodecisions;
endofgame:
	clearbeforeload(context);
	fadescreendowns(context);
	context.cx = 200;
	hangon(context);
	endgame(context);
	{ quickquit2(context); return; };
}



void __start(Context &context) { 
	static const uint8 src[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x13, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0xff, 0x00, 
		0xff, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x0d, 0x00, 0xb6, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x01, 0x2c, 0x00, 0x14, 0x00, 0x02, 0x00, 0x01, 0x01, 0x37, 
		0x00, 0x00, 0x00, 0x32, 0x14, 0x00, 0x18, 0x16, 0x00, 0x4a, 0x00, 0x01, 0x00, 0x00, 0x18, 0x21, 
		0x0a, 0x4b, 0x00, 0x01, 0x00, 0x01, 0x01, 0x2c, 0x00, 0x1b, 0x00, 0x02, 0x00, 0x02, 0x01, 0x2c, 
		0x00, 0x60, 0x00, 0x03, 0x00, 0x04, 0x01, 0x2c, 0x00, 0x76, 0x00, 0x02, 0x00, 0x05, 0x01, 0x2c, 
		0x0a, 0x00, 0x00, 0x02, 0x00, 0x00, 0x05, 0x16, 0x14, 0x35, 0x00, 0x03, 0x00, 0x00, 0x05, 0x16, 
		0x14, 0x28, 0x00, 0x01, 0x00, 0x02, 0x05, 0x16, 0x14, 0x32, 0x00, 0x01, 0x00, 0x03, 0x02, 0x0b, 
		0x0a, 0xc0, 0x00, 0x01, 0x00, 0x00, 0x02, 0x0b, 0x0a, 0xb6, 0x00, 0x02, 0x00, 0x01, 0x08, 0x0b, 
		0x0a, 0x00, 0x00, 0x02, 0x00, 0x01, 0x17, 0x00, 0x32, 0x00, 0x00, 0x03, 0x00, 0x00, 0x1c, 0x0b, 
		0x14, 0xfa, 0x00, 0x04, 0x00, 0x00, 0x17, 0x00, 0x32, 0x2b, 0x00, 0x02, 0x00, 0x08, 0x17, 0x0b, 
		0x28, 0x82, 0x00, 0x02, 0x00, 0x01, 0x17, 0x16, 0x28, 0x7a, 0x00, 0x02, 0x00, 0x02, 0x17, 0x16, 
		0x28, 0x69, 0x00, 0x02, 0x00, 0x03, 0x17, 0x16, 0x28, 0x51, 0x00, 0x02, 0x00, 0x04, 0x17, 0x0b, 
		0x28, 0x87, 0x00, 0x02, 0x00, 0x05, 0x17, 0x16, 0x28, 0x91, 0x00, 0x02, 0x00, 0x06, 0x04, 0x16, 
		0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x2d, 0x16, 0x1e, 0xc8, 0x00, 0x00, 0x00, 0x14, 0x2d, 0x16, 
		0x1e, 0x27, 0x00, 0x02, 0x00, 0x00, 0x2d, 0x16, 0x1e, 0x19, 0x00, 0x02, 0x00, 0x00, 0x08, 0x16, 
		0x28, 0x20, 0x00, 0x02, 0x00, 0x00, 0x07, 0x0b, 0x14, 0x40, 0x00, 0x02, 0x00, 0x00, 0x16, 0x16, 
		0x14, 0x52, 0x00, 0x02, 0x00, 0x00, 0x1b, 0x0b, 0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x14, 0x00, 
		0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x0e, 0x21, 0x28, 0x15, 0x00, 0x01, 0x00, 0x00, 0x1d, 0x0b, 
		0x0a, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x19, 0x00, 
		0x32, 0x04, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 0x1e, 0x79, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 
		0x1e, 0x00, 0x00, 0x14, 0x00, 0x00, 0x34, 0x16, 0x1e, 0xc0, 0x00, 0x02, 0x00, 0x00, 0x34, 0x16, 
		0x1e, 0xe9, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 0x28, 0x68, 0x00, 0x37, 0x00, 0x00, 0x35, 0x21, 
		0x00, 0x63, 0x00, 0x02, 0x00, 0x00, 0x32, 0x16, 0x28, 0x00, 0x00, 0x03, 0x00, 0x00, 0x32, 0x16, 
		0x1e, 0xa2, 0x00, 0x02, 0x00, 0x00, 0x34, 0x16, 0x1e, 0x39, 0x00, 0x02, 0x00, 0x00, 0x34, 0x16, 
		0x1e, 0x00, 0x00, 0x02, 0x00, 0x00, 0x36, 0x00, 0x00, 0x48, 0x00, 0x03, 0x00, 0x00, 0x37, 0x2c, 
		0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x0e, 0x16, 
		0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x0e, 0x16, 0x00, 0x2c, 0x01, 0x01, 0x00, 0x00, 0x0a, 0x16, 
		0x1e, 0xae, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x16, 0x14, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0b, 0x0b, 
		0x14, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0b, 0x0b, 0x1e, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0b, 0x16, 
		0x14, 0x00, 0x00, 0x32, 0x14, 0x00, 0x0e, 0x21, 0x28, 0x00, 0x00, 0x32, 0x14, 0x00, 0xff, 0x7c, 
		0xc0, 0x80, 0xc0, 0x1c, 0xc0, 0x20, 0xc0, 0x00, 0xc1, 0x10, 0xc0, 0x18, 0xc0, 0xf4, 0xc0, 0x0c, 
		0xc0, 0x24, 0xc0, 0x28, 0xc0, 0x2c, 0xc0, 0x30, 0xc0, 0x54, 0xc0, 0x78, 0xc0, 0x50, 0xc0, 0x74, 
		0xc0, 0x34, 0xc0, 0x38, 0xc0, 0x40, 0xc0, 0x44, 0xc0, 0x48, 0xc0, 0x3c, 0xc0, 0x14, 0xc0, 0x88, 
		0xc0, 0x8c, 0xc0, 0x90, 0xc0, 0x70, 0xc0, 0xfc, 0xc0, 0x6c, 0xc0, 0x58, 0xc0, 0x68, 0xc0, 0x04, 
		0xc1, 0x64, 0xc0, 0x60, 0xc0, 0x5c, 0xc0, 0x94, 0xc0, 0x04, 0xc0, 0xa4, 0xc0, 0x9c, 0xc0, 0xa0, 
		0xc0, 0xa8, 0xc0, 0xac, 0xc0, 0x98, 0xc0, 0xb0, 0xc0, 0xb4, 0xc0, 0xc8, 0xc0, 0xcc, 0xc0, 0xd4, 
		0xc0, 0xdc, 0xc0, 0xd8, 0xc0, 0x00, 0xc0, 0x08, 0xc0, 0x84, 0xc0, 0x84, 0xc0, 0x84, 0xc0, 0x84, 
		0xc0, 0x00, 0x3c, 0x21, 0x47, 0x0b, 0x52, 0x16, 0x5d, 0x01, 0x2c, 0x0a, 0x10, 0x04, 0x0b, 0x1e, 
		0x0e, 0x04, 0x16, 0x1e, 0x0e, 0x03, 0x21, 0x0a, 0x0e, 0x0a, 0x21, 0x1e, 0x0e, 0x0a, 0x16, 0x1e, 
		0x18, 0x09, 0x16, 0x0a, 0x0e, 0x02, 0x21, 0x00, 0x0e, 0x02, 0x16, 0x00, 0x0e, 0x06, 0x0b, 0x1e, 
		0x0e, 0x07, 0x0b, 0x14, 0x12, 0x07, 0x00, 0x14, 0x12, 0x07, 0x00, 0x1e, 0x12, 0x37, 0x2c, 0x00, 
		0x0e, 0x05, 0x16, 0x1e, 0x0e, 0x08, 0x00, 0x0a, 0x12, 0x08, 0x0b, 0x0a, 0x12, 0x08, 0x16, 0x0a, 
		0x12, 0x08, 0x21, 0x0a, 0x12, 0x08, 0x21, 0x14, 0x12, 0x08, 0x21, 0x1e, 0x12, 0x08, 0x21, 0x28, 
		0x12, 0x08, 0x16, 0x28, 0x12, 0x08, 0x0b, 0x28, 0x12, 0x15, 0x2c, 0x14, 0x12, 0xff, 0x2e, 0x05, 
		0x2f, 0x05, 0x33, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x46, 0x05, 0x2e, 0x05, 0x4d, 0x05, 
		0x5d, 0x05, 0x64, 0x05, 0x68, 0x05, 0x6c, 0x05, 0x70, 0x05, 0x7d, 0x05, 0x2e, 0x05, 0x2e, 0x05, 
		0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x9f, 0x05, 0x2e, 0x05, 0xb5, 0x05, 0xd4, 0x05, 0x2e, 0x05, 
		0xe1, 0x05, 0xf7, 0x05, 0x0d, 0x06, 0x26, 0x06, 0x39, 0x06, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 
		0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 
		0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x49, 0x06, 0x50, 0x06, 0x75, 0x06, 0x2e, 0x05, 
		0x2e, 0x05, 0x2e, 0x05, 0x2e, 0x05, 0x82, 0x06, 0x86, 0x06, 0x2e, 0x05, 0x8d, 0x06, 0xff, 0x0f, 
		0x01, 0x01, 0xff, 0x0c, 0x05, 0x00, 0x0d, 0x15, 0x00, 0x0f, 0x23, 0x00, 0x11, 0x32, 0x00, 0x12, 
		0x67, 0x00, 0x13, 0x6c, 0x00, 0xff, 0x12, 0x13, 0x00, 0x13, 0x17, 0x00, 0xff, 0x0c, 0x33, 0x00, 
		0x0d, 0x35, 0x00, 0x0e, 0x0e, 0x00, 0x0f, 0x14, 0x00, 0x00, 0x4e, 0x00, 0xff, 0x0c, 0x77, 0x00, 
		0x0c, 0x91, 0x00, 0xff, 0x0d, 0x10, 0x00, 0xff, 0x0d, 0x14, 0x00, 0xff, 0x0e, 0x10, 0x00, 0xff, 
		0x0f, 0x04, 0x00, 0x10, 0x08, 0x00, 0x11, 0x86, 0x00, 0x12, 0x99, 0x00, 0xff, 0x0d, 0x6c, 0x00, 
		0x0f, 0x46, 0x01, 0x0f, 0x4b, 0x01, 0x0f, 0x50, 0x01, 0x0f, 0x56, 0x01, 0x0f, 0x5c, 0x01, 0x0f, 
		0x62, 0x01, 0x12, 0x9f, 0x00, 0x12, 0xb2, 0x00, 0x93, 0xd9, 0x00, 0x54, 0xe4, 0x00, 0xff, 0x0d, 
		0x14, 0x00, 0x0d, 0x15, 0x00, 0x0f, 0x22, 0x00, 0x0d, 0x34, 0x00, 0x0d, 0x37, 0x00, 0x19, 0x39, 
		0x00, 0x15, 0x49, 0x00, 0xff, 0x0d, 0xc4, 0x00, 0x0d, 0xea, 0x00, 0x0d, 0x9c, 0x00, 0x0e, 0x81, 
		0x00, 0x0d, 0x7c, 0x00, 0x0f, 0xa2, 0x00, 0x0f, 0xc8, 0x00, 0x0f, 0xef, 0x00, 0x11, 0x63, 0x00, 
		0x0c, 0x34, 0x00, 0xff, 0x0f, 0x38, 0x00, 0x10, 0x40, 0x00, 0x13, 0x16, 0x00, 0x14, 0x21, 0x00, 
		0xff, 0x14, 0x0b, 0x00, 0x14, 0x0f, 0x00, 0x0f, 0x1c, 0x00, 0x0d, 0x50, 0x00, 0x15, 0x52, 0x00, 
		0x93, 0x57, 0x00, 0x57, 0x80, 0x00, 0xff, 0x0c, 0x0d, 0x00, 0x0e, 0x27, 0x00, 0x0c, 0x43, 0x00, 
		0x0c, 0x4b, 0x00, 0x0c, 0x53, 0x00, 0x0c, 0x5b, 0x00, 0x0f, 0x66, 0x00, 0xff, 0x16, 0x24, 0x00, 
		0x0d, 0x7d, 0x00, 0x12, 0x58, 0x00, 0x0f, 0x6b, 0x00, 0x0e, 0x7f, 0x00, 0x0e, 0x9a, 0x00, 0x93, 
		0xaa, 0x00, 0x57, 0xe8, 0x00, 0xff, 0x15, 0x10, 0x00, 0x15, 0x48, 0x00, 0x15, 0xcd, 0x00, 0x16, 
		0x3f, 0x00, 0x97, 0x63, 0x00, 0x58, 0x9e, 0x00, 0xff, 0x0d, 0x15, 0x00, 0x0e, 0x18, 0x00, 0x93, 
		0x32, 0x00, 0x57, 0x4b, 0x00, 0x18, 0x80, 0x00, 0xff, 0x53, 0x2e, 0x00, 0x10, 0xa7, 0x00, 0xff, 
		0x10, 0x13, 0x00, 0x0e, 0x24, 0x00, 0x10, 0x32, 0x00, 0x0e, 0x41, 0x00, 0x10, 0x51, 0x00, 0x0e, 
		0x60, 0x00, 0x10, 0x72, 0x00, 0x0e, 0x81, 0x00, 0x10, 0x93, 0x00, 0x0e, 0xa2, 0x00, 0x10, 0xb1, 
		0x00, 0x0e, 0xbf, 0x00, 0xff, 0x0d, 0x30, 0x00, 0x0e, 0x29, 0x00, 0x0f, 0x4e, 0x00, 0x10, 0x5c, 
		0x00, 0xff, 0x10, 0x73, 0x00, 0xff, 0x15, 0x67, 0x00, 0x14, 0xc7, 0x00, 0xff, 0x11, 0x35, 0x00, 
		0x11, 0x36, 0x00, 0x11, 0x37, 0x00, 0x11, 0x38, 0x00, 0x11, 0x39, 0x00, 0x11, 0x3a, 0x00, 0x11, 
		0x3b, 0x00, 0x11, 0x3d, 0x00, 0x11, 0x3f, 0x00, 0x11, 0x40, 0x00, 0x11, 0x41, 0x00, 0xff, 0x9c, 
		0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 
		0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 
		0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 
		0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 
		0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 
		0x99, 0x9f, 0x9e, 0x9c, 0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9e, 0x9c, 
		0x9a, 0x9f, 0x9a, 0x9c, 0x9e, 0xa0, 0x9b, 0x9d, 0x99, 0x9f, 0x9c, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x53, 0x30, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 
		0x2e, 0x53, 0x30, 0x32, 0x00, 0x49, 0x4e, 0x53, 0x54, 0x41, 0x4c, 0x4c, 0x2e, 0x44, 0x41, 0x54, 
		0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x43, 0x30, 0x30, 0x00, 0x44, 0x52, 
		0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x43, 0x30, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x43, 0x30, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 
		0x2e, 0x56, 0x30, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x56, 0x39, 
		0x39, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x30, 0x00, 0x44, 
		0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 
		0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 
		0x42, 0x2e, 0x47, 0x30, 0x38, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 
		0x30, 0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x37, 0x00, 
		0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x34, 0x00, 0x44, 0x52, 0x45, 
		0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x35, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 
		0x45, 0x42, 0x2e, 0x47, 0x30, 0x36, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 
		0x47, 0x31, 0x34, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x30, 0x31, 
		0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x30, 0x32, 0x00, 0x44, 0x52, 
		0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x31, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x54, 0x31, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 
		0x2e, 0x54, 0x31, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x31, 
		0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x32, 0x30, 0x00, 0x44, 
		0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x32, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 
		0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x32, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 
		0x42, 0x2e, 0x54, 0x32, 0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 
		0x32, 0x34, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x35, 0x30, 0x00, 
		0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x35, 0x31, 0x00, 0x44, 0x52, 0x45, 
		0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x38, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 
		0x45, 0x42, 0x2e, 0x54, 0x38, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 
		0x54, 0x38, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x38, 0x33, 
		0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x54, 0x38, 0x34, 0x00, 0x44, 0x52, 
		0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x56, 0x4f, 0x4c, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 
		0x57, 0x45, 0x42, 0x2e, 0x47, 0x30, 0x39, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 
		0x2e, 0x47, 0x31, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 
		0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 0x32, 0x00, 0x44, 
		0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 
		0x4d, 0x57, 0x45, 0x42, 0x2e, 0x47, 0x31, 0x35, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 
		0x42, 0x2e, 0x49, 0x30, 0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 
		0x30, 0x31, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 0x30, 0x32, 0x00, 
		0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 0x30, 0x33, 0x00, 0x44, 0x52, 0x45, 
		0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 0x30, 0x34, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 
		0x45, 0x42, 0x2e, 0x49, 0x30, 0x35, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 
		0x49, 0x30, 0x36, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x49, 0x30, 0x37, 
		0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x50, 0x41, 0x4c, 0x00, 0x11, 0x01, 
		0x40, 0x01, 0x9d, 0x00, 0xc6, 0x00, 0x44, 0xc3, 0x04, 0x01, 0x2c, 0x01, 0x00, 0x00, 0x2c, 0x00, 
		0x80, 0xc5, 0xd2, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x2c, 0x00, 0xdc, 0xc3, 0x90, 0x00, 0xb0, 0x00, 
		0x40, 0x00, 0x60, 0x00, 0x80, 0xc3, 0x00, 0x00, 0x32, 0x00, 0x32, 0x00, 0xc8, 0x00, 0x84, 0xc3, 
		0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x11, 0x01, 0x40, 0x01, 
		0x9d, 0x00, 0xc6, 0x00, 0x44, 0xc3, 0xff, 0x00, 0x26, 0x01, 0x00, 0x00, 0x18, 0x00, 0xc8, 0xc3, 
		0xf7, 0x00, 0x2d, 0x01, 0x28, 0x00, 0x38, 0x00, 0x48, 0xc3, 0x50, 0x00, 0x00, 0x01, 0x9e, 0x00, 
		0xca, 0x00, 0xe0, 0xc3, 0x50, 0x00, 0x2c, 0x01, 0x3a, 0x00, 0x92, 0x00, 0x98, 0xc3, 0x00, 0x00, 
		0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x11, 0x01, 0x40, 0x01, 0x9d, 0x00, 
		0xc6, 0x00, 0x44, 0xc3, 0xf7, 0x00, 0x2d, 0x01, 0x28, 0x00, 0x38, 0x00, 0x48, 0xc3, 0x50, 0x00, 
		0x2c, 0x01, 0x3a, 0x00, 0x92, 0x00, 0xbc, 0xc6, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 
		0xa0, 0xca, 0xff, 0xff, 0x11, 0x01, 0x40, 0x01, 0x9d, 0x00, 0xc6, 0x00, 0x7c, 0xc4, 0xf0, 0x00, 
		0x22, 0x01, 0x02, 0x00, 0x2c, 0x00, 0x94, 0xc4, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 
		0xa0, 0xca, 0xff, 0xff, 0x11, 0x01, 0x40, 0x01, 0x9d, 0x00, 0xc6, 0x00, 0x7c, 0xc4, 0x00, 0x00, 
		0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0xee, 0x00, 0x02, 0x01, 0x04, 0x00, 
		0x2c, 0x00, 0xc8, 0xc4, 0x68, 0x00, 0x7c, 0x00, 0x04, 0x00, 0x2c, 0x00, 0xcc, 0xc4, 0x18, 0x01, 
		0x34, 0x01, 0x04, 0x00, 0x2c, 0x00, 0xb0, 0xc4, 0x68, 0x00, 0xd8, 0x00, 0x8a, 0x00, 0xc0, 0x00, 
		0xd0, 0xc4, 0x11, 0x01, 0x40, 0x01, 0x9d, 0x00, 0xc6, 0x00, 0x7c, 0xc4, 0x00, 0x00, 0x40, 0x01, 
		0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x45, 0x58, 0x49, 0x54, 0x20, 0x48, 0x45, 0x4c, 
		0x50, 0x20, 0x4c, 0x49, 0x53, 0x54, 0x20, 0x52, 0x45, 0x41, 0x44, 0x20, 0x4c, 0x4f, 0x47, 0x4f, 
		0x4e, 0x20, 0x4b, 0x45, 0x59, 0x53, 0x20, 0x01, 0x00, 0x50, 0x55, 0x42, 0x4c, 0x49, 0x43, 0x20, 
		0x50, 0x55, 0x42, 0x4c, 0x49, 0x43, 0x20, 0x00, 0x00, 0x00, 0x42, 0x4c, 0x41, 0x43, 0x4b, 0x44, 
		0x52, 0x41, 0x47, 0x4f, 0x4e, 0x20, 0x52, 0x59, 0x41, 0x4e, 0x20, 0x00, 0x00, 0x00, 0x48, 0x45, 
		0x4e, 0x44, 0x52, 0x49, 0x58, 0x20, 0x4c, 0x4f, 0x55, 0x49, 0x53, 0x20, 0x00, 0x00, 0x00, 0x53, 
		0x45, 0x50, 0x54, 0x49, 0x4d, 0x55, 0x53, 0x20, 0x42, 0x45, 0x43, 0x4b, 0x45, 0x54, 0x54, 0x20, 
		0x00, 0xff, 0xff, 0x20, 0x00, 0x22, 0x52, 0x4f, 0x4f, 0x54, 0x20, 0x00, 0x22, 0x20, 0x00, 0x4e, 
		0x45, 0x54, 0x57, 0xe8, 0xc4, 0x45, 0x4c, 0x56, 0x41, 0x8c, 0xc6, 0x45, 0x4c, 0x56, 0x42, 0x9c, 
		0xc6, 0x45, 0x4c, 0x56, 0x43, 0x94, 0xc6, 0x45, 0x4c, 0x56, 0x45, 0x98, 0xc6, 0x45, 0x4c, 0x56, 
		0x46, 0xa0, 0xc6, 0x43, 0x47, 0x41, 0x54, 0x30, 0xc7, 0x52, 0x45, 0x4d, 0x4f, 0xa8, 0xc6, 0x42, 
		0x55, 0x54, 0x41, 0x3c, 0xc7, 0x43, 0x42, 0x4f, 0x58, 0x44, 0xc7, 0x4c, 0x49, 0x54, 0x45, 0x5c, 
		0xc6, 0x50, 0x4c, 0x41, 0x54, 0x40, 0xc7, 0x4c, 0x49, 0x46, 0x54, 0x7c, 0xc6, 0x57, 0x49, 0x52, 
		0x45, 0x84, 0xc6, 0x48, 0x4e, 0x44, 0x4c, 0x88, 0xc6, 0x48, 0x41, 0x43, 0x48, 0x80, 0xc6, 0x44, 
		0x4f, 0x4f, 0x52, 0xb4, 0xc6, 0x43, 0x53, 0x48, 0x52, 0x70, 0xc6, 0x47, 0x55, 0x4e, 0x41, 0x34, 
		0xc7, 0x43, 0x52, 0x41, 0x41, 0x64, 0xc6, 0x43, 0x52, 0x42, 0x42, 0x68, 0xc6, 0x43, 0x52, 0x43, 
		0x43, 0x6c, 0xc6, 0x53, 0x45, 0x41, 0x54, 0xf8, 0xc5, 0x4d, 0x45, 0x4e, 0x55, 0x98, 0xc7, 0x43, 
		0x4f, 0x4f, 0x4b, 0xac, 0xc6, 0x45, 0x4c, 0x43, 0x41, 0x4c, 0xc6, 0x45, 0x44, 0x43, 0x41, 0x50, 
		0xc6, 0x44, 0x44, 0x43, 0x41, 0x54, 0xc6, 0x41, 0x4c, 0x54, 0x52, 0x04, 0xc6, 0x4c, 0x4f, 0x4b, 
		0x41, 0x3c, 0xc6, 0x4c, 0x4f, 0x4b, 0x42, 0x40, 0xc6, 0x45, 0x4e, 0x54, 0x41, 0x10, 0xc6, 0x45, 
		0x4e, 0x54, 0x42, 0x24, 0xc6, 0x45, 0x4e, 0x54, 0x45, 0x28, 0xc6, 0x45, 0x4e, 0x54, 0x43, 0x18, 
		0xc6, 0x45, 0x4e, 0x54, 0x44, 0x2c, 0xc6, 0x45, 0x4e, 0x54, 0x48, 0x30, 0xc6, 0x57, 0x57, 0x41, 
		0x54, 0xf0, 0xc5, 0x50, 0x4f, 0x4f, 0x4c, 0x58, 0xc6, 0x57, 0x53, 0x48, 0x44, 0xf4, 0xc5, 0x47, 
		0x52, 0x41, 0x46, 0x44, 0xc6, 0x54, 0x52, 0x41, 0x50, 0x48, 0xc6, 0x43, 0x44, 0x50, 0x45, 0x28, 
		0xc7, 0x44, 0x4c, 0x4f, 0x4b, 0x08, 0xc6, 0x48, 0x4f, 0x4c, 0x45, 0x00, 0xc6, 0x44, 0x52, 0x59, 
		0x52, 0x0c, 0xc6, 0x48, 0x4f, 0x4c, 0x59, 0xfc, 0xc5, 0x57, 0x41, 0x4c, 0x4c, 0x2c, 0xc7, 0x42, 
		0x4f, 0x4f, 0x4b, 0x08, 0xc8, 0x41, 0x58, 0x45, 0x44, 0xb0, 0xc6, 0x53, 0x48, 0x4c, 0x44, 0x38, 
		0xc7, 0x42, 0x43, 0x4e, 0x59, 0xe8, 0xc5, 0x4c, 0x49, 0x44, 0x43, 0xe4, 0xc5, 0x4c, 0x49, 0x44, 
		0x55, 0xe0, 0xc5, 0x4c, 0x49, 0x44, 0x4f, 0xec, 0xc5, 0x50, 0x49, 0x50, 0x45, 0xa8, 0xc5, 0x42, 
		0x41, 0x4c, 0x43, 0x20, 0xc6, 0x57, 0x49, 0x4e, 0x44, 0x1c, 0xc6, 0x50, 0x41, 0x50, 0x52, 0xb4, 
		0xc7, 0x55, 0x57, 0x54, 0x41, 0xa0, 0xc5, 0x55, 0x57, 0x54, 0x42, 0xa0, 0xc5, 0x53, 0x54, 0x41, 
		0x54, 0xd8, 0xc7, 0x54, 0x4c, 0x49, 0x44, 0x9c, 0xc5, 0x53, 0x4c, 0x41, 0x42, 0xd8, 0xc5, 0x43, 
		0x41, 0x52, 0x54, 0xdc, 0xc5, 0x46, 0x43, 0x41, 0x52, 0xac, 0xc5, 0x53, 0x4c, 0x42, 0x41, 0xc0, 
		0xc5, 0x53, 0x4c, 0x42, 0x42, 0xc4, 0xc5, 0x53, 0x4c, 0x42, 0x43, 0xcc, 0xc5, 0x53, 0x4c, 0x42, 
		0x44, 0xc8, 0xc5, 0x53, 0x4c, 0x42, 0x45, 0xd0, 0xc5, 0x53, 0x4c, 0x42, 0x46, 0xd4, 0xc5, 0x50, 
		0x4c, 0x49, 0x4e, 0xb0, 0xc5, 0x4c, 0x41, 0x44, 0x44, 0xb8, 0xc5, 0x4c, 0x41, 0x44, 0x42, 0xbc, 
		0xc5, 0x47, 0x55, 0x4d, 0x41, 0xb4, 0xc5, 0x53, 0x51, 0x45, 0x45, 0x88, 0xc5, 0x54, 0x41, 0x50, 
		0x50, 0x8c, 0xc5, 0x47, 0x55, 0x49, 0x54, 0x90, 0xc5, 0x43, 0x4f, 0x4e, 0x54, 0x94, 0xc5, 0x42, 
		0x45, 0x4c, 0x4c, 0x98, 0xc5, 0x8c, 0x8c, 0x8c, 0x8c, 0x30, 0x30, 0x30, 0x30, 0x00, 0x30, 0x30, 
		0x00, 0x9d, 0x00, 0xb2, 0x00, 0x51, 0x00, 0x5e, 0x00, 0x58, 0xc7, 0xb3, 0x00, 0xc8, 0x00, 0x51, 
		0x00, 0x5e, 0x00, 0x5c, 0xc7, 0xc9, 0x00, 0xde, 0x00, 0x51, 0x00, 0x5e, 0x00, 0x60, 0xc7, 0x9d, 
		0x00, 0xb2, 0x00, 0x5f, 0x00, 0x70, 0x00, 0x64, 0xc7, 0xb3, 0x00, 0xc8, 0x00, 0x5f, 0x00, 0x70, 
		0x00, 0x68, 0xc7, 0xc9, 0x00, 0xde, 0x00, 0x5f, 0x00, 0x70, 0x00, 0x6c, 0xc7, 0x9d, 0x00, 0xb2, 
		0x00, 0x71, 0x00, 0x82, 0x00, 0x70, 0xc7, 0xb3, 0x00, 0xc8, 0x00, 0x71, 0x00, 0x82, 0x00, 0x74, 
		0xc7, 0xc9, 0x00, 0xde, 0x00, 0x71, 0x00, 0x82, 0x00, 0x78, 0xc7, 0x9d, 0x00, 0xb2, 0x00, 0x83, 
		0x00, 0x91, 0x00, 0x7c, 0xc7, 0xb3, 0x00, 0xde, 0x00, 0x83, 0x00, 0x91, 0x00, 0x80, 0xc7, 0xdc, 
		0x00, 0xea, 0x00, 0x98, 0x00, 0xa6, 0x00, 0x50, 0xc7, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 
		0x00, 0xa0, 0xca, 0xff, 0xff, 0xae, 0x00, 0xbc, 0x00, 0x84, 0x00, 0x94, 0x00, 0x50, 0xc7, 0x00, 
		0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x18, 0x01, 0x40, 0x01, 0xa0, 
		0x00, 0xc8, 0x00, 0x50, 0xc7, 0x8f, 0x00, 0x2c, 0x01, 0x06, 0x00, 0xc2, 0x00, 0xb8, 0xc7, 0x00, 
		0x00, 0x8f, 0x00, 0x06, 0x00, 0xc2, 0x00, 0xc0, 0xc7, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 
		0x00, 0xa0, 0xca, 0xff, 0xff, 0x68, 0x00, 0x80, 0x00, 0x3a, 0x00, 0x48, 0x00, 0xdc, 0xc7, 0x40, 
		0x00, 0x74, 0x00, 0x4c, 0x00, 0x6a, 0x00, 0xe0, 0xc7, 0x74, 0x00, 0xa8, 0x00, 0x4c, 0x00, 0x6a, 
		0x00, 0xe4, 0xc7, 0x40, 0x00, 0x74, 0x00, 0x6a, 0x00, 0x88, 0x00, 0xe8, 0xc7, 0x74, 0x00, 0xa8, 
		0x00, 0x6a, 0x00, 0x88, 0x00, 0xec, 0xc7, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 
		0xca, 0xff, 0xff, 0xba, 0x00, 0xca, 0x00, 0x9d, 0x00, 0xad, 0x00, 0x1c, 0xc8, 0xf3, 0x00, 0x03, 
		0x01, 0x83, 0x00, 0x93, 0x00, 0x18, 0xc8, 0x0c, 0x01, 0x1c, 0x01, 0xa8, 0x00, 0xb8, 0x00, 0x50, 
		0xc7, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x77, 0x00, 0xae, 
		0x00, 0x52, 0x00, 0x80, 0x00, 0x34, 0xc8, 0x46, 0x00, 0x89, 0x00, 0x3e, 0x00, 0x6f, 0x00, 0x80, 
		0xc8, 0xbc, 0x00, 0xfa, 0x00, 0x44, 0x00, 0x98, 0x00, 0x4c, 0xc8, 0x00, 0x00, 0x40, 0x01, 0x00, 
		0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0xec, 0x00, 0xfc, 0x00, 0x70, 0x00, 0x80, 0x00, 0x48, 
		0xc8, 0xbc, 0x00, 0xfa, 0x00, 0x40, 0x00, 0x98, 0x00, 0x58, 0xc8, 0x3e, 0x00, 0x98, 0x00, 0x38, 
		0x00, 0x85, 0x00, 0x74, 0xc8, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 
		0xff, 0x77, 0x00, 0xae, 0x00, 0x52, 0x00, 0x80, 0x00, 0x44, 0xc8, 0x46, 0x00, 0x8b, 0x00, 0x3e, 
		0x00, 0x6f, 0x00, 0x50, 0xc8, 0xec, 0x00, 0xfc, 0x00, 0x70, 0x00, 0x80, 0x00, 0x48, 0xc8, 0x00, 
		0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0xec, 0x00, 0xfc, 0x00, 0x70, 
		0x00, 0x80, 0x00, 0x48, 0xc8, 0xbc, 0x00, 0xfa, 0x00, 0x40, 0x00, 0x98, 0x00, 0x54, 0xc8, 0x3e, 
		0x00, 0x98, 0x00, 0x38, 0x00, 0x85, 0x00, 0x74, 0xc8, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 
		0x00, 0xa0, 0xca, 0xff, 0xff, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 
		0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 
		0x0a, 0x55, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x74, 0x6f, 0x20, 0x61, 0x6c, 0x6c, 0x6f, 0x63, 
		0x61, 0x74, 0x65, 0x20, 0x45, 0x78, 0x70, 0x61, 0x6e, 0x64, 0x65, 0x64, 0x20, 0x4d, 0x65, 0x6d, 
		0x6f, 0x72, 0x79, 0x2e, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 0x72, 0x65, 
		0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 0x72, 0x72, 
		0x6f, 0x72, 0x3a, 0x0d, 0x0a, 0x53, 0x6f, 0x75, 0x6e, 0x64, 0x20, 0x42, 0x6c, 0x61, 0x73, 0x74, 
		0x65, 0x72, 0x20, 0x63, 0x61, 0x72, 0x64, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x66, 0x6f, 0x75, 0x6e, 
		0x64, 0x20, 0x61, 0x74, 0x20, 0x61, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x20, 0x32, 0x32, 0x30, 
		0x20, 0x48, 0x65, 0x78, 0x2e, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 0x72, 
		0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 0x72, 
		0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 0x4f, 0x75, 0x74, 0x20, 0x6f, 0x66, 0x20, 0x42, 0x61, 0x73, 
		0x65, 0x20, 0x4d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x2e, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 
		0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 
		0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 0x4d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 
		0x20, 0x44, 0x65, 0x61, 0x6c, 0x6c, 0x6f, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x70, 0x72, 
		0x6f, 0x62, 0x6c, 0x65, 0x6d, 0x2e, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 
		0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 
		0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 0x41, 0x74, 0x20, 0x6c, 0x65, 0x61, 0x73, 0x74, 0x20, 
		0x35, 0x39, 0x30, 0x4b, 0x20, 0x6f, 0x66, 0x20, 0x62, 0x61, 0x73, 0x65, 0x20, 0x6d, 0x65, 0x6d, 
		0x6f, 0x72, 0x79, 0x20, 0x69, 0x73, 0x20, 0x72, 0x65, 0x71, 0x75, 0x69, 0x72, 0x65, 0x64, 0x2e, 
		0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 
		0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 
		0x0a, 0x53, 0x6f, 0x75, 0x6e, 0x64, 0x20, 0x42, 0x6c, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x6e, 
		0x6f, 0x74, 0x20, 0x66, 0x6f, 0x75, 0x6e, 0x64, 0x20, 0x6f, 0x6e, 0x20, 0x69, 0x6e, 0x74, 0x65, 
		0x72, 0x75, 0x70, 0x74, 0x20, 0x30, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 
		0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 
		0x72, 0x72, 0x6f, 0x72, 0x3a, 0x0d, 0x0a, 0x55, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x74, 0x6f, 
		0x20, 0x73, 0x65, 0x6c, 0x65, 0x63, 0x74, 0x20, 0x45, 0x4d, 0x4d, 0x20, 0x70, 0x61, 0x67, 0x65, 
		0x2e, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 
		0x65, 0x62, 0x20, 0x68, 0x61, 0x73, 0x20, 0x61, 0x6e, 0x20, 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 
		0x0d, 0x0a, 0x46, 0x69, 0x6c, 0x65, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x66, 0x6f, 0x75, 0x6e, 0x64, 
		0x2e, 0x63, 0x0d, 0x0a, 0x0d, 0x0a, 0x24, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 
		0x6c, 0x6f, 0x6f, 0x6b, 0x73, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x53, 0x6f, 0x75, 0x6e, 0x64, 0x20, 
		0x42, 0x6c, 0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x69, 0x6e, 0x66, 0x6f, 0x72, 0x6d, 0x61, 0x74, 
		0x69, 0x6f, 0x6e, 0x20, 0x69, 0x6e, 0x0d, 0x0a, 0x74, 0x68, 0x65, 0x20, 0x42, 0x4c, 0x41, 0x53, 
		0x54, 0x45, 0x52, 0x20, 0x65, 0x6e, 0x76, 0x69, 0x72, 0x6f, 0x6e, 0x6d, 0x65, 0x6e, 0x74, 0x20, 
		0x76, 0x61, 0x72, 0x69, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x28, 0x69, 0x6e, 0x20, 0x79, 0x6f, 0x75, 
		0x72, 0x20, 0x41, 0x55, 0x54, 0x4f, 0x45, 0x58, 0x45, 0x43, 0x2e, 0x42, 0x41, 0x54, 0x29, 0x0d, 
		0x0a, 0x0d, 0x0a, 0x49, 0x66, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x6e, 0x6f, 
		0x74, 0x20, 0x66, 0x6f, 0x75, 0x6e, 0x64, 0x20, 0x74, 0x68, 0x65, 0x6e, 0x20, 0x49, 0x52, 0x51, 
		0x20, 0x37, 0x2c, 0x20, 0x44, 0x4d, 0x41, 0x20, 0x63, 0x68, 0x61, 0x6e, 0x6e, 0x65, 0x6c, 0x20, 
		0x31, 0x20, 0x61, 0x6e, 0x64, 0x20, 0x62, 0x61, 0x73, 0x65, 0x0d, 0x0a, 0x61, 0x64, 0x64, 0x72, 
		0x65, 0x73, 0x73, 0x20, 0x32, 0x32, 0x30, 0x68, 0x20, 0x61, 0x72, 0x65, 0x20, 0x61, 0x73, 0x73, 
		0x75, 0x6d, 0x65, 0x64, 0x2e, 0x0d, 0x0a, 0x0d, 0x0a, 0x54, 0x6f, 0x20, 0x61, 0x6c, 0x74, 0x65, 
		0x72, 0x20, 0x61, 0x6e, 0x79, 0x20, 0x6f, 0x72, 0x20, 0x61, 0x6c, 0x6c, 0x20, 0x6f, 0x66, 0x20, 
		0x74, 0x68, 0x65, 0x73, 0x65, 0x20, 0x73, 0x65, 0x74, 0x74, 0x69, 0x6e, 0x67, 0x73, 0x20, 0x79, 
		0x6f, 0x75, 0x20, 0x63, 0x61, 0x6e, 0x20, 0x73, 0x70, 0x65, 0x63, 0x69, 0x66, 0x79, 0x20, 0x74, 
		0x68, 0x65, 0x6d, 0x0d, 0x0a, 0x6f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6f, 0x6d, 0x6d, 
		0x61, 0x6e, 0x64, 0x20, 0x6c, 0x69, 0x6e, 0x65, 0x2e, 0x20, 0x46, 0x6f, 0x72, 0x20, 0x65, 0x78, 
		0x61, 0x6d, 0x70, 0x6c, 0x65, 0x3a, 0x0d, 0x0a, 0x0d, 0x0a, 0x54, 0x79, 0x70, 0x65, 0x20, 0x44, 
		0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x20, 0x49, 0x37, 0x20, 0x41, 0x32, 0x32, 0x30, 0x20, 
		0x44, 0x31, 0x20, 0x74, 0x6f, 0x20, 0x72, 0x75, 0x6e, 0x20, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 
		0x65, 0x62, 0x20, 0x6f, 0x6e, 0x20, 0x49, 0x52, 0x51, 0x20, 0x37, 0x2c, 0x20, 0x44, 0x4d, 0x41, 
		0x0d, 0x0a, 0x20, 0x63, 0x68, 0x61, 0x6e, 0x6e, 0x65, 0x6c, 0x20, 0x31, 0x20, 0x61, 0x6e, 0x64, 
		0x20, 0x62, 0x61, 0x73, 0x65, 0x20, 0x61, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x20, 0x32, 0x32, 
		0x30, 0x68, 0x0d, 0x0a, 0x20, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x20, 0x49, 0x35, 
		0x20, 0x74, 0x6f, 0x20, 0x72, 0x75, 0x6e, 0x20, 0x44, 0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 
		0x20, 0x6f, 0x6e, 0x20, 0x49, 0x52, 0x51, 0x20, 0x35, 0x20, 0x61, 0x6e, 0x64, 0x0d, 0x0a, 0x20, 
		0x64, 0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, 0x20, 0x61, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x20, 
		0x6f, 0x66, 0x20, 0x32, 0x32, 0x30, 0x68, 0x2c, 0x20, 0x44, 0x4d, 0x41, 0x20, 0x31, 0x0d, 0x0a, 
		0x0d, 0x0a, 0x24, 0x0d, 0x0a, 0x0d, 0x0a, 0x54, 0x72, 0x79, 0x20, 0x74, 0x68, 0x65, 0x20, 0x44, 
		0x72, 0x65, 0x61, 0x6d, 0x77, 0x65, 0x62, 0x20, 0x43, 0x44, 0x20, 0x69, 0x6e, 0x20, 0x79, 0x6f, 
		0x75, 0x72, 0x20, 0x73, 0x74, 0x65, 0x72, 0x65, 0x6f, 0x2e, 0x2e, 0x2e, 0x2e, 0x0d, 0x0a, 0x0d, 
		0x0a, 0x0d, 0x0a, 0x24, 0x81, 0x00, 0xb8, 0x00, 0x52, 0x00, 0x80, 0x00, 0xc0, 0xc8, 0x50, 0x00, 
		0x93, 0x00, 0x3e, 0x00, 0x6f, 0x00, 0x80, 0xc8, 0xb7, 0x00, 0xfa, 0x00, 0x3e, 0x00, 0x6f, 0x00, 
		0xc4, 0xc8, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xa0, 0xca, 0xff, 0xff, 0x53, 0x50, 
		0x45, 0x45, 0x43, 0x48, 0x52, 0x32, 0x34, 0x43, 0x30, 0x30, 0x30, 0x35, 0x2e, 0x52, 0x41, 0x57, 
		0x00, 0x87, 0x83, 0x81, 0x82, 0x2c, 0x00, 0x46, 0x00, 0x20, 0x00, 0x2e, 0x00, 0x70, 0xc4, 0x00, 
		0x00, 0x32, 0x00, 0x00, 0x00, 0xb4, 0x00, 0x7c, 0xc3, 0xe2, 0x00, 0xf4, 0x00, 0x0a, 0x00, 0x1a, 
		0x00, 0x28, 0xc8, 0xe2, 0x00, 0xf4, 0x00, 0x1a, 0x00, 0x28, 0x00, 0x2c, 0xc8, 0xf0, 0x00, 0x04, 
		0x01, 0x64, 0x00, 0x7c, 0x00, 0xcc, 0xc9, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xd4, 
		0xc9, 0xff, 0xff, 0x2c, 0x00, 0x46, 0x00, 0x20, 0x00, 0x2e, 0x00, 0x70, 0xc4, 0x00, 0x00, 0x32, 
		0x00, 0x00, 0x00, 0xb4, 0x00, 0x7c, 0xc3, 0x12, 0x01, 0x24, 0x01, 0x0a, 0x00, 0x1a, 0x00, 0x28, 
		0xc8, 0x12, 0x01, 0x24, 0x01, 0x1a, 0x00, 0x28, 0x00, 0x2c, 0xc8, 0xf0, 0x00, 0x04, 0x01, 0x64, 
		0x00, 0x7c, 0x00, 0xcc, 0xc9, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xc8, 0x00, 0xd4, 0xc9, 0xff, 
		0xff, 0x00, 0x21, 0x0a, 0x0f, 0xff, 0x00, 0x16, 0x0a, 0x0f, 0xff, 0x00, 0x16, 0x00, 0x0f, 0xff, 
		0x00, 0x0b, 0x00, 0x0f, 0xff, 0x00, 0x0b, 0x0a, 0x0f, 0xff, 0x00, 0x00, 0x0a, 0x0f, 0xff, 0x01, 
		0x2c, 0x0a, 0x06, 0xff, 0x01, 0x2c, 0x00, 0x0d, 0xff, 0x02, 0x21, 0x00, 0x06, 0xff, 0x02, 0x16, 
		0x00, 0x05, 0xff, 0x02, 0x16, 0x0a, 0x10, 0xff, 0x02, 0x0b, 0x0a, 0x10, 0xff, 0x03, 0x2c, 0x00, 
		0x0f, 0xff, 0x03, 0x21, 0x0a, 0x06, 0xff, 0x03, 0x21, 0x00, 0x05, 0xff, 0x04, 0x0b, 0x1e, 0x06, 
		0xff, 0x04, 0x16, 0x1e, 0x05, 0xff, 0x04, 0x16, 0x14, 0x0d, 0xff, 0x0a, 0x21, 0x1e, 0x06, 0xff, 
		0x0a, 0x16, 0x1e, 0x06, 0xff, 0x09, 0x16, 0x0a, 0x06, 0xff, 0x09, 0x16, 0x14, 0x10, 0xff, 0x09, 
		0x16, 0x1e, 0x10, 0xff, 0x09, 0x16, 0x28, 0x10, 0xff, 0x09, 0x16, 0x32, 0x10, 0xff, 0x06, 0x0b, 
		0x1e, 0x06, 0xff, 0x06, 0x00, 0x0a, 0x0f, 0xff, 0x06, 0x00, 0x14, 0x0f, 0xff, 0x06, 0x0b, 0x14, 
		0x0f, 0xff, 0x06, 0x16, 0x14, 0x0f, 0xff, 0x07, 0x0b, 0x14, 0x06, 0xff, 0x07, 0x00, 0x14, 0x06, 
		0xff, 0x07, 0x00, 0x1e, 0x06, 0xff, 0x37, 0x2c, 0x00, 0x05, 0xff, 0x37, 0x2c, 0x0a, 0x05, 0xff, 
		0x05, 0x16, 0x1e, 0x06, 0xff, 0x05, 0x16, 0x14, 0x0f, 0xff, 0x05, 0x16, 0x0a, 0x0f, 0xff, 0x18, 
		0x16, 0x00, 0x0f, 0xff, 0x18, 0x21, 0x00, 0x0f, 0xff, 0x18, 0x2c, 0x00, 0x0f, 0xff, 0x18, 0x21, 
		0x0a, 0x0f, 0xff, 0x08, 0x00, 0x0a, 0x06, 0xff, 0x08, 0x0b, 0x0a, 0x06, 0xff, 0x08, 0x16, 0x0a, 
		0x06, 0xff, 0x08, 0x21, 0x0a, 0x06, 0xff, 0x08, 0x21, 0x14, 0x06, 0xff, 0x08, 0x21, 0x1e, 0x06, 
		0xff, 0x08, 0x21, 0x28, 0x06, 0xff, 0x08, 0x16, 0x28, 0x06, 0xff, 0x08, 0x0b, 0x28, 0x06, 0xff, 
		0x0b, 0x0b, 0x14, 0x0c, 0xff, 0x0b, 0x0b, 0x1e, 0x0c, 0xff, 0x0b, 0x16, 0x14, 0x0c, 0xff, 0x0b, 
		0x16, 0x1e, 0x0c, 0xff, 0x0c, 0x16, 0x14, 0x0c, 0xff, 0x0d, 0x16, 0x14, 0x0c, 0xff, 0x0d, 0x21, 
		0x14, 0x0c, 0xff, 0x0e, 0x2c, 0x14, 0x0c, 0xff, 0x0e, 0x21, 0x00, 0x0c, 0xff, 0x0e, 0x21, 0x0a, 
		0x0c, 0xff, 0x0e, 0x21, 0x14, 0x0c, 0xff, 0x0e, 0x21, 0x1e, 0x0c, 0xff, 0x0e, 0x21, 0x28, 0x0c, 
		0xff, 0x0e, 0x16, 0x00, 0x10, 0xff, 0x13, 0x00, 0x00, 0x0c, 0xff, 0x14, 0x00, 0x14, 0x10, 0xff, 
		0x14, 0x00, 0x1e, 0x10, 0xff, 0x14, 0x0b, 0x1e, 0x10, 0xff, 0x14, 0x00, 0x28, 0x10, 0xff, 0x14, 
		0x0b, 0x28, 0x10, 0xff, 0x15, 0x0b, 0x0a, 0x0f, 0xff, 0x15, 0x0b, 0x14, 0x0f, 0xff, 0x15, 0x00, 
		0x14, 0x0f, 0xff, 0x15, 0x16, 0x14, 0x0f, 0xff, 0x15, 0x21, 0x14, 0x0f, 0xff, 0x15, 0x2c, 0x14, 
		0x0f, 0xff, 0x15, 0x2c, 0x0a, 0x0f, 0xff, 0x16, 0x16, 0x0a, 0x10, 0xff, 0x16, 0x16, 0x14, 0x10, 
		0xff, 0x17, 0x16, 0x1e, 0x0d, 0xff, 0x17, 0x16, 0x28, 0x0d, 0xff, 0x17, 0x21, 0x28, 0x0d, 0xff, 
		0x17, 0x0b, 0x28, 0x0d, 0xff, 0x17, 0x00, 0x28, 0x0d, 0xff, 0x17, 0x00, 0x32, 0x0d, 0xff, 0x19, 
		0x0b, 0x28, 0x10, 0xff, 0x19, 0x0b, 0x32, 0x10, 0xff, 0x19, 0x00, 0x32, 0x10, 0xff, 0x1b, 0x0b, 
		0x14, 0x10, 0xff, 0x1b, 0x0b, 0x1e, 0x10, 0xff, 0x1d, 0x0b, 0x0a, 0x10, 0xff, 0x2d, 0x16, 0x1e, 
		0x0c, 0xff, 0x2d, 0x16, 0x28, 0x0c, 0xff, 0x2d, 0x16, 0x32, 0x0c, 0xff, 0x2e, 0x16, 0x28, 0x0c, 
		0xff, 0x2e, 0x0b, 0x32, 0x0c, 0xff, 0x2e, 0x16, 0x32, 0x0c, 0xff, 0x2e, 0x21, 0x32, 0x0c, 0xff, 
		0x2f, 0x00, 0x00, 0x0c, 0xff, 0x1a, 0x16, 0x14, 0x10, 0xff, 0x1a, 0x21, 0x0a, 0x10, 0xff, 0x1a, 
		0x21, 0x14, 0x10, 0xff, 0x1a, 0x21, 0x1e, 0x10, 0xff, 0x1a, 0x2c, 0x1e, 0x10, 0xff, 0x1a, 0x16, 
		0x1e, 0x10, 0xff, 0x1a, 0x0b, 0x1e, 0x10, 0xff, 0x1a, 0x0b, 0x14, 0x10, 0xff, 0x1a, 0x00, 0x14, 
		0x10, 0xff, 0x1a, 0x0b, 0x28, 0x10, 0xff, 0x1a, 0x00, 0x28, 0x10, 0xff, 0x1a, 0x16, 0x28, 0x10, 
		0xff, 0x1a, 0x0b, 0x32, 0x10, 0xff, 0x1c, 0x00, 0x1e, 0x0f, 0xff, 0x1c, 0x00, 0x14, 0x0f, 0xff, 
		0x1c, 0x00, 0x28, 0x0f, 0xff, 0x1c, 0x0b, 0x1e, 0x0f, 0xff, 0x1c, 0x0b, 0x14, 0x0f, 0xff, 0x1c, 
		0x16, 0x1e, 0x0f, 0xff, 0x1c, 0x16, 0x14, 0x0f, 0xff, 0xff, 0x4f, 0x42, 0x4a, 0x45, 0x43, 0x54, 
		0x20, 0x4e, 0x41, 0x4d, 0x45, 0x20, 0x4f, 0x4e, 0x45, 0x20, 0x00, 0x10, 0x12, 0x12, 0x11, 0x10, 
		0x10, 0x10, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 
		0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x2d, 
		0x00, 0x08, 0x00, 0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x00, 0x00, 0x0d, 
		0x00, 0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 
		0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x20, 0x44, 
		0x41, 0x54, 0x41, 0x20, 0x46, 0x49, 0x4c, 0x45, 0x20, 0x43, 0x4f, 0x50, 0x59, 0x52, 0x49, 0x47, 
		0x48, 0x54, 0x20, 0x31, 0x39, 0x39, 0x32, 0x20, 0x43, 0x52, 0x45, 0x41, 0x54, 0x49, 0x56, 0x45, 
		0x20, 0x52, 0x45, 0x41, 0x4c, 0x49, 0x54, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x30, 0x00, 0x05, 0xff, 0x21, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x01, 0x06, 0x02, 0xff, 0x03, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x31, 0x00, 0x01, 0xff, 0x2c, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x07, 0x02, 0xff, 0xff, 0xff, 
		0xff, 0x06, 0xff, 0xff, 0xff, 0x01, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x32, 0x00, 0x02, 0xff, 0x21, 0x00, 0xff, 0xff, 0xff, 0x00, 0x01, 0x00, 0xff, 0xff, 0x01, 
		0xff, 0x03, 0xff, 0xff, 0xff, 0x02, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x33, 0x00, 0x05, 0xff, 0x21, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x02, 0x02, 0x00, 0x02, 0x04, 
		0xff, 0x00, 0xff, 0xff, 0xff, 0x03, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x34, 0x00, 0x17, 0xff, 0x0b, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x01, 0x04, 0x00, 0x05, 0xff, 
		0xff, 0x03, 0xff, 0xff, 0xff, 0x04, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x35, 0x00, 0x05, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x01, 0x02, 0x00, 0x04, 0xff, 
		0xff, 0x03, 0xff, 0xff, 0xff, 0x05, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x36, 0x00, 0x05, 0xff, 0x0b, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x01, 0x00, 0x00, 0x01, 0x02, 
		0xff, 0x00, 0xff, 0xff, 0xff, 0x06, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x37, 0x00, 0xff, 0xff, 0x00, 0x14, 0xff, 0xff, 0xff, 0x00, 0x02, 0x02, 0xff, 0xff, 0xff, 
		0xff, 0x00, 0xff, 0xff, 0xff, 0x07, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x38, 0x00, 0x08, 0xff, 0x00, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x01, 0x02, 0xff, 0xff, 0xff, 
		0xff, 0x00, 0x0b, 0x28, 0x00, 0x08, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x39, 0x00, 0x09, 0xff, 0x16, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x04, 0x06, 0xff, 0xff, 0xff, 
		0xff, 0x00, 0xff, 0xff, 0xff, 0x09, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x31, 0x30, 0x00, 0x0a, 0xff, 0x21, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x02, 0x00, 0xff, 0xff, 0x02, 
		0x02, 0x04, 0x16, 0x1e, 0xff, 0x0a, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x31, 0x31, 0x00, 0x0b, 0xff, 0x0b, 0x14, 0xff, 0xff, 0xff, 0x00, 0x00, 0x04, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x0b, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x31, 0x32, 0x00, 0x0c, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 0x01, 0x04, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x0c, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x31, 0x33, 0x00, 0x0c, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 0x01, 0x04, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x0d, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x31, 0x34, 0x00, 0x0e, 0xff, 0x2c, 0x14, 0xff, 0xff, 0xff, 0x00, 0x00, 0x06, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x31, 0x39, 0x00, 0x13, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x04, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x13, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x30, 0x00, 0x16, 0xff, 0x00, 0x14, 0xff, 0xff, 0xff, 0x00, 0x01, 0x04, 0x02, 0x0f, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x14, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x31, 0x00, 0x05, 0xff, 0x0b, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x01, 0x04, 0x02, 0x0f, 0x01, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x15, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x32, 0x00, 0x16, 0xff, 0x16, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x00, 0x04, 0xff, 0xff, 0x01, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x16, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x33, 0x00, 0x17, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x01, 0x04, 0x02, 0x0f, 0x03, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x34, 0x00, 0x05, 0xff, 0x2c, 0x00, 0xff, 0xff, 0xff, 0x00, 0x01, 0x06, 0x02, 0x0f, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x18, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x35, 0x00, 0x16, 0xff, 0x0b, 0x28, 0xff, 0xff, 0xff, 0x00, 0x01, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x19, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x36, 0x00, 0x09, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 0x04, 0x02, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x37, 0x00, 0x16, 0xff, 0x0b, 0x14, 0xff, 0xff, 0xff, 0x00, 0x00, 0x06, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x1b, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x38, 0x00, 0x05, 0xff, 0x0b, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x02, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x1c, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x39, 0x00, 0x16, 0xff, 0x0b, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x00, 0x02, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x1d, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x35, 0x00, 0x05, 0xff, 0x16, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x01, 0x04, 0x01, 0x0f, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x05, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x34, 0x00, 0x17, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 0x01, 0x04, 0x02, 0x0f, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x04, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x31, 0x30, 0x00, 0x0a, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x03, 0x06, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x31, 0x32, 0x00, 0x0c, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 0x00, 0x02, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x0c, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x33, 0x00, 0x05, 0xff, 0x2c, 0x00, 0xff, 0xff, 0xff, 0x00, 0x01, 0x06, 0x02, 0xff, 0x04, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x34, 0x00, 0x05, 0xff, 0x16, 0x00, 0xff, 0xff, 0xff, 0x00, 0x03, 0x06, 0x00, 0xff, 0xff, 
		0xff, 0xff, 0x21, 0x00, 0x03, 0x18, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x32, 0x00, 0x16, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 0x01, 0x04, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x16, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x32, 0x00, 0x16, 0xff, 0x16, 0x14, 0xff, 0xff, 0xff, 0x00, 0x00, 0x02, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x16, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x31, 0x31, 0x00, 0x0b, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x0b, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x38, 0x00, 0x05, 0xff, 0x0b, 0x14, 0xff, 0xff, 0xff, 0x00, 0x00, 0x06, 0xff, 0xff, 0x02, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x1c, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x31, 0x00, 0x05, 0xff, 0x0b, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x01, 0x04, 0x02, 0x0f, 0x01, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x15, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x32, 0x36, 0x00, 0x09, 0xff, 0x00, 0x28, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x31, 0x39, 0x00, 0x13, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x02, 0x02, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x13, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x38, 0x00, 0x08, 0xff, 0x0b, 0x28, 0xff, 0xff, 0xff, 0x00, 0x00, 0x04, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x08, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x30, 0x31, 0x00, 0x01, 0xff, 0x2c, 0x0a, 0xff, 0xff, 0xff, 0x00, 0x03, 0x06, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x34, 0x35, 0x00, 0x23, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x00, 0x06, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x2d, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x34, 0x36, 0x00, 0x23, 0xff, 0x16, 0x28, 0xff, 0xff, 0xff, 0x00, 0x00, 0x04, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x2e, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x34, 0x37, 0x00, 0x23, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x2f, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x34, 0x35, 0x00, 0x23, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x04, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x2d, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x34, 0x36, 0x00, 0x23, 0xff, 0x16, 0x32, 0xff, 0xff, 0xff, 0x00, 0x00, 0x04, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x2e, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x35, 0x30, 0x00, 0x23, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x32, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x35, 0x31, 0x00, 0x23, 0xff, 0x0b, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x33, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x35, 0x32, 0x00, 0x23, 0xff, 0x16, 0x1e, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x34, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x35, 0x33, 0x00, 0x23, 0xff, 0x21, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x35, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x35, 0x34, 0x00, 0x23, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x36, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x52, 
		0x35, 0x35, 0x00, 0x0e, 0xff, 0x2c, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x02, 0x04, 0x01, 0x0a, 0x09, 0x08, 0x06, 
		0x0b, 0x04, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x02, 0x00, 
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 
		0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x01, 0x01, 0x01, 0x01, 0x01, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 0x30, 
		0x30, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 0x30, 0x31, 0x00, 0x44, 
		0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 0x30, 0x32, 0x00, 0x44, 0x52, 0x45, 0x41, 
		0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 0x30, 0x33, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 
		0x42, 0x2e, 0x44, 0x30, 0x34, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 
		0x30, 0x35, 0x00, 0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 0x30, 0x36, 0x00, 
		0x44, 0x52, 0x45, 0x41, 0x4d, 0x57, 0x45, 0x42, 0x2e, 0x44, 0x45, 0x4d, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
	context.ds.assign(src, src + sizeof(src));
dreamweb(context); 
}

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
		case 0xc224: width160(context); break;
		case 0xc228: doblocks(context); break;
		case 0xc22c: showframe(context); break;
		case 0xc230: frameoutv(context); break;
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
		case 0xc320: fillryan(context); break;
		case 0xc324: fillopen(context); break;
		case 0xc328: findallryan(context); break;
		case 0xc32c: findallopen(context); break;
		case 0xc330: obtoinv(context); break;
		case 0xc334: isitworn(context); break;
		case 0xc338: makeworn(context); break;
		case 0xc33c: examineob(context); break;
		case 0xc340: makemainscreen(context); break;
		case 0xc344: getbackfromob(context); break;
		case 0xc348: incryanpage(context); break;
		case 0xc34c: openinv(context); break;
		case 0xc350: showryanpage(context); break;
		case 0xc354: openob(context); break;
		case 0xc358: obicons(context); break;
		case 0xc35c: examicon(context); break;
		case 0xc360: obpicture(context); break;
		case 0xc364: describeob(context); break;
		case 0xc368: additionaltext(context); break;
		case 0xc36c: obsthatdothings(context); break;
		case 0xc370: getobtextstart(context); break;
		case 0xc374: searchforsame(context); break;
		case 0xc378: findnextcolon(context); break;
		case 0xc37c: inventory(context); break;
		case 0xc380: setpickup(context); break;
		case 0xc384: examinventory(context); break;
		case 0xc388: reexfrominv(context); break;
		case 0xc38c: reexfromopen(context); break;
		case 0xc390: swapwithinv(context); break;
		case 0xc394: swapwithopen(context); break;
		case 0xc398: intoinv(context); break;
		case 0xc39c: deletetaken(context); break;
		case 0xc3a0: outofinv(context); break;
		case 0xc3a4: getfreead(context); break;
		case 0xc3a8: getexad(context); break;
		case 0xc3ac: geteitherad(context); break;
		case 0xc3b0: getanyad(context); break;
		case 0xc3b4: getanyaddir(context); break;
		case 0xc3b8: getopenedsize(context); break;
		case 0xc3bc: getsetad(context); break;
		case 0xc3c0: findinvpos(context); break;
		case 0xc3c4: findopenpos(context); break;
		case 0xc3c8: dropobject(context); break;
		case 0xc3cc: droperror(context); break;
		case 0xc3d0: cantdrop(context); break;
		case 0xc3d4: wornerror(context); break;
		case 0xc3d8: removeobfrominv(context); break;
		case 0xc3dc: selectopenob(context); break;
		case 0xc3e0: useopened(context); break;
		case 0xc3e4: errormessage1(context); break;
		case 0xc3e8: errormessage2(context); break;
		case 0xc3ec: errormessage3(context); break;
		case 0xc3f0: checkobjectsize(context); break;
		case 0xc3f4: outofopen(context); break;
		case 0xc3f8: transfertoex(context); break;
		case 0xc3fc: pickupconts(context); break;
		case 0xc400: transfercontoex(context); break;
		case 0xc404: transfertext(context); break;
		case 0xc408: getexpos(context); break;
		case 0xc40c: purgealocation(context); break;
		case 0xc410: emergencypurge(context); break;
		case 0xc414: purgeanitem(context); break;
		case 0xc418: deleteexobject(context); break;
		case 0xc41c: deleteexframe(context); break;
		case 0xc420: deleteextext(context); break;
		case 0xc424: blockget(context); break;
		case 0xc428: drawfloor(context); break;
		case 0xc42c: calcmapad(context); break;
		case 0xc430: getdimension(context); break;
		case 0xc434: addalong(context); break;
		case 0xc438: addlength(context); break;
		case 0xc43c: drawflags(context); break;
		case 0xc440: eraseoldobs(context); break;
		case 0xc444: showallobs(context); break;
		case 0xc448: makebackob(context); break;
		case 0xc44c: showallfree(context); break;
		case 0xc450: showallex(context); break;
		case 0xc454: calcfrframe(context); break;
		case 0xc458: finalframe(context); break;
		case 0xc45c: adjustlen(context); break;
		case 0xc460: getmapad(context); break;
		case 0xc464: getxad(context); break;
		case 0xc468: getyad(context); break;
		case 0xc46c: autolook(context); break;
		case 0xc470: look(context); break;
		case 0xc474: dolook(context); break;
		case 0xc478: redrawmainscrn(context); break;
		case 0xc47c: getback1(context); break;
		case 0xc480: talk(context); break;
		case 0xc484: convicons(context); break;
		case 0xc488: getpersframe(context); break;
		case 0xc48c: starttalk(context); break;
		case 0xc490: getpersontext(context); break;
		case 0xc494: moretalk(context); break;
		case 0xc498: dosometalk(context); break;
		case 0xc49c: hangonpq(context); break;
		case 0xc4a0: redes(context); break;
		case 0xc4a4: newplace(context); break;
		case 0xc4a8: selectlocation(context); break;
		case 0xc4ac: showcity(context); break;
		case 0xc4b0: lookatplace(context); break;
		case 0xc4b4: getundercentre(context); break;
		case 0xc4b8: putundercentre(context); break;
		case 0xc4bc: locationpic(context); break;
		case 0xc4c0: getdestinfo(context); break;
		case 0xc4c4: showarrows(context); break;
		case 0xc4c8: nextdest(context); break;
		case 0xc4cc: lastdest(context); break;
		case 0xc4d0: destselect(context); break;
		case 0xc4d4: getlocation(context); break;
		case 0xc4d8: setlocation(context); break;
		case 0xc4dc: resetlocation(context); break;
		case 0xc4e0: readdesticon(context); break;
		case 0xc4e4: readcitypic(context); break;
		case 0xc4e8: usemon(context); break;
		case 0xc4ec: printoutermon(context); break;
		case 0xc4f0: loadpersonal(context); break;
		case 0xc4f4: loadnews(context); break;
		case 0xc4f8: loadcart(context); break;
		case 0xc4fc: lookininterface(context); break;
		case 0xc500: turnonpower(context); break;
		case 0xc504: randomaccess(context); break;
		case 0xc508: powerlighton(context); break;
		case 0xc50c: powerlightoff(context); break;
		case 0xc510: accesslighton(context); break;
		case 0xc514: accesslightoff(context); break;
		case 0xc518: locklighton(context); break;
		case 0xc51c: locklightoff(context); break;
		case 0xc520: input(context); break;
		case 0xc524: makecaps(context); break;
		case 0xc528: delchar(context); break;
		case 0xc52c: execcommand(context); break;
		case 0xc530: neterror(context); break;
		case 0xc534: dircom(context); break;
		case 0xc538: searchforfiles(context); break;
		case 0xc53c: signon(context); break;
		case 0xc540: showkeys(context); break;
		case 0xc544: read(context); break;
		case 0xc548: dirfile(context); break;
		case 0xc54c: getkeyandlogo(context); break;
		case 0xc550: searchforstring(context); break;
		case 0xc554: parser(context); break;
		case 0xc558: scrollmonitor(context); break;
		case 0xc55c: lockmon(context); break;
		case 0xc560: monitorlogo(context); break;
		case 0xc564: printlogo(context); break;
		case 0xc568: showcurrentfile(context); break;
		case 0xc56c: monmessage(context); break;
		case 0xc570: processtrigger(context); break;
		case 0xc574: triggermessage(context); break;
		case 0xc578: printcurs(context); break;
		case 0xc57c: delcurs(context); break;
		case 0xc580: useobject(context); break;
		case 0xc584: useroutine(context); break;
		case 0xc588: wheelsound(context); break;
		case 0xc58c: runtap(context); break;
		case 0xc590: playguitar(context); break;
		case 0xc594: hotelcontrol(context); break;
		case 0xc598: hotelbell(context); break;
		case 0xc59c: opentomb(context); break;
		case 0xc5a0: usetrainer(context); break;
		case 0xc5a4: nothelderror(context); break;
		case 0xc5a8: usepipe(context); break;
		case 0xc5ac: usefullcart(context); break;
		case 0xc5b0: useplinth(context); break;
		case 0xc5b4: chewy(context); break;
		case 0xc5b8: useladder(context); break;
		case 0xc5bc: useladderb(context); break;
		case 0xc5c0: slabdoora(context); break;
		case 0xc5c4: slabdoorb(context); break;
		case 0xc5c8: slabdoord(context); break;
		case 0xc5cc: slabdoorc(context); break;
		case 0xc5d0: slabdoore(context); break;
		case 0xc5d4: slabdoorf(context); break;
		case 0xc5d8: useslab(context); break;
		case 0xc5dc: usecart(context); break;
		case 0xc5e0: useclearbox(context); break;
		case 0xc5e4: usecoveredbox(context); break;
		case 0xc5e8: userailing(context); break;
		case 0xc5ec: useopenbox(context); break;
		case 0xc5f0: wearwatch(context); break;
		case 0xc5f4: wearshades(context); break;
		case 0xc5f8: sitdowninbar(context); break;
		case 0xc5fc: usechurchhole(context); break;
		case 0xc600: usehole(context); break;
		case 0xc604: usealtar(context); break;
		case 0xc608: opentvdoor(context); break;
		case 0xc60c: usedryer(context); break;
		case 0xc610: openlouis(context); break;
		case 0xc614: nextcolon(context); break;
		case 0xc618: openyourneighbour(context); break;
		case 0xc61c: usewindow(context); break;
		case 0xc620: usebalcony(context); break;
		case 0xc624: openryan(context); break;
		case 0xc628: openpoolboss(context); break;
		case 0xc62c: openeden(context); break;
		case 0xc630: opensarters(context); break;
		case 0xc634: isitright(context); break;
		case 0xc638: drawitall(context); break;
		case 0xc63c: openhoteldoor(context); break;
		case 0xc640: openhoteldoor2(context); break;
		case 0xc644: grafittidoor(context); break;
		case 0xc648: trapdoor(context); break;
		case 0xc64c: callhotellift(context); break;
		case 0xc650: calledenslift(context); break;
		case 0xc654: calledensdlift(context); break;
		case 0xc658: usepoolreader(context); break;
		case 0xc65c: uselighter(context); break;
		case 0xc660: showseconduse(context); break;
		case 0xc664: usecardreader1(context); break;
		case 0xc668: usecardreader2(context); break;
		case 0xc66c: usecardreader3(context); break;
		case 0xc670: usecashcard(context); break;
		case 0xc674: lookatcard(context); break;
		case 0xc678: moneypoke(context); break;
		case 0xc67c: usecontrol(context); break;
		case 0xc680: usehatch(context); break;
		case 0xc684: usewire(context); break;
		case 0xc688: usehandle(context); break;
		case 0xc68c: useelevator1(context); break;
		case 0xc690: showfirstuse(context); break;
		case 0xc694: useelevator3(context); break;
		case 0xc698: useelevator4(context); break;
		case 0xc69c: useelevator2(context); break;
		case 0xc6a0: useelevator5(context); break;
		case 0xc6a4: usekey(context); break;
		case 0xc6a8: usestereo(context); break;
		case 0xc6ac: usecooker(context); break;
		case 0xc6b0: useaxe(context); break;
		case 0xc6b4: useelvdoor(context); break;
		case 0xc6b8: withwhat(context); break;
		case 0xc6bc: selectob(context); break;
		case 0xc6c0: compare(context); break;
		case 0xc6c4: findsetobject(context); break;
		case 0xc6c8: findexobject(context); break;
		case 0xc6cc: isryanholding(context); break;
		case 0xc6d0: checkinside(context); break;
		case 0xc6d4: usetext(context); break;
		case 0xc6d8: putbackobstuff(context); break;
		case 0xc6dc: showpuztext(context); break;
		case 0xc6e0: findpuztext(context); break;
		case 0xc6e4: placesetobject(context); break;
		case 0xc6e8: removesetobject(context); break;
		case 0xc6ec: issetobonmap(context); break;
		case 0xc6f0: placefreeobject(context); break;
		case 0xc6f4: removefreeobject(context); break;
		case 0xc6f8: findormake(context); break;
		case 0xc6fc: switchryanon(context); break;
		case 0xc700: switchryanoff(context); break;
		case 0xc704: setallchanges(context); break;
		case 0xc708: dochange(context); break;
		case 0xc70c: autoappear(context); break;
		case 0xc710: getundertimed(context); break;
		case 0xc714: putundertimed(context); break;
		case 0xc718: dumptimedtext(context); break;
		case 0xc71c: setuptimeduse(context); break;
		case 0xc720: setuptimedtemp(context); break;
		case 0xc724: usetimedtext(context); break;
		case 0xc728: edenscdplayer(context); break;
		case 0xc72c: usewall(context); break;
		case 0xc730: usechurchgate(context); break;
		case 0xc734: usegun(context); break;
		case 0xc738: useshield(context); break;
		case 0xc73c: usebuttona(context); break;
		case 0xc740: useplate(context); break;
		case 0xc744: usewinch(context); break;
		case 0xc748: entercode(context); break;
		case 0xc74c: loadkeypad(context); break;
		case 0xc750: quitkey(context); break;
		case 0xc754: addtopresslist(context); break;
		case 0xc758: buttonone(context); break;
		case 0xc75c: buttontwo(context); break;
		case 0xc760: buttonthree(context); break;
		case 0xc764: buttonfour(context); break;
		case 0xc768: buttonfive(context); break;
		case 0xc76c: buttonsix(context); break;
		case 0xc770: buttonseven(context); break;
		case 0xc774: buttoneight(context); break;
		case 0xc778: buttonnine(context); break;
		case 0xc77c: buttonnought(context); break;
		case 0xc780: buttonenter(context); break;
		case 0xc784: buttonpress(context); break;
		case 0xc788: showouterpad(context); break;
		case 0xc78c: showkeypad(context); break;
		case 0xc790: singlekey(context); break;
		case 0xc794: dumpkeypad(context); break;
		case 0xc798: usemenu(context); break;
		case 0xc79c: dumpmenu(context); break;
		case 0xc7a0: getundermenu(context); break;
		case 0xc7a4: putundermenu(context); break;
		case 0xc7a8: showoutermenu(context); break;
		case 0xc7ac: showmenu(context); break;
		case 0xc7b0: loadmenu(context); break;
		case 0xc7b4: viewfolder(context); break;
		case 0xc7b8: nextfolder(context); break;
		case 0xc7bc: folderhints(context); break;
		case 0xc7c0: lastfolder(context); break;
		case 0xc7c4: loadfolder(context); break;
		case 0xc7c8: showfolder(context); break;
		case 0xc7cc: folderexit(context); break;
		case 0xc7d0: showleftpage(context); break;
		case 0xc7d4: showrightpage(context); break;
		case 0xc7d8: entersymbol(context); break;
		case 0xc7dc: quitsymbol(context); break;
		case 0xc7e0: settopleft(context); break;
		case 0xc7e4: settopright(context); break;
		case 0xc7e8: setbotleft(context); break;
		case 0xc7ec: setbotright(context); break;
		case 0xc7f0: dumpsymbol(context); break;
		case 0xc7f4: showsymbol(context); break;
		case 0xc7f8: nextsymbol(context); break;
		case 0xc7fc: updatesymboltop(context); break;
		case 0xc800: updatesymbolbot(context); break;
		case 0xc804: dumpsymbox(context); break;
		case 0xc808: usediary(context); break;
		case 0xc80c: showdiary(context); break;
		case 0xc810: showdiarykeys(context); break;
		case 0xc814: dumpdiarykeys(context); break;
		case 0xc818: diarykeyp(context); break;
		case 0xc81c: diarykeyn(context); break;
		case 0xc820: showdiarypage(context); break;
		case 0xc824: findtext1(context); break;
		case 0xc828: zoomonoff(context); break;
		case 0xc82c: saveload(context); break;
		case 0xc830: dosaveload(context); break;
		case 0xc834: getbackfromops(context); break;
		case 0xc838: showmainops(context); break;
		case 0xc83c: showdiscops(context); break;
		case 0xc840: loadsavebox(context); break;
		case 0xc844: loadgame(context); break;
		case 0xc848: getbacktoops(context); break;
		case 0xc84c: discops(context); break;
		case 0xc850: savegame(context); break;
		case 0xc854: actualsave(context); break;
		case 0xc858: actualload(context); break;
		case 0xc85c: selectslot2(context); break;
		case 0xc860: checkinput(context); break;
		case 0xc864: getnamepos(context); break;
		case 0xc868: showopbox(context); break;
		case 0xc86c: showloadops(context); break;
		case 0xc870: showsaveops(context); break;
		case 0xc874: selectslot(context); break;
		case 0xc878: showslots(context); break;
		case 0xc87c: shownames(context); break;
		case 0xc880: dosreturn(context); break;
		case 0xc884: error(context); break;
		case 0xc888: namestoold(context); break;
		case 0xc88c: oldtonames(context); break;
		case 0xc890: savefilewrite(context); break;
		case 0xc894: savefileread(context); break;
		case 0xc898: saveposition(context); break;
		case 0xc89c: loadposition(context); break;
		case 0xc8a0: loadseg(context); break;
		case 0xc8a4: makeheader(context); break;
		case 0xc8a8: storeit(context); break;
		case 0xc8ac: saveseg(context); break;
		case 0xc8b0: findlen(context); break;
		case 0xc8b4: scanfornames(context); break;
		case 0xc8b8: decide(context); break;
		case 0xc8bc: showdecisions(context); break;
		case 0xc8c0: newgame(context); break;
		case 0xc8c4: loadold(context); break;
		case 0xc8c8: loadspeech(context); break;
		case 0xc8cc: createname(context); break;
		case 0xc8d0: loadsample(context); break;
		case 0xc8d4: loadsecondsample(context); break;
		case 0xc8d8: soundstartup(context); break;
		case 0xc8dc: trysoundalloc(context); break;
		case 0xc8e0: setsoundoff(context); break;
		case 0xc8e4: checksoundint(context); break;
		case 0xc8e8: enablesoundint(context); break;
		case 0xc8ec: disablesoundint(context); break;
		case 0xc8f0: interupttest(context); break;
		case 0xc8f4: soundend(context); break;
		case 0xc8f8: out22c(context); break;
		case 0xc8fc: playchannel0(context); break;
		case 0xc900: playchannel1(context); break;
		case 0xc904: makenextblock(context); break;
		case 0xc908: volumeadjust(context); break;
		case 0xc90c: loopchannel0(context); break;
		case 0xc910: cancelch0(context); break;
		case 0xc914: cancelch1(context); break;
		case 0xc918: channel0only(context); break;
		case 0xc91c: channel1only(context); break;
		case 0xc920: channel0tran(context); break;
		case 0xc924: bothchannels(context); break;
		case 0xc928: saveems(context); break;
		case 0xc92c: restoreems(context); break;
		case 0xc930: domix(context); break;
		case 0xc934: dmaend(context); break;
		case 0xc938: startdmablock(context); break;
		case 0xc93c: setuppit(context); break;
		case 0xc940: getridofpit(context); break;
		case 0xc944: pitinterupt(context); break;
		case 0xc948: dreamweb(context); break;
		case 0xc94c: entrytexts(context); break;
		case 0xc950: entryanims(context); break;
		case 0xc954: initialinv(context); break;
		case 0xc958: pickupob(context); break;
		case 0xc95c: setupemm(context); break;
		case 0xc960: removeemm(context); break;
		case 0xc964: checkforemm(context); break;
		case 0xc968: checkbasemem(context); break;
		case 0xc96c: allocatebuffers(context); break;
		case 0xc970: clearbuffers(context); break;
		case 0xc974: clearchanges(context); break;
		case 0xc978: clearbeforeload(context); break;
		case 0xc97c: clearreels(context); break;
		case 0xc980: clearrest(context); break;
		case 0xc984: deallocatemem(context); break;
		case 0xc988: allocatemem(context); break;
		case 0xc990: parseblaster(context); break;
		case 0xc994: startup(context); break;
		case 0xc998: startup1(context); break;
		case 0xc99c: screenupdate(context); break;
		case 0xc9a0: watchreel(context); break;
		case 0xc9a4: checkforshake(context); break;
		case 0xc9a8: watchcount(context); break;
		case 0xc9ac: showtime(context); break;
		case 0xc9b0: dumpwatch(context); break;
		case 0xc9b4: showbyte(context); break;
		case 0xc9b8: onedigit(context); break;
		case 0xc9bc: twodigitnum(context); break;
		case 0xc9c0: showword(context); break;
		case 0xc9c4: convnum(context); break;
		case 0xc9c8: mainscreen(context); break;
		case 0xc9cc: madmanrun(context); break;
		case 0xc9d0: checkcoords(context); break;
		case 0xc9d4: identifyob(context); break;
		case 0xc9d8: checkifperson(context); break;
		case 0xc9dc: checkifset(context); break;
		case 0xc9e0: checkifex(context); break;
		case 0xc9e4: checkiffree(context); break;
		case 0xc9e8: isitdescribed(context); break;
		case 0xc9ec: findpathofpoint(context); break;
		case 0xc9f0: findfirstpath(context); break;
		case 0xc9f4: turnpathon(context); break;
		case 0xc9f8: turnpathoff(context); break;
		case 0xc9fc: turnanypathon(context); break;
		case 0xca00: turnanypathoff(context); break;
		case 0xca04: checkifpathison(context); break;
		case 0xca08: afternewroom(context); break;
		case 0xca0c: atmospheres(context); break;
		case 0xca10: walkintoroom(context); break;
		case 0xca14: afterintroroom(context); break;
		case 0xca18: obname(context); break;
		case 0xca1c: finishedwalking(context); break;
		case 0xca20: examineobtext(context); break;
		case 0xca24: commandwithob(context); break;
		case 0xca28: commandonly(context); break;
		case 0xca2c: printmessage(context); break;
		case 0xca30: printmessage2(context); break;
		case 0xca34: blocknametext(context); break;
		case 0xca38: personnametext(context); break;
		case 0xca3c: walktotext(context); break;
		case 0xca40: getflagunderp(context); break;
		case 0xca44: setwalk(context); break;
		case 0xca48: autosetwalk(context); break;
		case 0xca4c: checkdest(context); break;
		case 0xca50: bresenhams(context); break;
		case 0xca54: workoutframes(context); break;
		case 0xca58: getroomspaths(context); break;
		case 0xca5c: copyname(context); break;
		case 0xca60: findobname(context); break;
		case 0xca64: showicon(context); break;
		case 0xca68: middlepanel(context); break;
		case 0xca6c: showman(context); break;
		case 0xca70: showpanel(context); break;
		case 0xca74: roomname(context); break;
		case 0xca78: usecharset1(context); break;
		case 0xca7c: usetempcharset(context); break;
		case 0xca80: showexit(context); break;
		case 0xca84: panelicons1(context); break;
		case 0xca88: showwatch(context); break;
		case 0xca8c: gettime(context); break;
		case 0xca90: zoomicon(context); break;
		case 0xca94: showblink(context); break;
		case 0xca98: dumpblink(context); break;
		case 0xca9c: worktoscreenm(context); break;
		case 0xcaa0: blank(context); break;
		case 0xcaa4: allpointer(context); break;
		case 0xcaa8: hangonp(context); break;
		case 0xcaac: hangonw(context); break;
		case 0xcab0: hangoncurs(context); break;
		case 0xcab4: getunderzoom(context); break;
		case 0xcab8: dumpzoom(context); break;
		case 0xcabc: putunderzoom(context); break;
		case 0xcac0: crosshair(context); break;
		case 0xcac4: showpointer(context); break;
		case 0xcac8: delpointer(context); break;
		case 0xcacc: dumppointer(context); break;
		case 0xcad0: undertextline(context); break;
		case 0xcad4: deltextline(context); break;
		case 0xcad8: dumptextline(context); break;
		case 0xcadc: animpointer(context); break;
		case 0xcae0: setmouse(context); break;
		case 0xcae4: readmouse(context); break;
		case 0xcae8: mousecall(context); break;
		case 0xcaec: readmouse1(context); break;
		case 0xcaf0: readmouse2(context); break;
		case 0xcaf4: readmouse3(context); break;
		case 0xcaf8: readmouse4(context); break;
		case 0xcafc: readkey(context); break;
		case 0xcb00: convertkey(context); break;
		case 0xcb04: randomnum1(context); break;
		case 0xcb08: randomnum2(context); break;
		case 0xcb10: hangon(context); break;
		case 0xcb14: loadtraveltext(context); break;
		case 0xcb18: loadintotemp(context); break;
		case 0xcb1c: loadintotemp2(context); break;
		case 0xcb20: loadintotemp3(context); break;
		case 0xcb24: loadtempcharset(context); break;
		case 0xcb28: standardload(context); break;
		case 0xcb2c: loadtemptext(context); break;
		case 0xcb30: loadroom(context); break;
		case 0xcb34: loadroomssample(context); break;
		case 0xcb38: getridofreels(context); break;
		case 0xcb3c: getridofall(context); break;
		case 0xcb40: restorereels(context); break;
		case 0xcb44: restoreall(context); break;
		case 0xcb48: sortoutmap(context); break;
		case 0xcb4c: startloading(context); break;
		case 0xcb50: disablepath(context); break;
		case 0xcb54: findxyfrompath(context); break;
		case 0xcb58: findroominloc(context); break;
		case 0xcb5c: getroomdata(context); break;
		case 0xcb60: readheader(context); break;
		case 0xcb64: dontloadseg(context); break;
		case 0xcb68: allocateload(context); break;
		case 0xcb6c: fillspace(context); break;
		case 0xcb70: getridoftemp(context); break;
		case 0xcb74: getridoftemptext(context); break;
		case 0xcb78: getridoftemp2(context); break;
		case 0xcb7c: getridoftemp3(context); break;
		case 0xcb80: getridoftempcharset(context); break;
		case 0xcb84: getridoftempsp(context); break;
		case 0xcb88: readsetdata(context); break;
		case 0xcb8c: createfile(context); break;
		case 0xcb90: openfile(context); break;
		case 0xcb94: openfilenocheck(context); break;
		case 0xcb98: openforsave(context); break;
		case 0xcb9c: closefile(context); break;
		case 0xcba0: readfromfile(context); break;
		case 0xcba4: setkeyboardint(context); break;
		case 0xcba8: resetkeyboard(context); break;
		case 0xcbac: keyboardread(context); break;
		case 0xcbb0: walkandexamine(context); break;
		case 0xcbb4: doload(context); break;
		case 0xcbb8: generalerror(context); break;
		default: ::error("invalid call to %04x dispatched", (uint16)context.ax);
	}
}

} /*namespace*/
