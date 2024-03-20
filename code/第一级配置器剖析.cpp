#if 0
#include <new>
#define __THROW_BAD_ALLOC throw bad_alloc
#elif !defined(__THROW_BAD_ALLOC)

#include <iostream.h>
#define __THROW_BAD_ALLOC                                                      \
  cerr << "out of memory" << endl;                                             \
  exit(1)
#endif

// malloc-based allocator. ͨ���������B�� default alloc �ٶ�����
// һ������� thread-safe���K�Ҍ�춿��g���\�ñ��^��Ч��efficient����
// �����ǵ�һ����������
// ע�⣬�o��template�̈́e����������춡����̈́e������inst����ȫ�]�����È���
template <int inst> class __malloc_alloc_template {

private:
  // ���¶��Ǻ�ʽָ�ˣ�������ĺ�ʽ���Á�̎��ӛ���w�������r��
  // oom : out of memory.
  static void *oom_malloc(size_t);
  static void *oom_realloc(void *, size_t);
  static void (*__malloc_alloc_oom_handler)();

public:
  static void *allocate(size_t n) {
    void *result = malloc(n);
    //  ��һ��������ֱ��ʹ��malloc���ڴ治��ʱʹ��oom_malloc
    if (0 == result)
      result = oom_malloc(n);
    return result;
  }

  static void deallocate(void *p, size_t /* n */) {
    free(p);
  } //  ��һ��������ֱ��ʹ��free

  static void *reallocate(void *p, size_t /* old_sz */, size_t new_sz) {
    void *result = realloc(p, new_sz);
    //  ��һ��������ֱ��ʹ��realloc���ڴ治��ʱʹ��oom_realloc
    if (0 == result)
      result = oom_realloc(p, new_sz);
    return result;
  }
  // ����ģ�M C++ �� set_new_handler(). �Q��Ԓ�f�������͸�^����ָ�����Լ���
  // out-of-memory handler
  static void (*set_malloc_handler(void (*f)()))() {
    void (*old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = f;
    return (old);
  }
};

// malloc_alloc out-of-memory handling

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
template <int inst>
void (*__malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;
#endif

template <int inst> void *__malloc_alloc_template<inst>::oom_malloc(size_t n) {
  void (*my_malloc_handler)();
  void *result;

  for (;;) { // ���ϳ����ͷţ����õĹ���
    my_malloc_handler = __malloc_alloc_oom_handler;
    if (0 == my_malloc_handler) {
      __THROW_BAD_ALLOC; // ������ڴ治�㴦�����̡���û��ʵ�֣����ֱ���׳��쳣������ֹ����
    }
    (*my_malloc_handler)(); // ���ô�����̣���ͼ�ͷ��ڴ�
    result = malloc(n);     // �ٴγ����ͷ��ڴ�
    if (result)
      return (result);
  }
}

template <int inst>
void *__malloc_alloc_template<inst>::oom_realloc(void *p, size_t n) {
  void (*my_malloc_handler)();
  void *result;

  for (;;) {
    my_malloc_handler = __malloc_alloc_oom_handler;
    if (0 == my_malloc_handler) {
      __THROW_BAD_ALLOC;
    }
    (*my_malloc_handler)();
    result = realloc(p, n);
    if (result)
      return (result);
  }
}

typedef __malloc_alloc_template<0> malloc_alloc;