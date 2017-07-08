/*
class for a tracking a single blob
*/

#include "Blob.h"
#include "Arduino.h"
//============================================================
//blob class
//============================================================
/*Blob::Blob(int _id) {
id=_id;
active = false;
updated = false;
p = Vector(0,0,0);
initialPos = Vector(0,0,0);
target = Vector(0,0,0);
dir = Vector(0,0,0);
}*/
Blob::Blob() {
	active = false;
	updated = false;
	p = Vector();
	initialPos = Vector();
	target = Vector();
	dir = Vector();
	minNote = 21;
	maxNote = 108;
}
//============================================================
void Blob::newBlob(float _x, float _y, float _z) {
	p.set(_x, _y, _z);
	initialPos.set(_x, _y, _z);
	dir.set(0, 0, 0);
	target = p.get();
	updated = true;
	active = true;
	note = (int) map(_x,0,inputScalingFactor,minNote,maxNote);
}
//=========================================================
void Blob::setBlob(float _x, float _y, float _z) {
	dir.set(_x, _y, _z);
	dir.sub(p);
	p.set(_x, _y, _z);
	target.set(p.x,p.y,p.z);
	target.sub(dir);
	updated = true;
	active = true;
}
//=========================================================
void Blob::setInputScalingFactor(int i){
	inputScalingFactor = i;
}
//=========================================================
void Blob::reset() {
	updated=false;
}
//=========================================================
void Blob::disable() {
	updated = false;
	active = false;
}
void Blob::setID(int _id){
	id = _id;
}
//=========================================================
float Blob::getBend(){
	return (p.x - (int) initialPos.x) ;
}
//=========================================================
float Blob::getVelocity(){
	return map(p.z,0,1,0,127);
}
//=========================================================
float Blob::getModulation(){
	//Serial.println(p.y);
	return map(p.y,0,1,0,127);
}
//=========================================================
int Blob::getNote(){
	return note;
}
//=========================================================
float Blob::map(float val, float min, float max, float a, float b){
	return (((b-a) * (val-min)) / (max-min) ) + a;
}
