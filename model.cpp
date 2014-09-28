#include "stdafx.h"
#include <math.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <string.h>
#include <unordered_set>
#include "model.h"

	vec3::vec3(){
		x=0;y=0;z=0;
	}
	vec3::vec3(float xx, float yy, float zz){x=xx;y=yy;z=zz;}
	void vec3::operator+=(vec3 v){
		x+=v.x;
		y+=v.y;
		z+=v.z;
	}
	void vec3::print(){
		std::cout<<"("<<x<<", "<<y<<", "<<z<<")";
	}
	const char* vec3::toString(){
		char str[18];
		sprintf_s(str,"(%.3f,%.3f,%.3f)",x,y,z);
		return (const char*)str;
	}
	void vec3::operator-=(vec3 v){
		x+=-v.x;
		y+=-v.y;
		z+=-v.z;
	}
	void vec3::operator*=(float v){
		x*=v;
		y*=v;
		z*=v;
	}
	void vec3::operator/=(float v){
		x/=v;
		y/=v;
		z/=v;
	}
	vec3 vec3::operator+(vec3 v){
		return vec3(x+v.x,y+v.y,z+v.z);
	}
	vec3 vec3::operator-(vec3 v){
		return vec3(x-v.x,y-v.y,z-v.z);
	}
	vec3 vec3::operator*(float v){
		return vec3(x*v,y*v,z*v);
	}
	void vec3::operator*=(vec3 v){
		x*=v.x;
		y*=v.y;
		z*=v.z;
	}
	void vec3::operator/=(vec3 v){
		x/=v.x;
		y/=v.y;
		z/=v.z;
	}
	bool vec3::operator==(vec3 f)const{
		return ((int)x/3==(int)f.x/3) && ((int)y/3==(int)f.y/3) && ((int)z/3==(int)f.z/3);
	};
	vec3 vec3::operator/(float v){
		return vec3(x/v,y/v,z/v);
	}
	vec3 vec3::operator/(vec3 v){
		return vec3(x/v.x,y/v.y,z/v.z);
	}
	void vec3::operator=(vec3 v){
		x=v.x;
		y=v.y;
		z=v.z;
	}
	bool vec3::operator==(vec3 v){
		return (x==v.x&&y==v.y&&z==v.z);
	}
	bool vec3::operator!=(vec3 v){
		return (x!=v.x||y!=v.y||z!=v.z);
	}
	vec3 vec3::getsign(){
		return vec3(sign(x),sign(y),sign(z));
	}
	float vec3::length(){
		return sqrt((x*x)+(y*y)+(z*z));
	}
	float vec3::lengthsq(){
		return ((x*x)+(y*y)+(z*z));
	}
	void vec3::normalize(){
		float l=length();
		x/=l;y/=l;z/=l;
	}
	vec3 vec3::normalized(){
		vec3 n(x,y,z);
		n.normalize();
		return n;
	}
	bool vec3::isbetween(float a, float b, float num){
		return (num>=a&&num<=b)||(num>=b&&num<=a);
	}
	bool vec3::isbetween(vec3 a, vec3 b, bool issigned=false){
		return	isbetween(a.x,b.x,x)&&isbetween(a.y,b.y,y)&&isbetween(a.z,b.z,z);
	}
	bool vec3::isnormalized(){
		return (lengthsq()==1);
	}

	bool vec3::sameVector(vec3 v){
		vec3 self(x,y,z);
		float lensq=length();
		float vlensq=v.length();
		v/=vlensq;
		self/=lensq;
		if(v==self)return true;
		else return false;
	}
	float vec3::dot(vec3 v){
		return (x*v.x) + (y*v.y) + (z*v.z);
	}
	float vec3::specdot(vec3 v){//TODO: FIX
		return (x*v.x) + (z*v.z);
	}
	float vec3::trad(vec3 v){
		return (atan(z/x)-atan(v.z/v.x));
	}
	vec3 vec3::cross(vec3 v){
		using namespace std;
		float rx=(y*v.z) - (z*v.y);
		float ry=(z*v.x) - (x*v.z);
		float rz=(x*v.y) - (y*v.x);
		return vec3(rx,ry,rz);
	}
	float vec3::distancesq(vec3 b){
		return pow((x-b.x),2)+pow((y-b.y),2)+pow((z-b.z),2);
	}
	float vec3::distance(vec3 b){
		return sqrt(distancesq(b));
	}


vec2::vec2(float x, float y){
	this->x = x;
	this->y = y;
}
vec2::vec2(){
	x=0;
	y=0;
}
void vec2::operator=(vec2 v){
	x=v.x;
	y=v.y;
	}
bool vec2::operator==(vec2 v){
	return (x==v.x&&y==v.y);
}
bool vec2::operator!=(vec2 v){
	return !(x==v.x&&y==v.y);
}
void vec3::rotateXZ(float theta){
	using namespace std;
	float tempx=x;
	x=cos(atan2(z,x)+(theta*ML_PI/180.f))*sqrt(pow(z,2.0)+pow(x,2.0));
	z=sin(atan2(z,tempx)+(theta*ML_PI/180.f))*sqrt(pow(z,2.0)+pow(tempx,2.0));
}