#include "nrf24.hpp"
#include <memory.h>
#include <stdio.h>
#include "gpio.hpp"
#include "sleep.hpp"

const uint8_t NRF24::REGISTER_ADDRESS_MASK = 0x1F;

NRF24::NRF24(SPI &spi, const uint16_t ce_pin)
  : spi(spi)
  , ce_pin(ce_pin)
{
  rx_buf = new uint8_t [33];
  tx_buf = new uint8_t [33];
  memset(rx_buf, 0, 33);
  memset(tx_buf, 0xff, 33);
  GPIO::Open(ce_pin, GPIO::OUTPUT);
  GPIO::Write(ce_pin, GPIO::LOW);
}

NRF24::~NRF24()
{
  GPIO::Close(ce_pin);
  delete [] rx_buf;
  delete [] tx_buf;
}

int32_t NRF24::GetRegister(const uint8_t address, uint8_t &value)
{
  if (address > REGISTER_ADDRESS_MASK) {
    return -1;
  }
  tx_buf[0] = CMD_READ_REGISTER | (address & REGISTER_ADDRESS_MASK);
  tx_buf[1] = CMD_NOP;
  int32_t result = spi.Transfer(tx_buf, rx_buf, 2);
  if (result >= 0) {
    value = rx_buf[1];
  }
  return result;
}

int32_t NRF24::SetRegister(const uint8_t address, const uint8_t value)
{
  if (address > REGISTER_ADDRESS_MASK) {
    return -1;
  }
  tx_buf[0] = CMD_WRITE_REGISTER | (address & REGISTER_ADDRESS_MASK);
  tx_buf[1] = value;
  return spi.Transfer(tx_buf, rx_buf, 2);
}

int32_t NRF24::GetRxPipeAddress(const uint8_t pipe, uint64_t &address)
{
  if (pipe > 5) {
    return -1;
  }
  int32_t result = 0;
  uint64_t addr = 0;
  memset(tx_buf, 0xff, 6);
  if (pipe > 1) {
    tx_buf[0] = CMD_READ_REGISTER | REG_RX_ADDR_P1; // Read MSB    
    result = spi.Transfer(tx_buf, rx_buf, 6);
    if (result >= 0) {
      addr = ((uint64_t)(rx_buf[5]) << 32) | ((uint64_t)(rx_buf[4]) << 24) | ((uint64_t)(rx_buf[3]) << 16) | ((uint64_t)(rx_buf[2]) << 8);
      tx_buf[0] = CMD_READ_REGISTER | (REG_RX_ADDR_P0 + pipe); // Read LSB
      result = spi.Transfer(tx_buf, rx_buf, 2);
      if (result >= 0) {
	addr |= (uint64_t)(rx_buf[1]);
      }
    }
  }
  else {
    tx_buf[0] = CMD_READ_REGISTER | (REG_RX_ADDR_P0 + pipe);
    result = spi.Transfer(tx_buf, rx_buf, 6);
    if (result >= 0) {
      addr = ((uint64_t)(rx_buf[5]) << 32) | ((uint64_t)(rx_buf[4]) << 24) | ((uint64_t)(rx_buf[3]) << 16) | ((uint64_t)(rx_buf[2]) << 8) | (uint64_t)(rx_buf[1]);
    }
  }
  if (result >= 0) {
    address = addr;
  }
  return result;
}

int32_t NRF24::SetRxPipeAddress(const uint8_t pipe, const uint64_t address)
{
  if (pipe > 1) {
    return -1;
  }
  tx_buf[0] = CMD_WRITE_REGISTER | (REG_RX_ADDR_P0 + pipe);
  tx_buf[1] = static_cast<uint8_t>(address);
  tx_buf[2] = static_cast<uint8_t>(address >> 8);
  tx_buf[3] = static_cast<uint8_t>(address >> 16);
  tx_buf[4] = static_cast<uint8_t>(address >> 24);
  tx_buf[5] = static_cast<uint8_t>(address >> 32);
  return spi.Transfer(tx_buf, rx_buf, 6);
}

int32_t NRF24::GetTxAddress(uint64_t &address)
{
  int32_t result = 0;
  memset(tx_buf, 0xff, 6);
  tx_buf[0] = CMD_READ_REGISTER | REG_TX_ADDR;
  result = spi.Transfer(tx_buf, rx_buf, 6);
  if (result >= 0) {
    address = ((uint64_t)(rx_buf[5]) << 32) | ((uint64_t)(rx_buf[4]) << 24) | ((uint64_t)(rx_buf[3]) << 16) | ((uint64_t)(rx_buf[2]) << 8) | (uint64_t)(rx_buf[1]);
  }
  return result;
}

int32_t NRF24::SetTxAddress(const uint64_t address)
{
  tx_buf[0] = CMD_WRITE_REGISTER | REG_TX_ADDR;
  tx_buf[1] = static_cast<uint8_t>(address);
  tx_buf[2] = static_cast<uint8_t>(address >> 8);
  tx_buf[3] = static_cast<uint8_t>(address >> 16);
  tx_buf[4] = static_cast<uint8_t>(address >> 24);
  tx_buf[5] = static_cast<uint8_t>(address >> 32);
  return spi.Transfer(tx_buf, rx_buf, 6);
}

int32_t NRF24::SetRxPipeAddress(const uint8_t pipe, const uint8_t address)
{
  if (pipe < 2 && pipe > 5) {
    return -1;
  }
  tx_buf[0] = CMD_WRITE_REGISTER | (0x0A + pipe);
  tx_buf[1] = address;
  return spi.Transfer(tx_buf, rx_buf, 6);
}

int32_t NRF24::CE(const bool enable)
{
  return GPIO::Write(ce_pin, enable ? GPIO::HIGH : GPIO::LOW);
}

int32_t NRF24::PowerUp()
{
  uint8_t reg = 0;
  if (GetRegister(REG_CONFIG, reg) >= 0) {
    if ((reg & 0x02) == 0x02) {
      return 0;
    }
    else {
      reg |= 0x02;
      return SetRegister(REG_CONFIG, reg);
    }
  }
  return -1;
}

int32_t NRF24::PowerDown()
{
  uint8_t reg = 0;
  if (GetRegister(REG_CONFIG, reg) >= 0) {
    if ((reg & 0x02) == 0x02) {
      reg &= 0xFD;
      return SetRegister(REG_CONFIG, reg);
    }
    else {
      return 0;
    }
  }
  return -1;
}

int32_t NRF24::GetCRC(uint8_t &bytes)
{
  uint8_t reg = 0;
  if (GetRegister(REG_CONFIG, reg) >= 0) {
    if ((reg & 0x08) == 0x08) {
      if ((reg & 0x04) == 0x04) {
	bytes = 2;
      }
      else {
	bytes = 1;
      }
    }
    else {
      bytes = 0;
    }
    return 0;
  }
  return -1;
}

int32_t NRF24::SetCRC(const uint8_t bytes)
{
  uint8_t reg = 0;
  if (GetRegister(REG_CONFIG, reg) >= 0) {    
    uint8_t crc = 0;
    if (bytes == 1) { crc = 0x08; }
    else if (bytes == 2) { crc = 0x0C; }
    reg = (reg & 0xF3) | crc;
    return SetRegister(REG_CONFIG, reg);
  }
  return -1;
}

int32_t NRF24::GetChannel(uint8_t &channel)
{
  uint8_t reg = 0;
  if (GetRegister(REG_RF_CH, reg) >= 0) {
    channel = reg;
    return 0;
  }
  return -1;
}

int32_t NRF24::SetChannel(const uint8_t channel)
{
  if (channel <= 0x7f) {
    return SetRegister(REG_RF_CH, channel);
  }
  return -1;
}

int32_t NRF24::GetDataRate(DataRate &data_rate)
{
  uint8_t reg = 0;
  if (GetRegister(REG_RF_SETUP, reg) >= 0) {
    if ((reg & 0x20) == 0x20) {
      data_rate = DR_250KBPS;
    }
    else {
      if ((reg & 0x08) == 0x08) {
	data_rate = DR_2MBPS;
      }
      else {
	data_rate = DR_1MBPS;
      }
    }
    return 0;
  }
  return -1;
}

int32_t NRF24::SetDataRate(const DataRate data_rate)
{
  uint8_t reg = 0;
  if (GetRegister(REG_RF_SETUP, reg) >= 0) {
    uint8_t dr = 0;
    if (data_rate == DR_1MBPS) {
      dr = 0x08;
    }
    else if (data_rate == DR_2MBPS) {
      dr = 0x28;
    }
    else {
      dr = 0x20;
    }
    reg = (reg & 0xD7) | dr;
    return SetRegister(REG_RF_SETUP, reg);
  }
  return -1;
}

int32_t NRF24::GetPower(Power &power)
{
  uint8_t reg = 0;
  if (GetRegister(REG_RF_SETUP, reg) >= 0) {
    power = static_cast<Power>((reg & 0x06));
    return 0;
  }
  return -1;
}

int32_t NRF24::SetPower(const Power power)
{
  uint8_t reg = 0;
  if (GetRegister(REG_RF_SETUP, reg) >= 0) {
    reg = (reg & 0xF9) | static_cast<uint8_t>(power);
    return SetRegister(REG_RF_SETUP, reg);
  }
  return -1;
}

int32_t NRF24::GetRxPayloadLength(const uint8_t pipe, uint8_t &length)
{
  if (pipe > 5) {
    return -1;
  }
  return GetRegister(REG_RX_PW_P0 + pipe, length);
}

int32_t NRF24::SetRxPayloadLength(const uint8_t pipe, const uint8_t length)
{
  if (pipe > 5 || length > 32) {
    return -1;
  }
  return SetRegister(REG_RX_PW_P0 + pipe, length);
}

int32_t NRF24::ClearStatus()
{
  return SetRegister(REG_STATUS, STATUS_RX_DR | STATUS_TX_DS | STATUS_MAX_RT);
}

int32_t NRF24::WriteTxPayload(const uint8_t *data, const uint8_t len)
{
  if (data == 0 || len == 0 || len > 32) {
    return -1;
  }
  memcpy(tx_buf + 1, data, len);
  tx_buf[0] = CMD_WRITE_TX_PAYLOAD;
  if (spi.Transfer(tx_buf, rx_buf, (len + 1)) >= 0) {
    return len;
  }
  else {
    return -1;
  }
}

int32_t NRF24::ReadRxPayload(uint8_t *data, const uint8_t len)
{
  if (data == 0 || len == 0 || len > 32) {
    return -1;
  }
  memset(tx_buf, 0xff, (len+1));
  tx_buf[0] = CMD_READ_RX_PAYLOAD;
  int32_t result = spi.Transfer(tx_buf, rx_buf, (len + 1));
  if (result >= 0) {
    memcpy(data, rx_buf + 1, len);
    return len;
  }
  return -1;
}

int32_t NRF24::FlushRx()
{
  tx_buf[0] = CMD_FLUSH_RX;
  return spi.Transfer(tx_buf, rx_buf, 1);
}

int32_t NRF24::FlushTx()
{
  tx_buf[0] = CMD_FLUSH_TX;
  return spi.Transfer(tx_buf, rx_buf, 1);
}

int32_t NRF24::Listen()
{
  uint8_t reg;
  int32_t result = GetRegister(REG_CONFIG, reg);
  if (result >= 0) {
    reg |= (CONFIG_PWR_UP | CONFIG_PRIM_RX);
    result = SetRegister(REG_CONFIG, reg);
  }
  if (result >= 0) {
    result = ClearStatus();
  }
  if (result >= 0) {
    result = CE(true);
  }
  return result;
}

int32_t NRF24::StopListen()
{
  int32_t result = CE(false);
  FlushRx();
  FlushTx();
  return result;
}

int32_t NRF24::GetStatus(bool &data_ready, bool &data_sent, bool &max_retry)
{
  uint8_t reg;
  int32_t result = GetRegister(REG_STATUS, reg);
  if (result >= 0) {
    data_ready = (reg & STATUS_RX_DR) == STATUS_RX_DR;
    data_sent = (reg & STATUS_TX_DS) == STATUS_TX_DS;
    max_retry = (reg & STATUS_MAX_RT) == STATUS_MAX_RT;
  }
  return result;
}

int32_t NRF24::Send(const uint8_t *data, const uint8_t len)
{
  uint8_t reg = 0;
  CE(false);
  int32_t result = GetRegister(REG_CONFIG, reg);
  if (result >= 0) {
    reg = (reg & 0xFC) | CONFIG_PWR_UP;
    result = SetRegister(REG_CONFIG, reg);
  }
  if (result >= 0) {
    result = WriteTxPayload(data, len);
  }
  if (result >= 0) {
    result = ClearStatus();
  }
  CE(true);
  // Wait for data send or max retries
  do {
    result = GetRegister(REG_STATUS, reg);
  } while ((reg & STATUS_TX_DS) == STATUS_TX_DS || (reg & STATUS_MAX_RT) == STATUS_MAX_RT);
  // Clear status
  if ((reg & STATUS_TX_DS) == STATUS_TX_DS) {
    SetRegister(REG_STATUS, STATUS_TX_DS);
    result = 0;
  }
  if ((reg & STATUS_MAX_RT) == STATUS_MAX_RT) {
    SetRegister(REG_STATUS, STATUS_MAX_RT);
    result = -2;
  }
  CE(false);
  return result;
}

int32_t NRF24::Receive(uint8_t *data, const uint8_t len)
{
  uint8_t reg;
  int32_t result = GetRegister(REG_CONFIG, reg);
  if (result >= 0) {
    if ((reg & CONFIG_PRIM_RX) == 0) {
      result = -1;
    }
  }
  if (result >= 0) {
    result = GetRegister(REG_STATUS, reg); // Check status
  }
  if (result >= 0) {
    if ((reg & STATUS_RX_DR) == 0) {
      result = -1;
    }
  }
  if (result >= 0) {
    result = ReadRxPayload(data, len);
  }
  SetRegister(REG_STATUS, STATUS_RX_DR); // Clear status
  return result;  
}
