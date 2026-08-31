;hXSUPER.cp
;avance-version (21/06/23)
;JOS

;CS-CSA correlation S-F. Liu, J-D. Mao, K.Schmidt-Rohr, JMR 155, 15-28 (2002)
;recoupling of CSA by 2*360deg pulses

; includes saturation recovery sequence (optional)
; choose correct d21 for calculation of 
; (tR duration,e.g. 33us at 5 kHz, scalfac = 0.15
; 13C pulses: w1 = gamma B1 = 12.12 wR/2, p22 = (1/12.12 wR) / 2
; 1H decoupling: w1 = gamma B1 > 25 wR
; gamma-integral before TOSS by l5-loop (set l5 = # of sidebands, 4 typically)

; getting ready for ICONNMR - with automatic pulse power calculation

;$CLASS=SolidsIcon
;$DIM=2D
;$TYPE=CPMAS
;$SUBTYPE= CS-CSA correlation
;$COMMENT=


prosol relations=<solids_ICON>

#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>
#include <Decoup_Solids.incl>
#include <lgcalc.incl>
"plw13=plw0*pow(((cnst20+cnst39)*4*p0/1000000),2)"
"plw11=plw31*pow(((6.06*cnst31/10000)),2)"
"acqt0=0"

"l4=0"				; initial t1
"p22=(1s/cnst31)*(1.0/24.24)"	; pulse for 180deg rotations

"d21=(1s/cnst31)*0.2464-0.5u"	; start position of 4*180deg pulses
"d22=(0.5s/cnst31)-4*p22-d21-0.5u"
					; rest of tR/2 in CSA recoupling

"d25=(1s/cnst31)*0.1226-p1/2"	; TOSS delays
"d26=(1s/cnst31)*0.0773-p2"
"d27=(1s/cnst31)*0.2236-p2"
"d28=(1s/cnst31)*1.0433-p2"
"d29=(1s/cnst31)*0.7744-p2/2-de"
"in4=(1s/cnst31)/l5"
"in0=(1s/cnst31)*0.155"
"d0=0"
"inf1=in0"


 
Prepare, ze

#ifndef lacq
#include <acq_prot.incl>
        ;Max. 50 ms acquisition time
#endif
#ifndef lcp15
#include <p15_prot.incl>
			;make sure p15 does not exceed 10 msec 
			;let supervisor change this pulseprogram if 
			;more is needed
#endif

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m do:f2
 
 saturate, d20
          (p3 pl2 ph1):f2
          lo to saturate times l20
  d1
 
  rpp12				; reset 2*360deg phase supercycles
  rpp13
  rpp14
  rpp15
 (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph10):f2
 
  1u cw:f2 pl13:f2

3 d21			
  0.5u do:f2		; increase 1H dec. power during pulses
  (p22 pl11 ph12):f1	; 360deg/2 pulse
  (p22 ph13):f1		; 360deg/2 pulse
  (p22 ph14):f1		; 360deg/2 pulse
  (p22 ph15):f1		; 360deg/2 pulse
  d22 cw:f2 pl13:f2
  d22
  (p22 ph14^):f1 (1u do):f2	; 360deg/2 pulse
  (p22 ph15^):f1	; 360deg/2 pulse
  (p22 ph12^):f1	; 360deg/2 pulse
  (p22 ph13^):f1	; 360deg/2 pulse
  0.5u cw:f2 pl13:f2
  d21 			; finish 1 tR
  lo to 3 times l4
  (p1 pl1 ph7):f1	; store
  d4 do:f2		; z-filter and gamma-integral
  0.1u cpds2:f2		; TPPM decoupling during acq.
  (p1 ph8):f1		; read out

  d25			; ...
  (p2 ph3):f1
  d26
  (p2 ph4):f1
  d27
  (p2 ph5):f1
  d28
  (p2 ph6):f1
  d29 			; ... TOSS

  go=Start ph31 finally do:f2
  1m do:f2

  id4
  lo to Start times l5	; gamma-integral

4 dd4
  lo to 4 times l5 	; decrement d4 again

  10m mc #0 to Start F1PH(calph(ph7, +90), caldel(d0, +in0) & calclc(l4, 1))

HaltAcqu, 1m
Exit, exit

ph1 = 1 3		; 1H 90deg
ph2 = 0 0 1 1 2 2 3 3 	; 13C CP
ph3 = 1 1 2 2 3 3 0 0 	; #1 180deg TOSS
ph4 = 3 3 0 0 1 1 2 2 	; #2
ph5 = 1 1 2 2 3 3 0 0 
      3 3 0 0 1 1 2 2 	; #3
ph6 = 2 2 1 1 0 0 3 3 
      0 0 3 3 2 2 1 1 	; #4
ph7 = 1 1 2 2 3 3 0 0 	; z-store after t1
ph8 = 3 3 0 0 1 1 2 2 	; read out after z-filter
ph10= 0			; 1H CP
ph11= 2

ph12=0 2 1 3 2 0 3 1  0 0 1 1 2 2 3 3	; 180degs for "spin lock"
     2 1 2 3 0 3 0 1  0 1 2 1 2 3 0 3
     3 2 1 2 3 0 3 0  1 0 1 2 1 2 3 0 	; 2nd line shifted by one
     3 0 2 1 3 2 0 3  1 0 0 1 1 2 2 3 	; 1st line shifted by one

ph13=0 0 1 1 2 2 3 3  0 2 1 3 2 0 3 1	; switch 1st 8 with 2nd 8
     0 1 2 1 2 3 0 3  2 1 2 3 0 3 0 1
     1 0 1 2 1 2 3 0 3  2 1 2 3 0 3 0
     1 0 0 1 1 2 2 3 3  0 2 1 3 2 0 3

ph14=ph12+ph11
ph15=ph13+ph11

ph31= 0 2 1 3 2 0 3 1
 
;p0		 : H 90 reference pulse at plw0
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p15     : HX CP at sp1 (f1,X) and sp0 (f2,H) 
;p10 		: DUMBO decoupling pulse 26 to 28 us
;pl1     : X 90 at pl1
;pl2     : H pulse power
;pl12    : H dec power
;pl30	: H for 10 kHz spinlock field
;pl31	: X for 10kHz spinlock field
;sp0    : H CP power
;sp1    : X CP power
;cnst31 : MAS rate in Hz 
;d4: z-filter delay (ca. 1 ms)
;in4: =(1s/cnst31)/l5
;in0: =(1s/cnst31)*0.155 
;d20 : pulse spacing in saturation comb (optional)
;l20 : # of pulses in saturation comb (0 if undesired)
;l5: counter for gamma-integral (e.g. 4)
;cnst50	:90deg pulse for spinlock field Hz  (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 1H decoupling field in Hz for pl13 decoupling = cnst20 for FSLG experiments
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : not used, p31 (calculated) used instead as pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : C amplitude modulated contact pulse use constant amplitude square.100

; process with xfb, set paramter alpha to -1 in F1 and 
;use ptilt1 repeatedly until the spectrum / CSA in F1 displays properly in F1 without being folded



;$Id: $
