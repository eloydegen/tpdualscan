/*  T P D U A L S C A N . C

    Author: Michael Steiner
            <steiner@acm.org> <sti@zurich.ibm.com>
	    http://www.zurich.ibm.com/pub/sti/www/info.html

   This program allows you to set videoram for dualscan equipped thinkpads 
   (at least for the 750Cs but probably also for others) to allow running 
   XFree86 without patching it. (normally you got all twice if the upper 512K 
   of videoram are not disabled) 
 */

#define PROGNAME "tpdualscan"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

/* the prototypes for helper routines (freely adapted from Xfree86 */
static __inline__ void outb(short port, char val);
static __inline__ unsigned int inb(short port);
void EnableIOPorts(int num_ioports, unsigned *io_ports);
void DisableIOPorts(int num_ioports, unsigned *io_ports);


/* define IO ports for vga and pvga (we probably don't need all but why bother ..) */
unsigned PVGA_IOPorts[] = {
    /* normal vga ports */
    0x3B4, 0x3B5, 0x3BA, 0x3C0, 0x3C1, 0x3C2, 0x3C4, 0x3C5, 0x3C6, 0x3C7, 
    0x3C8, 0x3C9, 0x3CA, 0x3CB, 0x3CC, 0x3CE, 0x3CF, 0x3D4, 0x3D5, 0x3DA,
    /* extra ports for WD90C31 */
    0x23C0, 0x23C1, 0x23C2, 0x23C3, 0x23C4, 0x23C5 };
int Num_PVGA_IOPorts   = (sizeof(PVGA_IOPorts)/sizeof(PVGA_IOPorts[0]));

char usage[] =  "usage: "PROGNAME"(-d(isable) | -e(nable) | -s(tatus))";


void main (int argc, char *argv[]) {
    unsigned long			vgaIOBase;
    int 				colorEmulation, dualScan;
    enum {enable, disable, status}	mode = status;
    unsigned char			registr; /* ugly but register was already taken .. */

    /* parse command line */
    if ((argc > 2) || ((argc == 2) && (argv[1][0] != '-'))){
	fprintf(stderr, "%s\n", usage); exit(1);
    }
    if (argc == 2) {
	switch (argv[1][1]) {
	case 'h':
	    printf("%s\ndisable/enable upper 512K of VRAM for dual-scan thinkpads\n", usage); exit(0);
	case 'd':		/* disable */
	    mode = disable; break;
	case 'e':		/* enable */
	    mode = enable; break;
	case 's':		/* status */
	    mode = status; break;
	default:
	    fprintf(stderr, "invalid option %c\n%s\n", argv[1][1], usage); exit(1);
	}
    }

    /* enable VGA and WD ports */
    EnableIOPorts(Num_PVGA_IOPorts, PVGA_IOPorts);

    /* set vga IO base depend from color mode */
    colorEmulation = inb(0x3CC) & 0x01;
    if (!colorEmulation) {
	fprintf(stderr, "you are not running a dual scan thinkpad \n");
	exit(1);
    }
    vgaIOBase = (colorEmulation) ? 0x3D0 : 0x3B0;

    /* unlock flat panel registers PR18, PR19, PR!a, PR36-PR41 and PR44 */
    outb(vgaIOBase + 4, 0x34);	/* PR1B */
    registr = inb(vgaIOBase + 5); 
    outb(vgaIOBase + 5, ((registr & 0x1F) | 0xA0)); /* set bits 5-7 to 101 */
    
    /* check if we have a dual-scan LCD */
    outb(vgaIOBase + 4, 0x31);
    dualScan = ((inb(vgaIOBase + 5) & 0xB) == 0); /* dual scan ==  (PR18[bit:3,1,0] = 0)  */
    if (!dualScan) {
	fprintf(stderr, "you are not running a dual scan thinkpad \n");
	exit(1);
    }

    /*  clear bit 6 to disable upper 512k The remaining 512Kbytes
	of video memory are not disabled by clearing the bit, but it
	is actually used as a shadow of first 512Kbyte */
    outb(0x3CE, 0x0B);
    registr = inb(0x3CF);	/* PR1 */
	
    if (mode == disable) {
	printf("setting vramsize to 512K\n");
	outb(0x3CE, 0x0B); outb(0x3CF, registr & 0xBF); /* enable 512 */
    } else if (mode == enable) {
	printf("setting vramsize to 1024K\n");
	outb(0x3CE, 0x0B); outb(0x3CF, registr | 0xC0); /* enable 1024 */
    } else {
	printf("current vramsize is %dK\n", ( (registr & 0x40) ? 1024 : 512));
    }

    /* cleanup */
    /* lock flat panel registers PR18, PR19, PR!a, PR36-PR41 and PR44 */
    outb(vgaIOBase + 4, 0x34);	/* PR1B */
    registr = inb(vgaIOBase + 5); 
    outb(vgaIOBase + 5, (registr & 0x1F)); /* set bits 5-7 to other than 101 */
    
    /* enable VGA and WD ports */
    DisableIOPorts(Num_PVGA_IOPorts, PVGA_IOPorts);
}

/* helper functions (originaly borrowed from XFree86 but stripped unnecessary stuf) f*/

void EnableIOPorts(int num_ioports, unsigned *io_ports)
{
    int i;

    if (iopl(3) != 0) {			/* for ports > 3FF */
	fprintf(stderr, "%s: Failed to raise I/O privileges to 3 (%s)\n",
		PROGNAME, strerror(errno));
    }

    for (i = 0; i < num_ioports; i++)
    {
	unsigned port = io_ports[i];

	if (port > 0x3FF)
	    continue;

	if (ioperm(port, 1, 1) < 0) {
	    fprintf(stderr, "%s: Failed to enable I/O port 0x%x (%s)\n",
		    PROGNAME, port, strerror(errno));
	    exit(1);
	}
    }
    return;
}

void DisableIOPorts(int num_ioports, unsigned *io_ports)
{
    int i;

    if (iopl(0) != 0) {			/* for ports > 3FF */
	fprintf(stderr, "%s: Failed to lower I/O privileges to 0 (%s)\n",
		PROGNAME, strerror(errno));
    }

    for (i = 0; i < num_ioports; i++)
    {
	unsigned port = io_ports[i];

	if (port > 0x3FF)
	    continue;

	if (ioperm(port, 1, 0) < 0) {
	    fprintf(stderr, "%s: Failed to disable I/O port 0x%x (%s)\n",
		    PROGNAME, port, strerror(errno));
	}
    }
    return;
}

static __inline__ void
outb(port, val)
     short port;
     char val;
{
  __asm__ __volatile__("out%B0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
inb(port)
     short port;
{
  unsigned char ret;
  __asm__ __volatile__("in%B0 (%1)" :
		   "=a" (ret) :
		   "d" (port));
  return ret;
}
