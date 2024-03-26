section '.text' executable

extrn InitWindow
extrn WindowShouldClose
extrn CloseWindow
extrn BeginDrawing
extrn EndDrawing
extrn ClearBackground
extrn DrawText

;
;
; Raylib.initWindow
;
;   Initializes window and OpenGL context
;
;   INPUT: rax contains self
;   STACK:
;      - window width: Int
;      - window height: Int
;      - window title: String
;   OUTPUT: rax contains a new object
Raylib.initWindow:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, [int_slot]
    mov     rax, [rax]
    mov     rdi, rax

    mov     rax, qword [rbp + arg_1]
    add     rax, [int_slot]
    mov     rax, [rax]
    mov     rsi, rax

    mov     rax, qword [rbp + arg_2]
    add     rax, [str_field]
    mov     rdx, rax

    call    InitWindow

    mov     rax, Object_protObj
    call    Object.copy
    call    Object_init

    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

;
;
; Raylib.windowShouldClose
;
;   Check if KEY_ESCAPE pressed or Close icon pressed
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains a bool object
Raylib.windowShouldClose:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 16                    ; allocate 2 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    ; t1 <- WindowShouldClose()
    call    WindowShouldClose
    mov     qword [rbp - loc_1], rax

    ; t0 <- new Bool
    mov     rax, Bool_protObj
    call    Object.copy
    call    Bool_init
    mov     qword [rbp - loc_0], rax

    ; t0.val <- t1
    mov     rax, qword [rbp - loc_0]
    add     rax, [bool_slot]
    mov     rdi, qword [rbp - loc_1]
    mov     [rax], rdi

    ; return t0
    mov     rax, qword [rbp - loc_0]

    pop     rbx                        ; restore register
    add     rsp, 16                    ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Raylib.closeWindow
;
;   Close window and unload OpenGL context
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains a new object
Raylib.closeWindow:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    call    CloseWindow

    ; return new Object
    mov     rax, Object_protObj
    call    Object.copy
    call    Object_init

    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

;
;
; Raylib.beginDrawing
;
;   Setup canvas (framebuffer) to start drawing
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains self
Raylib.beginDrawing:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    call    BeginDrawing

    mov     rax, rbx                   ; return self

    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

;
;
; Raylib.endDrawing
;
;   End canvas drawing and swap buffers (double buffering)
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains self
Raylib.endDrawing:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    call    EndDrawing

    mov     rax, rbx                   ; return self

    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

;
;
; Raylib.clearBackground
;
;   Clear background with color
;
;   INPUT: rax contains self
;   STACK:
;      - color: Color (field1: String with 4 bytes)
;   OUTPUT: rax contains self
Raylib.clearBackground:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, 24
    mov     rax, [rax]
    add     rax, [str_field]

    mov     rdi, rax
    call    ClearBackground

    mov     rax, rbx                   ; return self

    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

;
;
; Raylib.drawText
;
;   Draw text (using default font)
;
;   INPUT: rax contains self
;   STACK:
;      - text: String
;      - x: Int
;      - y: Int
;      - font size: Int
;      - color: Color (field1: String with 4 bytes)
;   OUTPUT: rax contains self
Raylib.drawText:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, [str_field]
    mov     rdi, rax

    mov     rax, qword [rbp + arg_1]
    add     rax, [int_slot]
    mov     rsi, [rax]

    mov     rax, qword [rbp + arg_2]
    add     rax, [int_slot]
    mov     rdx, [rax]

    mov     rax, qword [rbp + arg_3]
    add     rax, [int_slot]
    mov     r10, [rax]

    mov     rax, qword [rbp + arg_4]
    add     rax, 24
    mov     rax, [rax]
    add     rax, [str_field]
    mov     r8, rax

    call    DrawText

    mov     rax, rbx

    pop     rbx                        ; restore register
    pop     rbp                        ; restore return address
    ret

