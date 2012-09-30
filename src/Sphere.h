#pragma once

#include "ofMain.h"
#include <stdio.h>
#include "Box.h"

class Sphere : public ofNode
{
public:
    Sphere(Box *bounceBox, ofVec3f centre, int radius, ofColor color);
	void	customDraw();
    void    click(ofVec3f clickIntersection, ofVec3f clickOrigin);
    float   findRayIntersection(ofVec3f origin, ofVec3f direction);

private:
    void updatePos();

    Box *bounceBox;
    ofVec3f centre;
    float radius;
    ofVec3f clickPt;
    ofVec3f currVel;
    ofColor color;
};
