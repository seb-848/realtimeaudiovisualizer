#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <ST7735_t3.h>


// Defines----------------------
#define LCD_RST 8  // reset
#define LCD_RS  9  // register select
#define LCD_CS  10 // chip select

// Constants--------------------
const int numBars = 32;
const int barWidth = 4;
const int spacing = 1;
const unsigned long PEAK_HOLD_DURATION = 300; // ms before peak starts falling

// Globals----------------------
float peakHold[numBars] = {0};  // holds the peak dB values
unsigned long peakHoldTime[numBars] = {0}; // timer for each peak

// Setup------------------------
AudioInputAnalog      mic(A2);              // mic setup on pin A2
                                            // audio object reading analog mic input
AudioAnalyzeFFT1024   fft;                  // FFT analyzer object from Teensy Audio Library
                                            // takes audio stream and does a 1024-point FFT
                                            // converting time-domain audio to frequency domain audio
AudioConnection       patchCord1(mic, fft); // connects mic to FFT object using Teensy's audio signal routing system

ST7735_t3 lcd = ST7735_t3(LCD_CS, LCD_RS, LCD_RST); // ST7735 object created
//------------------------------

void setup() {
    Serial.begin(115200);

    // Audio lib setup
    AudioMemory(12);                            // allocates memory for audio processing
                                                // so processing can be done in real time 3KB RAM
    fft.windowFunction(AudioWindowHanning1024); // smooths FFT input for better resolution
                                                // sets up Hanning window which reduces spectral leakage

    // LCD display
    lcd.initR(INITR_BLACKTAB);    // initialize screen registers
    lcd.setRotation(1);           // orientations of display (0-3)
    lcd.fillScreen(ST77XX_BLACK); // fills entire screen with one color

    // Draw dB scale once
    for (int db = -60; db <= 0; db += 20) {
      int y = map(db, -60, 0, 64, 0);
      lcd.drawLine(0, y, 3, y, ST77XX_WHITE);
      lcd.setCursor(5, y - 3);
      lcd.print(db);
      lcd.print("dB");
    }
}

void loop() {
  if (fft.available()) {
    lcd.fillRect(0, 0, lcd.width(), 64, ST77XX_BLACK);  // Clear only spectrum area

    for (int i = 0; i < numBars; i++) {
      float magnitude = fft.read(i);
      if (magnitude < 0.00001) magnitude = 0.00001;
      float dB = 20.0 * log10(magnitude);
      if (dB < -60) dB = -60;
      if (dB > 0) dB = 0;

      int barHeight = map(dB, -60, 0, 0, 64);
      int barX = i * (barWidth + spacing);

      // Color gradient: green to red
      uint16_t color;
      if (dB > -10) color = ST77XX_RED;
      else if (dB > -30) color = ST77XX_ORANGE;
      else color = ST77XX_GREEN;

      lcd.fillRect(barX, 64 - barHeight, barWidth, barHeight, color);

      // Peak Hold Logic
      if (dB > peakHold[i]) {
        peakHold[i] = dB;
        peakHoldTime[i] = millis();
      } else if (millis() - peakHoldTime[i] > PEAK_HOLD_DURATION) {
        peakHold[i] -= 0.5; // fall rate
        if (peakHold[i] < -60) peakHold[i] = -60;
      }

      // Draw peak bar
      int peakY = map(peakHold[i], -60, 0, 64, 0);
      lcd.drawFastHLine(barX, peakY, barWidth, ST77XX_WHITE);

      // Frequency Label
      // lcd.setTextSize(1);
      // lcd.setTextColor(ST77XX_CYAN);
      // lcd.setCursor(barX, 66);
      // lcd.print(i * (22050 / 2 / numBars) / 1000); // frequency in kHz
    }

    delay(20);
  }
}