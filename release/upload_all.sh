# $1 is the last number of the IP address of the PortaProg
(echo 'del srxetest.cmd';) | nc 192.168.3.$1 8888
(echo 'del srxetest.hex';) | nc 192.168.3.$1 8888
cp .pio/build/srxecore/firmware.hex srxetest.hex
(echo 'upload srxetest.cmd'; cat srxetest.cmd)                | nc 192.168.3.$1 8888
(echo 'send'; cat srxetest.hex; echo 'store srxetest.hex';)   | nc 192.168.3.$1 8888
