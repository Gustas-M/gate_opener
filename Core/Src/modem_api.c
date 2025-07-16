#include "cmsis_os.h"
#include "uart_api.h"
#include "modem_api.h"
#include "heap_api.h"
#include "message.h"

#define MODEM_API_TASK_STACK_SIZE 512

#define MESSAGE_QUEUE_SIZE 8
#define NO_MESSAGE_QUEUE_ATTRIBUTES NULL
#define MESSAGE_PRIORITY 0
#define MESSAGE_QUEUE_ATTR_BITS_DEFAULT 0
#define CB_MEM_DEFAULT NULL
#define CB_SIZE_DEFAULT 0
#define MQ_MEM_DEFAULT NULL
#define MQ_SIZE_DEFAULT 0

#define CALL_MUTEX_WAIT_TIME 10000

#define PHONE_NUMBER_LENGTH 12
#define CALL_COMMAND_LENGTH 22

typedef struct {
    osMessageQueueAttr_t queue_attributes;
    osMutexAttr_t mutex_attributes;
} sModemApiDesc_t;

typedef struct {
    osMessageQueueId_t message_queue_id;
    osMutexId_t mutex_id;
    bool isBusy;
} sModemApiTaskData_t;

const static osThreadAttr_t g_modem_api_task_attr = {
    .name = "modemApiTask",
    .stack_size = MODEM_API_TASK_STACK_SIZE,
    .priority = (osPriority_t)osPriorityNormal
};

static osThreadId_t g_modem_api_task_handle = NULL;

const static sModemApiDesc_t g_static_modem_api = {
	.queue_attributes = {
		.name = "Modem Queue",
		.attr_bits = MESSAGE_QUEUE_ATTR_BITS_DEFAULT,
		.cb_mem = CB_MEM_DEFAULT,
		.cb_size = CB_SIZE_DEFAULT,
		.mq_mem = MQ_MEM_DEFAULT,
		.mq_size = MQ_SIZE_DEFAULT
	},
	.mutex_attributes = {
		.name = "Modem Mutex",
		.attr_bits = osMutexRecursive,
		.cb_mem = CB_MEM_DEFAULT,
		.cb_size = CB_SIZE_DEFAULT
	}
};

sModemApiTaskData_t g_dynamic_modem_api = {
		.message_queue_id = NULL,
		.mutex_id = NULL,
		.isBusy = false
};

static bool MODEM_API_Check (void);
static bool MODEM_API_MakeCall (uint8_t *phone_number);

static void MODEM_API_Task (void *arguments) {
	sMessage_t message = {0};
	while (1) {
		if (g_dynamic_modem_api.isBusy) {
			continue;
		}

		if (osMessageQueueGet(g_dynamic_modem_api.message_queue_id, &message, MESSAGE_PRIORITY, osWaitForever) != osOK) {
			g_dynamic_modem_api.isBusy = true;
			MODEM_API_MakeCall(message.message);
	    }
	}

	HEAP_API_Free(message.message);

}

bool MODEM_API_Init (void) {
	if (!MODEM_API_Check()) {
		return false;
	}

	g_dynamic_modem_api.message_queue_id = osMessageQueueNew(MESSAGE_QUEUE_SIZE, sizeof(sMessage_t), &g_static_modem_api.queue_attributes);
    if (g_dynamic_modem_api.message_queue_id == NULL) {
        return false;
    }

    g_dynamic_modem_api.mutex_id = osMutexNew(&g_static_modem_api.mutex_attributes);
    if (g_dynamic_modem_api.mutex_id == NULL) {
        return false;
    }

    if (g_modem_api_task_handle == NULL) {
    	g_modem_api_task_handle = osThreadNew(MODEM_API_Task, NULL, &g_modem_api_task_attr);
        if (g_modem_api_task_handle == NULL) {
            return false;
        }
    }

    return true;
}

bool MODEM_API_AddCall (sMessage_t *phone_number) {
	if (phone_number == NULL) {
		return false;
	}

	if (osMessageQueuePut(g_dynamic_modem_api.message_queue_id, phone_number, MESSAGE_PRIORITY, osWaitForever) != osOK) {
		return false;
	}

	return true;
}

static bool MODEM_API_Check (void) {
	sMessage_t okMessage = {.message = (uint8_t*)"AT\r\n", .message_length = sizeof("AT\r\n")};
	UART_API_Send(eUartApiPort_ModemUsart, &okMessage, osWaitForever);
	osDelay(50);

	sMessage_t response = {0};
	UART_API_Receive(eUartApiPort_ModemUsart, &response, 250);
	if (strncmp((const char*)response.message, (const char*)okMessage.message, okMessage.message_length)) {
		return false;
	}

	return true;
}

static bool MODEM_API_MakeCall (uint8_t *phone_number) {
	if (osMutexAcquire(g_dynamic_modem_api.mutex_id, CALL_MUTEX_WAIT_TIME) != osOK) {
		return false;
	}

	uint8_t callCommand[CALL_COMMAND_LENGTH] = "ATD";
	strncat((char*)callCommand, (char*)phone_number, PHONE_NUMBER_LENGTH);
	strncat((char*)callCommand, (char*)";\r\n", 4);
	sMessage_t callMessage = {.message = callCommand, .message_length = sizeof(callCommand)};
	UART_API_Send(eUartApiPort_ModemUsart, &callMessage, osWaitForever);
	g_dynamic_modem_api.isBusy = false;

	osMutexRelease(g_dynamic_modem_api.mutex_id);
	return true;
}
