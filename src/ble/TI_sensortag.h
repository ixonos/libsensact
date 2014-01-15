#ifndef TI_SENSORTAG_H
#define TI_SENSORTAG_H

#include <stdbool.h>

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

//--------------------------------------------------------------------------------------------------
// TI specific UUID's:

// TI-specific base UIID (128 bit):
//# define TI_BASE_UUID                   0xF000xxxx04514000B000000000000000

// TI-specific sensor-UUID's (16 bit); so to create 128-bit UUID, replace 'xxxx' above with 16-bit value below:

#define IR_TEMPERATURE_DATA_UUID        0xaa01 // 0xf000aa0104514000b000000000000000
#define IR_TEMPERATURE_CONFIG_UUID      0xaa02 // 0xf000aa0204514000b000000000000000


//*************************************************************************************************************************

// TI specific attribute handles
#define TI_DEVICE_NAME_HND              0x03
#define TI_TEMP_RD_HND                  0x25
#define TI_TEMP_WR_HND                  0x29


typedef int (*Msg_Hdl) (unsigned char *buff, int length, void *value);

// GATT-attribute characteristic
typedef const struct CHAR_DEF
{
    char feature[7];
    bool data_config; // 0=data reading, 1=configuration (enable/disable)
    unsigned int UUID;

    unsigned char property;
    unsigned int handle;
    Msg_Hdl *msg_hdl;  // message handler

} char_dev;


int TI_sensortag_temperature(unsigned char *buff, int length, void *value);
int find_att_handle(/*session_manuf,*/ char *feature, bool data_config, unsigned int uuid, unsigned int *hnd, unsigned char *property, Msg_Hdl *handler);

#endif
