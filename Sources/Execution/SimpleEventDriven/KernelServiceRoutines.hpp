//
//  KernelServiceRoutines.hpp
//  Execution
//
//  Created by FireWolf on 2021-2-5.
//

#ifndef Execution_SimpleEventDrivenKernelServiceRoutines_hpp
#define Execution_SimpleEventDrivenKernelServiceRoutines_hpp

#include <Scheduler/Scheduler.hpp>
#include <Execution/Common/TaskConstraints.hpp>
#include <Execution/Common/KernelServiceRoutines.hpp>
#include <array>

template <typename Task, typename Event, size_t NumTasks>
requires std::unsigned_integral<Event>
struct TableBasedEventController
{
private:
    Task tasks[NumTasks];

public:
    void registerEvent(Event event, EventHandler handler)
    {
        this->tasks[event].setHandler(handler);
    }

    EventControlBlock* getRegisteredEvent(Event event)
    {
        return &this->tasks[event];
    }
};

/// Defines kernel service routines for the simple event driven execution model
namespace KernelServiceRoutines
{
    ///
    /// Specify the constraint of a functor that maps the given event to its handler control block
    ///
    template <typename Mapper, typename Task>
    concept Event2TaskMapper = requires(int event)
    {
        ///
        /// The mapper can be initialized with zero arguments
        ///
        std::default_initializable<Mapper>;

        ///
        /// The mapper must implement the operator `()` that consumes the event number and returns the handler control block
        ///
        { Mapper{}(event) } -> std::same_as<Task*>;
    };

    ///
    /// Kernel service routine to handle the request of sending an event
    ///
    /// @tparam Task Specify the type of the task control block that provides sequential access to system call arguments
    /// @tparam TaskScheduler Specify the type of the scheduler that provides the task creation handler
    /// @note This handler will notify the scheduler that a new event has been created.
    ///       The scheduler will return the next task that is selected to run.
    ///       Depending upon the actual scheduling policy, the current running task may be preempted.
    ///
    template <typename Task, typename TaskScheduler, typename TaskMapper>
    requires TaskConstraints::TaskProvidesSequentialSyscallArgumentsAccess<Task> &&
             Scheduler::ProvidesTaskCreationHandler<TaskScheduler, Task> &&
             Event2TaskMapper<TaskMapper, Task>
    struct SyscallSendEvent
    {
        Task* operator()(Task* task)
        {
            // Fetch the event number
            // TODO: Fetch the event data???
            auto event = task->template getSyscallArgument<int>();

            // Send the event
            pinfo("Task at 0x%p has requested to send the event %d.", task, event);

            return GetTaskScheduler<TaskScheduler>().onTaskCreated(task, TaskMapper{}(event));
        }
    };

    ///
    /// Kernel service routine to handle the task whose event handler has finished
    ///
    /// @tparam Task Specify the type of the task control block that provides sequential access to system call arguments and write access to the stack pointer
    /// @tparam TaskScheduler Specify the type of the scheduler that provides the task termination handler
    /// @note This handler will restore the stack pointer for the task and notify the scheduler that the task has finished.
    /// @note This handler is designed for event handlers that share the same user stack.
    ///
    template <typename Task, typename TaskScheduler>
    requires TaskConstraints::TaskProvidesSequentialSyscallArgumentsAccess<Task> &&
             TaskConstraints::TaskProvidesStackPointerWriteAccess<Task> &&
             Scheduler::ProvidesTaskTerminationHandler<TaskScheduler, Task>
    struct SyscallEventHandlerReturn
    {
        Task* operator()(Task* task)
        {
            // Fetch and restore the old stack pointer
            auto oldStackPointer = task->template getSyscallArgument<UInt8*>();

            task->setStackPointer(oldStackPointer);

            pinfo("Task stack pointer has been restored to 0x%p.", oldStackPointer);

            // Fetch the next event handler
            return GetTaskScheduler<TaskScheduler>().onTaskFinished(task);
        }
    };
}

#endif /* Execution_SimpleEventDrivenKernelServiceRoutines_hpp */
