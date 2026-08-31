# ppcatalogue-2.pdf 인용 펄스 프로그램 구조 분석

대상: `doc/ppcatalogue-2.pdf` — Teodor Parella, *Pulse Program Catalogue Vol. II: Biomolecular NMR Experiments* (NMRGuide, TopSpin 3.0, Bruker 2010, 527쪽, 카탈로그 쪽번호 467–993).
카탈로그 본문에서 인용된 펄스 프로그램 **525개**는 전부 `doc/pulseprogram/`에 존재한다(목록·파라미터셋 매핑: `doc/ppcatalogue-2-programs.txt`).

## 1. 카탈로그 자체의 구조

| 절 | 챕터 | 내용 |
|---|---|---|
| Protein NMR: 3D 기본 | 53–61 | 3D TOCSY/NOESY 동핵, X-edited TOCSY/NOESY, HCCH, HSQC-NOESY-HSQC, X-filtered |
| Backbone | 62–72 | HNCO, HNCA, HN(CA)CO, HN(CO)CA, sequential/intra-HNCA, HNCANNH, HA-detected |
| Backbone–Sidechain | 73–85 | CBCA(CO)NH, HN(CO)CACB, CBCANH, HNCACB, CC(CO)NH, HCC(CO)NH, HBHA(CO)NH |
| 기타 | 86–91 | APSY(`rd_`), SOFAST/BEST(`b_`, `sf`), MUSIC(`music_`), 방향족/메틸 선택, 13C-detected(`c_`) |
| Relaxation | 92–93 | HSQC/HNCO 기반 T1, T2, T1ρ, NOE, Rex (pseudo-3D) |
| Coupling constants | 94–101 | φ/ψ/ω/χ1/χ2 각도용 E.COSY·quantitative J, RDC(`gprc`), H-bond J(`gphb`) |
| Nucleic acids | 102–105 | HCN, HCNCH, HCP, HP, P-FIDS (`na_`) |

각 실험 항목의 지면 구성은 고정되어 있다:
1. **Experiment Description / Sample Requirements / Hardware Requirements / References**
2. 펄스 시퀀스 다이어그램 — 채널별(1H / 15N / 13C' / 13Cα / GZ)로 `p*`, `sp*`, `d*`, `pl*`, `gp*` 실제 파라미터 이름을 그림에 표기하고, 블록 ①–⑤에 coherence 흐름(`Iz → 2IxNz → 2NyCOz …`) 주석
3. **버전 목록** — `(pulseprogram | PARAMETERSET)` 형식. 예: `3D HNCO using TROSY (trhncogp3d | TRHNCOGP3D)`. 210개 프로그램에 파라미터셋 이름이 명시됨.
4. "Experiment Version vs Labeling Strategy" — 같은 실험의 변형을 고르는 기준(§4 참조)

## 2. 525개 프로그램의 공통 뼈대 (통계)

| 항목 | 값 |
|---|---|
| `$CLASS` | 전부 `HighRes` |
| `$DIM` | 3D 391, 2D 99, 4D 33, 5D 1, 6D 1 |
| `#include` | `Avance.incl` 525, `Delay.incl` 523, `Grad.incl` 511 — 사실상 항상 3종 세트 |
| `prosol relations` | `triple` 347, `triple_na` 61(핵산), `triple_c` 51(13C-detect), `triple2` 32 |
| `avance-version` | 21/09/15가 433개 (라이브러리 대개편 시점) |
| 획득 | `go=2` 511, `goscnp` 14(pseudo-3D 완화), `aqseq 321` 307 / `312` 80 |
| 위상 프로그램 | `ph31`(수신기) 525, `ph1`–`ph8` 대부분, `ph9`+ 는 EA/IPAP용 |

파일 배치는 라이브러리 표준과 같다: 헤더 주석 → `$` 태그 → `prosol relations` → `#include` → `"…"` 정의 → `aqseq` → 본문(`1 ze` … `exit`) → `ph*` → `;p1 : …` 파라미터 설명 → gradient 비율/파일 → `;preprocessor-flags` → `;$Id:$`.

### 예약 파라미터 슬롯 (triple-resonance 관례, `Relations.info`와 일치)

| 슬롯 | 의미 | 사용 파일 수 |
|---|---|---|
| `p1/p2`, `pl1` | f1(1H) 90°/180° | 453/446 |
| `p21/p22`, `pl3` | f3(15N) 90°/180° | 461/470 |
| `p3/p4`, `pl2` | f2(13C) hard 90°/180° (HCCH 계열) | 132/94 |
| `p13/p14` + `sp2/sp3` | C=O on-res 90°/180° shaped | 299/347 |
| `sp5` | Cα off-res 180° | 307 |
| `sp8` | C=O 90° 시간반전 (t1 뒤 back-transfer) | 295 |
| `sp7`, `sp13` | Cα on-res 180° / Cα·Cβ 광대역 180° | 160/136 |
| `p11` + `sp1`, `pl0` | 1H 물 flip-back(90° selective) | 352 |
| `p26`, `pl19` | DIPSI-2 1H 디커플링용 90° (`cpds1:f1`) | 214 |
| `p16`, `d16`, `gp1–gp5` | gradient 펄스/회복 | 501/503 |
| `pl16`, `cpd3:f3` | 획득 중 15N GARP | 311/327 |
| `pl12`, `cpd2:f2` | 13C 디커플링(HCCH, C-detect) | 160/162 |
| `pl17`, `cpd4:f4` | 2H 디커플링(`2h` 변형) | 64/64 |
| `d0/in0`, `d10/in10` | t1, t2 증분 (`in10=inf2/4`가 CT-N 표준) | 457/380 |
| `d20/in20`, `d29/in29`, `d30/in30` | semi-CT/CT 보조 증분 (증가·감소 짝) | 192/161/292 |
| `d21` 5.5m, `d23` 12m, `d26` 2.3m | 1/2J(NH), 1/4J(NCO), 1/4J'(NH) | 183/194/165 |
| `d11` 30m, `d12`, `d13` 4u | disk I/O, 전력 전환, 짧은 지연 | 522/276 |
| `cnst21/22/23` | C=O / Cα / Cβ(또는 Cali) offset ppm → `spoffs*` 계산 | 378/328/162 |
| `cnst62/63` | semi-CT용 `calph` 위상 보정 | 75/75 |
| `l0` | IPAP/S3E in-phase·anti-phase 교대 카운터 (`if "l0 %2 == 1"`) | 140 |

## 3. 3D triple-resonance 기준 구조 — `hncogp3d`

카탈로그가 "Understanding 3D Triple-Resonance Experiments" 장에서 표준으로 삼는 프로그램. 본문은 5개 블록으로 읽는다:

```
1 d11 ze / d11 pl16:f3            ; 준비
2 d11 do:f3                        ; 스캔 루프 헤드 (mc가 되돌아오는 지점)
3 d1 pl1:f1
  p1 ph1 · d26 · (center p2 / p22:f3) · d26 · (p1 ph2)      ① 1H→15N INEPT   Iz → 2IxNz
  p11:sp1 (물 flip-back) · p16:gp1
  (p21 ph3):f3 · d21 pl19 · p26 · DELTA2 cpds1:f1           ② N→CO 전이(J(NH) refocus와 병합, DIPSI-2 시작)
  (center p14:sp3 / p22) · d23 · (p21):f3 · p16:gp2
  (p13:sp2 ph4):f2 · d0 · (center p14:sp5 / p22) · d0        ③ CO 발전 t1 (Cα·N 디커플링)
  p14:sp3 · DELTA · p14:sp5 · (p13:sp8)
  p16:gp3 · cpds1
  (p21):f3 · d30 · sp5 · d30 · (center sp3 / p22 ph8)        ④ 15N constant-time 발전 t2
  d10 · sp5 · d29 · do:f1 · p26 · p16:gp4*EA · DELTA3        (d10↑ d29↑ d30↓ 가 CT를 유지)
  (center p1/p21 ph5) d26 (center p2/p22) d26 (center p1 ph2/p21 ph6) d26 (center p2/p22) d26
  p1 · DELTA1 · p2 · p16:gp5 · BLKGRAD                       ⑤ PEP(sensitivity-enhanced) back-transfer + EA 선택
  go=2 ph31 cpd3:f3                                          획득 (GARP 15N)
  d11 do:f3 mc #0 to 2
     F1PH(calph(ph4,+90), caldel(d0,+in0))                   t1: States-TPPI
     F2EA(calgrad(EA) & calph(ph6,+180), caldel(d10,+in10) & caldel(d29,+in29) & caldel(d30,-in30))  t2: Echo/Antiecho
exit
```

- `aqseq 321`: 획득 순서 t3, t2, t1 (F1=CO 안쪽 루프, F2=N 바깥 루프). `312`는 반대.
- `"td2=tdmax(td2,d30*2,in30)"`: CT 기간을 넘지 않도록 td2 상한 자동 계산.
- `"spoffs5=bf2*(cnst22/1000000)-o2"`: Cα shaped pulse 오프셋을 ppm 상수에서 계산. `music_`는 대신 `fq=cnst21(bf ppm):f2`로 캐리어 자체를 옮긴다.
- `DELTA*`는 `Delay.incl`에서 `define delay`만 되어 있고 각 프로그램이 `"DELTA1=p16+d16+d13+4u"`처럼 gradient·펄스 길이 보정용으로 정의한다.
- `EA`는 `Grad.incl`의 `define list<gradient> EA=<EA>` — `gp4*EA`가 echo/antiecho 부호를 바꾼다.

## 4. 변형 접미사가 코드에서 실제로 바뀌는 부분

`hncogp3d`와 diff한 결과. 변형은 **블록 ⑤(back-transfer)와 ①(준비)만 갈아끼우고 ②–④ 코어는 유지**하는 패턴이 지배적이다.

| 변형 | 이름 요소 | 코드에서의 차이 | `mc` 절 |
|---|---|---|---|
| PEP (기본) | `gp…3d` | 블록 ⑤ = 두 번의 retro-INEPT + `gp4*EA` | `F2EA` |
| WATERGATE | `wg` | 블록 ⑤ → `p11:sp1 ph6 · p16:gp4 · (center p2/p22) · sp1 · gp4` 3-9-19형 soft-hard-soft; EA 없음 | `F2PH` (States-TPPI) |
| TROSY | `tr` | ① 앞부분에 두 번째 INEPT 단(`DELTA1 · center p2/p22 · gp1 · sp1 · p1`); ② `cpds1` 1H 디커플링 삭제(`d23`만); ④ 앞에 `if "l0 %2==1" (p21 ph6) else (p21 ph7)` 로 TROSY 성분 선택; 획득 중 `cpd3` 없음 | `F2PH`/`F1I` |
| TROSY + EA | `tret` | `tr` 구조에 gradient EA 선택을 다시 도입 | `F2EA` |
| 2H 디커플링 | `2h` | 준비부에 `LOCKDEC_ON · LOCKH_ON · H2_PULSE · pl17:f4`, 루프에 `H2_LOCK · LOCKH_OFF`, Cα 발전 중 `cpd4:f4`, 끝에 `H2_LOCK` 복귀 | 동일 |
| BEST | `b_` | 모든 1H hard pulse → 아미드 선택 shaped(`p41:sp25` 90°, `p42:sp26` 180°, `p44:sp30`), `pl26` 저전력 15N 디커플링, `p29:gp3` 짧은 gradient, 물 flip-back 삭제(선택 여기라 불필요) | 동일 |
| intra-residue | `i` (`hncaigp3d`) | ② 뒤에 `fq=cnst21`로 캐리어를 CO로 옮긴 뒤 `sp2·sp5·sp3·sp8` CO 전이 블록을 삽입(N→CO→Cα), Cα 펄스는 `sp7`(on-res); 경로 `F3(N)→F2(C=O)→F2(Ca,t1)` | 동일 |
| 순차 | `seqtr…` | TROSY 기반으로 i−1 잔기 상관만 선택; t1·t2 모두 constant-time | `F2EA` |
| RDC/J | `gprc3d1–7`, `jc` | IPAP: `F1I(iu0,2)`로 in-phase/anti-phase 교대 획득, `l0` 짝수/홀수 분기 | `F1I` + `F1PH` + `F2EA` |
| H-bond J | `gphb` | `d23=66.6m`(기본 12m), `d25=16.6m` — long-range ²ʰJ(NC') 전이 | 동일 |
| 13C-detected | `c_…_ia3d`, `_s3` | 1H 경로 없이 `f2`(H) 여기 → `f3`(N) → `f1`(C') 검출; IPAP(`_ia`)/S3E(`_s3`) virtual decoupling을 `l0 %2` 분기로; `prosol triple_c`, `cpd2:f2` | `F1I`/`F2I` + `F1PH`(semi-CT: `d0↑ d20↑ d28↓ & calph(cnst63)`) |
| APSY | `rd_…_32` | `wr #0 if #0 zd` 수동 루프 + `id31/id32/dd42/id52`로 투영각(`cnst51`) 결정 — `mc` 매크로를 쓰지 않음 | (없음) |
| 완화 pseudo-3D | `hsqct1etf3gpsi3d`, `…t2…`, `…tr…`, `…noe…`, `…rex…` | `"TAU=vd_list-…"`, `goscnp` + `st vd_list.inc` + `lo to 3 times nbl` 인터리브, `ipp*`/`rppall` | `F1QF()` + `F2EA` |
| 핵산 | `na_` | HCN/HCP 경로(`F1(H6/8)→F2(C6/8)→F3(N1/9)`), `prosol triple_na`, 31P 채널 사용 | `F1PH & F2EA` |
| MUSIC | `music_<aa>_3d` | 아미노산 선택 CH2/CH3 필터 블록 + `LABEL_GLY`/`LABEL_CO` 등 `#ifdef`로 t1 핵 선택 | `#ifdef`로 `F1PH` 위상 부호가 갈림 |

## 5. `mc` 매크로 조합 규칙

- **CT 발전**: `caldel(d10,+in10) & caldel(d29,+in29) & caldel(d30,-in30)` — 증가 2개 + 감소 1개로 총 길이 고정.
- **semi-CT**: `caldel(d0,+in0) & caldel(d20,-in20) & calph(phN, cnst63)` — 위상 보정 상수 동반.
- **EA**: `calgrad(EA) & calph(phX,+180)` (+ 수신기 `ph31` 반전을 같이 넣는 파일도 있음).
- **IPAP/인터리브**: `F1I(iu0, 2)` 가 `F1PH` 앞에 온다(59개 파일).
- **pseudo-3D 완화**: `F1QF()` — t1 차원은 vd_list 인덱스일 뿐 위상/지연 증분 없음.
- 4D: `F3EA`/`F3PH` 추가(20/13개). 5D/6D 각 1개.

## 6. ZGOPTNS 전처리 플래그

`LABEL_CN`(72, 13C/15N 이중표지 시 추가 180°), `TRIMP`(37, trim pulse), `LABEL_CB`(33), `CALC_SP`(26, 13C spectral width × `cnst44`로 밴드선택 13C 펄스 자동 계산), `LABEL_CO`(13), `LABEL_ALA/VAL/GLY/GLU/…`(MUSIC 아미노산 선택), `LABEL_F1/F2`(13C-detect 인터리브 차원 선택). 각 파일 끝 `;preprocessor-flags-start … end` 블록에 설명이 있다.

## 7. 실무 요약

- 새 triple-resonance 시퀀스를 만들 때 출발점은 `hncogp3d`(PEP, `F2EA`) 또는 `hncogpwg3d`(WATERGATE, `F2PH`). 여기에 §4의 블록을 그대로 이식하면 라이브러리 관례와 일치한다.
- 파라미터 번호는 §2 표의 슬롯을 지켜야 `prosol relations=<triple>`과 카탈로그의 파라미터셋(`HNCOGP3D` 등)이 그대로 맞는다.
- 카탈로그(2010, TopSpin 3.0)와 현재 파일(대부분 21/09/15)은 본문 로직이 같고, 최신 파일은 `CALC_SP`, `td2=tdmax(...)`, `spoffs*` 계산 같은 편의 기능이 추가된 형태다.
