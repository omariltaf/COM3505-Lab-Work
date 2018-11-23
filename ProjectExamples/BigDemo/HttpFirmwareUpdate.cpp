/*
 *  HttpFirmwareUpdate.cpp - this is the class functions definition for HttpFirmwareUpdate
 *  The purpose of this class is to handle updating the firmware of the 
 *  unPhone device from a webserver so that it doesn't have to be done manually.
 * 
 * LIMITED TO WORK WITH HTTP :(
 *
 *  Created on: 20-06-2018
 *  Author: @HarryEH
 */

#include "HttpFirmwareUpdate.h"

/**
 * This is the constructor for the HttpFirmwareUpdate class
 * @param server - type is String. This is the url of the server that you want
 * to connect to.
 * @param version_number - type is String. This is the version number of the
 * firmware that the program is running.
 */
HttpFirmwareUpdate::HttpFirmwareUpdate(String server, String version_number, const int dev_build) {
  m_server     = server;
  m_version    = version_number;
  m_update_url = "/version_number?version=";
  m_dev_build  = "&dev_build=" + String(dev_build);
}

/**
 * This function checks for an update on the server. It sends the version number
 * to the server and processes the response. If the response includes a bin
 * file indicated by the presence of ".bin" then the function returns true.
 * @returns bool - this will be true if the server reports that an update needs
 * to be done.
 */
bool HttpFirmwareUpdate::checkForUpdate() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  HTTPClient m_client;

  m_client.begin(m_server+m_update_url+m_version+m_dev_build);

  if (m_client.GET() > 0) {
    
    m_bin_location = m_client.getString();
    
    m_client.end();
    
    return m_bin_location.indexOf(".bin") > 0;
    
  }

  m_client.end();
  return false;
}

/**
 * This function connects to a server and downloads the bin file. It then 
 * flashes the esp32 with the new firmware. 
 * @return void. 
 */
void HttpFirmwareUpdate::performUpdate() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  HTTPClient m_client;

  m_client.begin(m_server+m_bin_location);

  if (m_client.GET() > 0) {
    const int CONTENT_LENGTH = m_client.getSize();

    WiFiClient stream = m_client.getStream();

    if (Update.begin(CONTENT_LENGTH)) {
      printf("Begin OTA. This may take 2 - 5 mins to complete.");
      
      Update.writeStream(stream);

      if (Update.end()) {
        printf(String("OTA done!").c_str());
        if (Update.isFinished()) {
          printf(String("Update successfully completed. Rebooting.").c_str());
          ESP.restart();
        } else {
          printf(String("Update not finished? Something went wrong!").c_str());
        }
      } else {
        printf(String("Error Occurred. Error #: " + String(Update.getError())).c_str());
      }
    } else {
      printf("There wasn't enough space to start OTA!!");
    }

    stream.flush();
    
  }

  m_client.end();
  return;
}
