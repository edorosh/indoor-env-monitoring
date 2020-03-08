void checkBrightness() {
   // если темно
  if (analogRead(PHOTO_RES_PIN) < BRIGHT_THRESHOLD) {
    LCD_BRIGHT = LCD_BRIGHT_MIN;
    LED_BRIGHT_RED = LED_BRIGHT_RED_MIN;
    LED_BRIGHT_GREEN = LED_BRIGHT_GREEN_MIN;
    LED_BRIGHT_YELLOW = LED_BRIGHT_YELLOW_MIN;
  } 
  // если светло
  else {             
    LED_BRIGHT_RED = LED_BRIGHT_RED_MAX;
    LED_BRIGHT_GREEN = LED_BRIGHT_GREEN_MAX;
    LED_BRIGHT_YELLOW = LED_BRIGHT_YELLOW_MAX;
                         
    LCD_BRIGHT = LCD_BRIGHT_MAX;
  }


  #if (DEBUG == 1)
  Serial.print("Brightness modes");
  Serial.print(LCD_BRIGHT);
  Serial.print(LED_BRIGHT_RED);
  Serial.print(LED_BRIGHT_GREEN);
  Serial.print(LED_BRIGHT_YELLOW);
  Serial.println();
  #endif
}

void applyBrightness() {
  analogWrite(BACKLIGHT_PIN, LCD_BRIGHT);
}
