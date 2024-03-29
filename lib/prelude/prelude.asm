; self in rax
; arguments on the stack
; return value in rax

section '.data'
; Define some constants
obj_tag dq 0
obj_size dq 8
disp_tab dq 16
slot_0 dq 24
slot_1 dq 32
slot_2 dq 40

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
    call    allocator_init
    ; Call the main method
    mov     rax, Main_protObj
    call    Object.copy
    call    Main_init
    call    Main.main
    ; Exit the program
    mov     rax, 60
    xor     rdi, rdi
    syscall

;
;
; read syscall
;
;   INPUT: rax contains self
;   STACK:
;      - file descriptor: Int
;      - count: Int
;   OUTPUT: rax points to the string object containing the read string
;
Linux.read:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 56                    ; allocate 7 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- new Int
    mov     rax, Int_protObj
    call    Object.copy
    call    Int_init
    mov     qword [rbp - loc_0], rax

    ; t1 <- arg1.val
    mov     rax, [rbp + arg_1]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_1], rax

    ; t2 <- allocate_string(t1)
    mov     rdi, qword [rbp - loc_1]
    call    allocate_string
    mov     qword [rbp - loc_2], rax

    ; t3 <- *t2.s
    mov     rax, qword [rbp - loc_2]
    add     rax, [slot_1]
    mov     qword [rbp - loc_3], rax

    ; t4 <- arg0.val
    mov     rax, [rbp + arg_0]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_4], rax

    ; t5 <- syscall read(t4, t3, t1)
    mov     rdi, qword [rbp - loc_4]
    mov     rsi, qword [rbp - loc_3]
    mov     rdx, qword [rbp - loc_1]
    mov     rax, 0
    syscall
    mov     qword [rbp - loc_5], rax

    ; t0.val <- t5
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_5]
    mov     [rax], rdi

    ; t2.l <- t0
    mov     rax, qword [rbp - loc_2]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_0]
    mov     qword [rax], rdi

    ; return t2
    mov     rax, qword [rbp - loc_2]

    pop     rbx                        ; restore register
    add     rsp, 56                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; write syscall
;
;   INPUT: rax contains self
;   STACK:
;      - file descriptor: Int
;      - buf: String
;   OUTPUT: rax points to the int object containing the number of bytes written
;
Linux.write:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 56                    ; allocate 7 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- new Int
    mov     rax, Int_protObj
    call    Object.copy
    call    Int_init
    mov     qword [rbp - loc_0], rax

    ; t1 <- arg0.val
    mov     rax, [rbp + arg_0]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_1], rax

    ; t2 <- arg1.s
    mov     rax, [rbp + arg_1]
    add     rax, [slot_1]
    mov     qword [rbp - loc_2], rax

    ; t3 <- arg1.l.val
    mov     rax, [rbp + arg_1]
    add     rax, [slot_0]
    mov     rax, [rax]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_3], rax

    ; t4 <- syscall write(t1, t2, t3)
    mov     rdi, qword [rbp - loc_1]
    mov     rsi, qword [rbp - loc_2]
    mov     rdx, qword [rbp - loc_3]
    mov     rax, 1
    syscall
    mov     qword [rbp - loc_4], rax

    ; t0.val <- t4
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_4]
    mov     [rax], rdi

    ; return t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 56                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
; exit syscall
;
;   INPUT: rax contains self
;   STACK:
;      - status: Int
;   OUTPUT: never returns
;
Linux.exit:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables

    mov     rdi, [rbp + arg_0]
    add     rdi, [slot_0]
    mov     rdi, [rdi]

    mov     rax, 60
    syscall

;
;
; Copy method
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax points to the newly created copy.
;
Object.copy:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t1 <- sizeof(self)
    mov     rax, rbx
    add     rax, [obj_size]            ; get *self.size
    mov     rax, [rax]                 ; get self.size
    shl     rax, 3                     ; size = size * 8
    mov     qword [rbp - loc_1], rax

    ; t0 <- allocate(t1)
    mov     rdi, qword [rbp - loc_1]
    call    allocate
    mov     qword [rbp - loc_0], rax

    ; memcpy(t0, self, t1)
    mov     rdi, qword [rbp - loc_0]
    mov     rsi, rbx
    mov     rdx, qword [rbp - loc_1]
    call    memcpy

    ; t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Object.type_name
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax reference to class name string object
;
Object.type_name:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 40                    ; allocate 5 local variables
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
    add     rsp, 40                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Object.equals
;
;   Compares two objects for equality.
;   This represents the default implementation of the equals method.
;
;   INPUT: rax contains self
;   STACK:
;        x object
;   OUTPUT: rax contains a boolean object
Object.equals:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- new Bool
    mov     rax, Bool_protObj
    call    Object.copy
    call    Bool_init
    mov     qword [rbp - loc_0], rax

    ; cmp address of self == address of x
    mov     rdi, rbx
    mov     rsi, [rbp + arg_0]
    cmp     rdi, rsi
    setz    al
    movzx   rax, al
    mov     qword [rbp - loc_1], rax

    ; t0.val <- t1
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_1]
    mov     [rax], rdi

    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Byte.to_string
;
;   Converts a byte to a single character string (assume ASCII).
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains a string object
Byte.to_string:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 40                    ; allocate 5 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t3 <- 1
    mov     qword [rbp - loc_3], 1

    ; t0 <- new Int
    mov     rax, Int_protObj
    call    Object.copy
    call    Int_init
    mov     qword [rbp - loc_0], rax

    ; t1 <- allocate_string(t3)
    mov     rdi, qword [rbp - loc_3]
    call    allocate_string
    mov     qword [rbp - loc_1], rax

    ; t2 <- t1.s
    mov     rax, qword [rbp - loc_1]
    add     rax, [slot_1]
    mov     qword [rbp - loc_2], rax

    ; t4 <- self.val
    mov     rax, rbx
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_4], rax

    ; t1.s <- t4
    mov     rax, qword [rbp - loc_1]
    add     rax, [slot_1]
    mov     rdi, qword [rbp - loc_4]
    mov     byte [rax], dil

    ; t0.val <- t3
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_3]
    mov     [rax], rdi

    ; t1.l <- t0
    mov     rax, qword [rbp - loc_1]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_0]
    mov     qword [rax], rdi

    ; return t1
    mov     rax, qword [rbp - loc_1]

    pop     rbx                        ; restore register
    add     rsp, 40                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Byte.from_string
;
;   Converts a single character string to a byte (assume ASCII).
;
;   INPUT: rax contains self
;   STACK: contains a string object
;   OUTPUT: rax contains a byte object
Byte.from_string:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- arg0.s
    mov     rax, [rbp + arg_0]
    add     rax, [slot_1]
    movzx   rax, byte [rax]
    mov     qword [rbp - loc_0], rax

    ; self.val <- t0
    mov     rax, rbx
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_0]
    mov     [rax], rdi

    ; return self
    mov     rax, rbx

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Byte.from_int
;
;   Converts an integer to a byte.
;
;   INPUT: rax contains self
;   STACK: contains an int object
;   OUTPUT: rax contains a byte object
;
Byte.from_int:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- arg0.val
    mov     rax, [rbp + arg_0]
    add     rax, [slot_0]
    movzx   rax, byte [rax]
    mov     qword [rbp - loc_0], rax

    ; self.val <- t0
    mov     rax, rbx
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_0]
    mov     [rax], rdi

    ; return self
    mov     rax, rbx

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Byte.to_int
;
;   Converts a byte to an integer.
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains an int object
;
Byte.to_int:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- new Int
    mov     rax, Int_protObj
    call    Object.copy
    call    Int_init
    mov     qword [rbp - loc_0], rax

    ; t1 <- self.val
    mov     rax, rbx
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_1], rax

    ; t0.val <- t1
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_1]
    mov     byte [rax], dil

    ; return t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Float.from_int
;
;  Converts an integer to a float.
;
;  INPUT: rax contains self
;  STACK: contains an int object
;  OUTPUT: rax contains a float object
;
Float.from_int:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- arg0.val
    mov     rax, [rbp + arg_0]
    add     rax, [slot_0]
    movzx   rax, byte [rax]
    mov     qword [rbp - loc_0], rax

    ; self.val <- t0
    mov     rax, rbx
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_0]
    mov     [rax], rdi

    ; return self
    mov     rax, rbx

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Float.from_fraction
;
; Converts a fraction to a float.
;
; INPUT: rax contains self
; STACK:
;      - numerator: Int
;      - denominator: Int
; OUTPUT: rax contains a float object
;
Float.from_fraction:
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

    ; t1 <- arg0.val
    mov     rax, [rbp + arg_0]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_1], rax

    ; t2 <- arg1.val
    mov     rax, [rbp + arg_1]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_2], rax

    ; t3 <- t1 / t2
    mov     rax, qword [rbp - loc_1]
    cqo
    idiv    qword [rbp - loc_2]
    mov     qword [rbp - loc_3], rax

    ; self.val <- t3
    mov     rax, rbx
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_3]
    mov     [rax], rdi

    ; return self
    mov     rax, rbx

    pop     rbx                        ; restore register
    add     rsp, 40                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Float.to_int
;
;  Converts a float to an integer.
;
;  INPUT: rax contains self
;  STACK: empty
;  OUTPUT: rax contains an int object
;
Float.to_int:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- new Int
    mov     rax, Int_protObj
    call    Object.copy
    call    Int_init
    mov     qword [rbp - loc_0], rax

    ; t1 <- self.val
    mov     rax, rbx
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_1], rax

    ; t0.val <- t1
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_1]
    mov     byte [rax], dil

    ; return t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
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
String.concat:
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

    ; t2 <- self.l.val
    mov     rax, rbx
    add     rax, [slot_0]
    mov     rax, [rax]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_2], rax

    ; t3 <- arg1.l.val
    mov     rax, [rbp + arg_0]
    add     rax, [slot_0]
    mov     rax, [rax]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_3], rax

    ; t4 <- t2 + t3
    mov     rax, qword [rbp - loc_2]
    add     rax, qword [rbp - loc_3]
    mov     qword [rbp - loc_4], rax

    ; t1 <- allocate_string(t4)
    mov     rdi, qword [rbp - loc_4]
    call    allocate_string
    mov     qword [rbp - loc_1], rax

    ; rdi = t1.s, rsi = self.s, rdx = t2
    mov     rdi, qword [rbp - loc_1]
    add     rdi, [slot_1]
    mov     rsi, rbx
    add     rsi, [slot_1]
    mov     rdx, qword [rbp - loc_2]
    call    memcpy

    ; rdi = t1.s + t2, rsi = arg1.s, rdx = t3
    mov     rdi, qword [rbp - loc_1]
    add     rdi, [slot_1]
    add     rdi, qword [rbp - loc_2]
    mov     rsi, [rbp + arg_0]
    add     rsi, [slot_1]
    mov     rdx, qword [rbp - loc_3]
    call    memcpy

    ; t0.val <- t4
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_4]
    mov     [rax], rdi

    ; t1.l <- t0
    mov     rax, qword [rbp - loc_1]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_0]
    mov     [rax], rdi

    ; return t1
    mov     rax, qword [rbp - loc_1]

    pop     rbx                        ; restore register
    add     rsp, 40                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

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
String.substr:
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

    ; t3 <- arg1.val
    mov     rax, [rbp + arg_1]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_3], rax

    ; t1 <- allocate_string(t3)
    mov     rdi, qword [rbp - loc_3]
    call    allocate_string
    mov     qword [rbp - loc_1], rax

    ; t2 <- arg0.val
    mov     rax, [rbp + arg_0]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_2], rax

    ; rdi = t1.s, rsi = self.s + t2, rdx = t3
    mov     rdi, qword [rbp - loc_1]
    add     rdi, [slot_1]
    mov     rsi, rbx
    add     rsi, [slot_1]
    add     rsi, qword [rbp - loc_2]
    mov     rdx, qword [rbp - loc_3]
    call    memcpy

    ; t0.val <- t3
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_3]
    mov     [rax], rdi

    ; t1.l <- t0
    mov     rax, qword [rbp - loc_1]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_0]
    mov     [rax], rdi

    ; return t1
    mov     rax, qword [rbp - loc_1]

    pop     rbx                        ; restore register
    add     rsp, 40                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; allocate_string
;
;   INPUT: rdi contains the size in bytes
;   STACK: empty
;   OUTPUT: rax points to the newly allocated string object
;
allocate_string:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 32                    ; allocate 4 local variables

    ; t1 <- (rdi + 7) >> 3
    mov     rax, rdi
    add     rax, 7
    shr     rax, 3
    mov     qword [rbp - loc_1], rax

    ; t0 <- new String
    mov     rax, String_protObj
    call    Object.copy
    call    String_init
    mov     qword [rbp - loc_0], rax

    ; t2 <- size(t0) + t1
    mov     rax, qword [rbp - loc_0]
    add     rax, [obj_size]
    mov     rax, [rax]
    add     rax, qword [rbp - loc_1]
    mov     qword [rbp - loc_2], rax

    ; size(t0) <- t2
    mov     rax, qword [rbp - loc_0]
    add     rax, [obj_size]
    mov     rdi, qword [rbp - loc_2]
    mov     [rax], rdi

    ; t3 <- t0.copy()
    mov     rax, qword [rbp - loc_0]
    call    Object.copy
    call    String_init
    mov     qword [rbp - loc_3], rax

    ; should we free t0?

    ; return t3
    mov     rax, qword [rbp - loc_3]

    add     rsp, 32                    ; deallocate local variables
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

    pop     rbp                        ; restore return address
    ret
