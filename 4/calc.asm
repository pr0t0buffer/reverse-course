
name "calc"
;simple calc, which can only sum to numbers. made for emu8086
; clear screen and set cursor   
mov al, 02h
mov ah, 00h
int 10h

mov bh, 0
mov dx, 0
mov ah, 02h
int 10h  

; read and write first num  
mov bp,offset msg1
mov ax,1301h
mov bx, 0fh
mov cx,len1
mov dh, 0
mov dl, 0
int 10h        

xor ax, ax
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
   
;read and write second num    
mov bp,offset msg2
mov ax,1301h
mov bx, 0fh
mov cx,len2 
mov dh, 3
mov dl, 0
int 10h 
 
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

;sum them
mov bp,offset msg3
mov ax,1301h
mov bx, 0fh
mov cx,len3 
mov dh, 4
mov dl, 0
int 10h 

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

;exit
mov bp,offset exit_msg
mov ax,1301h
mov bx, 0fh
mov cx,len4 
mov dh, 5
mov dl, 0
int 10h 

mov ah, 0
int 16h

ret

msg1 db 0Dh,0Ah, 0Dh,0Ah, 'Enter first number: $'
len1 dw 18h
msg2 db "Enter second number: $"
len2 dw 15h
msg3 db "Result is: $"
len3 dw 0Bh
exit_msg db "Input smth to exit$"
len4 dw 12h
; first and second number:
num1 dw ?
num2 dw ?