;hNhC_pain2D.cp
;phase shifted C-[H]-C PAIN-CP
;
;version: 1.0/ TS3.5 /2015/05/29
;
;written: veda /2015/05/29
;checked: veda /2015/06/01 
;
;Proton-Assisted Insensitive Nuclei Cross Polarization
;for NC distance measurements
;using a pi phase shift in the middle of the PAIN mixing time
;on all channels at the same time
;
;recommendations:
;3.2 mm probe at 9 kHz MAS: ~ 40 kHz on 13C and 15N, 37.35 kHz on 1H
;3.2 mm probe at 20 kHz MAS: ~ 50 kHz on 13C and 15N, 62 kHz on 1H
;mixing time: 3 to 15 ms (the longer, the more remote signal)
;check reference for detailed information
;NOTE: if you cannot reach 50 kHz rf power on 15N, use slower
;spinning or slower rf amplitudes!
;
;programmed with mc command:
;select dimension under Parmode to run 'real nDs':
;1D C or 2D N-C
;
;$CLASS=BioSolids
;$DIM=1D, 2D
;$TYPE=Heteronuclear
;$SUBTYPE=PAIN, DCP
;$COMMENT=NC PAIN


;Avance II / AVIII version
;Parameters:
;f1 : C
;f2 : H
;f3 : N
;o1 : C offset (dependent on the region of interest)
;o1 : H offset (~ 3.5 ppm)
;o3 : N offset (~ 119 ppm)
;p1 : C 90 at pl1
;p2 : C 180 at pl1
;p3 : H 90 at pl2
;p4 : H 180 at pl2
;p9 : N-[H]-C PAIN mixing at pl9 (H), pl8 (C) and pl7 (N)(3 to 15 ms)
;PAINMIX : p9/2 for phase shift (is set)
;p21 : N 90 at pl21
;p22 : N 180 at pl21
;p25 : HN CP at sp43 (N) and sp42 (H)(1 to 3 ms)
;pl1 : C pulse power
;pl2 : H pulse power
;pl7 : N power level PAIN (equals rf amp. of 13C)(see recommendations above)
;pl8 : C power level PAIN (equals rf amp. of 15N)(see recommendations above)
;pl9 : H power level PAIN (see recommendations)
;pl12 : H high dec power during AQ
;pl16 : N Dec. during AQ (using 'waltz16_16nofq', 2 kHz)
;pl21 : N pulse power
;sp42 : H CP power (e.g. n=1 HH condition)
;sp43 : N CP power (e.g. n=1 HH condition)
;d0 : incremented delay (t1)
;d1 : recycle delay (t1)(1 to 5 times T1)
;d30 : extra time for constant duty cycle (-DCDC)
;pcpd2 : H dec pulse for cpdprg2 (~'p4-0.2' us)
;pcpd3 : N dec pulse for cpdprg3 (125 us / 2 kHz)
;cpdprg2 : H dec file (e.g. 'spinal64_12nofq', or 'swftppm_12nofq') at pl12
;cpdprg3 : N dec file (e.g. 'waltz16_16nofq', (2 kHz)) at pl16 (using '-DTC')
;spnam42 : H shape (e.g. 'ramp70100.1000')
;spnam43 : N shape (e.g. 'square.1000' (=no shape))
;cnst30 : expected td1 for use in constant duty cycle (-DCDC)
;cnst31 : MAS rotation rate in Hz
;inf1 : 1/SW(N) = 2 * DW(N)
;in0 : =inf1
;in30 : =inf1 (-DCDC)
;l0 : loopcounter for F1 (N)
;ZGOPTNS : -DCDC : for constant duty cycle
;          -Dlacq : acquisition times > 50ms
;          or blank
;FnMODE : States-TPPI (or TPPI, States)
;ns: min. 4
;
;############################################################
;#                                                          #
;# PAIN CP                                                  #
;# Lewandowski, J. et al., JACS, 2007,129(4),728-9.         #
;#                                                          #
;# Phase shifted PAIN CP                                    #
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
                            ;#           2D           #
                            ;##########################
#ifdef CDC2D
"T1evo=larger(td1,cnst30)"
"d30=T1evo*(in30+1u)"
#endif                      /* CDC2D */


define pulse PAINMIX
"PAINMIX=p9/2"              ; PAIN mixing is parted because of the phase shift

                            ;##########################
;$EXTERN                    ;# python insertion point #
                            ;##########################

Prepare, ze

"d30=d30/2"

"ONTIME=aq+d0+d30+p25+2*p9"

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

#include <p25bio_prot.incl>
        ;p25 max. 10 ms
#include <p9bio_prot.incl>
        ;p9 max. 30 ms

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m do:H do:N
  d1

if "l0>0"
{
  "d51=d0-1u"
}

;######################################################
;#         Initial excitation  and HN CP              #
;######################################################

  (p3  pl2 ph1):H
  (p25:sp43 ph2):N  (p25:sp42 ph0):H

;######################################################
;#                   t1 evolution  (N)                #
;######################################################

  0.5u pl12:H
  
if "l0>0"
{
  0.5u cpds2:H
  (center (d51) (p1 pl1 ph20 p2 ph21 p1 ph20):C)   ; to refocus scalar C-N couplings
  0.5u do:H
}

;######################################################
;#                  N-[H]-C PAIN CP                   #
;######################################################

  (PAINMIX pl9 ph3):H (PAINMIX pl8 ph3):C (PAINMIX pl7 ph3):N
  (PAINMIX pl9 ph4):H (PAINMIX pl8 ph4):C (PAINMIX pl7 ph4):N   ; 180 deg phase shift

;#######################################################
;#                     Acquisition                     #
;#######################################################

  0.5u pl12:H  pl16:N
  0.5u cpds2:H  cpds3:N

  gosc ph31        ;start ADC with ph31 signal routing

#ifdef CDC2D
  d30
#endif                                /* end of CDC2D */

1m do:H  do:N 
lo to Start times ns

30m mc #0 to Start
  F1PH(calph(ph2, +90), caldel(d0, +in0) & caldel(d30, -in30) & calclc(l0, 1))

HaltAcqu, 1m
exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 1                ; 1H cp
ph1 = 0 2              ; 1H p90 excite
ph2 = 0 0 2 2          ; 15N in HN cp
ph3 = 0 0 0 0  2 2 2 2 ; 1st part PAIN mixing on 1H, 13C, 15N
ph4 = 2 2 2 2  0 0 0 0 ; 2nd part PAIN mixing on 1H, 13C, 15N

ph31= 0 2 2 0       ; receiver

ph20= 0             ; C 90 hard pulse
ph21= 1             ; C 180 pulse
    
;#######################################################



;$Id:$
