# GALAXY dev log and TODO

## oct 2026
- experimenting with "glitch" functionality (inspired by glitch pedals and the delusion that midi can do it too). It's still evolving.

## sept 20206
- fixed the way rests are determined (there was a bug in this)
- fixed a bug in chord mode where a 0 pitch note wasn't stopped from being sent (this should be a rest, no "ON" message with 0 pitch should be sent)
- Dynamic note buffers heap memory: lives for the life of power on - no need to grow or shrink. I know it is not recommended for arduino But this cuts memory reserved globals by half. And I am not managing allocations for the duration of the the session so it should be fine.
- Note buffers have #defined types to use to specify the buffer type (mono or poly or a drum type). This was how it should have been done and now that I have a drum type (aka more types) it makes sense to fix it.

- added support for "drum track" which doesn't use the note or capture buffer, rather send notes based on bit reads if a byte or short
   and always uses the same note (the "root note"). Drum track is therefore a few different tracks, one for each drum "note". 
	- because the drum tracks don't need midi note buffers, this means I can use a few tracks with out taking up much space
	- designed for volca drum, at least at first
	- separate path to generate if a note buffer is a drum
	- different path to send a drum note
	- DIV switch to change speed of pattern, but there are less divisions for now
	- AlG to change which drum pattern is selected. Theres only a few but variety comes from mutation
	- reserving oct switch for randomness. left (0) is least random, right (2) is most random
	- the drum pattern uses the regular "rest pattern" since rests are built into the drum pattern (1 for drum hit 0 for rest)
	- changed the mutate rest function to be a bit shift
	
	-

## July 2026

- removed redundant loop over the channels in the generate() function
- modified how polyphonic notes are chosen in chord mode and
	- used constants set at generation time, so there is pure randomness when notes are chosen
- modified how extra notes are chosen. More pattern based, less complete random
- modified how rests are chosen for buffers with 2 or more ntoes in them
 - choice to subsititute at generation time

## Jan 2026


Jan 13

Tempo
- Sadly tempo is still an issue. Even sending the tempo value directly to my custom sequencer there is still drift. I'm kind of out of ideas for now and will pause this project for now.

- I decided that implementing some fucntionality for after-touch MIDI messages doesn't make sense, as the point of keybed for the arpeggiator is to input notes then alter params with the midi controller. I'm not constantly touching the keyboard to provide enough useful after-touch information

- I decided on a mode scheme for, implementing a totally new mode and changing how freeze/djent is done. I'll update the readme to reflect this. The new mode is less arp like and more base-line like. It features sets of repeating patterns. Number of repeats controlled by the "octave" switch. I believe this mode of generation can offer a bit of contrast to the other modes. I added freeze mode to every mode so that each mode can have a distinct pattern saved when swiching between modes. Yet to test if this is useful for making live music.

- I believe I have improved the arp generation even further - more variety, a bit less random and I tried to avoid more than one "expensive" shuffle operation per algorithm

- I did some cleaning up of the code as I needed to refactor a bunch of it to implement the new way of doing modes



Jan 07 ----------
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
   


## Sept 2025


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
