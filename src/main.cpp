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
}

void loop() {
  if (fft.available()) {
    lcd.fillScreen(ST77XX_BLACK);

    for (int i = 0; i < 64; i++) {
      float magnitude = fft.read(i) * 100.0;

      // Clamp and scale
      if (magnitude > 64) magnitude = 64;
      int barHeight = (int)magnitude;

      // Draw a vertical bar
      int barX = i * 2; // 2 pixels per bar
      lcd.fillRect(barX, 64 - barHeight, 2, barHeight, ST77XX_GREEN);
    }

    delay(20);
  }
}