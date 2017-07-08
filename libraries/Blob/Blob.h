
#ifndef Blob_h
#define Blob_h

// need?
// #include "Arduino.h" 
#include "Vector.h"

class Blob
{
	public:
	    bool active;
	    bool updated;
		int minNote;
		int maxNote;
		int id;
		int note;
		int inputScalingFactor;
	    Vector p;
	    Vector initialPos;
	    Vector target;
	    Vector dir;
		Blob();
		void newBlob(float _x,float _y, float _z);
		void setBlob(float _x,float _y, float _z);
		void setInputScalingFactor(int i);
		void reset();
		void disable();
		void setID(int _id);
		int getNote();
		float getBend();
		float getVelocity();
		float getModulation();
		float map(float val, float min, float max, float a, float b);
};
#endif

