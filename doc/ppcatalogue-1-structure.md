# ppcatalogue-1.pdf 인용 펄스 프로그램 구조 분석

대상: `doc/ppcatalogue-1.pdf` — Teodor Parella, *Pulse Program Catalogue Vol. I: 1D & 2D NMR Experiments* (NMRGuide, TopSpin 3.0, Bruker 2010-04-15, 469쪽).
본문(1–460쪽)에서 인용된 펄스 프로그램 **596개**는 전부 `doc/pulseprogram/`에 존재한다(목록·파라미터셋: `doc/ppcatalogue-1-programs.txt`). Vol. II 분석은 `doc/ppcatalogue-2-structure.md`.

부록(461–468쪽)은 TopSpin 3.0 `exp/stan/nmr/lists/pp` 전체 목록(1,140개)이다. 그중 1,119개가 현재 라이브러리에 같은 이름으로 남아 있고, 사라진 21개는 QNP 프로브용 `…qn` 계열(`zgflqn`, `zghfigqn`, `hfcoqfqn`…), LC-NMR `…ft` 계열(`lc1pnft`, `lc1prft`), `preempgp.a/.dru`, `zghc`, `cosyqftf`, `lc2wetdcus`, `hoesyfhqfqnrv` 이다.

## 1. 카탈로그 자체의 구조

| 절 | 챕터 | 내용 |
|---|---|---|
| 1D | 1–10 | 펄스 보정·성능 테스트(`zg30`, `lsnh`…), 기본 1D(`zg` 계열), T1/T2, 선택 1D(`sel*`), 용매 억제, 다중 presat/LC-NMR(`lc*`), 19F, 2H, 1D gradient, ERETIC |
| 동핵 2D | 11–20 | COSY, DQF-COSY, SECSY, RELAY, TOCSY(`mlev`/`dipsi2`), NOESY, ROESY, MQ(`dqs`), J-resolved |
| X-검출 | 21–28 | INEPT, DEPT, APT/JMOD, HETCOR(`hx*`), HOESY, INADEQUATE, 디커플러 보정(`decp90`, `dec180`) |
| 역상관 2D | 29–47 | HMQC, HSQC(다수 변형), CT, 다중도 편집(`ed`), spin-state(`ia`/`ss`), TROSY, CRINEPT, IDIS, HMQC/HSQC-COSY/TOCSY/ROESY/NOESY, HMBC, long-range J 측정 |
| 기타 | 48–52 | ADEQUATE(`adeq*`), DOSY(`ste*`/`led*`), STD(`std*`), CLEANEX(`*cx*`), 고체 |

지면 구성 두 종류:
- **실험 페이지** — `Experiment Description / Sample Requirements / Hardware Requirements / References / Related Experiments / Processing` 뒤에 채널별 다이어그램, 그리고 `(pulprog | PARSET)` 버전 목록(`(zgdc30 / zgdc / zg0dc | C13CPD)`처럼 여러 프로그램이 한 파라미터셋을 공유). 1장(보정·테스트)은 `NMR Experiment / Basic Parameter Set / Pulse Program / Sample / Basic acquisition parameters / Analysis` 고정 서식.
- **NMR Building Blocks(46항목 + 부록 12항목)** — Vol. I의 핵심. 시퀀스를 재사용 가능한 블록으로 분해해 **실제 코드 조각을 다이어그램 옆에 병기**하고 coherence 상태(`2IxSz → 2IzSz`)를 주석. 기본 요소(pre-scan/read pulse/purge), 필터(T2·T1ρ·DQF·z·MQF·BIRD·G-BIRD·X-filter·low-pass·diffusion·STD 포화 루프), 선택 여기(SPFGE), 용매 억제(presat/WATERGATE 3-9-19/90°sel/ES W5/ES 180°sel/WET), 발전 기간(가변·CT·다중도 편집), 동핵 혼합(MLEV-17, z-filtered DIPSI-2, NOESY, ROESY, T-ROESY), 1H→X 전이(INEPT, refocused, trim, zz-purge, flip-back, CPMG-INEPT), X→1H(reverse INEPT, WATERGATE, PEP, S3, IPAP, TROSY, half-TROSY, clean-TROSY). 다이어그램 옆 코드는 라이브러리 파일 그대로이므로 `grep`으로 역추적 가능.

## 2. 596개 프로그램의 공통 뼈대 (통계)

| 항목 | 값 |
|---|---|
| `$DIM` | 2D 401, 1D 188, 3D 7 |
| `$CLASS` | `HighRes` 595 + `HighRes HWT` 1 |
| `#include` | `Avance.incl` 596(전부), `Delay.incl` 353, `Grad.incl` 321, `De.incl` 13(`ACQ_START(phref,phrec)` = `(de adc phrec syrec)(1u 1u phref:r)` — 명시적 획득 매크로), `Sysconf.incl` 6, `Daz.incl` 1 |
| `prosol relations` | 대부분 없음(`default` 관계 사용). `triple` 47, `lcnmr` 38, `triple2` 5 |
| `avance-version` | `12/01/11` 355개 — Vol. II와 달리 **절반 이상이 2012년판 그대로**; 21/09/15는 54개 |
| 획득 | `go=2` 570, `goscnp` 10(STD·완화 인터리브), `aqseq` 는 3D 6개뿐 |

Vol. II 대비 차이: `d1`(464)·`d0`(360)·`d11`(322)·`d16`(308)가 핵심 지연이고 `d21/d23/d26`(triple-resonance 상수)는 거의 없다. `cnst2`(1J(XH), 188개) → `"d4=1s/(cnst2*4)"`, `cnst13`(long-range J, 29개) → `"d6=1s/(cnst13*2)"`, `cnst11/12`(HSQC 다중도 편집·CT), `cnst17`(adiabatic 펄스 보정 인자)이 지배적.

### 예약 파라미터 슬롯 (1D/2D 관례)

| 슬롯 | 의미 | 사용 |
|---|---|---|
| `p1/p2`, `pl1` | f1 90°/180° | 578/372 |
| `p0` | 가변 flip angle 읽기 펄스 (`zg30`은 `p1→p0`만 다름) | 107 |
| `p3/p4`, `pl2` | f2(X) 90°/180° | 204/119 |
| `p21/p22`, `pl3` | f3 90°/180° | 75/107 |
| `p16`, `d16`, `gp1–3` | gradient·회복 (2D 249개가 gradient 사용) | 287 |
| `p17`, `pl10` | purge/trim 펄스 (`pp`, `TRIMP`) | 64/95 |
| `p27`, `pl18`, `d19` | 3-9-19 WATERGATE 조각 펄스(`p27*0.231/0.692/1.462`)·binomial 지연 | 34 |
| `p11`+`sp1`, `pl0` | 물 flip-back 90° 선택 | 48 |
| `p12`+`sp1/sp2`, `p13`+`sp5`, `p14`+`sp3` | 선택 180°(ES), Cα/adiabatic 180° | — |
| `p28` | trim 펄스(INEPT) | 72 |
| `p6/p7`, `p5` | MLEV/DIPSI 스핀락 조각(`p6`=90°, `p7`=180°, `p5`=60°) | 69/43 |
| `pl12`, `cpd2:f2` | 획득 중 X 디커플링(GARP) | 212/221 |
| `pl9`, `cw:f1` + `ph29` | presat (`zgpr`: `d12 pl9 · d1 cw:f1 ph29 · 4u do:f1 · d12 pl1`) | 67/78 |
| `pl16`, `cpd3:f3` | f3 디커플링 | 70/73 |
| `d2`(1/2J), `d4`(1/4J), `d6`(1/2 nJ) | INEPT/HMQC/HMBC 전이 지연 | 99/57/23 |
| `d8` | NOESY/ROESY 혼합 시간 | 30 |
| `d12` 20u, `d13` 4u, `d19`, `d20` | 전력 전환, 짧은 지연, binomial, 확산 Δ | 199/112/37/64 |
| `d0/in0` | t1 증분 — `in0=inf1/2`(222, States-TPPI/EA) 또는 `in0=inf1`(136, TPPI/QF) | 360 |
| `l1` | MLEV/DIPSI 혼합 루프 (`lo to 4 times l1` 69개) | — |
| `gp6/gp7/gp8`, `p30`, `p19` | DOSY 확산(`gp6*diff`) / spoil gradient | 38/27/19 |

## 3. 기준 구조 — 1D `zg`와 2D `hsqcetgpsi`

### 1D: `zg`에서 변형이 어떻게 자라는가 (diff 결과)

```
1 ze
2 30m
  d1
  p1 ph1
  go=2 ph31
  30m mc #0 to 2 F0(zd)      ← 1D도 mc 매크로(F0(zd))로 끝난다 (188개 중 179개)
exit
```
| 변형 | 코드 변화 |
|---|---|
| `zg30` | `p1 ph1` → `p0 ph1` (파라미터셋에서 p0 = 30° 길이) |
| `zgpr` | `d1` → `d12 pl9:f1 · d1 cw:f1 ph29 · 4u do:f1 · d12 pl1:f1` (CW presat) |
| `zgig` | 준비부 `d11 pl12:f2`, 루프 헤드 `do:f2`, `go=2 ph31 cpd2:f2` (inverse-gated: 획득 중만 디커플링) |
| `zgdc` | `zgig`와 같되 `d1` 중에도 `cpd2` 유지 (NOE 포함) |
| `zgcw` | `d11 pl26:f2 · d11 cw:f2` … `d11 do:f2` (CW 디커플링) |
| `zgesgp`, `p3919gp`, `zggpwg`, `zgcppr` | §5의 용매 억제 블록을 `p1` 뒤에 삽입 |

1D 파일 중 9개만 `wr #0`+`lo to` 수동 루프(예: `t1ir`의 `vd_list` + `lo to 1 times td1`, STD의 `stdlist`·`goscnp`·`nbl` 인터리브).

### 2D: `hsqcetgpsi` (Vol. I이 building-block 해설의 기준으로 쓰는 HSQC)

```
① INEPT       p1 · d4 · (center p2 / p4:f2) · d4 · p1 ph2            2IzSz
   [zz-purge]  p11:sp1 flip-back · p16:gp1                            (블록 38)
② t1          (p3 ph3):f2 · d0 · [center p2 / p22:f3 if LABEL_CN] · d0 · p16:gp1*EA   (블록 24: 1H 180°로 X-H 디커플링)
③ back        (center p1 / p3 ph4:f2) · d24 · (center p2/p4) · d24 · (center p1 ph2 / p3 ph5) · d4 · (center p2/p4) · d4   ← PEP (블록 41)
④ acq         p16:gp2 · BLKGRAD · go=2 ph31 cpd2:f2
   mc          F1EA(calgrad(EA) & calph(ph5,+180), caldel(d0,+in0) & calph(ph3,+180) & calph(ph6,+180) & calph(ph31,+180))
```
- `hsqcetgpsisp2.2`: hard `p4` → `p14:sp3`(adiabatic 180°, `cnst17` 보정), `p24:sp7`, gradient `gp3/gp4` 추가, `DELTA2–4`로 펄스 길이 보정. `LABEL_CN` 분기 삭제.
- `hsqcedetgpsisp2.3`: 여기에 `p31:sp18` 다중도 편집 블록(`DELTA · sp18 · p2 · DELTA5 · sp18`) 삽입 → CH/CH₃ 양, CH₂ 음.
- `.2/.3` 접미사 = 같은 실험의 개선 구현(펄스 길이 보정·gradient 배치), 파라미터셋은 공유.

## 4. `mc` 매크로 — 2D 위상 모드는 절 하나로 결정된다

| 절 | 파일 수 | 의미 / 짝이 되는 정의 |
|---|---|---|
| `F1QF(caldel(d0,+in0))` | 71(+83 파일) | magnitude(`qf`), `in0=inf1` |
| `F1PH(calph(phN,+90), caldel(d0,+in0))` | 82 | States-TPPI(`ph`), `in0=inf1/2`; `calph` 2–5개 = 90° 이동할 펄스 여러 개 |
| `F1EA(calgrad(EA) [& calph(phN,+180)], caldel(d0,+in0) & calph(…,+180)…)` | 36+28+19+8… | echo/antiecho(`et`), `gp1*EA` 또는 `gp1*-1*EA` 동반 |
| `F1EA(calgrad(EA1) & calgrad(EA2), …)` | 6 | 두 gradient 리스트(ADEQUATE 등) |
| `F1I(iu0, 2)` | 11 | IPAP/인터리브(`ia`, `jc`) |
| `F1QF(calgrad(diff))` | 9 | DOSY — t1 대신 gradient 세기 리스트 `diff` 증가 |
| `F1PH(calph(phN,+45)…)`, `+270` | 3/3 | 45° 증분 = MQ 필터 위상 계단 |
| (mc 없음) | 27 | `wr #0 if #0` + `ip*/id0` 수동 루프(2012년판 오래된 파일) |

## 5. Building block ↔ 코드 대응표 (카탈로그가 다루는 46개 중 주요 항목)

| 블록 | 이름 코드 | 코드 서명 (라이브러리에서 `grep` 가능) |
|---|---|---|
| Purge before d1 | `pp` | `d12 pl10:f1 · p17 ph3 · p17*2 ph4 · d1 pl1:f1` (`cosygpppqf`) |
| CW presat | `pr` | `d12 pl9:f1 · d1 cw:f1 ph29 · 4u do:f1` |
| shaped presat | `ps` | `3 p18:sp6:f1 ph29 · 4u · lo to 3 times l6 · d12 pl1:f1` (`zgps`) |
| WATERGATE 3-9-19 | `19`, `p3919` | `pl18 · p27*0.231 ph3 · d19*2 · p27*0.692 · … · p0*0.231 ph4` |
| WATERGATE 90°sel | `wg` | `p16:gp · p11:sp1 · p2 · p11:sp1 · p16:gp` |
| Excitation sculpting | `es`(W5 `w5`) | `p16:gp1 · (p12:sp1 ph:r) · p2 · p16:gp1 · TAU · p16:gp2 · sp1 · p2 · gp2` 이중 SPFGE (`noesyesgpph`, `stddiffesgp`) |
| WET | `wt` | `(p11:sp7)·gp21 · (p11:sp8)·gp22 · (p11:sp9)·gp23 · (p11:sp10)·gp24 · WETWAIT pl1:f1` 4단 shaped+gradient (`wet`) |
| Water flip-back | `fp` | INEPT 뒤 `4u pl0:f1 · (p11:sp1 ph1:r):f1 · 4u pl1:f1` |
| zz-purge gradient | (INEPT 내) | `p16:gp1 · d16` 직후 `(p3 ph):f2` |
| Trim pulse | `TRIMP` 플래그 | `#ifdef TRIMP (p28 ph1)` |
| Adiabatic 180° | `sp` | `p14:sp3`, `DELTA=…-cnst17*p24/2` |
| DQ filter | `df` | `p1 ph2 · d13 · p1 ph3` (`cosydfph`; gradient 버전은 `d13` 자리에 `p16:gp`) |
| z-filter | `zf` | `p17 ph26 · d13 · p6 ph2 · vd_list · p6 ph3`, `mc … F0(vd_list.inc & zd)` — 무작위 지연을 `vd_list`로 (`selmlzf`) |
| ZQ 억제 | `zs` | chirp `(p32:sp29 ph4):f1` + `p16:gp1` 인접 배치, 두 번째는 `p32*0.75:sp29` (`dipsi2gpphzs`) |
| BIRD | `bi` | `p1 · d2 · (center p2 / p4:f2) · d2 · p1 ph2 · d7`(회복 지연) (`hmqcbiph`) |
| Low-pass J filter | `lp`, `l2`, `l3` | `d2 · p3:f2 ph3 · d6 …` 1–3단 (`hmbcgplpndqf`: `d2=1/2J(cnst2)`, `d6=1/2·nJ(cnst13)`) |
| Multiplicity editing | `ed` | `p31:sp18`(또는 `p4`) + `DELTA5` (`hsqcedetgpsisp2.3`) |
| CT 발전 | `ct` | `d0 … d20` 짝, `"d20=d23-p16-d16-p14*1.5-…"`, `mc`에서 `caldel(d0,+in0) & caldel(d20,-in20)` (`hsqcctetgpsp`) |
| MLEV-17 / DIPSI-2 | `ml`, `di` | `;begin MLEV17 … lo to 4 times l1 … ;end MLEV17` 루프 안 `(p6 ph p7 ph p6 ph)` 조합 + `p5`(60°) 트레일러, `pl10` |
| NOESY 혼합 | `no` | `d8`(`noesygpph`: `p1 ph2 · d8 · p1 ph3`); ES 버전은 `d8` 안에 gradient |
| ROESY / T-ROESY | `ro`, `troesy` | `4u pl11:f1 · p15 ph2` CW 스핀락 (`roesyph`); T-ROESY(`troesyph`)는 `p1 ph3 · roesylist:f1(오프셋 리스트) · p15:sp10 ph2:r · p1 ph4` — shaped 오프레조넌스 스핀락 |
| PEP (si) | `si` | back-INEPT 2회 (`d24` + `d4`) |
| IPAP / S3 | `ia`, `ss` | `F1I(iu0,2)` + `if "l0 %2"` 분기 |
| TROSY | `tr` | 디커플링 없는 back-transfer, `ph` 선택 |
| DOSY | `ste`/`led` + `bp`/`1s,2s,3s` | `p30:gp6*diff · d16 · p2 · p30:gp6*-1*diff`(bipolar), `p19:gp7/gp8` spoil, `F1QF(calgrad(diff))`, `FLAG_BLK` |
| STD 포화 루프 | `std` | `6 (p42:sp9):f2 · lo to 6 times l5`, `stdlist:f2` on/off 교대, `goscnp`+`nbl` |
| Inversion recovery | `t1ir` | `p2 · vd_list · p1`, `wr #0 if #0 vd_list.inc`, `lo to 1 times td1` |
| Inverse-gated / gated | `ig`, `gd` | `cpd2` 위치만 다름(§3) |

## 6. ZGOPTNS 전처리 플래그

`LABEL_CN`(83, 13C/15N 표지 시 `p22:f3` 추가), `TRIMP`(71), `CALC_SPOFFS`(27, shaped pulse 오프셋 자동 계산), `FLAG_BLK`(23, DOSY·선택 1D에서 gradient 앰프 blanking 방식 선택), `WAITFORSTART`(10, 외부 트리거), `ERNSTANG`(8, Ernst angle 계산), `CALC_POWER`(3), `LABEL_N`, `LABEL_D0`, `NOLOCK`, `DOPREP`.

## 7. 실무 요약

- Vol. I의 가치는 실험 목록보다 **building block ↔ 코드 조각 대응**에 있다. 새 시퀀스는 §5 표의 서명을 라이브러리에서 `grep`해 해당 블록을 통째로 옮기면 된다.
- 1D/2D 프로그램은 `prosol relations`를 대부분 쓰지 않고 `default` 관계로 동작하므로 `Relations.info`의 `default` 열 슬롯(§2)을 지켜야 `getprosol`이 맞는다.
- 2D 위상 모드(qf/ph/et)는 `mc` 절 + `in0` 정의 + gradient `*EA` 세 가지가 한 세트다. 하나만 바꾸면 안 된다.
- 355개가 2012년판 그대로이므로 `.2/.3` 신판(adiabatic, `DELTA` 보정)이 있으면 그쪽을 출발점으로 삼는 것이 낫다.
