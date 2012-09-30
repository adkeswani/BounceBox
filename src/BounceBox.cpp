#include "BounceBox.h"
#include <stdio.h>
#include <time.h>

//Actual webcam res is 640x480
#define WEBCAM_X_RES 320
#define WEBCAM_Y_RES 240

#define BOX_EDGE_LENGTH 100.0

#define CAMERA_DIST 195

#define SPHERE_RADIUS 5
#define SPHERE_SEPARATION 15

#define DEGREES_PER_REVOLUTION 360.0

#define COORD_SIZE 20

#define ROTATION_SPEED 0.75

#define H_MARGIN 1
#define SV_MARGIN 5

#define MIN_BLOB_AREA 10
#define MAX_BLOB_AREA 1000
#define NUM_BLOBS 1

#define RESET_CALIBRATION_KEY 'C'

const ofColor calibrationCoordColour = ofColor(255, 100, 100);

const int calibrationCoords[4][2] = {{(WEBCAM_X_RES / 4), (WEBCAM_Y_RES / 4)}, {(3 * WEBCAM_X_RES / 4), (WEBCAM_Y_RES / 4)}, {(3 * WEBCAM_X_RES / 4), (3 * WEBCAM_Y_RES / 4)}, {(WEBCAM_X_RES / 4), (3 * WEBCAM_Y_RES / 4)}};

const ofColor crosshairColour = ofColor(100, 100, 255);

const GLfloat lightPosition[] = {200.0, 400.0, 0, 0.0};

//--------------------------------------------------------------
// Setup and main drawing loop
//--------------------------------------------------------------
BounceBox::BounceBox() :
    box(BOX_EDGE_LENGTH)
{
}

void BounceBox::setup(){
    //Rendering
    ofSetVerticalSync(true);
    ofEnableSmoothing();
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);

    //Lighting
    GLfloat global_ambient[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    GLfloat diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    
    GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    
    GLfloat ambient[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

    //Camera
    camera.setTarget(box);
	camera.setDistance(CAMERA_DIST);
    camera.cacheMatrices();

    //Spheres
    spheres.push_back(Sphere(&box, ofVec3f(-SPHERE_SEPARATION, 0, 0), SPHERE_RADIUS, ofColor(255, 0, 0)));
    spheres.push_back(Sphere(&box, ofVec3f(0, 0, 0), SPHERE_RADIUS, ofColor(0, 255, 0)));
    spheres.push_back(Sphere(&box, ofVec3f(SPHERE_SEPARATION, 0, 0), SPHERE_RADIUS, ofColor(0, 0, 255)));

    //Webcam
    vidGrabber.listDevices();
    vidGrabber.initGrabber(WEBCAM_X_RES, WEBCAM_Y_RES);
    webcamImage.setUseTexture(true);
    webcamImage.allocate(WEBCAM_X_RES, WEBCAM_Y_RES);

    //Colour calibration
    resetCalibration();

    //Box rotation
    srand((unsigned)time(0));
    currRotation = ofVec3f(
        (float)rand()/(float)RAND_MAX * DEGREES_PER_REVOLUTION,
        (float)rand()/(float)RAND_MAX * DEGREES_PER_REVOLUTION,
        (float)rand()/(float)RAND_MAX * DEGREES_PER_REVOLUTION
    );
}

//Updates are done inside draw
void BounceBox::update(){
}

void BounceBox::draw(){
    //Get webcam image and display
    vidGrabber.grabFrame();
    webcamImage.setFromPixels(vidGrabber.getPixels(), WEBCAM_X_RES, WEBCAM_Y_RES);
    webcamImage.mirror(false, true);

    glDisable(GL_DEPTH_TEST);
         webcamImage.draw(0,0, APP_WIDTH, APP_HEIGHT);
	    ofDrawBitmapString("BounceBox", 10, 10);
    glEnable(GL_DEPTH_TEST);

    //Convert to HSV for token detection
    webcamImage.convertRgbToHsv();

    if (calibrating) {
        drawCalibrationCoord();
    } else {
        bounce();
    }
}

//-------------------------------------------------------------
// Colour calibration
//-------------------------------------------------------------
void BounceBox::resetCalibration(){
    minH = 255;
    minS = 255;
    minV = 255;
    maxH = 0;
    maxS = 0;
    maxV = 0;

    printf("Camera calibration:\n"
        "Position a uniquely-coloured token where the pink cross is drawn, then press any key. Repeat 3 more times.\n"
        "Then use the token to control the crosshair and press any key while the crosshair is over a sphere to push it.\n"
        "Press Shift + C to reset the calibration.\n");
    calibrating = true;
    currCalibrationCoord = 0;
}

//Displays a pink cross where the calibration is going to take its next colour from
void BounceBox::drawCalibrationCoord() {
    //Define rays in screen space and transform to world space
    ofVec3f	coordVert[2] = {ofVec3f(2 * calibrationCoords[currCalibrationCoord][0], 2 * calibrationCoords[currCalibrationCoord][1] - COORD_SIZE, -1), ofVec3f(2 * calibrationCoords[currCalibrationCoord][0], 2 * calibrationCoords[currCalibrationCoord][1] + COORD_SIZE, 1)};
	ofVec3f	coordHorz[2] = {ofVec3f(2 * calibrationCoords[currCalibrationCoord][0] - COORD_SIZE, 2 * calibrationCoords[currCalibrationCoord][1], -1), ofVec3f(2 * calibrationCoords[currCalibrationCoord][0] + COORD_SIZE, 2 * calibrationCoords[currCalibrationCoord][1], 1)};

    ofPushStyle();
        ofSetColor(calibrationCoordColour);
        ofLine(coordVert[0], coordVert[1]);
        ofLine(coordHorz[0], coordHorz[1]);
    ofPopStyle();
}

//Gets the next colour to calibrate from
//By getting the same colour from multiple coordinates we aim to minimise 
//the impact of different lighting in different parts of the image
void BounceBox::setCalibrationFromCoord() {
    //Assume you're looking from the camera's POV - the origin is at the top-right, and positive is down and left
    CvScalar s = cvGet2D(webcamImage.getCvImage(), calibrationCoords[currCalibrationCoord][1], calibrationCoords[currCalibrationCoord][0]);

    printf("H=%f, S=%f, V=%f\n", s.val[0], s.val[1], s.val[2]);

    //Hue is the colour, Saturation+Value determine the shade
    if (s.val[0] < minH) { minH = s.val[0]; }
    if (s.val[1] < minS) { minS = s.val[1]; }
    if (s.val[2] < minV) { minV = s.val[2]; }

    if (s.val[0] > maxH) { maxH = s.val[0]; }
    if (s.val[1] > maxS) { maxS = s.val[1]; }
    if (s.val[2] > maxV) { maxV = s.val[2]; }

    //Move to the next calibration coordinate
    currCalibrationCoord++;
    calibrating = (currCalibrationCoord != 4);
}


//--------------------------------------------------------------
// Bouncebox drawing
//--------------------------------------------------------------
void BounceBox::bounce() {
    detectToken();

    camera.begin();
        ofPushMatrix();
            //Set up rotation
            ofRotateX(currRotation.x);        
            ofRotateY(currRotation.y);        
            ofRotateZ(currRotation.z);        

            //Detect clicks on spheres
            if (clicked) {
                findSphereClick();
                clicked = false;
            }

            //Draw spheres
            glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
                for(std::vector<Sphere>::iterator sphere = spheres.begin(); sphere != spheres.end(); ++sphere) {
                    sphere->draw();
                }
            glDisable(GL_LIGHTING);
            glDisable(GL_LIGHT0);

            box.draw();
        ofPopMatrix();

        //Draw crosshair, no rotation
        drawCrosshair();
    camera.end();

    //Update box rotation
    currRotation += ROTATION_SPEED;
    if (currRotation.x >= DEGREES_PER_REVOLUTION || currRotation.x <= -DEGREES_PER_REVOLUTION) {
        currRotation.x = 0;
    }
    if (currRotation.y >= DEGREES_PER_REVOLUTION || currRotation.y <= -DEGREES_PER_REVOLUTION) {
        currRotation.y = 0;
    }
    if (currRotation.z >= DEGREES_PER_REVOLUTION || currRotation.z <= -DEGREES_PER_REVOLUTION) {
        currRotation.z = 0;
    }
}

//Determines the position of the coloured token
void BounceBox::detectToken() {
    //Threshold to get only token
    IplImage *imgThreshed = cvCreateImage(cvGetSize(webcamImage.getCvImage()), 8, 1);
    cvInRangeS(webcamImage.getCvImage(), cvScalar(minH - H_MARGIN, minS - SV_MARGIN, minV - SV_MARGIN), cvScalar(maxH + H_MARGIN, maxS + SV_MARGIN, maxV + SV_MARGIN), imgThreshed);

    //Convert back to OF image
    ofxCvGrayscaleImage imgThreshedOF;
    imgThreshedOF.allocate(WEBCAM_X_RES, WEBCAM_Y_RES);
    imgThreshedOF = imgThreshed;
    imgThreshedOF.erode();

    //Locate token
    contourFinder.findContours(imgThreshedOF, MIN_BLOB_AREA, MAX_BLOB_AREA, NUM_BLOBS, false);
    for (int i = 0; i < contourFinder.nBlobs; i++) {
        tokenPos = contourFinder.blobs.at(i).boundingRect.getCenter();
        tokenPos *= 2;
    }

    //Overlay thresholded image over webcam image to indicate detected token
    glDisable(GL_DEPTH_TEST);
        ofPushStyle();
            glColor4f(1.0, 1.0, 1.0, 0.8);
            imgThreshedOF.draw(0, 0, APP_WIDTH, APP_HEIGHT);
        ofPopStyle();
    glEnable(GL_DEPTH_TEST);
}

//When a key is pressed, uses the token position to detect which sphere has been clicked and where
void BounceBox::findSphereClick() {
    ofVec3f clickLine[2];
    //Transform position of token from screen to world using camera.screenToWorld
    //This assumes screen is currently showing camera's POV, but we have added extra rotation
    //Therefore we must apply our custom rotation to the position returned by screenToWorld
    
    tokenPos.z = -1;
    clickLine[0] = camera.screenToWorld(tokenPos);
    clickLine[0] = clickLine[0].rotate(-currRotation.x, ofVec3f(1,0,0)).rotate(-currRotation.y, ofVec3f(0,1,0)).rotate(-currRotation.z, ofVec3f(0,0,1));

    tokenPos.z = 1;
    clickLine[1] = camera.screenToWorld(tokenPos);
    clickLine[1] = clickLine[1].rotate(-currRotation.x, ofVec3f(1,0,0)).rotate(-currRotation.y, ofVec3f(0,1,0)).rotate(-currRotation.z, ofVec3f(0,0,1));

    //Get direction of clickLine
    ofVec3f clickLineDir = clickLine[1] - clickLine[0];

    float closestIntersectionDist = 0;
    Sphere *closestIntersectionSphere = NULL;

    //Check intersection of click ray with each sphere
    for(std::vector<Sphere>::iterator sphere = spheres.begin(); sphere != spheres.end(); ++sphere) {
        float clickToSphere = sphere->findRayIntersection(clickLine[0], clickLineDir);

        if (clickToSphere > 0) {
            //Check if this is the closest sphere to the click point
            if (closestIntersectionSphere == NULL || clickToSphere < closestIntersectionDist) {
                closestIntersectionDist = clickToSphere;
                closestIntersectionSphere = &(*sphere);
            }
        }
    }
    
    //Push the sphere that was closest to the click point
    if (closestIntersectionSphere != NULL) {
        closestIntersectionSphere->click(clickLine[0] + (clickLineDir.getNormalized().scale(closestIntersectionDist)), clickLine[0]);
    }
}

void BounceBox::drawCrosshair(){
    //Define rays in screen space and transform to world space
    ofVec3f	crosshairX[2] = {camera.screenToWorld(ofVec3f(tokenPos.x, 0, -1)), camera.screenToWorld(ofVec3f(tokenPos.x, ofGetHeight(), 1))};
	ofVec3f	crosshairY[2] = {camera.screenToWorld(ofVec3f(0, tokenPos.y, -1)), camera.screenToWorld(ofVec3f(ofGetWidth(), tokenPos.y, 1))};

    //Draw
    ofPushStyle();
        ofSetColor(crosshairColour);
        ofLine(crosshairX[0], crosshairX[1]);
        ofLine(crosshairY[0], crosshairY[1]);
    ofPopStyle();
}

//--------------------------------------------------------------
// Event handling
//--------------------------------------------------------------
void BounceBox::keyPressed(int key){
    if (calibrating) {
        setCalibrationFromCoord();
    } else if (key == RESET_CALIBRATION_KEY) {
        resetCalibration();
    } else {
        clicked = true;
    }
}

void BounceBox::keyReleased(int key){
}
void BounceBox::mouseMoved(int x, int y ){
}
void BounceBox::mouseDragged(int x, int y, int button){
}
void BounceBox::mousePressed(int x, int y, int button){
}
void BounceBox::mouseReleased(int x, int y, int button){
}
void BounceBox::windowResized(int w, int h){
}
void BounceBox::gotMessage(ofMessage msg){
}
void BounceBox::dragEvent(ofDragInfo dragInfo){ 
}
