#ifndef TI_SENSORTAG_H
#define TI_SENSORTAG_H

#include <stdbool.h>

#define PI_VAL 3.14159265358979323846
#define DEGREES_PER_RADIAN  (180.0/PI_VAL)

// properties of the attribute
enum
{
    ATT_BROADCAST =     0x01, // broadcast
    ATT_RD =            0x02, // read property
    ATT_WR_NO_RSP =     0x04, // write with no response
    ATT_WR_RSP =        0x08, // write with response
    ATT_NOTIFY =        0x10, // notification
    ATT_IND =           0x20, // indicate
    ATT_AUTH_SIGNED =   0x40, // authenticatedSignedWrites
    ATT_EXT_PROP =      0x80 // extendedProperties
};

// features
enum
{
    SENSACT_DEVICE_NAME = 1, // device name
    SENSACT_TEMP // temperature
};

enum
{
    READ_SENSOR = 0,
    ENABLE_SENSOR = 1
};

#define GATT_PRIM_SVC_UUID          0x2800
#define GATT_INCLUDE_UUID           0x2802
#define GATT_CHARAC_UUID            0x2803

//*************************************************************************************************************************

// The Bluetooth Base 128 bit UUID is defined that all 16-bit Attribute UUIDs use as a base. The Bluetooth_Base_UUID is:
// 0x0000xxxx00001000800000805F9B34FB

// Bluetooth Base-UUID's (16 bit); so to create 128-bit UUID, replace 'xxxx' above with 16-bit value below:
#define DEVICE_NAME_UUID                0x2a00

#define DEVINFO_FIRMWARE_REV_UUID       0x2A26
#define DEVINFO_FIRMWARE_REV_HND        0x18

#define DEVINFO_SOFTWARE_REV_UUID       0x2A28
#define DEVINFO_SOFTWARE_REV_HND        0x1C

//--------------------------------------------------------------------------------------------------
// TI specific UUID's:

// TI-specific base UIID (128 bit):
//# define TI_BASE_UUID                   0xF000xxxx04514000B000000000000000

// TI-specific sensor-UUID's (16 bit); so to create 128-bit UUID, replace 'xxxx' above with 16-bit value below:

// temperature
#define IR_TEMPERATURE_DATA_UUID        0xaa01 // 0xf000aa0104514000b000000000000000
#define IR_TEMPERATURE_CONFIG_UUID      0xaa02 // 0xf000aa0204514000b000000000000000

// humidity
#define TI_HUMID_DATA_UUID              0xaa21 // 0xf000aa2104514000b000000000000000
#define TI_HUMID_CONF_UUID              0xaa22 // 0xf000aa2204514000b000000000000000

// gyroscope
#define TI_GYRO_DATA_UUID              0xaa51
#define TI_GYRO_CONF_UUID              0xaa52

// magnetometer
#define TI_MAGN_DATA_UUID              0xaa31
#define TI_MAGN_CONF_UUID              0xaa32

// accelerometer
#define TI_ACCEL_DATA_UUID              0xaa11
#define TI_ACCEL_CONF_UUID              0xaa12

//*************************************************************************************************************************

// TI specific attribute handles

// temperature
#define TI_DEVICE_NAME_HND              0x03
#define TI_TEMP_RD_HND                  0x25
#define TI_TEMP_WR_HND                  0x29

// humidity
#define TI_HUMID_DATA_HND               0x38 // read data
#define TI_HUMID_NOT_HND                0x39 // notification
#define TI_HUMID_CONF_HND               0x3C // enable/disable sensor

// gyroscope
#define TI_GYRO_DATA_HND               0x57 //0x60 // read data
#define TI_GYRO_NOT_HND                0x58 //0x61 // notification
#define TI_GYRO_CONF_HND               0x5B //0x64 // enable/disable sensor

// magnetometer
#define TI_MAGN_DATA_HND               0x40 //0x46 // read data
#define TI_MAGN_NOT_HND                0x41 //0x47 // notification
#define TI_MAGN_CONF_HND               0x44 //0x4A // enable/disable sensor
#define TI_MAGN_PERIOD_HND             0x47 //0x4D // measurement period

// accelerometer
#define TI_ACCEL_DATA_HND               0x2D // read data
#define TI_ACCEL_NOT_HND                0x2E // notification
#define TI_ACCEL_CONF_HND               0x31 // enable/disable sensor
//#define TI_ACCEL_PERIOD_HND               0x34 // measurement period: [Input*10]ms


typedef int (*Msg_Hdl) (unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp);

// GATT-attribute characteristic
typedef const struct CHAR_DEF
{
    char feature[10];
    bool data_config; // 0=data reading, 1=configuration (enable/disable)
    unsigned int UUID;

    unsigned char property;
    unsigned int handle;
    Msg_Hdl *msg_hdl;  // message handler

} char_dev;


int TI_sensortag_temperature(unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp);
int TI_sensortag_humidity(unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp);
int TI_sensortag_gyroscope(unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp);
int TI_sensortag_magnetometer(unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp);
int TI_sensortag_accelerometer(unsigned char *buff, int length, void *value1, void *value2, void *value3, double *time_stamp);

int find_att_handle(/*session_manuf,*/ char *feature, bool data_config, unsigned int uuid, unsigned int *hnd, unsigned char *property, Msg_Hdl *handler);

#endif
