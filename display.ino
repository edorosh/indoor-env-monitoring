void digSeg(byte x, byte y, byte z1, byte z2, byte z3, byte z4, byte z5, byte z6) {   // отображение двух строк по три символа с указанием кодов символов (с)НР
  lcd.setCursor(x, y);
  lcd.write(z1); lcd.write(z2); lcd.write(z3);
  if (x <= 11) lcd.print(" ");
  lcd.setCursor(x, y + 1);
  lcd.write(z4); lcd.write(z5); lcd.write(z6);
  if (x <= 11) lcd.print(" ");
}

void drawDig(byte dig, byte x, byte y) {        // рисуем цифры (с)НР ---------------------------------------
  switch (dig) {            // двухстрочные цифры
    case 0:
      digSeg(x, y, 255, 1, 255, 255, 2, 255);
      break;
    case 1:
      digSeg(x, y, 32, 255, 32, 32, 255, 32);
      break;
    case 2:
      digSeg(x, y, 3, 3, 255, 255, 4, 4);
      break;
    case 3:
      digSeg(x, y, 3, 3, 255, 4, 4, 255);
      break;
    case 4:
      digSeg(x, y, 255, 0, 255, 5, 5, 255);
      break;
    case 5:
      digSeg(x, y, 255, 3, 3, 4, 4, 255);
      break;
    case 6:
      digSeg(x, y, 255, 3, 3, 255, 4, 255);
      break;
    case 7:
      digSeg(x, y, 1, 1, 255, 32, 255, 32);
      break;
    case 8:
      digSeg(x, y, 255, 3, 255, 255, 4, 255);
      break;
    case 9:
      digSeg(x, y, 255, 3, 255, 4, 4, 255);
      break;
    case 10:
      digSeg(x, y, 32, 32, 32, 32, 32, 32);
      break;
  }
}

void drawPPM(int dispCO2, byte x, byte y) {     // Уровень СО2 крупно на главном экране (с)НР ----------------------------
  if (dispCO2 / 1000 == 0) drawDig(10, x, y);
  else drawDig(dispCO2 / 1000, x, y);
  
  drawDig((dispCO2 % 1000) / 100, x + 4, y);
  drawDig((dispCO2 % 100) / 10, x + 8, y);
  drawDig(dispCO2 % 10 , x + 12, y);
  
  lcd.setCursor(15, 0);
  lcd.print("p");
}

void drawTemp(float dispTemp, byte x, byte y) { // Температура крупно на главном экране (с)НР ----------------------------
  if (dispTemp / 10 == 0) drawDig(10, x, y);
  else drawDig(dispTemp / 10, x, y);
  drawDig(int(dispTemp) % 10, x + 4, y);
  drawDig(int(dispTemp * 10.0) % 10, x + 9, y);

  // if (bigDig && DISPLAY_TYPE == 1) {
  //   lcd.setCursor(x + 7, y + 3);
  //   lcd.write(1);             // десятичная точка
  // }
  // else {
    lcd.setCursor(x + 7, y + 1);
    lcd.write(0B10100001);    // десятичная точка
  // }
  lcd.setCursor(x + 13, y);
  lcd.write(223);             // градусы
}

void drawPres(int dispPres, byte x, byte y) {   // Давление крупно на главном экране (с)НР ----------------------------
  drawDig((dispPres % 1000) / 100, x , y);
  drawDig((dispPres % 100) / 10, x + 4, y);
  drawDig(dispPres % 10 , x + 8, y);
  lcd.setCursor(x + 11, 1);
  lcd.setCursor(x + 11, 3);
  lcd.print("hPa");
}
