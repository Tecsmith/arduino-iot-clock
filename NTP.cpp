/**
 *
 */

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Time.h>
#include "NTP.h"

uint16_t SNTP_Port = 8888;
IPAddress SNTP_Server_IP[] = { (uint32_t)0, (uint32_t)0, (uint32_t)0 };  // uninitalized, use setServer

// A UDP instance to let us send and receive packets over UDP
bool _NTP_init = false;
EthernetUDP _NTP_UDP;

const uint8_t NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE ]; //buffer to hold incoming and outgoing packets 

/* NTPClass::NTPClass() {
  // do nothing
} */

bool NTPClass::available() {
  if (!_NTP_init) {
    _NTP_init = true;
    _NTP_UDP.begin(SNTP_Port);
  }
  return true;  // TODO : Find a better way to test if NTP is working
}

time_t NTPClass::get() {
  int retry;

  if (!available()) return 0;
  int svr = 0;

  while (svr < 3) {
    if (SNTP_Server_IP[svr][3] == 0) return 0;
    retry = 10;
    sendNTPpacket( SNTP_Server_IP[svr] );
    while (retry > 0) {
      delay(100);
      if ( _NTP_UDP.parsePacket() ) {  
        _NTP_UDP.read(packetBuffer, NTP_PACKET_SIZE);
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        return secsSince1900 - 2208988800UL;  // 70 years
      }
      retry--;
    }
    svr++;
  }
  return 0; // return 0 if unable to get the time
}

void NTPClass::read(tmElements_t &tm) {
  breakTime(get(), tm);
}

void NTPClass::begin(IPAddress timeServer) {
  SNTP_Server_IP[0] = timeServer;
}

void NTPClass::begin(IPAddress timeServer1, IPAddress timeServer2, IPAddress timeServer3) {
  SNTP_Server_IP[0] = timeServer1;
  SNTP_Server_IP[1] = timeServer2;
  SNTP_Server_IP[2] = timeServer3;
}

void NTPClass::sendNTPpacket(IPAddress timeServer) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // zeros                       // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;        // ?
  packetBuffer[13]  = 0x4E;      // ?
  packetBuffer[14]  = 49;        // ?
  packetBuffer[15]  = 52;        // ?
  // zeros                       // Reference and time stamps 

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 		   
  _NTP_UDP.beginPacket(timeServer, 123);  // NTP requests are to port 123
  _NTP_UDP.write(packetBuffer, NTP_PACKET_SIZE);
  _NTP_UDP.endPacket();
}

NTPClass NTP = NTPClass();  // instance
