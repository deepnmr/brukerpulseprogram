;hXcppi.cp

;JOS
;version: 3.0/ TS4.3.0 05/15/2023

;###############################################################
;#                                                             #
;#  H-X Cross Polarization Experiment                          #
;#                                                             #
;#  Adjust sp0, sp1, and p15 for maximum signal.               #
;#  The Hartman-Hahn CP condidition is B1(H)=B1(X)+-1          #
;#  sp0: usually a linear or tangenial amplitude ramped pulse  #
;#  sp1: usually a constant amplitude pulse           	       #
;#  p15: usually between 500-10000 us (sample-dependent)       #
;#  p16: 50 to 100us                                           #
;###############################################################
;
;$CLASS=SolidsIcon
;$DIM=1D
;$TYPE=CPMAS
;$SUBTYPE=editing
;$COMMENT=

prosol relations=<solids_ICON>
#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>

#include <Decoup_Solids.incl>

"spw2=plw0*pow((fieldH_corr*4*p0/(1000000*integfac2)),2)"

"acqt0=0"

Prepare, ze

#ifndef lacq
#include <acq_prot.incl>
        ;Max. 50 ms acquisition time
#endif
#ifndef longp15
#include <p15_prot.incl>
        ;p15 max. 10 ms
#endif

Start, 30m do:f2
  d1

  trigg
  (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph0):f2
  (p16:sp1 ph2):f1  (p16:sp2 ph4):f2

  go=Start ph31 cpds2:f2 finally do:f2
  1m do:f2
  30m mc #0 to Start F0(zd)

HaltAcqu, 1m
Exit, exit

ph0 = 0                ; H CP spin lock
ph1 = 1 3              ; H hard pulse
ph2 = 0 0 2 2 1 1 3 3  ; X CP spin lock 
ph3=  0
ph4=  2
ph10 = 2
ph31= 0 2 2 0 1 3 3 1  ; receiver



;basic cp editing experiment

;Avance NEO
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
;sp0    : H CP pulse shape
;sp1    : X CP pulse shape
;sp2	: H CP pulse shape
;cnst31 : MAS rate in Hz 
;cnst32  :1H decoupling field correction in Hz
;cnst33 : 1H spinlock field correction in Hz
;cnst34 : X spinlock field correction in Hz
;cnst50	: specified spinlock field Hz  X
;cnst51  : 1H homonuclear decoupling field in Hz for pl13 decoupling = cnst20 for FSLG experiments
;cnst52  : 1H heteronuclear decoupling field in Hz for pl12 decoupling
;cnst50	:90deg pulse for spinlock field Hz  (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 1H decoupling field in Hz for pl13 decoupling = cnst20 for FSLG experiments
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;pcpd2   : (calculated) used instead as pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : X amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DX90 : measure X 90 pulse (flipback after CP) 
;		   or -DfastMASDQ for fastMAS double quantum Cat fast MAS >20 kHz   
;          or -Dfastmas for x zero quantum CP 
;		   or -Dlgcp for Lee Goldburg CP
;		   or -Ddiy : do it yourself - set your own power levels
;          or -DINVERSE if the from nucleus should have the lower spinlock field than the "to (X)" nucleus
;          or -Dlacq : aq is longer than 50 ms
;          or blank
;		   for decoupling use:
;		       -Dlpdec for 1/4MASR low power decoupling
;			or -Dlpdec8 for 1/8th MASR low power decoupling
;			or -Dpidec for pi -pulse decoupling with pidecTAU_12 
;			or -Ddiydec for home built decoupling
;			or -DXiX for XiX decoupling



;$Id: hXcppi.cp,v 2.0  	Exp $
