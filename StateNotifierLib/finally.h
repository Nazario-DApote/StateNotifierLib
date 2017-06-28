#pragma once
#include <functional>
#include <stdexcept>

#if __cplusplus <= 199711L // C++ 11
/// https://www.codeproject.com/Tips/476970/finally-clause-in-Cplusplus

class finally
{
	std::function<void(void)> functor;
public:
	finally(const std::function<void(void)> &functor) : functor(functor) {}
	~finally()
	{
		functor();
	}
};
#else
// C++ 11
class finally
{
	std::function<void()> m_finalizer;
	finally() = delete;

public:
	finally(const finally& other) = delete;
	finally(std::function<void()> finalizer)
		: m_finalizer(finalizer)
	{
	}
	~finally()
	{
		if (m_finalizer)
			m_finalizer();
	}
};
#endif

/* Usage:
https://stackoverflow.com/questions/17356258/correctly-implement-finally-block-using-c-lambda

An alternative solution using std::function. No factory function needed. No per-use template instantiation (better footprint?!).
No std::move and && stuff needed, no auto needed ;)

int main( int argc, char * argv[] )
{
	bool something = false;
	try
	{
	try
	{
		std::cout << "starting" << std::endl;
		finally final([&something]() { something = true; });
		std::cout << "throwing" << std::endl;
		throw std::runtime_error("boom");
	}
	catch(std::exception & ex )
	{
		std::cout << "inner catch" << std::endl;
		throw;
	}
	}
	catch( std::exception & ex )
	{
		std::cout << "outer catch" << std::endl;
		if( something )
		{
			std::cout << "works!" << std::endl;
			}
			else
		{
			std::cout << "NOT working!" << std::endl;
			}
		}
		std::cout << "exiting" << std::endl;
		return 0;
	}
*/