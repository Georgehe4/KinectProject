// KinectProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "Windowing.h"
#include <Ole2.h>
#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>
#include "Sensor.h"
#include "model.h"
#include "Geom.h"
#include "ImageProcessor.h"
#include "GUI.h"
#include "Frame.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

struct cmdst{
	std::ofstream cmd;
};
cmdst otx={std::ofstream("prgm_out.txt")};

bool displayinit(){return true;}
bool loadscreen(){return true;}

using namespace std;
vector <DualPlane> groupOfBest;
int currBest=0;
vector <Frame> frames;
unsigned int frame_num=0;
GLUquadricObj *quadratic;
FileIO io;
math::ImageProcessor processor;
GLuint gl_rgbTexture;
vec3 draw_plane_pos;	//a plane to draw.
unsigned int plane_cursor=0;		//choose which plane to draw.
bool calculatedGStuff=false;

void eventhandle(struct HWND__ * a,unsigned int b,unsigned int c,long d){}

bool init(){
	cout<<"Initializing Sensor...";
	if(!initSensor())return false;
	cout<<"Done."<<endl;
	quadratic=gluNewQuadric();          // Create A Pointer To The Quadric Object ( NEW )
	gluQuadricNormals(quadratic, GLU_SMOOTH);   // Create Smooth Normals ( NEW )
	gluQuadricTexture(quadratic, GL_TRUE);      // Create Texture Coords ( NEW )
		glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); 
	glGenTextures(1,&gl_rgbTexture);
	glBindTexture(GL_TEXTURE_2D,gl_rgbTexture);
	glTexImage2D(GL_TEXTURE_2D,0,4,640,480,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	NuiCameraElevationSetAngle(0);
	return true;
}
float sin_time = 0;
void render(Plane p){
	
	vec3 temp = p.COG+p.normal*0.3f;
	glLineWidth(3.f);
	glPointSize(6.f);
	glColor3f(p.color.x*2,p.color.y*2,p.color.z*2);
	glBegin(GL_LINES);
	 glVertex3f(p.COG.x,-p.COG.y,p.COG.z);
	 glVertex3f(temp.x,-temp.y,temp.z);
	glEnd();
	glPointSize(1.f);
	glLineWidth(1.f);
	if(p.master!=0)glColor3f(0.5f+(0.5f*sin(sin_time+0.1f)),1,1);
	glBegin(GL_POINTS);
	int val = 0;
	int wrap = rand()%3;
	for(std::vector<vec3>::iterator it=p.pointsOnPlane.begin();it!=p.pointsOnPlane.end();it++){
		if(val++%5<wrap)continue;
		if(val>p.oldIndex)glColor3f(1,1,0.5f+(0.5f*sin(sin_time+0.33f)));
		glVertex3f((*it).x,-(*it).y,(*it).z);
	}
	if(p.master!=0){
		glColor3f(1,0,0.5f+(0.5f*sin(sin_time+0.1f)));
		for(std::vector<vec3>::iterator it=p.master->pointsOnPlane.begin();it!=p.master->pointsOnPlane.end();it++){
			if(val++%5==wrap)glVertex3f((*it).x,-(*it).y,(*it).z);
		}
	}
	glEnd();
	
	/*	glBegin(GL_TRIANGLES);
	glColor3f(1,1,1);
	 for(int i=0;i<3;i++){
		 glTexCoord2f(p.texCoords[i].x,p.texCoords[i].y);
		 glVertex3f(p.bounds[i].x,-p.bounds[i].y,p.bounds[i].z);
		 vec3 v = p.bounds[i];
		// cout<<"..("<<v.x<<", "<<v.y<<", "<<v.z<<")"<<endl;
	 }
	glEnd();
	*/
	glEnd();
	
}
void renderPoint(float r, float g, float b, float x, float y, float dist){
	float div=10;
	glColor3f(r,g,b);
	glVertex3f(x,y,dist);
}
void renderRGB(int x, int y, float r, float g, float b, float dist){
	glColor3f(r/256,g/256,b/256);
	glVertex3f(((float)x/320.f)-1.f, ((float)y/240.f)-1.f,0);
}
float camx=-2.5f;
float camy;
float camz=-2.5f;
float rotx=0;
float roty=180;

vec3 scene_rotation;

void camera(){
	float p=keys['Z']?0.1f:1.f;
	if(keys[VK_LEFT])roty+=p*10.f;
	if(keys[VK_RIGHT])roty-=p*10.f;
	if(keys[VK_DOWN])rotx-=p*10.f;
	if(keys[VK_UP])rotx+=p*10.f;
	if(keys['S']){
		camz+=p*cos(roty*3.14f/180.f);
		camx+=p*sin(roty*3.14f/180.f);
	}
	if(keys['W']){
		camz-=p*cos(roty*3.14f/180.f);
		camx-=p*sin(roty*3.14f/180.f);
	}
	if(keys['D']){
		camx+=p*cos(roty*3.14f/180.f);
		camz-=p*sin(roty*3.14f/180.f);
	}
	if(keys['A']){
		camx-=p*cos(roty*3.14f/180.f);
		camz+=p*sin(roty*3.14f/180.f);
	}
	if(keys['R'])camy+=p*0.2f;
	if(keys['F'])camy-=p*0.2f;
}
int mouseindex;
std::vector<Plane> planes;
inline bool isRealNumber(float x){
	return (x <= DBL_MAX && x >= -DBL_MAX); 
}
bool display(int last){
	int data_len = 2*640*480;
	using namespace std;
	camera(); 
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0,0,-1.f);
	glColor3f(1,1,1);
	glPrint(-1.1f,0.55f,"Frame %u, Plane %u, RLFrame: %u",frame_num,plane_cursor, framenum);

	glLoadIdentity();
	glRotatef(-rotx,1,0,0);
	glRotatef(-roty,0,1,0);
	glTranslatef(-camx,-camy,-camz);

	BYTE* visData;
	BYTE* rgbData;
	BYTE* curr;
	BYTE* rgbCurr;
	beginDepthCapture(curr);
	visData=curr;
	//cout<<".Capping RGB"<<endl;
	beginRGBCapture(rgbCurr);
	//cout<<"RGB CAPPED"<<endl;
	rgbData=rgbCurr;
	const BYTE* dataEnd = curr+(data_len);
	//cout<<"DISPLAYING"<<endl;
	BYTE a;
	BYTE b;
	BYTE rgbA;
	BYTE rgbB;
	BYTE rgbC;
	float x=0;
	float y=480;
	float z=0;
	//processor.setDepthImage(visData);
	//vector<Plane> planes;
	//processor.RANSAC(planes);
	long l1=0;
	long* temp=(&l1);
	NuiCameraElevationGetAngle(temp);
	//cout<<"camera: "<<(*temp)<<endl;
	math::setCameraAngle((*temp));
	if(keys['G']){
		//stitch the current frame with the next frame. If size==0, create the first frame.
		float averageTheta =0;
		float averageDivide=0;
		int orig_frame_num=framenum;
		Frame* frameA;
		Frame* frameB=0;
		if(frames.size()==0){						//size is 0. just create a new frame.
			frames.reserve(33);
			planes.clear();
			processor.RANSAC(visData, planes);
			Plane::calculateFloodFill(&planes, visData);
			frames.push_back(Frame(planes,visData,rgbData));
			planes.clear();
		}
		else{
			frameA = &frames[frames.size()-1];		//frame A is the last frame.
			for(int j=0;j<3;j++){
				groupOfBest.clear();
				//frames.clear();
				int num_frames_to_make = 1;
				if(frames.size()==0)num_frames_to_make=2;
				for(int i=0;i<num_frames_to_make;i++){
					reader.read("depthdata.bin",(char*)visData, 640*480*2,framenum);
					planes.clear();
					processor.RANSAC(visData, planes);
					Plane::calculateFloodFill(&planes, visData);
					if(frameB != 0)delete frameB;					//prevent memory leaks.
					frameB = new Frame(planes,visData,rgbData);		//frame B is this frame.
					planes.clear();
					framenum++;
				}

				std::vector<DualPlane> temporary = frameB->crossCheck(frameA);	//cross check this frame with the last frame.

				if(temporary.size()>20){
					int index=0;
					DualPlane* bestFit[20];
					float thetaMean=0;
					float meanDivide = 0;
					std::cout<<"..."<<endl;
					for(int i=0;i<20;i++){
						groupOfBest.push_back(temporary[i]);
						bestFit[i]=&temporary[i];
					}
					for(int i=0;i<5;i++){
						float tempTheta=abs(180.f/ML_PI*bestFit[i]->a->normal.trad(bestFit[i]->b->normal));
						if(isRealNumber(tempTheta)){
							if(tempTheta>90)tempTheta=180-tempTheta;
							thetaMean+=tempTheta;
							meanDivide++;
							std::cout<<tempTheta<<endl;
						}else std::cout<<"NaN"<<endl;
					}
					if(meanDivide==0)meanDivide=1;
					std::cout<<"Avg: "<<thetaMean<<"/"<<meanDivide;
					thetaMean/=meanDivide;
					averageTheta+=abs(thetaMean);
					std::cout<<" =\t"<<thetaMean<<endl;
					averageDivide++;
				}
				framenum = orig_frame_num;
			}
			if(averageDivide==0)averageDivide=1;
			std::cout<<"Total Average: "<<averageTheta<<"/"<<averageDivide<<":\t";
			averageTheta/=averageDivide;
			std::cout<<"Average Theta: "<<averageTheta<<endl;

			frameB->theta = frameA->theta + averageTheta;
			frameB->frameShift();
			frames.push_back(*frameB);
		}
	}
	glBindTexture(GL_TEXTURE_2D,gl_rgbTexture);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,640,480,0,GL_BGRA_EXT,GL_UNSIGNED_BYTE,rgbData);
	glColor3f(1,1,1);
	/*
	glBegin(GL_QUADS);
	 glVertex3f(-10,10,draw_plane_pos.z);
	 glVertex3f(10,10,draw_plane_pos.z);
	 glVertex3f(10,-10,draw_plane_pos.z);
	 glVertex3f(-10,-10,draw_plane_pos.z);
	glEnd();
	*/
	/*
	if(keys[VK_F5]){//TODO: FIX ANGLES GOING NEGATIVE Z
		keys[VK_F5] = false;
		if(frames.size()>1){
			std::cout<<"F5 Pressed. Stitching."<<endl;
			std::vector<DualPlane> temporary = frames[0].crossCheck(&frames[1]);
			if(temporary.size()>20){
				int index=0;
				DualPlane* bestFit[20];
				for(int i=0;i<20 && i<=temporary.size();i++){
					bestFit[index++] = &temporary[i];
					groupOfBest.push_back(temporary[i]);
				}
				float thetaAvg=0;
				float thetaMean=0;
				float meanDivide = 0;
				cout<<"..."<<endl;
				for(int i=0;i<5;i++){
					float tempTheta=abs(180.f/ML_PI*bestFit[i]->a->normal.trad(bestFit[i]->b->normal));
					if(isRealNumber(tempTheta)){
						if(tempTheta>90)tempTheta=180-tempTheta;
						thetaMean+=tempTheta;
						meanDivide++;
						cout<<tempTheta<<endl;
					}else cout<<"NaN"<<endl;
				}
				if(meanDivide==0)meanDivide=1;
				thetaMean/=meanDivide;
				for(int i=0;i<20;i++){	//print out both planes.
					otx.cmd<<bestFit[i]->confidence() <<":\t("<<bestFit[i]->a->COG.x<<", "<<bestFit[i]->a->COG.y<<", "<<bestFit[i]->a->COG.z<<")"
													<<", ("<<bestFit[i]->a->normal.x<<", "<<bestFit[i]->a->normal.y<<", "<<bestFit[i]->a->normal.z<<") - "
													<<bestFit[i]->a->pointsOnPlane.size()<<", "<<bestFit[i]->a->standardDeviation<<endl;
					otx.cmd<<"\t("<<bestFit[i]->b->COG.x<<", "<<bestFit[i]->b->COG.y<<", "<<bestFit[i]->b->COG.z<<")"
													<<", ("<<bestFit[i]->b->normal.x<<", "<<bestFit[i]->b->normal.y<<", "<<bestFit[i]->b->normal.z<<") - "
													<<bestFit[i]->b->pointsOnPlane.size()<<", "<<bestFit[i]->b->standardDeviation<<endl;
				}
				cout<<"Average rotation: "<<thetaMean<<endl;
				frames[1].theta=thetaMean;
			}
			//frames[1].frameShift();
		}
	}
	*/
	int index = 0;
	if(keys['8']){
		if(currBest<0){
			currBest=0;
		}
		if(groupOfBest.size()>currBest){
		render(*(groupOfBest[currBest].a));
		render(*(groupOfBest[currBest].b));
		}
		else{
			currBest=groupOfBest.size()-1;
		}
	}
	if(keys['9']){
		currBest--;
		keys['9']=false;
	}
	if(keys['0']){
		currBest++;
		keys['0']=false;
	}
	if(!keys['Y']){
		if(frames.size()>frame_num){
			for(std::vector<Plane>::iterator it=frames[frame_num].planes.begin();it!=frames[frame_num].planes.end();++it){
				if(index++==plane_cursor || keys[VK_F4])
				render(*it);
			}
		}
	}
	if(keys['U']){
		glBegin(GL_POINTS);
		for(int i=0;i<frames.size();i++){
			for(int j=0;j<frames[i].worldPoints.size();j++){
				WorldPoint temp=frames[i].worldPoints[j];
				renderPoint(temp.rgb.z/256.f, temp.rgb.y/256.f,temp.rgb.x/256.f, temp.position.x, -temp.position.y, temp.position.z);
			}
		}
		glEnd();
	}
	//glPrint(
	//cout<<"Planes:\t\t "<<planes.size()<<endl;
	//cout<<"Player Position: "<<camx<<", "<<camz<<endl;

	//getchar();
	static float rot=0;
	glPushMatrix();
	if(framenum==234)glRotatef(scene_rotation.y,0,1,0);
	glBegin(GL_POINTS);
	if(keys['Z'])scene_rotation.y+=0.0333f;
	if(keys['X'])scene_rotation.y-=0.0333f;
	if(keys['C'])cout<<"roty\t"<<scene_rotation.y<<"\t"<<frame_num<<endl;
	if(keys['Y']){
		while(curr < dataEnd){
			a=*(curr++);
			b=*(curr++);
			rgbC=*(rgbCurr++);
			rgbB=*(rgbCurr++);
			rgbA=*(rgbCurr++);
			rgbCurr++;
			z = (((b) << 8) + a) >> 3;
		//renderPoint(x+640,y,rgbA,rgbB,rgbC, z==0?0:toFeet(z));
			vec3 newVec= math::getWorldCoord(x,y,z);
			renderPoint((rgbA/256.f), (rgbB/256.f), (rgbC/256.f), newVec.x,newVec.y,newVec.z);
			x++;
			if(x>=640){
				x=0;
				y--;
			}
		}
	}
	glEnd();
		glPopMatrix();
	
	
	/*for(int i=0;i<5050;i++){
		glPointSize(2.f);
		vec3 random = processor.randPoint(visData,0,0,640,480);
		renderPoint(0,1,1,random.x,-random.y,random.z);
		glPointSize(1.f);
	}*/
	if(keys[VK_F3]){
		if(!keys[VK_LSHIFT])keys[VK_F3]=false;
		plane_cursor++;
	}
	if(keys[VK_F2]){
		if(!keys[VK_LSHIFT])keys[VK_F2]=false;
		plane_cursor--;
	}
	if(plane_cursor<0)plane_cursor=0;
	if(frames.size()>0){
		if(frame_num>frames.size()-1)frame_num=frames.size()-1;
		if(frames[frame_num].planes.size()>0)
			if(plane_cursor > frames[frame_num].planes.size())plane_cursor = frames[frame_num].planes.size()-1;
	}
	if(keys[VK_ADD]){
		framenum+=1;
		rgbframenum++;
		frame_num++;
		keys[VK_ADD]=false;
	}
	if(keys[VK_SUBTRACT]){
		framenum--;
		frame_num--;
		rgbframenum--;
		keys[VK_SUBTRACT]=false;
	}
	if(frame_num<0)frame_num=0;
	if(frame_num>=frames.size() && frames.size()>0)frame_num=frames.size()-1;
	if(keys['P']){
		framenum=0;
		rgbframenum=0;
	}
	if(keys['L']){
		//cout<<"Saving to file...."<<endl;
		//cout<<"fail."<<endl;
		io.write("rgbdata.bin",(char*)rgbData, 640*480*4,false);
		io.write("depthdata.bin",(char*)visData, 640*480*2,false);
		//cout<<"fail."<<endl;
	
	}
	if(keys['O']){
		//cout<<"Saving to file...."<<endl;
		//cout<<"fail."<<endl;
		io.write("rgbdata.bin",(char*)rgbData, 640*480*4,true);
		io.write("depthdata.bin",(char*)visData, 640*480*2,true);
		//cout<<"fail."<<endl;
	
	}
	releaseDepthCapture();
	releaseRGBCapture();

	return keys[VK_ESCAPE];
}
int _tmain(int argc, _TCHAR* argv[]){
	/*using namespace std;
	vec3 group[4];
	int counter=0;
	for(int i=-1;i<2;i++){
		for(int j=-1; j<2;j++){
			group[counter]=vec3(i,0,j);
			counter++;
		}
	}
	for(int i=0;i<4;i++){
		group[i].print();
		group[i].rotateXZ(10);
		group[i].print();
		cout<<endl;
	}
	getchar();
	return 0;*/
	srand(time(NULL));
	using namespace std;
	bool running = createWindow("Kinect Stitching A-1.3", 1366, 768, 32, false);
	BuildFont();
	//cout << "Done."<<endl;
	if(running){
		gameloop();
		otx.cmd.close();
		cout<<"Shutting down Nui.."<<endl;
		NuiShutdown();
		cout<<"Destoying window.."<<endl;
		destroyWindow();
	}
	KillFont();
	cout<<"Done"<<endl;
	return 0;
}

