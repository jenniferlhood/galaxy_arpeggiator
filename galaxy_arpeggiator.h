//CUSTOMIZATION SETTINGS

#define SERIALVIS //to use with my python serial visualizer. It logs specific messages to serial out.
//#define SNIFF //To intercept and log any midi messages observed by the arduino on the 5-pin midi input

//----- Device specific midi settings

//#define MIDICLOCKCOMMAND  //define to send a midi cc command to explicitly set the tempo, instead of using midi clock pulse + calculation (implemented by the galaxy_sequencer)
//#define MIDICLOCKCHAN   1   //channel for the midi cc tempo command
//#define MIDICLOCKNUMBER 94 //cc number for the tempo command

#define SEED 10 //change it occasionally for more variety

//Device specific options for parameter mutation via midi cc
#define MIDICC //send midicc on channel 1 - this is for effect pedals that accept midi CC like the OBNE Darkstar v3
#define CC_FREQ QUARTERNOTE
//#define DARKSTAR

#define VOLCAFM
#define VOLCAFMCHANNEL 3  //channel this synth is plugged into
#define FM_TRANSPOSE  40
#define FM_MOD_ATTACK 42
#define FM_MOD_DECAY  43
#define FM_CA_ATTACK  44
#define FM_CA_DECAY   45
#define FM_LFO_RATE   46
#define FM_LFO_DEPTH  47

#define NTS-1
#define NTS1CHANNEL   4
#define NTS1ATTACK    16
#define NTS1RELEASE   19
#define NTS1TREMRATE  21
#define NTS1TREMDEPTH 20
#define NTS1LFORATE   24
#define NTS1LFODEPTH  26
#define NTS1RESONANCE 44
#define NTS1OSCSHAPE  54
#define NTS1OSCALT    55


#define MINILOGUE
#define MINI_VCO1_SHAPE 36
#define MINI_VCO2_SHAPE 37
#define MINI_CROSSMOD_DEPTH 41
#define MINI_PITCH_EG_INT 42
#define MINI_FILTER 43
#define MINI_RES 44
#define MINI_FILTER_EG_INT 45
#define MINI_AMP_ATT 16
#define MINI_AMP_DEC 17
#define MINI_AMP_SUS 18
#define MINI_AMP_REL 19
#define MINI_EG_ATT 20
#define MINI_EG_DEC 21
#define MINI_EG_SUS 22
#define MINI_EG_REL 23
#define MINI_LFO_RATE 24
#define MINI_LFO_DEPTH 26
#define MINI_LFO_VOICE_DEPTH 27


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
#define SWITCH_0L_RESET   50  // play or reset (middle is stop)
#define SWITCH_0R_PLAY    52
#define SWITCH_4_OCT0     35  // no octave - switch left position output channel 4
#define SWITCH_4_OCT2     37  // 2 octaves - swich right position output channel 4 (one octave is middle position, switch open)
#define SWITCH_4_CLOCK    34  // send clock signal - switch left position output channel 4 
#define SWITCH_4_REGEN    36  // arpeggiator alternative algorithm - switch right position output channel 4 (standard is middle position, switch open)
#define SWITCH_3_OCT0     39 
#define SWITCH_3_OCT2     41
#define SWITCH_3_CLOCK    38 
#define SWITCH_3_REGEN    40
#define SWITCH_2_OCT0     43 
#define SWITCH_2_OCT2     45
#define SWITCH_2_CLOCK    42 
#define SWITCH_2_REGEN    44
#define SWITCH_1_OCT0     47 
#define SWITCH_1_OCT2     49
#define SWITCH_1_CLOCK    46 
#define SWITCH_1_REGEN    48
#define SWITCH_MODE_1     51
#define SWITCH_MODE_3     53

//defines for the analog inputs. used for potentiometers
#define ALG_POT_1   A8    // arp algorithm select for channel output 1
#define DIV_POT_1   A9    // time division for channe output 1
#define ALG_POT_2   A10   // arp algorithm select for channel output 2
#define DIV_POT_2   A11   // time division for channe output 2
#define ALG_POT_3   A12   // arp algorithm select for channel output 3
#define DIV_POT_3   A13   // time division for channe output 3
#define ALG_POT_4   A14   // arp algorithm select for channel output 4
#define DIV_POT_4   A15   // time division for channe output 4
#define TEMPO_POT   A7    // tempo control

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
#define PIN_LED_TEMPO     30
#define PIN_LED_PLAYING   6

//Generative parameters
#define OCTAVECHANGE 53 //note that determines if octaves added higher or lower (midi note ranges from 21 to 127, but notes higher than 115 often sound kinda bad)

#define REGEN_FACTOR 2 //how many arps before a regen/shuffle
//the reading from the algorithm pot that determines if "more randomness" is needed
// a design choice of the regen box is that when the knobs are turned up, more randomness is introduced

// This is the setting at which shuffling is applied 
#define GENERATIVE_SETTING 5

//not to be changed - based on physical build! My unit has only 4 output channels!!
#define MAX_CHANNELS 4

//change if you need more rhythmic time divisions or generative algorithms. These determine the maximum number read from the potentiometers
#define MAX_DIVS  10
#define MAX_ALG   10

#define MAX_REPEATS 4  //for constructing MODE2 algortithms



//TODO: testing which kinds of divisions are more useful musically

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

#define WHOLENOTE           (48 *2)
#define D_HALF              (32 *2)
#define HALFNOTE            (24 *2)
#define D_QUARTER           (18 *2) 
#define QUARTERNOTE         (12 *2) 
#define Q_TRIPLETS          (8  *2)
#define EIGHTHNOTE          (6  *2)
#define E_TRIPLETS          (4  *2)
#define SIXTEENTHNOTE       (3  *2)
#define SIXTEENTHNOTE_TRIP  (1  *2) //instead lets have dotted eightsths


#define DIVS_PER_BAR  WHOLENOTE //48 divisions per bar, 48 is a whole note, 24 is half, 12 is quarter etct





//a seeminly useful tempo range
#define MINTEMPO 20
#define MAXTEMPO 300 //I can't suport more than 510 - as a limitation of midiCC which is used to send a tempo value

//global modes
#define NORMAL  0
#define CHORD   1
#define REPEAT  2

//Per channel arpeggiator modes
#define CLOCK   0
#define ARP     1
#define FREEZE  2

//octave settings
#define DJENT   2