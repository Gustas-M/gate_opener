#include <math.h>
#include "cmsis_os.h"
#include "i2c_driver.h"
#include "modem_api.h"
#include "gps_app.h"

#define GPS_APP_TASK_STACK_SIZE 512
#define NO_THREAD_ARGUMENTS NULL

#define SYNC_CHAR1 0
#define SYNC_CHAR2 1
#define DEFAULT_SYNC1_VALUE 0xb5
#define DEFAULT_SYNC2_VALUE 0x62
#define UBX_CLASS 2
#define UBX_ID 3
#define DEFAULT_UBX_CLASS_VALUE 0x01
#define DEFAULT_UBX_ID_VALUE 0x07

#define LON_BUFF_INDEX 24
#define LAT_BUFF_INDEX 28

#define GPS_BUFF_SIZE 100
#define GPS_DATA_COUNT 1

#define EARTH_RADIUS_KM 6371.0

const static uint8_t ubx_nav_pvt[] = {
    0xB5, 0x62,     // UBX sync chars
    0x01, 0x07,     // Class = NAV, ID = PVT
    0x00, 0x00,     // Payload length = 0
    0x08, 0x19      // Checksum (CK_A, CK_B)
};

static sGpsData_t gps_data[GPS_DATA_COUNT] = {
	{
		.latitude = 0.0,
		.longitude = 0.0,
		.phone_number = "+37000000000",
		.inside_area = false,
	}
};

const static osThreadAttr_t g_static_gps_app_task_attr = {
    .name = "Gps App Task",
    .stack_size = GPS_APP_TASK_STACK_SIZE,
    .priority = (osPriority_t)osPriorityNormal
};

static osThreadId_t g_static_gps_app_task = NULL;

static bool UBX_MSG_Check (uint8_t *gps_buff);
static int32_t CoordinatesConversion (uint8_t *gps_buff);
static double DegreesToRadians(double degrees);
static double CalculateDistance(double lat1, double lon1, double lat2, double lon2);

static void GPS_APP_Task (void *argument) {
	uint8_t gps_buff[GPS_BUFF_SIZE];
	while (1) {
		I2C_Driver_Write(eI2cPort_Gps, (uint8_t*)ubx_nav_pvt, sizeof(ubx_nav_pvt));
		osDelay(100);
		I2C_Driver_Read(eI2cPort_Gps, gps_buff, GPS_BUFF_SIZE);

		if (!UBX_MSG_Check(gps_buff)) {
			continue;
		}

		double lon_deg = CoordinatesConversion(&gps_buff[LON_BUFF_INDEX]);
		double lat_deg = CoordinatesConversion(&gps_buff[LAT_BUFF_INDEX]);

		for (uint8_t i = 0; i < GPS_DATA_COUNT; i++) {
			double distance = CalculateDistance(lat_deg, lon_deg, gps_data[i].latitude, gps_data[i].longitude);
			if (!gps_data[i].inside_area) {
				if (distance <= 200) {
					sMessage_t call_message = {.message = (uint8_t*)gps_data[i].phone_number, sizeof(gps_data[i].phone_number)};
					gps_data[i].inside_area = true;
					MODEM_API_AddCall(&call_message);
				}
			} else {
				if (distance > 200) {
					gps_data[i].inside_area = false;
				}
			}
		}

		osDelay(500);
	}
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

static bool UBX_MSG_Check (uint8_t *gps_buff) {
	if ((gps_buff[SYNC_CHAR1] != DEFAULT_SYNC1_VALUE) && (gps_buff[SYNC_CHAR2] != DEFAULT_SYNC2_VALUE)) {
		return false;
	}

	if ((gps_buff[UBX_CLASS] != DEFAULT_UBX_CLASS_VALUE) && (gps_buff[UBX_ID] != DEFAULT_UBX_ID_VALUE)) {
		return false;
	}

	return true;
}

static int32_t CoordinatesConversion (uint8_t *gps_buff) {
	int32_t coordinate = (int32_t)(
	    ((uint32_t)gps_buff)       |
	    ((uint32_t)(gps_buff + 1) << 8)  |
	    ((uint32_t)(gps_buff + 2) << 16) |
	    ((uint32_t)(gps_buff + 3) << 24)
	);

	return coordinate / 1e7;
}

static double DegreesToRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

static double CalculateDistance(double lat1, double lon1, double lat2, double lon2) {
    double dLat = DegreesToRadians(lat2 - lat1);
    double dLon = DegreesToRadians(lon2 - lon1);

    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(DegreesToRadians(lat1)) * cos(DegreesToRadians(lat2)) *
               sin(dLon / 2) * sin(dLon / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RADIUS_KM * c;
}
