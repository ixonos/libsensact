/*
 * sens_emulator_engine.h
 *
 *  Created on: Jan 10, 2014
 *      Author: mollemi
 */

#ifndef SENS_EMULATOR_ENGINE_H_
#define SENS_EMULATOR_ENGINE_H_
#define shared_memory_engine 1234

/**
 *A little engine, with rpm and motor direction
 */
typedef struct {
	int rpm;
	const char * rpm_name;
	void (*setrpm)(int rpm);
	int (*getrpm)(void);

	char direction;
	const char * direction_name;
	void (*setdirection)(char direction);
	char (*getdirection)(void);

} engine_t;

/**
 * call to create a engine
 */
engine_t *create_emulator_engine();
/**
 * setRpm
 */
void setrpm(int newrpm);
/*
 * getRpm
 *
 */
int getrpm(void);
/*
 * setDirection of the engine
 */
void setdirection(char direction);
/*
 * getDirection of the engine
 */
char getdirection();
/*
 * Start the emulator
 */
void start();


#endif /* SENS_EMULATOR_ENGINE_H_ */
