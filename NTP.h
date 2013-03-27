/**
 *
 */

#ifndef NTP_h
#define NTP_h

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Time.h>

extern uint16_t SNTP_Port;
extern IPAddress SNTP_Server_IP[];
 
class NTPClass {
  public:
    /* NTPClass(); */
    static bool available();
    static time_t get();
    static void read(tmElements_t &tm);
    static void begin(IPAddress timeServer);
    static void begin(IPAddress timeServer1, IPAddress timeServer2, IPAddress timeServer3);
  private:
    static void sendNTPpacket(IPAddress timeServer);
};

extern NTPClass NTP;
 
#endif
