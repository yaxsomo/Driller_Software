#ifndef BACKOFF_HPP
#define BACKOFF_HPP

#include <boost/function.hpp>

/**
 * @brief Backoff strategies
 * 
 */
enum BackoffStrategy {
    NONE,
    FIXED,
    EXPONENTIAL,
    CUSTOM
};


/**
 * @brief Backoff class computing the delay between two attempts depending on the strategy
 * 
 */
class Backoff {
    public:
        /**
         * @brief Construct a new Backoff object
         * 
         */
        Backoff() = default;

        /**
         * @brief Construct a new Backoff object
         * 
         * @param customDelayFunction custom function computing the delay
         */
        Backoff(boost::function<int (int, int)> customDelayFunction);

        /**
         * @brief Method computing the delay between two attempts
         * 
         * @param strategy Strategy to use
         * @param delay Time delay used by the strategy
         * @return int Computed delay
         */
        int computeDelay(BackoffStrategy strategy, int delay, int attemptsMade);

        /**
         * @brief Set the Custom Delay Function object
         * 
         * @param customDelayFunction Function computing the delay
         */
        void setCustomDelayFunction(boost::function<int (int, int)> customDelayFunction);
        
    private:
        /**
         * @brief Function computing the delay
         * 
         */
        boost::function<int (int, int)> customDelayFunction;

        /**
         * @brief Fixed delay strategy
         * 
         * @param delay 
         * @return int Computed delay
         */
        int computeFixedDelay(int delay);

        /**
         * @brief Expontential delay strategy
         * 
         * @param delay 
         * @return int Computed delay
         */
        int computeExponentialDelay(int delay, int attempsMade);
};


#endif