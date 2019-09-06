#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);
uint8_t bell[8]  = {0x4, 0xe, 0xe, 0xe, 0x1f, 0x0, 0x4};
uint8_t clock[8] = {0x0,0xe,0x15,0x17,0x11,0xe,0x0};

unsigned long lastLcdUpdate = 0;


//setup
void lcd_setup() {
  // int lcd
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, bell);
  lcd.createChar(1, clock);
} //end lcd_setup()


// SHOW TIME ON LCD
void show_time_lcd(DateTime show) {
  // set time params
  get_time();
  // set output
  myDate = String(daysOfTheWeek[show.dayOfWeek()]) + " ";
  if (show.day() < 10)
    myDate += "0";
  myDate = myDate + show.day() + "/";
  if (show.month() < 10)
    myDate += "0";
  myDate = myDate + show.month() + "/" + show.year() ;

  byte tempHour = show.hour();
  if (tempHour > 12)
    tempHour = tempHour - 12;
  if (tempHour == 0)
    tempHour = 12;
  if (tempHour < 10)
    myTime = "0";
  else
    myTime = "";

  myTime = myTime + tempHour + ":";
  if (show.minute() < 10)
    myTime += "0";
  myTime = myTime + show.minute() + ":";
  if (show.second() < 10)
    myTime += "0";
  myTime = myTime + show.second() ;
  if (show.hour() > 11)
    myTime = myTime + " PM";
  else
    myTime = myTime + " AM";

  //Print on lcd
  lcd.setCursor(0, 0);
  lcd.print(myDate);
  lcd.setCursor(0, 1);
  lcd.print(myTime);

  lcd.setCursor(14, 1);
  if (alarmActiveNow) 
    lcd.write(1);
  else
    lcd.print(" ");
  
  lcd.setCursor(15, 1);
  if (alarmCheckOn)
    lcd.write(0);
  else
    lcd.print(" ");
  // set last lcd update
  lastLcdUpdate = millis();
} // end show_time_lcd()


void show_alarm_lcd() {
  lcd.setCursor(0, 0);
  lcd.print("Alarm Time");
  lcd.setCursor(0, 1);

  byte tempHour = alarmHour;
  if (tempHour > 12)
    tempHour = tempHour - 12;
  if (tempHour == 0)
    tempHour = 12;

  if (tempHour < 10)
    lcd.print("0");
  lcd.print(tempHour);
  lcd.print(":");
  if (alarmMinute < 10)
    lcd.print("0");
  lcd.print(alarmMinute);
  lcd.print(" ");
  if (alarmHour < 12)
    lcd.print("Am");
  else
    lcd.print("Pm");

}
