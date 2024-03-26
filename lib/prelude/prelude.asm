; self in rax
; arguments on the stack
; return value in rax

section '.data' writeable
; memory layout
heap_pos dq 0
heap_end dq 0

section '.data'
; Define some constants
obj_tag dq 0
obj_size dq 8
disp_tab dq 16
int_slot dq 24
bool_slot dq 24
str_size dq 24
str_field dq 32

loc_0 = 8
loc_1 = 16
loc_2 = 24
loc_3 = 32
loc_4 = 40
loc_5 = 48
loc_6 = 56
loc_7 = 64

arg_0 = 16
arg_1 = 24
arg_2 = 32
arg_3 = 40
arg_4 = 48
arg_5 = 56
arg_6 = 64
arg_7 = 72

; Define entry point
section '.text' executable
public _start
_start:
    ; Initialize the heap
    mov     rax, 12                    ; brk
    mov     rdi, 0                     ; increment = 0
    syscall
    mov     [heap_pos], rax            ; save the current position of the heap
    mov     [heap_end], rax            ; save the end of the heap
    ; Call the main method
    mov     rax, Main_protObj
    call    Object.copy
    call    Main_init
    call    Main.main
    ; Exit the program
    mov     rax, 60
    xor     rdi, rdi
    syscall
