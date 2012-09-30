#include "Sphere.h"

#define DECEL_RATE 0.99
#define VEL_SCALE 5

const GLfloat specular[] = {255.0, 255.0, 255.0, 0.5};
const GLfloat shininess[] = {128.0};

Sphere::Sphere (Box *bounceBox, ofVec3f centre, int radius, ofColor color) {
    this->bounceBox = bounceBox;
    this->centre = centre;
    this->radius = radius;
    this->clickPt = NULL;
    this->currVel = ofVec3f(0.0,0.0,0.0);
    this->color = color; 
}

//Finds the intersection between the given ray and this sphere
//Does not detect intersections behind the origin and returns a negative distance if there was no intersection
float Sphere::findRayIntersection(ofVec3f origin, ofVec3f direction) {
    //Get direction from origin of clickLine to centre of sphere
    ofVec3f originToCentreDir = centre - origin;

    //Get projection of clickToCentreDir onto direction (i.e. the vector in same direction as direction with distance based on perpendicular dropdown from clickToCentreDir)
    ofVec3f projection = direction.getScaled(direction.dot(originToCentreDir) / direction.length());
    
    //Get the point where the projection would lie on clickLine
    ofVec3f projPoint = origin + projection;

    //Find distance between projPoint and centre
    float projToCentre = projPoint.distance(centre);

    float originToIntersection = -1;

    //Check if that's within the radius, that means we have an intersection
    if (projToCentre < radius) {
        //Find the distance between the intersection and projPoint
        float intersectionToProj = sqrt(pow(radius, 2) - pow(projToCentre, 2));

        //Finally, the distance to the intersection point from the click origin
        float originToProj = origin.distance(projPoint);
        originToIntersection = originToProj - intersectionToProj;
    }

    return originToIntersection;
}

//Pushes the sphere at the given point from the direction indicated by the click origin
void Sphere::click(ofVec3f clickIntersection, ofVec3f clickOrigin) {
    //Store the clickPt so we can display a sphere there
    clickPt = clickIntersection; 

    //Velocity is determined by finding angle between:
    //- The vector from intersection to centre point
    //- The vector from click origin to centre
    //Then multiply cos of angle by vector from intersection to centre point, scale and add to current velocity
    ofVec3f clickIntersectionToCentre = centre - clickIntersection;
    ofVec3f clickOriginToCentre = centre - clickOrigin;

    float angle = clickOriginToCentre.angleRad(clickIntersectionToCentre);

    currVel += clickIntersectionToCentre.getScaled(VEL_SCALE * cos(angle));
}

//Updates the position of the sphere each frame
void Sphere::updatePos()
{
    centre += currVel;

    //Detect if the sphere has hit the side of the box and reverse the appropriate component of the velocity
    //For a 3D bounce, whatever component is perpendicular to the surface gets reversed
    if (abs(centre.x) > bounceBox->getSideLength() / 2) {
        currVel.x = -currVel.x;
        centre.x = bounceBox->getSideLength() * (centre.x > 0 ? 1 : -1) / 2;
        bounceBox->hit(true, false, false, centre, color);
    } else if (abs(centre.y) > bounceBox->getSideLength() / 2) {
        currVel.y = -currVel.y;
        centre.y = bounceBox->getSideLength() * (centre.y > 0 ? 1 : -1) / 2;
        bounceBox->hit(false, true, false, centre, color);
    } else if (abs(centre.z) > bounceBox->getSideLength() / 2) {
        currVel.z = -currVel.z;
        centre.z = bounceBox->getSideLength() * (centre.z > 0 ? 1 : -1) / 2;
        bounceBox->hit(false, false, true, centre, color);
    }

    currVel = currVel * DECEL_RATE;
}
   
void Sphere::customDraw()
{
    updatePos();

    GLfloat ambient[] = {color.r / 255.0, color.g / 255.0, color.b / 255.0, 1.0};

    //Enable lighting to create shiny sphere
    ofPushStyle();
    glPushAttrib(GL_LIGHTING_BIT);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        
        ofSphere(centre.x, centre.y, centre.z, radius);

        //Draw another small sphere where the sphere was clicked
        if (clickPt != NULL) {
            ofPushStyle();
                GLfloat ambient[] = {255.0, 255.0, 255.0, 255.0};
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

                ofSphere(clickPt.x, clickPt.y, clickPt.z, 1);
            ofPopStyle();
        }
    glPopAttrib();
    ofPopStyle();
}
