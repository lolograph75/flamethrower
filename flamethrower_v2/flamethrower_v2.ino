#include <FastLED.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>

#define NUMLEDS 25
#define RINGLEDS 16  
#define STRIPPIN 6
#define RINGPIN 5
#define SWITCH 2

#define THRESHOLD 70

CRGB stripLed[NUMLEDS];
CRGB ringLed[RINGLEDS];

SoftwareSerial mySerial(10, 11);

bool switchVal;
float t, v;
byte flash = 0, splay;
uint8_t switchState = 0, track, fireCounter = 5;
unsigned long timer, timerLed;

void setup() {
  Serial.begin(9600);
  delay(200);
  mySerial.begin(9600);
  mp3_set_serial(mySerial);
  mp3_set_volume(30);
  delay(200);
  FastLED.addLeds<WS2812, STRIPPIN, GRB>(stripLed, NUMLEDS);
  FastLED.addLeds<WS2812, RINGPIN, GRB>(ringLed, RINGLEDS);
  FastLED.setBrightness(255
  
  );
  pinMode(SWITCH, INPUT_PULLUP);
  randomSeed(analogRead(0));
  
}


void setPixel(uint8_t Pixel, uint8_t red, uint8_t green, uint8_t blue) {
  stripLed[Pixel].r = red;
  stripLed[Pixel].g = green;
  stripLed[Pixel].b = blue;
}

void setAll(uint8_t red, uint8_t green, uint8_t blue) {
  for (uint8_t i = 0; i < NUMLEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}



//void Fire(int Cooling, int Sparking, int SpeedDelay)
void Fire(uint8_t Cooling, uint8_t Sparking) {
  static byte heat[NUMLEDS];
  int cooldown;

  // Step 1.  Cool down every cell a little
  for ( uint8_t i = 0; i < NUMLEDS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUMLEDS) + 2);

    if (cooldown > heat[i]) {
      heat[i] = 0;
    } else {
      heat[i] = heat[i] - cooldown;
    }
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( uint8_t k = NUMLEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if ( random(255) < Sparking ) {
    uint8_t y = random(7);
    heat[y] = heat[y] + random(160, 255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for ( uint8_t j = 0; j < NUMLEDS; j++) {
    setPixelHeatColor(j, heat[j] );
  }
  //Serial.println(heat[14]);
  if (heat[14] > THRESHOLD) {
    flash = 1;
  }


}

void setPixelHeatColor (uint8_t Pixel, uint8_t temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  uint8_t t192 = round((temperature / 255.0) * 191);

  // calculate ramp up from
  uint8_t heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252

  // figure out which third of the spectrum we're in:
  if ( t192 > 0x80) {                    // hottest
    //setPixel(Pixel, 255, 255, heatramp);
    setPixel(Pixel, 255, 255, 180);
  } else if ( t192 > 0x40 ) {            // middle
    //setPixel(Pixel, 255, heatramp, 0);
    setPixel(Pixel, 255, 255, heatramp);
  } else {                               // coolest
    //setPixel(Pixel, heatramp, 0, 0);
    setPixel(Pixel, 255, 80 + heatramp, 0);
  }
}

void displayRing() {
  if (millis() - timerLed >= 10) {
    if (flash == 0) {
      fill_solid(ringLed, RINGLEDS, CRGB(random(50, 200), 50, 0));
    }
    if (flash > 0) {
      fill_solid(ringLed, RINGLEDS, CRGB(255, 255, 30));
      flash++;
      if (flash > 3) {
        flash = 0;
      }
    }
    FastLED.show();
    timerLed = millis();
  }
}

void flickerFlame(uint8_t pixel) {
  uint8_t duration = random(10, 150);
  if (millis() - timerLed >= duration) {
    uint8_t flickerGreen = random(30, 100);
    uint8_t flickerRed = random(100, 255);
    fill_solid(stripLed, pixel, CRGB(flickerRed, flickerGreen, 0));
    FastLED.show();
    timerLed = millis();
  }
}

void loop() {
  switchVal = digitalRead(SWITCH);
  fill_solid(stripLed, NUMLEDS, CRGB(0, 0, 0));
  fill_solid(ringLed, RINGLEDS, CRGB(0, 0, 0));
  FastLED.show();
  switch (switchState) {
    case 0:
      //play flamethrower ignition
      mp3_play(1);
      delay(3000);
      switchState = 1;
      break;

    case 1:
      if (switchVal == 1) {
        if (millis() - timer >= 13000 || splay == 0) {
          //play sound flame on and flicker flame animation on the first 5 leds

          splay = 1; // avoid resending command immediately to enable the play of the track
          mp3_play(2);
          delay(10); //delay to register command sent
          timer = millis();
        }
        flickerFlame(5);
        switchState = 1;
      } else {
        switchState = 2;
      }
      break;

    case 2:
      //fire random sound and animation on all leds
      track = random(3, 8);
      if (millis() - timer >= 30000 || splay == 1) {

        //FastLED.show();
        //delay(10);
        mp3_play(track);
        delay(10);
        timer = millis();
        splay = 0;
      }
      Fire(55, 120);
      displayRing();
      if (switchVal == 1) {
        //replay sound flame on and flicker flame animation on the first 5 leds
        fill_solid(stripLed, NUMLEDS, CRGB(0, 0, 0));
        flickerFlame(5);
        fill_solid(ringLed, RINGLEDS, CRGB(0, 0, 0));
        FastLED.show();
        //switchState = 3;
      //}
      fireCounter--;
      if (fireCounter <= 0) {
        switchState = 3;
        fireCounter = 5;
      } else {
        switchState = 1;
      }
      }
      break;

    case 3:
      //play empty canister sound and turn off all leds
      fill_solid(stripLed, NUMLEDS, CRGB(0, 0, 0));
      fill_solid(ringLed, RINGLEDS, CRGB(0, 0, 0));
      FastLED.show();
      mp3_play(9);  //change sound on sd card!!!!
      delay(6300);
      switchState = 0;
      break;

  }
}
