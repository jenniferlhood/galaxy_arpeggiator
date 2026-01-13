#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <MIDI.h>
#include "galaxy_arpeggiator.h"
#include "NoteBuffer.h"

MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI);

//for sending midi clock 
bool send_start = false;
bool send_stop = false;
bool send_tick = false; //master clock tick set by timer
bool replace_switch = false; //switch to signal a replacment note at next tick
bool sequence_event = false;
float tempo_delay;
short tempo; //we don't need more than 255?

//for not repeating work when a switch is engaged
bool play_switch = false;
bool reset_switch = false;

bool first_note = true;

byte mode_setting = 0; //0, 1, 2 

bool note_off_switch[MAX_CHANNELS] = {0};
//Generative settings obtained from arduino inputs
byte genType[MAX_CHANNELS] = {0};
bool gen_switch = false;
byte regen[MAX_CHANNELS] = {0}; //saved from the first set of switches. 1: regular sequence, 0: clock 2: regenerate
bool regen_switch[MAX_CHANNELS] = {0}; //a switch to signal that the squence should regenerate at the next tick
byte regen_bars[MAX_CHANNELS] = {0}; //for tracking when to regenerate a regenerative sequence

byte div_setting[MAX_CHANNELS] = {0}; 
byte octave[MAX_CHANNELS] = {0}; //saved from the second set of switches

//convenience arrays for checking the state of switches, or to lookup numeric values
byte div_reference[MAX_DIVS] = {WHOLENOTE, D_HALF, HALFNOTE, D_QUARTER, QUARTERNOTE, Q_TRIPLETS, EIGHTHNOTE, E_TRIPLETS, SIXTEENTHNOTE,SIXTEENTHNOTE_TRIP};
byte gen_pots[MAX_CHANNELS] = {ALG_POT_1, ALG_POT_2, ALG_POT_3, ALG_POT_4};
byte div_pots[MAX_CHANNELS] = {DIV_POT_1, DIV_POT_2, DIV_POT_3, DIV_POT_4};
byte regen_switches[MAX_CHANNELS] = {SWITCH_1_REGEN,SWITCH_2_REGEN,SWITCH_3_REGEN,SWITCH_4_REGEN};
byte clock_switches[MAX_CHANNELS] = {SWITCH_1_CLOCK,SWITCH_2_CLOCK,SWITCH_3_CLOCK,SWITCH_4_CLOCK};
byte oct0_switches[MAX_CHANNELS] = {SWITCH_1_OCT0,SWITCH_2_OCT0,SWITCH_3_OCT0,SWITCH_4_OCT0};
byte oct2_switches[MAX_CHANNELS] = {SWITCH_1_OCT2,SWITCH_2_OCT2,SWITCH_3_OCT2,SWITCH_4_OCT2};
byte div_leds[MAX_CHANNELS] = {LED_DIV_1, LED_DIV_2, LED_DIV_3, LED_DIV_4};
byte arp_leds[MAX_CHANNELS] = {LED_ARP_1, LED_ARP_2, LED_ARP_3, LED_ARP_4};

//Sequence trackers 
byte div_count = 0; 
int8_t active_note_count = 0; //only one buffer for captured notes
byte chanArray[MAX_CHANNELS] = {CHAN1,CHAN2,CHAN3,CHAN4}; //convenice array for loops over all the channels

NoteBuffer captureBuf =   NoteBuffer(0,0,false);
NoteBuffer generateBuf1 = NoteBuffer(1,CHAN1,false); //monophonic sequence
NoteBuffer generateBuf2 = NoteBuffer(1,CHAN2,false); //monophonic sequence
NoteBuffer generateBuf3 = NoteBuffer(1,CHAN3,true); //polyphonic sequences 
NoteBuffer generateBuf4 = NoteBuffer(1,CHAN4,false); //monophonic sequence
NoteBuffer *GenBuf[MAX_CHANNELS] = {&generateBuf1,&generateBuf2,&generateBuf3,&generateBuf4}; //convenience array for buffers
  
bool arp_on = false;

//midi cc controls
short cc_t_value = 0 ; //use t for MIDI cc mutating operations
byte gchoice1 = random(0,6); //position in set of values to chose from

void setup(){
  once:
  //Serial.begin(19200);
  #ifndef MIDIONLY
  Serial.begin(9600);
  #endif
  //Serial.println("[i] Setting up...");  
  
  pinMode(SWITCH_0R_PLAY, INPUT_PULLUP);
  pinMode(SWITCH_0L_RESET, INPUT_PULLUP);

  //pinMode(PIN_RAW_INPUT, INPUT_PULLUP);
  digitalWrite(PIN_LED_PLAYING, HIGH);
  digitalWrite(PIN_LED_TEMPO, HIGH);
  pinMode(PIN_LED_PLAYING, OUTPUT);
  pinMode(PIN_LED_TEMPO, OUTPUT);
  
  for (byte i=34; i<=53; i++){
    pinMode(i, INPUT_PULLUP);
  }
  for (byte i=22; i<=32; i++){
    pinMode(i, OUTPUT);
  }
  pinMode(ALG_POT_1, INPUT);
  pinMode(DIV_POT_1, INPUT);
  pinMode(ALG_POT_2, INPUT);
  pinMode(DIV_POT_2, INPUT);
  pinMode(ALG_POT_3, INPUT);
  pinMode(DIV_POT_3, INPUT);
  pinMode(ALG_POT_4, INPUT);
  pinMode(DIV_POT_4, INPUT);

  randomSeed(SEED);
  set_tempo();
  init_check();
  MsTimer2::set(tempo_delay, timer_callback);
  MsTimer2::start();
  
  //Set MIDI properties and callback handlers
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
  MIDI.setHandleNoteOn(onNoteOnCapture);
  MIDI.setHandleNoteOff(onNoteOffCapture);
  MIDI.setHandleActiveSensing(onActiveSensing);
  MIDI.setHandleAfterTouchChannel(onTouch);
}

void set_tempo(){
  uint32_t pot_read;
  //uint32_t temp_read;
  uint8_t  temp_read;
  //setting the timer based on 4 note ticks means we can't send events faster than this.
  //Using 4 PPQN delay ms = 5000/bpm - 4 clock ticks per quarter note
  
  pot_read = analogRead(TEMPO_POT);  

  //for smoothness reduce the resolution of the tempo setting
  // fluxuation in pot reads can cause weird tempo drift. Plus we can 
  // save 2 bits by only using multiples of 4 and send tempo as a single byte in a midi cc
  temp_read = map(pot_read, 0,1023, MINTEMPO/4, MAXTEMPO/4); 

  if (abs((temp_read << 2) - tempo) >= 4){
    tempo = temp_read << 2;    
    tempo_delay =  5000.0/tempo;
    
    #ifdef SERIALVIS
    Serial.println("TEMPO," + String(tempo) + ",0"); 
    #endif
    
    #ifdef MIDICLOCKCOMMAND    
    //Since tempo is always a multiple of 4, we can transmit higher tempo values if we use this last bit
    //byte is 0-255, so adding two extra byte gives values 0-510 but use only even numbers
    MIDI.sendControlChange(MIDICLOCKNUMBER, temp_read , MIDICLOCKCHAN);
    #endif

  }
}

void timer_callback(){
  send_tick = true;
}

void onTouch(byte channel, byte press) {
  //Serial.println("PRESS: " + String(press));
}

void onActiveSensing(){
  //Don't transmit this message through
  //MIDI.sendActiveSensing();
}

//track note off events and decrement the counter - this is how we know we're done with the current note sequence
void onNoteOffCapture(byte channel, byte pitch, byte velocity){
  bool err = false;
  if (pitch < 21) {
    #ifdef CAPTUREDEBUG
    Serial.println("BAD PITCH OFF low VALUE");
    #endif
    err = true;
  } else if (pitch > 127) {
    #ifdef CAPTUREDEBUG
    Serial.println("BAD PITCH OFF high VALUE");
    #endif
    err = true;
  }
  if(play_switch){
    active_note_count = (active_note_count - 1);
    if (active_note_count <= 0) {
      active_note_count = 0;
    }
    #ifdef CAPTUREDEBUG
    Serial.println("[D] CAPTURE: DEACTIVE: " + String(active_note_count));  
    #endif

  } else {
    active_note_count = 0;
    for(byte i=0; i< MAX_CHANNELS; i++){
      if (regen[i] != CLOCK && err == false){
        #ifdef SERIALVIS
        //Serial.println("[n] OFF: chan " + String(chanArray[i]) + " pitch: " + String(pitch) + " vel: 0" );
        Serial.println("OFF," +  String(pitch) + ",0," +  String(chanArray[i]));
        #endif
        MIDI.sendNoteOff(pitch , 0, chanArray[i]);
      } 

    }
  }
}
//Record note on and off events received from the midi input channel.
//Note on events will save notes to the buffer and increment an "active" counter
void onNoteOnCapture(byte channel, byte _pitch, byte _vel){
  //some data validation
  byte pitch = _pitch;
  byte vel = _vel;
  bool err = false;

  if (pitch < 21 && vel != 0) {
    #ifdef CAPTUREDEBUG
    Serial.println("[E] BAD PITCH ON low VALUE");
    #endif
    err = true;
  } else if (pitch > 127) {
    #ifdef CAPTUREDEBUG
    Serial.println("[E] BAD PITCH ON high VALUE");
    #endif
    err = true;
  }
  if (vel > 127){
    vel = 127;
    #ifdef CAPTUREDEBUG
    Serial.println("[E] BAD VEL VALUE");
    #endif
    err = true;
  }
  if (vel == 0){
    vel = 0;
    #ifdef CAPTUREDEBUG
    Serial.println("[E] VEL 0 note off ");
    #endif
    err = true;
  }
  
  if (play_switch) { //activate note capture
    gen_switch = true;
    if (active_note_count <= 0){
      #ifdef CAPTUREDEBUG
      Serial.println("[D] CAPTURE: **New set of notes**");
      #endif
      captureBuf.startNewSet();
      active_note_count = 0;      
      }
    active_note_count = active_note_count + 1;
    #ifdef CAPTUREDEBUG
    Serial.println("[D] CAPTURE: NEW ACTIVE: " + String(active_note_count));  
    #endif
    if (err) {
      return;
    }  

    for (byte i=0; i< MAX_CHANNELS; i++){
      if (regen[i] != CLOCK) { //only regen if ARP is on
        regen_switch[i] = true; //we've captured a note and should now make sure to regenerate
        //Serial.println("REGEN TRUE Channel " + String(chanArray[i]));
      }      
    }
    if (arp_on){
      MidiNote note(channel,pitch,vel,false);
      captureBuf.add(note);
    }

  } else {
    active_note_count = 0;
    for (byte i=0; i< MAX_CHANNELS; i++){
      if (regen[i] != CLOCK) {
        //TODO try sending original velocity
        #ifdef SERIALVIS
        //Serial.println("[n] ON: chan: " + String(chanArray[i]) + " pitch: " + String(pitch) + " vel: " + String(vel));
        Serial.println("ON," +  String(pitch) + "," + String(vel) +  "," +  String(chanArray[i]));
        #endif
        MIDI.sendNoteOn(pitch, vel, chanArray[i]);
      }
    }
  } 
}

/*===============================
  Generative functions 
 Called when generating sequences. Given the channel, these functions will add one or more notes
 to the generative buffer for that sequence.
=================================*/


//I really like this one
//simlar to alg2 but add octave of the second note
void repeat_alg5(byte chan_index, byte num_repeates){
  MidiNote new_note; 
  MidiNote next_note;
  byte tot = captureBuf.getCurNoteNum();
    
  if (num_repeates > MAX_REPEATS){
    num_repeates = MAX_REPEATS;
  }
   
  for (byte j=0; j< tot-1; j++) {
    new_note = captureBuf.getCurNote();
    next_note = captureBuf.getNote(j+1);
    for (byte k=0; k<num_repeates; k++) {
      byte choice = random(0,3); //add octave chance  
      GenBuf[chan_index]->add(new_note);  
      GenBuf[chan_index]->add(next_note);  
      if (choice == 0) {
        if ( next_note.pitch >= OCTAVECHANGE && next_note.vel != 0 ) {
          next_note.pitch = new_note.pitch - 12;
        } else if ( next_note.pitch < OCTAVECHANGE && next_note.pitch != 0){
          next_note.pitch = new_note.pitch + 12;
        } 
        GenBuf[chan_index]->add(next_note);
      }
    }    
    captureBuf.increment();
  }
}

//simlar to alg2 but replace the second note with an occasional octave of the first
void repeat_alg4(byte chan_index, byte num_repeates){
  MidiNote new_note; 
  MidiNote next_note;
  byte tot = captureBuf.getCurNoteNum();
    
  if (num_repeates > MAX_REPEATS){
    num_repeates = MAX_REPEATS;
  }
   
  for (byte j=0; j< tot-1; j++) {
    new_note = captureBuf.getCurNote();
    byte choice = random(0,3); //add octave chance  
    for (byte k=0; k<num_repeates; k++) {
      if (choice == 0) {
        if ( new_note.pitch >= OCTAVECHANGE && new_note.vel != 0 ) {
          next_note = new_note;
          next_note.pitch = new_note.pitch - 12;
        } else if ( new_note.pitch < OCTAVECHANGE && new_note.pitch != 0){
          next_note = new_note;
          next_note.pitch = new_note.pitch + 12;
        }
      } else {
        next_note = captureBuf.getNote(j+1);
      }
      GenBuf[chan_index]->add(new_note);  
      GenBuf[chan_index]->add(next_note);  
    }    
    captureBuf.increment();
  }
}


//1,2,3,4 -> 1,2,1,2,1,3,1,3,1,4,1,4 (4*(n-1))
void repeat_alg3(byte chan_index, byte num_repeates){
  MidiNote new_note; 
  MidiNote next_note;
  
  byte tot = captureBuf.getCurNoteNum();

 if (num_repeates > MAX_REPEATS){
    num_repeates = MAX_REPEATS;
  }
  
  for (byte j=0; j< tot-1; j++) {
    new_note = captureBuf.getCurNote();           
    next_note = captureBuf.getNote(j+1);
    for (byte k=0; k<num_repeates; k++) {
      GenBuf[chan_index]->add(new_note);  
      GenBuf[chan_index]->add(next_note);  
    }   
    captureBuf.increment();
  }
}

//1,2,3,4 -> 1,2,1,2,1,3,1,3,1,4,1,4 (4*(n-1))
void repeat_alg2(byte chan_index, byte num_repeates){
  MidiNote new_note = captureBuf.getCurNote(); 
  MidiNote next_note;
  byte tot = captureBuf.getCurNoteNum();
 
  if (num_repeates > MAX_REPEATS){
    num_repeates = MAX_REPEATS;
  }


  for (byte j=1; j< tot; j++) {        
    next_note = captureBuf.getNote(j);
    for (byte k=0; k<num_repeates; k++) {
      GenBuf[chan_index]->add(new_note);  
      GenBuf[chan_index]->add(next_note);  
    } 
    captureBuf.increment();
  }
}

//1,2,3,4 -> 1,1,1,2,2,2,3,3,3,3,4,4,4,
void repeat_alg1(byte chan_index, byte num_repeates){
  MidiNote new_note = captureBuf.getCurNote(); 
  byte tot = captureBuf.getCurNoteNum();
 
  if (num_repeates > MAX_REPEATS){
    num_repeates = MAX_REPEATS;
  }

  for (byte j=0; j< tot; j++) {        
    new_note = captureBuf.getNote(j);
    for (byte k=0; k<num_repeates; k++) {
      GenBuf[chan_index]->add(new_note);  
    } 
    captureBuf.increment();
  }
}

//TODO other patters
//after generating the repeats, apply a swap?
void repeating_sets(byte chan_index) {
  
  byte repeats = octave[chan_index] + 1
  
  switch(genType[i]){
    case 9:
    case 8:
    case 7:
      //random stuff needs to happen here (with octaves or patterns?
    case 6:
      repeat_alg5(chan_index, repeats);
      break;
    case 5:
      repeat_alg4(chan_index, repeats);
      break;
    case 4:
      repeat_alg3(chan_index, repeats);
      break;
    case 3:
      repeat_alg2(chan_index, repeats);
      break;
    case 2:
      repeat_alg1(chan_index, repeats); 
      break;
      //add octaves?
    case 1:
      repeat_alg1(chan_index, repeats);
      break;
  }
    

  repeat_alg1(chan_index, repeats);
  //add octaves to the end of the arp sequence

  /*

  //octaves can be determined by the alg, repeats determined by octave switch?
  if (octave[i] == 1) {
    addOctaves(i); 
  }

  switch(genType[chan_index]) { //no breaks so fall through (intented to have more notes for higher settings)
  case 11:
  case 10:
  case 9:
  case 8:
  //add octive
  //shuffle
  //1,3,1,3..
  case 7:
  case 6:
  //add octive
  //1,3,1,3....
  case 5:
  case 4:
  //1,3,1,3,2,3,2,3..k,n,k,n
  case 3:
  case 2: 
  //1,2,1,2,1,3,1,3
  case 1:  
    break;
  }
  */
}


void change_one_octave(byte chan_index){
  byte tot = GenBuf[chan_index]->getCurNoteNum();
  byte choice = random(0,tot);
  MidiNote new_note =  GenBuf[chan_index]->getNote(choice);
  choice = random(0,2);
  if (choice == 0 && new_note.pitch > 33 && new_note.vel != 0 ) {
    new_note.pitch = new_note.pitch - 12;
  } else if (choice == 1 && new_note.pitch < 108 && new_note.pitch != 0){
    new_note.pitch = new_note.pitch + 12;
  }
  #ifdef GENDEBUG
  Serial.println("[G] CHANGE OCTAVE: " + String(new_note.pitch));
  #endif
  GenBuf[chan_index]->add(new_note);
}

//adds octaves to the end of the currently generated notes
void addOctaves(byte chan_index){
  MidiNote new_note; 
  byte root = captureBuf.getRoot();
  byte tot = GenBuf[chan_index]->getCurNoteNum();
  #ifdef NOTEDEBUG
  Serial.println("[D] OCTAVE " + String(chanArray[chan_index]) + " " + String(octave[chan_index]));
  #endif
  
  if (root <= OCTAVECHANGE) { //ascending
    for (byte j=0; j < tot; j++){
      new_note = GenBuf[chan_index]->getNote(j);
      if (new_note.pitch != 0 && new_note.pitch <= 108) {         
        new_note.pitch = new_note.pitch + 12; 
      }
      GenBuf[chan_index]->add(new_note);        
    }
  } else {//descending
    for (byte j=0; j < tot; j++){
      new_note = GenBuf[chan_index]->getNote(j);
      if (new_note.pitch != 0 && new_note.pitch >= 33) {         
        new_note.pitch = new_note.pitch - 12;
      }
      GenBuf[chan_index]->add(new_note);
    }
  }
}

//add a root to the GenBuf and increment it
void add_root_octave_note(byte chan_index){
  MidiNote root_note;
  byte choice;
  root_note.pitch = captureBuf.getRoot();
  choice = random(0,2);
  if (choice == 0) {
    if (root_note.pitch > 44);
    root_note.pitch = root_note.pitch -12;
  } else {
    if (root_note.pitch < 79) {
      root_note.pitch += 12;
    }
  }
  root_note.vel = random(30,70); 
  #ifdef GENDEBUG
  Serial.println("[G] ADDING ROOT: " + String(root_note.pitch));
  #endif
  GenBuf[chan_index]->add(root_note);
}

//add a root to the GenBuf and increment it
void add_root_note(byte chan_index){
  MidiNote root_note;
  byte choice;
  root_note.pitch = captureBuf.getRoot();
  root_note.vel = random(40,80); 
  #ifdef GENDEBUG
  Serial.println("[G] ADDING ROOT: " + String(root_note.pitch));
  #endif
  GenBuf[chan_index]->add(root_note);
}

//add a root to the GenBuf and increment it
void add_rest_note(byte chan_index){
  MidiNote rest_note;
  rest_note.pitch = 0;
  rest_note.vel = 0;
  //#ifdef GENDEBUG
  //Serial.println("[G] ADDING REST: " + String(rest_note.pitch));
  //#endif
  GenBuf[chan_index]->add(rest_note);
}

void add_new_note(byte chan_index, byte choice){
 
  MidiNote new_note =  captureBuf.getNote(choice);
  new_note.vel = random(20,50);
  //#ifdef GENDEBUG
  //Serial.println("[G] ADDING NOTE: " + String(new_note.pitch));
  //#endif
  choice = random(0,3);
  if (choice == 0 && new_note.pitch > 33 & new_note.vel !=0) {
    new_note.pitch = new_note.pitch - 12;
  } else if (choice == 1 && new_note.pitch < 104 && new_note.pitch !=0){
    new_note.pitch = new_note.pitch + 12;
  }
  GenBuf[chan_index]->add(new_note);
}

//add the exiting notes but in reverse  to the buffer (doubles the length of the arp)
void add_reverse(byte chan_index) {
  byte tot = GenBuf[chan_index]->getCurNoteNum();
  tot=tot-1;
 
  MidiNote new_note;
  new_note.pitch = GenBuf[chan_index]->getRoot();
  new_note.vel = random(30,70);
  if (new_note.pitch < 104 && new_note.pitch != 0){
    new_note.pitch += 12;
  }

  GenBuf[chan_index]->add(new_note);
 
  for (signed char j=tot; j > -1; j--){
    new_note = GenBuf[chan_index]->getNote(j);
    GenBuf[chan_index]->add(new_note);
  }
}

//reverse all notes in the current generate buffer
void swap3(byte chan_index){
  byte tot = GenBuf[chan_index]->getCurNoteNum();
  
  if (tot % 2 == 1){
    #ifdef GENDEBUG
    Serial.println("[D] o SWAP3: 0 , " + String(tot-1) + " p: " + String(GenBuf[chan_index]->getNote(0).pitch) + " with: " + String(GenBuf[chan_index]->getNote(tot-1).pitch));         
    #endif
    GenBuf[chan_index]->swapNotes(0, tot-1);
    for (byte j=1; j < tot/2; j++){
      #ifdef GENDEBUG
      Serial.println("[D] o SWAP3: " + String(j) + " , " + String(tot-j-1) + " p: " + String(GenBuf[chan_index]->getNote(j).pitch) + " with: " + String(GenBuf[chan_index]->getNote(tot-j-1).pitch));         
      #endif
      GenBuf[chan_index]->swapNotes(j, tot-j-1);
    } 
  } else {
    for (byte j=0; j < tot/2; j++){
      #ifdef GENDEBUG
      Serial.println("[D] e SWAP3: " + String(j) + " , " + String(tot-j-1) + " p: " + String(GenBuf[chan_index]->getNote(j).pitch) + " with: " + String(GenBuf[chan_index]->getNote(tot-j-1).pitch));         
      #endif
      GenBuf[chan_index]->swapNotes(j, tot-j-1);
    }
  }
}

//reorder groups of three (1,2,3 -> 3,1,2)
//doing swap1, swap2 is (1,2,3,4 -> 1,3,4,2), (1,2,3,4,5,6) -> 1,3,6,2,5,4
//doing swap2, swap1 is (1,2,3,4 -> 4,2,1,3), (1,2,3,4,5 -> 4,2,1,3,5), (1,2,3,4,5,6 -> (2,1,4,3,6,5 -> 4,2,1,5,3,6)
void swap1(byte chan_index){
  byte tot = GenBuf[chan_index]->getCurNoteNum();
  for (byte j=0; j< tot; j++){
    if ((j%3)==0 && j < (tot-2)){ 
      
      #ifdef GENDEBUG
      Serial.println("[D] SWAP1: " + String(GenBuf[chan_index]->getNote(j).pitch) + " with: " + String(GenBuf[chan_index]->getNote(j+2).pitch));    
      #endif
      GenBuf[chan_index]->swapNotes(j, j+2);
      
      #ifdef GENDEBUG
      Serial.println("[D] SWAPed1: " + String(GenBuf[chan_index]->getNote(j).pitch) + " with: " + String(GenBuf[chan_index]->getNote(j+2).pitch));
      Serial.println("[D] now SWAP1: " + String(GenBuf[chan_index]->getNote(j+1).pitch) + " with: " + String(GenBuf[chan_index]->getNote(j+2).pitch));
      #endif
      GenBuf[chan_index]->swapNotes(j+1, j+2);
      #ifdef GENDEBUG
      Serial.println("[D] SWAPed1: " + String(GenBuf[chan_index]->getNote(j+1).pitch) + " with: " + String(GenBuf[chan_index]->getNote(j+2).pitch));
      #endif
    } 
  }
}

//swap every other note (1,2,3 -> 2,1,3) and (1,2,3,4 -> 2,1,4,3)
void swap2(byte chan_index){
  byte tot = GenBuf[chan_index]->getCurNoteNum();
  for (byte j=0; j< tot; j++){
    if (j%2==0 && j < (tot-1)){ //swap every other note but not the last note when there is an odd number
      #ifdef GENDEBUG
      Serial.println("[D] SWAP2: " + String(GenBuf[chan_index]->getNote(j).pitch) + " with: " + String(GenBuf[chan_index]->getNote(j+1).pitch));
      #endif
      GenBuf[chan_index]->swapNotes(j, j+1);
      #ifdef GENDEBUG
      Serial.println("[D] SWAPed2: " + String(GenBuf[chan_index]->getNote(j).pitch) + " with: " + String(GenBuf[chan_index]->getNote(j+1).pitch));
      #endif
    }  
  }
}

//reorders the notes in the current buffer (not a random shuffle)
void genShuffle(byte chan_index){
   byte choice = random(0,3);
  change_one_octave(chan_index);
  if (choice == 0) {
    swap3(chan_index);
  } else if (choice == 1){
    swap2(chan_index);
  } else {
    swap1(chan_index);
  }
}

/*
void generate_more(byte chan_index){
  //Serial.println("[i] GENERATING MORE VARIETY ");
  byte rests_freq = random(0,3);
  byte rest_num = random(1,5); //there's a big difference between 1-5 rests and 1-4 rests
  byte octave_chance = random(0,3);
  
  MidiNote new_note = captureBuf.getCurNote();
  MidiNote rest_note;

  for(byte k = 0; k< rest_num; k++){
    if(chan_index % rests_freq == 0){
      //Serial.println("[GM] Adding a rest");
      GenBuf[chan_index]->add(rest_note);
    }
  }
  
  if (octave_chance == 0 && new_note.pitch > 33 & new_note.vel !=0) {
    new_note.pitch = new_note.pitch - 12;
  } else if (octave_chance == 1 && new_note.pitch < 104 && new_note.pitch !=0){
    new_note.pitch = new_note.pitch + 12;
  }
  
  GenBuf[chan_index]->add(new_note);

}
*/


//when alg==0 and djent is on we can make some interesting rhythms
void generate_djent(byte chan_index, byte djentness){
  MidiNote rest_note;
  byte djenty_bar = random(1, djentness);
  
  for (int i = 0; i < djenty_bar; i++){
    GenBuf[chan_index]->add(rest_note);
  }
}

//convert the capture buffer into the generative buffer when we're on the ARP setting
void generate(){
  MidiNote new_note;
  byte tot;
  byte num_additions;
  tot = captureBuf.getCurNoteNum();

  if (tot > BUFFER_SIZE){
    Serial.println("[E] buffer overflow error!");
    tot = BUFFER_SIZE;
  }
  for (byte i=0; i< MAX_CHANNELS; i++) {
    //Check if no generation is needed for this channel
    if (regen[i] == CLOCK ||  regen[i] == FREEZE || regen_switch[i] == false ){
      continue; 
    }  
    #ifdef SERIALVIS
    //Serial.println("[i] GENERATE ARP: chan: " + String(chanArray[i]));
    Serial.println("GEN,0," + String(chanArray[i]) );
    #endif

    captureBuf.setRoot();
    GenBuf[i]->startNewSet(); 
    GenBuf[i]->setRoot(captureBuf.getRoot());
    
    //First gen setting is root note only
    if (genType[i] == 0 ){
      if (octave[i] == DJENT){
        
        num_additions = random(4,10);
        for (byte k=0; k < num_additions; k++) {
          generate_djent(i, 3);
          add_root_note(i);
        }
        
      } else {
        //Serial.println("adding root only");
        new_note.set(i, captureBuf.getRoot(), 40, false);
        GenBuf[i]->add(new_note);
      }
      continue;
    }

    //Generate for mode three "REPEATS"
    //for mode 2, monophonic alt arp different set of patterns
    if (mode_setting == REPEAT && tot > 2) {
      //second type of generation for mode 2
      repeating_sets(i);

      
      return;
    }

    //Generate for Normal/ chord modes 0 and 1 (also if there's less than 3 notes captured)
    for (byte j=0; j< tot; j++) {
      new_note = captureBuf.getCurNote();           
      #ifdef GENDEBUG
      Serial.println("[D] GEN new chan" + String(chanArray[i]) + " note: " + String(new_note.pitch) + " type: " + String(genType[i]));  
      #endif            
      GenBuf[i]->add(new_note);  
      if (octave[i] == DJENT) { //need to add some rests first
        generate_djent(i, 3);
      }      
      captureBuf.increment();
    }  

    if ((tot > 2) && (genType[i] > 0)) {
      switch(genType[i]) { //no breaks so fall through (intented to have more notes for higher settings)
      case 9:
        GenBuf[i]->setVelDrift(3);
        change_one_octave(i);
        add_root_note(i);
        add_new_note(i,3);
        change_one_octave(i);
        if(octave[i] == 1) {
          addOctaves(i);
        }
        add_new_note(i,2);
        add_reverse(i);
        change_one_octave(i);
        swap1(i);
        add_root_note(i);        
        break;
    
      case 8: 
        GenBuf[i]->setVelDrift(2);
        add_root_note(i);
        change_one_octave(i);
        add_root_octave_note(i);
        swap1(i); 
        add_root_note(i);
        add_reverse(i); 
        change_one_octave(i);   
        if(octave[i] == 1) {
          addOctaves(i);
        }
        swap2(i);
        break;
      case 7: //shuffle gets applied at the end of a bar
        GenBuf[i]->setVelDrift(2);
        add_root_note(i);
        add_new_note(i,1);
        change_one_octave(i);
        if(octave[i] == 1) {
          addOctaves(i);
        }
        add_reverse(i);
        swap1(i);
        break;       
      case 6: 
        GenBuf[i]->setVelDrift(1);
        add_root_note(i);
        change_one_octave(i);
        if(octave[i] == 1) {
          addOctaves(i);
        }
        add_reverse(i);
        swap2(i);   
        break;
      case 5:  //extra notes are added
        GenBuf[i]->setVelDrift(1);
        add_root_note(i);
        change_one_octave(i);
        swap1(i);       
        if(octave[i] == 1) {
          addOctaves(i);
        }
        add_reverse(i);      
        break;
      case 4:
        GenBuf[i]->setVelDrift(1);
        if(octave[i] == 1) {
          addOctaves(i);
        }
        add_reverse(i); 
        break;
      case 3:
        add_root_note(i);
        swap2(i);
        if(octave[i] == 1) {
          addOctaves(i);
        } 
        break;
      case 2:
        swap2(i);
        swap1(i);
        if(octave[i] == 1) {
          addOctaves(i);
        } 
        break; 
      case 1:  
        if(octave[i] == 1) {
          addOctaves(i);
        }     
        break;
      }
    } 
          
    if (genType[i] >= GENERATIVE_SETTING+1){
      //GenBuf[i]->shuffle();  
      genShuffle(i);
      #ifdef GENDEBUG
      Serial.println("[D] Generate SHUFFLE");
      #endif
    }  
    regen_switch[i] = false;
  } 
  
}

void check_and_send_note_off(byte i){
  MidiNote prev_note = GenBuf[i]->getPrevNote();
  MidiNote next_note; 
  byte choice;
  

  //for now all tracks act as mono unless chord mode enabled
  if (GenBuf[i]->poly == false) { //handle the monophic sequences    

    //check if prev note still on   
    //Serial.println("[D] All Notes Off (check) " + String(chanArray[i]));    


    //MIDI.sendControlChange(midi::AllNotesOff, 0, 0);
    MIDI.sendControlChange(midi::AllSoundOff, 0, chanArray[i]);  
    MIDI.sendNoteOff(prev_note.pitch, 0, chanArray[i]);
    //check if the current note is still on       
    next_note = GenBuf[i]->getCurNote();
    //if (next_note.on) {
    MIDI.sendNoteOff(next_note.pitch, 0, chanArray[i]);
    #ifdef SENDDEBUG
    Serial.println("[s] curr not off ->sent off. chan " + String(chanArray[i]) + " note: " + String(next_note.pitch) + " chan: " + String(next_note.chan) + " on: " + String(next_note.on));  
    #endif 
    GenBuf[i]->setCurNoteOff();
    //MIDI.sendNoteOff(next_note.pitch, 0, chanArray[i]);

    #ifdef SERIALVIS
    //Serial.println("[n] OFF: chan: " + String(chanArray[i]) + " pitch: " + String(next_note.pitch) + " vel: 0");
    Serial.println("OFF," +  String(next_note.pitch) + ",0," +  String(chanArray[i]));
    #endif
    //}
  }
  
  else {
    //polyphonic case 
    //TODO ensure this is working how I think it is
    //choice = random(0,POLYPHONY);
    choice = 0;
    if (choice == 0) {
      prev_note = GenBuf[i]->getPreviNote(POLYPHONY);

      MIDI.sendNoteOff(prev_note.pitch, 0, chanArray[i]);

      GenBuf[i]->setPrevNoteOff();          
      #ifdef SENDDEBUG
      Serial.println("[s poly] Sent prev off. chan " + String(chanArray[i]) + " note: " + String(prev_note.pitch) + " vel: 0 on:" + String(prev_note.on));  
      #endif
      #ifdef SERIALVIS
      //Serial.println("[n] OFF: chan: " + String(chanArray[i]) + " pitch: " + String(prev_note.pitch) + " vel: 0");
      Serial.println("OFF," +  String(prev_note.pitch) + ",0," +  String(chanArray[i]));
      #endif
    } 
  }
  


}

//sends the next note in the buffer and increments the read head for channel index i
void check_and_send_note_on(byte i){
  
  if (first_note){
    div_count=0;
    first_note=false;
  }
  
  MidiNote next_note = GenBuf[i]->getCurNote(); 
  byte num_chord = 0;

  if(next_note.pitch > 0 && next_note.vel > 0) { //only send a valid note
    MIDI.sendNoteOn(next_note.pitch, next_note.vel, chanArray[i]);
    #ifdef SERIALVIS    
    Serial.println("ON," +  String(next_note.pitch) + "," + String(next_note.vel) +  "," +  String(chanArray[i]));
    #endif
    GenBuf[i]->setCurNoteOn();
    #ifdef SENDDEBUG
    Serial.println("[s] Sent note on. chan " + String(chanArray[i]) + " note: " + String(next_note.pitch) + " vel: " + String(next_note.vel) + " on: " + String(next_note.on));
    #endif

  }

  //For polyphonic buffers, chord mode occurs for modes 1 and 2
  if (mode_setting >= CHORD && GenBuf[i]->poly){
    next_note = GenBuf[i]->getPrevNote();
    MIDI.sendNoteOn(next_note.pitch, next_note.vel, chanArray[i]);
    #ifdef SERIALVIS
    //Serial.println("[n] ON: chan: " + String(chanArray[i]) + " pitch: " + String(next_note.pitch) + " vel: " + String(next_note.vel));
    Serial.println("ON," +  String(next_note.pitch) + "," + String(next_note.vel) +  "," +  String(chanArray[i]));
    #endif
  } 
  GenBuf[i]->increment();  //move the play head    
}

//Turns off all notes in the buffer regardless of counter, using the getNote() which can accesses the full buffer
void send_all_gen_off(byte i){
  MidiNote n_note;
  byte max;
  max = GenBuf[i]->getMaxNoteNum();
  n_note = GenBuf[i]->getCurNote();
  GenBuf[i]->setCurNoteOff();
  MIDI.sendNoteOff(n_note.pitch, 0, chanArray[i]);
  for (byte j=0; j< max; j++) {
    n_note = GenBuf[i]->getNote(j, true);
    if (n_note.pitch > 0) {     
      MIDI.sendNoteOff(n_note.pitch, 0,chanArray[i]);
      GenBuf[i]->setNoteOff(j);
      #ifdef SERIALVIS
      //Serial.println("[n] OFF: chan: " + String(chanArray[i]) + " pitch: " + String(n_note.pitch) + " vel: 0");
      Serial.println("OFF," +  String(n_note.pitch) + ",0," +  String(chanArray[i]));
      #endif
    }
  }   
}



//--------- MIDI CC functions for specific synths -------------//
/* The follow are to deliver time based CC changes - based on sine/cose for organic sounding change in parameters
div_time: time parameter - shoudl be a counter for "time" or clock ticks
big: - 1 for big drift or 0 for small drive return - the byte to send as the CC value
*/
byte drift(byte div_time, byte type, bool big){
  if (type > 6)
    type = 6;
  switch(type){
    case 1:
    //for small: max 75, min 15
      if (big) {
        return (cos(div_time/3) + cos(div_time/1) + 2) *31;
      } 
        return (cos(div_time/3) + cos(div_time/1) + 3) * 15; 
    case 2:
    //for small: max 76, min 28
      if (big) {
        return (sin(div_time/5) + cos(div_time/3) + 2) * 31;
      }
        return (sin(div_time/5) + cos(div_time/3) + 4) * 13; 
    case 3:
    //for small: max 78, min 27 
      if (big) {
        return (sin(div_time/9) + sin(div_time/4) + 2) *31;
      }
      return (sin(div_time/9) + sin(div_time/4) + 4) *13; 
    case 4:
    //for small: max 66, min 14
      if (big) {
        return (sin(div_time/7) + sin(div_time/5) + cos(div_time/11) + 3) * 22;
      }
      return (sin(div_time/9) + sin(div_time/7) + cos(div_time/13) + 4) * 10; 
    case 5: 
    //slower and smooth
    //min 30 max 90 for small; min 0 max 120 for big
      if (big) {
        return (cos(div_time/15) + 1) * 60;
      } 
      return (cos(div_time/15) + 2) * 30;
    case 6:
      if (big) {
        return (sin(div_time/40) + 1) * 60;
      } 
        return (sin(div_time/40) + 2) * 30;
    default:
      return 20;
  }
}


//send a midicc message, values depend on time based changes rather than something set explitly
void volcaFMCC(){
  byte cc_attack=0;
  byte cc_decay =0;
  byte cc_rate  =0;
  byte cc_depth =0;
    
  if (cc_t_value %2 == 0) {
    cc_attack = drift(cc_t_value, 2, false); //attack
    cc_decay  = drift(cc_t_value, 3, false);
    cc_rate   = drift(cc_t_value, 5, false)-15;
    cc_depth  = (90 - cc_rate);
    //Serial.println("SENDING to RATE  " + String(cc_rate));
    //Serial.println("SENDING to DEPTH  " + String(cc_depth));
    MIDI.sendControlChange(FM_MOD_ATTACK, cc_attack, VOLCAFMCHANNEL);
    MIDI.sendControlChange(FM_MOD_DECAY, cc_decay+25, VOLCAFMCHANNEL);
    MIDI.sendControlChange(FM_LFO_RATE, cc_rate, VOLCAFMCHANNEL);
    MIDI.sendControlChange(FM_LFO_DEPTH, cc_depth, VOLCAFMCHANNEL);

  } else {
    cc_attack = drift(cc_t_value, 2, false); //attack
    cc_decay  = drift(cc_t_value, 1, false);
    //Serial.println("SENDING to CA DECAY  " + String(cc_decay));
    MIDI.sendControlChange(FM_CA_ATTACK, cc_attack, VOLCAFMCHANNEL);
    MIDI.sendControlChange(FM_CA_DECAY, cc_decay+35, VOLCAFMCHANNEL);
  }  
}

void NTS1CC(){
  byte cc_attack=0;
  byte cc_decay =0;
  byte cc_rate  =0;
  byte cc_depth =0;
  byte cc_res   =0;
  byte cc_osc   =0;

  if (cc_t_value %2 == 0) {
    cc_attack = drift(cc_t_value, 1, false) -12; 
    cc_decay  = drift(cc_t_value, 3,  false) + 40;
    //Serial.println("SENDING Att " + String(cc_attack) + "  dec " + String(cc_decay));
    MIDI.sendControlChange(NTS1ATTACK, cc_attack, NTS1CHANNEL);
    MIDI.sendControlChange(NTS1RELEASE, cc_decay, NTS1CHANNEL);

  } else {    
    cc_osc   = drift(cc_t_value, 5, false)/2; 
    cc_rate  = drift(cc_t_value, 6, false)/2; 
    cc_res    = drift(cc_t_value, 4, false) - 10;
    //Serial.println("SENDING rate " + String(cc_rate) + "  depth " + String(cc_depth) + " res " + String(cc_res) );
    MIDI.sendControlChange(NTS1OSCALT, cc_osc, NTS1CHANNEL);
    MIDI.sendControlChange(NTS1LFORATE, cc_rate, NTS1CHANNEL);
    MIDI.sendControlChange(NTS1RESONANCE, cc_res, NTS1CHANNEL);
    
  } 
}

void OBNEDarkstarCC(byte channel){
   
  byte cc_1_value; 
  byte cc_2_value; 
  byte cc_3_value; 
  byte cc_4_value; 
  byte cc_5_value;   



  byte values[] = {0, 5, 32, 64, 122, 64, 32}; //for pictch changes
  byte values2[] = {0, 100}; //on or off

  byte choice1 = 0;
  byte choice2 = 0;
  //designed for OBNE darkstar v3
  if (channel != 0){
    return; //assume channel 1 for now
  }

  switch(genType[channel]){
    case 0:
      break;
    case 1:  
    case 2:
      cc_1_value = drift(cc_t_value, 1, true); //decal
      //Serial.println("CC,15, " + String(cc_1_value) + "," + String(1));
      MIDI.sendControlChange(15, cc_1_value, 1);
      break;    
    case 3:
    case 4:
      cc_1_value = drift(cc_t_value, 2, true); // += 46 filter
      //Serial.println("CC,20 " + String(cc_1_value)+ "," + String(1));
      MIDI.sendControlChange(20, cc_1_value, 1);
      break;
    case 5:
    case 6:
      cc_1_value = drift(cc_t_value, 3, false) + 20; //decay
      cc_2_value = drift(cc_t_value, 4, false) + 20; //filter
      cc_3_value  = drift(cc_t_value, 6, false) + 40; //slow bitcrush/od
      //Serial.println(" CC 15 " + String(cc_3_value) + " CC 20 " + String(cc_2_value)+ " CC 19 " + String(cc_3_value)); 
      MIDI.sendControlChange(15, cc_1_value, 1);
      MIDI.sendControlChange(20, cc_2_value, 1);
      MIDI.sendControlChange(19, cc_3_value, 1);
      break;
    
    case 7:
    case 8:

      choice2 = random(0,3); //chance to change the pitch
      if (choice2 == 0) {
         cc_1_value = values[(cc_t_value % 2)+gchoice1]; //cycles between two values       
      } 
      choice2 = random(0,3); //chance to change the pitch
      if (choice2 == 0) {
        cc_2_value = values[((cc_t_value % 2)+ gchoice1 + 1)%7];
      }
      
      cc_3_value =  drift(cc_t_value, 4, false) + 5; //feedback
      cc_4_value  = drift(cc_t_value, 6, false) + 30; //slow filter

      MIDI.sendControlChange(16, cc_1_value, 1);
      MIDI.sendControlChange(17, cc_2_value, 1);
      MIDI.sendControlChange(21, cc_3_value, 1);
      MIDI.sendControlChange(20, cc_4_value, 1);

      //Serial.println("            -> CC  " + String(cc_1_value) + " " + String( cc_2_value) + " CC 20 " + String(cc_1_value) + " CC 21 " + String(cc_3_value));
       
    break;
    
    case 9:
    case 10:
      cc_1_value = drift(cc_t_value, 2, true);
      cc_2_value = drift(cc_t_value, 6, false) + 20;

      choice1 = random(0,6);
      if(choice1 == 0){
        cc_3_value  = drift(cc_t_value, 4, false);  //a bit chaotic because of time stuff
      }

      //Serial.println("CC 15 " + String(cc_1_value) + " CC 21 " + String(cc_2_value) + " CC 18 " + String(cc_3_value));
      MIDI.sendControlChange(15, cc_1_value, 1);
      MIDI.sendControlChange(21, cc_2_value, 1);
      MIDI.sendControlChange(18, cc_3_value, 1); //lag

      break;
    case 11:
      //cc_1_value = drift6(cc_t_value, true); // freeze on and off

      choice1 = random(0,6);
      choice2 = random(0,8);

      if (choice1 == 0){
        cc_1_value = values2[cc_t_value % 2];
      }

      cc_2_value = drift(cc_t_value, 3, false) + 20;
      cc_3_value = drift(cc_t_value, 6, false) + 30;
      cc_4_value = drift(cc_t_value, 5, false) + 20;
      if (choice2 == 0){
        cc_5_value  = drift(cc_t_value, 4, false)+ 10;      //todo test this one
      }
      //Serial.println("CC 24 " + String(cc_1_value) + ", CC 20 " + String(cc_2_value) + ", CC 19 " + String(cc_3_value) + ", CC 18 " + String(cc_5_value));
      MIDI.sendControlChange(24, cc_1_value, 1);
      MIDI.sendControlChange(20, cc_2_value, 1);
      MIDI.sendControlChange(19, cc_3_value, 1);
      MIDI.sendControlChange(15, cc_4_value, 1);
      MIDI.sendControlChange(18, cc_5_value, 1);
      break;
  }
}

//handle time-based cc changes. This function ticks the cc time
// don't want to use midi note tick values because of the jump fro 90 to 0, I'd rather it be more continuous
void send_midi_cc(){

  if (div_count % CC_FREQ == 0) {
    #ifdef VOLCAFM
    volcaFMCC();
    #endif

    #ifdef NTS-1
    NTS1CC();
    #endif

    #ifdef MIDICC
    cc_t_value += 1; //time tracker
    #endif
  }
}

//determine if an extra note should be sent as an addition to the current div pattern
bool send_extra_note(byte chan_index){
  if(genType[chan_index] >= GENERATIVE_SETTING && div_count % (div_setting[chan_index]/2) == 0){
    //number of extra notes based on the alg setting. Higher setting is more notes
    byte freq  = (GENERATIVE_SETTING +3 ) - (genType[chan_index] - GENERATIVE_SETTING); 
    byte chance = random(0, freq);
    if (chance == 0){
      return true;
    }
  } 
  return false;   
}

//a bar is defined by the current number of notes in the generated buffer
void bar_regenerate(byte chan_index){
  byte temp_bars;
  if (regen[chan_index] == ARP) { 
    if (genType[chan_index] > GENERATIVE_SETTING ) {          
      temp_bars = GenBuf[chan_index]->getBars();
      //in the case of constant regeneration of the sequence, we don't want to do this every single note, rather it should be after a full set of notes
      if (regen_bars[chan_index] != temp_bars) {   
        regen_bars[chan_index] = temp_bars;
        if((regen_bars[chan_index] % REGEN_FACTOR == 0)){          
          #ifdef GENDEBUG
          Serial.println("[g] REGENERATE " + String(bars) + " " + String(regen_bars[chan_index] % REGEN_FACTOR)+ " div count " + div_count);
          #endif 
          digitalWrite(arp_leds[chan_index], HIGH);
          
          #ifdef SERIALVIS
          //Serial.println("[i] REGEN: chan: " + String(chanArray[i]) + " bars: " + String(regen_bars[i]));
          Serial.println("REGEN," + String(regen_bars[chan_index]) + "," + String(chanArray[chan_index]));
          #endif
          genShuffle(chan_index);         
          digitalWrite(arp_leds[chan_index], LOW);
        }
      }
    }
  }
}

// Decide if a note should be sent for each channel, based on their various settings
void send_note(){
  MidiNote next_note;
  MidiNote prev_note;
  byte tot =  captureBuf.getCurNoteNum(); 
  
  
  //Send the next note from each buffer
  for(byte i=0; i< MAX_CHANNELS; i++){      
    if (note_off_switch[i]){ // in the case where a channel has switched to clock, turn notes off for this channel
      #ifdef SENDDEBUG
      Serial.println("[D] All Notes Off (main)" + String(chanArray[i]));
      #endif
      //send_all_gen_off(i);
      MIDI.sendControlChange(midi::AllNotesOff, 0, 0);
      MIDI.sendControlChange(midi::AllSoundOff, 0, chanArray[i]);
      note_off_switch[i] = false;
    }  

    if (regen[i] == CLOCK || tot ==0) {//don't send a note when there's no notes or this sequence is set to clock
      continue;
    }
    if(div_setting[i] == 0) {
      Serial.println("[E] div_setting is zero - check pot value");
      div_setting[i] = WHOLENOTE; //0 is undefined behaviour. Just count the full bar here.
    }   
      
    //TODO handle Mode 2 extra notes?
    if (div_count % div_setting[i] == 0 ||  (mode_setting < 2 && send_extra_note(i))){//we're at the start rhythm pattern (defined by div_setting) and ready to send regular note      
    
      //make sure all the previous notes are off
      check_and_send_note_off(i); 
      //generate the notes for the generate buffer
      //## REGENERATE FIRST OR SEND?
      // this must be done here because notes should be turned off before the buffer is shuffled.
      if (gen_switch){
        //#ifdef SENDEBUG
        Serial.println("[D] -- CALL GENERATE --");
        //#endif
        generate();
        gen_switch = false;
      }
      //check if the generated notes should be regenerated
      if (mode_setting < 2) {
        //TODO handle mode setting 2 bar rege
        bar_regenerate(i);
      }
      //send the next note
      check_and_send_note_on(i); 
    }   
   
  }
  #ifdef MIDICC
  send_midi_cc(); //send cc only once per send_note call
  #endif
}

void blink_leds(byte tick){
  //blink the TEMPO for qarter notes
  if (tick % QUARTERNOTE == 0) {
    digitalWrite(PIN_LED_TEMPO, HIGH);    
  } else {
    digitalWrite(PIN_LED_TEMPO, LOW);
  }
  if (play_switch) {
    for (byte i=0; i < MAX_CHANNELS; i++ ){
      if (regen[i] == CLOCK){
        continue;
      }
      if (tick % div_setting[i] == 0) {
        digitalWrite(div_leds[i], HIGH);
        //if (genType[i] > GENERATIVE_SETTING) {
        //  digitalWrite(arp_leds[i], HIGH);
        //}
      } else {
        digitalWrite(div_leds[i], LOW);
        //digitalWrite(arp_leds[i], LOW);
      }
    }
  }
}

//=========================
// Main state machine is managed here
//=========================
void do_timed_midi(){
  //static uint8_t  ticks = 0;
  bool reset_timer = false;

  if(send_start) {
    for(byte i=0; i< MAX_CHANNELS; i++){ 
      GenBuf[i]->restart();
    }
    set_tempo(); 
    
    #ifdef PLAYDEBUG
    Serial.println("[D] DO MIDI. Send Start: " + String(send_start) + " stop: " + String(send_stop) + " play switch: " + String(play_switch) + " reset " + String(reset_switch) + " cp " + String(captureBuf.getCurNoteNum()));
    #endif
    MIDI.sendRealTime(MIDI_NAMESPACE::Start);
    #ifdef SERIALVIS
    Serial.println("CC,Start,0");
    #endif

    send_start = false;
    send_stop = false;  
    div_count = 0;
    send_tick = true; 
  }

  if(send_tick) {    
    MIDI.sendRealTime(MIDI_NAMESPACE::Clock);
    send_tick = false;
    reset_timer = true;
    set_tempo(); 
    
    blink_leds(div_count);  
    if (play_switch) {
      send_note(); 
    }        
        
    div_count = (div_count + 1) %  DIVS_PER_BAR;//loop over the div set by potentiometer
  }   
  if(send_stop) {
    #ifdef PLAYDEBUG
    Serial.println("[D] DO MIDI. Send Start: " + String(send_start) + " stop: " + String(send_stop) + " play switch: " + String(play_switch) + " reset " + String(reset_switch) + " cp " + String(captureBuf.getCurNoteNum()));
    #endif
    MIDI.sendControlChange(midi::AllSoundOff, 0, 0);
    MIDI.sendRealTime(MIDI_NAMESPACE::Stop);
    MIDI.sendRealTime(MIDI_NAMESPACE::SystemReset);

    send_stop = false;
    div_count = 0;
    //Serial.println("[i] Stopping clock");
    #ifdef SERIALVIS
    //Serial.println("[c] Stop ");
    Serial.println("CC,Stop,0");
    #endif
  }
  if(reset_timer) {      
    MsTimer2::stop();
    MsTimer2::set(tempo_delay, timer_callback);
    MsTimer2::start();
    
    reset_timer = false;
  }
}
//=========================
// EVENT hanlders
//=========================
void reset_event() {
  byte tot;
  if (reset_switch){ //the reset switch is on, we've already reset
    return;
  }
  reset_switch = true;
  first_note = true;
  
  active_note_count = 0;

  #ifdef RESETDEBUG
  Serial.println("[D] RESET Send Start: " + String(send_start) + " stop: " + String(send_stop) + " play switch: " + String(play_switch) + " reset " + String(reset_switch) + " cp " + String(captureBuf.getCurNoteNum()));
  Serial.println("[D] CAPTURE BUF:");
  captureBuf.debug();
  #endif 
  captureBuf.resetAll();
  for(byte i=0; i<MAX_CHANNELS; i++){    
    tot = GenBuf[i]->getCurNoteNum();
    div_count=0;
    #ifdef RESETDEBUG
    Serial.println("[D] GEN BUF CHAN: " + String(chanArray[i]));
    GenBuf[i]->debug(); 
    #endif   

    MIDI.sendControlChange(midi::AllSoundOff, 0, chanArray[i]);  
    GenBuf[i]->resetAll();
  }

  #ifdef RESETDEBUG
  Serial.println("[D] All Notes Off (reset) 0");
  #endif
  MIDI.sendControlChange(midi::AllSoundOff, 0, 0);
  MIDI.sendRealTime(MIDI_NAMESPACE::Stop);
  MIDI.sendRealTime(MIDI_NAMESPACE::SystemReset);

  
  #ifdef SERIALVIS
  //Serial.println("[c] Stop");
  Serial.println("CC,Stop,0");
  //Serial.println("[c] SystemReset");
  Serial.println("CC,SystemReset,0");
  #endif
}
  // toggle running state 
void play_on_event(){
  if (!play_switch){
    //Serial.println("[i] PLAY ON ");
    #ifdef PLAYDEBUG
    Serial.println("[D] PLAY ON Send Start: " + String(send_start) + " stop: " + String(send_stop) + " play switch: " + String(play_switch) + " reset " + String(reset_switch) + " cp " + String(captureBuf.getCurNoteNum()));
    #endif
    play_switch = true;    
    send_start = true;
    for (byte i=0; i< MAX_CHANNELS; i++){
      if(regen[i] != CLOCK) {
        //Serial.println("[i] PLAYING channel(s) " + String(chanArray[i]));
        regen_switch[i] = true;
      }
    }
    digitalWrite(PIN_LED_PLAYING, LOW);
  }
}

void play_off_event(){
  if (play_switch){
    //Serial.println("[i] PLAY OFF ");
    #ifdef PLAYDEBUG
    Serial.println("[D] PLAY OFF Send stop: " + String(send_start) + " stop: " + String(send_stop) + " play switch: " + String(play_switch) + " reset " + String(reset_switch) + " cp " + String(captureBuf.getCurNoteNum()));
    #endif
    active_note_count = 0;
    play_switch = false;
    send_stop = true;
    gen_switch = false;
    for(byte i=0; i<MAX_CHANNELS; i++){
      //if(regen[i] == CLOCK){
      //  continue;
      //}
      GenBuf[i]->restart();
      div_count=0;
    }
    digitalWrite(PIN_LED_PLAYING, HIGH);
    for (byte i=0; i< MAX_CHANNELS; i++){
      digitalWrite(div_leds[i], LOW);
      digitalWrite(arp_leds[i], LOW);
    }
  } 
}

//==================
// Input Reads for global parameters and associated functions
//==================
void check_pots(){
  //read the alg (gentype) and div settigs 
  short temp_read;
  byte divs_read;
  byte choice;
  for (byte i=0; i< MAX_CHANNELS; i++){
    temp_read = map(analogRead(gen_pots[i]),0,1023,0,MAX_ALG); //readings are not stable,  ignore the last number
    if (temp_read >= MAX_ALG){
      temp_read = MAX_ALG-1;
    }
    if (genType[i] != temp_read){
      digitalWrite(arp_leds[i], HIGH);      
      
      #ifdef SERIALVIS
      //Serial.println("[i] SET: chan: " + String(chanArray[i]) + " mutation: " + String(temp_read));
      Serial.println("SET ALG," + String(temp_read)+ "," + String(chanArray[i]));
      #endif
      genType[i] = temp_read;
      gen_switch = true;
      regen_switch[i] = true;
      digitalWrite(arp_leds[i], LOW);
    }
    temp_read = analogRead(div_pots[i]); 
    divs_read = map(temp_read,0,1023,0,MAX_DIVS);
    if (divs_read >= MAX_DIVS){
      divs_read = MAX_DIVS-1;
    }
    if(div_setting[i] != div_reference[divs_read]) {
      digitalWrite(div_leds[i], HIGH);
      
      #ifdef SERIALVIS
      //Serial.println("[i] SET: chan: " + String(chanArray[i]) + " divs: " + String(div_reference[divs_read]));
      Serial.println("SET DIV," +  String(div_reference[divs_read]) + "," + String(chanArray[i]));
      #endif

      div_setting[i] = div_reference[divs_read];
      gen_switch = true;
      digitalWrite(div_leds[i], LOW);
    }
  }
}

void init_check(){
  
  byte tempL;
  byte tempR;
  if (digitalRead(SWITCH_0R_PLAY)  == LOW){
    //box was started with play in the on position
    //Serial.println("[i] Initializing... PLAY is ON ");
    play_switch = true;
    send_start = true;
    reset_switch = false;
    gen_switch = true;
  } else {
    //Serial.println("[i] Initializing... PLAY OFF ");
    play_switch = false;
    send_stop = true;
    reset_switch = false;
    gen_switch = false;
  }
  if (digitalRead(SWITCH_0L_RESET) == LOW){
    //Serial.println("[i] Initializing... PLAY OFF, RESET switch is ON ");
    //box was started with reset on 
    play_switch = false;
    send_stop = true;
    reset_switch = true;
    gen_switch = false;
  } else {
    reset_switch = false;
  }
}


void check_switches(){  
  byte local_arp_on = false;
  byte tempL;
  byte tempR;
  if (digitalRead(SWITCH_0R_PLAY)  == LOW){
    play_on_event();
  } else {
    play_off_event();
  }
  if (digitalRead(SWITCH_0L_RESET) == LOW){
    play_off_event();
    reset_event();
  } else {
      reset_switch = false;
  }
  //check the switches associated to the channels
  for(byte i=0;i<MAX_CHANNELS;i++){    
    tempL = digitalRead(clock_switches[i]);
    tempR = digitalRead(regen_switches[i]);
    if(tempL && tempR == HIGH) {
      if (regen[i] != ARP) {
        #ifdef SERIALVIS
        //Serial.println("[i] SET: chan: " + String(chanArray[i]) + " MUTATIONAL ARPEGGIATOR");
        if (GenBuf[i]->poly) {
          Serial.println("SET ARP,CHORD" + String(chanArray[i]) );
        } else {
          Serial.println("SET ARP,ARP," + String(chanArray[i]) );
        }
        #endif
        note_off_switch[i] = false; 
        regen_switch[i] = true;
        gen_switch = true;
        regen[i] = ARP;
      }      
      local_arp_on = true;      
    } else if (tempL == LOW){
      if (regen[i] != CLOCK ){ 
        #ifdef SERIALVIS
        //Serial.println("[i] SET: chan: " + String(chanArray[i]) + " MIDI CLOCK");
        Serial.println("SET ARP,clock," + String(chanArray[i]));
        #endif
        note_off_switch[i] = true; 
        regen_switch[i] = false;
        regen[i] = CLOCK; 
        digitalWrite(arp_leds[i], LOW);
        digitalWrite(div_leds[i], LOW);
      }
    } else if (tempR == LOW){
      if (regen[i] != FREEZE ){
        #ifdef SERIALVIS
        Serial.println("SET ARP,FREEZE," +String(chanArray[i]));
        #endif
        note_off_switch[i] = false;
        regen_switch[i] = false;
        gen_switch = false;
        regen[i] = FREEZE;
      }
      local_arp_on = true;  
    }    

    tempL = digitalRead(oct0_switches[i]);
    tempR = digitalRead(oct2_switches[i]);

    if(tempL && tempR == HIGH){
      if(octave[i] != 1){
        octave[i]=1;
        if (regen[i] != CLOCK && regen[i] != FREEZE) {
          #ifdef SERIALVIS
          //Serial.println("[i] SET: chan: " + String(chanArray[i]) + " oct: 2"); 
          Serial.println("SET OCT,1," +String(chanArray[i]));
          #endif
          regen_switch[i] = true;
          gen_switch = true;
        } 
      }

    } else if(tempL == LOW){
      if(octave[i] != 0){
        if (regen[i] != CLOCK && regen[i] != FREEZE) {
          #ifdef SERIALVIS
          //Serial.println("[i] SET: chan: " + String(chanArray[i]) + " oct: 1"); 
          Serial.println("SET OCT,0," + String(chanArray[i]));
          #endif

          regen_switch[i] = true;
          gen_switch = true; 
        }
        octave[i] = 0;
       }      

    } else if (tempR == LOW){
      if(octave[i] != DJENT) {  
        if (regen[i] != CLOCK && regen[i] != FREEZE) {
          #ifdef SERIALVIS
          //Serial.println("[i] SET: chan: " + String(chanArray[i]) + " oct: 3"); 
          Serial.println("SET OCT,2," + String(chanArray[i]));
          #endif
          regen_switch[i] = true;
          gen_switch = true;
        } 
        octave[i] = 2;
      }
    }
  }
  
  tempL = digitalRead(SWITCH_MODE_1);
  tempR = digitalRead(SWITCH_MODE_3);


  //Set a global mode for slightly different functionality
  if (tempL && tempR == HIGH){
    if (mode_setting != CHORD) {
      mode_setting = CHORD;
      gen_switch = true;
      #ifdef SERIALVIS
      //Serial.println("[i] SET: mode: 1");
      Serial.println("SET MODE,MODE 2 (CHORD),0");
      #endif

    }
  } else if (tempL == LOW) {
    if (mode_setting != NORMAL) {
      mode_setting = NORMAL;
      gen_switch = true;
      #ifdef SERIALVIS
      //Serial.println("[i] SET: mode: 0");
      Serial.println("SET MODE,MODE 1 (NORMAL),0");
      #endif

    }
    mode_setting = 0;
  } else if (tempR == LOW){
    if (mode_setting != REPEAT) {
      mode_setting = REPEAT;
      gen_switch = true;
      #ifdef SERIALVIS
      //Serial.println("[i] SET: mode: 2");
      Serial.println("SET MODE,MODE 3 (REPEATS),0");
      #endif
    }
  }

  //if only one is set to arppegiator then we need to capture notes
  if (local_arp_on){
    arp_on = true;
  } else {
    arp_on = false;
  }
}

void loop(){  
  check_pots();
  check_switches();
  bool msg =  MIDI.read();
  do_timed_midi(); //send midi events based on the clock pulse  
  #ifdef SNIFF  
  if (msg) {
    snif_midi_msg();
  }
  #endif
}


void snif_midi_msg(){
   //static uint8_t  ticks = 0; //for a tick counter for clock messages if needed
   //static uint8_t  old_ticks = 0;  
    switch (MIDI.getType()){
      case midi::NoteOff :
        {
            Serial.print("NoteOff, chan: ");
            Serial.print(MIDI.getChannel());
            Serial.print(" Note#: ");
            Serial.print(MIDI.getData1());
            Serial.print(" Vel#: ");
            Serial.println(MIDI.getData2());  
        }
        break;
      case midi::NoteOn :
        {
            Serial.print("NoteOn, chan: ");
            Serial.print(MIDI.getChannel());
            Serial.print(" Note#: ");
            Serial.print(MIDI.getData1());
            Serial.print(" Vel#: ");
            Serial.println(MIDI.getData2());
        break;
      case midi::AfterTouchPoly :
        {
          Serial.print("PolyAT, chan: ");
          Serial.print(MIDI.getChannel());
          Serial.print(" Note#: ");
          Serial.print(MIDI.getData1());
          Serial.print(" AT: ");
          Serial.println(MIDI.getData2());
        }
        break;
      case midi::ControlChange :
        {
          Serial.print("Controller, chan: ");
          Serial.print(MIDI.getChannel());
          Serial.print(" Controller#: ");
          Serial.print(MIDI.getData1());
          Serial.print(" Value: ");
          Serial.println(MIDI.getData2());
        }
        break;
      case midi::ProgramChange :
        {
          Serial.print("ProgChange, chan: ");
          Serial.print(MIDI.getChannel());
          Serial.print(" program: ");
          Serial.println(MIDI.getData1());
        }
        break;
      case midi::AfterTouchChannel :
        {
          Serial.print("ChanAT, chan: ");
          Serial.print(MIDI.getChannel());
          Serial.print(" program: ");
          Serial.println(MIDI.getData1());

        }
        break;
      case midi::PitchBend :
        {
          uint16_t val;
          Serial.print("Bend, chan: ");
          Serial.print(MIDI.getChannel());
          // concatenate MSB,LSB
          // LSB is Data1
          val = MIDI.getData2() << 7 | MIDI.getData1();
          Serial.print(" value: 0x");
          Serial.println(val, HEX);
        }
        break;
      case midi::SystemExclusive :
        {
          // Sysex is special.
          // could contain very long data...
          // the data bytes form the length of the message,
          // with data contained in array member
          uint16_t length;
          const uint8_t  * data_p;
          Serial.print("SysEx, chan: ");
          Serial.print(MIDI.getChannel());
          length = MIDI.getSysExArrayLength();
          Serial.print(" Data: 0x");
          data_p = MIDI.getSysExArray();
          for (uint16_t idx = 0; idx < length; idx++)
          {
            Serial.print(data_p[idx], HEX);
            Serial.print(" 0x");
          }
          Serial.println();
        }
        break;
      case midi::TimeCodeQuarterFrame :
        {
          // MTC is also special...
          // 1 byte of data carries 3 bits of field info 
          //      and 4 bits of data (sent as MS and LS nybbles)
          // It takes 2 messages to send each TC field,
          Serial.print("TC 1/4Frame, type: ");
          Serial.print(MIDI.getData1() >> 4);
          Serial.print("Data nybble: ");
          Serial.println(MIDI.getData1() & 0x0f);
        }
        break;
      case midi::SongPosition :
        {
          // Data is the number of elapsed sixteenth notes into the song, set as 
          // 7 seven-bit values, LSB, then MSB.
          Serial.print("SongPosition ");
          Serial.println(MIDI.getData2() << 7 | MIDI.getData1());
        }
        break;
      case midi::SongSelect :
        {
          Serial.print("SongSelect ");
          Serial.println(MIDI.getData1());
        }
        break;
      case midi::TuneRequest :
        {
          Serial.println("Tune Request");
        }
        break;
      case midi::Clock :
        {
          //ticks++;
          //Serial.print("Clock ");
          //Serial.println(ticks);
        }
        break;
      case midi::Start :
        {
          //ticks = 0;
          Serial.println("Starting");
        }
        break;
      case midi::Continue :
        {
          //ticks = old_ticks;
          Serial.println("continuing");
        }
        break;
      case midi::Stop :
        {            
          //old_ticks = ticks;
          Serial.println("Stopping");
        }
        break;
      case midi::ActiveSensing :
        {
          Serial.println("ActiveSense");
        }
        break;
      case midi::SystemReset :
        {
          Serial.println("SYSTEM RESET");
        }
        break;
      case midi::InvalidType :
        {
          Serial.println("Invalid Type");
        }
        break;
      default:
        {
          Serial.println();
        }
        break;
    }
  }
}