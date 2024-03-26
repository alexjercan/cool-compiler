section '.text' executable

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
    sub     rsp, 56                    ; allocate 7 local variables
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

    ; t2 <- self.l.val
    mov     rax, rbx
    add     rax, [str_size]
    mov     rax, [rax]
    add     rax, [int_slot]
    mov     rax, [rax]
    mov     qword [rbp - loc_2], rax

    ; t3 <- arg1.l.val
    mov     rax, [rbp + arg_0]
    add     rax, [str_size]
    mov     rax, [rax]
    add     rax, [int_slot]
    mov     rax, [rax]
    mov     qword [rbp - loc_3], rax

    ; t4 <- t2 + t3
    mov     rax, qword [rbp - loc_2]
    add     rax, qword [rbp - loc_3]
    mov     qword [rbp - loc_4], rax

    ; t5 <- t1.s + t4
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_field]
    add     rax, qword [rbp - loc_4]
    mov     qword [rbp - loc_5], rax

    ; cmp t5 <= heap_end
    mov     rax, qword [rbp - loc_5]
    cmp     rax, qword [heap_end]
    jle     .ok_concat

    ; malloc_64k()
    call    malloc_64k

.ok_concat:

    ; rdi = t1.s, rsi = self.s, rdx = self.l.val
    mov     rdi, qword [rbp - loc_1]
    add     rdi, [str_field]
    mov     rsi, rbx
    add     rsi, [str_field]
    mov     rdx, qword [rbp - loc_2]
    call    memcpy

    ; rdi = t1.s + self.l.val, rsi = arg1.s, rdx = arg1.l.val
    mov     rdi, qword [rbp - loc_1]
    add     rdi, [str_field]
    add     rdi, qword [rbp - loc_2]
    mov     rsi, [rbp + arg_0]
    add     rsi, [str_field]
    mov     rdx, qword [rbp - loc_3]
    call    memcpy

    ; t0.val <- t4
    mov     rax, qword [rbp - loc_0]
    add     rax, [int_slot]
    mov     rdi, qword [rbp - loc_4]
    mov     [rax], rdi

    ; t1.l <- t0
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_size]
    mov     rdi, qword [rbp - loc_0]
    mov     [rax], rdi

    ; t6 <- ((t5 - t1 + 7) >> 3) << 3
    mov     rax, qword [rbp - loc_5]
    sub     rax, qword [rbp - loc_1]
    add     rax, 7
    shr     rax, 3
    shl     rax, 3
    mov     qword [rbp - loc_6], rax

    ; size(t1) <- t6 >> 3
    mov     rax, qword [rbp - loc_6]
    shr     rax, 3
    mov     rdi, rax
    mov     rax, qword [rbp - loc_1]
    add     rax, [obj_size]
    mov     [rax], rdi

    ; heap_pos <- heap_pos + t6
    mov     rax, qword [rbp - loc_6]
    add     [heap_pos], rax

    mov     rax, qword [rbp - loc_1]   ; get t1

    pop     rbx                        ; restore register
    add     rsp, 56                    ; deallocate local variables
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
    sub     rsp, 48                    ; allocate 6 local variables
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

    ; t2 <- arg0.val
    mov     rax, [rbp + arg_0]
    add     rax, [int_slot]
    mov     rax, [rax]
    mov     qword [rbp - loc_2], rax

    ; t3 <- arg2.val
    mov     rax, [rbp + arg_1]
    add     rax, [int_slot]
    mov     rax, [rax]
    mov     qword [rbp - loc_3], rax

    ; t4 <- t1.s + t3
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_field]
    add     rax, qword [rbp - loc_3]
    mov     qword [rbp - loc_4], rax

    ; cmp t4 <= heap_end
    mov     rax, qword [rbp - loc_4]
    cmp     rax, qword [heap_end]
    jle     .ok_substr

    ; malloc_64k()
    call    malloc_64k

.ok_substr:

    ; rdi = t1.s, rsi = self.s + t2, rdx = t3
    mov     rdi, qword [rbp - loc_1]
    add     rdi, [str_field]
    mov     rsi, rbx
    add     rsi, [str_field]
    add     rsi, qword [rbp - loc_2]
    mov     rdx, qword [rbp - loc_3]
    call    memcpy

    ; t0.val <- t3
    mov     rax, qword [rbp - loc_0]
    add     rax, [int_slot]
    mov     rdi, qword [rbp - loc_3]
    mov     [rax], rdi

    ; t1.l <- t0
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_size]
    mov     rdi, qword [rbp - loc_0]
    mov     [rax], rdi

    ; t5 <- ((t4 - t1 + 7) >> 3) << 3
    mov     rax, qword [rbp - loc_4]
    sub     rax, qword [rbp - loc_1]
    add     rax, 7
    shr     rax, 3
    shl     rax, 3
    mov     qword [rbp - loc_5], rax

    ; size(t1) <- t5 >> 3
    mov     rax, qword [rbp - loc_5]
    shr     rax, 3
    mov     rdi, rax
    mov     rax, qword [rbp - loc_1]
    add     rax, [obj_size]
    mov     [rax], rdi

    ; heap_pos <- heap_pos + t5
    mov     rax, qword [rbp - loc_5]
    add     [heap_pos], rax

    mov     rax, qword [rbp - loc_1]   ; get t1

    pop     rbx                        ; restore register
    add     rsp, 48                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret
;
;
; String.ord
;
;   Converts a single character string to an integer.
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains an integer object
String.ord:
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

    ; t1 <- self.s
    mov     rax, rbx
    add     rax, [str_field]
    movzx   rax, byte [rax]
    mov     qword [rbp - loc_1], rax

    ; t0.val <- t1
    mov     rax, qword [rbp - loc_0]
    add     rax, [int_slot]
    mov     rdi, qword [rbp - loc_1]
    mov     [rax], rdi

    ; return t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret
