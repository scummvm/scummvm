#pragma once

//////////////////////////////////////////////////////////////////////////////  
// FAST INVERSE SQUARE ROOT
// By Chris Lomont
// http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
//
// ћаксимальна€ погрешность 0.0017758, приблизительно в 6 раз быстрее 1/sqrtf()
//////////////////////////////////////////////////////////////////////////////  
inline float invSqrtFast(float x)
{
	x += 1e-7f; // ƒобавка, устран€юща€ деление на 0
	float xhalf = 0.5f*x;
	int i = *(int*)&x; // get bits for floating value
	i = 0x5f375a86 - (i>>1); // gives initial guess y0
	x = *(float*)&i; // convert bits back to float
	x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
	return x;
}

// ¬ 3 раза быстрее за счет проверки аргументов, точна€.
inline float fmodFast(float a, float b)
{
	float result;
	_asm
	{
		fld b
			fld a
cycle_fast_fmod:
		fprem
			fnstsw ax
			sahf
			jp short cycle_fast_fmod
			fstp st(1)
			fstp result
	}
	return result;
}

inline unsigned int F2DW( float f ) 
{ 
	return *((unsigned int*)&f); 
}

inline float DW2F( unsigned int f ) 
{ 
	return *((float*)&f); 
}

