/* Copyright © 1999 Intel Corp.  */

#ifndef __MACROS_H__
#define __MACROS_H__

#define READ_REGISTER_ULONG(reg) (*(volatile unsigned long * const)(reg))
#define WRITE_REGISTER_ULONG(reg, val) (*(volatile unsigned long * const)(reg)) = (val)
#define READ_REGISTER_USHORT(reg) (*(volatile unsigned short * const)(reg))
#define WRITE_REGISTER_USHORT(reg, val) (*(volatile unsigned short * const)(reg)) = (val)
#define READ_REGISTER_UCHAR(reg) (*(volatile unsigned char * const)(reg))
#define WRITE_REGISTER_UCHAR(reg, val) (*(volatile unsigned char * const)(reg)) = (val)

#define EXT(d, b) ((((unsigned int)(d))>>(b))&1)
#define EXTV(d, hb, lb)\
 (((((unsigned int)(d))>>(lb))&((((unsigned int)0xffffffff)<<(31-(hb)))>>((lb)+(31-(hb))))) & 0xffffffff)


//
// Macro to ensure bitfield compatibility across compilers
//
#define WRITE_BITFIELD(_type,_ptr,_field,_value)	\
		{											\
		register union {							\
		  _type s;							\
		  unsigned int d;					\
		} foo;										\
		foo.d = *((volatile unsigned *)(_ptr));		\
		foo.s._field = (_value);						\
		*(volatile unsigned *)(_ptr) = foo.d;		\
		}

#define IOW_REG_BITWRITE(_type,_ptr,_field,_value)	       		\
		{							\
		register union {					\
		  _type s;						\
		  unsigned int d;					\
		} foo;							\
		foo.d = 0;						\
		foo.s._field = _value;					\
		*(volatile unsigned *)(_ptr) = foo.d;			\
		}

#define IOW_REG_SET(_type,_ptr,_value)					\
		{							\
		*(volatile unsigned *)(_ptr) = _value;			\
		}
#define IOW_REG_OR(_type,_ptr,_value)					\
		{							\
		*(volatile unsigned *)(_ptr) |= _value;			\
		}
#define IOW_REG_AND(_type,_ptr,_value)					\
		{							\
		*(volatile unsigned *)(_ptr) &= _value;			\
		}
#define IOW_REG_FIELD(_type,_ptr,_field,_value)		       		\
		{							\
		register union {					\
		  _type s;						\
		  unsigned int d;					\
		} foo;							\
		foo.d = *(volatile unsigned *)(_ptr);			\
		foo.s._field = _value;					\
		*(volatile unsigned *)(_ptr) = foo.d;			\
		}
#define IOW_REG_BITSET(_type,_ptr,_field,_value)	       		\
		{							\
		register union {					\
		  _type s;						\
		  unsigned int d;					\
		} foo;							\
		foo.d = 0;						\
		foo.s._field = _value;					\
		*(volatile unsigned *)(_ptr) = foo.d;			\
		}
#define IOW_REG_GET(_type,_ptr,_field,_value)				\
		{							\
		register union {					\
		  _type s;						\
		  unsigned int d;					\
		} foo;							\
		foo.d = *(volatile unsigned *)(_ptr);			\
		_value = foo.s._field;					\
		}

/* Halfword I/O register access macros
 */
#define IOH_REG_SET(_type,_ptr,_value)					\
		{							\
		*(volatile unsigned short *)(_ptr) = _value;		\
		}
#define IOH_REG_OR(_type,_ptr,_value)					\
		{							\
		*(volatile unsigned short *)(_ptr) |= _value;		\
		}
#define IOH_REG_AND(_type,_ptr,_value)					\
		{							\
		*(volatile unsigned short *)(_ptr) &= _value;		\
		}
#define IOH_REG_FIELD(_type,_ptr,_field,_value)				\
		{							\
		register union {					\
		  _type s;						\
		  unsigned short d;					\
		} foo;							\
		foo.d = *(volatile unsigned short *)(_ptr);		\
		foo.s._field = _value;					\
		*(volatile unsigned short *)(_ptr) = foo.d;		\
		}
#define IOH_REG_BITSET(_type,_ptr,_field,_value)			\
		{							\
		register union {					\
		  _type s;						\
		  unsigned short d;					\
		} foo;							\
		foo.d = 0;						\
		foo.s._field = _value;					\
		*(volatile unsigned short *)(_ptr) = foo.d;		\
		}
#define IOH_REG_GET(_type,_ptr,_field,_value)				\
		{							\
		register union {					\
		  _type s;						\
		  unsigned short d;					\
		} foo;							\
		foo.d = *(volatile unsigned short *)(_ptr);		\
		_value = foo.s._field;					\
		}

/* Byte I/O register access macros
 */
#define IOB_REG_SET(_type,_ptr,_value)					\
		{							\
		*(volatile unsigned char *)(_ptr) = _value;		\
		}
#define IOB_REG_OR(_type,_ptr,_value)					\
		{							\
		*(volatile unsigned char *)(_ptr) |= _value;		\
		}
#define IOB_REG_AND(_type,_ptr,_value)					\
		{							\
		*(volatile unsigned char *)(_ptr) &= _value;		\
		}
#define IOB_REG_FIELD(_type,_ptr,_field,_value)				\
		{							\
		register union {					\
		  _type s;						\
		  unsigned char d;					\
		} foo;							\
		foo.d = *(volatile unsigned char *)(_ptr);		\
		foo.s._field = _value;					\
		*(volatile unsigned char *)(_ptr) = foo.d;		\
		}
#define IOB_REG_BITSET(_type,_ptr,_field,_value)			\
		{							\
		register union {					\
		  _type s;						\
		  unsigned char d;					\
		} foo;							\
		foo.d = 0;						\
		foo.s._field = _value;					\
		*(volatile unsigned char *)(_ptr) = foo.d;		\
		}
#define IOB_REG_GET(_type,_ptr,_field,_value)				\
		{							\
		register union {					\
		  _type s;						\
		  unsigned char d;					\
		} foo;							\
		foo.d = *(volatile unsigned char *)(_ptr);		\
		_value = foo.s._field;					\
		}
#endif //__MACROS_H__



