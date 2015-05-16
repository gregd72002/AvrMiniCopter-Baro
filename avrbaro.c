#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <getopt.h>

#include "bmpsensor/bmp180.h"
#include "ms5611/ms5611.h"

#include "routines.h"
#include "average.h"
#include "avrbaro.h"

int ret;
int background = 0;
int stop = 0;
int state = 0;
int type = 0;
struct s_average average;
struct s_baro baro;
uint8_t baro_address = 0x77;

int sock = 0,len;
char sock_path[256] = "/dev/avrspi";
struct sockaddr_un address;
unsigned char buf[256];

int verbose = 0;
int loopDelay;


float altitude(float P, float P0)
        // Given a pressure measurement P (mb) and the pressure at a baseline P0 (mb),
        // return altitude (meters) above baseline.
{
        return round(10.0f*44330.0f*(1.0f-pow(P/P0,1.0f/5.255f)))/10.0f;
}


int sendMsg(int t, int v) {
	static struct local_msg m;
	int count = 0;
	m.c = 0;
	m.t = t;
	m.v = v;
	pack_lm(buf,&m);
	ret = write(sock,buf,LOCAL_MSG_SIZE);
	if (ret<=0) {
		perror("AVRBARO: writing");
		return -1;
	}

	if (ioctl(sock, TIOCINQ, &count)!=0) {
		printf("Ioctl failed.\n");
		return -1;
	}

	if (count) { //read any available data
		ret = read(sock,buf,256);
		if (ret>0)
			switch(buf[0]) {
				case 1:
					if (verbose) printf("disconnect request\n");
					close(sock);
					state = 0;
					break;
				case 2:
					if (verbose) printf("reset request\n");
					state = 1;
					break;
			}

		if (verbose==2) {
			printf("Received %i bytes: ",ret);
			for (int i=0;i<ret;i++) printf("%i ",buf[i]);
			printf("\n");
		}
	}

	return 0;
}

void catch_signal(int sig)
{
	printf("AVRBARO signal: %i\n",sig);
	stop = 1;
}

long dt_ms = 0;
static struct timespec ts,t1,t2,*dt;

unsigned long c = 0,k = 0;

int init() {
	if (verbose>0) printf("AVRBARO: Initialization... %02x\n",baro_address);
	switch (type) {
		case 0: ret=bs_open(baro_address); break;
		case 1: ret=ms_open(baro_address); break;
	}

	if (ret<0) {
		printf("AVRBARO: Failed to initiate pressure sensor! [%s]\n", strerror(ret));
		return -1;
	}

	float p0 = 0.f;
	for (int i=0;i<15;i++) {
		switch (type) {
			case 0: bs_update(&baro); break;
			case 1: ms_update(&baro); break;
		}

		if (verbose==2) printf("T: %2.2f\tP: %2.2f\n",baro.t,baro.p);

		if (i>=5) p0 += baro.p;	//discard first 5 readings
	}
	baro.p0 = p0/10.f;

	if (verbose>0) printf("AVRBARO: Initialization successful!\n");

	return 0;
}

int setup() {
	if (verbose) printf("Connecting to socket\n");
	sock=socket(AF_UNIX,SOCK_STREAM,0);
	if (sock<0) {
		perror("AVRBARO: openning unix socket");
		return -1;
	}

	bzero(&address,sizeof(address));
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, sock_path);
	len = strlen(address.sun_path) + sizeof(address.sun_family);
	if (connect(sock, (struct sockaddr *) &address, len) < 0) {
		close(sock);
		printf("%s\n",sock_path);
		perror("connecting socket");
		printf("Check if avrspi is running\n");
		return -1;
	}

	buf[0] = 1;
	if (write(sock,buf,1)!=1) {
		perror("Sending");
		return -1;
	}

	if (verbose) printf("Connected.\n");

	return 0;
}

void readData() {
	int ret;

	do {
		switch (type) {
			case 0: ret = bs_update(&baro); break;
			case 1: ret = ms_update(&baro); break;
		}
		if (ret==0) {
			baro.alt = altitude(baro.p, baro.p0);
			average_add(&average,baro.alt*100.f);
		} else { 
			if (verbose) printf("AVRBARO: Barometer reading error!\n");
			state = 1;
			return;
		}

		clock_gettime(CLOCK_REALTIME,&t2);                                           
		dt = TimeSpecDiff(&t2,&t1);
		dt_ms = dt->tv_sec*1000 + dt->tv_nsec/1000000;


	} while (dt_ms<loopDelay);

	int alt = average_get(&average);
	if (sendMsg(14,alt)<0) {
		state = 0; 
		return;
	}

	if (verbose>=2) {
		alt = baro.alt * 100.f;
		printf("T: %2.2f\tAlt: %i\tP: %2.2f\n",baro.t,alt,baro.p);
	}
}

void loop() {
	int ret;
	clock_gettime(CLOCK_REALTIME,&t2);                                           
	ts = t1 = t2;
	loopDelay = 500; 
	if (verbose) printf("AVRBARO: Starting main loop...\n");
	while (!stop) {
		clock_gettime(CLOCK_REALTIME,&t2);                                           
		dt = TimeSpecDiff(&t2,&t1);
		dt_ms = dt->tv_sec*1000 + dt->tv_nsec/1000000;
		if (dt_ms<loopDelay) {
			mssleep(loopDelay-dt_ms);
			continue; //do not flood AVR with data - will cause only problems; each loop sends 4 msg; 50ms should be enough for AVR to consume them
		}
		t1 = t2;

		switch (state) {
			case 0: 
				loopDelay = 500;
				ret = setup();
				if (ret == 0) state = 1;
				break;
			case 1:
				loopDelay = 500;
				ret = init();
				if (ret == 0) state = 2;
				break; 
			case 2:
				loopDelay = 100;
				readData();
				break;

		}
	}
}

void print_usage() {
	printf("-b - run in the background\n");
	printf("-v [level] - verbose mode\n");
	printf("-u [SOCKET] - socket to connect to (defaults to %s)\n",sock_path);
	printf("-t [type] - baro type; 0 for bmp085; 1 for ms5611\n");
	printf("-a [address] - baro address; (defaults to %02x)\n",baro_address);
}

int main(int argc, char **argv) {

	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);

	int option;
	verbose = 0;
	type = 0;
	while ((option = getopt(argc, argv,"bv:a:p:t:")) != -1) {
		switch (option)  {
			case 'a': baro_address = strtol(optarg,NULL,16); break;
			case 'b': background = 1; break;
			case 'v': verbose=atoi(optarg); break;
			case 'u': strcpy(sock_path,optarg); break;
			case 't': type=atoi(optarg); break;
			default:
				  print_usage();
				  return -1;
		}
	}

	if (background) {
		if (daemon(0,1) < 0) {
			perror("AVRBARO: daemon");
			return -1;
		}
		if (verbose) printf("AVRBARO: Running in the background\n");
	}

	average_init(&average,3);

	loop();

	average_destroy(&average);
	if (verbose) printf("AVRBARO Closing.\n");
	close(sock);


	return 0;
}

