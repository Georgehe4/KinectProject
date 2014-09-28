#include "FileIO.h"
NUI_SKELETON_FRAME ourframe;
INuiSensor* sensor;
HANDLE depthStream;
HANDLE rgbStream;
int  rgbframenum=0;	//=23
int framenum=0;
HANDLE hNextDepthFrameEvent;
HANDLE hNextColorFrameEvent;
INuiFrameTexture* texture;
INuiFrameTexture* rgbTexture;
const NUI_IMAGE_FRAME* frame;
const NUI_IMAGE_FRAME* rgbFrame;
bool disp=false;
FileIO reader;
BYTE* rgbcapture=new BYTE[640*480*4];
BYTE* depthcapture=new BYTE[640*480*2];
void reset(){
	framenum=0;
}
bool initSensor(){
    int numSensors;
    if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1 || NuiCreateSensorByIndex(0, &sensor) < 0){
		disp=true; 
		return true;
	}

	NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH | NUI_INITIALIZE_FLAG_USES_COLOR);
	hNextDepthFrameEvent= CreateEvent(NULL, TRUE, FALSE, NULL);
    sensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH,            // Depth camera or rgb camera?
        NUI_IMAGE_RESOLUTION_640x480,    // Image resolution
        0,      // Image stream flags, e.g. near mode
        2,      // Number of frames to buffer
        hNextDepthFrameEvent,   // Event handle
        &depthStream);
	hNextColorFrameEvent= CreateEvent(NULL, TRUE, FALSE, NULL);
	sensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,            // Depth camera or rgb camera?
        NUI_IMAGE_RESOLUTION_640x480,    // Image resolution
        0,      // Image stream flags, e.g. near mode
        2,      // Number of frames to buffer
        hNextColorFrameEvent,   // Event handle
        &rgbStream);
	return true;
}
void beginRGBCapture(BYTE* &buff){
	if(disp){
		reader.read("rgbdata.bin",(char*)rgbcapture,640*480*4,rgbframenum);
		buff=rgbcapture;
		return;
	}
		//NuiCameraElevationSetAngle(0);
	WaitForSingleObject(hNextColorFrameEvent,INFINITE);
//	std::cout<<"RGBCAPPING1"<<std::endl;
	HRESULT hr=NuiImageStreamGetNextFrame( rgbStream, 0, &rgbFrame);
	//	std::cout<<"RGBCAPPING2"<<std::endl;
	if(FAILED(hr)){
	//	std::cout<<"failed"<<std::endl;
	}
	else{
		NUI_LOCKED_RECT LockedRect;
		rgbTexture= rgbFrame->pFrameTexture;
		rgbTexture->LockRect(0, &LockedRect, NULL, 0);
        BYTE* curr = (BYTE*) LockedRect.pBits;
		buff=curr; //depth data
	}
	//	std::cout<<"RGBCAPPING3"<<std::endl;
}
void beginDepthCapture(BYTE* &buff){
	if(disp){
	reader.read("depthdata.bin",(char*)depthcapture, 640*480*2,framenum);
	buff=depthcapture;
	return;
	}
	//NuiCameraElevationSetAngle(0);
	WaitForSingleObject(hNextDepthFrameEvent,INFINITE);
	HRESULT hr=NuiImageStreamGetNextFrame( depthStream, 0, &frame);
	if(FAILED(hr)){
		std::cout<<"failed"<<std::endl;
	}
	else{
		NUI_LOCKED_RECT LockedRect;
		texture = frame->pFrameTexture;
		texture->LockRect(0, &LockedRect, NULL, 0);
        BYTE* curr = (BYTE*) LockedRect.pBits;
		buff=curr; //depth data
	}
}
void releaseDepthCapture(){
	if(disp){
		return;
	}
	texture->UnlockRect(0);
	NuiImageStreamReleaseFrame( depthStream, frame);
}
void releaseRGBCapture(){
	if(disp){
		return;
	}
	rgbTexture->UnlockRect(0);
	NuiImageStreamReleaseFrame( rgbStream, rgbFrame);
}