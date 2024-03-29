section '.text' executable

extrn time

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

extrn InitWindow
extrn WindowShouldClose
extrn CloseWindow
extrn BeginDrawing
extrn EndDrawing
extrn ClearBackground
extrn DrawText
extrn SetTargetFPS
extrn DrawRectangle
extrn IsKeyPressed
extrn DrawCircle
extrn SetRandomSeed
extrn GetRandomValue

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
;   OUTPUT: rax contains self
Raylib.initWindow:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     rdi, rax

    mov     rax, qword [rbp + arg_1]
    add     rax, [slot_0]
    mov     rax, [rax]
    mov     rsi, rax

    mov     rax, qword [rbp + arg_2]
    add     rax, [slot_1]
    mov     rdx, rax

    call    InitWindow

    mov     rax, rbx                   ; return self

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
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
    sub     rsp, 24                    ; allocate 3 local variables
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
; Raylib.closeWindow
;
;   Close window and unload OpenGL context
;
;   INPUT: rax contains self
;   STACK: empty
;   OUTPUT: rax contains self
Raylib.closeWindow:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    call    CloseWindow

    mov     rax, rbx                   ; return self

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
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
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    call    BeginDrawing

    mov     rax, rbx                   ; return self

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
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
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    call    EndDrawing

    mov     rax, rbx                   ; return self

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
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
;      - color: Color (field1: Int)
;   OUTPUT: rax contains self
Raylib.clearBackground:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, 24
    mov     rax, [rax]
    add     rax, [slot_0]
    mov     rax, [rax]

    mov     rdi, rax
    call    ClearBackground

    mov     rax, rbx                   ; return self

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
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
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, [slot_1]
    mov     rdi, rax

    mov     rax, qword [rbp + arg_1]
    add     rax, [slot_0]
    mov     rsi, [rax]

    mov     rax, qword [rbp + arg_2]
    add     rax, [slot_0]
    mov     rdx, [rax]

    mov     rax, qword [rbp + arg_3]
    add     rax, [slot_0]
    mov     rcx, [rax]

    mov     rax, qword [rbp + arg_4]
    add     rax, 24
    mov     rax, [rax]
    add     rax, [slot_0]
    mov     r8, [rax]

    call    DrawText

    mov     rax, rbx

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Raylib.setTargetFPS
;
;   Set target FPS (maximum)
;
;   INPUT: rax contains self
;   STACK:
;      - fps: Int
;   OUTPUT: rax contains self
Raylib.setTargetFPS:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, [slot_0]
    mov     rdi, [rax]

    call    SetTargetFPS

    mov     rax, rbx

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
    pop     rbp                        ; restore return address
    ret


;
;
; Raylib.drawRectangle
;
;   Draw a color-filled rectangle
;
;   INPUT: rax contains self
;   STACK:
;      - x: Int
;      - y: Int
;      - width: Int
;      - height: Int
;      - color: Color (field1: Int)
;   OUTPUT: rax contains self
Raylib.drawRectangle:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, [slot_0]
    mov     rdi, [rax]

    mov     rax, qword [rbp + arg_1]
    add     rax, [slot_0]
    mov     rsi, [rax]

    mov     rax, qword [rbp + arg_2]
    add     rax, [slot_0]
    mov     rdx, [rax]

    mov     rax, qword [rbp + arg_3]
    add     rax, [slot_0]
    mov     rcx, [rax]

    mov     rax, qword [rbp + arg_4]
    add     rax, [slot_0]
    mov     rax, [rax]
    add     rax, [slot_0]
    mov     r8, [rax]

    call    DrawRectangle

    mov     rax, rbx

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
    pop     rbp                        ; restore return address
    ret


;
;
; Raylib.isKeyPressed
;
;   Check if a key has been pressed once
;
;   INPUT: rax contains self
;   STACK:
;      - key: Int
;   OUTPUT: rax contains a bool object
Raylib.isKeyPressed:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, [slot_0]
    mov     rdi, [rax]

    ; t1 <- IsKeyPressed(key)
    call    IsKeyPressed
    mov     qword [rbp - loc_1], rax

    ; t0 <- new Bool
    mov     rax, Bool_protObj
    call    Object.copy
    call    Bool_init
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
; Raylib.drawCircle
;
;   Draw a color-filled circle
;
;   INPUT: rax contains self
;   STACK:
;      - centerX: Int
;      - centerY: Int
;      - radius: Float
;      - color: Color (field1: Int)
;   OUTPUT: rax contains self
Raylib.drawCircle:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, [slot_0]
    mov     rdi, [rax]

    mov     rax, qword [rbp + arg_1]
    add     rax, [slot_0]
    mov     rsi, [rax]

    mov     rax, qword [rbp + arg_2]
    add     rax, [slot_0]
    mov     rax, [rax]
    movd    xmm0, eax

    mov     rax, qword [rbp + arg_3]
    add     rax, [slot_0]
    mov     rax, [rax]
    add     rax, [slot_0]
    mov     rdx, [rax]

    call    DrawCircle

    mov     rax, rbx

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Raylib.setRandomSeed
;
;   Set the seed for the random number generator
;
;   INPUT: rax contains self
;   STACK:
;      - key: Int
;   OUTPUT: rax contains self
Raylib.setRandomSeed:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 8                     ; allocate 1 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, [slot_0]
    mov     rdi, [rax]

    ; SetRandomSeed(key)
    call    SetRandomSeed

    mov     rax, rbx

    pop     rbx                        ; restore register
    add     rsp, 8                     ; deallocate local variables
    pop     rbp                        ; restore return address
    ret

;
;
; Raylib.getRandomValue
;
;   Get a random value between min and max (both included)
;
;   INPUT: rax contains self
;   STACK:
;      - min: Int
;      - max: Int
;   OUTPUT: rax contains an Int object
Raylib.getRandomValue:
    push    rbp                        ; save return address
    mov     rbp, rsp                   ; set up stack frame
    sub     rsp, 24                    ; allocate 3 local variables
    push    rbx                        ; save register
    mov     rbx, rax                   ; save self

    mov     rax, qword [rbp + arg_0]
    add     rax, [slot_0]
    mov     rdi, [rax]

    mov     rax, qword [rbp + arg_1]
    add     rax, [slot_0]
    mov     rsi, [rax]

    ; t1 <- GetRandomValue(min, max)
    call    GetRandomValue
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
