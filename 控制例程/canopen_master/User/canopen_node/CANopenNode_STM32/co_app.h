#ifndef __CO_APP_H__
#define __CO_APP_H__
#include "main.h"
#include "fdcan.h"
#include "301/CO_SDOclient.h"

void canopen_init(void);
void canopen_app(void);
void canopen_para_init(void);
uint8_t canopen_write_clock(uint16_t _year, uint8_t _mon, uint8_t _day, uint8_t _hour, uint8_t _min, uint8_t _sec, uint32_t _ms, uint32_t _Interval_ms);

CO_SDO_abortCode_t read_sdo(uint8_t nodeId, uint16_t index, uint8_t subIndex, uint8_t *buf, size_t bufSize, size_t *readSize);
CO_SDO_abortCode_t write_sdo(uint8_t nodeId, uint16_t index, uint8_t subIndex, uint8_t *data, size_t dataSize);			

#endif /* __CANOPEN_BSP_H_ */

