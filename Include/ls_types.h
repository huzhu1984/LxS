#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int LS_INT, LS_S32, LS_BOOL;
typedef unsigned int LS_UINT, LS_U32;
typedef short LS_SHORT, LS_S16;
typedef unsigned short LS_USHORT, LS_U16;
typedef char LS_CHAR, LS_S8;
typedef unsigned char LS_UCHAR, LS_U8;
typedef void LS_VOID;

#define LS_TRUE         (1)
#define LS_FALSE        (0)

#define LS_OK           (0)
#define LS_ERR          (-1)

#define LS_NEW(type, args...)           new_##type(args)
#define LS_CALL(obj, method, args...)   obj->method(obj, ##args)
#define LS_DELETE(obj)                  do{if(NULL!=obj)obj->del(obj);}while(0)

typedef struct PRIVATE_T PRIVATE_T;

#ifdef __cplusplus
}
#endif

#endif // TYPES_H
