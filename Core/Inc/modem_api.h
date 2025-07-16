#ifndef INC_MODEM_API_H_
#define INC_MODEM_API_H_

#include <stdbool.h>
#include "message.h"

bool MODEM_API_Init (void);
bool MODEM_API_AddCall (sMessage_t *phone_number);

#endif /* INC_MODEM_API_H_ */
