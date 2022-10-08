//
//  KernelServiceRoutines.hpp
//  Execution
//
//  Created by FireWolf on 2021-2-5.
//

#ifndef Execution_CommonKernelServiceRoutines_hpp
#define Execution_CommonKernelServiceRoutines_hpp

#include <Debug.hpp>

/// Declare a global task scheduler with the given type and name
#define OSDeclareTaskScheduler(type, name) \
static type name;

/// Declare a global task scheduler with the given type and name
/// as well as the kernel service routine that retrieves the global scheduler
#define OSDeclareTaskSchedulerWithKernelServiceRoutine(type, name) \
OSDeclareTaskScheduler(type, name)                                 \
namespace KernelServiceRoutines                                    \
{                                                                  \
    template <typename S>                                          \
    S& GetTaskScheduler()                                          \
    {                                                              \
        return name;                                               \
    }                                                              \
}

/// Declare a global task controller with the given type and name
#define OSDeclareTaskController(type, name) \
static type name;

/// Declare a global task controller with the given type and name
/// as well as the kernel service routine that retrieves the global controller
#define OSDeclareTaskControllerWithKernelServiceRoutine(type, name) \
OSDeclareTaskController(type, name)                                 \
namespace KernelServiceRoutines                                     \
{                                                                   \
    template <typename C>                                           \
    C& GetTaskController()                                          \
    {                                                               \
        return name;                                                \
    }                                                               \
}

/// Declare the shared stack pointer for all tasks as well as the kernel service routine to access it
#define OSDeclareSharedTaskStackPointer(name) \
static UInt8* name;                           \
namespace KernelServiceRoutines               \
{                                             \
    UInt8*& GetSharedTaskStackPointer()       \
    {                                         \
        return name;                          \
    }                                         \
}

/// Defines helper functions for some kernel service routines
namespace KernelServiceRoutines
{
    ///
    /// Get the current task scheduler
    ///
    /// @tparam S Specify the type of the task scheduler
    /// @return The current task scheduler.
    /// @note This function must be implemented by kernel developers.
    ///       Kernel service routines rely on this function to reschedule tasks if necessary.
    ///       The implementation can be as simple as returning the global scheduler variable on a single-core system,
    ///       or returning the scheduler attached to the current interrupted processor on a multi-core system.
    ///
    template <typename S>
    S& GetTaskScheduler();

    ///
    /// Get the current task controller
    ///
    /// @tparam C Specify the type of the task controller
    /// @return The current task controller.
    /// @note This function must be implemented by kernel developers.
    ///       Kernel service routines rely on this function to retrieve task control blocks if necessary.
    ///       The implementation can be as simple as returning the global controller variable.
    ///
    template <typename C>
    C& GetTaskController();

    ///
    /// Get the current shared stack pointer for all tasks
    ///
    /// @return The current shared stack pointer.
    /// @note This function must be implemented by kernel developers,
    ///       if tasks running on the system shares the same stack.
    /// @see Task control block components - shared stack support for details.
    ///
    UInt8*& GetSharedTaskStackPointer();

    template <typename Controller>
    concept TaskControllerProvidesBasicAllocationSupport = requires(Controller& controller, typename Controller::Task* task)
    {
        ///
        /// Task controller can allocate a free task control block
        ///
        { controller.allocate() } -> std::same_as<typename Controller::Task*>;

        ///
        /// Task controller can release a free task control block
        ///
        { controller.release(task) } -> std::same_as<void>;
    };

    ///
    /// Kernel service routine to report an error when the service identifier cannot be recognized
    ///
    template <typename Task>
    struct UnknownServiceIdentifier
    {
        Task* operator()([[maybe_unused]] Task* task)
        {
            pfatal("Unknown system call identifier.");
        }
    };
}

#endif /* Execution_CommonKernelServiceRoutines_hpp */
