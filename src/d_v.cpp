#include    "all.h"

int Win_95 = 0;

void    billstart( void );
void    billstop( void );

/*static void settext( void ) {
    if( Vid640 ) {
        close640();
        Vid640 = 0;
    }
    if( Vid320 ) {
        settextmode();
        Vid320 = 0;
    }
} */

/*static void vegeszoveg( void ) {
	printf( "Thank you for playing Elasto Mania.\n" );
    printf( "\n" );
    //printf( "Evaluation version for Epic MegaGames.\n" );
    //printf( "Evaluation version for Broderbund Software, Inc.\n" );
    printf( "Evaluation version.\n" );
    printf( "Do not distribute!\n" );
    printf( "Copyright 1996 Bal zs R¢zsa\n" );
    printf( "\n" );
    //printf( "Original idea, Programming, Sound, Levels: Balazs Rozsa\n" );
    //printf( "Art: Barczi Imre, Balazs Rozsa\n" );
    //printf( "Testers: Barczi Imre, Csaba Rozsa, Andras Torok, Csaba Fekete,\n" );
    //printf( "           Tamas Kristyan, Akos Nemeth Buhin\n" );
    //printf( "Special thanks to Gabor Gerenyi and Tamas Kristyan for some sound files.\n" );
} */



/*static void vegeszoveg( void ) {
	#ifdef SARVARI
		printf( "Elasto Mania shareware version 1.2 Copyright (C) 1997, 1998 Bal zs R¢zsa       \n" );
		printf( "                                                                               \n" );
		printf( "You are welcome to distribute this program freely, as long as you don't        \n" );
		printf( "change it in any way. If you like Elasto Mania and continue to use it,         \n" );
		printf( "I ask you to register. I depend on your registrations.                         \n" );
		printf( "                                                                               \n" );
		printf( "When you register Elasto Mania for $29.95, you will get the registered         \n" );
		printf( "version, which has 26 more new levels and the full editor. Please add $4 for   \n" );
		printf( "shipping. Registered users of previous versions can upgrade to version 1.2     \n" );
		printf( "for free, read order.txt.                                                      \n" );
		printf( "                                                                               \n" );
		printf( "To register, send a check or money order payable to: Pik A Program (tm)        \n" );
		printf( "                                                     13 Saint Marks Place      \n" );
		printf( "                                                     NY, NY 10003              \n" );
		printf( "                                                     USA                       \n" );
		printf( "For Credit Card orders, call TOLL FREE (ORDERS ONLY) 1-800-867-3447.           \n" );
		printf( "For Online Compuserve orders, GO SWREG (product ID.: 12817).                   \n" );
		printf( "You can register and immediately download Elastomania on the WEB at            \n" );
		printf( "                                                http://www.pik.com             \n" );
		printf( "                                                                               \n" );
		printf( "If you have any question about Elasto Mania, visit www.elastomania.com    \n" );
		printf( "or send an e-mail to:     feedback@elastomania.com                             \n" );
		printf( "                                                                               \n" );
		printf( "Press a key!\n" );
		//getche();
		//exit( 0 );
	#else
		for( int i = 0; i < 6; i++ )
			printf( "\n" );
		printf( "Elasto Mania registered version 1.2 Copyright (C) 1997, 1998 Bal zs R¢zsa \n" );
		printf( "                                                                               \n" );
		printf( "                          Do not distribute!                                   \n" );
		printf( "                                                                               \n" );
		printf( "               Thank you for registering Elasto Mania.                    \n" );
        printf( "If you encounter problems running the game, please refer to the file           \n" );
        printf( "tech_inf.txt. In this file you can find information on how to get technical    \n" );
        printf( "support also.                                                                  \n" );
        printf( "                                                                               \n" );
		printf( "You can find the homepage of Elasto Mania at:                            \n" );
		printf( "  www.elastomania.com				\n" );
		printf( "Here you can download some more levels to play.                                  \n" );
		printf( "\n" );
		printf( "Press a key!\n" );
	#endif
} */

static int Margrafikus = 0;

// Ezekbol nem johet vissza:
static void directhiba( char* text1, char* text2, char* text3, char* text4 ) {

    // torlendo:
    //mk_getextchar();

    if( Margrafikus ) {
        grafikushiba( text1, text2, text3 );
        mk_getextchar();
    }

    settextmode();
	/*if( strcmp( text1, "Kiir szoveg!" ) == 0 ) {
		//vegeszoveg();
		stophang();
		mk_emptychar();
		while( !mk_kbhit() )
			;
		if( Billint )
			billstop();
		else
			getch();
		// Visszaall sor elejere:
		#ifdef WATCOM
			_settextposition( 24, 1 );
			_outtext( "                                      " );
			_settextposition( 23, 60 );
			_outtext( "  " );
		#else
			gotoxy( 1, 23 );
			printf( "                                      " );
			gotoxy( 46, 21 );
		#endif
		exit( 0 );

		//mv_startstopper();
		//while( 1 ) {
		//	if( kbhit() || mv_stopperido() > 2*182.0 ) {
		//		//if( kbhit() )
		//		//  getch();
		//		exit( 0 );
		//	}
		//}
	} */

	printf( "%s\n", text1 );
    if( text2 )
        printf( "%s\n", text2 );
    if( text3 )
        printf( "%s\n", text3 );
    if( text4 )
        printf( "%s\n", text4 );
    printf( "Press a key!\n" );

    mk_emptychar();
    while( !mk_kbhit() )
        ;
    if( Billint )
        billstop();
    else
        getch();
    exit( 0 );
}

static int Tovabb;

void hiba( char* text1, char* text2, char* text3 ) {
    Tovabb = 0;
    if( Tovabb )
        return;
	//if( strcmp( text1, "Kiir szoveg!" ) == 0 )
	//    directhiba( text1, text2, text3, NULL );
    if( access( "message.inf", 0 ) == 0 )
        directhiba( "message.inf accessed\n", text1, text2, text3 );
    directhiba( "Sorry, internal error.", NULL, NULL, NULL );
}

void uzenet( char* text1, char* text2, char* text3 ) {
    // about azert kell, hogy kezdo memchecknel ne agazzon el:
    if( strstr( text1, "memory" ) && !strstr( text1, "about" ) ) {
        // Ha memory szerepel text1-ben:
        if( access( "message.inf", 0 ) == 0 ) // Most mem-et is csak inf-re adja:
            directhiba( "message.inf accessed\n", text1, text2, text3 );
        directhiba( "Sorry, out of memory!", NULL, NULL, NULL );
    }
    directhiba( text1, text2, text3, NULL );
}

// Ugyanaz mint random, de az nincs WATCOM-ban:
int s_random( int tartomany ) {
    return rand() % tartomany;
}

// Normalisra allitja orat:
static void setseb( void ) {
	unsigned short sebesseg = 0xffff;
	outp( 0x43, 0x34 );
	outp( 0x40, (unsigned char)(sebesseg & 0x0ff) );
	outp( 0x40, (unsigned char)(sebesseg >> 8) );
}

int Billint = 1;

void main( /*int argc, const char* argv[]*/ void ) {
	//void midnightteszt( void );
	//midnightteszt(); // Ez ki is lep

	void becsomagol( void );
	//becsomagol();

	makeupgradefile(); // upgrade.cpp-ben egy define-nal lehet aktivizalni

	void kulonexe_szetpcx( void );
	//kulonexe_szetpcx();

	//makelgrfile();

	// Ez bar mindig meghivodik, altalaban ki van define-olva:
	titkositreadme();

	// Ha nem shareware, megprobal upgrade-elni emupg.res-bol:
	upgradehakell();

	Hangenabled = 0;
	#ifdef WATCOM
		Hangenabled = 1;
	#endif

	#ifdef DOSX286
		Hangenabled = 0;
	#endif

	setseb();

	initqopen();

    //tesztkoveto();

	void initmenukep1( void );
    initmenukep1();
    void initmenukep2( void );
	initmenukep2();

	// ITT ALLITJUK BE MODOT:
    setmode8_l( 4 );
    init_d_dd();

    Margrafikus = 1;

    //hiba( "grproba!" );

    void kulonexe_abcmake( void );
	//kulonexe_abcmake();

	mk_init();
    if( Billint )
        billstart();

    void    initmou( int xsize, int ysize );
    initmou( 640, 480 );
    setmou( 320, 240 );
    getmou( &Moux, &Mouy );

    // Megprobaljuk kozepre hozni egeret:
    for( int i = 0; i < 1000; i++ )
        getmou( &Moux, &Mouy );

    // Ezt vegleges verzioba bele kell tenni:
	//check_kommand( argv[0] );

	// Ez mindig meghivodik, de csak #define BELSOTIR eseten mukodik:
	hakellmentbelleveket();

	// Ez mindig meghivodik, de csak #define KIIRSHAREWARELISTA eseten
	// hmukodik:
	kiirsharewarelista();

    mv_main();

    hiba( "main vegere nem szabadna jonnie" );
}

void mv_check( /*char* honnan,*/ int nemkellnyomas ) {
	nemkellnyomas++;
};

/*void BYTECOPY( void* dest, void* source, int size ) {
	if( (int)dest > 0x1fffffff || (int)source > 0x1fffffff )
		hiba( "BYTECOPY-ban (int)dest||source > 0x1fffffff!" );
	memcpy( dest, source, size );
} */

// ALLOC OVERRIDE:

int Allokalt = 0;
int Alloksize = 0;

#define ALLOKTOMBSIZE (5000)

struct allokstruct {
	int size; // Ha size nagyobb nulla, akkor aktiv
	void* ptr;
};

static allokstruct Alloktomb[ALLOKTOMBSIZE];

static int Elsoallokhivas = 1;

void *operator new( size_t size ) {
	if( Elsoallokhivas ) {
		Elsoallokhivas = 0;
		for( int i = 0; i < ALLOKTOMBSIZE; i++ ) {
			Alloktomb[i].size = 0;
			Alloktomb[i].ptr = NULL;
		}
	}

	if( size <= 0 )
		hiba( "void *operator new-ban size <= 0!" );
	Allokalt++;
	Alloksize += size;
	void* ptr = malloc( size );

	// Keres ures helyet tombben:
	for( int i = 0; i < ALLOKTOMBSIZE; i++ ) {
		if( Alloktomb[i].size == 0 ) {
			// Megvan ures hely:
			Alloktomb[i].size = size;
			Alloktomb[i].ptr = ptr;

			return ptr;
		}
	}

	hiba( "void *operator new-ban nnics tombben hely!" );
	return NULL;
}

void operator delete ( void* ptr ) {
	if( ptr == NULL )
		hiba( "void operator delete-ben ptr == NULL!" );

	// Megkeresi tombben mutatot:
	for( int i = 0; i < ALLOKTOMBSIZE; i++ ) {
		if( Alloktomb[i].ptr == ptr ) {
			// Megvan mutato:
			Alloksize -= Alloktomb[i].size;
			Allokalt--;

			// Lenullazzuk torolt teruletet, hogyha valaki ervenytelenul
			// hasznalja, gp-zen:
			BYTESET( ptr, 0, Alloktomb[i].size );

			Alloktomb[i].size = 0;
			Alloktomb[i].ptr = NULL;

			free( ptr );
			return;
		}
	}

	hiba( "void operator delete-ben nem talalja tombben ptr-t!" );
}


