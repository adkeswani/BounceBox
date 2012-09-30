#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "Sphere.h"
#include "Box.h"

#define APP_WIDTH 640
#define APP_HEIGHT 480

class BounceBox : public ofBaseApp{
	public:
        BounceBox();
       	void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    private:
        void resetCalibration();
        void setCalibrationFromCoord();
        void drawCrosshair();
        void detectToken();
        void findSphereClick();
        void bounce();
        void drawCalibrationCoord();

        ofEasyCam camera;

        Box box;
        vector<Sphere> spheres;

        bool clicked;

        ofVec3f currRotation;

        ofVideoGrabber vidGrabber;
        ofxCvColorImage webcamImage;

        ofxCvContourFinder contourFinder;
		ofVec3f tokenPos;

        bool calibrating;
        int minH, minS, minV;
        int maxH, maxS, maxV;
        int currCalibrationCoord;
};
