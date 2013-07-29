#include <cstdlib>
#include <cstdio>
#include <memory.h>
#include "spi.hpp"
#include "nrf24.hpp"
#include "sleep.hpp"

int main(int argc, char** argv)
{
  int32_t result = 0;
  SPI spi;
  result = spi.Open("/dev/spidev0.0");
  if (result == 0) {
    NRF24 nrf(spi, 25); // above SPI bus and GPIO 25 as CE.
    nrf.PowerUp();
    nrf.SetChannel(1);
    nrf.SetDataRate(NRF24::DR_250KBPS);
    nrf.SetPower(NRF24::PW_0DBM);
    nrf.SetCRC(1);
    nrf.SetRxPayloadLength(0, 32);
    nrf.SetRxPayloadLength(1, 32);
    // uint64_t txAddress = 0x7365727631ULL; // "serv1"
    // uint64_t rxAddress = 0x636C696531ULL; // "clie1"
    uint64_t txAddress = 0x3176726573ULL; // "serv1"
    uint64_t rxAddress = 0x3165696C63ULL; // "clie1"
    nrf.SetRxPipeAddress(0, txAddress); 
    nrf.SetTxAddress(txAddress);
    nrf.SetRxPipeAddress(1, rxAddress);

    nrf.ClearStatus();
    nrf.FlushRx();
    nrf.FlushTx();

    uint8_t *tx_buf = new uint8_t [32];
    uint8_t *rx_buf = new uint8_t [32];
    memset(tx_buf, 0, 32);
    memset(rx_buf, 0, 32);
    bool more = false;
    do {
      more = false;
      tx_buf[0] = 0x01;
      nrf.Send(tx_buf, 32);
      msleep(10);
      nrf.Listen();
      bool ready = false, ds = false, mr = false;
      do {
	nrf.GetStatus(ready, ds, mr);	
      } while (ready == false);
      if (nrf.Receive(rx_buf, 32) >= 0) {
	uint8_t command = rx_buf[0];
	int32_t offset = 1;
	if (command == 0x81) {
	  int16_t code = 0;
	  memcpy(&code, rx_buf + offset, sizeof(int16_t));
	  offset += sizeof(int16_t);
	  if (code == 0) {
	    uint64_t device = 0;
	    memcpy(&device, rx_buf + offset, sizeof(uint64_t));
	    offset += sizeof(uint64_t);
	    int32_t value = 0;
	    memcpy(&value, rx_buf + offset, sizeof(int32_t));
	    double real_value = value / 1000000.0;
	    printf("Device: %016llx: %f\n", device, real_value);
	    more = true;
	  }
	  else if (code == 15) {
	    printf("No more sensors\n");
	  }
	  else {
	    printf("Unknown code %d\n", code);
	  }
	}
	else if (command == 0xff) {
	  int16_t code = 0;
	  memcpy(&code, rx_buf + offset, sizeof(int16_t));
	  printf("Error: %d\n", code);
	}
      }
      nrf.StopListen();
      msleep(10);
    } while (more);
    printf("End...\n");
    nrf.PowerDown();
  }
  return 0;
}
