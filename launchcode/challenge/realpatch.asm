bits 32

; eax = current character
mov eax, [ebp+8]
pushad

call get_values
  ; This is the expected code
  db 'XXXXXXXX'

  ; Get a handle to our "checker"
  get_values:
    pop esi

  ; Get the "index"
  mov ecx, [0x0101400c]

  ; Go to the "index"
  mov edx, esi
  add edx, ecx

  ; Check if we're good
  cmp byte [edx], al

  jz its_good

  ; If it's wrong, reset the count
  mov dword [0x0101400c], 0
  popad
  ret

its_good:
  ; If it's right, increment the index
  inc ecx
  mov [0x0101400c], ecx

  ; Check if we're done
  cmp ecx, 0x08
  jl notdone

  ; If we ARE done, decode the actual flag
  ; esi = already the decoder string
  ; edi = the text
  mov edi, 0x010140c0
  xor ecx, ecx

; A little decoder loop
top:
  mov al, [esi+ecx] ; al = this byte of the decoder string

  mov edx, ecx ; Multiply ecx by 4, so we can index into the encoded key
  shl edx, 2

  xor [edi+edx+0], al ; First byte
  xor [edi+edx+1], al ; Second byte
  xor [edi+edx+2], al ; Third byte
  xor [edi+edx+3], al ; First byte

  inc ecx
  cmp ecx, 0x08
  jl top

  ; Put it in a messagebox!
  push 0
  push 0x010140c0
  push 0x010140c0
  push 0
  call [0x010011a8]

notdone:
  popad
  ret
ret
