#pragma once
#include <tether.hpp>

// This will only compile for clang/maybe gcc compiler.
// Im using LLVM-MSVC to create this project, maybe this will become a const array
// of bytes in the future so that we dont have compiler issues.
extern "C" __declspec(naked) void __execute_tether_region(
    unsigned token,
    __tether_registers* pckt,
    fn_tether_region* handlers) {
  __asm {
    .seh_proc FUNC
    FUNC:
        PUSHFQ
        PUSH RDX

        ; Put the address of the tether handler on top of the stack
        MOV RCX, [R8+RCX*8]
        PUSH RCX

        ; Swap GPR registers
        MOV RAX, RDX
        XCHG R15, [RAX+0x00]
        XCHG R14, [RAX+0x08]
        XCHG R13, [RAX+0x10]
        XCHG R12, [RAX+0x18]
        XCHG R11, [RAX+0x20]
        XCHG R10, [RAX+0x28]
        XCHG R9,  [RAX+0x30]
        XCHG R8,  [RAX+0x38]
        XCHG RDI, [RAX+0x40]
        XCHG RSI, [RAX+0x48]
        XCHG RBP, [RAX+0x50]
        XCHG RBX, [RAX+0x58]
        XCHG RDX, [RAX+0x60]
        XCHG RCX, [RAX+0x68]

        ; Load flags
        PUSH [RAX+0x78]
        POPFQ

        XCHG RAX, [RAX+0x70]

        ; Top of the stack is the tether handler
        CALL [RSP]

        ; Save RAX on the stack and exchange it for the cpu context ptr
        XCHG RAX, [RSP+0x8]

        XCHG [RAX+0x00], R15
        XCHG [RAX+0x08], R14
        XCHG [RAX+0x10], R13
        XCHG [RAX+0x18], R12
        XCHG [RAX+0x20], R11
        XCHG [RAX+0x28], R10
        XCHG [RAX+0x30], R9
        XCHG [RAX+0x38], R8
        XCHG [RAX+0x40], RDI
        XCHG [RAX+0x48], RSI
        XCHG [RAX+0x50], RBP
        XCHG [RAX+0x58], RBX
        XCHG [RAX+0x60], RDX
        XCHG [RAX+0x68], RCX

        ; Save RAX now.
        MOV RCX, [RSP+0x8]
        MOV [RAX+0x70], RCX

        ; Save flags from tether handler
        PUSHFQ
        POP QWORD PTR [RAX+0x78]

        POP RCX
        POP RDX
        POPFQ
        RET
    .seh_endproc
  }
}