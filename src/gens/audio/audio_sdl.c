/***************************************************************************
 * Gens: Audio Handler - SDL Backend.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "audio.h"
#include "audio_sdl.h"
#include "audio_mmx.h"
#include <SDL/SDL.h>

// Gens includes.
#include "emulator/g_main.hpp"
#include "gens_core/misc/cpuflags.h"
#include "gens_core/mem/mem_m68k.h"

// C includes.
#include <string.h>
#include <time.h>

// Function prototypes.
static int	audio_sdl_init(void);
static int	audio_sdl_end(void);

static int	audio_sdl_write_sound_buffer(void *dump_buf);
static void	audio_sdl_clear_sound_buffer(void);

static void	audio_sdl_wait_for_audio_buffer(void);

// SDL audio variables.
static int audio_sdl_len;
static unsigned char *audio_sdl_pMsndOut = NULL;
static unsigned char *audio_sdl_audiobuf = NULL;

// SDL functions.
static void audio_sdl_callback(void *user, uint8_t *buffer, int len);

// Audio Backend struct.
audio_backend_t audio_backend_sdl =
{
	.init = audio_sdl_init,
	.end = audio_sdl_end,
	
	.write_sound_buffer = audio_sdl_write_sound_buffer,
	.clear_sound_buffer = audio_sdl_clear_sound_buffer,
	
	.play_sound = NULL,
	.stop_sound = NULL,
	
	.wait_for_audio_buffer = audio_sdl_wait_for_audio_buffer
};


/**
 * audio_sdl_init(): Initialize the SDL audio subsystem.
 * @return 0 on success; non-zero on error.
 */
int audio_sdl_init(void)
{
	if (audio_initialized)
		return -1;
	
	// Make sure sound is shut down first.
	audio_sdl_end();
	
	// Determine the segment length.
	audio_calc_segment_length();
	
	int i;
	int videoLines = (CPU_Mode ? 312 : 262);
	for (i = 0; i < videoLines; i++)
	{
		Sound_Extrapol[i][0] = ((audio_seg_length * i) / videoLines);
		Sound_Extrapol[i][1] = (((audio_seg_length * (i + 1)) / videoLines) - Sound_Extrapol[i][0]);
	}
	for (i = 0; i < audio_seg_length; i++)
		Sound_Interpol[i] = ((videoLines * i) / audio_seg_length);
	
	// Clear the segment buffers.
	memset(Seg_L, 0x00, sizeof(Seg_L));
	memset(Seg_R, 0x00, sizeof(Seg_R));
	
	// Attempt to initialize SDL audio.
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
		return -1;
	
	// Allocate the segment buffer.
	audio_sdl_pMsndOut = (unsigned char*)(malloc(audio_seg_length << 2));
	
	// Set up the SDL audio specification.
	SDL_AudioSpec spec;
	spec.freq = audio_get_sound_rate();
	spec.format = AUDIO_S16SYS;
	spec.channels = !(audio_get_stereo()) ? 1 : 2; // TODO: Initializing 1 channel seems to double-free if it's later changed...
	spec.samples = 1024;
	spec.callback = audio_sdl_callback;
	spec.userdata = NULL;
	
	// Initialize the audio buffer.
	audio_sdl_audiobuf = (unsigned char*)(calloc(1, (spec.samples * spec.channels * 2 * 4) * sizeof(short)));
	
	if (SDL_OpenAudio(&spec, 0) != 0)
	{
		// Could not open audio.
		free(audio_sdl_audiobuf);
		audio_sdl_audiobuf = NULL;
		return -2;
	}
	SDL_PauseAudio(0);
	
	// Sound is initialized.
	audio_initialized = TRUE;
	return 0;
}


/**
 * audio_sdl_end(): Shut down the SDL audio subsystem.
 * @return 0 on success; non-zero on error.
 */
int audio_sdl_end(void)
{
	SDL_PauseAudio(1);
	
	// Free the audio buffers.
	free(audio_sdl_audiobuf);
	audio_sdl_audiobuf = NULL;
	free(audio_sdl_pMsndOut);
	audio_sdl_pMsndOut = NULL;
	
	audio_sound_is_playing = FALSE;
	audio_initialized = FALSE;
	
	// Shut down SDL audio.
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	
	return 0;
}


/**
 * audio_sdl_callback(): SDL audio callback.
 * @param user
 * @param buffer
 * @param len
 */
static void audio_sdl_callback(void *user, uint8_t *buffer, int len)
{
	if (audio_sdl_len < len)
	{
		memcpy(buffer, audio_sdl_audiobuf, audio_sdl_len);
		audio_sdl_len = 0;
		return;
	}
	
	memcpy(buffer, audio_sdl_audiobuf, len);
	audio_sdl_len -= len;
	memcpy(audio_sdl_audiobuf, audio_sdl_audiobuf + len, audio_sdl_len);
}


/**
 * audio_sdl_write_sound_buffer(): Write the sound buffer to the audio output.
 * @param dump_buf Sound dumping buffer.
 * @return 0 on success; non-zero on error.
 */
static int audio_sdl_write_sound_buffer(void *dump_buf)
{
	SDL_LockAudio();
	
	// TODO: Fix dumpBuf support.
#if 0
	if (dump_buf)
	{
		if (audio_get_stereo())
			audio_dump_sound_stereo(dump_buf, audio_seg_length);
		else
			audio_dump_sound_mono(dump_buf, audio_seg_length);
	}
#endif
	
	if (audio_get_stereo())
	{
#ifdef GENS_X86_ASM
		if (CPU_Flags & CPUFLAG_MMX)
			writeSoundStereo_MMX(Seg_L, Seg_R, (short*)(audio_sdl_pMsndOut), audio_seg_length);
		else
#endif
			audio_write_sound_stereo((short*)audio_sdl_pMsndOut, audio_seg_length);
	}
	else
	{
#ifdef GENS_X86_ASM
		if (CPU_Flags & CPUFLAG_MMX)
			writeSoundMono_MMX(Seg_L, Seg_R, (short*)(audio_sdl_pMsndOut), audio_seg_length);
		else
#endif
			audio_write_sound_mono((short*)audio_sdl_pMsndOut, audio_seg_length);
	}
	
	memcpy(audio_sdl_audiobuf + audio_sdl_len, audio_sdl_pMsndOut, audio_seg_length * 4);
	audio_sdl_len += audio_seg_length * 4;
	
	SDL_UnlockAudio();
	
	// TODO: Figure out if there's a way to get rid of this.
	struct timespec rqtp = {0, 1000000};
	while (audio_sdl_len > 1024 * 2 * 2 * 4)
	{
		nanosleep(&rqtp, NULL);	
		if (fast_forward)
			audio_sdl_len = 1024;
	} //SDL_Delay(1); 
	
	return 0;
}


static void audio_sdl_clear_sound_buffer(void)
{
	// NOTE: This isn't used with SDL.
}


/**
 * audio_sdl_wait_for_audio_buffer(): Wait for the audio buffer to empty out.
 * This function is used for Auto Frame Skip.
 */
void audio_sdl_wait_for_audio_buffer(void)
{
	audio_sdl_write_sound_buffer(NULL);
	while (audio_sdl_len <= (audio_seg_length * audio_seg_to_buffer))
	{
		Update_Frame_Fast();
		audio_sdl_write_sound_buffer(NULL);
	}
}
