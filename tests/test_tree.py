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
