#ifdef USE_MISC_COLORS
const ULONG misc_colors[48] =
{
	0xffffffff,0x66666666,0x00000000,
	0xa0a0a0a0,0xa0a0a0a0,0xa0a0a0a0,
	0xffffffff,0x00000000,0x00000000,
	0xbbbbbbbb,0xaaaaaaaa,0x99999999,
	0xffffffff,0xaaaaaaaa,0x99999999,
	0xffffffff,0xffffffff,0xffffffff,
	0x66666666,0x88888888,0xbbbbbbbb,
	0xeeeeeeee,0x00000000,0x99999999,
	0x00000000,0x00000000,0x00000000,
	0xffffffff,0xaaaaaaaa,0x88888888,
	0xffffffff,0xeeeeeeee,0x00000000,
	0x00000000,0x88888888,0x00000000,
	0x00000000,0xdddddddd,0x00000000,
	0x00000000,0xcccccccc,0xcccccccc,
	0x00000000,0x66666666,0xffffffff,
	0x00000000,0x00000000,0xaaaaaaaa,
};
#endif

#define MISC_WIDTH        23
#define MISC_HEIGHT        9
#define MISC_DEPTH         4
#define MISC_COMPRESSION   0
#define MISC_MASKING       2

#ifdef USE_MISC_HEADER
const struct BitMapHeader misc_header =
{ 23,9,0,0,4,2,0,0,0,44,44,320,256 };
#endif

#ifdef USE_MISC_BODY
const UBYTE misc_body[144] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x38,
0x38,0x00,0x00,0x38,0x00,0x00,0x38,0x38,0x00,0x00,0x38,0x00,0x38,0x00,0x44,
0x44,0x44,0x00,0x22,0x7e,0x22,0x00,0x5c,0x5c,0x00,0x00,0x7e,0x22,0x7e,0x00,
0x80,0x80,0x80,0x00,0x02,0x7e,0x02,0x00,0x7c,0x7c,0x00,0x00,0x7e,0x02,0x7e,
0x00,0x80,0x80,0x80,0x00,0x02,0x7e,0x02,0x00,0x7c,0x7c,0x00,0x00,0x7e,0x02,
0x7e,0x00,0x80,0x80,0x80,0x00,0x04,0x3c,0x04,0x00,0x38,0x38,0x00,0x00,0x3c,
0x04,0x3c,0x00,0x40,0x40,0x40,0x00,0x38,0x38,0x38,0x00,0x00,0x00,0x00,0x00,
0x38,0x38,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, };
#endif
