#ifndef utility_h
#define utility_h


void checkDelayAndRun (unsigned long &lastCheckedMillis, int msCheckDelay, void (*f)(void) ) {
  unsigned long currentTimeMillis = millis();

  if (currentTimeMillis - lastCheckedMillis >= msCheckDelay) {
    lastCheckedMillis = currentTimeMillis;
    (*f)();
  }
}

#endif

