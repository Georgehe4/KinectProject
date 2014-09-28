class Image{
public:
	Image(int x, int y, int w, int h): width(w), height(h), len(w*h){
		this->x=x;
		this->y=y;
		data=new short[len];
	}
	~Image(){
		delete data;
	}
	const int width;
	const int height;
	const int len;
	int depth;
	int x;
	int y;
	short* data;
};