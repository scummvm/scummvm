#ifndef COMMON_USERACTION
#define COMMON_USERACTION

#include "common/events.h"
#include "common/list.h"
#include "common/str.h"

namespace Common {

struct HardwareKey;
class Keymap;


enum UserActionType {
	kGenericUserActionType,

	// common actions
	kDirectionUpUserAction,
	kDirectionDownUserAction,
	kDirectionLeftUserAction,
	kDirectionRightUserAction,
	kLeftClickUserAction,
	kRightClickUserAction,
	kSaveUserAction,
	kMenuUserAction,

	kUserActionTypeMax
};

enum UserActionCategory {
	kGenericUserActionCategory,
	// classes of action - probably need to be slightly more specific than this
	kInGameUserAction,   // effects the actual gameplay
	kSystemUserAction,   //show a menu / change volume / etc

	kUserActionCategoryMax
};

struct UserAction {
	/** unique id used for saving/loading to config */
	int32 id;
	/** Human readable description */
	String description;
	/** Events to be sent when mapped key is pressed */
	List<Event> events;

	UserActionCategory category;
	UserActionType type;
	int priority;
	int group;
	int flags;

private:
	/** Hardware key that is mapped to this UserAction */
	const HardwareKey *_hwKey;
	Keymap *_parent;

public:
	UserAction(	String des = "", 
		UserActionCategory cat = kGenericUserActionCategory,
		UserActionType ty = kGenericUserActionType,
		int pr = 0, int gr = 0, int fl = 0 );

	void setParent(Keymap *parent);

	void mapKey(const HardwareKey *key);

	const HardwareKey *getMappedKey() const;
};

} // end of namespace Common

#endif