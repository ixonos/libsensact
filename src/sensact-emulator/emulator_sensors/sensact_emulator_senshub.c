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
#include <stdio.h>
#include <sys/shm.h>
#include <errno.h>
#include "sensact_emulator_senshub.h"

senshub_t *senshub;
void *shared_mem_senshub = (void*) 0;
int shmid;
senshub_t * create_senshub_emulator() {

	shmid = shmget((key_t) shared_memory_senshub, sizeof(senshub_t),
			0666 | IPC_CREAT);
	if (shmid == -1) {
		printf("shmget failed %s \n", strerror( errno));
	} else {

		shared_mem_senshub = shmat(shmid, (void *) 0, 0);
		senshub = (senshub_t*) shared_mem_senshub;
	}

	if (senshub != NULL) {
		senshub->roll = 0;
		senshub->roll_name = "roll";
		senshub->setroll = setroll;
		senshub->getroll = getroll;

		senshub->pitch = 0;
		senshub->pitch_name = "pitch";
		senshub->setpitch = setpitch;
		senshub->getpitch = getpitch;

		senshub->yaw = 0;
		senshub->yaw_name = "yaw";
		senshub->getyaw = getyaw;
		senshub->setyaw = setyaw;

		senshub->light = 50;
		senshub->light_name = "light";
		senshub->setlight = setlight;
		senshub->getlight = getlight;

		senshub->presure = 95000;
		senshub->presure_name = "pressure";
		senshub->setpresure = setpresure;
		senshub->getpresure = getpresure;

		senshub->objtemp = 15;
		senshub->objtemp_name = "objTemp";
		senshub->setobjtemp = setobjtemp;
		senshub->getobjtemp = getobjtemp;

		senshub->ambtemp = 30;
		senshub->ambtemp_name = "ambTemp";
		senshub->getambtemp = getambtemp;
		senshub->setambtemp = setambtemp;

		senshub->humidity = 50;
		senshub->humidity_name = "humidity";
		senshub->sethumidity = sethumidity;
		senshub->gethumidity = gethumidity;
	}
	return senshub;
}

void destroy_senshub_emulator() {
	shmdt(shared_mem_senshub);
}

int getroll() {
	return senshub->roll;
}
void setroll(int roll) {
	senshub->roll = roll;
}

int getpitch() {
	return senshub->pitch;
}
void setpitch(int pitch) {
	senshub->pitch = pitch;
}

int getyaw() {
	return senshub->yaw;
}
void setyaw(int yaw) {
	senshub->yaw = yaw;
}

float getlight() {
	return senshub->light;
}
void setlight(float light) {
	senshub->light = light;
}

float getpresure() {
	return senshub->presure;
}
void setpresure(float presure) {
	senshub->presure = presure;
}

float getobjtemp() {
	return senshub->objtemp;
}
void setobjtemp(float objtemp) {
	senshub->objtemp = objtemp;
}

float getambtemp() {
	return senshub->ambtemp;
}
void setambtemp(float ambtemp) {
	senshub->ambtemp = ambtemp;
}

float gethumidity() {
	return senshub->humidity;
}
void sethumidity(float humidity) {
	senshub->humidity = humidity;
}

