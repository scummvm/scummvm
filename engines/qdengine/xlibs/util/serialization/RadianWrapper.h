#pragma once

class RadianWrapper
{
public:
	RadianWrapper(float& angleRadian, float angleMin = -2*M_PI, float angleMax = 2*M_PI) : angleRadian_(angleRadian), 
		angleMin_(angleMin), angleMax_(angleMax)
	{}

	bool serialize(Archive& ar, const char* name, const char* nameAlt)
	{
		if(ar.isEdit()){
			float angleDegree = R2G(angleRadian_);
			bool nodeExists = ar.serialize(angleDegree, name, nameAlt);
			angleRadian_ = clamp(G2R(angleDegree), angleMin_, angleMax_);
			return nodeExists;
		}
		else
			return ar.serialize(angleRadian_, name, nameAlt);
	}
private:
	float& angleRadian_;
	float angleMin_;
	float angleMax_;
};
