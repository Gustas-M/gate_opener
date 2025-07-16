#ifndef INC_GPS_APP_H_
#define INC_GPS_APP_H_

typedef struct {
	double latitude;
	double longitude;
	char *phone_number;
	bool inside_area;
} sGpsData_t;

bool GPS_APP_Init (void);

#endif /* INC_GPS_APP_H_ */
