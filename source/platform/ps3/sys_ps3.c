/*
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

#include "../../nzportable_def.h"
#include "errno.h"

// PS3 Headers
#include <sys/systime.h>
#include <sys/process.h>
#include <io/pad.h>
// TODO: gem stuff for PSMove eventually

#include <sys/stat.h>
#include <unistd.h>

#define QUAKE_HUNK_MB			24 		// cypress -- usable quake hunk size in mB
#define QUAKE_HUNK_MB_NEW3DS	72		// ^^ ditto, but n3ds
// TODO: Decide how big the PS3 hunk should be

#define LINEAR_HEAP_SIZE_MB		16		// cypress -- we lower this as much as possible while still remaining
										// bootable so we can up the quake hunk and actually viable memory.

// Stack size from Vita, no idea if it's right
SYS_PROCESS_PARAM(1001, 0x800000);

qboolean isDedicated;

/*
===============================================================================

FILE IO

===============================================================================
*/

#define MAX_HANDLES             10
FILE    *sys_handles[MAX_HANDLES];

int             findhandle (void)
{
	int             i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int             pos;
	int             end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	
	return filelength(f);
}

int Sys_FileOpenWrite (char *path)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s: %s", path,strerror(errno));
	sys_handles[i] = f;
	
	return i;
}

void Sys_FileClose (int handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}

void Sys_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

int     Sys_FileTime (char *path)
{
	FILE    *f;
	
	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);
		return 1;
	}
	
	return -1;
}

void Sys_mkdir (char *path)
{
	mkdir(path, 0777);
}

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}

void Sys_PrintSystemInfo(void)
{
	Con_Printf ("PS3 NZP v%4.1f (PKG: "__TIME__" "__DATE__")\n", (double)(VERSION));
}

void Sys_SystemError(char *error)
{
	FILE* f = f = fopen("log.txt", "a+");
	fwrite(error, 1, strlen(error), f);
	fclose(f);
	Sys_Quit();
}

void Sys_Printf (char *fmt, ...)
{
	va_list         argptr;
	
	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr);
}

void Sys_Quit (void)
{
	Host_Shutdown();

	// TODO: PS3GL has no deinit
	//gfxExit();
	exit(0);
}

double Sys_FloatTime(void)
{
    static uint64_t initial_tb = 0;
    static uint64_t tb_freq = 0;

    if (tb_freq == 0)
        tb_freq = sysGetTimebaseFrequency();

    uint64_t tb = __builtin_ppc_mftb();

    if (initial_tb == 0)
        initial_tb = tb;

    return (double)(tb - initial_tb) / (double)tb_freq;
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void Sys_Sleep (void)
{
}

void Sys_DefaultConfig(void)
{
	// naievil -- fixme I didn't do this
	// fancyTODO: Bind PS3
	Cbuf_AddText ("bind ABUTTON +right\n");
	Cbuf_AddText ("bind BBUTTON +lookdown\n");
	Cbuf_AddText ("bind XBUTTON +lookup\n");
	Cbuf_AddText ("bind YBUTTON +left\n");
	Cbuf_AddText ("bind LTRIGGER +jump\n");
	Cbuf_AddText ("bind RTRIGGER +attack\n");
	Cbuf_AddText ("bind PADUP \"impulse 10\"\n");
	Cbuf_AddText ("bind PADDOWN \"impulse 12\"\n");
	//Cbuf_AddText ("lookstrafe \"1.000000\"\n");
	//Cbuf_AddText ("lookspring \"0.000000\"\n");
}

#define PS3_SEND_KEY(QUAKEBTN, PADBTN) \
if (currentPadData.PADBTN != previousPadData.PADBTN) \
	Key_Event(QUAKEBTN, currentPadData.PADBTN);
void Sys_SendKeyEvents (void)
{
	padInfo padInfo;
	ioPadGetInfo(&padInfo);
	if (!padInfo.status[0]) return;

	static padData previousPadData;
	static padData currentPadData;
	ioPadGetData(0, &currentPadData);	
	PS3_SEND_KEY(K_SELECT,     BTN_SELECT);
	PS3_SEND_KEY(K_ESCAPE,     BTN_START);
	PS3_SEND_KEY(K_UPARROW,    BTN_UP);
	PS3_SEND_KEY(K_DOWNARROW,  BTN_DOWN);
	PS3_SEND_KEY(K_LEFTARROW,  BTN_LEFT);
	PS3_SEND_KEY(K_RIGHTARROW, BTN_RIGHT);
	PS3_SEND_KEY(K_AUX4,       BTN_SQUARE);
	PS3_SEND_KEY(K_AUX3,       BTN_TRIANGLE);
	PS3_SEND_KEY(K_AUX2,       BTN_CROSS);
	PS3_SEND_KEY(K_AUX1,       BTN_CIRCLE);
	PS3_SEND_KEY(K_AUX5,       BTN_L1);
	PS3_SEND_KEY(K_AUX7,       BTN_R1);
	PS3_SEND_KEY(K_AUX6,       BTN_L2);
	PS3_SEND_KEY(K_AUX8,       BTN_R2);

	if(previousPadData.button != currentPadData.button)
		previousPadData = currentPadData;
}

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}

void Sys_CaptureScreenshot(void)
{
	Sys_Error("Not implemented!");
}

//=============================================================================

bool game_running;
int main (int argc, char **argv)
{
	static double time, oldtime;
	static quakeparms_t parms;
	//new3ds_flag = false;

	//osSetSpeedupEnable(true);

	//APT_CheckNew3DS(&new3ds_flag);

	//gfxInit(GSP_BGR8_OES, GSP_RGB565_OES, false); 
	//gfxSetDoubleBuffering(GFX_BOTTOM, false);
	//gfxSwapBuffersGpu();

	//uint8_t model;

	//cfguInit();
	//CFGU_GetSystemModel(&model);
	//cfguExit();
	
	ioPadInit(1);
	//if(model != CFG_MODEL_2DS && new3ds_flag == true)
	//	gfxSetWide(true);
	
	//chdir("sdmc:/3ds/nzportable");;

	//if (new3ds_flag == true)
		parms.memsize = QUAKE_HUNK_MB_NEW3DS * 1024 * 1024;
	//else
	//	parms.memsize = QUAKE_HUNK_MB * 1024 * 1024;
	
	parms.membase = malloc(parms.memsize);
	parms.basedir = "/dev_hdd0/game/NZPORTABL/USRDIR";

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	Host_Init (&parms);

	oldtime = Sys_FloatTime();

	game_running = true;
	while (/*aptMainLoop() && */game_running)
	{
		time = Sys_FloatTime();

		double dt = time - oldtime;

		// clamp to avoid giant frame jumps
		if (dt > 0.1)
			dt = 0.1;

		if (dt < 0.001)
    		dt = 0.001;

		Host_Frame (dt);
		oldtime = time;
	}

	return 0;
}


