### Arduino examples

#### Makefile

If you want compiling code and flashing it to Arduino directly from RPi need install/configure [Arduino-Makefile](https://github.com/sudar/Arduino-Makefile).

Install necessary packages
```
$ sudo apt-get install python-serial git
```

Download [Arduino IDE 1.6.13](https://www.arduino.cc/en/Main/OldSoftwareReleases#previous) to `/opt` and:
```
$ cd /opt
$ tar xf arduino-1.6.13-linuxarm.tar.xz && rm -rf arduino-1.6.13-linuxarm.tar.xz
$ ln -s arduino-1.6.13 arduino
$ cd arduino
$ git clone https://github.com/sudar/Arduino-Makefile.git
$ cd Arduino-Makefile && git checkout 1.6.0
```

Add PJON library to Arduino IDE (assume that you downloaded it to `/opt/libraries/PJON`):
```
$ cd /opt/arduino/libraries
$ ln -s /opt/libraries/PJON
```
