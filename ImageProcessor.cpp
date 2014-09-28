#include "stdafx.h"
#include "model.h";
#include "Geom.h";
#include "ImageProcessor.h";
#include <iostream>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <vector>

float cameraAngle;
float math::toFeet(float in){
	return 0.0033f*in + 0.0006f;
}
void math::ImageProcessor::RANSAC(unsigned char* data, std::vector<Plane> * temp, int x, int y, int w, int h){
	Plane best_plane(vec3(),vec3());
	int best_score=0;
	/*for(int i=0;i<100;i++){		//solve for a hundred planes;
			
	}*/
}
float math::getDepth(unsigned char* data, int x, int y){
	int ind = (y*640*2) + (x*2);
	BYTE a = data[ind];
	BYTE b = data[ind+1];
	return ((((b) << 8) + a) >> 3);
}
vec3 math::randPoint(unsigned char* image, int x, int y, int w, int h){
	float z = 0;
	vec3 temp(0,0,0.007);
	int timeout = 5;
	while(temp.z<=0.008){
		if(timeout--<0)return vec3(0,0,0);
		int xx = rand()%w+x;
		int yy = rand()%h+y;
		vec3 t=getWorldCoord(xx,yy, getDepth(image,xx,yy));
		temp.x=t.x;temp.y=t.y;temp.z=t.z;
	}
	return temp;
}
vec3 math::randPoint(unsigned char* image, int x, int y, int w, int h, vec2* texc){
	float z = 0;
	vec3 temp(0,0,0.007);
	int timeout = 5;
	while(temp.z<=0.008){
		if(timeout--<0)return vec3(0,0,0);
		int xx = rand()%w+x;
		int yy = rand()%h+y;
		texc->x = float(xx)/640.f;
		texc->y = float(yy)/480.f;
		vec3 t=getWorldCoord(xx,yy, getDepth(image,xx,yy));
		temp.x=t.x;temp.y=t.y;temp.z=t.z;
	}
	return temp;
}
void math::ImageProcessor::RANSAC(unsigned char* data, std::vector<Plane> &planes){
	std::vector<Plane>* temp = &planes;
	float best_score=0;
	std::cout<<"RANSAC"<<std::endl;
	ranMediator(planes,data,0,0,640,480);
	std::cout<<"Culling planes.\r\n";
	using std::cout;
	using std::endl;
	for(int i=0;i<planes.size();i++){
		for(int j=0;j<i;j++){
			if(abs(planes[i].normal.dot(planes[j].normal))>0.999 &&	//2 degrees
				(abs(planes[i].getDistance(planes[j].COG))<1.f || abs(planes[j].getDistance(planes[i].COG))<1.f)
				&& planes[i].COG.distancesq(planes[j].COG)<1.f){

				if(i<j)planes[j].master=&planes[i];
				else planes[i].master = &planes[j];
			}
		}
	}
}
vec3 formFromAngles(float x_angle_rads, float y_angle_rads){
	return vec3(sin(x_angle_rads)*sin(y_angle_rads), sin(y_angle_rads), cos(x_angle_rads)*sin(y_angle_rads));
}
vec3 formFromPixels(int x, int y){
	return formFromAngles((x-320)*(27.5f/320.f)*(ML_PI/180.f),(x-240)*(21.5f/240.f)*(ML_PI/180.f));
}
//TODO: change 39 back to 10 for return condition.
void math::ImageProcessor::ranMediator(std::vector<Plane> &planes, unsigned char* data, int x, int y, int w, int h){
	if(w<=10||h<=10)return;
	//std::cout<<"RANSAC "<<x<<", "<<y<<": ("<<w<<", "<<h<<std::endl;
	Plane p;
	using namespace std;
	int conf = ranstep(&p,data,x,y,w,h);
	if(conf<60){
		if(w>h){
			ranMediator(planes,data,x,y,w/2,h);
			ranMediator(planes,data,x+w/2,y,w/2,h);
		}else{
			ranMediator(planes,data,x,y,w,h/2);
			ranMediator(planes,data,x,y+h/2,w,h/2);
		}
	}else{
		/*
		vec3 vec_a=getWorldCoord(x,y,(getDepth(data,x,y)));
		vec3 vec_b=getWorldCoord(x+w,y,(getDepth(data,x+w,y)));
		vec3 vec_c=getWorldCoord(x+w,y+h,(getDepth(data,x+w,y+h)));
		vec3 vec_d=getWorldCoord(x,y+h,(getDepth(data,x,y+h)));
		
		p.bounds[0]=p.getIntersectionWithLine(vec_a,vec3(0,0,1));
		p.bounds[1]=p.getIntersectionWithLine(vec_b,vec3(0,0,1));
		p.bounds[2]=p.getIntersectionWithLine(vec_c,vec3(0,0,1));
		p.bounds[3]=p.getIntersectionWithLine(vec_d,vec3(0,0,1));
		
		p.texCoords[0]=math::undoWorldCoord(p.bounds[0]);
		p.texCoords[1]=math::undoWorldCoord(p.bounds[1]);
		p.texCoords[2]=math::undoWorldCoord(p.bounds[2]);
		*/
		//p.texCoords[3]=math::undoWorldCoord(p.bounds[3]);
		planes.push_back(p);
	}
}
int math::ImageProcessor::ranstep(Plane* temp, unsigned char* data, int x, int y, int w, int h){
	using std::cout;
	using std::endl;
	float max_confidence=0;
	Plane best_plane;
	for(int j=0;j<250;j++){
		vec2 texca;
		vec2 texcb;
		vec2 texcc;
		Plane p(randPoint(data,x,y,w,h, &texca),randPoint(data,x,y,w,h, &texcb),randPoint(data,x,y,w,h, &texcc));
		//cout<<"n "<<"("<<p.normal.x<<", "<<p.normal.y<<", "<<p.normal.z<<")"<<endl;
		float confidence=0;
		float COG_divide = 0;
		float num_skips=0;
		for(int i=0;i<200;i++){
			vec3 randpt = randPoint(data,x,y,w,h);
			if(randpt.z<=0.007){
				num_skips++;
				continue;
			}
			float distance = abs((p.point-randpt).dot(p.normal));
			//cout<<"dist  "<<distance<<endl;
	
			vec3 v=randPoint(data,x,y,w,h);
			if(v==vec3(0,0,0)){
				continue;
				num_skips++;
			}
			//cout<<"random "<<x<<"|"<<y<<"|"<<w<<"|"<<h<<endl;
			//cout<<"c "<<"("<<v.x<<", "<<v.y<<", "<<v.z<<")"<<endl;
			//cout<<"r "<<"("<<randpt.x<<", "<<randpt.y<<", "<<randpt.z<<")"<<endl;
			//cout<<"c "<<"("<<p.x<", "<<p.y<<", "<<p.z<<")"<<endl;
			if(distance<0.01f){
				confidence++;
				COG_divide++;
				p.COG+=v;
			}
			else if(distance<0.05f){
				COG_divide++;
				p.COG+=v;
			}
		}
		p.COG/=COG_divide;	//to find average...
		confidence *= (100.f/(100.f-num_skips));
		if(confidence>max_confidence){
			max_confidence=confidence;
			best_plane=p;
		}
	}
	(*temp)=best_plane;
	return max_confidence;
}
void math::setCameraAngle(long in){
	cameraAngle=((float)in)/(180.f*ML_PI);
	//std::cout<<cameraAngle<<std::endl;
}
vec3 math::getWorldCoord(float x,float y, float z){
	z=toFeet(z);
	return vec3(z*tan((x-320)*0.00149989232f),z*tan((y-240)*0.00156352412f+cameraAngle), z);
}
vec2 math::undoWorldCoord(vec3 undothis){//will only change x and y
	float x=undothis.x;
	float y=undothis.y;
	float z=undothis.z;
	return vec2((atan(x/z)/0.00149989232f+320),((atan(y/z)-cameraAngle)/0.00156352412f+240));
}
/*Plane RANSAC(int x, int y, int w, int h, float* use){
 for(int i=0;i<100;i++){
//	 ranstep();
 }
}*/
