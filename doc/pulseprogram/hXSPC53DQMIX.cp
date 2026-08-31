;hXSPC53DQMIX.cp
;
;version: 3.0/ TS4.4.0 09/01/2023
; JOS

; from avance-version (29/05/2019) JOS

;####################################################################################################################
;#  H-X Cross Polarization Experiment with SQDQ correlation element                         						#
;#  Reference:  Hohwy, M.,, Rienstra, C.M., Jaroniec, C.P., and Griffin, R.G.; J. Chem. Phys. 110, 7983, (1999)		#
;#              Hohwy, M.,, Rienstra, C.M.,  and Griffin, R.G.; J. Chem. Phys. 117, 4973, (2002)					#
;#              Brinkmann, A., Eden, M., Levitt, M.H.; J. Chem. Phys. 112, 8539 - 8554 (2000)                       #
;#																													#
;#  set cnst50 to reasonable spinlock field in Hz (40000 - 60000)													#
;#  run experiment - if you want to optimize the HH match POPT on cnst33 or cnst34 which is +-5000Hz 			    #
;#	ensure that the rf-power at the spinlock fields do not exceed the eprmitted rf-power if using cnst33/34         #
;#  default spnam0 can bechanged and updates rf-power automatically													#
;#  SPC53 recoupling optimize if needed POPT on cnst35 +- 5000Hz which is +-10% of rf power				#
;####################################################################################################################
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
#include <lgcalc.incl>
"plw11=plw31*pow((((10/3)*cnst31+cnst35)/(10000)),2)"
"plw14=plw30*pow(((cnst31+cnst39)/10000),2)"

"plw13=plw0*pow(((cnst20+cnst38)*4*p0/1000000),2)"

define pulse pul360
"pul360=(3s/cnst31)/10"
define pulse pul90
"pul90=pul360/4"
define pulse pul270
"pul270=pul360*0.75"

define delay rotorp
define delay t1incr
"rotorp=(1s/cnst31)"
"t1incr=rotorp/l3"
define pulse zfilt
"in0=t1incr"
"inf1=in0"
"d31=rotorp"
"l0=0"
"d0=0" 
"l1=d8*cnst31/3s"
define delay mix
"mix=l1*(3s/cnst31)"
"zfilt=1s/cnst31-p1/2-1.5u"



"acqt0=-(p1*2/PI)"

Prepare, ze
mix

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

  d1
  if "l0>0"
{
  "d51=d0-0.4u"
}


  1m rpp7
  1m rpp8
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance

  (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2

if "l0>0"
{
  0.4u pl12:f2
  d51  cpds2:f2
 } 

  (p1 pl1 ph3):f1  (0.5u do):f2
 
  (zfilt ph0 pl14):f2 
  (0.5up ph0 pl13):f2 
  (1u cpds4):f2
3 (pul90 pl11 ph7):f1		;SPC53 DQ mixing
  (pul360 ph8 ipp8):f1   
  (pul270 ph7 ipp7):f1
  lo to 3 times 5
  lo to 3 times l1			 
0.5u do:f2
(zfilt ph0 pl14):f2
(1up ph0 pl12):f2
(p1 pl1 ph5):f1  (1u cpds2):f2		;flip into the xy plane
 
   go=Start ph31 finally do:f2
  1m do:f2 						;switch decoupler off
  
 1m mc #0 to Start
   F1PH(calph(ph2, +90), caldel(d0, +in0) & calclc(l0,1))
 


HaltAcqu, 1m
Exit, exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 0                ; H CP spin lock
ph1 = 1 ;3              ; H hard pulse
ph2 = 0   ; X CP spin lock 
ph3 = {{1}*4}^2 ;1   ; X hard pulse
                   
ph5= 0 1 2 3

ph7= (degree, 90.0) 0 72 144 216 288 180 252 324 36 108 
ph8= (degree, 90.0) 180 252 324 36 108 0 72 144 216 288 


ph10= 0
ph31= {{0 1}^2}^2  ; receiver

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
;cnst32 : 1H pl12 decoupling power correction factor in %
;cnst33 : 1H spinlock power correction factor in %
;cnst35 : X spc53recoupling rf-power correction factor in %
;cnst38 : 1H pl13 decoupling power correction factor in %
;cnst39 : 1H pl14 DARR condition correction factor in %
;cnst50	:90deg pulse for spinlock field Hz  (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 1H decoupling field in Hz for pl13 decoupling = cnst20 for FSLG experiments
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;l3		 : for SWH as l3*cnst31 
;pcpd2   : pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : C amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DX90 : measure X 90 pulse (flipback after CP) -DfastMASDQ  or -DfastmasZQ for double 
;			or zero quantum CP at fast MAS >20 kHz
;			-Ddiy : do it yourself - set your own power levels
;          -Dlacq : aq is longer than 50 ms

;$Id: hXSPC53DQMIX.cp                  Exp $
