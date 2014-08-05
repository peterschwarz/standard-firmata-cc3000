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
#include "utility/socket.h"

#define SERIAL_LOG(msg) Serial.println(F(msg));
#define SERIAL_LOG_STR(text, value)  Serial.print(F(text)); Serial.println(value);
#define SERIAL_LOG_INT(text, value)  Serial.print(F(text)); Serial.println(value, DEC);

// #define DISPLAY_FREE_RAM 1
#ifdef DISPLAY_FREE_RAM
  #include "utility/debug.h"

 #define LOG_FREE_RAM  SERIAL_LOG_INT("Free RAM: ", getFreeRam());
#else
 #define LOG_FREE_RAM 
#endif

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10


bool displayConnectionDetails(Adafruit_CC3000 *cc3000)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000->getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000->printIPdotsRev(ipAddress);
    Serial.println();
    return true;
  }
}

/******************************************************************************
 * Definitions
 ******************************************************************************/

WifiStream::WifiStream()
{
}

int WifiStream::begin(const char* wlan_ssid, const char* wlan_password, uint8_t wlan_security, uint16_t port)
{
    LOG_FREE_RAM

    Adafruit_CC3000 cc3000(ADAFRUIT_CC3000_CS, 
                           ADAFRUIT_CC3000_IRQ, 
                           ADAFRUIT_CC3000_VBAT,
                           SPI_CLOCK_DIVIDER);


    if (!cc3000.begin())
    {
        SERIAL_LOG("Couldn't begin()! Check your wiring?");
        return 0;  
    }
    
    SERIAL_LOG_STR("\nAttempting to connect to ", wlan_ssid);
    if (!cc3000.connectToAP(wlan_ssid, wlan_password, wlan_security)) {
        SERIAL_LOG_STR("Unable to connect to ", wlan_ssid)
        return 0;
    }

    LOG_FREE_RAM
     
    // Serial.println(F("Connected!"));
    int dhcp_attempts = 20;
    while (!cc3000.checkDHCP())
    {
        if (dhcp_attempts <= 0) {
            SERIAL_LOG("DHCP timeout occurred.")
            return 0;
        }
        delay(100); 
        dhcp_attempts--;

    }  

        /* Display the IP address DNS, Gateway, etc. */  
    while (! displayConnectionDetails(&cc3000)) 
    {
        delay(1000);
    }

    SERIAL_LOG_INT("\nStarting server on port ", port)
    server = new Adafruit_CC3000_Server(port);
    server->begin();

    LOG_FREE_RAM
    return 1;
}

Adafruit_CC3000_ClientRef WifiStream::connect_client() 
{
    LOG_FREE_RAM
    Adafruit_CC3000_ClientRef newClient = server->available();
    if (newClient) {
        SERIAL_LOG("New Client Connected")
    }
    return newClient;
}

int WifiStream::available()
{
    return connect_client().available();
}

int WifiStream::read()
{
    return connect_client().read();
}

void WifiStream::flush() {
    while(available()) read();
}

int WifiStream::peek() {
    return -1;
}

size_t WifiStream::write(uint8_t outcomingByte) {  
    return connect_client().write(outcomingByte);
}
