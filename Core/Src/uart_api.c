/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "cmsis_os.h"
#include "uart_api.h"
#include "uart_driver.h"
#include "heap_api.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define MESSAGE_QUEUE_SIZE 8
#define NO_MESSAGE_QUEUE_ATTRIBUTES NULL
#define MESSAGE_PRIORITY 0
#define MESSAGE_QUEUE_ATTR_BITS_DEFAULT 0
#define CB_MEM_DEFAULT NULL
#define CB_SIZE_DEFAULT 0
#define MQ_MEM_DEFAULT NULL
#define MQ_SIZE_DEFAULT 0

#define UART_API_TASK_STACK_SIZE 512

#define USART1_BUFFER_SIZE 64
#define USART2_BUFFER_SIZE 1024
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 typedef enum {
    eUartApiState_First = 0,
    eUartApiState_Initialize = eUartApiState_First,
    eUartApiState_Collect,
    eUartApiState_Flush,
    eUartApiState_Last
 } eUartApiState_t;

typedef struct {
    eUartApiPort_t uart_port;
    size_t buffer_size;
    osMessageQueueAttr_t queue_attributes;
    osMutexAttr_t mutex_attributes;
} eUartApiDesc_t;

typedef struct {
    osMessageQueueId_t message_queue_id;
    uint8_t *buffer;
    size_t index;
    uint8_t *delimiter;
    size_t delimiter_length;
    eUartApiState_t state;
    bool is_initialized;
    osMutexId_t mutex_id;
} sUartApiTaskData_t;
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
const static osThreadAttr_t g_uart_api_task_attr = {
    .name = "uartApiTask",
    .stack_size = UART_API_TASK_STACK_SIZE,
    .priority = (osPriority_t)osPriorityNormal
};

const static eUartApiDesc_t g_static_uart_api_lut[eUartApiPort_Last] = {
    [eUartApiPort_Usart1] = {
        .uart_port = eUartPort_Usart1,
        .buffer_size = USART1_BUFFER_SIZE,
        .queue_attributes = {
            .name = "USART1 Queue",
            .attr_bits = MESSAGE_QUEUE_ATTR_BITS_DEFAULT,
            .cb_mem = CB_MEM_DEFAULT,
            .cb_size = CB_SIZE_DEFAULT,
            .mq_mem = MQ_MEM_DEFAULT,
            .mq_size = MQ_SIZE_DEFAULT
        },
        .mutex_attributes = {
            .name = "USART1 Mutex",
            .attr_bits = osMutexRecursive,
            .cb_mem = CB_MEM_DEFAULT,
            .cb_size = CB_SIZE_DEFAULT
        }
    }
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static osThreadId_t g_uart_api_task_handle = NULL;
static sUartApiTaskData_t g_dynamic_uart_api_lut[eUartApiPort_Last] = {
    [eUartApiPort_Usart1] = {
        .message_queue_id = NULL,
        .mutex_id = NULL,
        .buffer = NULL,
        .index = 0,
        .delimiter = NULL,
        .delimiter_length = 0,
        .is_initialized = false,
        .state = eUartApiState_Initialize
    }
};
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
static void UART_API_Task (void *argument);
static bool UART_API_IsDelimiterReceived (sUartApiTaskData_t *task_data);
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void UART_API_Task (void *argument) {
    while (1) {
        for (eUartApiPort_t port = eUartApiPort_First; port < eUartApiPort_Last; port++) {
            if (g_dynamic_uart_api_lut[port].is_initialized == false) {
                continue;
            }

            switch (g_dynamic_uart_api_lut[port].state) {
                case eUartApiState_Initialize: {

                    g_dynamic_uart_api_lut[port].buffer = HEAP_API_Calloc(g_static_uart_api_lut[port].buffer_size, sizeof(char));

                    if (g_dynamic_uart_api_lut[port].buffer == NULL) {
//                        DEBUG_API_Print(g_static_uart_api_lut[port].uart_port, "Failure to allocate memory for a buffer\n");
                        continue;
                    }

                    g_dynamic_uart_api_lut[port].index = 0;
                    g_dynamic_uart_api_lut[port].state = eUartApiState_Collect;
                }
                case eUartApiState_Collect: {
                    uint8_t symbol;

                    while (UART_Driver_Read(port, &symbol) == true) {
                        g_dynamic_uart_api_lut[port].buffer[g_dynamic_uart_api_lut[port].index] = symbol;
                        g_dynamic_uart_api_lut[port].index++;

                        if (g_dynamic_uart_api_lut[port].index >= g_static_uart_api_lut[port].buffer_size) {
                            memset(g_dynamic_uart_api_lut[port].buffer, 0, g_static_uart_api_lut[port].buffer_size);
                            g_dynamic_uart_api_lut[port].index = 0;
//                            DEBUG_API_Print(g_static_uart_api_lut[port].uart_port, "Failed to find a delimiter before reaching max buffer size\n");
                            break;
                        }

                        if (symbol != g_dynamic_uart_api_lut[port].delimiter[g_dynamic_uart_api_lut[port].delimiter_length - 1]) {
                            continue;
                        }

                        if (UART_API_IsDelimiterReceived(&g_dynamic_uart_api_lut[port]) == true) {
                            g_dynamic_uart_api_lut[port].state = eUartApiState_Flush;
                        }
                    }

                    if (g_dynamic_uart_api_lut[port].state != eUartApiState_Flush) {
                        break;
                    }
                }
                case eUartApiState_Flush: {
                    sMessage_t message = {.message = g_dynamic_uart_api_lut[port].buffer, .message_length = g_dynamic_uart_api_lut[port].index};
                    osStatus_t status = osMessageQueuePut(g_dynamic_uart_api_lut[port].message_queue_id, &message, MESSAGE_PRIORITY, osWaitForever);

                    if (status != osOK) {
//                        DEBUG_API_Print(g_static_uart_api_lut[port].uart_port, "Failed to put message into the queue\n");
                        break;
                    }

                    g_dynamic_uart_api_lut[port].state = eUartApiState_Initialize;
                    break;
                }
                default: {
//                    DEBUG_API_Print(g_static_uart_api_lut[port].uart_port, "API Task state was not found\n");
                    break;
                }
            }
        }
    }
}

static bool UART_API_IsDelimiterReceived (sUartApiTaskData_t *task_data) {
    if ((task_data->buffer == NULL) || (task_data->index < task_data->delimiter_length) || (task_data->delimiter == NULL) || (task_data->delimiter_length <= 0)) {
        return false;
    }

    uint8_t *start_index = task_data->buffer + task_data->index - task_data->delimiter_length;

    return memcmp(start_index, task_data->delimiter, task_data->delimiter_length);
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool UART_API_Init (eUartApiPort_t port, uint32_t baudrate, uint8_t *delimiter, size_t delimiter_length) {
    if ((port < eUartApiPort_First) || (port >= eUartApiPort_Last)) {
        return false;
    }

    if (UART_Driver_Init(port, baudrate) == false) {
        return false;
    }

    if (delimiter_length <= 0) {
        return false;
    }

    g_dynamic_uart_api_lut[port].delimiter = delimiter;
    g_dynamic_uart_api_lut[port].delimiter_length = delimiter_length;

    g_dynamic_uart_api_lut[port].message_queue_id = osMessageQueueNew(MESSAGE_QUEUE_SIZE, sizeof(sMessage_t), &g_static_uart_api_lut[port].queue_attributes);
    if (g_dynamic_uart_api_lut[port].message_queue_id == NULL) {
        return false;
    }

    g_dynamic_uart_api_lut[port].mutex_id = osMutexNew(&g_static_uart_api_lut[port].mutex_attributes);
    if (g_dynamic_uart_api_lut[port].mutex_id == NULL) {
        return false;
    }

    if (g_uart_api_task_handle == NULL) {
        g_uart_api_task_handle = osThreadNew(UART_API_Task, NULL, &g_uart_api_task_attr);
        if (g_uart_api_task_handle == NULL) {
            return false;
        }
    }

    g_dynamic_uart_api_lut[port].is_initialized = true;
    return true;
}

bool UART_API_Send (eUartApiPort_t port, sMessage_t *message, uint32_t wait_time) {
    if ((port < eUartApiPort_First) || (port >= eUartApiPort_Last) || (message == NULL)) {
        return false;
    }

    if (g_dynamic_uart_api_lut[port].mutex_id == NULL) {
    	return false;
    }

    if (osMutexAcquire(g_dynamic_uart_api_lut[port].mutex_id, wait_time) != osOK) {
    	return false;
    }

    if (UART_Driver_TransmitMultipleBytes(port, message->message, message->message_length) == false) {
        osMutexRelease(g_dynamic_uart_api_lut[port].mutex_id);
    	return false;
    }

    if (osMutexRelease(g_dynamic_uart_api_lut[port].mutex_id) != osOK) {
    	return false;
    }

    return true;
}


bool UART_API_Receive (eUartApiPort_t port, sMessage_t *message, uint32_t wait_time) {
    if ((port < eUartApiPort_First) || (port >= eUartApiPort_Last) || (message == NULL) || (wait_time <= 0)) {
        return false;
    }

    osStatus_t status = osMessageQueueGet(g_dynamic_uart_api_lut[port].message_queue_id, message, MESSAGE_PRIORITY, wait_time);
    if (status != osOK) {
        return false;
    }

    return true;
}
