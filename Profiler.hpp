#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <chrono>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <string>
#include <mutex>
#include <fstream>
#include <iostream>

/* 
    Create profiler files in the form of .json
    To be used in chromo://tracing in web browser
*/

namespace Profiler {

    struct Result
    {
        std::string name;
        long long start;
        long long end;
        uint32_t thread_id;
    };

    struct Session
    {
        std::string Name;
    };

    class Recorder
    {
    private:
        Session* current_session;
        std::mutex mutex;
        std::ofstream output_stream;
        bool first_record;
    public:
        Recorder()
            : current_session(nullptr)
        {
        }

        void BeginSession(const std::string& name, const std::string& filepath = "results.json")
        {
			std::lock_guard<std::mutex> lock(mutex);
            first_record = true;
			if (current_session)
			{
				// If there is already a current session, then close it before beginning new one.
				// Subsequent profiling output meant for the original session will end up in the
				// newly opened session instead.  That's better than having badly formatted
				// profiling output.
				std::out << "Instrumentor::BeginSession(" << name << ") when session " << current_session->Name << " already open." << std::endl;
				EndSession();
			}
			output_stream.open(filepath);

			if (output_stream.is_open())
			{
				current_session = new Session({name});
				WriteHeader();
			}
			else
			{
				std::out << "Instrumentor could not open results file " << filepath << "." << std::endl;
			}
        }

        void EndSession()
        {
            std::lock_guard<std::mutex> lock(mutex);
			if (current_session)
			{
				WriteFooter();
				output_stream.close();
				delete current_session;
				current_session = nullptr;
			}
        }

        void WriteRecord(const Result& result)
        {
			std::stringstream json;
            json << std::setprecision(3) << std::fixed;

            if(first_record) {
                json << "{";
                first_record = false;
            } else {
                json << ",{";
            }

            std::string name = result.name;
			std::replace(name.begin(), name.end(), '"', '\'');
            
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << (result.end - result.start) << ',';
			json << "\"name\":\"" << name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << result.thread_id << ",";
			json << "\"ts\":" << result.start;
			json << "}";

			std::lock_guard<std::mutex> lock(mutex);
			if (current_session)
			{
				output_stream << json.str();
				output_stream.flush();
			}
        }

        void WriteHeader()
        {
            output_stream << "{\"otherData\": {},\"traceEvents\":[";
            output_stream.flush();
        }

        void WriteFooter()
        {
            output_stream << "]}";
            output_stream.flush();
        }

        static Recorder& Get()
        {
            static Recorder instance;
            return instance;
        }
    };

    class Timer {
    public: 
        Timer(const char* setName)
            : name(setName), stopped(false)
        {
            start_timepoint = std::chrono::high_resolution_clock::now();
        }

        ~Timer()
        {
            if(stopped == false) {
                Stop();
            }
        }

        void Stop()
        {
            auto end_timepoint = std::chrono::high_resolution_clock::now();
            auto start = std::chrono::time_point_cast<std::chrono::microseconds>(start_timepoint).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint).time_since_epoch().count();

            uint32_t thread_id = (uint32_t) std::hash<std::thread::id>{}(std::this_thread::get_id());
            Recorder::Get().WriteRecord({ name, start, end, thread_id });

            stopped = true;
        }

    private:
        const char* name;
        bool stopped = false;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_timepoint;
    };

}

#define DEBUG_PROFILE 1
#if DEBUG_PROFILE

	#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
		#define FUNC_SIG __PRETTY_FUNCTION__
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
		#define FUNC_SIG __PRETTY_FUNCTION__
	#elif (defined(__FUNCSIG__) || (_MSC_VER))
		#define FUNC_SIG __FUNCSIG__
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
		#define FUNC_SIG __FUNCTION__
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
		#define FUNC_SIG __FUNC__
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
		#define FUNC_SIG __func__
	#elif defined(__cplusplus) && (__cplusplus >= 201103)
		#define FUNC_SIG __func__
	#else
		#define FUNC_SIG "FUNC_SIG unknown!"
	#endif

	#define PROFILE_BEGIN_SESSION(name, filepath) Profiler::Recorder::Get().BeginSession(name, filepath)
	#define PROFILE_END_SESSION() Profiler::Recorder::Get().EndSession()
	#define PROFILE_SCOPE(name) Profiler::Timer __profiler_scope_timer__(name);
	#define PROFILE_FUNCTION() Profiler::Timer __profiler_scope_timer__(FUNC_SIG);
#else
	#define PROFILE_BEGIN_SESSION(name, filepath)
	#define PROFILE_END_SESSION()
	#define PROFILE_SCOPE(name)
	#define PROFILE_FUNCTION()
#endif

#endif  //PROFILER_HPP