#include "ofMain.h"
#include "BounceBox.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( ){
    ofAppGlutWindow window;
	ofSetupOpenGL(&window, APP_WIDTH, APP_HEIGHT, OF_WINDOW);
	ofRunApp(new BounceBox());
}
