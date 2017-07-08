//============================================================
// blobBlobTracker class
//============================================================

#include "BlobTracker.h"
#include "Blob.h"
#include "Arduino.h"

//============================================================
// initialize Blobtracker (number of Blobs, inputAR-width, inputAR-height)
BlobTracker::BlobTracker(int maxBlobs, int maxX, int maxY) {
	numBlobs = maxBlobs;
	inputX = maxX;
	inputY = maxY;
	maxError = 1.0f;
	minReleasePressure = 0.003;
	minAttackPressure  = 0.007;
	activeBlobs = 0;
	normalizationValue = 512;
	for (i=0; i<numBlobs; i++) {
		blobs[i].setID(i);
		blobs[i].setInputScalingFactor(maxX);
		newBlobs[i].setID(i);
		newBlobs[i].setInputScalingFactor(maxX);
	}
	// bendrange in semis
	pitchBendRange = 24;
	// factor to match semis to bars
	pitchBendFactor =  inputX / (blobs[0].maxNote-blobs[0].minNote);
}
//============================================================
void BlobTracker::setMinMaxNote(float min, float max){
	for (i=0; i<numBlobs; i++) {
		blobs[i].minNote = min;
		blobs[i].maxNote = max;
		pitchBendFactor =  inputX / (blobs[0].maxNote-blobs[0].minNote);
	}
}
//============================================================
void BlobTracker::setPitchBendRange(int range) {
	// update this function!
	pitchBendRange = range;
}
//============================================================
void BlobTracker::setMaxError(float error) {
	maxError = error;
}
//============================================================
void BlobTracker::setNormalizationValue(int value){
	normalizationValue = value;
}
//============================================================
// sensor-initialisation -> buid errorCorrection-Ar!
void BlobTracker::initializeSensors(int ip[][2]){
	for (int x = 0; x<inputX; x++) {
		for (int y = 0; y<inputY; y++) {
			errorCorrection[x][y] = ip[x][y]; // + input[x][y]; //... why?
		}
	}
}

//============================================================
void BlobTracker::update(int ip[8][2]) {
	blobID = 0;
	activeBlobs = 0;
	//float minimumSensorValue = 0.05;
	for (int x = 0; x<inputX; x++) {
		for (int y = 0; y<inputY; y++) {
			// use absolute values? that might help with some problems...! (because then i could just normalize around 512, no?) .. need to think about that!
			input[x][y] = float(ip[x][y] - errorCorrection[x][y]) / normalizationValue;
			if(input[x][y] < minReleasePressure) input[x][y] = 0; // ignore values smaller than the absolute minimum pressure. that might be the wrong threshold value... !
			//input[x][y] = input[x][y] > minReleasePressure ? input[x][y] : 0;
		}
	}
	//find blobs. go through input, find all center bars. make new newBlob for each.
	for (i=1; i<inputX-1; i++) {
		if (isCenterBar(i) && blobID<numBlobs) {
			offset = parabolicInterpolation();
			z = (input[i-1][0]+input[i][0]+input[i+1][0]+input[i-1][1]+input[i][1]+input[i+1][1])/6; // not sure, but might just work!^^ (sum of all 6 sensors)
			y = (input[i-1][0]+input[i][0]+input[i+1][0]) / z;

			if(z > minAttackPressure){ // like this?
				(newBlobs[blobID++]).setBlob(getXPosition(i, offset), y, z); // normalize z value..!
			}

		}
	}
	// update blobs
	for (oldBlob = 0; oldBlob < numBlobs; oldBlob++) {
		if (!blobs[oldBlob].active) continue;
		for (newBlob = 0; newBlob< numBlobs; newBlob++) {
			if (!newBlobs[newBlob].active) continue;
			if (isSameBlob(oldBlob, newBlob)) {
				blobs[oldBlob].setBlob(newBlobs[newBlob].p.x, newBlobs[newBlob].p.y, newBlobs[newBlob].p.z);
				newBlobs[newBlob].active = false; // processed!
				midiNoteUpdate(blobs[oldBlob].id, blobs[oldBlob].getNote(), blobs[oldBlob].getVelocity(), blobs[oldBlob].getBend(),blobs[oldBlob].getModulation());
				activeBlobs++;
			}
		}
	}
	// not updated == delete
	for (oldBlob = 0; oldBlob < numBlobs; oldBlob++) {
		if (blobs[oldBlob].active && !blobs[oldBlob].updated) {
			blobs[oldBlob].disable(); // no longer existing.
			midiNoteOff(blobs[oldBlob].id, blobs[oldBlob].getNote(), blobs[oldBlob].getVelocity(), blobs[oldBlob].getBend(),blobs[oldBlob].getModulation());
		}
	}

	// new blobs!
	nextFreeID = 0;
	for (newBlob=0; newBlob < numBlobs; newBlob++) {
		if (!newBlobs[newBlob].active)continue;
		//find nextfreeID
		while (blobs[nextFreeID].active && nextFreeID < numBlobs-1) {
			nextFreeID++;
		}
		blobs[nextFreeID].newBlob(newBlobs[newBlob].p.x, newBlobs[newBlob].p.y, newBlobs[newBlob].p.z);
		midiNoteOn(blobs[nextFreeID].id, blobs[nextFreeID].getNote(), blobs[nextFreeID].getVelocity(), blobs[nextFreeID].getBend(),blobs[nextFreeID].getModulation());
		activeBlobs++;
	}
	// reset blobs
	for (i=0; i < numBlobs; i++) {
		blobs[i].reset(); // need?
		newBlobs[i].reset();
	}
}
//============================================================
float BlobTracker::getXPosition(int a, float b) {
	return a+b;
}
//============================================================
// true if value is bigger then previous and next values
bool BlobTracker::isCenterBar(int i) {
	a = input[i-1][0] + input[i-1][1];
	b = input[i][0]   + input[i][1];
	c = input[i+1][0] + input[i+1][1];
	if ( a<b && b>c ) {
		return true;
	}
	return false;
}
//============================================================
// simplyfied parabolic interpolation function ( for x values: -1, 0, +1);
float BlobTracker::parabolicInterpolation() {
	A = ((b - a) - (c - b)) / -2;
	B = ((a - b) + (b - c)) / -2;
	return -B / (2*A);
}
//============================================================
// if(distance oldblob(target) to newblob (current position) < maxError)
bool BlobTracker::isSameBlob(int index1, int index2) {
	if (blobs[index1].target.dist(newBlobs[index2].p) < maxError) {
		return true;
	}
	return false;
}
//============================================================
void BlobTracker::midiNoteOn(int channel, int note, float velocity, float pitchbend,float modulation){
	pitchbend = resizePitchBend(pitchbend);
	int msb = ( (int) pitchbend & (0x007F<<7))>>7;  // bend
	int lsb =  (int)pitchbend	& 0x007F;
	Serial.write(0xE0+channel);		//send bendStatus
	Serial.write(lsb);				//send bendA
	Serial.write(msb);				//send BendB

	Serial.write(0x90 + channel);	//send note on
	Serial.write(note);				//send note
	Serial.write((int)velocity);	//send velocity

	Serial.write(0xB0+channel);		//send modulation
	Serial.write(1);
	Serial.write((int)modulation);
}
//============================================================
void BlobTracker::midiNoteOff(int channel, int note, float velocity, float pitchbend,float modulation){
	pitchbend = resizePitchBend(pitchbend);
	Serial.write(0x80 + channel);	//send note on or note off command
	Serial.write(note);				//send pitch data
	Serial.write(0);				//send velocity data
}
//============================================================
void BlobTracker::midiNoteUpdate(int channel, int note, float velocity, float pitchbend,float modulation){
	pitchbend = resizePitchBend(pitchbend);
	Serial.write(0xB0+channel);		// velocity ( als "expression")
	Serial.write(7);				// 11: expression, 7: volume
	Serial.write((int)velocity);
	Serial.write(0xA0+channel);		// aftertouch -> need??
	Serial.write(note);
	Serial.write((int)velocity);
	Serial.write(0xB0+channel);		// mod
	Serial.write(1);
	Serial.write((int)modulation);
	int msb = ( (int) pitchbend & (0x007F<<7))>>7;  // bend
	int lsb =  (int)pitchbend	& 0x007F;
	Serial.write(0xE0+channel);
	Serial.write(lsb);
	Serial.write(msb);
}
//============================================================
int BlobTracker::resizePitchBend(float pitchbend){
	pitchbend = map(pitchbend * pitchBendFactor,-pitchBendRange,pitchBendRange,0,16382);// scale bars to semis, scale semis to pitch bend range
	//pitchbend = pitchbend < 0 ? 0 : pitchbend;											 // lower  clamp
	if(pitchbend < 0) pitchbend = 0;
	//	pitchbend = pitchbend > 16382 ? 16382 : pitchbend;									    // higher clamp
	else if(pitchbend > 16382) pitchbend = 16382;
	return (int) pitchbend;
}
//============================================================
float BlobTracker::map(float val, float min, float max, float a, float b){
	return (((b-a) * (val-min)) / (max-min) ) + a;
}
//============================================================
//Make sure you properly release the array or memory leaks will build up! // but i don't really care, so...^^
BlobTracker::~BlobTracker()
{
	/*
	for(i = 0; i < inputX; ++i) {
	delete [] input[i];
}
delete [] input;
delete [] blobs;
delete [] newBlobs;
*/
}
