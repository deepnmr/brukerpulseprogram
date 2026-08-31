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
        "hncogp3d": {
            "parset": "HNCOGP3D",
            "dim": "3D",
            "desc": "HNCO",
            "requires": ["f3", "gradient", "13C/15N labeled"],
            "notes": "Set cnst21/cnst22",
            "alt": ["trhncogp3d", "b_hncogp3d"],
        },
        "zg30": {
            "parset": "PROTON",
            "dim": "1D",
            "desc": "1D 30 deg",
            "requires": [],
            "notes": "",
            "alt": ["zg"],
        },
        "cosygpqf": {
            "parset": None,
            "dim": "2D",
            "desc": "COSY",
            "requires": ["gradient"],
            "notes": "",
            "alt": [],
        },
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


def _exists_all(name):
    return True


def _exists_none(name):
    return False


def _exists_unknown(name):
    return None


def test_report_found():
    text = pp.format_report(MINI, "hncogp3d", ["Bio"], _exists_all)
    lines = text.split("\n")
    assert lines[0] == "Recommended pulse program:  hncogp3d   [found in lists/pp]", (
        lines[0]
    )
    assert (
        lines[1]
        == "Parameter set:              HNCOGP3D   ->  rpar HNCOGP3D all ; getprosol"
    ), lines[1]
    assert lines[2] == "Dimension:                  3D"
    assert lines[3] == "Requires:                   f3, gradient, 13C/15N labeled"
    assert lines[4] == "Description:                HNCO"
    assert lines[5] == "Notes:                      Set cnst21/cnst22"
    assert (
        lines[6] == "Alternatives:               trhncogp3d (found), b_hncogp3d (found)"
    ), lines[6]
    assert lines[7] == "Your answers:               Bio"
    assert len(lines) == 8, lines


def test_report_not_found_puts_existing_alt_first():
    def exists(name):
        return name == "b_hncogp3d"

    text = pp.format_report(MINI, "hncogp3d", ["Bio"], exists)
    lines = text.split("\n")
    assert lines[0].endswith("[NOT found]"), lines[0]
    assert (
        lines[6]
        == "Alternatives:               b_hncogp3d (found), trhncogp3d (NOT found)"
    ), lines[6]


def test_report_unchecked_and_no_parset():
    text = pp.format_report(MINI, "cosygpqf", ["Small", "COSY"], _exists_unknown)
    lines = text.split("\n")
    assert lines[0].endswith("[not checked]")
    assert (
        lines[1]
        == "Parameter set:              (no standard parameter set - start from a similar experiment)"
    ), lines[1]
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
            f.write(";zg\n")
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


def main():
    tests = [
        (n, f)
        for n, f in sorted(globals().items())
        if n.startswith("test_") and callable(f)
    ]
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
