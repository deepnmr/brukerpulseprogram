;hXPASS
;avance-version (21/06/23)
;(TopSpin 4.1)
;fixed 092024

; PASS CS-CSA experiment 
; see Levitt et al.
; used for continous (2D)  variation of pulse timings.


;$COMMENT=PASS CS-CSA experiment, continuous or discontinuous variation of delays
;$CLASS=SolidsIcon
;$DIM=2D
;$TYPE=cross polarisation
;$SUBTYPE=CS-CSA correlation
;$OWNER=Bruker


prosol relations=<solids_ICON>

#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>
#include <Decoup_Solids.incl>


"plw13=plw0*pow(((cnst51+cnst38)*4*p0/(1000000)),2)"

"d21=(1s/(6*cnst31))-p2/2"	
"d22=(1s/(6*cnst31))-p2"
"d23=d22"	
"d24=d21"

define delay danger
"in0=1s*td1/cnst31"
"inf1=in0"
"in21=1s/(cnst31*6*td1)"

"in24=5s/(cnst31*6*td1)"
"danger=d22-((td1-1)*in21)"
;"td1=tdmax(td1,d22+in21, in21)"  ;limit td1 to avoid a crash in d22 does not work as in21 is tied to td1

"acqt0=0"

	
 
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
danger
;######################################################
;#           Start of Active Pulse Program            #
;######################################################
 Start, 30m do:f2

  d1
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance

  (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
  d21 cpds4:f2
  (p2 pl1 ph11):f1			
  d22	
  (p2 ph12):f1			
  d23
  (p2 ph13):f1			
  d22
  (p2 ph14):f1			
  d23
  (p2 ph15):f1			
  d24
  go=Start ph31 ph30:r cpds2:f2 finally do:f2
  
  10m mc #0 to Start F1QF(caldel(d21, +in21) & caldel(d22, -in21) & caldel(d23, +in21) & caldel(d24, +in24))
   
HaltAcqu, 1m
Exit, exit

ph1= 0 0 2 2 
ph2= 0
ph0= 1
ph11=(3) 0 1 2
ph12=(3) {0}*3 {1}*3 {2}*3
ph13=(3) {0}*9 {1}*9 {2}*9
ph14=(3) {0}*27 {1}*27 {2}*27
ph15=(3) {0}*81 {1}*81 {2}*81
ph31= 0 1 2 3
ph30=(3) ph11*2-ph12*2+ph13*2-ph14*2+ph15*2-ph31+ph1 



;Avance NEO/ AVIII version
;parameters:
;p1      : X 90 at pl1
;p3      : H 90 at pl2
;p15     : HX CP at sp1 (f1,C) and sp0 (f2,H)
;pl1     : X 90 at pl1
;pl2     : H pulse power
;pl12    : H dec power calculates
;pl13    : H dec power during recoupling cw decoupling
;pl30		 : H for 10 kHz spinlock field
;pl31	   : X for 10kHz spinlock field
;pl32	   : H for 10 kHz spinlock field for decoupling
;sp0     : H CP power calcualted
;sp1     : X CP power calculated
;cnst31  : MAS rate in Hz 

;cnst50	 : max spinlock field in Hz  (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 1H decoupling field in Hz for pl13 decoupling cw decoupling during PASS
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
/*;pcpd2   : calculated see by using "s pcpd2", p31 (calculated) used instead as pulse length in decoupling sequence cpdprg2*/
;cpdprg2 : swftppm_12, decoupling program
;spnam0  : H amplitude modulated contact pulse use  tanhc60 or any ramped pulse ramp50100.100 or ramp70100.100
;spnam1  : C amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -Dlacq : aq is longer than 50 ms and you are careful with decoupling
;          or blank
;ns	     : 243 * n
;FnMode  : QF
;MC2 	  : QF
;si1 : =TD1, no zerofilling, no F1-window function (!)



;$Id: $
