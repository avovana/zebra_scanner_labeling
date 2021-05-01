/*
 * ©2015 Symbol Technologies LLC. All rights reserved.
 */


#ifndef MAIN_H_
#define MAIN_H_

/* 
 C++11X support Dual ABI issue 
*/
#if __GNUC__ >= 5
#define _GLIBCXX_USE_CXX11_ABI 0
#endif


#include <iostream>
#include <unistd.h>

using namespace std;


#endif /* MAIN_H_ */
