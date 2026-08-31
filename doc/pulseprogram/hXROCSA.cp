;hXROCSA.cp  (TopSpin 4.1)
;avance-version (21/06/23)

;JOS  10/10/2020
;ready for ICONNMR - pulse calculation

;recoupling of CSA: J.C.C. Chan, R. Tycko, J. Chem. Phys. 118, 8378-8389 (2003) 

;includes saturation recovery sequence (optional)

;common solutions for pulse from a = 0.0329 and b = 0.4671 
; tR duration,e.g. 100us at 10 kHz
; X pulse: w1 = gamma B1 = 4.28 * wR, p21 = ((1/wR) * 0.467) / 8
; power level automatically calculatedm change O1P to be close to resoancnes of interest
; 1H decoupling: w1 = gamma B1 = cnst51 >> 4.28 * wR  if possible 12*wR, avoid w1 (1H) 1.5, 2 or 2.5 * w1(13C)
; not as critircal for COOH groups 



;$COMMENT=ROCSA sequence by recoupling of CSA by POST, optional saturation recovery
;$CLASS=SolidsIcon
;$DIM=2D
;$TYPE=cross polarisation
;$SUBTYPE=CS-CSA correlation
;$OWNER=Bruker


prosol relations=<solids_ICON>

#include <Delay.incl>
#include <trigg.incl>
		; definition of external trigger output$
#include <hX_cp.incl>
#include <Decoup_Solids.incl>
#include <lgcalc.incl>

"plw13=plw0*pow(((cnst20+cnst39)*4*p0/1000000),2)"
"plw11=plw31*pow(((4.2817*cnst31/10000)),2)"

define loopcounter td1half
"l4=0"				; initial t1
"d21=0.0329/cnst31"
define delay trocsa_a
"trocsa_a=(1s/cnst31)*0.0329"	; start position of the POST
define delay trocsa_b
"trocsa_b=(1s/cnst31)*0.4671"
define pulse p90_rocsa
"p90_rocsa=((1s/cnst31)*0.4671)/8"	; 90 degree pulse for 13C
define pulse p270_rocsa
"p270_rocsa=3*p90_rocsa"
define pulse p360_rocsa
"p360_rocsa= ((1s/cnst31)*0.4671)/2" 
define delay rocsa_break
"rocsa_break=(1s/cnst31) * (1 - 2* (0.0329 + 0.4671))" ;start position for the second POST  ==0 in this implementation

"in4=(1s/cnst31)"
"in0=(1s/cnst31)"
"inf1=in0"
"acqt0=-p1*2/3.1416"

  ze
1 10m
2 100m do:f2
  saturate, d20
     (p3 pl12 ph1):f2
  lo to saturate times l20
  d1
1m rpp12
1m rpp13
 (p3 pl2 ph1):f2
  (p15:sp1 ph2):f1  (p15:sp0 ph10):f2
3 trocsa_a	cpds4:f2
  (p90_rocsa pl11 ph12):f1	; 90 deg pulse
  (p360_rocsa ph13):f1		  ; 360deg pulse
  (p270_rocsa ph12):f1		  ; 270deg pulse
  (p270_rocsa ph12):f1	    ; 270deg pulse
  (p360_rocsa ph13^):f1	    ; 360deg pulse
  (p90_rocsa ph12^):f1	      ; 90deg pulse
  trocsa_a 			          ; finish 1 tR
  lo to 3 times l4
  (p1 pl1 ph7):f1	    ; store
  0.5u do:f2		      ; trun off 1H decoupler
  
  (p1 ph8):f1 (1u cpds2):f2		      ; read out
  go=2 ph31 finally do:f2
  1m do:f2
  10m mc #0 to 1 F1PH(calph(ph7, +90), caldel(d0, +in0) & calclc(l4, l1))
exit

ph1 = 1 3		; 1H 90deg
ph2 = 0  	; 13C CP
ph7 = 3 1  	; z-store after t1
ph8 = 0 1 2 3 	; read out after z-filter
ph10= 0			; 1H CP
ph11= 2

#ifdef bign4
ph12=0 1 2 3                  ;90degs 
ph13=2 3 0 1                  ;360degs              ;360degs 
#elif bign3
ph12=(6) 0 2 4                 ;90degs 
ph13=(6) 3 5 1               ;360degs 
#else
ph12=0 2                  ;90degs (bign2)
ph13=2 0 				;360deg pulse
#endif 

ph31= 0 1 2 3
 


;Avance NEO/ AVIII version
;parameters:
;p1      : X 90 at pl1(acqt0-calculation & -DX90)
;p3      : H 90 at pl2
;p15     : HX CP at sp41 (f1,C) and sp40 (f2,H)
;pl1     : X 90 at pl1
;pl11		 : calculated based on MASR (cnst31)
;pl2     : H pulse power
;pl12    : H dec power
;pl30	: H for 10 kHz spinlock field
;pl31	: X for 10kHz spinlock field
;pl32	: H for 10 kHz spinlock field for decoupling
;sp0    : H CP power
;sp1    : X CP power
;cnst31 : MAS rate in Hz 
;cnst50	:90deg pulse for spinlock field Hz  (n-0.5)*MASR  for H (n+0.5)*MASR
;cnst51  : 1H decoupling field during recoupling high! 3* 13C Field if possible!
;cnst52  : 1H heteronuclear decoupling in Hz pl12 decoupling during data acquisition
;cnst53  : fast MAS CP condition n*cnst53*MASR condition for C and n*(cnst53+1) for H (ZQ) or n*(1-cnst53) for DQ cnst53 <1!
;cnst54	 : calculated  cnst54=cnst53+1 or cnst54=1-cnst53 
;d1      : recycle delay; 1 to 5 times T1
;l1:  l1 = 2 for bign 2 and 4 l1=3 for bign3
;in4: =(1s/cnst31)
;in0: =(1s/cnst31)
;d20 : pulse spacing in saturation comb (optional)
;pcpd2   : not used, p31 (calculated) used instead as pulse length in decoupling sequence cpdprg2
;cpdprg2 : spinal64, swftppm, tppm etc. decoupling program
;spnam0 : H amplitude modulated contact pulse use tan modulated pulse tanhc60 or tanhc
;spnam1 : C amplitude modulated contact pulse use constant amplitude square.100
;ZGOPTNS : -DX90 : measure X 90 pulse (flipback after CP) -DfastMASDQ  or -DfastmasZQ for double or zero quantum CP at fast MAS >20 kHz
;					-Ddiy : do it yourself - set your own power levels
;          -Dlacq : aq is longer than 50 ms
;          or blank



;$Id: $
