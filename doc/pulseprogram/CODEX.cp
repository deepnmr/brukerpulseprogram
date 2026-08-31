;CODEX.cp


;JOS
;version: 1.0/ TS4.3.0 05/17/2023
; TS4.5 11/07/2024

;centerband-only detection of exchange
;no z-filter after CP
;improved phase cycle after Reichert et al., JMR 151 (2001), 129-135


;$CLASS=Solids
;$DIM=1D
;$TYPE=cross polarisation
;$SUBTYPE=simple 1D
;$COMMENT=basic cp experiment, arbitrary contact and decoupling schemes
prosol relations=<solids_ICON>
#include <Avance.incl>

#include <hX_cp.incl>
#include <Delay.incl>
#include <Decoup_Solids.incl>

    "p2=2*p1"
    "TAU1=0.25s/cnst31"
	"TAU2=0.25s/cnst31-p1-2u"
	"TAU3=0.25s/cnst31-p2"  
	"TAU4=0.25s/cnst31-p1-p1/2"
	
	"d14=0"
	"in14=1s/(cnst31*l5)"
	"COUNTER=d8*1s*cnst31"
	"FACTOR1=2*l0"
#ifdef Scodex							/S signal for depahsing */
  "DELTA=(1s/cnst31)-p1-0.5u"				;z-filer
  "DELTA1=(COUNTER*1s/cnst31)-p1-0.5u"    ;dephasing
#else									/*S0 reference signal */
  "DELTA1=(1s/cnst31)-p1-0.5u"			;dephasing
  "DELTA=(COUNTER*1s/cnst31)-p1-0.5u"     ;z-filter
#endif /* Scodex */

    "TAU5=p3+p15-p1-1s*l31/cnst31" 	;rotor synch.delay in tm
	 "DELTA2=l0*1s/cnst31"

1 ze
DELTA2
Start, d1 do:f2              	;recycle delay
 
  rpp5				;reset phase lists for xy-4

  trigpe4                       ;record rotor phase


  (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1 (p15:sp0 ph23):f2 	;contact pulse
  0.5u  pl12:f2
  1.5u cpds2:f2 		
    TAU1  	;tr/4	
    TAU2		;tr/4 
  (p2 pl1 ph5):f1             ;13C 180 deg. pulse
    TAU1

3   TAU3
  (p2 pl1 ph6):f1             ;13C 180 deg. pulse
    TAU1
    TAU3
  (p2 pl1 ph5):f1
    TAU1
  lo to 3 times l0

    TAU4           ;tr/4
  (p1 pl1 ph8):f1 (1u do):f2           ;first mixing time
    DELTA1 							/* mixing */
    trigpe4                       ;same phase as above

    TAU5

  (p1 pl1 ph9):f1 (0.5u pl12 1.5u cpds2):f2
    TAU4                    ;tr/4   

4   TAU1    
  (p2 pl1 ph5):f1          ;13C 180 deg. pulse
    TAU3
    TAU1
  (p2 pl1 ph6):f1
    TAU3
  lo to 4 times l0

    TAU1                  ;tr/4
  (p2 pl1 ph5):f1            ;13C 180 deg. pulse
    TAU1
    TAU4     	;tr/4

   (p1 pl1 ph3):f1 (1u do):f2           ; second z-filter
   DELTA 
   trigpe4                       ;same phase as above
   TAU5
   
  (p1 pl1 ph4):f1 (0.5u pl12 1.5u cpds2):f2
go=Start ph31 finally do:f2
 
  10m mc #0 to Start F0(zd)

exit

ph0 = 0			;reference phase for detection
ph1 = 0 2	        	;1H 90 excitation
ph23= 1			;1H CP
ph2 = 0			;13C CP
ph3 = {3}*64 {2}*64		;store for second z period
ph4 = {1 1 2 2 3 3 0 0}*16	;readout after second z period 

ph5= 0			;xy-4
ph6= 1

ph8 = {{{3}*8}^3}^2  	;store +cos for tm
      	;+sin
      	;-cos
     	;-sin
;read after tm
ph9 = {{{{{1}*8}^3}*2}^2}^1 ;
	
ph31= {{{{0 2 1 3}^2}*2}^2}^2 
  

;Avance NEO
;parameters:
;p0		 : H 90 reference pulse at plw0
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p15     : HX CP at sp1 (f1,X) and sp0 (f2,H) 
;pl0	 : H 90 with p0 reference pulse
;pl1     : X 90 at pl1
;pl2	 : H 90
;pl12    : H dec power calculated
;pl31	   : X for 10kHz spinlock field
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
;d8			: mixing delay DELTA
;l0			: recoupling duration check delay DELTA2
;pcpd2   : (calculated) 
;cpdprg2 : swftppm, spinal64, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : X amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DScodex for S0 expt. (DELTA1 & DELTA swapped)
;        : -Dlgcp for Lee Goldburg CP
;		   or -Ddiy : do it yourself - set your own power levels
;          or -DINVERSE if the from nucleus should have the lower spinlock field than the "to (X)" nucleus
;          or -Dlacq : aq is longer than 50 ms 
;		   or -DfastMASDQ for fastMAS double quantum Cat fast MAS >20 kHz   
;          or -Dfastmas for x zero quantum CP 
;          or blank
;		   for decoupling use:
;		       -Dlpdec for 1/4MASR low power decoupling
;				or -Dpidec for pi -pulse decoupling with pidecTAU_12 
;			or -Ddiydec for home built decoupling
;			or -DXiX for XiX decoupling
;ns 			: 64 x n


;$Id: CODEX.cp,v1.0   Exp $
