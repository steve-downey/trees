# Examples of Transclusion


## Declaration of the `name` function

```cpp
namespace example {
  std::string_view name();
}
```


## Definition of the `name` function

```cpp
std::string_view example::name() {
  static std::string my_name{"Steve"};
  return my_name;
}
```


## Testing the `name` function

```cpp
TEST(TestName, Steve) { ASSERT_EQ(example::name(), "Steve"); }
```


## Saying hello with the `name` function

```cpp
int main() { std::println("Hello, {}!", example::name()); }
```


## The Transclusion for the example above

```org
#+transclude: [[file:hello.cpp::710c39c6-c7e1-403f-a9f0-9f8ecf890dc9]] :lines 2- :src cpp :end "710c39c6-c7e1-403f-a9f0-9f8ecf890dc9 end"
```
