/*
 * 
 * (c) Peter Hällman 2022
 * This code is licensed under MIT license (see LICENSE.txt for details)
 */
#include "beep2mqtt.h"

// one global is allowed?
ops *op; //holds options from config and other housekeeping vars
int pstrcpy(char **dest, char *src)
{
	int len;
	if (*dest != NULL)
		free(*dest);
	len = strlen(src);
	if ((*dest = malloc(len + 1)) == NULL)
		EE("Malloc error");
	memset(*dest, 0, len + 1);
	strcpy(*dest, src);
	return len;
}
void print_usage(char **argv)
{

	printf("Usage:\n %s [options] \n\n", argv[0]);
	printf("Options:\n");
	printf("-c --configuration=<file>\trequired json configuration file\n");
	printf("-v --verbose\t\t\tprint detection events\n\n");
}
void parse_ops(int argc, char **argv, char **cnf_filename)
{

	int c;
	while (1)
	{
		static struct option long_options[] =
			{
				{"configuration", 1, 0, 'c'},
				{"verbose", 0, 0, 'v'},
				{0, 0, 0, 0}
			};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long(argc, argv, "c:v",
						long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
		case 'c':
			pstrcpy(cnf_filename, optarg);
			break;

		case 'v':
			op->verbose=1;
			break;
		default:
			print_usage(argv);
			exit(EXIT_SUCCESS);
		}
	}
	//did we get required option?
	if(!*cnf_filename)
	{
		print_usage(argv);
		exit(EXIT_SUCCESS);


	}
}
void load_config(char *filename)
{
	FILE *infile;
	struct stat s;
	char *filebuf;
	cJSON *config;
	char *error_ptr;

	if ((infile = fopen(filename, "r")) == NULL)
		EE("Unable to open %s", filename);
	if (fstat(fileno(infile), &s))
		EE("Unable to stat %s", filename);

	filebuf = malloc(s.st_size);
	memset(filebuf, 0, s.st_size);
	if (fread(filebuf, sizeof(char), s.st_size, infile) < s.st_size)
		EE("Read error reading json file");
	fclose(infile);

	if (!(config = cJSON_Parse(filebuf)))
	{
		error_ptr = (char *)cJSON_GetErrorPtr();
		EE("Json parse error before : %s", error_ptr);
	}

	cJSON *read_siz_j = cJSON_GetObjectItemCaseSensitive(config, "read_siz");
	if (!cJSON_IsNumber(read_siz_j))
		EE("Expected number for item 'read_siz'");
	op->read_siz = read_siz_j->valueint;

	cJSON *sample_rate_j = cJSON_GetObjectItemCaseSensitive(config, "sample_rate");
	if (!cJSON_IsNumber(sample_rate_j))
		EE("Expected number for item 'sample_rate'");
	op->sample_rate = sample_rate_j->valueint;

	cJSON *boi_low_j = cJSON_GetObjectItemCaseSensitive(config, "boi_low");
	if (!cJSON_IsNumber(boi_low_j))
		EE("Expected number for item 'boi_low'");
	op->boi_low = boi_low_j->valueint;

	cJSON *boi_high_j = cJSON_GetObjectItemCaseSensitive(config, "boi_high");
	if (!cJSON_IsNumber(boi_high_j))
		EE("Expected number for item 'boi_high'");
	op->boi_high = boi_high_j->valueint;

	cJSON *boi_mag_j = cJSON_GetObjectItemCaseSensitive(config, "boi_mag");
	if (!cJSON_IsNumber(boi_mag_j))
		EE("Expected number for item 'boi_mag'");
	op->boi_mag = boi_mag_j->valueint;

	cJSON *ttl_j = cJSON_GetObjectItemCaseSensitive(config, "ttl");
	if (!cJSON_IsNumber(ttl_j))
		EE("Expected number for item 'ttl'");
	op->ttl = ttl_j->valueint;

	cJSON *hit_trig_j = cJSON_GetObjectItemCaseSensitive(config, "hit_trig");
	if (!cJSON_IsNumber(hit_trig_j))
		EE("Expected number for item 'hit_trig'");
	op->hit_trig = hit_trig_j->valueint;

	cJSON *mqtt_tele_ival_j = cJSON_GetObjectItemCaseSensitive(config, "mqtt_tele_ival");
	if (!cJSON_IsNumber(mqtt_tele_ival_j))
		EE("Expected number for item 'mqtt_tele_ival'");
	op->mqtt_tele_ival = mqtt_tele_ival_j->valueint;

	cJSON *device_j = cJSON_GetObjectItemCaseSensitive(config, "device");
	if (!cJSON_IsString(device_j) || device_j->valuestring == NULL)
		EE("Expected string for item 'device'");
	pstrcpy(&op->device, device_j->valuestring);

	cJSON *mqtt_srv_j = cJSON_GetObjectItemCaseSensitive(config, "mqtt_srv");
	if (!cJSON_IsString(mqtt_srv_j) || mqtt_srv_j->valuestring == NULL)
		EE("Expected string for item 'mqtt_srv'");
	pstrcpy(&op->mqtt_srv, mqtt_srv_j->valuestring);

	cJSON *mqtt_pw_j = cJSON_GetObjectItemCaseSensitive(config, "mqtt_pw");
	if (!cJSON_IsString(mqtt_pw_j) || mqtt_pw_j->valuestring == NULL)
		EE("Expected string for item 'mqtt_pw'");
	pstrcpy(&op->mqtt_pw, mqtt_pw_j->valuestring);

	cJSON *mqtt_user_j = cJSON_GetObjectItemCaseSensitive(config, "mqtt_user");
	if (!cJSON_IsString(mqtt_user_j) || mqtt_user_j->valuestring == NULL)
		EE("Expected string for item 'mqtt_user'");
	pstrcpy(&op->mqtt_user, mqtt_user_j->valuestring);

	cJSON *mqtt_topic_j = cJSON_GetObjectItemCaseSensitive(config, "mqtt_topic");
	if (!cJSON_IsString(mqtt_topic_j) || mqtt_topic_j->valuestring == NULL)
		EE("Expected string for item 'mqtt_topic'");
	pstrcpy(&op->mqtt_topic, mqtt_topic_j->valuestring);

	cJSON *mqtt_on_msg_j = cJSON_GetObjectItemCaseSensitive(config, "mqtt_on_msg");
	if (!cJSON_IsString(mqtt_on_msg_j) || mqtt_on_msg_j->valuestring == NULL)
		EE("Expected string for item 'mqtt_on_msg'");
	pstrcpy(&op->mqtt_on_msg, mqtt_on_msg_j->valuestring);

	cJSON *mqtt_off_msg_j = cJSON_GetObjectItemCaseSensitive(config, "mqtt_off_msg");
	if (!cJSON_IsString(mqtt_off_msg_j) || mqtt_off_msg_j->valuestring == NULL)
		EE("Expected string for item 'mqtt_off_msg'");
	pstrcpy(&op->mqtt_off_msg, mqtt_off_msg_j->valuestring);

	cJSON *mode_j = cJSON_GetObjectItemCaseSensitive(config, "mode");
	if (!cJSON_IsString(mode_j) || mode_j->valuestring == NULL)
		EE("Expected string for item 'mode'");
	if (!strcmp("sense", mode_j->valuestring))
		op->mode = SENSE;
	else if (!strcmp("sample", mode_j->valuestring))
		op->mode = SAMPLE;
	else if (!strcmp("write", mode_j->valuestring))
		op->mode = WRITE;
	else
		EE("Unknown mode:%s\n misspelled or whitespace?", mode_j->valuestring);
	cJSON_Delete(config);
	free(filebuf);
}

// removes  elements if ttl expired

void clean_hitlist(struct hit_list *hlist, int ttl)
{
	int i;
	if (!hlist->hits)
		return; // list was clean, do nothing
	// check first and oldest timestamp
	if (hlist->hit_time[0] + ttl < time(NULL))
	{
		for (i = 0; i < hlist->hits - 1; i++)
		{
			hlist->hit_time[i] = hlist->hit_time[i + 1];
		}
		hlist->hits--;
		clean_hitlist(hlist, ttl);
	}
	return;
}
// runs clean_hitlist of old entries then
// adds a hit and timestamp
// returns true if hit_trig is met
int add_hitlist(struct hit_list *hlist)
{
	int i;

	clean_hitlist(hlist, op->ttl);
	hlist->hits++;
	hlist->hit_time[hlist->hits - 1] = time(NULL);

	if (hlist->hits < op->hit_trig)
	{
		return 0;
	}
	if (hlist->hits == op->hit_trig)
		return 1;
	if (hlist->hits == op->hit_trig + 1)
	{ // don't increase, add new time and
		// discard oldest, decrease hits
		for (i = 0; i < op->hit_trig; i++)
		{
			hlist->hit_time[i] = hlist->hit_time[i + 1];
		}
		hlist->hit_time[op->hit_trig - 1] = time(NULL);
		hlist->hits--;
		return 1;
	}
	else
		return 0;
}
//used by qsort when running 'sense'
static int compare_bucket_list(const void *p1, const void *p2)
{
	struct bucket_list b_a, b_b;
	b_a = *((struct bucket_list *)p1);
	b_b = *((struct bucket_list *)p2);
	if (b_a.magnitude == b_b.magnitude)
		return 0;
	else if (b_a.magnitude > b_b.magnitude)
		return -1;
	else
		return 1;
}
//mqtt callback
int msg_arrived(void *context, char *topi, int topic_len, MQTTAsync_message *msg)
{
	// nothing yet
	// if implemented test if library still
	// loses subscriptions after reconnect
	return 0;
}
//mqtt callback
void cb_con_fail(void *context, MQTTAsync_failureData *response)
{

	EE("MQTT connection failure");
}
//mqtt callback
void cb_con_success(void *context, MQTTAsync_successData *response)
{
	if(op->verbose)
		printf("mqtt connected\n");
	return;
}
// sends msg in op->mqtt_topic
void send_mqtt(MQTTAsync *mq_client, char *msg)
{
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

	opts.context = mq_client;
	opts.onSuccess = cb_con_success;
	opts.onFailure = cb_con_fail;

	pubmsg.payload = msg;
	pubmsg.payloadlen = strlen(msg);
	pubmsg.qos = 0;
	pubmsg.retained = 0;
	MQTTAsync_sendMessage(*mq_client, op->mqtt_topic, &pubmsg, &opts);
}
// what the name says
void setup_mqtt(MQTTAsync *mq_client, MQTTAsync_connectOptions *mq_opts)
{
	int mq_res;
	if ((mq_res = MQTTAsync_create(mq_client, op->mqtt_srv, "smokey", MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
		EE("Cannot create mqtt object, %d\n", mq_res);

	if ((mq_res = MQTTAsync_setCallbacks(*mq_client, NULL, NULL, msg_arrived, NULL)) != MQTTASYNC_SUCCESS)
		EE("Cannot create mqtt callbacks, %d\n", mq_res);

	mq_opts->keepAliveInterval = 20;
	mq_opts->cleansession = 1;
	mq_opts->onSuccess = cb_con_success;
	mq_opts->onFailure = cb_con_fail;
	mq_opts->context = *mq_client;
	mq_opts->automaticReconnect = 1;
	mq_opts->username = op->mqtt_user;
	mq_opts->password = op->mqtt_pw;

	if ((mq_res = MQTTAsync_connect(*mq_client, mq_opts)) != MQTTASYNC_SUCCESS)
		EE("Failed to connect, %d\n", mq_res); // this does not get called, wierd
}
// setup soundcard for capture
int setup_scap(snd_pcm_t **capture_handle, snd_pcm_uframes_t *frames, char *device, int mode)
{
	snd_pcm_hw_params_t *hw_params;
	int err;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; // signed 16-bit little endian
	unsigned int rate = 44100;
	int dir; // not clear why needed

	if ((err = snd_pcm_open(capture_handle, device, SND_PCM_STREAM_CAPTURE, 0)))
		EE("cannot open audio device %s (%s)\n", device, snd_strerror(err));

	if ((err = snd_pcm_hw_params_malloc(&hw_params)))
		EE("cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));

	if ((err = snd_pcm_hw_params_any(*capture_handle, hw_params)))
		EE("cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));

	if ((err = snd_pcm_hw_params_set_access(*capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)))
		EE("cannot set access type (%s)\n", snd_strerror(err));

	if ((err = snd_pcm_hw_params_set_format(*capture_handle, hw_params, format)))
		EE("cannot set sample format (%s)\n", snd_strerror(err));

	if ((err = snd_pcm_hw_params_set_rate_near(*capture_handle, hw_params, &rate, 0)))
		EE("cannot set sample rate (%s)\n", snd_strerror(err));

	if (mode == STEREO)
	{
		if ((err = snd_pcm_hw_params_set_channels(*capture_handle, hw_params, 2)))
		{
			EW("cannot set channel count (%s)\n", snd_strerror(err));
			snd_pcm_close(*capture_handle);
			return err;
		}
	}
	else
	{
		if ((err = snd_pcm_hw_params_set_channels(*capture_handle, hw_params, 1)))
		{
			EW("cannot set channel count (%s)\n", snd_strerror(err));
			EW("will try stereo, might still work \n");
			snd_pcm_close(*capture_handle);
			return err;
		}
	}

	if ((err = snd_pcm_hw_params(*capture_handle, hw_params)))
		EE("cannot set parameters (%s)\n", snd_strerror(err));

	if ((err = snd_pcm_prepare(*capture_handle)))
		EE("cannot prepare audio interface for use (%s)\n", snd_strerror(err));
	// get size of a capture frame in frames, which should have been named 'frame'
	// and that I end up not using because some cards give a non pow 2 value
	snd_pcm_hw_params_get_period_size(hw_params, frames, &dir);

	snd_pcm_hw_params_get_period_time(hw_params, &rate, &dir); //  rate to period time in us
	snd_pcm_hw_params_free(hw_params);
	return 0;
}
// only used for testing, dump raw sound to disk
void write_raw(snd_pcm_t *cap_hand, unsigned char *buf, int buf_siz, int mode, int fd)
{

	int err, i;

	int16_t *wbufp;
	int16_t *wbuf;
	wbuf = malloc(sizeof(int16_t) * op->read_siz);
	wbufp = wbuf;
	// get READ_SIZ number of frames

	err = snd_pcm_readi(cap_hand, buf, op->read_siz);
	if (err == -EPIPE)
	{
		/* EPIPE == overrun */
		EW("soundcard overrun occurred\n");
		snd_pcm_prepare(cap_hand);
	}
	else if (err < 0)
		EW("soundcard error from read: %s\n", snd_strerror(err));

	else if (err != op->read_siz)
		EW("soundcard short read, read %d frames\n", err);

	for (i = 0; i < buf_siz; i++)
	{
		*wbufp = (int16_t)(*(buf + i) | (*(buf + i + 1) << 8));
		wbufp++;
		i = i + mode; // STEREO = + 2, MONO = + 0
		i++;
	}

	err = write(fd, (unsigned char *)wbuf, sizeof(int16_t) * op->read_siz);
	free(wbuf);
}
// get a list of higest bucket, for initial config to detect boi (bucket of interest)
// a lot of noise in low buckets from fftw though..
void sense(snd_pcm_t *cap_hand, double *in,
		   fftw_complex *out, fftw_plan *plan, unsigned char *buf, int buf_siz, int mode)
{

	int err, i;
	double *inp;
	float hz_per_bin;
	struct bucket_list *magnitudes;
	struct bucket_list *magp;
	magnitudes = malloc(sizeof(struct bucket_list) * op->read_siz / 2);

	magp = magnitudes;
	hz_per_bin = (float)op->sample_rate / op->read_siz;
	// get READ_SIZ number of frames

	err = snd_pcm_readi(cap_hand, buf, op->read_siz);
	if (err == -EPIPE)
	{
		/* EPIPE == overrun */
		EW("soundcard overrun occurred\n");
		snd_pcm_prepare(cap_hand);
	}
	else if (err < 0)
		EW("soundcard error from read: %s\n", snd_strerror(err));

	else if (err != op->read_siz)
		EW("soundcard short read, read %d frames\n", err);

	/*
	 * Skip every other 16 bits to make mono if stereo
	 * Pack two bytes into a signed 16 bit int
	 * before loading into double for fft
	 *
	 */
	inp = in;
	for (i = 0; i < buf_siz; i++)
	{
		*inp = (double)(int16_t)(*(buf + i) | (*(buf + i + 1) << 8));
		inp++;
		i = i + mode; // STEREO = + 2, MONO = + 0
		i++;
	}
	fftw_execute(*plan);
	for (i = 1; i < op->read_siz / 2; i++) // index 0 is DC, not interesting
	{
		magp->magnitude = (double)sqrt(creal(out[i]) * creal(out[i]) +
									   cimag(out[i] * cimag(out[i])));
		magp->bucket = i;
		magp++;
	}
	qsort(magnitudes, op->read_siz / 2, sizeof(struct bucket_list), compare_bucket_list);
	magp = magnitudes;
	//CLRSCR(); 
	// This could be done a bit better.. 
	for (i = 0; i < 15; i++)
	{
		printf("[%d] %.1f  %.1fHz\n", magp->bucket, magp->magnitude, (float)(magp->bucket * hz_per_bin));
		magp++;
	}

	free(magnitudes);
}
/*
* The workhorse of the application
* reads op->read_siz bytes from soundcard
* 
* Transforms to frequency plane and checks if
* the frequency in boi_low to boi_high exists
* with enough magnitude. If so, return magnitude
*/
int sample_fft(snd_pcm_t *cap_hand, double *in,
			   fftw_complex *out, fftw_plan *plan, unsigned char *buf, int buf_siz, int mode)
{
	int err, i;
	double *inp;
	double magnitude;
	// get READ_SIZ number of frames

	err = snd_pcm_readi(cap_hand, buf, op->read_siz);
	if (err == -EPIPE)
	{
		/* EPIPE == overrun */
		EW("soundcard overrun occurred\n");
		snd_pcm_prepare(cap_hand);
	}
	else if (err < 0)
		EW("soundcard error from read: %s\n", snd_strerror(err));

	else if (err != op->read_siz)
		EW("soundcard short read, read %d frames\n", err);

	/*
	 * Skip every other 16 bits to make mono is stereo
	 * Pack two bytes into a signed 16 bit int
	 * before loading into double
	 *
	 */
	inp = in;
	for (i = 0; i < buf_siz; i++)
	{
		*inp = (double)(int16_t)(*(buf + i) | (*(buf + i + 1) << 8));
		inp++;
		i = i + mode; // STEREO = + 2, MONO = + 0
		i++;
	}
	fftw_execute(*plan);
	for (i = 1; i < op->read_siz / 2; i++) // index 0 is DC, not interesting
	{   //get back into real numbers from complex
		magnitude = (double)sqrt(creal(out[i]) * creal(out[i]) +
								 cimag(out[i] * cimag(out[i])));
		// Anyting of interest in the real numbers?
		if (magnitude > op->boi_mag && (i >= op->boi_low && i <= op->boi_high))
			return magnitude;
	}
	return 0;
}
// eternity loop function
void do_sample(snd_pcm_t *capture_handle, double *in, fftw_complex *out,
			   fftw_plan *plan, unsigned char *buffer, int buf_siz, int soundmode)
{
	double hit_mag;
	struct hit_list hitlist;
	// prepare hitlist
	hitlist.hit_time = malloc(sizeof(time_t) * (op->hit_trig + 1)); // one extra needed in add function
	hitlist.hits = 0;

	while (1)
	{
		if ((hit_mag = sample_fft(capture_handle, in, out, plan, buffer, buf_siz, soundmode)))
		{
			if (add_hitlist(&hitlist))
				op->alarm = 1;

			else
			{
				if(op->verbose)
					printf("Detection: magnitude %.0f   total in queue %d\n", hit_mag, hitlist.hits);
			}
		}
		clean_hitlist(&hitlist, op->ttl); // needed for 'alarm' to timeout
		if (hitlist.hits < op->hit_trig)
			op->alarm = 0;
	}
}
// eternity loop function
void do_sense(snd_pcm_t *capture_handle, double *in, fftw_complex *out,
			  fftw_plan *plan, unsigned char *buffer, int buf_siz, int soundmode)
{
	fprintf(stderr, "Starting sense, writing to stdout\n");
	while (1)
	{

		sense(capture_handle, in, out, plan, buffer, buf_siz, soundmode);
	}
}
// eternity loop function
void do_write_raw(snd_pcm_t *capture_handle, unsigned char *buffer, int buf_siz, int soundmode)
{
	int fd;
	if ((fd = open("testsound.pcm", O_WRONLY | O_CREAT | O_TRUNC, (S_IRUSR | S_IWUSR))) < 0)
		EE("create file error");
	while (1)
	{
		write_raw(capture_handle, buffer, buf_siz, soundmode, fd);
	}
}
// runs from its own thread
// handles mqtt in a loop
void *do_mqtt(void *args) {
	int seconds = 0;
	int state = 0;
	MQTTAsync mq_client;
	MQTTAsync_connectOptions mq_con_ops = MQTTAsync_connectOptions_initializer;
	setup_mqtt(&mq_client, &mq_con_ops);

	while (1)
	{
		if (op->alarm == 1 && (state == 0)) // alarm just on
		{
			send_mqtt(&mq_client, op->mqtt_on_msg);
			state = 1;
		}
		else if (op->alarm == 0 && state == 1) // alarm just off
		{
			send_mqtt(&mq_client, op->mqtt_off_msg);
			state = 0;
		}

		// send periodically
		if (seconds == op->mqtt_tele_ival)
		{
			if (state) // alarm is on
			{
				send_mqtt(&mq_client, op->mqtt_on_msg);
				seconds = 0;
			}
			else
			{
				send_mqtt(&mq_client, op->mqtt_off_msg);
				seconds = 0;
			}
		}
		sleep(1);
		seconds++;
	}
}
int main(int argc, char *argv[])
{
	int buf_siz;
	int soundmode = MONO;
	double *in;			   // inbuffer for fft
	unsigned char *buffer; // Holds soundsamples
	char *cnf_filename=NULL; //configuration file filename
	snd_pcm_t *capture_handle; // 
	snd_pcm_uframes_t frames;
	fftw_complex *out; //out buffer after fft
	fftw_plan plan; 
	pthread_t t1; // thread messing with mqtt

	// get memory for options
	op = malloc(sizeof(ops));
	memset(op, 0, sizeof(ops));
	// get command line options
	parse_ops(argc, argv, &cnf_filename);
	// load conf.json into op, global pointer
	load_config(cnf_filename);
	free(cnf_filename);

	//  prepare sound device, try mono, if fails try stereo
	//  setup_scap return 0 on success
	if (setup_scap(&capture_handle, &frames, op->device, MONO))
	{
		if (setup_scap(&capture_handle, &frames, op->device, STEREO))
			EE("unable to setup stereo or mono channels");
		soundmode = STEREO;
	}
	// Allocate buffer for audio
	if (soundmode)					// MONO == 0 STEREO == 2
		buf_siz = op->read_siz * 4; // 2bytes per sample, 2 channels in stereo
	else
		buf_siz = op->read_siz * 2; // 2bytes per sample

	buffer = malloc(buf_siz);
	/*
	 * Setup FFTW
	 */
	in = malloc(sizeof(double) * op->read_siz);
	out = (fftw_complex *)fftw_malloc((sizeof(fftw_complex) * op->read_siz / 2) + 1); // library discard after nyqvist
	plan = fftw_plan_dft_r2c_1d(op->read_siz, in, out, FFTW_MEASURE);

	// lose 'pop', discard first frame
	snd_pcm_readi(capture_handle, buffer, frames);

	switch (op->mode)
	{
	case SAMPLE:
		pthread_create(&t1, NULL, do_mqtt, NULL);
		do_sample(capture_handle, in, out, &plan, buffer, buf_siz, soundmode);
		break;
	case SENSE:
		do_sense(capture_handle, in, out, &plan, buffer, buf_siz, soundmode);
		break;
	case WRITE:
		do_write_raw(capture_handle, buffer, buf_siz, soundmode);
		break;
	default:
		EE("Unknown mode");
	}

	return 0;
}