;hXHETCORsd.cp

;JOS
;version: 3.0/ TS4.3.0 05/15/2023

;###############################################################
;#                                                             #
;#  H-X correlation Experiment                                 #
;#                                                             #
;#  Set p15 short 100 -200us                                   #
;#  cnst51=100000 homonuclear decoupling field in Hz-          #
;#                                                             #
;###############################################################

;$COMMENT= 
;$CLASS=SolidsIcon
;$DIM=2D
;$TYPE=CPMAS
;$SUBTYPE=HETCOR

prosol relations=<solids_ICON>
#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>
#include <Decoup_Solids.incl>
"plw13=plw0*pow(((cnst20+cnst38)*4*p0/(1000000)),2)"
define delay mixing
"COUNTER=d8*cnst31"
"mixing=COUNTER*1s/cnst31-p1"

define pulse pma
"pma=(p3*547)/900"

#include <lgcalc.incl>
"in0=(0.578*l3*4*p5)"
"inf1=in0"

"l0=0"

"acqt0=-1.05u"

Prepare, ze

#ifndef lacq
#include <acq_prot.incl>
        ;Max. 50 ms acquisition time
#endif
#ifndef longp15
#include <p15_prot.incl>
        ;p15 max. 10 ms
#endif

Start, 10m do:f2
  d1
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance
  (p3 pl2 ph0):f2
#ifdef nodec
 if "l0==1"{
"d0=in0"
}
if "l0>0"{
  (center (d0) (p2 pl1 ph1):f1) 
}
(p3 pl2 ph7):f2
mixing
(p3 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph10):f2
#elif dumbo
  1u fq=cnst24:f2
3 (p10:sp10 ph13):f2
  lo to 3 times l0
  1u fq=cnst21:f2
  (p15:sp1 ph2):f1  (p15:sp0 ph10):f2
#elif lgcp

  (pma ph1):f2
  1u fq=cnst24:f2
3 (p5 pl13 fq=cnst22 ph3):f2
  (p5 fq=cnst23 ph4):f2
  (p5 fq=cnst22 ph3):f2
  (p5 fq=cnst23 ph4):f2
  lo to 3 times l0
  (pma ph7 pl2 fq=cnst21):f2
  (p3 ph7 pl2):f2
 mixing
   (pma ph1 pl2):f2
0.5u fq=cnst19:f2
  (p15:sp1 ph2):f1  (p15:sp0 ph10):f2
  0.5u fq=0.0:f2
#else
  (pma ph1):f2
  1u fq=cnst24:f2
3 (p5 pl13 fq=cnst22 ph3):f2
  (p5 fq=cnst23 ph4):f2
  (p5 fq=cnst22 ph3):f2
  (p5 fq=cnst23 ph4):f2
  lo to 3 times l0
  (pma ph7 pl2 fq=cnst21):f2
   (p3 ph7 pl2):f2
 mixing
   (p3 ph1 pl2):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph10):f2
#endif 
  0.05u pl12:f2
  1u cpds2:f2
  go=Start ph31 cpds2:f2 finally do:f2
  1m do:f2
  10m mc #0 to Start F1PH(calph(ph0, +90), caldel(d0, +in0) & calclc(l0, l3))

HaltAcqu, 1m
Exit, exit


ph0=1 3
ph1=1
ph2=0 0 2 2 1 1 3 3 
ph13=2
ph3=0
ph4=2
ph7=3 
ph10=0
ph31=0 2 2 0 1 3 3 1


;#######################################################


;cp HETCOR experiment
;Avance NEO  version
;parameters:
;p0		 : H 90 reference pulse at plw0
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p15     : HX CP at sp1 (f1,X) and sp0 (f2,H) 
;p10 		: DUMBO decoupling pulse 26 to 28 us
;pl0	 : H 90 with p0 reference pulse
;pl1     : X 90 at pl1
;pl2     : H pulse power
;pl12    : H dec power calculated
;pl13    : H HD power calculated
;pl31	   : X for 10kHz spinlock field
;sp0    : H CP pulse shape
;sp1    : X CP pulse shape
;cnst20  : 1H homonuclear decoupling field in Hz for pl13 decoupling = cnst20 for FSLG experiments
;cnst24	 : 1H offset during FSLG - approximately -2400 \n
;			make sure the 1H spectrum resides above or below the center of the spctrum in F1
;cnst31 : MAS rate in Hz 
;cnst32  :1H decoupling field correction in Hz
;cnst33 : 1H spinlock field correction in Hz
;cnst34 : X spinlock field correction in Hz
;cnst38  :1H homonuclear decoupling field correction in Hz
;cnst50	: specified spinlock field Hz  X
;cnst52  : 1H heteronuclear decoupling field in Hz for pl12 decoupling
;cnst53  : fast MAS CP condition not equals n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;l0		 : j-evolution start delay = l0*4*p5
;l3		 : t1 increment = l3*l0*4*p5
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : (calculated) 
;cpdprg2 : swftppm, spinal64, tppm etc. decoupling program
;spnam0  : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1  : X amplitude modulated contact pulse use constant amplitude square.100
;spnam10 : dumbo or dumbo_1+0 or edumbo22_1+0
;ZGOPTNS : -Dflsg  or -Ddumbo, depending on the homonuclear decoupling desired
;					 -Dnodec  for WISE style HETCOR - no 1H homonuclear decoupling
;          -Dlacq : aq is longer than 50 ms
;          or blank



;$Id: hXHETCOR.cp,v 2.0        ber Exp $
