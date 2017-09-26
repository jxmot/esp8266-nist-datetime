# ESP8266-nist-datetime (*a work in progress*)


* [Overview](https://bitbucket.org/jxmot/esp8266-config-data/overview#markdown-header-overview)
    * [History](https://bitbucket.org/jxmot/esp8266-config-data/overview#markdown-header-history)
* [Requirements](https://bitbucket.org/jxmot/esp8266-config-data/overview#markdown-header-requirements)
* [Running the Application](https://bitbucket.org/jxmot/esp8266-config-data/overview#markdown-header-running-the-application)
    * [Next Steps](https://bitbucket.org/jxmot/esp8266-config-data/overview#markdown-header-next-steps)
* [Additional Notes](https://bitbucket.org/jxmot/esp8266-config-data/overview#markdown-header-additional-notes)
    * [ESP8266 Library Problems](https://bitbucket.org/jxmot/esp8266-config-data/overview#markdown-header-esp8266-library-problems)

# Overview

This repository contains code intended for the ESP8266. Its purpose is to demonstrate one method of keeping time on the ESP8266. This method utilizes the NIST Daytime and NIST Internet Time services.

# Project Status

This project is currently a **_work in progress_**. And *most* of the time the master branch will build and *do something*.

*It's guaranteed to work... unless it doesn't*

## History

I wanted to see if the ESP8266 was capable of keeping time and synchronize with NIST periodically. So after some resarch I came acroos this : <https://www.arduino.cc/en/Tutorial/UdpNtpClient>

That was the starting point for the code in this repository. I've added a few lines, did some refactoring, added some comments, etc etc. It's all a blur.

But here are some highlights of what I've done so far - 

* I'm reusing parts from one of my other projects within this one - <https://bitbucket.org/jxmot/esp8266-config-data>
* I simplified stuff. I thought I was doing something "clever" but it turned out it was just "overkill".

# Requirements

You will need the following - 

* ESP8266 - I'm using a NodeMCU 1.0, and mine is a 12-E
* Arduino IDE - Installed and made ready for use with the ESP8266
* This application utilizes the ESP8266 SPIFFS. Please take a look at <https://bitbucket.org/jxmot/esp8266-config-data> for important information before proceeding.

# Running the Application


## Next Steps



# Additional Notes

## ESP8266 Library Problems

As I was trying out different bits of code I discovered that there are a few functions that don't exist in the ESP8266 Arduino libraries. Specifically, the ones I came across were - 

* strptime()
* sscanf()

