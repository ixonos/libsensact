#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include "sensact-emulator-test.h"
#include "../../include/sensact/sensact-emulator.h"
#include "../emulator_sensors/sensact_emulator_engine.h"
#include "../emulator_sensors/sensact_emulator_senshub.h"

int tests_run = 0;

void *shared_memory;
senshub_t * senshub;
engine_t *engine;
/**
 *dummy to write RPM
 */
void emulator_set_rpm(int rpm) {
	engine->rpm = rpm;
}

/**
 * sweeps the engine from 1 to max to 1 - 10 times
 */
static char * test_sweep_engine_fast() {

	int rpm = 0;
	emulator_set_rpm(rpm);
	int count;
	for (count = 0; count < 10; count++) {
		for (rpm = 0; rpm < 8000; rpm++) {
			emulator_set_rpm(rpm);
		}
		sleep(1);
		mu_assert("error, engine->rpm != 7999", engine->rpm == 7999);
		for (rpm = 8000; rpm > 0; rpm--) {
			emulator_set_rpm(rpm);
		}
		sleep(1);
		mu_assert("error, engine->rpm != 1", engine->rpm == 1);
	}

	return 0;
}
/**
 * sweeps the engine from 1 to 8000 - 10 times
 */
static char * test_sweep_engine_zero_max() {

	int rpm = 0;
	emulator_set_rpm(rpm);
	int count;
	for (count = 0; count < 10; count++) {
		for (rpm = 0; rpm < 8000; rpm++) {
			usleep(1000);
			emulator_set_rpm(rpm);
		}
		mu_assert("error, engine->rpm != 7999", engine->rpm == 7999);
	}

	return 0;
}
/**
 * sweeps the engine from 1 to 8000 to 1 - 10 times
 */
static char * test_sweep_engine_zero_max_zero() {

	int rpm = 0;
	emulator_set_rpm(rpm);
	int count;
	for (count = 0; count < 10; count++) {
		for (rpm = 0; rpm < 8000; rpm++) {
			usleep(1000);
			emulator_set_rpm(rpm);
		}
		mu_assert("error, engine->rpm != 7999", engine->rpm == 7999);
		for (rpm = 8000; rpm > 0; rpm--) {
			usleep(1000);
			emulator_set_rpm(rpm);
		}
		mu_assert("error, engine->rpm != 1", engine->rpm == 1);
	}

	return 0;
}
/**
 * sets the rpm in jumps of 1000 and reverses the engine
 */

static char * test_sweep_engine_and_reverse() {

	int rpm = 0;

	int count;
	for (count = 0; count < 8; count++) {
		emulator_set_rpm(count * 1000);
		setdirection(count % 2 > 0);
		sleep(2);
	}
	emulator_set_rpm(1000);
	setdirection(1);

	return 0;
}
/**
 * send some values after the the senshub orientation part
 */
static char * test_orientation_sweep() {

	int i, count = 0;

	for (count = 0; count < 10; count++) {
		for (i = 0; i < 1000; i++) {
			senshub->pitch = i;
			senshub->yaw = i;
			senshub->roll = 0;
			usleep(8000);
		}
		sleep(5);
	}
	for (count = 0; count < 10; count++) {
		for (i = 0; i < 1000; i++) {
			senshub->yaw = i;
			usleep(1000);
		}
	}
	sleep(5);

	for (count = 0; count < 10; count++) {
		for (i = 0; i < 1000; i++) {
			senshub->roll = i;
			usleep(3000);
		}
	}

	return 0;
}

static char * test_temperature_sweep() {
	senshub->ambtemp = 50;
	return 0;
}

static char* runsenshub_tests() {

	mu_run_test(test_orientation_sweep);
	return 0;
}

static char* runengine_tests() {

	 mu_run_test(test_sweep_engine_fast);
	 mu_run_test(test_sweep_engine_zero_max);
	 mu_run_test(test_sweep_engine_zero_max_zero);
	 mu_run_test(test_sweep_engine_and_reverse);

	return 0;
}

static char * all_tests() {
	char * retval = 0;
	retval = runengine_tests();
	retval = runsenshub_tests();
	return retval;
}
/*
 * setup called before all tests
 *
 */
int setup() {
	int retval = 0;
	int shmid = 0;

	shmid = shmget((key_t) shared_memory_engine, sizeof(engine_t), 0666 | IPC_CREAT);
	if (shmid != -1) {
		shared_memory = shmat(shmid, (void *) 0, 0);
		engine = (engine_t *) shared_memory;
		retval = 1;
	}
	shmid = shmget((key_t) shared_memory_senshub, sizeof(senshub_t), 0666 | IPC_CREAT);
	if (shmid != -1) {
		shared_memory = shmat(shmid, (void *) 0, 0);
		senshub = (senshub_t *) shared_memory;
		retval = 1;
	}
	return retval;
}



int main(int argc, char **argv) {
	int retval = setup();
	if (retval) {
		char *result = all_tests();
		if (result != 0) {
			printf("%s\n", result);
		} else {
			printf("ALL TESTS PASSED\n");
		}
		printf("Tests run: %d\n", tests_run);
		return result != 0;
	} else {
		printf("setup failed!");
	}
	return retval;
}
