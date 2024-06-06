#ifndef __QD_SOUND_HANDLE_H__
#define __QD_SOUND_HANDLE_H__

#include "qdengine/core/system/sound/snd_sound.h"


namespace QDEngine {

//! Класс для управления звуками.
class qdSoundHandle : public sndHandle {
public:
	qdSoundHandle() : owner_(NULL) { };
	virtual ~qdSoundHandle() { };

	void set_owner(qdNamedObject *p) {
		owner_ = p;
	}
	const qdNamedObject *owner() const {
		return owner_;
	}

private:
	qdNamedObject *owner_;
};

} // namespace QDEngine

#endif /* __QD_SOUND_HANDLE_H__ */
