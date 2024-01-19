#include <cstddef>
enum
{
  __ALIGN = 8
}; // 小型区块的上调边界
enum
{
  __MAX_BYTES = 128
}; // 小型区块的上限大小
enum
{
  __NFREELISTS = __MAX_BYTES / __ALIGN
}; // free-lists个数
template <bool threads, int inst>
class __default_alloc_template
{
private:
  // Really we should use static const int x = N
  // instead of enum { x = N }, but few compilers accept the former.
  static size_t ROUND_UP(size_t bytes)
  {
    // 将bytes上调至8的倍数
    return (((bytes) + __ALIGN - 1) & ~(__ALIGN - 1));
  }

  // free-lists 的节点结构
  union obj
  {
    union obj *free_list_link;
    char client_data[1]; /* The client sees this.        */
  };

private:
  static obj *__VOLATILE free_list[__NFREELISTS]; // 十六个free-lists

  // 以下函数根据区块大小，决定使用第n号于ree-list。n从1起算
  static size_t FREELIST_INDEX(size_t bytes)
  {
    return (((bytes) + __ALIGN - 1) / __ALIGN - 1);
  }

  // 返回一个大小为n的对象，并可能加入大小为n的其它区块到free list
  static void *refill(size_t n);
  // 配置一大块空间，可容纳nobjs个大小为 "size"的区块
  // 如果配置nobjs个区块有所不便，nobjs可能会降低
  static char *chunk_alloc(size_t size, int &nobjs);

  // Chunk allocation state.
  static char *start_free; // 内存池起始位置
  static char *end_free;   // 内存池结束位置
  static size_t heap_size;

public:
  /* n must be > 0 */
  static void *allocate(size_t n)
  {
    obj *__VOLATILE *my_free_list;
    obj *__RESTRICT result;

    // 首先，函数会判断n是否大于__MAX_BYTES，如果大于，则调用malloc_alloc的allocate函数来分配内存空间，并返回结果。

    if (n > (size_t)__MAX_BYTES)
    {
      return (malloc_alloc::allocate(n));
    }

    // 如果n小于等于__MAX_BYTES，则会根据n的大小计算出一个索引值，然后将free_list数组中对应索引位置的指针赋值给my_free_list。
    my_free_list = free_list + FREELIST_INDEX(n);

#ifndef _NOTHREADS
    /*REFERENCED*/
    lock lock_instance;
#endif
    // 然后，函数会将my_free_list指针指向的对象赋值给result。如果result为0，则表示对应的free_list链表为空，这时会调用refill函数来重新填充空间，并返回结果。
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
    // 如果n大于最大字节数__MAX_BYTES，则调用malloc_alloc的deallocate函数释放内存，并返回。
    if (n > (size_t)__MAX_BYTES)
    {
      malloc_alloc::deallocate(p, n);
      return;
    }
    // 根据n的大小计算出对应的自由链表索引，将my_free_list指向对应的自由链表。
    my_free_list = free_list + FREELIST_INDEX(n);
    // acquire lock
#ifndef _NOTHREADS
    /*REFERENCED*/
    lock lock_instance;
#endif /* _NOTHREADS */
    // 将q插入到自由链表中，即将q的free_list_link指向当前自由链表的头节点，然后将my_free_list指向q。
    q->free_list_link = *my_free_list;
    *my_free_list = q;
  }

  static void *reallocate(void *p, size_t old_sz, size_t new_sz);
};
// 以下是 static data member 的定義與初值設定
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