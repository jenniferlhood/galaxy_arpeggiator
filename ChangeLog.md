# GALAXY dev log and TODO

## Jan 2026

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
