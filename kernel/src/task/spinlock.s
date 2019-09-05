[bits 32]
[section .text]

[global acquire_spinlock]
acquire_spinlock:
  push ebp
  mov ebp, esp

  lock bts word [ebp + 8], 0
  jnc .acquired

  ; Function
.retry:
  pause

  bt word [ebp + 8], 0
  jc .retry

  lock bts word [ebp + 8], 0
  jc .retry

.acquired:
  mov esp, ebp
  pop ebp
  ret

[global release_spinlock]
release_spinlock:
  push ebp
  mov ebp, esp

  ; Function
  lock btr word [ebp + 8], 0
  jc .wasLocked
  jnc .wasUnlocked
.wasLocked:
  mov eax, 0
  jmp .return

.wasUnlocked:
  mov eax, 1
  jmp .return

.return:
  mov esp, ebp
  pop ebp
  ret
