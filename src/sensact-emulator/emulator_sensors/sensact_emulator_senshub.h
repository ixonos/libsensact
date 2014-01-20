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
 * call to create a engine and attach shared memory
 */
senshub_t *create_senshub_emulator();
/**
 * Detach the shared memory
 */
void destroy_senshub_emulator(void);

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
