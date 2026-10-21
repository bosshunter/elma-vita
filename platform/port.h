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

// ---------------------------------------------------------------------------
// (port) GPU drawing (vita_gpu.cpp). Gameplay and the menus are drawn by the
// Vita's GPU (sceGxm, through vita2d): the level picture the original builds
// when a level is loaded (ecset) is turned into textures piece by piece (the
// pieces are also made ahead on another CPU core), and the sky, the objects,
// the bikes, the navigator and the timer are drawn on top. Zooming is just a
// different scale. An empty file "cpurajz.inf" in the data folder switches
// back to the CPU drawing of v1.0, for comparing.
// ---------------------------------------------------------------------------
class pic8;
class kiskep;
int  port_gpu( void );           // nonzero: the GPU draws gameplay and menus
void port_gpu_init( void );      // after vita2d_init
void port_gpu_szunet( void );    // the level is about to change: stop using it
extern int Port_lgrgen;          // changes with every LGR file loaded

// Gameplay frame. paletta: the level's 256 colours (as ddpal keeps them),
// atlatszo: the transparent colour of the level pieces, szintgen: changes
// with every new level picture.
void port_gj_kezd( const void* paletta, int atlatszo, int szintgen );
void port_gj_vege( int kepmentes ); // shows the frame (kepmentes: save a screenshot first)
// The screen rectangle (x, y, w, h) tiled with this picture (the gaps between the views):
void port_gj_hatter( pic8* minta, int x, int y, int w, int h );
// One view: its rectangle on the screen and the size of the picture drawn
// into it (bigger than the rectangle when zoomed out). The coordinates of the
// following calls are in that picture, like the game's screen buffer: x to
// the right, y up from the bottom row.
void port_gj_nezet( int sx, int sy, int sw, int sh, int vw, int vh );
void port_gj_skala( int vw, int vh );   // changes the picture size (after the zoomed part)
void port_gj_nezet_vege( void );
void port_gj_eg( pic8* peg, int egxmod, int egymod, int egdx ); // the sky (moves at half speed)
// The level pieces covering the view; reteg 0: behind the bikes, 1: in front.
// (xe1, ye1): the level pixel at the view's bottom left corner.
void port_gj_reteg( int reteg, int xe1, int ye1 );
void port_gj_kep( pic8* p, int x, int y );   // a picture like blt8 (sprite: transparent colour)
// A bike part: picture k's pixel (0,0) is at r, (xsize-1,0) at r+u,
// (0,ysize-1) at r+v:
void port_gj_negyszog( kiskep* k, float rx, float ry, float ux, float uy, float vx, float vy );
// The navigator: rectangle (x, y, w, h) of p (rows like the screen buffer),
// drawn at the same place in the view:
void port_gj_resz( pic8* p, int x, int y, int w, int h );
// The timer: every pixel of the rectangle gets the colour lookup[colour under
// it], like pic8::vizszegmens_look does on the CPU:
void port_gj_look( int x, int y, int w, int h, const unsigned char* lookup );
// Makes level pieces (kiraj320.cpp): reteg 0/1, into dest (stride bytes a row),
// with the thread's own cursors (kurzorszam: 0 main, 1 helper):
extern void (*Port_csempefv)( int reteg, unsigned char* dest, int stride,
							  int x0, int y0, int w, int h, int kurzorszam );

// (port) Saves a file in the background (state.dat, state34.dat): a thread on
// another CPU core writes it, and only if its content changed since the last
// time. port_fajlmentes_var waits until everything is written (at exit).
void port_fajlmentes( const char* nev, const void* adat, int hossz );
void port_fajlmentes_var( void );

// Menus: the two background pictures (plain and with the balls' picture) and
// 'verzio', which changes whenever they were redrawn; then the frame: discs
// cut out of the pictures (the balls) and the helmet.
struct port_korong { float x, y, r; int golyobol; };
void port_gm_hatter( pic8* sima, pic8* golyo, int verzio );
void port_gm_rajz( int db, const port_korong* k, pic8* sisak, int sisakx, int sisaky );
void port_gm_ujra( int dialoggal ); // the last menu frame again (dialoggal: with the system dialog)

// Nonzero while a gameplay frame is being drawn by the GPU (dobozki and the
// timer's colour lookup send their drawing to the GPU then):
extern int Port_gjrajz;
// What is on the screen now: 0 the CPU screen buffer, 1 a GPU gameplay frame,
// 2 a GPU menu frame (vita_platform.cpp):
extern int Port_kepforras;
// The 256 colours on the screen now (ABGR):
const unsigned int* port_paletta( void );

#endif
