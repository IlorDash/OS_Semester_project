/*******************************************************************************
 * Copyright (c) 2022 Sergey Balabaev (sergei.a.balabaev@gmail.com)                    *
 *                                                                             *
 * The MIT License (MIT):                                                      *
 * Permission is hereby granted, free of charge, to any person obtaining a     *
 * copy of this software and associated documentation files (the "Software"),  *
 * to deal in the Software without restriction, including without limitation   *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell   *
 * copies of the Software, and to permit persons to whom the Software is       *
 * furnished to do so, subject to the following conditions:                    *
 * The above copyright notice and this permission notice shall be included     *
 * in all copies or substantial portions of the Software.                      *
 *                                                                             *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,             *
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR       *
 * OTHER DEALINGS IN THE SOFTWARE.                                             *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <pthread.h>

#define ARG_CNT 4
#define QUIT_HELP_OPT_POS 1

#define READ_BUTTON_LEN 17 //hh:mm:ss.ssssss_n

void help();

int writeFifo(char * fifoName, char * str) {

	//printf("In write fifo = %s\n", fifoName);

	int fifo_d = open(fifoName, O_WRONLY);
	if (fifo_d == -1) {
		printf ( "Failed to open named pipe = %s", fifoName);
		return -1;
	}

	write(fifo_d, str, strlen(str) + 1);
	close(fifo_d);
	return 0;
}

int readFifo(char * fifo_name, char* read_str, int str_len) {
	int fifo_d = open(fifo_name, O_RDONLY);

	if (fifo_d == -1) {
		printf ( "Failed to open named pipe = %s",  fifo_name);
		return -1;
	}

	int read_num = read(fifo_d, read_str, str_len);
	close(fifo_d);

	//printf("65) In read fifo = %s, Read = %s\n", fifo_name, read_str);

	return read_num;
}

int getButtonState(char * fifo_name) {
	char button_str[100] = {0};
	//printf("72) init button_str = %s\n", button_str);
	int button_state_pos = readFifo(fifo_name, button_str, 100);
	if (button_state_pos < 0) {
		printf ( "Failed to open get Button state pipe = %s",  fifo_name);
		return -1;
	}
	char* last_new_line = strrchr(button_str, '\n');
	//printf("button_str = %s", button_str);
	//printf("button_str last = %d\n", last_new_line[-1] - '0');
	return last_new_line[-1] - '0';
}

float getRange(char * fifo_name) {
	char range_str[32] = {0};

	if (readFifo(fifo_name, range_str, 32) < 0) {
		printf ( "Failed to open get Range pipe = %s",  fifo_name);
		return -1;
	}

	float range = 0.0f;
	sscanf(range_str, "%f", &range);
	return range;
}

int setNote(char * fifo_name, char * note) {
	if (writeFifo(fifo_name, note) < 0) {
		printf ( "Failed to open set Note pipe = %s",  fifo_name);
		return -1;
	}

	return 0;
}


int stop_prog_recv = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *readStopProgFifoThread() {
	char stop_prog_str[16] = {'\0'};
	pthread_mutex_lock(&mutex);
	while (1) {
		int readFifoNum = readFifo("stop_prog", stop_prog_str, 16);
		if (!strncmp(stop_prog_str, "stop", strlen("stop"))) {
			stop_prog_recv = 1;
			pthread_mutex_unlock(&mutex);
			pthread_exit(0);
		}
	}
}




int main(int argc, char *argv[]) {

	pthread_t thread;
	if (pthread_create(&thread, NULL, readStopProgFifoThread, NULL) != 0) {
		fprintf(stderr, "error: pthread_create was failed\n");
		exit(-1);
	}
	// if (pthread_join(thread, NULL) != 0) {
	// 	fprintf(stderr, "error: pthread_create was failed\n");
	// 	exit(-1);
	// }


	const int notes_num = 12;
	float max_range = 0.1f;
	float min_range = 0.0005f;
	float range_step = (max_range - min_range) / notes_num;

	int volume = 1;


	int range_fd;
	char range_fifo[16] = {0};

	int note_fd;
	char note_fifo[16] = {0};

	int button_0_fd;
	char button_0_fifo[16] = {0};

	int button_1_fd;
	char button_1_fifo[16] = {0};

	int quiet = 0;

	if (argc > 1) {
		if ((strcmp(argv[QUIT_HELP_OPT_POS], "-h") == 0)) {
			help();
			return 0;
		}
	}

	if (argc > ARG_CNT) {
		if ((strcmp(argv[QUIT_HELP_OPT_POS], "-q") == 0)) {
			quiet = 1;
		}
	} else {
		printf("Lack of arguments\n");
		return -1;
	}

	int range_fifo_arg = 1;
	if (quiet) {
		range_fifo_arg++;
	}
	strcpy(range_fifo, argv[range_fifo_arg]);

	int note_fifo_arg = range_fifo_arg + 1;
	strcpy(note_fifo, argv[note_fifo_arg]);

	int button_0_fifo_arg = note_fifo_arg + 1;
	strcpy(button_0_fifo, argv[button_0_fifo_arg]);

	int button_1_fifo_arg = button_0_fifo_arg + 1;
	strcpy(button_1_fifo, argv[button_1_fifo_arg]);

	if (!quiet) {
		printf("Received range_fifo = %s\nplay_note_fifo = %s\n", range_fifo, note_fifo);
	}

	float range = 0.0f;

	while (1) {

		if (!pthread_mutex_trylock(&mutex)) {
			if (stop_prog_recv) {
				printf("Received stop prog\n");
				exit(0);
			}
			pthread_mutex_unlock(&mutex);
		}

		int button_0_state;
		int button_1_state;
		button_0_state = getButtonState(button_0_fifo);
		button_1_state = getButtonState(button_1_fifo);

		//printf("button_0_state = %d, button_1_state = %d\n", button_0_state, button_1_state);

		if ((button_0_state < 0) || (button_1_state < 0)) {
			return -1;
		}

		if (!button_0_state) {
			min_range = getRange(range_fifo);
			range_step = (max_range - min_range) / notes_num;

			if (!quiet) {
				time_t rawtime;
				struct tm * timeinfo;
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );
				printf ( "Time %s, set min range %0.6f\n", asctime (timeinfo), min_range);
			}
		} else if (!button_1_state) {
			max_range = getRange(range_fifo);
			range_step = (max_range - min_range) / notes_num;

			if (!quiet) {
				time_t rawtime;
				struct tm * timeinfo;
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );
				printf ( "Time %s, set max range %0.6f\n", asctime (timeinfo), max_range);
			}

		} else {

			range = getRange(range_fifo);
			if (range < 0) {
				return -1;
			}

			char note_str[3];
			//note_fd = open(note_fifo, O_WRONLY);

			if (range >= max_range) {
				sprintf(note_str, "A#");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else if (range >= (max_range - range_step)) {
				sprintf(note_str, "A");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else if (range >= (max_range - (2 * range_step))) {
				sprintf(note_str, "B");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else if (range >= (max_range - (3 * range_step))) {
				sprintf(note_str, "C#");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else if (range >= (max_range - (4 * range_step))) {
				sprintf(note_str, "C");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else if (range >= (max_range - (5 * range_step))) {
				sprintf(note_str, "D#");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else if (range >= (max_range - (6 * range_step))) {
				sprintf(note_str, "D");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else if ((range >= (max_range - (7 * range_step)))) {
				sprintf(note_str, "E");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else if (range >= (max_range - (8 * range_step))) {
				sprintf(note_str, "F#");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else	if (range >= (max_range - (9 * range_step))) {
				sprintf(note_str, "F");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else	if (range >= (max_range - (10 * range_step))) {
				sprintf(note_str, "G#");
				//write(note_fd, note_str, strlen(note_str) + 1);
			} else	if (range < (max_range - (10 * range_step))) {
				sprintf(note_str, "G");
				//write(note_fd, note_str, strlen(note_str) + 1);
			}
			//close(note_fd);
			if (setNote(note_fifo, note_str) < 0) {
				printf("Failed set note\n");
				exit(-1);
			}
			if (!quiet) {
				time_t rawtime;
				struct tm * timeinfo;
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );
				printf ( "Time %s, note %s, volume %d\n", asctime (timeinfo), note_str, volume);
			}
			sleep(1);	// duration of note .wav
		}
	}

	return 0;
}

void help() {
	printf("    Use this application for playing termenwoks\n");
	printf("    execute format: ./combiner RANGE_FIFO_NAME PLAY_NOTE_FIFO_NAME BUTTON_0_FIFO BUTTON_1_FIFO\n");
	printf("    -h - help\n");
	printf("    -q - quiet\n");
	printf("    RANGE_FIFO_NAME - name of created fifo rangefinder output\n");
	printf("    PLAY_NOTE_FIFO_NAME - name of created fifo play_note input\n");
	printf("    BUTTON_0_FIFO/BUTTON_1_FIFO - name of created fifos for buttons output\n");
}
