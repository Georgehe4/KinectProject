#include <vector>
namespace math{
	vec2 undoWorldCoord(vec3 undothis);
	vec3 getWorldCoord(float x ,float y, float z);
	float toFeet(float in);
	void setCameraAngle(long in);
	float getDepth(unsigned char* data, int x, int y);
	vec3 randPoint(unsigned char* image, int x, int y, int w, int h);
	vec3 randPoint(unsigned char* image, int x, int y, int w, int h, vec2* tex);


class ImageProcessor{
private:
	std::vector<Plane> process(unsigned char* image, int x, int y, int w, int h);
public:
	std::vector<Plane> process(unsigned char* image);
	void RANSAC(unsigned char* data, std::vector<Plane> * temp, int x, int y, int w, int h);
	void RANSAC(unsigned char* data, std::vector<Plane> &planes);
	void ranMediator(std::vector<Plane> &planes, unsigned char* data, int x, int y, int w, int h);
	//one step of RANSAC.
	int ranstep(Plane* temp, unsigned char* data, int x, int y, int w, int h);

};



};