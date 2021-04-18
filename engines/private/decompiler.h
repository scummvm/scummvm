#include <vector>
#include <string>

namespace Private {

const std::string kHeader("Precompiled Game Matrix");

const unsigned char kCodeString = 0x01;
const unsigned char kCodeShortLiteral = 0x02;
const unsigned char kCodeBraceClose = 0x04;
const unsigned char kCodeRect = 0x2e;
const unsigned char kCodeRects = 0x4f;
const unsigned char kCodeShortId = 0x50;

const std::vector<std::string> kCodeTable = {"",        //
					     "",        // 0x01  (string)
					     "",        // 0x02  (short literal)
					     " {\n",    // 0x03
					     "}\n",     // 0x04
					     "(",       // 0x05
					     ")",       // 0x06
					     "",        //
					     "",        //
					     "",        //
					     ",",       // 0x0a
					     "",        //
					     "%",       // 0x0c
					     "",        //
					     ";\n",     // 0x0e
					     "!",       // 0x0f
					     "-",       // 0x10
					     "+",       // 0x11
					     "=",       // 0x12
					     ">",       // 0x13
					     "<",       // 0x14
					     "if ",     // 0x15
					     "else ",   // 0x16
					     "Exit",    // 0x17
					     "goto",    // 0x18
					     "Mask",    // 0x19
					     "MaskDrawn",           // 0x1a
					     "Movie",               // 0x1b
					     "Transition",          // 0x1c
					     "ThumbnailMovie",      // 0x1d
					     "BustMovie",           // 0x1e
					     "ViewScreen",          // 0x1f
					     "VSPicture",           // 0x20
					     "Bitmap",              // 0x21
					     "Timer",               // 0x22
					     "SoundArea",           // 0x23
					     "Sound",               // 0x24
					     "SoundEffect",         // 0x25
					     "SyncSound",           // 0x26
					     "LoopedSound",         // 0x27
					     "NoStopSounds",        // 0x28
					     "Resume",              // 0x29
					     "Inventory",           // 0x2a
					     "SetFlag",             // 0x2b
					     "ChgMode",             // 0x2c
					     "PoliceBust",          // 0x2d
					     "CRect",               // 0x2e   overridden with "RECT" if in "define rects" block
					     "",                    //
					     "Random",              // 0x30
					     "SafeDigit",           // 0x31
					     "LoseInventory",       // 0x32
					     "",                    //
					     "PaperShuffleSound",   // 0x34
					     "Quit",                // 0x35
					     "DossierAdd",          // 0x36
					     "DossierBitmap",       // 0x37
					     "DossierPrevSuspect",  // 0x38
					     "DossierNextSuspect",  // 0x39
					     "DossierChgSheet",     // 0x3a
					     "DiaryLocList",        // 0x3b
					     "DiaryPage",           // 0x3c
					     "DiaryInvList",        // 0x3d
					     "DiaryPageTurn",       // 0x3e
					     "DiaryGoLoc",          // 0x3f
					     "SaveGame",            // 0x40
					     "LoadGame",            // 0x41
					     "RestartGame",         // 0x42
					     "AskSave",             // 0x43
					     "SetModifiedFlag",     // 0x44
					     "PhoneClip",           // 0x45
					     "PoliceClip",          // 0x46
					     "AMRadioClip",         // 0x47
					     "\nsetting ",          // 0x48
					     "debug ",              // 0x49
					     "\ndefine ",           // 0x4a
					     "",                    //
					     "variables",           // 0x4c
					     "",                    //
					     "",                    //
					     "rects",               // 0x4f
					     ""};                   // 0x50  (short id)


class Decompiler {
public:
	Decompiler(const std::string &filename, bool mac = false);
	Decompiler(std::vector<unsigned char> &buffer, bool mac = false);
	virtual ~Decompiler();
	void getResult(std::string &result) const;
private:
	void decompile(std::vector<unsigned char> &buffer, bool mac);
	std::string _result;
};

}
