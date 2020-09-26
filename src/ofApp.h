#pragma once

#include "ofMain.h"
#include "ofxKinectTracker.h"
#include "ofxGui.h"
#include "ofxOsc.h"


#define HOST "127.0.0.1"
/// send port
#define PORT 3333

struct touch {
    int id;
    ofVec3f position;
    ofVec3f direction;
    float speed;
};

class ofApp : public ofBaseApp{

	public:
    void initGui();
    void updateGui();
    void drawGui();

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

    void clearActiveArea();
    void loadActiveArea(std::string filename);
    void saveAA();
    void saveActiveArea(std::string filename);
    void handleTouches(ofRectangle bounds);
    void sendBlobs(vector<touch> touches);

		ofxKinectTracker tracker;
    ofVec3f drawOffset;
    bool backgroundSnap;

    bool guiActive;
    ofxPanel gui;
    ofxPanel areaGui;

    ofxFloatSlider bandwidth;
    ofxFloatSlider distance;
    ofxFloatSlider threshold;
    ofxIntSlider blur;
    ofxFloatSlider minBlobSize;
    ofxToggle editMode;
    ofxToggle playMode;
    ofxButton clear;
    ofxButton save;
    ofxToggle flipX;
    ofxToggle flipY;

    ofPolyline activeArea;

    //Comms
    bool isConnected;
    ofxOscSender sender;
    bool waitingForTouches;
};
