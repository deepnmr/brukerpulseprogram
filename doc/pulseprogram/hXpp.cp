;hX.cp
;avance-version (20/09/30)
;JOS
;checked 2.7.2020 krgr
;fixed  diydec JOS August2020

; getting ready for ICONNMR - pulse calculation

;  H-X Cross Polarization Experiment                          

;
;$CLASS=SolidsIcon
;$DIM=1D
;$TYPE=CPMAS
;$SUBTYPE=
;$COMMENT=

prosol relations=<solids_ICON>
#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>
#include <Decoup_Solids.incl>



#ifdef X90
"acqt0=-(p1*2/PI)"
#else
"acqt0=-1.05u"
#endif

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
 (p1 pl1 ph21):f1 
  d1
 
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance

  (p1 pl1 ph20):f1 (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
 
#ifdef X90 
  (p1 pl1 ph3):f1  
#endif                                 /*end of C90*/ 
  
;#######################################################
;#                     Acquisition                     #
;#######################################################

  go=Start ph31 cpds2:f2 finally do:f2
 
  30m mc #0 to Start F0(zd)

HaltAcqu, 1m
Exit, exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 0                ; H CP spin lock
ph1 = 1 3              ; H hard pulse
ph2 = 0 0 2 2 1 1 3 3  ; X CP spin lock 
ph20 = 1 3 3 1 2 0 0 2
#ifdef X90 
ph3 = 1 1 3 3 2 2 0 0  ; X hard pulse
#endif                    /*end of C90*/ 
ph10 = 2
ph21 = 3 1 1 3 2 0 0 2
ph31= 0 2 2 0 1 3 3 1  ; receiver

;#######################################################


;basic cp experiment

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
;pl32	: H fro 10 kHz spinlock field for decoupling
;sp0    : H CP power
;sp1    : X CP power
;cnst31 : MAS rate in Hz 
;cnst32 : 1H pl12 decoupling power correction frequency
;cnst33 : 1H spinlock power correction frequency
;cnst34 : X spinlock field correction in Hz
;cnst50	: X spinlock field for CP
;cnst51  : 1H decoupling field in Hz for pl13 decoupling
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : decoupling pulse calculated
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : C amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DX90 : measure X 90 pulse (flipback after CP) -DfastMASDQ  or -DfastmasZQ for double or zero quantum CP at fast MAS >20 kHz
;					-Ddiy : do it yourself - set your own power levels
;          -Dlacq : aq is longer than 50 ms
;          or blank



;$Id: hXpp.cp,v 1.4 2020/09/30 14:44:45 ber Exp $
