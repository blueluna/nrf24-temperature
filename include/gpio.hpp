/* 
 * Copyright (c) 2011, Erik Svensson <erik.public@gmail.com>
 */

#ifndef GPIO_HPP
#define	GPIO_HPP

#include <stdint.h>

class GPIO
{
public:
  static const uint8_t INPUT = 0;
  static const uint8_t OUTPUT = 1;
  static const uint8_t LOW = 0;
  static const uint8_t HIGH = 1;

public:
  GPIO();
  virtual ~GPIO();
  
  static int32_t Open(const uint16_t port, uint8_t output);
  static int32_t Close(const uint16_t port);
  
  static int32_t Read(const uint16_t port);
  static int32_t Write(const uint16_t port, uint8_t value);
};

#endif	/* GPIO_HPP */
