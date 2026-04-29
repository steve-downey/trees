#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>
#include <smd/typeclass/applicative.hpp>

using namespace smd::tree;

int main(){
  auto f = FixTree<std::function<int(int)>>::leaf(
    [](int x){return x+1;});
  auto x = FixTree<int>::node(
    FixTree<int>::leaf(1),
    FixTree<int>::leaf(2));

  auto r = smd::map<smd::applicative_tag, decltype(x)>::apply(f,x);
}
