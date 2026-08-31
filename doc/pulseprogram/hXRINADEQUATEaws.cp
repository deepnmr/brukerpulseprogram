;hXRINADEQUATEaws.cp
;

; version: 1.0/ TS4.1.2 06/16/2021  JOS
;
;basic J-based SQDQ correlation experiment with refocussing and z-filter for pure in phase signals
;written by JOS and Len Mueller 2/18/2002 added Z-filter 7/12/2004
;updated and prepared for release by JOS 7/31/2011
;may require long experimetnal times for natural abundance samples
;Reference:
;A. Lesage, M. Bardet, and L. Emsley JACS 121, 10987 (1999)
;
;
;
;$CLASS=Solids
;$DIM=2D
;$TYPE= CPMAS RINADEQUATE 
;$SUBTYPE= Refocussed Inadequate J-based
;$COMMENT= 
;"cnst50=1000000/(4*cnst49)"

prosol relations=<solids_JOS>
define loopcounter numrot
"numrot=cnst50/(cnst31)"

define loopcounter fieldX_Hz
define loopcounter fieldH_Hz

#ifdef lowpower
  "cnst52=2500"
#endif

#include <trigg.incl>
        ; definition of external trigger output$



#ifdef fastMASDQ
"cnst54=abs(trunc(cnst53)+1-cnst53)"
"fieldX_Hz=(cnst53)*cnst31"
"fieldH_Hz=cnst54*cnst31"
"spw0=plw30*pow(10,(2*log10((cnst54)*cnst31/(10000*integfac0))))*(1+(cnst33/100))"
"spw1=plw31*pow(10,(2*log10(cnst53*cnst31/(10000*integfac1))))"
#elif fastMASZQ
"cnst54=1+cnst53"
"fieldX_Hz=(cnst53)*cnst31"
"fieldH_Hz=cnst54*cnst31"
"spw0=plw30*pow(10,(2*log10((cnst54)*cnst31/(10000*integfac0))))*(1+(cnst33/100))"
"spw1=plw31*pow(10,(2*log10(cnst53*cnst31/(10000*integfac1))))"
#elif diy			/* choose your own relation */
;set your own power levels, start with prosol values and calculate
#else
"fieldX_Hz=(numrot-0.5)*cnst31"
"fieldH_Hz=(numrot+0.5)*cnst31"
"spw0=plw30*pow(10,(2*log10((numrot+0.5)*cnst31/(10000*integfac0))))*(1+(cnst33/100))"
"spw1=plw31*pow(10,(2*log10((numrot-0.5)*cnst31/(10000*integfac1))))"
#endif
#ifdef lpdec
"cnst62=0.25*cnst31"

#elif diydec 
						; set your own decoupling field
#else
"cnst62=cnst52"
#endif
"plw13=plw30*pow(10,(2*log10(cnst51/(10000))))*(1+(cnst38/100))"
"plw12=plw30*pow(10,(2*log10(cnst62/(10000))))*(1+(cnst32/100))"
"plw14=plw30*pow(10,(2*log10(1*cnst31/(10000))))*(1+(cnst39/100))"
"pcpd2=p31"
"p63=1000000/(2*cnst51)"

"p31=1000000/(2*cnst62)"

#ifndef diydec
"pcpd2=p31"
#endif

"d11=30m"
#include <WeightedSampling.incl>

"p2=2*p1"

define delay dtau1
"dtau1=((1s/cnst31)*l5-2*p1)"
define delay dtau3
"dtau3=((1s/cnst31)*l7-2*p1)"
define delay dtauz
"dtauz=((1s/cnst31)*l8-p1)"
define pulse pfilt
"pfilt=dtauz"
"d0=0"
"in0=inf1"



Prepare, ze
1 0.05u
 lo to 1 times fieldX_Hz
2 0.05u
lo to 2 times fieldH_Hz

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
  
3 d11 do:f2
#  ifdef AWS
  subr qsin_2d:aws(16, 2)
#  endif /*AWS*/
4 d1 do:f2 
  trigg
  10u
  (p1 ph4):f1 (p3 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph3):f2
  dtau1  cpds2:f2   
  (p2 ph5):f1
  dtau1 

  (p1 ph6):f1

  d0

  (p1 ph9):f1
  dtau3 
  (p2 ph10):f1
  dtau3

  (p1 ph15):f1 (1u do):f2
  (pfilt ph3 pl14):f2
  
  (p1 ph16):f1 (1u cpds1):f2 

  go=3 ph31 
  3m do:f2
  10m mc #0 to 3 
	   F1PH(calph(ph2, +45) & calph(ph5, +45) & calph(ph6, +45), caldel(d0, +in0))

HaltAcqu, 1m
exit

ph1={1}*64 {3}*64
ph3=0
ph2=(8) 0 2 4 6 
ph4=1 2 3 0
ph5=(8) 0 2 4 6 2 4 6 0 4 6 0 2 6 0 2 4
ph6=(8) 2 4 6 0

ph9=0  
ph10={1}*16 {2}*16 {3}*16 {0}*16 
ph15=0   
ph16=2  

ph31= {{{{{0 2 0 2}^2}*2}^2}*2}^2

;Avance NEO version
;parameters:
;d0 : incremented delay (2D) [0 usec]
;d1 : recycle delay
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
;cnst32  : 1H decoupling power correction factor in % plw12
;cnst33 : 1H spinlock power correction factor in %
;cnst38  : 1H decoupling power correction factor in % plw13 
;cnst50	:90deg pulse for spinlock field Hz  (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 1H decoupling field in Hz for pl13 decoupling during J-evolution and t1
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling during acquisition
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : not used, p31 (calculated) used instead as pulse length in decoupling sequence cpdprg2
;cpdprg1 : swftppm_12, swftppm, tppm etc. decoupling program
;cpdprg2 : swftppm_13.63, decoupling during J evolution
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : X amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DX90 : measure X 90 pulse (flipback after CP) -DfastMASDQ  
;          or -DfastmasZQ for double or zero quantum CP at fast MAS >20 kHz
;		   or -Ddiy : do it yourself - set your own power levels
;          or -Dlacq : aq is longer than 50 ms
;          or blank
;NS : 32 * n                     
;l5 : rp's for J refocusing dtau1= 3.5 - 5.5 ms for 45 -70 Hz coupling
;l7 : rp's for J refocusing dtau3= 3.5 - 5.5 ms for 45 -70 Hz coupling
;l8 : rp's for z-filter short for natural abundance 2 - 5 ms for labeled systems
;in0 : = inf1 -> SWH in F1 = n*rotation rate
;FnMODE: States-TPPI, STATES, TPPI


;$Id: hXRINADEQUATEaws.cp ,v 1.0   Exp $
