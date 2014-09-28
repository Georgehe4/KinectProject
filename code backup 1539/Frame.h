#include <vector>
#include "WorldPoint.h"

class Frame{
	public:
		std::vector<WorldPoint> worldPoints;
		std::vector<Plane> planes;
		Frame(std::vector<Plane> a, unsigned char* vision,unsigned char* rgbvision);
		void addPlane(Plane a);
		bool shifted;
		float theta;
		vec3 angles;
		unsigned char *visionData;
		unsigned char *rgbVisionData;
		std::vector<DualPlane> crossCheck(Frame* b);
		void frameShift();
};