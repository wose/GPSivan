#ifndef _ENCODER_H_
#define _ENCODER_H_

class encoder
{
 public:
  encoder(int pin_a, int pin_b);
  ~encoder();

 private:
  int _pin_a;
  int _pin_b;
  volatile long _value;
  volatile int _lastEncoded;

 private:
  void setup_encoder(int pin_a, int pin_b);
  void update_encoder();
};

#endif
