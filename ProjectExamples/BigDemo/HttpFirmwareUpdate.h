/*
 *  HttpFirmwareUpdate.h - this is the class definition for HttpFirmwareUpdate
 *  The purpose of this class is to handle updating the firmware of the 
 *  unPhone device from a webserver so that it doesn't have to be done manually.
 * 
 * LIMITED TO WORK WITH HTTP :(
 *
 *  Created on: 20-06-2018
 *  Author: @HarryEH
 */

#ifndef HTTPFIRMWAREUPDATE_H_
#define HTTPFIRMWAREUPDATE_H_

#include <Update.h>
#include <HTTPClient.h>

//////////////////////////////////////////////////////////////////////////////
/**
 * Defintion of HttpFirmwareUpdate class. 
 */
class HttpFirmwareUpdate {
  private:
    String m_server;
    String m_version;
    String m_update_url;
    String m_bin_location;
    String m_dev_build;

  public:
    HttpFirmwareUpdate(String, String, int);
    bool checkForUpdate();
    void performUpdate();
};
//////////////////////////////////////////////////////////////////////////////

#endif
