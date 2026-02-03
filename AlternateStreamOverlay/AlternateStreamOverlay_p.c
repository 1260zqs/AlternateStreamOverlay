

/* this ALWAYS GENERATED file contains the proxy stub code */


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

#if defined(_M_AMD64)


#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4152 )  /* function/data pointer conversion in expression */

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#include "ndr64types.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif /* __RPCPROXY_H_VERSION__ */


#include "AlternateStreamOverlay_i.h"

#define TYPE_FORMAT_STRING_SIZE   3                                 
#define PROC_FORMAT_STRING_SIZE   1                                 
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   0            

typedef struct _AlternateStreamOverlay_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } AlternateStreamOverlay_MIDL_TYPE_FORMAT_STRING;

typedef struct _AlternateStreamOverlay_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } AlternateStreamOverlay_MIDL_PROC_FORMAT_STRING;

typedef struct _AlternateStreamOverlay_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } AlternateStreamOverlay_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax_2_0 = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};

static const RPC_SYNTAX_IDENTIFIER  _NDR64_RpcTransferSyntax_1_0 = 
{{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}};

#if defined(_CONTROL_FLOW_GUARD_XFG)
#define XFG_TRAMPOLINES(ObjectType)\
NDR_SHAREABLE unsigned long ObjectType ## _UserSize_XFG(unsigned long * pFlags, unsigned long Offset, void * pObject)\
{\
return  ObjectType ## _UserSize(pFlags, Offset, (ObjectType *)pObject);\
}\
NDR_SHAREABLE unsigned char * ObjectType ## _UserMarshal_XFG(unsigned long * pFlags, unsigned char * pBuffer, void * pObject)\
{\
return ObjectType ## _UserMarshal(pFlags, pBuffer, (ObjectType *)pObject);\
}\
NDR_SHAREABLE unsigned char * ObjectType ## _UserUnmarshal_XFG(unsigned long * pFlags, unsigned char * pBuffer, void * pObject)\
{\
return ObjectType ## _UserUnmarshal(pFlags, pBuffer, (ObjectType *)pObject);\
}\
NDR_SHAREABLE void ObjectType ## _UserFree_XFG(unsigned long * pFlags, void * pObject)\
{\
ObjectType ## _UserFree(pFlags, (ObjectType *)pObject);\
}
#define XFG_TRAMPOLINES64(ObjectType)\
NDR_SHAREABLE unsigned long ObjectType ## _UserSize64_XFG(unsigned long * pFlags, unsigned long Offset, void * pObject)\
{\
return  ObjectType ## _UserSize64(pFlags, Offset, (ObjectType *)pObject);\
}\
NDR_SHAREABLE unsigned char * ObjectType ## _UserMarshal64_XFG(unsigned long * pFlags, unsigned char * pBuffer, void * pObject)\
{\
return ObjectType ## _UserMarshal64(pFlags, pBuffer, (ObjectType *)pObject);\
}\
NDR_SHAREABLE unsigned char * ObjectType ## _UserUnmarshal64_XFG(unsigned long * pFlags, unsigned char * pBuffer, void * pObject)\
{\
return ObjectType ## _UserUnmarshal64(pFlags, pBuffer, (ObjectType *)pObject);\
}\
NDR_SHAREABLE void ObjectType ## _UserFree64_XFG(unsigned long * pFlags, void * pObject)\
{\
ObjectType ## _UserFree64(pFlags, (ObjectType *)pObject);\
}
#define XFG_BIND_TRAMPOLINES(HandleType, ObjectType)\
static void* ObjectType ## _bind_XFG(HandleType pObject)\
{\
return ObjectType ## _bind((ObjectType) pObject);\
}\
static void ObjectType ## _unbind_XFG(HandleType pObject, handle_t ServerHandle)\
{\
ObjectType ## _unbind((ObjectType) pObject, ServerHandle);\
}
#define XFG_TRAMPOLINE_FPTR(Function) Function ## _XFG
#define XFG_TRAMPOLINE_FPTR_DEPENDENT_SYMBOL(Symbol) Symbol ## _XFG
#else
#define XFG_TRAMPOLINES(ObjectType)
#define XFG_TRAMPOLINES64(ObjectType)
#define XFG_BIND_TRAMPOLINES(HandleType, ObjectType)
#define XFG_TRAMPOLINE_FPTR(Function) Function
#define XFG_TRAMPOLINE_FPTR_DEPENDENT_SYMBOL(Symbol) Symbol
#endif



extern const AlternateStreamOverlay_MIDL_TYPE_FORMAT_STRING AlternateStreamOverlay__MIDL_TypeFormatString;
extern const AlternateStreamOverlay_MIDL_PROC_FORMAT_STRING AlternateStreamOverlay__MIDL_ProcFormatString;
extern const AlternateStreamOverlay_MIDL_EXPR_FORMAT_STRING AlternateStreamOverlay__MIDL_ExprFormatString;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO IAlternateStreamOverlayIcon_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IAlternateStreamOverlayIcon_ProxyInfo;

#ifdef __cplusplus
namespace {
#endif

extern const MIDL_STUB_DESC Object_StubDesc;
#ifdef __cplusplus
}
#endif


extern const MIDL_SERVER_INFO IAlternateStreamContext_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IAlternateStreamContext_ProxyInfo;



#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const AlternateStreamOverlay_MIDL_PROC_FORMAT_STRING AlternateStreamOverlay__MIDL_ProcFormatString =
    {
        0,
        {

			0x0
        }
    };

static const AlternateStreamOverlay_MIDL_TYPE_FORMAT_STRING AlternateStreamOverlay__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */

			0x0
        }
    };


/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IAlternateStreamOverlayIcon, ver. 0.0,
   GUID={0xCFC72A5F,0xE58B,0x43BD,{0x89,0x68,0xBA,0x89,0xAF,0xB2,0x24,0x0D}} */

#pragma code_seg(".orpc")
static const unsigned short IAlternateStreamOverlayIcon_FormatStringOffsetTable[] =
    {
    0
    };



/* Object interface: IAlternateStreamContext, ver. 0.0,
   GUID={0x96296677,0x6A18,0x4F22,{0xAC,0x2E,0x84,0xA6,0x50,0xC4,0x08,0x1D}} */

#pragma code_seg(".orpc")
static const unsigned short IAlternateStreamContext_FormatStringOffsetTable[] =
    {
    0
    };



#endif /* defined(_M_AMD64)*/



/* this ALWAYS GENERATED file contains the proxy stub code */


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

#if defined(_M_AMD64)




#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif


#include "ndr64types.h"
#include "pshpack8.h"
#ifdef __cplusplus
namespace {
#endif


typedef 
NDR64_FORMAT_UINT32
__midl_frag1_t;
extern const __midl_frag1_t __midl_frag1;

static const __midl_frag1_t __midl_frag1 =
(NDR64_UINT32) 0 /* 0x0 */;
#ifdef __cplusplus
}
#endif


#include "poppack.h"



/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IAlternateStreamOverlayIcon, ver. 0.0,
   GUID={0xCFC72A5F,0xE58B,0x43BD,{0x89,0x68,0xBA,0x89,0xAF,0xB2,0x24,0x0D}} */

#pragma code_seg(".orpc")
static const FormatInfoRef IAlternateStreamOverlayIcon_Ndr64ProcTable[] =
    {
    0
    };


static const MIDL_SYNTAX_INFO IAlternateStreamOverlayIcon_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    AlternateStreamOverlay__MIDL_ProcFormatString.Format,
    &IAlternateStreamOverlayIcon_FormatStringOffsetTable[-3],
    AlternateStreamOverlay__MIDL_TypeFormatString.Format,
    0,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &IAlternateStreamOverlayIcon_Ndr64ProcTable[-3],
    0,
    0,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO IAlternateStreamOverlayIcon_ProxyInfo =
    {
    &Object_StubDesc,
    AlternateStreamOverlay__MIDL_ProcFormatString.Format,
    &IAlternateStreamOverlayIcon_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)IAlternateStreamOverlayIcon_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO IAlternateStreamOverlayIcon_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    AlternateStreamOverlay__MIDL_ProcFormatString.Format,
    (unsigned short *) &IAlternateStreamOverlayIcon_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)IAlternateStreamOverlayIcon_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(3) _IAlternateStreamOverlayIconProxyVtbl = 
{
    0,
    &IID_IAlternateStreamOverlayIcon,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy
};

const CInterfaceStubVtbl _IAlternateStreamOverlayIconStubVtbl =
{
    &IID_IAlternateStreamOverlayIcon,
    &IAlternateStreamOverlayIcon_ServerInfo,
    3,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};


/* Object interface: IAlternateStreamContext, ver. 0.0,
   GUID={0x96296677,0x6A18,0x4F22,{0xAC,0x2E,0x84,0xA6,0x50,0xC4,0x08,0x1D}} */

#pragma code_seg(".orpc")
static const FormatInfoRef IAlternateStreamContext_Ndr64ProcTable[] =
    {
    0
    };


static const MIDL_SYNTAX_INFO IAlternateStreamContext_SyntaxInfo [  2 ] = 
    {
    {
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    AlternateStreamOverlay__MIDL_ProcFormatString.Format,
    &IAlternateStreamContext_FormatStringOffsetTable[-3],
    AlternateStreamOverlay__MIDL_TypeFormatString.Format,
    0,
    0,
    0
    }
    ,{
    {{0x71710533,0xbeba,0x4937,{0x83,0x19,0xb5,0xdb,0xef,0x9c,0xcc,0x36}},{1,0}},
    0,
    0 ,
    (unsigned short *) &IAlternateStreamContext_Ndr64ProcTable[-3],
    0,
    0,
    0,
    0
    }
    };

static const MIDL_STUBLESS_PROXY_INFO IAlternateStreamContext_ProxyInfo =
    {
    &Object_StubDesc,
    AlternateStreamOverlay__MIDL_ProcFormatString.Format,
    &IAlternateStreamContext_FormatStringOffsetTable[-3],
    (RPC_SYNTAX_IDENTIFIER*)&_RpcTransferSyntax_2_0,
    2,
    (MIDL_SYNTAX_INFO*)IAlternateStreamContext_SyntaxInfo
    
    };


static const MIDL_SERVER_INFO IAlternateStreamContext_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    AlternateStreamOverlay__MIDL_ProcFormatString.Format,
    (unsigned short *) &IAlternateStreamContext_FormatStringOffsetTable[-3],
    0,
    (RPC_SYNTAX_IDENTIFIER*)&_NDR64_RpcTransferSyntax_1_0,
    2,
    (MIDL_SYNTAX_INFO*)IAlternateStreamContext_SyntaxInfo
    };
CINTERFACE_PROXY_VTABLE(3) _IAlternateStreamContextProxyVtbl = 
{
    0,
    &IID_IAlternateStreamContext,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy
};

const CInterfaceStubVtbl _IAlternateStreamContextStubVtbl =
{
    &IID_IAlternateStreamContext,
    &IAlternateStreamContext_ServerInfo,
    3,
    0, /* pure interpreted */
    CStdStubBuffer_METHODS
};

#ifdef __cplusplus
namespace {
#endif
static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    AlternateStreamOverlay__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x60001, /* Ndr library version */
    0,
    0x8010274, /* MIDL Version 8.1.628 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    0x2000001, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };
#ifdef __cplusplus
}
#endif

const CInterfaceProxyVtbl * const _AlternateStreamOverlay_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IAlternateStreamOverlayIconProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IAlternateStreamContextProxyVtbl,
    0
};

const CInterfaceStubVtbl * const _AlternateStreamOverlay_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IAlternateStreamOverlayIconStubVtbl,
    ( CInterfaceStubVtbl *) &_IAlternateStreamContextStubVtbl,
    0
};

PCInterfaceName const _AlternateStreamOverlay_InterfaceNamesList[] = 
{
    "IAlternateStreamOverlayIcon",
    "IAlternateStreamContext",
    0
};


#define _AlternateStreamOverlay_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _AlternateStreamOverlay, pIID, n)

int __stdcall _AlternateStreamOverlay_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _AlternateStreamOverlay, 2, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _AlternateStreamOverlay, 2, *pIndex )
    
}

EXTERN_C const ExtendedProxyFileInfo AlternateStreamOverlay_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _AlternateStreamOverlay_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _AlternateStreamOverlay_StubVtblList,
    (const PCInterfaceName * ) & _AlternateStreamOverlay_InterfaceNamesList,
    0, /* no delegation */
    & _AlternateStreamOverlay_IID_Lookup, 
    2,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* defined(_M_AMD64)*/

