/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_bus.h"
#include "uart_driver.h"
#include "ring_buffer.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
typedef void (*EnableClock_t)(uint32_t periph);
typedef struct {
    USART_TypeDef *port;
    uint32_t baud_rate;
    uint32_t data_width;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t transfer_direction;
    uint32_t hardware_flow_control;
    uint32_t over_sampling;
    uint32_t clock;
    uint32_t ringbuffer_capacity;
    IRQn_Type irqn;
    EnableClock_t enable_clock;
} sUartDesc_t;
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static sUartDesc_t g_static_uart_lut[eUartPort_Last] = {
    [eUartPort_Usart1] = {
        .port = USART1,
        .baud_rate = 9600,
        .data_width = LL_USART_DATAWIDTH_8B,
        .stop_bits = LL_USART_STOPBITS_1,
        .parity = LL_USART_PARITY_NONE,
        .transfer_direction = LL_USART_DIRECTION_TX_RX,
        .hardware_flow_control = LL_USART_HWCONTROL_NONE,
        .over_sampling = LL_USART_OVERSAMPLING_16,
        .clock = LL_APB2_GRP1_PERIPH_USART1,
        .ringbuffer_capacity = 128,
        .irqn = USART1_IRQn,
        .enable_clock = LL_APB2_GRP1_EnableClock
    }
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static sRingBuffer_t *g_static_uart_rb[eUartPort_Last] = {0};
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
static void UART_Driver_IRQReceive (eUartPort_t port);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void UART_Driver_IRQReceive (eUartPort_t port) {
    if ((port < eUartPort_First) || (port >= eUartPort_Last)) {
        return;
    }

    if (g_static_uart_rb[port] != NULL) {
        if ((LL_USART_IsActiveFlag_RXNE(g_static_uart_lut[port].port)) && (LL_USART_IsEnabledIT_RXNE(g_static_uart_lut[port].port))) {
            uint8_t data = LL_USART_ReceiveData8(g_static_uart_lut[port].port);
            RingBufferEnqueue(g_static_uart_rb[port], data);
        }
    }

    return;
}

void USART1_IRQHandler (void) {
    UART_Driver_IRQReceive(eUartPort_Usart1);
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
bool UART_Driver_Init (eUartPort_t port, uint32_t baudrate) {
    if ((port < eUartPort_First) || (port >= eUartPort_Last)) {
        return false;
    }

    LL_USART_InitTypeDef usart_init_struct = {0};

    g_static_uart_lut[port].enable_clock(g_static_uart_lut[port].clock);

    usart_init_struct.BaudRate = (baudrate == 0) ? g_static_uart_lut[port].baud_rate : baudrate;
    usart_init_struct.DataWidth = g_static_uart_lut[port].data_width;
    usart_init_struct.StopBits = g_static_uart_lut[port].stop_bits;
    usart_init_struct.Parity = g_static_uart_lut[port].parity;
    usart_init_struct.TransferDirection = g_static_uart_lut[port].transfer_direction;
    usart_init_struct.HardwareFlowControl = g_static_uart_lut[port].hardware_flow_control;
    usart_init_struct.OverSampling = g_static_uart_lut[port].over_sampling;

    NVIC_SetPriority(g_static_uart_lut[port].irqn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(g_static_uart_lut[port].irqn);

    if (LL_USART_Init(g_static_uart_lut[port].port, &usart_init_struct) != SUCCESS) {
        return false;
    }
    LL_USART_ConfigAsyncMode(g_static_uart_lut[port].port);
    LL_USART_EnableIT_RXNE(g_static_uart_lut[port].port);
    LL_USART_Enable(g_static_uart_lut[port].port);

    if (LL_USART_IsEnabledIT_RXNE(g_static_uart_lut[port].port) == 0) {
        return false;
    }

    g_static_uart_rb[port] = RingBufferInit(g_static_uart_lut[port].ringbuffer_capacity);
    if (g_static_uart_rb[port] == NULL) {
        return false;
    }

    if (LL_USART_IsEnabled(g_static_uart_lut[port].port) == 0) {
        return false;
    }

    return true;
}

bool UART_Driver_TransmitByte (eUartPort_t port, uint8_t data) {
    if ((port < eUartPort_First) || (port >= eUartPort_Last)) {
        return false;
    }

    while (!LL_USART_IsActiveFlag_TXE(g_static_uart_lut[port].port)) {};
    LL_USART_TransmitData8(g_static_uart_lut[port].port, data);

    return true;
}

bool UART_Driver_TransmitMultipleBytes (eUartPort_t port, uint8_t *data, size_t size) {
    if ((port < eUartPort_First) || (port >= eUartPort_Last) || (data == NULL) || (size == 0)) {
        return false;
    }

    for (size_t i = 0; i < size; i++) {
        UART_Driver_TransmitByte(port, data[i]);
    }

    return true;
}


bool UART_Driver_Read (eUartPort_t port, uint8_t *data) {
    if ((port < eUartPort_First) || (port >= eUartPort_Last) || (g_static_uart_rb[port] == NULL) || (data == NULL)) {
        return false;
    }

    return RingBufferDequeue(g_static_uart_rb[port], data);
}
