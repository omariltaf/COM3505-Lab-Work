// Ex07.h

/////////////////////////////////////////////////////////////////////////////
// NOTE!!!
// DON'T edit these files, do your coding in MyThing!
/////////////////////////////////////////////////////////////////////////////

#ifndef LAB_07_H 
#define LAB_07_H

// there's a thousand ways to do this... below a version that is relatively
// simple to use and to code, based on an array of C strings that are
// concatenated into an Arduino (C++) String (so beware memory fragmentation!)
//
// the replacement_t type definition allows specification of a subset of the
// "boilerplate" strings, so we can e.g. replace only the title, or etc.

const char *boiler[] = { // boilerplate: constants & pattern parts of template
  "<html><head><title>",                                                // 0
  "default title",                                                      // 1
  "</title>\n",                                                         // 2
  "<meta charset='utf-8'>",                                             // 3

  // adjacent strings in C are concatenated:
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
  "<style>body{background:#FFF; color: #000; font-family: sans-serif;", // 4

  "font-size: 150%;}</style>\n",                                        // 5
  "</head><body>\n<h2>",                                                // 6
  "Welcome to Thing!",                                                  // 7
  "</h2>\n<p><a href='/'>Home</a>&nbsp;&nbsp;&nbsp;</p>\n",             // 8
  "</body></html>\n\n",                                                 // 9
};

typedef struct { int position; const char *replacement; } replacement_t;
void getHtml(String& html, const char *[], int, replacement_t [], int);
// getting the length of an array in C can be complex...
// https://stackoverflow.com/questions/37538/how-do-i-determine-the-size-of-my-array-in-c
#define ALEN(a) ((int) (sizeof(a) / sizeof(a[0]))) // only in definition scope!
#define GET_HTML(strout, boiler, repls) \
  getHtml(strout, boiler, ALEN(boiler), repls, ALEN(repls));

#endif
