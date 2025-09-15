#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <system.h>
#include <sys/alt_alarm.h>
#include <io.h>
#include "system.h"

#include "fatfs.h"
#include "diskio.h"
#include "ffconf.h"

#include "ff.h"
#include "monitor.h"
#include "uart.h"

#include "alt_types.h"

#include <altera_up_avalon_audio.h>
#include <altera_up_avalon_audio_and_video_config.h>
#include "sys/time.h"

#define ESC 27
#define CLEAR_LCD_STRING "[2J"
#define BUF_SIZE 1000

DIR Dir;
FILINFO Finfo;
FATFS Fatfs[_VOLUMES];
FIL File;
FILE *lcd = NULL;

char fileNames[20][20];
unsigned int fileSizes[20];
int numFiles = 0;
int curIndex = 0;
int playing = 0; // bool
int playMode = 2; // 2: normal, 3: half-speed, 4: double-speed, 5: mono-left audio
int terminate = 0; // bool
int stopped = 1; // bool
int trackChange = 0; // bool

//can combine stopped and playing


int buttonState = 0xf;
int toPerform = 0; // dispatched change

// playback modes
char* modeStrings[] =  {"STOPPED\0", "PAUSED\0", "PBACK-NORM SPD\0",
	"PBACK-HALF SPD\0", "PBACK-DBL SPD\0", "PBACK-MONO\0"};

char *ptr;

alt_up_audio_dev * audio_dev;

void writeToLCD(int index, int mode);

static void timer_ISR(void* context, alt_u32 id) {
	int newButtonState = IORD(PUSH_PIO_BASE, 0);

	switch (toPerform) {

		case 0b0111: // prev track
			terminate = 1;
			curIndex = curIndex > 0 ? (curIndex-1)%numFiles : numFiles-1;
			writeToLCD(curIndex, playing ? playMode : 0);
			trackChange = 1;
			toPerform = 0;
			break;
		case 0b1011: // stop
			toPerform = 0;
			if (stopped) break; // if alr in stopped don't do nothing
			stopped = 1;
			terminate = 1;
			writeToLCD(curIndex, 0);
			playing = 0;
			break;
		case 0b1101: // play/pause
			stopped = 0;
			playing = !playing;
			writeToLCD(curIndex, playing ? playMode : 1);
			toPerform = 0;
			break;
		case 0b1110: // next track
			terminate = 1;
			curIndex = (curIndex+1)%numFiles;
			writeToLCD(curIndex, playing ? playMode : 0);
			trackChange = 1;
			toPerform = 0;
			break;
		default: // falling edge
			toPerform = newButtonState;
			break;


	}

	// update button state for comparison
	buttonState = newButtonState;


	// clear timer
	IOWR(TIMER_0_BASE, 1, 0);
	IOWR(TIMER_0_BASE, 3, 0);

	// clear pb irq
	IOWR(PUSH_PIO_BASE, 3, 0x0);




}

static void PB_ISR(void* context, alt_u32 id) {

	IOWR(TIMER_0_BASE, 1, 0b101); // start timer

	IOWR(PUSH_PIO_BASE, 3, 0x0); // clear request

}


int isWav(char* name) {
	if (strlen(name) > 4) {
		int len = strlen(name);
		// check file extension
		if (name[len-1] == 'V' && name[len-2] == 'A' && name[len-3] == 'W' && name[len-4] == '.') return 1;
	}
	return 0;
}


void printFiles() {
	for (int i = 0; i < 20; i++) {
		if (strlen(fileNames[i])) printf("Name: %s, Size: %d\n", fileNames[i], fileSizes[i]);
	}
}

void getWavFiles() {
	f_opendir(&Dir, ptr);

	f_readdir(&Dir, &Finfo);

	int i = 0;

	do  {
		if (isWav((char*)Finfo.fname)) { // store wav files
			strcpy(fileNames[i], Finfo.fname);
			fileSizes[i] = Finfo.fsize;
			i++;
		}
	} while (!f_readdir(&Dir, &Finfo) && strlen(Finfo.fname) > 0);

	numFiles = i; // store file count

}

void put_rc(FRESULT rc)
{
    const char *str =
        "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
        "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
        "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
        "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
    FRESULT i;

    for (i = 0; i != rc && *str; i++) {
        while (*str++);
    }
    xprintf("rc=%u FR_%s\n", (uint32_t) rc, str);
}

void writeToLCD(int index, int mode) {
	if (lcd != NULL) return; // ensure no other ISR is writing to LCD

	lcd = fopen("/dev/lcd_display", "w");

	fprintf(lcd, "%c%s", ESC, CLEAR_LCD_STRING); // clear display

	if (lcd!=NULL) {
		fprintf(lcd, "%d - %s\n", index+1, fileNames[index]); // write 1-indexed number and name
		fprintf(lcd, modeStrings[mode]);
	}

	fclose(lcd);
	lcd = NULL;

}

void playTrack(int index){
	int speed = 4;
	int mono = 0;
	int switches = IORD(SWITCH_PIO_BASE, 0) & 0b11;
	if (switches == 0b00) { // normal pb
		writeToLCD(curIndex, 2);
		playMode = 2;
	}
	else if (switches == 0b01) {  // half speed
		speed = 2;
		writeToLCD(curIndex, 3);
		playMode = 3;
	}
	else if (switches == 0b10) { // double speed
		speed = 8;
		writeToLCD(curIndex, 4);
		playMode = 4;
	}
	else if (switches == 0b11) { // mono
		mono = 1;
		writeToLCD(curIndex, 5);
		playMode = 5;
	}
	else return;

	f_open(&File, fileNames[index], 1);
	int p1 = fileSizes[index];

	uint32_t count = BUF_SIZE;
	uint8_t buf[BUF_SIZE];
	unsigned int r_buf;
	unsigned int l_buf;
	while (p1 > 0)
	{
		f_read(&File, buf, count, &count); // count as output param for eof

		for (int i = 0; i < count; i+=speed) {
			int ind = i%4==0 ? i : i-2; // if half speed, round down to nearest multiple of 4
			l_buf = (unsigned int)((buf[ind+1] << 8) | buf[ind]);
			r_buf = (unsigned int)((buf[ind+3]<< 8) | buf[ind+2]);
			while ((alt_up_audio_write_fifo_space(audio_dev,
								ALT_UP_AUDIO_LEFT)) == 0) {}
			while ((alt_up_audio_write_fifo_space(audio_dev,
					ALT_UP_AUDIO_RIGHT)) == 0) {}
			alt_up_audio_write_fifo(audio_dev, mono ? &(l_buf) : &(r_buf), 1, ALT_UP_AUDIO_RIGHT);
			alt_up_audio_write_fifo(audio_dev, &(l_buf), 1, ALT_UP_AUDIO_LEFT);
			if (terminate) {
				terminate = 0;
				f_close(&File); // safe exit
				return;
			}
		}
		p1-=count;
		while (!playing) { // poll for terminate while paused
			if (terminate) {
				terminate = 0;
				f_close(&File); // safe exit
				return;
			}
		}
	}
	f_close(&File);
}

int main()
{

  // enable pb irq
  IOWR(PUSH_PIO_BASE, 2, 0xf);

  // clear requests
  IOWR(PUSH_PIO_BASE, 3, 0x0);

  // enable timer irq
  alt_irq_register(TIMER_0_IRQ, (void *) 0, timer_ISR); // Register the timer ISR
  IOWR(TIMER_0_BASE, 2, 100); // Set period of timer

  // config pb irq register
  alt_irq_register( PUSH_PIO_IRQ, (void *) 0, PB_ISR);

  // init disk at 0 addr
  disk_initialize((uint8_t) 0);

  // init file system at 0 addr
  put_rc(f_mount((uint8_t) 0, &Fatfs[0]));

  // store wav files
  getWavFiles();

  // print files and sizes
  printFiles();


  audio_dev = alt_up_audio_open_dev ("/dev/Audio");
  if ( audio_dev == NULL)
  alt_printf ("Error: could not open audio device \n");

  // track 1 and stopped by default
  writeToLCD(curIndex, 0);


  while (1) {
	  while (!playing) {} // wait for unpause
	  playTrack(curIndex); // play track at index
	  if(!trackChange)  { // handle file termination
		  stopped = 1;
		  playing = 0;
		  writeToLCD(curIndex, 0);
	  }
	  else { // handle track change
		  if (!playing) stopped = 1;
		  trackChange = 0;
	  }

  }

  return 0;
}


