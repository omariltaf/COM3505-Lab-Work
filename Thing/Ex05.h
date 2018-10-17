// Ex05.h

/////////////////////////////////////////////////////////////////////////////
// NOTE!!!
// DON'T edit these files, do your coding in MyThing!
/////////////////////////////////////////////////////////////////////////////

#ifndef LAB_05_H 
#define LAB_05_H

// what time did we start?
unsigned long firstSliceMillis;

// what time did we last run this action?
unsigned long lastSliceMillis;

// a control iterator for slicing up the main loop ///////////////////////////
int loopIteration = 0;
const int LOOP_ROLLOVER = 25000000; // how many loops per action sequence

#endif
