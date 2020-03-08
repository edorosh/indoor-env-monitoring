void setLED(byte color) {
  // сначала всё выключаем
  analogWrite(LED_R, 0);
  analogWrite(LED_G, 0);
  analogWrite(LED_Y, 0);
  
  switch (color) {    // 0 выкл, 1 красный, 2 зелёный, 3 жёлтый
    case 0:
      break;
    case 1: analogWrite(LED_R, LED_BRIGHT_RED);
      break;
    case 2: analogWrite(LED_G, LED_BRIGHT_GREEN);
      break;
    case 3:
      analogWrite(LED_Y, LED_BRIGHT_YELLOW);
      break;
  }
}
