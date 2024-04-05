#ifndef __QD_SOUND_HANDLE_H__
#define __QD_SOUND_HANDLE_H__

#include "snd_sound.h"

//! Класс для управления звуками.
class qdSoundHandle : public sndHandle
{
public:
	qdSoundHandle() : owner_(NULL) { };
	virtual ~qdSoundHandle(){ };

	void set_owner(qdNamedObject* p){ owner_ = p; }
	const qdNamedObject* owner() const { return owner_; }

private:
	qdNamedObject* owner_;
};

#endif /* __QD_SOUND_HANDLE_H__ */

