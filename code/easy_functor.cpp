#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

template <typename T> class print {
public:
  void operator()(const T &elem) { cout << elem << ' '; }
};

int main() {
  int a[6] = {0, 1, 2, 3, 4, 5};
  vector<int> v(a, a + 6);
  for_each(v.begin(), v.end(), print<int>());
  // print<int>是一个临时对象，不是一个函数调用操作
}