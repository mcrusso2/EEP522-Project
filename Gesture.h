
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "paj7620.h"

#define GESTURE_SPI_USELESS 0xff

class Pixart_Gesture
{
   protected:
      uint8_t spi_cs;
      uint8_t i2c_addr;
      void writeReg(uint8_t addr, uint8_t value);
      void readRegs(uint8_t addr, uint8_t *values, int size);
      uint8_t readReg(uint8_t addr);
};


class paj7620 : public Pixart_Gesture
{
   public:
      paj7620() {}
      bool init();
      bool getResult(paj7620_gesture_t& res);

   private:
      bool setReportMode(uint8_t reportMode);
};
