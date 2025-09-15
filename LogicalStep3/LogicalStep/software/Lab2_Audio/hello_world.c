#include "altera_up_avalon_audio.h"
int main(void)
{
	alt_up_audio_dev * audio_dev;
	/* used for audio record/playback */
	unsigned int l_buf;
	unsigned int r_buf;
	// open the Audio port
	audio_dev = alt_up_audio_open_dev ("/dev/Audio");
	if ( audio_dev == NULL)
		alt_printf ("Error: could not open audio device \n");
	else
		alt_printf ("Opened audio device \n");

	/* read and echo audio data */
	while(1)
	{
		int fifospace = alt_up_audio_read_fifo_avail (audio_dev, ALT_UP_AUDIO_RIGHT);
			if ( fifospace > 0 ) // check if data is available
				{
				// read audio buffer
				alt_up_audio_read_fifo (audio_dev, &(r_buf), 1, ALT_UP_AUDIO_RIGHT);
				alt_up_audio_read_fifo (audio_dev, &(l_buf), 1, ALT_UP_AUDIO_LEFT);
				// write audio buffer
				alt_up_audio_write_fifo (audio_dev, &(r_buf), 1, ALT_UP_AUDIO_RIGHT);
				alt_up_audio_write_fifo (audio_dev, &(l_buf), 1, ALT_UP_AUDIO_LEFT);
				}
		}
}


di 0
fi 0
fo 1 computer.wav
fp 684712