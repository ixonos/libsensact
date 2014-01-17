/*
 * sensact_emulator_senshub.h
 *
 *  Created on: Jan 10, 2014
 *      Author: mollemi
 */

#ifndef SENS_EMULATOR_SENSHUB_H_
#define SENS_EMULATOR_SENSHUB_H_

#define shared_memory_senshub 1235
/**
 *A TI senshub emulation
 */

typedef struct {
	int roll;
	const char *roll_name;
	void (*setroll)(int roll);
	int (*getroll)(void);

	int pitch;
	const char *pitch_name;
	void (*setpitch)(int roll);
	int (*getpitch)(void);

	int yaw;
	const char *yaw_name;
	void (*setyaw)(int yaw);
	int (*getyaw)(void);

	float light;
	const char *light_name;
	void (*setlight)(float light);
	float (*getlight)(void);

	float presure;
	const char *presure_name;
	void (*setpresure)(float presure);
	float (*getpresure)(void);

	float objtemp;
	const char *objtemp_name;
	float (*getobjtemp)(void);
	void (*setobjtemp)(float objtemp);

	float ambtemp;
	const char *ambtemp_name;
	float (*getambtemp)(void);
	void (*setambtemp)(float ambtemp);

	float humidity;
	const char *humidity_name;
	void (*sethumidity)(float humidity);
	float (*gethumidity)(void);
} senshub_t;

/**
 * call to create a engine
 */
senshub_t *create_senshub_emulator();

int getroll();
void setroll(int roll);

int getpitch();
void setpitch(int pitch);

int getyaw();
void setyaw(int yaw);

float getlight();
void setlight(float light);

float getpresure();
void setpresure(float presure);

float getobjtemp();
void setobjtemp(float objtemp);

float getambtemp();
void setambtemp(float ambtemp);

float gethumidity();
void sethumidity(float humidity);

#endif
