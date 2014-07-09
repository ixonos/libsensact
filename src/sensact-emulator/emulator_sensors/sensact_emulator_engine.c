/*
 * Copyright (c) 2013-2014, Ixonos Denmark ApS
 * Copyright (c) 2013-2014, Michael Møller
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>
#include "sensact_emulator_engine.h"

void *shared_mem_engine = (void*) 0;
engine_t * engine;
int shmid;
engine_t *create_emulator_engine() {

	shmid = shmget((key_t) shared_memory_engine, sizeof(engine_t),
			0666 | IPC_CREAT);
	if (shmid == -1) {
		printf("shmget failed %s \n", strerror( errno));

	} else {
		shared_mem_engine = shmat(shmid, (void *) 0, 0);
		engine = (engine_t*) shared_mem_engine;
	}

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
/**
 * detach memory
 */
void destroy_engine_emulator() {
	shmdt(shared_mem_engine);
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

