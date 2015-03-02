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

#include "routines.h"


int ret;
int background = 0;
int stop = 0;
int bs_err = 0;
int initialized = 0;

int sock = 0,len;
char sock_path[256] = "/dev/avrspi";
struct sockaddr_un address;
unsigned char buf[256];

int verbose = 0;
int delay;

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
		stop = 1;
	}

	if (ioctl(sock, TIOCINQ, &count)!=0) {
		printf("Ioctl failed.\n");
		stop = 1;
	}

	if (count) { //read any available data
		ret = read(sock,buf,256);
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

void init() {
	if (verbose>0) printf("AVRBARO: Initialization...\n");
	ret=bs_open();
	if (ret<0) {
		printf("AVRBARO: Failed to initiate pressure sensor! [%s]\n", strerror(ret));
		return;
	}

	if (verbose>0) printf("AVRBARO: Initialization successful!\n");
	initialized = 1;

	mssleep(100);
}

int setup() {
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
	
	return 0;
}

void loop() {
	int ret;
	clock_gettime(CLOCK_REALTIME,&t2);                                           
	ts = t1 = t2;
	delay = 20;
	int alt = 0;
	int alt_c = 0;
	if (verbose) printf("AVRBARO: Starting main loop...\n");
	while (!stop) {
		clock_gettime(CLOCK_REALTIME,&t2);                                           
		dt = TimeSpecDiff(&t2,&t1);
		dt_ms = dt->tv_sec*1000 + dt->tv_nsec/1000000;

		if (dt_ms<delay) {
			mssleep(delay-dt_ms);
			continue; //do not flood AVR with data - will cause only problems; each loop sends 4 msg; 50ms should be enough for AVR to consume them
		}
		t1 = t2;
		if (!initialized) {
			init();
		} else { //initialized 
			c++;

			ret = bs_update(c*dt_ms);
			if (ret>=0) {
				if (ret == 1) { //pressure updated
					alt += (bs.alt*100.f);
					alt_c++;
					if (alt_c == 3) {
						sendMsg(14,alt/alt_c); //in cm
						if (verbose>=2)
							printf("T: %2.2f\tAlt: %i\tP: %2.2f\n",bs.t,alt/alt_c,bs.p);
						alt_c = 0;
						alt = 0;
					}
				}
				else if (ret == 2) c = 0; //end of cycle
			} else 
				if (verbose) printf("AVRBARO: Barometer reading error!\n");
		}
	}
}

void print_usage() {
	printf("-b - run in the background\n");
	printf("-v [level] - verbose mode\n");
	printf("-u [SOCKET] - socket to connect to (defaults to %s)\n",sock_path);
}

int main(int argc, char **argv) {

	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);

	int option;
	verbose = 0;
	while ((option = getopt(argc, argv,"bv:a:p:")) != -1) {
		switch (option)  {
			case 'b': background = 1; break;
			case 'v': verbose=atoi(optarg); break;
			case 'u': strcpy(sock_path,optarg); break;
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

	ret = setup();
	if (ret<0) return 0;

	loop();
	if (verbose) printf("AVRBARO Closing.\n");
	close(sock);

	return 0;
}

