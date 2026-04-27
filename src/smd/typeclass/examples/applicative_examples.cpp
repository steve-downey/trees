
#include <smd/tree/fix_tree.hpp>
#include <smd/tree/fix_tree_applicative.hpp>

using namespace smd::tree;

// 3f0c8d0e
int main(){
  auto f = FixTree<std::function<int(int)>>::leaf(
    [](int x){return x+1;});
  auto x = FixTree<int>::node(
    FixTree<int>::leaf(1),
    FixTree<int>::leaf(2));
  auto r = apply(f,x);
}
// 3f0c8d0e end
