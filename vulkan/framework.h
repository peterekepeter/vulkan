#pragma once

#define __STRINGIZE_DETAIL(X) #X
#define __STRINGIZE(X) __STRINGIZE_DETAIL(X)
void __throw_formatted_message(const char* message);
#define __throw_with_location_in_source(M) __throw_formatted_message(M "\n  at" __FILE__ "(" __STRINGIZE(__LINE__) ")\n")

#define assert(C,M) {if(!(C)){ throw std::runtime_error("Assertion failed: " M); }}
#define expect(C,M) {if(!(C)){ throw std::runtime_error("Didn't meet expectation: " M); }}
#define ensure(C,M) {if(!(C)){ throw std::runtime_error("Failed to ensure that the " M); }}
