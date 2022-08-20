;以下部分定义那些XIP文件被包含到ROM映像中
XIPFILES
;  rom文件名			原始文件名								   属性
   device.exe =:  $(BUILDRELEASE)\device.exe		                  SH
   gwme.exe =:  $(BUILDRELEASE)\gwme.exe		                      SH
   libbase.so =:  $(BUILDRELEASE)\libbase.so		                  SH
   shell.exe =:  $(BUILDRELEASE)\shell.exe		                  SH
   desktop.exe =:  $(BUILDRELEASE)\desktop.exe		                  SH
   usualapi.exe =:  $(BUILDRELEASE)\usualapi.exe		                  SH
;   fileview.exe =: $(BUILDRELEASE)\fileview.exe                 SH
   keyboard.exe =: $(BUILDRELEASE)\keyboard.exe                 SH
         
;以下部分定义那些BINARY文件被包含到ROM映像中
ROMFILES
;  rom文件名			原始文件名								   属性	

   test.exe =: $(BUILDRELEASE)\test.exe                 A
   kingmos.reg =: $(BUILDRELEASE)\kingmos.reg                SH
   EngbLowerCase.bmp =: $(__AMBO_OS_HOME)\Loadfile\EngbLowerCase.bmp          A  
   EngbLowerCaseInv.bmp =: $(__AMBO_OS_HOME)\Loadfile\EngbLowerCaseInv.bmp          A  
   EngbUpperCase.bmp =: $(__AMBO_OS_HOME)\Loadfile\EngbUpperCase.bmp          A  
   EngbUpperCaseInv.bmp =: $(__AMBO_OS_HOME)\Loadfile\EngbUpperCaseInv.bmp          A  
   pyimage.bmp =: $(__AMBO_OS_HOME)\Loadfile\pyimage.bmp          A  
   EngbLowerCase.bmp =: $(__AMBO_OS_HOME)\Loadfile\EngbLowerCase.bmp          A  
   mlgsign.bmp =: $(__AMBO_OS_HOME)\Loadfile\mlgsign.bmp          A  
	