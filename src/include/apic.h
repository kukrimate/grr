#ifndef _SPEC_APIC_H
#define _SPEC_APIC_H

/*
 * Local APIC registers (Intel SDM Vol 3 11.4.1)
 */

#define LAPIC_ID                      0x0020  // Local APIC ID
#define LAPIC_VER                     0x0030  // Local APIC Version
#define LAPIC_TPR                     0x0080  // Task Priority
#define LAPIC_APR                     0x0090  // Arbitration Priority
#define LAPIC_PPR                     0x00a0  // Processor Priority
#define LAPIC_EOI                     0x00b0  // EOI
#define LAPIC_RRD                     0x00c0  // Remote Read
#define LAPIC_LDR                     0x00d0  // Logical Destination
#define LAPIC_DFR                     0x00e0  // Destination Format
#define LAPIC_SVR                     0x00f0  // Spurious Interrupt Vector
#define LAPIC_ISR                     0x0100  // In-Service (8 registers)
#define LAPIC_TMR                     0x0180  // Trigger Mode (8 registers)
#define LAPIC_IRR                     0x0200  // Interrupt Request (8 registers)
#define LAPIC_ESR                     0x0280  // Error Status
#define LAPIC_CMCI                    0x02f0  // LVT Corrected Machine Check Interrupt
#define LAPIC_ICRLO                   0x0300  // Interrupt Command (bits 0-31)
#define LAPIC_ICRHI                   0x0310  // Interrupt Command (bits 32-63)
#define LAPIC_TIMER                   0x0320  // LVT Timer
#define LAPIC_THERMAL                 0x0330  // LVT Thermal Sensor
#define LAPIC_PERF                    0x0340  // LVT Performance Counter
#define LAPIC_LINT0                   0x0350  // LVT LINT0
#define LAPIC_LINT1                   0x0360  // LVT LINT1
#define LAPIC_ERROR                   0x0370  // LVT Error
#define LAPIC_TICR                    0x0380  // Initial Count (for Timer)
#define LAPIC_TCCR                    0x0390  // Current Count (for Timer)
#define LAPIC_TDCR                    0x03e0  // Divide Configuration (for Timer)

#define LAPIC_SIZE                    0x03ff  // LAPIC register block size

/*
 * Local APIC base address (Intel SDM Vol 3 11.4.3)
 */

#define IA32_APIC_BASE                0x001b

#define APIC_BSP_FLAG                 0x000000100
#define APIC_GLOBAL_ENABLE            0x000000800
#define APIC_BASE_MASK                0xffffff000

/*
 * Local APIC ID (Intel SDM Vol 3 11.4.6)
 */

#define LAPIC_ID_OFFSET               24

/*
 * Local APIC Version (Intel SDM Vol 3 11.4.8)
 */

#define LAPIC_VER_VER                 0x000000ff
#define LAPIC_VER_MAX_LVT             0x00ff0000
#define LAPIC_VER_EOI_SUPPRESS        0x01000000

/*
 * EOI register
 */

#define LAPIC_EOI_VALUE               0x00000000

/*
 * Spurious Vector Register (Intel SDM Vol 3 11.9)
 */

#define LAPIC_SVR_VECTOR_MASK         0x000000ff
#define LAPIC_SVR_SOFTWARE_ENABLE     0x00000100
#define LAPIC_SVR_FOCUS_PROC_CHECK    0x00000200
#define LAPIC_SVR_EOI_SUPPRESS        0x00001000

/*
 * Local Vector Table (Intel SDM Vol 3 11.5.1)
 */

#define LAPIC_LVT_VECTOR_MASK         0x000000ff

// Delivery Mode
#define LAPIC_LVT_FIXED               0x00000000
#define LAPIC_LVT_SMI                 0x00000200
#define LAPIC_LVT_NMI                 0x00000400
#define LAPIC_LVT_EXTINT              0x00000700
#define LAPIC_LVT_INIT                0x00000500

// Delivery Status
#define LAPIC_LVT_IDLE                0x00000000
#define LAPIC_LVT_SEND_PENDING        0x00001000

// Interrupt Input Pin Polarity
#define LAPIC_LVT_HIGH                0x00000000
#define LAPIC_LVT_LOW                 0x00002000

// Remote IRR
#define LAPIC_LVT_REMOTE_IRR          0x00004000

// Trigger Mode
#define LAPIC_LVT_EDGE                0x00000000
#define LAPIC_LVT_LEVEL               0x00008000

// Mask
#define LAPIC_LVT_NOT_MASKED          0x00000000
#define LAPIC_LVT_MASKED              0x00010000

// Timer mode
#define LAPIC_LVT_ONESHOT             0x00000000
#define LAPIC_LVT_PERIODIC            0x00020000
#define LAPIC_LVT_TSC_DEADLINE        0x00060000

/*
 * Error Status Register (Intel SDM Vol 3 11.5.3)
 */

#define LAPIC_ESR_SEND_CHECKSUM       (1<<0)
#define LAPIC_ESR_RECV_CHECKSUM       (1<<1)
#define LAPIC_ESR_SEND_ACCEPT         (1<<2)
#define LAPIC_ESR_RECV_ACCEPT         (1<<3)
#define LAPIC_ESR_REDIR_IPI           (1<<4)
#define LAPIC_ESR_SEND_ILL_VECTOR     (1<<5)
#define LAPIC_ESR_RECV_ILL_VECTOR     (1<<6)
#define LAPIC_ESR_ILL_REGISTER        (1<<7)

/*
 * Interrupt Command Register (Intel SDM Vol 3 11.6.1)
 */

#define LAPIC_ICR_VECTOR              0x000000ff

// Delivery Mode
#define LAPIC_ICR_FIXED               0x00000000
#define LAPIC_ICR_LOWEST              0x00000100
#define LAPIC_ICR_SMI                 0x00000200
#define LAPIC_ICR_NMI                 0x00000400
#define LAPIC_ICR_INIT                0x00000500
#define LAPIC_ICR_STARTUP             0x00000600

// Destination Mode
#define LAPIC_ICR_PHYSICAL            0x00000000
#define LAPIC_ICR_LOGICAL             0x00000800

// Delivery Status
#define LAPIC_ICR_IDLE                0x00000000
#define LAPIC_ICR_SEND_PENDING        0x00001000

// Level
#define LAPIC_ICR_DEASSERT            0x00000000
#define LAPIC_ICR_ASSERT              0x00004000

// Trigger Mode
#define LAPIC_ICR_EDGE                0x00000000
#define LAPIC_ICR_LEVEL               0x00008000

// Destination Shorthand
#define LAPIC_ICR_NO_SHORTHAND        0x00000000
#define LAPIC_ICR_SELF                0x00040000
#define LAPIC_ICR_ALL_INCLUDING_SELF  0x00080000
#define LAPIC_ICR_ALL_EXCLUDING_SELF  0x000c0000

/*
 * Local APIC Timer (Intel SDM VOl 3 11.5.4)
 */

#define LAPIC_TIMER_DIV2              0
#define LAPIC_TIMER_DIV4              1
#define LAPIC_TIMER_DIV8              2
#define LAPIC_TIMER_DIV16             3
#define LAPIC_TIMER_DIV32             4
#define LAPIC_TIMER_DIV64             5
#define LAPIC_TIMER_DIV128            6
#define LAPIC_TIMER_DIV1              7

#endif
