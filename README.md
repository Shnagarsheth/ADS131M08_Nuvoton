# ADS131M08_Nuvoton

> Communication of Nuvoton Controllers with ADS131M08 8-Channel Delta-Sigma ADC (TI). 

> **Tested OK with Nuvoton M452RDAE** 

> Adapted from ADS131M08_Arduino library by moothknight.

> Tweaked a few things for Nuvoton controllers and some other things.  

> Many Thanks 🙏  to moothknight and to the original creator of ADS131M04 Library by Daniele Valentino Bella, for Imperial College London Rocketry (LGPL).
___

***Pin Config for test-setup***
* PD.2 --> DRDY (18)
* PD.3 --> SYNCRST (16)
* PB.4 --> CS (17)
* PB.5 --> MOSI (DIN - 21)
* PB.6 --> MISO (DOUT - 20)
* PB.7 --> SCLK (19)
* PD.12 --> XTAL1(CLKIN -23)
___
***References***

1. [M452RDAE DataSheet](https://www.nuvoton.com/resource-files/DS_M451_Series_EN_Rev2.08.pdf) 
2. [ADS131M08 DataSheet](https://www.ti.com/lit/ds/symlink/ads131m08.pdf?ts=1647933622037&ref_url=https%253A%252F%252Fwww.google.com%252F)
3. Link to [ADS13M08_Arduino](https://github.com/moothyknight/ADS131M08_Arduino) by moothknight
4. Link to the original [ADS131M04](https://github.com/moothyknight/ADS131M08_Arduino) by Daniele Valentino Bella
___   