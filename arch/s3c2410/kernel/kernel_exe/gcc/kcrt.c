#include <eversion.h>
//
//  ���ļ���Ϊ gcc ����׼���ģ� ���û��һ��C �ļ���gcc����ȥ������
//

extern void _startup( void );

void kernel_ctr0( void )
{
	_startup();  //��������� at: boot.s
}
