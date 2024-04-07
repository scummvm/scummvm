#ifndef __MILLISECONS_WRAPPER_H_INCLUDED__
#define __MILLISECONS_WRAPPER_H_INCLUDED__

class MillisecondsWrapper
{
public:
	MillisecondsWrapper(int& timeInMiliseconds) :
	  timeInMiliseconds_(timeInMiliseconds)	{ }

	bool serialize(Archive& ar, const char* name, const char* nameAlt)
	{
		if(ar.isEdit()){
			float timeInSeconds = timeInMiliseconds_ / 1000.0f;
			bool nodeExists = ar.serialize(timeInSeconds, name, nameAlt);
			timeInMiliseconds_ = int(timeInSeconds * 1000.0f);
			return nodeExists;
		}
		else
			return ar.serialize(timeInMiliseconds_, name, nameAlt);
	}
private:
	int& timeInMiliseconds_;
};

#endif // __MILLISECONS_WRAPPER_H_INCLUDED__
