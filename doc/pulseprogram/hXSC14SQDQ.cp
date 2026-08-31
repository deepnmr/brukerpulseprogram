;hXSC14SQDQ.cp
;
;version: 3.0/ TS4.4.0 09/01/2023
; JOS

; from avance-version (29/05/2019) JOS

;####################################################################################################################
;#  H-X Cross Polarization Experiment with SQDQ correlation element                         						#
;#  Reference:  Hohwy, M.,, Rienstra, C.M., Jaroniec, C.P., and Griffin, R.G.; J. Chem. Phys. 110, 7983, (1999)		#
;#             Hohwy, M.,, Rienstra, C.M.,  and Griffin, R.G.; J. Chem. Phys. 117, 4973, (2002)						#
;#              Brinkmann, A., Eden, M., Levitt, M.H.; J. Chem. Phys. 112, 8539 - 8554 (2000)                       #
;#																													#
;#				DQ Mixing experiment with SC14 mixing                         										#
;# 				2d version: C-C correlation through dipolar mixing (NOESY type)										#
;# 				DQ evolution unsynchronized with rotation															#
;#																													#		
;#  set cnst50 to reasonable spinlock field in Hz (40000 - 60000)													#
;#  run experiment - if you want to optimize the HH match POPT on cnst33 or cnst34 which is +-5000Hz 			    #
;#	ensure that the rf-power at the spinlock fields do not exceed the eprmitted rf-power if using cnst33/34         #
;#  default spnam0 can bechanged and updates rf-power automatically													#
;#  SC14 recoupling optimize l1 and l2 for best signal using POPT 													#
;#  and POPT on cnst35 +- 5000Hz which is +-10% of rf power															#
;####################################################################################################################
;
;$CLASS=SolidsIcon
;$DIM=2D
;$TYPE=CPMAS
;$SUBTYPE=X-X Correlation
;$COMMENT=

prosol relations=<solids_ICON>
#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>
#include <Decoup_Solids.incl>
#include <lgcalc.incl>

"plw11=plw31*pow((((3.5*cnst31)+cnst35)/(10000)),2)"
"plw13=plw0*pow(((cnst20+cnst39)*4*p0/1000000),2)"

define pulse pul180
"pul180=(1s/cnst31)/7"
define pulse pul90
"pul90=pul180/2"

define delay rotorp
"rotorp=(1s/cnst31)"

define delay t1incr
#ifdef lsw
"t1incr=rotorp/l3"
#else
"t1incr=l3*rotorp"
#endif

"in0=t1incr"
"inf1=in0"
"d31=rotorp"
"d0=0" 
"l0=0"
define delay dqevol
"dqevol=(l1+l2)*2*pul180"

"acqt0=-(p1*2/PI)"

Prepare, ze
	t1incr
	dqevol
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

Start, 30m do:f2
#ifdef lsw
  "cnst27=-180*(d0)/d31"		
			
  1m ip13+cnst27
  1m ip14+cnst27
#endif
StartGo,  d1
 if "l0>0"
{
  "d51=d0-3u"
}               
  1m rpp11
  1m rpp12
  1m rpp13
  1m rpp14
  trigg
  1u fq=0.0:f2                     ;set 1H on resonance

  (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
 
  (p1 pl1 ph3):f1  (0.5u pl13 1u cpds4):f2

3 (pul180 pl11 ph11 ipp11 ipp13 ipp14):f1	;SC14 DQ excitation
  (pul180 ph12 ipp12):f1   
  lo to 3 times l1			;l1 = multiple of 7 
 if "l0>0"
{ 
  0.1u do:f2 
  0.4u pl12:f2
  d51 cpds2:f2
 0.5u do:f2 
 0.5u pl13:f2
 1.5u cpds4:f2
 }
5 (pul180 ph13 ipp13):f1			;SC14 DQ reconversion
  (pul180 ph14 ipp14):f1
  
  lo to 5 times l2		;l2 = multiple of 7 
   

  (p1 pl1 ph5):f1  (0.1u do 0.4u pl12 1u cpds2):f2				;flip into the xy plane
  gosc ph31 					;finally do:f2
  1m do:f2 						;switch decoupler off
  40u ip13						;shift DQ conversion block by 90deg to 
  40u ip14
 
  lo to StartGo times ns
30m mc #0 to Start
  F1PH(calph(ph11, +45) & calph(ph12, +45), caldel(d0, +in0) & calclc(l0,1))


HaltAcqu, 1m
Exit, exit


;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 0                ; H CP spin lock
ph1 = {1}*16 {3}*16
ph2 = 0   ; X CP spin lock 
ph3= {3}*16 {1}*16
                   

ph5= {{{1 2}^2}^1}^2

ph11= (degree, 45.00) 0 128.57 257.14 25.71 154.29 282.86 51.43
		180 308.57 77.14 205.71 334.29 102.86 231.43
		25.71  257.14 128.57 0 231.43 102.86 334.29	
	205.71 77.14 308.57 180  51.43 282.86 154.29
		180 308.57 77.14 205.71 334.29 102.86 231.43
		0 128.57 257.14 25.71 154.29 282.86 51.43
		205.71  77.14 308.57 180 51.43 282.86 154.29	
	25.71 257.14 128.57 0  231.43 102.86 334.29
ph12=(degree, 45.00) 0 128.57 257.14 25.71 154.29 282.86 51.43
		180 308.57 77.14 205.71 334.29 102.86 231.43
		257.14 128.57 0 231.43 102.86 334.29 	205.71 
		77.14 308.57 180  51.43 282.86 154.29 25.71  
		180 308.57 77.14 205.71 334.29 102.86 231.43
		0 128.57 257.14 25.71 154.29 282.86 51.43
		77.14 308.57 180 51.43 282.86 154.29 25.71 
		257.14 128.57 0  231.43 102.86 334.29 205.71  

ph13=(degree, 90.00) 90 218.57 347.14 115.71 244.29 12.86 141.43
                  270 38.57 167.14 295.71 64.29 192.86 321.43
		115.71 347.14 218.57 90 321.43 192.86 64.29
		295.71 167.14 38.57 270 141.43 12.86 244.29
	  270 38.57 167.14 295.71 64.29 192.86 321.43
		90 218.57 327.24 115.71 244.29 12.86 141.43
		295.71 167.14 38.57 270 241.43 12.86 244.29
		115.71 347.14 218.57 90 321.43 192.86 64.29	
								   
ph14=(degree, 90.00) 90 218.57 347.14 115.71 244.29 12.86 141.43
                  270 38.57 167.14 295.71 64.29 192.86 321.43
		347.14 218.57 90 321.43 192.86 64.29 295.71 
		167.14 38.57 270 141.43 12.86 244.29 115.71 
	  270 38.57 167.14 295.71 64.29 192.86 321.43
		90 218.57 327.24 115.71 244.29 12.86 141.43
		167.14 38.57 270 241.43 12.86 244.29 115.71 
		347.14 218.57 90 321.43 192.86 64.29 295.71 

ph10= 0
ph31= {{{0 3}^2}^1}^2  ; receiver

;#######################################################


;basic cp experiment

;Avance NEO/ AVIII version
;parameters:
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p15     : HX CP at sp41 (f1,C) and sp40 (f2,H)
;pl1     : X 90 at pl1
;pl11		 : SPC5 recoupling power - calculated
;pl2     : H pulse power
;pl12    : H dec power
;pl30	: H for 10 kHz spinlock field
;pl31	: X for 10kHz spinlock field
;pl32	: H for 10 kHz spinlock field for decoupling
;sp0    : H CP power
;sp1    : X CP power
;cnst31 : MAS rate in Hz 
;cnst32 : 1H pl12 decoupling power correction factor in %
;cnst33 : 1H spinlock power correction factor in %
;cnst35 : X spc53recoupling rf-power correction factor in %
;cnst38 : 1H pl13 decoupling power correction factor in %
;cnst50	:90deg pulse for spinlock field Hz  (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 1H decoupling field in Hz for pl13 decoupling = cnst20 for FSLG experiments
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;l1		 : DQ conversion block length (4 pi pulses long)*l1
;l2		 : DQ reconversion block length (4 pi pulse long)*l2
;l3		 : determines spectral width swh=1/(l3*MASR) for large SW in F1:  -Dlsw swh=l3/MASR
;cpdprg1 : cw_13 decoupling during recoupling
;pcpd2   : pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : C amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DX90 : measure X 90 pulse (flipback after CP) -DfastMASDQ  or -DfastmasZQ for double or zero quantum CP at fast MAS >20 kHz
;					-Ddiy : do it yourself - set your own power levels; -DINVERSE  X is +1 H -1 condition  
;          -Dlacq : aq is longer than 50 ms
;          -Ddiydec: setup own decoupling 
;          or -Dlsw (large sweep width requires processing efforts: shift in ppm using "xfshear rotate shift" 
;			with: shift = cnst31*(L3-2)*bf1/1000000.

;$Id: hXSPC5SQDQ.cp ,v 1.1                    Exp $