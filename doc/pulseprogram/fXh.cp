;fXh.cp

;version: 3.0/ TS4.4.0 09/01/2023
; JOS

;###############################################################
;#                                                             #
;#  F-X with H and F decoupling Cross Polarization Experiment  #
;#                                                             #
;#  Adjust sp0, sp1, and p15 for maximum signal.               #
;#  The Hartman-Hahn CP condidition is B1(H)=B1(X)+-1          #
;#  sp0: usually a linear or tangenial amplitude ramped pulse  #
;#  sp1: usually a constant amplitude pulse       		       #
;#  p15: usually between 500-10000 us (sample-dependent)       #
;#                                                             #
;###############################################################

;$CLASS=SolidsIcon
;$DIM=1D
;$TYPE=CPMAS
;$SUBTYPE= HFX
;$COMMENT=

prosol relations=<solids_ICON>
#include <Delay.incl>
#include <hX_cp.incl>
#include <Decoup_fh_Solids.incl>
#include <trigg.incl>
        ; definition of external trigger output$
"d31=1s/cnst31"
"acqt0=-1.1u"
 
Prepare, ze
d31

#ifndef lacq
#include <acq_prot.incl>
        ;Max. 50 ms acquisition time
#endif
#ifndef longp15
#include <p15_prot.incl>
        ;p15 max. 10 ms
#endif

Start, 30m do:f2 do:f3 
   d1
  trigg

  1u fq=0.0:f2                     ;set 1H on resonance

  (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
  0.1u pl12:f3 pl13:f2
  1u cpds2:f2 cpds3:f3
  go=Start ph31 finally do:f2 do:f3
  1m do:f2  do:f3
  30m mc #0 to Start F0(zd)

HaltAcqu, 1m
Exit, exit

ph0 = 0                ; H CP spin lock
ph1 = 1 3              ; H hard pulse
ph2 = 0 0 2 2 1 1 3 3  ; X CP spin lock 

ph10 = 2 

ph31= 0 2 2 0 1 3 3 1  ; receiver

;#######################################################


;basic HFX cp experiment

;Avance NEO / AVIII (?) version
;parameters:
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p15     : HX CP at sp41 (f1,C) and sp40 (f2,H)
;pl1     : X 90 at pl1
;pl2     : H pulse power
;pl12    : H dec power
;pl13    : F dec power 
;pl30	 : F reference for 10 kHz spinlock field
;pl31	 : X for 10kHz spinlock field
;pl37	 : H decoupling 10 kHz reference F3 power for decoupling
;sp0     : F CP power
;sp1     : X CP power
;cnst31  : MAS rate in Hz
;cnst32  : 19F decoupling power correction factor in %
;cnst33	 : 19F spinlock CP correction
;cnst34  : 19F spinlock power correction factor in %
;cnst37  : 1H decoupling power correction factor in %
;cnst50	 : desired spinlock field in Hz for CP X (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 19F decoupling field in Hz for pl13 decoupling 
;cnst61  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition not equals n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : pulse length in decoupling sequence cpdprg2
;cpdprg2 : pidec_13 (19F pi decoupling)
;cpdprg3 : swftppm_12 or similar  (1H decoupling)
;spnam0 : F amplitude modulated pulse variable amplitude CP
;spnam1 : C ramp use e.g. square.100 for square pulse CP 
;ZGOPTNS :  -Dpidec (pi-decouping for 19F)-DCP for basic experiment
;					 -DfastMASDQ  or -DfastmasZQ for double or zero quantum CP at fast MAS >60 kHz
;          -Dlacq : aq is longer than 50 ms
;          or blank

;$Id: fXh.cp,v 1.2                    Exp $
