#define TALKBUTTONPIN 2
#define UPBUTTONPIN 4
#define DOWNBUTTONPIN 5
#define LEDPIN 13


#define DEBOUNCE 10  // button debouncer
#define LONG_PRESS_TIME 1000 // define for long press millis
#define NUMBUTTONS 3
#define TALKBUTTON 0
#define UPBUTTON 1
#define DOWNBUTTON 2

uint8_t pinout[NUMBUTTONS] = {TALKBUTTONPIN, UPBUTTONPIN, DOWNBUTTONPIN};

//checkButtons variable
uint8_t pressed[NUMBUTTONS], justpressed[NUMBUTTONS], justreleased[NUMBUTTONS], longPressed[NUMBUTTONS];
uint8_t previousstate[NUMBUTTONS];
uint8_t currentstate[NUMBUTTONS];
unsigned long btnPressMillis[NUMBUTTONS] = {0, 0, 0};

void loop_ui() {
  static unsigned long lasttime;
  uint8_t index;

  for (index = 0; index < NUMBUTTONS; index++) {
    justpressed[index] = 0;       // when we start, we clear out the "just" indicators
    justreleased[index] = 0;
    longPressed[index] = 0;
  }

  if (millis() < lasttime) {
    lasttime = millis(); // we wrapped around, lets just try again
  }

  if ((lasttime + DEBOUNCE) > millis()) {
    return; // not enough time has passed to debounce
  }
  // ok we have waited DEBOUNCE milliseconds, lets reset the timer
  lasttime = millis();

  for (index = 0; index < NUMBUTTONS; index++) {
    // read the button
    currentstate[index] = digitalRead(pinout[index]);
    // if we are the same state
    if (currentstate[index] == previousstate[index]) {
      // when just pressed
      if ((pressed[index] == LOW) && (currentstate[index] == LOW)) {
        // just pressed
        justpressed[index] = 1;
        btnPressMillis[index] = millis();
      }
      // else when just released
      else if ((pressed[index] == HIGH) && (currentstate[index] == HIGH)) {
        // just released
        justreleased[index] = 1;
      } //end if
      // set the current PRESS state
      pressed[index] = !currentstate[index];  // remember, digital HIGH means NOT pressed

      // check for long press
      if ( pressed[index] && millis() - btnPressMillis[index] >= LONG_PRESS_TIME) {
        longPressed[index] = 1;
      } //end if
    } //end if
    previousstate[index] = currentstate[index];   // keep a running tally of the buttons
  } // end for
}


uint8_t checkAndResetLongPress(uint8_t btn)
{
  uint8_t returnVal = 0;
  if (longPressed[btn])
  {
    returnVal = 1;
    btnPressMillis[btn] = millis();
  }
  return returnVal;
}// end checkAndResetLongPress


void ui_setup() {
  // setting pins
  pinMode(TALKBUTTONPIN, INPUT_PULLUP);
  pinMode(UPBUTTONPIN, INPUT_PULLUP);
  pinMode(DOWNBUTTONPIN, INPUT_PULLUP);
  pinMode(LEDPIN, OUTPUT);
}

void blink_error() {
  while (1) {
    digitalWrite(LEDPIN, HIGH);
    delay(500);
    digitalWrite(LEDPIN, LOW);
    delay(500);
  } //end while
}
