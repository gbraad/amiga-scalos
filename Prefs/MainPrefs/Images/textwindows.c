#ifdef USE_TEXTWINDOWS_COLORS
const ULONG textwindows_colors[96] =
{
	0xffffffff,0xeeeeeeee,0x00000000,
	0xa0a0a0a0,0xa0a0a0a0,0xa0a0a0a0,
	0x33333333,0x77777777,0xaaaaaaaa,
	0xffffffff,0x00000000,0x00000000,
	0x99999999,0x99999999,0x99999999,
	0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa,
	0xffffffff,0xaaaaaaaa,0x99999999,
	0xffffffff,0xffffffff,0xffffffff,
	0x77777777,0x77777777,0x77777777,
	0xeeeeeeee,0x00000000,0x99999999,
	0x00000000,0x00000000,0x00000000,
	0x00000000,0x88888888,0x00000000,
	0x00000000,0xdddddddd,0x00000000,
	0x00000000,0xcccccccc,0xcccccccc,
	0x00000000,0x66666666,0xffffffff,
	0x00000000,0x00000000,0xaaaaaaaa,
	0x66666666,0x22222222,0x00000000,
	0xeeeeeeee,0x55555555,0x22222222,
	0xaaaaaaaa,0x55555555,0x22222222,
	0xffffffff,0xcccccccc,0xaaaaaaaa,
	0x33333333,0x33333333,0x33333333,
	0x44444444,0x44444444,0x44444444,
	0x55555555,0x55555555,0x55555555,
	0x66666666,0x66666666,0x66666666,
	0x77777777,0x77777777,0x77777777,
	0x88888888,0x88888888,0x88888888,
	0x99999999,0x99999999,0x99999999,
	0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa,
	0xcccccccc,0xcccccccc,0xcccccccc,
	0xdddddddd,0xdddddddd,0xdddddddd,
	0xeeeeeeee,0xeeeeeeee,0xeeeeeeee,
	0xffffffff,0xffffffff,0xffffffff,
};
#endif

#define TEXTWINDOWS_WIDTH        23
#define TEXTWINDOWS_HEIGHT       14
#define TEXTWINDOWS_DEPTH         5
#define TEXTWINDOWS_COMPRESSION   0
#define TEXTWINDOWS_MASKING       2

#ifdef USE_TEXTWINDOWS_HEADER
const struct BitMapHeader textwindows_header =
{ 23,14,0,0,5,2,0,0,0,44,22,320,512 };
#endif

#ifdef USE_TEXTWINDOWS_BODY
const UBYTE textwindows_body[280] = {
0xff,0xff,0xf8,0x00,0xff,0xff,0xf8,0x00,0xff,0xff,0xf8,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x90,0x00,0x90,0x00,0xff,0xff,0xfc,0x00,0x90,0x00,
0x90,0x00,0x00,0x01,0x24,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x7f,
0xff,0xfc,0x00,0x80,0x00,0x00,0x00,0x7f,0xff,0xfe,0x00,0x00,0x00,0x00,0x00,
0xbf,0xff,0xf8,0x00,0xc0,0x00,0x0c,0x00,0xbf,0xff,0xf8,0x00,0x40,0x00,0x06,
0x00,0x00,0x00,0x00,0x00,0xa1,0x04,0x18,0x00,0xde,0xfb,0xec,0x00,0xa1,0x04,
0x18,0x00,0x40,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0xbf,0xff,0xf8,0x00,0xc0,
0x00,0x0c,0x00,0xbf,0xff,0xf8,0x00,0x40,0x00,0x06,0x00,0x00,0x00,0x00,0x00,
0xa7,0x0c,0x78,0x00,0xd8,0xf3,0x8c,0x00,0xa7,0x0c,0x78,0x00,0x40,0x00,0x06,
0x00,0x00,0x00,0x00,0x00,0xbf,0xff,0xf8,0x00,0xc0,0x00,0x0c,0x00,0xbf,0xff,
0xf8,0x00,0x40,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0xa3,0x1c,0x38,0x00,0xdc,
0xe3,0xcc,0x00,0xa3,0x1c,0x38,0x00,0x40,0x00,0x06,0x00,0x00,0x00,0x00,0x00,
0xbf,0xff,0xf8,0x00,0xc0,0x00,0x0c,0x00,0xbf,0xff,0xf8,0x00,0x40,0x00,0x06,
0x00,0x00,0x00,0x00,0x00,0xbf,0xff,0xf8,0x00,0xff,0xff,0xfc,0x00,0xbf,0xff,
0xf8,0x00,0x40,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x10,0x00,0xff,
0xff,0xfc,0x00,0x80,0x00,0x10,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,
0x80,0x00,0x00,0x00,0xff,0xff,0xfc,0x00,0x80,0x00,0x00,0x00,0x7f,0xff,0xfe,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x7f,0xff,0xfe,0x00,0x00,0x00,0x00,0x00, };
#endif
