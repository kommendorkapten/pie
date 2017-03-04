/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
* This file is part of pie project
*
* The contents of this file are subject to the terms of the Common
* Development and Distribution License (the "License"). You may not use this
* file except in compliance with the License. You can obtain a copy of the
* License at http://opensource.org/licenses/CDDL-1.0. See the License for the
* specific language governing permissions and limitations under the License. 
* When distributing the software, include this License Header Notice in each
* file and include the License file at http://opensource.org/licenses/CDDL-1.0.
*/

#ifndef __AVX_CMP_H__
#define __AVX_CMP_H__

#ifndef _CMP_EQ_OQ 
# define _CMP_EQ_OQ    0x00
#endif
#ifndef _CMP_LT_OS
# define _CMP_LT_OS    0x01
#endif
#ifndef _CMP_LE_OS
# define _CMP_LE_OS    0x02
#endif
#ifndef _CMP_UNORD_Q
# define _CMP_UNORD_Q  0x03
#endif
#ifndef _CMP_NEQ_UQ
# define _CMP_NEQ_UQ   0x04
#endif
#ifndef _CMP_NLT_US
# define _CMP_NLT_US   0x05
#endif
#ifndef _CMP_NLE_US
# define _CMP_NLE_US   0x06
#endif
#ifndef _CMP_ORD_Q
# define _CMP_ORD_Q    0x07
#endif
#ifndef _CMP_EQ_UQ
# define _CMP_EQ_UQ    0x08
#endif
#ifndef _CMP_NGE_US
# define _CMP_NGE_US   0x09
#endif
#ifndef _CMP_NGT_US
# define _CMP_NGT_US   0x0a
#endif
#ifndef _CMP_FALSE_OQ
# define _CMP_FALSE_OQ 0x0b
#endif
#ifndef _CMP_NEQ_OQ
# define _CMP_NEQ_OQ   0x0c
#endif
#ifndef _CMP_GE_OS
# define _CMP_GE_OS    0x0d
#endif
#ifndef _CMP_GT_OS
# define _CMP_GT_OS    0x0e
#endif
#ifndef _CMP_TRUE_UQ
# define _CMP_TRUE_UQ  0x0f
#endif
#ifndef _CMP_EQ_OS
# define _CMP_EQ_OS    0x10
#endif
#ifndef _CMP_LT_OQ
# define _CMP_LT_OQ    0x11
#endif
#ifndef _CMP_LE_OQ
# define _CMP_LE_OQ    0x12
#endif
#ifndef _CMP_UNORD_S
# define _CMP_UNORD_S  0x13
#endif
#ifndef _CMP_NEQ_US
# define _CMP_NEQ_US   0x14
#endif
#ifndef _CMP_NLT_UQ
# define _CMP_NLT_UQ   0x15
#endif
#ifndef _CMP_NLE_UQ
# define _CMP_NLE_UQ   0x16
#endif
#ifndef _CMP_ORD_S
# define _CMP_ORD_S    0x17
#endif
#ifndef _CMP_EQ_US
# define _CMP_EQ_US    0x18
#endif
#ifndef _CMP_NGE_UQ
# define _CMP_NGE_UQ   0x19
#endif
#ifndef _CMP_NGT_UQ
# define _CMP_NGT_UQ   0x1a
#endif
#ifndef _CMP_FALSE_OS
# define _CMP_FALSE_OS 0x1b
#endif
#ifndef _CMP_NEQ_OS
# define _CMP_NEQ_OS   0x1c
#endif
#ifndef _CMP_GE_OQ
# define _CMP_GE_OQ    0x1d
#endif
#ifndef _CMP_GT_OQ
# define _CMP_GT_OQ    0x1e
#endif
#ifndef _CMP_TRUE_US
# define _CMP_TRUE_US  0x1f
#endif

#endif /* __AVX_CMP_H__ */
