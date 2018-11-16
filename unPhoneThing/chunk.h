/////////////////////////////////////////////////////////////////////////////
// chunk.h //////////////////////////////////////////////////////////////////

bool DBG = true; // debug switch

// macros for debug calls to Serial.printf, with (D) and without (DD)
// new line, and to Serial.println (DDD)
#define D(args...)   { if(DBG) { Serial.printf(args); Serial.println(); } }
#define DD(args...)  { if(DBG) Serial.printf(args); }
#define DDD(s)       { if(DBG) Serial.println(s); }

// parent for device element classes
class Chunk {
public:
  int begin();  // initialisation
  void test();  // validation
};
