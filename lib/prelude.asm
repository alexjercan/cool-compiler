format ELF64 executable 3

; self in rax
; arguments on the stack
; return value in rax

; Define entry point
entry _start
segment readable executable
_start:
    ; Testing if IO.out_string works
    mov     rax, String_protObj
    push    rax
    call    IO.out_string
    ; Exit the program
    mov     rax, 60
    xor     edi, edi
    syscall

; Define some constants
segment readable
obj_tag dq 0
obj_size dq 8
disp_tab dq 16
int_slot dq 24
bool_slot dq 24
str_size dq 24
str_field dq 32

;
;
; Copy method
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax points to the newly created copy.
;
segment readable executable
Object.copy:

;
;
; Object.abort
;
;   The abort method for the object class (usually inherited by
;   all other classes)
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: never returns
;
segment readable executable
Object.abort:

;
;
; Object.type_name
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax reference to class name string object
;
segment readable executable
Object.type_name:

;
;
; IO.out_string
;
;   Prints out the contents of a string object argument
;   which is on top of the stack.
;
;   INPUT: rax contains self
;   STACK:
;        x string object
;   OUTPUT: rax contains self
;
segment readable executable
IO.out_string:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rax                        ; save self

    mov     rax, [rbp + 16]            ; get x

    mov     rbx, rax                   ; get x
    add     rbx, [str_field]           ; get *x.s
    mov     rsi, rbx                   ; buf = *x.s

    mov     rbx, rax                   ; get x
    add     rbx, [str_size]            ; get *x.l
    mov     rbx, [rbx]                 ; get x.l
    add     rbx, [int_slot]            ; get *x.l.val
    mov     rbx, [rbx]                 ; get x.l.val
    mov     rdx, rbx                   ; count = x.l.val

    mov     rax, 1                     ; write
    mov     rdi, 1                     ; fd = stdout
    syscall

    pop     rax                        ; return self
    pop     rbp                        ; restore return address
    ret

;
;
; IO.out_int
;
;   Prints out the contents of an integer object on top of the
;   stack.
;
;   INPUT: rax contains self
;   STACK:
;        x int object
;   OUTPUT: rax contains self
;
segment readable executable
IO.out_int:

;
;
; IO.in_int
;
;   Returns an integer object read from the terminal in rax.
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains the integer object read from the terminal
;
segment readable executable
IO.in_int:

;
;
; IO.in_string
;
;   Returns a string object read from the terminal, removing the
;   '\n'
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains the string object read from the terminal
;
segment readable executable
IO.in_string:

;
;
; String.length
;
;   Returns Int Obj with string length of self
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax the int object which is the size of the string
;
segment readable executable
String.length:

;
;
; String.concat
;
;   Returns a the concatenation of self and arg1
;
;   INPUT: rax contains self
;   STACK:
;        s string object
;   OUTPUT: rax the string object which is the concatenation of self and arg1
;
segment readable executable
String.concat:

;
;
; String.substr(i,l)
;		Returns the sub string of self from i with length l
;		Offset starts at 0.
;
;	INPUT:	rax contains self
;	STACK:
;		i int object
;		l int object
;	OUTPUT:	rax contains the string object which is the sub string of self
;
segment readable executable
String.substr:

