#pragma once
#include <Windows.h>
#include <tether.hpp>

/**
 * The 32-bit EFLAGS register contains a group of status flags, a control flag,
 * and a group of system flags. The status flags (bits 0, 2, 4, 6, 7, and 11) of
 * the EFLAGS register indicate the results of arithmetic instructions, such as
 * the ADD, SUB, MUL, and DIV instructions. The system flags and IOPL field in
 * the EFLAGS register control operating-system or executive operations.
 *
 * @see Vol1[3.4.3(EFLAGS)] (reference)
 */
typedef union {
  struct {
    /**
     * @brief Carry flag
     *
     * [Bit 0] Set if an arithmetic operation generates a carry or a borrow out
     * of the mostsignificant bit of the result; cleared otherwise. This flag
     * indicates an overflow condition for unsigned-integer arithmetic. It is
     * also used in multiple-precision arithmetic.
     */
    UINT32 CarryFlag : 1;
#define EFLAGS_CARRY_FLAG_BIT 0
#define EFLAGS_CARRY_FLAG_FLAG 0x01
#define EFLAGS_CARRY_FLAG_MASK 0x01
#define EFLAGS_CARRY_FLAG(_) (((_) >> 0) & 0x01)

    /**
     * [Bit 1] Reserved - always 1
     */
    UINT32 ReadAs1 : 1;
#define EFLAGS_READ_AS_1_BIT 1
#define EFLAGS_READ_AS_1_FLAG 0x02
#define EFLAGS_READ_AS_1_MASK 0x01
#define EFLAGS_READ_AS_1(_) (((_) >> 1) & 0x01)

    /**
     * @brief Parity flag
     *
     * [Bit 2] Set if the least-significant byte of the result contains an even
     * number of 1 bits; cleared otherwise.
     */
    UINT32 ParityFlag : 1;
#define EFLAGS_PARITY_FLAG_BIT 2
#define EFLAGS_PARITY_FLAG_FLAG 0x04
#define EFLAGS_PARITY_FLAG_MASK 0x01
#define EFLAGS_PARITY_FLAG(_) (((_) >> 2) & 0x01)
    UINT32 Reserved1 : 1;

    /**
     * @brief Auxiliary Carry flag
     *
     * [Bit 4] Set if an arithmetic operation generates a carry or a borrow out
     * of bit 3 of the result; cleared otherwise. This flag is used in
     * binary-coded decimal (BCD) arithmetic.
     */
    UINT32 AuxiliaryCarryFlag : 1;
#define EFLAGS_AUXILIARY_CARRY_FLAG_BIT 4
#define EFLAGS_AUXILIARY_CARRY_FLAG_FLAG 0x10
#define EFLAGS_AUXILIARY_CARRY_FLAG_MASK 0x01
#define EFLAGS_AUXILIARY_CARRY_FLAG(_) (((_) >> 4) & 0x01)
    UINT32 Reserved2 : 1;

    /**
     * @brief Zero flag
     *
     * [Bit 6] Set if the result is zero; cleared otherwise.
     */
    UINT32 ZeroFlag : 1;
#define EFLAGS_ZERO_FLAG_BIT 6
#define EFLAGS_ZERO_FLAG_FLAG 0x40
#define EFLAGS_ZERO_FLAG_MASK 0x01
#define EFLAGS_ZERO_FLAG(_) (((_) >> 6) & 0x01)

    /**
     * @brief Sign flag
     *
     * [Bit 7] Set equal to the most-significant bit of the result, which is the
     * sign bit of a signed integer. (0 indicates a positive value and 1
     * indicates a negative value.)
     */
    UINT32 SignFlag : 1;
#define EFLAGS_SIGN_FLAG_BIT 7
#define EFLAGS_SIGN_FLAG_FLAG 0x80
#define EFLAGS_SIGN_FLAG_MASK 0x01
#define EFLAGS_SIGN_FLAG(_) (((_) >> 7) & 0x01)

    /**
     * @brief Trap flag
     *
     * [Bit 8] Set to enable single-step mode for debugging; clear to disable
     * single-step mode.
     */
    UINT32 TrapFlag : 1;
#define EFLAGS_TRAP_FLAG_BIT 8
#define EFLAGS_TRAP_FLAG_FLAG 0x100
#define EFLAGS_TRAP_FLAG_MASK 0x01
#define EFLAGS_TRAP_FLAG(_) (((_) >> 8) & 0x01)

    /**
     * @brief Interrupt enable flag
     *
     * [Bit 9] Controls the response of the processor to maskable interrupt
     * requests. Set to respond to maskable interrupts; cleared to inhibit
     * maskable interrupts.
     */
    UINT32 InterruptEnableFlag : 1;
#define EFLAGS_INTERRUPT_ENABLE_FLAG_BIT 9
#define EFLAGS_INTERRUPT_ENABLE_FLAG_FLAG 0x200
#define EFLAGS_INTERRUPT_ENABLE_FLAG_MASK 0x01
#define EFLAGS_INTERRUPT_ENABLE_FLAG(_) (((_) >> 9) & 0x01)

    /**
     * @brief Direction flag
     *
     * [Bit 10] Controls string instructions (MOVS, CMPS, SCAS, LODS, and STOS).
     * Setting the DF flag causes the string instructions to auto-decrement (to
     * process strings from high addresses to low addresses). Clearing the DF
     * flag causes the string instructions to auto-increment (process strings
     * from low addresses to high addresses).
     */
    UINT32 DirectionFlag : 1;
#define EFLAGS_DIRECTION_FLAG_BIT 10
#define EFLAGS_DIRECTION_FLAG_FLAG 0x400
#define EFLAGS_DIRECTION_FLAG_MASK 0x01
#define EFLAGS_DIRECTION_FLAG(_) (((_) >> 10) & 0x01)

    /**
     * @brief Overflow flag
     *
     * [Bit 11] Set if the integer result is too large a positive number or too
     * small a negative number (excluding the sign-bit) to fit in the
     * destination operand; cleared otherwise. This flag indicates an overflow
     * condition for signed-integer (two's complement) arithmetic.
     */
    UINT32 OverflowFlag : 1;
#define EFLAGS_OVERFLOW_FLAG_BIT 11
#define EFLAGS_OVERFLOW_FLAG_FLAG 0x800
#define EFLAGS_OVERFLOW_FLAG_MASK 0x01
#define EFLAGS_OVERFLOW_FLAG(_) (((_) >> 11) & 0x01)

    /**
     * @brief I/O privilege level field
     *
     * [Bits 13:12] Indicates the I/O privilege level of the currently running
     * program or task. The current privilege level (CPL) of the currently
     * running program or task must be less than or equal to the I/O privilege
     * level to access the I/O address space. The POPF and IRET instructions can
     * modify this field only when operating at a CPL of 0.
     */
    UINT32 IoPrivilegeLevel : 2;
#define EFLAGS_IO_PRIVILEGE_LEVEL_BIT 12
#define EFLAGS_IO_PRIVILEGE_LEVEL_FLAG 0x3000
#define EFLAGS_IO_PRIVILEGE_LEVEL_MASK 0x03
#define EFLAGS_IO_PRIVILEGE_LEVEL(_) (((_) >> 12) & 0x03)

    /**
     * @brief Nested task flag
     *
     * [Bit 14] Controls the chaining of interrupted and called tasks. Set when
     * the current task is linked to the previously executed task; cleared when
     * the current task is not linked to another task.
     */
    UINT32 NestedTaskFlag : 1;
#define EFLAGS_NESTED_TASK_FLAG_BIT 14
#define EFLAGS_NESTED_TASK_FLAG_FLAG 0x4000
#define EFLAGS_NESTED_TASK_FLAG_MASK 0x01
#define EFLAGS_NESTED_TASK_FLAG(_) (((_) >> 14) & 0x01)
    UINT32 Reserved3 : 1;

    /**
     * @brief Resume flag
     *
     * [Bit 16] Controls the processor's response to debug exceptions.
     */
    UINT32 ResumeFlag : 1;
#define EFLAGS_RESUME_FLAG_BIT 16
#define EFLAGS_RESUME_FLAG_FLAG 0x10000
#define EFLAGS_RESUME_FLAG_MASK 0x01
#define EFLAGS_RESUME_FLAG(_) (((_) >> 16) & 0x01)

    /**
     * @brief Virtual-8086 mode flag
     *
     * [Bit 17] Set to enable virtual-8086 mode; clear to return to protected
     * mode without virtual-8086 mode semantics.
     */
    UINT32 Virtual8086ModeFlag : 1;
#define EFLAGS_VIRTUAL_8086_MODE_FLAG_BIT 17
#define EFLAGS_VIRTUAL_8086_MODE_FLAG_FLAG 0x20000
#define EFLAGS_VIRTUAL_8086_MODE_FLAG_MASK 0x01
#define EFLAGS_VIRTUAL_8086_MODE_FLAG(_) (((_) >> 17) & 0x01)

    /**
     * @brief Alignment check (or access control) flag
     *
     * [Bit 18] If the AM bit is set in the CR0 register, alignment checking of
     * user-mode data accesses is enabled if and only if this flag is 1. If the
     * SMAP bit is set in the CR4 register, explicit supervisor-mode data
     * accesses to user-mode pages are allowed if and only if this bit is 1.
     *
     * @see Vol3A[4.6(ACCESS RIGHTS)]
     */
    UINT32 AlignmentCheckFlag : 1;
#define EFLAGS_ALIGNMENT_CHECK_FLAG_BIT 18
#define EFLAGS_ALIGNMENT_CHECK_FLAG_FLAG 0x40000
#define EFLAGS_ALIGNMENT_CHECK_FLAG_MASK 0x01
#define EFLAGS_ALIGNMENT_CHECK_FLAG(_) (((_) >> 18) & 0x01)

    /**
     * @brief Virtual interrupt flag
     *
     * [Bit 19] Virtual image of the IF flag. Used in conjunction with the VIP
     * flag. (To use this flag and the VIP flag the virtual mode extensions are
     * enabled by setting the VME flag in control register CR4.)
     */
    UINT32 VirtualInterruptFlag : 1;
#define EFLAGS_VIRTUAL_INTERRUPT_FLAG_BIT 19
#define EFLAGS_VIRTUAL_INTERRUPT_FLAG_FLAG 0x80000
#define EFLAGS_VIRTUAL_INTERRUPT_FLAG_MASK 0x01
#define EFLAGS_VIRTUAL_INTERRUPT_FLAG(_) (((_) >> 19) & 0x01)

    /**
     * @brief Virtual interrupt pending flag
     *
     * [Bit 20] Set to indicate that an interrupt is pending; clear when no
     * interrupt is pending. (Software sets and clears this flag; the processor
     * only reads it.) Used in conjunction with the VIF flag.
     */
    UINT32 VirtualInterruptPendingFlag : 1;
#define EFLAGS_VIRTUAL_INTERRUPT_PENDING_FLAG_BIT 20
#define EFLAGS_VIRTUAL_INTERRUPT_PENDING_FLAG_FLAG 0x100000
#define EFLAGS_VIRTUAL_INTERRUPT_PENDING_FLAG_MASK 0x01
#define EFLAGS_VIRTUAL_INTERRUPT_PENDING_FLAG(_) (((_) >> 20) & 0x01)

    /**
     * @brief Identification flag
     *
     * [Bit 21] The ability of a program to set or clear this flag indicates
     * support for the CPUID instruction.
     */
    UINT32 IdentificationFlag : 1;
#define EFLAGS_IDENTIFICATION_FLAG_BIT 21
#define EFLAGS_IDENTIFICATION_FLAG_FLAG 0x200000
#define EFLAGS_IDENTIFICATION_FLAG_MASK 0x01
#define EFLAGS_IDENTIFICATION_FLAG(_) (((_) >> 21) & 0x01)
    UINT32 Reserved4 : 10;
  };

  UINT32 AsUInt;
} EFLAGS;

// Probably only need to do this once. When looking for code to optimize
// consider executing this only once and caching the system eflags result.
__declspec(naked) EFLAGS __get_system_flags() {
  __asm {
	PUSHFQ
	MOV EAX, [RSP]
	ADD RSP, 8  // Faster than POPFQ
	RET
  }
}

/// <summary>
/// This sanitizes the supplied eflags so that only the status flags are copied from the client.
/// 
/// This returns the original clients EFLAGS value.
/// </summary>
/// <param name="pckt"></param>
/// <returns></returns>
__forceinline EFLAGS sanitize_eflags(__tether_registers* pckt) {
  auto eflags = reinterpret_cast<EFLAGS*>(&pckt->rflags);
  EFLAGS result = *eflags;
  auto current_system_flags = __get_system_flags();
  current_system_flags.SignFlag = eflags->SignFlag;
  current_system_flags.OverflowFlag = eflags->OverflowFlag;
  current_system_flags.ParityFlag = eflags->ParityFlag;
  current_system_flags.ZeroFlag = eflags->ZeroFlag;
  current_system_flags.CarryFlag = eflags->CarryFlag;
  *reinterpret_cast<EFLAGS*>(&pckt->rflags) = current_system_flags;
  return result;
}