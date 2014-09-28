#include "stdafx.h"
#include "WorldPoint.h"

WorldPoint::WorldPoint(vec3 posd, vec3 rgbd){
	position=posd;
	rgb=rgbd;
}
void WorldPoint::shift(float theta){
	position.rotateXZ(theta);
}