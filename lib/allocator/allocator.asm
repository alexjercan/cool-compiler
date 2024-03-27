section '.data' writeable

; memory layout
heap_pos dq 0
heap_end dq 0

section '.text' executable

;
;
; allocator_init
;
;   INPUT: nothing
;   STACK: empty
;   OUTPUT: nothing
allocator_init:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame

    mov     rax, 12                    ; brk
    mov     rdi, 0                     ; increment = 0
    syscall
    mov     [heap_pos], rax            ; save the current position of the heap
    mov     [heap_end], rax            ; save the end of the heap

    pop     rbp                        ; restore return address
    ret

;
;
; allocate
;
;   INPUT: rdi contains the size in bytes
;   STACK: empty
;   OUTPUT: rax points to the newly allocated memory
;
allocate:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 16                    ; allocate 2 local variables

    ; t0 <- heap_pos
    mov     rax, qword [heap_pos]
    mov     qword [rbp - loc_0], rax

    ; t1 <- t0 + rdi
    mov     rax, qword [rbp - loc_0]
    add     rax, rdi
    mov     qword [rbp - loc_1], rax

    ; cmp t1 <= heap_end
    mov     rax, qword [rbp - loc_1]
    cmp     rax, qword [heap_end]
    jle     .alloc_ok

    mov     rax, 12                    ; brk
    mov     rdi, 0x10000               ; 64K bytes (larger obj. will fail)
    add     rdi, [heap_end]            ; new end of the heap
    syscall

    mov     [heap_end], rax            ; save the new end of the heap

.alloc_ok:

    ; heap_pos <- t1
    mov     rax, qword [rbp - loc_1]
    mov     qword [heap_pos], rax

    ; return t0
    mov     rax, qword [rbp - loc_0]

    add     rsp, 16                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret
