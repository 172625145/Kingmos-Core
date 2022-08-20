#include <eversion.h>
//
//  该文件是为 gcc 连接准备的， 如果没有一个C 文件，gcc不会去做连接
//

extern void _startup( void );

void kernel_ctr0( void )
{
	_startup();  //真正的入口 at: boot.s
}
