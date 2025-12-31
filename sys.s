.intel_syntax noprefix

.global _start
.global write
.global read
.global fork
.global execve
.global _exit
.global environ
.global wait
.global chdir
.global pipe
.global dup2
.global close

.text

write:
    mov rax, 1
    syscall
    ret

read:
    mov rax, 0
    syscall
    ret

fork:
    mov rax, 57
    syscall
    ret

execve:
    mov rax, 59
    syscall
    ret

_exit:
    mov rax, 60
    syscall
    ret

wait:
    mov rax, 61
    mov rdi, -1
    mov rsi, 0
    mov rdx, 0
    mov r10, 0
    syscall
    ret

chdir:
    mov rax, 80
    syscall
    ret

pipe:
    mov rax, 22
    syscall
    ret

dup2:
    mov rax, 33
    syscall
    ret

close:
    mov rax, 3
    syscall
    ret

_start:
    pop rdi
    mov rsi, rsp
    lea rdx, [rsp + rdi * 8 + 8]
    mov [environ], rdx
    call main
    mov rdi, rax
    call _exit

.data
environ:
    .quad 0
