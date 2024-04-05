#ifndef __EFFECT_CONTAINER_H__
#define __EFFECT_CONTAINER_H__

#include "XMath\Colors.h"
#include "Serialization\StringTableReferencePolymorphic.h"

class EffectKey;
class Archive;

//-----------------------------------------------------------------------------------
// новая система эффектов

/// контейнер спецэффектов
/**
физически - файл .eff на диске + необходимые модели и текстуры
*/

class EffectContainer : public PolymorphicBase
{
public:
	EffectContainer();
	~EffectContainer();

	void serialize(Archive& ar);

	EffectKey* getEffect(float scale = 1.f, Color4c skin_color = Color4c(255, 255, 255, 255)) const;

	const char* fileName() const { return fileName_.c_str(); }
	void setFileName(const char* fileName){ fileName_ = fileName; }
	static void setTexturesPath(const char* path){ texturesPath_ = path; }
	void preloadLibrary();
private:
	std::string fileName_;
	static std::string texturesPath_;
};

typedef StringTable<StringTableBasePolymorphic<EffectContainer> > EffectLibrary;
typedef StringTableReferencePolymorphic<EffectContainer, false> EffectReference;

#endif//__EFFECT_CONTAINER_H__
