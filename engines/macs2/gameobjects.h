#pragma once
class gameobjects {
};
#pragma once

#include "common/array.h"
#include "common/singleton.h"

namespace Macs2 {

class GameObject {
public:
	// These are the values read by fn0037_07F8
	Common::Array<uint16> Values;
};

class GameObjects : public Common::Singleton<GameObjects> {

public:
	// Maximum of 200h objects
	// How to address them in the original code:
	// mov	di,[bp+6h]
	//	shl di, 2h;
	// les di, [di + 77Ch]
	Common::Array<GameObject> Objects;
};

} // namespace Macs2
