;hXT12D.cp
 

; developed JOS August2020
; getting ready for ICONNMR - pulse calculation
; version: 2.0/ TS4.1.2 06/16/2021  JOS

;  H-X Cross Polarization with T1 Experiment using the Torchia trick                          

;  set cnst50 to reasonable spinlock field in Hz (40000 - 60000)
;  run experiment - if you want to optimize the HH match POPT on pldb30 +-2 dB                
;  default spnam0 can bechanged and updates rf-power automatically
;
;$CLASS=SolidsIcon
;$DIM=1D
;$TYPE=CPMAS
;$SUBTYPE=
;$COMMENT=


prosol relations=<solids_ICON>
#include <Delay.incl>
define loopcounter fieldX_Hz
define loopcounter fieldH_Hz

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
#elif INVERSE
"fieldX_Hz=cnst50"
"fieldH_Hz=cnst50-cnst31"
"spw0=plw30*pow(10,(2*log10((cnst50-cnst31)/(10000*integfac0))))*(1+(cnst33/100))"	; 1H Pulse -1 condition	
"spw1=plw31*pow(10,(2*log10(cnst50/(10000*integfac1))))"					; X pulse +1 condition
#else
"fieldX_Hz=(cnst50)"
"fieldH_Hz=(cnst50+cnst31)"
"spw0=plw30*pow(10,(2*log10(((cnst50)+(1*cnst31))/(10000*integfac0))))*(1+(cnst33/100))"	;1H Pulse
"spw1=plw31*pow(10,(2*log10((cnst50)/(10000*integfac1))))"					;X pulse
#endif
#ifdef lpdec
"cnst62=0.25*cnst31"
#elif lpdec8
"cnst62=0.125*cnst31"
#elif diydec 
						; set your own decoupling field
#else
"cnst62=cnst52"
#endif
"plw13=plw37*pow(10,(2*log10(cnst61/(10000))))*(1+(cnst37/100))"							;19F decoupling F3
"plw12=plw30*pow(10,(2*log10(cnst62/(10000))))*(1+(cnst32/100))"							;1H decoupling  F2

#ifndef diydec
"pcpd2=1000000/(2*cnst62)"
"pcpd3=1000000/(2*cnst61)"
#endif	

"acqt0=0"

# ifdef pidec
define pulse pi22
"pi22=1000000/(2*cnst61)"
"TAU=1s/cnst31-pi22"
# endif /* pidec */

define list<delay> vd_list=<$VDLIST>

"acqt0=0"

#ifdef X90
"acqt0=-(p1*2/PI)"
#endif

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

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m do:f2 do:f3
  d1
 
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance

  (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
  (p1 pl1 ph3):f1
 vd_list
  (p1 pl1 ph5):f1 
;#######################################################
;#                     Acquisition                     #
;#######################################################

  go=Start ph31 cpds2:f2 cpds3:f3 finally do:f2 do:f3
 
 10m mc #0 to Start F1QF(calclist(vd_list, 1))

HaltAcqu, 1m
Exit, exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 0              ; H CP spin lock
ph1 = 1              ; H hard pulse
ph2 = 0  						 ; X CP spin lock 

ph3 = 1 3					   ; X hard pulse
ph5 = 0 0 2 2 1 1 3 3 ; detection X
ph10 = 0
ph31= 0 2 2 0 1 3 3 1  ; receiver

;#######################################################


;cp X T1 experiment

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
;pl32	: H fro 10 kHz spinlock field for decoupling
;sp0    : H CP power
;sp1    : X CP power
;cnst31 : MAS rate in Hz 
;cnst32 : 1H decoupling power correction factor in %
;cnst33 : 1H spinlock power correction factor in %
;cnst50	:90deg pulse for spinlock field Hz  (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 1H decoupling field in Hz for pl13 decoupling = cnst20 for FSLG experiments
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : C amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS :  -DfastMASDQ  or -DfastmasZQ for double or zero quantum CP at fast MAS >20 kHz
;					-Ddiy : do it yourself - set your own power levels
;          -Dlacq : aq is longer than 50 ms
;          or blank

;$Id: hXT12D.cp,v 2.0     Exp $
