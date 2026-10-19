#include	"all.h"


/*void waitforcathode( void ) {
	asm	push	ds

	asm	mov		dx, 3DAh
ret1:
	asm	in		al, dx
	asm	and		al, 8
	asm	jnz		ret1
ret2:
	asm	in		al, dx
	asm	and		al, 8
	asm	jz		ret2

	asm	pop		ds
} */

void waitforcathode( void ) {
	/* Vita port: no VGA port access; vsync is handled by the platform layer. */
}
