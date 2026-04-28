#include <functional>
#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>
#include <smd/tree/zip_tree_applicative.hpp>
#include <smd/typeclass/applicative.hpp>

using namespace smd::tree;

int main(){
  auto f = FixTree<std::function<int(int)>>::leaf([](int x){return x+1;});
  auto x = FixTree<int>::node(FixTree<int>::leaf(1),FixTree<int>::leaf(2));

  auto r1 = smd::invoke<FixTree<int>>(f,x);
  auto r2 = smd::invoke_with<smd::zip_tree_map<int>>(f,x);
}
