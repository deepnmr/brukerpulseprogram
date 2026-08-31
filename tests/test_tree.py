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
                seen_leaves.add(target[len(pp.LEAF_PREFIX) :])
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
        assert isinstance(leaf["requires"], list) and isinstance(leaf["alt"], list), (
            name
        )


sys.path.insert(0, os.path.join(ROOT, "tools"))
import build_leaves as bl  # noqa: E402


def test_read_header_hncogp3d():
    h = bl.read_header(os.path.join(PP_DIR, "hncogp3d"))
    assert h["dim"] == "3D", h
    assert h["desc"] == (
        "HNCO; 3D sequence with; inverse correlation for triple resonance using multiple; "
        "inept transfer steps"
    ), h["desc"]
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
            "hncogp3d": {
                "parset": None,
                "desc": None,
                "dim": None,
                "requires": [],
                "notes": "keep",
                "alt": ["x"],
            },
            "zg": {
                "parset": "MYSET",
                "desc": "my desc",
                "dim": None,
                "requires": ["custom"],
                "notes": "",
                "alt": [],
            },
        }
    }
    filled = bl.fill_leaves(tree, PP_DIR, {"hncogp3d": "HNCOGP3D"})
    a = tree["leaves"]["hncogp3d"]
    assert (
        a["parset"] == "HNCOGP3D" and a["dim"] == "3D" and a["desc"].startswith("HNCO")
    )
    assert a["requires"] == ["f3", "gradient"], a["requires"]
    assert a["notes"] == "keep" and a["alt"] == ["x"]
    b = tree["leaves"]["zg"]
    assert b["parset"] == "MYSET" and b["desc"] == "my desc" and b["dim"] == "1D"
    assert b["requires"] == ["custom"]
    assert ("hncogp3d", "parset") in filled and ("zg", "parset") not in filled


def test_fill_leaves_force_overwrites_auto_fields_only():
    tree = {
        "leaves": {
            "zg": {
                "parset": "MYSET",
                "desc": "my desc",
                "dim": "9D",
                "requires": ["custom"],
                "notes": "n",
                "alt": ["a"],
            }
        }
    }
    bl.fill_leaves(tree, PP_DIR, {"zg": "PROTON"}, force=True)
    z = tree["leaves"]["zg"]
    assert z["parset"] == "PROTON" and z["desc"] == "1D sequence" and z["dim"] == "1D"
    assert z["requires"] == ["custom"] and z["notes"] == "n" and z["alt"] == ["a"]


def _path(labels_to_pick):
    """Drive walk() by option LABEL text instead of index."""
    picks = list(labels_to_pick)

    def ask(question, labels):
        want = picks.pop(0)
        assert want in labels, "%r not offered for %r; offered %r" % (
            want,
            question,
            labels,
        )
        return labels.index(want)

    return ask


def test_path_sm_cosy_purge():
    leaf, _ = pp.walk(
        real_tree(),
        _path(
            [
                "Small molecule (1D / 2D)",
                "COSY",
                "Magnitude (fast, no phasing)",
                "Purge pulses before d1",
            ]
        ),
    )
    assert leaf == "cosygpppqf", leaf


def test_path_sm_hsqc_edited_adiabatic():
    leaf, _ = pp.walk(
        real_tree(),
        _path(
            [
                "Small molecule (1D / 2D)",
                "HSQC (1H-13C one-bond)",
                "Multiplicity edited (CH/CH3 up, CH2 down)",
                "Adiabatic 180 pulses, sensitivity improved (recommended)",
            ]
        ),
    )
    assert leaf == "hsqcedetgpsisp2.3", leaf


def test_path_sm_hmbc_standard():
    leaf, _ = pp.walk(
        real_tree(),
        _path(
            [
                "Small molecule (1D / 2D)",
                "HMBC (1H-13C long-range)",
                "Magnitude, low-pass J filter, no decoupling (standard)",
            ]
        ),
    )
    assert leaf == "hmbcgplpndqf", leaf


def test_path_sm_zg30():
    leaf, _ = pp.walk(
        real_tree(),
        _path(
            [
                "Small molecule (1D / 2D)",
                "1D 1H",
                "30 deg pulse, no suppression (routine)",
            ]
        ),
    )
    assert leaf == "zg30", leaf


def test_path_bio_hnco_pep():
    leaf, _ = pp.walk(
        real_tree(),
        _path(
            [
                "Protein / nucleic acid (isotope labeled)",
                "Backbone assignment (triple resonance)",
                "HNCO",
                "PEP (< 25 kDa)",
            ]
        ),
    )
    assert leaf == "hncogp3d", leaf


def test_path_bio_hnco_trosy():
    leaf, _ = pp.walk(
        real_tree(),
        _path(
            [
                "Protein / nucleic acid (isotope labeled)",
                "Backbone assignment (triple resonance)",
                "HNCO",
                "TROSY (> 25 kDa)",
            ]
        ),
    )
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
                leaves.add(target[len(pp.LEAF_PREFIX) :])
            else:
                stack.append(target)
    bad = [
        n for n in leaves if not any("labeled" in r for r in t["leaves"][n]["requires"])
    ]
    assert not bad, "bio leaves without labeling requirement: %s" % sorted(bad)


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
