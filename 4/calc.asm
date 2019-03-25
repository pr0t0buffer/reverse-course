
name "calc"
;simple calc, which can only sum to numbers. made for emu8086
PUTC    MACRO   char
        PUSH    AX
        MOV     AL, char
        MOV     AH, 0Eh
        INT     10h     
        POP     AX
ENDM   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; read and write first num      
xor ax, ax
lea dx, msg1
mov ah, 09h
int 21h         

xor dx, dx
start_read_first:

    mov ah, 0
    int 16h  
        
    ;enter means we are done
    cmp ah, 0x1c
    je exit_read_first   
    
    ;not a num - ignore
    cmp ah, 2
    jb start_read_first   
      
    ;not a num - ignore
    cmp ah, 0xb
    ja start_read_first                   
    
    ;it's a digit so go on  
    mov ah, 0xe
    int 10h
    ;save value to cx
    mov ah, 0 
    sub al, 30h
    mov cx, ax
    ;mov previous to ax 
    mov ax, dx  
    ;multiply it by 10
    mov bx, 10
    mul bx  
    ;add current to multiplied previous
    mov dx, ax
    add dx, cx
     
    jmp start_read_first  
    
exit_read_first:

    
mov num1, dx

; new line:
putc 0Dh
putc 0Ah
   
;read and write second num    
xor ax, ax
lea dx, msg2
mov ah, 09h    
int 21h 
 
xor dx, dx
start_read_second:

     mov ah, 0
    int 16h  
        
    ;enter means we are done
    cmp ah, 0x1c
    je exit_read_second   
    
    ;not a num - ignore
    cmp ah, 2
    jb start_read_second   
      
    ;not a num - ignore
    cmp ah, 0xb
    ja start_read_second                   
    
    ;it's a digit so go on  
    mov ah, 0xe
    int 10h
    
    mov ah, 0 
    sub al, 30h
    mov cx, ax
     
    mov ax, dx  
    
    mov bx, 10
    mul bx  
    
    mov dx, ax
    add dx, cx
     
    jmp start_read_second  
    
exit_read_second:

mov num2, dx

putc 0Dh
putc 0Ah   

;sum them
lea dx, msg3
mov ah, 09h   
int 21h 

mov ax, num1
add ax, num2  

;divider
mov bx, 0xa
;counter
mov cx, 0
;convert num to str
L:
    mov dx, 0
    div bx
    push dx
    inc cx
    cmp ax, 0
    jne L
;print result 
L1:
pop dx
add dx, 0x30 
mov al, dl
mov ah, 0xe 
int 10h
dec cx
jne L1  
          
putc 0Dh
putc 0Ah 

;exit
lea dx, exit_msg
mov ah, 09h   
int 21h 

mov ah, 0
int 16h

ret

msg1 db 0Dh,0Ah, 0Dh,0Ah, 'Enter first number: $'
msg2 db "Enter second number: $"
msg3 db "Result is: $"
exit_msg db "Input smth to exit$"
; first and second number:
num1 dw ?
num2 dw ?