serial-driver
=============

A simple serial driver developed for the linux kernel 3.11.10 [EOL].

##Install

####Clone sources
    git clone https://github.com/4ppolo/serial-driver.git
  
####Set udev permissions
    echo 'KERNEL=="serial_driver", NAME=="serial_driver", MODE="666"' | sudo tee -a /etc/udev/rules.d/50-udev.rules
  
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
- Virtual Machine > Settings > Add Device... > Serial Port > Add...
- Save As : `/tmp/com2`
- Open `Virtual Machines.localized/[vm-name].vmwarevm/[vm-name].vmx`
- Change `serial1.fileType = "file"` to `serial.fileType = "pipe"`

####Host
`user@host$ socat unix-connect:/tmp/com2 stdio`

####Guest
`user@guest$ cutecom`

`Device : /dev/serial_driver`

  
  
