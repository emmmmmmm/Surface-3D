//============================================================
// blobtracker class
//============================================================

#ifndef BlobTracker_h
#define BlobTracker_h

#include "Blob.h"
#include "Vector.h"

class BlobTracker {
	
	public:
		float pitchBendRange;
  		float maxError; 
		float minReleasePressure;
		float minAttackPressure;
		BlobTracker(int maxBlobs,int maxX,int maxY);
		~BlobTracker();
		void setMinMaxNote(float min, float max);
		void setPitchBendRange(int range);
		void setNormalizationValue(int value);
		void setMaxError(float error);
		void update(int ip[][2]);
		void initializeSensors(int ip[][2]);
		
	private:
		Blob blobs[5];				// adjust for higher number of max blobs
		Blob newBlobs[5];			// adjust for higher number of max blobs
		int i;
		int nextFreeID;
		int blobID;
		int oldBlob;
		int newBlob;
		int activeBlobs;
		int numBlobs;
		int inputX;
		int inputY;
		int errorCorrection[64][2];	// adjust for bigger input matrix!
		float pitchBendFactor;
		float input[64][2]; 		// adjust for bigger input matrix!
		float normalizationValue;
		float A; 
		float B;
		float a;
		float b;
		float c;
		float offset;
		float y;
		float z;   
		bool isCenterBar(int i);
		bool isSameBlob(int index1, int index2);
		float getXPosition(int a, float b);
		float parabolicInterpolation();
		int resizePitchBend(float pitchbend);		
		void midiNoteOn(int channel, int note, float velocity, float pitchbend,float modulation);
		void midiNoteOff(int channel, int note, float velocity, float pitchbend,float modulation);
  	  	void midiNoteUpdate(int channel, int note, float velocity, float pitchbend,float modulation);
		float map(float val, float min, float max, float a, float b);

};
#endif

