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

Note that the sdr code is written exclusively for sdrplay devices. The reason is that these are high quality devices, with a fairly well documented API, and with good developer support.

# Installation
Before installing this software, make sure to install the sdrplay API version 3.15 or higher (https://www.sdrplay.com/api/)


Also run following commands to install some prerequisites:

sudo apt-get install libcjson-dev

sudo apt-get install libsqlite3-dev

sudo apt-get install libmicrohttpd-dev

sudo apt-get install sqlite3

sudo apt-get install autotools-dev autoconf


then git clone this repository and do the following:

autoreconf

automake --add-missing

autoreconf

./configure

make

sudo make install



