# STL六大组件综述

**contianer**：容器，一种class template

**algorithm**：算法，一种function template

**任何一个算法所标识的区间都是一个前闭后开的区间**

**iterator**：迭代器，泛型指针，主要对operator*，operator->，operator++，operator--进行了重载，是一种class template

**functor**：仿函数，一种重载了operator()的class或class template

<a href="./code/easy_functor.cpp">一个简易的functor实例</a>

**adapter**：配接器，用来修饰container或functor或iterator接口的东西

**allocator**：配置器，负责空间配置和管理

# 空间配置器——Allocator

## Allocator的必要接口

<img src='./img/allocator_1.jpg'>

<img src='./img/allocator_2.jpg'>

## 一般设计的allocator

1.   allocator主要是对operator new和operator delete的包装
2.   allocate负责内存配置，deallocate负责内存释放。construct负责构造对象，destory负责对象析构

## new和delete的原理

### new

1.   调用::operator new配置内存
2.   调用构造函数构造对象内容

### delete

1.   调用析构函数
2.   调用operator delete释放内存

## SGI的分配器

SGI没有使用标准的STL分配器，而是自己书写的Alloc分配器

<img src='./img/allocator_3.jpg'>



### 对象的构造与析构——construct()和destory()

<img src='./img/conde1.jpg'>



#### __destory函数

判断是否拥有trivial destructor（默认构造函数），如果没有说明应该存在一些手动的内存释放，需要调用destory函数

```C++
// 判斷元素的數值型別（value type）是否有 trivial destructor
template <class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T*) 
{
	typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor; 
  	__destroy_aux(first, last, trivial_destructor());
}
// 如果元素的數值型別（value type）有 non-trivial destructor…
template <class ForwardIterator> inline void
__destroy_aux(ForwardIterator first, ForwardIterator last, __false_type) 
{   
    for ( ; first < last; ++first)
        destroy(&*first); 
}
// 如果元素的數值型別（value type）有 trivial destructor…
template <class ForwardIterator>
inline void __destroy_aux(ForwardIterator, ForwardIterator, __true_type) {}
```

使用trivial destructor的元素在使用destory函数（传入首位迭代器的版本）是并没有调用析构函数，因为如果这个范围很大，调用多次析构函数效率很低。

### 空间的配置与释放——std::alloc

考虑到小型区块可能造成内存破碎问题，SGI设计了双层级配置器。

第一级 配置器直接使用 malloc() 和 free()，

第二级配置器则视情况采用不同的策略： 当配置区块超过 128bytes，视之为**「足够大」**，便呼叫**第一级配置器**；当配置区 块小于 128bytes，视之为**「过小」**，为了降低额外负担， 便采用复杂的 **memory pool **整理方式，而不再求助于第一级配置器。

其中\_\_malloc_alloc_template 就是第一级配置器，\_\_default_alloc_ template 就是第二级配置器。

<img src='./img/alloc1.jpg'>

### 第一级配置器 __malloc_alloc_template

**<a href="./code/第一级配置器剖析.cpp">第一级配置器剖析代码</a>**

第一级配置器使用了malloc()，free()，realloc()来执行内存的配置，释放和重配置。实现了类似C++ new-handler机制

所谓C++ new handler机制是，你可以要求系统在内存配置需求无法被满足时，调用一个你所指定的函数。换句话说，一旦::operator new无法完成任务，在丢出std::bad_alloc异常状态之前，会先调用由客端指定的处理例程。该处理例程通常即被称为new-handler。

### 第二级配置器 __deafault_alloc_template

区块小于128bytes时，就使用内存池管理。第二级配置器将小额区块上调至8的倍数，并维护16个free-lists，各自管理大小为8*n(1~16)的小额区块。

free-lists使用union以共享空间

```C++
union obj{
    union obj* free_list_link;
    char client_data[1];	// the client see this.
}
```

<img src='./img/freelist.jpg'>

**<a href="./code/第二级配置器剖析.cpp">第二级配置器剖析代码</a>**

#### 空间配置函数allocate()

```C++
  static void *allocate(size_t n) {
    obj *__VOLATILE *my_free_list;
    obj *__RESTRICT result;

    // 首先，函数会判断n是否大于__MAX_BYTES，如果大于，则调用malloc_alloc的allocate函数来分配内存空间，并返回结果。

    if (n > (size_t)__MAX_BYTES) {
      return (malloc_alloc::allocate(n));
    }

    // 如果n小于等于__MAX_BYTES，则会根据n的大小计算出一个索引值，然后将free_list数组中对应索引位置的指针赋值给my_free_list。
    my_free_list = free_list + FREELIST_INDEX(n);

    // 然后，函数会将my_free_list指针指向的对象赋值给result。如果result为0，则表示对应的free_list链表为空，这时会调用refill函数来重新填充空间，并返回结果。
    result = *my_free_list;
    if (result == 0) {
      void *r = refill(ROUND_UP(n));
      return r;
    }
    // 最后，如果free_list链表不为空，则将result指针指向的对象从链表中取出，并将其下一个节点赋值给my_free_list指针。 
    *my_free_list = result->free_list_link;
    return (result);
  };
```

<img src='./img/freelist1.jpg'>

#### 空间释放函数deallocate()

```C++
  /* p may not be 0 */
  static void deallocate(void *p, size_t n) {
    obj *q = (obj *)p;
    obj *__VOLATILE *my_free_list;
    // 如果n大于最大字节数__MAX_BYTES，则调用malloc_alloc的deallocate函数释放内存，并返回。
    if (n > (size_t)__MAX_BYTES) {
      malloc_alloc::deallocate(p, n);
      return;
    }
    // 根据n的大小计算出对应的自由链表索引，将my_free_list指向对应的自由链表。
    my_free_list = free_list + FREELIST_INDEX(n);
    // 将q插入到自由链表中，即将q的free_list_link指向当前自由链表的头节点，然后将my_free_list指向q。
      //调整freelist，回收区块
    q->free_list_link = *my_free_list;
    *my_free_list = q;
  }
```

<img src='./img/freelist12.jpg'>

#### 重新填充free lists——refill()

```C++
template <bool threads, int inst>
void *__default_alloc_template<threads, inst>::refill(size_t n) {
  // 需要分配的内存块数量
  int nobjs = 20;
  char *chunk = chunk_alloc(n, nobjs);
  obj *__VOLATILE *my_free_list;
  obj *result;
  obj *current_obj, *next_obj;
  int i;
  // 如果只获得一个区块，这个区块就分配给调用者用，free list无新节点
  if (1 == nobjs)
    return (chunk);
  // 否则准备调整free list，纳入新节点
  my_free_list = free_list + FREELIST_INDEX(n);

  /* Build free list in chunk */
  result = (obj *)chunk;
  // 以下导引free list指向新配置的空间
  *my_free_list = next_obj = (obj *)(chunk + n);
  // 将free list中的节点串接起来，形成一个链表
  for (i = 1;; i++) {
    current_obj = next_obj;
    next_obj = (obj *)((char *)next_obj + n);
    if (nobjs - 1 == i) {
      current_obj->free_list_link = 0;
      break;
    } else {
      current_obj->free_list_link = next_obj;
    }
  }
  return (result);
}
```

#### 从内存池取得空间——chunk_alloc()

```C++
template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::chunk_alloc(size_t size, int &nobjs)
{
  char *result;
  // 计算需要分配的总字节数
  size_t total_bytes = size * nobjs;
  // 计算剩余可用内存的字节数
  size_t bytes_left = end_free - start_free;

  if (bytes_left >= total_bytes)
  {
    // 内存池剩余空间完全满足需求量
    result = start_free;
    start_free += total_bytes;
    return (result);
  }
  else if (bytes_left >= size)
  {
    // 内存池足够分配一个及以上的区块
    nobjs = bytes_left / size; // 重新计算nobjs，看能分配多少区块
    total_bytes = size * nobjs;
    result = start_free;
    start_free += total_bytes;
    return (result);
  }
  else
  {
    // 内存池空间连一个区块大小都无法提供
    size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
    // Try to make use of the left-over piece.
    // 发挥内存池剩余零头空间的剩余价值
    if (bytes_left > 0)
    {
      // 内存池剩下的零头先分配给free list
      // 首先寻找合适的free list
      obj *__VOLATILE *my_free_list = free_list + FREELIST_INDEX(bytes_left);

      // 调整free list，将内存池的剩余空间编入
      ((obj *)start_free)->free_list_link = *my_free_list;
      *my_free_list = (obj *)start_free;
    }

    // 调用malloc函数获取更多的内存以扩充内存池，配置heap空间
    start_free = (char *)malloc(bytes_to_get);
    if (0 == start_free)
    {
      // heap空间不足，malloc失败
      int i;
      obj *__VOLATILE *my_free_list, *p;
      // Try to make do with what we have.  That can't hurt.  We do not try smaller requests, since that tends to result in disaster on multi-process machines.
      // 尝试检视我们手上的东西，这不会造成伤害。我们不会尝试配置更小的区块，因为这将在多线程机器上造成灾难

      // 以下搜寻适当的free list，即尚有未用区块且区块够大的free list
      for (i = size; i <= __MAX_BYTES; i += __ALIGN)
      {
        my_free_list = free_list + FREELIST_INDEX(i);
        p = *my_free_list;
        // 如果有未用区块，调整free list以释放出来
        if (0 != p)
        {
          *my_free_list = p->free_list_link;
          start_free = (char *)p;
          end_free = start_free + i;
          return (chunk_alloc(size, nobjs));
          // 递归调用自己修正nobjs
          //  Any leftover piece will eventually make it to the
          //  right free list.
        }
      }
      end_free = 0;                                              // In case of exception.
      start_free = (char *)malloc_alloc::allocate(bytes_to_get); // 最后的希望——out-of-memory机制
      // This should either throw an exception or remedy the situation.  Thus we assume it succeeded.
      // 这应该抛出异常或纠正这种情况。因此，我们假定它成功了
    }
    heap_size += bytes_to_get;
    end_free = start_free + bytes_to_get;
    // 递归调用自己修正nobjs
    return (chunk_alloc(size, nobjs));
  }
}
```

<img src='./img/memorypool.jpg'>

假设一开始调用chunk_alloc(32,20)，malloc()配置40个32bytes区块，第一个传回给客端，剩下19个交给free_list[3]维护，余下20个给内存池。

当再次调用chunk_alloc(64,20)时，free_list[7]空空如也，需要向内存池要求支持。内存池返回10个区块，一个交给客端，余下9个交给free_listy[7]维护。

再次调用chunk_alloc(96,20)，内存池也空了，需要用malloc配置新的区块扩充内存池

## 内存基本处理工具

####uninitialized_copy

```C++
template <class InputIterator, class ForwardIterator> ForwardIterator 
uninitialized_copy(InputIterator first, InputIterator last,ForwardIterator result);
```

针对输入范围内每个迭代器i，此函数会呼叫construct，产生\*i的复制品

uninitialized_copy使得我们能够将内存的配置和对象构造行为分开来

#### uninitialized_fill

```C++
template <class ForwardIterator, class T>
void uninitialized_fill(ForwardIterator first, ForwardIterator last,const T& x);
```

uninitialized_fill会针对操作范围内的每个迭代器i ,调用construct(&*i, x),    在i所指之处产生x的复制品

#### uninitialized_fill_n

```C++
template <class ForwardIterator, class Size, class T> ForwardIterator
uninitialized_fill_n(ForwardIterator first, Size n, const T& x);
```

同上，为指定范围内的所有元素设定处置，调用copy constructor

<img src='./img/memory.jpg'>

**三个记忆基本函数的泛型版本与特化版本**