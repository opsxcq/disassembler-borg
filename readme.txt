Borg Notes
==========

There are some notes at the bottom of this file (just tend to be working notes from which I add and delete stuff and generally take absolutely no notice of it ;)) which summarise the list of things to be done yet, and any current bugs. Basically if it's on this list then it isn't implemented yet!  Suggestions for change are also welcome, in fact I would appreciate just receiving an email with your top ten list of things you would like to see changed in Borg. If you are reporting problems back it would be useful to know as much about any problems as possible :-
1. Which file was loaded up ?
2. What was the problem ?
3. Is it reproducible, if so how ? If not then what was happening at the time ?

I haven't done much testing in some areas (eg Z80 processors) and so there could well be a few errors in some parts. 

A lot of the current work on Borg is on making the code a bit better in terms of structure and readability. I am trying to move variables from global->module->local definitions, renaming of variables and routines, splitting up difficult and hard to follow code into simpler and easier to follow routines, and reconsidering many of the techniques that have been used. Routines have been moved in and out of classes and there is still quite a bit of restructuring to do.

Current Bugs:
-------------
1. Some data can be lost at the end of a seg when two segs overlap (happens in PE files when resources are loaded and segs created for each resource).

To Do:
------
1. Complete NE format. NE needs sorting out - imported functions, exported functions and the whole rest of the NE format. Very basic load at the moment.
2. LE/LX file format. Don't think I have time to look at this for quite a while - anyone want to take it on ? May need a rethink on segment types, and identifying 16/32 bit code. This won't be looked at by me for some time to come (probably).
3. Dos exes. Better dseg detection - practically no detection of dseg at the moment. Load additional code at end of DOS exe, as an option. Possibly extend seg in a DOS exe when code runs over seg bounds. More reloc/offset identifications.
4. Split seg option.
5. Work on xrefs. More xrefs/Check what is being xreffed.
6. Work on save as text and save as asm options. Full xrefs in save as text option. Xrefs -> loc_n in "save as asm" options. Need to look at save as asm and improve it more. 
7. Work on interface. Icons at top of screen. Tooltip help.
8. Data identification. Identification of strings in data. Identification of bytes/words/dwords in data.
9. Work on general navigation and use options. Jump to ADDR. Offsets by current seg/any seg. Change processor type during running. View summary info option. Database integrity check and recovery option. Help files (someone to take this on ?). Search engine additions (I have some plans).
10. Internal changes to program. Definite data and possible data priorities. Arg type changed from offset type - need to delete xref. Rep table, for rep KNI insts.
11. Better name demangler is needed.
12. Comment and Help Support - eg for Windows functions and for DOS calls.
13. Library routine identification.
14. Code flow analysis. Pseudo-code output and partial decompilation.
15. Renaming stack variables.
16. Macro support (perl suggested, or ficl a possibility - or 4th).
17. VB 'disassembly', which after all is only turning tokens into asm type instructions. So this could be added in more or less the same way as a different processor, although further code would be needed I don't think it would be too hard - an interactive VB disassembler ? Java bytecode ?
18. Support for unpackers, either a front end run of an unpacker program or inclusion of code for a built in unpacker. [decompression/decryption through emulation and control on loading file].
19. A resource parser - data analysis of version_info structure, menus, accelerators, message tables (? anything else i missed ?).
20. Built in debugger.
21. Hex editting, and updating the original file. Patching.
22. Identify args of windows routines.
23. OBJ file support. Allow analysis of obj files from within Borg, to generate a library of function names for loading in order to add compilers or to add obj files for a specific project.
24. Support debug sections.
25. search for next code/undefined.
26. jump to address.
27. rework some of the older code. Rewrite main engine for readability, more routines, better naming and clearer function use. separate disio and disasm better.
