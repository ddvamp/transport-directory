#ifndef TEST_RUNNER_H_
#define TEST_RUNNER_H_ 1

#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace testing {

inline void AssertEqual(auto const &lhs, auto const &rhs,
	std::optional<std::string_view> message = std::nullopt) noexcept(false)
{
	if (lhs == rhs) {
		return;
	}
	std::ostringstream os;
	os << "Assertion failed: ";
   	if constexpr (requires { os << lhs; }) {
		os << lhs;
	} else {
		os << "actual";
	}
	os << " != ";
	if constexpr (requires { os << rhs; }) {
		os << rhs;
	} else {
		os << "expected";
	}
	if (message) {
		os << " Hint: " << *message;
	}
	throw std::runtime_error{std::move(os).str()};
}

inline void Assert(bool b,
	std::optional<std::string_view> message = std::nullopt) noexcept(false)
{
	AssertEqual(b, false, message);
}

class TestRunner
{
public:
	template <typename TestFunc>
		requires requires (TestFunc func) { func(); }
	void RunTest(TestFunc func, std::string_view test_name)
	{
		try {
			func();
			std::cerr << test_name << " OK" << std::endl;
		} catch (std::exception &e) {
			++fail_count;
			std::cerr << test_name << " fail: " << e.what() << std::endl;
		} catch (...) {
			++fail_count;
			std::cerr << test_name <<
				" fail: Unknown exception caught" << std::endl;
		}
	}

	~TestRunner()
	{
		if (fail_count != 0) {
			std::cerr << fail_count << " unit tests failed. " << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}

private:
	int fail_count = 0;
};

#define ASSERT_EQUAL(x, y) { \
	std::ostringstream os; \
	os << #x << " != " << #y << ", " \
	   << __FILE__ << ":" << __LINE__; \
	testing::AssertEqual(x, y, std::move(os).str()); \
}

#define ASSERT(x) { \
	std::ostringstream os; \
	os << #x << " is false, " \
	   << __FILE__ << ":" << __LINE__; \
	testing::Assert(x, std::move(os).str()); \
}

#define RUN_TEST(tr, func) \
	tr.RunTest(func, #func)

} // namespace testing

#endif /* TEST_RUNNER_H_ */
