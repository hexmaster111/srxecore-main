(echo 'spi'; echo 'send'; cat .pio/build/srxecore/firmware.hex; echo 'write';) | nc 192.168.3.$1 8888
