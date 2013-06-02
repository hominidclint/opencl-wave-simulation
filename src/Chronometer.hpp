// Copyright (c) 2012, Hannes Würfel <hannes.wuerfel@student.hpi.uni-potsdam.de>
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.


#ifndef CHRONOMETRY_H
#define CHRONOMETRY_H

#include <ctime>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>

/**
*   @brief Simple Start-Stop Timer class with some extra features for time stamps.
*/
class Chronometer
{
public:

    /**
    *   @brief Standard Constructor.
    */
    Chronometer() : m_startTime(0), m_stopTime(0) {};

    /**
    *   @brief Starts the chronometry.
    */
    inline void start()
    {
		m_startTime = clock() / static_cast<long double>(CLOCKS_PER_SEC);
    }

    /**
    *   @brief Stops the chronometry in millisecond resolution.
    */
    inline void stop()
    {
		m_stopTime = clock() / static_cast<long double>(CLOCKS_PER_SEC);
    }

    /**
    *   @brief Getter for the startTime.
    */
    inline long double getStartTime()
    {
        return m_startTime;
    }

    /**
    *   @brief Getter for the stopTime.
    */
    inline long double getStopTime()
    {
        return m_stopTime;
    }

    /**
    *   @Return Getter for the elapsedTime.
    */
    inline long double getElapsedTime()
    {
        return m_stopTime - m_startTime;
    }

    /**
    *   @Return Getter for the passed Time.
    */
    inline long double getPassedTimeSinceStart()
    {
        return (clock() / static_cast<long double>(CLOCKS_PER_SEC)) - m_startTime;
    }

    /**
    *   @brief Creates logging time stamps
    *   @brief param provides time information in millisecond resolution
    */
    static std::string systemTimeStamp(long double now)
    {
        tm* tm;
        time_t now__ = static_cast<time_t>(now);
        double intpart;
        double fractpart = modf(now, &intpart);

        if((tm = localtime(&now__)) == NULL)
        {
            std::cout << "Error extracting time\n";
            return std::string("");
        }

        std::stringstream sstream;
        sstream << tm->tm_year+1900 << "-" << tm->tm_mon+1 << "-" << tm->tm_mday
                << " " << tm->tm_hour << ":" << tm->tm_min << ":" << tm->tm_sec
                << ":" << static_cast<int>(fractpart * 1000);

        return sstream.str();    
    }

    /**
    *   @brief Creates logging time stamps
    *   @brief param provides time information in millisecond resolution
    */
    static std::string systemTimeStamp(time_t now)
    {
        tm* tm;
        time_t now__ = now;
        double intpart;
        double fractpart = modf((double)now, &intpart);

        if((tm = localtime(&now__)) == NULL)
        {
            std::cout << "Error extracting time\n";
            return std::string("");
        }

        std::stringstream sstream;
        sstream << tm->tm_year+1900 << "-" << tm->tm_mon+1 << "-" << tm->tm_mday
                << " " << tm->tm_hour << ":" << tm->tm_min << ":" << tm->tm_sec
                << ":" << static_cast<int>(fractpart * 1000);

        return sstream.str();    
    }

    /**
    *   @brief Returns the date.
    */
    static std::string systemDate()
    {
        tm* tm;

        time_t now = time(NULL);
        if((tm = localtime(&now)) == NULL)
        {
            std::cout << "Error extracting time\n";
            return std::string("");
        }

        std::stringstream sstream;
        sstream << tm->tm_year+1900 << "-" << tm->tm_mon+1 << "-" << tm->tm_mday;

        return sstream.str();
    }

    /**
    *   @brief Returns the current time.
    */
    static std::string systemTime()
    {
        tm* tm;

        time_t now = time(NULL);
        if((tm = localtime(&now)) == NULL)
        {
            std::cout << "Error extracting time\n";
            return std::string("");
        }

        std::stringstream sstream;
        sstream << tm->tm_hour << ":" << tm->tm_min << ":" << tm->tm_sec;

        return sstream.str();
    }

    /**
    *   @brief Busy wait to synchronize different instances or computers.
    *   @param t Specifies the start time.
    */
    static void waitUntilTimeTriggered(const tm* t)
    {
        for(;;)
        {
            time_t currentTime;
            time_t syncTime = mktime(const_cast<tm*>(t));
            currentTime = time(NULL);

            if(currentTime >= syncTime)
            {
                break;
            }
        }
    }

private:
    long double m_startTime;
    long double m_stopTime;

};

#endif // CHRONOMETRY_H
