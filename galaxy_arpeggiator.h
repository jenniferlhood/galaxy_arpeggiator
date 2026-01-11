
//CUSTOMIZATION SETTINGS

#define SERIALVIS //to use with my python serial visualizer. It logs specific messages to serial out.
//#define SNIFF //To intercept and log any midi messages observed by the arduino on the 5-pin midi input



//----- Device specific midi settings
//#define MIDICC //send midicc on channel 1 - this is for effect pedals that accept midi CC like the OBNE Darkstar v3
#define MIDICLOCKCOMMAND  //define to send a midi cc command to explicitly set the tempo, instead of using midi clock pulse + calculation (implemented by the galaxy_sequencer)
#define MIDICLOCKCHAN 1   //channel for the midi cc tempo command
#define MIDICLOCKNUMBER 94 //cc number for the tempo command

#define AFTERTOUCH // - Do things with aftertouch messages
#define AFTERTOUCHVEL // - Aftertouch modifies velocity

#define AFTERTOUCHCHANNEL 1 //channel for aftertouch - functionality might depend on gear used?




//#define CC_SPEED 3 //cannot be less than 1 or more than 10 



//#define CAPTUREDEBUG
//#define PLAYDEBUG
//#define NOTEDEBUG
//#define RESETDEBUG
//#define SENDDEBUG
//#define GENDEBUG
//#define DRUMDEBUG
//#define SEQDEBUG


//change these if a midi device uses a fixed channel
#define CHAN1 1
#define CHAN2 2
#define CHAN3 3 //polyphonic chanel
#define CHAN4 4 //polyphonic channel


//Defines for the Digital analog input pins for three-way SPDT switches. The pin # mapping depends on how Arduino unit is wired up to the switches and pots
#define SWITCH_0L_RESET 50  // play or reset (middle is stop)
#define SWITCH_0R_PLAY 52
#define SWITCH_4_OCT0 35    // no octave - switch left position output channel 4
#define SWITCH_4_OCT2 37    // 2 octaves - swich right position output channel 4 (one octave is middle position, switch open)
#define SWITCH_4_CLOCK 34   // send clock signal - switch left position output channel 4 
#define SWITCH_4_REGEN 36   // arpeggiator alternative algorithm - switch right position output channel 4 (standard is middle position, switch open)
#define SWITCH_3_OCT0 39 
#define SWITCH_3_OCT2 41
#define SWITCH_3_CLOCK 38 
#define SWITCH_3_REGEN 40
#define SWITCH_2_OCT0 43 
#define SWITCH_2_OCT2 45
#define SWITCH_2_CLOCK 42 
#define SWITCH_2_REGEN 44
#define SWITCH_1_OCT0 47 
#define SWITCH_1_OCT2 49
#define SWITCH_1_CLOCK 46 
#define SWITCH_1_REGEN 48
#define SWITCH_MODE_1 51
#define SWITCH_MODE_3 53


//defines for the analog inputs. used for potentiometers
#define ALG_POT_1 A8      // arp algorithm select for channel output 1
#define DIV_POT_1 A9      // time division for channe output 1
#define ALG_POT_2 A10     // arp algorithm select for channel output 2
#define DIV_POT_2 A11     // time division for channe output 2
#define ALG_POT_3 A12     // arp algorithm select for channel output 3
#define DIV_POT_3 A13     // time division for channe output 3
#define ALG_POT_4 A14     // arp algorithm select for channel output 4
#define DIV_POT_4 A15     // time division for channe output 4
#define TEMPO_POT A7      // tempo control

//onboard button/pot
#define PIN_RAW_INPUT 3


//LED Pins
#define LED_ARP_1 28
#define LED_DIV_1 31
#define LED_ARP_2 26
#define LED_DIV_2 29
#define LED_ARP_3 24
#define LED_DIV_3 27
#define LED_ARP_4 22
#define LED_DIV_4 25
#define PIN_LED_TEMPO 30
#define PIN_LED_PLAYING 6

//Generative parameters

#define OCTAVECHANGE 53 //note that determines if octaves added higher or lower (midi note ranges from 21 to 127, but notes higher than 115 often sound kinda bad)
#define POLYPHONY 4 //max notes to play simultaneously for polyphonic synthesizers

#define REGEN_FACTOR 2 //how many arps before a regen/shuffle
//the reading from the algorithm pot that determines if "more randomness" is needed
// a design choice of the regen box is that when the knobs are turned up, more randomness is introduced

// This is the setting at which shuffling is applied 
#define GENERATIVE_SETTING 6

//sequence defines
#define MAX_SEQUENCE_SIZE 64

//not to be changed - based on physical build! My unit has only 4 output channels!!
#define MAX_CHANNELS 4

//change if you need more rhythmic time divisions or generative algorithms. These determine the maximum number read from the potentiometers
#define MAX_DIVS 10
#define MAX_ALG 12 

#define DIVS_PER_BAR (48*2) //48 divisions per bar, 48 is a whole note, 24 is half, 12 is quarter etct
#define MAX_TICKS2  96// max notes per buffer


//TODO: testing which kinds of divisions are more useful musically
#define WHOLENOTE (48 * 2)
#define D_HALF (32 * 2)
#define HALFNOTE (24* 2)
#define D_QUARTER (18*2) 
#define QUARTERNOTE (12*2) 
#define Q_TRIPLETS (8*2)
#define EIGHTHNOTE (6*2)
#define E_TRIPLETS (4*2)
#define SIXTEENTHNOTE (3*2)
#define SIXTEENTHNOTE_TRIP (1*2) //instead lets have dotted eightsths

/*
#define DIVS_PER_BAR 48
#define WHOLENOTE 48
#define D_HALF 32
#define HALFNOTE 24
#define D_QUARTER 18 //TOD is this more interesting than 2 16?
#define QUARTERNOTE 12 //TODO these may be eight notes...TODO
#define Q_TRIPLETS 8
#define EIGHTHNOTE 6
#define E_TRIPLETS 4
#define SIXTEENTHNOTE 3
#define SIXTEENTHNOTE_TRIP 1 //instead lets have dotted eightsths
*/


//a seeminly useful tempo range
#define MINTEMPO 20
#define MAXTEMPO 300 //I can't suport more than 510 - as a limitation of midiCC which is used to send a tempo value

#define CLOCK 0
#define ARP 1
#define REGEN 2
