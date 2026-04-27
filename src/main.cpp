
#include <functional>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/zip_list.hpp>
#include <smd/typeclass/zip_list_applicative.hpp>

using namespace smd;

int main(){
  zip_list<std::function<int(int)>> f{{[](int x){return x+1;}}};
  zip_list<int> xs{{1,2,3}};
  auto r = invoke<zip_list<int>>(f,xs);
}
