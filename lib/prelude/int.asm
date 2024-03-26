section '.text' executable

;
;
; Int.chr
;
;   Converts an integer to a single character string (assume ASCII).
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains a string object
Int.chr:
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

    ; t2 <- *t1.s
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_field]
    mov     qword [rbp - loc_2], rax

    ; t3 <- 1
    mov     rax, 1
    mov     qword [rbp - loc_3], rax

    ; cmp t2 + t3 <= heap_end
    mov     rax, qword [rbp - loc_2]
    add     rax, qword [rbp - loc_3]
    cmp     rax, [heap_end]
    jle     .ok_chr

    ; malloc_64k()
    call    malloc_64k

.ok_chr:
    ; t4 <- self.val
    mov     rax, rbx
    add     rax, [int_slot]
    mov     rax, [rax]
    mov     qword [rbp - loc_4], rax

    ; t1.s <- t4
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_field]
    mov     rdi, qword [rbp - loc_4]
    mov     byte [rax], dil

    ; t0.val <- t3
    mov     rax, qword [rbp - loc_0]
    add     rax, [int_slot]
    mov     rdi, qword [rbp - loc_3]
    mov     [rax], rdi

    ; t1.l <- t0
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_size]
    mov     rdi, qword [rbp - loc_0]
    mov     qword [rax], rdi

    ; t5 <- (t3 + 7) >> 3
    mov     rax, qword [rbp - loc_3]
    add     rax, 7
    shr     rax, 3
    mov     qword [rbp - loc_5], rax

    ; size(t1) <- t5
    mov     rax, qword [rbp - loc_1]
    add     rax, [obj_size]
    mov     rdi, qword [rbp - loc_5]
    mov     [rax], rdi

    ; heap_pos <- heap_pos + (t3 << 3)
    mov     rax, qword [rbp - loc_3]
    shl     rax, 3
    add     [heap_pos], rax

    ; return t1
    mov     rax, qword [rbp - loc_1]

    pop     rbx                        ; restore register
    add     rsp, 56                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret
