#ifndef __LZ77_H
#define __LZ77_H

class CLZ77
{
private:
	long LZComp(unsigned char *s1, unsigned char *s2, long maxlen);
	unsigned char *FindLZ(unsigned char *source, unsigned char *s, long slen, long border, long mlen, long &len);
public:
	CLZ77();
	virtual ~CLZ77();

	void Encode(unsigned char *target, long &tlen, const unsigned char *source, long slen);
	long Decode(unsigned char *target, long &tlen, const unsigned char *source, long slen);
	long GetMaxEncoded(long len);
	long GetMaxDecoded(unsigned char *source);

//	virtual void OnStep() = 0;
};

#endif
