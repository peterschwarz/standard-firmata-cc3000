/*
  WifiStream.cpp - Streaming Ethernet Arduino library 
                   (c) 2014 Peter Schwarz (peter.schwarz@bicycle.io)
*/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "WifiStream.h"

#include <ccspi.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

#define DISPLAY_FREE_RAM 1
/**
 * Utilities
 */
uint16_t checkFirmwareVersion(Adafruit_CC3000 *cc3000)
{
    uint8_t major, minor;
    uint16_t version;
    
  #ifndef CC3000_TINY_DRIVER  
    if(!cc3000->getFirmwareVersion(&major, &minor))
    {
      Serial.println(F("Unable to retrieve the firmware version!\r\n"));
      version = 0;
    }
    else
    {
      Serial.print(F("Firmware V. : "));
      Serial.print(major); Serial.print(F(".")); Serial.println(minor);
      version = major; version <<= 8; version |= minor;
    }
  #endif
    return version;
}

void logFreeRam() 
{
    #ifdef DISPLAY_FREE_RAM
    Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
    #endif
}

/******************************************************************************
 * Definitions
 ******************************************************************************/

WifiStream::WifiStream()
{
    cc3000 = new Adafruit_CC3000(ADAFRUIT_CC3000_CS, 
                             ADAFRUIT_CC3000_IRQ, 
                             ADAFRUIT_CC3000_VBAT,
                             SPI_CLOCK_DIVIDER);
    client = 0;
}

int WifiStream::begin(const char* wlan_ssid, const char* wlan_password, uint8_t wlan_security, uint16_t port)
{
    logFreeRam();

    if (!cc3000->begin())
    {
        Serial.println(F("Couldn't begin()! Check your wiring?"));
        return 0;  
    }
    
    Serial.print(F("\nAttempting to connect to ")); Serial.println(wlan_ssid);
    if (!cc3000->connectToAP(wlan_ssid, wlan_password, wlan_security)) {
        Serial.println(F("Failed!"));
        return 0;
    }

    logFreeRam();
     
    Serial.println(F("Connected!"));
    
   return complete_configuration(port);
}

int WifiStream::complete_configuration(uint16_t port)
{
    while (!cc3000->checkDHCP())
    {
        delay(100); // ToDo: Insert a DHCP timeout!
    }  

        /* Display the IP address DNS, Gateway, etc. */  
    while (! displayConnectionDetails(&Serial)) 
    {
        delay(1000);
    }

    server = new Adafruit_CC3000_Server(port);

    logFreeRam();
    return 1;
}

bool WifiStream::displayConnectionDetails(Stream* debugStream)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000->getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    debugStream->println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    debugStream->print(F("\nIP Addr: ")); cc3000->printIPdotsRev(ipAddress);
    debugStream->print(F("\nNetmask: ")); cc3000->printIPdotsRev(netmask);
    debugStream->print(F("\nGateway: ")); cc3000->printIPdotsRev(gateway);
    debugStream->print(F("\nDHCPsrv: ")); cc3000->printIPdotsRev(dhcpserv);
    debugStream->print(F("\nDNSserv: ")); cc3000->printIPdotsRev(dnsserv);
    debugStream->println();
    return true;
  }
}

int WifiStream::connect_client() 
{
    logFreeRam();
    if (!(client && client->connected())) {
        Adafruit_CC3000_ClientRef newClient = server->available();
        if (!newClient)         
            return 0;

        client = &newClient;   
    }
    return 1;
}

int WifiStream::available()
{
    return connect_client() ? client->available() : 0;
}

int WifiStream::read()
{
    return connect_client() ? client->read() : 0;
}

void WifiStream::flush() {
    
}

int WifiStream::peek() {
    return 0;
}

size_t WifiStream::write(uint8_t outcomingByte) {  
    return connect_client() ? client->write(outcomingByte) : 0;
}
