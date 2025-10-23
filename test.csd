<CsoundSynthesizer>
<CsOptions>
-+rtmidi=NULL -M0 --midi-key=5 --midi-velocity=6 -n -t 60
</CsOptions>
<CsInstruments>

sr = 44100
ksmps = 32
nchnls = 2
0dbfs = 1

chnset 1, "cutoff"

;load soundfonts
;isf	sfload	"assets/FluidR3_GM.sf2"
	;sfplist isf
	;sfplist ir
;	sfpassign	0, isf	

isf1 sfload "assets/000-001-Bright_Yamaha_Grand.sf2"
	sfpassign	0, isf1

isf1 sfload "assets/000-003-Honky_Tonk.sf2"
	sfpassign	3, isf1


instr update_tempo
    iTempo = p4
    prints "changing tempo to %d \n", iTempo
    tempo iTempo, 60
endin

instr 1	; play guitar from score and midi keyboard - preset index = 0
    DictionarySetValue 1, 2
    prints "hello world\n"

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
    prints "instr1 inum = %f ivel = %f\n", inum, ivel
    chnset a1, "instr_1_left"
    chnset a2, "instr_1_right"
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
kSig1 downsamp a1
kSig2 downsamp a2
    prints "instr2 inum = %f ivel = %f\n", inum, ivel
    printks "instr2 kamp = %f kCutoff = %f\n", 1, kamp, kCutoff
    printks "instr2 a1 = %f a2 = %f\n", 1, kSig1, kSig2
    chnset a1, "instr_2_left"
    chnset a2, "instr_2_right"
	outs	a1, a2
	
endin

instr 3
	mididefault	60, p3
	midinoteonkey	p4, p5
inum	init	p4
ivel	init	p5
ivel	init	ivel/127
kamp	linsegr	1, 1, 1, .1, 0
kCutoff chnget "cutoff"
al  lfo kamp, p4, 0
a1  chnget "instr_3_input_left"
a2  chnget "instr_3_input_right"

kSig1 downsamp a1
kSig2 downsamp a2

a1  = a1 * ivel * al * kCutoff
a2  = a2 * ivel * al * kCutoff
    prints "instr3 inum = %f ivel = %f\n", inum, ivel
    printks "instr3 kamp = %f kCutoff = %f\n", 1, kamp, kCutoff
    printks "instr3 a1 = %f a2 = %f\n", 1, kSig1, kSig2
    chnset a1, "instr_3_left"
    chnset a2, "instr_3_right"
	outs	a1, a2
endin


instr 4
    a1  chnget "instr_4_input_left"
    a2  chnget "instr_4_input_right"
    chnset a1, "instr_4_left"
    chnset a2, "instr_4_right"
endin


instr 5
    a1, a2 inch 1, 2
    outs	a1, a2
endin

instr 6
    iskptim  = .3
    ibufsize = 64
    ar1, ar2 mp3in "assets/hello.mp3", iskptim, 0, 0, ibufsize
             outs ar1 * 2, ar2 * 2
endin

instr 7

ktrans linseg 1, 5, 2, 10, -2
a1, a2 diskin2 "assets/hello.mp3", ktrans, 0, 1, 0, 32
       outs a1, a2

endin

instr 8

ktrans linseg 1, 5, 2, 10, -2
a1, a2 diskin2 "assets/hello.wav", ktrans, 0, 1, 0, 32
       outs a1, a2

endin

instr 9

ktrans linseg 1, 5, 2, 10, -2
a1, a2 diskin2 "assets/hello.ogg", ktrans, 0, 1, 0, 32
       outs a1, a2

endin


instr 10

kstatus, kchan, kdata1, kdata2 midiin

prints "hello world\n"

endin

massign 0, 0
massign 1, 1
massign 2, 2
massign 3, 3

iTempo = 60
tempo iTempo, 60

</CsInstruments>
<CsScore>
i10 0 -1

r10
i1 0.00 0.25 60 100
i1 0.25 0.25 62 <
i1 0.50 0.25 65 <
i1 0.75 0.25 69 10
s

i1 1 1 60 100
i1 + 1 62 <
i1 + 1 65 <
i1 + 1 69 10

i6 5 1 60 100

i2 6 1 60 100
i2 + 1 62 <
i2 8 1 65 <
i2 8 1 69 10

i7 10 1 60 100
i8 12 1 60 100
i9 14 1 60 100

;i3 0 3600

i4 0 3600
i5 0 3600

f0 z
</CsScore>
</CsoundSynthesizer>
