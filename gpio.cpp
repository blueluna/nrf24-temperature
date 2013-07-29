/*
 * Copyright (c) 2011, Erik Svensson <erik.public@gmail.com>
 */

#include "gpio.hpp"
#include <cstdio>

GPIO::GPIO()
{
}

GPIO::~GPIO()
{
}

int32_t GPIO::Open(uint16_t port, uint8_t direction)
{
  FILE *f;
  f = fopen("/sys/class/gpio/export", "w");
  if (f == 0) {
    return -1;
  }
  fprintf(f, "%u\n", port);
  fclose(f);
  
  char file[128];
  sprintf(file, "/sys/class/gpio/gpio%u/direction", port);
  f = fopen(file, "w");
  if (f == 0) {
    return -1;
  }
  if (direction == INPUT) {
    fprintf(f, "in\n");
  }
  else {
    fprintf(f, "out\n");
  }
  fclose(f);
  return 0;
}

int32_t GPIO::Close(uint16_t port)
{
  FILE *f;
  f = fopen("/sys/class/gpio/unexport", "w");
  if (f == 0) {
    return -1;
  }
  fprintf(f, "%d\n", port);
  fclose(f);
  return 0;
}

int32_t GPIO::Read(uint16_t port)
{
  FILE *f;
  char file[128];
  sprintf(file, "/sys/class/gpio/gpio%u/value", port);
  f = fopen(file, "r");
  if (f == 0) {
    return -1;
  }
  int i;
  fscanf(f, "%d", &i);
  fclose(f);
  return i;
}

int32_t GPIO::Write(uint16_t port, uint8_t value)
{
  FILE *f;
  char file[128];
  sprintf(file, "/sys/class/gpio/gpio%u/value", port);
  f = fopen(file, "w");
  if (f == 0) {
    return -1;
  }
  if (value == LOW) {
    fprintf(f, "0\n");
  }
  else {
    fprintf(f, "1\n");
  }
  fclose(f);
  return 0;
}
