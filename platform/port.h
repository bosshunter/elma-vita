// port.h - PS Vita port glue shared between the game code and the platform layer.
// (port) Part of the PS Vita adaptation of the Elasto Mania source code.
#ifndef ELMA_PORT_H
#define ELMA_PORT_H

// Physical Vita screen:
#define PORT_FIZX (960)
#define PORT_FIZY (544)

// The classic 640x480 screen used by the level editor (stretched over the
// whole display). The menus are drawn at 960x544, see menukep.h:
#define PORT_KLASSZX (640)
#define PORT_KLASSZY (480)
#define PORT_KLASSZ_OFSX ((PORT_FIZX-PORT_KLASSZX)/2)
#define PORT_KLASSZ_OFSY ((PORT_FIZY-PORT_KLASSZY)/2)

// (port) In-game zoom (Zoom In / Zoom Out keys). Zooming out draws a bigger
// area and shrinks it into the view, so the textures must be at least
// PORT_ZOOMSZEL wide and the work picture PORT_ZOOMSZEL x PORT_ZOOMMAG:
#define PORT_ZOOMMIN  (0.3)
#define PORT_ZOOMMAX  (2.0)
#define PORT_ZOOMSZEL ((int)(PORT_FIZX/PORT_ZOOMMIN)+4)
#define PORT_ZOOMMAG  ((int)(PORT_FIZY/PORT_ZOOMMIN)+4)

// (port) Key codes. 0..255 are the original DirectInput keyboard codes (the
// Vita's own buttons send some of these, see vita_platform.cpp). Above that,
// every button of every external controller has its own code, so any button
// can be bound to any player in Options / Customize controls:
//   PORT_PADKOD( pad 0..3, button bit 0..31 ) = 256 + pad*32 + bit
#define PORT_PADSZAM (4)
#define PORT_PADKOD( pad, bit ) (256 + (pad)*32 + (bit))
#define PORT_KODSZAM (256 + PORT_PADSZAM*32)

// Size of the screen the game is currently drawing into: 960x544, except in
// the level editor, which keeps the classic 640x480 screen.
extern int Port_kepx, Port_kepy;
// Updates Port_kepx/Port_kepy (called when the screen is locked):
void port_kepmeretfrissit( void );

// Nonzero while gameplay is running (full 960x544 view and gameplay buttons):
extern int Port_szeles;
// Nonzero while gameplay button mapping is active (gameplay, or when the
// "Customize controls" screen is waiting for a button):
extern int Port_jatekgombok;

// Put one of these at the top of a function to switch modes for its duration:
struct port_jatek_guard {
	int regiszeles, regigombok;
	port_jatek_guard( void );
	~port_jatek_guard( void );
};
struct port_gombfelvetel_guard {
	int regigombok;
	port_gombfelvetel_guard( void );
	~port_gombfelvetel_guard( void );
};

// Opens the Vita on-screen keyboard and feeds the result into the game's
// keyboard character queue, so the game's own text-entry loop processes it:
//   OK     -> 'torlesszam' x 'torlokod' (to clear the old text), the typed
//             characters, then MK_ENTER (unless enterrel is 0)
//   Cancel -> MK_ESC
// csakszam: numeric keyboard. maxhossz: maximum number of characters.
void port_ime_bevitel( const char* cim, const char* kezdo, int maxhossz,
					   int csakszam, int torlesszam, int torlokod,
					   int enterrel = 1 );

#endif
