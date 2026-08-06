
#define BUFFER_SIZE 16
#define MAX_SEQUENCE_SIZE 96

//extra note patterns
#define NUM_PATTERNS 20 //number of patterns, to select one during (re)-generation

#define T uint8_t   //size of pattern - (1 bit per beat of pattern)
#define NOTE_COUNTER 8     //for 8 bits uint_8
#define REST_COUNTER 8   //maybe for rests?

//patterns (0 for off 1 for on)



#define p0 (T)0b11000000 //80
#define p1 (T)0b10010000 //80
#define p2 (T)0b00010100 // 0x82
#define p3 (T)0b10001000
#define p4 (T)0b01000010 
#define p5 (T)0b01001000 

#define p6  (T)0b10110000 
#define p7  (T)0b11000100
#define p8  (T)0b10011000 
#define p9  (T)0b10100100 
#define p10 (T)0b10010001
#define p11 (T)0b01011000
#define p12 (T)0b00110100 
#define p13 (T)0b01010010 // 0xa0

#define p14 (T)0b11000011
#define p15 (T)0b01010011
#define p16 (T)0b11001100
#define p17 (T)0b10011010
#define p18 (T)0b11010100
#define p19 (T)0b00111100
##define p19 (T)0b01110100