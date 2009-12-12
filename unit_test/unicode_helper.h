/** 
 *  ==============================================================================
 * 
 *          \file   unicode_helper.h
 *
 *        \author   chenghuige at gmail.com 
 *
 *          \date   2009-12-10 12:36:22.674313
 *  
 *   Description:
 *
 *  ==============================================================================
 */

#include <iostream>
#include <gtest/gtest.h> 
#include <boost/progress.hpp>
using namespace std;

int main(int argc, char *argv[])
{
  //boost::progress_timer timer;
  
  //testing::GTEST_FLAG(output) = "xml:";
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
