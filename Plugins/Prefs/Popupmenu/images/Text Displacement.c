#ifdef USE_TEXT DISPLACEMENT_COLORS
const ULONG Text Displacement_colors[48] =
{
	0x66666666,0x66666666,0x55555555,
	0xaaaaaaaa,0x99999999,0x99999999,
	0x77777777,0x77777777,0x66666666,
	0x00000000,0x66666666,0xffffffff,
	0x88888888,0x77777777,0x77777777,
	0x88888888,0x99999999,0xaaaaaaaa,
	0x99999999,0xaaaaaaaa,0xbbbbbbbb,
	0xffffffff,0xffffffff,0xffffffff,
	0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa,
	0x99999999,0xaaaaaaaa,0xcccccccc,
	0xaaaaaaaa,0xaaaaaaaa,0xcccccccc,
	0xbbbbbbbb,0xcccccccc,0xdddddddd,
	0xffffffff,0xaaaaaaaa,0x99999999,
	0xcccccccc,0xcccccccc,0xdddddddd,
	0x00000000,0x00000000,0x00000000,
	0xffffffff,0xffffffff,0xffffffff,
};
#endif

#define TEXT DISPLACEMENT_WIDTH       149
#define TEXT DISPLACEMENT_HEIGHT       40
#define TEXT DISPLACEMENT_DEPTH         4
#define TEXT DISPLACEMENT_COMPRESSION   1
#define TEXT DISPLACEMENT_MASKING       2

#ifdef USE_TEXT DISPLACEMENT_HEADER
const struct BitMapHeader Text Displacement_header =
{ 149,40,0,0,4,2,1,0,0,44,22,320,512 };
#endif

#ifdef USE_TEXT DISPLACEMENT_BODY
const UBYTE Text Displacement_body[1837] = {
0xed,0x00,0xed,0x00,0xed,0x00,0xed,0x00,0x01,0x00,0x20,0xf2,0x00,0x02,0x07,
0x00,0x00,0x01,0x00,0x20,0xf3,0x00,0x03,0x20,0x07,0x00,0x00,0x01,0x00,0x20,
0xf3,0x00,0x00,0x20,0xfe,0x00,0xf1,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,
0xf2,0x00,0x02,0x07,0x00,0x00,0x01,0x00,0x20,0xf3,0x00,0x03,0x20,0x07,0x00,
0x00,0x01,0x00,0x20,0xf3,0x00,0x00,0x20,0xfe,0x00,0xf1,0x00,0x00,0x20,0xfe,
0x00,0x01,0x00,0x20,0xf2,0x00,0x02,0x07,0x00,0x00,0x01,0x00,0x20,0xf3,0x00,
0x03,0x20,0x07,0x00,0x00,0x01,0x00,0x20,0xf3,0x00,0x00,0x20,0xfe,0x00,0xf1,
0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf2,0x00,0x02,0x07,0x00,0x00,0x01,
0x00,0x20,0xf4,0xff,0x04,0xf8,0x20,0x07,0x00,0x00,0x01,0x00,0x20,0xf4,0xff,
0x01,0xf8,0x20,0xfe,0x00,0xff,0x00,0xf4,0xff,0x01,0xf8,0x20,0xfe,0x00,0x01,
0x00,0x20,0xf4,0x00,0x04,0x08,0x00,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,
0x00,0x04,0x08,0x20,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,
0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,
0x20,0xf4,0x00,0x04,0x08,0x00,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,
0x04,0x08,0x20,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,
0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,
0xf4,0x00,0x04,0x08,0x00,0xff,0xe0,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x04,
0x08,0x20,0xff,0xe0,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,0xfe,
0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,
0x00,0x04,0x08,0x00,0x7f,0xe0,0x00,0x08,0x00,0x20,0x87,0xf0,0x00,0x00,0x32,
0x00,0x0c,0xfb,0x00,0x04,0x08,0x20,0x7f,0xe0,0x00,0x08,0x00,0x20,0x87,0xf0,
0x00,0x00,0x32,0x00,0x0c,0xfb,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x06,
0x87,0xf0,0x00,0x00,0x32,0x00,0x0c,0xfa,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,
0x20,0xf4,0x00,0x04,0x08,0x00,0x3f,0xe0,0x00,0x08,0x00,0x20,0x84,0x90,0x00,
0x00,0x10,0x00,0x04,0xfb,0x00,0x04,0x08,0x20,0x3f,0xe0,0x00,0x08,0x00,0x20,
0x84,0x90,0x00,0x00,0x10,0x00,0x04,0xfb,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,
0x00,0x06,0x84,0x90,0x00,0x00,0x10,0x00,0x04,0xfa,0x00,0x00,0x20,0xfe,0x00,
0x01,0x00,0x20,0xf4,0x00,0x04,0x08,0x00,0x1f,0xc0,0x00,0x08,0x00,0x20,0x80,
0x80,0x00,0x40,0x10,0x00,0x04,0xfd,0x00,0x06,0x02,0x00,0x08,0x20,0x1f,0xc0,
0x00,0x08,0x00,0x20,0x80,0x80,0x00,0x40,0x10,0x00,0x04,0xfd,0x00,0x03,0x02,
0x00,0x08,0x20,0xfe,0x00,0xff,0x00,0xff,0x80,0x04,0x00,0x40,0x10,0x00,0x04,
0xfd,0x00,0x03,0x02,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x04,
0x08,0x00,0x0f,0x80,0x00,0x13,0x00,0x20,0x80,0x83,0x1b,0xf0,0x76,0x3b,0xc4,
0x61,0x8c,0xb6,0x19,0x67,0x80,0x08,0x20,0x0f,0x80,0x00,0x10,0x00,0x20,0x80,
0x83,0x1b,0xf0,0x76,0x3b,0xc4,0x61,0x8c,0xb6,0x19,0x67,0x80,0x08,0x20,0xfe,
0x00,0xff,0x00,0x0e,0x80,0x83,0x1b,0xf0,0x76,0x3b,0xc4,0x61,0x8c,0xb6,0x19,
0x67,0x80,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x04,0x08,0x00,0x07,
0x00,0x00,0x13,0x00,0x20,0x80,0x84,0x8a,0x40,0x92,0x49,0x24,0x92,0x52,0x49,
0x24,0x92,0x00,0x08,0x20,0x07,0x00,0x00,0x10,0x00,0x20,0x80,0x84,0x8a,0x40,
0x92,0x49,0x24,0x92,0x52,0x49,0x24,0x92,0x00,0x08,0x20,0xfe,0x00,0xff,0x00,
0x0e,0x80,0x84,0x8a,0x40,0x92,0x49,0x24,0x92,0x52,0x49,0x24,0x92,0x00,0x00,
0x20,0xfe,0x00,0x13,0x00,0x20,0x1f,0x78,0x7b,0xbf,0x6d,0x9e,0xdb,0x8d,0xe1,
0xb6,0xc3,0x6d,0xff,0xff,0xdf,0xff,0xe0,0x00,0x02,0x00,0x20,0x9f,0xf2,0xff,
0x01,0xe0,0x00,0x10,0x00,0x20,0x80,0x87,0x84,0x40,0x92,0x61,0x24,0x72,0x1e,
0x49,0x3c,0x92,0x00,0x08,0x20,0xfe,0x00,0xff,0x00,0x0e,0x80,0x87,0x84,0x40,
0x92,0x61,0x24,0x72,0x1e,0x49,0x3c,0x92,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,
0x20,0xf4,0x00,0x00,0x08,0xfd,0x00,0x10,0x00,0x20,0x80,0x84,0x04,0x40,0x92,
0x19,0x24,0x92,0x10,0x49,0x20,0x92,0x00,0x08,0x20,0xfe,0x00,0x10,0x00,0x20,
0x80,0x84,0x04,0x40,0x92,0x19,0x24,0x92,0x10,0x49,0x20,0x92,0x00,0x08,0x20,
0xfe,0x00,0xff,0x00,0x0e,0x80,0x84,0x04,0x40,0x92,0x19,0x24,0x92,0x10,0x49,
0x20,0x92,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x00,0x08,0xfd,
0x00,0x10,0x00,0x20,0x80,0x86,0x4a,0x40,0x92,0x49,0x24,0x92,0x59,0x49,0x32,
0x92,0x00,0x08,0x20,0xfe,0x00,0x10,0x00,0x20,0x80,0x86,0x4a,0x40,0x92,0x49,
0x24,0x92,0x59,0x49,0x32,0x92,0x00,0x08,0x20,0xfe,0x00,0xff,0x00,0x0e,0x80,
0x86,0x4a,0x40,0x92,0x49,0x24,0x92,0x59,0x49,0x32,0x92,0x00,0x00,0x20,0xfe,
0x00,0x01,0x00,0x20,0xf4,0x00,0x00,0x08,0xfd,0x00,0x10,0x00,0x20,0x81,0xc3,
0x9b,0x30,0x6f,0x71,0xce,0x69,0x8e,0xed,0x9d,0xd9,0x80,0x08,0x20,0xfe,0x00,
0x10,0x00,0x20,0x81,0xc3,0x9b,0x30,0x6f,0x71,0xce,0x69,0x8e,0xed,0x9d,0xd9,
0x80,0x08,0x20,0xfe,0x00,0xff,0x00,0x0e,0x81,0xc3,0x9b,0x30,0x6f,0x71,0xce,
0x69,0x8e,0xed,0x9d,0xd9,0x80,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,
0x00,0x08,0xfd,0x00,0x02,0x00,0x20,0x80,0xfd,0x00,0x00,0x01,0xfa,0x00,0x01,
0x08,0x20,0xfe,0x00,0x02,0x00,0x20,0x80,0xfd,0x00,0x00,0x01,0xfa,0x00,0x01,
0x08,0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xfd,0x00,0x00,0x01,0xf9,0x00,0x00,
0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x00,0x08,0xfd,0x00,0x02,0x00,0x20,
0x80,0xfd,0x00,0x00,0x01,0xfa,0x00,0x01,0x08,0x20,0xfe,0x00,0x02,0x00,0x20,
0x80,0xfd,0x00,0x00,0x01,0xfa,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x00,
0x80,0xfd,0x00,0x00,0x01,0xf9,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,
0x00,0x00,0x08,0xfd,0x00,0x02,0x00,0x20,0x80,0xfd,0x00,0x01,0x03,0x80,0xfb,
0x00,0x01,0x08,0x20,0xfe,0x00,0x02,0x00,0x20,0x80,0xfd,0x00,0x01,0x03,0x80,
0xfb,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xfd,0x00,0x01,0x03,
0x80,0xfa,0x00,0x00,0x20,0xfe,0x00,0x02,0x00,0x20,0x1f,0xf4,0xff,0x03,0xdf,
0xff,0xe0,0x00,0x02,0x00,0x20,0x9f,0xf2,0xff,0x01,0xe0,0x00,0x02,0x00,0x20,
0x80,0xf5,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,
0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x04,0x08,0x00,0x07,0x00,0x00,0x02,
0x00,0x20,0x80,0xf5,0x00,0x04,0x08,0x20,0x07,0x00,0x00,0x02,0x00,0x20,0x80,
0xf5,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,
0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x04,0x08,0x00,0x0f,0x80,0x00,0x02,0x00,
0x20,0x80,0xf5,0x00,0x04,0x08,0x20,0x0f,0x80,0x00,0x02,0x00,0x20,0x80,0xf5,
0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,
0x00,0x01,0x00,0x20,0xf4,0x00,0x04,0x08,0x00,0x1f,0xc0,0x00,0x02,0x00,0x20,
0x80,0xf5,0x00,0x04,0x08,0x20,0x1f,0xc0,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,
0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,
0x01,0x00,0x20,0xf4,0x00,0x04,0x08,0x00,0x3f,0xe0,0x00,0x02,0x00,0x20,0x80,
0xf5,0x00,0x04,0x08,0x20,0x3f,0xe0,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,
0x08,0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,
0x00,0x20,0xf4,0x00,0x04,0x08,0x00,0x7f,0xe0,0x00,0x02,0x00,0x20,0x80,0xf5,
0x00,0x04,0x08,0x20,0x7f,0xe0,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,
0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,
0x20,0xf4,0x00,0x04,0x08,0x00,0xff,0xe0,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,
0x04,0x08,0x20,0xff,0xe0,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,
0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,
0xf4,0x00,0x04,0x08,0x00,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x04,
0x08,0x20,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,0xfe,
0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,
0x00,0x04,0x08,0x00,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x04,0x08,
0x20,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,0xfe,0x00,
0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,
0x04,0x08,0x00,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x04,0x08,0x20,
0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,
0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x04,
0x08,0x00,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x04,0x08,0x20,0x07,
0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,
0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x04,0x08,
0x00,0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x04,0x08,0x20,0x07,0x00,
0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x00,
0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x04,0x08,0x00,
0x07,0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x04,0x08,0x20,0x07,0x00,0x00,
0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x00,0x80,
0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x04,0x08,0x00,0x07,
0x00,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x04,0x08,0x20,0x07,0x00,0x00,0x02,
0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,
0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf4,0x00,0x00,0x08,0xfd,0x00,0x02,
0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,0xfe,0x00,0x02,0x00,0x20,0x80,0xf5,
0x00,0x01,0x08,0x20,0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,
0x00,0x01,0x00,0x20,0xf4,0x00,0x00,0x08,0xfd,0x00,0x02,0x00,0x20,0x80,0xf5,
0x00,0x01,0x08,0x20,0xfe,0x00,0x02,0x00,0x20,0x80,0xf5,0x00,0x01,0x08,0x20,
0xfe,0x00,0xff,0x00,0x00,0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x02,0x00,0x20,
0x7f,0xf5,0xff,0x00,0xf8,0xfd,0x00,0x01,0x00,0x20,0xf4,0xff,0x01,0xf8,0x20,
0xfe,0x00,0x01,0x00,0x20,0xf4,0xff,0x01,0xf8,0x20,0xfe,0x00,0xff,0x00,0x00,
0x80,0xf4,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xef,0x00,0x01,0x00,0x20,
0xf3,0x00,0x00,0x20,0xfe,0x00,0x01,0x00,0x20,0xf3,0x00,0x00,0x20,0xfe,0x00,
0xf1,0x00,0x00,0x20,0xfe,0x00,0xed,0x00,0xed,0x00,0xed,0x00,0xed,0x00,0xed,
0x00,0xed,0x00,0xed,0x00,0xed,0x00, };
#endif