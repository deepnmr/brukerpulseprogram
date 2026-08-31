;hXmulticp_comp.cp

;version: 1.0/ TS4.2.0 /2020/11/05

;multiple CP  



;###############################################################
;#                                                             #
;#  H-C Cross Polarization Experiment                          #
;#                                                             #
;#  R.L. Johnson and K. Schmidt-Rohr, Quantitative solid-state #
;# 13C NMR with signal enhancement by multiple                 #
;# cross polarization, JMR 239 (2014) 44-49 				   #
;#															   #
;# Pu Duan and K. Schmidt-Rohr, Composite-pulse and partially  #
;# dipolar dephased multiCP for improved quantitative 		   #
;# solid-state 13C NMR, JMR 285 (2017) 68-78                   #
;#                                                             #
;###############################################################

;$CLASS=SolidsIcon
;$DIM=1D
;$TYPE=CPMAS
;$SUBTYPE=
;$COMMENT=

prosol relations=<solids_ICON>

#include <trigg.incl>
        ; definition of external trigger output$
#include <Delay.incl>
#include <hX_cp.incl>
#include <Decoup_Solids.incl>

"plw14=plw30*pow((fieldH_corr/(10000)),2)"

"p2=p1*2"

define delay recoverM
"recoverM=d11/2" 
"d13=1s/cnst31-p2/2-0.1u"	;rotor synch
"d3=d13-de+0.1u"						;rotor synch
"p5=p6"			; 100us good starting point  - BOH comment
"p16=p15"

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

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m do:f2
  
  d1

  trigg
  1u fq=0.0:f2                     ;set 1H on resonance
3 recoverM
  (p3 pl2 ph1):f2
  (p1 ph14 pl1):f1 (p1 pl14 ph0):f2
  (p2 ph15):f1		(p2 pl14 ph0):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
  (p2 ph16 pl1):f1		(p2 pl14 ph0):f2
  (p1 ph14 pl1):f1  (p1 pl14 ph0):f2
				   (p3 pl2 ph12):f2
	recoverM
  lo to 3 times l0
  recoverM
  (p3 pl2 ph11):f2
 (p1 ph24 pl1):f1 (p1 pl14 ph0):f2
  (p2 ph25):f1		(p2 pl14 ph0):f2
  (p15:sp1 ph22):f1  (p15:sp0 ph0):f2
 
  0.1u pl12:f2
	d13  cpds2:f2
 (p2 pl1 ph3):f1
  d3

;#######################################################
;#                     Acquisition                     #
;#######################################################

  go=Start ph31 finally do:f2
  1m do:f2
  30m mc #0 to Start F0(zd)

HaltAcqu, 1m
Exit, exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 2                ; H CP spin lock
ph1 = 1                ; H hard pulse

ph2 = {2 2}^1^2^3		   ; C CP spin lock  

ph3 ={{0 0}^1^2^3}^1^2^3	; 180 C pulse EXOR cycle
ph11= 1 
ph12= 3
ph13= 1 1 2 2 3 3 0 0
ph14= {0 0}^1^2^3		
ph15= (8) 3 3 5 5 7 7 1 1
ph16=(8) 5 5 7 7 1 1 3 3

ph22= 0 2 1 3 2 0 3 1
ph24= 2 0 3 1 0 2 1 3
ph25= (8) 7 3 1 5 3 7 5 1
ph10=2

ph31= {{{0 2}^1}^2}^2  ; receiver

;#######################################################

;Avance NEO/ AVIII version
;parameters:
;p0		 : H 90 reference pulse
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at p2
;p15     : HX CP at sp41 (f1,C) and sp40 (f2,H)
;pl1     : X 90 at pl1
;pl2     : H pulse power
;pl12    : H dec power
;pl0	: H for reference
;pl31	: X for 10kHz spinlock field
;sp0    : H CP power
;sp1    : X CP power
;cnst31 : MAS rate in Hz 
;cnst33 : 1H spinlock power correction factor in %
;cnst50	:90deg pulse for spinlock field Hz  (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;d11	 : = 1H T1/2
;pcpd2   :  (calculated) 
;cpdprg2 :  swftppm, spinal64, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : X amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DX90 : measure X 90 pulse (flipback after CP) -DfastMASDQ  or -DfastmasZQ for double or zero quantum CP at fast MAS >20 kHz
;			-Ddiy : do it yourself - set your own power levels
;			-DINVERSE to have X field larger than H field - 19F 1H CP for example
;          -Dlacq : aq is longer than 50 ms
;          -Ddiydec : set your own decoupling field and rf-power

;$Id: hXmulticp_comp.cp                   Exp $
