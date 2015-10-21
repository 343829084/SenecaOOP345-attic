// g++   -std=c++11 -Wa,-adhln -O0 rl-value.cpp -o rl-value > rl-value.mixed-listing.4.9 && ./rl-value
// g++-5 -std=c++11 -Wa,-adhln -O0 rl-value.cpp -o rl-value > rl-value.mixed-listing.5.x && ./rl-value

// Question:
// 'int 99;: Does it wind up as a constant in the literal pool or is it loaded as an in-line assembler constant?
// Answer:
//  g++ 4.9.2 - in-line assembler
//  g++ 5.1.1 - in-line assembler

#include <iostream>
#include <string>
#include <typeinfo>     // typeid
#include <cassert>      // assert
#include <exception>
#include <csignal>

#define TYPEID(X) std::cout << "typeid(" #X ") fields: .name()=" << typeid(X).name() << ", .hash_code()=" << typeid(X).hash_code() << "\n";

template<typename T> void Print(const T&  x) { std::cout << "l-value: value=" << x << " "; TYPEID(x) }
template<typename T> void Print(const T&& x) { std::cout << "r-value: value=" << x << " "; TYPEID(x) }

template<typename T> void Zap(T* pointer) { *pointer = T(0); }
template<typename T> void Inc(T* pointer) { (*pointer)++; }

void Work()
{
#ifdef __GNUC__  // either g++ or clang++
  std::cout << "\nFILE '" << __FILE__ << "' compiled " 
            << __DATE__ << " at " << __TIME__ << " by: "
  #ifdef __clang__
    << "clang++ compiler "
  #else
    << "g++ compiler "
  #endif
   << __VERSION__
   << "\n";
#endif

   Print(99);                           // R-value in-line assembler constant
   Print(int(99));                      // R-value in-line assembler constant

// volatile 
   const int c99 = 99;                  // constant literal pool

   Print(c99);                          // L-value, being const, cannot write c99 = 42.
// Zap(&c99);                           // syntax error: assignment of read-only location ‘* pointer’
// Inc(&c99);                           // syntax error: increment of read-only location ‘* pointer’
   Print(&c99);                         // R-value, but note address of an L-value is an R-value

   const int* pi = &c99;                // syntax error if c99 declared volatile
                                        // error: invalid conversion from ‘const volatile int*’ to ‘const int*’ [-fpermissive]

   Print(&pi);                          // R-value, but note address of an L-value is an R-value
// Zap(pi);                             // syntax error: assignment of read-only location ‘* pointer’
// Inc(pi);                             // syntax error: increment of read-only location ‘* pointer’

   std::cout << "calling 'Zap'/'Inc' pointer functions with const override\n";
   Zap(const_cast<int*>(pi));           // compiles
   Print(c99);                          // L-value, prints 0, Zap(pi=&c99) cleared c99
   Inc(const_cast<int*>(pi));           // compiles
   Print(c99);                          // L-value, prints 1
   Inc(const_cast<int*>(&c99));         // compiles
   Print(c99);                          // L-value, prints 2

   Print( "... 99 ..." );               // L-value

   Print( std::string("... 99 ...") );  // R-value

   assert(c99 == 99);                   // expect assert failure, c99 is 2, but no failure, why?  Try declaring c99 volatile.
   Print(c99);                          // yes, prints 2
}

int main(int argc, char**argv)
{
  // what happened to the abort call generated by assert?
  // Was it received?

  auto signal_handler = [] (int signal)  -> void
    {
      if (signal == SIGABRT) {
        std::cerr << "SIGABRT received\n";
      } else {
        std::cerr << "Unexpected signal " << signal << " received\n";
      }
      std::_Exit(EXIT_FAILURE);
    };

  if( SIG_ERR == signal(SIGABRT, signal_handler) ) {
    std::cerr << "Setup failed\n";
    return EXIT_FAILURE;
  }

  try { 
    Work(); 
  } catch(std::exception& e) { 
    std::cerr << e.what() << "\n"; 
  } catch(...)               { 
    std::cerr << "nullptr ... throw\n"; 
  }
}
