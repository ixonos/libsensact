/*
 * sensact-emulator.h
 *
 *  Created on: Jan 3, 2014
 *      Author: mollemi
 */

#ifndef SENSACT_EMULATOR_H_
#define SENSACT_EMULATOR_H_

#include "../../sensact-emulator/emulator_sensors/sensact_emulator_engine.h"
#include "../../sensact-emulator/emulator_sensors/sensact_emulator_senshub.h"
struct emulator_t
{
	struct emulator_device_handle *device;
};
int emulator_connect(int device_id, void *device);
int emulator_disconnect(int handle);
int emulator_reconnect(int handle);
int emulator_write(int handle, char *data, int length, int timeout);
int emulator_read(int handle, char *data, int length, int timeout);

senshub_t * getEmuSensHub();
engine_t * getEmuEngine();

extern struct sa_backend_t emulator_backend;
#endif /* SENSACT_EMULATOR_H_ */
