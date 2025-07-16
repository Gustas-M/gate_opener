#ifndef INC_I2C_DRIVER_H_
#define INC_I2C_DRIVER_H_

#include <stdbool.h>

typedef enum {
	eI2cPort_First = 0,
	eI2cPort_Gps = eI2cPort_First,
	eI2cPort_Last
} eI2cPort_t;

bool I2C_Driver_Init (eI2cPort_t i2c_port);
bool I2C_Driver_Write (eI2cPort_t i2c, uint8_t *buffer, size_t byte_count);
bool I2C_Driver_Read (eI2cPort_t i2c, uint8_t *rec, size_t byte_count);

#endif /* INC_I2C_DRIVER_H_ */
