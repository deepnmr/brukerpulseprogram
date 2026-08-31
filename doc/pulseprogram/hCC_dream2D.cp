;hCC_dream2D.cp
;
;version: 1.0/ TS3.2 /2015/06/25
;
;written: veda 2014/07/01
;checked: veda 2015/06/25
;
;1D or 2D aliphatic 13C-13C correlation experiment using DREAM
;= Dipolar Recoupling Enhancement through Amplitude Modulation
;
;uses adiabatic scheme for transfer mainly between Ca and Cb, but
;other side chain carbons can be detected as well with an optimized offset
;-> depends on the B0 field and MAS rate!
;
;programmed with mc command: select dimension under Parmode to
;run 1D: (CXCX) or 2D: CX-CX
;
;13C 90deg hard pulse optimization, triple channel mode and long 
; acq. possible via 'ZGOPTNS'
;
;$CLASS=BioSolids
;$DIM=1D, 2D
;$TYPE=CPMAS and homonuclear
;$SUBTYPE=DQ filter, CP
;$COMMENT= HC CP with adiabatic CC DREAM mixing


;Avance II / AVIII version
;parameters:
;f1 : C
;f2 : H
;f3 : N (using '-DTC')
;o1 : C offset (center of aliphatics, no need to be equal to
;     Dream offset --> see cnst37)
;o2 : H offset (~ 3.5 ppm)
;o3 : N offset (~ 119 ppm)(using '-DTC')
;p1 : C 90 at pl1
;p3 : H 90 at pl2
;p15 : HC CP at sp41 (C) and sp40 (H)(1 to 3 ms)
;p21 : N 90 at pl21 (using '-DTC')
;p22 : N 180 at pl21 (using '-DTC')
;p37 : Dream contact time (3 to 7 ms)
;pl1 : C pulse power (using '-DC90')
;pl2 : H pulse power
;pl3 : not used
;pl12 : H dec power (e.g. 'spinal64_12nofq' or 'sltppm_12nofq')
;pl21 : N pulse power
;sp37 : C Dream power (~0.45 * MAS rate)
;sp40 : H HC CP power (e.g. n=1 HH condition)
;sp41 : C HC CP power (e.g. n=1 HH condition)
;d0 : incremented delay (t1)
;d1 : recycle delay; 1 to 5 times T1 (~ 2 to 3 s)
;d30 : extra time for constant duty cycle (-DCDC)
;pcpd2 : H dec pulse for cpdprg2 (~p4-0.2 us)
;cpdprg2 : H dec file (e.g. 'spinal64_12nofq', or 'sltppm_12nofq') at pl12
;spnam37 : C adiabatic shape during Dream transfer - depends on MAS rate!
;          (e.g. 'dream_vR13k_d2p5k.1000' used for 13 kHz MAS (= 'vR') and
;          an amplitude modulation of 2.5 kHz (= 'd'))
;          general shape parameters for Dream: Solids > TangAmpMod:
;          size of shape = 1000 (fix), amplitude of modulation = 2500 to 4500,
;          amplitude scaling factor = 100 % (fix), mean amplitude = 0.45 * MAS
;          rate, dipolar couplings = 1000 (fix)
;spnam40 : H shape (ramp up for HC CP, e.g. 'ramp.70100.1000')
;spnam41 : C shape (for HC CP, e.g. 'square.1000' (=no shape))
;spoffs37 : diff. from current carrier to DREAM freq. (in Hz, is calculated)
;spoal37: phaseramp alignment for spoffs37 for Dream (is set)
;cnst10 : C carrier frequency o1 (is set)
;cnst30 : expected td1 for use in constant duty cycle (-DCDC)
;cnst31 : MAS rotation rate in Hz
;cnst37 : C offset during DREAM transfer (e.g. between Ca-Cb or Cb-Cg)
;inf1 : 1/SW(C) = 2 * DW(C)
;in0 : = inf1
;in30 : = inf1 (-DCDC)
;l0 : loopcounter for F1 (C)
;ZGOPTNS : -DCDC : for constant duty cycle
;          -DC90  : optimize 13C 90 pulse (ZERO crossing)
;          -DTC : probe in triple channel mode (standard: double channel HC)
;          -Dlacq : acquisition times > 50ms
;          or blank
;FnMODE : States-TPPI (or TPPI, States)
;ns : MIN. 4 (full:4)


;######################################################
;#                                                    #
;#  DREAM - theoretical expertise                     #
;#  R. Verel et al. Chem Phys Lett 287:421-428, 1998  #
;#                                                    #
;#  DREAM - practical expertise                       #
;#  T. Westfeld et al. J Biomol NMR 53:103-12, 2012   #
;#                                                    #
;######################################################


prosol relations=<biosolCHN>


#include <CHN_defs.incl>
        ; defines which channel corresponds to which nucleus
#include <TauR_def.incl>
        ; definition of rotor period dependent delays

"p22=p21*2"

"cnst10=o1/bf1"                           ; o1=cnst10 for calc.
"spoffs37=bf1*((cnst37-cnst10)/1000000)"  ; offset during DREAM
"spoal37=0"                               ; phaseramp aligned at beginning
"acqt0=-(p1*2/3.1416)-0.5u"               ; baseopt correction

"in0=inf1"                  ;##########################
"d0=1u"                     ;#    t1_init => 0, 0     #
"in30=inf1"                 ;##########################
"l0=0"

define delay ONTIME         ;##########################
define loopcounter T1evo    ;#   Constant Duty Cycle  #
                            ;##########################
#ifdef CDC
"T1evo=larger(td1,cnst30)"
"d30=T1evo*(in30+1u)"
#else
"d30=1u"
#endif                      /* CDC */


                            ;##########################
;$EXTERN                    ;# python insertion point #
                            ;##########################


Prepare, ze

"d30=d30/2"

"ONTIME=aq+d0+d30+p15+p37" 

;######################################################
;#               Protections: Pre-Check               #
;######################################################

#ifdef lacq
#else
#include <acq_prot.incl>
        ;Max. 50 ms acquisition time
#include <ONTIME_prot.incl>
        ;total RF deposition restriction
#endif                 /* end of lacq */

#include <TauR_prot.incl>
        ;min. spinning 2.5 kHz
#include <p15bio_prot.incl>
        ;p15 max. 10 ms
#include <DREAMbio_prot.incl>
        ;p37 max. 10 ms

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m  do:H  

  0.5u fq=cnst10(bf ppm):C             ; C: on res.

#ifdef TC
  0.5u do:N
#endif                  /* end of TC */

  d1

if "l0>0"
{
  "d51=d0-1u"
}

;######################################################
;#              Cross/Direct Polarization             #
;######################################################

  (p3  pl2 ph1):H
  (p15:sp41 ph2):C  (p15:sp40 ph0):H

#ifdef C90             ; brings magn. to z
  (p1 pl1 ph22):C
#endif                 /* end of C90 */ 

;######################################################
;#                t1 evolution (C)                    #
;######################################################

  0.5u pl12:H
  0.5u cpds2:H

#ifdef TC             /* triple channel mode: dec. on 15N */
if "l0>0"
{
  (center (d51) (p21 pl21 ph20 p22 ph21 p21 ph20):N)
}
#else
if "l0>0"
{
  d51
}
#endif              /* end of TC */

  0.5u do:H

;######################################################
;#                   DREAM MIXING                     #
;######################################################

  (p37:sp37 ph3):C  (0.5u cpds2):H    ; Ca-Cb freq.
  0.5u do:H

;#######################################################
;#                     Acquisition                     #
;#######################################################

#ifdef TC
  0.5u cpds2:H pl16:N
  0.5u cpds3:N
#else
  0.5u cpds2:H
#endif

gosc ph31               ;start ADC with ph31 signal routing

#ifdef CDC
  d30
#endif                  /*end of CDC*/

1m do:H

#ifdef TC
1u do:N
#endif

lo to Start times ns

30m mc #0 to Start
  F1PH(calph(ph2, +90), caldel(d0, +in0) & caldel(d30, -in30) & calclc(l0, 1))

HaltAcqu, 1m
exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0=  0             ; H CP spin lock
ph1=  1 3           ; H hard pulse
ph2=  1 1 3 3       ; C CP spin lock
ph3=  1             ; C DREAM

ph31= 0 2 2 0       ; receiver

#ifdef TC
ph20= 0             ; N 90 hard pulse
ph21= 1             ; N 180 pulse
#endif

#ifdef C90 
ph22= 0             ;C hard pulse 
#endif              /* end of C90 */ 

;#######################################################



;$Id: $
