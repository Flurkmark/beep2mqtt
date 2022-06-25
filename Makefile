CC=gcc
MODCFLAGS := -Wall -O2 -fomit-frame-pointer -pipe
LIBS :=  -lasound -lfftw3 -lm -lpaho-mqtt3a -pthread
MAILMANDLIBS := -L$(LIBDIR) -lmysqlclient -lz -lpstring 
all: beep2mqtt err_exit.o cJSON.o
beep2mqtt: beep2mqtt.o err_exit.o cJSON.o
	$(CC) $(MODCFLAGS) $^ $(LIBS) -o $@

beep2mqtt.o: beep2mqtt.c
	$(CC) $(MODCFLAGS) -c $<  -o $@
err_exit.o: err_exit.c
	$(CC) $(MODCFLAGS) -c $<  -o $@
cJSON.o: cJSON.c
	$(CC) $(MODCFLAGS) -c $<  -o $@

clean:
	rm -f *.o
distclean: clean
	rm -f beep2mqtt
	rm -f *~
# $@ = target, $< first prerequisite, $^ all prerequisites.
