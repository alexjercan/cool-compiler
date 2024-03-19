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
    xor     rdi, rdi
    syscall

segment readable writable
heap_pos dq 0
heap_end dq 0

; Define the messages
segment readable
_abort_msg db "Abort called from class ", 0
_abort_msg_len dq $ - _abort_msg
_nl db 10, 0
_nl_len dq $ - _nl


; Define some constants
segment readable
obj_tag dq 0
obj_size dq 8
int_slot dq 16
bool_slot dq 16
str_size dq 16
str_field dq 24
read_len dq 1024

loc_0 = 8
loc_1 = 16
loc_2 = 24
loc_3 = 32
loc_4 = 40
loc_5 = 48

arg_0 = 16
arg_1 = 24
arg_2 = 32

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
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- heap_pos
    mov     rax, qword [heap_pos]
    mov     qword [rbp - loc_0], rax

    ; t1 <- sizeof(self)
    mov     rax, rbx
    call    sizeof
    mov     qword [rbp - loc_1], rax

    ; t2 <- t0 + t1
    mov     rax, qword [rbp - loc_0]
    add     rax, qword [rbp - loc_1]
    mov     qword [rbp - loc_2], rax

    ; cmp t2 <= heap_end
    mov     rax, qword [rbp - loc_2]
    cmp     rax, qword [heap_end]
    jle     .copy

    ; malloc_64k()
    call    malloc_64k

.copy:
    ; memcpy(t0, self, t1)
    mov     rdi, qword [rbp - loc_0]
    mov     rsi, rbx
    mov     rdx, qword [rbp - loc_1]
    call    memcpy

    ; heap_pos <- t2
    mov     rax, qword [rbp - loc_2]
    mov     qword [heap_pos], rax

    ; t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
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
    mov     rdi, 1                     ; fd = stdout
    mov     rsi, _abort_msg            ; str "Abort called from class "
    mov     rdx, [_abort_msg_len]      ; length of the string
    mov     rax, 1                     ; write
    syscall

    ; t0 <- self@Object.type_name()
    mov     rax, rbx
    call    Object.type_name

    ; t1 <- self@IO.out_string(t0)
    push    rax
    mov     rax, rbx
    call    IO.out_string
    pop     rax

    mov     rdi, 1                     ; fd = stdout
    mov     rsi, _nl                   ; str = "\n"
    mov     rdx, [_nl_len]             ; length of the string
    mov     rax, 1                     ; write
    syscall

    mov     rax, 60
    xor     rdi, rdi
    syscall

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
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 32                    ; allocate 4 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- class_nameTab
    mov    rax, class_nameTab
    mov    qword [rbp - loc_0], rax

    ; t1 <- obj_tag(self)
    mov    rax, rbx
    add    rax, [obj_tag]
    mov    rax, [rax]
    mov    qword [rbp - loc_1], rax

    ; t2 <- t1 * 8
    mov    rax, qword [rbp - loc_1]
    shl    rax, 3
    mov    qword [rbp - loc_2], rax

    ; t3 <- t0 + t2
    mov    rax, qword [rbp - loc_0]
    add    rax, qword [rbp - loc_2]
    mov    qword [rbp - loc_3], rax

    ; deref t3
    mov    rax, qword [rbp - loc_3]
    mov    rax, [rax]

    pop     rbx                        ; restore register
    add     rsp, 32                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

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

    ; deref x.s
    mov     rax, [rbp + arg_0]         ; get x
    add     rax, [str_field]           ; get *x.s
    mov     rsi, rax                   ; buf = *x.s

    ; deref x.l.val
    mov     rax, [rbp + arg_0]         ; get x
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

    ; deref x.val
    mov     rax, [rbp + arg_0]         ; get x
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
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 40                    ; allocate 5 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, Int_protObj
    call    Object.copy
    call    Int_init
    mov     qword [rbp - loc_0], rax   ; t0 <- new int object

    mov     rax, rbx                   ; get self
    call    IO.in_string               ; read string from terminal
    mov     qword [rbp - loc_1], rax   ; t1 <- string object

    mov     rax, qword [rbp - loc_1]   ; get t1
    add     rax, [str_field]           ; get *t1.s
    mov     qword [rbp - loc_2], rax   ; t2 <- *t1.s (start pointer)

    mov     rax, qword [rbp - loc_1]   ; get t1
    add     rax, [str_size]            ; get *t1.l
    mov     rax, [rax]                 ; get t1.l
    add     rax, [int_slot]            ; get *t1.l.val
    mov     rax, [rax]                 ; get t1.l.val
    mov     qword [rbp - loc_3], rax   ; t3 <- t1.l.val

    mov     rdi, qword [rbp - loc_2]   ; get *t1.s
    mov     rsi, qword [rbp - loc_3]   ; get t1.l.val
    call    parseInt                   ; convert string to int
    mov     qword [rbp - loc_4], rax   ; t4 <- int value

    mov     rax, qword [rbp - loc_0]   ; get t0
    add     rax, [int_slot]            ; get *t0.val
    mov     rdi, qword [rbp - loc_4]   ; get t4
    mov     qword [rax], rdi           ; *t0.val <- t4

    mov     rax, qword [rbp - loc_0]   ; get t0

    add     rsp, 40                    ; deallocate local variables
    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

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
; TODO: Kind of have to also check for EOF
segment readable executable
IO.in_string:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 40                    ; allocate 5 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- new Int
    mov     rax, Int_protObj
    call    Object.copy
    call    Int_init
    mov     qword [rbp - loc_0], rax

    ; t1 <- new String
    mov     rax, String_protObj
    call    Object.copy
    call    String_init
    mov     qword [rbp - loc_1], rax

    ; t2 <- *t1.s
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_field]
    mov     qword [rbp - loc_2], rax

    ; t3 <- *t1.s
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_field]
    mov     qword [rbp - loc_3], rax

.again:
    ; cmp t3 + read_len <= heap_end
    mov     rax, qword [rbp - loc_3]
    add     rax, [read_len]
    cmp     rax, [heap_end]
    jle     .read

    ; malloc_64k()
    call    malloc_64k

.read:
    ; t4 <- syscall read(stdin, t3, read_len)
    mov     rdi, 0
    mov     rsi, qword [rbp - loc_3]
    mov     rdx, [read_len]
    mov     rax, 0
    syscall
    mov     qword [rbp - loc_4], rax

    ; t3 <- t3 + t4
    mov     rax, qword [rbp - loc_4]
    add     qword [rbp - loc_3], rax
    mov     rax, qword [rbp - loc_3]

    ; cmp *t3 - 1 != '\n'
    mov     al, byte [rax - 1]
    cmp     al, 10
    jne     .again

    ; t4 <- t3 - t2
    mov     rax, qword [rbp - loc_3]
    sub     rax, qword [rbp - loc_2]
    mov     qword [rbp - loc_4], rax

    ; t0.val <- t4 - 1
    mov     rax, qword [rbp - loc_0]
    add     rax, [int_slot]
    mov     rdi, qword [rbp - loc_4]
    dec     rdi
    mov     qword [rax], rdi

    ; t1.l <- t0
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_size]
    mov     rdi, qword [rbp - loc_0]
    mov     qword [rax], rdi

    mov     rax, qword [rbp - loc_1]

    pop     rbx                        ; restore register
    add     rsp, 40                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

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
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, rbx                   ; get self
    add     rax, [str_size]            ; get *self.l
    mov     rax, [rax]                 ; get self.l

    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

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
;	INPUT: rax contains self
;	STACK:
;		i int object
;		l int object
;	OUTPUT:	rax contains the string object which is the sub string of self
;
segment readable executable
String.substr:

;
;
; sizeof
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains the size of the object
;
sizeof:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, rbx                   ; get self
    add     rax, [obj_size]            ; get *self.size
    mov     rax, [rax]                 ; get self.size

    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

;
;
; malloc_64k
;   INPUT: none
;   STACK: empty
;   OUTPUT: none
;
malloc_64k:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, 12                    ; brk
    mov     rdi, 0x10000               ; 64K bytes (larger obj. will fail)
    add     rdi, [heap_end]            ; new end of the heap
    syscall
    mov     [heap_end], rax            ; save the new end of the heap

    mov     rax, rbx                   ; restore self
    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

;
;
; memcpy
;   INPUT:
;       rdi points to destination
;       rsi points to source
;       rdx contains the number of bytes to copy
;   STACK: empty
;   OUTPUT: nothing
;
memcpy:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

.next_byte:
    cmp     rdx, 0                     ; check if done
    jle     .done

    mov     al, byte [rsi]             ; get byte from self
    mov     byte [rdi], al             ; copy byte to new object

    inc     rdi                        ; increment destination
    inc     rsi                        ; increment source
    dec     rdx                        ; decrement count

    jmp .next_byte
.done:

    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

;
;
; parseInt
;   INPUT:
;       rdi points to the string
;       rsi contains the length of the string
;   STACK: empty
;   OUTPUT: rax contains the integer value of the string
parseInt:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 16                    ; allocate 2 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rcx, 10                    ; base = 10
    mov     rax, 0                     ; result = 0
    mov     qword [rbp - loc_0], rax   ; t0 <- result

.next_digit:
    cmp     rsi, 0                     ; check if done
    jle     .done

    xor     rax, rax                   ; clear rax
    mov     al, byte [rdi]             ; get byte from string
    cmp     rax, '0'
    jl      .done
    cmp     rax, '9'
    jg      .done
    sub     rax, '0'                   ; convert to integer

    mov     qword [rbp - loc_1], rax   ; t1 <- digit
    mov     rax, qword [rbp - loc_0]   ; get t0
    mul     rcx
    add     rax, qword [rbp - loc_1]
    mov     qword [rbp - loc_0], rax   ; t0 <- t0 * 10 + t1

    inc     rdi                        ; increment string
    dec     rsi                        ; decrement count

    jmp     .next_digit
.done:

    mov     rax, qword [rbp - loc_0]      ; get t0

    pop     rbx                        ; restore register
    add     rsp, 16                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret
