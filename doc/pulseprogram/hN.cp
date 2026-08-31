;hN.cp

;version: 1.0/ TS3.2 /2013/06/17

;written for BioToolkit: JOS WTF 10.5.2013
;double-checked: VEDA SEWE July 2013

;basic cp experiment

;Avance II / AVIII version 
;parameters:
;p3      : H 90 at pl2
;p21     : N 90 at pl21 (-DN90)
;p25     : HN CP at sp43 (f1,N) and sp42 (f2,H)
;pl1     : not used, C 90 power
;pl2     : H pulse power
;pl12    : H dec power
;pl21    : N pulse power (-DN90)
;sp42    : H CP power
;sp43    : N CP power
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam42 : H ramp use e.g. ramp.10070 for variable amplitude CP
;spnam43 : N ramp use e.g. square.100 for square pulse CP
;ZGOPTNS : -Dlacq : aq is longer than 50 ms
;          -DN90 : optimize N 90 power
;          or blank

;############################################################
;#                                                          #
;#  H-N Cross Polarization Experiment                       #
;#                                                          #
;#  Adjust sp42, sp43, and p25 for maximum signal.          #
;#  The Hartman-Hahn CP condidition is B1(H)=B1(N)+-1       #
;#  sp42 is usually a linear or tangential ramp             #
;#  sp43 is usually a rectangular constant amplitude pulse  #
;#  p25: usually between 500-2000us (sample-dependent)      #
;#                                                          #
;############################################################


;$COMMENT= HN CP
;$CLASS=BioSolids
;$DIM=1D
;$TYPE=CPMAS
;$SUBTYPE=Setup


prosol relations=<biosolNHC>

#include <NHC_defs.incl>
        ; defines which channel corresponds to which nucleus
#include <trigg.incl>
        ; definition of external trigger output

define delay ONTIME

"acqt0=-(p21*2/3.1416)-1u"

"ONTIME=aq+p25"

                            ;##########################
;$EXTERN                    ;# Python insertion point #
                            ;##########################
Prepare, ze

;######################################################
;#               Protections: Pre-Check               #
;######################################################

ONTIME

#ifndef lacq
#include <acq_prot.incl>
        ;Max. 50 ms acquisition time
#include <ONTIME_prot.incl>
        ;total RF deposition restriction
#endif

#include <p25bio_prot.incl>
        ;p25 max. 10 ms

;######################################################
;#           Start of Active Pulse Program            #
;######################################################

Start, 30m do:H
  d1

  trigg
  1u fq=0.0:H                     ;set 1H on resonance

  (p3  pl2 ph1):H
  (p25:sp43 ph2):N  (p25:sp42 ph0):H
  (1u pl12):H

#ifdef N90 
  (p21 pl21 ph3):N  
#endif                                 /*end of N90*/ 

  1u cpds2:H  

;#######################################################
;#                     Acquisition                     #
;#######################################################

  go=Start ph31
  10m do:H 
  30m mc #0 to Start F0(zd)
  
HaltAcqu, 1m
Exit, exit

;#####################################
;#             Phase Cycle           #
;#####################################

ph0 = 0                ; H CP Spin lock
ph1 = 1 3              ; H Hard Pulse
ph2 = 0 0 2 2 1 1 3 3  ; N CP Spin Lock 

#ifdef N90 
ph3 = 1 1 3 3 2 2 0 0  ; N hard pulse
#endif                     /*end of N90*/ 


ph31= 0 2 2 0 1 3 3 1  ; receiver

;#######################################################



;$Id:$
