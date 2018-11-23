// WiFiHTML.h

#ifndef WIFIHTML_H_
#define WIFIHTML_H_

/////////////////////////////////////////////////////////////////////////////
// page generation stuff ////////////////////////////////////////////////////
const char* pageTop = "<html><head><title> unphone";
const char* pageTop2 = "</title>\n"
  "<meta charset=\"utf-8\">"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
  "<style>body{background:#FFF;color: #000;font-family: sans-serif;font-size: 150%;}</style>"
  "</head><body>\n";
const char* pageDefault = // TODO build the growbeds according to their num
  "<h2>Welcome to unPhone</h2>\n"
  "<h2>Control</h2>\n"
  "<p><ul>\n"
  "<li><a href='/wifi'>Join a wifi network</a></li>\n"
  "</ul></p>\n";
const char* pageFooter =
  "\n<p><a href='/'>Home</a>&nbsp;&nbsp;&nbsp;"
  "<a href='/wifi'>WiFi</a>&nbsp;&nbsp;&nbsp;"
  "</p></body></html>";

#endif

