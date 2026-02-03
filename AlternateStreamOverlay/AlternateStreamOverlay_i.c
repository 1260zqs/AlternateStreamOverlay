

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 11:14:07 2038
 */
/* Compiler settings for AlternateStreamOverlay.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        EXTERN_C __declspec(selectany) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif // !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IAlternateStreamOverlayIcon,0xCFC72A5F,0xE58B,0x43BD,0x89,0x68,0xBA,0x89,0xAF,0xB2,0x24,0x0D);


MIDL_DEFINE_GUID(IID, IID_IAlternateStreamContext,0x96296677,0x6A18,0x4F22,0xAC,0x2E,0x84,0xA6,0x50,0xC4,0x08,0x1D);


MIDL_DEFINE_GUID(IID, LIBID_AlternateStreamOverlayLib,0xC0034A83,0x7FA3,0x43A7,0x82,0xDD,0x3A,0xDF,0x66,0xF0,0x08,0x6C);


MIDL_DEFINE_GUID(CLSID, CLSID_AlternateStreamOverlayIcon,0xFBAB8C58,0x59C9,0x4522,0xB7,0xE8,0x1A,0x6A,0xA2,0x3B,0x31,0x8C);


MIDL_DEFINE_GUID(CLSID, CLSID_AlternateStreamContext,0x4A26024B,0x5499,0x45BE,0xA2,0x3F,0x4A,0x65,0xE8,0x88,0x88,0xD0);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



