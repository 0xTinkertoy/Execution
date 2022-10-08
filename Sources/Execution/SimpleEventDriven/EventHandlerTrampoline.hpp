//
//  EventHandlerTrampoline.hpp
//  Execution
//
//  Created by FireWolf on 2021-2-5.
//

#ifndef Execution_EventHandlerTrampoline_hpp
#define Execution_EventHandlerTrampoline_hpp

#include <Debug.hpp>
#include "Syscall.hpp"

///
/// A code injector for the dispatcher to setup the execution context (if necessary) for a **preemptive** event handler that is selected to run
///
/// @note This injector setups the context for the next task that has a higher priority than the current one.
///       i.e. Only a higher priority handler can preempt a lower one.
///       This injector also assumes that event handlers are one-shot and will run to completion without blocking.
/// @note Kernel developers must implement the architecture-dependent context builder.
///       The context builder should implement the operator `()` that has the same signature as this functor.
///
template <typename Task, typename ContextBuilder>
struct PreemptiveEventHandlerTrampolineContextInjector
{
    void operator()(Task* prev, Task* next)
    {
        // Guard: Build the context if and only if the next task has a higher priority than the interrupted one
        // Only a high priority handler can preempt a lower one
        if (*next > *prev)
        {
            pinfo("The next event handler has a higher priority than the previous one.");

            ContextBuilder{}(prev, next);
        }
    }
};

///
/// A code injector for the dispatcher to setup the execution context (if necessary) for a **cooperative** event handler that is selected to run
///
/// @note This injector setups the context for the next task that is not the same as the current one.
///       i.e. A new task cannot preempt the current running one.
///       This injector also assumes that event handlers are one-shot and will run to completion without blocking.
/// @note Kernel developers must implement the architecture-dependent context builder.
///       The context builder should implement the operator `()` that has the same signature as this functor.
///
template <typename Task, typename ContextBuilder>
struct CooperativeEventHandlerTrampolineContextInjector
{
    void operator()(Task* prev, Task* next)
    {
        // Guard: Build the context if and only if the next task is not the current one
        if (next != prev)
        {
            // The previous
            pinfo("The next event handler is not the same as the previous one.");

            ContextBuilder{}(prev, next);
        }
    }
};

/// Private trampoline function to bootstrap the event handler
/// The trampoline ensures that the control is handed back to kernel after the event handler finishes
static void EventHandlerTrampoline(void (*handler)(), uint8_t* oldStack)
{
    handler();

    sysEventHandlerReturn(oldStack);
}

#endif /* Execution_EventHandlerTrampoline_hpp */
