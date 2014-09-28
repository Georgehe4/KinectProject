#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "model.h"


/*struct X{int i,j,k;};
struct hash_X{
	size_t operator()(const X &x) const{
		return 0;
	}
};
*/

	
class DualPlane;
namespace std{
	template<>
	struct hash<DualPlane>{
		size_t operator() (const DualPlane& f);
	};
};
class Plane;
namespace std{
	template<>
	struct hash<vec3>{
		size_t operator() (const vec3& f);
	};
};

class Plane{
public:
	Plane(vec3 a, vec3 b, vec3 c);
	Plane(vec3 normal, vec3 point);
	Plane();

	void shiftPlane(float theta);
	Plane* master;
	void merge(const Plane b);
	vec3 normal, point;
	char* rgbImage;
	vec3 bounds[4];
	vec2 texCoords[3];
	vec3 color;
	vec3 COG;		//center of gravity
	int oldIndex;
	float standardDeviation;
	unsigned char* self_data;
	std::vector<vec3> pointsOnPlane;										//world  coordinates for all point on plane.

	void operator=(Plane p);
	void setTo(Plane p);
	vec3 getNormal();
	float getDistance(vec3 in);
	vec3 getIntersectionWithLine(vec3 B, vec3 V);
	void floodFillStep(vec2 pt, unsigned char* data, std::queue<vec2> *toCheck);
	void floodFill(unsigned char* data);
	void buildDataMap();
	void destroyDataMap();
	void RANSACUpdateMathematicalModel();
	void calculateStandardDeviation();

	static void calculateFloodFill(std::vector<Plane>* allPlanes, unsigned char* visData);
};
class DualPlane{
public:
	DualPlane(Plane* aa, Plane* bb);
	Plane* a;
	float confidence();
	Plane* b;
	static std::unordered_map<DualPlane, int> allPlanePairs;				//plane pair mapped to # overlap.
	bool operator==(DualPlane f)const;
};