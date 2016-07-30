#pragma once


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#ifndef _MSC_VER
#include <stdint.h>
#else
typedef signed __int8     int8_t;
typedef signed __int16    int16_t;
typedef signed __int32    int32_t;
typedef signed __int64    int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;
#endif

#define endll endl << endl // double end line definition

class vector3f
{
public:

	vector3f() : x(0.0f), y(0.0f), z(0.0f) {};
	vector3f(float xf, float yf, float zf) : x(xf), y(yf), z(zf) {};
	static vector3f
		scalevec(vector3f& vector, float scalar)
	{
		vector3f rt = vector;
		rt.x *= scalar;
		rt.y *= scalar;
		rt.z *= scalar;
		return rt;
	}

	static vector3f
		normalize(vector3f& vector)
	{
		float dist = sqrtf(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
		if (dist > 1e-6)
		{
			vector.x /= dist;
			vector.y /= dist;
			vector.z /= dist;
		}
		return vector;
	}

	static float
		dot(vector3f v0, vector3f v1)
	{
		return v0.x*v1.x + v0.y*v1.y + v0.z*v1.z;
	}

	static vector3f
		cross(vector3f v0, vector3f v1)
	{
		vector3f rt;
		rt.x = v0.y*v1.z - v0.z*v1.y;
		rt.y = v0.z*v1.x - v0.x*v1.z;
		rt.z = v0.x*v1.y - v0.y*v1.x;
		return rt;
	}

	static vector3f
		subtract(vector3f v0, vector3f v1)
	{
		vector3f rt;
		rt.x = v1.x - v0.x;
		rt.y = v1.y - v0.y;
		rt.z = v1.z - v0.z;
		return rt;
	}

public:
	float x;
	float y;
	float z;

};

class Matrix {

public:

	// constructor / deconstructor
	Matrix();                                                  // init empty 0x0 matrix
	Matrix(const int32_t m, const int32_t n);                   // init empty mxn matrix
	Matrix(const int32_t m, const int32_t n, const double* val_); // init mxn matrix with values from array 'val'
	Matrix(const Matrix &M);                                   // creates deepcopy of M
	~Matrix();

	// assignment operator, copies contents of M
	Matrix& operator= (const Matrix &M);

	// copies submatrix of M into array 'val', default values copy whole row/column/matrix
	void getData(double* val_, int32_t i1 = 0, int32_t j1 = 0, int32_t i2 = -1, int32_t j2 = -1);

	// set or get submatrices of current matrix
	Matrix getMat(int32_t i1, int32_t j1, int32_t i2 = -1, int32_t j2 = -1);
	void   setMat(const Matrix &M, const int32_t i, const int32_t j);

	// set sub-matrix to scalar (default 0), -1 as end replaces whole row/column/matrix
	void setVal(double s, int32_t i1 = 0, int32_t j1 = 0, int32_t i2 = -1, int32_t j2 = -1);

	// set (part of) diagonal to scalar, -1 as end replaces whole diagonal
	void setDiag(double s, int32_t i1 = 0, int32_t i2 = -1);

	// clear matrix
	void zero();

	// extract columns with given index
	Matrix extractCols(std::vector<int> idx);

	// create identity matrix
	static Matrix eye(const int32_t m);
	void          eye();

	// create matrix with ones
	static Matrix ones(const int32_t m, const int32_t n);

	// create diagonal matrix with nx1 or 1xn matrix M as elements
	static Matrix diag(const Matrix &M);

	// returns the m-by-n matrix whose elements are taken column-wise from M
	static Matrix reshape(const Matrix &M, int32_t m, int32_t n);

	// create 3x3 rotation matrices (convention: http://en.wikipedia.org/wiki/Rotation_matrix)
	static Matrix rotMatX(const double &angle);
	static Matrix rotMatY(const double &angle);
	static Matrix rotMatZ(const double &angle);

	// simple arithmetic operations
	Matrix  operator+ (const Matrix &M); // add matrix
	Matrix  operator- (const Matrix &M); // subtract matrix
	Matrix  operator* (const Matrix &M); // multiply with matrix
	Matrix  operator* (const double &s);  // multiply with scalar
	Matrix  operator/ (const Matrix &M); // divide elementwise by matrix (or vector)
	Matrix  operator/ (const double &s);  // divide by scalar
	Matrix  operator- ();                // negative matrix
	Matrix  operator~ ();                // transpose
	double   l2norm();                   // euclidean norm (vectors) / frobenius norm (matrices)
	double   mean();                     // mean of all elements in matrix

	// complex arithmetic operations
	static Matrix cross(const Matrix &a, const Matrix &b);    // cross product of two vectors
	static Matrix inv(const Matrix &M);                       // invert matrix M
	bool   inv();                                             // invert this matrix
	double  det();                                             // returns determinant of matrix
	bool   solve(const Matrix &M, double eps = 1e-20);            // solve linear system M*x=B, replaces *this and M
	bool   lu(int32_t *idx, double &d, double eps = 1e-20);        // replace *this by lower upper decomposition
	void   svd(Matrix &U, Matrix &W, Matrix &V);                 // singular value decomposition *this = U*diag(W)*V^T

	// print matrix to stream
	friend std::ostream& operator<< (std::ostream& out, const Matrix& M);

	// direct data access
	double   **val;
	int32_t   m, n;

private:

	void allocateMemory(const int32_t m_, const int32_t n_);
	void releaseMemory();
	inline double pythag(double a, double b);

};
