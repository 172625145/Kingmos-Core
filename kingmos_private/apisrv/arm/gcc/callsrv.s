/* ******************************************************
 * @copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
 * ******************************************************
 * 
 * ******************************************************
 * �ļ�˵����ϵͳ�������幦��
 * �汾�ţ�1.0.0
 * ����ʱ�ڣ�2001-04-04
 * ���ߣ�����
 * �޸ļ�¼��
 * *******************************************************
 */

.nolist
.include "linkage.inc"
.list

.extern    _Sys_EntryTry

	.text
    @ DWORD WINAPI CALL_SERVER( CALLTRAP * lpcs, ... );
	@ �������ļ�eapisrv.h
    @ r0 = lpcs
	@ r1 = arg1
	@ r2 = arg2
	@ ...

    ENTRY CALL_SERVER
	ldr r12, [r0]
	ldr r0,  [r0,#0x4]
    swi 0


    @LRESULT WINAPI Sys_ImplementCallBack( LPCALLBACKDATA, ... );
    @LRESULT WINAPI Sys_ImplementCallBack4( LPCALLBACKDATA, ... );
    ENTRY Sys_ImplementCallBack
	ldr  r12, =0xF00404B8
	swi 0

    ENTRY Sys_ImplementCallBack4
	ldr  r12, =0xF00404B9
	swi 0

    ENTRY Sys_CaptureException    

    @ copy to EXCEPTION_CONTEXT struct
	mov    r12, #0
	add    r1, pc, #ExceptionRet-(.+8)
	stmfd  sp!, {r1}		@ return address	
	stmfd  sp!, {r4 - r12, sp, lr}

	@ call sys api
	mov  r0, sp	
	stmfd sp!, {lr}
    bl   _Sys_CaptureException
    ldmfd sp!, {lr}
   
    add  sp, sp, #48		@ popup EXCEPTION_CONTEXT struct 
    mov   pc, lr			@ common  return 

ExceptionRet:    
    mov   pc, lr			@ common  return 


