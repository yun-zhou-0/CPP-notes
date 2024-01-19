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
    cout << "�ļ���ʧ�ܣ�" << endl;
    return 0;
  }

  // ��ȡ����
  // ��һ��

  // char buf[1024] = {0};
  // while (ifs >> buf)
  // {
  //   cout << buf << endl;
  // }

  // �ڶ���

  // char buf1[1024] = {0};
  // while (ifs.getline(buf1, sizeof(buf1)))
  // {
  //   cout << buf1 << endl;
  // }

  // ������

  // string a;
  // while (getline(ifs, a))
  // {
  //   cout << a << endl;
  // }

  // ������
  char c;
  while ((c = ifs.get()) != EOF) {
    cout << c;
  }

  ifs.close();

  // �������ļ�
  ofstream ofss("erjinzhi.txt", ios::out | ios::binary);
  // ����ֱ��ʹ�ù��캯��������д��

  Person p = {"Mizuki", 18};
  ofss.write((const char *)&p, sizeof(Person));
  ofss.close();
  // �����ƶ��ļ�һ��ʹ��read
  ifstream ifss;
  ifss.open("erjinzhi.txt", ios::in | ios::binary);
  if (!ifss.is_open()) {
    cout << "�ļ���ʧ�ܣ�" << endl;
    return 0;
  }
  Person pp;

  ifss.read((char *)&pp, sizeof(Person));
  cout << "----------------" << endl;
  cout << "Name: " << pp.name << " Age: " << pp.age << endl;
  ifss.close();
}