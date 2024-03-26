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
