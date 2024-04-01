extrn time
extrn srand
extrn random

;
;
; Time.time
;
;   Get current time in seconds since epoch
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains an Int object
Time.time:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t1 <- time(NULL)
    mov     rdi, 0
    call    time
    mov     qword [rbp - loc_1], rax

    ; t0 <- new Int
    mov     rax, Int_protObj
    call    Object.copy
    call    Int_init
    mov     qword [rbp - loc_0], rax

    ; t0.val <- t1
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_1]
    mov     [rax], rdi

    ; return t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Random.srand
;
;   Seed the random number generator
;
;   INPUT: rax contains self
;   STACK: int seed
;   OUTPUT: SELF
Random.srand:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; srand(seed)
    mov     rdi, qword [rbp + arg_0]
    call    srand

    ; return self
    mov     rax, rbx

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Random.random
;
;   Get a random number between 0 and MAX_INT
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains an Int object
;
Random.random:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t1 <- rand()
    call    random
    mov     qword [rbp - loc_1], rax

    ; t0 <- new Int
    mov     rax, Int_protObj
    call    Object.copy
    call    Int_init
    mov     qword [rbp - loc_0], rax

    ; t0.val <- t1
    mov     rax, qword [rbp - loc_0]
    add     rax, [slot_0]
    mov     rdi, qword [rbp - loc_1]
    mov     [rax], rdi

    ; return t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

