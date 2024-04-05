#ifndef __EDITOR_VISUAL_H_INCLUDED__
#define __EDITOR_VISUAL_H_INCLUDED__

enum UniverseObjectClass;
class BaseUniverseObject;
class UnitBase;

namespace EditorVisual{

	enum RadiusType{
		RADIUS_OBJECT,
		RADIUS_TOOL,
		RADIUS_IMPASSABILITY
	};

	enum TextType{
		TEXT_LABEL,
		TEXT_PROPERTIES
	};

	enum CrossType{
		CROSS_CENTER
	};

	class Interface{
	public:
		virtual bool isVisible(UniverseObjectClass objectClass) = 0;
		virtual void beforeQuant() = 0;
		virtual void afterQuant() = 0;
		virtual void drawImpassabilityRadius(UnitBase& unit) = 0;
		virtual void drawCross(const Vect3f& position, float size, CrossType crossType = CROSS_CENTER, bool selected = false) = 0;
		virtual void drawRadius(const Vect3f& position, float radius, RadiusType radiusType = RADIUS_OBJECT, bool selected = false) = 0;
		virtual void drawText(const Vect3f& position, const char* text, TextType textType = TEXT_LABEL) = 0;
		virtual void drawOrientationArrow(const Se3f& pose, bool selected) = 0;
	};

};

EditorVisual::Interface& editorVisual();

#endif
