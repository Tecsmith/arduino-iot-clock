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
  RTC.readTemperature(temp);
}
