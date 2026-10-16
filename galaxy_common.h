
#define DRUM_BUFF 1
#define BUFFER_SIZE 10
#define MAX_SEQUENCE_SIZE 48



//extra note patterns
#define NUM_PATTERNS 24 //number of patterns, to select one during (re)-generation

#define T uint16_t   //size of pattern - (1 bit per beat of pattern)
#define NOTE_COUNTER 16     //for 8 bits uint_8
#define REST_COUNTER 16   //maybe for rests?

//buffer types



//patterns (0 for off 1 for on) for rests (0 don't send rest, 1 send rest) and extra notes

#define p0 (T)(0b00000000 << 8) + 0b01000010
#define p1 (T)(0b00001000 << 8) + 0b00001001
#define p2 (T)(0b00010100 << 8) + 0b00000110
#define p3 (T)(0b10001000 << 8) + 0b00001010

#define p4 (T)(0b00000000 << 8) + 0b01000010
#define p5 (T)(0b00001000 << 8) + 0b01001001
#define p6 (T)(0b10110000 << 8) + 0b00110000
#define p7 (T)(0b00000101 << 8) + 0b01000101

#define p8  (T)(0b00000000 << 8) + 0b00001100
#define p9  (T)(0b00000100 << 8) + 0b10100100
#define p10 (T)(0b00010001 << 8) + 0b00010001
#define p11 (T)(0b01011000 << 8) + 0b01011000

#define p12 (T)(0b00110100 << 8) + 0b00110100
#define p13 (T)(0b01101000 << 8) + 0b01100100
#define p14 (T)(0b01100000 << 8) + 0b00000011
#define p15 (T)(0b00110000 << 8) + 0b00110110

#define p16 (T)(0b00000000 << 8) + 0b00000101
#define p17 (T)(0b00001000 << 8) + 0b00011001
#define p18 (T)(0b00000100 << 8) + 0b01001101
#define p19 (T)(0b00000001 << 8) + 0b00010110

#define p20 (T)(0b00100001 << 8) + 0b00010010
#define p21 (T)(0b00000100 << 8) + 0b01011001
#define p22 (T)(0b01000101 << 8) + 0b11010010
#define p23 (T)(0b00001100 << 8) + 0b00011001


//baseline patterns for drums
#define NUM_DRUM_PATTERNS 12
#define MAX_PER_BUFF 4
#define DRUM_REGEN 6
#define DRUM_OTHER_REGEN 1

#define NUM_DRUM_OTHER 4

#define kick0 (T)(0b10000000 << 8) + 0b10000000
#define kick1 (T)(0b10000000 << 8) + 0b10100000
#define kick2 (T)(0b10100000 << 8) + 0b10110000
#define kick3 (T)(0b11000000 << 8) + 0b10100000
#define kick4 (T)(0b10100001 << 8) + 0b10100000
#define kick5 (T)(0b10100000 << 8) + 0b10110001
#define kick6 (T)(0b11000000 << 8) + 0b10101000
#define kick7 (T)(0b11010000 << 8) + 0b10110000
#define kick8 (T)(0b10100001 << 8) + 0b11000001
#define kick9 (T)(0b10011000 << 8) + 0b10011001
#define kick10 (T)(0b10110001 << 8) + 0b10011010

#define sn0 (T)(0b00001000 << 8) + 0b00001000
#define sn1 (T)(0b00001000 << 8) + 0b00001100
#define sn2 (T)(0b00001000 << 8) + 0b00001010
#define sn3 (T)(0b00101000 << 8) + 0b00011000
#define sn4 (T)(0b00101000 << 8) + 0b00001000
#define sn5 (T)(0b00011000 << 8) + 0b00001000
#define sn6 (T)(0b00001100 << 8) + 0b00011000

#define hh0 (T)(0b01010011 << 8) + 0b01011101
#define hh1 (T)(0b11001010 << 8) + 0b01101010
#define hh2 (T)(0b10111010 << 8) + 0b10101110
#define hh3 (T)(0b10101011 << 8) + 0b01011101
#define hh4 (T)(0b11111101 << 8) + 0b11111011



//volca drum patterns

#define VD0 60
#define VD1 62
#define VD2 64
#define VD3 65
#define VD4 67
#define VD5 69

