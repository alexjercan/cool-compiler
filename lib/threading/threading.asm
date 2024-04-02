section '.text' executable

extrn pthread_create
extrn pthread_join

;
;
; Helper method to wrap the `run` method of a thread object.
;
;  INPUT: rdi contains the thread object
;  STACK: none
;  OUTPUT: none
pthread_thread:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; PThread.run(thread)
    mov     rax, rdi
    push    0
    push    rax
    call    PThread.run
    add     rsp, 16

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; PThread.spawn
;
; Spawns a new thread object and calls the `run` method on it.
;
;   INPUT: rax contains self
;   STACK:
;      - thread: Thread - object with a `run` method
;   OUTPUT: rax contains new Int object with the thread id
;
PThread.spawn:
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

    ; t1 <- *t0.val
    mov     rax, [rbp - loc_0]
    add     rax, [slot_0]
    mov     qword [rbp - loc_1], rax

    ; t2 <- pthread_create(t1, 0, pthread_thread, arg_0)
    mov     rdi, [rbp - loc_1]
    mov     rsi, 0
    mov     rdx, pthread_thread
    mov     rcx, [rbp + arg_0]
    call    pthread_create

    ; return t0
    mov     rax, [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret


;
; PThread.join
;
; Joins a thread object.
;
;   INPUT: rax contains self
;   STACK:
;      - thread: Int - thread id
;   OUTPUT: a Ref object containing the return value of the thread
;
PThread.join:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t0 <- thread.val
    mov     rax, [rbp + arg_0]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     qword [rbp - loc_0], rax

    ; t1 <- pthread_join(t0, 0)
    mov     rdi, [rbp - loc_0]
    mov     rsi, 0
    call    pthread_join
    mov     qword [rbp - loc_1], rax

    pop     rbx                        ; restore register
    add     rsp, 24                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret
