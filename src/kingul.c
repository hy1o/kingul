/*  
    kingul - Korean Keyboard for Kindle Paperwhite3
    Version - 0.3
    Copyright (c) 2016 by hylo (hylo.cafe24.com), with MIT license:
    http://www.opensource.org/licenses/mit-license.php
    
  If you want to look more about how Hangul is processed in here,
  https://en.wikipedia.org/wiki/Korean_language_and_computers
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <unistd.h> 
#include <X11/keysym.h>
#include <X11/XKBlib.h>

#define DELAY_ACTIVE	10000;	    // 10ms
#define DELAY_INACTIVE	3000000;    // 3s
#define DELAY_SLEEP	10000000;    // 10s

#define IS_CONSONANT(x) (0x3131<=x&&x<=0x314E)
#define IS_VOWEL(x)	(0x314F<=x&&x<=0x3163)
#define IS_SYLLABLES(x) (0xac00<=x&&x<=0xd7a3)
#define IS_JAMO(x)	(IS_CONSONANT(x)||IS_VOWEL(x))
#define IS_HANGUL(x)	(IS_JAMO(x)||IS_SYLLABLES(x))

#define DISASSEMBLE(xx, c1, v, c2)	\
    do {				\
    int x = xx - 0xac00;		\
    c2 = x % 28;			\
    v = ((x-c2)/28) % 21;		\
    c1 = (((x-c2)/28)-v)/21;} while(0)

#define ASSEMBLE(x, c1, v, c2)		\
    do{x = 0xac00 + (((c1*21)+v)*28+c2);} while(0)

#define d_log(x, ...) \
    if (debug && (x)) printf(__VA_ARGS__)

//ㄱㄲㄳㄴㄵㄶㄷㄸㄹㄺㄹㄻㄼㄽㄾㄿㅀㅁㅂㅃㅄㅅㅆㅇㅈㅉㅊㅋㅍㅎ
//ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ
int init_cons_ofs[] = {0, 1, -1, 2, 92, 93, 3, 4/*23*/, 5, 39012, 
		    39016, 39017, 39020, -1, -1, 26, 6, 7, 8, 33, 
		    9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
int final_cons_ofs[] = {1, 2, 3, 4, 5, 6, 7, 35, 8, 9, 
		    10, 11, 12, 13, 14, 15, 16, 17, 50751, 18, 
		    19, 20, 21, 22, 50770, 23, 24, 25, 26, 27};

int period = 300;
int cnt = 0;
bool debug = false;
int delay = DELAY_INACTIVE;

void sendKey (XEvent *xev, KeySym keysym, int numBS, int victim);
void printKeyEvent (XKeyEvent e);
int getHangul(int keysym, int prevKeysym, int *prevHangul);
int get_jamo_index(int jamo, int final);
int final_to_init_cons (int jamo);
int compose_vowels(int v1, int v2);
int compose_final_consonants(int c1, int c2);
int decompose_consonants(int c, int *c_first, int *c_second);


int main(int argc, char *argv[]) 
{
    Window root, screen;
    Window parent, *children; 
    Display * display;
    XEvent xev;
    int minKeycode, maxKeycode;
    int hangul, prevHangul;
    int keysym, prevKeysym = 0;
    int revert_to, numBS;
    int ret, i;
    unsigned int num_children;
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

	delay = DELAY_INACTIVE;
	ret = XGetInputFocus(display, &screen, &revert_to);
	/*
	if (ret) { // it always give BadRequest(1), but the right screen 
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

	if (!XQueryTree(display, screen, &root, &parent, &children, &num_children)) {
	    if (children) XFree((char *)children);
	    usleep(delay);
	    continue;
	}
	if (children) XFree((char *)children);

	// Adjust the delay
	if (screen != 0 && XFetchName(display, screen, &name) > 0) {
	    d_log(cnt/period, "Window name: %s\n", name);
	    if (strstr(name, "searchBar") || strstr(name, "NoteEditorDialog")) {
		delay = DELAY_ACTIVE;
	    } else if (strstr(name, "screenSaver")) {
		delay = DELAY_SLEEP;
	    }
	    XFree(name);
	/* There are some windows without a name: Experimental browser */
	} else if (parent != 0 && XFetchName(display, parent, &name) > 0) {
	    d_log(cnt/period, "Parents window name: %s\n", name);
	    if (strstr(name, "com.lab126.browser")) {
		delay = DELAY_ACTIVE;
	    }
	    XFree(name);
	}

	if (screen == 0x0) {
	    usleep(delay);
	    continue;
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
	    usleep(delay);
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
	usleep(delay);
    } /* while */

    XCloseDisplay(display);
    return 0;
}

void sendKey (XEvent *xev, KeySym keysym, int numBS, int victim)
{
    int i;

    if (keysym == 0)
	return;

    /* backspace */
    xev->xkey.keycode = 0x16;
    xev->type = KeyPress;
    xev->xkey.type = KeyPress;
    xev->xkey.state = 0;
    for (i = 0; i < numBS; ++i)
	XSendEvent(xev->xkey.display, xev->xkey.window, False, KeyPressMask, (XEvent*)xev);

    /* composed character */
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
     * pc2 - previous final consonant
     */
    int c1, v, c2, pc2;
    int jamo, ret;

    c1 = v = c2 = pc2 = ret = 0;
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
		if(c2 != 0 && ((jamo = compose_final_consonants(c2, jamo)) < 0))
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
		    {
			DISASSEMBLE(prevKeysym, c1, v, c2);
			if(decompose_consonants(c2, &pc2, &c2) < 0)
			    return 0;
			ASSEMBLE(*prevHangul, c1, v, pc2);
			c1 = c2;
		    }
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
	/* Special consonants 
	 * case 3: case 5: case 6: 
	 * case 9: case 10: case 11: 
	 * case 12: case 13:case 14: 
	 * case 15: case 18:  
	 */
	case 1:	case 2:		
	case 4:	case 7:
	    return init_cons_ofs[fin-1];
	case  8: case 16: case 17:
	    return init_cons_ofs[fin];
	case 19: case 20: case 21: 
	case 22:
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
    if (v1 == 8 && v2 == 0) /* ㅗ and ㅏ */
	return 9;
    if (v1 == 8 && v2 == 1) /* ㅗ and ㅐ */
	return 10;
    if (v1 == 8 && v2 == 20) /* ㅗ and ㅣ */
	return 11;
    if (v1 == 13 && v2 == 4) /* ㅜ and ㅓ */
	return 14;
    if (v1 == 13 && v2 == 5) /* ㅜ and ㅔ */
	return 15;
    if (v1 == 13 && v2 == 20) /* ㅜ and ㅣ */
	return 16;
    if (v1 == 18 && v2 == 20) /* ㅡ and ㅣ */
	return 19;
    return -1;
}

int compose_final_consonants(int c1, int c2)
{
    if (c1 == 1 && c2 == 19) /* ㄳ */
	return 3;
    if (c1 == 4 && c2 == 22) /* ㄵ */
	return 5;
    if (c1 == 4 && c2 == 27) /* ㄶ */
	return 6;
    if (c1 == 8 && c2 == 1) /* ㄺ */
	return 9;
    if (c1 == 8 && c2 == 16) /* ㄻ */
	return 10;
    if (c1 == 8 && c2 == 17) /* ㄼ */
	return 11;
    if (c1 == 8 && c2 == 19) /* ㄽ */
	return 12;
    if (c1 == 8 && c2 == 25) /* ㄾ */
	return 13;
    if (c1 == 8 && c2 == 26) /* ㄿ */
	return 14;
    if (c1 == 8 && c2 == 27) /* ㅀ */
	return 15;
    if (c1 == 17 && c2 == 19) /* ㅄ */
	return 18;
    return -1;
}

/* returns c_first as final consonant, c_second as initial */
int decompose_consonants(int c, int *c_first, int *c_second)
{
    if (c == 3) {
	*c_first = 1;
	*c_second = 9;
    } else if (c == 5) {
	*c_first = 4;
	*c_second = 12;
    } else if (c == 6) {
	*c_first = 4;
	*c_second = 18;
    } else if (c == 9) {
	*c_first = 8;
	*c_second = 0;
    } else if (c == 10) {
	*c_first = 8;
	*c_second = 6;
    } else if (c == 11) {
	*c_first = 8;
	*c_second = 7;
    } else if (c == 12) {
	*c_first = 8;
	*c_second = 9;
    } else if (c == 13) {
	*c_first = 8;
	*c_second = 16;
    } else if (c == 14) {
	*c_first = 8;
	*c_second = 17;
    } else if (c == 15) {
	*c_first = 8;
	*c_second = 18;
    } else if (c == 18) {
	*c_first = 17;
	*c_second = 9;
    } else {
	return -1;
    }

    return 1;
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
