# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repository is

A reference workspace for Bruker TopSpin NMR pulse programs: the stock TopSpin pulse program library plus the official manual. There is no build, lint, or test tooling — pulse programs are plain text interpreted by TopSpin on the spectrometer and cannot be compiled or run here. Validation means reading the manual and cross-checking against sibling programs in the library.

- `doc/pulse-programming.pdf` — *"NMR Pulse Programming for AVANCE NEO and Fourier 80"* (H166633 rev 023, Dec 2025). Authoritative syntax reference; read with the `pages` parameter. Ch. 2 basic syntax, 3 pulses, 4 delays, 5 simultaneous pulses/delays, 6 decoupling.
- `doc/pulseprogram/` — ~2,000 files, the TopSpin `exp/stan/nmr/lists/pp` library (mostly `;avance-version (21/09/15)` and later; `Update.info` lists what changed per release, newest 25/07/14).
- `doc/ppcatalogue-1.pdf` / `doc/ppcatalogue-2.pdf` — Parella, *Pulse Program Catalogue* Vol. I (1D & 2D) / Vol. II (Biomolecular), TopSpin 3.0, 2010. Per-experiment pulse diagrams with real parameter names, `(pulseprogram | PARAMETERSET)` version lists, and (Vol. I) the "NMR Building Blocks" section that pairs each sequence element with its actual code. Structural analyses of the cited programs: `doc/ppcatalogue-1-structure.md` (596 programs), `doc/ppcatalogue-2-structure.md` (525); name→parameter-set tables: `doc/ppcatalogue-{1,2}-programs.txt`.

## Library conventions (read these before writing or naming anything)

- `Pulprog.info` — the naming scheme. Name = experiment stem + two-letter codes in alphabetical order, e.g. `hmbcgplpndqf` = hmbc + gp (`:gp` gradients) + lp (low-pass filter) + nd (no decoupling) + qf (absolute value). Look up codes there instead of guessing.
- `Relations.info` / `Param.info` / `Param_solids.info` — which `p*`, `pl*`, `sp*`, `d*`, `cnst*` numbers are reserved for what, and how they map to prosol. Reuse the standard slots (p1/pl1 = f1 90°, p3/pl2 = f2 90°, p21/pl3 = f3 90°, p16/d16 = gradient, d11 = disk I/O, d1 = relaxation…) so `getprosol` works.
- Name prefixes: `b_` BEST, `na_` nucleic acids, `c_` / `n_` 13C- / 15N-detected, `music_` MUSIC, `sf_` SOFAST, `ht_` Hadamard, `hos_` ALSOFAST, `reset_` pure-shift (PSYCHE / ZS) family, `npt_` = `;$CLASS=HighRes HWT` hardware/performance-test programs (tracked separately in `Update_hwt.info`, `;$HIDE=y`).
- Suffixes: `.2`, `.3`, `.4` = alternative implementations of the same experiment (different pulse scheme, e.g. semi-constant-time vs constant-time), not versions; `.incl` = `#include` headers; `.cp`/`.dp`/`.dcp`/`.tcp` = solids CP/DP building blocks; `.bsh` = TopSolids BSH sequences; `.info` = documentation.
- Metadata tags every program carries and TopSpin's browser filters on: `;$CLASS=` (`HighRes`, `Solids`, `BioSolids`, `SolidsIcon`, `… Incl`, `… Info`), `;$DIM=`, `;$TYPE=`, `;$SUBTYPE=`, `;$COMMENT=`. `prosol relations=<triple>` (or `triple_c`, `triple_na`, `solids_cp`, `lcnmr`…) selects the prosol relation table.

## Anatomy of a pulse program

Every file follows the same layout (see `zg` for the minimal case, `hsqcetgpsi` for a typical 2D):

1. Header comments: `;name`, `;avance-version (yy/mm/dd)`, description, literature refs, `;$CLASS…` tags, optional `prosol relations=`.
2. `#include <Avance.incl>` (always), plus `<Grad.incl>`, `<Delay.incl>` as needed; solids use `<Avancesolids.incl>` and the `*_prot.incl` protection files.
3. Quoted definitions: `"p2=p1*2"`, `"d0=3u"`, `"in0=inf1/2"`, `"acqt0=…"`; `#ifdef` blocks for ZGOPTNS flags such as `LABEL_CN`, `TRIMP`.
4. Body: numbered labels, `ze`, delays/pulses with `:f2`, `:gp1`, `pl1:f1`, `go=2 ph31`, then the `mc` macro with `F1EA(…)`/`F1PH(…)` increment clauses, `exit`.
5. Phase programs `ph0`–`ph31`.
6. Parameter comments `;p1 : f1 channel - 90 degree high power pulse`, `;gpz1: 80%`, `;gpnam1: SMSQ10.100` — these are parsed by TopSpin's `ased`/prosol, keep the exact `;name: text` form.
7. `;preprocessor-flags-start` … `;preprocessor-flags-end` block documenting `-D` options, then `;$Id:$`.

Do not delete `;avance-version`, `;begin ___`/`;end ___`, or the preprocessor-flags comments — NMRSIM and TopSpin evaluate them (`Pulprog.info`).

## Working in this repo

- Target hardware is AVANCE NEO / Fourier 80 (TopSpin 4.x). `.bsh` and some solids files still note "Avance II / AVIII"; check the manual before reusing constructs from them.
- New or modified programs go in `doc/pulseprogram/` alongside the originals; derive from the closest existing program (`grep -l '^;\$TYPE=…' doc/pulseprogram/*`, or search by description line) rather than writing from scratch.
- Bruker raw data (`fid`, `ser`, `acqus`, `pdata/`) must never be committed — gitignore it.

## pp_selector (the tool this repo builds)

- `src/pp_selector.py` (Jython 2.7 runner, must stay Python 2/3 compatible: no f-strings, no pathlib, no `io` module — TopSpin 5 bundles a broken Jython `io`, use `codecs.open`; never reference `__file__`, TopSpin aborts the script) + `src/pp_tree.json` (decision tree). Spec: `docs/superpowers/specs/2026-08-31-pp-selector-design.md`.
- `python3 tests/test_tree.py` — run after any change; it validates the tree against `doc/pulseprogram/` and the catalogue tables.
- `python3 tools/build_leaves.py` — refills `parset`/`desc`/`dim` for leaves; never edit those by hand, edit `requires`/`notes`/`alt` instead.
- Console smoke test: `python3 src/pp_selector.py`.
