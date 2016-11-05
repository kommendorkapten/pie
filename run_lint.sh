#!/bin/sh

set -u

l_p="-Xc99 -m64 -u -m -errchk=%all -Ncheck=%all"
#l_p="${l_p} -errwarn=%all"
l_p="${l_p} -Nlevel=1 -errtags=yes"
l_p="${l_p} -erroff=E_FUNC_RET_ALWAYS_IGNOR,E_SIGN_EXTENSION_PSBL,E_CAST_INT_TO_SMALL_INT,E_FUNC_DECL_VAR_ARG,E_ASGN_RESET,E_ENUM_UNUSE,E_CONSTANT_CONDITION"


io=`find io -name '*.c'`
lint ${l_p} ${io}

msg=`find msg -name '*.c'`
lint ${l_p} ${msg}

lib=`find lib -name '*.c'`
lint ${l_p} ${lib}

wsrv=`find wsrv -name '*.c` 
lint -DEVENT_POLL=1 -I/usr/local/include ${l_p} -errhdr=no%/usr/local/include ${wsrv}

base="pie_bm.c pie_cspace.c pie_render.c"
lint ${l_p} -errhdr=no%/usr/include ${base}
