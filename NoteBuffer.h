#define BUFFER_SIZE 12
#define MAX_SEQUENCE_SIZE 96


class MidiNote {
  public:
    byte chan;
    byte pitch;
    byte vel;
    bool on; //flag for if the noteOn has been sent
    MidiNote(byte chan, byte pitch, byte vel, bool _on);
    MidiNote();
    void set(byte chan, byte pitch, byte vel, bool _on);
};

MidiNote::MidiNote(){
  chan = 0;
  pitch = 0;
  vel = 0;
  on = false;
}

MidiNote::MidiNote(byte _chan, byte _pitch, byte _vel, bool _on) : chan(_chan), pitch(_pitch), vel(_vel), on(_on) {
}

void MidiNote::set(byte _chan, byte _pitch, byte _vel, bool _on) {
  chan = _chan; 
  pitch = _pitch; 
  vel = _vel; 
  on= _on;
}


//TODO parent and child classes for different types of buffers
class NoteBuffer {
  public:
    bool poly; //is this a buffer for a polysynth? (false for monosynth)
    
    NoteBuffer(byte _num_notes, byte channel, bool poly);
    void add(MidiNote note);
    MidiNote getNote(byte i);
    MidiNote getNote(byte i, bool full_buffer);
    MidiNote getCurNote();
    MidiNote getPrevNote();
    MidiNote getPreviNote(byte i);
    MidiNote getAbsPrevNote();
    byte getCurNoteNum();
    byte getMaxNoteNum();
    void increment();
    void setRoot();
    void setNotePitch(byte i, byte pitch);
    void setNoteVel(byte i, byte vel);
    byte getNoteVel(byte i);
    byte getRoot();
    byte getBars(); 
    void setRoot(byte pitch);
    void restart();
    void setNoteOff(byte index);
    void setCurNoteOn();
    void setCurNoteOff();
    void setPrevNoteOff();
    void startNewSet();
    void resetAll();
    void debug();
    void shuffle();
    void replace(MidiNote note);
    void replace(MidiNote note, byte index);
    void swapNotes(byte index1, byte index2);
   
    void setVelDrift(byte setting); //by default there will be a low drift. setting means to set it to a high drift
    void setOctDrift(bool setting);

  private:
    byte channel;
    byte curr_note_num;
    byte write_index; //place to insert new notes and expand the current selection of notes
    byte read_offset; //ofset of current
    byte cur_pos; //start position in the circular buffer
    byte max_note_num; //allows buffer size smaller than the max GENERATE_SIZE
    byte type; //0: small buffer, 1: large buffer
    byte bars; 
    byte root_note; //the pitch of the lowest note of the active set of notes in the buffer
    
    //automate variation over time
    // as notes are added to the buffer we can add automations to create variation and a natural feel
    int vel_drift;
    byte dr_ampl; //the amount the velocity should drift over the buffer
    byte dr_speed; //the speed at which it drifts
    byte dr_base; 
    byte oct_drift;
    bool oct_setting;

    MidiNote buffer[MAX_SEQUENCE_SIZE];
};


//initialize the buffer with max number of notes
NoteBuffer::NoteBuffer(byte _type, byte _channel, bool _poly) 
  : type(_type), write_index(0), read_offset(0), cur_pos(0), channel(_channel), poly(_poly), bars(0) {
  if (_type == 0){
    max_note_num = BUFFER_SIZE; //set the maximum for the cicular buffer
  } else { 
    max_note_num = MAX_SEQUENCE_SIZE;
  } 

  vel_drift = 0;
  oct_drift = 0;
  
  setVelDrift(0);
  setOctDrift(true);
}

//add a note to the buffer, return the index
//circular buffer means we overwrite when it gets to the max
void NoteBuffer::add(MidiNote note){
  byte pos = write_index  % max_note_num; 
  buffer[pos].pitch = note.pitch;
  //do midi data validation here
  if (note.pitch >= 127) { 
    buffer[pos].pitch = 127; //can just act as a rest 
  }
  if (note.pitch <= 0) {
    buffer[pos].pitch = 0; //to denote a rest
  }
  
  if (note.vel >= 127) {
    buffer[pos].vel = 126;
  }
  
  buffer[pos].vel = note.vel;
  
  if (note.vel <= 0){
    buffer[pos].vel = 0; //also a rest
  } else {
    buffer[pos].vel = random(dr_base,dr_base+5) + dr_ampl*sin(vel_drift/dr_speed);
    vel_drift++;
    //Serial.println("VEL DRIFT: " + String(buffer[pos].vel) + " amp " + String(dr_ampl) + " speed " + String(dr_speed));
  }
  

  buffer[pos].chan = channel; //the channel assigned to the current buffer
  buffer[pos].on = false; //this note has not been sent
  write_index = ((write_index + 1) % max_note_num); 
  curr_note_num++;
  //oct_drift = (oct_drift + 1) % 3;
  //Serial.println("OCT DRIFT " + String(oct_drift));
  #ifdef NOTEDEBUG
  Serial.println("[D] ADDING note " + String(note.pitch) + " to pos: " + String(pos) + " num notes: "+ String(curr_note_num));
  #endif
  if (curr_note_num >= max_note_num){
    curr_note_num = max_note_num; //active notes are the whole buffer when full
  }
}

//replace the note in the current position 
void NoteBuffer::replace(MidiNote note){
  byte pos = (cur_pos + read_offset) % max_note_num;
  buffer[pos].pitch = note.pitch;
  buffer[pos].vel = note.vel;
  buffer[pos].chan = channel;
  buffer[pos].on = note.on;
  
} 

//replace the note in the current position 
void NoteBuffer::replace(MidiNote note, byte i){
  byte pos = (cur_pos + (i % curr_note_num)) % max_note_num;
  buffer[pos].pitch = note.pitch;
  buffer[pos].vel = note.vel;
  buffer[pos].chan = channel;
  buffer[pos].on = note.on;
 // Serial.println("REPLACE: cur_pos: " + String(cur_pos) + " read_offset: " + String(read_offset) + " curr note num: " + String(curr_note_num));
} 

void NoteBuffer::swapNotes(byte i, byte j){
  byte pos1 = (cur_pos + (i % curr_note_num)) % max_note_num;
  byte pos2 = (cur_pos + (j % curr_note_num)) % max_note_num;
  
  MidiNote tempNote;
  tempNote = buffer[pos1];
  //Serial.println(": " + String(buffer[cur_pos + (i % curr_note_num) ].pitch) + ": " + String(buffer[cur_pos + (1 % curr_note_num)].pitch)+ ": " + String(buffer[cur_pos + (2 % curr_note_num)].pitch)+ ": " + String(buffer[cur_pos + (3 % curr_note_num)].pitch));
  //Serial.println("SWAP : " + String(pos1) + " , " + String(pos2) + " p1: " + String(tempNote.pitch)+ " p2: " + String(buffer[pos2].pitch));
  replace(buffer[pos2], i);
  //Serial.println(" 0 1: " + String(buffer[cur_pos + (i % curr_note_num) ].pitch) + " 1: " + String(buffer[cur_pos + (1 % curr_note_num)].pitch)+ " 2: " + String(buffer[cur_pos + (2 % curr_note_num)].pitch)+ " 3: " + String(buffer[cur_pos + (3 % curr_note_num)].pitch));
  replace(tempNote, j);
  //Serial.println(" 0 2: " + String(buffer[cur_pos + (i % curr_note_num) ].pitch) + " 1: " + String(buffer[cur_pos + (1 % curr_note_num)].pitch)+ " 2: " + String(buffer[cur_pos + (2 % curr_note_num)].pitch)+ " 3: " + String(buffer[cur_pos + (3 % curr_note_num)].pitch));
}

byte NoteBuffer::getCurNoteNum(){
  return curr_note_num;
}

byte NoteBuffer::getMaxNoteNum(){
  return max_note_num;
}

byte NoteBuffer::getRoot(){ 
  return root_note;
}

byte NoteBuffer::getBars(){ 
  return bars;
}

void NoteBuffer::setVelDrift(byte setting){
  //Note greater than 50 amplitude could result in bad midi values
  if(setting == 3) { //fast and high
    dr_base = 60;
    dr_ampl = 50; //the amount the velocity should drift 
    dr_speed = 6;
  } else if (setting == 2) {
    dr_base = 45;
    dr_ampl = 30; //the amount the velocity should drift over the buffer
    dr_speed = 8;
  } else if (setting == 1) {
    dr_base = 30;
    dr_ampl = 15; //the amount the velocity should drift over the buffer
    dr_speed = 10;
  } else {
    dr_base = 20;
    dr_ampl = 7; //the amount the velocity should drift over the buffer
    dr_speed = 12;
  }
}

void NoteBuffer::setOctDrift(bool setting) {

}

//used to steal the root note from the capture buffer
void NoteBuffer::setRoot(byte pitch){
  root_note = pitch;
}

void NoteBuffer::setNotePitch(byte i, byte pitch){
  byte pos = (cur_pos + (i % curr_note_num)) % max_note_num;
  buffer[pos].pitch = pitch;
}

void NoteBuffer::setNoteVel(byte i, byte vel){
  byte pos = (cur_pos + (i % curr_note_num)) % max_note_num;
  buffer[pos].vel = vel;
}

byte NoteBuffer::getNoteVel(byte i){
  byte pos = (cur_pos + (i % curr_note_num)) % max_note_num;
  return buffer[pos].vel;
}


//TODO capture buffer should calculate it's own root when it is being created
void NoteBuffer::setRoot(){
  if (curr_note_num == 0){
    return;
  }
  root_note = 127;
  for (byte i=0; i< curr_note_num; i++) {
    byte pos = (cur_pos + i) % max_note_num;
    if ((buffer[pos].pitch >= 21 ) && (buffer[pos].pitch < root_note)) { //21 is the lowest audible note, anything lower is likey 0 (initial val, a rest) or a mistake
      root_note = buffer[pos].pitch;
    }
  }
  if (root_note == 127)
    root_note = 0;
}

MidiNote NoteBuffer::getNote(byte i){
 return getNote(i, false);
}

//returns the ith note from currently active set of notes in the buffer, does not alter the buffer
MidiNote NoteBuffer::getNote(byte i, bool full_buffer){
  byte pos;
  if(full_buffer) { 
    pos = (cur_pos + i) % max_note_num;
  } else {
    pos = (cur_pos + (i % curr_note_num)) % max_note_num;
  }
  //Serial.println("GET NOTE: " + String(buffer[pos].pitch) + " pos: " + String(pos) + " cur_pos: " + String(cur_pos) + " i: " + String(i) + " cur_note_num: " + String(curr_note_num) + " pos % c: " +String (((byte) pos % curr_note_num)));
  MidiNote new_note(buffer[pos].chan, buffer[pos].pitch, buffer[pos].vel, buffer[pos].on);
  return new_note;
}
//gets a new copy of the current note in the buffer, relative to cur_pos and increments read_offset
MidiNote NoteBuffer::getCurNote(){
  byte pos = (cur_pos + read_offset) % max_note_num;
  //Serial.println("getCurNote: Chan" + String(channel) + " pitch: "+ String(buffer[pos].pitch) + " pos " + String(pos) + " cur_pos " + String(cur_pos) + " read_offset " + String(read_offset) + " cur_note_num " + String(curr_note_num));
  MidiNote new_note(buffer[pos].chan, buffer[pos].pitch, buffer[pos].vel, buffer[pos].on);
  return new_note;
}

void NoteBuffer::increment(){
  read_offset = ((read_offset + 1) % curr_note_num) % max_note_num;
  if (read_offset == 0) {    
    bars = bars + 1;
  }
 // Serial.println("INC: cur_pos: " + String(cur_pos) + " read_offset: " + String(read_offset) + " curr note num: " + String(curr_note_num));
}

//gets the previous note (one behind read_offset, relative to cur_pos) 
MidiNote NoteBuffer::getPrevNote(){ 
  byte pos = (cur_pos + ((read_offset + curr_note_num - 1) % curr_note_num)) % max_note_num;
  //Serial.println("PREV positions: " + String(buffer[pos].pitch) + " pos " + String(pos) + " cur_pos " + String(cur_pos) + " read_offset " + String(read_offset) + " cur_note_num " + String(curr_note_num) + " " +String ((read_offset - 1) % curr_note_num));
  MidiNote new_note(buffer[pos].chan, buffer[pos].pitch, buffer[pos].vel, buffer[pos].on);
  return new_note;
}

//gets the previous note (one behind read_offset, relative to cur_pos) 
MidiNote NoteBuffer::getPreviNote(byte i){ 
  byte pos = (cur_pos + ((read_offset + curr_note_num - i) % curr_note_num)) % max_note_num;
  //Serial.println("PREV positions: " + String(buffer[pos].pitch) + " pos " + String(pos) + " cur_pos " + String(cur_pos) + " read_offset " + String(read_offset) + " cur_note_num " + String(curr_note_num) + " " +String ((read_offset - 1) % curr_note_num));
  MidiNote new_note(buffer[pos].chan, buffer[pos].pitch, buffer[pos].vel, buffer[pos].on);
  return new_note;
}


//gets the previous note (one behind read_offset, relative to cur_pos) 
MidiNote NoteBuffer::getAbsPrevNote(){ 
  byte pos = cur_pos + (read_offset - 1) % max_note_num;
  //Serial.println("PREV positions: " + String(buffer[pos].pitch) + " pos " + String(pos) + " cur_pos " + String(cur_pos) + " read_offset " + String(read_offset) + " cur_note_num " + String(curr_note_num) + " " +String ((read_offset - 1) % curr_note_num));
  MidiNote new_note(buffer[pos].chan, buffer[pos].pitch, buffer[pos].vel, buffer[pos].on);
  return new_note;
}

//sets the note at the current read position to on
void NoteBuffer::setCurNoteOn(){
  byte pos = (cur_pos + read_offset) % max_note_num;
  buffer[pos].on = true;
  //Serial.println("[D] - CurrNoteOn. Note " + String(buffer[pos].pitch) + " Set to on? " + String(buffer[pos].on));
}

//sets the note at the current read position to off
void NoteBuffer::setCurNoteOff(){
  byte pos = (cur_pos + read_offset) % max_note_num;
  buffer[pos].on = false;
  //Serial.println("[D] - CurrNoteOff. Note " + String(buffer[pos].pitch) + " Set to on? " + String(buffer[pos].on));
}

//sets the note at the ith position to off - for ensuring all notes in buffer will be turned off
// regardless of active set (for correcting message interruptions and notes kept on)
void NoteBuffer::setNoteOff(byte i){
  byte pos = cur_pos + i;
  pos = pos % max_note_num; //TODO test this change from curr_note_num
  buffer[pos].on = false;
}

//sets the note at the previoius read position to off
void NoteBuffer::setPrevNoteOff(){
  byte real_pos = (cur_pos + ((read_offset + curr_note_num - 1) % curr_note_num)) % max_note_num;
  buffer[real_pos].on = false;
}

//set the counters to the beginning of the buffer
void NoteBuffer::resetAll(){
  read_offset = 0;
  write_index = 0;
  curr_note_num = 0;
  root_note = 0;
  cur_pos = 0;
  //TODO just for debug
  for (byte i =0; i< max_note_num; i++){
    buffer[i].pitch = 0;
    buffer[i].vel = 0;
    buffer[i].chan = channel;
    buffer[i].on = false;
  }
}

//restart the position of the read head to the current position 
// - the read index need only be set to 0
void NoteBuffer::restart(){
  read_offset = 0;
#ifdef PLAYDEBUG
  Serial.println("[D] RESTART type " + String(type) + " write idx: " + String(write_index) + " cur_pos: " + String(cur_pos) + " read offset: " + String(read_offset) + " cur num: " + String(curr_note_num) + " root: " + String(root_note));
#endif 
}

void NoteBuffer::startNewSet(){
  //Serial.println("[D] Start new set");
  curr_note_num = 0;
  read_offset = 0;
  cur_pos = write_index;
  root_note = 0;
}

void NoteBuffer::shuffle(){
  MidiNote temp_note;
  byte j;
  byte pos_j;
  byte pos_i;

  if (curr_note_num <= 2) { //not much to shuffle
    return;
  }
  for (int i=0; i < curr_note_num; i++) {
    pos_i = (cur_pos + i) % max_note_num;
    j = random(0, curr_note_num);  // a random index within the range of the active note set
    pos_j = (cur_pos + j) % max_note_num;
    temp_note = buffer[pos_j];
    buffer[pos_j] =  buffer[pos_i];
    buffer[pos_i] = temp_note;
  }
}


void NoteBuffer::debug(){
  MidiNote new_note;
  Serial.println("[D] type " + String(type) + " MAX: " + String(max_note_num) + " write idx: " + String(write_index) + " cur_pos: " + String(cur_pos) + " read offset: " + String(read_offset) + " cur num: " + String(curr_note_num) + " root: " + String(root_note));
  for (byte i=0; i< max_note_num; i++) {
    new_note = buffer[i];
    Serial.print("[D-" + String(i) + "] ");
    if (i == write_index) {
      Serial.print("=w==> ");
    }
    if (i == cur_pos) {
      Serial.print("~c~~> ");
    }
    if (i == cur_pos + read_offset){
      Serial.print("-r--> ");
    }
    Serial.println(" pitch: " + String(new_note.pitch) + " vel: " + String(new_note.vel) + " chan: " + String(new_note.chan) + " on:" + String(new_note.on));
  }
}
