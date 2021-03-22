#include <iostream>
#include <random>
#include "TrafficLight.h"



TrafficLightPhase MessageQueue::receive()
{      
        std::unique_lock<std::mutex> uLock(_mutex);

        _condition.wait(uLock, [this] { return !_queue.empty(); }); 
      
        TrafficLightPhase msg = std::move(_queue.back());
        _queue.pop_back();

        return msg; 
}


void MessageQueue::send(TrafficLightPhase &&msg)
{               
        std::lock_guard<std::mutex> uLock(_mutex);       
        
        _queue.push_back(std::move(msg));
        std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
        _condition.notify_one(); 
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _phaseQueue = std::make_shared<MessageQueue>();
}

TrafficLight::~TrafficLight()
{
    
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while (true)
    {
        auto phase = _phaseQueue->receive();
        if (phase == TrafficLightPhase::green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 

    std::thread t(&TrafficLight::cycleThroughPhases, this);
    threads.emplace_back(std::move(t));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::random_device rd;  
    std::mt19937 generator(rd()); 
    std::uniform_int_distribution<> distribution(4000, 6000);     

    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    lastUpdate = std::chrono::system_clock::now();
    auto randomDelay = distribution(generator);

    while (true)
    {        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();

        if (timeSinceLastUpdate >= randomDelay)
        {
            if (_currentPhase == TrafficLightPhase::red)
                _currentPhase = TrafficLightPhase::green;
            else
                _currentPhase = TrafficLightPhase::red;
      
            _phaseQueue->send(std::move(_currentPhase));
            lastUpdate = std::chrono::system_clock::now();
        }
    }

}

