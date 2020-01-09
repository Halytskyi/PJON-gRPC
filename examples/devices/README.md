# Arduino devices

## Makefile

If you want compiling code and flashing it to Arduino directly from RPi need install/configure [Arduino-Makefile](https://github.com/sudar/Arduino-Makefile).

Install necessary packages

```bash
apt-get install python-serial git -y
```

Download [Arduino IDE 1.8.10](https://www.arduino.cc/en/Main/OldSoftwareReleases#previous) to `/opt` and run commands

```bash
cd /opt
tar xf arduino-1.8.10-linuxarm.tar.xz
rm -rf arduino-1.8.10-linuxarm.tar.xz
ln -s arduino-1.8.10 arduino
cd /opt
git clone https://github.com/sudar/Arduino-Makefile.git
cd Arduino-Makefile && git checkout 1.6.0
```

Add PJON library to Arduino IDE (assume that you downloaded it to `/opt/libraries/PJON`):

```bash
cd /opt/arduino/libraries
ln -s /opt/libraries/PJON
```
