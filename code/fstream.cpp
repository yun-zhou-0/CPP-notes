#include <fstream>
#include <iostream>
#include <string>
using namespace std;

class Person {
public:
  char name[40];
  int age;
};
int main() {
  ofstream ofs;
  ofs.open("test.txt", ios::out);
  ofs << "operator" << endl;
  ofs << "name:Mizuki" << endl;
  ofs << "gender:Male" << endl;
  ofs << "color:Blue" << endl;
  ofs.close();

  ifstream ifs;
  ifs.open("test.txt", ios::in);
  if (!ifs.is_open()) {
    cout << "文件打开失败！" << endl;
    return 0;
  }

  // 读取数据
  // 第一种

  // char buf[1024] = {0};
  // while (ifs >> buf)
  // {
  //   cout << buf << endl;
  // }

  // 第二种

  // char buf1[1024] = {0};
  // while (ifs.getline(buf1, sizeof(buf1)))
  // {
  //   cout << buf1 << endl;
  // }

  // 第三种

  // string a;
  // while (getline(ifs, a))
  // {
  //   cout << a << endl;
  // }

  // 第四种
  char c;
  while ((c = ifs.get()) != EOF) {
    cout << c;
  }

  ifs.close();

  // 二进制文件
  ofstream ofss("erjinzhi.txt", ios::out | ios::binary);
  // 可以直接使用构造函数在这里写好

  Person p = {"Mizuki", 18};
  ofss.write((const char *)&p, sizeof(Person));
  ofss.close();
  // 二进制读文件一般使用read
  ifstream ifss;
  ifss.open("erjinzhi.txt", ios::in | ios::binary);
  if (!ifss.is_open()) {
    cout << "文件打卡失败！" << endl;
    return 0;
  }
  Person pp;

  ifss.read((char *)&pp, sizeof(Person));
  cout << "----------------" << endl;
  cout << "Name: " << pp.name << " Age: " << pp.age << endl;
  ifss.close();
}