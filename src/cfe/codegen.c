#include "cmplrs/usys.h"
#include "cmplrs/ucode.h"
#include "common.h"

typedef struct UnkCodegen_Struct_s {
    char pad0[4];
    int unk4;
    char pad8[0x10];
    int unk18;
} UnkCodegen_Struct;

struct Uw_if {     /* output format for each type	*/
    int If_nbytes; /*  # of bytes to read (doesn't include opcode or val)*/
    int If_hasval; /*  if it has a value		*/
}; 

/* U-code output formats	*/
enum /*				*/
    Uw_otype {
        Uwof_init,   /* the init statement		*/
        Uwof_comm,   /* the comment statement	*/
        Uwof_lca,    /* the lca statement		*/
        Uwof_ldc,    /* the ldc statement		*/
        Uwof_normal, /* normal format of printf 	*/
                     /*	format and pointers	*/
        Uwof_undef   /* still working ones of this 	*/
                     /*	type			*/
    }; /*				*/
/*				*/
struct Uw_of {             /* output format for each type	*/
    enum Uw_otype Of_type; /*    the output format type to	*/
    /*		choose		*/
    int Of_nbytes;   /* number of bytes		*/
    int Of_hasval;   /* set if it has a constant	*/
    char *Of_format; /*    a "printf" style output	*/
    /*		format string	*/
}; /*				*/

typedef struct UnkStruct_RuntimeId_s {
    char pad[0x3C];
    int unk3C;
} UnkStruct_RuntimeId;

extern int Label_sufix;
extern struct Bcrec *Ucp;
extern char Uout_buff[];
extern struct Uw_of Uw_of[];
extern int addr_dtype;
extern FILE *Ucw_binout;

void UWRITE(void *ucode, int size) {
    if (debug_arr[0x55] >= 3) {
        fprintf(dbgout, "writing ucode size=%d\n", size);
    }
    if (fwrite(ucode, 1, size, Ucw_binout) != (unsigned int) size) {
        error(0x50087, LEVEL_FATAL, -1, infile ? infile : "");
    }
}

// This match is just *wrong*
void UW_CONST_str(char *str) {
    size_t len; /* compiler-managed */

    if (strlen(str) >= Filenamelen + 1) {
        len = Filenamelen;
    } else {
        len = strlen(str);
    }
    UWRITE(&len, 4U);
    len = (int) (len + 3) / 4;
    UWRITE(&len, 4U);
    if ((len & 1) == 1) {
        len++;
    }
    UWRITE(str, len * 4);
}

void UW_CONST_m(int arg0, void *arg1) {
    int sp24; /* compiler-managed */

    UWRITE(&arg0, 4U);
    sp24 = (int) (arg0 + 3) / 4;
    UWRITE(&sp24, 4U);
    if ((sp24 & 1) == 1) {
        sp24 += 1;
    }
    sp24 *= 4;
    UWRITE(arg1, sp24);
}

void UW_CONST_s(int arg0, int arg1) {
    char filename[Filenamelen];
    unsigned int sp24;

    sprintf(filename, "%d", arg1);
    sp24 = strlen(filename);
    UWRITE(&sp24, 4U);
    sp24 = (int) (sp24 + 3) / 4;
    UWRITE(&sp24, 4U);
    if ((sp24 & 1) == 1) {
        sp24 += 1;
    }
    UWRITE(filename, sp24 * 4);
}

void UW_CONST_f(char *arg0, double arg2, int arg4) {
    char buf[1024];
    char *ptr;

    if (arg0 == NULL) {
        sprintf(buf, "%.16e", arg2);
        if (strncmp(buf, "inf", strlen("inf")) == 0) {
            error(0x3010C, LEVEL_WARNING, arg4, "infinity");
        } else if (strncmp(buf, "-inf", strlen("-inf")) == 0) {
            error(0x3010C, LEVEL_WARNING, arg4, "-infinity");
        }
        UW_CONST_str(buf);
        return;
    }
    ptr = arg0;
    while (*ptr != 0) {
        ptr++;
    }

    if ((arg0 < ptr)
        && (((ptr[-1] == 'F')) || (ptr[-1] == 'f') || (ptr[-1] == 'L') || (ptr[-1] == 'l'))) {
        ptr[-1] = '\0';
    }
    UW_CONST_str(arg0);
}

void UW_CONST_l(int arg0) {
    char buf[1024];

    sprintf(buf, "L%d", arg0);
    UW_CONST_str(buf);
}

void UW_CONST_i(int arg0) {
    int pad;
    int sp20;

    sp20 = arg0;
    UWRITE(&sp20, 8U);
}

void UW_CONST_lli(int arg0, int arg1) {
    UWRITE(&arg0, sizeof(arg0));
}

int U_DT(UnkCodegen_Struct *arg0) {

    switch (arg0->unk4) {
        case 19:
        case 24:
            return 9;
        case 22:
            return addr_dtype;
        case 21:
            return 2;
        case 17:
        case 18:
            return 0xB;
        case 4:
            return 0xA;
        case 10:
            __assert("FALSE", "codegen.c", 0xC0);
            /* fallthrough */
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 20:
            if ((unsigned int) arg0->unk18 >= 0x21U) {
                return 5;
            }
            return 6;
        case 11:
            __assert("FALSE", "codegen.c", 0xD2);
            /* fallthrough */
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            if ((unsigned int) arg0->unk18 >= 0x21U) {
                return 7;
            }
            return 8;
        case 1:
        case 2:
        case 3:
            if (arg0->unk18 == 0x20) {
                return 0xD;
            }
            return 0xC;
        default:
            return __assert("FALSE", "codegen.c", 0xE9);
    }
}


int init_codegen(char *arg0, FILE *arg1, int arg2) {
    UnkStruct_RuntimeId *temp_v0;

    Label_sufix = 0xA;
    if (arg1 != 0) {
        Ucw_binout = arg1;
    } else {
        Ucw_binout = stdout;
    }
    Ucp = (struct Bcrec*)Uout_buff;
    if (bit_size[9] == 0x40) {
        addr_dtype = 15;
    } else {
        addr_dtype = 0;
    }
    Ucp->Opc = 7;
    if (options[OPTION_ENDIANNESS] != 0) {
        Ucp->I1 = 0;
    } else {
        Ucp->I1 = 1;
    }
    Ucp->Uopcde.uiequ1.Length = 7;
    Ucp->Uopcde.uiequ1.offset = 10;

    Ucp = (struct Bcrec*)((char*)Ucp + Uw_of[Ucp->Opc].Of_nbytes);
    if ((char*)Ucp - (char*)&Uout_buff > 0x800) {
        UWRITE(Uout_buff, (char*)Ucp - (char*)Uout_buff);
        Ucp = (struct Bcrec*)Uout_buff;
    }
    
    Ucp->Opc = 0x63;
    Ucp->I1 = 0;
    Ucp->Uopcde.uiequ1.Length = arg2;

    Ucp = (struct Bcrec*)((char*)Ucp + Uw_of[Ucp->Opc].Of_nbytes);
    if ((char*)Ucp - (char*)Uout_buff > 0x800) {
        UWRITE(Uout_buff, (char*)Ucp - (char*)Uout_buff);
        Ucp = (struct Bcrec*)Uout_buff;
    }
    
    Ucp->Opc = 0x13;
    Ucp->Dtype = 9;

    Ucp = (struct Bcrec*)((char*)Ucp + Uw_of[Ucp->Opc].Of_nbytes);
    if ((char*)Ucp - (char*)Uout_buff > 0x800) {
        UWRITE(Uout_buff, (char*)Ucp - (char*)Uout_buff);
        Ucp = (struct Bcrec*)Uout_buff;
    }
    
    UWRITE(Uout_buff, (char*)Ucp - (char*)Uout_buff);
    Ucp = (struct Bcrec*)Uout_buff;

    UW_CONST_m((size_t) strlen(arg0) >= 1024 + 1 ? 1024 : strlen(arg0), arg0);
    if (options[3] & 8) {
        fprintf(stderr, "%s: ", myname);
        fflush(stderr);
    }
    if (options[15] != 0) {
        temp_v0 = get_runtime_id("_RtlCheckStack", void_type, 0);
        if (temp_v0 != 0) {
            Ucp->Opc = 0x63;
            Ucp->I1 = 2;
            Ucp->Uopcde.uiequ1.Length = (int) temp_v0->unk3C;

            Ucp = (struct Bcrec*)((char*)Ucp + Uw_of[Ucp->Opc].Of_nbytes);
            if ((char*)Ucp - (char*)Uout_buff > 0x800) {
                UWRITE(Uout_buff, (char*)Ucp - (char*)Uout_buff);
                Ucp = (struct Bcrec*)Uout_buff;
            }
        }
    }
    return 1;
}

static void func_004628B0(void) {
    Ucp->Opc = Ustp;
    Ucp->I1 = 0;
    Ucp = (struct Bcrec*)((char*)Ucp + Uw_of[Ucp->Opc].Of_nbytes);
    if ((char*)Ucp - (char*)Uout_buff >= 0x801) {
        UWRITE(Uout_buff, (char*)Ucp - (char*)Uout_buff);
        Ucp = (struct Bcrec *) Uout_buff;
    }
    UWRITE(Uout_buff, (char*)Ucp - (char*)Uout_buff);
    Ucp = (struct Bcrec *) Uout_buff;
}


#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/uw_init.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_00463488.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_00463894.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_00464D18.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_00464F1C.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_0046508C.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/LDC_integer.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_00465654.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_00465D90.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_00466804.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_00468244.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_004688C8.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_0046A1D8.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/codegen.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_0046ACAC.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_0046ACB4.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_0046AEC0.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_0046AF34.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_0046B038.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_0046B094.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/func_0046B13C.s")

#pragma GLOBAL_ASM("asm/7.1/functions/cfe/codegen/Set_Small_Data_Upper_Limit.s")
