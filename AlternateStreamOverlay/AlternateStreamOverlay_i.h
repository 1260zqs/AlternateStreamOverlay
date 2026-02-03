

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Fri Jun 21 11:35:59 2013
 */
/* Compiler settings for .\AlternateStreamOverlay.idl:
    Oicf, W1, Zp8, env=Win64 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __AlternateStreamOverlay_i_h__
#define __AlternateStreamOverlay_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IAlternateStreamOverlayIcon_FWD_DEFINED__
#define __IAlternateStreamOverlayIcon_FWD_DEFINED__
typedef interface IAlternateStreamOverlayIcon IAlternateStreamOverlayIcon;
#endif 	/* __IAlternateStreamOverlayIcon_FWD_DEFINED__ */


#ifndef __IAlternateStreamContext_FWD_DEFINED__
#define __IAlternateStreamContext_FWD_DEFINED__
typedef interface IAlternateStreamContext IAlternateStreamContext;
#endif 	/* __IAlternateStreamContext_FWD_DEFINED__ */


#ifndef __AlternateStreamOverlayIcon_FWD_DEFINED__
#define __AlternateStreamOverlayIcon_FWD_DEFINED__

#ifdef __cplusplus
typedef class AlternateStreamOverlayIcon AlternateStreamOverlayIcon;
#else
typedef struct AlternateStreamOverlayIcon AlternateStreamOverlayIcon;
#endif /* __cplusplus */

#endif 	/* __AlternateStreamOverlayIcon_FWD_DEFINED__ */


#ifndef __AlternateStreamContext_FWD_DEFINED__
#define __AlternateStreamContext_FWD_DEFINED__

#ifdef __cplusplus
typedef class AlternateStreamContext AlternateStreamContext;
#else
typedef struct AlternateStreamContext AlternateStreamContext;
#endif /* __cplusplus */

#endif 	/* __AlternateStreamContext_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IAlternateStreamOverlayIcon_INTERFACE_DEFINED__
#define __IAlternateStreamOverlayIcon_INTERFACE_DEFINED__

/* interface IAlternateStreamOverlayIcon */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAlternateStreamOverlayIcon;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CFC72A5F-E58B-43BD-8968-BA89AFB2240D")
    IAlternateStreamOverlayIcon : public IUnknown
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IAlternateStreamOverlayIconVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAlternateStreamOverlayIcon * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAlternateStreamOverlayIcon * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAlternateStreamOverlayIcon * This);
        
        END_INTERFACE
    } IAlternateStreamOverlayIconVtbl;

    interface IAlternateStreamOverlayIcon
    {
        CONST_VTBL struct IAlternateStreamOverlayIconVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAlternateStreamOverlayIcon_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAlternateStreamOverlayIcon_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAlternateStreamOverlayIcon_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAlternateStreamOverlayIcon_INTERFACE_DEFINED__ */


#ifndef __IAlternateStreamContext_INTERFACE_DEFINED__
#define __IAlternateStreamContext_INTERFACE_DEFINED__

/* interface IAlternateStreamContext */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAlternateStreamContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("96296677-6A18-4F22-AC2E-84A650C4081D")
    IAlternateStreamContext : public IUnknown
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IAlternateStreamContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAlternateStreamContext * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAlternateStreamContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAlternateStreamContext * This);
        
        END_INTERFACE
    } IAlternateStreamContextVtbl;

    interface IAlternateStreamContext
    {
        CONST_VTBL struct IAlternateStreamContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAlternateStreamContext_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAlternateStreamContext_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAlternateStreamContext_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAlternateStreamContext_INTERFACE_DEFINED__ */



#ifndef __AlternateStreamOverlayLib_LIBRARY_DEFINED__
#define __AlternateStreamOverlayLib_LIBRARY_DEFINED__

/* library AlternateStreamOverlayLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_AlternateStreamOverlayLib;

EXTERN_C const CLSID CLSID_AlternateStreamOverlayIcon;

#ifdef __cplusplus

class DECLSPEC_UUID("FBAB8C58-59C9-4522-B7E8-1A6AA23B318C")
AlternateStreamOverlayIcon;
#endif

EXTERN_C const CLSID CLSID_AlternateStreamContext;

#ifdef __cplusplus

class DECLSPEC_UUID("4A26024B-5499-45BE-A23F-4A65E88888D0")
AlternateStreamContext;
#endif
#endif /* __AlternateStreamOverlayLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


