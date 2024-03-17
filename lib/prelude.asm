format ELF64 executable 3

; self in rax
; arguments on the stack
; return value in rax

; Define entry point
entry _start
segment readable executable
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
    xor     edi, edi
    syscall

segment readable writable
heap_pos dq 0
heap_end dq 0

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
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    push    [heap_pos]                 ; push new object position
    mov     rbx, rax                   ; save self

    mov     rax, rbx                   ; get self
    add     rax, [obj_size]            ; get *self.size
    mov     rax, [rax]                 ; get self.size (in qwords)
    shl     rax, 3                     ; get self.size (in bytes)

    add     rax, [heap_pos]            ; heap_pos after copy
    cmp     rax, [heap_end]            ; check if there is enough space
    jle     _oc_ok

    ; allocate more space
    mov     rax, 12                    ; brk
    mov     rdi, 0x10000               ; 64K bytes (larger obj. will fail)
    add     rdi, [heap_end]            ; new end of the heap
    syscall
    mov     [heap_end], rax            ; save the new end of the heap

_oc_ok:
    mov     rax, rbx                   ; get self
    add     rax, [obj_size]            ; get *self.size
    mov     rax, [rax]                 ; get self.size (in qwords)
_oc_loop:
    mov     rdi, [heap_pos]            ; get new object position
    mov     rsi, [rbx]                 ; get *self[i]
    add     rbx, 8                     ; next slot
    dec     rax                        ; decrement counter
    mov     [rdi], rsi                 ; copy slot
    add     [heap_pos], 8              ; increment new object position
    cmp     rax, 0                     ; check if done
    jg      _oc_loop

    pop     rax                        ; return new object
    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

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
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, [rbp + 16]            ; get x
    add     rax, [str_field]           ; get *x.s
    mov     rsi, rax                   ; buf = *x.s

    mov     rax, [rbp + 16]            ; get x
    add     rax, [str_size]            ; get *x.l
    mov     rax, [rax]                 ; get x.l
    add     rax, [int_slot]            ; get *x.l.val
    mov     rax, [rax]                 ; get x.l.val
    mov     rdx, rax                   ; count = x.l.val

    mov     rax, 1                     ; write
    mov     rdi, 1                     ; fd = stdout
    syscall

    mov     rax, rbx                   ; restore self and return it
    pop     rbx                        ; restore register
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
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, [rbp + 16]            ; get x
    add     rax, [int_slot]            ; get *x.val
    mov     rax, [rax]                 ; get x.val

    push    rbx                        ; save register

    ; print the integer
    xor     rcx, rcx                   ; count = 0
    mov     rbx, 10                    ; base = 10
.IO.out_int.stack:
    xor     rdx, rdx                   ; clear rdx
    div     rbx                        ; rax = rax / 10, rdx = rax % 10
    add     rdx, '0'                   ; convert remainder to ASCII
    push    rdx                        ; save remainder
    inc     rcx                        ; increment count
    test    rax, rax                   ; check if done
    jnz     .IO.out_int.stack          ; loop if not done
.IO.out_int.write:
    lea     rsi, [rsp]                 ; buf = *stack
    mov     rdx, 1                     ; count = 1
    mov     rax, 1                     ; write
    mov     rdi, 1                     ; fd = stdout
    push    rcx                        ; save count
    syscall
    pop     rcx                        ; restore count

    pop     rdx                        ; throw away remainder
    dec     rcx                        ; decrement count
    jnz     .IO.out_int.write          ; loop if not done

    pop     rbx                        ; restore register

    mov     rax, rbx                   ; restore self and return it
    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

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

