#ifdef _WIN32_WCE

typedef void (*pAction)();

enum ActionType {
	ACTION_NONE = 0,
	ACTION_PAUSE = 1,
	ACTION_SAVE = 2,
	ACTION_QUIT = 3,
	ACTION_SKIP = 4,
	ACTION_HIDE = 5,
	ACTION_KEYBOARD = 6,
	ACTION_SOUND = 7,
	ACTION_RIGHTCLICK = 8,
	ACTION_CURSOR = 9,
	ACTION_SUBTITLES = 10
};

struct oneAction {
	unsigned char	action_key;
	int				action_type;
};


#define NUMBER_ACTIONS 10
#define TOTAL_ACTIONS 10

#define GAPI_KEY_BASE 1000
#define GAPI_KEY_VKA 1
#define GAPI_KEY_VKB 2
#define GAPI_KEY_VKC 3
#define GAPI_KEY_VKSTART 4
#define GAPI_KEY_CALENDAR 5
#define GAPI_KEY_CONTACTS 6
#define GAPI_KEY_INBOX 7
#define GAPI_KEY_ITASK 8
#define GAPI_KEY_VKUP 9
#define GAPI_KEY_VKDOWN 10
#define GAPI_KEY_VKLEFT 11
#define GAPI_KEY_VKRIGHT 12

#define INTERNAL_KEY_CALENDAR 0xc1
#define INTERNAL_KEY_CONTACTS 0xc2
#define INTERNAL_KEY_INBOX 0xc3
#define INTERNAL_KEY_ITASK 0xc4

void GAPIKeysInit(pAction*);
void GAPIKeysGetReference(void);
const unsigned char getGAPIKeyMapping(short);
const char* getGAPIKeyName(unsigned char);
struct oneAction* getAction(int);
void processAction (short);
void clearActionKey (unsigned char);
const unsigned char* getActionKeys(void);
void setActionKeys(unsigned char *);
const char* getActionName(int);
void setActionTypes(unsigned char *);
const unsigned char* getActionTypes();
void setNextType(int);
void setPreviousType(int);

#endif
