#include "stdafx.h"
#include "Geom.h"
#include "model.h"
#include "Frame.h"
#include "ImageProcessor.h"
#include <iostream>
Frame::Frame(std::vector<Plane> a, unsigned char* vision,unsigned char* rgbvision){
	std::cout<<"Creating frame..."<<std::endl;
	rgbVisionData=new unsigned char [640*480*4];
	visionData=new unsigned char [640*480*2];
	memcpy(visionData,vision,640*480*2);
	memcpy(rgbVisionData,rgbvision,640*480*4);
	for(int i=0;i<640*480;i++){
		worldPoints.push_back(WorldPoint(math::getWorldCoord(i%640,(int)i/640,math::getDepth(visionData,i%640,(int)i/640)),
			vec3(rgbVisionData[i*4],rgbVisionData[i*4+1],rgbVisionData[i*4+2])));
	}
	std::vector<Plane> tempPlanes;
	for(int i=0; i<a.size();i++){
		Plane temp=a[i];
		if(temp.master==0){
			temp.RANSACUpdateMathematicalModel();
			tempPlanes.push_back(temp);
		}
	}
	for(int i=0;i<tempPlanes.size();i++){
		for(int j=0;j<i;j++){
			if(abs(tempPlanes[i].normal.dot(tempPlanes[j].normal))> 0.9848f &&	//10 degrees
				(abs(tempPlanes[i].getDistance(tempPlanes[j].COG))<1.f || abs(tempPlanes[j].getDistance(tempPlanes[i].COG))<1.f)
				&& tempPlanes[i].COG.distancesq(tempPlanes[j].COG)<1.f){

				if(i<j)tempPlanes[j].master=&tempPlanes[i];
				else tempPlanes[i].master = &tempPlanes[j];
			}
		}
	}
	std::cout<<"Frame Floodfill 2...\t";
	Plane::calculateFloodFill(&tempPlanes, vision);
	/*
	for(std::vector<Plane>::iterator it=tempPlanes.begin();it!=tempPlanes.end();++it){
		Plane* absoluteMaster=&(*it);
		while(absoluteMaster->master!=0)absoluteMaster = absoluteMaster->master;	//get to the top of the chain of command.
		int max_ind = 640*480;
		for(int i=0;i<max_ind;i++){
			absoluteMaster->self_data[i]|=(*it).self_data[i];						//OR operator.
		}
	}
	for(std::vector<Plane>::iterator it=tempPlanes.begin();it!=tempPlanes.end();++it){
		int old_size = (*it).pointsOnPlane.size();
		(*it).pointsOnPlane.clear();
		(*it).pointsOnPlane.reserve(old_size);
		int max_ind = 640*480;
		int x=0;
		int y=0;
		for(int i=0;i<max_ind;i++){
			if((*it).self_data[i]==1){
				vec3 worldPos = math::getWorldCoord(x,y, math::getDepth(visionData,x,y));
				(*it).pointsOnPlane.push_back(worldPos);
			}
			x++;
			if(x>=640){
				x=0;
				y++;
			}
		}
	}
	*/
	std::cout<<"Recalculating planes...\t";
	for(int i=0; i<tempPlanes.size();i++){
		Plane temp=tempPlanes[i];
		if(temp.master==0){
			temp.RANSACUpdateMathematicalModel();
			temp.calculateStandardDeviation();
			planes.push_back(temp);
		}
	}
	std::cout<<"Constructor finished.\r\n";
	shifted=false;
};
void Frame::addPlane(Plane temp){
	if(temp.master==0){
		temp.RANSACUpdateMathematicalModel();
		planes.push_back(temp);
	}
}
std::vector<DualPlane> Frame::crossCheck(Frame* b){
	std::vector<DualPlane>comparison;

	for(int i=0;i<planes.size();i++){
		for(int j=0;j<b->planes.size();j++){
			DualPlane temp(&(planes[i]),&(b->planes[j])) ;
			int k=0;
			float tempConf=temp.confidence();
			while(k<comparison.size() && tempConf<comparison[k].confidence()){
				k++;
			}
			comparison.insert(comparison.begin()+k,temp);
		}
	}
	return comparison;
}
void Frame::frameShift(){
	for(int i=0; i<planes.size();i++){
		planes[i].shiftPlane(theta);
	}
	for(int i=0; i<640*480;i++){
		worldPoints[i].shift(theta);
	}
	shifted=true;
}