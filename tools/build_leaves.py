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
    print(
        "filled %d fields in %d leaves" % (len(filled), len(set(n for n, _ in filled)))
    )


if __name__ == "__main__":
    main(sys.argv[1:])
