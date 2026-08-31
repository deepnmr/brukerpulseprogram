;hCNzTEDOR2D.cp

;version   TS3.5  2016/12/23

;Tested 12/06  on AV III console
;updated and ..  JOS07/14/2011
;updated and debugged JOS 11/15/2016  with suggestions and correctiosn by D Mukhopadhyay, Jaroniec Group 

;Avance III version
;parameters:
;d1      : recycle delay

;p1      : C 90 at pl1
;p2      : C 180 at pl1
;p3      : H 90 at pl2
;p4      : H 180 at pl2
;p9		   : z-filter delay 100 -500us at power level pl14 on H approximately 1*MAS rotation rate
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
;d8			 : TEDOR total mixing time <1ms - to several 10 ms
;d30     : extra time for -DCDC2D/3D (t1)
;d31 	 : taur
;pcpd2   : H dec pulse cpdprg2 (~H180 + 0.3 us for swftppm)
;cpdprg2 : Decoupling program during REDOR element. use 'swftppm' (at pl12 or pl13) or sltppm for fast MAS >40 kHz and low power decoupling
;cpdprg3 : Decoupling program  during data acquisition in t1 and t2 use 'swftppm' (at pl12 or pl13) or sltppm for fast MAS >40 kHz and low power decoupling
;spnam40 : H ramp use e.g. ramp.10070 for variable amplitude HCa CP
;spnam41 : C ramp use e.g. square.100 for square pulse HCa CP
;spnam50 : Ca ramp use e.g. 'tcn' for amplitude modulated tangential
;          pulse CaN CP
;spnam51 : CO ramp use e.g. 'tcn' for amplitude modulated tangential
;          pulse NCO CP
;cnst10  : Frequency reset to Carrier (o1, usually ~100 ppm)
;cnst21  : Frequency offset for CO (~175 ppm)
;cnst22  : Frequency offset for Ca (~55 ppm)
;cnst29  : expected td2 for use in -DCDC3D
;cnst30  : expected td1 for use in -DCDC2D/3D
;cnst31  : MAS rotation rate in Hz
;inf1    : 1/SW(N) = 2 * DW(N)
;in0     : increment in F1 (N)
;in30    : increment in F1 (Ca) for -DCDC2D/3D
;l0      : loopcounter for F1 
;ZGOPTNS : -DlargeSW   for large SW (unlikely)
;					 -DXY16 : for xy16 phase cycle during REDOR block  - hardly required and potentially detremental for signal
;          or blank
;FnMODE  : States-TPPI is recommended (for easy rotor sync.)
;          but mc command allows for other FnModes as well
;ns 		 : min. 16;   32 for full phase cycle
;l1 		 : loop counter to determine Tmix L1*4*2*rotor cycle
;l8 		 : loop counter to determine z-filter delay Delta (tauz) L8*rotor cycle want ~200 microseconds
;l9 		 : determines t1 increment in integer rotor periods  or fractions thereof with flag largeSW
;deltaz  : z-filter duration (l8*taur)
;tau     : Compensation time to keep rotor sync.
;redmix  : TEDOR block total mixing time duration


;######################################################
;#                                                    #
;#  3D TEDOR NMR Experiments for simultaneous	      #
;#  Measurement of Multiple Carbon-Nitrogen Distances #
;# in Uniformly 13C, 15N-Labeled Solids		      #
;#   C.P. Jaroniec, C. Filip, and R.G.Griffin         #
;#  JACS 124 2002, 124, 10728-10742		      #
;#                                                    #
;######################################################

;$COMMENT=heteronuclear correlation  CN
;$CLASS=BioSolids
;$DIM=2D
;$TYPE=cross polarisation
;$SUBTYPE=TEDOR


prosol relations=<biosolCHN>

#include <CHN_defs.incl>
        ; defines which channel corresponds to which nucleus
#include <trigg.incl>
        ; definition of external trigger output

#include <TauR_def.incl>
        ; definition of rotor period dependent delays


"p22=2*p21"
"d25=0.25s/cnst31-1u"
"d26=0.25s/cnst31-(p22/2)"	; one-quarter rotor cycle ...
"d27=0.25s/cnst31-(p1)"
"d29=0.75s/cnst31-(p2/2)"
"d31=1s/cnst31"
"l1=d8/(4*2*TauR)"          ; TauR = time of one rotor period
"l8=p9*1u/(TauR)"
"l10=0"

#include <trigg.incl>

"d0=1s/(l9*cnst31)"
"acqt0=1u*cnst11"
define delay t1incr

"l0=0"


# ifdef largeSW
define delay tau
"tau=(1s*(2*l9-1)/(l9*cnst31))-(2*p21)-p1"
"l10=1"
"l11=l10 %l9"
"t1incr=1s/(l9*cnst31)"
"in0=t1incr"
"inf1=in0"
#else
define delay tau
"tau=(1s*1/cnst31)-(2*p21)-p1"



"t1incr=(1s*l9)/(cnst31)"
"in0=t1incr"
"inf1=in0"                  ;##########################
"d0=1u"                     ;#    t1_init => 0, 0     #
"in30=inf1"                 ;##########################

#endif

define pulse tauz
"tauz=l8*1s/cnst31"
define delay redmix
"redmix=l1*8*TauR"


define delay ONTIME         ;##########################
define loopcounter T1evo    ;# Power Deposition Calcs #
#ifdef CDC                  ;#   Constant Duty Cycle  #
"T1evo=larger(td1,cnst30)"  ;#                        #
"d30=T1evo*(in30+2u)"       ;#                        #
#endif                      ;##########################

                            ;##########################
;$EXTERN                    ;# python insertion point #
                            ;##########################

Prepare, ze

"d30=d30/2"

"ONTIME=aq+d0+d30+p15" 

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

  t1incr
  tau
	redmix

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m  do:H
  d1

if "l0>0"
{
  "d51=d0"
}

#ifdef largeSW
  "l11=l10 %l9" 
	"tau=(1s*(2*l9-l11)/(l9*cnst31))-(2*p21)-p1"
#endif

  1m rpp17	       	              	; reset phase list ph8 ...
  1m rpp18
  1m rpp20
  1m rpp21
	trigg
  1u fq=0.0:H                     ;set 1H on resonance
;######################################################
;#              Cross/Direct Polarization             #
;######################################################

  (p3  pl2 ph1):H
  (p15:sp41 ph2):C  (p15:sp40 ph0):H


;######################################################
;#                   REDOR Mixing                    #
;######################################################

   1u cpds2:H
   d25  pl1:C 						; tppm decoupling at pl12 if synchronized with TPPM use special decoupling sequence
5  d26 										; First 1/2 REDOR period several F3 pulses ...
  (p22 pl21 ph17^):N         	; ... at intervals of ...
   d26	  								; ... one-half rotor cycle
   d26
  (p22 ph17^):N	       	; ... at intervals of ...
   d26	                	; F1 pulse
   d26 										; several F3 pulses ...
  (p22 ph17^):N        	; ... at intervals of ...
   d26	    		      		; ... at intervals of ...
   d26										; ... one-half rotor cycle
  (p22 ph17^):N	       	; ... at intervals of ...
   d26	    			       	; F1 pulse
lo to 5 times l1 
  d29
  (p2 ph3):C         		; Hahn echo refocussing pulse on F1
  d29         						
6 d26 										; second 1/2 REDOR period several F3 pulses ...
  (p22 ph18^):N        	; ... at intervals of ...
   d26	              		; ... at intervals of ...
   d26										; ... one-half rotor cycle
  (p22 ph18^):N	       	; ... at intervals of ...
   d26	        	       	; 
   d26 										; several N pulses ...
  (p22 ph18^):N        	; ... at intervals of ...
   d26	        	    		; ... at intervals of ...
   d26										; ... one-half rotor cycle
  (p22 ph18^):N	       		; ... at intervals of ...
   d26	        	      	; F1 pulse
lo to 6 times l1 					; 
   d27										; End of REDOR period
  (p1 ph4):C (1u do):H 	; pi/2 pulse on f1			 
  (tauz pl14 ph19):H			;Z-filter integer rotor period
  (p21 ph5):N            ; pi/2 on f2 begin rotor period at beginning of pulse t1 evolution starts here

;######################################################
;#                   t1 evolution                     #
;######################################################

if "l0>0"{
 d51 cpds3:H
  }


;######################################################
;#                   REDOR Mixing                    #
;######################################################

  (p21 ph6):N  (1u do):H		;pi/2 on f3 End of t1 evolution 
  tau                  		;for rotor synchronization 
  (p1 pl1 ph7):C  (1u cpds2):H  				; rotor period done at end of p1!
   1u 
   d25  pl1:C 						; tppm decoupling at pl12
7  d26 										; several F3 pulses ...
  (p22 pl21 ph20^):N        		; ... at intervals of ...
   d26	          				; ... one-half rotor cycle
   d26
  (p22 ph20^):N	       		; ... at intervals of ...
   d26	 
   d26 										; several N pulses ...
  (p22 ph20^):N        		; ... at intervals of ...
   d26            				; ... one-half rotor cycle
   d26
  (p22 ph20^):N	       		; ... at intervals of ...
   d26	 
lo to 7 times l1 
  d29
  (p2 ph8):C           	; Hahn echo refocussing pulse
  d29  
8 d26 										; several F3 pulses ...
  (p22 ph21^):N        		; ... at intervals of ...
   d26	              		; ... one-half rotor cycle
   d26
  (p22 ph21^):N	       		; ... at intervals of ...
   d26	 
   d26 										; several N pulses ...
  (p22 ph21^):N        		; ... at intervals of ...
   d26	          				; ... one-half rotor cycle
   d26
  (p22 ph21^):N	       		; ... at intervals of ...
   d26	 
lo to 8 times l1
  d27			      					; Hahn echo occurs about now
  (p1 ph9):C (1u do):H
  (tauz pl14 ph19):H
  (p1 ph10):C (1u cpds3):H

;#######################################################
;#                     Acquisition                     #
;#######################################################
gosc ph31            ;start ADC with ph31 signal routing

#ifdef CDC
  d30
#endif                                    /*end of CDC*/

1m do:H
lo to Start times ns

30m mc #0 to Start
  F1PH(calph(ph6, -90), caldel(d0, +in0) & caldel(d30, -in30) & calclc(l0, 1) & calclc(l10, 1))

HaltAcqu, 1m
exit


;#####################################
;#             Phase Cycle           #
;#####################################

ph0= 0															; phHCP
ph1= 1 															; phH90
ph2= 0 															; phXhx
ph3= 0 0 1 1												; phX180p1
ph4= 0 															; phX90p1
ph5= 0		 													; phY90p1
ph6= 0 2 0 2												; phY90p2
ph7= 0 															; phX90p2
ph8= {0}*4 {1}*4										; phX180p2
ph9= {1}*8 {3}*8										; phX90p3
ph10= {0}*16 {2}*16									; phX90p4
ph19= 0															; phHtauz
#ifdef XY16
ph17= 0 1 0 1 1 0 1 0 2 3 2 3 3 2 3 2	; phYredor		xy16
ph18= 0 1 0 1 1 0 1 0 2 3 2 3 3 2 3 2	; phYredor		xy16
ph20= 0 1 0 1 1 0 1 0 2 3 2 3 3 2 3 2	; phYredor		xy16
ph21= 0 1 0 1 1 0 1 0 2 3 2 3 3 2 3 2	; phYredor		xy16
#else
ph17= 0 1 0 1 					    	; phYredor	xy4  
ph18= 0 1 0 1 							; phYredor	xy4		
ph20= 0 1 0 1 							; phYredor	xy4		
ph21= 0 1 0 1 							; phYredor	xy4	
#endif
ph31= 1 3 3 1 3 1 1 3 3 1 1 3 1 3 3 1
      3 1 1 3 1 3 3 1 1 3 3 1 3 1 1 3	; PhRe



;$Id:$
