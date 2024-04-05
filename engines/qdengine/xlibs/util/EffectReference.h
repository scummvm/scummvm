#ifndef __EFFECT_REFERENCE_H__
#define __EFFECT_REFERENCE_H__

#include "XMath\Colors.h"
#include "Serialization\SerializationTypes.h"
#include "Units\Object3dxInterface.h"
#include "EffectContainer.h"

/// параметры спецэффекта
class EffectAttribute
{
public:
	EffectAttribute();
	EffectAttribute(const EffectReference& effectReference, bool isCycled);
	virtual ~EffectAttribute(){ };

	bool isEmpty() const { return effectReference_.get() == 0; }

	bool legionColor() const { return legionColor_; }

	void serialize(Archive& ar);

	bool isCycled() const { return isCycled_; }
	bool stopImmediately() const { return stopImmediately_; }
	bool bindOrientation() const { return bindOrientation_; }

	bool onWaterSurface() const { return (waterPlacementMode_ == WATER_SURFACE); }

	bool switchOffUnderWater() const { return switchOffUnderWater_; }
	bool switchOffUnderLava() const { return switchOffUnderLava_; }
	bool switchOffByDay() const { return switchOffByDay_; }
	bool switchOffOnIce() const { return switchOffOnIce_; }
	bool switchOnOnIce() const { return switchOnIce_; }

	bool switchOffByInterface() const { return switchOffByInterface_; }

	bool ignoreFogOfWar() const { return ignoreFogOfWar_; }
	bool ignoreInvisibility() const { return ignoreInvisibility_; }
	bool ignoreDistanceCheck() const { return ignoreDistanceCheck_; }

	float scale() const { return scale_; }
	
	const EffectReference& effectReference() const { return effectReference_; }
	EffectKey* effect(float scale = -1.f, Color4c skin_color = Color4c(255,255,255,255)) const { return isEmpty() ? 0 : effectReference_->getEffect(scale > 0.f ? scale : scale_, skin_color); }

	enum WaterPlacementMode {
		/// ставить на дно
		WATER_BOTTOM,
		/// ставить на поверхность воды
		WATER_SURFACE
	};

protected:

	/// зацикливать эффект или нет
	bool isCycled_;
	/// обрывать
	bool stopImmediately_;

	/// ориентировать эффект по объекту
	bool bindOrientation_;

	/// окрашивать в цвет легиона
	bool legionColor_;

	/// выключать в воде
	bool switchOffUnderWater_;
	/// выключать в лаве
	bool switchOffUnderLava_;
	/// выключать днем
	bool switchOffByDay_;
	/// выключать на льду
	bool switchOffOnIce_;
	/// включать только на льду
	bool switchOnIce_;
	/// true если виден в тумане войны
	bool ignoreFogOfWar_;
	/// true если виден на невидимом юните
	bool ignoreInvisibility_;
	/// true если не должен пропадать при удалении камеры
	bool ignoreDistanceCheck_;
	/// отключать вместе с интерфейсом
	bool switchOffByInterface_;

	/// режим устаноки эффекта на воду
	WaterPlacementMode waterPlacementMode_;

	/// масштаб эффекта
	float scale_;

	/// ссылка на эффект из библиотеки эффектов
	EffectReference effectReference_;
};

class EffectAttributeAttachable : public EffectAttribute
{
	bool onlyForActivePlayer_;
	///масштабировать ли спецэффект по размеру объекта
	bool scaleByModel_;

	bool needNodeName_;
	bool synchronizationWithModelAnimation_;
	/// прерывается анимацией
	bool switchOffByAnimationChain_;
	Object3dxNode node_;

public:
	EffectAttributeAttachable(bool need_node_name = true);
	EffectAttributeAttachable(const EffectReference& effectReference, bool isCycled);

	bool onlyForActivePlayer() const { return onlyForActivePlayer_; }
	bool scaleByModel() const {return scaleByModel_;}
	int node() const { return node_; };
	bool isSynchronize() const {return synchronizationWithModelAnimation_;}
	bool switchOffByAnimationChain() const { return switchOffByAnimationChain_; }

	void serialize(Archive& ar);
};

#endif //__EFFECT_REFERENCE_H__
