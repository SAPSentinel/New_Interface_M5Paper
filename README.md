<p align="center">
<img src="https://img.shields.io/github/last-commit/sharandac/My-TTGO-Watch.svg?style=for-the-badge" />
&nbsp;
<img src="https://img.shields.io/github/license/sharandac/My-TTGO-Watch.svg?style=for-the-badge" />
&nbsp;
<a href="https://www.buymeacoffee.com/sharandac" target="_blank"><img src="https://img.shields.io/badge/Buy%20me%20a%20coffee-%E2%82%AC5-orange?style=for-the-badge&logo=buy-me-a-coffee" /></a>
</p>
<hr/>

# My-M5Paper v1.1

A GUI named hedge for M5Paper.

## Features

* BLE communication
* Time synchronization via BLE
* Notification via BLE
* Step counting
* Wake-up on wrist rotation
* Quick actions:

  * WiFi
  * Bluetooth
  * GPS
  * Luminosity
  * Sound volume

* Multiple watch faces:

  * Embedded (digital)
  * [Community based watchfaces](https://sharandac.github.io/My-TTGO-Watchfaces/)

* Multiple 'apps':

  * Music (control the playback of the music on your phone)
  * Navigation (displays navigation instructions coming from the companion app)
  * Map (displays a map)
  * Notification (displays the last notification received)
  * Stopwatch (with all the necessary functions such as play, pause, stop)
  * Alarm
  * Step counter (displays the number of steps and daily objective)
  * Weather
  * Calendar
  * IR remote
  * ...

* Companion apps: Gadgetbridge

## Install

Clone this repository and open it with platformIO. Select the right env and then build and upload.
Or follow the great step by step [tutorial](https://www.youtube.com/watch?v=wUGADCnerCs) from [ShotokuTech](https://github.com/ShotokuTech).

Please check out
    https://github.com/sharandac/My-TTGO-Watch/blob/709ed0c5863435aa966c1d6f44552ddc0909a57c/src/hardware/wifictl.cpp#L256-L261
to setup your wifi when wps or input via display is not possible.

If you are interested in native Linux support, please install sdl2, curl and mosquitto dev lib and change the env to emulator_* in platformIO.

```bash
sudo apt-get install libsdl2-dev libcurl4-gnutls-dev libmosquitto-dev build-essential
```

# Known issues

* the webserver crashes the ESP32 really often
* the battery indicator is not accurate, rather a problem with the power management unit ( axp202 )

## Development on the Windows platform

The development tools have a known issue with the size of the project on Windows platforms. When the program is built you may receive the following error:

    xtensa-esp32-elf-g++: error: CreateProcess: No such file or directory
    *** [.pio\build\t-watch2020-v1\firmware.elf] Error 1

This issue has not been seen on Linux or other platforms. This is a linker issue and can be fixed by removing apps. To remove unneeded apps you can simply delete the appropriate directory in /src/app. Then simply recompile. App can be added in the same way. But note that the app must support autocall_function . This allows the automatic integration of apps without touching the rest of the code ( [the magic behind autocall_function](autocall.md) ).

Since each app includes a different set of files, you may need to delete several apps to reduce it small enough for the Windows build.

# How to use

Cf. [Usage](USAGE.md)



# For the programmers

Cf. [contribution guide](CONTRIBUTING.md)<br>
app autocall function [the magic behind autocall_function](autocall.md) or add a app without touching the rest

# Interface



## M5Paper ( downscaled )

![screenshot](images/image1.png)
![screenshot](images/image2.png)
![screenshot](images/image3.png)

## M5Core2

![screenshot](images/m5core2_img1.png)
![screenshot](images/m5core2_img2.png)
![screenshot](images/m5core2_img3.png)
![screenshot](images/m5core2_img4.png)

## WT32-SC01

![screenshot](images/WT32_SC01_img1.png)
![screenshot](images/WT32_SC01_img2.png)
![screenshot](images/WT32_SC01_img3.png)

## TTGO T-Watch 2021

![screenshot](images/twatch2021_img1.png)
![screenshot](images/twatch2021_img2.png)
![screenshot](images/twatch2021_img3.png)

# Contributors

Special thanks to the following people for their help:


https://github.com/sharandac/My-TTGO-Watch


<p>
If you want to donate to the author then you can buy me a coffee.
<br/><br/>
<a href="https://www.buymeacoffee.com/sharandac" target="_blank"><img src="https://img.shields.io/badge/Buy%20me%20a%20coffee-%E2%82%AC5-orange?style=for-the-badge&logo=buy-me-a-coffee" /></a>
</p>
