# beep2mqtt
This program listens for a range or specific frequency from a microphone. 
When this frequency is detected above a level and during a set time, a message is sent through mqtt.

I wrote it to detect if the smoke detector went off in a auxillary building and trigger a response from Home Assistant.

Please do not rely on this software for anything relating to safety for humans or animals. 

I bought a cheap USB soundcard and a microphone and am running it on a raspberry pi. 

## Installation on a raspberry
This program requires a few libraries. 
libpaho-mqtt was not available on my distribution and had to be downloaded and compiled, your milage may vary. Just install libssl-dev, download libpaho-mqtt and run `make` and `make install` if not available.
Libpaho is found here: 

`git clone https://github.com/eclipse/paho.mqtt.c`


I am assuming a debian based distribution. Install prerequisites as root, if you are not root, prepend `sudo` to the commands below. Example from Ubuntu 21.10.

### Required libraries and tools
```
apt-get update
apt-get install build-essential
apt-get install fftw3-dev
apt-get install libasound-dev
apt-get install libpaho-mqtt-dev
apt-get install git
```
### Download and compile beep2mqtt
```
git clone https://github.com/Flurkmark/beep2mqtt.git
cd beep2mqtt
make
```
## Configuration file
beep2mqtt uses a json file for its configuration.
A sample file called `b2mqtt_conf.json' is provided.
It looks like this:
```
{
        "read_siz": 32768,
        "sample_rate" : 44100,
        "boi_low" : 2292,
        "boi_high" : 2294,
        "boi_mag" : 1000000,
        "hit_trig" : 5,
        "ttl" : 5,
        "device" : "hw:1,0",
        "mode" : "sense", 
        "mqtt_srv" : "tcp://127.0.0.1:1883",
        "mqtt_pw" : "password",
        "mqtt_user" : "username",
        "mqtt_topic" : "mancave/smoke",
        "mqtt_on_msg" : "smoke",
        "mqtt_off_msg" : "clear",
        "mqtt_tele_ival" : 60

}
```
### Individual fields
- `read_siz`
Number of frames read from soundcard each iteration. 
Default value with default sample rate corresponds to 743ms sample size. I found this value to be adequate. Value should be a power of 2.

- `sample_rate`
Number of sound sample points per second. Possible values are hardware dependent.

- `boi_low`
Low bucket of interest. Each sound sample is transformed into a array of frequencies. Each element in the array is a 'bucket'. With default sample rate and read size each bucket holds roughly 1.3hz.
This is the low threshold.

- `boi_high`
See above, high threshold. If you only want to sense one bucket, set `boi_low` and `boi_high` to the same value.
- `boi_mag` How 'loud' must the frequency be to trigger? Set high enough to avoid false positives.
- `hitlist_trig` To avoid false positives a boi must be present in several samples. This does not need to be consecutive. Instead each time a boi with proper magnitude is found it's added to a list together with a timestamp. When 'hitlist_trig' number of samples have been found and have not timed out, mqtt message is sent.
- `ttl` How many seconds each item in hitlist is valid. Do not set significantly lower than 'hit_trig' or you may never trigger mqtt send.
- `device` The alsa sounddevice name. This might take some trial and error. If on a desktop audacity will display each cards name. Otherwise just start from 0,0 and try 1,0, 2,0 and so on.
- `mode`
The program can run in three different modes.
  - `sample` mode is the standard mode of operation. The program takes its samples, checks for a boi and magnitude, send mqtt if conditions are met.  
  - `write` mode write dumps incoming audio to a file called `testsound.pcm`. Use this if you need to verity that the microphone and soundcard are working and providing a clean sound.
  - `sense` mode will print a list of the highest scoring buckets and its magnitude for each sample. Used to find your specific boi_low and boi_high and boi_mag for your specific use case. The default is what my cheap smoke detector happens to be. You are totally excused for using this mode while screaming, whistling and singing to view your beautiful frequencies.
- `mqtt_srv` IP address of your mqtt server.
- `mqtt_pw` mqtt password.
- `mqtt_user` mqtt username.
- `mqtt_topic` topic to publish on. 
- `mqtt_on_msg` Message to publish in topic if enough hits exist in hitlist, i.e. sound the alarm. 
- `mqtt_off_msg` Message to publish in topic when conditions are no longer met, i.e. alarm off.
- `mqtt_tele_ival` Send status periodically to keep for example Home Assistant happy.
## Figuring out buckets and magnitudes
### Find the buckets and magnitude
Download and compile as stated above. 
Set mode to 'sense' in config file. The program will print indefinitely until killed with for example ctrl-c. It takes a while to start due to some initializations. It will print a line when ready.
It might be easier to redirect output to a file, for example:

`./beep2mqtt -c b2mqtt_conf.json > outdata.txt`

Press ctrl-c when you are done.
Sort the data and have a look.

`sort -k2 -n outdata.txt`

A typical output when I run this and press the test button on the smoke detector, end of sort output:
```
[2271] 12909184.6  3056.4Hz
[2274] 13434351.3  3060.4Hz
[2267] 14341863.7  3051.0Hz
[2267] 15491824.3  3051.0Hz
[2271] 15679604.7  3056.4Hz
[2275] 17166055.6  3061.8Hz
[2273] 18505395.3  3059.1Hz
[2270] 21316507.7  3055.0Hz
[2269] 22123821.8  3053.7Hz
[2272] 25170954.4  3057.7Hz
[2271] 27199593.1  3056.4Hz
[2271] 31795160.7  3056.4Hz
[2272] 33084305.4  3057.7Hz
[2269] 34153882.4  3053.7Hz
[2270] 37705102.5  3055.0Hz
[2267] 38419776.0  3051.0Hz
[2268] 43583555.6  3052.3Hz
[2269] 50319173.7  3053.7Hz
[2270] 54314688.0  3055.0Hz
[2270] 59316031.2  3055.0Hz

```
The values in [] are the bucket number. Next value is the magnitude. Lastly is the frequency, just for information. We can see here that there is something going on around bucket 2270. For some margin I set buckets to 2268-2271 with a magnitude of 1M. If you encounter false positives try to either increase magnitude or narrow bucket span. I have never had music blasting on full volume trigger the alarm, but anything is possible. Update the config. I edit 'b2mqtt_conf.json and change `boi_high` to 2271 and `boi_low` to 2268. I leave `boi_mag` at 1M for safety margin. 

### Test
Correct all fields in configuration file, change 'mode' to 'sample'.
Start beep2mqtt in verbose mode to see what happens when triggering the alarm sound:

`./beep2mqtt -c b2mqtt_conf.json -v`

A typical output that works will look like this when pressing the detector test button:
```
mqtt connected
Detection: magnitude 2072463   total in queue 1
Detection: magnitude 5052304   total in queue 2
Detection: magnitude 5778983   total in queue 3
Detection: magnitude 3524887   total in queue 4
mqtt connected
```
If the queue number climbs, it's working. ctrl-c and kill the program.

### Copy executable and configuration file somewhere
For example (as root, or prepend sudo):
```
cp beep2mqtt /usr/local/sbin/
cp b2mqtt_conf.json /etc/
```
### Setup as service
There is a sample systemctl file provided. To use with defaults:
```
cp beep2mqtt.service /etc/systemd/system/
systemctl daemon-reload
systemctl enable beep2mqtt.service
systemctl start beep2mqtt.service
```
If you want to verify:
`systemctl status beep2mqtt.service `
Output should look something like this:
```
● beep2mqtt.service - Beep to MQTT
     Loaded: loaded (/etc/systemd/system/beep2mqtt.service; enabled; vendor preset: enabled)
     Active: active (running) since Sat 2022-06-25 18:33:50 UTC; 3min 15s ago
   Main PID: 12584 (beep2mqtt)
      Tasks: 4 (limit: 4430)
     CGroup: /system.slice/beep2mqtt.service
             └─12584 /usr/local/sbin/beep2mqtt -c /etc/b2mqtt_conf.json

Jun 25 18:33:50 r401 systemd[1]: Started Beep to MQTT.
```
# Home Assistant integration
Here is an example of a entry in 'configuration.yaml' to get a smoke detector entity that beep2mqtt turns on and off. Make sure to change relevant topics and messages if you don't go with the defaults. 
From 'configuration.yaml':
```
binary_sensor:
  - platform: mqtt
    name: "smoke_outhouse"
    unique_id: "smokeouthouse"
    state_topic: "mancave/smoke"
    payload_on: "smoke"
    payload_off: "clear"
    device_class: smoke
```
# Final words
I hope this software may benefit you somehow. Personally I hope that this was all a waste of time and it shall never hear a smoke detector going off. I have never interacted with soundcards before. It's tested on three separate hardwares, I would not be surprised if I have done something stupid and it breaks for you. If so, please let me know. Good luck, I hope you'll have no fires. I'm thinking of using it to whistle to turn on the lights...
