;hXXCORDxy4.cp
;avance-version (21/06/23)

;version: 1.0/ TS3.2 /2014/01/22

; version: 2.0/ TS4.1.2 06/16/2021  JOS

;cp-spin diffusion
;CORDxy4 (combined R2 driven) spin diffusion experiments
;written JOS October 2013
;update by JOS Jan 2014
;
;;Reference:  
;2D  cp  CORDxy4 (combined R2 driven) spin diffusion experiments 
;for CC correlation
;###########################################################
;#                                                         #
;# Hou et al JMR 232 (2013) 18-30; Broadband 	           #
;# homonuclear correlation spectroscopy driven by combined #
;# R2v n sequences under fast magic angle spinning for NMR #
;# structural analysis of organic and biological solids    #
;#                                                         #
;###########################################################

;  set cnst50 to reasonable spinlock field in Hz (40000 - 60000)
;  run experiment - if you want to optimize the HH match POPT on cnst33 +-20 which is +-20% of applied rf-power               
;  default spnam0 can bechanged and updates rf-power automatically

;
;$CLASS=SolidsIcon
;$DIM=2D
;$TYPE=cross polarisation
;$SUBTYPE=homonuclear correlation
;$COMMENT=exchange NMR in rotating solids using CORD for improved spin exchange, rotor synchronised


prosol relations=<solids_ICON>

define loopcounter numrot
"numrot=cnst50/(cnst31)"

define loopcounter fieldX_Hz
define loopcounter fieldH_Hz
define delay TauR

#include <trigg.incl>
		; definition of external trigger output$

#ifdef fastMASDQ
"cnst54=abs(1-cnst53)"
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
"fieldX_Hz=(numrot+0.5)*cnst31"
"fieldH_Hz=(numrot-0.5)*cnst31"
"spw0=plw30*pow(10,(2*log10((numrot-0.5)*cnst31/(10000*integfac0))))*(1+(cnst33/100))"
"spw1=plw31*pow(10,(2*log10((numrot+0.5)*cnst31/(10000*integfac1))))"
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
"p31=1000000/(2*cnst62)"

#ifndef diydec
"pcpd2=p31"
#endif

"plw14=plw30*pow(10,(2*log10(cnst31/(10000))))*(1+(cnst39/100))"
"plw15=plw30*pow(10,(2*log10(cnst31/(20000))))*(1+(cnst39/100))"
"p14=1s/(2*cnst31)"
"d31=1s/cnst31"
"p11=(1s/cnst31)/2"


define pulse mixing

define delay cordmix

define loopcounter cord
"cord=d8/(24*d31)"							
"l11=cord"
"mixing=(cord*24*d31)"
"l0=0"
"inf1=1s/(l3*cnst31)"										
"in0=inf1"
"d0=0"
"l0=0"

"cordmix=mixing"


"acqt0=-(p1*2/3.1416)-0.5u"




	
Prepare, ze
10 0.05u
 lo to 10 times fieldX_Hz
20 0.05u
lo to 20 times fieldH_Hz
;######################################################
;#               Protections: Pre-Check               #
;######################################################

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


#include <TauR_prot.incl>	
 			;min. spinning 2.5 kHz

#include <DARR_prot.incl>
            ;CORD mixing max. 1 sec 


;######################################################
;#           Start of Active Pulse Program            #
;######################################################
  (p14 pl14 ph10):f2
  d31
2 10m do:f2
  d1  do:f2
  
  10u pl2:f2 pl1:f1    ;preselect pl2 drive power for F2
    trigg

  1u fq=0.0:f2				;set 1H on resonance
 
if "l0>0"
{
  "d51=d0-2u"
} 
  (p3 pl2 ph1):f2																				;proton 90 pulse
  (p15:sp1 ph2):f1 (p15:sp0 ph10):f2        ;contact pulse
 if "l0==1"{
   "d0=in0"
   }
if "l0>0"
  {  		;use cpdprg2=tppm15, SPINAL64 or XiX
  d51 cpds2:f2
}
  (p1 pl1 ph3):f1 (1u do):f2

   cordmix cpds3:f2

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  (p1 ph5):f1 (1u do):f2
  go=2 ph31 cpds2:f2 finally do:f2
  1m do:f2
  10m mc #0 to 2 F1PH(calph(ph2,+90),caldel(d0,+in0) & calclc(l0, 1)) 
  1m do:f2
HaltAcqu, 1m
6 exit


ph1=1 3
ph2=1
ph3={0}*8 {2}*8
ph5={{{0}*2}^2}^1
ph10=0
ph11=0
ph13=2 0
ph17= 1 3 2 2 1 3 2 2 2 0 3 3 2 0 3 3
      3 1 0 0 3 1 0 0 0 2 1 1 0 2 1 1			;XY814
ph31=  {{{0 2}^2}^1}^2



;Avance NEO  version
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
;cnst38 : 1H pl13 decoupling power correction factor in %
;cnst39 : 1H pl14 decoupling power correction factor in %
;cnst50	:  90deg pulse for spinlock field Hz  (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 1H decoupling field in Hz for pl13 decoupling = cnst20 for FSLG experiments
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;cpdprg3 : cord 
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : C amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DX90 : measure X 90 pulse (flipback after CP) -DfastMASDQ  or -DfastmasZQ for double or zero quantum CP at fast MAS >20 kHz
;					-Ddiy : do it yourself - set your own power levels
;          -Dlacq : aq is longer than 50 ms
;          or blank



;$Id: $
