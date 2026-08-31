;hXT12D.cp
 

; developed JOS August2020
; getting ready for ICONNMR - pulse calculation
; version: 3.0/ TS4.3.0 05/15/2023
; version: 3.1/ TS4.5.0 05/15/2025 using p0 and plw0 for 1H referencing

;  H-X Cross Polarization with T1 Experiment using the Torchia trick                          

;
;$CLASS=SolidsIcon
;$DIM=2D
;$TYPE=CPMAS T1 experiment
;$SUBTYPE= pseudo 2D
;$COMMENT= Torchia Trick

prosol relations=<solids_ICON>

#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>
#include <Decoup_Solids.incl>

define list<delay> vd_list=<$VDLIST>

"acqt0=-(p1*2/PI)"


Prepare, ze
d31
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

Start, 10m do:f2
   d1
 
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance

  (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
  (p1 pl1 ph3):f1 (0.5u pl12):f2
 vd_list
  (p1 pl1 ph5):f1 (1u cpds2):f2
;#######################################################
;#                     Acquisition                     #
;#######################################################

  go=Start ph31 finally do:f2
 
 10m mc #0 to Start F1QF(calclist(vd_list, 1))

HaltAcqu, 1m
Exit, exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 0              ; H CP spin lock
ph1 = 1              ; H hard pulse
ph2 = 0  						 ; X CP spin lock 

ph3 = 1 3					   ; X hard pulse
ph5 = 0 0 2 2 1 1 3 3 ; detection X
ph10 = 0
ph31= 0 2 2 0 1 3 3 1  ; receiver

;#######################################################


;cp X T1 experiment

;Avance NEO/ AVIII version
;parameters:
;p0	     : H 90 reference pulse with plw0 reference rf-power
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p15     : HX CP at sp41 (f1,C) and sp40 (f2,H)
;pl0	   : H reference pulse power
;pl1     : X 90 at pl1
;pl2     : H pulse power
;pl12    : H dec power
;pl31	: X for 10 kHz spinlock field
;sp0    : H CP pulse shape
;sp1    : X CP pulse shape
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
;pcpd2   : pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : C amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS :  -DfastMASDQ  or -DfastmasZQ for double or zero quantum CP at fast MAS >20 kHz
;					-Ddiy : do it yourself - set your own power levels
;          -Dlacq : aq is longer than 50 ms
;          or blank

;$Id: hXT12D.cp,v 3.0     Exp $
