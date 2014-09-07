unsigned long lastChange;
int interval = 2000;

boolean timer_check() {
  unsigned long now = millis();
  if(interval == 0) {
    lastChange = now;
    return true;
  }
  if( (now-lastChange) >= interval) {
    lastChange += interval;
    return true;
  }
  return false; 
}

void timer_reset() {
  lastChange = millis();
}

