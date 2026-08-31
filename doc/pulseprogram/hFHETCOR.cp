;hFHETCOR.cp

;version: 1.0/ TS4.0.7 08/29/2019
; JOS

;###############################################################
;#                                                             #
;#  CPWISE experiment, useful for HF correlation    		   #
;#                                                             #
;#  Adjust sp0, sp1, and p15 for maximum signal.               #
;#  The Hartman-Hahn CP condidition is B1(H)=B1(X)+-1          #
;#  sp0: usually a linear or tangenial amplitude ramped pulse  #
;#  sp1: usually a onstant amplitude pulse       	   	       #
;#  p15: usually between 500-10000 us (sample-dependent)       #
;#                                                             #
;###############################################################

;$CLASS=SolidsIcon
;$DIM=2D
;$TYPE=CPMAS
;$SUBTYPE= WISE HETCOR
;$COMMENT=

prosol relations=<solids_ICON>
define loopcounter numrot
"numrot=cnst50/cnst31"



define loopcounter fieldH_Hz
define loopcounter fieldF_Hz

#include <trigg.incl>
        ; definition of external trigger output$


#ifdef fastMASDQ
"cnst54=abs(trunc(cnst53)+1-cnst53)"
"cnst54=abs(cnst54)"
"fieldH_Hz=(cnst53)*cnst31"
"fieldF_Hz=cnst54*cnst31"
"spw0=plw30*pow(10,(2*log10((cnst53)*cnst31/(10000*integfac0))))*(1+(cnst33/100))"
"spw1=plw31*pow(10,(2*log10(cnst54*cnst31/(10000*integfac1))))"
#elif fastMASZQ
"cnst54=1+cnst53"

"fieldH_Hz=(cnst53)*cnst31"
"fieldF_Hz=cnst54*cnst31"
"spw0=plw30*pow(10,(2*log10((cnst53)*cnst31/(10000*integfac0))))*(1+(cnst33/100))"
"spw1=plw31*pow(10,(2*log10(cnst54*cnst31/(10000*integfac1))))"
#elif diy			/* choose your own relation */
;set your own power levels, start with prosol values and calculate
#elif INVERSE
"fieldH_Hz=(numrot-1.5)*cnst31"
"fieldF_Hz=(numrot-0.5)*cnst31"
"spw0=plw30*pow(10,(2*log10((numrot-0.5)*cnst31/(10000*integfac0))))*(1+(cnst33/100))"
"spw1=plw31*pow(10,(2*log10((numrot-1.5)*cnst31/(10000*integfac1))))"
#else
"fieldH_Hz=(numrot-1.5)*cnst31"
"fieldF_Hz=(numrot-0.5)*cnst31"
"spw0=plw30*pow(10,(2*log10((numrot-1.5)*cnst31/(10000*integfac0))))*(1+(cnst33/100))"
"spw1=plw31*pow(10,(2*log10((numrot-0.5)*cnst31/(10000*integfac1))))"
#endif
#ifdef lpdec
"cnst62=0.25*cnst31"

#elif diydec 
						; set your own decoupling field
#else
"cnst62=cnst52"
#endif

"plw12=plw30*pow(10,(2*log10(cnst62/(10000))))*(1+(cnst32/100))"
"p31=1000000/(2*cnst62)"
"pcpd2=p31"


"acqt0=0"

# ifdef pidec
"p22=p31"
"d30=1s/cnst31-p22"
# endif /* pidec */
"l0=0"
"d0=0"
"inf1=1s/(l3*cnst31)"
"in0=inf1"
"acqt0=0"
 
Prepare, ze
  1 0.05u
 lo to 1 times fieldH_Hz
2 0.05u
lo to 2 times fieldF_Hz

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
  (p31 ph10 pl12):f2
 
  
 
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance

  (p3 pl2 ph1):f2
 if "l0==1"{
"d0=in0"
}
if "l0>0"{
  d0
}
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
  1u cpds2:f2 

  go=Start ph31 finally do:f2 
  1m do:f2  
  30m mc #0 to Start F1PH(calph(ph1, +90), caldel(d0, +in0) & calclc(l0, 1) )

HaltAcqu, 1m
Exit, exit

ph0 = 0                ; H CP spin lock
ph1 = 1 3              ; H hard pulse
ph2 = 0 0 2 2 1 1 3 3  ; X CP spin lock 
ph11 = 0 0 2 2 1 1 3 3
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
;pl13		 : F dec power
;pl30	   : Hfor 10 kHz spinlock field
;pl31	   : X for 10kHz spinlock field
;pl32		 : F decoupling 10 kHz reference power for decoupling
;pl33	   : H decoupling 10 kHZ reference power for decoupling
;sp0     : F CP power
;sp1     : X CP power
;cnst31  : MAS rate in Hz
;cnst32  : f2  decoupling power correction factor in %
;cnst33 : f2 spinlock power correction factor in %
;cnst50	 : desired spinlock field in Hz for CP X (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 19F decoupling field in Hz for pl13 decoupling 
;cnst61  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition not equals n*cnst53*MASR condition for X and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : pulse length in decoupling sequence cpdprg2
;cpdprg2 : pidec_13 (19F pi decoupling)
;cpdprg3 : swftppm_12 or similar  (1H decoupling)
;spnam0 : H(f2) amplitude modulated pulse variable amplitude CP
;spnam1 : X (f1) ramp use e.g. square.100 for square pulse CP 
;ZGOPTNS :  -Dpidec (pi-decouping for 19F) -DINVERSE for F observe HF CP; 
;					 -DfastMASDQ  or -DfastmasZQ for double or zero quantum CP at fast MAS >60 kHz
;          -Dlacq : aq is longer than 50 ms
;          or blank

;$Id: hFHETCOR.cp,v 1.1                    Exp $
