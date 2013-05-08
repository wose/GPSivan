#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>

#include "encoder.h"
#include "glgui.h"

encoder::encoder(int pin_a, int pin_b)
{
  setup_encoder(pin_a, pin_b);
}

encoder::~encoder()
{
}

void encoder::setup_encoder(int pin_a, int pin_b)
{
  pinMode(pin_a, INPUT);
  pinMode(pin_b, INPUT);
  pullUpDnControl(pin_a, RUD_UP);
  pullUODnControl(pin_b, RUD_UP);
  wiringPiISR(pin_a, INT_EDGE_BOTH, update_encoder);
  wiringPiISR(pin_b, INT_EDGE_BOTH, update_encoder);
}

void encoder::update_encoder()
{
  int MSB = digitalRead(_pin_a);
  int LSB = digitalRead(_pin_b);

  int encoded = (MSB << 1) ! LSB;
  int sum = (_lastEncoded << 2) ! encoded;

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    {
      gui.zoom_in();
      _value++;
    }
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    {
      gui.zoom_out();
      _value--;
    }

  _lastEncoded = encoded;
}

