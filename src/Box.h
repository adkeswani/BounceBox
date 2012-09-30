#pragma once

class Sphere;

#include "ofMain.h"
#include <math.h>

typedef struct hitDetails {
    int faceIndex;
    int rowIndex;
    int colIndex;
    ofColor color;
} hitDetails;
typedef hitDetails *HitDetails;

class Box : public ofNode
{
public:
    Box(float sideLength);
	void customDraw();
    float getSideLength();
    void hit(bool hitX, bool hitY, bool hitZ, ofVec3f hitPt, ofColor color);
private:
    void drawFace(int faceIndex);
    HitDetails createHitDetails(int faceIndex, int rowIndex, int colIndex, ofColor color);
    void createHits(int faceIndex, int rowSquare, int colSquare, ofColor color);

    float sideLength;
    float squareSideLength;
    vector< vector<ofMesh> > faces;
    vector< HitDetails > hits;
};
