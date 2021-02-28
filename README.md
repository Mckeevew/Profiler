# Profiler
Simple Profiler. Will output json files that can be read by chrome tracing. Can be found in chrome web browser at chrome://tracing.

# Usage

PROFILE_BEGIN_SESSION("Startup", "profile_results.json");

PROFILE_END_SESSION();

PROFILE_FUNCTION();

PROFILE_SCOPE("Name of Scope");
