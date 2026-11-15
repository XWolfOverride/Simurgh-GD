/* Simurgh: modificado 2026-11-15 - sin definicion duplicada de la variable debug (ahora extern en fxc.h) */
/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.84
 *  Last stable release   :
 *  Project documentation : http://fenix.divsite.net
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  Copyright © 1999 José Luis Cebrián Pagüe
 *  Copyright © 2002 Fenix Team
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#ifdef TARGET_BEOS
#include <dlfcn.h>
#endif

#include <fnx_loadlib.h>

#include "fxc.h"
#include "messages.h"


/* ---------------------------------------------------------------------- */
/* Módulo principal del compilador. Contiene código que inicializa los    */
/* identificadores conocidos, así como las funciones de compilado de      */
/* nivel más alto                                                         */
/* ---------------------------------------------------------------------- */

/* Tipos */
FXC_TLS int identifier_dword ;
FXC_TLS int identifier_word ;
FXC_TLS int identifier_byte ;
FXC_TLS int identifier_int ;
FXC_TLS int identifier_short ;
FXC_TLS int identifier_char ;
FXC_TLS int identifier_unsigned ;
FXC_TLS int identifier_signed ;
FXC_TLS int identifier_string ;
FXC_TLS int identifier_float ;
FXC_TLS int identifier_struct ;
FXC_TLS int identifier_type ;

FXC_TLS int identifier_declare ;

FXC_TLS int identifier_program ;
FXC_TLS int identifier_debug ;
FXC_TLS int identifier_const ;
FXC_TLS int identifier_begin ;
FXC_TLS int identifier_end ;
FXC_TLS int identifier_global ;
FXC_TLS int identifier_local ;
FXC_TLS int identifier_public ;
FXC_TLS int identifier_private ;
FXC_TLS int identifier_const ;
FXC_TLS int identifier_dup ;
FXC_TLS int identifier_from ;
FXC_TLS int identifier_step ;
FXC_TLS int identifier_to ;
FXC_TLS int identifier_if ;
FXC_TLS int identifier_else ;
FXC_TLS int identifier_elseif ;
FXC_TLS int identifier_for ;
FXC_TLS int identifier_while ;
FXC_TLS int identifier_frame ;
FXC_TLS int identifier_clone ;

FXC_TLS int identifier_onexit ;

FXC_TLS int identifier_switch ;
FXC_TLS int identifier_case ;
FXC_TLS int identifier_default ;
FXC_TLS int identifier_repeat ;
FXC_TLS int identifier_until ;
FXC_TLS int identifier_loop ;
FXC_TLS int identifier_break ;
FXC_TLS int identifier_continue ;
FXC_TLS int identifier_return ;
FXC_TLS int identifier_process ;
FXC_TLS int identifier_function ;
FXC_TLS int identifier_bandoffset ;
FXC_TLS int identifier_offset ;
FXC_TLS int identifier_sizeof ;
FXC_TLS int identifier_pointer ;

FXC_TLS int identifier_and ;
FXC_TLS int identifier_or  ;
FXC_TLS int identifier_xor ;
FXC_TLS int identifier_not ;

FXC_TLS int identifier_band ;
FXC_TLS int identifier_bor  ;
FXC_TLS int identifier_bxor ;
FXC_TLS int identifier_bnot ;

FXC_TLS int identifier_plus ;
FXC_TLS int identifier_minus ;
FXC_TLS int identifier_plusplus ;
FXC_TLS int identifier_minusminus ;
FXC_TLS int identifier_equal ;
FXC_TLS int identifier_multiply ;
FXC_TLS int identifier_mod ;
FXC_TLS int identifier_divide ;
FXC_TLS int identifier_semicolon ;
FXC_TLS int identifier_colon ;
FXC_TLS int identifier_comma ;
FXC_TLS int identifier_ror ;
FXC_TLS int identifier_rol ;
FXC_TLS int identifier_rightp ;
FXC_TLS int identifier_leftp ;
FXC_TLS int identifier_rightb ;
FXC_TLS int identifier_leftb ;
FXC_TLS int identifier_point ;
FXC_TLS int identifier_twopoints ;

FXC_TLS int identifier_eq ;
FXC_TLS int identifier_ne ;
FXC_TLS int identifier_gte ;
FXC_TLS int identifier_lte ;
FXC_TLS int identifier_lt ;
FXC_TLS int identifier_gt ;
FXC_TLS int identifier_question ;

FXC_TLS int identifier_plusequal ;
FXC_TLS int identifier_minusequal ;
FXC_TLS int identifier_multequal ;
FXC_TLS int identifier_divequal ;
FXC_TLS int identifier_modequal ;
FXC_TLS int identifier_orequal ;
FXC_TLS int identifier_xorequal ;
FXC_TLS int identifier_andequal ;
FXC_TLS int identifier_rorequal ;
FXC_TLS int identifier_rolequal ;
FXC_TLS int identifier_mouse ;
FXC_TLS int identifier_include ;

FXC_TLS int identifier_import ;

FXC_TLS int reserved_words ;

void compile_init ()
{
    /* Initialize reserved words */

    identifier_dword        = identifier_add("DWORD") ;
    identifier_word         = identifier_add("WORD") ;
    identifier_byte         = identifier_add("BYTE") ;
    identifier_int          = identifier_add("INT") ;
    identifier_short        = identifier_add("SHORT") ;
    identifier_char         = identifier_add("CHAR") ;
    identifier_unsigned     = identifier_add("UNSIGNED") ;
    identifier_signed       = identifier_add("SIGNED") ;
    identifier_float        = identifier_add("FLOAT") ;
    identifier_string       = identifier_add("STRING") ;
    identifier_struct       = identifier_add("STRUCT") ;

    identifier_declare      = identifier_add("DECLARE") ;

    identifier_include      = identifier_add("INCLUDE") ;
    identifier_program      = identifier_add("PROGRAM") ;
    identifier_debug        = identifier_add("DEBUG") ;
    identifier_const        = identifier_add("CONST") ;
    identifier_begin        = identifier_add("BEGIN") ;
    identifier_end          = identifier_add("END") ;
    identifier_process      = identifier_add("PROCESS") ;
    identifier_function     = identifier_add("FUNCTION") ;
    identifier_global       = identifier_add("GLOBAL") ;
    identifier_local        = identifier_add("LOCAL") ;
    identifier_public       = identifier_add("PUBLIC") ;
    identifier_private      = identifier_add("PRIVATE") ;
    identifier_dup          = identifier_add("DUP") ;
    identifier_from         = identifier_add("FROM") ;
    identifier_to           = identifier_add("TO") ;
    identifier_step         = identifier_add("STEP") ;
    identifier_for          = identifier_add("FOR") ;
    identifier_while        = identifier_add("WHILE") ;
    identifier_repeat       = identifier_add("REPEAT") ;
    identifier_until        = identifier_add("UNTIL") ;
    identifier_switch       = identifier_add("SWITCH") ;
    identifier_case         = identifier_add("CASE") ;
    identifier_default      = identifier_add("DEFAULT") ;
    identifier_loop         = identifier_add("LOOP") ;
    identifier_break        = identifier_add("BREAK") ;
    identifier_continue     = identifier_add("CONTINUE") ;
    identifier_return       = identifier_add("RETURN") ;
    identifier_if           = identifier_add("IF") ;
    identifier_else         = identifier_add("ELSE") ;
    identifier_elseif       = identifier_add("ELSEIF") ;
    identifier_frame        = identifier_add("FRAME") ;
    identifier_clone        = identifier_add("CLONE") ;

    identifier_onexit       = identifier_add("ONEXIT") ;

    identifier_and          = identifier_add("AND");
    identifier_or           = identifier_add("OR");
    identifier_xor          = identifier_add("XOR");
    identifier_not          = identifier_add("NOT");

    identifier_band         = identifier_add("BAND");
    identifier_bor          = identifier_add("BOR");
    identifier_bxor         = identifier_add("BXOR");
    identifier_bnot         = identifier_add("BNOT");

    identifier_sizeof       = identifier_add("SIZEOF");
    identifier_offset       = identifier_add("OFFSET");
    identifier_pointer      = identifier_add("POINTER");
    identifier_type         = identifier_add("TYPE");

    identifier_bandoffset   = identifier_add("&");

    identifier_add_as   ("!", identifier_not) ;
    identifier_add_as   ("&&", identifier_and) ;
    identifier_add_as   ("||", identifier_or) ;
    identifier_add_as   ("^^", identifier_xor) ;

    identifier_add_as   ("~", identifier_bnot) ;
//    identifier_add_as   ("&", identifier_band) ;
    identifier_add_as   ("|", identifier_bor) ;
    identifier_add_as   ("^", identifier_bxor) ;

    identifier_plus         = identifier_add("+") ;
    identifier_minus        = identifier_add("-") ;
    identifier_plusplus     = identifier_add("++") ;
    identifier_minusminus   = identifier_add("--") ;
    identifier_multiply     = identifier_add("*") ;
    identifier_mod          = identifier_add("%") ;
    identifier_divide       = identifier_add("/") ;
    identifier_equal        = identifier_add("=") ;
    identifier_semicolon    = identifier_add(";") ;
    identifier_comma        = identifier_add(",") ;
    identifier_ror          = identifier_add(">>") ;
    identifier_rol          = identifier_add("<<") ;
    identifier_rightp       = identifier_add(")") ;
    identifier_leftp        = identifier_add("(") ;
    identifier_rightb       = identifier_add("]") ;
    identifier_leftb        = identifier_add("[") ;
    identifier_point        = identifier_add(".") ;
    identifier_twopoints    = identifier_add("..") ;
    identifier_question     = identifier_add("?") ;

    identifier_add_as   ("MOD", identifier_mod) ;
//    identifier_add_as   (":", identifier_semicolon) ;           // <<<--- Conflicto con identifier_colon
    identifier_add_as   ("ELIF", identifier_elseif);
    identifier_add_as   ("ELSIF", identifier_elseif);

    identifier_colon        = identifier_add(":") ;             // <<<--- Conflicto con identifier_semicolon

    identifier_eq           = identifier_add("==") ;
    identifier_ne           = identifier_add("!=") ;
    identifier_gt           = identifier_add(">") ;
    identifier_lt           = identifier_add("<") ;
    identifier_gte          = identifier_add(">=") ;
    identifier_lte          = identifier_add("<=") ;

    identifier_add_as   ("<>", identifier_ne) ;
    identifier_add_as   ("=>", identifier_gte) ;
    identifier_add_as   ("=<", identifier_lte) ;

    identifier_plusequal    = identifier_add("+=") ;
    identifier_andequal     = identifier_add("&=") ;
    identifier_xorequal     = identifier_add("^=") ;
    identifier_orequal      = identifier_add("|=") ;
    identifier_divequal     = identifier_add("/=") ;
    identifier_modequal     = identifier_add("%=") ;
    identifier_multequal    = identifier_add("*=") ;
    identifier_minusequal   = identifier_add("-=") ;
    identifier_rorequal     = identifier_add(">>=") ;
    identifier_rolequal     = identifier_add("<<=") ;

    identifier_import     = identifier_add("IMPORT") ;

    reserved_words        = identifier_next_code() ;

    identifier_mouse      = identifier_add("MOUSE") ;

    varspace_init (&global) ;
    varspace_init (&local) ;
    globaldata = segment_new() ;
    localdata = segment_new() ;

}

extern void token_dump() ;

void compile_error (const char *fmt, ...)
{
    char text[4000] ;
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);

    fprintf (stdout, MSG_COMPILE_ERROR, files[current_file], line_count, text) ;
    token_dump () ;
    fxc_fatal (2) ;
}

FXC_TLS int imports[1024] ;
FXC_TLS int nimports = 0 ;

// Funcion nula
void * compile_export ( const char * name )
{
    return ( NULL ) ;
}

void compile_import ()
{
    const char  * filename ;
    void        * library = NULL;
    dlfunc      RegisterFunctions ;

#if defined( TARGET_Linux ) || defined ( TARGET_BEOS ) || defined ( TARGET_MAC ) || defined ( TARGET_BSD )
    char        soname[1024];
    char        * ptr;
#endif
    token_next() ;
    if (token.type != STRING) {
        compile_error (MSG_STRING_EXP) ;
    }

    /* Bibliotecas que Simurgh trae integradas: Import "image" sigue aceptandose (codigo antiguo) pero no hace nada */
    {
        const char * lib = string_get(token.code) ;
        static const char * const builtin[] = { "image", "image.dll", 0 } ;
        int b ;
        for (b = 0 ; builtin[b] ; b++)
        {
            const char * p = lib, * q = builtin[b] ;
            while (*p && TOLOWER(*p) == *q) p++, q++ ;
            if (*p == 0 && *q == 0) return ;
        }
    }

    imports[nimports++] = token.code ;

    filename = string_get(token.code) ;

#if defined( TARGET_Linux ) || defined ( TARGET_BEOS ) || defined ( TARGET_BSD )
    #define DLLEXT      ".so"
    #define SIZEDLLEXT  3

    snprintf (soname, 1024, "./%s" DLLEXT, filename);
#endif

#ifdef TARGET_MAC
    int indexMac;
    int contmac=0;
    char *nameMac=files[current_file];

    /* Get the real directory for Mac OS X */

    while (contmac<strlen(nameMac))
    {
        if (nameMac[contmac]=='/') {
            indexMac=contmac;
        }
        contmac++;
    }

    contmac=0;

    while (contmac<indexMac)
    {
        files[path_file][contmac]=nameMac[contmac];
        contmac++;
    }
    files[path_file][contmac]='\0';

    #define DLLEXT      ".dylib"
    #define SIZEDLLEXT  6

    snprintf (soname, 1024, "%s/%s" DLLEXT, files[path_file], filename);
#endif

#ifndef TARGET_Win32
    /* Clean the name (strip .DLL, and use lowercase) (Para operativos no WINDOWS) */

    for (ptr = soname ; *ptr ; ptr++)
        *ptr = TOLOWER(*ptr);

    if (strlen(soname) > (4+SIZEDLLEXT) && strcmp(ptr-(4+SIZEDLLEXT), ".dll" DLLEXT) == 0) {
        strcpy (ptr-(4+SIZEDLLEXT), DLLEXT);
    }

    filename = soname;
#endif

    library  = dlopen (filename, RTLD_NOW | RTLD_GLOBAL) ;
    if (!library) compile_error (dlerror()) ;

    RegisterFunctions = dlsym (library, "RegisterFunctions") ;
    if (!RegisterFunctions) compile_error(MSG_NO_COMPATIBLE_DLL) ;

    (*RegisterFunctions)(compile_export, sysproc_add) ;
}

void compile_constants ()
{
    int code;
    expresion_result res = {0} ;

    for (;;)
    {
        token_next() ;
        if (token.type == NOTOKEN) {
            break ;
        }
        if (token.type != IDENTIFIER) {
            compile_error (MSG_CONSTANT_NAME_EXP) ;
        }
        if (token.code == identifier_semicolon) {
            continue ;
        }
        if (token.code == identifier_begin ||
            token.code == identifier_local ||
            token.code == identifier_public ||
            token.code == identifier_private ||
            token.code == identifier_global)
        {
            token_back() ;
            return ;
        }

        if (token.code == identifier_end) {
            return ;
        }

        if (token.code < reserved_words) {
            compile_error (MSG_INVALID_IDENTIFIER);
        }

        code = token.code ;

        token_next() ;
        if (token.type != IDENTIFIER || token.code != identifier_equal) {
            compile_error (MSG_EXPECTED, "=") ;
        }

        res = compile_expresion(1, 0, 0) ;
        if (typedef_base(res.type) != TYPE_STRING &&
            typedef_base(res.type) != TYPE_FLOAT &&
            typedef_base(res.type) != TYPE_INT &&
            typedef_base(res.type) != TYPE_DWORD)
        {
            compile_error (MSG_INVALID_TYPE);
        }

        constants_add (code, res.type, typedef_base(res.type) == TYPE_FLOAT ? *(int *)&res.fvalue : res.value) ;

        token_next() ;
        if (token.type == IDENTIFIER && token.code == identifier_semicolon) {
            continue ;
        }
        compile_error (MSG_EXPECTED, ";") ;
    }
}

void compile_process ()
{
    PROCDEF * proc ;
    VARIABLE  * var ;
    int is_function = 0 ;
    int type_implicit = 1;
    int code, tcode, params ;
    BASETYPE type, typeb ;
    TYPEDEF ctype, ctypeb ;

    int signed_prefix = 0;
    int unsigned_prefix = 0;

    tcode = token.code ;

    if (tcode == identifier_function) { /* Es funcion? */
        is_function = 1;
    }

    if ((tcode == identifier_process || tcode == identifier_function)) { /* Si proceso o funcion, obtengo el signo */
        token_next() ;
        tcode = token.code;
    }

    if (token.code == identifier_signed) { /* signed */
        signed_prefix = 1;
        token_next();
        tcode = token.code;
    } else if (token.code == identifier_unsigned) { /* unsigned */
        unsigned_prefix = 1;
        token_next();
        tcode = token.code;
    }

    if (segment_by_name(token.code)) { /* Nombre del Segmento al que pertenece */
        tcode = identifier_pointer ;
        token_next() ;
    }

    if (identifier_is_basic_type(token.code)) { /* Salto identificador de tipo basico */
        tcode = token.code;
        token_next();
    }

    while (token.code == identifier_pointer || token.code == identifier_multiply) { /* Salto el indentificador POINTER */
        tcode = token.code ;
        token_next() ;
    }

    // Check if the process name is valid

    if (token.type != IDENTIFIER || token.code < reserved_words) {
        compile_error (MSG_PROCNAME_EXP) ;
    }
    code = token.code ;

    // Create the process if it is not defined already
    proc = procdef_search (code) ;
    if (!proc) {
        proc = procdef_new (procdef_getid(), code) ;
    } else if (proc->defined) {
        compile_error (MSG_PROC_ALREADY_DEFINED);
    }

    // Parse the process parameters

    token_next () ;
    if (token.type != IDENTIFIER || token.code != identifier_leftp) {
        compile_error (MSG_EXPECTED, "(") ;
    }

    proc->defined = 1 ;

    if (is_function) {

        proc->flags |= PROC_FUNCTION;
    }

    if (tcode == identifier_float){
        proc->type = TYPE_FLOAT ;
    }

    if (tcode == identifier_string){
        proc->type = TYPE_STRING ;
    }

    if (tcode == identifier_word){
        proc->type = signed_prefix ? TYPE_SHORT : TYPE_WORD;
        signed_prefix = unsigned_prefix = 0;
    }

    if (tcode == identifier_dword){
        proc->type = signed_prefix ? TYPE_INT : TYPE_DWORD;
        signed_prefix = unsigned_prefix = 0;
    }

    if (tcode == identifier_byte){
        proc->type = signed_prefix ? TYPE_SBYTE : TYPE_BYTE;
        signed_prefix = unsigned_prefix = 0;
    }

    if (tcode == identifier_int){
        proc->type = unsigned_prefix ? TYPE_DWORD : TYPE_INT;
        signed_prefix = unsigned_prefix = 0;
    }

    if (tcode == identifier_short){
        proc->type = unsigned_prefix ? TYPE_WORD : TYPE_SHORT;
        signed_prefix = unsigned_prefix = 0;
    }

    if (tcode == identifier_char){
        proc->type = TYPE_CHAR;
    }

    if (tcode == identifier_pointer || tcode == identifier_multiply){
        proc->type = TYPE_POINTER ;
    }

    if (signed_prefix || unsigned_prefix){
        compile_error(MSG_INVALID_TYPE);
    }

    token_next() ;

    /* Recoge los parámetros y su tipo */

    params = 0 ;
    type = TYPE_INT;
    typeb = TYPE_INT;
    type_implicit = 1;
    ctype = typedef_new(type) ;
    ctypeb = ctype ;
    signed_prefix = unsigned_prefix = 0;
    while (token.type != IDENTIFIER || token.code != identifier_rightp)
    {
        type = typeb;
        ctype = ctypeb;

        /* Recogo signo del parametro */
        if (token.type == IDENTIFIER && token.code == identifier_unsigned){
            unsigned_prefix = 1;
            token_next();
        }else if (token.type == IDENTIFIER && token.code == identifier_signed){
            signed_prefix = 1;
            token_next();
        }

        /* Recogo tipo del parametro */
        if (token.type == IDENTIFIER && token.code == identifier_dword) {
            type_implicit = 0;
            type = signed_prefix ? TYPE_INT : TYPE_DWORD;
            unsigned_prefix = signed_prefix = 0;
            ctype = typedef_new(type) ;
            token_next() ;
        } else if (token.type == IDENTIFIER && token.code == identifier_int) {
            type_implicit = 0;
            type = unsigned_prefix ? TYPE_DWORD : TYPE_INT;
            unsigned_prefix = signed_prefix = 0;
            ctype = typedef_new(type) ;
            token_next() ;
        } else if (token.type == IDENTIFIER && token.code == identifier_word) {
            type_implicit = 0;
            type = signed_prefix ? TYPE_SHORT : TYPE_WORD ;
            unsigned_prefix = signed_prefix = 0;
            ctype = typedef_new(type) ;
            token_next() ;
        } else if (token.type == IDENTIFIER && token.code == identifier_short) {
            type_implicit = 0;
            type = unsigned_prefix ? TYPE_WORD : TYPE_SHORT;
            unsigned_prefix = signed_prefix = 0;
            ctype = typedef_new(type) ;
            token_next() ;
        } else if (token.type == IDENTIFIER && token.code == identifier_byte) {
            type_implicit = 0;
            type = signed_prefix ? TYPE_SBYTE : TYPE_BYTE ;
            unsigned_prefix = signed_prefix = 0;
            ctype = typedef_new(type) ;
            token_next() ;
        } else if (token.type == IDENTIFIER && token.code == identifier_char) {
            type_implicit = 0;
            type = TYPE_CHAR ;
            ctype = typedef_new(type) ;
            token_next() ;
        } else if (token.type == IDENTIFIER && token.code == identifier_string) {
            type_implicit = 0;
            type = TYPE_STRING ;
            ctype = typedef_new(type) ;
            token_next() ;
        } else if (token.type == IDENTIFIER && token.code == identifier_float) {
            type_implicit = 0;
            type = TYPE_FLOAT ;
            ctype = typedef_new(type) ;
            token_next() ;
        } else if (token.type == IDENTIFIER && segment_by_name(token.code)) {
            type_implicit = 0;
            ctype = *typedef_by_name(token.code) ;
            type = TYPE_STRUCT ;
            token_next() ;
        }

        if (signed_prefix || unsigned_prefix)
        {
            compile_error(MSG_INVALID_TYPE);
            signed_prefix = unsigned_prefix = 0;
        }


        typeb = type;
        ctypeb = ctype;

        while (token.type == IDENTIFIER && (token.code == identifier_pointer || token.code == identifier_multiply))
        {
            type_implicit = 0;
            type = TYPE_POINTER ;
            ctype = typedef_pointer(ctype) ;
            token_next() ;
        }

        if (type == TYPE_STRUCT)
        {
            type_implicit = 0;
            type = TYPE_POINTER ;
            ctype = typedef_pointer(ctype) ;
        }

        if (token.type != IDENTIFIER || token.code < reserved_words)
            compile_error (MSG_INVALID_PARAM) ;

        /* Check if the process was used before declared */
        if (!proc->declared) {
            var = varspace_search (&local, token.code) ;
            if (var) {
                /* El parámetro es en realidad un local */
                if (type_implicit) {
                    type = typedef_base(var->type);
                    ctype = var->type;
                }
                if (typedef_base(var->type) != type) {
                    if (typedef_is_integer(var->type) && typedef_is_integer(typedef_new(type)))
                    {
                        /*
                            A parameter was used before the process is declared. The
                            data type declared is different from the data type used,
                            but both are integers. The error is ignored, but no
                            conversion is done. This can lead to type conversion issues.
                        */
                    } else
                        compile_error (MSG_INVALID_PARAMT) ;
                }
                codeblock_add (&proc->code, MN_LOCAL, var->offset) ;
                codeblock_add (&proc->code, MN_PRIVATE, proc->pridata->current) ;
                codeblock_add (&proc->code, MN_PTR, 0) ;

                if (typedef_base(var->type) == TYPE_STRING)
                    codeblock_add (&proc->code, MN_LET | MN_STRING, 0) ;
                else
                    codeblock_add (&proc->code, MN_LET, 0) ;

                codeblock_add (&proc->code, MN_POP, 0) ;

                if (proc->privars->reserved == proc->privars->count)
                    varspace_alloc (proc->privars, 16) ;

                proc->privars->vars[proc->privars->count].type   = typedef_new (TYPE_DWORD);
                proc->privars->vars[proc->privars->count].offset = proc->pridata->current ;
                proc->privars->vars[proc->privars->count].code   = -1 ;

                proc->privars->count++ ;

                segment_add_dword (proc->pridata, 0) ;
            } else {
                var = varspace_search (&global, token.code);
                if (var)
                {
                    /* El parámetro es en realidad un global */
                    if (type_implicit)
                    {
                        type = typedef_base(var->type);
                        ctype = var->type;
                    }
                    if (typedef_base(var->type) != type)
                        compile_error (MSG_INVALID_PARAMT) ;

                    codeblock_add (&proc->code, MN_GLOBAL, var->offset) ;
                    codeblock_add (&proc->code, MN_PRIVATE, proc->pridata->current) ;
                    codeblock_add (&proc->code, MN_PTR, 0) ;

                    if (typedef_base(var->type) == TYPE_STRING)
                        codeblock_add (&proc->code, MN_LET | MN_STRING, 0) ;
                    else
                        codeblock_add (&proc->code, MN_LET, 0) ;

                    codeblock_add (&proc->code, MN_POP, 0) ;

                    if (proc->privars->reserved == proc->privars->count)
                        varspace_alloc (proc->privars, 16) ;

                    proc->privars->vars[proc->privars->count].type   = typedef_new (TYPE_DWORD);
                    proc->privars->vars[proc->privars->count].offset = proc->pridata->current ;
                    proc->privars->vars[proc->privars->count].code   = -1 ;

                    proc->privars->count++ ;

                    segment_add_dword (proc->pridata, 0) ;
                }
                else
                {
                    /* Crear la variable privada */
                    if (proc->privars->reserved == proc->privars->count)
                        varspace_alloc (proc->privars, 16) ;
                    if (type == TYPE_STRING)
                        varspace_varstring (proc->privars, proc->pridata->current) ;
                    proc->privars->vars[proc->privars->count].type   = ctype;
                    proc->privars->vars[proc->privars->count].offset = proc->pridata->current ;
                    proc->privars->vars[proc->privars->count].code   = token.code ;
                    proc->privars->count++ ;
                    segment_add_dword (proc->pridata, 0) ;
                }
            }
        }

        if (proc->declared && proc->paramtype[params] != type)
            compile_error (MSG_PROTO_ERROR) ;

        if (proc->params != -1)
        {
            /* El proceso fue usado previamente */

            if (proc->paramtype[params] == TYPE_UNDEFINED) {
                proc->paramtype[params] = type ;
            } else if ((proc->paramtype[params] == TYPE_DWORD || proc->paramtype[params] == TYPE_INT) &&
                         (type == TYPE_DWORD ||
                          type == TYPE_INT   ||
                          type == TYPE_BYTE  ||
                          type == TYPE_WORD  ||
                          type == TYPE_SHORT ||
                          type == TYPE_SBYTE
                         )) {
                proc->paramtype[params] = type ;
            } else if (type == TYPE_DWORD && (proc->paramtype[params] == TYPE_BYTE || proc->paramtype[params] == TYPE_WORD)) {
                proc->paramtype[params] = type ;
            } else if (proc->paramtype[params] != type) {
                compile_error (MSG_INVALID_PARAMT) ;
            }
        } else {
            proc->paramtype[params] = type;
        }
        params++ ;

        if (params == MAX_PARAMS)
            compile_error (MSG_TOO_MANY_PARAMS) ;

        token_next() ;

        /* Valor por defecto: function f(a, b=0, c="texto"); puede ir en cualquier parametro */
        if (token.type == IDENTIFIER && token.code == identifier_equal)
        {
            expresion_result def = compile_constant_expr () ;
            BASETYPE dt = typedef_base (def.type) ;
            int pi = params - 1 ;

            if (type == TYPE_STRING)
            {
                if (dt != TYPE_STRING) compile_error (MSG_BAD_DEFAULT, params) ;
                proc->paramdef_kind[pi]  = 2 ;
                proc->paramdef_value[pi] = def.value ;
            }
            else if (type == TYPE_FLOAT)
            {
                float fv ;
                if (dt == TYPE_FLOAT) fv = def.fvalue ;
                else if (dt == TYPE_STRING) { compile_error (MSG_BAD_DEFAULT, params) ; fv = 0 ; }
                else fv = (float) def.value ;
                proc->paramdef_kind[pi]  = 3 ;
                proc->paramdef_value[pi] = *(int *)&fv ;
            }
            else
            {
                if (dt == TYPE_STRING || dt == TYPE_FLOAT) compile_error (MSG_BAD_DEFAULT, params) ;
                proc->paramdef_kind[pi]  = 1 ;
                proc->paramdef_value[pi] = def.value ;
            }
            token_next() ;
        }

        if (token.type == IDENTIFIER && token.code == identifier_comma)
            token_next() ;
    } /* END while (token.type != IDENTIFIER || token.code != identifier_rightp) */

    if (proc->params == -1 || !proc->sig_known) {
        /* Sin firma definitiva (nunca visto, o solo llamado antes): la declaracion fija el numero de parametros */
        proc->params = params ;
    } else if (proc->params != params) {
        compile_error (MSG_INCORRECT_PARAMC, identifier_name(code), proc->params) ;
    }
    proc->sig_known = 1 ;
    procdef_check_calls (proc) ;
    token_next () ;

    if (token.type == IDENTIFIER && token.code == identifier_semicolon)
        token_next () ;

    /* Se admite una sección interna de datos locales; sin embargo,
       los datos declarados aquí tienen el mismo efecto que si son
       declarados externamente y afectarán a todos los procesos (Ya no va mas esto, ahora los datos locales son locales, Splinter) */

    if (!proc->declared) {
        while ( token.type == IDENTIFIER && ( token.code == identifier_local  ||
                                              token.code == identifier_public ||
                                              token.code == identifier_private ) ) {
            if ( token.code == identifier_local || token.code == identifier_public )
            {
                /* (2006/11/19 19:34 GMT-03:00, Splinter - jj_arg@yahoo.com) */
                /* Ahora las declaraciones locales, son solo locales al proceso, pero visibles desde todo proceso */
                /* Se permite declarar local/publica una variable que haya sido declarada global, es una variable propia, no es la global */
                VARSPACE * v[] = { &local, proc->privars, NULL };
                compile_varspace ( proc->pubvars, proc->pubdata, 1, 1, 0, v, DEFAULT_ALIGNMENT ) ;
            } else if ( token.code == identifier_private ) {
                /* (2006/11/19 19:34 GMT-03:00, Splinter - jj_arg@yahoo.com) */
                /* Se permite declarar privada una variable que haya sido declarada global, es una variable propia, no es la global */
                VARSPACE * v[] = { &local, proc->pubvars, NULL };
                compile_varspace ( proc->privars, proc->pridata, 1, 1, 0, v, DEFAULT_ALIGNMENT ) ;
            }

            token_next () ;
        }
    }

    /* Gestiona procesos cuyos parámetros son variables locales */

    {
        if (token.type != IDENTIFIER || token.code != identifier_begin) {
            compile_error (MSG_NO_BEGIN) ;
        }

        compile_block (proc) ;

        if (token.type == IDENTIFIER && token.code == identifier_else) {
            compile_error (MSG_ELSE_WOUT_IF) ;
        }
    }

    if (token.type != IDENTIFIER || token.code != identifier_end) {
        compile_error (MSG_NO_END) ;
    }

    {
        codeblock_add (&proc->code, MN_END, 0) ;

        if (debug)
        {
            printf ("\n\n---------- Process %d (%s)\n\n", proc->typeid, identifier_name(code)) ;

            if ( proc->privars->count ) {
                printf ("---- Private variables\n") ;
                varspace_dump (proc->privars, 0) ;
                printf ("\n") ;
            }

            if ( proc->pubvars->count ) {
                printf ("---- Public variables\n") ;
                varspace_dump (proc->pubvars, 0) ;
                printf ("\n") ;
            }

            //segment_dump  (proc->pridata) ;
            codeblock_dump (&proc->code) ;
        }
    }

    proc->declared = 1 ;

}

void compile_program ()
{
    int code ;

    /* Ahora lo del program es opcional :-P */

    token_next() ;
    if (token.type == IDENTIFIER && token.code == identifier_program)
    {
        token_next() ;
        if (token.type != IDENTIFIER || token.code < reserved_words) {
            compile_error (MSG_PROGRAM_NAME_EXP) ;
        }
        token_next() ;
        if (token.type != IDENTIFIER || token.code != identifier_semicolon) {
            compile_error (MSG_EXPECTED, ";") ;
        }
    }
    else {
        token_back() ;
    }

    mainproc = procdef_new(procdef_getid(), identifier_search_or_add("MAIN")) ;

    for (;;)
    {
        token_next () ;
        while (token.type == IDENTIFIER && token.code == identifier_semicolon) {
            token_next() ;
        }

        if (token.type == IDENTIFIER && token.code == identifier_import) {
            compile_import() ;
        } else if (token.type == IDENTIFIER && token.code == identifier_const) {
            compile_constants() ;
        } else if (token.type == IDENTIFIER && token.code == identifier_local) {
            /* (2006/11/19 19:34 GMT-03:00, Splinter - jj_arg@yahoo.com) */
            VARSPACE * v[] = { &global, NULL };
            compile_varspace ( &local, localdata, 1, 1, 0, v, DEFAULT_ALIGNMENT ) ;
        } else if (token.type == IDENTIFIER && token.code == identifier_global) {
            /* (2006/11/19 19:34 GMT-03:00, Splinter - jj_arg@yahoo.com) */
            VARSPACE * v[] = { &local, NULL };
            compile_varspace ( &global, globaldata, 1, 1, 0, v, DEFAULT_ALIGNMENT ) ;
        } else if (token.type == IDENTIFIER && token.code == identifier_private) {
            /* (2006/11/19 19:34 GMT-03:00, Splinter - jj_arg@yahoo.com) */
            VARSPACE * v[] = { &local, &global, NULL };
            compile_varspace ( mainproc->privars, mainproc->pridata, 1, 1, 0, v, DEFAULT_ALIGNMENT ) ;
        } else if (token.type == IDENTIFIER && token.code == identifier_begin) {
            if (mainproc->defined) {
                /* Hack para poder redefinir el proceso principal */
                mainproc->code.current -= 1 ;
            }
            mainproc->defined = 1 ;
            compile_block (mainproc) ;
            if (token.type == IDENTIFIER && token.code == identifier_else) {
                compile_error (MSG_ELSE_WOUT_IF) ;
            }
            if (token.type != IDENTIFIER || token.code != identifier_end) {
                compile_error (MSG_NO_END) ;
            }
            codeblock_add (&mainproc->code, MN_END, 0) ;
        } else if (token.type == IDENTIFIER && token.code == identifier_type) { /* Tipo de dato definido por el usuario */
            segment  * s = segment_new() ;
            VARSPACE * v = varspace_new() ;
            TYPEDEF    t = typedef_new(TYPE_STRUCT);

            t.chunk[0].count = 1 ;

            token_next() ;
            if ((code = token.code) < reserved_words || token.type != IDENTIFIER) {
                compile_error (MSG_INVALID_TYPE) ;
            }

            t.varspace = v ;
            typedef_name (t, code) ;
            segment_name (s, code) ;

            /* (2006/11/19 19:34 GMT-03:00, Splinter - jj_arg@yahoo.com) */
            compile_varspace (v, s, 0, 1, 0, NULL, 0) ;
            if (token.code != identifier_end) {
                compile_error (MSG_NO_END) ;
            }
        } else if (token.type == IDENTIFIER && token.code == identifier_declare) {
            compile_error (MSG_DECLARE_REMOVED) ;
        } else if (token.type == IDENTIFIER &&
                                    (token.code == identifier_process  ||
                                     token.code == identifier_function ||
                                     identifier_is_basic_type(token.type))) { /* Definición de proceso */
            compile_process() ;

        } else if (segment_by_name(token.code)) {
            compile_process() ;

        } else {
            break ;
        }
    }

    if (debug)
    {
        printf ("\n----- Main procedure\n\n") ;
        varspace_dump (mainproc->privars, 0) ;
        //segment_dump  (mainproc->pridata) ;
        printf ("\n");
        codeblock_dump (&mainproc->code) ;
        printf ("\n") ;
    }

    if (global.count && debug)
    {
        printf ("\n---- Global variables\n\n") ;
        varspace_dump (&global, 0) ;
    }

    if (local.count && debug)
    {
        printf ("\n---- Local variables\n\n") ;
        varspace_dump (&local, 0) ;
        //segment_dump (localdata) ;
    }

    if (token.type != NOTOKEN) {
        compile_error (MSG_UNEXPECTED_TOKEN) ;
    }

    program_postprocess() ;

    if (!mainproc->defined) {
        compile_error (MSG_NO_MAIN) ;
    }

#ifdef DEBUG_VERBOSE
    identifier_dump() ;
    string_dump() ;
#endif
}

void cmain_reset (void)
{
	nimports = 0 ;
	memset (imports, 0, sizeof(imports)) ;
	reserved_words = 0 ;
}
