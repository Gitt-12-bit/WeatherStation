#ifndef _NTP_H
#define _NTP_H

#include "time.h"

void ntpBegin(){

  configTzTime(
    "EST+5",
    "pool.ntp.org",
    "time.nist.gov"
  );

  Serial.println("[NTP] Sync started");
}

#endif