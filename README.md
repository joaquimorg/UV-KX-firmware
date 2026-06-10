# My fork of Quansheng UV-K5/K6/5R custom firmware

This repository is a fork of [Armel custom firmware](https://github.com/armel/uv-k5-firmware-custom)

## ..... Work in PROGRESS - not for everyday use .....

> [!WARNING]  
> To test this version you must bear in mind that it can corrupt the data in the EEPROM, so you must make a backup before using this version.
>
> **If you want to have a more stable FW and even with more features, I advise you to look at other options.**

> [!WARNING]  
> Use this firmware at your own risk (entirely). There is absolutely no guarantee that it will work in any way shape or form on your radio(s), it may even brick your radio(s), in which case, you'd need to buy another radio.
>

<img src="images/uv-k5-screenshot-1.png" alt="VFO" width="524" />
<img src="images/uv-k5-screenshot-2.png" alt="MENU" width="524" />
<img src="images/uv-k5-screenshot-3.png" alt="Menu" width="524" />
<img src="images/uv-k5-screenshot-5.png" alt="Messenger" width="524" />
<img src="images/uv-k5-screenshot_spectrum.png" alt="Spectrum Analyzer" width="524" />

# What's new: 4-slot watch (A/B/C/D)

This version replaces the classic dual watch (A/B) with a **4-slot watch**. The radio now manages four VFO slots — A, B, C and D — and each one can independently hold a memory channel or a VFO frequency.

**How it works**

- When the watch is enabled (`RX MODE` menu → `WATCH ALL`), the idle receiver cycles A → B → C → D → A (100 ms dwell per slot). With `MAIN ONLY` it stays on the selected slot.
- The **A/B key (key 2)** cycles the selected slot through all four. The selected slot is the *main* one: it is shown in detail at the top of the screen, it is the one you edit (frequency entry, channel up/down, F+3 MR/VFO toggle, F+1 band), and **TX always happens on it**.
- The main screen shows the selected slot in full detail (name bar, big frequency, modulation/bandwidth/power, CTCSS/DCS or step) and the other three slots as compact rows below a dotted divider — slot letter, channel name (or `M-xxx`/`F-xxx`) and frequency. A receiving row is fully inverted; the last-RX slot letter blinks. The status line shows `DW` while cycling, or `DW-x` when parked on slot *x*.

**Storage (EEPROM)**

Slots C and D are stored in a previously unused EEPROM area, so the existing layout (channels, names, calibration, settings) is untouched and remains compatible:

| Address | Size | Content |
|---|---|---|
| `0x1F90` | 16 B | Slot C frequency-mode record (same format as a channel record) |
| `0x1FA0` | 16 B | Slot D frequency-mode record |
| `0x1FB0` | 8 B | Slot C/D channel indices (screen/MR/freq channel) |

A virgin (0xFF) region boots to sane defaults. A full factory reset also wipes this area. Note: unlike A/B, which keep one frequency record per band, C and D keep a single record — changing band on C/D overwrites their previous frequency settings.

**Behaviour changes**

- `RX MODE` menu is now just `MAIN ONLY` / `WATCH ALL` — the **cross-band modes were removed**, since TX is always on the selected slot.
- The dead NOAA code was removed.
- With scan ranges, the range stop frequency now comes from the **next slot in the cycle** (it used to come from "the other VFO").
- The keypad lock icon moved to the bottom status line.

**CHIRP**

The bundled driver (`CHIRP/uvk5_egzumer_f4hwn_ver_4_3_0.py`) was updated: it maps the new `0x1F90` region, uploads it with the normal write, exposes *VFO C/D Channel/Band* and *VFO C/D Frequency* in Basic Settings, offers all four slots in *Main VFO*, and reduces *RX Mode* to the two supported options.

# Differences from the original Armel firmware
- Reorganized code layout: main code in `src/` and linker files in `linker/` for easier navigation/builds.
- New U8G2-based GUI with custom fonts/icons (`fonts_icons/`, `src/ui/font/`) and a new welcome screen.
- New screens/apps: Messenger and redesigned menu/UI (`src/app/messenger.c`, `src/ui/`).
- Additional tools and assets: `display-explorer/` for display exploration and bundled `utils/k5prog`.
- Prebuilt binaries in this fork under `firmware/` (`uv-kx_V1.0.bin` and `.packed.bin`);
- Updated Makefile for easier compilation.

- F+5 to open minimal Spectrum Analyzer app.

- F+M to open Messenger app.
<img src="images/UV-K5 - keypad - T9.jpg" alt="T9 keypad" width="524" width="524" />

# UV-Kx Web tools
You can use [UV-Kx Web tools](https://home.joaquim.org/k5/) to manage your radio data (channels, settings, etc.) from your web browser. It works with this firmware.

# Open re-implementation of the Quansheng UV-K5/K6/5R v2.1.27 firmware

This repository is a fork of [Egzumer custom firmware](https://github.com/egzumer/uv-k5-firmware-custom), who was a merge of [OneOfEleven custom firmware](https://github.com/OneOfEleven/uv-k5-firmware-custom) with [fagci spectrum analizer](https://github.com/fagci/uv-k5-firmware-fagci-mod/tree/refactor) plus my few changes.

All is a cloned and customized version of DualTachyon's open firmware found [here](https://github.com/DualTachyon/uv-k5-firmware) ... a cool achievement !

> [!NOTE]
> EN - About Chirp, as many others firmwares, you need to use a dedicated driver available on [this repository](https://github.com/armel/uv-k5-chirp-driver). 
>
> _FR - A propos de Chirp, comme beaucoup d'autres firmwares, vous devez utiliser un pilote dédié disponible sur [ce dépôt](https://github.com/armel/uv-k5-chirp-driver)._

> [!WARNING]
> EN - THIS FIRMWARE HAS NO REAL BRAIN. PLEASE USE YOUR OWN. Use this firmware at your own risk (entirely). There is absolutely no guarantee that it will work in any way shape or form on your radio(s), it may even brick your radio(s), in which case, you'd need to buy another radio.
Anyway, have fun.
>
> _FR - CE FIRMWARE N'A PAS DE VÉRITABLE CERVEAU. VEUILLEZ UTILISER LE VÔTRE. Utilisez ce firmware à vos risques et périls. Il n'y a absolument aucune garantie qu'il fonctionnera d'une manière ou d'une autre sur votre (vos) radio(s), il peut même bousiller votre (vos) radio(s), dans ce cas, vous devrez acheter une autre radio. Quoi qu'il en soit, amusez-vous bien._

> [!CAUTION]
> EN - I recommend to backup your eeprom with [k5prog](https://home.joaquim.org/k5/) before playing with alternative firmwares. It's a good reflex to have. 
>
> _FR - Je recommande de sauvegarder votre eeprom avec [k5prog](https://home.joaquim.org/k5/) avant de jouer avec des firmwares alternatifs. C'est un bon réflexe à avoir._

# Donations

Special thanks to Jean-Cyrille F6IWW (2 times), Fabrice 14RC123, David F4BPP, Olivier 14RC206, Frédéric F4ESO, Stéphane F5LGW, Jorge Ornelas (4 times), Laurent F4AXK, Christophe Morel, Clayton W0LED, Pierre Antoine F6FWB, Jean-Claude 14FRS3306, Thierry F4GVO, Eric F1NOU, PricelessToolkit, Ady M6NYJ, Tom McGovern (4 times), Joseph Roth, Pierre-Yves Colin, Frank DJ7FG, Marcel Testaz, Brian Frobisher, Yannick F4JFO, Paolo Bussola, Dirk DL8DF, Levente Szőke (2 times), Bernard-Michel Herrera, Jérôme Saintespes, Paul Davies, RS (3 times), Johan F4WAT, Robert Wörle, Rafael Sundorf, Paul Harker, Peter Fintl, Pascal F4ICR (2 times), Mike DL2MF, Eric KI1C (2 times), Phil G0ELM, Jérôme Lambert, Meinhard Frank Günther, Eliot Vedel, Alfonso EA7KDF, Jean-François F1EVM, Robert DC1RDB, Ian KE2CHJ, Daryl VK3AWA, Roberto Brunelli, Robert Boardman, Stephen Oliver, Nicolas F4INE and William Bruno for their [donations](https://www.paypal.com/paypalme/F4HWN). That’s so kind of them. Thanks so much 🙏🏻

## Table of Contents

- [My fork of Quansheng UV-K5/K6/5R custom firmware](#my-fork-of-quansheng-uv-k5k65r-custom-firmware)
  - [..... Work in PROGRESS - not for everyday use .....](#-work-in-progress---not-for-everyday-use-)
- [What's new: 4-slot watch (A/B/C/D)](#whats-new-4-slot-watch-abcd)
- [Differences from the original Armel firmware](#differences-from-the-original-armel-firmware)
- [UV-Kx Web tools](#uv-kx-web-tools)
- [Open re-implementation of the Quansheng UV-K5/K6/5R v2.1.27 firmware](#open-re-implementation-of-the-quansheng-uv-k5k65r-v2127-firmware)
- [Donations](#donations)
  - [Table of Contents](#table-of-contents)
  - [Main features and improvements from F4HWN:](#main-features-and-improvements-from-f4hwn)
  - [Main features from Egzumer:](#main-features-from-egzumer)
  - [Manual](#manual)
  - [Radio performance](#radio-performance)
  - [Compiler](#compiler)
  - [Credits](#credits)
  - [Other sources of information](#other-sources-of-information)
  - [License](#license)

## Main features and improvements from F4HWN:

* several firmware versions:
    * Bandscope (with spectrum analyzer made by Fagci),
    * Broadcast (with commercial FM radio support),
    * Basic (with spectrum analyzer and commercial FM radios support, but without certain functions such as Vox, Aircopy, etc.),
    * RescueOps (specifically designed for first responders: firefighters, sea rescue, mountain rescue),
    * Game (with a small breakout game),
* improve default power settings level: 
    * Low1 to Low5 (<~20mW, ~125mW, ~250mW, ~500mW, ~1W), 
    * Mid ~2W, 
    * High ~5W,
    * User (see SetPwr),
* improve S-Meter (IARU Region 1 Technical Recommendation R.1 for VHF/UHF - [read more](https://hamwaves.com/decibel/en/)),
   * S-Meter (S0/S9) Level EEPROM settings that were introduced in the Egzumer firmware are now ignored and replaced by hardcoded values to comply with the IARU Recommendation.     
* improve bandscope (Spectrum Analyser):
    * add channel name,
    * add save of some spectrum parameters,
* improve UI: 
    * menu index is always visible, even if a menu is selected,
    * s-meter new design (Classic or Tiny), 
    * MAIN ONLY screen mode, 
    * DUAL and CROSS screen mode, 
    * RX blink on VFO RX, 
    * RX LED blink, 
    * Squelch level and Monitor,
    * Step value,
    * CTCSS or DCS value,
    * KeyLock message,
    * last RX,
    * move BatTxt menu from 34/63 to 30/63 (just after BatSave menu 29/63),
    * rename BackLt to BLTime,
    * rename BltTRX to BLTxRx,
    * improve memory channel input,
    * improve keyboard frequency input,
    * add percent and gauge to Air Copy,
    * improve audio bar,
    * and more...
* new menu entries and changes:
    * add SetPwr menu to set User power (<20mW, 125mW, 250mW, 500mW, 1W, 2W or 5W),
    * add SetPTT menu to set PTT mode (Classic or OnePush),
    * add SetTOT menu to set TOT alert (Off, Sound, Visual, All),
    * add SetCtr menu to set contrast (0 to 15),
    * add SetInv menu to set screen in invert mode (Off or On),
    * add SetEOT menu to set EOT (End Of Transmission) alert (Off, Sound, Visual, All),
    * add SetMet menu to set s-meter style (Classic or Tiny),
    * add SetLck menu to set what is locked (Keys or Keys + PTT),
    * add SetGUI menu to set font size on the VFO baseline (Classic or Tiny),
    * add TXLock menu to open TX on channel,
    * add SetTmr menu to set RX and TX timers (Off or On),
    * add SetOff menu to set the delay before the transceiver goes into deep sleep (Off or 1 minute to 2 hours),
    * add SetNFM menu to set Narrow width (12.5kHz or 6.25kHz),
    * rename BatVol menu (52/63) to SysInf, which displays the firmware version in addition to the battery status,
    * improve PonMsg menu,
    * improve BackLt menu,
    * improve TxTOut menu,
    * improve ScnRev menu (CARRIER from 250ms to 20s, STOP, TIMEOUT from 5s to 2m)
    * improve KeyLck menu (OFF, delay from 15s to 10m)
    * add HAM CA F Lock band (for Canadian zone),
    * add PMR 446 F Lock band,
    * add FRS/GMRS/MURS F Lock band,
    * remove blink and SOS functionality, 
    * remove AM Fix menu (AM Fix is ENABLED by default),
    * add support of 3500mAh battery,
* improve status bar:
    * add SetPtt mode in status bar,
    * change font and bitmaps,
    * move USB icon to left of battery information,
    * add RX and TX timers,
* improve lists and scan lists options:
    * add new list 3,
    * add new list 0 (channel without list...),
    * add new scan lists options,
        * scan list 0 (all channels without list),
        * scan list 1,
        * scan list 2,
        * scan list 3,
        * scan lists [1, 2, 3],
        * scan all (all channels with or without list),
    * add scan list shortcuts,
* add resume mode on startup (scan, bandscope and broadcast FM),
* new actions:
    * RX MODE,
    * MAIN ONLY,
    * PTT, 
    * WIDE NARROW,
    * 1750Hz,
    * MUTE,
    * POWER HIGH (RescueOps),
    * REMOVE OFFSET (RescueOps),
* new key combinations:
    * add the F + UP or F + DOWN key combination to dynamically change the Squelch level,
    * add the F + F1 or F + F2 key combination to dynamically change the Step,
    * add F + 8 to quickly switch backlight between BLMin and BLMax on demand (this bypass BackLt strategy),
    * add F + 9 to return to BackLt strategy,
    * add long press on MENU, in * SCAN mode, to temporarily exclude a memory channel,
    * add short press on [0, 1, 2, 3, 4 or 5], in * SCAN mode, to dynamically change scan list.
* many fix:
    * squelch, 
    * s-meter,
    * DTMF overlaying, 
    * scan list 2 ignored, 
    * scan range limit,
    * clean display on startup,
    * no more PWM noise,
    * and more...
* enabled AIR COPY
* disabled ENABLE_DTMF_CALLING,
* disabled SCRAMBLER,
* remove 200Tx, 350Tx and 500Tx,
* unlock TX on all bands needs only to be repeat 3 times,
* code refactoring and many memory optimization,
* displays the live screen of the Quansheng K5 on your computer via a USB-to-Serial cable,
* and more...

## Main features from Egzumer:
* many of OneOfEleven mods:
   * AM fix, huge improvement in reception quality
   * long press buttons functions replicating F+ action
   * fast scanning
   * channel name editing in the menu
   * channel name + frequency display option
   * shortcut for scan-list assignment (long press `5 NOAA`)
   * scan-list toggle (long press `* Scan` while scanning)
   * configurable button function selectable from menu
   * battery percentage/voltage on status bar, selectable from menu
   * longer backlight times
   * mic bar
   * RSSI s-meter
   * more frequency steps
   * squelch more sensitive
* fagci spectrum analyzer (**F+5** to turn on)
* some other mods introduced by me:
   * SSB demodulation (adopted from fagci)
   * backlight dimming
   * battery voltage calibration from menu
   * better battery percentage calculation, selectable for 1600mAh or 2200mAh
   * more configurable button functions
   * long press MENU as another configurable button
   * better DCS/CTCSS scanning in the menu (`* SCAN` while in RX DCS/CTCSS menu item)
   * Piotr022 style s-meter
   * restore initial freq/channel when scanning stopped with EXIT, remember last found transmission with MENU button
   * reordered and renamed menu entries
   * LCD interference crash fix
   * many others...

 ## Manual

Up to date manual is available in the [Wiki section](https://github.com/armel/uv-k5-firmware-custom/wiki)

## Radio performance

Please note that the Quansheng UV-Kx radios are not professional quality transceivers, their
performance is strictly limited. The RX front end has no track-tuned band pass filtering
at all, and so are wide band/wide open to any and all signals over a large frequency range.

Using the radio in high intensity RF environments will most likely make reception anything but
easy (AM mode will suffer far more than FM ever will), the receiver simply doesn't have a
great dynamic range, which results in distorted AM audio with stronger RX'ed signals.
There is nothing more anyone can do in firmware/software to improve that, once the RX gain
adjustment I do (AM fix) reaches the hardwares limit, your AM RX audio will be all but
non-existent (just like Quansheng's firmware).
On the other hand, FM RX audio will/should be fine.

But, they are nice toys for the price, fun to play with.

## Compiler

arm-none-eabi GCC version 10.3.1 is recommended, which is the current version on Ubuntu 22.04.03 LTS.
Other versions may generate a flash file that is too big.
You can get an appropriate version from: https://developer.arm.com/downloads/-/gnu-rm

clang may be used but isn't fully supported. Resulting binaries may also be bigger.
You can get it from: https://releases.llvm.org/download.html

## Credits

Many thanks to various people:

* [Egzumer](https://github.com/egzumer)
* [OneOfEleven](https://github.com/OneOfEleven)
* [DualTachyon](https://github.com/DualTachyon)
* [Mikhail](https://github.com/fagci)
* [Andrej](https://github.com/Tunas1337)
* [Manuel](https://github.com/manujedi)
* @wagner
* @Lohtse Shar
* [@Matoz](https://github.com/spm81)
* @Davide
* @Ismo OH2FTG
* [OneOfEleven](https://github.com/OneOfEleven)
* @d1ced95
* and others I forget

## Other sources of information

[ludwich66 - Quansheng UV-K5 Wiki](https://github.com/ludwich66/Quansheng_UV-K5_Wiki/wiki)<br>
[amnemonic - tools and sources of information](https://github.com/amnemonic/Quansheng_UV-K5_Firmware)

## License

Copyright 2023 Dual Tachyon
https://github.com/DualTachyon

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
