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

    ; t1 <- new String
    mov     rax, String_protObj
    call    Object.copy
    call    String_init
    mov     qword [rbp - loc_1], rax

    ; t2 <- *t1.s
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_field]
    mov     qword [rbp - loc_2], rax

    ; t3 <- arg1.val
    mov     rax, [rbp + arg_1]
    add     rax, [int_slot]
    mov     rax, [rax]
    mov     qword [rbp - loc_3], rax

    ; cmp t2 + t3 <= heap_end
    mov     rax, qword [rbp - loc_2]
    add     rax, qword [rbp - loc_3]
    cmp     rax, [heap_end]
    jle     .read

    ; malloc_64k()
    call    malloc_64k

.read:
    ; t4 <- arg0.val
    mov     rax, [rbp + arg_0]
    add     rax, [int_slot]
    mov     rax, [rax]
    mov     qword [rbp - loc_4], rax

    ; t5 <- syscall read(t4, t2, t3)
    mov     rdi, qword [rbp - loc_4]
    mov     rsi, qword [rbp - loc_2]
    mov     rdx, qword [rbp - loc_3]
    mov     rax, 0
    syscall
    mov     qword [rbp - loc_5], rax

    ; t0.val <- t5
    mov     rax, qword [rbp - loc_0]
    add     rax, [int_slot]
    mov     rdi, qword [rbp - loc_5]
    mov     [rax], rdi

    ; t1.l <- t0
    mov     rax, qword [rbp - loc_1]
    add     rax, [str_size]
    mov     rdi, qword [rbp - loc_0]
    mov     qword [rax], rdi

    ; t6 <- (t5 + 7) >> 3
    mov     rax, qword [rbp - loc_5]
    add     rax, 7
    shr     rax, 3
    mov     qword [rbp - loc_6], rax

    ; size(t1) <- t6
    mov     rax, qword [rbp - loc_1]
    add     rax, [obj_size]
    mov     rdi, qword [rbp - loc_6]
    mov     [rax], rdi

    ; heap_pos <- heap_pos + (t6 << 3)
    mov     rax, qword [rbp - loc_6]
    shl     rax, 3
    add     [heap_pos], rax

    ; return t1
    mov     rax, qword [rbp - loc_1]

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
    add     rax, [int_slot]
    mov     rax, [rax]
    mov     qword [rbp - loc_1], rax

    ; t2 <- arg1.s
    mov     rax, [rbp + arg_1]
    add     rax, [str_field]
    mov     qword [rbp - loc_2], rax

    ; t3 <- arg1.l.val
    mov     rax, [rbp + arg_1]
    add     rax, [str_size]
    mov     rax, [rax]
    add     rax, [int_slot]
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
    add     rax, [int_slot]
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

    mov     rdi, [rsp + arg_0]
    add     rdi, [int_slot]
    mov     rdi, [rdi]

    mov     rax, 60
    syscall
