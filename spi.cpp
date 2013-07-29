#include "spi.hpp"

#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <inttypes.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

SPI::SPI()
  : fd(-1)
  , speed(2000000)
  , bits(8)
  , mode(0)
{
}

SPI::SPI(const uint32_t speed, const uint8_t bits, const uint8_t mode)
  : fd(-1)
  , speed(speed)
  , bits(bits)
  , mode(mode)
{
}

SPI::~SPI()
{
  if (fd > 0) {
    close(fd);
  }
}

int32_t SPI::Open(const char *devicefile)
{
  int32_t result = 0;
  fd = open(devicefile, O_RDWR);
  if (fd < 0) {
    fd = -1;
    result = -1;
    perror("can't open device");
  }
  int ret;
  /*
   * spi mode
   */
  if (result == 0) {
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) {
      result = -1;
      perror("can't set spi write mode");
    }
  }
  if (result == 0) {
    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1) {
      result = -1;
      perror("can't set spi read mode");
    }
  }

  /*
   * bits per word
   */
  if (result == 0) {
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {
      result = -1;
      perror("can't set write bits per word");
    }
  }

  if (result == 0) {
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) {
      result = -1;
      perror("can't set read bits per word");
    }
  }

  /*
   * max speed hz
   */
  if (result == 0) {
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
      result = -1;
      perror("can't set write max speed hz");
    }
  }

  if (result == 0) {
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
      result = -1;
      perror("can't set read max speed hz");
    }
  }

  if (fd >= 0 && result < 0) {
    close(fd);
  }

  return result;
}

int32_t SPI::Transfer(uint8_t *tx, uint8_t *rx, const uint16_t len)
{
  int32_t result = 0;
  int ret;
  // One byte is transfered at once
  struct spi_ioc_transfer tr;
  tr.tx_buf = (unsigned long)tx;
  tr.rx_buf = (unsigned long)rx;
  tr.len = len;
  tr.delay_usecs = 0;
  tr.cs_change = 1;
  tr.speed_hz = speed;
  tr.bits_per_word = bits;

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1) {
    perror("can't send spi message");
  }
  result = ret;
  return result;
}

int32_t SPI::Transfer(const uint8_t tx, uint8_t &rx)
{
  int32_t result = 0;
  int ret;
  // One byte is transfered at once
  uint8_t tx_buf[] = {0};
  tx_buf[0] = tx;

  uint8_t rx_buf[ARRAY_SIZE(tx_buf)] = {0};
  struct spi_ioc_transfer tr;
  tr.tx_buf = (unsigned long)tx_buf;
  tr.rx_buf = (unsigned long)rx_buf;
  tr.len = ARRAY_SIZE(tx_buf);
  tr.delay_usecs = 0;
  tr.cs_change = 1;
  tr.speed_hz = speed;
  tr.bits_per_word = bits;

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1) {
    perror("can't send spi message");
  }
  else {
    rx = rx_buf[0];
  }
  result = ret;
  return result;
}

