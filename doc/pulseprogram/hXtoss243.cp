;cptoss243.av
;Cross-polarization with TOSS sideband suppression,
;CPD decoupling and a shaped H-nucleus CP pulse. The 5-pulse
;TOSS sequence used here occupies 1.0 rotor cycles, and is
;well-spaced enough to be used at moderate spinning rates 
;(up to 12.5 kHz).  For decoupling, use either TPPM, SPINAL-64, 
;CW or another solids decoupling sequence.  
;References:
;
;  TOSS sequence:
;	O. N. Antzutkin, Prog. NMR. Spectros., v35, p203-266 (1999).
;
;	Z. Song, O. N. Antzutkin, X. Feng and M. H. Levitt, Solid 
;		State NMR, v2, p143 (1993).
;
;Converted to Avance version 08/17/06  JOS  from FGV

;$CLASS=SolidsIcon
;$DIM=1D
;$TYPE=cross polarisation
;$SUBTYPE=side band suppression
;$COMMENT=basic CP experiment with TOtal Suppresson of Sidebands

prosol relations=<solids_ICON>
#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>
#include <Decoup_Solids.incl>

"acqt0=0"
baseopt_echo

"p2=p1*2"					;determine X90 from X180
"d21=1u"					;power switching delay		
"d31=1s/cnst31"					;one rotor period
"d23=d31*l30"					;optional shifted echo

"d25=0.20290s/cnst31-p1-d21"			;calculate TOSS timings
"d26=0.12288s/cnst31-p2"
"d27=0.17422s/cnst31-p2"
"d28=0.17422s/cnst31-p2"
"d29=0.12288s/cnst31-p2+d23"
"d30=0.20290s/cnst31-p1+d23"


1 ze						;accumulate into an empty memory
2 10m
3 d1 do:f2					;recycle delay, decoupler off in go-loop

;perform protection checks
#include <p15_prot.incl>	
			;make sure p15 does not exceed 10 msec	
			;let supervisor change this pulseprogram if 
			;more is needed
#ifndef lacq		
			;disable protection file for long acquisition change decoupling power !!! or you risk probe damage
			;if you set the label lacq (ZGOPTNS -Dlacq), the protection is disabled

#include <aq_prot.incl>	
			;allows max. 50 msec acquisition time, supervisor
			;may change  to max. 1s at less than 5 % duty cycle
			;and reduced decoupling field			
#endif
  10u pl1:f1 pl2:f2				;preselect drive power, pl1 for F1, pl12 for F2
  trigg						;trigger for scope, 10 usec
  p3:f2 ph1					;proton 90 pulse
  (p15:sp1 ph2):f1 (p15:sp0 ph10):f2 		;CP contact pulse with square or ramp
						;shape on F2, at pl2 proton power level
  d21 pl12:f2 								;switch F2 power level back to pl12
  d25 pl1:f1 cpds2:f2 				;set for TOSS5 sequence at pl11, p2=X180 pulse at pl11
  (p2 ph11):f1					;TOSS5 pi pulse #1 at pl11	
  d26  		
  (p2 ph12):f1					;TOSS5 pi pulse #2 at pl11	
  d27
  (p2 ph13):f1					;TOSS5 pi pulse #3 at pl11	
  d28
  (p2 ph14):f1					;TOSS5 pi pulse #4 at pl11	
  d29
  (p2 ph15):f1					;TOSS5 pi pulse #5 at pl11 
  d30
go=3 ph31 ph30:r finally do:f2

  10m mc #0 to 2 F0(zd)			;save data to disk

HaltAcqu, 1m					;jump address for protection files
exit						;quit

ph1= (360) 0 0 180 180 
ph2= 0
ph10= 1
ph11= (360) 0 120 240
ph12= (360) {0}*3 {120}*3 {240}*3
ph13= (360) {0}*9 {120}*9 {240}*9
ph14= (360) {0}*27 {120}*27 {240}*27
ph15= (360) {0}*81 {120}*81 {240}*81
ph31= (360) 0 90 180 240
ph30= (360) ph11*2-ph12*2+ph13*2-ph14*2+ph15*2-ph31+ph1 




;set:

;Avance NEO
;parameters:
;p0		 : H 90 reference
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p2		 : X 180 deg pulse at pl1
;p15     : HX CP at sp41 (f1,C) and sp40 (f2,H)
;pl0	 : H power reference
;pl1     : X 90 at pl1
;pl2     : H pulse power
;pl12    : H dec power
;pl30	: H for 10 kHz spinlock field
;pl31	: X for 10kHz spinlock field
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
;ZGOPTNS :    -Ddiy : do it yourself - set your own power levels
;          or -DINVERSE if the from nucleus should have the lower spinlock field than the "to (X)" nucleus
;          or -Dlacq : aq is longer than 50 ms
;          or blank
;		   for decoupling use:
;		       -Dlpdec for 1/4MASR low power decoupling
;			or -Dlpdec8 for 1/8th MASR low power decoupling
;			or -Dpidec for pi -pulse decoupling with pidecTAU_12 
;			or -Ddiydec for home built decoupling
;			or -DXiX for XiX decoupling
;d1: recycle delay
;d3: receiver delay

;ns:  243 * n