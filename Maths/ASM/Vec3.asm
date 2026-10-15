; Windows x64 calling convention
; Args: rcx, rdx, r8, r9 (puis stack)
.code
; void ASM_DotProductBatch_AOS(const float* a, const float* b, float* result, size_t count)
;   rcx = a       (tableau de Vec3f : x,y,z contigus)
;   rdx = b
;   r8  = result
;   r9  = count
ASM_DotProductBatch_AOS PROC
    test r9, r9
    jz done

    loop_start:
    movss xmm0 , dword ptr [rcx]    ; load a.x
    movss xmm1 , dword ptr [rcx+4]    ; load a.y
    movss xmm2 , dword ptr [rcx+8]    ; load a.z
    unpcklps xmm0, xmm1                 ; xmm0 = [ax, ay, 0, 0]
    unpcklps xmm0, xmm2                 ; xmm0 = [ax, ay, az, 0]


    movss xmm1 , dword ptr [rdx]    ; load b.x
    movss xmm2 , dword ptr [rdx+4]    ; load b.y
    movss xmm3 , dword ptr [rdx+8]    ; load b.z
    unpcklps xmm1, xmm2                 ; xmm1 = [bx, by, 0, 0]
    unpcklps xmm1, xmm3                 ; xmm1 = [bx, by, bz, 0]

    mulps xmm0 , xmm1    ; multiply a and b component-wise

    movaps xmm1 , xmm0

    shufps xmm1, xmm1, 4Eh  ; shuffle to get y and z components
    addss xmm0 , xmm1    ; a.y * b.y + a.z * b.z

    movaps xmm1 , xmm0
    shufps xmm1, xmm1, 11h  ; shuffle to get x component
    addss xmm0 , xmm1    ; a.x * b.x + a.y * b.y + a.z * b.z

    movss  dword ptr [r8], xmm0    ; store result

    add rcx, 12
    add rdx, 12
    add r8, 4
    dec r9
    jnz loop_start

    done:

    ret

ASM_DotProductBatch_AOS ENDP
END
