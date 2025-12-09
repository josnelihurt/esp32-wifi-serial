#pragma once

namespace jrb::wifi_serial {

/**
 * @brief Interface for task execution
 * 
 * Base interface for all tasks in the system. Tasks are registered with
 * the TaskRegistry and executed during setup and loop phases.
 */
class ITask {
public:
    virtual ~ITask() = default;
    
    /**
     * @brief Initialize task during setup phase
     * 
     * Called once during system initialization. Default implementation does nothing.
     */
    virtual void setup() {}
    
    /**
     * @brief Execute task during loop phase
     * 
     * Called repeatedly in the main loop. Must be implemented by all tasks.
     */
    virtual void loop() = 0;
};

}  // namespace jrb::wifi_serial

