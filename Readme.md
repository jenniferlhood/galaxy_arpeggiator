# GALAXY ARPEGGIATOR
## Generative Arduino MIDI controller


![Galaxy Arpeggiator current build](galaxy.jpg)

 Galaxy Arpeggiator is a midi controller intended to be used with multiple hardware or software synths simultaneously. Up to four different arpeggio patterns (different by note sequence and rhythm) can sent to to different devices via midi channel. Arpeggio patterns are not fully random, they are chosent from a seqence of notes delivered to the controller via an external keybed that sends midi note information. 
 
 This midi controller cannot generate music on it's own. Unlike other arpeggiators where you control the melodic information by chosing the key, mode, intervals or chord type of the created arpeggio, I wanted a controller that created arpeggios from notes that I am playing. I wanted an experience that was more like a dynamic instrument rather than a programming interface. I want no menu diving. All functionally should be avaialble from a button, dial or switch, much like an actual playable instrument.

The Galaxy arepeggator can be used to create soundscapes, evolving melodic lines, harmonies and baselines, playable simultaneously from the input notes. 

This gear description is Arduino based, I used the Arduino Mega for its larger selection of input pins. The code in this repository is to be run on on the Arduino, and it is not complete. I am changing functionality as I use the instrument, and updating it for my personal compositional taste and style and adjusting it to suit my musical purposes. My goal isn't to make it more generalizable or suitable for any other usecase than my own.

The midi in this device follows the MIDI Associations' [MIDI 1.0 Core Specification](https://drive.google.com/file/d/1ewRrvMEFRPlKon6nfSCxqnTMEu70sz0c/view) which include full schematics and parts list. Step by step guides for building circuits for MIDI input, output and through can be found on the [Notes and Volts blog](https://www.notesandvolts.com/2012/01/fun-with-arduino-midi-input-basics.html) which includes detailed video tutorials.

## Examples of use

[Early prototype in a dollar store box](https://studio.youtube.com/video/bu5mDEMsVZg/edit) controlling 3 different synths. The composition has two different melodies and pads (the base line is not driven by the arpeggiator). The video has annotations for what the arpeggator is doing to modify notes and rhythms.

[Galaxy Arpeggiator in wooden housing](https://www.youtube.com/watch?v=z5hB5zzREvU) creating melody, baseline and pad patterns from input notes


## General functionality


- Play notes on an external keyboard and the arpeggiator will create patterns and send them to the recieving devices. The input notes are captured when the first note is pressed (first midi note on message) until the last note is released (midi note off message) - it's intended to capture chords but allows for entering more notes than three or four. The input buffer can currently caputre up to 12 notes. If play is on the the arpeggiator doesn't wait for all notes to be captured, and starts generating from the notes it has in the input buffer. Each new note capture results in a new arpeggio being generated and if the inputs notes are released the buffer is cleared
- Arpeggios are generated based on the parameters set by each channel's two potentiometers and two switches. The parameters can be modified during play for interesting effects without any new notes being added to the capture


## Interface Layout
```

Top view
_____________________	
 |                 |
 |     O  O  O  O  |
 |  O  O  O  O  O  |
 |  -  -  -  -  -  |
 |  -  -  -  -  -  |
 |_________________|

O : potentiometer
- : three-way (SPDT) switches


Back view
_______________________
|                     |
|  0  0  0  0  0  X   |
|_____________________|

0 : 5-pin midi DIN
X : hole for Arduino USB connection
```

The physical midi controller is a wooden box with control components on the top and inputs/outputs on the back. I used 6" cedar planks for the sides and back, and clear acrylic sheet (1/8") for the face and bottom because it seemed like a cool idea to be able to peer inside at the DIY electrical components.

The controls on the top are organized into 5 columns. From left to right, the 
First colum is a general colum. The Potentiometer is used for tempo control, the two switches are, top to bottom: reset /stop / play and mode 1/ mode 2 / mode 3.
The following columns are areggiator parameter controls for the four channels.

MIDI channel numbers that correspond to the four outputs are configurable in the header file, but currently compiled in and cannot be changed at run time. By default they are set to channels 1,2,3,4 in order.

## Parameter description: Potentiometers
### Global tempo and MIDI clock
 - Tempo ranges from 20 to 300 bpm with a resolution of 4 bpm
 - Tempo LED flashes each quarter note
 - Sends a MIDI clock pulse at 4 PPQN
 - Side note: I implement a custom MIDI CC (cc# 94) that sends the tempo value directly to recieving equipment (who need to implement it). For me this produces more accurate synchronization than calculting tempo from clock pulses as I don't need to account for latency issues (a bit of a limitation of using the Arduino platform). Using 4 bpm resolution gives me 2 extra bits to use for higher tempo values: I can compress a tempo range from 0-510 bpm to a range of 0-127
  - Settings for tempo and synchronization can be configured in project header

### MIDI Channel pots

- The top potentiometer for each channel is the algorithm select. Currently there are 12 algorithms that move from simple to complex as the dial is turned up. The simplest algorithm at setting zero is "root note" where the pattern is simply the lowest note of the input chord. Turning up the dial moves through some standard patterns (in order, reversed, ascending/descending) after about half way on the dial, patterns become randomized and will slowly mutate as they are played. Also about half way on the dial, dynamics are modified (by velocity value) and become more exaggerated (note not all synths implement MIDI velocity).

- The bottom potentiometer is the divisions toggle, which effects the time divisions of the notes in the arpeggio. Setting of zero represent whole notes (assuming 4/4 time), but this depends on the synth clock settings. Currently the controller operations on a 4 PPQN (four pulses per quarter note) system, so the slowest division emits a note after every 16 MIDI clock pulses.
Time division settings cab be changed by modifying the defintions in the header file.  Turning up the knob converts the pattern into faster rhythms. Currently not all patterns are even time divisions and, you will observe irregular divisions for some division values. If multiple synths are connected to the galaxy arpeggiator then polyrhythms can be created by using different division settings for different channels


## Parameter description: Switches
### Global switches 

#### Playback control

- The top switch in the global column is a toggle for playing or stopping the notes from being sent.
- Left: Reset, Middle: Stop, Right: Play
- When the switch is set to play, the argeggiator sends out a MIDI play message (which could trigger connected devices to play if they have sequencers or arpeggiators. These should be turned off in the devices settings if this is not desired behaviour). The arpeggator will begin its internal timer and start sending out midi clock pulses, so even though notes are not yet being sent, connected equipment can be synchronized if they respond to MIDI clock. When MIDI notes are sent to the Galaxy Arpeggiator by connected keyboard, the arpeggiator begins to generate note sequences and send them out to attached devices.
- When the switch is in the Middle/ stop position, a MIDI Stop message is sent, and notes from the generated arpeggios are no longer sent. MIDI clock is no longer sent. The arpeggio maintains it's generated buffer and position in that buffer, so if switched back into play, the arpeggio sequence continues, however arps start from position 0, not from where they left off.
- When the switch is put in Reset mode, several MIDI messages are sent: Stop, SystemReset, AllSoundOff to the global channel message is sent. This is because there are different uses of these MIDI messages by different equipment.  All note buffers are cleared, and new notes must be entered next time the unit is switched to Play.

#### Mode Control

Top switch in the first column toggles the mode for the arpeggiator. MOdes have the effect of modifying global functionaly, meaning they will change how all the other channels operate. Currently, this only imacts the functionaly of the "regen" or "alt arp" switch but it's possible that I could want this to have wider impact (alter random factors, change the range of octaves chosen or even which arpeggio patterns are avaialble)


- left is mode 1: Normal mode
	- The "regen" or "alt arp" pattern is DJENT MODE
- middle is mode 2:
	- polyphonic mode is switched on for channels designated with this mode
- right is mode 3:
	- The "regen" or "alt arp" pattern is FREEZE MODE
	- if a channel has an arp pattern created from djent alg, (and it's alg switch is in djent mode), it will be frozen in djent mode and pattern will not resume until mode is switched to 2 or 1


#### Polyphonic mode

- This function is still being developed. However, the idea is that for polyphonic synths, it would be nice to create evolving chords from the generated arp sequence.
By default the Galaxy Arpeggiator designates channels 3 and 4 as "polyphonic" meaning, that polyphonic mode will be used for the mode toggle switch for these channels.

- For modes 2 and 3 (or just mode 3, TBD) an arpeggio will send out two notes at the same time instead of just one. I know this isn't really a full chord, but has the effect of implying a chord, espeically if other channels are sending out different notes at the same time. The two notes are chosen from the generated sequence. Currently, it is not more sophisticated than chosing the current note to be sent a second note a some n notes from the current note where n is a value between {1, nummber of notes in the generated buffer}
 When sequences get randomized by the Arp parameter, this will end up producing different types of intervals.

### MIDI Channel switches

- The switches under columns 2-5 affect the arpeggios sent to the individual columns.

#### Arpeggiator control

- The top swich controls the mode for the individual arpeggio. In the left-most switch position, the MIDI channel is set to "clock". This means a MIDI device listening on the corresponding channel will receive only a MIDI clock pulse. This lets the galaxy arpeggiator act as synchronization unit other equipment like drum machines or guitar effect pedals that accept MIDI clock (Pedals from Chase Bliss or Old Blood Noise Endeavers for example). Entering clock mode while playback is on also has the effect of "muting" a synth that was receiving MIDI notes, so this mode can be used musically as well.
 
 - In the middle switch position, the channel is set to Arp, the main generative algorithm. When a channel is in Arp mode and set to play, the MIDI controller sends out MIDI notes as determined by the algorithm set by the top potentiometer.
 
 - In the right switch position, the channel is set to "alt arp" or "regen" whose function changes depending on the global mode setting. 

 - Currently, the regen mode can be 1 of two modes to the generated arpeggio: "djent" or "freeze" mode. When in global modes 1 and 2, channels alt arp function is djent mode. Djent mode introduces random rest notes into the arpeggio pattern, adding more rhythmic variety. Global mode 3 changes the channel alt mode to freeze mode. Freeze mode freezes the current pattern, even if new notes are entered or a regeneration cycle occurs that might otherwise mutate the pattern. With one channel in freeze mode, and another not, new notes generated arpeggios have the imact of creating generative harmonies. The division pot and tempo still affects arpeggios in freeze mode.
 When freeze mode is exited, either by changing the channe switch back to arps or changing the mode switch, an arpeggio from the currently captured input notes will be created (aka the pattern is no longer "frozen")
 

#### Octave control
- The bottom switch for each channel determine how many octaves to use in generated arpeggios. From left: 0, or no additional octaves than the received via MIDI input. Midde: add an additional ocatve - determined by input notes. High notes result in lower octaves generated, low notes result in higher octaves. Right: 2 additional octaves are added. The point at which Galaxy Arpeggiator decides to add higher or lower octaves is configurable in the project header.


