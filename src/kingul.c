/*  
    kingul - Korean Keyboard for Kindle Paperwhite3
    Version - 0.1
    Copyright (c) 2016 by hylo (www.hylo.cafe24.com), with MIT license:
    http://www.opensource.org/licenses/mit-license.php
    
  If you want to look more about how Hangul is processed in here,
  https://en.wikipedia.org/wiki/Korean_language_and_computers
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <unistd.h> 
#include <X11/keysym.h>
#include <X11/XKBlib.h>

#define DELAY 10000

#define IS_CONSONANT(x) (0x3131<=x&&x<=0x314E)
#define IS_VOWEL(x)	(0x314F<=x&&x<=0x3163)
#define IS_SYLLABLES(x) (0xac00<=x&&x<=0xd7a3)
#define IS_JAMO(x)	(IS_CONSONANT(x)||IS_VOWEL(x))
#define IS_HANGUL(x)	(IS_JAMO(x)||IS_SYLLABLES(x))

#define DISASSEMBLE(x, c1, v, c2)	\
    do {				\
    x -= 0xac00;			\
    c2 = x % 28;			\
    v = ((x-c2)/28) % 21;		\
    c1 = (((x-c2)/28)-v)/21;} while(0)

#define ASSEMBLE(x, c1, v, c2)		\
    do{x = 0xac00 + (((c1*21)+v)*28+c2);} while(0)

#define d_log(x, ...) \
    if (debug && (x)) printf(__VA_ARGS__)

//ㄱㄲㄳㄴㄵㄶㄷㄸㄹㄺㄹㄻㄼㄽㄾㄿㅀㅁㅂㅃㅄㅅㅆㅇㅈㅉㅊㅋㅍㅎ
//ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ
int init_cons_ofs[] = {0, 1, -1, 2, 92, 93, 3, 23, 5, 39012, 
		    39016, 39017, 39020, -1, -1, 26, 6, 7, 8, 33, 
		    9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
int final_cons_ofs[] = {1, 2, 3, 4, 5, 6, 7, 35, 8, 9, 
		    10, 11, 12, 13, 14, 15, 16, 17, 50751, 18, 
		    19, 20, 21, 22, 50770, 23, 24, 25, 26, 27};

int period = 300;
int cnt = 0;
bool debug = false;

void sendKey (XEvent *xev, KeySym keysym, int numBS, int victim);
void printKeyEvent (XKeyEvent e);
int getHangul(int keysym, int prevKeysym, int *prevHangul);
int get_jamo_index(int jamo, int final);
int final_to_init_cons (int jamo);
int compose_vowels(int v1, int v2);
int compose_consonants(int c1, int c2);
int decompose_consonants(int c, int *c_first, int *c_second);


int main(int argc, char *argv[]) 
{
    Window root, screen;
    Display * display;
    XEvent xev;
    int minKeycode, maxKeycode;
    int hangul, prevHangul;
    int keysym, prevKeysym = 0;
    int revert_to, numBS;
    int ret, i;
    char *name = NULL;

    XSetWindowAttributes attr;
    XWindowAttributes wattr;

    display = XOpenDisplay(NULL);
    if(display == NULL) {
	printf("Could not open display\n");
	return 1;
    }

    XDisplayKeycodes(display, &minKeycode, &maxKeycode);
    d_log(true, "minKeycode=%d, maxKeycode=%d\n", minKeycode, maxKeycode);

    root = DefaultRootWindow(display);
    printf("Root Window: %x\n", root);

    if (XFetchName(display, root, &name) > 0) {
	printf("Window name: %s\n", name);
	XFree(name);
    }

    while(true)	{
	if(cnt > period) 
	    cnt = 0;

	ret = XGetInputFocus(display, &screen, &revert_to);
	/*
	if (ret) { // it always give BadRequest(1), but right screen 
	    printf("get input focus failed ret = %d\n", ret);
	    printf("Focused window = %x\n", screen);
	    continue;
	} else */ if (screen == None) {
	    printf("No focused window\n");
	    continue;
	} else {
	    d_log(cnt/period, "Focused window = %x\n", screen);
	    // FIXME: If window has changed, initialize prevKeysym
	}

	if (XFetchName(display, screen, &name) > 0) {
	    d_log(cnt/period, "Window name: %s\n", name);
#if 0
	    /* There is some windows doesn't have a name: Experimental browser */
	    if (!strstr(name, "searchBar") && !strstr(name, "NoteEditorDialog")) {
		XFree(name);
		usleep(DELAY);
		continue;
	    }
#endif
	    XFree(name);
	}

	attr.event_mask = KeyReleaseMask;
	ret = XGetWindowAttributes(display, screen, &wattr);
	if (wattr.all_event_masks & ButtonPressMask)
	    attr.event_mask &= ~ButtonPressMask;
	attr.event_mask &= ~SubstructureRedirectMask;

	XSelectInput(display, screen, attr.event_mask);
	
	/* grab key events
	 * XNextEvent is blocking call, so use XCheckWindowEvent instead*/
	if (!XCheckWindowEvent(display, screen, KeyReleaseMask, &xev)) {
	    cnt++;
	    usleep(DELAY);
	    continue;
	}

	prevKeysym = keysym;
	keysym = (int) XLookupKeysym(&xev.xkey, 0);
	if (debug) printKeyEvent(xev.xkey);

	switch(xev.type) {
	    case KeyPress:
		d_log(true, "Key press - 0x%x\n", keysym);
		break;
	    case KeyRelease:
		d_log(true, "Key release - 0x%x\n", keysym);
		d_log(true, "keysym=0x%x, prev=0x%x\n", keysym, prevKeysym);

		if (!IS_SYLLABLES(keysym)) {
	 	    prevHangul = 0;
		    numBS = 2;
		    hangul = getHangul(keysym, prevKeysym, &prevHangul);

		    if (IS_HANGUL(hangul)) {
			if (IS_HANGUL(prevHangul)) {
			    sendKey(&xev, prevHangul, numBS, maxKeycode-1);
			    numBS = 0;
			}

			sendKey(&xev, hangul, numBS, maxKeycode);
		    }
		}
		break;
	} /* switch(xev.type) */
	cnt++;
	usleep(DELAY);
    } /* while */

    XCloseDisplay(display);
    return 0;
}

void sendKey (XEvent *xev, KeySym keysym, int numBS, int victim)
{
    int i;

    if (keysym == 0)
	return;

    xev->xkey.keycode = 0x16;
    xev->type = KeyPress;
    xev->xkey.type = KeyPress;
    for (i = 0; i < numBS; ++i)
	XSendEvent(xev->xkey.display, xev->xkey.window, False, KeyPressMask, (XEvent*)xev);

    XChangeKeyboardMapping(xev->xkey.display, victim, 1, &keysym, 1);
    xev->xkey.keycode = victim;
    xev->type = KeyPress;
    xev->xkey.type = KeyPress;
    XSendEvent(xev->xkey.display, xev->xkey.window, True, KeyPressMask, (XEvent*)xev);
    xev->type = KeyRelease;
    xev->xkey.type = KeyRelease;
    XSendEvent(xev->xkey.display, xev->xkey.window, True, KeyReleaseMask, (XEvent*)xev);
}

int getHangul(int keysym, int prevKeysym, int *prevHangul)
{
    /* c1 - initial consonant
     * v - vowel
     * c2 - final consonant
     */
    int c1, v, c2;
    int jamo, ret;

    c1 = v = c2 = ret = 0;
    if (IS_HANGUL(keysym) && IS_HANGUL(prevKeysym))
    {
	// current keysym should be c2
	if (IS_SYLLABLES(prevKeysym))
	{
	    /* disassemble previous syllable */
	    DISASSEMBLE(prevKeysym, c1, v, c2);
	    if (IS_CONSONANT(keysym))
	    {
		if((jamo = get_jamo_index(keysym, 1)) < 0)
		    return 0;
		if(c2 != 0 && ((jamo = compose_consonants(c2, jamo)) < 0))
		    return 0;
		c2 = jamo;
	    }
	    else if (IS_VOWEL(keysym))
	    {
		if((jamo=get_jamo_index(keysym, 0)) < 0)
		    return 0;
		if (c2 != 0)
		{
		    ASSEMBLE(*prevHangul, c1, v, 0);
		    if(!IS_HANGUL(*prevHangul))
			return 0;
		    if((c1=final_to_init_cons(c2)) < 0)
			return 0;
		    c2 = 0;
		    v = jamo;
		}
		else
		{
		    if((jamo = compose_vowels(v, jamo)) < 0)
			return 0;
		    v = jamo;
		}
	    }
	    ASSEMBLE(ret, c1, v, c2);
	}
	// current keysym is highly possibly a vowel
	else if (IS_CONSONANT(prevKeysym))
	{
	    c1 = get_jamo_index(prevKeysym, 0);
	    if (IS_CONSONANT(keysym))
		return 0;

	    else if (IS_VOWEL(keysym))
	    {
		if((jamo = get_jamo_index(keysym, 0)) < 0)
		    return 0;
		v = jamo;
	    }
	    ASSEMBLE(ret, c1, v, c2);
	}
	// it's not the case
	else if (IS_VOWEL(prevKeysym))
	{
	}
    }

    if (IS_HANGUL(ret))
	return ret;
    else
	return 0;
}

int get_jamo_index(int jamo, int final)
{
    if (IS_CONSONANT(jamo)) {
	jamo -= 0x3131;
	return (final==0? init_cons_ofs[jamo]:final_cons_ofs[jamo]); 
    } else if(IS_VOWEL(jamo)) {
	jamo -= 0x314f;
	return jamo;
    }

    return -1;
}

int final_to_init_cons (int fin)
{
    switch (fin)
    {
	case 1:	case 2:	case 3:
	case 4:	case 5:	case 6:
	case 7:
	    return init_cons_ofs[fin-1];
	case  8: case  9: case 10:
	case 11: case 12: case 13:
	case 14: case 15: case 16: 
	case 17:
	    return init_cons_ofs[fin];
	case 18: case 19: case 20:
	case 21: case 22:
	    return init_cons_ofs[fin+1];
	case 23: case 24: case 25:
	case 26: case 27:
	    return init_cons_ofs[fin+2];
	case 35:
	    return init_cons_ofs[7];
	case 50751: /* 0x3143 - 0xD7E6 */
	    return init_cons_ofs[18];
	case 50770: /* 0x3149 - 0xD7F7 */
	    return init_cons_ofs[24];
    }

    return -1;
}

int compose_vowels(int v1, int v2)
{
    if (v1 == 8 && v2 == 0) /* o and ah */
	return 9;
    if (v1 == 8 && v2 == 1) /* o and ae */
	return 10;
    if (v1 == 8 && v2 == 20) /* o and ee */
	return 11;
    if (v1 == 13 && v2 == 4) /* oo and uh */
	return 14;
    if (v1 == 13 && v2 == 5) /* oo and eh */
	return 15;
    if (v1 == 13 && v2 == 20) /* oo and ee */
	return 16;
    if (v1 == 18 && v2 == 20) /* eu and ee */
	return 19;
    return -1;
}

int compose_consonants(int c1, int c2)
{
    if (c1 == 1 && c2 == 19) /* k s */
	return 3;
    if (c1 == 4 && c2 == 22) /* n z */
	return 5;
    if (c1 == 4 && c2 == 27) /* n h */
	return 6;
    if (c1 == 8 && c2 == 1) /* l k */
	return 9;
    if (c1 == 8 && c2 == 16) /* l m */
	return 10;
    if (c1 == 8 && c2 == 17) /* l b */
	return 11;
    if (c1 == 8 && c2 == 19) /* l s */
	return 12;
    if (c1 == 8 && c2 == 25) /* l t */
	return 13;
    if (c1 == 8 && c2 == 26) /* l p */
	return 14;
    if (c1 == 8 && c2 == 27) /* l h */
	return 15;
    if (c1 == 17 && c2 == 19) /* b s */
	return 18;
    return -1;
}

int decompose_consonants(int c, int *c_first, int *c_second)
{
    // FIXME: To be implemented
    return -1;
}

void printKeyEvent (XKeyEvent e)
{
    printf("serial = %d\n" \
	   "send_event = %d\n" \
	   "window = 0x%x\n" \
	   "root = 0x%x\n" \
	   "subwindow = 0x%x\n" \
	   "x, y = %d,%d,\n" \
	   "x-root, y-root = %d,%d,\n" \
	   "keycode = 0x%x\n" \
	   "same_screen = %d\n" \
	   "state = %d\n\n", 
	   e.serial, e.send_event, e.window, e.root,
	   e.subwindow, e.x, e.y, e.x_root, e.y_root, 
	   e.keycode, e.same_screen, e.state);
}
