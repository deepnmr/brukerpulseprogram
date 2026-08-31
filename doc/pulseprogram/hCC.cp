;hCC.cp
;
;version: 1.0/ TS3.2 /2013/06/17
;
;written for BioToolkit: JOS WTF 10.5.2013
;double-checked: VEDA SEWE July 2013
;
;2D 13C-13C correlation (PDSD/DARR) experiment
;
;$CLASS=BioSolids
;$DIM=2D
;$TYPE=CPMAS
;$SUBTYPE=ZQ mixing
;$COMMENT= HC CP with C Z-mixing


;Avance II / AVIII version
;parameters:
;p1      : C 90 at pl1
;p3      : H 90 at pl2
;p15     : HC CP at sp41 (f1,C) and sp40 (f2,H)
;p21     : N 90 at pl21 (-DTC)
;p22     : N 180 at pl21 (-DTC)
;pl1     : C pulse power (-DC90)
;pl2     : H pulse power
;pl12    : H dec power
;pl14    : H DARR power (= 1*cnst31 (MAS freq); for PDSD = 0 W)
;pl21    : N pulse power
;sp40    : H CP power
;sp41    : C CP power
;d0      : incremented delay (t1)
;d1      : recycle delay; 1 to 5 times T1
;d8      : PDSD/DARR mixing time
;d30     : extra time for constant duty cycle 
;pcpd2   : pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam40 : H ramp use e.g. ramp.10070 for variable amplitude CP
;spnam41 : C ramp use e.g. square.100 for square pulse CP 
;cnst30  : expected td1 for use in constant duty cycle (-DCDC)
;cnst31  : MAS rotation rate in Hz
;inf1    : 1/SW(C) = 2 * DW(C)
;in0     : = inf1
;in30    : = inf1
;l0      : loopcounter for F1
;l1      : loopcounter for mixing time
;ZGOPTNS : -DCDC : for constant duty cycle
;          -DDP : direct C excitation 
;          -DTC : probe in triple channel mode 
;          -Dlacq : aq is longer than 50 ms
;          or blank
;FnMODE: TPPI, States or States-TPPI
;ns: MIN. 4 (full:16)

;######################################################
;#                                                    #
;#  PDSD & DARR/RAD                                   #
;#  Szeverenyi, NM and Sullivan, MJ and Maciel, GE.   #
;#    Journ Magn Reson 47(3):462-475, 1982            #
;#  Takegoshi, K; Nakamura, S; Terao, T.              #
;#    Chem Phys Lett. 344:631-637, 2001               #
;#  CR Morcombe, V Gaponenko, RA Byrd, & KW Zilm      #
;#    J Am Chem Soc 2004, 126, 7196-7197.             #
;#                                                    #
;#  Adjust pl14 = wr on 1H channel for DARR           #
;#  Adjust pl14 = 0 W for PDSD                        #
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
;#                   Define Mixing                    #
;#  PDSD/DARR is Default, add Selective 90 w/ option  #
;######################################################

"l1=d8/TauR"          ; TauR = time of one rotor period
define pulse mixing
"mixing=(l1*TauR)"                        ; mixing = d8


"acqt0=-(p1*2/3.1416)-0.5u"

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
#include <DARR_prot.incl>
        ;PDSD/DARR mixing max. 1 sec 
#include <t1_prot.incl>
        ;check d0 and d30

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m  do:H
  d1

  trigg
  1u fq=0.0:H                     ;set 1H on resonance

if "l0>0"
{
  "d51=d0-2u"
}

;######################################################
;#              Cross/Direct Polarization             #
;######################################################

#ifdef DP
  (p1 pl1 ph10):C 
#else
  (p3  pl2 ph1):H
  (p15:sp41 ph2):C  (p15:sp40 ph0):H
#endif                                    /*end of DP*/

;######################################################
;#                   t1 evolution                     #
;######################################################

  (0.5u pl12):H

#ifdef TC          /* in case of triple channel mode */
if "l0>0"                /* dec. on 15N is turned on */
{
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
  0.5u cpds2:H  
  d51 
  0.5u do:H
}
#endif                                  /* end of TC */

;######################################################
;#           SPIN DIFFUSION (PDSD, DARR)              #
;######################################################

  (p1 pl1 ph3):C                  ;store  magn. along Z 
  (mixing ph20 pl14):H
  (p1 pl1 ph4):C  (0.5u pl12):H                ;readout

;#######################################################
;#                     Acquisition                     #
;#######################################################

  0.5u cpds2:H

gosc ph31            ;start ADC with ph31 signal routing

#ifdef CDC
  d30
#endif                                    /*end of CDC*/

1m do:H
lo to Start times ns

30m mc #0 to Start
  F1PH(calph(ph3, -90), caldel(d0, +in0) & caldel(d30, -in30) & calclc(l0, 1))

HaltAcqu, 1m
exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 0                   ; H CP spin lock
ph1 = 1 3                 ; H hard pulse
ph2 = {0}*8 {2}*8         ; C CP spin lock 
ph3 = 1 1 3 3             ; C 90 to store magn. along Z
ph4 = 1 1 2 2  3 3 0 0    ; C 90 read out

#ifdef DP 
ph10= 1 3 3 1  2 0 0 2    ; C 90 DP
#endif

ph20= 0                   ; N 90 hard pulse and H DARR mixing
ph21= 1                   ; N 180 pulse

ph31= 2 0 1 3  0 2 3 1 
      0 2 3 1  2 0 1 3    ; receiver

;#######################################################



;$Id:$
