;hXAPT.cp 

;version: 1.0/ TS4.0.7 08/28/2019
; JOS
;fixed  diydec JOS August2020
;version: 2.0/ TS4.1.2 06/16/2021

; pulse program for the solid-state APT experiment
; setup as pseudo 2D set 
; for positive CH2 and quaternary signals between 2ms and 4ms evolution time
; and negative CH CH3 signals
; uses frequency switched LG decoupling assuming 100kHz
; p0 J-evolution priod to 980us and increment by inp0 196 us in 4 steps 
; or 8 to 12 steps with P0 = 196us and 8 to 16 increments on 196us inp0
;
; Reference:  A. Lesage et al., J. Am. Chem. Soc. 120 (28), 7095-7100, (1998)

;$COMMENT=J-edited solid-state APT experiment, FSLG for homonuclear decoupling
;$CLASS=SolidsIcon
;$DIM=1D /2D
;$TYPE=CP with J-driven multiplicity editing
;$SUBTYPE=editing
;$OWNER=Bruker

prosol relations=<solids_ICON>

#include <trigg.incl>
        ; definition of external trigger output$
#include <Delay.incl>
#include <hX_cp.incl>
#include <Decoup_Solids.incl>

"plw13=plw0*pow(((cnst20+cnst38)*4*p0/(1000000)),2)"

#include <lgcalc.incl>

"p2=2*p1"
"p4=2*p3"
define delay evol
"evol=l0*p5*2"
define delay increm
"increm=l3*p5*2"
"inf1=increm"
"acqt0=0"

Prepare, ze

evol 
increm
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
   d1 
   1u fq=0.0:f2                     ;set 1H on resonance
  (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
3 (p5 pl13 ph21 fq=cnst22):f2
  (p5 pl13 ph22 fq=cnst23):f2
  lo to 3 times l0
  (p2 pl1 ph6):f1 (p4 pl2 ph7 fq=cnst21):f2
4 (p5 pl13 ph21 fq=cnst22):f2
  (p5 pl13 ph22 fq=cnst23):f2
  lo to 4 times l0
  go=Start ph31 cpds2:f2 finally do:f2

  10m mc #0 to Start F1QF(calclc(l0, l3))
HaltAcqu, 1m
Exit, exit

ph0 = 0
ph1 = 1 3
ph2 = 0 0 
ph6 = 1 1 2 2 3 3 0 0
ph7 = 1
ph21= 0
ph22= 2
ph31= 0  2  2  0 

;Avance NEO/ AVIII version
;parameters:
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p15     : HX CP at sp41 (f1,C) and sp40 (f2,H)
;pl1     : X 90 at pl1
;pl2     : H pulse power
;pl12    : H dec power
;pl30	: H for 10 kHz spinlock field
;pl31	: X for 10kHz spinlock field
;sp0    : H CP power
;sp1    : X CP power
;cnst31 : MAS rate in Hz 
;cnst32  :1H decoupling field correction in Hz
;cnst33 : 1H spinlock field correction in Hz
;cnst34 : X spinlock field correction in Hz
;cnst50	: specified spinlock field Hz  X
;cnst51  : 1H homonuclear decoupling field in Hz for pl13 decoupling = cnst20 for FSLG experiments
;cnst52  : 1H heteronuclear decoupling field in Hz for pl12 decoupling
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : (calculated) 
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : X amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DX90 : measure X 90 pulse (flipback after CP) -DfastMASDQ  or -DfastmasZQ for double or zero quantum CP at fast MAS >20 kHz
;					-Ddiy : do it yourself - set your own power levels
;          -Dlacq : aq is longer than 50 ms
;          or blank
;ns : 8 * n
;FnMode :QF(no frequency)
 
;$Id: hXAPT.cp,                         Exp $

;$Id: hXAPT.cp,  v 1.1                    Exp $