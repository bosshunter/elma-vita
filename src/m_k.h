// m_k.h

int mk_getextchar( void );
void mk_emptychar( void );
int mk_kbhit( void );
int mk_getstate( int kod );

#ifdef W_95

#define MK_ESC    (1) // DIK_ESCAPE
#define MK_ENTER  (400)
#define MK_UP     (401)
#define MK_DOWN   (402)
#define MK_LEFT   (403)
#define MK_RIGHT  (404)
#define MK_PGUP   (405)
#define MK_PGDOWN (406)
#define MK_DEL	  (407) 
#define MK_BACKSPACE (408)

#else

#define MK_ESC    (1)
#define MK_ENTER  (28)
#define MK_UP     (22) // (72) // ez nem 40-el kisebb, mivel az SPACE lenne
#define MK_DOWN   (40) // (80)
#define MK_LEFT   (35) // (75)
#define MK_RIGHT  (37) // (77)
#define MK_PGUP   (33) // (73)
#define MK_PGDOWN (41) // (81)
#define MK_DEL	  (43) // (83)
#define MK_BACKSPACE (14)

#endif

