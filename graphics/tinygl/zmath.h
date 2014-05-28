#ifndef GRAPHICS_TINYGL_ZMATH_H
#define GRAPHICS_TINYGL_ZMATH_H

namespace TinyGL {

// Matrix & Vertex
class Vector3 {
public:
	Vector3();
	Vector3(const Vector3 &other);
	Vector3(float x, float y, float z);
	
	float getX() const { return _v[0]; }
	float getY() const { return _v[1]; }
	float getZ() const { return _v[2]; }

	void setX(float val) { _v[0] = val; }
	void setY(float val) { _v[1] = val; }
	void setZ(float val) { _v[2] = val; }

	void normalize();

	Vector3& operator=(const Vector3 &other)
	{
		memcpy(_v,other._v,sizeof(_v));
		return *this;
	}

	Vector3 operator-() const
	{
		return Vector3(-_v[0],-_v[1],_v[2]);
	}

	Vector3 operator*(float factor) const {
		return Vector3(_v[0] * factor, _v[1] * factor, _v[2] * factor);
	}

	Vector3 operator+(const Vector3 &other) const {
		return Vector3(_v[0] + other._v[0], _v[1] + other._v[1], _v[2] + other._v[2]);
	}

	Vector3 operator-(const Vector3 &other) const {
		return Vector3(_v[0] - other._v[0], _v[1] - other._v[1], _v[2] - other._v[2]);
	}

	Vector3& operator*=(float factor) {
		_v[0] *= factor;
		_v[1] *= factor;
		_v[2] *= factor;
		return *this;
	}

	Vector3& operator+=(float factor) {
		_v[0] += factor;
		_v[1] += factor;
		_v[2] += factor;
		return *this;
	}

	Vector3& operator-=(float factor) {
		_v[0] -= factor;
		_v[1] -= factor;
		_v[2] -= factor;
		return *this;
	}
private:
	float _v[3];
};

class Vector4 {
public:
	Vector4();
	Vector4(float x, float y, float z, float w);

	float getX() const { return _v[0]; }
	float getY() const { return _v[1]; }
	float getZ() const { return _v[2]; }
	float getW() const { return _v[3]; }

	void setX(float val) { _v[0] = val; }
	void setY(float val) { _v[1] = val; }
	void setZ(float val) { _v[2] = val; }
	void setW(float val) { _v[3] = val; }


private:
	float _v[4];
};

class Matrix4 {
public:
	Matrix4();
	Matrix4(const Matrix4 &other);

	bool IsIdentity() const;

	Matrix4& operator=(const Matrix4 &other)
	{
		memcpy(_m,other._m,sizeof(_m));
		return *this;
	}
	
	Matrix4 operator+(const Matrix4& b) const
	{
		Matrix4 result;
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				result.set(i,j, get(i,j) + b.get(i,j));
			}
		}
		return result;
	}

	Matrix4 operator-(const Matrix4& b) const
	{
		Matrix4 result;
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				result.set(i,j, get(i,j) - b.get(i,j));
			}
		}
		return result;
	}

	Matrix4 operator*(const Matrix4 &b) const
	{
		Matrix4 result;
		float s;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				s = 0.0;
				for (int k = 0; k < 4; k++)
					s += this->get(i,k) * b.get(k,j);
				result.set(i,j,s);
			}
		}
		return result;
	}

	static Matrix4 identity();

	void fill(float val) {
		for(int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				_m[i][j] = val;
			}
		}
	}

	inline void set(int x,int y,float val) {
		_m[x][y] = val;
	}

	inline float get(int x, int y) const {
		return _m[x][y];
	}

	Matrix4 transpose() const;
	Matrix4 inverseOrtho() const;
	Matrix4 inverse() const;
	Matrix4 rotation(float t, int u) const;

	Vector3 transform(const Vector3 &vector) const;
	Vector3 transform3x3(const Vector3 &vector) const; // Transform the vector as if this were a 3x3 matrix.
	Vector4 transform(const Vector4 &vector) const;
private:
	float _m[4][4];
};

struct M4 {
	float m[4][4];
};


struct M3 {
    float m[3][3];
};

struct M34 {
    float m[3][4];
};


#define X v[0]
#define Y v[1]
#define Z v[2]
#define W v[3]

struct V3 {
	float v[3];
};

struct V4 {
	float v[4];
};

void gl_M4_Id(M4 *a); // Done
int gl_M4_IsId(const M4 *a); // Done
void gl_M4_Move(M4 *a, const M4 *b); // Done (= operator)
void gl_MoveV3(V3 *a, const V3 *b); // Done (= operator)
void gl_MulM4V3(V3 *a, const M4 *b, const V3 *c); // Done
void gl_MulM3V3(V3 *a, const M4 *b, const V3 *c); // Done

void gl_M4_MulV4(V4 *a, const M4 *b, const V4 *c); // Done
void gl_M4_InvOrtho(M4 *a, const M4 &b); // Done
void gl_M4_Inv(M4 *a, const M4 *b); // Done
void gl_M4_Mul(M4 *c, const M4 *a, const M4 *b); // Done
void gl_M4_MulLeft(M4 *c, const M4 *a); // Done
void gl_M4_Transpose(M4 *a, const M4 *b); // Done
void gl_M4_Rotate(M4 *c, float t, int u); // Done
int gl_V3_Norm(V3 *a); // Done

V3 gl_V3_New(float x, float y, float z); // Done
V4 gl_V4_New(float x, float y, float z, float w); // Done

int gl_Matrix_Inv(float *r, float *m, int n); // Only for internal use - could be removed.

} // end of namespace TinyGL

#endif
