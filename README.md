# Profiler

Simple logging library I made for myself. Will print to the consol and a results.log file. This logger attempts to replicate the style and syntax of std::out.

# Usage

PROFILE_BEGIN_SESSION("Startup", "profile_results.json");

PROFILE_END_SESSION();

PROFILE_FUNCTION();

PROFILE_SCOPE("Name of Scope");