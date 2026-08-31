# pp_selector — TopSpin 펄스 프로그램 추천 도구 설계

작성: 2026-08-31

## 1. 목적

TopSpin 안에서 실행되어 사용자에게 몇 가지 질문(시료 종류 → 실험 → 변형)을 던지고, 그 답에 맞는 Bruker 표준 펄스 프로그램과 파라미터셋을 **추천만** 한다. 데이터셋을 바꾸지 않는다. 지식의 근거는 Parella의 *Pulse Program Catalogue* Vol. I(1D/2D)·Vol. II(생체분자)와 라이브러리 파일 헤더다.

## 2. 결정 사항

| 항목 | 결정 |
|---|---|
| 실행 환경 | TopSpin 내부 Jython 2.7 스크립트(`edpy`). 3.x/4.x 공통 |
| 출력 동작 | 추천 표시만(`VIEWTEXT`). `rpar`/`PULPROG` 변경 없음 |
| 실험 범위 | 카탈로그 Vol. I 2–51장(소분자 1D/2D) + Vol. II 53–105장(생체분자). 고체(52장) 제외 |
| 방식 | 데이터 주도 결정 트리(`pp_tree.json`) + 얇은 러너(`pp_selector.py`) |
| UI 언어 | 영어(라벨은 json에만 있으므로 번역은 json 편집으로 가능) |

## 3. 파일 구성

```
src/pp_selector.py      Jython 2.7 러너. 외부 의존성 없음
src/pp_tree.json        결정 트리 + 리프 데이터
tools/build_leaves.py   CPython. 리프의 parset/desc/dim 자동 채움
tests/test_tree.py      CPython. 트리 무결성 + 러너 로직 검사 (프레임워크 없음)
README.md               설치(파일 2개 복사)·TopSpin 경로 확인 절차
```

배포: `src/pp_selector.py`, `src/pp_tree.json`을 `<TOPSPIN>/exp/stan/nmr/py/user/`에 복사. TopSpin 명령줄에서 `pp_selector`.

## 4. `pp_tree.json` 형식

```json
{
  "version": 1,
  "topspin_home": null,
  "root": "domain",
  "nodes": {
    "<node-id>": {
      "q": "질문 문장",
      "opts": [["라벨", "<node-id> 또는 L:<leaf-name>"], ...]
    }
  },
  "leaves": {
    "<pulseprogram-name>": {
      "parset":   "HNCOGP3D",            // 빌드 도구가 채움. 없으면 null
      "desc":     "HNCO, 3D sequence ...", // 빌드 도구: ;avance-version 다음 줄부터 최대 4행
      "dim":      "3D",                  // 빌드 도구: ;$DIM
      "requires": ["f3", "gradient", "13C/15N labeled"],  // 수동(f3/gradient는 빌드 도구가 제안)
      "notes":    "Set cnst21/cnst22; getprosol after rpar",  // 수동
      "alt":      ["hncogpwg3d", "trhncogp3d", "b_hncogp3d"]  // 수동. 형제 변형
    }
  }
}
```

규칙
- `opts` 대상이 `L:`로 시작하면 리프, 아니면 노드 ID. "Back"은 러너가 자동으로 붙이므로 `opts`에 넣지 않는다.
- 노드는 실험별로 따로 쓴다(공통 "위상 모드" 노드 재사용 금지 — 실험마다 존재하는 변형이 다르다).
- 트리는 카탈로그 챕터 순서를 따른다. 루트: `Small molecule (1D/2D)` / `Protein / nucleic acid (labeled)`.
- 첫 버전 리프 목표 150–200개. 카탈로그가 대표로 지목한 프로그램을 리프로, 형제는 `alt`에.
- 리프 이름·`alt` 이름은 모두 `doc/pulseprogram/`에 실제 존재해야 한다(테스트로 강제).

## 5. 러너 동작

```
main()
  tree = load_tree(<script dir>/pp_tree.json)
  result = walk(tree, ask_topspin)        # 리프 이름 또는 None(취소)
  if result: show(tree, result, answers)
```

- `walk(tree, ask)`: 노드 스택으로 진행. `ask(question, labels)`는 선택 인덱스(0..n-1) 또는 `-1`(Back)을 돌려준다. Back은 스택 pop, 루트에서 Back은 `None` 반환. 순수 함수 — TopSpin 없이 테스트 가능.
- `ask_topspin`: 옵션 ≤ 4개 → `SELECT(title, question, labels + ["Back"])`. 5개 이상 → `INPUT_DIALOG`의 콤보 박스 한 칸 + `["OK", "Back"]`.
- `check_local(name)`: `<TOPSPIN>/exp/stan/nmr/lists/pp/<name>` 존재 여부. TOPSPIN 경로는 `sys.registry.getProperty("XWINNMRHOME")` → `tree["topspin_home"]` → 없으면 확인 생략(`[not checked]`).
- `show`: `VIEWTEXT`에 아래 순서로 출력.

```
Recommended pulse program:  hncogp3d          [found in lists/pp | NOT found | not checked]
Parameter set:              HNCOGP3D   ->  rpar HNCOGP3D all ; getprosol
Dimension:                  3D
Requires:                   f3 channel, gradients, 13C/15N-labeled sample
Description:                <desc>
Notes:                      <notes>
Alternatives:               hncogpwg3d (found), trhncogp3d (found), b_hncogp3d (NOT found)
Your answers:               Protein > Backbone > HNCO > PEP
```
추천 프로그램이 없고 `alt` 중 존재하는 것이 있으면 그것을 "Alternatives" 맨 앞에 둔다. 파라미터셋이 `null`이면 그 줄에 `(no standard parameter set — start from a similar experiment)`.

오류
- json 파싱 실패, 노드/리프 ID 누락, `root` 없음 → `MSG("pp_selector: tree error: <detail>")` 후 종료.
- 사용자 취소(대화상자 닫기, 루트 Back) → 조용히 종료.

## 6. 호환성 규칙 (Jython 2.7)

- Python 2/3 겸용 문법만: `print()` 함수형, f-string·`pathlib`·타입 힌트 금지, 파일은 `io.open(path, encoding="utf-8")`.
- `from TopCmds import *`는 `try/except ImportError`로 감싸고 실패 시 콘솔 더미(`SELECT`→`input`, `VIEWTEXT`→`print`)로 대체. 러너 로직은 CPython에서 import·실행 가능해야 한다.
- 문자열은 ASCII로 유지(TopSpin 대화상자 폰트 문제 회피).

## 7. 빌드 도구 `tools/build_leaves.py`

- 입력: `src/pp_tree.json`, `doc/pulseprogram/`, `doc/ppcatalogue-1-programs.txt`, `doc/ppcatalogue-2-programs.txt`.
- 각 리프에 대해 `parset`(매핑 파일에서, 없으면 `null`), `desc`(헤더에서 `;avance-version` 다음 줄부터 빈 주석/`;$` 전까지 최대 4행), `dim`(`;$DIM`)을 채운다. `requires`가 비어 있으면 본문 `:f3` → `"f3"`, `:gp` → `"gradient"`를 제안값으로 넣는다.
- 이미 값이 있는 필드(`requires`, `notes`, `alt`, 그리고 수동으로 고친 `desc`)는 덮어쓰지 않는다. 덮어쓰려면 `--force`.
- 결과를 같은 파일에 정렬된 키로 다시 쓴다.

## 8. 테스트 `tests/test_tree.py` (`python3 tests/test_tree.py`)

1. 트리 무결성: 모든 `opts` 대상이 존재, 고아 노드 없음, 도달 불가 리프 없음, 순환 없음, 노드마다 `opts` ≥ 2.
2. 라이브러리 대조: 리프·`alt` 이름이 `doc/pulseprogram/`에 존재. `parset`이 매핑 파일과 불일치하면 실패(`null`은 허용).
3. 러너: 미리 정한 답 시퀀스로 `walk`를 실행해 예상 리프 도달(대표 경로 6개: zg30, cosygpppqf, hsqcedetgpsisp2.3, hmbcgplpndqf, hncogp3d, trhncogp3d), Back 한 단계, 루트 Back → `None`.
4. `show` 문자열 생성(가짜 `check_local`)이 위 서식과 일치.

## 9. 범위 밖 (첫 버전)

설치 스크립트, `rpar`/`PULPROG` 자동 적용, 자유 텍스트 검색, 고체 프로그램, 한국어 UI, 카탈로그에 없는 프로그램(라이브러리 나머지 ~1,400개)의 자동 분류.

## 10. 미확인 사항

- `sys.registry.getProperty("XWINNMRHOME")`이 대상 TopSpin 버전 전부에서 동작하는지. 실제 분광기 PC에서 1회 확인 필요. 실패해도 `[not checked]`로 동작한다.
