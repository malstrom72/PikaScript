/**
	Runs deep recursion with the library's default depth limit on a 1 MB stack (the Windows default, `ulimit -s 1024`
	in build.sh), untraced and traced, and checks that every case throws instead of overflowing the native stack.
	PikaCmd raises both the stack and the depth limit, so its tests cannot catch this.
**/

#include <stdio.h>
#include <stdexcept>
#include <string>
#include "../src/PikaScript.h"
#include "../src/PikaScriptImpl.h"

typedef Pika::StdScript Script;

static Script::Value boom(const Script::Frame&) { throw std::runtime_error("boom"); }

static bool run(int traceLevel, const std::string& source, const std::string& expected) {
	Script::FullRoot root;
	root.registerNative("boom", boom);
	if (traceLevel > 0) root.evaluate("trace(function { }, " + Pika::intToString<std::string>(traceLevel) + ")");
	std::string result = "no exception";
	try {
		root.evaluate(source);
	} catch (const Script::Xception& x) {
		result = x.getError();
	} catch (const std::exception& x) {
		result = x.what();
	}
	const bool ok = (result == expected);
	if (!ok) printf("FAIL trace %d: %s: %s\n", traceLevel, source.substr(0, 70).c_str(), result.c_str());
	return ok;
}

int main() {
	const std::string TOO_DEEP = "Recursion too deep";
	const std::string CASES[][2] = {
		{ "::deep = function { ::deep() }; ::deep()", TOO_DEEP },
		{ "::deep = function { if (true) { x = 1 + ::deep() } }; ::deep()", TOO_DEEP },
		{ "::deep = function { evaluate('::deep()') }; ::deep()", TOO_DEEP },
		{ std::string(20000, '(') + "1" + std::string(20000, ')'), TOO_DEEP },
		{ "::deep = function { if (try(>::deep()) !== void) boom() }; ::deep()", "boom" }								// A C++ exception from the deepest level.
	};
	const int TRACE_LEVELS[] = { 0, 1, 2, 3, 22 };
	int failures = 0;
	for (size_t i = 0; i < sizeof (TRACE_LEVELS) / sizeof (*TRACE_LEVELS); ++i)
		for (size_t j = 0; j < sizeof (CASES) / sizeof (*CASES); ++j)
			if (!run(TRACE_LEVELS[i], CASES[j][0], CASES[j][1])) ++failures;
	if (failures != 0) {
		printf("%d stack test(s) failed\n", failures);
		return 1;
	}
	printf("Stack tests passed\n");
	return 0;
}
