#include <cstddef>
enum
{
  __ALIGN = 8
}; // С��������ϵ��߽�
enum
{
  __MAX_BYTES = 128
}; // С����������޴�С
enum
{
  __NFREELISTS = __MAX_BYTES / __ALIGN
}; // free-lists����
template <bool threads, int inst>
class __default_alloc_template
{
private:
  // Really we should use static const int x = N
  // instead of enum { x = N }, but few compilers accept the former.
  static size_t ROUND_UP(size_t bytes)
  {
    // ��bytes�ϵ���8�ı���
    return (((bytes) + __ALIGN - 1) & ~(__ALIGN - 1));
  }

  // free-lists �Ľڵ�ṹ
  union obj
  {
    union obj *free_list_link;
    char client_data[1]; /* The client sees this.        */
  };

private:
  static obj *__VOLATILE free_list[__NFREELISTS]; // ʮ����free-lists

  // ���º������������С������ʹ�õ�n����ree-list��n��1����
  static size_t FREELIST_INDEX(size_t bytes)
  {
    return (((bytes) + __ALIGN - 1) / __ALIGN - 1);
  }

  // ����һ����СΪn�Ķ��󣬲����ܼ����СΪn���������鵽free list
  static void *refill(size_t n);
  // ����һ���ռ䣬������nobjs����СΪ "size"������
  // �������nobjs�������������㣬nobjs���ܻή��
  static char *chunk_alloc(size_t size, int &nobjs);

  // Chunk allocation state.
  static char *start_free; // �ڴ����ʼλ��
  static char *end_free;   // �ڴ�ؽ���λ��
  static size_t heap_size;

public:
  /* n must be > 0 */
  static void *allocate(size_t n)
  {
    obj *__VOLATILE *my_free_list;
    obj *__RESTRICT result;

    // ���ȣ��������ж�n�Ƿ����__MAX_BYTES��������ڣ������malloc_alloc��allocate�����������ڴ�ռ䣬�����ؽ����

    if (n > (size_t)__MAX_BYTES)
    {
      return (malloc_alloc::allocate(n));
    }

    // ���nС�ڵ���__MAX_BYTES��������n�Ĵ�С�����һ������ֵ��Ȼ��free_list�����ж�Ӧ����λ�õ�ָ�븳ֵ��my_free_list��
    my_free_list = free_list + FREELIST_INDEX(n);

#ifndef _NOTHREADS
    /*REFERENCED*/
    lock lock_instance;
#endif
    // Ȼ�󣬺����Ὣmy_free_listָ��ָ��Ķ���ֵ��result�����resultΪ0�����ʾ��Ӧ��free_list����Ϊ�գ���ʱ�����refill�������������ռ䣬�����ؽ����
    result = *my_free_list;
    if (result == 0)
    {
      void *r = refill(ROUND_UP(n));
      return r;
    }
    *my_free_list = result->free_list_link;
    return (result);
  };

  /* p may not be 0 */
  static void deallocate(void *p, size_t n)
  {
    obj *q = (obj *)p;
    obj *__VOLATILE *my_free_list;
    // ���n��������ֽ���__MAX_BYTES�������malloc_alloc��deallocate�����ͷ��ڴ棬�����ء�
    if (n > (size_t)__MAX_BYTES)
    {
      malloc_alloc::deallocate(p, n);
      return;
    }
    // ����n�Ĵ�С�������Ӧ������������������my_free_listָ���Ӧ����������
    my_free_list = free_list + FREELIST_INDEX(n);
    // acquire lock
#ifndef _NOTHREADS
    /*REFERENCED*/
    lock lock_instance;
#endif /* _NOTHREADS */
    // ��q���뵽���������У�����q��free_list_linkָ��ǰ���������ͷ�ڵ㣬Ȼ��my_free_listָ��q��
    q->free_list_link = *my_free_list;
    *my_free_list = q;
  }

  static void *reallocate(void *p, size_t old_sz, size_t new_sz);
};
// ������ static data member �Ķ��x�c��ֵ�O��
template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::start_free = 0;
template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::end_free = 0;
template <bool threads, int inst>
size_t __default_alloc_template<threads, inst>::heap_size = 0;
template <bool threads, int inst>
__default_alloc_template<threads, inst>::obj *volatile __default_alloc_template<
    threads, inst>::free_list[__NFREELISTS] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};