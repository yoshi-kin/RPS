#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <bitset>
#include <fstream>
#include <string>
#include <memory>
#include <iterator>
#include "cfr_plus.hpp"

// #include <cereal/cereal.hpp>
// #include <cereal/types/string.hpp>
// #include <cereal/archives/binary.hpp>
// #include "tree.hpp"
// #include <boost/type_index.hpp>

// #include <boost/serialization/serialization.hpp>
// #include <boost/serialization/nvp.hpp>
// #include <boost/serialization/string.hpp>
// #include <boost/archive/text_oarchive.hpp>
// #include <boost/archive/text_iarchive.hpp>
// #include <boost/archive/binary_iarchive.hpp>
// #include <boost/archive/binary_oarchive.hpp>


void trainRPS(int iterations) {
  Game game;
  CFR cfr(game);
  for (int i = 0; i < iterations; ++i) {
    cfr.minimize(game);
  }

  for (auto strategy : cfr.getSumStrategyArray()) {
    double normalizing_sum = 0. ;
    for (auto& [action, probability] : strategy) {
      normalizing_sum += probability;
    }
    for (auto& [action, probability] : strategy) {
      std::cout << action << " : " << probability / normalizing_sum << std::endl;
    }
  }
}


int main() {
  trainRPS(1000);
  return 0;
}