/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "ring_buffer.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
struct sRingBuffer_t {
    uint8_t *buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
};
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
sRingBuffer_t *RingBufferInit (size_t capacity) {
    if (capacity <= 0) {
        return NULL;
    }

    sRingBuffer_t *r_buffer = (sRingBuffer_t *)calloc(1, sizeof(sRingBuffer_t));
    if (r_buffer == NULL) {
        return NULL;
    }

    r_buffer->buffer = (uint8_t *)calloc(capacity, sizeof(uint8_t));
    if (r_buffer->buffer == NULL) {
        free(r_buffer);
        return NULL;
    }

    r_buffer->capacity = capacity;
    r_buffer->head = 0;
    r_buffer->tail = 0;
    r_buffer->count = 0;

    return r_buffer;
}


bool RingBufferIsEmpty (sRingBuffer_t *r_buffer) {
    if (r_buffer == NULL) {
        return true;
    }

    return r_buffer->count == 0;
}

bool RingBufferIsFull (sRingBuffer_t *r_buffer) {
    if (r_buffer == NULL) {
        return true;
    }

    return r_buffer->count == r_buffer->capacity;
}


bool RingBufferEnqueue (sRingBuffer_t *r_buffer, uint8_t data) {
    if (r_buffer == NULL) {
        return false;
    }

    if (RingBufferIsFull(r_buffer)) {
        r_buffer->tail = (r_buffer->tail + 1) % r_buffer->capacity;
    }

    r_buffer->buffer[r_buffer->head] = data;
    r_buffer->head = (r_buffer->head + 1) % r_buffer->capacity;

    if (r_buffer->count < r_buffer->capacity) {
        r_buffer->count++;
    }

    return true;
}

bool RingBufferDequeue (sRingBuffer_t *r_buffer, uint8_t *data) {
    if ((RingBufferIsEmpty(r_buffer)) || (data == NULL)) {
        return false;
    }

    *data = r_buffer->buffer[r_buffer->tail];
    r_buffer->tail = (r_buffer->tail + 1) % r_buffer->capacity;
    r_buffer->count--;

    return true;
}

bool RingBufferFree (sRingBuffer_t *r_buffer) {
    if (r_buffer == NULL) {
        return false;
    }

    free(r_buffer->buffer);
    free(r_buffer);

    return true;
}
