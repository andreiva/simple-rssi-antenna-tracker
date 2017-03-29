Simple RSSI Antenna Tracker
===========================

This tracker uses [Quanum RC540R 5.8GHz 40CH Race Band FPV Diversity Receiver](https://www.hobbyking.com/en_us/5-8ghz-32ch-8-race-channels-diversity-vrx.html)
which has two video receivers with RSSI output. RSSI is measured with an Arduino's analog inputs, signal strength is compared and tracker head is turned towards stronger signal with servo.

This tracker relies solely on RSSI, no GPS or compass is involved.
This means you can track any 5.8GHz signal source to which the receiver is tuned to.
No pairing, no telemetry, no configuration, just tune and track.

###Pros:
* It's cheap and simple. You probably already have most of the the parts.
* no GPS needed
* no telemetry needed
* can track any 5.8GHz source

###Cons:
* tracking is relative
* tracker doesn't really know where the target is
* may wander off target
* tracks only in horizontal axis

# What will you need
* [Quanum RC540R 5.8GHz 40CH Race Band FPV Diversity Receiver](https://www.hobbyking.com/en_us/5-8ghz-32ch-8-race-channels-diversity-vrx.html)
    * known to work with: FR632, Boscam D58-2
* Two directional 5.8GHz antennas, helical or patch
* [Arduino Pro Mini ATmega32U4](http://www.ebay.com/sch/i.html?_from=R40&_sacat=0&_nkw=Arduino++atmega+32u4&rt=nc&LH_BIN=1) $4
* One servo, 9 gramms or similar 4$
* [5V voltage regulator](http://www.ebay.com/sch/i.html?_from=R40&_trksid=p2047675.m570.l1313.TR0.TRC0.H0.TRS0&_nkw=MP1584EN&_sacat=0)
* tracker head mount, 3D printed or made from scrap

###Youtube video
[https://www.youtube.com/watch?v=aACTKziuDVE](https://www.youtube.com/watch?v=aACTKziuDVE)

###Timer.h Arduino library
To install Timer library, download the ZIP file here:
[https://github.com/JChristensen/Timer](https://github.com/JChristensen/Timer)

How to install libraries
[https://www.youtube.com/watch?v=Alz-rX6lkEw](https://www.youtube.com/watch?v=Alz-rX6lkEw)