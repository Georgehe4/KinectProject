#include "stdafx.h"
#include "model.h"
#include <string.h>
#include "Geom.h"
#include "ImageProcessor.h"
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>



//initialize static members of Plane class.

std::unordered_map<DualPlane, int> DualPlane::allPlanePairs;


//define member functions of Plane class.
vec3 Plane::getNormal(){
	return normal;
}
void Plane::operator=(Plane p){
	setTo(p);
}
void Plane::setTo(Plane p){
	normal=p.normal;
	point=p.point;
	color=p.color;
	bounds[0]=p.bounds[0];
	bounds[1]=p.bounds[1];
	bounds[2]=p.bounds[2];
	//bounds[3]=p.bounds[3];
	//texCoords[3]=p.texCoords[3];
	texCoords[0]=p.texCoords[0];
	texCoords[1]=p.texCoords[1];
	texCoords[2]=p.texCoords[2];
	master=p.master;
	standardDeviation=p.standardDeviation;
	COG=p.COG;
}
Plane::Plane(vec3 a, vec3 b, vec3 c){
	color=vec3(rand()%256/256.f,rand()%256/256.f,rand()%256/256.f);
	point = a;
	vec3 v1 = (b-a);
	vec3 v2 = (c-a);
	normal= v1.cross(v2);
	normal.normalize();
	bounds[0]=a;
	bounds[1]=b;
	bounds[2]=c;
	master=0;
	self_data=0;
	oldIndex=0;
	standardDeviation=-1;
}
Plane::Plane(){
	color=vec3(rand()%256/256.f,rand()%256/256.f,rand()%256/256.f);
	normal=vec3();
	point=vec3();
	master=0;
	self_data=0;
	oldIndex=0;
	standardDeviation=-1;
}
void Plane::merge(const Plane b){
	pointsOnPlane.insert(pointsOnPlane.end(),b.pointsOnPlane.begin(),b.pointsOnPlane.end());
}
Plane::Plane(vec3 normal, vec3 point){
	color=vec3(rand()%256/256.f,rand()%256/256.f,rand()%256/256.f);
	this->normal = normal;
	this->point = point;
	master=0;
	self_data=0;
	oldIndex=0;
	standardDeviation=-1;
}
void Plane::buildDataMap(){
	self_data = new unsigned char[640*480];
	for(int i=0;i<640*480;i++){
		self_data[i]=0;
	}
}
void Plane::destroyDataMap(){
	delete[] self_data;
	self_data = 0;
}
float Plane::getDistance(vec3 in){
	return (in-point).dot(normal);
}
vec3 Plane::getIntersectionWithLine(vec3 B, vec3 V){
	using namespace std;
	float dist = getDistance(B);
	vec3 perpendicular = B-normal*getDistance(B);
	float cosTheta = (perpendicular-B).dot(V)/(V.length()*(perpendicular-B).length());
	float h = -dist/cosTheta;
	return B+(V*h);
}
void Plane::RANSACUpdateMathematicalModel(){
	if(pointsOnPlane.size() == 0)return;
	Plane best_plane;
	float max_confidence=0;
	for(int i=0;i<500;i++){
		Plane temp(pointsOnPlane[rand()%pointsOnPlane.size()],pointsOnPlane[rand()%pointsOnPlane.size()],pointsOnPlane[rand()%pointsOnPlane.size()]);
		float confidence = 0;
		for(int j=0;j<400;j++){
			vec3 testPoint = pointsOnPlane[rand()%pointsOnPlane.size()];
			if(getDistance(testPoint)<0.015)confidence++;
		}
		if(confidence>max_confidence){
			max_confidence = confidence;
			best_plane = temp;
		}
	}
	normal=best_plane.normal;
	point=best_plane.point;

	//now that we have calculated RANSAC, determine the center of gravity.

	float weightTotal=0;		//total weight values...the amount to divide by at the end.
	vec3 average;				//running average.
	area=0;
	for(std::vector<vec3>::iterator it=pointsOnPlane.begin();it!=pointsOnPlane.end();++it){
		vec3 temp = (*it);
		float weight = 1.f/(temp.lengthsq());		//weight is equal to 1/(distance squared).
		weightTotal+=weight;
		float areaWeight = temp.z*tan(57.*ML_PI/(640.0*180.0));
		areaWeight*=	temp.z*tan(43.*ML_PI/(480.0*180.0));
		areaWeight*=4.;
		area+=(areaWeight);
		average+=(temp*weight);
	}
	average/=weightTotal;
	COG = average;
}
//returns true if the point was added, false otherwise.
inline bool isRealNumber(float x){
	return (x <= DBL_MAX && x >= -DBL_MAX); 
}
void Plane::floodFillStep(vec2 pt, unsigned char* data, std::queue<vec2> *toCheck){
	if(!isRealNumber(pt.x) || !isRealNumber(pt.y))return;
	//std::cout<<pt.x<<", "<<pt.y<<"\t";
	if(pt.x<0||pt.y<0||pt.x>=640||pt.y>=480)return;												//return if point not in image.
	vec3 worldPos = math::getWorldCoord(pt.x,pt.y, math::getDepth(data,pt.x,pt.y));
	if(self_data[int(pt.y)*640 + int(pt.x)]!=0){		//already visited. return.
		return;
	}
	if(abs(getDistance(worldPos))>0.1f){
		self_data[int(pt.y)*640 + int(pt.x)]=2;
	}
	else{
		self_data[int(pt.y)*640 + int(pt.x)] = 1;
		pointsOnPlane.push_back(worldPos);
		toCheck->push(vec2(pt.x-1,pt.y));
		toCheck->push(vec2(pt.x,pt.y-1));
		toCheck->push(vec2(pt.x,pt.y+1));
		toCheck->push(vec2(pt.x+1,pt.y));
	}
}
DualPlane::DualPlane(Plane* aa, Plane* bb){
		a=aa;
		b=bb;
	}
bool DualPlane:: operator==(DualPlane f)const{
		return ((a==f.a && b==f.b) ||(a==f.b && b==f.a));
	};
float DualPlane::confidence(){
	if(a->standardDeviation >0.85f || b->standardDeviation >0.85f || a->pointsOnPlane.size()<100 || b->pointsOnPlane.size()<100)return 0.f; 
	int as=a->pointsOnPlane.size();
	int bs=b->pointsOnPlane.size();
	if(a->COG.distancesq(b->COG)>1.f || abs(a->normal.dot(b->normal))<0.866 || abs(a->normal.dot(b->normal))>0.998	//angle between 1-30deg.
		|| abs((180.f/ML_PI)*a->normal.trad(b->normal))>15 || abs((180.f/ML_PI)*a->normal.trad(b->normal))<1){	//15deg.
		return 0;
	}
	float conf = (1.f-(float)(abs(a->area-b->area))/(float)(a->area+b->area));
	conf*=conf;
	if( as<10000 || bs<10000){
		conf = (conf*0.5f);
	}
	else if( as<20000 || bs<20000){
		conf = (conf*0.75f);
	}

	return conf;
}
void Plane::shiftPlane(float theta){
	for(int i=0;i<pointsOnPlane.size();i++){
		pointsOnPlane[i].rotateXZ(theta);
	}
}
void Plane::floodFill(unsigned char* data){
	using std::vector;
	using std::queue;
	using std::cout;
	using std::endl;
	if(self_data==0)buildDataMap();

	queue<vec2> toCheck;
	vec2 beginPt = math::undoWorldCoord(point);
	memset(self_data, 0, 640*480);					//erase everything already in temp_data.
	toCheck.push(beginPt);
	while(toCheck.size()>0){
		vec2 next = toCheck.front();
		toCheck.pop();
		floodFillStep(next, data, &toCheck);
	}

	//now that we have calculated flood fill, determine the center of gravity.

	float weightTotal=0;		//total weight values...the amount to divide by at the end.
	vec3 average;				//running average.

	for(std::vector<vec3>::iterator it=pointsOnPlane.begin();it!=pointsOnPlane.end();++it){
		vec3 temp = (*it);
		float weight = 1.f/(temp.lengthsq());		//weight is equal to 1/(distance squared).
		weightTotal+=weight;
		average+=(temp*weight);
	}
	average/=weightTotal;
	COG = average;
}
void Plane::calculateFloodFill(std::vector<Plane>* allPlanes, unsigned char* visData){
	using namespace std;
	cout<<"Clearing shared points array..."<<endl;

	cout<<"Performing flood fill..."<<endl;
	int ind=0;
	int total_planes_already_processed=0;
	for(std::vector<Plane>::iterator it=allPlanes->begin();it!=allPlanes->end();++it){
		vector<vec2> sharedPoints;
		(*it).floodFill(visData);
		(*it).oldIndex = (*it).pointsOnPlane.size();
		//cout<<ind<<": "<<(*it).pointsOnPlane.size()<<endl;
		ind++;
	}
	
	//logical OR...parent.data = parent.data OR child.data.
	for(std::vector<Plane>::iterator it=allPlanes->begin();it!=allPlanes->end();++it){
		Plane* absoluteMaster=&(*it);
		while(absoluteMaster->master!=0)absoluteMaster = absoluteMaster->master;	//get to the top of the chain of command.
		int max_ind = 640*480;
		for(int i=0;i<max_ind;i++){
			absoluteMaster->self_data[i]|=(*it).self_data[i];						//OR operator.
		}
	}
	for(std::vector<Plane>::iterator it=allPlanes->begin();it!=allPlanes->end();++it){
		int old_size = (*it).pointsOnPlane.size();
		(*it).pointsOnPlane.clear();
		(*it).pointsOnPlane.reserve(old_size);
		int max_ind = 640*480;
		int x=0;
		int y=0;
		for(int i=0;i<max_ind;i++){
			if((*it).self_data[i]==1){
				vec3 worldPos = math::getWorldCoord(x,y, math::getDepth(visData,x,y));
				(*it).pointsOnPlane.push_back(worldPos);
			}
			x++;
			if(x>=640){
				x=0;
				y++;
			}
		}
	}
	cout<<"Calculating plane intersections..."<<endl;
}
void Plane::calculateStandardDeviation(){
	float numerator = 0;
	float temp=0;
	for(int i=0;i<pointsOnPlane.size();i++){
		temp = getDistance(pointsOnPlane[i]);
		numerator+=(temp*temp);
	}
	standardDeviation = numerator/pointsOnPlane.size();
}
namespace std {
	size_t hash<DualPlane>::operator() (const DualPlane &f){
		return (f.a->bounds[0].x*480 + f.a->bounds[0].y) * (f.b->bounds[0].x*480 + f.b->bounds[0].y);
	}
}
namespace std {
	size_t hash<vec3>::operator() (const vec3 &f){
		return (int)((f.x*480.f) * (f.y) *1000.f);
	}
}