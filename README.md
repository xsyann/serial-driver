serial-driver
=============

A simple serial driver developed for the linux kernel 3.11.10 [EOL].

##Install

####Clone sources
    git clone https://github.com/4ppolo/serial-driver.git

####Set udev permissions
    echo 'KERNEL=="serial_driver*", NAME=="%k", MODE="666"' | sudo tee -a /etc/udev/rules.d/50-udev.rules
    sudo udevadm control --reload-rules

####Make & Load module
    make
    make load

##Uninstall
    make unload

## Install Socat on host

http://www.dest-unreach.org/socat/

http://www.dest-unreach.org/socat/download/socat-2.0.0-b7.tar.gz

####Mac OS Install
    tar xzvf socat-2.0.0-b7.tgz
    cd socat-2.0.0-b7
    ./configure
    echo '#define __APPLE_USE_RFC_2292' > temp_file.c
    cat xio-ip6.c >> tmp.c
    mv tmp.c xio-ip6.c
    make
    make install

## Install Cutecom on guest
    sudo apt-get install cutecom

## Test

####Configure VM serial port
- Open `Virtual Machines.localized/[vm-name].vmwarevm/[vm-name].vmx`
- Remove all existing serial ports

- Add 4 serial ports

        serial0.present = "TRUE"
        serial0.fileName = "/tmp/com1"
        serial0.fileType = "pipe"
        serial1.present = "TRUE"
        serial1.fileType = "pipe"
        serial1.fileName = "/tmp/com2"
        serial2.present = "TRUE"
        serial2.fileType = "pipe"
        serial2.fileName = "/private/tmp/com3"
        serial3.present = "TRUE"
        serial3.fileType = "pipe"
        serial3.fileName = "/private/tmp/com4"

####Host
    user@host$ socat unix-connect:/tmp/com1 stdio

####Guest
    user@guest$ cutecom &

`Device : /dev/serial_driver0`

    user@guest$ cat /proc/serial_driver
    0: port: 000003F8 irq: 4 tx:0 rx:36 RTS|CTS|DTR|DSR|CD
    1: port: 000002F8 irq: 3 tx:0 rx:0
    2: port: 000003E8 irq: 4 tx:0 rx:0
    3: port: 000002E8 irq: 3 tx:0 rx:0
