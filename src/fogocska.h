
void fogocskareset( void );
void fogocska( double t );
// Aafogo alapjan csak idoket allitja:
void fogocska_r( double t );

extern int Aafogo, Kozelvannak;
extern int Aleszfogo; // Legkozelebbi jateknal
extern double Afogoido, Bfogoido;

// (port) Flag Tag with 1..4 players. With 2 players the original functions
// above run unchanged (so .rec replays stay identical) and these arrays are
// filled from them by fogo_ketbol(); with 3 or 4 players the _n functions run.
extern int Fogo;              // player holding the flag (0..3)
extern int Kozel[4];          // player is touching the holder (no pass until apart)
extern double Fogoido[4];     // time each player has held the flag
extern int Kovfogo;           // who starts with the flag next game (3/4 players)
void fogo_ketbol( void );     // 2 players: copy Aafogo... into the arrays
void fogocskareset_n( int n );
void fogocska_n( double t, int n );
void fogocska_r_n( double t );
// The holder (player 'ki') died at 'hely' and restarts: the flag goes to the
// player nearest that place.
void fogo_meghalt_n( int ki, int n, vekt2 hely );
// Players out of the game for good (killed by a killer object): the flag never
// goes to them. Set by lejatszo.
extern int Fogohalott[4];
