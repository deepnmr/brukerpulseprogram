;hChhC.cp 
;
;version: 1.0/ TS3.2 /2013/06/17
;
;written for BioToolkit: JOS WTF 10.5.2013
;double-checked: VEDA SEWE July 2013
;
;2D long-range correlation experiment via Proton-Proton-through space mixing
;zero-quantum version
;
;$CLASS=BioSolids
;$DIM=2D
;$TYPE=Homonuclear
;$SUBTYPE=ZQ mixing
;$COMMENT=Transverse Proton Mixing


;Avance II / AVIII version
;Parameters:
;p1 	 : pulse width for C 90 at pl1
;p3 	 : pulse width for H 90 at pl2
;p15	 : pulse width for first CP (HC, long: 500-2000 us)	
;p21	 : pulse width for N 90 at pl21
;p22	 : pulse width for N 180 at pl21
;p44	 : pulse width for second CP (CH, short: <500 us)
;p48	 : pulse width for third CP (HC, short: <500 us)
;pl1 	 : C pulse power
;pl2	 : H pulse power
;pl12	 : H dec power
;pl21	 : N pulse power
;sp40	 : H HC CP power
;sp41	 : C HC CP power
;sp44	 : H power for second CP (CH)
;sp45	 : C power for second CP (CH)
;sp48 	 : H power for third CP (HC)
;sp49	 : C power for third CP (HC)
;d0      : incremented delay (t1)
;d1      : recycle delay; 1 to 5 times T1
;d9      : HH mixing time (short (less than 750 us); 300 us ~ 6 Angstroms)
;d30     : extra time for constant duty cycle 
;pcpd2   : pulse length in decoupling sequence
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam40 : H ramp use e.g. ramp.10070 for variable amplitude CP
;spnam41 : C ramp use e.g. square.100 for square pulse CP
;spnam44 : H ramp use e.g. ramp.10070 for variable amplitude CP
;spnam45 : C ramp use e.g. square.100 for square pulse CP
;spnam48 : H ramp use e.g. ramp.10070 for variable amplitude CP
;spnam49 : C ramp use e.g. square.100 for square pulse CP
;cnst30  : expected td1 for use in constant duty cycle (CDC)
;cnst31  : MAS rotation rate in Hz
;inf1:  1/SW(C) = 2 * DW(C)
;in0 : = inf1
;in30: = inf1
;l0 : loopcounter for F1
;l1 : loopcounter for mixing time
;ZGOPTNS : -DCDC : for constant duty cycle
;           -DDP : direct C excitation 
;           -DTC : probe in triple channel mode
;		   or blank
;FnMODE: TPPI, States or States-TPPI
;ns: MIN. 4 (full:16)


;######################################################
;#                                                    #
;#  CHHC                                              #
;#  Lange, A.; Becker, S.; Seidel, K.; Pongs, O.;     #
;#    and Baldus,M.  Angw. Chem. 2005, 44, 2089-92    #
;#  Lange, A.; Seidel, K.; Verdier, L.; Luca, S.;     # 
;#    and Baldus, M.,  JACS 2003, 125, 12640-48       #
;#  Lange, A.; Luca, S.; and Baldus, M.;              #
;#     JACS 2002, 124, 9704-5                         #
;#                                                    #
;######################################################


prosol relations=<biosolCHN>


#include <CHN_defs.incl>
			; defines which channel corresponds to which nucleus
#include <trigg.incl>
			; definition of external trigger output

#include <TauR_def.incl>
                        ; definition of rotor period dependent delays


;######################################################
;#          Define Mixing Parameters                  #
;######################################################

"l1=d9/TauR"				; TauR = time of one rotor period
define delay mixing
"mixing=(l1*TauR)"			;mixing = d9

"acqt0=-1u"

"in0=inf1"                  ;##########################
"d0=1u"                     ;#    t1_init => 0, 0     #
"in30=inf1"                 ;##########################
"l0=0"

define delay ONTIME         ;##########################
define loopcounter T1evo    ;# Power Deposition Calcs #
                            ;#   Constant Duty Cycle  #
                            ;##########################
#ifdef CDC
"T1evo=larger(td1,cnst30)"
"d30=T1evo*(in30+2u)"
#endif                      /* CDC */


                            ;##########################
;$EXTERN                    ;# python insertion point #
                            ;##########################

Prepare, ze

"d30=d30/2"

"ONTIME=aq+d0+d30+p15+p44+p48"   

;######################################################
;#               Protections: Pre-Check               #
;######################################################

#include <TauR_prot.incl>	
 			;min. spinning 2.5 kHz
#include <acq_prot.incl>	
			;Max. 50 ms acquisition time
#include <HHmix_prot.incl>
			;d9 max. 750 us
#include <p15bio_prot.incl>
			;p15 max. 10 ms
#include <p44bio_prot.incl>
			;p44 max. 10 ms
#include <p48bio_prot.incl>
			;p48 max. 10 ms
#include <t1_prot.incl>
			;check t1 evolution time
#include <ONTIME_prot.incl>
			;total RF depsosition restriction

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m do:H
  d1

  trigg
  1u fq=0.0:H				;set 1H on resonance
  
;######################################################
;#              Cross/Direct Polarization             #
;######################################################

#ifdef DP
   (p1 pl1 ph10):C 
#else
  (p3  pl2  ph1):H
  (p15:sp41 ph2):C  (p15:sp40 ph0):H
#endif									/*end of DP*/
    
;######################################################
;#                   t1 evolution                     #
;######################################################

  (0.5u pl12):H

#ifdef TC			/* in case of triple channel mode dec. on 15N is turned on */
if "l0>0"
 {
   "d51=d0-2u"

   0.5u cpds2:H
   (center 
	(d51)   
	(p21 pl21 ph20 p22 ph21 p21 ph20):N		
    )
   0.5u do:H
}
#else
if "l0>0"
  {  		
  "d51=d0-2u"

  0.5u cpds2:H  
  d51 
  0.5u do:H
}
#endif				/* end of TC */

;######################################################
;#    			  CP to H; Mix; CP to C       #
;######################################################

  (p44:sp45 ph3):C  (p44:sp44 ph4):H

  (lalign  (p1 pl1 ph9):C  (p3  pl2 ph5):H)  ;store  magn. along Z
  
   mixing

  (p3  pl2 ph6):H					 ; readout	
  (p48:sp49 ph8):C  (p48:sp48 ph7):H

;#######################################################
;#                     Acquisition                     #
;#######################################################

 (0.5u pl12):H 
 0.5u cpds2:H 

gosc ph31 	           	;start ADC with ph31 signal routing

#ifdef CDC
  d30                   
#endif					/*end of CDC*/

1m do:H 				;decoupler off
lo to Start times ns

30m mc #0 to Start
  F1PH(calph(ph2, +90), caldel(d0, +in0) & caldel(d30, -in30) & calclc(l0, 1))

HaltAcqu, 1m
exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0=  0				; H CP spin lock during first cp
ph1=  {{1}*8}^2			; H hard pulse
ph2=  0				; C CP spin lock during first cp
ph3=  0 0 2 2			; C CP spin lock during second cp
ph4=  1				; H CP spin lock during second cp
ph5=  0				; H hard pulse for Z-filter
ph6=  0 2			; H hard pulse for readout
ph7=  1				; H CP spin lock during third cp
ph8= {{0}*4}^2			; C CP spin lock during third cp
ph9=  1				; C hard pulse for Z-filter

ph20= 0				; N 90 hard pulse
ph21= 1				; N 180 pulse

ph31= {{{2 0}^2}^2}^2		; receiver

;#######################################################



;$Id:$
