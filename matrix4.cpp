#include "matrix4.h"

Matrix4::Matrix4( void )
{
	pos_.set( 0.f, 0.f, 0.f );
	rot_.setAsIdentity();
}

void Matrix4::translate( float x, float y, float z )
{
	Vector3d v;

	v.set( x, y, z );

	rot_.transform( v );

	pos_ += v;
}

