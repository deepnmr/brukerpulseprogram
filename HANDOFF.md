# HANDOFF: pp_selector — TopSpin용 펄스 프로그램 추천 스크립트 (구현 완료, 잔여 검증 항목만 남음)

**Written:** 2026-09-01 · **Working dir:** `/Users/donghanlee/work/projects/brukerpulseprogram` · **Branch:** `main`

## Goal
TopSpin 명령줄에서 `pp_selector`를 치면 몇 가지 질문 후 Bruker 표준 펄스 프로그램 + 파라미터 세트를 추천하는 Jython 스크립트.
수용 기준(스펙 `docs/superpowers/specs/2026-08-31-pp-selector-design.md`):
- `python3 tests/test_tree.py` 전부 통과
- 실제 TopSpin 안에서 `pp_selector` 실행 → 추천 창에 `[found in lists/pp]` 표시
- Python 2/3 겸용 (Jython 2.7): f-string·pathlib·`io` 모듈·`__file__` 금지

## Status
**구현 계획 8개 Task 모두 완료, 커밋됨, 작업 트리 clean.** 이 handoff 작성 시점 기준 미커밋 변경 없음 (`git status` 비어 있음).

- 최신 커밋: `94eb388 fix: run under TopSpin 5 Jython (codecs.open instead of broken io; no __file__ lookup)`
- `python3 tests/test_tree.py` → `31 tests, 0 failed` (2026-09-01 재확인)
- 트리: 43 nodes / 191 leaves, `topspin_home` 미설정(None) — XWINNMRHOME 레지스트리로 자동 탐지됨
- 로컬 TopSpin 5.0.0(`/opt/topspin5.0.0`)에 배포된 `src/pp_selector.py`, `src/pp_tree.json` 사본이 HEAD와 **동일함**(`diff -q`로 확인)
- 2026-09-01 10:25 및 11:22(리팩터 `e69a623` 이후) TopSpin 5 실 GUI에서 `pp_selector` 실행 → `hncogp3d [found in lists/pp]` 확인, SELECT·INPUT_DIALOG·Back 경로 모두 동작 (명령 로그 `/opt/topspin5.0.0/prog/curdir/donghanlee/history_j.txt`)

남은 것은 아래 "Open questions"의 문서 정합성·미검증 항목뿐. 새 기능 요청은 없음.

## What worked
- 파일 읽기는 `codecs.open(path, encoding="utf-8")` (`src/pp_selector.py:24`) → TopSpin 5 Jython의 `io` 모듈은 `cannot import name BlockingIOError`로 깨져 있음. **[still applied]**
- `__file__` 참조 제거, `sys.argv[0]`으로 스크립트 경로 획득 → TopSpin이 `__file__` 조회 시 "Command cancelled"로 스크립트를 중단시킴(catchable NameError가 아님). **[still applied]**
- 설치 경로는 `sys.registry.getProperty("XWINNMRHOME")` (`src/pp_selector.py:89`), 실패 시 `pp_tree.json`의 `topspin_home` 폴백(`:92`). TopSpin 5에서 레지스트리 방식이 동작함. **[still applied]**
- 콘솔 폴백: stdin EOF를 cancel로 처리 (커밋 `719d940`). **[still applied]**
- 배포: `src/pp_selector.py` + `src/pp_tree.json`을 `/opt/topspin5.0.0/exp/stan/nmr/py/user/`에 복사하면 재시작 없이 인식됨(`PY_DIRS=py/user;py`).
- 리프 메타데이터(`parset`/`desc`/`dim`)는 `python3 tools/build_leaves.py`로 채움; `requires`/`notes`/`alt`만 수동 편집.

## What didn't work
- `sendgui`로 TopSpin에 명령 보내기 → macOS에서 `DYLD_LIBRARY_PATH`/`XWINNMRHOME` 설정해도 segfault. 재시도 금지. 대신 System Events로 창 unminimize(`AXMinimized` false) → `cliclick`으로 실제 클릭 → `pbcopy` + ⌘V로 명령 붙여넣기.
- AppleScript `click at` → Swing이 무시함. `cliclick` 써야 함.
- 명령을 키 입력으로 타이핑 → 한국어 IME가 `pp_selector`를 `ㅔㅔ_ㄴㄷㅣㄷㅊㅅㅐㄱ`로 바꿔 "Command not implemented". 반드시 클립보드 붙여넣기.
- `screencapture -R` 크롭 → 좌표 스케일이 전체 캡처와 달라 클릭 좌표 계산에 못 씀. 전체 `screencapture -x`(4608×1920 px, pt = px/2, 외장 모니터 2304×960 pt)만 사용.

## Key files & commands
- `src/pp_selector.py` — Jython 러너 (load/validate/walk, report, TopSpin 다이얼로그 어댑터, 콘솔 폴백, `main`). `:144` TopCmds import, `:171` 콘솔용 `INPUT_DIALOG` 대체, `:229` checker 생성.
- `src/pp_tree.json` — 결정 트리. `nodes`: `{"q", "opts": [["label", "node-id|L:program"]]}`, `leaves`: `{parset, desc, dim, requires, notes, alt}`. Back 옵션은 러너가 추가하므로 트리에 넣지 말 것.
- `tests/test_tree.py` — `python3 tests/test_tree.py` → 기대 출력 마지막 줄 `31 tests, 0 failed`.
- `tools/build_leaves.py` — `python3 tools/build_leaves.py` 로 리프 `parset`/`desc`/`dim` 재생성. 손으로 편집 금지.
- `python3 src/pp_selector.py` — 콘솔 스모크 테스트(숫자로 선택, 빈 입력 = Back).
- `docs/superpowers/specs/2026-08-31-pp-selector-design.md` — 스펙. `docs/superpowers/plans/2026-08-31-pp-selector.md` — 구현 계획, 체크박스 전부 [x].
- `README.md` — 설치/실행/트리 편집 안내.
- `CLAUDE.md` — 저장소 규칙 + pp_selector 절.
- 배포 대상: `/opt/topspin5.0.0/exp/stan/nmr/py/user/{pp_selector.py,pp_tree.json}`.
- 메모리: `~/.claude/projects/-Users-donghanlee-work-projects-brukerpulseprogram/memory/topspin5-local-install.md` — TopSpin 5 GUI 구동 절차 상세.

## Next steps
1. ~~스펙 `io.open` 문구~~ — 2026-09-01 `codecs.open` + `__file__` 금지로 갱신 (완료).
2. ~~README 버전 범위~~ — 2026-09-01 "TopSpin 3.x – 5.x"로 갱신, 5.0.0만 검증됨을 명시 (완료).
3. ~~`INPUT_DIALOG` Back 동작~~ — 2026-09-01 TopSpin 5 GUI에서 검증됨: 옵션 8개 노드(HNCO 목록)에서 Back → 이전 질문(Experiment class)으로 복귀. 어댑터 수정 불필요.
4. 실제 분광기 PC(TopSpin 3.x/4.x)에 배포 시 README 절차대로 `XWINNMRHOME` 값 확인. `None`이면 `pp_tree.json`에 `"topspin_home"` 추가.

## Open questions / risks
- TopSpin 3.x / 4.x 에서의 동작 — **unverified**(로컬에는 5.0.0만 있음). `codecs.open`/`sys.registry`는 구버전 Jython에서도 표준이라 동작할 것으로 예상하나 확인 안 됨.
- 계획 self-review에 기록된 판단: 리프 `hccconhgp3d2` 라벨 "HCC(CO)NH"는 카탈로그 변형 1/2/3 구분이 없어 임의 선택; `zg`는 1D 1H와 1D 13C 노드 양쪽에 의도적으로 중복.
- TopSpin GUI를 셸에서 자동 조작할 때는 위 "What didn't work"의 제약(sendgui 불가, IME, 좌표 스케일)을 반드시 따를 것.
