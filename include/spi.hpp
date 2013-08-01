#ifndef SPI_H
#define SPI_H

#include <stdint.h>

class SPI
{
public:
  SPI();
  SPI(const uint32_t speed, const uint8_t bits, const uint8_t mode);
  virtual ~SPI();

  int32_t Open(const char *device);

  int32_t Transfer(const uint8_t tx, uint8_t &rx);
  int32_t Transfer(uint8_t *tx, uint8_t *rx, const uint16_t len);

private:
  int fd; /* File handle */

  uint32_t speed;
  uint8_t bits;
  uint8_t mode;
};

#endif /* SPI_H */


