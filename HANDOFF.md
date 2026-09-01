# HANDOFF: pp_selector — TopSpin용 펄스 프로그램 추천 스크립트 (구현·검증·문서 정리 완료)

**Written:** 2026-09-01 · **Working dir:** `/Users/donghanlee/work/projects/brukerpulseprogram` · **Branch:** `main` (= `origin/main`, `c41e2d6`)

## Goal
TopSpin 명령줄에서 `pp_selector`를 치면 몇 가지 질문 후 Bruker 표준 펄스 프로그램 + 파라미터 세트를 추천하는 Jython 스크립트.
수용 기준(스펙 `docs/superpowers/specs/2026-08-31-pp-selector-design.md`):
- `python3 tests/test_tree.py` 전부 통과
- 실제 TopSpin 안에서 `pp_selector` 실행 → 추천 창에 `[found in lists/pp]` 표시
- Python 2/3 겸용 (Jython 2.7): f-string·pathlib·타입 힌트·`io` 모듈·`__file__` 금지

## Status
**할 일 없음.** 구현 계획 8개 Task 완료, 리팩터링 완료, TopSpin 5 GUI 검증 완료, 문서 정합성 항목 모두 정리. 작업 트리 clean (`git status` 비어 있음), 미커밋 변경 없음.

2026-09-01 이 세션에서 한 일 (모두 `main`에 merge됨):
- PR #1 `HANDOFF.md` 최초 작성 → 이 과정에서 GitHub 저장소 `deepnmr/brukerpulseprogram` (public) 신규 생성, `origin` 연결
- PR #2 `e69a623` `src/pp_selector.py` 리팩터링 (−57/+33줄): 옵션 범위 검증을 `walk` 한 곳으로 통합, `format_report`를 `(label, value)` 테이블로, `STATUS`/`ALT_STATUS` 병합, 미사용 `IN_TOPSPIN` 삭제. 출력·동작 동일
- TopSpin 5 GUI에서 리팩터 후 재검증 (11:22): Protein → Backbone assignment(3) → HNCO(1) → PEP(1) → `hncogp3d [found in lists/pp]`, alternatives 3개 `(found)`. SELECT(≤4옵션)·INPUT_DIALOG(5/8/11옵션)·INPUT_DIALOG의 **Back**(이전 질문 복귀) 모두 동작
- PR #3 `CLAUDE.md` pp_selector 절에 배포·GUI 검증·HANDOFF·리모트 4줄 추가
- PR #4 `README.md:7` "Install (TopSpin 3.x – 5.x)", `:16` "Verified on TopSpin 5.0.0 (macOS)…" 명시
- PR #5 스펙 `:96` `io.open` → `codecs.open`, `io`/`__file__` 금지 명시

현재 상태 확인값:
- `python3 tests/test_tree.py` → `31 tests, 0 failed`
- 트리: 43 nodes / 191 leaves, `topspin_home` 미설정 — XWINNMRHOME 레지스트리로 자동 탐지
- `/opt/topspin5.0.0/exp/stan/nmr/py/user/{pp_selector.py,pp_tree.json}` 가 HEAD와 동일 (`diff -q`)
- `gh` 활성 계정이 `deepnmr`로 바뀌어 있음 (원래 `dleess`). 다른 프로젝트에서 `gh auth switch --user dleess` 필요할 수 있음

## What worked
- `codecs.open(path, encoding="utf-8")` (`src/pp_selector.py:24`) — TopSpin 5 Jython의 `io`는 `cannot import name BlockingIOError`로 깨져 있음. **[still applied]**
- `__file__` 미참조, `globals().get("__file__") or sys.argv[0]` (`src/pp_selector.py:186` `_script_dir`) — 맨 `__file__` 조회는 "Command cancelled"로 스크립트를 중단시킴(NameError로 못 잡음). **[still applied]**
- `sys.registry.getProperty("XWINNMRHOME")` (`src/pp_selector.py:85`), 실패 시 `pp_tree.json`의 `topspin_home` 폴백 (`:88`). **[still applied]**
- `ask_topspin` (`src/pp_selector.py:174`)은 SELECT/INPUT_DIALOG 반환값을 그대로 넘기고 `walk`가 `choice is None or not 0 <= choice < len(labels)`로 Back 처리 — TopSpin에서 SELECT는 int 인덱스(Back 버튼 = len(labels)), INPUT_DIALOG는 `[text]` 또는 Back/닫기 시 `None` 반환함을 실기기로 확인. **[still applied]**
- 콘솔 폴백 (`src/pp_selector.py:136` `from TopCmds import …` ImportError 분기): stdin EOF = cancel. **[still applied]**
- 배포: 두 파일을 `/opt/topspin5.0.0/exp/stan/nmr/py/user/`에 복사만 하면 재시작 없이 반영 (`PY_DIRS=py/user;py`).
- TopSpin GUI 셸 자동 조작 (2026-09-01 검증된 절차):
  1. `osascript` System Events로 `java` 프로세스 창 `AXMinimized` false + frontmost
  2. 명령 입력: `printf 'pp_selector' | pbcopy` → `cliclick c:<명령창 x,y>` → `cliclick kd:cmd t:v ku:cmd`
  3. 제출: `osascript -e 'tell application "System Events" to tell process "java" to key code 36'` (이것만 됨)
  4. 다이얼로그 버튼: `cliclick m:X,Y; cliclick dd:X,Y; sleep 0.15; cliclick du:X,Y` (느린 클릭)
  5. 좌표: `screencapture -x` 전체(4608×1920 px) → `sips -Z 1600` → pt = 축소이미지 좌표 × 1.44 (외장 모니터 2304×960 pt)
  6. INPUT_DIALOG 숫자 입력: 텍스트 필드 클릭 → ⌘A → `pbcopy`+⌘V (숫자도 클립보드로)
  7. 진행 상태는 TopSpin 상태바 "pp_selector.py: in progress / finished"로 판단

## What didn't work
- `sendgui` → macOS에서 `DYLD_LIBRARY_PATH`/`XWINNMRHOME` 설정해도 segfault. 재시도 금지.
- AppleScript `click at` → Swing이 무시.
- `cliclick kp:return` / `kp:enter` → TopSpin 명령창에서 무시됨. `key code 36`만 동작.
- `cliclick c:X,Y` 빠른 클릭 → Swing 다이얼로그 버튼(SELECT 옵션, OK, Back)이 무시함. `dd`/`du` 느린 클릭 필요.
- 키 입력으로 명령 타이핑 → 한국어 IME가 `pp_selector`를 `ㅔㅔ_ㄴㄷㅣㄷㅊㅅㅐㄱ`로 변환("Command not implemented"). 클립보드 붙여넣기만.
- `screencapture -R` 크롭 → 스케일이 달라 클릭 좌표 계산에 못 씀(결과 창 캡처용으로만).
- System Events로 `window "pp_selector"`의 `buttons` 조회 → 신호등 3개만 나오고 Swing 버튼은 AX 트리에 없음. `static text`는 읽힘. 버튼 위치는 스크린샷으로.
- 스크립트 실행 직후 Enter를 한 번 더 보내면 첫 SELECT 다이얼로그가 닫혀 곧바로 "finished" 됨 — 제출은 한 번만.
- `history_j.txt`(`/opt/topspin5.0.0/prog/curdir/donghanlee/history_j.txt`)는 지연·누락이 있어 실행 여부 판단에 부적합.

## Key files & commands
- `src/pp_selector.py` — Jython 러너. `:22` `load_tree`, `:32` `validate_tree`, `:55` `walk`, `:82` `find_topspin_home`, `:103` `format_report`, `:136` TopCmds import/콘솔 폴백, `:174` `ask_topspin`, `:186` `_script_dir`, `:195` `main`.
- `src/pp_tree.json` — 결정 트리. `nodes`: `{"q", "opts": [["label", "node-id|L:program"]]}`, `leaves`: `{parset, desc, dim, requires, notes, alt}`. Back 옵션은 러너가 추가하므로 트리에 넣지 말 것.
- `tests/test_tree.py` — `python3 tests/test_tree.py` → 마지막 줄 `31 tests, 0 failed`.
- `tools/build_leaves.py` — `python3 tools/build_leaves.py` 로 리프 `parset`/`desc`/`dim` 재생성. 손으로 편집 금지.
- `python3 src/pp_selector.py` — 콘솔 스모크 (`printf '1\n2\n1\n1\n' | python3 src/pp_selector.py` → `noesyhsqcf3gpsi3d` 보고서).
- Jython 문법 검사: `grep -nE 'f"|f'"'"'|pathlib|-> |: (str|int|dict)\b|\bio\.' src/pp_selector.py` → 매치 없어야 함 (주석의 `__file__`만 예외).
- `docs/superpowers/specs/2026-08-31-pp-selector-design.md` — 스펙. `docs/superpowers/plans/2026-08-31-pp-selector.md` — 구현 계획(전부 [x]).
- `README.md`, `CLAUDE.md` — 사용자/에이전트 안내. CLAUDE.md pp_selector 절에 배포·검증 규칙 있음.
- 배포 대상: `/opt/topspin5.0.0/exp/stan/nmr/py/user/{pp_selector.py,pp_tree.json}` — `src/` 변경 후 항상 `cp`로 동기화.
- 메모리: `~/.claude/projects/-Users-donghanlee-work-projects-brukerpulseprogram/memory/topspin5-local-install.md` — GUI 조작 절차 상세(위 "What worked" 7단계와 동일 내용).
- 리모트: `https://github.com/deepnmr/brukerpulseprogram` (public). 흐름: 브랜치 → `gh pr create` → `gh pr merge --merge --delete-branch` → `git checkout main && git pull --ff-only`.

## Next steps
없음. 새 요청이 오면:
1. `src/` 수정 → `python3 tests/test_tree.py` → `/opt/topspin5.0.0/…/py/user/`에 `cp` → 다이얼로그 어댑터를 건드렸다면 위 GUI 절차로 재검증.
2. 실제 분광기 PC(TopSpin 3.x/4.x)에 배포할 때: README "One-time check" 절차로 `XWINNMRHOME` 확인, `None`이면 `pp_tree.json`에 `"topspin_home"` 추가.

## Open questions / risks
- TopSpin 3.x / 4.x 동작 — **unverified** (로컬은 5.0.0만). `codecs.open`/`sys.registry`/`TopCmds` API는 동일하므로 동작 예상.
- 저장소가 public이며 `doc/pulseprogram/` (Bruker 라이브러리)과 `doc/*.pdf` (Bruker 매뉴얼, Parella 카탈로그)가 포함됨 — 사용자가 저작권 우려를 듣고 public을 선택함. 문제가 되면 `gh repo edit deepnmr/brukerpulseprogram --visibility private`.
- 계획 self-review 판단: 리프 `hccconhgp3d2` 라벨 "HCC(CO)NH"는 카탈로그 변형 1/2/3 구분이 없어 임의 선택; `zg`는 1D 1H와 1D 13C 노드 양쪽에 의도적으로 중복.
- TopSpin 결과 창(VIEWTEXT `pp_selector`)이 11:22 검증 후 열린 채 남아 있을 수 있음. 닫아도 무방.
