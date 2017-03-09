del borgdist.zip
del borgsrc.zip
pkzip -ex borgdist borg*.exe readme.txt version.txt borghelp.zip
pkzip -ex borgsrc *.cpp *.h *.rh *.ico *.rc readme.txt version.txt *.mak
