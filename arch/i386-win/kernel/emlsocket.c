/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

/*****************************************************
文件说明：调用 MS Socket 的接口（需包含 emlsocket.h ）
版本号：  1.0.0
开发时期：2004-12-16
作者：    李林
修改记录：
******************************************************/
#include <windows.h>
#include <winsock.h>

/******************************************************/

int	WINAPI	Win32_WSAStartup( WORD wVersionRequired, LPWSADATA lpWSAData )
{	
	return WSAStartup( wVersionRequired, lpWSAData );
}

int	WINAPI	Win32_WSACleanup(void)
{
	return WSACleanup();
}
void	WINAPI	Win32_WSASetLastError( int nErrSock )
{
	WSASetLastError( nErrSock );
}
int	WINAPI	Win32_WSAGetLastError(void)
{
	return WSAGetLastError();
}


SOCKET	WINAPI	Win32_socket( int af, int type, int protocol )
{
	return socket( af, type, protocol );
}

int	WINAPI	Win32_bind( SOCKET s, const SOCKADDR* myaddr, int addr_len )
{
	return bind( s, myaddr, addr_len );
}
int	WINAPI	Win32_connect( SOCKET s, const SOCKADDR* addr, int addr_len )
{
	return connect( s, addr, addr_len );
}
int	WINAPI	Win32_listen( SOCKET s, int queue_len )
{
	return listen( s, queue_len );
}
SOCKET	WINAPI	Win32_accept( SOCKET s, OUT SOCKADDR* addr, OUT int* paddr_len )
{
	return accept( s, addr, paddr_len );
}
int	WINAPI	Win32_ioctlsocket( SOCKET s, long cmd, IN OUT DWORD* argp )
{
	return ioctlsocket( s, cmd, argp );
}
int	WINAPI	Win32_send( SOCKET s, const char* buf, int len, int flags )
{
	return send( s, buf, len, flags );
}
int	WINAPI	Win32_sendto( SOCKET s, const char* buf, int len, int flags, const SOCKADDR* to, int tolen )
{
	return sendto( s, buf, len, flags, to, tolen );
}
int	WINAPI	Win32_recv( SOCKET s, OUT char* buf, int len, int flags )
{
	return recv( s, buf, len, flags );
}
int	WINAPI	Win32_recvfrom( SOCKET s, OUT char* buf, int len, int flags, OUT SOCKADDR* from, IN OUT int* fromlen )
{
	return recvfrom( s, buf, len, flags, from, fromlen );
}
int	WINAPI	Win32_select( int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const struct timeval* timeout )
{
	return select( nfds,  readfds, writefds, exceptfds, timeout );
}


int	WINAPI	Win32_getsockopt( SOCKET s, int level, int optname, OUT char* optval, IN OUT int* optlen )
{
	return getsockopt( s, level, optname,  optval, optlen );
}
int	WINAPI	Win32_setsockopt( SOCKET s, int level, int optname, const char* optval, int optlen )
{
	return setsockopt( s, level, optname, optval, optlen );
}
int	WINAPI	Win32_getpeername( SOCKET s, OUT SOCKADDR* name, IN OUT int* namelen )
{
	return getpeername(  s, name, namelen );
}
int	WINAPI	Win32_getsockname( SOCKET s, OUT SOCKADDR* name, IN OUT int* namelen )
{
	return getsockname( s, name, namelen );
}

int	WINAPI	Win32_shutdown( SOCKET s, int how )
{
	return shutdown( s, how );
}
int	WINAPI	Win32_closesocket( SOCKET s )
{
	return closesocket( s );
}


int	WINAPI	Win32_gethostname( OUT char* name, int namelen )
{
	return gethostname( name, namelen );
}
HOSTENT*	WINAPI	Win32_gethostbyname( const char* name )
{
	return gethostbyname( name );
}
HOSTENT*	WINAPI	Win32_gethostbyaddr( const char* addr, int len, int type )
{
	return gethostbyaddr( addr, len, type );
	
}

HOSTENT*	WINAPI	Win32_gethostbynameex( const char* name, OUT LPBYTE pBufInfo, int len )
{	
	return gethostbyname( name );	
}

HOSTENT*	WINAPI	Win32_gethostbyaddrex( const char* addr, int len, int type, OUT LPBYTE pBufInfo, int lenInfo )
{
	return gethostbyaddr( addr, len, type );
}


// IP层提供的 函数

