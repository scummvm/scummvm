#ifndef D3DUTILS_H
#define D3DUTILS_H
/*
**-----------------------------------------------------------------------------
** Name:    D3DUtils.h
** Purpose: Various D3D utility functions
** Notes:
**
** Copyright (c) 1995 - 1997 by Microsoft, all rights reserved
**-----------------------------------------------------------------------------
*/

typedef float VALUE3D;
typedef struct _MATRIX3D {
    union {
        struct {
            VALUE3D        _11, _12, _13, _14;
            VALUE3D        _21, _22, _23, _24;
            VALUE3D        _31, _32, _33, _34;
            VALUE3D        _41, _42, _43, _44;
			
        };
        VALUE3D m[4][4];
    };
    _MATRIX3D() { }
    _MATRIX3D( VALUE3D _m00, VALUE3D _m01, VALUE3D _m02, VALUE3D _m03,
		VALUE3D _m10, VALUE3D _m11, VALUE3D _m12, VALUE3D _m13,
		VALUE3D _m20, VALUE3D _m21, VALUE3D _m22, VALUE3D _m23,
		VALUE3D _m30, VALUE3D _m31, VALUE3D _m32, VALUE3D _m33
        ) 
	{
		m[0][0] = _m00; m[0][1] = _m01; m[0][2] = _m02; m[0][3] = _m03;
		m[1][0] = _m10; m[1][1] = _m11; m[1][2] = _m12; m[1][3] = _m13;
		m[2][0] = _m20; m[2][1] = _m21; m[2][2] = _m22; m[2][3] = _m23;
		m[3][0] = _m30; m[3][1] = _m31; m[3][2] = _m32; m[3][3] = _m33;
	}
	
    VALUE3D& operator()(int iRow, int iColumn) { return m[iRow][iColumn]; }
    const VALUE3D& operator()(int iRow, int iColumn) const { return m[iRow][iColumn]; }
} MATRIX3D, *LPDMATRIX3D;

/*
**-----------------------------------------------------------------------------
** Function Prototypes
**-----------------------------------------------------------------------------
*/

// generic simple matrix routines
MATRIX3D ZeroMatrix();
MATRIX3D IdentityMatrix();

MATRIX3D ProjectionMatrix(const float near_plane, const float far_plane, const float fov);
MATRIX3D ViewMatrixByDir(const Vect3f& from, 
						  const Vect3f& view_dir, 
						  const Vect3f& world_up, 
						  const Vect3f& cam_up);
MATRIX3D ViewMatrix(const Vect3f & from, const Vect3f & at, 
						const Vect3f & world_up, 
						const Vect3f& cam_up);

MATRIX3D RotateXMatrix(const float rads);
MATRIX3D RotateYMatrix(const float rads);
MATRIX3D RotateZMatrix(const float rads);
MATRIX3D TranslateMatrix(const float dx, const float dy, const float dz);
MATRIX3D TranslateMatrix(const Vect3f & v);
MATRIX3D ScaleMatrix(const float size);
MATRIX3D ScaleMatrix(const float a, const float b, const float c);
MATRIX3D ScaleMatrix(const Vect3f & v);

MATRIX3D MatrixMult(const MATRIX3D & a, const MATRIX3D & b);
MATRIX3D MatrixInverse(const MATRIX3D & m);
MATRIX3D MatrixTranspose(const MATRIX3D & m);

Vect3f TransformVector(const Vect3f & v, const MATRIX3D & m);
Vect3f TransformNormal(const Vect3f & v, const MATRIX3D & m);

/*
**-----------------------------------------------------------------------------
**	End of File
**-----------------------------------------------------------------------------
*/
#endif // D3DUTILS_H


