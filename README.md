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
