;version: 3.0/ TS4.3 05/15/2023
;  JOS

;################################################################
;#                                                              #
;#  H-X Cross Polarization Experiment                           #
;#                                                              #
;#  Adjust sp0, sp1, and p15 for maximum signal.                #
;#  The Hartman-Hahn CP condidition is B1(H)=B1(X)+-1           #
;#  sp0: usually a linear or tangenial amplitude ramped pulse   #
;#  sp1: usually a constant amplitude pulse                     #
;#  p15: usually between 500-10000 us (sample-dependent)        #
;#  p16: 50 to 100us                                            #
;#  TOSS sequence 2.2412 rotor revolutions; tossa for 2 RP      #
;#  Basic TOSS  W. T. Dixon et al., Total Suppression           # 
;#	of Sidebands in CPMAS C-13 NMR", JMR 49, 341-345 (1982) #
;################################################################
;
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
#ifdef tossa
define delay de25
"de25=0.1885s/cnst31-(p2/2.0)-0.1u"
define delay de26
"de26=0.0412s/cnst31-p2"
define delay de27
"de27=0.5818s/cnst31-p2"
define delay de28
"de28=0.9588s/cnst31-p2"
define delay de29
"de29=0.2297s/cnst31-(p2/2.0)-de"
#else
define delay de25
"de25=0.1226s/cnst31-(p2/2.0)-0.1u"
define delay de26
"de26=0.0773s/cnst31-p2"
define delay de27
"de27=0.2236s/cnst31-p2"
define delay de28
"de28=1.0433s/cnst31-p2"
define delay de29
"de29=0.7744s/cnst31-(p2/2.0)-de"
#endif

"acqt0=0"
baseopt_echo

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
#include <rot_prot.incl>
			;protect against misset cnst31, must be more than 1000

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m do:f2
  d1
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance

  (p3 pl12 ph1):f2
  (p15:sp1 ph2):f1 (p15:sp0 ph10):f2
  0.1u pl12:f2
  (de25 cpds2):f2
  (p2 pl1 ph5):f1
  de26 
  (p2 ph6):f1
  de27
  (p2 ph7):f1
  de28
  (p2 ph8):f1
  de29
  go=Start ph31 finally do:f2
  30m mc #0 to Start F0(zd)
HaltAcqu, 1m
exit

;#####################################
;#             Phase Cycle           #
;#####################################

#ifdef tossa
ph0= 0
ph1= 2 0
ph2= 1 1 2 2 3 3 0 0
ph5= 1 1 2 2 3 3 0 0
ph6= 3 3 0 0 1 1 2 2
ph7= 3 3 0 0 1 1 2 2
ph8= 0 0 3 3 2 2 1 1
ph10= 3
ph31= 0 2 1 3 2 0 3 1
 
#else 
ph0= 0
ph1= 2 0
ph2= 1 1 2 2 3 3 0 0
ph5= 1 1 2 2 3 3 0 0
ph6= 3 3 0 0 1 1 2 2
ph7= 1 1 2 2 3 3 0 0
ph8= 2 2 1 1 0 0 3 3
ph10= 3
ph31= 0 2 1 3 2 0 3 1

#endif


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

;$Id: hXtoss.cp,v 3.0   Exp $
