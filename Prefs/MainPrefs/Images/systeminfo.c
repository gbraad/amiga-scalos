#ifdef USE_SYSTEMINFO_COLORS
const ULONG systeminfo_colors[48] =
{
	0x00000000,0xeeeeeeee,0x00000000,
	0x99999999,0x99999999,0x99999999,
	0x44444444,0x44444444,0x44444444,
	0x66666666,0x66666666,0x66666666,
	0x00000000,0x66666666,0x99999999,
	0x33333333,0x77777777,0xaaaaaaaa,
	0x33333333,0x88888888,0x33333333,
	0x33333333,0x33333333,0x33333333,
	0x66666666,0x88888888,0xbbbbbbbb,
	0x88888888,0x66666666,0x44444444,
	0xaaaaaaaa,0x77777777,0x66666666,
	0x88888888,0x88888888,0x88888888,
	0xefefefef,0xe7e7e7e7,0x14141414,
	0xbbbbbbbb,0xbbbbbbbb,0xbbbbbbbb,
	0xcccccccc,0xcccccccc,0xcccccccc,
	0xffffffff,0xffffffff,0xffffffff,
};
#endif

#define SYSTEMINFO_WIDTH        37
#define SYSTEMINFO_HEIGHT       30
#define SYSTEMINFO_DEPTH         4
#define SYSTEMINFO_COMPRESSION   0
#define SYSTEMINFO_MASKING       2

#ifdef USE_SYSTEMINFO_HEADER
const struct BitMapHeader systeminfo_header =
{ 37,30,0,0,4,2,0,0,0,44,44,320,256 };
#endif

#ifdef USE_SYSTEMINFO_BODY
const UBYTE systeminfo_body[720] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x37,0xff,0xff,0x60,0x00,0x01,0xcf,0xff,0xff,0xe0,0x00,
0x01,0xff,0xff,0xff,0xa0,0x00,0x01,0xff,0xff,0xff,0xc0,0x00,0x02,0xd7,0xdd,
0x50,0x60,0x00,0x07,0xef,0xff,0xff,0xe0,0x00,0x07,0xff,0xff,0xff,0xa0,0x00,
0x07,0xff,0xff,0xff,0x80,0x00,0x04,0x1d,0xff,0xf0,0x20,0x00,0x0f,0xef,0xff,
0xff,0xe0,0x00,0x0f,0xfd,0xff,0xf0,0xa0,0x00,0x0f,0xfc,0x00,0x00,0x80,0x00,
0x08,0x1f,0x39,0x01,0x20,0x00,0x1f,0xee,0x00,0x5f,0xe0,0x00,0x1f,0xff,0xff,
0xa1,0xa0,0x00,0x1f,0xfc,0x00,0x01,0x80,0x00,0x10,0x1e,0xfe,0x01,0x60,0x00,
0x3f,0xee,0x00,0x1f,0xe0,0x00,0x3f,0xff,0xff,0xe1,0xe0,0x00,0x3f,0xfc,0x00,
0x01,0x80,0x00,0x23,0xf2,0xd6,0x81,0x60,0x00,0x3f,0xfe,0x00,0x2f,0xe0,0x00,
0x20,0x0f,0xd7,0xd1,0xe0,0x00,0x23,0xfc,0x28,0x01,0x80,0x00,0x2b,0x65,0xab,
0x01,0x60,0x00,0x34,0xfe,0x00,0x1f,0xe0,0x00,0x3f,0x01,0xab,0xe1,0xe0,0x00,
0x3f,0x40,0x54,0x01,0x80,0x00,0x27,0x00,0xc7,0x81,0x60,0x00,0x3f,0xff,0x00,
0x0f,0xe0,0x00,0x20,0x00,0xc7,0xf1,0xe0,0x00,0x24,0x00,0x38,0x01,0x80,0x00,
0x38,0x6a,0x2b,0x01,0x60,0x00,0x3f,0xf7,0x80,0x1f,0xe0,0x00,0x3e,0x08,0x2b,
0xe1,0xe0,0x00,0x3e,0x28,0x54,0x01,0x80,0x00,0x24,0x10,0x16,0x81,0x60,0x00,
0x3b,0xef,0xc0,0x2f,0xe0,0x00,0x3c,0x18,0x17,0xd1,0xe0,0x00,0x3c,0x18,0x28,
0x01,0x80,0x00,0x2d,0x3c,0xde,0x01,0x60,0x00,0x37,0xdb,0xc0,0x1f,0xe0,0x00,
0x38,0x3c,0x7f,0xe1,0xe0,0x00,0x3c,0x3c,0x00,0x01,0x80,0x00,0x25,0x54,0xb9,
0x01,0x60,0x00,0x3f,0xba,0xe0,0x5f,0xe0,0x00,0x38,0x5d,0x1f,0xa1,0xe0,0x00,
0x39,0x5c,0x00,0x01,0x80,0x00,0x29,0xb4,0x14,0x01,0x60,0x00,0x37,0x4e,0x60,
0x3f,0xe0,0x00,0x38,0xbd,0x9f,0xc1,0xe0,0x00,0x39,0xbc,0x00,0x01,0x80,0x00,
0x20,0x54,0x80,0x01,0x60,0x00,0x3f,0xab,0x61,0x7f,0xe0,0x00,0x38,0x5c,0x9e,
0x81,0xe0,0x00,0x38,0x5c,0x00,0x01,0x80,0x00,0x2d,0x15,0xa0,0x01,0x60,0x00,
0x37,0xeb,0xe4,0xff,0xe0,0x00,0x38,0x1c,0x1b,0x01,0xe0,0x00,0x39,0x1c,0x00,
0x01,0x80,0x00,0x25,0x10,0x9f,0xff,0x60,0x00,0x3f,0xef,0xff,0xff,0xe0,0x00,
0x38,0x1c,0x1f,0xff,0xe0,0x00,0x3c,0x1c,0x1f,0xfe,0x80,0x00,0x28,0x9c,0x00,
0x08,0xe0,0x00,0x37,0xe3,0xff,0xff,0x60,0x00,0x3c,0x1c,0x2f,0x2f,0xc0,0x00,
0x3c,0x1c,0x0f,0x27,0xa0,0x00,0x24,0x7e,0x60,0xd9,0xe0,0x00,0x3b,0xff,0xff,
0xfe,0xe0,0x00,0x3c,0x10,0x4e,0xdf,0x80,0x00,0x3c,0x2c,0x4e,0xdf,0x20,0x00,
0x2a,0x00,0x5d,0x2d,0xe0,0x00,0x35,0xff,0xfa,0xdf,0xe0,0x00,0x3e,0x00,0x85,
0x21,0x00,0x00,0x3e,0x00,0xad,0x28,0x60,0x00,0x20,0x66,0x28,0x01,0xe0,0x00,
0x3f,0xff,0xdf,0xff,0xe0,0x00,0x36,0x00,0x00,0x00,0x20,0x00,0x36,0x42,0x30,
0x00,0x40,0x00,0x18,0x39,0x17,0xf3,0xc0,0x00,0x3f,0xff,0xef,0xff,0xc0,0x00,
0x00,0x38,0x01,0x00,0x40,0x00,0x10,0x00,0x1a,0xe0,0x80,0x00,0x2f,0xa1,0x8b,
0xe7,0x80,0x00,0x3f,0xff,0xf7,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x80,0x00,
0x28,0x00,0x0d,0x81,0x00,0x00,0x17,0xff,0xe5,0xff,0x00,0x00,0x1f,0xff,0xfb,
0xff,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x16,0x80,0x86,0x06,0x00,0x00,
0x0b,0xff,0x7a,0xfc,0x00,0x00,0x0f,0xff,0xfd,0xfc,0x00,0x00,0x00,0x00,0x08,
0x04,0x00,0x00,0x0b,0xff,0x63,0x78,0x00,0x00,0x00,0x00,0x1d,0x00,0x00,0x00,
0x00,0x00,0x1e,0x80,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x19,
0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 };
#endif