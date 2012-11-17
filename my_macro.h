#ifndef _MY_MACRO_H_
#define _MY_MACRO_H_


#define	_LoByte(a)	(unsigned char)(a)
#define	_HiByte(a)	*(((unsigned char *)(&a)) + 1)


#endif //_MY_MACRO_H_
