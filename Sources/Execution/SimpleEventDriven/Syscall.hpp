//
//  Syscall.hpp
//  Execution
//
//  Created by FireWolf on 2021-2-5.
//

#ifndef Execution_SimpleEventDrivenSyscall_hpp
#define Execution_SimpleEventDrivenSyscall_hpp

#include <Types.hpp>

// The kernel must implement the following system calls

///
/// [SYSCALL] Set the handler for an event
///
/// @param event The event number
/// @param handler The new event handler
///
void sysSetEventHandler(int event, void(*handler)());

///
/// [SYSCALL] Send an event
///
/// @param event The event number
///
void sysSendEvent(int event);

///
/// [SYSCALL] Return from the event handler
///
/// @param oldStack The old stack pointer
/// @note This is a private system call used by the trampoline function.
///
void sysEventHandlerReturn(UInt8* oldStack);

#endif /* Execution_SimpleEventDrivenSyscall_hpp */
