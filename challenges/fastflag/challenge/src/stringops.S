.intel_syntax noprefix

.global strlen_fast
.global strcmp_fast

.equ EQUAL_EACH, 0b1000
.equ NEGATIVE_POLARITY, 0b010000

.text

strlen_fast:
  # rdi = string
  mov rax, -16
  mov rdx, rdi
  pxor xmm0, xmm0

  STRLEN_LOOP:
  add rax, 16
  pcmpistri xmm0, [rdx + rax], EQUAL_EACH
  jnz STRLEN_LOOP

  add rax, rcx
  ret

strcmp_fast:
  # rdi = string1 (ecx)
  # rsi = string2 (edx)
  mov rax, rdi
  sub rax, rsi # distance between
  sub rsi, 16

  STRCMP_LOOP:
  add rsi, 16
  movdqu xmm0, [rsi]
  # find the first *different* bytes, hence negative polarity
  pcmpistri xmm0, [rsi + rax], EQUAL_EACH + NEGATIVE_POLARITY
  ja STRCMP_LOOP

  jc STRCMP_DIFF

  # the strings are equal
  xor rax, rax
  ret

  STRCMP_DIFF:
  # subtract the first different bytes
  add rax, rsi
  movzx rax, byte ptr[rax + rcx]
  movzx rdx, byte ptr[rsi + rcx]
  sub rax, rdx
  sar rax, 63  # find sign bit
  or rax, 1
  inc rcx
  mul rcx
  ret
