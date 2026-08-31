;hC_fsREDOR.cp

;version   TS3.5 /2017/01/10

;written for TopSolids Bio: JOS 10.1.2017

;2D frequency selective REDOR experiment for exact NC heteronuclear distance measurments
;Tested 01/10/2016  on AV III console 700SB HCN 1.3mm probe 
;Avance III version
;parameters:
;d1      : recycle delay

;p1      : C 90 at pl1
;p2      : C 180 at pl1
;p3      : H 90 at pl2
;p4      : H 180 at pl2
;p9		   : z-filter delay pl14 on H
;p15     : HC CP at sp41 (C) and sp40 (H)
;p21     : N 90 at pl21	
;p22     : N 180 at pl21
;pl1     : C pulse power
;pl2     : H pulse power
;pl5     : N NCa CP power
;pl6     : N NCO CP power
;pl12    : H high dec power
;pl13    : H high dec power
;pl14		 : H low power decoupling during z filter approx 1* rotation frequency (DARR)
;pl16    : N Dec. during AQ (using 'waltz16', 2 kHz)
;pl21    : N pulse power
;sp40    : H CP power (n=1 HH condition)
;sp41    : C CP power (n=1 HH condition)
;d0      : incremented delay (t1)
;d1      : recycle delay; 1 to 5 times T1
;d8	     : TEDOR total mixing time 
;d30     : extra time for -DCDC2D/3D (t1)
;d31 		 : taur
;pcpd2   : H dec pulse cpdprg2 (~H180 + 0.3 us for swftppm)
;cpdprg2 : Decoupling program during REDOR element. use 'swftppm' (at pl12 or pl13) or sltppm for fast MAS >40 kHz and low power decoupling
;cpdprg3 : Decoupling program  during data acquisition in t1 and t2 use 'swftppm' (at pl12 or pl13) or sltppm for fast MAS >40 kHz and low power decoupling
;spnam40 : H ramp use e.g. ramp.10070 for variable amplitude HCa CP
;spnam41 : C ramp use e.g. square.100 for square pulse HCa CP
;spnam20 : selective pulse on C  Gauss or similar
;spnam21 : selective pulse on N Gauss or similar

;cnst10  : Frequency reset to Carrier (o1, usually ~100 ppm)
;cnst21  : Frequency offset for CO (~175 ppm)
;cnst22  : Frequency offset for Ca (~55 ppm)
;cnst31  : MAS rotation rate in Hz

;in0     : increment of dephasign steps
;l1      : loopcounter for REDOR mixing period 
;l2			 : integer for C selective, rotor synchronized pulse
;l4			 : L4<L2 integer for N selective, rotor synchronized pulse
;l3			 : loopcounter for REDOR mixing time increment
;ZGOPTNS : -DlargeSW   for large SW (unlikely)
;					 -DXY16 : for xy16 phase cycle during REDOR block  - hardly required and potentially detremental for signal
;          or blank
;FnMODE  : States-TPPI is recommended (for easy rotor sync.)
;          but mc command allows for other FnModes as well
;ns 		 : n*8
;mix		 : longest REDOR mixing time 
;redmix  : TEDOR block total mixing time duration determined by 0.5*td1


;#########################################################
;#                                                       #
;#  Frequency Selective Heteronuclear Dipolar Recoupling #
;#  in Rotating Solids: Accurate 13C-15N Distance    	 #
;#  Measurement in Uniformly 13C, 15N-Labeled Solids	 #
;#  C.P.Jaroniec, B.A.Tounge, J.Herzfeld and R.G.Griffin #
;#  JACS 2001, 124, 3507 - 3519			         #
;#                                                       #
;#########################################################

;$COMMENT=heteronuclear correlation  CN
;$CLASS=BioSolids
;$DIM=2D
;$TYPE=cross polarisation
;$SUBTYPE=REDOR


prosol relations=<biosolCHN>

#include <CHN_defs.incl>
        ; defines which channel corresponds to which nucleus
#include <trigg.incl>
        ; definition of external trigger output

#include <TauR_def.incl>
        ; definition of rotor period dependent delays

"p22=2*p21"
"d25=0.25s/cnst31"
"d26=0.25s/cnst31-(p22/2)"	; one-quarter rotor cycle ...
"d28=0.75s/cnst31-1u"
"d29=1.25s/cnst31-1u"
"d31=1s/cnst31"
"p2=2*p1"
"p20=2*l2*1s/cnst31"
"p19=2*l4*1s/cnst31"

"acqt0=0"
define delay REDincr
define loopcounter nfid
"nfid=td1/2"
"l0=0"


define delay mix
"mix=4*l3*nfid/cnst31"
"REDincr=4*l3/cnst31"

if "l1=0" {
"mix=4*l1*l3*nfid/cnst31"
"REDincr=4*l1*l3/cnst31"
 }

"in0=REDincr"
define delay ONTIME         ;##########################
                            ;# Power Deposition Calcs #
                            ;##########################

                            ;##########################
;$EXTERN                    ;# python insertion point #
                            ;##########################

Prepare, ze

"ONTIME=aq+mix+p15" 
ONTIME
;######################################################
;#               Protections: Pre-Check               #
;######################################################

#include <TauR_prot.incl>
        ;min. spinning 2.5 kHz

#ifndef lacq
#include <acq_prot.incl>
        ;Max. 50 ms acquisition time
#include <ONTIME_prot.incl>
        ;total RF deposition restriction
#endif

#include <p15bio_prot.incl>
        ;p15 max. 10 ms

#include <t1_prot.incl>
        ;check d0 and d30				
REDincr
mix
Start, 30m do:H
  d1 do:H

  10u pl1:C                  	;set power level to drive HP amplifier
  10u pl2:H                  	;set decoupler power
  10u pl21:N
  1m rpp17	       	              	; reset phase list ph8 ...
  1m rpp18
  trigg                       	;additional trigger available on HP router
   1u fq=0.0:H                     ;set 1H on resonance
;######################################################
;#              Cross/Direct Polarization             #
;######################################################

  (p3  pl2 ph1):H
  (p15:sp41 ph2):C  (p15:sp40 ph0):H


;######################################################
;#                   REDOR Mixing S signal            #
;######################################################

   TauR cpds2:H						; tppm decoupling at pl12 if synchronized with TPPM use special decoupling sequence
   d25  pl1:C   					
5  d26 										; First 1/2 REDOR period several F3 pulses ...
  (p22 ph17^):N         	; ... at intervals of ...
   d26	  								; ... one-half rotor cycle
   d26
  (p22 ph17^):N	       		; ... at intervals of ...
   d26	                	; F1 pulse
   d26 										; several F3 pulses ...
  (p22 ph17^):N        		; ... at intervals of ...
   d26	    		      		; ... at intervals of ...
   d26										; ... one-half rotor cycle
  (p22 ph17^):N	       		; ... at intervals of ...
   d26	    			       	; F1 pulse
lo to 5 times l1 
   d28
 	 1u do:H
   (center (p19:sp21 ph4):N (p20:sp20 ph3):C (p20 pl12 ph10):H )        		; Hahn echo refocussing pulse on F1
 	 1u cpds2:H
   d29         						
6  d26 						; second 1/2 REDOR period several F3 pulses ...
  (p22 ph18^):N        		; ... at intervals of ...
   d26	              		; ... at intervals of ...
   d26										; ... one-half rotor cycle
  (p22 ph18^):N	       		; ... at intervals of ...
   d26	        	       	; 
   d26 										; several F3 pulses ...
  (p22 ph18^):N        		; ... at intervals of ...
   d26	        	    		; ... at intervals of ...
   d26										; ... one-half rotor cycle
  (p22 ph18^):N	       		; ... at intervals of ...
   d26	        	      	; F1 pulse
lo to 6 times l1 					; 
   d28										; End of REDOR period
  go=Start ph31
  1m do:H
  30m wr #0 if #0 zd 			; Recording REDOR dephasing signal S


12 d1 do:H
  10u pl1:C                  	;set power level to drive HP amplifier
  10u pl2:H                  	;set decoupler power
  10u pl3:N

  1m rpp17	       	              	; reset phase list ph8 ...
  1m rpp18
  trigg                       	;additional trigger available on HP router
;######################################################
;#              Cross/Direct Polarization             #
;######################################################

  (p3  pl2 ph1):H
  (p15:sp41 ph2):C  (p15:sp40 ph0):H


;######################################################
;#                   REDOR Mixing S0 signal           #
;######################################################
   d31 cpds2:H
   d25  pl1:C 					; tppm decoupling at pl12 if synchronized with TPPM use special decoupling sequence
15  d26 										; First 1/2 REDOR period several F3 pulses ...
  (p22 ph17^):N         	; ... at intervals of ...
   d26	  								; ... one-half rotor cycle
   d26
  (p22 ph17^):N	       		; ... at intervals of ...
   d26	                	; F1 pulse
   d26 										; several F3 pulses ...
  (p22 ph17^):N        		; ... at intervals of ...
   d26	    		      		; ... at intervals of ...
   d26										; ... one-half rotor cycle
  (p22 ph17^):N	       		; ... at intervals of ...
   d26	    			       	; F1 pulse
lo to 15 times l1 
  d28
 1u do:H
   (center  (p20:sp20 ph3):C (p20 pl13 ph10):H )        		; Hahn echo refocussing pulse on F1
 1u cpds2:H
   d29         						
16 d26 						; second 1/2 REDOR period several F3 pulses ...
  (p22 ph18^):N        		; ... at intervals of ...
   d26	              		; ... at intervals of ...
   d26										; ... one-half rotor cycle
  (p22 ph18^):N	       		; ... at intervals of ...
   d26	        	       	; 
   d26 										; several F3 pulses ...
  (p22 ph18^):N        		; ... at intervals of ...
   d26	        	    		; ... at intervals of ...
   d26										; ... one-half rotor cycle
  (p22 ph18^):N	       		; ... at intervals of ...
   d26	        	      	; F1 pulse
lo to 16 times l1 					; 
   d28										; End of REDOR period
  go=12 ph31
  1m do:H
  30m wr #0 if #0 zd 			; Recording REDOR reference signal S0

30 1u iu1
  lo to 30 times l3
lo to Start times nfid
 
HaltAcqu, 1m
exit


ph0= 1									; phHCP
ph1= {0}*4 {2}*4				; phH90
ph2= 0 									; phCCP
ph3= 0 1 2 3 						; phC180sel
ph4= 0  								; phN180sel
 
ph10= 0									; phHdec
ph17= 0 1 0 1 					; phNredor	xy4  	 
ph18= 0 1 0 1 					; phNredor	xy4	 


ph31= 0 2 0 2 2 0 2 0



;$Id:$
