#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <math.h>

#include "TI_sensortag.h"
#include "ble.h"
#include "gatt_att.h"

// characteristic table
static const struct CHAR_DEF char_table[]=
{
    {   {'D','e','v',0},      READ_SENSOR, DEVICE_NAME_UUID,             ATT_RD,                 TI_DEVICE_NAME_HND,   NULL},
    {   {'T','e','m','p',0},  ENABLE_SENSOR, IR_TEMPERATURE_CONFIG_UUID, ATT_WR_NO_RSP,          TI_TEMP_WR_HND,       NULL},
    {   {'T','e','m','p',0},  READ_SENSOR, IR_TEMPERATURE_DATA_UUID,     ATT_RD,                 TI_TEMP_RD_HND,       ((Msg_Hdl*) &TI_sensortag_temperature)},
    {   {0},                  0,           0,                            0,                      0,                    0}
} ;

int find_att_handle(/*session_manuf,*/ char *feature, bool data_config, unsigned int uuid, 
		    unsigned int *hnd, unsigned char *property, Msg_Hdl *handler)
{
    // find handle of the attribute by symbolic feature (e.g. 'temp')
    // and return handle and property of this attribute

    struct CHAR_DEF const * char_ptr;

    for (char_ptr = char_table; sizeof(char_table)/sizeof(char_table[0]); char_ptr++)
    {
	if ( (strcmp(char_ptr->feature, feature)==0) && (char_ptr->data_config == data_config) ) // found
	{
	    *hnd = char_ptr->handle;
	    *property = char_ptr->property;

	    if (handler != NULL)
	        *handler = (Msg_Hdl) char_ptr->msg_hdl;

	    return 0;
	}
    }

    return -1; // handle not found for this manufacturer
}


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

int TI_sensortag_temperature(unsigned char *buff, int length, void *value)
{
    double ambient = 0;
    double target = 0;

    if (length != 4)  // four bytes needed
	return -1;

    // Little endian- formatted
    ambient= (double) convert_int16_le(buff, 2) / 128.0;

    target = extractTargetTemperature(buff, ambient);

    printf("Temperature: ambient: %lf [°C], target: %lf [°C]\n", ambient, target);

    if (value != NULL)
        memcpy(value, &target, sizeof(double));

    return 0;  // return later also ambient !
}

