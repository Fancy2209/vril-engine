/*
Copyright (C) 2017 Felipe Izzo
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <stdio.h>
#include <switch.h>
#include "../../nzportable_def.h"

#define SAMPLE_RATE   	22050
#define NUM_SAMPLES 	2048
#define SAMPLE_SIZE		4
#define BUFFER_SIZE 	NUM_SAMPLES*SAMPLE_SIZE

static int sound_initialized = 0;

qboolean SNDDMA_Init(void)
{
	sound_initialized = 0;
    return false;
}

int SNDDMA_GetDMAPos(void)
{
	return 0;
}

void SNDDMA_Shutdown(void)
{
	return;
}

void SNDDMA_Submit(void)
{
}