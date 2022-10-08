//
//  TaskControlBlockComponents.hpp
//  Execution
//
//  Created by FireWolf on 2021-3-22.
//

#ifndef Execution_TaskControlBlockComponents_hpp
#define Execution_TaskControlBlockComponents_hpp

#include <Types.hpp>
#include <cstdarg>
#include "KernelServiceRoutines.hpp"
#include "ExecutionContext.hpp"

/// Define components that can be selected to assemble a task control block
namespace TaskControlBlockComponents
{
    ///
    /// Provide shared stack support for a task
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @note This component can be used to satisfy the task control block constraints
    ///       `TaskProvidesStackPointerReadAccess` and `TaskProvidesStackPointerWriteAccess`.
    ///
    template <typename Task>
    struct SharedStackSupport
    {
        UInt8* getStackPointer()
        {
            return KernelServiceRoutines::GetSharedTaskStackPointer();
        }

        void setStackPointer(UInt8* newStackPointer)
        {
            KernelServiceRoutines::GetSharedTaskStackPointer() = newStackPointer;
        }
    };

    ///
    /// Provide dedicated non-recyclable stack support for a task
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @note This component can be used to satisfy the task control block constraint `TaskHasDedicatedStack`.
    ///
    template <typename Task>
    struct DedicatedNonRecyclableStackSupport
    {
    private:
        UInt8* stackPointer;

    public:
        UInt8* getStackPointer()
        {
            return this->stackPointer;
        }

        void setStackPointer(UInt8* newStackPointer)
        {
            this->stackPointer = newStackPointer;
        }
    };

    ///
    /// Provide dedicated non-recyclable stack support for a task
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @note This component can be used to satisfy the task control block constraint `TaskHasDedicatedRecyclableStack`.
    ///
    template <typename Task>
    struct DedicatedRecyclableStackSupport: DedicatedNonRecyclableStackSupport<Task>
    {
    private:
        UInt8* stack;

    public:
        UInt8* getPrivateStack()
        {
            return this->stack;
        }

        void setPrivateStack(const UInt8* newStack)
        {
            this->stack = newStack;
        }
    };

    ///
    /// Provide dedicated non-recyclable stack support for a task
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @tparam Context Specify the type of the interrupt context stored on the stack
    /// @note This component can be used to satisfy the task control block constraint `TaskCanInvokeSystemCall`.
    ///
    template <typename Task, typename Context>
    requires ExecutionContextProvidesSystemCallSupport<Context>
    struct SystemCallSupport
    {
    private:
        Context* getExecutionContext()
        {
            return reinterpret_cast<Context*>(static_cast<Task*>(this)->getStackPointer());
        }

    public:
        template <typename Arg>
        Arg getSyscallArgument()
        {
            va_list* ptr = this->getExecutionContext()->getSyscallArgumentList();

            return va_arg(*ptr, Arg);
        }

        void setSyscallKernelReturnValue(int retVal)
        {
            this->getExecutionContext()->setSyscallKernelReturnValue(retVal);
        }
    };

    ///
    /// Provide unique numeric identifier support for a task
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @tparam Identifier Specify the type of the numeric task identifier
    /// @note This component can be used to satisfy the task control block constraint `TaskHasUniqueIdentifier`.
    ///
    template <typename Task, typename Identifier>
    requires std::unsigned_integral<Identifier>
    struct UniqueNumericIdentifierSupport
    {
    private:
        Identifier identifier;

    public:
        Identifier getUniqueIdentifier()
        {
            return this->identifier;
        }

        void setUniqueIdentifier(Identifier newIdentifier)
        {
            this->identifier = newIdentifier;
        }
    };

    ///
    /// Provide unique numeric identifier support for a task without declaring the instance variable
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @tparam Identifier Specify the type of the numeric task identifier
    /// @note This component can be used to satisfy the task control block constraint `TaskHasUniqueIdentifier`.
    /// @note The concrete task control block must declare the instance variable `identifier`.
    /// @note By default, the identifier is interpreted as a UInt32 value,
    ///       so developers must ensure that the real type can hold the value.
    ///
    template <typename Task, typename Identifier = UInt32>
    requires std::unsigned_integral<Identifier>
    struct UniqueNumericIdentifierSupportWithoutDeclaration
    {
        Identifier getUniqueIdentifier()
        {
            return static_cast<Task*>(this)->identifier;
        }

        void setUniqueIdentifier(Identifier newIdentifier)
        {
            static_cast<Task*>(this)->identifier = newIdentifier;
        }
    };

    ///
    /// Provide priority level support for a task
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @tparam Priority Specify the type of the task priority level
    /// @note This component can be used to satisfy the task control block constraint `TaskIsPrioritizableByPriority`.
    ///
    template <typename Task, typename Priority>
    struct PriorityLevelSupport
    {
    private:
        Priority priority;

    public:
        const Priority& getPriority()
        {
            return this->priority;
        }

        void setPriority(Priority newPriority)
        {
            this->priority = newPriority;
        }
    };

    ///
    /// Provide priority level support for a task
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @tparam Priority Specify the type of the numeric task priority level
    /// @note This component can be used to satisfy the task control block constraint `TaskIsPrioritizableByPriority`.
    /// @note The concrete task control block must declare the instance variable `priority`.
    /// @note By default, the priority is interpreted as a UInt32 value,
    ///       so developers must ensure that the real type can hold the value.
    ///
    template <typename Task, typename Priority = UInt32>
    requires std::unsigned_integral<Priority>
    struct PriorityLevelSupportWithoutDeclaration
    {
        const Priority& getPriority()
        {
            return static_cast<Task*>(this)->priority;
        }

        void setPriority(Priority newPriority)
        {
            static_cast<Task*>(this)->priority = newPriority;
        }
    };

    ///
    /// Provide unique numeric identifier support for a task
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @tparam State Specify the type of the task state
    /// @note This component can be used to satisfy the task control block constraint `TaskHasExplicitState`.
    ///
    template <typename Task, typename State>
    struct StateSupport
    {
    private:
        State state;

    public:
        State getState()
        {
            return this->state;
        }

        void setState(State newState)
        {
            this->state = newState;
        }
    };

    ///
    /// Provide unique numeric identifier support for a task
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @tparam State Specify the type of the task state
    /// @note This component can be used to satisfy the task control block constraint `TaskHasExplicitState`.
    /// @note The concrete task control block must declare the instance variable `state`.
    /// @note By default, the state is interpreted as a UInt32 value,
    ///       so developers must ensure that the real type can hold the value.
    ///
    template <typename Task, typename State = UInt32>
    requires std::unsigned_integral<State>
    struct StateSupportWithoutDeclaration
    {
        State getState()
        {
            return static_cast<Task*>(this)->state;
        }

        void setState(State newState)
        {
            static_cast<Task*>(this)->state = newState;
        }
    };

    ///
    /// Provide the event handler component for a task
    ///
    /// @tparam Task Specify the type of the concrete task control block
    /// @tparam EventHandler Specify the type of the event handler
    ///
    template <typename Task, typename EventHandler>
    struct EventHandlerSupport
    {
    private:
        EventHandler handler;

    public:
        using EventHandlerType = EventHandler;

        EventHandler getHandler() const
        {
            return this->handler;
        }

        void setHandler(EventHandler newHandler)
        {
            this->handler = newHandler;
        }
    };
}

#endif /* Execution_TaskControlBlockComponents_hpp */
