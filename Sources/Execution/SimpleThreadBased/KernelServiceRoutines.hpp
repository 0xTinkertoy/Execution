//
//  KernelServiceRoutines.hpp
//  Execution
//
//  Created by FireWolf on 2021-3-4.
//

#ifndef Execution_SimpleThreadBasedKernelServiceRoutines_hpp
#define Execution_SimpleThreadBasedKernelServiceRoutines_hpp

#include <Scheduler/Scheduler.hpp>
#include <Execution/Common/TaskConstraints.hpp>
#include <Execution/Common/KernelServiceRoutines.hpp>

/// Defines kernel service routines for the simple thread based execution model
namespace KernelServiceRoutines::CreateThread
{
    /// Private subroutine to create and initialize a task control block
    namespace KPI
    {
        ///
        /// [KPI] Private subroutine to allocate a dedicated stack for a task dynamically
        ///
        /// @tparam Task Specify the type of a task that has a dedicated stack
        /// @note This subroutine also adjusts task's stack pointer to the bottom of allocated stack.
        /// @note The kernel does not care about reclaiming the stack memory back.
        /// @example The kernel has a prior knowledge that each new tasks will never terminate.
        /// @seealso `AssignStack` if the kernel prefers to assign a pre-allocated stack for the task.
        ///
        template <typename Task>
        requires TaskConstraints::TaskHasDedicatedStack<Task>
        struct AllocateDedicatedStack
        {
            /// Define the argument type
            using Arg = size_t;

            ///
            /// Allocate a private stack to the given task dynamically
            ///
            /// @param task A non-null task control block
            /// @param stackSize The stack size
            /// @return `true` on success, `false` if the kernel runs out of memory.
            ///
            bool operator()(Task* task, size_t stackSize)
            {
                auto* stack = new UInt8[stackSize];

                if (stack == nullptr)
                {
                    return false;
                }

                pinfo("Allocated stack starts at 0x%p, length = %d bytes.", stack, stackSize);

                task->setStackPointer(stack + stackSize);

                return true;
            }
        };

        ///
        /// [KPI] Private subroutine to allocate a dedicated recyclable stack for a task dynamically
        ///
        /// @tparam Task Specify the type of a task that has a dedicated stack
        /// @note This subroutine also adjusts task's stack pointer to the bottom of allocated stack.
        /// @note The kernel is responsible for managing the memory of the stack.
        ///       Developers must include `ReleaseDedicatedStack` when building the finalizer for the `FinishThread` service routine.
        /// @seealso `AssignStack` if the kernel prefers to assign a pre-allocated stack for the task.
        ///
        template <typename Task>
        requires TaskConstraints::TaskHasDedicatedRecyclableStack<Task>
        struct AllocateDedicatedRecyclableStack
        {
            /// Define the argument type
            using Arg = size_t;

            ///
            /// Allocate a private stack to the given task dynamically
            ///
            /// @param task A non-null task control block
            /// @param stackSize The stack size
            /// @return `true` on success, `false` if the kernel runs out of memory.
            ///
            bool operator()(Task* task, size_t stackSize)
            {
                auto* stack = new UInt8[stackSize];

                if (stack == nullptr)
                {
                    return false;
                }

                pinfo("Allocated stack starts at 0x%p, length = %d bytes.", stack, stackSize);

                task->setPrivateStack(stack);

                task->setStackPointer(stack + stackSize);

                return true;
            }
        };

        ///
        /// [KPI] Private subroutine to assign a pre-allocated stack to a task
        ///
        /// @tparam Task Specify the type of a task that has a dedicated stack
        /// @note This subroutine also adjusts task's stack pointer to the bottom of allocated stack.
        /// @note The kernel is NOT responsible for managing the memory of the stack.
        /// @seealso `AllocateDedicatedStack` if the kernel prefers to allocate a dedicated stack for the task dynamically.
        /// @note This initializer cannot be used to build the service routine that services the system call of creating new threads.
        ///       Use `AssignDedicatedStackWithSize` instead.
        ///
        template <typename Task>
        requires TaskConstraints::TaskHasDedicatedRecyclableStack<Task>
        struct AssignDedicatedStack
        {
            ///
            /// Assign the given stack to the given task
            ///
            /// @tparam N The stack size
            /// @param task A non-null task control block
            /// @param stack The stack to be assigned to the task
            /// @return `true` always.
            ///
            template <size_t N>
            bool operator()(Task* task, UInt8 (&stack)[N])
            {
                task->setPrivateStack(stack);

                task->setStackPointer(stack + N);

                return true;
            }
        };

        ///
        /// [KPI] Private subroutine to assign a pre-allocated stack to a task
        ///
        /// @tparam Task Specify the type of a task that has a dedicated stack
        /// @note This subroutine also adjusts task's stack pointer to the bottom of allocated stack.
        /// @note The kernel is NOT responsible for managing the memory of the stack.
        /// @seealso `AllocateDedicatedStack` if the kernel prefers to allocate a dedicated stack for the task dynamically.
        /// @note This initializer can be used to build the service routine that services the system call of creating new threads.
        ///
        template <typename Task>
        requires TaskConstraints::TaskHasDedicatedRecyclableStack<Task>
        struct AssignDedicatedRecyclableStackWithSize
        {
            /// Define the argument type
            using Arg = std::pair<UInt8*, size_t>;

            ///
            /// Assign the given stack to the given task
            ///
            /// @param task A non-null task control block
            /// @param stack The stack to be assigned to the task and its size
            /// @return `true` always.
            ///
            bool operator()(Task* task, std::pair<UInt8*, size_t> stack)
            {
                task->setPrivateStack(stack.first);

                task->setStackPointer(stack.first + stack.second);

                return true;
            }
        };

        ///
        /// [KPI] Private subroutine to set up the execution context for a task
        ///
        /// @tparam Task Specify the type of a  task that has a dedicated stack
        /// @tparam TaskContextBuilder Specify the type of the architecture-specific context builder
        /// @note This subroutine assumes that kernel has assigned a shared or private stack to the given task.
        ///
        template <typename Task, typename TaskContextBuilder>
        requires TaskConstraints::TaskHasDedicatedStack<Task>
        struct SetupExecutionContext
        {
            /// Define the argument type
            using Arg = const UInt8*;

            ///
            /// Setup the execution context for the given task
            ///
            /// @param task A non-null task control block
            /// @param entryPoint The thread entry point
            /// @return `true` always.
            ///
            bool operator()(Task* task, const UInt8* entryPoint)
            {
                precondition(task->getStackPointer() != nullptr, "No stack is assigned to the given task.");

                TaskContextBuilder{}(task, entryPoint);

                return true;
            }
        };

        ///
        /// [KPI] Private subroutine to assign an unique identifier to a task
        ///
        /// @tparam Task Specify the type of a task that has a dedicated stack
        ///
        template <typename Task>
        requires TaskConstraints::TaskHasUniqueIdentifier<Task>
        struct AssignUniqueIdentifier
        {
            /// Define the argument type
            using Arg = UInt32;

            ///
            /// Assign the given identifier to the task
            ///
            /// @param task A non-null task control block
            /// @param identifier The task identifier
            /// @return `true` always.
            ///
            bool operator()(Task* task, UInt32 identifier)
            {
                task->setUniqueIdentifier(identifier);

                return true;
            }
        };

        ///
        /// [KPI] Private subroutine to assign a priority level a task
        ///
        /// @tparam Task Specify the type of a task that has a dedicated stack
        ///
        template <typename Task>
        requires TaskConstraints::TaskIsPrioritizableByPriority<Task>
        struct AssignPriority
        {
            /// Define the argument type
            using Arg = typename Task::Priority;

            ///
            /// Assign the given priority level to the task
            ///
            /// @param task A non-null task control block
            /// @param priority The task priority level
            /// @return `true` always.
            ///
            bool operator()(Task* task, typename Task::Priority priority)
            {
                task->setPriority(priority);

                return true;
            }
        };

        ///
        /// [KPI] Invoke a list of task control block initializers with supplied arguments
        ///
        /// @tparam Task Specify the type of a task control block
        /// @tparam Initializers Specify zero or more task control block initializers
        ///
        template <typename Task, typename... Initializers>
        struct TaskInitializerBuilderWithArgs
        {
            ///
            /// Execute selected list of task control block initializers
            ///
            /// @param task A non-null task control block
            /// @param args A list of arguments passed to initializers;
            ///             The number of arguments must be identical to that of initializers;
            ///             The type of each argument must match the one specified by the corresponding initializer.
            /// @return `true` on success, `false` otherwise.
            ///
            template <typename... Args>
            requires (sizeof...(Initializers) == sizeof...(Args))
            bool operator()(Task* task, Args&&... args)
            {
                // This is equivalent to invoking each initialize with the given argument
                // If one of them returns false, the rest of them will not be executed
                // Some initializer always returns true, so the compiler will optimize out unnecessary checks
                return ((Initializers{}(task, std::forward<Args>(args))) && ...);
            }
        };
    }

    ///
    /// Build the kernel service routine that creates a new thread with supplied arguments
    ///
    /// @tparam Task Specify the type of a task control block
    /// @tparam TaskScheduler Specify the type of a task scheduler
    /// @tparam TaskController Specify the type of a task controller
    /// @tparam Initializers Specify zero or more task control block initializers
    /// @note This service routine is expected to be invoked in the kernel.
    ///       It is useful to create threads at kernel initialization time.
    ///
    template <typename Task, typename TaskScheduler, typename TaskController, typename... Initializers>
    requires TaskConstraints::TaskCanInvokeSystemCall<Task> &&
             Scheduler::ProvidesTaskCreationHandler<TaskScheduler, Task> &&
             TaskControllerProvidesBasicAllocationSupport<TaskController>
    struct ServiceRoutineBuilder
    {
        ///
        /// Execute selected initializers with supplied arguments
        ///
        /// @param task The current running task
        /// @param args Zero or more arguments, each of which is passed to the corresponding initializer
        /// @return The next task that is selected to run.
        ///
        template <typename... Args>
        Task* operator()(Task* task, Args&&... args)
        {
            // Guard: Allocate a task control block
            TaskController& controller = GetTaskController<TaskController>();

            Task* newTask = controller.allocate();

            if (newTask == nullptr)
            {
                perr("Failed to allocate a task control block.");

                // TODO: kIOReturnValue
                task->setSyscallKernelReturnValue(-1);

                return task;
            }

            // Guard: Initialize the task control block
            if (!KPI::TaskInitializerBuilderWithArgs<Task, Initializers...>{}(newTask, std::forward<Args>(args)...))
            {
                perr("Failed to initialize the task control block.");

                controller.release(newTask);

                // TODO: kIOReturnValue
                task->setSyscallKernelReturnValue(-1);

                return task;
            }

            // A new task has been created
            // Notify the scheduler
            return GetTaskScheduler<TaskScheduler>().onTaskCreated(task, newTask);
        }

        ///
        /// Execute selected initializers with supplied arguments
        ///
        /// @param task The current running task
        /// @param args Zero or more arguments, each of which is passed to the corresponding initializer
        /// @return The next task that is selected to run.
        /// @note This is a helper used by `ServiceRoutineBuilderWithTaskArgs` to
        ///       execute initializers with system call arguments conveniently.
        ///
        template <typename... Args>
        static inline Task* execute(Task* task, Args&&... args)
        {
            return ServiceRoutineBuilder{}(task, std::forward<Args>(args)...);
        }
    };

    ///
    /// Build the kernel service routine that creates a new thread with arguments supplied by the task
    ///
    /// @tparam Task Specify the type of a task control block
    /// @tparam TaskScheduler Specify the type of a task scheduler
    /// @tparam Initializers Specify zero or more task control block initializers
    /// @note This service routine is expected to be invoked in the kernel to service the system call.
    ///
    template <typename Task, typename TaskScheduler, typename TaskController, typename... Initializers>
    requires TaskConstraints::TaskCanInvokeSystemCall<Task> &&
             Scheduler::ProvidesTaskCreationHandler<TaskScheduler, Task> &&
             TaskControllerProvidesBasicAllocationSupport<TaskController>
    struct ServiceRoutineBuilderWithTaskArgs
    {
        Task* operator()(Task* task)
        {
            // Guard: Retrieve system call arguments
            // We cannot simply do `std::invoke(ServiceRoutineBuilderWithArgs::execute, task->getSystemCallArgument<typename Initializers::Arg>()...)`,
            // because C++ does not specify the order of evaluation in this case.
            // But `getSystemCallArgument()` is stateful, and the order of evaluation does matter here,
            // so we use a index sequence to build a tuple that collects system call arguments in order.
            std::tuple<typename Initializers::Arg...> arguments;

            auto collector = [&]<std::size_t... I>(std::index_sequence<I...>) -> void
            {
                // A fold expression to collect system arguments one by one
                ((std::get<I>(arguments) = task->template getSyscallArgument<typename Initializers::Arg>()) , ...);
            };

            collector(std::index_sequence_for<Initializers...>());

            // Execute initializers with collected system call arguments
            return std::apply(ServiceRoutineBuilder<Task, TaskScheduler, Initializers...>::execute,
                              std::tuple_cat(std::make_tuple(task), arguments));
        }
    };
}

#endif /* Execution_SimpleThreadBasedKernelServiceRoutines_hpp */
