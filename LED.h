
struct LED : Service::LightBulb {
  int ledPin;
  SpanCharacteristic *power;

  LED(int ledPin)
    : Service::LightBulb() {

    power = new Characteristic::On();  // this is where we create the On Characterstic we had previously defined in setup().  Save this in the pointer created above, for use below
    this->ledPin = ledPin;             // don't forget to store ledPin...
    pinMode(ledPin, OUTPUT);           // ...and set the mode for ledPin to be an OUTPUT (standard Arduino function)
  }                                    // end constructor

  boolean update() {

    digitalWrite(ledPin, power->getNewVal());

    return (true);  // return true to indicate the update was successful (otherwise create code to return false if some reason you could not turn on the LED)
  }

  void setLED(boolean state) {
    digitalWrite(ledPin, state);
    power->setVal(state);
  }

  boolean getLED() {
    return digitalRead(ledPin);
  }
};
