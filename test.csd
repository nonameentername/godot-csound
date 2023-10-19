<CsoundSynthesizer>
<CsOptions>
-i adc -o dac -B 4096 -+rtmidi=NULL -M0 --midi-key-cps=4 --midi-velocity-amp=5 -n
</CsOptions>
<CsInstruments>

sr = 44100
ksmps = 1
nchnls = 2
0dbfs = 1


;load soundfonts
isf	sfload	"assets/example.sf2"
	;sfplist isf
	;sfplist ir
	sfpassign	0, isf	

instr 1	; play guitar from score and midi keyboard - preset index = 0

	mididefault	60, p3
	midinoteonkey	p4, p5
inum	init	p4
ivel	init	p5
ivel	init	ivel/127					;make velocity dependent
kamp	linsegr	1, 1, 1, .1, 0
kamp	= kamp/3000						;scale amplitude
kfreq	init	1						;do not change freq from sf
kCutoff chnget "cutoff"
a1,a2	sfplay3	ivel, inum, kamp*ivel*kCutoff, kfreq, 0			;preset index = 0
	outs	a1, a2
	
	endin
	
instr 2	; play harpsichord from score and midi keyboard - preset index = 1

	mididefault	60, p3
	midinoteonkey	p4, p5
inum	init	p4
ivel	init	p5
ivel	init	ivel/127					;make velocity dependent
kamp	linsegr	1, 1, 1, .1, 0
kamp	= kamp/2000						;scale amplitude
kfreq	init	1						;do not change freq from sf
kCutoff chnget "cutoff"
a1,a2	sfplay3	ivel, inum, kamp*ivel*kCutoff, kfreq, 3			;preset index = 1
	outs	a1, a2
	
endin



</CsInstruments>
<CsScore>
f0 3600

i1 0 1 60 100
i1 + 1 62 <
i1 + 1 65 <
i1 + 1 69 10

i2 5 1 60 100
i2 + 1 62 <
i2 7 1 65 <
i2 7 1 69 10

</CsScore>
</CsoundSynthesizer>
