;hXRINADEQUATE.cp
;
;version: 4.0/ TS4.1.4 / 05/24/2022
;
;basic J-based SQDQ correlation experiment with refocussing and z-filter for pure in phase signals
;written by JOS and Len Mueller 2/18/2002 added Z-filter 7/12/2004
;updated and prepared for release by JOS 7/31/2011
;may require long experimetnal times for natural abundance samples
;Reference:
;A. Lesage, M. Bardet, and L. Emsley JACS 121, 10987 (1999)

;
;$CLASS=Solids
;$DIM=2D
;$TYPE=
;$SUBTYPE=
;$COMMENT=
;"cnst50=1000000/(4*cnst49)"

#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>

#include <Decoup_Solids.incl>

"plw13=plw0*pow(((cnst51+cnst38)*4*p0/(1000000)),2)"

"plw14=plw0*pow((cnst31*4*p0/(1000000)),2)"

"p63=1000000/(2*cnst51)"
#ifdef AWS
#include <WeightedSampling.incl>
#endif
"p2=2*p1"


"l5=d5*cnst31"
"l7=d7*cnst31"
"l8=d14*cnst31"
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

"acqt0=-p1*2/3.1416"

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
  
3 10m do:f2
#  ifdef AWS
  subr qsin_2d:aws(64, 2)
#  endif /*AWS*/
4 d1 do:f2 
  trigg
  10u
  (p1 ph4):f1 (p3 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph3):f2
  dtau1  cpds4:f2   
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
  
  (p1 ph16):f1 (1u cpds2):f2 

  go=4 ph31 
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


;
;Avance III version
;parameters:
;d0 : incremented delay (2D) [0 usec]
;d1 : recycle delay
;d5 : coherence evolution time 1
;d7 : coherence evolution time 2
;d14: z-filter ( short for natural abundance and 5 ms or more for 13C labeled
;p1 : X 90 degree pulse
;p2 : X 180 degree decoupling pulse during t1 CS evolution of Y nucleus
;p3 : H 90 degree pulse
;p31 : decoupling pulse for spinal64.13
;p15 : contact time; plw3 (rf-channel f3) and spw0 (rf-channel f2)
;pl1 : rf-power contact pulse on X
;pl11 : rf-power on X for 90 and 180 degree pulses
;pl12 : rf-power for 1H pi/2 pulse p3 and standard proton decoupling during J-evolution
;pl13 : rf=power for cw decoupling on 1H during acquisition
;sp0 : rf-power for proton contact pulse, 50 -20% ramped amplitude modulation
;spnam0 : file name for variable amplitude CP
;cpdprg1 : sequence used for decoupling (swftppm_13)
;cpdprg2 : sequence used for decoupling (swftppm_12spinal64, tppm15, cw, etc.)
;pcpd2 : pulse length in decoupling sequence
;cnst31 : MAS spinning speed 
;NS : 32 * n                     
;l5 : rp's for J refocusing dtau1= 3.5 - 5.5 ms for 45 -70 Hz coupling
;l7 : rp's for J refocusing dtau3= 3.5 - 5.5 ms for 45 -70 Hz coupling
;l8 : rp's for z-filter short for natural abundance 2 - 5 ms for labeled systems
;in0 : = inf1 -> SWH in F1 = n*rotation rate
;FnMODE: States-TPPI, STATES, TPPI
;

;$Id: hXRINADEQUATE.cp,v  Exp $
