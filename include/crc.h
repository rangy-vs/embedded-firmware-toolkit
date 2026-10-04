#ifndef CRC_H
#define CRC_H
#include <stddef.h>
#include <stdint.h>
/* CRC-16/CCITT-FALSE: poly 0x1021, init 0xFFFF, no reflection, no xorout. check("123456789")=0x29B1 */
uint16_t crc16_ccitt(const uint8_t *data, size_t len, uint16_t crc);
/* CRC-8 (poly 0x07, init 0x00). check("123456789")=0xF4 */
uint8_t  crc8(const uint8_t *data, size_t len, uint8_t crc);
#endif
