package kinmath;

import java.awt.Point;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

import SimpleOpenNI.SimpleOpenNI;
import processing.core.PImage;

/**
 * Handles math used to transform image coordinates
 * to world coordinates and vice versa.
 * @author Ashwin
 *
 */
public class Kinect {
	static{
		world_matrix = new TMatrix(3,4);
	}
	private Kinect(){
	}
	private static PImage depthImage;
	private static float floor_angle;
	private static float floor_height;
	private static Plane floor_plane;
	public static final TMatrix world_matrix;
	private static SimpleOpenNI ni;
	private static int[] lastDepthMap;
	private static boolean ready=false;
	public static boolean isReady(){
		return ready;
	}
	public static int[] lastDepthMap(){
		return lastDepthMap;
	}
	public static int[] depthMap(){
		return ni.depthMap();
	}
	public static PImage depthImage(){
		return depthImage;
	}
	public static void setNI(SimpleOpenNI newni){
		Kinect.ni=newni;
	}
	/**
	 * Is meant to be called every frame. Refreshes depth, ir, rgb images +
	 * performs per-frame calculations such as floor calculation.
	 */
	public static void update(){
		if(depthImage!=null){
			lastDepthMap=ni.depthMap();
			ready=true;
		}
		ni.update();
		depthImage = ni.depthImage();
	}
	public static PImage irImage(){
		return ni.irImage();
	}
	public static PImage rgbImage(){
		return ni.rgbImage();
	}
	/**
	 * Returns the x angle that corresponds to the specific x pixel,
	 * in degrees.
	 */
	public static PImage getDepthImage(){
		return depthImage;
	}
	public static int getDepth(int x, int y){
		return (Kinect.ni.depthMap()[(y*640) + x]);
	}
	public static float getXAngle(float xpixel){
		return (xpixel-320.f)*28.5f/320.f;
	}
	public static float getYAngle(float ypixel){
		return -(ypixel-240.f)*43.f/480.f;
	}
	public static float getXPixel (double xangle){
		return (float) (((xangle*320)/(28.5))+320);
	}
	public static float getYPixel (float yangle){
		return (float) (((yangle*240)/(43.f))+240);
	}
	public static float getFloorHeight(){
		return floor_height;
	}
	public static float getExpectedFloorDepth(int pixel_x, int pixel_y){
		float xrot = Kinect.getYAngle(pixel_y);
		float yrot = Kinect.getXAngle(pixel_x);
		Vector viewVector = new Vector((float)Math.sin(Math.toRadians(yrot)), (float)Math.sin(Math.toRadians(xrot)), 1);
		//Vector p = Kinect
		Plane p = Kinect.getFloorPlane();
		Vector p0 = p.point;
		Vector a0 = new Vector(0,0,0);
		Vector V = viewVector;
		Vector n = p.normal;
		float t = Vector.diff(p0, a0).getDotProduct(n) / V.getDotProduct(n);
		return t;
	}
	/**
	 * 
	 * @param x		x screen coordinate
	 * @param y		y screen coordinate
	 * @param depth	depth in feet
	 * @return	Vector corresponding to world coordinate position.
	 * The kinect itself is at point (0,0,0). Positive z is outwards,
	 * positive x is right, positive y is up.
	 */
	public static Vector getPosition(int x, int y, float depth){
		
		return p_getPosition(x,y,depth);
	}
	private static Vector p_getPosition (float x, float y, float depth){
		Vector v=new Vector();
		v.x=(float) (depth*Math.sin(Math.toRadians(getXAngle(x))));
		v.y=(float) (depth*Math.sin(Math.toRadians(getYAngle(y))));
		//v.z=(float) (depth*Math.cos(Math.toRadians(getXAngle(x))));
		v.z=(float) (depth);
		//return new Vector(getXAngle(x), getYAngle(y), depth);
		return v;
	}
	public static Vector getWorldPosition (float x, float y, float depth){
		Vector v=new Vector();
		v.x=(float) (depth*Math.sin(Math.toRadians(getXAngle(x))));
		v.y=(float) (depth*Math.sin(Math.toRadians(getYAngle(y)-getFloorAngle())));
		//v.z=(float) (depth*Math.cos(Math.toRadians(getXAngle(x))));
		v.z=(float) (depth);
		//return new Vector(getXAngle(x), getYAngle(y), depth);
		return v;
	}
	public static Point getScreenPosition(Vector v){
		Point p=new Point();
		p.x = (int) getXPixel(Math.toDegrees(Math.asin(v.x/v.z)));
		p.y = 560 - (int) getXPixel(Math.toDegrees(Math.asin(v.y/v.z)) + getFloorAngle());
		return p;
	}
	public static Vector getKinectCoordsFromWorldCoords(Vector v){
		//simply rotate 'v' around the x axis. viewAngle of 0 corresponds to the vector (0,0,1). Positive angle is down.
		// z = cos(theta);
		// y = sin(theta);
		double ang = Math.toRadians(floor_angle);
		Vector out = new Vector(v.x,v.y,v.z);
		out.z=(float) (v.z*Math.cos(-ang) + v.y*Math.sin(-ang));
		out.y=(float) (v.y*Math.cos(-ang) - v.z*Math.sin(-ang));
		return out;
	}
	@Deprecated
	/**
	 * Use getScreenPosition(Vector) instead.
	 */
	public static Point getScreenCoordsFromKinectCoords(Vector v){
		//System.out.
		ln(v);
		//3D projection of vector 'v' onto a view plane.
		Plane viewPlane = new Plane(new Vector(0,0,1), new Vector(0,0,1));
		Vector ray = Vector.mult(-1, v).normal();
		//System.out.println(ray + ", "+ray.length());
		Vector projected = viewPlane.getIntersectionWithLine(v, ray);
		//System.out.println("\t"+projected);
		projected.x*=28.5f;
		projected.y*=-21.5f;
		projected.x=24.289f*projected.x + 320.02f;
		projected.y = 41.035f*projected.y + 311.01f;
		Point viewPoint = new Point((int)(projected.x), (int)(projected.y));
		return viewPoint;
	}
	public static Vector getExpectedWorldFloorPosition(float x, float y){
		return getWorldPosition(x,y,getExpectedFloorDepth((int)x, (int)y));
	}
	private static Vector randPosition(int timeout){
		if(timeout==-1)timeout=Integer.MAX_VALUE;
		int d=0;
		int x=0;
		int y=0;
		while(timeout-->0){
			x = (int)(Math.random()*640);
			y = (int)(Math.random()*480);
			d = Kinect.getDepth(x,y);
			if(d>1){
				return Kinect.getPosition(x, y);
			}
		}
		return null;
	}
	public static void calculateFloor(){
		Plane f= RANSACPlane(100,200);
		Vector f_normal=f.normal;
		Vector pos = Kinect.getPosition(320, 240);
		pos.multiply(-1);
		pos.normalize();
		double angle = Math.acos(pos.getDotProduct(f_normal));
		angle-=(Math.PI/2.f);
		floor_angle = (float)Math.toDegrees(angle);
		floor_angle = ((float)((int)(floor_angle*1000)))/1000.f;
		floor_plane = f;
		floor_height = getExpectedWorldFloorPosition(320, 240).y;
	}
	public static float getFloorAngle(){
		return floor_angle;
	}
	public static Plane getFloorPlane(){
		return floor_plane;
	}
	public static Plane RANSACPlane(int lookNumber, int testNumber){
		Plane best_fit=new Plane(new Vector(0,1,0), new Vector(0,0,0));
		float max_fit=0;
		for(int ln=0;ln<lookNumber;ln++){
			Plane potential = new Plane(randPosition(-1), randPosition(-1), randPosition(-1));
			if(potential.normal.z<0)potential.normal.multiply(-1);
			float sum_fit=0;
			for(int tn=0;tn<testNumber;tn++){
				//System.out.println(ln+", "+tn);
				Vector test = randPosition(-1);
				if(test.z<0)test.multiply(-1);
				float dist = Math.abs(potential.distance(test));
				//System.out.println(dist+": "+potential.normal+" ||  "+potential.point+" <- "+test);
				//System.out.println(dist);
				if(dist<0.1){
					sum_fit+=1;
				}
			}
			if(sum_fit>max_fit){
				max_fit=sum_fit;
				best_fit=potential;
			}
		}
		//System.out.println("MAX: "+max_fit);
		return best_fit;
	}
	public static Vector RANSACNormal(int lookNumber, int testNumber){
		Vector best_fit=new Vector(0,1,0);
		float max_fit=0;
		for(int ln=0;ln<lookNumber;ln++){
			Vector potential = Vector.getNormal(randPosition(-1), randPosition(-1), randPosition(-1));
			if(potential.z<0)potential.multiply(-1);
			float sum_fit=0;
			for(int tn=0;tn<testNumber;tn++){
				//System.out.println(ln+", "+tn);
				Vector test = Vector.getNormal(randPosition(-1), randPosition(-1), randPosition(-1));
				if(test.z<0)test.multiply(-1);
				double angle = Math.toDegrees(Math.acos(test.getDotProduct(potential)));
				if(angle<10.0)sum_fit+=1;
			}
			if(sum_fit>max_fit){
				max_fit=sum_fit;
				best_fit=potential;
			}
		}
		//System.out.println("MAX: "+max_fit);
		return best_fit;
	}
	public static Vector RANSACNormal(){
		return RANSACNormal(640,480);
	}
	public static Vector getPosition(int x, int y){
		if(y>=480||x>=640)return null;
		return getPosition(x,y, Kinect.toFeet(ni.depthMap()[(y*640) + x]));
	}
	public static Vector getWorldPosition(int x, int y){
		if(y>=480||x>=640)return null;
		return getWorldPosition(x,y, Kinect.toFeet(ni.depthMap()[(y*640) + x]));
	}
	public static float toFeet(float depth){
		return 0.0033f*depth + 0.0006f;
	}
	public static float distToPlane(Vector in){
		return Vector.diff(in, floor_plane.point).getDotProduct(floor_plane.normal);
	}
	
}
