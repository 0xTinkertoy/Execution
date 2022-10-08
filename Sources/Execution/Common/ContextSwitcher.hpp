//
//  ContextSwitcher.hpp
//  Execution
//
//  Created by FireWolf on 2021-2-3.
//

#ifndef Execution_ContextSwitcher_hpp
#define Execution_ContextSwitcher_hpp

#include <concepts>
#include "TaskConstraints.hpp"

/// Specify the constraint for the context switcher
template <typename Switcher>
concept ContextSwitcher = requires(typename Switcher::Task* prev, typename Switcher::Task* next)
{
    ///
    /// The context switcher must explicitly define the task type as well as the service identifier type
    ///
    typename Switcher::Task;
    typename Switcher::ServiceIdentifier;

    ///
    /// The task control block must provide full access to its stack pointer
    ///
    TaskConstraints::TaskProvidesStackPointerReadAccess<typename Switcher::Task>;
    TaskConstraints::TaskProvidesStackPointerWriteAccess<typename Switcher::Task>;

    ///
    /// The context switcher must implement the static function that switches tasks and provides kernel entry and exit points
    ///
    /// @note Signature: `static ServiceIdentifier switchTask(Task* from, Task* to)`.
    ///
    { Switcher::switchTask(prev, next) } -> std::same_as<typename Switcher::ServiceIdentifier>;
};

#endif /* Execution_ContextSwitcher_hpp */
