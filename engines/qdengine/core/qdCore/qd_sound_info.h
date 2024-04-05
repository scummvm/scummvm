
#ifndef __QD_SOUND_INFO_H__
#define __QD_SOUND_INFO_H__

#include "qd_named_object.h"

class qdSound;

//! Информация о звуке.
class qdSoundInfo : public qdNamedObject
{
public:
	qdSoundInfo(){ }
	~qdSoundInfo(){ }

	//! Флаги.
	enum {
		//! зацикливать звук при проигрывании
		LOOP_SOUND_FLAG = 0x01
	};

	int named_object_type() const { return QD_NAMED_OBJECT_SOUND_INFO; }

	qdSound* sound() const;
};

#endif /* __QD_SOUND_INFO_H__ */
