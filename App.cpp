/**
 *
 */

#include "App.h"
#include <avr/pgmspace.h>
#include <Time.h>
#include <TimeZone.h>

// init DMD libs
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

// AU Eastern Time Zone (Sydney, NSW)
TimeChangeRule zoneSTD = {"AEST", First, Sun, Apr, 2, +600};  // Standard time
TimeChangeRule zoneDST = {"AEDT", First, Sun, Oct, 2, +660};  // Daylight time
Timezone timezone(zoneDST, zoneSTD);
byte lm35 = A0;

tmElements_t time;
bool isDst;
tpElements_t temp;
char buff[8];

/**
 * Read the current time
 */
void App::getTime() {
  time_t local = timezone.toLocal( now() );
  isDst = timezone.locIsDST(local);
  breakTime(local, time);
}

/**
 * Read the current temprature
 */
void App::getTemp() {
  unsigned int t_lm35 = analogRead(lm35);
  float tmp = (float)t_lm35 * (5.0 / 1023.0);   // 5.0V Arduino VREF, 1023 ADC range
  tmp = (tmp /* - 0.0 */ ) * 100.0;  // 0.0V Offset Voltage, 10mV/°C on LM35
  temp.Temp = int(tmp);
  temp.Dec2 = (tmp - static_cast<int>(tmp)) * 100;
  temp.Temp = temp.Temp - 5;  // Why is my LM35 out by 5°C ???
}
