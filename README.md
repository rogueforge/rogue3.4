# Rogue 3.4

Reconstructed source for Rogue 3.4 (Toy, Arnold, Wichman; Berkeley 1981).
Compiled with a modified 2.8BSD toolchain, it byte-exactly reproduces the
executable `2.8/usr/bin.v7/ucb/rogue`
(md5 `88f791623b2a684d2555891c0f5b6735`) from CSRG Archive CD-ROM Disc 1.
`rogue.doc` is the formatted guide (`2.8/usr/doc/rogue.doc`,
md5 `e7113c0e9aedca6edf47f2127cfad4f7`). `rogue.6` is the manual page from
4.1BSD (`usr/man/man6/rogue.6`, md5 `f84eef587bc38bc0823f2cd029df030b`).

The binary was stripped, so identifier names are inferred (mostly from other
rogue sources); comments and whitespace are likewise inferred from available
sources, as is most of the Makefile.
