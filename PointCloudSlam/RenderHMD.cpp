#define _VR 1
#include "RenderHMD.h"
#include <iostream>
#include <iomanip>
#include <GL/glew.h>
#include <GLFW/glfw3.h> 
#include <openvr.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <vector>
#include <cstdint>

#define PI (3.1415927f)

struct line_vertex_vive
{
	unsigned char r, g, b, a;
	float x, y, z;
};

line_vertex_vive g_lineVertices_vive[] =
{
	{ 255, 0, 0, 255, 0.0f, 0.0f, 0.0f },
	{ 255, 0, 0, 255, 1.0f, 0.0f, 0.0f },
	{ 0, 255, 0, 255, 0.0f, 0.0f, 0.0f },
	{ 0, 255, 0, 255, 0.0f, 1.0f, 0.0f },
	{ 0, 0, 255, 255, 0.0f, 0.0f, 0.0f },
	{ 0, 0, 255, 255, 0.0f, 0.0f, 1.0f }
};

class Vector3 {
public:
	float x, y, z;
	/** initializes to zero */
	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	explicit Vector3(const class Vector4&);

	float dot(const Vector3& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	Vector3 cross(const Vector3& other) const {
		return Vector3(y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x);
	}

	Vector3& operator+=(const Vector3& v) {
		x += v.x; y += v.y; z += v.z;
		return *this;
	}

	Vector3 operator+(const Vector3& v) const {
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3& operator-=(const Vector3& v) {
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}

	Vector3 operator-(const Vector3& v) const {
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator-() const {
		return Vector3(-x, -y, -z);
	}

	Vector3 operator*(float s) const {
		return Vector3(x * s, y * s, z * s);
	}

	Vector3 operator/(float s) const {
		return Vector3(x / s, y / s, z / s);
	}

	float length() const {
		return sqrt(x * x + y * y + z * z);
	}

	Vector3 normalize() const {
		return *this / length();
	}

	float& operator[](int i) {
		return (&x)[i];
	}

	float operator[](int i) const {
		return (&x)[i];
	}
};


class Vector4 {
public:
	float x, y, z, w;
	/** initializes to zero */
	Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	Vector4(const Vector3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
	float dot(const Vector4& other) const {
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}
	float& operator[](int i) {
		return (&x)[i];
	}
	float operator[](int i) const {
		return (&x)[i];
	}
};


Vector3::Vector3(const Vector4& v) : x(v.x), y(v.y), z(v.z) {}


class Matrix4x4 {
public:
	/** row-major */
	float data[16];

	/** row-major */
	Matrix4x4(float a, float b, float c, float d,
		float e, float f, float g, float h,
		float i, float j, float k, float l,
		float m, float n, float o, float p) {
		data[0] = a; data[1] = b; data[2] = c; data[3] = d;
		data[4] = e; data[5] = f; data[6] = g; data[7] = h;
		data[8] = i; data[9] = j; data[10] = k; data[11] = l;
		data[12] = m; data[13] = n; data[14] = o; data[15] = p;
	}

	/** initializes to the identity matrix */
	Matrix4x4() {
		memset(data, 0, sizeof(float) * 16);
		data[0] = data[5] = data[10] = data[15] = 1.0f;
	}

	Matrix4x4(const Matrix4x4& M) {
		memcpy(data, M.data, sizeof(float) * 16);
	}

	Matrix4x4& operator=(const Matrix4x4& M) {
		memcpy(data, M.data, sizeof(float) * 16);
		return *this;
	}

	static Matrix4x4 zero() {
		return Matrix4x4(0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f);
	}

	static Matrix4x4 roll(float radians) {
		const float c = cos(radians), s = sin(radians);
		return Matrix4x4(c, -s, 0.0f, 0.0f,
			s, c, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}

	static Matrix4x4 yaw(float radians) {
		const float c = cos(radians), s = sin(radians);
		return Matrix4x4(c, 0.0f, s, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			-s, 0.0f, c, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}

	static Matrix4x4 pitch(float radians) {
		const float c = cos(radians), s = sin(radians);
		return Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, c, -s, 0.0f,
			0.0f, s, c, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}

	static Matrix4x4 scale(float x, float y, float z) {
		return Matrix4x4(x, 0.0f, 0.0f, 0.0f,
			0.0f, y, 0.0f, 0.0f,
			0.0f, 0.0f, z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}

	static Matrix4x4 translate(float x, float y, float z) {
		return Matrix4x4(1.0f, 0.0f, 0.0f, x,
			0.0f, 1.0f, 0.0f, y,
			0.0f, 0.0f, 1.0f, z,
			0.0f, 0.0f, 0.0f, 1.0f);
	}

	static Matrix4x4 translate(const Vector3& v) {
		return translate(v.x, v.y, v.z);
	}

	/**
	Maps the view frustum to the cube [-1, +1]^3 in the OpenGL style.

	\param verticalRadians Vertical field of view from top to bottom
	\param nearZ Negative number
	\param farZ Negative number less than (higher magnitude than) nearZ. May be negative infinity
	*/
	static Matrix4x4 perspective(float pixelWidth, float pixelHeight, float nearZ, float farZ, float verticalRadians, float subpixelShiftX = 0.0f, float subpixelShiftY = 0.0f) {
		const float k = 1.0f / tan(verticalRadians / 2.0f);

		const float c = (farZ == -INFINITY) ? -1.0f : (nearZ + farZ) / (nearZ - farZ);
		const float d = (farZ == -INFINITY) ? 1.0f : farZ / (nearZ - farZ);

		Matrix4x4 P(k * pixelHeight / pixelWidth, 0.0f, subpixelShiftX * k / (nearZ * pixelWidth), 0.0f,
			0.0f, k, subpixelShiftY * k / (nearZ * pixelHeight), 0.0f,
			0.0f, 0.0f, c, -2.0f * nearZ * d,
			0.0f, 0.0f, -1.0f, 0.0f);

		return P;
	}

	/**
	Maps the view frustum to the cube [-1, +1]^3 in the OpenGL
	style by orthographic projection in which (0, 0) will become
	the top-left corner of the screen after the viewport is
	applied and (pixelWidth - 1, pixelHeight - 1) will be the
	lower-right corner.

	\param nearZ Negative number
	\param farZ Negative number less than (higher magnitude than) nearZ. Must be finite
	*/
	static Matrix4x4 ortho(float pixelWidth, float pixelHeight, float nearZ, float farZ) {
		return Matrix4x4(2.0f / pixelWidth, 0.0f, 0.0f, -1.0f,
			0.0f, -2.0f / pixelHeight, 0.0f, 1.0f,
			0.0f, 0.0f, -2.0f / (nearZ - farZ), (farZ + nearZ) / (nearZ - farZ),
			0.0f, 0.0f, 0.0f, 1.0f);
	}

	Matrix4x4 transpose() const {
		return Matrix4x4(data[0], data[4], data[8], data[12],
			data[1], data[5], data[9], data[13],
			data[2], data[6], data[10], data[14],
			data[3], data[7], data[11], data[15]);
	}

	/** Computes the inverse by Cramer's rule (based on MESA implementation) */
	Matrix4x4 inverse() const {
		Matrix4x4 result;
		const float* m = data;
		float* inv = result.data;

		inv[0] = m[5] * m[10] * m[15] -
			m[5] * m[11] * m[14] -
			m[9] * m[6] * m[15] +
			m[9] * m[7] * m[14] +
			m[13] * m[6] * m[11] -
			m[13] * m[7] * m[10];

		inv[4] = -m[4] * m[10] * m[15] +
			m[4] * m[11] * m[14] +
			m[8] * m[6] * m[15] -
			m[8] * m[7] * m[14] -
			m[12] * m[6] * m[11] +
			m[12] * m[7] * m[10];

		inv[8] = m[4] * m[9] * m[15] -
			m[4] * m[11] * m[13] -
			m[8] * m[5] * m[15] +
			m[8] * m[7] * m[13] +
			m[12] * m[5] * m[11] -
			m[12] * m[7] * m[9];

		inv[12] = -m[4] * m[9] * m[14] +
			m[4] * m[10] * m[13] +
			m[8] * m[5] * m[14] -
			m[8] * m[6] * m[13] -
			m[12] * m[5] * m[10] +
			m[12] * m[6] * m[9];

		inv[1] = -m[1] * m[10] * m[15] +
			m[1] * m[11] * m[14] +
			m[9] * m[2] * m[15] -
			m[9] * m[3] * m[14] -
			m[13] * m[2] * m[11] +
			m[13] * m[3] * m[10];

		inv[5] = m[0] * m[10] * m[15] -
			m[0] * m[11] * m[14] -
			m[8] * m[2] * m[15] +
			m[8] * m[3] * m[14] +
			m[12] * m[2] * m[11] -
			m[12] * m[3] * m[10];

		inv[9] = -m[0] * m[9] * m[15] +
			m[0] * m[11] * m[13] +
			m[8] * m[1] * m[15] -
			m[8] * m[3] * m[13] -
			m[12] * m[1] * m[11] +
			m[12] * m[3] * m[9];

		inv[13] = m[0] * m[9] * m[14] -
			m[0] * m[10] * m[13] -
			m[8] * m[1] * m[14] +
			m[8] * m[2] * m[13] +
			m[12] * m[1] * m[10] -
			m[12] * m[2] * m[9];

		inv[2] = m[1] * m[6] * m[15] -
			m[1] * m[7] * m[14] -
			m[5] * m[2] * m[15] +
			m[5] * m[3] * m[14] +
			m[13] * m[2] * m[7] -
			m[13] * m[3] * m[6];

		inv[6] = -m[0] * m[6] * m[15] +
			m[0] * m[7] * m[14] +
			m[4] * m[2] * m[15] -
			m[4] * m[3] * m[14] -
			m[12] * m[2] * m[7] +
			m[12] * m[3] * m[6];

		inv[10] = m[0] * m[5] * m[15] -
			m[0] * m[7] * m[13] -
			m[4] * m[1] * m[15] +
			m[4] * m[3] * m[13] +
			m[12] * m[1] * m[7] -
			m[12] * m[3] * m[5];

		inv[14] = -m[0] * m[5] * m[14] +
			m[0] * m[6] * m[13] +
			m[4] * m[1] * m[14] -
			m[4] * m[2] * m[13] -
			m[12] * m[1] * m[6] +
			m[12] * m[2] * m[5];

		inv[3] = -m[1] * m[6] * m[11] +
			m[1] * m[7] * m[10] +
			m[5] * m[2] * m[11] -
			m[5] * m[3] * m[10] -
			m[9] * m[2] * m[7] +
			m[9] * m[3] * m[6];

		inv[7] = m[0] * m[6] * m[11] -
			m[0] * m[7] * m[10] -
			m[4] * m[2] * m[11] +
			m[4] * m[3] * m[10] +
			m[8] * m[2] * m[7] -
			m[8] * m[3] * m[6];

		inv[11] = -m[0] * m[5] * m[11] +
			m[0] * m[7] * m[9] +
			m[4] * m[1] * m[11] -
			m[4] * m[3] * m[9] -
			m[8] * m[1] * m[7] +
			m[8] * m[3] * m[5];

		inv[15] = m[0] * m[5] * m[10] -
			m[0] * m[6] * m[9] -
			m[4] * m[1] * m[10] +
			m[4] * m[2] * m[9] +
			m[8] * m[1] * m[6] -
			m[8] * m[2] * m[5];

		float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
		return result / det;
	}

	float& operator()(int r, int c) {
		return data[r * 4 + c];
	}

	float operator()(int r, int c) const {
		return data[r * 4 + c];
	}

	Vector4 row(int r) const {
		const int i = r * 4;
		return Vector4(data[i], data[i + 1], data[i + 2], data[i + 3]);
	}

	Vector4 col(int c) const {
		return Vector4(data[c], data[c + 4], data[c + 8], data[c + 12]);
	}

	Matrix4x4 operator*(const Matrix4x4& B) const {
		Matrix4x4 D;
		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				D(r, c) = row(r).dot(B.col(c));
			}
		}
		return D;
	}

	Matrix4x4 operator*(const float s) const {
		Matrix4x4 D;
		for (int i = 0; i < 16; ++i) {
			D.data[i] = data[i] * s;
		}
		return D;
	}

	Matrix4x4 operator/(const float s) const {
		Matrix4x4 D;
		for (int i = 0; i < 16; ++i) {
			D.data[i] = data[i] / s;
		}
		return D;
	}

	Vector4 operator*(const Vector4& v) const {
		Vector4 d;
		for (int r = 0; r < 4; ++r) {
			d[r] = row(r).dot(v);
		}
		return d;
	}
};



class Matrix3x3 {
public:
	/** row-major */
	float data[9];

	/** row-major */
	Matrix3x3(float a, float b, float c,
		float d, float e, float f,
		float g, float h, float i) {
		data[0] = a; data[1] = b; data[2] = c;
		data[3] = d; data[4] = e; data[5] = f;
		data[6] = g; data[7] = h; data[8] = i;
	}

	/** Takes the upper-left 3x3 submatrix */
	Matrix3x3(const Matrix4x4& M) {
		data[0] = M.data[0]; data[1] = M.data[1]; data[2] = M.data[2];
		data[3] = M.data[4]; data[4] = M.data[5]; data[5] = M.data[6];
		data[6] = M.data[8]; data[7] = M.data[9]; data[8] = M.data[10];
	}

	/** initializes to the identity matrix */
	Matrix3x3() {
		memset(data, 0, sizeof(float) * 9);
		data[0] = data[4] = data[8] = 1.0f;
	}

	Matrix3x3(const Matrix3x3& M) {
		memcpy(data, M.data, sizeof(float) * 9);
	}

	Matrix3x3& operator=(const Matrix3x3& M) {
		memcpy(data, M.data, sizeof(float) * 9);
		return *this;
	}

	static Matrix3x3 zero() {
		return Matrix3x3(0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f);
	}

	static Matrix3x3 roll(float radians) {
		const float c = cos(radians), s = sin(radians);
		return Matrix3x3(c, -s, 0.0f,
			s, c, 0.0f,
			0.0f, 0.0f, 1.0f);
	}

	static Matrix3x3 yaw(float radians) {
		const float c = cos(radians), s = sin(radians);
		return Matrix3x3(c, 0.0f, s,
			0.0f, 1.0f, 0.0f,
			-s, 0.0f, c);
	}

	static Matrix3x3 pitch(float radians) {
		const float c = cos(radians), s = sin(radians);
		return Matrix3x3(1.0f, 0.0f, 0.0f,
			0.0f, c, -s,
			0.0f, s, c);
	}

	static Matrix3x3 scale(float x, float y, float z) {
		return Matrix3x3(x, 0.0f, 0.0f,
			0.0f, y, 0.0f,
			0.0f, 0.0f, z);
	}

	Matrix3x3 transpose() const {
		return Matrix3x3(data[0], data[3], data[6],
			data[1], data[4], data[7],
			data[2], data[5], data[8]);
	}

	/** Computes the inverse by Cramer's rule */
	Matrix3x3 inverse() const {
		const Matrix3x3& m(*this);
		const float det = m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) -
			m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) +
			m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0));

		return Matrix3x3(
			(m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) / det,
			(m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2)) / det,
			(m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)) / det,

			(m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2)) / det,
			(m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0)) / det,
			(m(1, 0) * m(0, 2) - m(0, 0) * m(1, 2)) / det,

			(m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) / det,
			(m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1)) / det,
			(m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) / det);
	}

	float& operator()(int r, int c) {
		return data[r * 3 + c];
	}

	float operator()(int r, int c) const {
		return data[r * 3 + c];
	}

	Vector3 row(int r) const {
		const int i = r * 3;
		return Vector3(data[i], data[i + 1], data[i + 2]);
	}

	Vector3 col(int c) const {
		return Vector3(data[c], data[c + 3], data[c + 6]);
	}

	Matrix3x3 operator*(const Matrix3x3& B) const {
		Matrix3x3 D;
		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 3; ++c) {
				D(r, c) = row(r).dot(B.col(c));
			}
		}
		return D;
	}

	Matrix3x3 operator*(const float s) const {
		Matrix3x3 D;
		for (int i = 0; i < 9; ++i) {
			D.data[i] = data[i] * s;
		}
		return D;
	}

	Matrix3x3 operator/(const float s) const {
		Matrix3x3 D;
		for (int i = 0; i < 9; ++i) {
			D.data[i] = data[i] / s;
		}
		return D;
	}

	Vector3 operator*(const Vector3& v) const {
		Vector3 d;
		for (int r = 0; r < 3; ++r) {
			d[r] = row(r).dot(v);
		}
		return d;
	}
};

std::ostream& operator<<(std::ostream& os, const Vector3& v) {
	return os << "Vector3(" << std::setprecision(2) << v.x << ", " << v.y << ")";
}


std::ostream& operator<<(std::ostream& os, const Vector4& v) {
	return os << "Vector4(" << v.x << ", " << v.y << ", " << v.z << ")";
}


std::ostream& operator<<(std::ostream& os, const Matrix4x4& M) {
	os << "\nMatrix4x4(";

	for (int r = 0, i = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c, ++i) {
			os << M.data[i];
			if (c < 3) { os << ", "; }
		}
		if (r < 3) { os << ",\n          "; }
	}

	return os << ")\n";
}


std::ostream& operator<<(std::ostream& os, const Matrix3x3& M) {
	os << "\nMatrix3x3(";

	for (int r = 0, i = 0; r < 3; ++r) {
		for (int c = 0; c < 3; ++c, ++i) {
			os << M.data[i];
			if (c < 2) { os << ", "; }
		}
		if (r < 2) { os << ",\n          "; }
	}

	return os << ")\n";
}

void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if ((type == GL_DEBUG_TYPE_ERROR) || (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)) {
		fprintf(stderr, "GL Debug: %s\n", message);
	}
}


GLFWwindow* initOpenGL(int width, int height, const std::string& title) {
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW\n");
		::exit(1);
	}

	// Without these, shaders actually won't initialize properly
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

#   ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#   endif

	GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW\n");
		glfwTerminate();
		::exit(2);
	}
	glfwMakeContextCurrent(window);

	// Start GLEW extension handler, with improved support for new features
	glewExperimental = GL_TRUE;
	glewInit();

	// Clear startup errors
	while (glGetError() != GL_NONE) {}

#   ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glEnable(GL_DEBUG_OUTPUT);
#       ifndef _OSX
	// Causes a segmentation fault on OS X
	glDebugMessageCallback(debugCallback, nullptr);
#       endif
#   endif

	// Negative numbers allow buffer swaps even if they are after the vertical retrace,
	// but that causes stuttering in VR mode
	glfwSwapInterval(0);

	fprintf(stderr, "GPU: %s (OpenGL version %s)\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

	// Bind a single global vertex array (done this way since OpenGL 3)
	{ GLuint vao; glGenVertexArrays(1, &vao); glBindVertexArray(vao); }

	// Check for errors
	{ const GLenum error = glGetError(); assert(error == GL_NONE); }

	return window;
}


std::string loadTextFile(const std::string& filename) {
	std::stringstream buffer;
	buffer << std::ifstream(filename.c_str()).rdbuf();
	return buffer.str();
}


GLuint compileShaderStage(GLenum stage, const std::string& source) {
	GLuint shader = glCreateShader(stage);
	const char* srcArray[] = { source.c_str() };

	glShaderSource(shader, 1, srcArray, NULL);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE) {
		GLint logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

		std::vector<GLchar> errorLog(logSize);
		glGetShaderInfoLog(shader, logSize, &logSize, &errorLog[0]);

		fprintf(stderr, "Error while compiling\n %s\n\nError: %s\n", source.c_str(), &errorLog[0]);
		assert(false);

		glDeleteShader(shader);
		shader = GL_NONE;
	}

	return shader;
}

GLuint createShaderProgram(const std::string& vertexShaderSource, const std::string& pixelShaderSource) {
	GLuint shader = glCreateProgram();

	glAttachShader(shader, compileShaderStage(GL_VERTEX_SHADER, vertexShaderSource));
	glAttachShader(shader, compileShaderStage(GL_FRAGMENT_SHADER, pixelShaderSource));
	glLinkProgram(shader);

	return shader;
}

namespace Cube {
	const float position[][3] = { -.5f, .5f, -.5f, -.5f, .5f, .5f, .5f, .5f, .5f, .5f, .5f, -.5f, -.5f, .5f, -.5f, -.5f, -.5f, -.5f, -.5f, -.5f, .5f, -.5f, .5f, .5f, .5f, .5f, .5f, .5f, -.5f, .5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f, -.5f, .5f, -.5f, -.5f, -.5f, -.5f, -.5f, -.5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, -.5f, .5f, .5f, -.5f, .5f, .5f, .5f, .5f, -.5f, -.5f, .5f, -.5f, -.5f, -.5f, .5f, -.5f, -.5f, .5f, -.5f, .5f };
	const float normal[][3] = { 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f };
	const float tangent[][4] = { 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 0.f, -1.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f, 1.f };
	const float texCoord[][2] = { 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f };
	const int   index[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23 };
};

/** Loads a 24- or 32-bit BMP file into memory */
void loadBMP(const std::string& filename, int& width, int& height, int& channels, std::vector<std::uint8_t>& data) {
	std::fstream hFile(filename.c_str(), std::ios::in | std::ios::binary);
	if (!hFile.is_open()) { throw std::invalid_argument("Error: File Not Found."); }

	hFile.seekg(0, std::ios::end);
	size_t len = hFile.tellg();
	hFile.seekg(0, std::ios::beg);
	std::vector<std::uint8_t> header(len);
	hFile.read(reinterpret_cast<char*>(header.data()), 54);

	if ((header[0] != 'B') && (header[1] != 'M')) {
		hFile.close();
		throw std::invalid_argument("Error: File is not a BMP.");
	}

	if ((header[28] != 24) && (header[28] != 32)) {
		hFile.close();
		throw std::invalid_argument("Error: File is not uncompressed 24 or 32 bits per pixel.");
	}

	const short bitsPerPixel = header[28];
	channels = bitsPerPixel / 8;
	width = header[18] + (header[19] << 8);
	height = header[22] + (header[23] << 8);
	std::uint32_t offset = header[10] + (header[11] << 8);
	std::uint32_t size = ((width * bitsPerPixel + 31) / 32) * 4 * height;
	data.resize(size);

	hFile.seekg(offset, std::ios::beg);
	hFile.read(reinterpret_cast<char*>(data.data()), size);
	hFile.close();

	// Flip the y axis
	std::vector<std::uint8_t> tmp;
	const size_t rowBytes = width * channels;
	tmp.resize(rowBytes);
	for (int i = height / 2 - 1; i >= 0; --i) {
		const int j = height - 1 - i;
		// Swap the rows
		memcpy(tmp.data(), &data[i * rowBytes], rowBytes);
		memcpy(&data[i * rowBytes], &data[j * rowBytes], rowBytes);
		memcpy(&data[j * rowBytes], tmp.data(), rowBytes);
	}

	// Convert BGR[A] format to RGB[A] format
	if (channels == 4) {
		// BGRA
		std::uint32_t* p = reinterpret_cast<std::uint32_t*>(data.data());
		for (int i = width * height - 1; i >= 0; --i) {
			const unsigned int x = p[i];
			p[i] = ((x >> 24) & 0xFF) | (((x >> 16) & 0xFF) << 8) | (((x >> 8) & 0xFF) << 16) | ((x & 0xFF) << 24);
		}
	}
	else {
		// BGR
		for (int i = (width * height - 1) * 3; i >= 0; i -= 3) {
			std::swap(data[i], data[i + 2]);
		}
	}
}



/** Called by initOpenVR */
std::string getHMDString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = nullptr) {
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, peError);
	if (unRequiredBufferLen == 0) {
		return "";
	}

	char* pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;

	return sResult;
}


/** Call immediately before initializing OpenGL

\param hmdWidth, hmdHeight recommended render target resolution
*/
vr::IVRSystem* initOpenVR(uint32_t& hmdWidth, uint32_t& hmdHeight) {
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem* hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None) {
		fprintf(stderr, "OpenVR Initialization Error: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return nullptr;
	}

	const std::string& driver = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	const std::string& model = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String);
	const std::string& serial = getHMDString(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
	const float freq = hmd->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);

	//get the proper resolution of the hmd
	hmd->GetRecommendedRenderTargetSize(&hmdWidth, &hmdHeight);

	fprintf(stderr, "HMD: %s '%s' #%s (%d x %d @ %g Hz)\n", driver.c_str(), model.c_str(), serial.c_str(), hmdWidth, hmdHeight, freq);

	// Initialize the compositor
	vr::IVRCompositor* compositor = vr::VRCompositor();
	if (!compositor) {
		fprintf(stderr, "OpenVR Compositor initialization failed. See log file for details\n");
		vr::VR_Shutdown();
		assert("VR failed" && false);
	}

	return hmd;
}

/**
*/
void getEyeTransformations
(vr::IVRSystem*  hmd,
vr::TrackedDevicePose_t* trackedDevicePose,
float           nearPlaneZ,
float           farPlaneZ,
float*          headToWorldRowMajor3x4,
float*          ltEyeToHeadRowMajor3x4,
float*          rtEyeToHeadRowMajor3x4,
float*          ltProjectionMatrixRowMajor4x4,
float*          rtProjectionMatrixRowMajor4x4) {

	assert(nearPlaneZ < 0.0f && farPlaneZ < nearPlaneZ);

	vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

#   if defined(_DEBUG) && 0
	fprintf(stderr, "Devices tracked this frame: \n");
	int poseCount = 0;
	for (int d = 0; d < vr::k_unMaxTrackedDeviceCount; ++d)	{
		if (trackedDevicePose[d].bPoseIsValid) {
			++poseCount;
			switch (hmd->GetTrackedDeviceClass(d)) {
			case vr::TrackedDeviceClass_Controller:        fprintf(stderr, "   Controller: ["); break;
			case vr::TrackedDeviceClass_HMD:               fprintf(stderr, "   HMD: ["); break;
			case vr::TrackedDeviceClass_Invalid:           fprintf(stderr, "   <invalid>: ["); break;
			case vr::TrackedDeviceClass_Other:             fprintf(stderr, "   Other: ["); break;
			case vr::TrackedDeviceClass_TrackingReference: fprintf(stderr, "   Reference: ["); break;
			default:                                       fprintf(stderr, "   ???: ["); break;
			}
			for (int r = 0; r < 3; ++r) {
				for (int c = 0; c < 4; ++c) {
					fprintf(stderr, "%g, ", trackedDevicePose[d].mDeviceToAbsoluteTracking.m[r][c]);
				}
			}
			fprintf(stderr, "]\n");
		}
	}
	fprintf(stderr, "\n");
#   endif

	assert(trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid);
	const vr::HmdMatrix34_t head = trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;

	const vr::HmdMatrix34_t& ltMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Left);
	const vr::HmdMatrix34_t& rtMatrix = hmd->GetEyeToHeadTransform(vr::Eye_Right);

	for (int r = 0; r < 3; ++r) {
		for (int c = 0; c < 4; ++c) {
			ltEyeToHeadRowMajor3x4[r * 4 + c] = ltMatrix.m[r][c];
			rtEyeToHeadRowMajor3x4[r * 4 + c] = rtMatrix.m[r][c];
			headToWorldRowMajor3x4[r * 4 + c] = head.m[r][c];
		}
	}

	const vr::HmdMatrix44_t& ltProj = hmd->GetProjectionMatrix(vr::Eye_Left, -nearPlaneZ, -farPlaneZ, vr::API_OpenGL);
	const vr::HmdMatrix44_t& rtProj = hmd->GetProjectionMatrix(vr::Eye_Right, -nearPlaneZ, -farPlaneZ, vr::API_OpenGL);

	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			ltProjectionMatrixRowMajor4x4[r * 4 + c] = ltProj.m[r][c];
			rtProjectionMatrixRowMajor4x4[r * 4 + c] = rtProj.m[r][c];
		}
	}
}

/** Call immediately before OpenGL swap buffers */
void submitToHMD(GLint ltEyeTexture, GLint rtEyeTexture, bool isGammaEncoded) {
	const vr::EColorSpace colorSpace = isGammaEncoded ? vr::ColorSpace_Gamma : vr::ColorSpace_Linear;

	const vr::Texture_t lt = { reinterpret_cast<void*>(intptr_t(ltEyeTexture)), vr::API_OpenGL, colorSpace };
	vr::VRCompositor()->Submit(vr::Eye_Left, &lt);

	const vr::Texture_t rt = { reinterpret_cast<void*>(intptr_t(rtEyeTexture)), vr::API_OpenGL, colorSpace };
	vr::VRCompositor()->Submit(vr::Eye_Right, &rt);

	// Tell the compositor to begin work immediately instead of waiting for the next WaitGetPoses() call
	vr::VRCompositor()->PostPresentHandoff();
}

GLFWwindow* window = nullptr;

#ifdef _VR
vr::IVRSystem* hmd = nullptr;
#endif

#ifndef Shape
#   define Shape Cube
#endif

void render_hmd() {
	std::cout << "Minimal OpenGL 4.1 Example by Morgan McGuire\n\nW, A, S, D, C, Z keys to translate\nMouse click and drag to rotate\nESC to quit\n\n";
	std::cout << std::fixed;

	uint32_t framebufferWidth = 1280, framebufferHeight = 720;
#   ifdef _VR
	const int numEyes = 2;
	hmd = initOpenVR(framebufferWidth, framebufferHeight);
	assert(hmd);
#   else
	const int numEyes = 1;
#   endif

	const int windowHeight = 720;
	const int windowWidth = (framebufferWidth * windowHeight) / framebufferHeight;

	window = initOpenGL(windowWidth, windowHeight, "minimalOpenGL");

	Vector3 bodyTranslation(0.0f, 1.6f, 5.0f);
	Vector3 bodyRotation;

	//////////////////////////////////////////////////////////////////////
	// Allocate the frame buffer. This code allocates one framebuffer per eye.
	// That requires more GPU memory, but is useful when performing temporal 
	// filtering or making render calls that can target both simultaneously.

	GLuint framebuffer[numEyes];
	glGenFramebuffers(numEyes, framebuffer);

	GLuint colorRenderTarget[numEyes], depthRenderTarget[numEyes];
	glGenTextures(numEyes, colorRenderTarget);
	glGenTextures(numEyes, depthRenderTarget);
	for (int eye = 0; eye < numEyes; ++eye) {
		glBindTexture(GL_TEXTURE_2D, colorRenderTarget[eye]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_2D, depthRenderTarget[eye]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, framebufferWidth, framebufferHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[eye]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorRenderTarget[eye], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthRenderTarget[eye], 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/////////////////////////////////////////////////////////////////
	// Load vertex array buffers

	GLuint positionBuffer = GL_NONE;
	glGenBuffers(1, &positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Shape::position), Shape::position, GL_STATIC_DRAW);

	GLuint texCoordBuffer = GL_NONE;
	glGenBuffers(1, &texCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Shape::texCoord), Shape::texCoord, GL_STATIC_DRAW);

	GLuint normalBuffer = GL_NONE;
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Shape::normal), Shape::normal, GL_STATIC_DRAW);

	GLuint tangentBuffer = GL_NONE;
	glGenBuffers(1, &tangentBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Shape::tangent), Shape::tangent, GL_STATIC_DRAW);

	const int numVertices = sizeof(Shape::position) / sizeof(Shape::position[0]);

	GLuint indexBuffer = GL_NONE;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Shape::index), Shape::index, GL_STATIC_DRAW);
	const int numIndices = sizeof(Shape::index) / sizeof(Shape::index[0]);

	/////////////////////////////////////////////////////////////////////
	// Create the main shader
	const GLuint shader = createShaderProgram(loadTextFile("min.vrt"), loadTextFile("min.pix"));

	// Binding points for attributes and uniforms discovered from the shader
	const GLint positionAttribute = glGetAttribLocation(shader, "position");
	const GLint normalAttribute = glGetAttribLocation(shader, "normal");
	const GLint texCoordAttribute = glGetAttribLocation(shader, "texCoord");
	const GLint tangentAttribute = glGetAttribLocation(shader, "tangent");
	const GLint colorTextureUniform = glGetUniformLocation(shader, "colorTexture");

	const GLuint uniformBlockIndex = glGetUniformBlockIndex(shader, "Uniform");
	const GLuint uniformBindingPoint = 1;
	glUniformBlockBinding(shader, uniformBlockIndex, uniformBindingPoint);

	GLuint uniformBlock;
	glGenBuffers(1, &uniformBlock);

	{
		// Allocate space for the uniform block buffer
		GLint uniformBlockSize;
		glGetActiveUniformBlockiv(shader, uniformBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockSize);
		glBindBuffer(GL_UNIFORM_BUFFER, uniformBlock);
		glBufferData(GL_UNIFORM_BUFFER, uniformBlockSize, nullptr, GL_DYNAMIC_DRAW);
	}

	const GLchar* uniformName[] = {
		"Uniform.objectToWorldNormalMatrix",
		"Uniform.objectToWorldMatrix",
		"Uniform.modelViewProjectionMatrix",
		"Uniform.light",
		"Uniform.cameraPosition" };

	const int numBlockUniforms = sizeof(uniformName) / sizeof(uniformName[0]);
#   ifdef _DEBUG
	{
		GLint debugNumUniforms = 0;
		glGetProgramiv(shader, GL_ACTIVE_UNIFORMS, &debugNumUniforms);
		for (GLint i = 0; i < debugNumUniforms; ++i) {
			GLchar name[1024];
			GLsizei size = 0;
			GLenum type = GL_NONE;
			glGetActiveUniform(shader, i, sizeof(name), nullptr, &size, &type, name);
			std::cout << "Uniform #" << i << ": " << name << "\n";
		}
		assert(debugNumUniforms >= numBlockUniforms);
	}
#   endif

	// Map uniform names to indices within the block
	GLuint uniformIndex[numBlockUniforms];
	glGetUniformIndices(shader, numBlockUniforms, uniformName, uniformIndex);
	assert(uniformIndex[0] < 10000);

	// Map indices to byte offsets
	GLint  uniformOffset[numBlockUniforms];
	glGetActiveUniformsiv(shader, numBlockUniforms, uniformIndex, GL_UNIFORM_OFFSET, uniformOffset);
	assert(uniformOffset[0] >= 0);

	// Load a texture map
	GLuint colorTexture = GL_NONE;
	{
		int textureWidth, textureHeight, channels;
		std::vector<std::uint8_t> data;
		loadBMP("color.bmp", textureWidth, textureHeight, channels, data);

		glGenTextures(1, &colorTexture);
		glBindTexture(GL_TEXTURE_2D, colorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, textureWidth, textureHeight, 0, (channels == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	GLuint trilinearSampler = GL_NONE;
	{
		glGenSamplers(1, &trilinearSampler);
		glSamplerParameteri(trilinearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(trilinearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(trilinearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(trilinearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

#   ifdef _VR
	vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
#   endif

	// Main loop:
	int timer = 0;
	while (!glfwWindowShouldClose(window)) {
		assert(glGetError() == GL_NONE);

		const float nearPlaneZ = -0.1f;
		const float farPlaneZ = -100.0f;
		const float verticalFieldOfView = 45.0f * PI / 180.0f;

		Matrix4x4 eyeToHead[numEyes], projectionMatrix[numEyes], headToBodyMatrix;
#       ifdef _VR
		getEyeTransformations(hmd, trackedDevicePose, nearPlaneZ, farPlaneZ, headToBodyMatrix.data, eyeToHead[0].data, eyeToHead[1].data, projectionMatrix[0].data, projectionMatrix[1].data);
#       else
		projectionMatrix[0] = Matrix4x4::perspective(float(framebufferWidth), float(framebufferHeight), nearPlaneZ, farPlaneZ, verticalFieldOfView);
#       endif

		// printf("float nearPlaneZ = %f, farPlaneZ = %f; int width = %d, height = %d;\n", nearPlaneZ, farPlaneZ, framebufferWidth, framebufferHeight);

		const Matrix4x4& bodyToWorldMatrix =
			Matrix4x4::translate(bodyTranslation) *
			Matrix4x4::roll(bodyRotation.z) *
			Matrix4x4::yaw(bodyRotation.y) *
			Matrix4x4::pitch(bodyRotation.x);

		const Matrix4x4& headToWorldMatrix = bodyToWorldMatrix * headToBodyMatrix;

		for (int eye = 0; eye < numEyes; ++eye) {
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer[eye]);
			glViewport(0, 0, framebufferWidth, framebufferHeight);

			glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			const Matrix4x4& objectToWorldMatrix = Matrix4x4::translate(0.0f, 0.5f, 0.0f) * Matrix4x4::yaw(PI / 3.0f);
			const Matrix3x3& objectToWorldNormalMatrix = Matrix3x3(objectToWorldMatrix).transpose().inverse();
			const Matrix4x4& cameraToWorldMatrix = headToWorldMatrix * eyeToHead[eye];

			const Vector3& light = Vector3(1.0f, 0.5f, 0.2f).normalize();

			// Draw the background
			//drawSky(framebufferWidth, framebufferHeight, nearPlaneZ, farPlaneZ, cameraToWorldMatrix.data, projectionMatrix[eye].inverse().data, &light.x);

			//gluPerspective(60, framebufferWidth / framebufferHeight, 0.1, 100);
			//glTranslatef(0.0f, -1.0f, -5.0f);
			//glRotatef(0.0f, 1.0f, 0.0f, 0.0f);
			//glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
			//glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
			//glInterleavedArrays(GL_C4UB_V3F, 0, g_lineVertices_vive);
			//glDrawArrays(GL_LINES, 0, 6);

			////////////////////////////////////////////////////////////////////////
			// Draw a mesh
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glEnable(GL_CULL_FACE);
			glDepthMask(GL_TRUE);

			glUseProgram(shader);

			// in position
			glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
			glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(positionAttribute);

			// in normal
			glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
			glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(normalAttribute);

			// in tangent
			if (tangentAttribute != -1) {
				// Only bind if used
				glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
				glVertexAttribPointer(tangentAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(tangentAttribute);
			}

			// in texCoord 
			glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
			glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(texCoordAttribute);

			// indexBuffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

			// uniform colorTexture (samplers cannot be placed in blocks)
			const GLint colorTextureUnit = 0;
			glActiveTexture(GL_TEXTURE0 + colorTextureUnit);
			glBindTexture(GL_TEXTURE_2D, colorTexture);
			glBindSampler(colorTextureUnit, trilinearSampler);
			glUniform1i(colorTextureUniform, colorTextureUnit);

			// Other uniforms in the interface block
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, uniformBindingPoint, uniformBlock);

				GLubyte* ptr = (GLubyte*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
				// mat3 is passed to openGL as if it was mat4 due to padding rules.
				for (int row = 0; row < 3; ++row) {
					memcpy(ptr + uniformOffset[0] + sizeof(float) * 4 * row, objectToWorldNormalMatrix.data + row * 3, sizeof(float) * 3);
				}

				memcpy(ptr + uniformOffset[1], objectToWorldMatrix.data, sizeof(objectToWorldMatrix));

				const Matrix4x4& modelViewProjectionMatrix = projectionMatrix[eye] * cameraToWorldMatrix.inverse() * objectToWorldMatrix;
				memcpy(ptr + uniformOffset[2], modelViewProjectionMatrix.data, sizeof(modelViewProjectionMatrix));
				memcpy(ptr + uniformOffset[3], &light.x, sizeof(light));
				const Vector4& cameraPosition = cameraToWorldMatrix.col(3);
				memcpy(ptr + uniformOffset[4], &cameraPosition.x, sizeof(Vector3));
				glUnmapBuffer(GL_UNIFORM_BUFFER);
			}
			glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

			const Matrix4x4& modelViewProjectionMatrix = projectionMatrix[eye] * cameraToWorldMatrix.inverse() * objectToWorldMatrix;
			const Vector4& cameraPosition = cameraToWorldMatrix.col(3);

			
#           ifdef _VR
			{
				const vr::Texture_t tex = { reinterpret_cast<void*>(intptr_t(colorRenderTarget[eye])), vr::API_OpenGL, vr::ColorSpace_Gamma };
				vr::VRCompositor()->Submit(vr::EVREye(eye), &tex);
			}
#           endif
		} // for each eye

		////////////////////////////////////////////////////////////////////////
#       ifdef _VR
		// Tell the compositor to begin work immediately instead of waiting for the next WaitGetPoses() call
		vr::VRCompositor()->PostPresentHandoff();
#       endif

		// Mirror to the window
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
		glViewport(0, 0, windowWidth, windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBlitFramebuffer(0, 0, framebufferWidth, framebufferHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NONE);

		// Display what has been drawn on the main window
		glfwSwapBuffers(window);

		// Check for events
		glfwPollEvents();

		// Handle events
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
		}

		// WASD keyboard movement
		const float cameraMoveSpeed = 0.01f;
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(0, 0, -cameraMoveSpeed, 0)); }
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(0, 0, +cameraMoveSpeed, 0)); }
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(-cameraMoveSpeed, 0, 0, 0)); }
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)) { bodyTranslation += Vector3(headToWorldMatrix * Vector4(+cameraMoveSpeed, 0, 0, 0)); }
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_C)) { bodyTranslation.y -= cameraMoveSpeed; }
		if ((GLFW_PRESS == glfwGetKey(window, GLFW_KEY_SPACE)) || (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Z))) { bodyTranslation.y += cameraMoveSpeed; }

		// Keep the camera above the ground
		if (bodyTranslation.y < 0.01f) { bodyTranslation.y = 0.01f; }

		static bool inDrag = false;
		const float cameraTurnSpeed = 0.005f;
		if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
			static double startX, startY;
			double currentX, currentY;

			glfwGetCursorPos(window, &currentX, &currentY);
			if (inDrag) {
				bodyRotation.y -= float(currentX - startX) * cameraTurnSpeed;
				bodyRotation.x -= float(currentY - startY) * cameraTurnSpeed;
			}
			inDrag = true; startX = currentX; startY = currentY;
		}
		else {
			inDrag = false;
		}

		++timer;
	}

#   ifdef _VR
	if (hmd != nullptr) {
		vr::VR_Shutdown();
	}
#   endif

	// Close the GL context and release all resources
	glfwTerminate();
}