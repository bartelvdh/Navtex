# Navtex
Navtex receiver based on SDRPLAY SDR devices, meant for practical use on recreational boats


The main dish of this project is an sdr (software defined radio) experiment.
As a recreational sailor and licenced Ham operator, I though it was a fun and useful project to go for.
The main trigger is the fact that no good and usable navtex decoding software for use in leisure crafts exists at this moment.
Projects like openplotter have brought together a great deal of excellent software to cover a lot of the navigation and comminication needs of a recreational boat. However a navtext receiver is still a missing component in that ecosystem.
Sure there's excellent software like YAND and FLDigi. The thing is that those are just meant for the occasional HAM that wants to capture some navtex messages for fun.
As a sailor you need to be able to receive those messages without being in front of a GUI at the exact moment the messages are transmitted. Reception should happen in background. Next to that Navtex software should allow you to select messages based on message types (subject indicator) and message sources (transmitter identity), so the B1 and B2 characters of the navtex message, like a standard commercial NAVTEX receiver, more or less.

So I decided to create this background process that allows navtex demodulation and decoding.
Next to that I created a web-based GUI application for displaying messages. The gui application is based on the react javascript framework. The communication with the backend is via a json based (REST) API. This API is exposed by a lightweight http server in the backend. Communication between the web server and the receiver process runs via an SQLite database.

The capture code is split into hardware-specific backends and a hardware-independent I/Q pipeline (`receiver/iq_pipeline.c`). The pipeline consumes signed 16-bit I/Q samples and feeds the existing DSP and optional debug WAV recording. By default, the receiver automatically tries the RTL-SDR Blog V4 first, then SDRplay; if neither is available, it retries detection every 60 seconds. Once a backend starts capturing, it remains in use until capture ends or the device disconnects; only then does the receiver try the other backend. Use `-r` to force RTL-SDR or `-s` to force SDRplay. The `-p` antenna-port option selects SDRplay automatically because it is SDRplay-only. The backend selects a device identifying itself as manufacturer `RTLSDRBlog` and product `Blog V4`; the V4-capable library handles its HF upconverter when tuned to 504 kHz. See the [RTL-SDR Blog driver](https://github.com/rtlsdrblog/rtl-sdr-blog-minimal) for V4-compatible library installation instructions.

The receiver and web server use an SQLite database to communicate.

# Building and installing the rtl-sdr library for RTL-SDR Blog V4 (and V4L)

This is needed for all of the build or installation options found below

```sh
sudo apt update 
sudo apt install libusb-1.0-0-dev git cmake build-essential 
pkg-config build-essential 
sudo apt install debhelper  
git clone https://github.com/osmocom/rtl-sdr 
cd rtl-sdr 
sudo dpkg-buildpackage -b --no-sign 
cd ..  
sudo dpkg -i librtlsdr0_*.deb 
sudo dpkg -i librtlsdr-dev_*.deb 
sudo dpkg -i rtl-sdr_*.deb
```


# Building a Debian package (arm64) for the navtex software
The Debian package installs the programs as systemd services running under a dedicated `navtex` user. If the `plugdev` group exists, package setup adds `navtex` to it so the service can access RTL-SDR devices whose udev permissions use that group. On systems with different RTL-SDR udev permissions, grant the service user access to the device's configured group. The package stores the web interface in `/usr/share/navtex/www` and keeps the writable database in `/var/lib/navtex`.

The SDRplay API version 3.15 or newer is loaded at runtime only when using an SDRplay device. It is not bundled with the package; install it from SDRplay if you use SDRplay hardware. RTL-SDR users do not need the SDRplay runtime library. Building from source still requires the SDRplay development headers.

On an arm64 Debian or Raspberry Pi OS build environment, install the Debian build dependencies:

```sh
sudo apt install build-essential debhelper dh-autoreconf autoconf automake \
  autotools-dev libcjson-dev libgnutls28-dev libmicrohttpd-dev \
  librtlsdr-dev libsqlite3-dev sqlite3
```

Install the SDRplay development files from SDRplay to obtain the headers needed for building, then build the package with:

git clone https://github.com/bartelvdh/Navtex

cd Navtex

```sh
dpkg-buildpackage -us -uc -b
cd..
```


# Installing from .deb package

WHen installing from scratch, first install the necessary dependencies: 

```sh
apt-get install libgnutls28-dev libmicrohttpd12t64 libmicrohttpd-dev libusb-1.0-0-dev libcjson-dev libsqlite3-dev
```


Install the resulting `navtex` `.deb` with `dpkg`, and reboot the raspberry pi:

```sh
sudo dpkg -i navtex*.deb
sudo shutdown -r now
```

The package enables and starts both services; check them with `systemctl status navtexreceiver navtexwebserver`. 
The web interface listens on port 8080.


# Classic build from source
The build includes both capture backends, so building from source requires the SDRplay API 3.15 or newer development headers and the RTL-SDR development library. At runtime, install the SDRplay API only if using SDRplay. The RTL-SDR Blog V4 backend also needs a V4-capable `librtlsdr`.




run following commands to install some prerequisites:

```sh
sudo apt-get install libcjson-dev
sudo apt-get install libsqlite3-dev
sudo apt-get install libmicrohttpd-dev
sudo apt-get install sqlite3
sudo apt-get install autotools-dev autoconf
```

then git clone this repository:

```sh
git clone https://github.com/bartelvdh/Navtex
```

and do the following:

```sh
cd Navtex
autoreconf
automake --add-missing
autoreconf
./configure
make
sudo make install
```


# Install based on pre-built packages
If pre-built packages exist, or can be downloaded, the install using those packages is probably the easiest option. 

These are the packages needed: 
rtl-sdr_2.0.3_arm64.deb
librtlsdr0_2.0.3_arm64.deb
librtlsdr-dev_2.0.3_arm64.deb
navtex_0.01-1_arm64.deb

This is the installation procedure: 

```sh
sudo apt update
sudo apt upgrade
sudo apt install libusb-1.0-0-dev

sudo dpkg -i librtlsdr0_*.deb 
sudo dpkg -i librtlsdr-dev_*.deb 
sudo dpkg -i rtl-sdr_*.deb

apt-get install  libmicrohttpd12t64 libcjson-dev 

sudo dpkg -i navtex*.deb
sudo shutdown -r now
```


# Get time from GPS

Navtex needs the realtime clock to be set to allow giving messages received a timestamp. 
If you intend to use the raspberry pi disconnected from the internet, you may want to connect a GPS antenna (via USB) and install gpsd and crony: 

apt-get install gpsd chrony


Put these lines in /etc/default/gpsd :

```sh
START_DAEMON="true"
USBAUTO="true"
DEVICES="/dev/ttyACM0"
GPSD_OPTIONS="-n"
```

run  

```sh
sudo systemctl enable --now gpsd
```



Put the follwing config in /etc/chrony/conf.d/gps.conf

```sh
#Example configuration using SHM or SOCK from gpsd and optional PPS
refclock SHM 0 refid GPS precision 1e-1 offset 0.0 delay 0.2
#If using a PPS pin (e.g., /dev/pps0):
#refclock PPS /dev/pps0 lock GPS refid PPS

#Allow step adjustments on boot if time is off
makestep 1 3

```

and run

sudo systemctl restart chrony



# use raspberry pi as wireless access point

You will have to perform the initial installation of packages on the PI while it is connected to the internet. 
However later on, you may want configure it to be fully autonomous and act as a wireless access point. 

```sh
sudo nmcli connection add \
  type wifi \
  ifname wlan0 \
  con-name navtex-ap \
  autoconnect no \
  ssid NAVTEX-PI


sudo nmcli connection modify navtex-ap \
  802-11-wireless.mode ap \
  802-11-wireless.band bg \
  ipv4.method shared \
  ipv4.addresses 192.168.4.1/24 \
  ipv6.method disabled \
  wifi-sec.key-mgmt wpa-psk \
  wifi-sec.psk "YourStrongPassword"
```

```sh
sudo nmcli connection up navtex-ap  
```
(this will cut your existing wifi connection and actually turn the pi into a wireless access point)


Enable automatic startup of the access point:

```sh
sudo nmcli connection modify navtex-ap \
    connection.autoconnect yes \
    connection.autoconnect-priority 100
```

Disable automatic startup of the access point:

```sh
sudo nmcli connection modify navtex-ap \
    connection.autoconnect no
```


# options for the navtexreceiver process

By default, the receiver tries the RTL-SDR Blog V4 before SDRplay. Run `navtexreceiver -r` to force RTL-SDR or `navtexreceiver -s` to force SDRplay. The `-d` option enables the 15-minute I/Q WAV recording, and `-b` enables the bias tee for SDRplay or RTL-SDR devices. The antenna-port option `-p` is SDRplay-only; it automatically forces the SDRplay backend and cannot be combined with `-r`.

In case you installed the software via a .deb package, you can permanently change the options passed to the navtexreceiver process by editing
/usr/lib/systemd/system/navtexreceiver.service

find the line 

```sh
ExecStart=/usr/bin/navtexreceiver
```

e.g. to always enable the bias T voltage, change it into

```sh
ExecStart=/usr/bin/navtexreceiver -b 
```