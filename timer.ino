/** timer.ino
  *
  * Simple timer - Code comes from the Metro library: https://github.com/thomasfredericks/Metro-Arduino-Wiring
  * but was rewritten in Sketch. 
  * 
  * This file is licensed differently than the main travelsensor.ino:
  */
  
// The MIT License (MIT)
//
// Copyright (c) 2013 thomasfredericks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

