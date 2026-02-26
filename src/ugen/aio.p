#include "common.h"
#include "report.h"
#include "cmplrs/usys.h"
#include "cmplrs/ucode.h"
#include "cmplrs/binasm.h"
#include "cmplrs/uoptions.h"
#include "cmplrs/stinfc.h"

type
    hex_str = packed array [1..10] of char; /* :=  "0123456789ABCDEF" */

var
    source_language: integer;
    hex_tab : array [0..15] of char;
    in_file: File of binasm;
    current_filen: integer;
    current_linen: integer;
    source_file: Text;
    source_file_name: Filename;

#define ST_COPY(ustr, st) len := 1; while (st^[len] <> chr(0)) do begin ustr[len] := st^[len]; len := len + 1 end;

procedure write_instruction(var arg0: text); external; 
procedure write_directive(var arg0: text); external; 

procedure put_integer_ws(var pFile: text; val: integer);
begin
    if val = 0 then return;
        if val > 0 then begin
            write(pFile, '+');
        end;
    write(pFile, val:1:10);
end;

procedure put_sym(var pFile: Text; idn: integer);
var
    sp40: Stringtext;
    str: pst_string;
    len: integer;

begin
    if (idn <> 0) then begin
        if (idn < 0) then begin
            write(pFile, '$', abs(idn):1);
        end else if ((st_fglobal_idn(idn) <> 1) and (source_language <> C_SOURCE)) then begin
            write(pFile, "$$", idn:1);
        end else begin
            sp40.ss := "";
            str := st_str_idn(idn);

            if (str = pointer(-1)) then begin
                write(pFile, "$$", idn:1);
            end else begin
                ST_COPY(sp40.ss, str);
            end;

            write(pFile, sp40.ss:0);
        end;
        
    end;
end;

procedure hex8(number: integer; var str: hex_str);
var
    i: integer;
    digit: 0..15;
begin
     str[1] := '0';
     str[2] := 'x';

    for i := 3 to 10 do begin
        digit := rshift(number, 32 - (i - 2) * 4) & 15;
        str[i] := hex_tab[digit];
    end;
    
     str[1] := '0';
     str[2] := 'x';
end;    

procedure put_hex10(var arg0: Text; arg1: integer);
var
    sp26: hex_str;
begin
    hex8(arg1, sp26);
    write(arg0, sp26);
end;

procedure hex_2(digit: char; var str: hex_str);
begin
    str[1] := hex_tab[(rshift(digit, 4) & 255)];
    str[2] := hex_tab[ord(digit) -((rshift(digit, 4) & 255) * 16)];
end; 

procedure put_alpha(var arg0: text; arg1: char);
var
    sp26: hex_str;
begin
    if not(arg1 in [' '..'~']) or (arg1 = '"') or (arg1 = '\') then begin
        hex_2(arg1, sp26);
        write(arg0, "\\X", sp26);
    end else begin
        write(arg0, arg1);
    end;
end;

GLOBAL_ASM("asm/7.1/functions/ugen/aio/write_instruction.s")

procedure print_source(var arg0: Text; idn: integer; arg2: integer);
var
    str: pst_string;
    len: integer;
begin
    if (idn <> current_filen) then begin
        current_filen := idn;
        if (idn = 0) then begin
            idn := 2;
        end;

        str := st_str_idn(idn);
        for len := 1 to FileNameLen do begin
            source_file_name[len] := ' ';
        end;
        ST_COPY(source_file_name, str);
        if (source_file_name[1] = ' ') then begin
            report_error(Internal, 977, "aio.p", "file name is null");
            return;
        end;
        current_linen := 16#7FFFFFFF;
    end;

    if (arg2 < (current_linen - 1)) then begin
        reset(source_file, source_file_name);
        if (eof(source_file)) then begin
            return;
        end;
        current_linen := 1;
    end;

    if (((arg2 - current_linen) >= 6)) then begin
        repeat begin
            if (eof(source_file)) then begin
                return;
            end;
        
            while not eoln(source_file) do begin
                get(source_file);
            end;

            get(source_file);
            current_linen := current_linen + 1;
        end until not ((current_linen < arg2));
    end;

    while (current_linen <= arg2) do begin
        write(arg0, " #", current_linen:4, chr(9));

        while not (eoln(source_file)) do begin
            write(arg0, source_file^);
            get(source_file);
        end;

        writeln(arg0);
        get(source_file);
        current_linen := current_linen + 1;
    end;
end;

GLOBAL_ASM("asm/7.1/functions/ugen/aio/write_directive.s")

procedure put_string(var arg0: text; arg1: boolean);
label
    loop_end;
var
    var_s2: u16;
    var_s1: integer;
    var_s0: cardinal;
    var_s6: cardinal;
begin
    if arg1 then begin
        write(arg0, '"');
    end;

    var_s2 := in_file^.length;
    for var_s6 := 1 to integer(var_s2 - 1) div 16 + 1 do begin {Get align 16}
        get(in_file);
        for var_s0 := 1 to 16 do begin
            var_s1 := lshift(var_s6, 4) - 16 + var_s0;
            if (var_s1 <= var_s2) then begin
                put_alpha(arg0, in_file^.data[var_s0]);
            end else begin
                goto loop_end;
            end;
        end;
    end;

loop_end:
    if arg1 then begin
        write(arg0, '"');
    end;
end;


procedure output_inst_ascii(var arg0: Stringtext; var arg1: Text);
begin
    reset(in_file, arg0.ss); {Just need to do an addr here..}

    while not eof(in_file) do begin 
        if in_file^.instr = iocode then begin
            write_instruction(arg1);
        end else begin
            write_directive(arg1);
        end;
        get(in_file)
    end;
    
end;