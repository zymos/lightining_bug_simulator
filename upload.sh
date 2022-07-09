echo "uploading via avrdude"
avrdude -D -v -p atmega328p -c arduino -b 57600 -D -P /dev/ttyUSB0 -U flash:w:.pio/build/pro8MHzatmega328/firmware.hex:i