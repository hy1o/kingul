# kingul
## Korean Keyboard for Kindle Paperwhite3

Kingul is a compound word of 'Kindle' and 'Hangul', name of the Korean alphabet.
Since PW3 doesn't support Korean keyboard, I made one for myself.
Unlike the other languages, Korean alphabet can be composed with some of its own to make another character(syllable) with a new unicode value.

e.g. 한(U+D55C) = ㅎ(U+1112), ㅏ(U+1161), ㄴ(U+11AB)

The display sequence on the screen is ㅎ -> 하(another composed character) -> 한.
In other words, it needs a postprocessing after a key is typed.
This hack provides you such a postprocessing on Kindle PW3 device.


## 1) How it works
When you type a key on the screen, it sends to the appropriate window an event that indicates the key is pressed or released.
This hack continoulsy tries to catch a currently focused window, peeks the key event, does the postprocessing by entering or deleting characters.

## 2) Limitations
(1) One-way editing

(If you press backspace, whole composed character will be deleted)


## 3) Installation
Prerequisites: Jailbroken PW3 device, KUAL + MRPI Installer

Put the install.bin file to Kindle/mrpackages and run it with MRPI helper

## 4) Uninstallation
Put the uinstall.bin file to Kindle/mrpackages and run it with MRPI helper

## 5) Developer Notes
(1) XGrabKeyboard doesn't work. It is possible that the keyboard process has already grabbed it from the start. Or maybe it's due to the limitation of kindle's X11 library. Anyway, I couldn't 'intercept' the message between the keyboard process and X11 windows such as search bar, note dialog, and so forth. Instead, I can 'peek' the messages and with sending backspace key event, unwanted characters can be deleted while filling up the space with characters we want.

(2) The keyboard used by kindle pw3 is capable of generating keycodes range of 8-255 as I queried with X11 API. This keycode is translated into keysym which is the actual value for the character on the screen. In addition, I observed that keycodes from 0xdc to 0xff is dynamically mapped on runtime. The method I used in this hack also changes the mapping of 0xff and 0xfe. But it's harmless.

(3) The keyboard configuration file on the device, storing the keyboard options in the setting, is generated fresh on every reboot. I couldn't find the source of this conf. So, this conf file is being modified on every boot for now.

## 6) TODO
