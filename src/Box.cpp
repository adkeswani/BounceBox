#include "Box.h"

#define NUM_FACES_ON_BOX 6
#define NUM_SQUARES_PER_SIDE 15
#define MESH_TRANSPARENCY 0.5

#define FRONT 0
#define BACK 1
#define LEFT 2
#define RIGHT 3
#define TOP 4
#define BOTTOM 5

#define TRANSPARENCY_THRESHOLD 0.05

//Create the meshes that make up the box
//The box is made up of a vector of faces
//Each face is made up of a vector of rows
//Each row is an ofMesh
Box::Box(float sideLength) {
    this->sideLength = sideLength;
    squareSideLength = sideLength / NUM_SQUARES_PER_SIDE;
    
    //Loop through for each face
    for (int k = 0; k < NUM_FACES_ON_BOX; k++) {
        vector<ofMesh> face;

        //Loop through each row on a face
        for (int i = 0; i < NUM_SQUARES_PER_SIDE; i++) {
            ofMesh row;
            row.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

            float currYDisp = i * squareSideLength;
            
            int currIndex = 0;
        
            //Start row
            row.addIndex(ofIndexType(currIndex++));
            row.addColor(ofColor(255, 255, 255, 255));
            row.addVertex(ofVec3f(0, currYDisp, 0));
            row.addIndex(ofIndexType(currIndex++));
            row.addColor(ofColor(255, 255, 255, 255));
            row.addVertex(ofVec3f(0, currYDisp + squareSideLength, 0));

            //Create row
            for (int j = 1; j <= NUM_SQUARES_PER_SIDE; j++) {
                row.addIndex(ofIndexType(currIndex++));
                row.addColor(ofColor(255, 255, 255, 255));
                row.addVertex(ofVec3f(j * squareSideLength, currYDisp, 0));
                row.addIndex(ofIndexType(currIndex++));
                row.addColor(ofColor(255, 255, 255, 255));
                row.addVertex(ofVec3f(j * squareSideLength, currYDisp + squareSideLength, 0));
            }
            face.push_back(row);
        }
        faces.push_back(face);
    }
}

//Does the setup for and calls the drawFace method for each face
//such that they form a box and so that the hit methods work correctly
void Box::customDraw()
{
    ofPushMatrix();
        ofTranslate(-sideLength / 2, -sideLength / 2, sideLength / 2);
        drawFace(FRONT);
    ofPopMatrix();

    ofPushMatrix();
        ofTranslate(-sideLength / 2, -sideLength / 2, -sideLength / 2);
        drawFace(BACK);
    ofPopMatrix();

    ofPushMatrix();
        ofRotateY(-90);
        ofTranslate(-sideLength / 2, -sideLength / 2, -sideLength / 2);
        drawFace(RIGHT);
    ofPopMatrix();

    ofPushMatrix();
        ofRotateY(-90);
        ofTranslate(-sideLength / 2, -sideLength / 2, sideLength / 2);
        drawFace(LEFT);
    ofPopMatrix();

    ofPushMatrix();
        ofRotateX(90);
        ofTranslate(-sideLength / 2, -sideLength / 2, -sideLength / 2);
        drawFace(TOP);
    ofPopMatrix();

    ofPushMatrix();
        ofRotateX(90);
        ofTranslate(-sideLength / 2, -sideLength / 2, sideLength / 2);
        drawFace(BOTTOM);
    ofPopMatrix();
}

//Returns true and frees a hitDetail struct if it is almost transparent
bool isAlmostTransparent(HitDetails hit) {
    bool almostTransparent = false;

    if ((*hit).color.a <= TRANSPARENCY_THRESHOLD) {
        almostTransparent = true;
        free(hit);
    }

    return almostTransparent;
}

//Draws up a face of the box. Matrices to position face must be
//set up before calling this method
void Box::drawFace(int faceIndex) {
    vector<ofMesh> face = faces.at(faceIndex);

    //Start by drawing all of the coloured squares that represents where the box has been hit

    //Set all colours to transparent black
    for(int i = 0; i < NUM_SQUARES_PER_SIDE; i++) {
        vector<ofIndexType> rowIndices = face.at(i).getIndices();
        for(std::vector<ofIndexType>::iterator index = rowIndices.begin(); index != rowIndices.end(); ++index) {
            face.at(i).setColor(*index, ofColor(0, 0, 0, 0));
        }
    }

    //Set the colours where hits have occurred
    for(std::vector<HitDetails>::iterator hit = hits.begin(); hit != hits.end(); ++hit) {
        if ((*hit)->faceIndex == faceIndex) {
            
            face.at((*hit)->rowIndex).setColor((*hit)->colIndex, (*hit)->color);

            (*hit)->color.a *= 0.99;
        }
    }

	glDisable(GL_DEPTH_TEST);
        //Redraw the now-coloured box
        for(std::vector<ofMesh>::iterator row = face.begin(); row != face.end(); ++row) {
            row->draw();
        }

        //Make the box wireframe a transparent white by setting all colours to this colour
        for(std::vector<ofMesh>::iterator row = face.begin(); row != face.end(); ++row) {
            vector<ofIndexType> rowIndices = row->getIndices();
            for(std::vector<ofIndexType>::iterator index = rowIndices.begin(); index != rowIndices.end(); ++index) {
                row->setColor(*index, ofColor(255, 255, 255, 32));
            }
        }

        //Draw wireframe
        for(std::vector<ofMesh>::iterator row = face.begin(); row != face.end(); ++row) {
            row->drawWireframe();
        }
	glEnable(GL_DEPTH_TEST);

    //Delete hits if they are almost transparent
    hits.erase(remove_if(hits.begin(), hits.end(), isAlmostTransparent), hits.end());
}

//Determines the square where a hit occurred and accordingly sets up the arguments for createHits
void Box::hit(bool hitX, bool hitY, bool hitZ, ofVec3f hitPt, ofColor color) {
    //Determine which square would have been hit
    int hitXSquare = (int)((hitPt.x + (sideLength / 2)) / squareSideLength);
    if (hitXSquare >= NUM_SQUARES_PER_SIDE) {
        hitXSquare = NUM_SQUARES_PER_SIDE - 1;
    }

    int hitYSquare = (int)((hitPt.y + (sideLength / 2)) / squareSideLength);
    if (hitYSquare >= NUM_SQUARES_PER_SIDE) {
        hitYSquare = NUM_SQUARES_PER_SIDE - 1;
    }

    int hitZSquare = (int)((hitPt.z + (sideLength / 2)) / squareSideLength);
    if (hitZSquare >= NUM_SQUARES_PER_SIDE) {
        hitZSquare = NUM_SQUARES_PER_SIDE - 1;
    }

    //Create hit on the appropriate face
    color.a = 255;
    if (hitZ && hitPt.z > 0) {
        createHits(FRONT, hitYSquare, hitXSquare, ofColor(color));
    } else if (hitZ && hitPt.z < 0) {
        createHits(BACK, hitYSquare, hitXSquare, ofColor(color));
    } else if (hitX && hitPt.x > 0) {
        createHits(RIGHT, hitYSquare, hitZSquare, ofColor(color));
    } else if (hitX && hitPt.x < 0) {
        createHits(LEFT, hitYSquare, hitZSquare, ofColor(color));
    } else if (hitY && hitPt.y > 0) {
        createHits(TOP, hitZSquare, hitXSquare, ofColor(color));
    } else if (hitY && hitPt.y < 0) {
        createHits(BOTTOM, hitZSquare, hitXSquare, ofColor(color));
    }
}

//Creates and adds a hitDetails struct for a hit on the given face and rows
void Box::createHits(int faceIndex, int rowSquare, int colSquare, ofColor color) {
    int baseColIndex = colSquare * 2;

    //Set the colours for indices in the row the hit square is in
    for (int i = 0; i < 4; i++) {
        hits.push_back(createHitDetails(faceIndex, rowSquare, baseColIndex + i, color));
    }

    //Set the colours for indices in the row below the hit square
    if (rowSquare > 0) {
        hits.push_back(createHitDetails(faceIndex, rowSquare - 1, baseColIndex + 1, ofColor(color)));
        hits.push_back(createHitDetails(faceIndex, rowSquare - 1, baseColIndex + 3, ofColor(color)));
    }

    //Set the colours for indices in the row above the hit square
    if (rowSquare < NUM_SQUARES_PER_SIDE - 1) {
        hits.push_back(createHitDetails(faceIndex, rowSquare + 1, baseColIndex + 0, ofColor(color)));
        hits.push_back(createHitDetails(faceIndex, rowSquare + 1, baseColIndex + 2, ofColor(color)));
    }
}

//Provide simple way to initialize a hitDetail struct
HitDetails Box::createHitDetails(int faceIndex, int rowIndex, int colIndex, ofColor color) {
    HitDetails hd = (HitDetails)malloc(sizeof(hitDetails));
    hd->faceIndex = faceIndex;
    hd->rowIndex = rowIndex;
    hd->colIndex = colIndex;
    hd->color = color;

    return hd;
}

float Box::getSideLength() {
    return sideLength;
}
