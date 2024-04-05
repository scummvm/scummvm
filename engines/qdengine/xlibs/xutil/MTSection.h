#ifndef _MT_SECTION_H_
#define _MT_SECTION_H_

#include <Windows.h>

class MTSection
{
protected:
	//Этот класс при текущей реализации нежелательно копировать. Да и не нужно реально это смотря по коду.
	MTSection(const MTSection& in)
	{
		InitializeCriticalSection(&cs);
		num_lock=0;
	}
	void operator =(const MTSection& in) {} //Don't copy this object
public:
	MTSection()
	{
		InitializeCriticalSection(&cs);
		num_lock=0;
	}

	~MTSection()
	{
		DeleteCriticalSection(&cs);
	}


	void lock()
	{
#ifdef start_timer_auto
		start_timer_auto();
#endif
		EnterCriticalSection(&cs);
		num_lock++;
	}

	void unlock()
	{
		num_lock--;
		xassert(num_lock>=0);
		LeaveCriticalSection(&cs);
	}

	bool locked() const { return num_lock > 0; }

private:
	CRITICAL_SECTION cs;
	int num_lock;

	void lockInternal()
	{
		EnterCriticalSection(&cs);
		num_lock++;
	}

	void unlockInternal()
	{
		num_lock--;
		xassert(num_lock>=0);
		LeaveCriticalSection(&cs);
	}

	friend class MTAutoInternal;
};

class MTAuto
{
public:
	MTAuto(MTSection& section)
		: section_(section)
	{
		section_.lock();
	}

	~MTAuto()
	{
		section_.unlock();
	}

private:
	MTSection& section_;
};

class MTAutoInternal
{
public:
	MTAutoInternal(MTSection& section)
		: section_(section)
	{
		section_.lockInternal();
	}

	~MTAutoInternal()
	{
		section_.unlockInternal();
	}

private:
	MTSection& section_;
};


#endif _MT_SECTION_H_