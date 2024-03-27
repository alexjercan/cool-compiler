section '.text' executable

extrn malloc

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

    pop     rbp                        ; restore return address
    ret

;
;
; allocate
;
;   This function allocates memory using the malloc function.
;
;   INPUT: rdi contains the size in bytes
;   STACK: empty
;   OUTPUT: rax points to the newly allocated memory
;
allocate:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame

    call    malloc

    pop     rbp                        ; restore return address
    ret
