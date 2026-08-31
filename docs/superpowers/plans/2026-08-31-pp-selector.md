# pp_selector Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A TopSpin-internal Jython script that asks the user a few questions and recommends a Bruker standard pulse program + parameter set, driven by an editable JSON decision tree.

**Architecture:** `src/pp_selector.py` is a thin runner: a pure `walk(tree, ask)` state machine plus a TopSpin dialog adapter and a `VIEWTEXT` report. All knowledge lives in `src/pp_tree.json` (nodes = questions, leaves = programs). `tools/build_leaves.py` (CPython) fills leaf metadata from the library headers and the catalogue mapping files. `tests/test_tree.py` (CPython, no framework) validates tree integrity, library existence, and runner paths.

**Tech Stack:** Jython 2.7 (TopSpin 3.x/4.x) for the runner — Python 2/3-compatible syntax only; CPython 3 for tools/tests; stdlib only.

**Spec:** `docs/superpowers/specs/2026-08-31-pp-selector-design.md`

## Global Constraints

- Runner (`src/pp_selector.py`) must run under Jython 2.7 **and** CPython 3: `from __future__ import print_function`, no f-strings, no `pathlib`, no type hints, files opened with `io.open(path, encoding="utf-8")`.
- Runner strings are ASCII only (TopSpin dialog font safety).
- `from TopCmds import *` wrapped in `try/except ImportError` with console fallbacks so the runner imports under CPython.
- Tool never modifies the dataset: no `rpar`, no `PUTPAR`, no `XCMD`.
- Every leaf name and every `alt` name must exist as a file in `doc/pulseprogram/` (enforced by tests).
- Option targets: `"L:<name>"` = leaf, anything else = node id. "Back" is added by the runner, never listed in `opts`.
- Nodes are per-experiment; do not share "phase mode"/"solvent" nodes between experiments.
- Tests run with `python3 tests/test_tree.py` from the repo root; exit code 0 = pass.
- Commit after every task; author `Donghan Lee <kbsi.bionmr@gmail.com>` is already configured by the first commit's flags — use `git -c user.name='Donghan Lee' -c user.email='kbsi.bionmr@gmail.com' commit ...` if no global identity is set.

---

## File Structure

| File | Responsibility |
|---|---|
| `src/pp_selector.py` | Runner: `load_tree`, `validate_tree`, `walk`, `find_topspin_home`, `make_checker`, `format_report`, `ask_topspin`, `main`. Only file copied to TopSpin together with the JSON. |
| `src/pp_tree.json` | Decision tree + leaf data. Authored by hand (nodes, `requires`/`notes`/`alt`), filled by the build tool (`parset`/`desc`/`dim`). |
| `tools/build_leaves.py` | CPython. Reads library headers + `doc/ppcatalogue-{1,2}-programs.txt`, fills empty leaf fields, rewrites JSON with sorted keys. |
| `tests/test_tree.py` | CPython. `test_*` functions with plain `assert`; a `main()` that runs them all and prints a summary. |
| `README.md` | Install (copy two files), run (`pp_selector`), one-time check of `XWINNMRHOME`, how to edit the tree. |

---

### Task 1: Runner core — `load_tree`, `validate_tree`, `walk`

**Files:**
- Create: `src/pp_selector.py`
- Create: `tests/test_tree.py`

**Interfaces:**
- Produces:
  - `LEAF_PREFIX = "L:"`, `BACK = -1`
  - `class TreeError(Exception)`
  - `load_tree(path) -> dict` (raises `TreeError`)
  - `validate_tree(tree) -> None` (raises `TreeError`)
  - `walk(tree, ask) -> (leaf_name_or_None, answers_list)`; `ask(question, labels) -> int index or BACK`

- [ ] **Step 1: Write the failing tests**

`tests/test_tree.py`:

```python
"""Tests for pp_selector. Run: python3 tests/test_tree.py"""
from __future__ import print_function
import io
import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "src"))

import pp_selector as pp  # noqa: E402

TREE_PATH = os.path.join(ROOT, "src", "pp_tree.json")
PP_DIR = os.path.join(ROOT, "doc", "pulseprogram")

MINI = {
    "version": 1,
    "topspin_home": None,
    "root": "domain",
    "nodes": {
        "domain": {"q": "Sample?", "opts": [["Small", "sm"], ["Bio", "L:hncogp3d"]]},
        "sm": {"q": "Exp?", "opts": [["1D", "L:zg30"], ["COSY", "L:cosygpqf"]]},
    },
    "leaves": {
        "hncogp3d": {"parset": "HNCOGP3D", "dim": "3D", "desc": "HNCO",
                      "requires": ["f3", "gradient", "13C/15N labeled"],
                      "notes": "Set cnst21/cnst22", "alt": ["trhncogp3d", "b_hncogp3d"]},
        "zg30": {"parset": "PROTON", "dim": "1D", "desc": "1D 30 deg", "requires": [], "notes": "", "alt": ["zg"]},
        "cosygpqf": {"parset": None, "dim": "2D", "desc": "COSY", "requires": ["gradient"], "notes": "", "alt": []},
    },
}


def scripted(answers):
    """ask() that returns the given indices in order."""
    seq = list(answers)

    def ask(question, labels):
        assert isinstance(question, str) and labels, (question, labels)
        return seq.pop(0)
    return ask


def test_walk_reaches_leaf():
    leaf, answers = pp.walk(MINI, scripted([0, 1]))
    assert leaf == "cosygpqf", leaf
    assert answers == ["Small", "COSY"], answers


def test_walk_leaf_at_root():
    leaf, answers = pp.walk(MINI, scripted([1]))
    assert leaf == "hncogp3d"
    assert answers == ["Bio"]


def test_walk_back_one_level():
    leaf, answers = pp.walk(MINI, scripted([0, pp.BACK, 1]))
    assert leaf == "hncogp3d", leaf
    assert answers == ["Bio"], answers


def test_walk_back_at_root_cancels():
    leaf, answers = pp.walk(MINI, scripted([pp.BACK]))
    assert leaf is None and answers == []


def test_validate_rejects_unknown_target():
    bad = json.loads(json.dumps(MINI))
    bad["nodes"]["sm"]["opts"][0][1] = "L:nope"
    try:
        pp.validate_tree(bad)
    except pp.TreeError as e:
        assert "nope" in str(e)
    else:
        assert False, "expected TreeError"


def test_validate_rejects_missing_root():
    bad = json.loads(json.dumps(MINI))
    bad["root"] = "ghost"
    try:
        pp.validate_tree(bad)
    except pp.TreeError:
        pass
    else:
        assert False, "expected TreeError"


def test_validate_rejects_single_option():
    bad = json.loads(json.dumps(MINI))
    bad["nodes"]["sm"]["opts"] = [["1D", "L:zg30"]]
    try:
        pp.validate_tree(bad)
    except pp.TreeError:
        pass
    else:
        assert False, "expected TreeError"


def main():
    tests = [(n, f) for n, f in sorted(globals().items()) if n.startswith("test_") and callable(f)]
    failed = 0
    for name, fn in tests:
        try:
            fn()
            print("ok   ", name)
        except Exception as e:  # noqa: BLE001
            failed += 1
            print("FAIL ", name, "-", repr(e))
    print("%d tests, %d failed" % (len(tests), failed))
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `python3 tests/test_tree.py`
Expected: `ImportError: No module named pp_selector` (traceback, exit 1)

- [ ] **Step 3: Write the runner core**

`src/pp_selector.py`:

```python
"""pp_selector - recommend a Bruker pulse program by asking questions.

Runs inside TopSpin (Jython 2.7, 3.x/4.x) via the `edpy` mechanism and
also imports under CPython 3 for testing. Knowledge lives in pp_tree.json
next to this file. This tool never modifies the dataset.
"""
from __future__ import print_function
import io
import json
import os
import sys

LEAF_PREFIX = "L:"
BACK = -1


class TreeError(Exception):
    pass


def load_tree(path):
    try:
        with io.open(path, encoding="utf-8") as f:
            tree = json.load(f)
    except (IOError, OSError, ValueError) as e:
        raise TreeError("cannot read %s: %s" % (path, e))
    validate_tree(tree)
    return tree


def validate_tree(tree):
    nodes = tree.get("nodes")
    leaves = tree.get("leaves")
    if not isinstance(nodes, dict) or not isinstance(leaves, dict):
        raise TreeError("'nodes' and 'leaves' must be objects")
    root = tree.get("root")
    if root not in nodes:
        raise TreeError("root %r is not a node" % (root,))
    for nid, node in nodes.items():
        opts = node.get("opts")
        if not node.get("q") or not isinstance(opts, list) or len(opts) < 2:
            raise TreeError("node %r needs 'q' and at least 2 'opts'" % (nid,))
        for opt in opts:
            if not (isinstance(opt, list) and len(opt) == 2):
                raise TreeError("node %r: option must be [label, target]" % (nid,))
            target = opt[1]
            if target.startswith(LEAF_PREFIX):
                if target[len(LEAF_PREFIX):] not in leaves:
                    raise TreeError("node %r -> unknown leaf %r" % (nid, target))
            elif target not in nodes:
                raise TreeError("node %r -> unknown node %r" % (nid, target))


def walk(tree, ask):
    """Walk the tree. ask(question, labels) returns an index or BACK.

    Returns (leaf_name, answers) or (None, []) when cancelled.
    """
    stack = [tree["root"]]
    answers = []
    while stack:
        node = tree["nodes"][stack[-1]]
        labels = [opt[0] for opt in node["opts"]]
        choice = ask(node["q"], labels)
        if choice is None or choice < 0 or choice >= len(labels):
            stack.pop()
            if answers:
                answers.pop()
            continue
        label, target = node["opts"][choice]
        answers.append(label)
        if target.startswith(LEAF_PREFIX):
            return target[len(LEAF_PREFIX):], answers
        stack.append(target)
    return None, []
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `python3 tests/test_tree.py`
Expected: 7 lines starting with `ok`, then `7 tests, 0 failed`

- [ ] **Step 5: Commit**

```bash
git add src/pp_selector.py tests/test_tree.py
git commit -m "feat: pp_selector runner core (load/validate/walk) with tests"
```

---

### Task 2: Report formatting and local library check

**Files:**
- Modify: `src/pp_selector.py` (append after `walk`)
- Modify: `tests/test_tree.py` (append tests before `main`)

**Interfaces:**
- Consumes: `MINI` fixture from Task 1.
- Produces:
  - `find_topspin_home(tree) -> str or None`
  - `pp_dir(home) -> str`
  - `make_checker(home) -> callable(name) -> True/False/None`
  - `format_report(tree, name, answers, exists) -> str`
  - `STATUS`, `ALT_STATUS` dicts keyed by `True/False/None`

- [ ] **Step 1: Write the failing tests**

Append to `tests/test_tree.py` (above `def main`):

```python
def _exists_all(name):
    return True


def _exists_none(name):
    return False


def _exists_unknown(name):
    return None


def test_report_found():
    text = pp.format_report(MINI, "hncogp3d", ["Bio"], _exists_all)
    lines = text.split("\n")
    assert lines[0] == "Recommended pulse program:  hncogp3d   [found in lists/pp]", lines[0]
    assert lines[1] == "Parameter set:              HNCOGP3D   ->  rpar HNCOGP3D all ; getprosol", lines[1]
    assert lines[2] == "Dimension:                  3D"
    assert lines[3] == "Requires:                   f3, gradient, 13C/15N labeled"
    assert lines[4] == "Description:                HNCO"
    assert lines[5] == "Notes:                      Set cnst21/cnst22"
    assert lines[6] == "Alternatives:               trhncogp3d (found), b_hncogp3d (found)", lines[6]
    assert lines[7] == "Your answers:               Bio"
    assert len(lines) == 8, lines


def test_report_not_found_puts_existing_alt_first():
    def exists(name):
        return name == "b_hncogp3d"
    text = pp.format_report(MINI, "hncogp3d", ["Bio"], exists)
    lines = text.split("\n")
    assert lines[0].endswith("[NOT found]"), lines[0]
    assert lines[6] == "Alternatives:               b_hncogp3d (found), trhncogp3d (NOT found)", lines[6]


def test_report_unchecked_and_no_parset():
    text = pp.format_report(MINI, "cosygpqf", ["Small", "COSY"], _exists_unknown)
    lines = text.split("\n")
    assert lines[0].endswith("[not checked]")
    assert lines[1] == "Parameter set:              (no standard parameter set - start from a similar experiment)", lines[1]
    assert lines[5] == "Notes:                      -"
    assert lines[6] == "Alternatives:               -"
    assert lines[7] == "Your answers:               Small > COSY"


def test_make_checker_without_home_returns_none():
    chk = pp.make_checker(None)
    assert chk("zg") is None


def test_make_checker_with_home(tmp_home=None):
    import tempfile
    import shutil
    home = tempfile.mkdtemp()
    try:
        d = pp.pp_dir(home)
        os.makedirs(d)
        with io.open(os.path.join(d, "zg"), "w", encoding="utf-8") as f:
            f.write(u";zg\n")
        chk = pp.make_checker(home)
        assert chk("zg") is True
        assert chk("nope") is False
    finally:
        shutil.rmtree(home)


def test_find_topspin_home_falls_back_to_tree():
    t = dict(MINI)
    t["topspin_home"] = "/opt/topspin"
    # Under CPython sys.registry does not exist -> fallback path
    assert pp.find_topspin_home(t) == "/opt/topspin"
    t["topspin_home"] = None
    assert pp.find_topspin_home(t) is None
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `python3 tests/test_tree.py`
Expected: 6 `FAIL` lines mentioning `AttributeError: module 'pp_selector' has no attribute 'format_report'` (and `make_checker`, `find_topspin_home`); exit 1.

- [ ] **Step 3: Implement report and checker**

Append to `src/pp_selector.py`:

```python
STATUS = {True: "found in lists/pp", False: "NOT found", None: "not checked"}
ALT_STATUS = {True: "found", False: "NOT found", None: "not checked"}


def find_topspin_home(tree):
    """TopSpin install dir: Jython registry first, then tree setting, else None."""
    home = None
    registry = getattr(sys, "registry", None)
    if registry is not None:
        try:
            home = registry.getProperty("XWINNMRHOME")
        except Exception:  # noqa: BLE001 - any Java-side failure
            home = None
    return home or tree.get("topspin_home") or None


def pp_dir(home):
    return os.path.join(home, "exp", "stan", "nmr", "lists", "pp")


def make_checker(home):
    """Return exists(name) -> True/False, or -> None when no home is known."""
    if not home:
        return lambda name: None
    d = pp_dir(home)
    return lambda name: os.path.isfile(os.path.join(d, name))


def _row(label, value):
    return "%-28s%s" % (label + ":", value)


def format_report(tree, name, answers, exists):
    leaf = tree["leaves"][name]
    found = exists(name)
    lines = [_row("Recommended pulse program", "%s   [%s]" % (name, STATUS[found]))]
    parset = leaf.get("parset")
    if parset:
        lines.append(_row("Parameter set", "%s   ->  rpar %s all ; getprosol" % (parset, parset)))
    else:
        lines.append(_row("Parameter set", "(no standard parameter set - start from a similar experiment)"))
    lines.append(_row("Dimension", leaf.get("dim") or "?"))
    lines.append(_row("Requires", ", ".join(leaf.get("requires") or []) or "-"))
    lines.append(_row("Description", leaf.get("desc") or "-"))
    lines.append(_row("Notes", leaf.get("notes") or "-"))
    alts = list(leaf.get("alt") or [])
    if found is False:
        alts.sort(key=lambda a: 0 if exists(a) else 1)  # stable: existing first
    alt_text = ", ".join("%s (%s)" % (a, ALT_STATUS[exists(a)]) for a in alts) or "-"
    lines.append(_row("Alternatives", alt_text))
    lines.append(_row("Your answers", " > ".join(answers)))
    return "\n".join(lines)
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `python3 tests/test_tree.py`
Expected: `13 tests, 0 failed`

- [ ] **Step 5: Commit**

```bash
git add src/pp_selector.py tests/test_tree.py
git commit -m "feat: report formatting and lists/pp existence check"
```

---

### Task 3: TopSpin dialog adapter, console fallback, `main`

**Files:**
- Modify: `src/pp_selector.py` (append)

**Interfaces:**
- Consumes: `walk`, `load_tree`, `format_report`, `make_checker`, `find_topspin_home`.
- Produces: `ask_topspin(question, labels) -> int`, `main()`; module-level `IN_TOPSPIN` bool.

- [ ] **Step 1: Append adapter and main**

Append to `src/pp_selector.py`:

```python
TITLE = "pp_selector"
MAX_BUTTONS = 4  # more options than this -> numbered list + text entry

try:
    from TopCmds import SELECT, INPUT_DIALOG, VIEWTEXT, MSG  # noqa: F401  (TopSpin builtins)
    IN_TOPSPIN = True
except ImportError:
    IN_TOPSPIN = False

    def _readline(prompt):
        try:
            return raw_input(prompt)  # noqa: F821 (Python 2)
        except NameError:
            return input(prompt)

    def SELECT(title, message, buttons):  # noqa: N802 - mimic TopSpin API
        print(title)
        print(message)
        for i, b in enumerate(buttons):
            print("  %d) %s" % (i, b))
        try:
            return int(_readline("> "))
        except ValueError:
            return -1

    def INPUT_DIALOG(title, header, items, values, comments, types, buttons):  # noqa: N802
        print(title)
        print(header)
        raw = _readline("%s " % items[0])
        return [raw] if raw.strip() else None

    def VIEWTEXT(title, header, text):  # noqa: N802
        print(title)
        print(header)
        print(text)

    def MSG(text):  # noqa: N802
        print(text)


def ask_topspin(question, labels):
    """Ask one question. Returns option index or BACK."""
    if len(labels) <= MAX_BUTTONS:
        idx = SELECT(TITLE, question, list(labels) + ["Back"])
        if idx is None or idx < 0 or idx >= len(labels):
            return BACK
        return idx
    header = question + "\n" + "\n".join("%d) %s" % (i + 1, l) for i, l in enumerate(labels))
    result = INPUT_DIALOG(TITLE, header, ["Choice number:"], ["1"], [""], ["1"], ["OK", "Back"])
    if not result:
        return BACK
    try:
        n = int(str(result[0]).strip())
    except ValueError:
        return BACK
    if 1 <= n <= len(labels):
        return n - 1
    return BACK


def _script_dir():
    try:
        return os.path.dirname(os.path.abspath(__file__))
    except NameError:  # __file__ missing under some TopSpin invocations
        return os.getcwd()


def main():
    path = os.path.join(_script_dir(), "pp_tree.json")
    try:
        tree = load_tree(path)
    except TreeError as e:
        MSG("pp_selector: tree error: %s" % e)
        return
    name, answers = walk(tree, ask_topspin)
    if name is None:
        return
    exists = make_checker(find_topspin_home(tree))
    VIEWTEXT(TITLE, "Recommendation", format_report(tree, name, answers, exists))


if __name__ == "__main__":
    main()
```

TopSpin API notes for the implementer (from Bruker's Python programming manual; verify on a real TopSpin once, see Task 8 README):
- `SELECT(title, message, buttons)` returns the index of the pressed button; closing the dialog returns -1.
- `INPUT_DIALOG(title, header, items, values, comments, types, buttons)` returns the list of field values, or `None` when cancelled/closed. `types` `"1"` = text field.
- `VIEWTEXT(title, header, text)` opens a read-only text window.

- [ ] **Step 2: Smoke-test the console fallback with a scripted session**

Create a temporary tree and run the module under CPython:

```bash
mkdir -p /tmp/ppsel && python3 - <<'EOF'
import json
json.dump({
  "version":1,"topspin_home":None,"root":"domain",
  "nodes":{"domain":{"q":"Sample?","opts":[["Small","L:zg30"],["Bio","L:hncogp3d"]]}},
  "leaves":{"zg30":{"parset":"PROTON","dim":"1D","desc":"1D","requires":[],"notes":"","alt":[]},
            "hncogp3d":{"parset":"HNCOGP3D","dim":"3D","desc":"HNCO","requires":["f3"],"notes":"","alt":[]}}
}, open("/tmp/ppsel/pp_tree.json","w"))
EOF
cp src/pp_selector.py /tmp/ppsel/ && printf '1\n' | python3 /tmp/ppsel/pp_selector.py
```

Expected output ends with:
```
Recommended pulse program:  hncogp3d   [not checked]
Parameter set:              HNCOGP3D   ->  rpar HNCOGP3D all ; getprosol
...
Your answers:               Bio
```

- [ ] **Step 3: Verify Jython-compatible syntax**

Run: `python3 -c "import ast,sys; ast.parse(open('src/pp_selector.py').read())" && grep -nE 'f"|f'"'"'|pathlib|-> |: (str|int|dict)\b' src/pp_selector.py`
Expected: no grep matches (exit 1 from grep is fine), no SyntaxError.

- [ ] **Step 4: Run tests**

Run: `python3 tests/test_tree.py`
Expected: `13 tests, 0 failed`

- [ ] **Step 5: Commit**

```bash
git add src/pp_selector.py
git commit -m "feat: TopSpin dialog adapter, console fallback and main()"
```

---

### Task 4: Tree integrity and library-existence tests + minimal tree

**Files:**
- Create: `src/pp_tree.json` (minimal, grows in Tasks 6–7)
- Modify: `tests/test_tree.py` (append)

**Interfaces:**
- Consumes: `pp.load_tree`, `pp.LEAF_PREFIX`, `TREE_PATH`, `PP_DIR`.
- Produces: `real_tree()` helper, `parset_map()` helper used by Task 6–8 tests.

- [ ] **Step 1: Write the failing tests**

Append to `tests/test_tree.py` (above `def main`):

```python
def real_tree():
    return pp.load_tree(TREE_PATH)


def parset_map():
    """pulseprogram -> parameter set from doc/ppcatalogue-{1,2}-programs.txt ('-' = unknown)."""
    m = {}
    for vol in ("1", "2"):
        p = os.path.join(ROOT, "doc", "ppcatalogue-%s-programs.txt" % vol)
        with io.open(p, encoding="utf-8") as f:
            for line in f:
                if line.startswith("#") or not line.strip():
                    continue
                name, ps = line.rstrip("\n").split("\t")
                if ps != "-":
                    m.setdefault(name, ps)
    return m


def test_real_tree_loads_and_validates():
    t = real_tree()
    assert t["root"] in t["nodes"]


def test_real_tree_no_orphan_nodes_and_all_leaves_reachable():
    t = real_tree()
    seen_nodes, seen_leaves = set(), set()
    stack = [t["root"]]
    while stack:
        nid = stack.pop()
        if nid in seen_nodes:
            continue
        seen_nodes.add(nid)
        for _, target in t["nodes"][nid]["opts"]:
            if target.startswith(pp.LEAF_PREFIX):
                seen_leaves.add(target[len(pp.LEAF_PREFIX):])
            else:
                stack.append(target)
    orphans = set(t["nodes"]) - seen_nodes
    unreachable = set(t["leaves"]) - seen_leaves
    assert not orphans, "orphan nodes: %s" % sorted(orphans)
    assert not unreachable, "unreachable leaves: %s" % sorted(unreachable)


def test_real_tree_has_no_cycles():
    t = real_tree()
    WHITE, GREY, BLACK = 0, 1, 2
    color = dict((n, WHITE) for n in t["nodes"])

    def visit(nid):
        color[nid] = GREY
        for _, target in t["nodes"][nid]["opts"]:
            if target.startswith(pp.LEAF_PREFIX):
                continue
            if color[target] == GREY:
                raise AssertionError("cycle through %s -> %s" % (nid, target))
            if color[target] == WHITE:
                visit(target)
        color[nid] = BLACK
    visit(t["root"])


def test_real_tree_leaf_and_alt_files_exist():
    t = real_tree()
    missing = []
    for name, leaf in t["leaves"].items():
        for n in [name] + list(leaf.get("alt") or []):
            if not os.path.isfile(os.path.join(PP_DIR, n)):
                missing.append(n)
    assert not missing, "not in doc/pulseprogram: %s" % sorted(set(missing))


def test_real_tree_parsets_match_catalogue():
    t = real_tree()
    m = parset_map()
    bad = []
    for name, leaf in t["leaves"].items():
        ps = leaf.get("parset")
        if ps and name in m and m[name] != ps:
            bad.append((name, ps, m[name]))
    assert not bad, "parset mismatch (leaf, json, catalogue): %s" % bad


def test_real_tree_leaf_fields_present():
    t = real_tree()
    for name, leaf in t["leaves"].items():
        for key in ("parset", "desc", "dim", "requires", "notes", "alt"):
            assert key in leaf, "%s lacks %s" % (name, key)
        assert isinstance(leaf["requires"], list) and isinstance(leaf["alt"], list), name
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `python3 tests/test_tree.py`
Expected: 6 `FAIL` lines with `TreeError: cannot read .../src/pp_tree.json`; exit 1.

- [ ] **Step 3: Create the minimal tree**

`src/pp_tree.json`:

```json
{
  "version": 1,
  "topspin_home": null,
  "root": "domain",
  "nodes": {
    "domain": {
      "q": "Sample type?",
      "opts": [
        ["Small molecule (1D / 2D)", "sm_exp"],
        ["Protein / nucleic acid (isotope labeled)", "bio_exp"]
      ]
    },
    "sm_exp": {
      "q": "Experiment?",
      "opts": [
        ["1D 1H", "sm_1h"],
        ["1D 13C", "sm_13c"]
      ]
    },
    "sm_1h": {
      "q": "Pulse angle / solvent suppression?",
      "opts": [
        ["30 deg pulse, no suppression (routine)", "L:zg30"],
        ["90 deg pulse (quantitative), no suppression", "L:zg"],
        ["Presaturation", "L:zgpr"],
        ["WATERGATE 3-9-19 (gradients)", "L:p3919gp"],
        ["Excitation sculpting (gradients)", "L:zgesgp"],
        ["WET", "L:wet"]
      ]
    },
    "sm_13c": {
      "q": "Decoupling?",
      "opts": [
        ["Broadband decoupled (NOE enhanced), 30 deg", "L:zgdc30"],
        ["Broadband decoupled, 90 deg", "L:zgdc"],
        ["Inverse gated (quantitative, no NOE)", "L:zgig"],
        ["Gated (coupled spectrum with NOE)", "L:zggd"],
        ["Coupled, no decoupling", "L:zg"]
      ]
    },
    "bio_exp": {
      "q": "Experiment class?",
      "opts": [
        ["2D 1H-15N HSQC / TROSY", "bio_hn"],
        ["Backbone assignment (triple resonance)", "bio_bb"]
      ]
    },
    "bio_hn": {
      "q": "Protein size / conditions?",
      "opts": [
        ["< 25 kDa, H2O: HSQC with PEP and water flip-back", "L:hsqcetfpf3gpsi"],
        ["> 25 kDa or high field: TROSY", "L:trosyetf3gpsi"]
      ]
    },
    "bio_bb": {
      "q": "Experiment?",
      "opts": [
        ["HNCO", "bio_hnco"],
        ["HNCA", "bio_hnca"]
      ]
    },
    "bio_hnco": {
      "q": "Version?",
      "opts": [
        ["PEP (< 25 kDa)", "L:hncogp3d"],
        ["TROSY (> 25 kDa)", "L:trhncogp3d"]
      ]
    },
    "bio_hnca": {
      "q": "Version?",
      "opts": [
        ["PEP (< 25 kDa)", "L:hncagp3d"],
        ["TROSY (> 25 kDa)", "L:trhncagp3d"]
      ]
    }
  },
  "leaves": {
    "zg30": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": ["zg"]},
    "zg": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": ["zg30"]},
    "zgpr": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "Set o1 on the solvent line; pl9 = presaturation power", "alt": ["zgcppr"]},
    "p3919gp": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "d19 = 1/(2*distance water-signal of interest in Hz)", "alt": ["zggpwg"]},
    "zgesgp": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": ["zggpw5"]},
    "wet": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": ["wetdc"]},
    "zgdc30": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": ["zgpg30"]},
    "zgdc": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": ["zgpg"]},
    "zgig": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "Use d1 >= 5*T1 for quantitation", "alt": ["zgig30"]},
    "zggd": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": ["zggd30"]},
    "hsqcetfpf3gpsi": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": ["hsqcfpf3gpphwg", "hsqcetf3gpsi"]},
    "trosyetf3gpsi": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": ["trosyf3gpph19", "trosyetf3gpsi.2"]},
    "hncogp3d": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "Set cnst21 (CO ppm), cnst22 (Ca ppm); getprosol after rpar", "alt": ["hncogpwg3d", "trhncogp3d", "b_hncogp3d"]},
    "trhncogp3d": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "No decoupling during acquisition (TROSY); set cnst21/cnst22", "alt": ["trhncoetgp3d", "trhncogp2h3d"]},
    "hncagp3d": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "Set cnst21 (CO ppm), cnst22 (Ca ppm)", "alt": ["hncagpwg3d", "trhncagp3d", "hncaigp3d"]},
    "trhncagp3d": {"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": ["trhncaetgp3d", "trhncagp2h3d"]}
  }
}
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `python3 tests/test_tree.py`
Expected: `19 tests, 0 failed` (parset test passes because all parsets are `null`; existence test passes — every name above is in `doc/pulseprogram/`. If a name is reported missing, fix the name, do not delete the test.)

- [ ] **Step 5: Commit**

```bash
git add src/pp_tree.json tests/test_tree.py
git commit -m "feat: minimal decision tree with integrity and library tests"
```

---

### Task 5: `tools/build_leaves.py` — fill `parset`/`desc`/`dim`

**Files:**
- Create: `tools/build_leaves.py`
- Modify: `tests/test_tree.py` (append)

**Interfaces:**
- Produces (importable from tests via `sys.path`):
  - `read_header(path) -> dict(desc=str, dim=str or None, uses_f3=bool, uses_gp=bool)`
  - `load_parsets(doc_dir) -> dict name->parset`
  - `fill_leaves(tree, pp_dir, parsets, force=False) -> list of (name, field) filled`
  - CLI: `python3 tools/build_leaves.py [--force]` rewrites `src/pp_tree.json` in place.

- [ ] **Step 1: Write the failing tests**

Append to `tests/test_tree.py` (above `def main`):

```python
sys.path.insert(0, os.path.join(ROOT, "tools"))
import build_leaves as bl  # noqa: E402


def test_read_header_hncogp3d():
    h = bl.read_header(os.path.join(PP_DIR, "hncogp3d"))
    assert h["dim"] == "3D", h
    assert h["desc"] == ("HNCO; 3D sequence with; inverse correlation for triple resonance using multiple; "
                         "inept transfer steps"), h["desc"]
    assert h["uses_f3"] is True and h["uses_gp"] is True


def test_read_header_zg():
    h = bl.read_header(os.path.join(PP_DIR, "zg"))
    assert h["dim"] == "1D"
    assert h["desc"] == "1D sequence", h["desc"]
    assert h["uses_f3"] is False and h["uses_gp"] is False


def test_load_parsets():
    m = bl.load_parsets(os.path.join(ROOT, "doc"))
    assert m["hncogp3d"] == "HNCOGP3D"
    assert m["b_hncogp3d"] == "B_HNCOGP3D"
    assert "cosygpqf" in m  # Vol. I entry: COSYGPSW


def test_fill_leaves_fills_empty_only():
    tree = {
        "leaves": {
            "hncogp3d": {"parset": None, "desc": None, "dim": None, "requires": [], "notes": "keep", "alt": ["x"]},
            "zg": {"parset": "MYSET", "desc": "my desc", "dim": None, "requires": ["custom"], "notes": "", "alt": []},
        }
    }
    filled = bl.fill_leaves(tree, PP_DIR, {"hncogp3d": "HNCOGP3D"})
    a = tree["leaves"]["hncogp3d"]
    assert a["parset"] == "HNCOGP3D" and a["dim"] == "3D" and a["desc"].startswith("HNCO")
    assert a["requires"] == ["f3", "gradient"], a["requires"]
    assert a["notes"] == "keep" and a["alt"] == ["x"]
    b = tree["leaves"]["zg"]
    assert b["parset"] == "MYSET" and b["desc"] == "my desc" and b["dim"] == "1D"
    assert b["requires"] == ["custom"]
    assert ("hncogp3d", "parset") in filled and ("zg", "parset") not in filled


def test_fill_leaves_force_overwrites_auto_fields_only():
    tree = {"leaves": {"zg": {"parset": "MYSET", "desc": "my desc", "dim": "9D",
                              "requires": ["custom"], "notes": "n", "alt": ["a"]}}}
    bl.fill_leaves(tree, PP_DIR, {"zg": "PROTON"}, force=True)
    z = tree["leaves"]["zg"]
    assert z["parset"] == "PROTON" and z["desc"] == "1D sequence" and z["dim"] == "1D"
    assert z["requires"] == ["custom"] and z["notes"] == "n" and z["alt"] == ["a"]
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `python3 tests/test_tree.py`
Expected: traceback `ModuleNotFoundError: No module named 'build_leaves'`; exit 1.

- [ ] **Step 3: Implement the build tool**

`tools/build_leaves.py`:

```python
#!/usr/bin/env python3
"""Fill parset/desc/dim of leaves in src/pp_tree.json from the library and catalogue maps.

Usage: python3 tools/build_leaves.py [--force]
Hand-written fields (requires, notes, alt) are never overwritten; with --force
the auto fields (parset, desc, dim) are recomputed even if already set.
"""
import io
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TREE_PATH = os.path.join(ROOT, "src", "pp_tree.json")
PP_DIR = os.path.join(ROOT, "doc", "pulseprogram")
DOC_DIR = os.path.join(ROOT, "doc")
MAX_DESC_LINES = 4


def read_header(path):
    """desc = up to 4 comment lines after ';avance-version', joined by '; '."""
    with io.open(path, encoding="latin-1") as f:
        text = f.read()
    lines = text.split("\n")
    desc = []
    started = False
    for line in lines:
        if not line.startswith(";"):
            if started:
                break
            continue
        body = line[1:].strip()
        if body.startswith("avance-version"):
            started = True
            continue
        if not started:
            continue
        if not body or body.startswith("$") or body.startswith("(use parameterset"):
            if desc:
                break
            continue
        desc.append(body)
        if len(desc) >= MAX_DESC_LINES:
            break
    m = re.search(r"^;\$DIM=(.*)$", text, re.M)
    dim = m.group(1).strip() if m else None
    return {
        "desc": "; ".join(desc),
        "dim": dim or None,
        "uses_f3": bool(re.search(r":f3\b", text)),
        "uses_gp": bool(re.search(r":gp\d", text)),
    }


def load_parsets(doc_dir):
    m = {}
    for vol in ("1", "2"):
        p = os.path.join(doc_dir, "ppcatalogue-%s-programs.txt" % vol)
        with io.open(p, encoding="utf-8") as f:
            for line in f:
                if line.startswith("#") or not line.strip():
                    continue
                name, ps = line.rstrip("\n").split("\t")
                if ps != "-":
                    m.setdefault(name, ps)
    return m


def fill_leaves(tree, pp_dir, parsets, force=False):
    filled = []
    for name, leaf in sorted(tree["leaves"].items()):
        path = os.path.join(pp_dir, name)
        if not os.path.isfile(path):
            print("WARNING: %s not in %s" % (name, pp_dir), file=sys.stderr)
            continue
        h = read_header(path)
        auto = {"parset": parsets.get(name), "desc": h["desc"] or None, "dim": h["dim"]}
        for key, value in auto.items():
            if force or not leaf.get(key):
                if leaf.get(key) != value:
                    leaf[key] = value
                    filled.append((name, key))
        if not leaf.get("requires"):
            req = []
            if h["uses_f3"]:
                req.append("f3")
            if h["uses_gp"]:
                req.append("gradient")
            if req:
                leaf["requires"] = req
                filled.append((name, "requires"))
        for key, default in (("requires", []), ("notes", ""), ("alt", [])):
            leaf.setdefault(key, default)
    return filled


def main(argv):
    force = "--force" in argv
    with io.open(TREE_PATH, encoding="utf-8") as f:
        tree = json.load(f)
    filled = fill_leaves(tree, PP_DIR, load_parsets(DOC_DIR), force=force)
    with io.open(TREE_PATH, "w", encoding="utf-8") as f:
        json.dump(tree, f, indent=2, sort_keys=True, ensure_ascii=True)
        f.write("\n")
    print("filled %d fields in %d leaves" % (len(filled), len(set(n for n, _ in filled))))


if __name__ == "__main__":
    main(sys.argv[1:])
```

Note: `sort_keys=True` also sorts `"nodes"` by id and `"opts"` stays in author order (lists are not sorted). Node order in the file is cosmetic; the runner only follows `root` and targets.

- [ ] **Step 4: Run tests to verify they pass**

Run: `python3 tests/test_tree.py`
Expected: `24 tests, 0 failed`. If `test_read_header_hncogp3d` fails on the exact `desc` prefix, print `bl.read_header(...)["desc"]` and adjust the expected string to the real first four header lines joined by `; ` — the rule (4 lines after `;avance-version`) is the contract, the literal is derived from it.

- [ ] **Step 5: Run the tool on the minimal tree and re-test**

Run: `python3 tools/build_leaves.py && python3 tests/test_tree.py && git diff --stat`
Expected: `filled ... fields in 16 leaves`; tests `24 tests, 0 failed` (parset test now compares real parsets: `zg30 -> PROTON`? — the catalogue lists `zg30` under several parsets; `setdefault` keeps the first, which is what the test compares against, so it passes by construction). `src/pp_tree.json` now has `parset`/`desc`/`dim` populated and `requires` suggested.

- [ ] **Step 6: Commit**

```bash
git add tools/build_leaves.py tests/test_tree.py src/pp_tree.json
git commit -m "feat: build_leaves tool fills leaf metadata from library and catalogue"
```

---

### Task 6: Small-molecule tree (Catalogue Vol. I)

**Files:**
- Modify: `src/pp_tree.json` (replace `sm_exp` and add nodes/leaves below)
- Modify: `tests/test_tree.py` (append path tests)

**Interfaces:**
- Consumes: `walk`, `real_tree`, `scripted` (Task 1), build tool (Task 5).
- Produces: node ids listed below (referenced by README examples).

- [ ] **Step 1: Write the failing path tests**

Append to `tests/test_tree.py` (above `def main`):

```python
def _path(labels_to_pick):
    """Drive walk() by option LABEL text instead of index."""
    picks = list(labels_to_pick)

    def ask(question, labels):
        want = picks.pop(0)
        assert want in labels, "%r not offered for %r; offered %r" % (want, question, labels)
        return labels.index(want)
    return ask


def test_path_sm_cosy_purge():
    leaf, _ = pp.walk(real_tree(), _path(["Small molecule (1D / 2D)", "COSY",
                                          "Magnitude (fast, no phasing)", "Purge pulses before d1"]))
    assert leaf == "cosygpppqf", leaf


def test_path_sm_hsqc_edited_adiabatic():
    leaf, _ = pp.walk(real_tree(), _path(["Small molecule (1D / 2D)", "HSQC (1H-13C one-bond)",
                                          "Multiplicity edited (CH/CH3 up, CH2 down)",
                                          "Adiabatic 180 pulses, sensitivity improved (recommended)"]))
    assert leaf == "hsqcedetgpsisp2.3", leaf


def test_path_sm_hmbc_standard():
    leaf, _ = pp.walk(real_tree(), _path(["Small molecule (1D / 2D)", "HMBC (1H-13C long-range)",
                                          "Magnitude, low-pass J filter, no decoupling (standard)"]))
    assert leaf == "hmbcgplpndqf", leaf


def test_path_sm_zg30():
    leaf, _ = pp.walk(real_tree(), _path(["Small molecule (1D / 2D)", "1D 1H",
                                          "30 deg pulse, no suppression (routine)"]))
    assert leaf == "zg30", leaf
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `python3 tests/test_tree.py`
Expected: 3 `FAIL` (`'COSY' not offered`, `'HSQC (1H-13C one-bond)' not offered`, `'HMBC ...' not offered`); `test_path_sm_zg30` passes; exit 1.

- [ ] **Step 3: Author the small-molecule nodes**

Replace node `sm_exp` and add the nodes below to `src/pp_tree.json` `"nodes"` (keep `sm_1h`, `sm_13c` from Task 4). Format: `node id` — question — `label → target`.

**`sm_exp`** — "Experiment?"
1. `1D 1H` → `sm_1h`
2. `1D 13C` → `sm_13c`
3. `1D 13C multiplicity (DEPT / APT)` → `sm_dept`
4. `1D selective (1D NOESY / TOCSY / COSY)` → `sm_sel`
5. `T1 / T2 measurement` → `sm_relax`
6. `COSY` → `sm_cosy`
7. `TOCSY` → `sm_tocsy`
8. `NOESY` → `sm_noesy`
9. `ROESY` → `sm_roesy`
10. `HSQC (1H-13C one-bond)` → `sm_hsqc`
11. `HMQC (1H-13C one-bond, simpler)` → `sm_hmqc`
12. `HMBC (1H-13C long-range)` → `sm_hmbc`
13. `HSQC/HMQC-TOCSY / -NOESY (combined)` → `sm_hx2d`
14. `DOSY (diffusion)` → `sm_dosy`
15. `STD (ligand screening)` → `sm_std`
16. `J-resolved` → `sm_jres`
17. `INADEQUATE / ADEQUATE (13C-13C)` → `sm_inad`

**`sm_dept`** — "Which editing?"
- `DEPT-135 (CH/CH3 up, CH2 down)` → `L:dept135`
- `DEPT-90 (CH only)` → `L:dept90`
- `DEPT-45 (all protonated carbons)` → `L:dept45`
- `DEPTQ (includes quaternary C, adiabatic)` → `L:deptqgpsp`
- `APT` → `L:apt`
- `JMOD` → `L:jmod`

**`sm_sel`** — "Which 1D selective experiment?"
- `1D NOESY (gradient, selective)` → `L:selnogp`
- `1D NOESY with zero-quantum suppression` → `L:selnogpzs`
- `1D ROESY` → `L:selrogp`
- `1D TOCSY` → `L:selmlgp`
- `1D COSY` → `L:selcogp`

**`sm_relax`** — "Which?"
- `T1 inversion recovery` → `L:t1ir`
- `T2 CPMG` → `L:cpmg`
- `T2 CPMG with presaturation` → `L:cpmgpr1d`

**`sm_cosy`** — "Phase mode?"
- `Magnitude (fast, no phasing)` → `sm_cosy_qf`
- `Phase-sensitive double-quantum filtered (DQF)` → `sm_cosy_df`
- `Phase-sensitive, phase-cycled (no gradients)` → `L:cosyph`

**`sm_cosy_qf`** — "Solvent suppression?"
- `None` → `L:cosygpqf`
- `Presaturation` → `L:cosygpprqf`
- `Purge pulses before d1` → `L:cosygpppqf`
- `Multiple-quantum filtered` → `L:cosygpmfqf`

**`sm_cosy_df`** — "Solvent suppression?"
- `None, phase-cycled` → `L:cosydfph`
- `None, gradient echo-antiecho` → `L:cosydfetgp.2`
- `Presaturation` → `L:cosydfphpr`
- `Excitation sculpting` → `L:cosydfesgpph`

**`sm_tocsy`** — "Solvent suppression / mixing?"
- `None, DIPSI-2 with zero-quantum suppression (recommended)` → `L:dipsi2gpphzs`
- `Presaturation` → `L:dipsi2gpphpr`
- `WATERGATE 3-9-19` → `L:dipsi2gpph19`
- `Excitation sculpting` → `L:dipsi2esgpph`
- `MLEV-17, phase-cycled (classic)` → `L:mlevph`

**`sm_noesy`** — "Solvent suppression?"
- `None, gradient` → `L:noesygpph`
- `Presaturation` → `L:noesygpphpr`
- `WATERGATE 3-9-19` → `L:noesygpph19`
- `Excitation sculpting` → `L:noesyesgpph`
- `Zero-quantum suppression` → `L:noesygpphzs`
- `Flip-back + WATERGATE (peptides in H2O)` → `L:noesyfpgpphwg`

**`sm_roesy`** — "Variant?"
- `CW spin-lock, phase-cycled` → `L:roesyph`
- `With presaturation` → `L:roesyphpr`
- `Gradient, excitation sculpting` → `L:roesyesgpph`
- `Transverse ROESY (suppresses TOCSY transfer)` → `L:troesyph`

**`sm_hsqc`** — "Variant?"
- `Standard (no editing)` → `sm_hsqc_plain`
- `Multiplicity edited (CH/CH3 up, CH2 down)` → `sm_hsqc_ed`
- `Constant-time (high F1 resolution)` → `L:hsqcctetgpsp`

**`sm_hsqc_plain`** — "Pulses / solvent?"
- `Adiabatic 180 pulses, sensitivity improved (recommended)` → `L:hsqcetgpsisp2.2`
- `Hard pulses, sensitivity improved` → `L:hsqcetgpsi`
- `Hard pulses, no PEP (shortest)` → `L:hsqcetgp`
- `With presaturation` → `L:hsqcetgpprsisp2.2`
- `Phase-cycled, no gradients` → `L:hsqcph`

**`sm_hsqc_ed`** — "Pulses?"
- `Adiabatic 180 pulses, sensitivity improved (recommended)` → `L:hsqcedetgpsisp2.3`
- `Adiabatic, sensitivity improved (older variant)` → `L:hsqcedetgpsisp2.2`
- `Hard pulses` → `L:hsqcedetgp`

**`sm_hmqc`** — "Variant?"
- `Gradient echo-antiecho` → `L:hmqcetgp`
- `Magnitude, gradient` → `L:hmqcgpqf`
- `Phase-cycled` → `L:hmqcph`
- `With BIRD filter (suppresses 12C-H)` → `L:hmqcbiph`

**`sm_hmbc`** — "Variant?"
- `Magnitude, low-pass J filter, no decoupling (standard)` → `L:hmbcgplpndqf`
- `Magnitude, 2-fold low-pass filter` → `L:hmbcgpl2ndqf`
- `Phase-sensitive echo-antiecho, 3-fold low-pass filter` → `L:hmbcetgpl3nd`
- `With presaturation` → `L:hmbcgplpndprqf`
- `Constant-time (sharper F1)` → `L:hmbcctetgpl2nd`
- `Accordion (wide range of nJ)` → `L:hmbcacgplpndqf`

**`sm_hx2d`** — "Which combined experiment?"
- `HSQC-TOCSY (adiabatic, sensitivity improved)` → `L:hsqcdietgpsisp`
- `HSQC-TOCSY (hard pulses)` → `L:hsqcdietgpsi`
- `HMQC-TOCSY` → `L:hmqcdietgp`
- `HSQC-NOESY` → `L:hsqcetgpnosp`
- `HSQC-ROESY` → `L:hsqcetgprosp`

**`sm_dosy`** — "Variant?"
- `Bipolar LED, 2 spoil gradients (standard)` → `L:ledbpgp2s`
- `Bipolar STE, 1 spoil gradient` → `L:stebpgp1s`
- `Bipolar LED with presaturation` → `L:ledbpgppr2s`
- `Double STE (convection compensated)` → `L:dstebpgp3s`
- `1D setup (optimize gradient range)` → `L:ledbpgp2s1d`

**`sm_std`** — "Variant?"
- `STD with excitation sculpting` → `L:stddiffesgp`
- `STD, no suppression` → `L:stddiff`
- `STD with WATERGATE 3-9-19` → `L:stddiffgp19`
- `STD-TOCSY` → `L:stdmlevesgpph`
- `STD-NOESY` → `L:stdnoesyesgpph`

**`sm_jres`** — "Which?"
- `Homonuclear J-resolved (magnitude)` → `L:jresqf`
- `Homonuclear, gradients + presaturation` → `L:jresgpprqf`
- `Heteronuclear J-resolved` → `L:hjresqf`

**`sm_inad`** — "Which?"
- `2D INADEQUATE (magnitude)` → `L:inadqf`
- `2D INADEQUATE (phase-sensitive)` → `L:inadph`
- `1D INADEQUATE` → `L:inad1d`
- `1,1-ADEQUATE (1H-detected, 1J(CC))` → `L:adeq11etgprdsp`
- `1,n-ADEQUATE (long-range)` → `L:adeq1netgprdsp`

- [ ] **Step 4: Add the leaves with hand-written `notes`/`alt`**

Add every `L:` target above to `"leaves"` with the template `{"parset": null, "desc": null, "dim": null, "requires": [], "notes": "", "alt": []}` and these hand-written values (leaves not listed keep empty `notes`/`alt`):

| leaf | notes | alt |
|---|---|---|
| dept135 | `cnst2 = 1J(CH), typically 145 Hz` | `deptsp135` |
| deptqgpsp | `Quaternary carbons negative` | `deptqsp` |
| selnogp | `Set d8 = mixing time; sp2 = selective 180 (Gaus1_180r.1000)` | `selnogpzs` |
| selmlgp | `d9 = TOCSY mixing (60-120 ms)` | `selmlgp.2` |
| cosygpqf | `Fast routine COSY; ph variant for better resolution` | `cosygpppqf`, `cosyqf` |
| cosygpppqf | `Purge pulses suppress residual signals between scans` | `cosygpqf` |
| cosydfph | `Longer than magnitude COSY; process in ph mode` | `cosydfetgp.2` |
| dipsi2gpphzs | `d9 = mixing time (60-100 ms); zero-quantum suppression cleans cross peaks` | `dipsi2gpph19`, `dipsi2phpr` |
| mlevph | `d9 = mixing time; classic phase-cycled version` | `mlevgpph19` |
| noesygpph | `d8 = mixing time (small molecules 0.5-1 s); ROESY for mid-size molecules` | `noesyesgpph`, `noesygpphpr` |
| noesyesgpph | `Excitation sculpting: sp1 = water-selective 180` | `noesygpph19`, `noesygpphzs` |
| roesyph | `p15 = spin-lock (200-300 ms) at pl11` | `roesyesgpph`, `troesyph` |
| troesyph | `Off-resonance shaped spin-lock (roesylist) removes TOCSY artifacts` | `roesyadjsphpr` |
| hsqcetgpsisp2.2 | `cnst2 = 1J(CH) 145 Hz; adiabatic p14:sp3 (Crp60,0.5,20.1)` | `hsqcetgpsi`, `hsqcetgpsisp2.3` |
| hsqcetgpsi | `cnst2 = 1J(CH); hard 13C 180 - use sp2.2 version above 500 MHz` | `hsqcetgpsisp2.2` |
| hsqcedetgpsisp2.3 | `cnst2 = 1J(CH); CH/CH3 positive, CH2 negative` | `hsqcedetgpsisp2.2`, `hsqcedetgp` |
| hsqcctetgpsp | `d23 = constant-time delay (1/1J(CC) approx 7-8 ms)` | `hsqcctetgpsisp` |
| hmqcbiph | `d7 = BIRD recovery delay` | `hmqcbindph` |
| hmbcgplpndqf | `cnst2 = 1J(CH) 145 Hz (low-pass), cnst13 = nJ(CH) 8-10 Hz` | `hmbcetgpl3nd`, `hmbcgpl2ndqf` |
| hmbcetgpl3nd | `Phase-sensitive; 3-fold filter for wide 1J range` | `hmbcgplpndqf` |
| hsqcdietgpsisp | `d9 = TOCSY mixing time` | `hsqcdietgpsi` |
| ledbpgp2s | `d20 = diffusion time, p30 = gradient length; gpz6 array via difframp` | `stebpgp1s`, `dstebpgp3s` |
| stddiffesgp | `fq2list = on/off resonance saturation; d20 = saturation time` | `stddiff`, `stddiffgp19` |
| t1ir | `vdlist = recovery delays` | `t1irpg` |
| cpmg | `vclist or d20 loop for echo times` | `cpmg1d` |
| inadqf | `13C-13C; needs high concentration or long time` | `inadph` |
| adeq11etgprdsp | `1H-detected 13C-13C connectivity` | `adeq11etgpsp` |

- [ ] **Step 5: Run the build tool, then tests**

Run: `python3 tools/build_leaves.py && python3 tests/test_tree.py`
Expected: `28 tests, 0 failed`. Typical failures and fixes: a leaf name not in `doc/pulseprogram/` → fix the spelling in the JSON (check with `ls doc/pulseprogram | grep <stem>`); a parset mismatch → remove the hand-set parset and let the tool fill it.

- [ ] **Step 6: Commit**

```bash
git add src/pp_tree.json tests/test_tree.py
git commit -m "feat: small-molecule decision tree (catalogue Vol. I)"
```

---

### Task 7: Biomolecular tree (Catalogue Vol. II)

**Files:**
- Modify: `src/pp_tree.json` (replace `bio_exp`, `bio_hn`, `bio_bb`; add nodes/leaves)
- Modify: `tests/test_tree.py` (append)

**Interfaces:**
- Consumes: Task 6 helpers. Produces: node ids below.

- [ ] **Step 1: Write the failing tests**

Append to `tests/test_tree.py` (above `def main`):

```python
def test_path_bio_hnco_pep():
    leaf, _ = pp.walk(real_tree(), _path(["Protein / nucleic acid (isotope labeled)",
                                          "Backbone assignment (triple resonance)", "HNCO",
                                          "PEP (< 25 kDa)"]))
    assert leaf == "hncogp3d", leaf


def test_path_bio_hnco_trosy():
    leaf, _ = pp.walk(real_tree(), _path(["Protein / nucleic acid (isotope labeled)",
                                          "Backbone assignment (triple resonance)", "HNCO",
                                          "TROSY (> 25 kDa)"]))
    assert leaf == "trhncogp3d", leaf


def test_bio_leaves_declare_labeling():
    """Every leaf reachable under bio_exp must state an isotope requirement."""
    t = real_tree()
    stack, seen, leaves = ["bio_exp"], set(), set()
    while stack:
        nid = stack.pop()
        if nid in seen:
            continue
        seen.add(nid)
        for _, target in t["nodes"][nid]["opts"]:
            if target.startswith(pp.LEAF_PREFIX):
                leaves.add(target[len(pp.LEAF_PREFIX):])
            else:
                stack.append(target)
    bad = [n for n in leaves if not any("labeled" in r for r in t["leaves"][n]["requires"])]
    assert not bad, "bio leaves without labeling requirement: %s" % sorted(bad)
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `python3 tests/test_tree.py`
Expected: `test_path_bio_hnco_pep` passes (already in minimal tree); `test_bio_leaves_declare_labeling` FAILS listing the 6 bio leaves; exit 1.

- [ ] **Step 3: Author the biomolecular nodes**

**`bio_exp`** — "Experiment class?"
1. `2D 1H-15N HSQC / TROSY` → `bio_hn`
2. `3D NOESY / TOCSY-HSQC (X-edited)` → `bio_xedit`
3. `Backbone assignment (triple resonance)` → `bio_bb`
4. `Side-chain assignment` → `bio_sc`
5. `HCCH (aliphatic side chains)` → `bio_hcch`
6. `Relaxation (T1 / T2 / NOE)` → `bio_relax`
7. `Coupling constants / RDC / H-bonds` → `bio_j`
8. `Nucleic acids` → `bio_na`
9. `Fast methods (SOFAST / BEST / APSY)` → `bio_fast`
10. `Amino-acid selective (MUSIC)` → `bio_music`
11. `13C-detected (paramagnetic / IDP)` → `bio_cdet`

**`bio_hn`** — "Protein size / conditions?"
- `< 25 kDa, H2O: HSQC with PEP and water flip-back` → `L:hsqcetfpf3gpsi`
- `< 25 kDa: HSQC with WATERGATE` → `L:hsqcfpf3gpphwg`
- `> 25 kDa or high field: TROSY` → `L:trosyetf3gpsi`
- `TROSY with WATERGATE (deuterated / very large)` → `L:trosyf3gpph19`
- `Fast: SOFAST-HMQC` → `L:sfhmqcf3gpph`
- `Fast: BEST-HSQC` → `L:b_hsqcetf3gpsi`

**`bio_xedit`** — "Which?"
- `3D 15N-NOESY-HSQC (PEP)` → `L:noesyhsqcf3gpsi3d`
- `3D 15N-NOESY-HSQC (WATERGATE)` → `L:noesyhsqcf3gpwg3d`
- `3D 15N-TOCSY-HSQC` → `L:dipsihsqcf3gpsi3d`
- `3D 13C-NOESY-HSQC` → `L:noesyhsqcetgpsi3d`
- `3D 15N-NOESY-TROSY (large)` → `L:noesytretf3gp3d`
- `4D 13C/15N HSQC-NOESY-HSQC` → `L:hsqcnoesyhsqccngp4d`

**`bio_bb`** — "Experiment?"
1. `HNCO` → `bio_hnco`
2. `HNCA` → `bio_hnca`
3. `HN(CA)CO` → `bio_hncaco`
4. `HN(CO)CA` → `bio_hncoca`
5. `HNCACB` → `bio_hncacb`
6. `CBCA(CO)NH` → `bio_cbcaconh`
7. `HN(CO)CACB` → `bio_hncocacb`
8. `HNCANNH / HN(COCA)NNH (sequential via 15N)` → `bio_hncannh`

Version nodes (question "Version?"). Each option list: `PEP (< 25 kDa)`, `WATERGATE (better water suppression)`, `TROSY (> 25 kDa)`, `TROSY + 2H decoupling (deuterated)`, `BEST (fast)`, plus extras where noted:

| node | PEP | WATERGATE | TROSY | TROSY+2H | BEST | extra |
|---|---|---|---|---|---|---|
| `bio_hnco` | hncogp3d | hncogpwg3d | trhncogp3d | trhncogp2h3d | b_hncogp3d | — |
| `bio_hnca` | hncagp3d | hncagpwg3d | trhncagp3d | trhncagp2h3d | b_hncagp3d | `Intra-residue only (i)` → hncaigp3d |
| `bio_hncaco` | hncacogp3d | hncacogpwg3d | trhncacogp3d | trhncacogp2h3d | b_hncacogp3d | — |
| `bio_hncoca` | hncocagp3d | hncocagpwg3d | trhncocagp3d | trhncocagp2h3d | b_hncocagp3d | — |
| `bio_hncacb` | hncacbgp3d | hncacbgpwg3d | trhncacbgp3d | trhncacbgp2h3d | b_hncacbgp3d | `Intra-residue only (i)` → hncacbigp3d |
| `bio_cbcaconh` | cbcaconhgp3d | cbcaconhgpwg3d | trcbcaconhgp3d | (none) | (none) | `TROSY, echo-antiecho` → trcbcaconhetgp3d |
| `bio_hncocacb` | hncocacbgp3d | hncocacbgpwg3d | trhncocacbgp3d | trhncocacbgp2h3d | b_hncocacbgp3d | — |
| `bio_hncannh` | hncannhgp3d | hncannhgpwg3d | trhncannhgp3d | (none) | (none) | `HN(COCA)NNH (PEP)` → hncocannhgp3d; `HN(COCA)NNH TROSY` → trhncocannhgp3d |

**`bio_sc`** — "Which?"
- `CC(CO)NH (13C side chain to NH)` → `L:ccconhgp3d`
- `HCC(CO)NH (1H side chain to NH)` → `L:hccconhgp3d2`
- `HBHA(CO)NH` → `L:hbhaconhgpwg3d`
- `HBHANH` → `L:hbhanhgpwg3d`
- `CBCANH` → `L:cbcanhgp3d`
- `Aromatic (HB)CB(CGCD)HD` → `L:hbcbcgcdhdgp`

**`bio_hcch`** — "Which?"
- `HCCH-TOCSY (DIPSI mixing)` → `L:hcchdigp3d`
- `HCCH-COSY` → `L:hcchcosygp3d`
- `4D HCCH-TOCSY` → `L:hcchdigp4d`

**`bio_relax`** — "Which?"
- `15N T1 (pseudo-3D, PEP)` → `L:hsqct1etf3gpsi3d`
- `15N T2 (CPMG)` → `L:hsqct2etf3gpsi3d`
- `15N T1rho` → `L:hsqctretf3gpsi3d`
- `1H-15N heteronuclear NOE` → `L:hsqcnoef3gpsi`
- `TROSY-based T1 (large proteins)` → `L:trt1etf3gpsi3d`
- `TROSY-based T2` → `L:trt2etf3gpsi3d`
- `Rex (CPMG relaxation dispersion)` → `L:hsqcrexetf3gpsi3d`

**`bio_j`** — "Which?"
- `3J(HNHA) - HNHA` → `L:hnhagp3d`
- `RDC: IPAP 1H-15N HSQC` → `L:hsqcf3gpiaphwg`
- `RDC: IPAP-HNCO (1J CaC', 3J H(N)Ca)` → `L:hncogprc3d1`
- `Hydrogen bonds: 3hJ(NC') HNCO` → `L:hncogphb3d`
- `chi1: HNHB` → `L:hnhbgp3d`

**`bio_na`** — "Which?"
- `Imino 1H-15N HSQC (jump-return)` → `L:na_hsqcf3gpjrphxy`
- `HCN 3D (base-sugar, PEP)` → `L:na_hcnetgpsi3d`
- `HCN 2D (multiple quantum)` → `L:na_hcnmqgpphpr`
- `HCP 3D (31P backbone)` → `L:na_hcpetgpsi3d`
- `HNN-COSY (hydrogen bonds)` → `L:na_hnncosygpphwg`
- `3D 15N-NOESY-HSQC (WATERGATE)` → `L:na_noesyhsqcf3gpwg3d`

**`bio_fast`** — "Which?"
- `APSY (3,2)-HNCO` → `L:rd_hnco_32`
- `APSY (4,2)-HNCOCA` → `L:rd_hncoca_42`
- `BEST-HNCO` → `L:b_hncogp3d`
- `BEST-TROSY HNCO` → `L:b_trhncogp3d`
- `SOFAST-HMQC 2D` → `L:sfhmqcf3gpph`
- `BEST-HSQC 2D` → `L:b_hsqcetf3gpsi`

**`bio_music`** — "Amino acid type?"
- `Gly (and Asn/Gln side-chain NH2)` → `L:music_gly_3d`
- `Ala / Val / Ile / Leu (methyl)` → `L:music_lavia_3d`
- `Thr / Ala / Val / Ile` → `L:music_tavi_3d`
- `Cys / Met` → `L:music_cm_3d`
- `Asp / Glu` → `L:music_de_3d`
- `Ser` → `L:music_ser_3d`
- `Ile` → `L:music_ile_3d`
- `Lys / Arg` → `L:music_kr_3d`
- `Gln / Asn` → `L:music_qn_3d`
- `Pro (i+1)` → `L:music_pro_1_3d`
- `Aromatic (Phe / His / Tyr / Trp)` → `L:music_fhyw_3d`
- `Trp (2D)` → `L:music_trpe_2d`

**`bio_cdet`** — "Which?"
- `2D CACO (IPAP)` → `L:c_caco_ia`
- `2D CON` → `L:c_con_iasq`
- `3D CBCACON` → `L:c_cbcacon_ia3d`
- `3D HNCO 13C-detected (IPAP)` → `L:c_hnco_ia3d`
- `2D 13C-13C COSY` → `L:c_cosy`
- `3D CANCO` → `L:c_canco_ia3d`

- [ ] **Step 4: Add the leaves with `requires` labeling rules and notes**

Add every `L:` target above to `"leaves"` (template as in Task 6). Then apply labeling with this one-off script (run once; result is committed):

```bash
python3 - <<'EOF'
import json, io, re
p = "src/pp_tree.json"
t = json.load(io.open(p, encoding="utf-8"))
def reach(root):
    out, stack, seen = set(), [root], set()
    while stack:
        n = stack.pop()
        if n in seen: continue
        seen.add(n)
        for _, tg in t["nodes"][n]["opts"]:
            (out.add if tg.startswith("L:") else stack.append)(tg[2:] if tg.startswith("L:") else tg)
    return out
hn = reach("bio_hn") | reach("bio_relax") | {"hsqcf3gpiaphwg", "na_hsqcf3gpjrphxy", "na_hnncosygpphwg", "na_noesyhsqcf3gpwg3d"}
for name in reach("bio_exp"):
    req = [r for r in t["leaves"][name]["requires"] if "labeled" not in r]
    if name.startswith("na_"):
        req.append("13C/15N labeled RNA/DNA" if name not in hn else "15N labeled RNA/DNA")
    elif name in hn:
        req.append("15N labeled")
    else:
        req.append("13C/15N labeled")
    if "2h" in name:
        req.append("2H labeled")
    t["leaves"][name]["requires"] = req
json.dump(t, io.open(p, "w", encoding="utf-8"), indent=2, sort_keys=True)
EOF
```

Hand-written `notes`/`alt` for the main families (others keep defaults):

| leaf | notes | alt |
|---|---|---|
| hsqcetfpf3gpsi | `Standard 15N HSQC; water flip-back keeps water along z` | `hsqcfpf3gpphwg`, `hsqcetf3gpsi` |
| trosyetf3gpsi | `TROSY: no 15N decoupling during acquisition; use above ~25 kDa or > 600 MHz` | `trosyf3gpph19`, `trosyetf3gpsi.2` |
| sfhmqcf3gpph | `SOFAST: short d1 (0.1-0.3 s); band-selective 1H pulses` | `hetsfhmqcf3gpph` |
| noesyhsqcf3gpsi3d | `d8 = NOESY mixing (80-150 ms)` | `noesyhsqcetf3gp3d`, `noesyhsqcf3gpwg3d` |
| hncogp3d | `Set cnst21 (CO ppm), cnst22 (Ca ppm); getprosol after rpar; aqseq 321` | `hncogpwg3d`, `trhncogp3d`, `b_hncogp3d` |
| hncogpwg3d | `WATERGATE back-transfer; States-TPPI in F2 (no EA)` | `hncogp3d` |
| trhncogp3d | `TROSY; no decoupling during acquisition; cnst21/cnst22` | `trhncoetgp3d`, `trhncogp2h3d` |
| trhncogp2h3d | `Requires 2H lock-switch (pl17, cpd4:f4)` | `trhncogp3d` |
| b_hncogp3d | `BEST: amide-selective shaped 1H pulses; d1 0.2-0.4 s` | `b_trhncogp3d` |
| hncagp3d | `cnst21/cnst22; both i and i-1 Ca; use hncaigp3d for intra only` | `hncagpwg3d`, `trhncagp3d`, `hncaigp3d` |
| hncacbgp3d | `cnst21/cnst22/cnst23 (Cb); Ca and Cb opposite sign` | `hncacbgpwg3d`, `trhncacbgp3d` |
| cbcaconhgp3d | `Pair with HNCACB for sequential walk` | `cbcaconhgpwg3d`, `trcbcaconhgp3d` |
| hcchdigp3d | `d9 = 13C DIPSI-3 mixing (approx 12-20 ms); 13C carrier at 40 ppm` | `hcchdigp4d`, `hcchcosygp3d` |
| hsqct1etf3gpsi3d | `vdlist = relaxation delays; interleaved via nbl` | `hsqct1etgpsi3d` |
| hsqcnoef3gpsi | `Two interleaved spectra (sat / no sat); long d1 (>= 5 s)` | `trnoef3gpsi` |
| hncogprc3d1 | `IPAP: in-phase and anti-phase interleaved (F1I); record isotropic and aligned` | `hncogprcwg3d1` |
| rd_hnco_32 | `Projection angle via cnst51; process with APSY software` | `rd_hncoca_42` |
| music_gly_3d | `ZGOPTNS -DLABEL_GLY / -DLABEL_CO select t1 nucleus` | `music_gly_3d_2` |
| c_hnco_ia3d | `13C-detected; IPAP virtual decoupling (l0 parity); prosol triple_c` | `c_hncoca_ia3d` |
| na_hcnetgpsi3d | `Correlates H6/H8-C6/C8-N1/N9 and H1'-C1'-N1/N9` | `na_hcnmqgpphpr` |

- [ ] **Step 5: Run the build tool, then tests**

Run: `python3 tools/build_leaves.py && python3 tests/test_tree.py`
Expected: `31 tests, 0 failed`. Fix any `not in doc/pulseprogram` name by checking `ls doc/pulseprogram | grep <stem>`.

- [ ] **Step 6: Commit**

```bash
git add src/pp_tree.json tests/test_tree.py
git commit -m "feat: biomolecular decision tree (catalogue Vol. II)"
```

---

### Task 8: README, CLAUDE.md pointer, end-to-end console run

**Files:**
- Create: `README.md`
- Modify: `CLAUDE.md` (add a "pp_selector" section after "Working in this repo")

- [ ] **Step 1: Write README.md**

```markdown
# pp_selector — pulse program recommender for TopSpin

Asks a few questions inside TopSpin and recommends a Bruker standard pulse
program and parameter set. It only displays a recommendation; it never
changes the current dataset.

## Install (TopSpin 3.x / 4.x)

Copy two files into the user Python directory of your TopSpin installation:

    <TOPSPIN>/exp/stan/nmr/py/user/pp_selector.py
    <TOPSPIN>/exp/stan/nmr/py/user/pp_tree.json

(`<TOPSPIN>` is e.g. `/opt/topspin4.1.4` or `C:\Bruker\TopSpin4.1.4`.)

## Run

Type `pp_selector` on the TopSpin command line. Answer the dialogs
(`Back` returns to the previous question). The result window shows the
program, the parameter set (`rpar <SET> all ; getprosol`), requirements,
description, notes, alternatives, and your answers.

`[found in lists/pp]` / `[NOT found]` tells you whether the program exists in
this TopSpin's `exp/stan/nmr/lists/pp`. `[not checked]` means the install
path could not be determined — see next section.

## One-time check: TopSpin install path

The script reads `sys.registry.getProperty("XWINNMRHOME")`. Verify once on
the spectrometer PC by running in the TopSpin command line:

    edpy   (new file, paste:)  import sys; MSG(sys.registry.getProperty("XWINNMRHOME"))

If it shows the install directory, nothing to do. If it shows `None`, set
`"topspin_home"` in `pp_tree.json` to the install directory, e.g.
`"topspin_home": "/opt/topspin4.1.4"`.

Also verify once that `INPUT_DIALOG` returns `None` when `Back`/close is
pressed (used for questions with more than 4 options); if it returns the
values instead, pressing `Back` there behaves like `OK` — report it and the
adapter will be adjusted.

## Editing the tree

`pp_tree.json`:

- `nodes`: `{"q": "question", "opts": [["label", "node-id or L:program"], ...]}`.
  `L:` marks a leaf. Do not add a "Back" option; the runner adds it.
- `leaves`: `{"parset", "desc", "dim", "requires", "notes", "alt"}` per program.
  `parset`/`desc`/`dim` are filled by `python3 tools/build_leaves.py` from the
  library in `doc/pulseprogram/` and the catalogue tables; edit `requires`,
  `notes`, `alt` by hand.

Run `python3 tests/test_tree.py` after editing. It checks that every target
exists, every program name is a real library file, no node is orphaned, and
representative answer paths reach the expected programs.

## Test outside TopSpin

`python3 src/pp_selector.py` runs the same dialogs on the console (numbers
select options; empty input = Back).
```

- [ ] **Step 2: Add CLAUDE.md section**

Append to `CLAUDE.md` after the "Working in this repo" section:

```markdown
## pp_selector (the tool this repo builds)

- `src/pp_selector.py` (Jython 2.7 runner, must stay Python 2/3 compatible: no f-strings, no pathlib, `io.open`) + `src/pp_tree.json` (decision tree). Spec: `docs/superpowers/specs/2026-08-31-pp-selector-design.md`.
- `python3 tests/test_tree.py` — run after any change; it validates the tree against `doc/pulseprogram/` and the catalogue tables.
- `python3 tools/build_leaves.py` — refills `parset`/`desc`/`dim` for leaves; never edit those by hand, edit `requires`/`notes`/`alt` instead.
- Console smoke test: `python3 src/pp_selector.py`.
```

- [ ] **Step 3: End-to-end console run against the real tree**

Run: `printf '2\n3\n1\n1\n' | python3 src/pp_selector.py | tail -8`
(2 = bio, 3 = backbone, 1 = HNCO, 1 = PEP; the `bio_exp`/`bio_bb` questions use the numbered-list dialog so the numbers are 1-based; the root `SELECT` is 0-based so `1` there means the second button.)
Expected: last 8 lines are the report with `Recommended pulse program:  hncogp3d   [not checked]` and `Your answers:               Protein / nucleic acid (isotope labeled) > Backbone assignment (triple resonance) > HNCO > PEP (< 25 kDa)`. If the numbering differs, read the printed menus and adjust the input — the check is that a 4-answer path yields the hncogp3d report.

- [ ] **Step 4: Full test run and Jython syntax check**

Run: `python3 tests/test_tree.py && python3 -c "import ast; ast.parse(open('src/pp_selector.py').read())" && ! grep -nE 'f"|pathlib|-> ' src/pp_selector.py`
Expected: `31 tests, 0 failed`, no syntax error, no grep hits.

- [ ] **Step 5: Commit**

```bash
git add README.md CLAUDE.md
git commit -m "docs: README (install, path check, tree editing) and CLAUDE.md pointer"
```

---

## Self-review

- **Spec coverage:** §3 files → Tasks 1–5, 8. §4 JSON format/rules → Tasks 4, 6, 7 (per-experiment nodes, `L:` prefix, Back not in opts). §5 runner flow, dialogs, report format, error handling → Tasks 1–3. §6 Jython compatibility → Global Constraints + Task 3 Step 3 / Task 8 Step 4 checks. §7 build tool → Task 5. §8 tests 1–4 → Tasks 4 (integrity, library), 6–7 (paths), 2 (report). §9 out of scope respected. §10 XWINNMRHOME check → README (Task 8).
- **Placeholder scan:** none; every code step has full code; tree content is enumerated.
- **Type consistency:** `walk` returns `(name_or_None, answers)` everywhere; `exists(name)` returns `True/False/None` in `make_checker`, `format_report`, tests; `fill_leaves(tree, pp_dir, parsets, force)` signature matches Task 5 tests and `main`.
- **Known judgment calls:** leaf `hccconhgp3d2` labeled "HCC(CO)NH" — the catalogue lists variants 1/2/3 without a one-line distinction; the header `desc` filled by the build tool will show which nucleus evolves. `zg` appears under both 1D 1H and 1D 13C nodes on purpose.
