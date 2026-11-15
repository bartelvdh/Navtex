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

The capture code is split into hardware-specific backends and a hardware-independent I/Q pipeline (`receiver/iq_pipeline.c`). The pipeline consumes signed 16-bit I/Q samples and feeds the existing DSP and optional debug WAV recording. By default, the receiver automatically tries the RTL-SDR Blog V4 first, then SDRplay; if neither is available, it retries detection every 60 seconds. Once a backend starts capturing, it remains in use until capture ends or the device disconnects; only then does the receiver try the other backend. Use `-r` to force RTL-SDR or `-s` to force SDRplay. The `-p` antenna-port option selects SDRplay automatically because it is SDRplay-only. Building the RTL-SDR backend requires `librtlsdr-dev`, and runtime use requires a V4-capable `librtlsdr` build. The backend selects a device identifying itself as manufacturer `RTLSDRBlog` and product `Blog V4`; the V4-capable library handles its HF upconverter when tuned to 504 kHz. See the [RTL-SDR Blog driver](https://github.com/rtlsdrblog/rtl-sdr-blog-minimal) for V4-compatible library installation instructions.

The receiver and web server use the SQLite database at `/var/lib/navtex/Navtex.db`. Debian packages install a database template under `/usr/share/navtex` and initialize the writable database on first installation.

# Debian package (arm64)
The Debian package installs the programs as systemd services running under a dedicated `navtex` user. If the `plugdev` group exists, package setup adds `navtex` to it so the service can access RTL-SDR devices whose udev permissions use that group. On systems with different RTL-SDR udev permissions, grant the service user access to the device's configured group. The package stores the web interface in `/usr/share/navtex/www` and keeps the writable database in `/var/lib/navtex`.

The SDRplay API version 3.15 or newer is loaded at runtime only when using an SDRplay device. It is not bundled with the package; install it from SDRplay if you use SDRplay hardware. RTL-SDR users do not need the SDRplay runtime library. Building from source still requires the SDRplay development headers.

On an arm64 Debian or Raspberry Pi OS build environment, install the Debian build dependencies:

```sh
sudo apt install build-essential debhelper dh-autoreconf autoconf automake \
  autotools-dev libcjson-dev libgnutls28-dev libmicrohttpd-dev \
  librtlsdr-dev libsqlite3-dev sqlite3
```

Install the SDRplay development files from SDRplay to obtain the headers needed for building, then build the package with:

```sh
dpkg-buildpackage -us -uc -b
```

Install the resulting `navtex` `.deb` with `apt` or `dpkg`. The package enables and starts both services; check them with `systemctl status navtexreceiver navtexwebserver`. The web interface listens on port 8080.

# Installation
The build includes both capture backends, so building from source requires the SDRplay API 3.15 or newer development headers and the RTL-SDR development library. At runtime, install the SDRplay API only if using SDRplay. The RTL-SDR Blog V4 backend also needs a V4-capable `librtlsdr`.

These are the instructions to install updated drivers for the RTL-SDR Blog V4 (and V4L):
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



Also run following commands to install some prerequisites:

sudo apt-get install libcjson-dev
sudo apt-get install libsqlite3-dev
sudo apt-get install libmicrohttpd-dev
sudo apt-get install sqlite3
sudo apt-get install autotools-dev autoconf


then git clone this repository:

git clone https://github.com/bartelvdh/Navtex


and do the following:

cd Navtex

autoreconf

automake --add-missing

autoreconf

./configure

make

sudo make install

If you intend to use the raspberry pi disconnected from the internet, you may want to connect a GPS antenna (via USB) and install gpsd and crony: 
apt-get install gpsd gpsd-clients chrony

By default, the receiver tries the RTL-SDR Blog V4 before SDRplay. Run `navtexreceiver -r` to force RTL-SDR or `navtexreceiver -s` to force SDRplay. The `-d` option enables the 15-minute I/Q WAV recording, and `-b` enables the bias tee for SDRplay or RTL-SDR devices. The antenna-port option `-p` is SDRplay-only; it automatically forces the SDRplay backend and cannot be combined with `-r`.
