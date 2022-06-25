#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <MQTTAsync.h>
#include "err_exit.h"
#include "cJSON.h"
#include <pthread.h>
#include <getopt.h>
#define MONO 0
#define STEREO 2

#define SAMPLE 1
#define SENSE 2
#define WRITE 3
#define CLRSCR() printf("\e[1;1H\e[2J")
typedef struct ops
{
    char *device;
    char *mqtt_srv;
    char *mqtt_pw;
    char *mqtt_user;
    char *mqtt_topic;
    char *mqtt_on_msg;
    char *mqtt_off_msg;
    int mqtt_tele_ival;
    int read_siz;
    int sample_rate;
    int hz_per_bin;
    int boi_low;
    int boi_high;
    int boi_mag;
    int ttl;
    int mode;
    int hit_trig;
    int alarm;
    int verbose;

} ops;
struct bucket_list
{
    double magnitude;
    int bucket;
};
struct hit_list
{
    time_t *hit_time;
    int hits;
};
void print_usage(char **);
void parse_ops(int argc, char **, char **);
void load_config(char *);
void clean_hitlist(struct hit_list *, int);
int add_hitlist(struct hit_list *);
static int compare_bucket_list(const void *, const void *);
int msg_arrived(void *, char *, int , MQTTAsync_message *);
void cb_con_fail(void * , MQTTAsync_failureData *);
void cb_con_success(void *, MQTTAsync_successData *);
void send_mqtt(MQTTAsync *, char *);
void setup_mqtt(MQTTAsync *, MQTTAsync_connectOptions *);
int setup_scap(snd_pcm_t **, snd_pcm_uframes_t *, char *, int);
void write_raw(snd_pcm_t *, unsigned char *, int, int, int);
void sense(snd_pcm_t *, double *,fftw_complex *, fftw_plan *, unsigned char *, int, int);
int sample_fft(snd_pcm_t *, double *,fftw_complex *, fftw_plan *, unsigned char *, int , int);
void do_sample(snd_pcm_t *, double *, fftw_complex *,fftw_plan *, unsigned char *, int, int);
void do_sense(snd_pcm_t *, double *, fftw_complex *,fftw_plan *, unsigned char *, int, int);
void do_write_raw(snd_pcm_t *, unsigned char *, int, int);
void *do_mqtt(void *);

