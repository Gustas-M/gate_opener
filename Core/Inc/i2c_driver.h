#ifndef INC_I2C_DRIVER_H_
#define INC_I2C_DRIVER_H_

#define MAX_CONF_COUNT 8

#include <stdbool.h>

typedef enum {
	eI2cPort_First = 0,
	eI2cPort_Gps = eI2cPort_First,
	eI2cPort_Last
} eI2cPort_t;

typedef struct {
	uint8_t reg;
	uint8_t val;
} sI2cRegConfData_t;

typedef struct {
	size_t reg_count;
	sI2cRegConfData_t reg_conf_data[MAX_CONF_COUNT];
} sI2cConf_t;

bool I2C_Driver_Init (eI2cPort_t i2c_port);
bool I2C_Driver_Conf(eI2cPort_t i2c_port);
bool I2C_Driver_Write (I2C_TypeDef *port, uint8_t address, uint8_t reg, uint8_t *buffer, uint32_t byte_count);
bool I2C_Driver_Read (uint8_t reg, I2C_TypeDef *port, uint8_t dev_addr, uint8_t *rec, size_t byte_count);
bool I2C_Driver_WriteConfiguration (sI2cRegConfData_t reg_data, I2C_TypeDef *port, uint8_t address);


#endif /* INC_I2C_DRIVER_H_ */
