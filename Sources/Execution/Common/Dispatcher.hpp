//
//  Dispatcher.hpp
//  Execution
//
//  Created by FireWolf on 2021-2-3.
//

#ifndef Execution_Dispatcher_hpp
#define Execution_Dispatcher_hpp

#include "ContextSwitcher.hpp"
#include <concepts>
#include <Debug.hpp>

///
/// Define a function that redirects the invocation to a concrete routine block
///
/// @param Func The function name
/// @param Task The type of the task on the system; typically the task control block
/// @param RoutineBlock The type of the routine block that implements the operator `()`
/// @note The dispatcher relies on the function pointer to redirect the request to the handler,
///       while handlers are implemented as individual modular building blocks.
///       This macro is convenient to define a function that delegates to the actual routine block,
///       so developers can take the address of this function and pass the value to the dispatcher.
///
#define OSDefineAndRouteKernelRoutine(Func, Task, RoutineBlock) \
static Task* Func(Task* task)                                   \
{                                                               \
    return RoutineBlock{}(task);                                \
}                                                               \

/// Define the interface of the kernel service routine mapper
template <typename Mapper, typename Identifier>
concept KernelServiceRoutineMapper = requires(const Identifier& identifier)
{
    ///
    /// The mapper must explicitly define the type of task handled by the kernel service routine
    ///
    typename Mapper::Task;

    ///
    /// The mapper can be initialized with zero arguments
    ///
    requires std::default_initializable<Mapper>;

    ///
    /// The mapper must implement the operator `()` that consumes a service identifier and produces a non-null function pointer.
    ///
    /// @note Signature `Task* (*)(Task*) operator()(const Identifier& identifier)`.
    ///
    { Mapper{}(identifier) } -> std::same_as<typename Mapper::Task* (*)(typename Mapper::Task*)>;
};

///
/// Define the interface of injecting code before a task is switched to run
///
/// @note i.e. Before the invocation of `ContextSwitcher::switchTask(from:to:)`.
///
template <typename Injector, typename Task>
concept DispatcherCodeInjector = requires(Task* prev, Task* next)
{
    ///
    /// The injector can be initialized with zero arguments
    ///
    requires std::default_initializable<Injector>;

    ///
    /// The injector must implement the operator () that consumes the task that is interrupted and the task that is selected to run
    ///
    { Injector{}(prev, next) } -> std::same_as<void>;
};

///
/// The dispatcher acts as the front desk for all system calls, hardware interrupts and exceptions
///
/// @tparam Task Specify the type of runnable task
/// @tparam ServiceIdentifier Specify the type of the service identifier returned by the context switcher
/// @tparam RoutineMapper Specify the mapper that maps an identifier to the corresponding service routine
/// @tparam ContextSwitcher Specify the type of the context switcher
/// @tparam Injector A list of injector to inject code before the task is switched to run
/// @note The dispatcher relies on the context switcher that provides both kernel entry and exit points.
///       It uses the service identifier returned by `ContextSwitcher::switch(from:to:)` to invoke the corresponding service routine.
///       The exact meaning of the identifier is up to developers. For example, it can be the trap number on x86.
///       As such, developers should provide a routine mapper that takes an identifier and returns a function pointer.
///       The returned function takes a reference to the current interrupted task and returns the non-null next task.
///       The dispatcher then invokes the context switcher to switch from the current task to the next one.
/// @note Tinkertoy provides a convenient macro to declare the routine function and routes to existing modular building blocks.
/// @see `OSDefineAndRouteKernelRoutine` for detailed explanation.
/// @see Refer to the constraint definition of `KernelServiceRoutine` and `ContextSwitcher`.
///
template <typename Task, typename ServiceIdentifier, typename ServiceRoutineMapper, typename Switcher, DispatcherCodeInjector<Task>... Injector>
requires KernelServiceRoutineMapper<ServiceRoutineMapper, ServiceIdentifier> &&
         ContextSwitcher<Switcher> &&
         std::same_as<typename Switcher::ServiceIdentifier, ServiceIdentifier> &&
         std::same_as<typename Switcher::Task, Task> &&
         std::same_as<typename ServiceRoutineMapper::Task, Task>
class Dispatcher
{
private:
    /// The task that is interrupted (by a system call, hardware interrupt, exception, ...)
    Task* prev;

    /// The task that is selected to run
    Task* next;

public:
    ///
    /// Create a dispatcher with initial tasks
    ///
    /// @param prev The task that is interrupted
    /// @param next The task that is selected to run
    /// @note If the system supports the idle task, pass the idle task to `prev`
    ///       to assume that it is running before entering the kernel.
    /// @note Pass the first task tha will run on the system to `next`.
    ///
    Dispatcher(Task* prev, Task* next)
    {
        this->prev = prev;

        this->next = next;
    }

    ///
    /// The kernel dispatcher loop
    ///
    __attribute__((noreturn))
    void dispatch()
    {
        while (true)
        {
            // Perform code injections
            ((Injector{}(this->prev, this->next)), ...);

            // Switch the task and exit the kernel
            // When the function returns, we are back to the kernel
            ServiceIdentifier identifier = Switcher::switchTask(this->prev, this->next);

            // Enter the kernel
            this->prev = this->next;

            // Invoke the kernel service routine
            this->next = ServiceRoutineMapper{}(identifier)(this->prev);
        }
    }
};

#endif /* Execution_Dispatcher_hpp */
