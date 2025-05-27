mmc list
查看mmc列表

mmc dev

mmc part

ls mmc1:1

printenv查看环境变量

printenv variable
打印具体某个环境变量

setenv variable
设置某个环境变量

saveenv保存环境变量

setenv ipaddr 192.168.120.230;setenv serveraddr 192.168.120.10;tftpboot 0x80300000 sdcard.img.gz;unzip 0x80300000 0x90000000 0x40000000;mmc write 0x90000000 0 8192;tftpboot 0x80200000 os.bin;go 0x80200000

setenv ipaddr 192.168.120.230;setenv serveraddr 192.168.120.10;tftpboot 0x80200000 os.bin;go 0x80200000

bootp 0x80300000 sdcard.img.gz;unzip 0x80300000 0x90000000 0x40000000;mmc write 0x90000000 0 8192
bootp 0x80200000 
go 0x80200000

##########
apt update && apt install minicom -y && minicom -D /dev/ttyUSB1 -b 115200
minicom -D /dev/ttyUSB0 -b 115200
/cg/control off && /cg/control on