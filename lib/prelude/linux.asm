section '.text' executable

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
;      - count: Int
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

    ; t3 <- arg2.val
    mov     rax, [rbp + arg_2]
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
;
; close syscall
;
;   INPUT: rax contains self
;   STACK:
;      - file descriptor: Int
;   OUTPUT: rax points to the int object containing the return value
;
Linux.close:
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

    ; t1 <- arg0.val
    mov     rax, [rbp + arg_0]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_1], rax

    ; t2 <- syscall close(t1)
    mov     rdi, qword [rbp - loc_1]
    mov     rax, 3
    syscall
    mov     qword [rbp - loc_2], rax

    ; t0.val <- t2
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_2]
    mov     [rax], rdi

    ; return t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; socket syscall
;
;   INPUT: rax contains self
;   STACK:
;      - domain: Int
;      - type: Int
;      - protocol: Int
;   OUTPUT: rax points to the Int object containing the file descriptor
;
Linux.socket:
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

    ; t3 <- arg2.val
    mov     rax, [rbp + arg_2]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_3], rax

    ; t4 <- syscall socket(t1, t2, t3)
    mov     rdi, qword [rbp - loc_1]
    mov     rsi, qword [rbp - loc_2]
    mov     rdx, qword [rbp - loc_3]
    mov     rax, 41
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
    add     rsp, 40                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret
;
;
; accept syscall
;
;   INPUT: rax contains self
;   STACK:
;      - sockfd: Int
;      - addr: SockAddr
;      - addrlen: Int
;   OUTPUT: rax points to the Int object containing the file descriptor
;
Linux.accept:
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

    ; t2 <- *arg1.sa_data.s
    mov     rax, [rbp + arg_1]
    add     rax, [slot_0]
    mov     rax, [rax]
    add     rax, [slot_1]
    mov     qword [rbp - loc_2], rax

    ; t3 <- *arg2.val
    mov     rax, [rbp + arg_2]
    add     rax, [slot_0]
    mov     qword [rbp - loc_3], rax

    ; t4 <- syscall accept(t1, t2, t3)
    mov     rdi, qword [rbp - loc_1]
    mov     rsi, qword [rbp - loc_2]
    mov     rdx, qword [rbp - loc_3]
    mov     rax, 43
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
    add     rsp, 40                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; bind syscall
;
;   INPUT: rax contains self
;   STACK:
;      - sockfd: Int
;      - addr: SockAddr
;      - addrlen: Int
;   OUTPUT: rax points to the Int object containing the return value
;
Linux.bind:
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

    ; t2 <- *arg1.sa_data.s
    mov     rax, [rbp + arg_1]
    add     rax, [slot_0]
    mov     rax, [rax]
    add     rax, [slot_1]
    mov     qword [rbp - loc_2], rax

    ; t3 <- arg2.val
    mov     rax, [rbp + arg_2]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_3], rax

    ; t4 <- syscall bind(t1, t2, t3)
    mov     rdi, qword [rbp - loc_1]
    mov     rsi, qword [rbp - loc_2]
    mov     rdx, qword [rbp - loc_3]
    mov     rax, 49
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
    add     rsp, 40                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; listen syscall
;
;   INPUT: rax contains self
;   STACK:
;      - sockfd: Int
;      - backlog: Int
;   OUTPUT: rax points to the Int object containing the return value
;
Linux.listen:
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

    ; t3 <- syscall listen(t1, t2)
    mov     rdi, qword [rbp - loc_1]
    mov     rsi, qword [rbp - loc_2]
    mov     rax, 50
    syscall
    mov     qword [rbp - loc_3], rax

    ; t0.val <- t3
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_3]
    mov     [rax], rdi

    ; return t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 40                    ; deallocate local variables
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
