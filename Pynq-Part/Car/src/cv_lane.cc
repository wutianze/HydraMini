#include "cv_lane.h"
using namespace std;

ExpMovingAverage::ExpMovingAverage() {
this->alpha = 0.7;
this->unset = true;
}
  
void ExpMovingAverage::clear() {
this->unset = true;
}
void ExpMovingAverage::add(double value) {
        if (this->unset) {
            this->oldValue = value;
			this->unset = false;
        }
        double newValue = this->oldValue + this->alpha * (value - this->oldValue);
        this->oldValue = newValue;
    }
  
double ExpMovingAverage::get() {
return this->oldValue;	}


CvPoint2D32f sub(CvPoint2D32f b, CvPoint2D32f a) { return cvPoint2D32f(b.x-a.x, b.y-a.y); }
CvPoint2D32f mul(CvPoint2D32f b, CvPoint2D32f a) { return cvPoint2D32f(b.x*a.x, b.y*a.y); }
CvPoint2D32f add(CvPoint2D32f b, CvPoint2D32f a) { return cvPoint2D32f(b.x+a.x, b.y+a.y); }
CvPoint2D32f mul(CvPoint2D32f b, float t) { return cvPoint2D32f(b.x*t, b.y*t); }
float dot(CvPoint2D32f a, CvPoint2D32f b) { return (b.x*a.x + b.y*a.y); }
float dist(CvPoint2D32f v) { return sqrtf(v.x*v.x + v.y*v.y); }

CvPoint2D32f point_on_segment(CvPoint2D32f line0, CvPoint2D32f line1, CvPoint2D32f pt){
	CvPoint2D32f v = sub(pt, line0);
	CvPoint2D32f dir = sub(line1, line0);
	float len = dist(dir);
	float inv = 1.0f/(len+1e-6f);
	dir.x *= inv;
	dir.y *= inv;

	float t = dot(dir, v);
	if(t >= len) return line1;
	else if(t <= 0) return line0;

	return add(line0, mul(dir,t));
}

float dist2line(CvPoint2D32f line0, CvPoint2D32f line1, CvPoint2D32f pt){
	return dist(sub(point_on_segment(line0, line1, pt), pt));
}
//--------------------------------------------------------------------

//#define USE_VIDEO 1
#define SHOW_TMP_FRAME 1

Status laneR, laneL;
void crop(IplImage* src,  IplImage* dest, CvRect rect) {
    cvSetImageROI(src, rect); 
    cvCopy(src, dest); 
    cvResetImageROI(src); 
}

void FindResponses(IplImage *img, int startX, int endX, int y, std::vector<int>& list)
{
    // scans for single response: /^\_

	const int row = y * img->width * img->nChannels;
	unsigned char* ptr = (unsigned char*)img->imageData;

    int step = (endX < startX) ? -1: 1;
    int range = (endX > startX) ? endX-startX+1 : startX-endX+1;

    for(int x = startX; range>0; x += step, range--)
    {
        if(ptr[row + x] <= BW_TRESHOLD) continue; // skip black: loop until white pixels show up

        // first response found
        int idx = x + step;

        // skip same response(white) pixels
        while(range > 0 && ptr[row+idx] > BW_TRESHOLD){
            idx += step;
            range--;
        }

		// reached black again
        if(ptr[row+idx] <= BW_TRESHOLD) {
            list.push_back(x);
        }

        x = idx; // begin from new pos
    }
}

//return if can find a line
bool processSide(std::vector<Lane> lanes, IplImage *edges, bool right) {

	Status* side = right ? &laneR : &laneL;

	// response search
	int w = edges->width;
	int h = edges->height;
	const int BEGINY = 0;
	const int ENDY = h-1;
	const int ENDX = right ? (w-BORDERX) : BORDERX;
	int midx = w/2;
	int midy = edges->height/2;
	// unsigned char* ptr = (unsigned char*)edges->imageData;

	// show responses
	int* votes = new int[lanes.size()];
	for(unsigned int i=0; i<lanes.size(); i++) votes[i++] = 0;

	for(int y=ENDY; y>=BEGINY; y-=SCAN_STEP) {
		std::vector<int> rsp;
		FindResponses(edges, midx, ENDX, y, rsp);

		if (rsp.size() > 0) {
			int response_x = rsp[0]; // use first reponse (closest to screen center)

			float dmin = 9999999;
			float xmin = 9999999;
			int match = -1;
			for (unsigned int j=0; j<lanes.size(); j++) {
				// compute response point distance to current line
				float d = dist2line(
						cvPoint2D32f(lanes[j].p0.x, lanes[j].p0.y), 
						cvPoint2D32f(lanes[j].p1.x, lanes[j].p1.y), 
						cvPoint2D32f(response_x, y));

				// point on line at current y line
				int xline = (y - lanes[j].b) / lanes[j].k;
				int dist_mid = abs(midx - xline); // distance to midpoint

				// pick the best closest match to line & to screen center
				if (match == -1 || (d <= dmin && dist_mid < xmin)) {
					dmin = d;
					match = j;
					xmin = dist_mid;
					break;
				}
			}

			// vote for each line
			if (match != -1) {
				votes[match] += 1;
			}
		}
	}

	int bestMatch = -1;
	int mini = 9999999;
	for (unsigned int i=0; i<lanes.size(); i++) {
		int xline = (midy - lanes[i].b) / lanes[i].k;
		int dist = abs(midx - xline); // distance to midpoint

		if (bestMatch == -1 || (votes[i] > votes[bestMatch] && dist < mini)) {
			bestMatch = i;
			mini = dist;
		}
	}

	if (bestMatch != -1) {
		Lane* best = &lanes[bestMatch];
		float k_diff = fabs(best->k - side->k.get());
		float b_diff = fabs(best->b - side->b.get());

		bool update_ok = (k_diff <= K_VARY_FACTOR && b_diff <= B_VARY_FACTOR) || side->reset;

		//printf("side: %s, k vary: %.4f, b vary: %.4f, need update: %s\n", 
		//	(right?"RIGHT":"LEFT"), k_diff, b_diff, (update_ok?"no":"yes"));
		//printf("\r\033[k");
		
		if (update_ok) {
			// update is in valid bounds
			side->k.add(best->k);
			side->b.add(best->b);
			side->reset = false;
			side->lost = 0;
		} else {
			// can't update, lanes flicker periodically, start counter for partial reset!
			side->lost++;
			if (side->lost >= MAX_LOST_FRAMES && !side->reset) {
				side->reset = true;
			}
		}

	} else {
		//printf("no lanes detected - lane tracking lost! counter increased\n");
		side->lost++;
		if (side->lost >= MAX_LOST_FRAMES && !side->reset) {
			// do full reset when lost for more than N frames
			side->reset = true;
			side->k.clear();
			side->b.clear();
		}
		return false;
	}

	delete[] votes;
	return true;
}

//return 0 if can find both, 1 for cannot find left, 2 for cannot find right, 3 for cannot find both
int processLanes(CvSeq* lines, IplImage* edges, IplImage* temp_frame) {

	// classify lines to left/right side
	std::vector<Lane> left, right;

	for(int i = 0; i < lines->total; i++ )
    {
        CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
		int dx = line[1].x - line[0].x;                      //line[0]: rho; line[1]
		int dy = line[1].y - line[0].y;
		float angle = atan2f(dy, dx) * 180/CV_PI;

		if (fabs(angle) <= LINE_REJECT_DEGREES) { // reject near horizontal lines
			continue;
		}

		// assume that vanishing point is close to the image horizontal center
		// calculate line parameters: y = kx + b;
		dx = (dx == 0) ? 1 : dx; // prevent DIV/0!  
		float k = dy/(float)dx;
		float b = line[0].y - k*line[0].x;

		// assign lane's side based by its midpoint position 
		int midx = (line[0].x + line[1].x) / 2;
		if (midx < temp_frame->width/2) {
			left.push_back(Lane(line[0], line[1], angle, k, b));
		} else if (midx > temp_frame->width/2) {
			right.push_back(Lane(line[0], line[1], angle, k, b));
		}
    }

	// show Hough lines
    #ifdef SHOW_TMP_FRAME
    for	(unsigned int i=0; i<right.size(); i++) {
    	cvLine(temp_frame, right[i].p0, right[i].p1, CV_RGB(0, 0, 255), 2);
    }
    
    for	(unsigned int i=0; i<left.size(); i++) {
    	cvLine(temp_frame, left[i].p0, left[i].p1, CV_RGB(255, 0, 0), 2);
    }
    #endif
	int canFind = 0;
	if(!processSide(left, edges, false))canFind++;
	if(!processSide(right, edges, true))canFind = canFind + 2;
    
    #ifdef SHOW_TMP_FRAME
	// show computed lanes
	int x = temp_frame->width * 0.55f;
	int x2 = temp_frame->width;
	cvLine(temp_frame, cvPoint(x, laneR.k.get()*x + laneR.b.get()), 
		cvPoint(x2, laneR.k.get() * x2 + laneR.b.get()), CV_RGB(255, 0, 255), 2);

	x = temp_frame->width * 0;
	x2 = temp_frame->width * 0.45f;
	cvLine(temp_frame, cvPoint(x, laneL.k.get()*x + laneL.b.get()), 
		cvPoint(x2, laneL.k.get() * x2 + laneL.b.get()), CV_RGB(255, 0, 255), 2);
    #endif
	return canFind;
}

int find_lane(IplImage* frame_get, IplImage* temp_frame, IplImage* grey, IplImage* edges, CvMemStorage* houghStorage){
    if (frame_get == NULL) {
		fprintf(stderr, "Error: null frame received\n");
		assert(0);
	}
	CvSize newSz;
	newSz.width = LANE_DET_WIDTH;
	newSz.height = LANE_DET_HEIGHT;
	IplImage* frame=cvCreateImage(newSz,frame_get->depth,frame_get->nChannels);
	cvResize(frame_get,frame,CV_INTER_AREA);
	// we're interested only in road below horizont - so crop top image portion off
	crop(frame, temp_frame, cvRect(0,LANE_DET_HEIGHT/3,LANE_DET_WIDTH,LANE_DET_HEIGHT/2));
	cvCvtColor(temp_frame, grey, CV_BGR2GRAY); // convert to grayscale
	
	// Perform a Gaussian blur ( Convolving with 5 X 5 Gaussian) & detect edges
	cvSmooth(grey, grey, CV_GAUSSIAN, 5, 5);
	cvCanny(grey, edges, CANNY_MIN_TRESHOLD, CANNY_MAX_TRESHOLD);

	// do Hough transform to find lanes
	double rho = 1;
	double theta = CV_PI/180;
	CvSeq* lines = cvHoughLines2(edges, houghStorage, CV_HOUGH_PROBABILISTIC, 
		rho, theta, HOUGH_TRESHOLD, HOUGH_MIN_LINE_LENGTH, HOUGH_MAX_LINE_GAP);

	int canF = processLanes(lines, edges, temp_frame);
	 
    #ifdef SHOW_TMP_FRAME
	// show middle line
	cvLine(temp_frame, cvPoint(LANE_DET_WIDTH/2,0), 
	cvPoint(LANE_DET_WIDTH/2,LANE_DET_HEIGHT/2), CV_RGB(255, 255, 0), 1);
		
	//cvShowImage("Grey", grey);
	//cvShowImage("Edges", edges);
	cvShowImage("Color", temp_frame);
	
	//cvMoveWindow("Grey", 0, 0); 
	//cvMoveWindow("Edges", 0, frame_size.height+25);
	cvMoveWindow("Color", 0, 2*(LANE_DET_HEIGHT/2+25)); 
	cvWaitKey(5);
    #endif
	return canF;
}
