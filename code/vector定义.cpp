template<class T, class Alloc = alloc>
class vector
{
public:
  // vectorǶ���ͱ���
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type* iterator;
  typedef value_type& reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

protected:
  typedef simple_alloc<value_type, Alloc> data_allocator;
  iterator start;          // ��ʾĿǰʹ�ÿռ��ͷ
  iterator finish;         // ��ʾĿǰʹ�ÿռ��β
  iterator end_of_storage; // ��ʾĿǰ���ÿռ��β

  void insert_aux(iterator position, const T& x);
  void deallocate()
  {
    if (start)
      data_allocator::deallocate(start, end_of_storage - start);
  }

  void fill_initialize(size_type n, const T& value)
  {
    start = allocate_and_fill(n, value);
    finish = start + n;
    end_of_storage = finish;
  }

public:
  iterator begin() { return start; }
  iterator end() { return finish; }
  // vector�Ĵ�С������Ԫ�صĸ�����
  size_type size() const { return size_type(end() - begin()); }
  // vector������
  size_type capacity() const { return size_type(end_of_storage - begin()); }
  bool empty() const { return begin() == end(); }
  reference operator[](size_type n) { return *(begin() + n); }

  // �����ǹ��캯��
  vector()
    : start(0)
    , finish(0)
    , end_of_storage(0)
  {
  }
  vector(size_type n, const T& value) { fill_initialize(n, value); }
  vector(int n, const T& value) { fill_initialize(n, value); }
  vector(long n, const T& value) { fill_initialize(n, value); }
  explicit vector(size_type n) { fill_initialize(n, T()); }

  // ��������
  ~vector()
  {
    destroy(start, finish);
    deallocate();
  }

  reference front() { return *begin(); }    // ��һ��Ԫ��
  reference back() { return *(end() - 1); } // ���һ��Ԫ��

  void push_back(const T& x) // ��Ԫ�ز���β��
  {
    if (finish != end_of_storage) {
      construct(finish, x);
      ++finish;
    } else
      insert_aux(end(), x);
  }
  void pop_back() // ��β��Ԫ��ȡ��
  {
    --finish;
    destroy(finish);
  }
  iterator erase(iterator position) // ���ĳλ���ϵ�Ԫ��
  {
    if (position + 1 != end())
      copy(position + 1, finish, position); // ����Ԫ����ǰ�ƶ�
    --finish;
    destroy(finish);
    return position;
  }
  void resize(size_type new_size, const T& x)
  {
    if (new_size < size())
      erase(begin() + new_size, end());
    else
      insert(end(), new_size - size(), x);
  }
  void resize(size_type new_size) { resize(new_size, T()); }
  void clear() { erase(begin(), end()); }

protected:
  // ���ÿռ䲢��������
  iterator allocate_and_fill(size_type n, const T& x)
  {
    iterator result = data_allocator::allocate(n);
    uninitialized_fill_n(result, n, x);
    return result;
  }
};