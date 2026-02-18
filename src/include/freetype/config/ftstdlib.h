/***************************************************************************/
/*                                                                         */
/*  ftstdlib.h                                                             */
/*                                                                         */
/*    ANSI-specific library and header configuration file (specification   */
/*    only).                                                               */
/*                                                                         */
/*  Copyright 2002, 2003, 2004 by                                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


/*************************************************************************/
/*                                                                       */
/* This file is used to group all #includes to the ANSI C library that   */
/* FreeType normally requires.  It also defines macros to rename the     */
/* standard functions within the FreeType source code.                   */
/*                                                                       */
/* Load a file which defines __FTSTDLIB_H__ before this one to override  */
/* it.                                                                   */
/*                                                                       */
/*************************************************************************/


#ifndef __FTSTDLIB_H__
#define __FTSTDLIB_H__


#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>

#define ft_ptrdiff_t ptrdiff_t


/**********************************************************************/
/*                                                                    */
/*                           integer limits                           */
/*                                                                    */
/* UINT_MAX and ULONG_MAX are used to automatically compute the size  */
/* of `int' and `long' in bytes at compile-time.  So far, this works  */
/* for all platforms the library has been tested on.                  */
/*                                                                    */
/* Note that on the extremely rare platforms that do not provide      */
/* integer types that are _exactly_ 16 and 32 bits wide (e.g. some    */
/* old Crays where `int' is 36 bits), we do not make any guarantee    */
/* about the correct behaviour of FT2 with all fonts.                 */
/*                                                                    */
/* In these case, "ftconfig.h" will refuse to compile anyway with a   */
/* message like "couldn't find 32-bit type" or something similar.     */
/*                                                                    */
/* IMPORTANT NOTE: We do not define aliases for heap management and   */
/*                 i/o routines (i.e. malloc/free/fopen/fread/...)    */
/*                 since these functions should all be encapsulated   */
/*                 by platform-specific implementations of            */
/*                 "ftsystem.c".                                      */
/*                                                                    */
/**********************************************************************/


#include <limits.h>

#define FT_UINT_MAX  UINT_MAX
#define FT_INT_MAX   INT_MAX
#define FT_ULONG_MAX ULONG_MAX


/**********************************************************************/
/*                                                                    */
/*                 character and string processing                    */
/*                                                                    */
/**********************************************************************/


#include <ctype.h>

#define ft_isalnum  isalnum
#define ft_isupper  isupper
#define ft_islower  islower
#define ft_isdigit  isdigit
#define ft_isxdigit isxdigit


#include <ph_string.h>

#define ft_memcmp  ph_memcmp
#define ft_memcpy  ph_memcpy
#define ft_memmove ph_memmove
#define ft_memset  ph_memset
#define ft_strcat  ph_strcat
#define ft_strcmp  ph_strcmp
#define ft_strcpy  ph_strcpy
#define ft_strlen  ph_strlen
#define ft_strncmp ph_strncmp
#define ft_strncpy ph_strncpy
#define ft_strrchr ph_strrchr


// #include <stdio.h>

#define ft_sprintf sprintf


/**********************************************************************/
/*                                                                    */
/*                             sorting                                */
/*                                                                    */
/**********************************************************************/


#include <ph_qsort.h>
#include <stdlib.h>

#define ft_qsort ph_qsort
#define ft_exit  exit /* only used to exit from unhandled exceptions */

#define ft_atol ph_atol


/**********************************************************************/
/*                                                                    */
/*                         execution control                          */
/*                                                                    */
/**********************************************************************/


#include <ph_setjmp.h>

#define ft_jmp_buf jmp_buf /* note: this cannot be a typedef since */
						   /*       jmp_buf is defined as a macro  */
						   /*       on certain platforms           */

#define ft_setjmp  ph_setjmp  /* same thing here */
#define ft_longjmp ph_longjmp /* "               */


/* the following is only used for debugging purposes, i.e. when */
/* FT_DEBUG_LEVEL_ERROR or FT_DEBUG_LEVEL_TRACE are defined     */
/*                                                              */
#include <stdarg.h>
#include <sys/types.h>


#endif /* __FTSTDLIB_H__ */


/* END */
