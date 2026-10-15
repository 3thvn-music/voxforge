#pragma once
#include <cstdio>
#include <string>

namespace vftest {

struct Counters { int checks = 0, failures = 0; };
inline Counters& counters() { static Counters c; return c; }

inline void check(bool ok, const std::string& name, double measured, double limit, const char* relation)
{
    ++counters().checks;
    if (!ok) ++counters().failures;
    std::printf("[%s] %s: measured %.4f %s %.4f\n", ok ? "PASS" : "FAIL", name.c_str(), measured, relation, limit);
}
inline void checkLE(const std::string& n, double measured, double limit) { check(measured <= limit, n, measured, limit, "<="); }
inline void checkGE(const std::string& n, double measured, double limit) { check(measured >= limit, n, measured, limit, ">="); }
inline void checkTrue(const std::string& n, bool v) { check(v, n, v ? 1.0 : 0.0, 1.0, "=="); }

/** Informational result: printed, never counted as a failure. Used only for documented known limitations. */
inline void info(const std::string& name, double measured, double limit, const char* relation, bool wouldPass)
{
    std::printf("[%s] %s: measured %.4f %s %.4f\n", wouldPass ? "INFO-OK" : "KNOWN-LIMITATION", name.c_str(), measured, relation, limit);
}

inline int finish()
{
    std::printf("\n%d checks, %d failed\n", counters().checks, counters().failures);
    return counters().failures == 0 ? 0 : 1;
}

} // namespace vftest
