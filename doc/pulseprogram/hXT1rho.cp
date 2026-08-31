;hXT1rho.cp

; developed JOS August2020
; getting ready for ICONNMR - pulse calculation
; version: 3.0/ TS4.5.0 05/15/2025

;  H-X Cross Polarization with T1rho Experiment                          
; uses VPLIST for X spinlock pulse width
;
;$CLASS=SolidsIcon
;$DIM=pseudo 2D
;$TYPE=CPMAS
;$SUBTYPE=T1/T2
;$COMMENT=X T1rho CP based


prosol relations=<solids_ICON>
#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>
#include <Decoup_Solids.incl>

"plw14=plw31*pow((cnst14/(10000)),2)"
define list<pulse> vp_list = <$VPLIST>

"acqt0=0"

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

#ifndef lvp
#include <vp_protX.incl>
     ;make sure vplist max value does not
     ;exceed 100 ms
#endif

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m do:f2
  d1
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance

  (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
  (vp_list pl14 ph2):f1 (0.1u pl12):f2
;#######################################################
;#                     Acquisition                     #
;#######################################################
  go=Start ph31 cpds2:f2 finally do:f2
 
 10m mc #0 to Start F1QF(calclist(vp_list, 1))

HaltAcqu, 1m
Exit, exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 0              ; H CP spin lock
ph1 = 1 3             ; H hard pulse
ph2 = 0 0 2 2 1 1 3 3 						 ; X CP spin lock 
ph10 = 0
ph31= 0 2 2 0 1 3 3 1  ; receiver

;#######################################################


;cp X T1 experiment

;Avance NEO 
;parameters:
;p0	     : H 90 reference pulse with plw0 reference rf-power
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p15     : HX CP at sp41 (f1,C) and sp40 (f2,H)
;pl0	   : H reference pulse power
;pl1     : X 90 at pl1
;pl11	 : X spinlock pulse power
;pl2     : H pulse power
;pl12    : H dec power
;pl30	: H for 10 kHz spinlock field
;pl31	: X for 10kHz spinlock field
;sp0    : H CP pulse shape
;sp1    : X CP pulse shape
;cnst14 : X spinlock field in Hz
;cnst32  :1H decoupling field correction in Hz
;cnst33 : 1H CP spinlock field correction in Hz
;cnst34 : X CP spinlock field correction in Hz
;cnst50	: specified spinlock field Hz  X
;cnst51  : 1H homonuclear decoupling field in Hz for pl13 decoupling = cnst20 for FSLG experiments
;cnst52  : 1H heteronuclear decoupling field in Hz for pl12 decoupling;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : C amplitude modulated contact pulse use constant amplitude square.100
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
;      	or -Dlvp to use spin lock pulses longer than 100 ms


;$Id: hXT1rho.cp,v 3.0                    Exp $
