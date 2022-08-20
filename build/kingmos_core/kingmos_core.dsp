# Microsoft Developer Studio Project File - Name="kingmos_core" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=kingmos_core - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "kingmos_core.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "kingmos_core.mak" CFG="kingmos_core - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "kingmos_core - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "kingmos_core - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "kingmos_core - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\include" /I "..\..\arch\i386-win\include" /I "..\..\drivers\include" /I "..\..\kingmos_private\include" /I "..\..\kingmos_private\os\wapsrv\inc" /I "..\..\kingmos_private\os\netsrv\inc" /I "..\..\kingmos_private\os\internet\inc" /D "NDEBUG" /D "VC386" /D "KINGMOS" /D "KINGMOS_CORE" /D "WIN32" /D "_MBCS" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "kingmos_core - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /I "..\..\include" /I "..\..\arch\i386-win\include" /I "..\..\drivers\include" /I "..\..\kingmos_private\include" /I "..\..\kingmos_private\os\wapsrv\inc" /I "..\..\kingmos_private\os\netsrv\inc" /I "..\..\kingmos_private\os\internet\inc" /D "_DEBUG" /D "VC386" /D "KINGMOS" /D "KINGMOS_CORE" /D "WIN32" /D "_MBCS" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Winmm.lib Ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "kingmos_core - Win32 Release"
# Name "kingmos_core - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Application"

# PROP Default_Filter ""
# Begin Group "apmain"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\application\apmain\apmain.c
# End Source File
# End Group
# Begin Group "test"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\application\test\test.c
# End Source File
# End Group
# End Group
# Begin Group "arch"

# PROP Default_Filter ""
# Begin Group "i386-win"

# PROP Default_Filter ""
# Begin Group "kernel"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\arch\i386-win\kernel\debugwin.c"
# End Source File
# Begin Source File

SOURCE="..\..\arch\i386-win\kernel\emldsk.c"
# End Source File
# Begin Source File

SOURCE="..\..\arch\i386-win\kernel\emlos.c"
# End Source File
# Begin Source File

SOURCE="..\..\arch\i386-win\kernel\emlsocket.c"
# End Source File
# Begin Source File

SOURCE="..\..\arch\i386-win\kernel\kingmos.rc"
# End Source File
# Begin Source File

SOURCE="..\..\arch\i386-win\kernel\regio.c"
# End Source File
# End Group
# Begin Group "drivers_pdd"

# PROP Default_Filter ""
# Begin Group "display16bpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\arch\i386-win\drivers\display16bpp\w32cfg.c"
# End Source File
# Begin Source File

SOURCE="..\..\arch\i386-win\drivers\display16bpp\windisp.c"
# End Source File
# Begin Source File

SOURCE="..\..\arch\i386-win\drivers\display16bpp\windrv.c"
# End Source File
# End Group
# Begin Group "sysgwdi"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\arch\i386-win\drivers\sysgwdi\sysgwdi.c"
# End Source File
# End Group
# Begin Group "keybrd"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\arch\i386-win\drivers\keybrd\keybrd.c"
# End Source File
# End Group
# Begin Group "mouse"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\arch\i386-win\drivers\mouse\mouse.c"
# End Source File
# End Group
# Begin Group "debuger"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\arch\i386-win\drivers\debuger\windebug.c"
# End Source File
# End Group
# End Group
# Begin Group "sysdev"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\arch\i386-win\sysdev\sysdev.c"
# End Source File
# End Group
# Begin Group "boot"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\arch\i386-win\boot\boot.c"
# End Source File
# End Group
# End Group
# End Group
# Begin Group "component"

# PROP Default_Filter ""
# Begin Group "desktop"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\component\desktop\desktop.c
# End Source File
# End Group
# Begin Group "keyboard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\component\keyboard\engkbd.c
# End Source File
# Begin Source File

SOURCE=..\..\component\keyboard\hwfunc.c
# End Source File
# Begin Source File

SOURCE=..\..\component\keyboard\keybdsrv.c
# End Source File
# Begin Source File

SOURCE=..\..\component\keyboard\keyboard.c
# End Source File
# Begin Source File

SOURCE=..\..\component\keyboard\keyvirtualtochar.c
# End Source File
# Begin Source File

SOURCE=..\..\component\keyboard\lxsearch.c
# End Source File
# Begin Source File

SOURCE=..\..\component\keyboard\pykbd.c
# End Source File
# Begin Source File

SOURCE=..\..\component\keyboard\pysearch.c
# End Source File
# Begin Source File

SOURCE=..\..\component\keyboard\pytab.c
# End Source File
# Begin Source File

SOURCE=..\..\component\keyboard\spellwnd.c
# End Source File
# End Group
# Begin Group "shell"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\component\shell\eshell.c
# End Source File
# Begin Source File

SOURCE=..\..\component\shell\shellsrv.c
# End Source File
# End Group
# Begin Group "base"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\component\base\base.c
# End Source File
# End Group
# End Group
# Begin Group "drivers"

# PROP Default_Filter ""
# Begin Group "fontcfg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\drivers\fontcfg\sysfont.c
# End Source File
# End Group
# Begin Group "kfsromdisk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\drivers\kfsromdisk\filerom.c
# End Source File
# Begin Source File

SOURCE=..\..\drivers\kfsromdisk\kromdisk.c
# End Source File
# End Group
# Begin Group "ramdisk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\drivers\ramdisk\ramdisk.c
# End Source File
# End Group
# Begin Group "com No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\drivers\com\ser_drv.c

!IF  "$(CFG)" == "kingmos_core - Win32 Release"

!ELSEIF  "$(CFG)" == "kingmos_core - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Group "private"

# PROP Default_Filter ""
# Begin Group "apicall"

# PROP Default_Filter ""
# Begin Group "apisrv"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\apilib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\caretlib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\classlib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\corelib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\devlib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\dlglib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\filelib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\fsmgrlib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\gdilib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\keybdllib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\menulib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\msglib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\reglib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\rgnlib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\shelllib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\sysetlib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\tablelib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\usuallib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\wavelib.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\apisrv\winlib.c
# End Source File
# End Group
# Begin Group "unicode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\unicode\gb2312d.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\unicode\unicode.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\apisrv\unicode\utf8.c
# End Source File
# End Group
# End Group
# Begin Group "os"

# PROP Default_Filter ""
# Begin Group "core"

# PROP Default_Filter ""
# Begin Group "kernel_core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\alarmmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\apisrv.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\blkheap.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\coresrv.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\downup.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\hmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\intrapi.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\isr.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\kalloc.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\kassert.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\klheap.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\klmisc.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\ksysmem.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\pagemgr.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\process.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\program.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\qemodule.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\sche.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\semaphor.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\sleep.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\start.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\system.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\thread.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\kernel\tick.c
# End Source File
# End Group
# Begin Group "win32_core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\emlcpu.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\inittss.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\intrlock.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\irq.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\mmu.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\oeminit.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\oemintr.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\oemtime.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\switch.asm

!IF  "$(CFG)" == "kingmos_core - Win32 Release"

!ELSEIF  "$(CFG)" == "kingmos_core - Win32 Debug"

# Begin Custom Build
InputDir=\kingmos\Kingmos_core\kingmos_private\os\core\win32
IntDir=.\Debug
InputPath=..\..\kingmos_private\os\core\win32\switch.asm
InputName=switch

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\masm\ml.exe -c -coff -Cx -Fo$(IntDir)\$(InputName).obj $(InputDir)\$(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\switchto.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\sysintr.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\try.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\core\win32\virmem.c
# End Source File
# End Group
# End Group
# Begin Group "device"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\device\device.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\device\devsrv.c
# End Source File
# End Group
# Begin Group "gwme_drv"

# PROP Default_Filter ""
# Begin Group "gdidrv"

# PROP Default_Filter ""
# Begin Group "32bpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gdidrv\32bpp\gdidrv32b.c
# End Source File
# End Group
# Begin Group "16bpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gdidrv\16bpp\gdidrv16b.c
# End Source File
# End Group
# Begin Group "1bpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gdidrv\1bpp\gdidrv1b.c
# End Source File
# End Group
# Begin Group "sysfont"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gdidrv\sysfont\efont8x6.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gdidrv\sysfont\efont8x8.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gdidrv\sysfont\engcode.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gdidrv\sysfont\phonetic.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gdidrv\sysfont\symfont.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gdidrv\sysfont\sysfont16.c
# End Source File
# End Group
# End Group
# Begin Group "gwme"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\calibrat.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\caret.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\class.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\gdi.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\gwme.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\gwmesrv.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\imageobj.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\keydrv.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\largenum.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\loadimg.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\message.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\msgqueue.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\paintmsg.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\posdrv.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\rgn.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\stockobj.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\sysset.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\timer.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\win.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\winfont.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_drv\gwme\wintimer.c
# End Source File
# End Group
# End Group
# Begin Group "gwme_gui"

# PROP Default_Filter ""
# Begin Group "defproc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\defproc\defproc.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\defproc\dialog.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\defproc\msgbox.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\defproc\stockstr.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\defproc\touchcal.c
# End Source File
# End Group
# Begin Group "wndclass"

# PROP Default_Filter ""
# Begin Group "sysclass"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\wndclass\sysclass\sysclass.c
# End Source File
# End Group
# Begin Group "static"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\wndclass\static\static.c
# End Source File
# End Group
# Begin Group "button"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\wndclass\button\button.c
# End Source File
# End Group
# Begin Group "combobox"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\wndclass\combobox\combbox.c
# End Source File
# End Group
# Begin Group "edit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\wndclass\edit\edit.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\wndclass\edit\editbase.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\wndclass\edit\multiple.c
# End Source File
# End Group
# Begin Group "menu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\wndclass\menu\menu.c
# End Source File
# End Group
# Begin Group "list"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\wndclass\list\list.c
# End Source File
# End Group
# Begin Group "scrollbar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\gwme_gui\wndclass\scrollbar\scrlbar.c
# End Source File
# End Group
# End Group
# End Group
# Begin Group "filesys"

# PROP Default_Filter ""
# Begin Group "kmfs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\filesys\kmfs\kmfs.c
# End Source File
# End Group
# Begin Group "fsmain"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\kingmos_private\os\filesys\fsmain\filesrv.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\filesys\fsmain\fsdmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\kingmos_private\os\filesys\fsmain\fsmain.c
# End Source File
# End Group
# End Group
# End Group
# End Group
# Begin Group "library"

# PROP Default_Filter ""
# Begin Group "stdlib"

# PROP Default_Filter ""
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\library\stdlib\common\_sprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\assert.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\bheap.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\dbgalloc.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\dbgcs.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\ectype.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\filepath.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\fncmp.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\malloc.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\objlist.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\ptrlist.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\rect.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\stdlib.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\time.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\common\ttime.c
# End Source File
# End Group
# Begin Group "i386"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\library\stdlib\i386\exception.asm

!IF  "$(CFG)" == "kingmos_core - Win32 Release"

!ELSEIF  "$(CFG)" == "kingmos_core - Win32 Debug"

# Begin Custom Build
InputDir=\kingmos\Kingmos_core\library\stdlib\i386
IntDir=.\Debug
InputPath=..\..\library\stdlib\i386\exception.asm
InputName=exception

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\masm\ml.exe -c -coff -Cx -Fo$(IntDir)\$(InputName).obj $(InputDir)\$(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\i386\longjmp.asm

!IF  "$(CFG)" == "kingmos_core - Win32 Release"

!ELSEIF  "$(CFG)" == "kingmos_core - Win32 Debug"

# Begin Custom Build
InputDir=\kingmos\Kingmos_core\library\stdlib\i386
IntDir=.\Debug
InputPath=..\..\library\stdlib\i386\longjmp.asm
InputName=longjmp

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\masm\ml.exe -c -coff -Cx -Fo$(IntDir)\$(InputName).obj $(InputDir)\$(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\i386\setjmp.asm

!IF  "$(CFG)" == "kingmos_core - Win32 Release"

!ELSEIF  "$(CFG)" == "kingmos_core - Win32 Debug"

# Begin Custom Build
InputDir=\kingmos\Kingmos_core\library\stdlib\i386
IntDir=.\Debug
InputPath=..\..\library\stdlib\i386\setjmp.asm
InputName=setjmp

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\masm\ml.exe -c -coff -Cx -Fo$(IntDir)\$(InputName).obj $(InputDir)\$(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\i386\string.c
# End Source File
# Begin Source File

SOURCE=..\..\library\stdlib\i386\wstring.c
# End Source File
# End Group
# End Group
# Begin Group "userlib"

# PROP Default_Filter ""
# Begin Group "classdef"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\library\userlib\classdef\strclass.c
# End Source File
# End Group
# Begin Group "universe"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\library\userlib\universe\imglist.c
# End Source File
# Begin Source File

SOURCE=..\..\library\userlib\universe\playsnd.c
# End Source File
# Begin Source File

SOURCE=..\..\library\userlib\universe\stracmp.c
# End Source File
# Begin Source File

SOURCE=..\..\library\userlib\universe\universe.c
# End Source File
# Begin Source File

SOURCE=..\..\library\userlib\universe\ymdlunar.c
# End Source File
# End Group
# End Group
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
