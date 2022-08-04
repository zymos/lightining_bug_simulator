echo "uploading via avrdude"
avrdude -D -v -p atmega328p -c arduino -b 57600 -D -P /dev/ttyUSB0 -U flash:w:firmware-non-inv.hex:i
