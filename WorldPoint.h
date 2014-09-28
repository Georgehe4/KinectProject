#include "model.h"
class WorldPoint{
public:
	vec3 position;
	vec3 rgb;
	WorldPoint(vec3 posd, vec3 rgbd);
	void shift(float theta);
};