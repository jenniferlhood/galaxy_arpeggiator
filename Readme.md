# GALAXY ARPEGGIATOR
## Generative Arduino MIDI controller

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
- Arpeggios are generated based on the parameters set by each channel's two potentiometers and two switches. The parameters can be modified during play for intersting effects without any new notes being added to the capture


## Interface Layout
```

Top view
____________________	
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

The physical midi controller is a wooden box with control components on the top and inputs/outputs on the back. I used 6" cedar planks for the sides and back, and clear acrylic sheet (1/8") for the face and bottom because it seemed like a cool idea to be able to peer inside at the DIY electrical compoonents.

The controls on the top are organized into 5 columns. From left to right, the 
First colum is a general colum. The Potentiometer is used for Tempo control, the two switches are, top to bottom: reset /stop / play and mode 1/ mode 2 / mode 3.
The following colums are areggiator parameter controls for the four channels.

MIDI channel numbers that correspond to the four outputs are configurable in the header file, but currently compiled in and cannot be changed at run time. By default they are set to channels 1,2,3,4 in order.

## Parameter description: Potentiometers
#### Global tempo and MIDI clock
 - Tempo ranges from 20 to 300 bpm with a resolution of 4 bpm
 - Tempo LED flashes each quarter note
 - Sends a MIDI clock pulse at 4 PPQN
 - Side note: I implement a custom MIDI CC (cc# 94) that sends the tempo value directly to recieving equipment (who need to implement it). For me this produces more accurate synchronization than calculting tempo from clock pulses as I don't need to account for latency issues (a bit of a limitation of using the Arduino platform). Using 4 bpm resolution gives me 2 extra bits to use for higher tempo values: I can compress a tempo range from 0-510 bpm to a range of 0-127
  - Settings for tempo and synchronization can be configured in project header

#### MIDI Channels 

- The top potentiometer for each channel is the algorithm select. Currently there are 12 algorithms that move from simple to complex as the dial is turned up. The simplest algorithm at setting zero is "root note" where the pattern is simply the lowest note of the input chord. Turning up the dial moves through some standard patterns (in order, reversed, ascending/descending) after about half way on the dial, patterns become randomized and will slowly mutate as they are played. Also about half way on the dial, dynamics are modified (by velocity value) and become more exaggerated (note not all synths implement MIDI velocity).

- The bottom potentiometer is the divisions toggle, which effects the time divisions of the notes in the arpeggio. Setting of zero represent whole notes (assuming 4/4 time), but this depends on the synth clock settings. Currently the controller operations on a 4 PPQN (four pulses per quarter note) system, so the slowest division emits a note after every 16 MIDI clock pulses.
but could be changed by modifying the defintions in the header file. The tempo  and represents the slowest rate that notes are played per channel. Turning up the knob converts the pattern into faster rhythms. Currently not all patterns are even time divisions and, you will observe irregular divisions for some pot values. If multiple synths are connected to the regen box then polyrhythms can be created by using different division settings for different channels


### Parameter description: Switches

- The top switch is a toggle for arpeggiator algorithm (or alg). The three options are: left:clock, middle: stardard arp or right: alt function arp. The effect of the "alt function" or sometimes called "regen" setting depends on what mode the box is operating in. Mode settings are global settings, and the modes are described in the "globals" section. Modes are not fully implemented in my current version

- In the left-most position, the channel is set to "clock". This means a MIDI device listening on the corresponding channel will receive only a MIDI clock pulse. This is useful for synchronizing other equipment like drum machines or just other synths that you don't want receiving midi note information. In the middle position, the channel is set to Arp, the main generative algorithm. When in Arp, and set to play, the MIDI controller sends out midi notes as determined by the algorithm set by the top potentiometer. The right most is "regen" setting. When global mode is set to 0 (left), then this regen mode is a "djent" mode. The algorithm has more randomness at every setting and rests are introduced. This can give patterns a different rhythmic character. When mode is set to 1, this "regen" functions as a "freeze" setting, freeing the current algorithm setting in place. Changes to the potentiometer or notes sent to midi in will not change what notes are set to the channel while freeze is switched on. However, switching off freeze or changing the mode means the pattern will regenerate to the current pattern in the buffer and adopt the settings from the algorithm potentiometer. The division pot still influences patterns when in freeze mode. The idea is to freeze the notes of the arpeggio, so you can send different arpeggios to different instruments with the same controller (something you might ordinarily have to use different input devices to accomplish).

Top switch - playback

- left is reset
  - clears the generative note buffer

- middle is stop
  - stop playback but does not clear note buffer

- right is Play
  - starts master clock and sends MIDI play message

Note:
- when starting from stopped, all generated sequences start from position zero. This is in contrast to the per-channel clock/play switch behaviour



## Algorithms, Djent mode and Random



## Mode switch

Top switch in the first column toggles the mode for the arpeggiator. MOdes have the effect of modifying global functionaly, meaning they will change how all the other channels operate. Currently, this only imacts the functionaly of the "regen" or "alt arp" switch but it's possible that I could want this to have wider impact (alter random factors, change the range of octaves chosen or even which arpeggio patterns are avaialble)


- left is mode 1: Normal mode
	- The "regen" or "alt arp" pattern is DJENT MODE
- middle is mode 2:
	- polyphonic mode is switched on for channels designated with this mode
- right is mode 3:
	- The "regen" or "alt arp" pattern is FREEZE MODE
	- if a channel has an arp pattern created from djent alg, (and it's alg switch is in djent mode), it will be frozen in djent mode and pattern will not resume until mode is switched to 2 or 1



## Polyphonic mode

This function is still being developed. However, the idea is that for polyphonic synths, it would be nice to create evolving chords from the generated arp sequence.
By default the Galaxy Arpeggiator designates channels 3 and 4 as "polyphonic" meaning, that polyphonic mode will be used for the mode toggle switch for these channels.

For modes 2 and 3 (or just mode 3, TBD) an arpeggio will send out two notes at the same time instead of just one. I know this isn't really a full chord, but has the effect of implying a chord, espeically if other channels are sending out different notes at the same time. The two notes are chosen from the generated sequence. Currently, it is not more sophisticated than chosing the current note to be sent a second note a some n notes from the current note where n is a value between {1, nummber of notes in the generated buffer}
 When sequences get randomized by the Arp parameter, this will end up producing different types of intervals.


## GALAXY dev log and TODO

### Jan 2026

--
Tempo
- Solved sync issues with the custom midi sequencer (galaxy sequencer) by using custom midi cc #94

Implement after touch functionality
- Velocity? Midi CC?

Improved Arps
- some of the random arps are too similar
- idea: repeated sets, so not just reordering and adding octaves, repeat a group of two or three notes example:
	input notes: (1st, 3rd, 5th)
	generated: (3rd, 1st, 5th, 8th, 10th, 1st, 5th)
	generated with sets: (3rd, 1st, 3rd, 1st, 5th, 8th, 10th, 1st, 5th, 10th, 1st, 5th)
	generated with sets: (3rd, 1st, 5th,3rd, 1st, 5th,3rd, 1st, 5th,8th, 10th, 1st,8th, 10th, 1st,8th, 10th, 1st,)
	    - I think this could lead to less repetative simple sequences and more "song like" or evolving patterns, if played slowly

### TODO carrying over
- decide on which will be the modes, finally? I don't like how the modes behave differently for poly and mono modes. So after trying the existing solution for a bit I have come down to:

Option 1 (current solution):
- mode 1: everyone mono + djent mode
- mode 2: polyphonic + djent mode. 
	- Djent for poly mode will have rests sometimes for one of the intervals
- mode 3: Poly OR mono + freeze mode

Option 2:
- mode 1: everyone mono + djent mode
- mode 2: everyone mono + freeze mode 
- mode 3: poly mode: (mono is mono + freeze, polyphonic + freeze )
  - this means no poly + djent, but maybe more reasonable transition between mono and poly because it doesn't mean "unfreezing" for mono synths to transition from poly mode to mono mode 
  - maybe djent makes more sense for lead lines, maybe rests make less sense for pads/chords?
   


### Sept 2025
MIDI OUT
- send midi out to only the single midi change (pins 14 and 15)
- Separated serial out and midi out using the arduino Mega's multiple serial pin out

FIX CHORD MODE based on the following plan:

Monophonic:
Modes should be djent, (djent or freeze). freeze which affects only the third setting for each channel. Not sure if the best approach for second mode is djent or freeze, but it will be whatever makes the most sense after playing either options.

Polyphonic 
Modes are:
Arp+ djent, chord1+( freeze,/ or djent) chord2+freeze

So mode 0 is the same for mono and poly.

Modes 1 and 2 are chord modes. The importance of rests in this mode is maybe unclear, but I think everything should freeze or unfreeze in the same mode? Or djent or undjent. For ease of knowing where each of the 4 channels is in the pattern.

Chord mode 1 should occur when poly is in regular arp mode, and this mode is not too dissimilar milar from arp mode. Two notes are played simultaneously. Already implemented. The third option for the channel: freeze or djent. Need to experiment.

Chord mode 2: all notes played simultaneously. "All notes" is n, the number of notes in the capture buffer, then there shall be n rest, or n beats of sustain (before note off). This should get more interesting as notes are as more notes are added. Not yet implemented!

TODO write an "octave shift" function 
for higher levels of alg, not just calling change one octave,
shift all notes (maybe shift the currently added note up or down one octave using the replace function)

