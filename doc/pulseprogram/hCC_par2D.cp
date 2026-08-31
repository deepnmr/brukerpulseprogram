;hChC_par2D.cp
;phase shifted C-[H]-C PAR
;
;version: 1.0/ TS3.5 /2015/05/29
;
;written: veda /2015/05/29
;checked: veda /2015/06/01
;
;Proton-Assisted Recoupling for CC distance measurements
;using a pi phase shift in the middle of the PAR mixing time
;on both channels at the same time
;
;recommendations:
;3.2 mm probe at 10 kHz MAS: ~ 47 kHz on 13C and 42 kHz on 1H
;3.2 mm probe at 20 kHz MAS: ~ 55 kHz on 13C and 47 kHz on 1H
;mixing time: 3 to 15 ms (the longer, the more remote signal)
;check references for detailed information
;
;programmed with mc command:
;select dimension under Parmode to run 'real nDs':
;1D C or 2D C-C
;
;triple channel mode and long acquisition possible via 'ZGOPTNS'
;
;$CLASS=BioSolids
;$DIM=1D, 2D
;$TYPE=Homonuclear
;$SUBTYPE=PAR, CP
;$COMMENT=CC PAR


;Avance II / AVIII version
;Parameters:
;f1 : C
;f2 : H
;f3 : N (using '-DTC')
;o1 : C offset (dependent on the region of interest)
;o1 : H offset (~ 3.5 ppm)
;o3 : N offset (~ 119 ppm)(using '-DTC')
;p1 : C 90 at pl1
;p2 : C 180 at pl1
;p3 : H 90 at pl2
;p4 : H 180 at pl2
;p10 : C-[H]-C PAR mixing at pl10 (H) and pl11 (C)(3 to 15 ms)
;p15 : HC CP at sp41 (C) and sp40 (H)(1 to 3 ms)
;PARMIX : p10/2 for phase shift (is set)
;p21 : N 90 at pl21 (using '-DTC')
;p22 : N 180 at pl21 (using '-DTC')
;pl1 : C pulse power
;pl2 : H pulse power
;pl10 : H power level PAR (see recommendations)
;pl11 : C power level PAR (see recommendations)
;pl12 : H high dec power during AQ
;pl16 : N Dec. during AQ (using 'waltz16_16nofq', 2 kHz)(using '-DTC')
;pl21 : N pulse power (using '-DTC')
;sp40 : H CP power (e.g. n=1 HH condition)
;sp41 : C CP power (e.g. n=1 HH condition)
;d0 : incremented delay (t1)
;d1 : recycle delay (t1)(1 to 5 times T1)
;d30 : extra time for constant duty cycle (-DCDC)
;pcpd2 : H dec pulse for cpdprg2 (~p4-0.2 us)
;pcpd3 : N dec pulse for cpdprg3 (125 us / 2 kHz)(using '-DTC')
;cpdprg2 : H dec file (e.g. 'spinal64_12nofq', or 'swftppm_12nofq') at pl12
;cpdprg3 : N dec file (e.g. 'waltz16_16nofq', (2 kHz)) at pl16 (using '-DTC')
;spnam40 : H shape (e.g. 'ramp70100.1000')
;spnam41 : C shape (e.g. 'square.1000' (=no shape))
;cnst30 : expected td1 for use in constant duty cycle (-DCDC)
;cnst31 : MAS rotation rate in Hz
;inf1 : 1/SW(C) = 2 * DW(C)
;in0 : =inf1
;in30 : =inf1 (-DCDC)
;l0 : loopcounter for F1 (C)
;ZGOPTNS : -DTC : use triple channel mode (default: double channel)
;          -DCDC : for constant duty cycle
;          -Dlacq : acquisition times > 50ms
;          or blank
;FnMODE : States-TPPI (or TPPI, States)
;ns : min. 4
;
;############################################################
;#                                                          #
;# PAR                                                      #
;# De Paepe, G. et al., JCP, 2008,129,245101 1-21.          #
;#                                                          #
;# Phase shifted PAR                                        #
;# Giffard, M. et al., PCCP, 2012,14,7246-55.               #
;#                                                          #
;############################################################


prosol relations=<biosolCHN>


#include <CHN_defs.incl>
        ; defines which channel corresponds to which nucleus

"p2=2*p1"
"p22=p21*2"

"acqt0=-(p1*2/3.1416)-0.5u" ; baseopt correction

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
#endif                      /* CDC */


define pulse PARMIX
"PARMIX=p10/2"               ; PAR mixing is parted because of the phase shift


                            ;##########################
;$EXTERN                    ;# python insertion point #
                            ;##########################

Prepare, ze

"d30=d30/2"

"ONTIME=aq+d0+d30+p15+2*p10"

;######################################################
;#               Protections: Pre-Check               #
;######################################################

#ifdef lacq
#else
#include <acq_prot.incl>
        ;Max. 50 ms acquisition time
#include <ONTIME_H_prot.incl>
        ;total RF deposition restriction to < 1 s
#endif                 /* end of lacq */

#include <p15bio_prot.incl>
        ;p15 max. 10 ms
#include <p10bio_prot.incl>
        ;p10 max. 30 ms

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m do:H 

#ifdef TC
1u do:N
#endif             /* end of TC */

  d1

if "l0>0"
{
  "d51=d0-1u"
}

;######################################################
;#         Initial excitation  and HC CP              #
;######################################################

  (p3  pl2 ph1):H
  (p15:sp41 ph2):C  (p15:sp40 ph0):H

;######################################################
;#                   t1 evolution  (C)                #
;######################################################

  0.5u pl12:H 

#ifdef TC  
if "l0>0"
{
  0.5u cpds2:H
  (center (d51) (p22 pl21 ph20):N)   ; to refocus scalar C-N couplings
  0.5u do:H
}
#else
if "l0>0"
{
  0.5u cpds2:H
  d51
  0.5u do:H
}
#endif             /* end of TC */

;######################################################
;#            C-[H]-C PAR mixing                      #
;######################################################

  (PARMIX pl11 ph3):C  (PARMIX pl10 ph3):H
  (PARMIX pl11 ph4):C  (PARMIX pl10 ph4):H        ; 180 deg phase shift

;#######################################################
;#                     Acquisition                     #
;#######################################################

#ifdef TC
  0.5u pl12:H  pl16:N
  0.5u cpds2:H  cpds3:N
#else
  0.5u pl12:H
  0.5u cpds2:H
#endif                   /* end of TC */

  gosc ph31             ;start ADC with ph31 signal routing

#ifdef CDC
  d30
#endif                   /* end of CDC */

#ifdef TC
1m do:H  do:N 
#else
1m do:H
#endif                   /* end of TC */

lo to Start times ns

30m mc #0 to Start
  F1PH(calph(ph2, +90), caldel(d0, +in0) & caldel(d30, -in30) & calclc(l0, 1))

HaltAcqu, 1m
exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 1                 ; 1H cp
ph1 = 0 2               ; 1H p90 excite
ph2 = 0 0 2 2           ; 13C in HC cp
ph3 = 0 0 0 0  2 2 2 2  ; 1st part PAR mixing on 1H and 13C
ph4 = 2 2 2 2  0 0 0 0  ; 2nd part PAR mixing on 1H and 13C

ph31= 0 2 2 0        ; receiver

#ifdef TC
ph20= 0              ; N 180
#endif               /* end of TC */

;#######################################################



;$Id:$
