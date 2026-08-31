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
                if target[len(LEAF_PREFIX) :] not in leaves:
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
            return target[len(LEAF_PREFIX) :], answers
        stack.append(target)
    return None, []


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
        lines.append(
            _row("Parameter set", "%s   ->  rpar %s all ; getprosol" % (parset, parset))
        )
    else:
        lines.append(
            _row(
                "Parameter set",
                "(no standard parameter set - start from a similar experiment)",
            )
        )
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
