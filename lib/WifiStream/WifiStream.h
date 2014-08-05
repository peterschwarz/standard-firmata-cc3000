/*
  WifiStream.h - Streaming Ethernet Arduino library 
                 (c) 2014 Peter Schwarz (peter.schwarz@bicycle.io)
*/
#ifndef WIFI_STREAM_H
#define WIFI_STREAM_H

#include <inttypes.h>
#include <Stream.h>
#include <SPI.h>
#include <Adafruit_CC3000.h>



class WifiStream : public Stream
{
  private:
    
    Adafruit_CC3000_Server *server;
    Adafruit_CC3000_ClientRef *client;

    Adafruit_CC3000_ClientRef connect_client();

  public:       
    WifiStream();
    

    // Begins with static credentials
    int begin(const char* wlan_ssid, const char* wlan_password, uint8_t wlan_security, uint16_t port);

    // overriden Stream class functions
    virtual size_t write(uint8_t byte);
    virtual int read();
    virtual int available();
    virtual void flush();
    virtual int peek();   
};

#endif /* WIFI_STREAM_H */
