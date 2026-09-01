"""pp_selector - recommend a Bruker pulse program by asking questions.

Runs inside TopSpin (Jython 2.7, 3.x/4.x) via the `edpy` mechanism and
also imports under CPython 3 for testing. Knowledge lives in pp_tree.json
next to this file. This tool never modifies the dataset.
"""

from __future__ import print_function
import codecs
import json
import os
import sys

LEAF_PREFIX = "L:"
BACK = -1


class TreeError(Exception):
    pass


def load_tree(path):
    try:
        with codecs.open(path, encoding="utf-8") as f:
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


TITLE = "pp_selector"
MAX_BUTTONS = 4  # more options than this -> numbered list + text entry

try:
    from TopCmds import SELECT, INPUT_DIALOG, VIEWTEXT, MSG  # noqa: F401  (TopSpin builtins)

    IN_TOPSPIN = True
except ImportError:
    IN_TOPSPIN = False

    try:
        _input = raw_input  # noqa: F821 (Python 2 / Jython)
    except NameError:
        _input = input

    def _readline(prompt):
        try:
            return _input(prompt)
        except EOFError:  # piped stdin ran out -> treat as Back/cancel
            return ""

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
    header = (
        question + "\n" + "\n".join("%d) %s" % (i + 1, l) for i, l in enumerate(labels))
    )
    result = INPUT_DIALOG(
        TITLE, header, ["Choice number:"], ["1"], [""], ["1"], ["OK", "Back"]
    )
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
    # TopSpin's xpy leaves __file__ unset (and a bare __file__ lookup aborts the
    # script with "Command cancelled"), but sys.argv[0] holds the script path.
    path = globals().get("__file__") or (sys.argv[0] if sys.argv else "")
    if path:
        return os.path.dirname(os.path.abspath(path))
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
