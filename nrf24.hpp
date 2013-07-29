#ifndef NRF24_H
#define NRF24_H

#include <stdint.h>
#include "spi.hpp"

class NRF24
{
public:
  enum Command {
    CMD_READ_REGISTER = 0x00, // Address mask == 0x1F
    CMD_WRITE_REGISTER = 0x20, // Address mask == 0x1F
    CMD_READ_RX_PAYLOAD = 0x61,
    CMD_WRITE_TX_PAYLOAD = 0xA0,
    CMD_FLUSH_TX = 0xE1,
    CMD_FLUSH_RX = 0xE2,
    CMD_REUSE_TX_PAYLOAD = 0xE3,
    CMD_READ_RX_PAYLOAD_WIDTH = 0x60,
    CMD_WRITE_ACK_PAYLOAD = 0xA1, // Response pipe mask == 0x07
    CMD_WRITE_TX_PAYLOAD_NO_ACK = 0xB0,
    CMD_NOP = 0xFF
  };

  enum Register {
    REG_CONFIG = 0x00,
    REG_EN_AA = 0x01,
    REG_EN_RX_ADDR = 0x02,
    REG_SETUP_AW = 0x03,
    REG_SETUP_RETR = 0x04,
    REG_RF_CH = 0x05,
    REG_RF_SETUP = 0x06,
    REG_STATUS = 0x07,
    REG_OBSERVE_TX = 0x08,
    REG_RPD = 0x09,
    REG_RX_ADDR_P0 = 0x0A,
    REG_RX_ADDR_P1 = 0x0B,
    REG_RX_ADDR_P2 = 0x0C,
    REG_RX_ADDR_P3 = 0x0D,
    REG_RX_ADDR_P4 = 0x0E,
    REG_RX_ADDR_P5 = 0x0F,
    REG_TX_ADDR = 0x10,
    REG_RX_PW_P0 = 0x11,
    REG_RX_PW_P1 = 0x12,
    REG_RX_PW_P2 = 0x13,
    REG_RX_PW_P3 = 0x14,
    REG_RX_PW_P4 = 0x15,
    REG_RX_PW_P5 = 0x16,
    REG_FIFO_STATUS = 0x17,
    REG_DYNPD = 0x1C,
    REG_FEATURE = 0x1D,
  };

  enum DataRate {
    DR_1MBPS = 0x00,
    DR_2MBPS = 0x01,
    DR_250KBPS = 0x02,
  };

  enum Power {
    PW_N18DBM = 0x00,
    PW_N12DBM = 0x02,
    PW_N6DBM = 0x04,
    PW_0DBM = 0x06,
  };

  enum Config {
    CONFIG_PWR_UP = 0x02,
    CONFIG_PRIM_RX = 0x01,
  };

  enum Status {
    STATUS_RX_DR = 0x40,
    STATUS_TX_DS = 0x20,
    STATUS_MAX_RT = 0x10,
    STATUS_TX_FIFO_FULL = 0x01,
  };

  static const uint8_t REGISTER_ADDRESS_MASK;

public:
  NRF24(SPI &spi, const uint16_t ce_pin);
  virtual ~NRF24();

  int32_t GetRegister(const uint8_t address, uint8_t &value);
  int32_t SetRegister(const uint8_t address, const uint8_t value);

  int32_t PowerUp();
  int32_t PowerDown();

  int32_t SetCRC(const uint8_t bytes);
  int32_t GetCRC(uint8_t &bytes);

  int32_t SetChannel(const uint8_t channel);
  int32_t GetChannel(uint8_t &channel);

  int32_t GetDataRate(DataRate &data_rate);
  int32_t SetDataRate(const DataRate data_rate);

  int32_t GetPower(Power &power);
  int32_t SetPower(const Power power);

  int32_t GetRxPayloadLength(const uint8_t pipe, uint8_t &length);
  int32_t SetRxPayloadLength(const uint8_t pipe, const uint8_t length);

  int32_t ClearStatus();

  int32_t GetRxPipeAddress(const uint8_t pipe, uint64_t &address);
  int32_t SetRxPipeAddress(const uint8_t pipe, const uint64_t address);
  int32_t SetRxPipeAddress(const uint8_t pipe, const uint8_t address);

  int32_t GetTxAddress(uint64_t &address);
  int32_t SetTxAddress(const uint64_t address);

  int32_t WriteTxPayload(const uint8_t *data, const uint8_t len);
  int32_t ReadRxPayload(uint8_t *data, const uint8_t len);

  int32_t FlushRx();
  int32_t FlushTx();

  int32_t Listen();
  int32_t StopListen();

  int32_t GetStatus(bool &data_ready, bool &data_sent, bool &max_retry);

  int32_t Send(const uint8_t *data, const uint8_t len);
  int32_t Receive(uint8_t *data, const uint8_t len);

protected:
  int32_t CE(const bool enable);

private:
  SPI &spi;
  uint16_t ce_pin;

  uint8_t *rx_buf;
  uint8_t *tx_buf;
};

#endif /* NRF24_H */
