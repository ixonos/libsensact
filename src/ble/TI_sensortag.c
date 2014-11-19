#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "TI_sensortag.h"
#include "ble.h"
#include "gatt_att.h"

// magnetometer-variables are in micro teslas (notice: scale for magnetic field of the earth: ~20- ~65 uT)
float max_magnetics_field = 0.0;
float min_magnetics_field = 150.0;

// gyroscope-variables
double gyroZangle = 0;
double first_time_stamp = 0, new_time_stamp = 0, old_time_stamp = 0;
double gyroRateValue0 = 0; // 0 Value (latest measurement)
double gyroRateValue1 = 0; //-1 Value (second latest measurement) for Runge-Kutta integration
double gyroRateValue2 = 0; //-2 Value (third latest measurement) for Runge-Kutta integration
double gyroRateValue3 = 0; //-3 Value (fourth latest measurement) for Runge-Kutta integration

// characteristic table for TI-sensortag:
// contains symbolic feature (e.g. 'magnetometer'), BLE UUID (data/configuration) and address of the response-message handler
static const struct CHAR_DEF TI_sensortag_table[]=
{
    {   {'T','e','m','p',0},                     ENABLE_SENSOR,  IR_TEMPERATURE_CONFIG_UUID,    NULL},
    {   {'T','e','m','p',0},                     READ_SENSOR,    IR_TEMPERATURE_DATA_UUID,      ((Msg_Hdl*) &TI_sensortag_temperature)},
    {   {'h','u','m','i','d','i','t','y',0},     ENABLE_SENSOR,  TI_HUMID_CONF_UUID,            NULL},
    {   {'h','u','m','i','d','i','t','y',0},     READ_SENSOR,    TI_HUMID_DATA_UUID,            ((Msg_Hdl*) &TI_sensortag_humidity)},
    {   {'g','y','r','o','s','c','o','p','e',0}, ENABLE_SENSOR,  TI_GYRO_CONF_UUID,             NULL},
    {   {'g','y','r','o','s','c','o','p','e',0}, READ_SENSOR,    TI_GYRO_DATA_UUID,             ((Msg_Hdl*) &TI_sensortag_gyroscope)},
    {   {'m','a','g','n','e','t','o','m',0},     ENABLE_SENSOR,  TI_MAGN_CONF_UUID,             NULL},
    {   {'m','a','g','n','e','t','o','m',0},     READ_SENSOR,    TI_MAGN_DATA_UUID,             ((Msg_Hdl*) &TI_sensortag_magnetometer)},
    {   {'a','c','c','e','l','e','r','o',0},     ENABLE_SENSOR,  TI_ACCEL_CONF_UUID,            NULL},
    {   {'a','c','c','e','l','e','r','o',0},     READ_SENSOR,    TI_ACCEL_DATA_UUID,            ((Msg_Hdl*) &TI_sensortag_accelerometer)},

    {   {'H','W','r','e','v',0},                 READ_SENSOR,    DEVINFO_FIRMWARE_REV_UUID,     NULL},
    {   {'S','W','r','e','v',0},                 READ_SENSOR,    DEVINFO_SOFTWARE_REV_UUID,     NULL},
    {   {0},                                     0,              0,                              0  }
} ;

int find_att_handle(/*session_manuf,*/ char *feature, bool data_config, struct characteristics_table_t *char_ptr,
		    unsigned int *hnd, unsigned char *property, Msg_Hdl *handler)
{
    // find handle of the attribute by symbolic feature (e.g. 'temp')
    // and return handle and property of this attribute

    struct CHAR_DEF const * sensor_char_ptr;
    struct dynamic_char_t *dyn_char_ptr = &char_ptr->dyn_attr[0];
    int i, j, nbr_sensors;

    nbr_sensors = sizeof(TI_sensortag_table)/sizeof(TI_sensortag_table[0]);

    for (i = 0, sensor_char_ptr = TI_sensortag_table; i < nbr_sensors; i++, sensor_char_ptr++)
    {
	if ( (strcmp(sensor_char_ptr->feature, feature) == 0) && (sensor_char_ptr->data_config == data_config) ) // found
	{
	    for (j = 0; j < char_ptr->nbr_attr; j++, dyn_char_ptr++)
	    {
	        if (sensor_char_ptr->UUID == dyn_char_ptr->UUID) {
		    *hnd = dyn_char_ptr->handle; // this is the handle used for accessing the needed sensor !
		    //printf("sensor handle: 0x%x\n", dyn_char_ptr->handle);
		    break;
		}
	    }

	    if (handler != NULL)
	        *handler = (Msg_Hdl) sensor_char_ptr->msg_hdl;

	    return 0;
	}
    }

    return -1; // handle not found for this manufacturer
}

float RadiansToDegrees(float rads)
{

  // Correct for when signs are reversed.
/*  if(rads < 0)
    rads += 2*3.14;

  // Check for wrap due to addition of declination.
  else if(rads > 2*3.14)
    rads -= 2*3.14;
*/
  // Convert radians to degrees
  float angle = DEGREES_PER_RADIAN * rads;

  return angle;
}

// see e.g. 'http://processors.wiki.ti.com/index.php/SensorTag_User_Guide#Gatt_Server' for sensor-values handling:


double extractTargetTemperature(unsigned char *buff, double ambient) 
{
    double Vobj2 = 0;

    // Little endian- formatted
    Vobj2= (double) convert_int16_le(buff, 0);

    Vobj2 *= 0.00000015625; 
    double Tdie = ambient + 273.15;
    double S0 = 6.4E-14; //5.593E-14;	// Calibration factor
    double a1 = 1.75E-3;
    double a2 = -1.678E-5;
    double b0 = -2.94E-5;
    double b1 = -5.7E-7;
    double b2 = 4.63E-9;
    double c2 = 13.4;
    double Tref = 298.15;
    double S = S0*(1+a1*(Tdie - Tref)+a2*pow((Tdie - Tref), 2));
    double Vos = b0 + b1*(Tdie - Tref) + b2*pow((Tdie - Tref), 2);
    double fObj = (Vobj2 - Vos) + c2*pow((Vobj2 - Vos), 2);
    double tObj = pow(pow(Tdie,4) + (fObj/S), .25);
    tObj -= 273.15;

/*
printf("Vobj2 = %.*f\n", 18, Vobj2);
printf("Tdie = %.*f\n", 18, Tdie);
printf("S0 = %.*f\n", 18, S0);
printf("a1 = %.*f\n", 18, a1);
printf("a2 = %.*f\n", 18, a2);
printf("b0 = %.*f\n", 18, b0);
printf("b1 = %.*f\n", 18, b1);
printf("b2 = %.*f\n", 18, b2);
printf("c2 = %.*f\n", 18, c2);
printf("Tref = %.*f\n", 18, Tref);
printf("S = %.*f\n", 18, S);
printf("Vos = %.*f\n", 18, Vos);
printf("fObj = %.*f\n", 18, fObj);
printf("tObj = %.*f\n", 18, tObj);*/

    return tObj;
}

int TI_sensortag_temperature(unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp)
{
    double ambient = 0;
    double target = 0;

    if (length != 4)  // four bytes needed
	return -1;

    // Little endian- formatted
    ambient= (double) convert_int16_le(buff, 2) / 128.0;

    target = extractTargetTemperature(buff, ambient);

    //printf("Temperature: ambient: %lf [°C], target: %lf [°C], time: %f [sec]\n", ambient, target, *time_stamp);

    if (value1 != NULL)
        memcpy(value1, &target, sizeof(double));

    return 0;  // return later also ambient !
}


int TI_sensortag_humidity(unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp)
{
    double temperature = 0;
    double humidity = 0;

    if (length != 4)  // four bytes needed
	return -1;

    temperature = -46.85 + 175.72 / 65536.0 * (double)(convert_int16_le(buff, 0));

    // relative humidity
    humidity = -6.0 + 125.0 / 65536.0 * (double)((convert_int16_le(buff, 2) & ~0x0003));

    //printf("Humidity: %lf, Temperature: %lf [°C]\n", humidity, temperature);

    if (value1 != NULL)
        memcpy(value1, &humidity, sizeof(double));

    return 0;  // return later also temperature ?
}

int TI_sensortag_gyroscope(unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp)
{
    double delta_time = 0; // integration interval in fraction of seconds (it's in seconds, because gyro-rate is degrees/seconds)

    if (length != 6)  // six bytes needed
        return -1;

    new_time_stamp = *time_stamp;

    // Converting from raw data to degrees/second
    double x_rate = ( (double)(convert_int16_le(buff, 0)) ) * 500.0 / 65536.0;
    double y_rate = ( (double)(convert_int16_le(buff, 2)) ) * 500.0 / 65536.0;
    double z_rate = ( (double)(convert_int16_le(buff, 4)) ) * 500.0 / 65536.0;

    z_rate -= -0.07; //0.6; // this is experimental offset for decreasing zero point-drift; unfortunately varies in different TI-sensortags :(

    //printf("x_rate: %lf, y_rate: %lf, z_rate: %lf, time: %lf\n", x_rate, y_rate, z_rate, new_time_stamp);

    gyroRateValue0 = z_rate;

/*    if (abs(gyroRateValue0 - gyroRateValue1) >= 200.0)
    {
	gyroRateValue0 = 0; // filter too big changes
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    else*/
	gyroRateValue1 = gyroRateValue0;

    if (!first_time_stamp)
    {
        // this is first measurement, start calculating angles just after having two measurements
        gyroZangle = 0;
	old_time_stamp = new_time_stamp;
	first_time_stamp = new_time_stamp;
	gyroRateValue1 = gyroRateValue0;
	gyroRateValue2 = 0;
	gyroRateValue3 = 0;
    }
    else
    {
	delta_time = new_time_stamp - old_time_stamp; // time difference between two sequential measurement

//	printf("old time: %lf, new time: %lf, time diff: %lf [sec]\n", old_time_stamp, new_time_stamp, delta_time);

	old_time_stamp = new_time_stamp;

	// Integrate gyroscope rotation rate (angular speed) over the the time:
	// change of the angular speed integrated by the time is change of the angle.
	// Integrate by Runge-Gutta method:
	gyroZangle += (gyroRateValue3 + 2 * gyroRateValue2 + 2 * gyroRateValue1 + gyroRateValue0) * delta_time / 6;
	// gyroZangle += gyroRateValue0 * delta_time; // simple integration

//	printf("gyroZangle: %lf\n\n", gyroZangle);

	if (gyroZangle >= 360.0)
	    gyroZangle -= 360.0;
	else if (gyroZangle <= -360.0)
	  gyroZangle += 360.0;

	// update earlier measurements according to the new measurement
	gyroRateValue3 = gyroRateValue2;
	gyroRateValue2 = gyroRateValue1;
	gyroRateValue1 = gyroRateValue0;
    }

    if (value1 != NULL)
        memcpy(value1, &gyroZangle, sizeof(double));

    return 0;
}

int TI_sensortag_magnetometer(unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp)
{
    double yaw = 0;

    if (length != 6)  // six bytes needed
        return -1;

    // Calculate magnetic-field strength from the raw data (unit: micro-tesla, range: -1000/+1000)
    float xAxis = (convert_int16_le(buff, 0)) * -2000.0 / 65536.0; // uT
    float yAxis = (convert_int16_le(buff, 2)) * 2000.0 / 65536.0; // uT
    float zAxis = (convert_int16_le(buff, 4)) * 2000.0 / 65536.0; // uT

    //printf("xAxis: %f [uT], yAxis: %f [uT], zAxis: %f [uT], time: %f [sec]\n", xAxis, yAxis, zAxis, *time_stamp);

    if (xAxis >= max_magnetics_field)
        max_magnetics_field = xAxis;

    if (xAxis <= min_magnetics_field && xAxis > 0)
        min_magnetics_field = xAxis;

    //printf("max_magnetics_field: %f [uT], min_magnetics_field: %f [uT]\n", max_magnetics_field, min_magnetics_field);

    if ( (max_magnetics_field > min_magnetics_field) && (xAxis > min_magnetics_field) && (xAxis < max_magnetics_field) )
    {
        yaw = ( (xAxis - min_magnetics_field) / (max_magnetics_field - min_magnetics_field) ) * 180.0;
    }

//    yaw = RadiansToDegrees(atan2(zAxis, xAxis));
    //printf("The yaw: %lf°\n", yaw);

    if (value1 != NULL)
        memcpy(value1, &yaw, sizeof(double));

    return 0;
}

int TI_sensortag_accelerometer(unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp)
{
    if (length != 3)  // three bytes needed
        return -1;

   /*  See http://processors.wiki.ti.com/index.php/SensorTag_User_Guide#Gatt_Server:
     *
     * The accelerometer has the range [-2g, 2g] with unit (1/64)g.
     *
     * To convert from unit (1/64)g to unit g we divide by 64.
     *
     * (g = 9.81 m/s^2)
     *
     * */

    // Lets gather accelerometer-sensor data and calculate pitch- and roll-angles based on acceleration.
    // The sensor naturally detects all kind of acceleration, but for pitch and roll only gravity acceleration
    // can be used. So this calculation is reliable only, if there are no quick movements of the sensortag.
    // Notice: yaw-angle (rotation over z-axis) cannot be measured by accelerometer-sensor,
    // because x-axis and y-axis sensors measure zero-gravity, if sensor is in orthogonal position to the gravity-vector...

    // raw accelerometer-data for different gravity-components (= corresponding to x/y/z-directions).
    float xAxis = (float)( (int8_t)buff[0] ) / 64 * 1;
    float yAxis = (float)( (int8_t)buff[1] ) / 64 * 1;
    float zAxis = (float)( (int8_t)buff[2] ) / 64 * 1;

    //printf("x: %f [g], y: %f [g], z: %f [g], time: %f [sec]\n", xAxis, yAxis, zAxis, *time_stamp);

    double pitch = RadiansToDegrees(atan2(yAxis, sqrt( pow(xAxis,2) + pow(zAxis,2) ) ) ); // pitch is rolling-angle over y-axis

    // pitch in 0...360°
/*    if ( (yAxis > 0 && zAxis < 0) || (yAxis < 0 && zAxis < 0) )
      pitch = 180 - pitch;
    else if (yAxis < 0 && zAxis > 0)
      pitch = 360 + pitch;
*/
    double roll = RadiansToDegrees(atan2(xAxis, sqrt( pow(yAxis,2) + pow(zAxis,2) ) ) ); // roll is rolling-angle over x-axis
    roll *= -1; // for rotating in UI 'correctly'
    // roll in 0...360°
/*    if ( (xAxis > 0 && zAxis < 0) || (xAxis < 0 && zAxis < 0) )
      roll = 180 - roll;
    else if (xAxis < 0 && zAxis > 0)
      roll = 360 + roll;
*/
    //printf("pitch: %lf°, roll: %lf°\n", pitch, roll);

    if (value1 != NULL)
        memcpy(value1, &pitch, sizeof(double));

    if (value2 != NULL)
        memcpy(value2, &roll, sizeof(double));

    return 0;
}
