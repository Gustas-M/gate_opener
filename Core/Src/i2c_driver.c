#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_bus.h"
#include "i2c_driver.h"

#define GPS_APP_TASK_STACK_SIZE 512

#define NO_THREAD_ARGUMENTS NULL

typedef void (*EnableClock_t)(uint32_t periph);

typedef struct {
	I2C_TypeDef *port;
	uint32_t periph_mode;
	uint32_t clock_speed;
	uint32_t duty_cycle;
	uint32_t analog_filter;
	uint32_t digital_filter;
	uint32_t own_address;
	uint32_t type_acknowledge;
	uint32_t own_addr_size;
	uint8_t dev_addr;
	EnableClock_t enable_clock;
	uint32_t clock;
} sI2cDesc_t;

const static sI2cDesc_t g_static_i2c_lut[eI2cPort_Last] = {
	[eI2cPort_Gps] = {
		.port = I2C1,
		.periph_mode = LL_I2C_MODE_I2C,
		.clock_speed = 100000,
		.duty_cycle = LL_I2C_DUTYCYCLE_2,
		.analog_filter = LL_I2C_ANALOGFILTER_DISABLE,
		.digital_filter = 0x00,
		.own_address = 0,
		.type_acknowledge = LL_I2C_ACK,
		.own_addr_size = LL_I2C_OWNADDRESS1_7BIT,
		.dev_addr = 0x53,
		.enable_clock = LL_APB1_GRP1_EnableClock,
		.clock = LL_APB1_GRP1_PERIPH_I2C1
	}
};

const static osThreadAttr_t g_static_gps_app_task_attr = {
    .name = "Gps App Task",
    .stack_size = LED_APP_TASK_STACK_SIZE,
    .priority = (osPriority_t)osPriorityNormal
};

static osThreadId_t g_static_gps_app_task = NULL;

static void GPS_APP_Task (void) {
	while (1) {

	}
}

bool I2C_Driver_Init (eI2cPort_t i2c_port) {
	LL_I2C_InitTypeDef i2c_init_stuct = {0};

	g_static_i2c_lut[i2c_port].enable_clock(g_static_i2c_lut[i2c_port].clock);

	LL_I2C_DisableOwnAddress2(g_static_i2c_lut[i2c_port].port);
	LL_I2C_DisableGeneralCall(g_static_i2c_lut[i2c_port].port);
	LL_I2C_EnableClockStretching(g_static_i2c_lut[i2c_port].port);

	i2c_init_stuct.PeripheralMode = g_static_i2c_lut[i2c_port].periph_mode;
	i2c_init_stuct.ClockSpeed = g_static_i2c_lut[i2c_port].clock_speed;
	i2c_init_stuct.DutyCycle = g_static_i2c_lut[i2c_port].duty_cycle;
	i2c_init_stuct.AnalogFilter = g_static_i2c_lut[i2c_port].analog_filter;
	i2c_init_stuct.DigitalFilter = g_static_i2c_lut[i2c_port].digital_filter;
	i2c_init_stuct.OwnAddress1 = g_static_i2c_lut[i2c_port].own_address;
	i2c_init_stuct.TypeAcknowledge = g_static_i2c_lut[i2c_port].type_acknowledge;
	i2c_init_stuct.OwnAddrSize = g_static_i2c_lut[i2c_port].own_addr_size;

	if (LL_I2C_Init(g_static_i2c_lut[i2c_port].port, &i2c_init_stuct) != SUCCESS ) {
		return false;
	}

	LL_I2C_SetOwnAddress2(g_static_i2c_lut[i2c_port].port, 0);
	LL_I2C_Enable(g_static_i2c_lut[i2c_port].port);

	return true;
}

bool GPS_APP_Init (void) {
	if (g_static_gps_app_task == NULL) {
		g_static_gps_app_task = osThreadNew(GPS_APP_Task, NO_THREAD_ARGUMENTS, &g_static_gps_app_task_attr);
		if (g_static_gps_app_task == NULL) {
			return false;
		}
	}

	return true;
}

bool I2C_Driver_Write (I2C_TypeDef *port, uint8_t address, uint8_t reg, uint8_t *buffer, uint32_t byte_count) {
    if ((port == NULL) || (buffer == NULL) || (byte_count == 0)) {
        return false;
    }

    address = address << 1;
    uint8_t address_w = address | 0;

    LL_I2C_GenerateStartCondition(port);

    while (!LL_I2C_IsActiveFlag_SB(port)) {};

    LL_I2C_TransmitData8(port, address_w);

    while (!LL_I2C_IsActiveFlag_TXE(port)) {};

    LL_I2C_TransmitData8(port, reg);

    while (!LL_I2C_IsActiveFlag_ADDR(port)) {};

    while (byte_count > 0) {
        while (!LL_I2C_IsActiveFlag_TXE(port)) {};

        LL_I2C_TransmitData8(port, *buffer);
        buffer++;
        byte_count--;
    }

    while (!LL_I2C_IsActiveFlag_TXE(port)) {};
    LL_I2C_GenerateStopCondition(port); // stop

    return true;
}


bool I2C_Driver_Read (uint8_t reg, I2C_TypeDef *port, uint8_t address, uint8_t *rec, size_t byte_count) {
	if ((port == NULL) || (rec == NULL) || (byte_count == 0)) {
	        return false;
	}

	address = address << 1;
	uint8_t address_w = address | 0;
	uint8_t address_r = address | 1;

	while (LL_I2C_IsActiveFlag_BUSY(I2C1));

	LL_I2C_GenerateStartCondition(I2C1);
	while (!LL_I2C_IsActiveFlag_SB(I2C1));

	LL_I2C_TransmitData8(I2C1, address_w);
	while (!LL_I2C_IsActiveFlag_ADDR(I2C1));
	LL_I2C_ClearFlag_ADDR(port);

	while (!LL_I2C_IsActiveFlag_TXE(I2C1));
	LL_I2C_TransmitData8(I2C1, reg);
	while (!LL_I2C_IsActiveFlag_TXE(I2C1));

	LL_I2C_GenerateStopCondition(port);
	LL_I2C_GenerateStartCondition(I2C1);
	while (!LL_I2C_IsActiveFlag_SB(I2C1));

	LL_I2C_TransmitData8(I2C1, address_r);
	while (!LL_I2C_IsActiveFlag_ADDR(I2C1));
	LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
	LL_I2C_ClearFlag_ADDR(port);

	while (!LL_I2C_IsActiveFlag_RXNE(I2C1));
	*rec = LL_I2C_ReceiveData8(port);

	LL_I2C_GenerateStopCondition(I2C1);

	return true;
}
