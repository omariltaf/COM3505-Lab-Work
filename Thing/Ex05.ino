// Ex05.ino
// loop slicing

/////////////////////////////////////////////////////////////////////////////
// NOTE!!!
// DON'T edit these files, do your coding in MyThing!
/////////////////////////////////////////////////////////////////////////////

// setup and loop ///////////////////////////////////////////////////////////
void setup05() {
  setup04();            // previous setups: get the MAC, set up GPIO pins, ...
  dln(startupDBG, "\nsetup05..."); // debug printout on serial, with newline

  firstSliceMillis = millis();        // remember when we began
  lastSliceMillis = firstSliceMillis; // an approximation to use in first loop
}

void loop05() { // Q what types of timings do different slices equate to?
  int sliceSize = 1000000;
  int TICK_DO_SOMETHING = 1;
  int TICK_DO_SOMETHING_ELSE = 9999999;

  // actions on individual iterations
  if(loopIteration == TICK_DO_SOMETHING) { // do some a task for this iteration
    dbg(loopDBG, "doing something, loopIteration number = ");
    dln(loopDBG, loopIteration);
  } else if(loopIteration == TICK_DO_SOMETHING_ELSE) { // some other task...
    dbg(loopDBG, "doing something else, loopIteration number = ");
    dln(loopDBG, loopIteration);
  }

  // actions on each X slices
  if(loopIteration % sliceSize == 0) { // a slice every sliceSize iterations
    dbg(loopDBG, "loopIteration number = ");
    dbg(loopDBG, loopIteration);
    dbg(loopDBG, ", slice lasted ");
    dbg(loopDBG, millis() - lastSliceMillis);
    dbg(loopDBG, " milliseconds");
    dbg(loopDBG, "; slice size is ");
    dln(loopDBG, sliceSize);

    lastSliceMillis = millis();
  }

  // roll over (alternative: just let the counter overflow...)
  if(loopIteration++ == LOOP_ROLLOVER) {
    loopIteration = 0;

    dbg(loopDBG, "loopIteration rolling over; ");
    dbg(loopDBG, LOOP_ROLLOVER);
    dbg(loopDBG, " loops lasted ");
    dbg(loopDBG, millis() - firstSliceMillis);
    dln(loopDBG, " milliseconds...; rolling over");
  }
}
