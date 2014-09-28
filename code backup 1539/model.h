#pragma once
#include <math.h>
#include <string>
#ifndef PI
#define PI (3.14159265358979323)
#define ML_PI PI
#define TAU 6.28318530718
#define sign(a) (a>0?1:a<0?-1:0)
#endif

class vec3{
public:
	float x,y,z;
	vec3();
	void rotateXZ(float theta);
	float specdot(vec3 v);
	float trad(vec3 v);
	vec3(float x, float y, float z);
	void operator+=(vec3 o);
	void operator-=(vec3 o);
	void operator/=(vec3 o);
	void operator*=(vec3 o);
	void operator/=(float o);
	void operator*=(float o);
	vec3 operator+(vec3 o);
	vec3 operator-(vec3 o);
	vec3 operator/(vec3 o);
	vec3 operator/(float o);
	vec3 operator*(vec3 o);
	vec3 operator*(float o);
	bool operator==(vec3 o) const;
	void operator=(vec3 v);
	bool operator==(vec3 v);
	bool operator!=(vec3 v);
	vec3 getsign();
	float length();
	float lengthsq();
	void normalize();
	vec3 normalized();
	bool isbetween(float a, float b, float num);
	bool isbetween(vec3 a, vec3 b, bool issigned);
	bool isnormalized();
	bool sameVector(vec3 v);
	float dot(vec3 v);
	vec3 cross(vec3 v);
	float distancesq(vec3 b);
	float distance(vec3 b);
	const char* toString();
	void print();
};
class vec2{
public:
	float x, y;
	vec2(float x, float y);
	vec2();
	void operator=(vec2 v);
	bool operator==(vec2 v);
	bool operator!=(vec2 v);
	bool operator==(vec2 v) const{
		return (x==v.x&&y==v.y);
	};
};