/*

*/

#include <BlobTracker.h>
#include <Blob.h>
#include <Vector.h>
//==================================
//GLOBALS:
//==================================
BlobTracker t(16, 64, 2);		    // Tracker initialisation (Blobs, inputAR-width,inputAR-height)
int data [64] [2];			        // array to handle input data
int muxPin[] = {3, 2, 4, 5};        // {4, 5, 3, 2};
int analogPin[] = {A1, A0, A2, A3, A4, A5, A6, A7};  // well... just don't think about it to much^^

const unsigned char PS_16 = (1 << ADPS2);            // used to reset prescaler bits !
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

//==================================
// SETUP
//==================================
void setup() {
    ADCSRA &= ~PS_128;              // remove bits set by Arduino library
    ADCSRA |= PS_16;                // set our own prescaler to 16
    for (int i = 0; i < 4; i++)
        pinMode(muxPin[i], OUTPUT);
    Serial.begin(115200);           // for midi
    delay(50);                      // make sure everything is' spinning nicly before initializing the blobtracker!

    setAr();	                    // initialize sensors:)
    t.initializeSensors(data);      // set up error correction AR
    t.setNormalizationValue(100);   // default: 512, should be somewhere around 50-100 i think... need to experiment! :)
    t.setPitchBendRange(5);		    // default: 24
    t.setMaxError(1);               // find right value! (1 seems to work ok for now)
    t.setMinMaxNote(50, 58);        // set lowest and highest key (in midi-val); //  make adjustable?
    t.minReleasePressure = 0.01;    // adjust attack-and release tresholds!
    t.minAttackPressure  = 0.02;

}
//==================================
// LOOP
//==================================
void loop() {
    setAr();			// update input data
    t.update(data);	    // update BlobTracker
    delay(1);			// need? ... naah! :DEC
}
//==================================
// INPUT MANAGEMENT
//==================================
void setAr() {
    // loop through all sensors:
    // for every mux
    for (int sensor = 0; sensor < 16; sensor++) {     // for every sensor
        for (int bit = 0; bit < 4; bit++) {           // set mux output
            if (bitRead(sensor, bit) == 1) digitalWrite(muxPin[bit], HIGH);
            else                           digitalWrite(muxPin[bit], LOW);
        }
        for (int mux = 0; mux < 8; mux++) {
            // read sensors into data-array
            // maybe abs isn't perfect after all? ... need to test! :)
            if (sensor < 8) data[mux * 8 + sensor][0]      = abs(analogRead(analogPin[mux]) - 512);
            else            data[mux * 8 + 15 - sensor][1] = abs(analogRead(analogPin[mux]) - 512);
        }
    }
}
