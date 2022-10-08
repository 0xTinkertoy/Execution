//
//  ExecutionContext.hpp
//  Execution
//
//  Created by FireWolf on 2021-3-23.
//

#ifndef Execution_ExecutionContext_hpp
#define Execution_ExecutionContext_hpp

#include <concepts>
#include <Types.hpp>

/// Define the constraint on the execution context to provide system call support
template <typename Context>
concept ExecutionContextProvidesSystemCallSupport = requires(Context& context, int krv)
{
    ///
    /// The execution context must provide read access to the register that stores the system call identifier
    ///
    { context.getSyscallIdentifier() } -> std::same_as<UInt32>;

    ///
    /// The execution context must provide read access to the register that stores the pointer to the system call argument list
    ///
    { context.getSyscallArgumentList() } -> std::same_as<va_list*>;

    ///
    /// The execution context must provide write access to the register that stores the kernel return value
    ///
    { context.setSyscallKernelReturnValue(krv) } -> std::same_as<void>;
};

#endif /* Execution_ExecutionContext_hpp */
