#include <math.h>
#include "matrix3.h"

Matrix3::setAsIdentity( void )
{
	right_.set(1.f, 0.f, 0.f);
	up_.set(0.f, 1.f, 0.f);
	at_.set(0.f, 0.f, 0.f);
}

Matrix3::buildFromPitchYawRoll( float pitch, float yaw, float roll )
{
    Matrix3 temp1, temp2;

	temp1.constructAroundPitch( pitch );
	constructAroundRoll( roll );

	(*this) *= temp1;

	temp2.constructAroundYaw( yaw );

	(*this) *= temp2;
}

#define  MYPI   3.141592654 
#define  DEGTORAD(a)   (a*MYPI/180.0) 
#define  RADTODEG(a)   (a*180.0/MYPI) 

float RadianToDegree( float rad )
{
	return RADTODEG(rad);
}

float DegreeToRadian( float degrees )
{
	return DEGTORAD(degrees);
}

// right
Matrix3::constructAroundPitch( float pitch )
{
	float cosa;
	float sina;

	cosa = (float)cos( DegreeToRadian(pitch) );
	sina = (float)sin( DegreeToRadian(pitch) );

	right_.set( 1.f, 0.f, 0.f );
	up_.set( 0.f, cosa, -sina );
	at_.set( 0.f, sina, cosa );
}

// up
Matrix3::constructAroundYaw( float yaw )
{
	float cosa;
	float sina;

	cosa = (float)cos( DegreeToRadian(yaw) );
	sina = (float)sin( DegreeToRadian(yaw) );

	right_.set( cosa, 0.f, sina );
	up_.set( 0.f, 1.f, 0.f );
	at_.set( -sina, 0.f, cosa );
}

// at
Matrix3::constructAroundRoll( float roll )
{
	float cosa;
	float sina;

	cosa = (float)cos( DegreeToRadian(roll) );
	sina = (float)sin( DegreeToRadian(roll) );

	right_.set( cosa, -sina, 0.f );
	up_.set( sina, cosa, 0.f );
	at_.set( 0.f, 0.f, 1.f );
}

/*
0 1 2 3
4 5 6 7
8 9 10 11
*/

// WARNING: Still buggy in some occasions.
void Matrix3::getPitchYawRoll( float* pPitch, float* pYaw, float* pRoll )
{
	float D;
	float C;
	float ftrx;
	float ftry;
	float angle_x;
	float angle_y;
	float angle_z;

    angle_y = D =  asin( right_.z() );        /* Calculate Y-axis angle */
    C           =  cos( angle_y );
    angle_y		=  RadianToDegree( angle_y );

    if ( fabs( C ) > 0.005 )             /* Gimball lock? */
      {
      ftrx      =  at_.z() / C;           /* No, so get X-axis angle */
      ftry      = -up_.z()  / C;

      angle_x  = RadianToDegree(atan2( ftry, ftrx ));

      ftrx      =  right_.x() / C;            /* Get Z-axis angle */
      ftry      = -right_.y() / C;

      angle_z  = RadianToDegree(atan2( ftry, ftrx ));
      }
    else                                 /* Gimball lock has occurred */
      {      
		angle_x  = 0;                      /* Set X-axis angle to zqero */

      ftrx      =  up_.y();                 /* And calculate Z-axis angle */
      ftry      =  up_.x();

      angle_z  = RadianToDegree(atan2( ftry, ftrx ));
      }

    /* return only positive angles in [0,360] */
    if (angle_x < 0) angle_x += 360;
    if (angle_y < 0) angle_y += 360;
    if (angle_z < 0) angle_z += 360;

	if( pPitch)
		*pPitch = angle_x;

	if( pYaw )
        *pYaw = angle_y;

	if( pRoll )
		*pRoll = angle_z;
}

float Matrix3::getPitch()
{
	float pitch;

	getPitchYawRoll( &pitch, 0, 0);

	return pitch;
}

float Matrix3::getYaw()
{
	float yaw;

	getPitchYawRoll( 0, &yaw, 0);

	return yaw;
}

float Matrix3::getRoll()
{
	float roll;

	getPitchYawRoll( 0, 0, &roll);

	return roll;
}

void Matrix3::transform( Vector3d v )
{
	float x;
	float y;
	float z;

	x = v.dotProduct( right_.x(), up_.x(), at_.x() );
	y = v.dotProduct( right_.x(), up_.x(), at_.x() );
	z = v.dotProduct( right_.x(), up_.x(), at_.x() );

	v.set( x, y, z );
}
