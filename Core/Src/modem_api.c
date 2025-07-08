#include "modem_api.h"
#include "cmsis_os.h"
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

static void MODEM_API_MakeCall (uint8_t *phone_number) {
	g_dynamic_modem_api.isBusy = true;

	//calling

	g_dynamic_modem_api.isBusy = false;
}

static void MODEM_API_Task (void) {
	sMessage_t message = {0};
	while (1) {
		if (g_dynamic_modem_api.isBusy) {
			continue;
		}

		if (osMessageQueueGet(g_dynamic_modem_api.message_queue_id, NULL, MESSAGE_PRIORITY, osWaitForever) != osOK) {
			//number received, make a call
	    }
	}

	HEAP_API_Free(message.message);

}

bool MODEM_API_Init (void) {
	g_dynamic_modem_api.message_queue_id = osMessageQueueNew(MESSAGE_QUEUE_SIZE, sizeof(sMessage_t), &g_static_modem_api.queue_attributes);
    if (g_dynamic_modem_api.message_queue_id == NULL) {
        return false;
    }

    g_dynamic_modem_api.mutex_id = osMutexNew(&g_static_modem_api.mutex_attributes);
    if (g_dynamic_modem_api.mutex_id == NULL) {
        return false;
    }

    if (g_modem_api_task_handle == NULL) {
    	g_modem_api_task_handle = osThreadNew(NULL, NULL, &g_modem_api_task_attr);
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
