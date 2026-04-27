#ifndef INCLUDE_SMD_TREE_FIX_TREE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_HPP

#include <memory>
#include <variant>

namespace smd::tree {

template<class T>
class FixTree {
  struct Leaf { T value; };
  struct Node {
    std::shared_ptr<FixTree> l;
    std::shared_ptr<FixTree> r;
  };

  std::variant<Leaf,Node> d_;

public:
  static FixTree leaf(T v){ return FixTree(Leaf{v}); }
  static FixTree node(FixTree a, FixTree b){
    return FixTree(Node{
      std::make_shared<FixTree>(a),
      std::make_shared<FixTree>(b)
    });
  }

  bool is_leaf() const { return std::holds_alternative<Leaf>(d_); }
  const T& value() const { return std::get<Leaf>(d_).value; }
  const FixTree& left() const { return *std::get<Node>(d_).l; }
  const FixTree& right() const { return *std::get<Node>(d_).r; }

private:
  FixTree(Leaf l):d_(l){} 
  FixTree(Node n):d_(n){}
};

}

#endif
