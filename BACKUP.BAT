@echo off
attrib -a *.bak
attrib -a *.csm
attrib -a *.obj
attrib -a *.~*
xcopy/a/y *.* a:
attrib -a *.*
echo Please Insert Disk 2
pause
attrib -a datafiles\*.dll
attrib -a datafiles\*.bak
attrib -a datafiles\*.csm
xcopy/a/y datafiles\*.* a:\datafiles
attrib -a datafiles\*.*
xcopy/a/y wpage\*.* a:\wpage
attrib -a wpage\*.*
xcopy/a/y beta\*.* a:\beta
attrib -a beta\*.*
@echo on