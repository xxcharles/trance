
#include <iostream>

#include "tree.hpp"
#include "forest.hpp"

int main(int argc, char** argv)
{
  rnnp::Tree tree;
  rnnp::Forest forest;
  
  while (std::cin >> tree) {
    forest.assign(tree);
    
    std::cout << forest << std::endl;
  }
}
