// (c) 1999 Balmer - Cyborix
#ifndef _BITSET_H_
#define _BITSET_H_
class BitSet
{
protected:
	bool b[256];
public:
	BitSet(){for(int i=0;i<256;i++)b[i]=false;}
	inline bool In(unsigned char c){return b[c];}
	inline void Set(unsigned char c){b[c]=1;}
	inline void Reset(unsigned char c){b[c]=0;}
	inline void Set(unsigned char  cmin,unsigned char  cmax){for(int i=cmin;i<=cmax;i++)b[i]=true;}

	inline bool In(char c){return In((unsigned char)c);}
	inline void Set(char c){Set((unsigned char)c);}
	inline void Reset(char c){b[c]=0;}
	inline void Set(char cmin,char cmax){Set((unsigned char)cmin,(unsigned char)cmax);}

	BitSet& operator +=(BitSet& bs)
	{
		for(int i=0;i<256;i++)
			if(bs.b[i])	
				b[i]=true;
		return *this;
	}
	BitSet& operator =(BitSet& bs)
	{
		for(int i=0;i<256;i++)b[i]=bs.b[i];
		return *this;
	}

	void not(){for(int i=0;i<256;i++)b[i]=!b[i];}
};

#endif  _BITSET_H_
