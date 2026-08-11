/**
 * \file
 * Main function comes here.
 */
/****************************************************
 * Author: Edmundo LOPEZ
 * email:  lopezed5@etu.unige.ch
 *
 * **************************************************/

#include <hello.hh>
#include <iostream>

int main (int argc, char *argv[])
  {
    Hello h;
    std::cout << h.foo(2,3) << std::endl;
  }
