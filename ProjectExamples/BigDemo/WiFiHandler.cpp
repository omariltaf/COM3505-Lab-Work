// // WiFiHandler.cpp
// 
// #include "WiFiHandler.h"
// #include "WiFiHTML.h"
// #include "unphone.h"
// 
// //////////////////////////////////////////////////////////////////////////////
// /**
//  * Forward declarations of the functions defined in WiFiHandler.cpp.
//  */
// void startAP();
// String genAPForm();
// void startWebServer();
// void handler_end();
// void handleNotFound(AsyncWebServerRequest *request);
// void handle_root(AsyncWebServerRequest *request);
// void handle_wifi(AsyncWebServerRequest *request);
// void handle_wifi_connect(AsyncWebServerRequest *request);
// //////////////////////////////////////////////////////////////////////////////
// 
// //////////////////////////////////////////////////////////////////////////////
// /**
//  * Declarations of various variables and objects
//  */
// AsyncWebServer *server;
// IPAddress apIP(192, 168, 99, 1);
// IPAddress netMsk(255, 255, 255, 0);
// const byte DNS_PORT = 53;
// // 307 is temporary redirect. if we used 301 we'd probably break the user's
// // browser for sites they were captured from until they cleared their cache
// int TEMPORARY_REDIRECT = 307;
// DNSServer dnsServer;
// bool last_loop_status = false;
// //////////////////////////////////////////////////////////////////////////////
// 
// /**
//  * Function that starts the web server, the access point
//  * and the captive portal functionality.
//  */
// void wifi_handler_startup() {
// 
//   WiFi.begin();
// 
//   last_loop_status = (WiFi.status() == WL_CONNECTED);
// 
//   if (last_loop_status) {
//     return;
//   }
// 
//   startAP();
//   startWebServer();
// 
//   Serial.printf("joinme will direct captured clients to: %s\n",
//                 apIP.toString().c_str());
//   dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
//   dnsServer.start(DNS_PORT, "*", apIP);
//   Serial.println("joinme captive dns server started");
// 
// }// End of wifi_handler_startup
// 
// /**
//  * This function ends the web server, sets the WiFi mode of the 
//  * device and stops the captive portal.
//  */
// void wifi_handler_end(){
// 
//     dnsServer.stop();
//     server = NULL;
//     WiFi.mode(WIFI_STA);
// 
// }// End of wifi_handler_end
// 
// /**
//  * This function handles the behaviour at each iteration of the loop
//  */
// void wifi_handler_turn() {
// 
//   const bool CONNECTION = (WiFi.status() == WL_CONNECTED);
//   
//   if (last_loop_status != CONNECTION) {
//       if (CONNECTION) {
//           wifi_handler_end();
//       } else {
//           wifi_handler_startup();
//       }
//   }
// 
//   last_loop_status = CONNECTION;
// 
//   if (!CONNECTION) {
//     dnsServer.processNextRequest();
//   } 
// }// End of wifi_handler_turn
// 
// /**
//  * This function starts and sets up the access point 
//  */
// void startAP() {
//   WiFi.mode(WIFI_AP_STA);
//   
//   if (!WiFi.softAPConfig(apIP, apIP, netMsk)){
//       // FIXME: failed
//   }
// 
//   String SSID = "unphone-"+String(getMAC(new char[13]));
//   WiFi.softAP(SSID.c_str(),"password");
// }// End of startAP
// 
// /**
//  * This function starts the web server and handles the routing 
//  */
// void startWebServer() {
//   server = new AsyncWebServer(80);
//   server->on("/", handle_root);
//   server->onNotFound(handle_root);
//   server->on("/wifi", handle_wifi);
//   server->on("/wfchz", handle_wifi_connect);
//   server->begin();
// }// End of startWebServer
// 
// /**
//  * This function handles the root page of the site
//  * @param request of type AsyncWebServerRequest*, this is
//  * the asynchronous request from the user that is accessing
//  * the web server on the access point.
//  */
// void handle_root(AsyncWebServerRequest* request) {
//   String toSend = pageTop;
//   toSend += pageTop2;
//   toSend += pageDefault;
//   toSend += pageFooter;
//   request->send(200, "text/html", toSend);
// }// End of handle_root
// 
// /**
//  * This function handles the behaviour for the /wifi page
//  * @param request of type AsyncWebServerRequest*, this is
//  * the asynchronous request from the user that is accessing
//  * the web server on the access point.
//  */
// void handle_wifi(AsyncWebServerRequest* request) {
//   String toSend = genAPForm();
//   request->send(200, "text/html", toSend);
// }// End of handle_wifi
// 
// /**
//  * This function handles the behaviour of the POST to /wfchz
//  * which comes from the form on the /wifi page.
//  * @param request of type AsyncWebServerRequest*, this is
//  * the asynchronous request from the user that is accessing
//  * the web server on the access point.
//  */
// void handle_wifi_connect(AsyncWebServerRequest* request) {
//   String toSend = pageTop;
//   toSend += ": joining wifi network";
//   toSend += pageTop2;
//   String ssid = "";
//   String key = "";
// 
//   for(uint8_t i = 0; i < request->args(); i++ ) {
//     if(request->argName(i) == "hssid" && request->arg(i) != "")
//       ssid = request->arg(i);
//     else if(request->argName(i) == "ssid")
//       ssid = request->arg(i);
//     else if(request->argName(i) == "key")
//       key = request->arg(i);
//   }
// 
//   if(ssid == "") {
//     toSend += "<h2>Ooops, no SSID...?</h2>";
//     toSend += "<p>Looks like a bug :-(</p>";
//   } else {
//     toSend += "<h2>Done! Now trying to join network...</h2>";
//     char ssidchars[ssid.length()+1];
//     char keychars[key.length()+1];
//     ssid.toCharArray(ssidchars, ssid.length()+1);
//     key.toCharArray(keychars, key.length()+1);
//     WiFi.disconnect(true);
//     WiFi.begin(ssidchars, keychars);
//   }
// 
//   toSend += pageFooter;
//   request->send(200, "text/html", toSend);
// }// End of handle_wifi_connect
// 
// /**
//  * This function creates the form for the WiFi page
//  * ie shows all the available SSIDs.
//  */
// String genAPForm() {
//   String f = pageTop;
//   f.concat(": Wifi Config");
//   f.concat(pageTop2);
//   f.concat("<h2>Choose a wifi access point to join</h2>\n");
//   f.concat("<h3>Signal strength in brackets, lower is better</h3><p>\n");
//   
//   const char *checked = " checked";
// 
//   int n = WiFi.scanNetworks();
// 
//   if(n == 0) {
//     f.concat("No wifi access points found :-( ");
//   } else {
//     f.concat("<form method='POST' action='wfchz'> ");
//     for(int i = 0; i < n; ++i) {
//       // print SSID and RSSI for each network found
// 
//       f.concat("<input type='radio' name='ssid' value='");
//       f.concat(WiFi.SSID(i));
//       f.concat("'");
//       f.concat(checked);
//       f.concat(">");
//       f.concat(WiFi.SSID(i));
//       f.concat(" (");
//       f.concat(WiFi.RSSI(i));
//       f.concat(" dBm)");
//       f.concat("<br/>\n");
//       checked = "";
//     }
//   }
//   f.concat("<br/>Hidden SSID: <input type='textarea' name='hssid'><br/><br/> ");
//   f.concat("<br/>Pass key: <input type='textarea' name='key'><br/><br/> ");
//   f.concat("<input type='submit' value='Submit'></form></p>");
//   f.concat("<a href='/'>Back</a><br/><a href='/wifi'>Try again?</a></p>\n");
// 
//   f.concat(pageFooter);
//   return f;
// }// End of genAPForm
// 
// // <EOF>
