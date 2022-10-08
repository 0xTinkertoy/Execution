//
//  TaskConstraints.hpp
//  Execution
//
//  Created by FireWolf on 2021-2-5.
//

#ifndef Execution_TaskConstraints_hpp
#define Execution_TaskConstraints_hpp

#include <concepts>
#include <Types.hpp>
#include <Scheduler/Constraint/Prioritizable.hpp>

/// Defines constraints on the task control block
namespace TaskConstraints
{
    ///
    /// Define the constraint on the task control block
    ///
    /// @note A task control block exposes a public function to retrieve each system call argument sequentially.
    ///       i.e. The provider function is stateful and may work with `va_list` behind the scene.
    ///       The first invocation returns the 1st argument, while the second invocation returns the second one.
    ///
    template <typename Task>
    concept TaskProvidesSequentialSyscallArgumentsAccess = requires(Task& task)
    {
        ///
        /// Signature:
        /// template <typename T>
        /// T getSyscallArgument();
        ///
        { task.template getSyscallArgument<int>() } -> std::same_as<int>;
    };

    ///
    /// Define the constraint on the task control block
    ///
    /// @note A task that can invoke system call.
    ///
    template <typename Task>
    concept TaskCanInvokeSystemCall = requires(Task& task, int kernelReturnValue)
    {
        ///
        /// The kernel can retrieve the system call arguments sequentially
        ///
        requires TaskProvidesSequentialSyscallArgumentsAccess<Task>;

        ///
        /// The kernel can set the kernel return value properly
        ///
        { task.setSyscallKernelReturnValue(kernelReturnValue) } -> std::same_as<void>;
    };

    ///
    /// Define the constraint on the task control block
    ///
    /// @note A task control block provides read access to its stack pointer.
    ///
    template <typename Task>
    concept TaskProvidesStackPointerReadAccess = requires(Task& task)
    {
        ///
        /// Task control block provides the getter of the current stack pointer (i.e. the top of the stack)
        ///
        { task.getStackPointer() } -> std::same_as<UInt8*>;
    };

    ///
    /// Define the constraint on the task control block
    ///
    /// @note A task control block provides write access to its stack pointer.
    ///
    template <typename Task>
    concept TaskProvidesStackPointerWriteAccess = requires(Task& task, UInt8* newStackPointer)
    {
        ///
        /// Task control block provides the setter of the current stack pointer (i.e. set the new top of the stack)
        ///
        { task.setStackPointer(newStackPointer) } -> std::same_as<void>;
    };

    ///
    /// Define the constraint on the task control block
    ///
    /// @note A task has its own stack but the kernel does not care about reclaiming the stack memory back
    /// @example The kernel has a prior knowledge that each new tasks will never terminate.
    ///
    template <typename Task>
    concept TaskHasDedicatedStack = requires(Task& task, UInt8* stack)
    {
        ///
        /// Task control block provides R/W access to its current stack pointer
        ///
        requires TaskProvidesStackPointerReadAccess<Task>;
        requires TaskProvidesStackPointerWriteAccess<Task>;
    };

    ///
    /// Define the constraint on the task control block
    ///
    /// @note A task has its own recyclable stack and thus its control block stores the start address of the stack.
    /// @example The kernel needs to retrieve the start address of the stack memory to release it if necessary.
    ///
    template <typename Task>
    concept TaskHasDedicatedRecyclableStack = requires(Task& task, UInt8* stack)
    {
        ///
        /// Task control block provides R/W access to its current stack pointer
        ///
        requires TaskHasDedicatedStack<Task>;

        ///
        /// Task control block provides R/W access to the start address of its private stack
        ///
        { task.getPrivateStack() } -> std::same_as<UInt8*>;
        { task.setPrivateStack(stack) } -> std::same_as<void>;
    };

    ///
    /// Define the constraint on the task control block
    ///
    /// @note A task has a numeric unique identifier, e.g. task id, thread id, process id.
    ///
    template <typename Task>
    concept TaskHasUniqueIdentifier = requires(Task& task, UInt32 identifier)
    {
        ///
        /// Task control block provides R/W access to its identifier
        ///
        /// @note The task control block may choose any unsigned integral types to store its identifier internally.
        ///       e.g. Use 4 bits if there are less than 8 tasks that can coexist on the system.
        ///
        { task.getUniqueIdentifier() } -> std::unsigned_integral;
        { task.setUniqueIdentifier(identifier) } -> std::same_as<void>;
    };

    ///
    /// Define the constraint on the task control block
    ///
    /// @note A task is prioritizable by priority.
    ///
    template <typename Task>
    concept TaskIsPrioritizableByPriority = PrioritizableByMutablePriority<Task>;
}

#endif /* Execution_TaskConstraints_hpp */
