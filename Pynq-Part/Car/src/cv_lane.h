/*
 * @Author: Sauron Wu
 * @GitHub: wutianze
 * @Email: 1369130123qq@gmail.com
 * @Date: 2019-11-12 12:38:24
 * @LastEditors: Sauron Wu
 * @LastEditTime: 2019-12-02 16:57:27
 * @Description: 
 */
#include <assert.h>
#include <stdio.h>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>
#include <list>

class ExpMovingAverage {
private:
	double alpha; // [0;1] less = more stable, more = less stable
    double oldValue;
	bool unset;
public:
    ExpMovingAverage();

	void clear();

    void add(double value);

	double get();
};

CvPoint2D32f sub(CvPoint2D32f b, CvPoint2D32f a);
CvPoint2D32f mul(CvPoint2D32f b, CvPoint2D32f a);
CvPoint2D32f add(CvPoint2D32f b, CvPoint2D32f a);
CvPoint2D32f mul(CvPoint2D32f b, float t);
float dot(CvPoint2D32f a, CvPoint2D32f b);
float dist(CvPoint2D32f v);

CvPoint2D32f point_on_segment(CvPoint2D32f line0, CvPoint2D32f line1, CvPoint2D32f pt);
float dist2line(CvPoint2D32f line0, CvPoint2D32f line1, CvPoint2D32f pt);
//----------------------------------------------------------------------


#define LANE_DET_WIDTH 160
#define LANE_DET_HEIGHT 120
struct Lane {
	Lane(){}
	Lane(CvPoint a, CvPoint b, float angle, float kl, float bl): p0(a),p1(b),angle(angle),
		votes(0),visited(false),found(false),k(kl),b(bl) { }

	CvPoint p0, p1;
	int votes;
	bool visited, found;
	float angle, k, b;
};

struct Status {
	Status():reset(true),lost(0){}
	ExpMovingAverage k, b;
	bool reset;
	int lost;
};

#define GREEN CV_RGB(0,255,0)
#define RED CV_RGB(255,0,0)
#define BLUE CV_RGB(255,0,255)
#define PURPLE CV_RGB(255,0,255)

extern Status laneR, laneL; 

enum{
    SCAN_STEP = 5,			  // in pixels
	LINE_REJECT_DEGREES = 10, // in degrees
    BW_TRESHOLD = 250,		  // edge response strength to recognize for 'WHITE'
    BORDERX = 10,			  // px, skip this much from left & right borders
	MAX_RESPONSE_DIST = 5,	  // px
	
	CANNY_MIN_TRESHOLD = 1,	  // edge detector minimum hysteresis threshold
	CANNY_MAX_TRESHOLD = 100, // edge detector maximum hysteresis threshold

	HOUGH_TRESHOLD = 50,		// line approval vote threshold
	HOUGH_MIN_LINE_LENGTH = 50,	// remove lines shorter than this treshold
	HOUGH_MAX_LINE_GAP = 100,   // join lines to one with smaller than this gaps

};

#define K_VARY_FACTOR 0.3f
#define B_VARY_FACTOR 10
#define MAX_LOST_FRAMES 5

int find_lane(IplImage*, IplImage*, IplImage*, IplImage*, CvMemStorage*);
