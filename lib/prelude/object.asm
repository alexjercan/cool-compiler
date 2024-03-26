section '.text' executable

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

    ; t0 <- heap_pos
    mov     rax, qword [heap_pos]
    mov     qword [rbp - loc_0], rax

    ; t1 <- sizeof(self)
    mov     rax, rbx
    add     rax, [obj_size]            ; get *self.size
    mov     rax, [rax]                 ; get self.size
    shl     rax, 3                     ; size = size * 8
    mov     qword [rbp - loc_1], rax

    ; t2 <- t0 + t1
    mov     rax, qword [rbp - loc_0]
    add     rax, qword [rbp - loc_1]
    mov     qword [rbp - loc_2], rax

    ; cmp t2 <= heap_end
    mov     rax, qword [rbp - loc_2]
    cmp     rax, qword [heap_end]
    jle     .copy

    ; malloc_64k()
    call    malloc_64k

.copy:
    ; memcpy(t0, self, t1)
    mov     rdi, qword [rbp - loc_0]
    mov     rsi, rbx
    mov     rdx, qword [rbp - loc_1]
    call    memcpy

    ; heap_pos <- t2
    mov     rax, qword [rbp - loc_2]
    mov     qword [heap_pos], rax

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
    sub     rsp, 32                    ; allocate 4 local variables
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
    add     rsp, 32                    ; deallocate local variables
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
    sub     rsp, 16                    ; allocate 2 local variables
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
    add     rax, [bool_slot]
    mov     rdi, qword [rbp - loc_1]
    mov     [rax], rdi

    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 16                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret
