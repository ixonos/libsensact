#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/shm.h>
#include <time.h>
#include "sensact_emulator_engine.h"

#define SIG SIGRTMIN
#define CLOCKID CLOCK_REALTIME

void *shared_mem = (void*) 0;
engine_t * engine;
int shmid;
engine_t *create_emulator_engine() {

	shmid = shmget((key_t) shared_memory_engine, sizeof(engine_t), 0666 | IPC_CREAT);
	if (shmid == -1) {
		printf("shmget failed\n");
	}

	shared_mem = shmat(shmid, (void *) 0, 0);
	engine = (engine_t*)shared_mem;

	if (engine != NULL) {
		engine->setdirection = setdirection;
		engine->getdirection = getdirection;
		engine->direction = 0;
		engine->direction_name = "direction";
		engine->rpm = 1000;
		engine->rpm_name = "rpm";
		engine->getrpm = getrpm;
		engine->setrpm = setrpm;

	}
	return engine;
}

void setrpm(int newrpm) {
	engine->rpm = newrpm;
}

int getrpm(void) {

	return engine->rpm;
}

void setdirection(char newdirection) {
	engine->direction = newdirection;
}

char getdirection() {
	return engine->direction;
}

