
#define BUFFER_SIZE 16
#define MAX_SEQUENCE_SIZE 96

//extra note patterns
#define NUM_PATTERNS 20 //number of patterns, to select one during (re)-generation

#define T uint8_t   //size of pattern - (1 bit per beat of pattern)
#define NOTE_COUNTER 8     //for 8 bits uint_8
#define REST_COUNTER 8   //maybe for rests?

//patterns (0 for off 1 for on)

#define p0 (T)0b10000000 //80

#define p1 (T)0b11000000 //80
#define p2 (T)0b00010100 // 0x82
#define p3 (T)0b10001000
#define p4 (T)0b01000001 
#define p5 (T)0b01000010 
#define p5 (T)0b01001000 
#define p7 (T)0b00011000 

#define p8  (T)0b10110000 
#define p9  (T)0b11001000
#define p10 (T)0b10011000 
#define p11 (T)0b10100100
#define p12 (T)0b01110000
#define p13 (T)0b00111000 
#define p14 (T)0b11011000 // 0xa0
#define p15 (T)0b11000011
#define p16 (T)0b01010011
#define p17 (T)0b11001100
#define p18 (T)0b11011000
#define p19 (T)0b00111100