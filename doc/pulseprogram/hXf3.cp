;hXf3.cp

;JOS
;version: 3.0/ TS4.3.0 05/15/2023

;  H-X Cross Polarization Experiment                          

;  run experiment - if you want to optimize the HH match POPT on cnst33 +-30 check if procheck still permits cnst33= +30               
;  default spnam0 can be changed and updates rf-power automatically
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
;#include <Decoup_Solids.incl>
#include<Decoup_f3_Solids.incl>

#ifdef X90
"acqt0=-(p1*2/PI)"
#else
"acqt0=-1.05u"
#endif

Prepare, ze
d31
3 1u
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

Start, 10m 
  d1
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance

  (p3 pl2 ph1):f2
#ifdef lgcp
  (pma ph11):f2
  0.5u fq=cnst19:f2
#endif
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2

#ifdef X90 
  (p1 pl1 ph3):f1 (0.05u pl12 1u cpds3):f3
go=Start ph31 finally do:f3
#elif decf3
0.05u pl13:f3
1u cpds3:f3
go=Start ph31 finally do:f3  /* only f3 decoupling */ 
                                /*end of C90*/ 

#else 			/* f2 and f3 decoupling */
0.05u pl12:f2 pl13:f3
1u cpds2:f2 cpds3:f3
 go=Start ph31 finally do:f2 do:f3

#endif

;#######################################################
;#                     Acquisition                     #
;#######################################################

 
 
  10m mc #0 to Start F0(zd)

HaltAcqu, 1m
Exit, exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 0                ; H CP spin lock
ph1 = 1 3              ; H hard pulse
ph2 = 0 0 2 2 1 1 3 3  ; X CP spin lock 
ph11 = 3
#ifdef X90 
ph3 = 1 1 3 3 2 2 0 0  ; X hard pulse
#endif                    /*end of C90*/ 
ph10 = 2
ph31= 0 2 2 0 1 3 3 1  ; receiver

;#######################################################


;basic cp experiment

;Avance NEO
;parameters:
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p15     : HX CP at sp41 (f1,C) and sp40 (f2,H)
;pl1     : X 90 at pl1
;pl2     : H pulse power
;pl12    : H dec power
;pl30	: H for 10 kHz spinlock field
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
;pcpd2   : (calculated) 
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : X amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DX90 : measure X 90 pulse (flipback after CP) 
;		   or -DfastMASDQ for fastMAS double quantum Cat fast MAS >20 kHz   
;          or -Dfastmas for x zero quantum CP 
;		   or -Dlgcp for Lee Goldburg CP
;		   or -Ddiy : do it yourself - set your own power levels
;          or -DINVERSE if the from nucleus should have the lower spinlock field than the "to (X)" nucleus
;          or -Dlacq : aq is longer than 50 ms
;          or blank
;		   for decoupling use:
;		       -Dlpdec for 1/4MASR low power decoupling
;			or -Dlpdec8 for 1/8th MASR low power decoupling
;			or -Dpidec for pi -pulse decoupling with pidecTAU_12 
;			or -Ddiydec for home built decoupling
;			or -DXiX for XiX decoupling



;$Id: hX.cp,v3.0   Exp $