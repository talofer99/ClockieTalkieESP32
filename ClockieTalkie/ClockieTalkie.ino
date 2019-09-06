/*
    Clockie Talkie.
    The talking rtc arduino clock.
    Code by: Tal Ofer (talofer99@hotmail.com) 

    I2C LCD
    Tiny RTC
    Audio output, Pin 3 and Gnd using the Talkie library
    Talk button on Pin 2
    Up button on pin 4
    Down button on pin 5
    
    System UI button press functions: 
    
    Talk button Short press - say time
    Talk button Long press  - toggle in/out of EDIT TIME
    On edit time, short press on talk move position
    On edit time, up/down button changes the time value.
    Up button Long press - toggle in/out of Alarm Time set.
    In Alarm time set, up = hours, Down = minutes, Long press on Hours go back to RUN.
    DOWN button Long press - toggle ON/OFF the alarm feature (bell on lcd)
    Alarm will sound every 10 Second till the down button is pressed short of long (to turn off the alarm at all)
    To turn the alarm off press the down button 
*/

#include <Wire.h>
#include "rtc.h"
#include "lcd.h"
#include "talk.h"
#include "ui.h"

#define SSRun 0
#define SSSetTime 1
#define SSSetAlarm 2
#define UPDATEHOURS 0
#define UPDATEMINUTES 1
int valueinsecondes[] = {3600, 60, 1};
byte cursorposition[] = {1, 4, 7};

boolean flagFromLongPress;
uint8_t systemState;
uint8_t updatePosition;
unsigned long lastAlarmMiliis;
boolean isAlarmPlaying;

void setup()
{
  // start serial
  Serial.begin(115200);
  Serial.println("System started");
  // ui setup
  ui_setup();

  //setup lcd
  lcd_setup();

  // start rtc
  if (!setup_rtc()) {
    // on fail blink led
    blink_error();
  } //end if

  // update lcd
  get_time();
  show_time_lcd(now);

  // states
  set_system_state(0);
}


void loop() {
  // run ui loop - mainly for button check
  loop_ui();
  // check for playing alarm
  if (isAlarmPlaying) {
    // it will return true when it got to the end of the play.
    if (say_time_by_position()) {
      digitalWrite(LEDPIN, LOW); // turn led on (on finish if play we turn it off)
      isAlarmPlaying = false; // set playing flag

    } //end if
  } //end if


  // if the talk button was pressed for long time
  if (checkAndResetLongPress(TALKBUTTON)) {
    if (systemState == SSSetTime) {
      rtc.adjust(adjsTo); // we do it here so we save only on switching and not on setting the new state. (will cause error on start)
      set_system_state(SSRun);
    } else if  (systemState == SSRun) {
      set_system_state(SSSetTime);
    }
    // we set the flag that is used when relased - not to auto play.
    flagFromLongPress = true;
  } //end if

  // run by system state
  switch (systemState) {
    case SSRun:
      // if say time button was released
      if (justreleased[TALKBUTTON]) {
        if (flagFromLongPress) {
          flagFromLongPress = false;
        } else {
          digitalWrite(LEDPIN, HIGH);
          isAlarmPlaying = true ;

          //say_time();

        } //end if
      } //end if

      // if  UP long pressed we SET ALARM MODE
      if (checkAndResetLongPress(UPBUTTON)) {
        flagFromLongPress = true;
        set_system_state(SSSetAlarm);
        return;
      }

      // if long presses the DOWN button WE toogle the alarmCheckOn
      if (checkAndResetLongPress(DOWNBUTTON)) {
        alarmCheckOn = !alarmCheckOn;
        alarmActiveNow = false;
        digitalWrite(LEDPIN, LOW);
        show_time_lcd(rtc.now());
        return;
      }

      // if its time to update the lcd (once every 1 sec)
      if (lastLcdUpdate + 1000 < millis()) {
        // update time for alarm and show time
        get_time();
        // show on lcd
        show_time_lcd(now);
        //check for alarm
        check_alarm();
      } //end if

      // if button press to call off the alarm
      if (alarmActiveNow && pressed[DOWNBUTTON]) {
        alarmActiveNow = false;
        isAlarmPlaying = false; // set playing flag
        say_time_position = 0; // reset the postion
        digitalWrite(LEDPIN, LOW);
        show_time_lcd(rtc.now());
        return;
      } //end if

      break;

    case SSSetTime:
      // if say time button was released
      if (justreleased[TALKBUTTON]) {
        if (flagFromLongPress) {
          flagFromLongPress = false;
        } else {
          //change update position
          updatePosition = (updatePosition + 1) % 3;
          // set in lcd
          set_position(updatePosition);
        } //end if
      } //end if

      // up button
      if (justreleased[UPBUTTON]) {
        adjustRTC(valueinsecondes[updatePosition]);
      } //end if
      if (justreleased[DOWNBUTTON]) {
        adjustRTC(-valueinsecondes[updatePosition]);
      } //end if
      break;

    case SSSetAlarm:
      // if long presses the UP button go into ALARM SET MODE
      if (checkAndResetLongPress(UPBUTTON)) {
        set_system_state(SSRun);
        return;
      }

      // up to move hourse
      if (justreleased[UPBUTTON]) {
        if (flagFromLongPress) {
          flagFromLongPress = false;
        } else {
          alarmHour++;
          if (alarmHour == 24)
            alarmHour = 0;

          show_alarm_lcd();
        }
      }//end if

      // up to move hourse
      if (justreleased[DOWNBUTTON]) {
        alarmMinute++;
        if (alarmMinute == 60) {
          alarmMinute = 0;
        } //end if

        show_alarm_lcd();
      }//end if

      break;
  } //end switch
} //end loop


// adjust rtc
void adjustRTC(int gap) {
  digitalWrite(LEDPIN, HIGH);
  uint32_t adjUnix = adjsTo.unixtime();
  adjsTo = DateTime(adjUnix + gap);
  show_time_lcd(adjsTo);
  set_position(updatePosition);
  digitalWrite(LEDPIN, LOW);
}

void set_position(uint8_t pos) {
  lcd.setCursor(cursorposition[pos], 1);
}

void set_system_state(uint8_t newState) {
  switch (newState) {
    case SSRun:
      lcd.noBlink();
      lcd.clear();
      show_time_lcd(rtc.now());
      break;
    case SSSetTime:
      lcd.clear();
      lcd.blink();
      adjsTo = rtc.now();
      show_time_lcd(adjsTo);
      updatePosition = 0;
      set_position(updatePosition);
      break;
    case SSSetAlarm:
      lcd.clear();
      lcd.noBlink();
      show_alarm_lcd();
      break;
  }//end switch

  systemState = newState;
} //end set_system_state


void check_alarm() {
  // if alam is on (bell)
  if (alarmCheckOn) {
    // if time to rais the alarm
    if (!alarmActiveNow && alarmHour == Hours && alarmMinute == Minutes && Secs < 2) {
      // set active now
      alarmActiveNow = true;
      //set last time
      lastAlarmMiliis = millis();
      // set led
      digitalWrite(LEDPIN, HIGH);
      // update screen to show the alarm is working
      show_time_lcd(now);
      // say time
      isAlarmPlaying = true;
    } else if (alarmActiveNow) {
      // if 10 seconds since last one
      if (lastAlarmMiliis + 10000 < millis()) {
        isAlarmPlaying = true;
        lastAlarmMiliis = millis();
      } //end if
    } // end if
  } //end if
} // check_alarm()
