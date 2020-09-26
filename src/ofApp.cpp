#include "ofApp.h"

void ofApp::initGui() {
  guiActive = true;
  gui.setup("Calibration", "calibration.xml", 10, 10);
  gui.add(bandwidth.setup("Bandwidth", 5.0, 0.0, 255.0));
  gui.add(distance.setup("Distance", 80.0, 0.0, 255.0));
  gui.add(threshold.setup("Threshold", 5.0, 0.0, 20.0));
  gui.add(blur.setup("Blur Amount", 9, 0, 100));
  gui.add(minBlobSize.setup("Minimum Blob Area", 100.0, 0.0, 1000.0));
  gui.loadFromFile("calibration.xml");

  clear.addListener(this, &ofApp::clearActiveArea);
  save.addListener(this, &ofApp::saveAA);

  areaGui.setup("Active area", "area.xml", 10, 140);
  areaGui.add(editMode.setup("Edit Mode", false));
  areaGui.add(playMode.setup("Play Mode", true));
  areaGui.add(flipX.setup("Flip X", false));
  areaGui.add(flipY.setup("Flip Y", false));
  areaGui.add(clear.setup("Clear area"));
  areaGui.add(save.setup("Save"));
  areaGui.loadFromFile("area.xml");

  updateGui();
}

void ofApp::updateGui() {
  tracker.setThreshold(threshold);
  float blurValue = (blur % 2 == 0) ? blur+1 : blur;
  tracker.setBlurAmount(blurValue);
  tracker.setBlur(blur > 0);

  float minDepth = distance;
  float maxDepth = distance + bandwidth;

  tracker.setMinDepth(minDepth);
  tracker.setMaxDepth(maxDepth);
  tracker.setMinBlobSize(minBlobSize);
}

void ofApp::drawGui() {
  gui.draw();

  areaGui.draw();
}

//--------------------------------------------------------------
void ofApp::setup(){
  tracker.init();
  tracker.setMode(TRACK_DEPTH);
  backgroundSnap = false;

  drawOffset.x = ofGetWidth()/2.0 - 640/2.0;
  drawOffset.y = ofGetHeight()/2.0 - 480/2.0;

  initGui();

  //OSC
  isConnected = sender.setup(HOST, PORT);
  if(isConnected){
    ofLogNotice("KinectTouchFabric") << "Successfuly connected";
  }
  else {
    ofLogNotice("KinectTouchFabric") << "Error connecting to server";
  }

  //Active area loading
  loadActiveArea("activeArea.json");
}

void ofApp::loadActiveArea(std::string filename) {
  ofJson js;
  ofFile file(filename);
	if(file.exists()){
		file >> js;
		for(auto & vert: js){
      activeArea.addVertex(ofVec3f(vert["x"], vert["y"], vert["z"]));
		}
    activeArea.close();
	}
}

void ofApp::saveAA() {
  saveActiveArea("activeArea.json");
}

void ofApp::saveActiveArea(std::string filename) {
  ofJson toSave;
  auto verts = activeArea.getVertices();
  for(auto & v : verts)
  {
    ofJson pt;
	  pt["x"] = v.x;
	  pt["y"] = v.y;
    pt["z"] = v.z;
    toSave.push_back(pt);
  }
  ofSaveJson(filename, toSave);
}

//--------------------------------------------------------------
void ofApp::update(){
  ofBackground(100,100,100);

  if(guiActive)
  {
      updateGui();
  }

  tracker.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
  if(!backgroundSnap && ofGetFrameNum() > 60) {
    tracker.grabBackground();
    backgroundSnap = true;
  }
  if(editMode || playMode)
  {
    ofRectangle bounds = activeArea.getBoundingBox();

    ofPushMatrix();
    ofTranslate(drawOffset.x, drawOffset.y);
    tracker.drawRGB(0,0);
    ofNoFill();
    ofSetLineWidth(5);
    ofSetColor(200, 0, 0);
    activeArea.draw();

    if(playMode){
      handleTouches(bounds);
    }

    ofSetLineWidth(2);
    ofSetColor(0, 200, 0);
    ofDrawRectangle(bounds);
    ofPopMatrix();
  }
  else {
    tracker.drawDebug(ofGetWidth()/2.0 - 640/2.0, ofGetHeight()/2.0 - 480/2.0); //Draws all the visualizations
  }

  if(guiActive){
    drawGui();
    ofFill();
    if(isConnected){
      ofSetColor(0, 200, 0);
    }
    else {
      ofSetColor(200, 0, 0);
    }
    ofDrawEllipse(ofGetWidth()-20, 10, 10, 10);
  }
}

void ofApp::handleTouches(ofRectangle bounds) {
  vector<ofxKinectBlob> blobs = tracker.getActiveBlobs();
  vector<touch> active_touches;

  for(uint8_t i=0; i < blobs.size(); i++)
  {
    ofxKinectBlob b = blobs[i];

    if(activeArea.inside(b.blob.centroid)){
      ofDrawEllipse(b.blob.centroid.x, b.blob.centroid.y, 10, 10);
      touch t;
      t.position = ofVec3f(
        (b.blob.centroid.x - bounds.getMinX())/bounds.getWidth(),
        (b.blob.centroid.y - bounds.getMinY())/bounds.getHeight(),
         b.blob.area / bounds.getArea());
      t.direction = b.direction;
      t.id = b.id;
      t.speed = b.speed;
      active_touches.push_back(t);
    }
  }
  if(active_touches.size() > 0)
  {
    sendBlobs(active_touches);
    waitingForTouches = false;
  }
  else if(!waitingForTouches){
    sendBlobs(active_touches);
    waitingForTouches = true;
  }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  switch (key){
		case ' ':
			tracker.grabBackground();
			break;
    case 'w':
      tracker.setThreshold(tracker.getThreshold()+1);
      break;
    case 's':
      tracker.setThreshold(tracker.getThreshold()-1);
      break;
    case '\t':
      guiActive = !guiActive;
      break;
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
  if(editMode)
  {
    int xt = x - (ofGetWidth()/2.0 - 640/2.0);
    int yt = y - (ofGetHeight()/2.0 - 480/2.0);
    activeArea.addVertex(ofVec3f(xt,yt,0));
    activeArea.close();
  }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}

void ofApp::clearActiveArea() {
  activeArea.clear();
}


void ofApp::sendBlobs(vector<touch> touches){
  ofxOscMessage m;
  ofJson touchesJson;

  for(vector<touch>::iterator t=touches.begin(); t<touches.end(); t++)
  {
    ofJson touch;

    touch["id"] = t->id;
    touch["speed"] = t->speed;
    flipX ? touch["direction"]["x"] = t->direction.x : touch["direction"]["x"] = -t->direction.x;
    flipY ? touch["direction"]["y"] = t->direction.y : touch["direction"]["y"] = -t->direction.y;
    flipX ? touch["position"]["x"] = t->position.x : touch["position"]["x"] = 1.0-t->position.x;
    flipY ? touch["position"]["y"] = t->position.y : touch["position"]["y"] = 1.0-t->position.y;
    touch["position"]["z"] = t->position.z;

    touchesJson.push_back(touch);
    if(touchesJson.size() > 4){
      m.setAddress("/touches");
    	m.addStringArg(touchesJson.dump());
    	sender.sendMessage(m, false);
      touchesJson.clear();
    }
  }

	m.setAddress("/touches");
	m.addStringArg(touchesJson.dump());
	sender.sendMessage(m, false);
}
