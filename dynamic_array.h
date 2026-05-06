#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <string.h>
#include <stdlib.h>

typedef struct
{
	size_t capacity;
	size_t count;
} _meta_header_t;

#if defined(DARRAY_NO_ASSERT)
#	define DARRAY_ASSERT(...)
#else
#	include <assert.h>
#   define DARRAY_ASSERT(_expr) assert(_expr)
#endif

#define DARRAY_DEFAULT_CAPACITY 16

// --- initialization --- //

#define darray_push_back(_arr, _val)                                                                      \
	do { 															                                      \
		if (_arr == NULL) { 									                                          \
			_meta_header_t* h = malloc(sizeof(*_arr) * DARRAY_DEFAULT_CAPACITY + sizeof(_meta_header_t)); \
			DARRAY_ASSERT(h != NULL && "failed to allocate memory.");			                          \
			h->capacity = DARRAY_DEFAULT_CAPACITY; 										                  \
			h->count = 0; 											                                      \
			_arr = (void*)(h + 1); 										                                  \
		} 																                                  \
		_meta_header_t* h = (_meta_header_t*)(_arr) - 1; 							                      \
		if (h->count >= h->capacity) { 									                                  \
			h->capacity *= 2; 											                                  \
			h = realloc(h, sizeof(*_arr) * h->capacity + sizeof(_meta_header_t));                         \
			DARRAY_ASSERT(h != NULL && "failed to reallocate memory."); 			                      \
			_arr = (void*)(h + 1); 										                                  \
		} 																	                              \
		_arr[h->count++] = _val; 										                                  \
	} while (0)

#define darray_reserve(_arr, _capacity) 				                                    \
	do {															                        \
		if (_arr == NULL && _capacity > 0) {									            \
			_meta_header_t* h = malloc(sizeof(*_arr) * _capacity + sizeof(_meta_header_t)); \
			DARRAY_ASSERT(h != NULL && "failed to allocate memory.");			            \
			h->capacity = _capacity; 										                \
			h->count = 0;													                \
			_arr = (void*)(h + 1);												            \
		}														                            \
	} while (0)

#define darray_free(_arr)                      \
	do {                                       \
		if (_arr != NULL) {                    \
			free((_meta_header_t*)(_arr) - 1); \
			_arr = NULL;                       \
		}                                      \
	} while (0)

// --- resizing --- //

#define darray_resize(_arr, _size) 				                            \
	do {															        \
		if (_arr != NULL && _size > 0 && _size != darray_capacity(_arr)) {  \
			_meta_header_t* h = (_meta_header_t*)(_arr) - 1;                \
			h = realloc(h, sizeof(*_arr) * _size + sizeof(_meta_header_t)); \
			DARRAY_ASSERT(h != NULL && "failed to reallocate memory.");	    \
			h->capacity = _size; 									        \
			_arr = (void*)(h + 1);									        \
		}														            \
	} while (0)

#define darray_resize_with_value(_arr, _size, _val) 				        \
	do {															        \
		if (_arr != NULL && _size > 0 && _size != darray_capacity(_arr)) {  \
			_meta_header_t* h = (_meta_header_t*)(_arr) - 1;                \
			h = realloc(h, sizeof(*_arr) * _size + sizeof(_meta_header_t)); \
			DARRAY_ASSERT(h != NULL && "failed to reallocate memory.");	    \
			h->capacity = _size; 									        \
			_arr = (void*)(h + 1);									        \
			for (size_t i = darray_count(_arr); i < _size; i++) {           \
				_arr[i] = _val;                                             \
			}                                                               \
			h->count = _size;                                               \
		}														            \
	} while (0)

#define darray_shrink_to_fit(_arr)                                             \
	do {                                                                       \
		if (_arr != NULL && !darray_is_empty(_arr)) {                          \
			_meta_header_t* h = (_meta_header_t*)(_arr) - 1;                   \
			h = realloc(h, sizeof(*_arr) * h->count + sizeof(_meta_header_t)); \
			DARRAY_ASSERT(h != NULL && "failed to reallocate memory.");        \
			h->capacity = h->count;                                            \
			_arr = (void*)(h + 1);                                             \
		}                                                                      \
	} while (0)

// --- erasing --- //

#define darray_erase(_arr, _pos)                                                                              \
	do {                                                                                                      \
		if (_arr != NULL && !darray_is_empty(_arr)                                                            \
				         && (_pos) < darray_count(_arr)                                                       \
				         && (_pos) >= 0) {                                                                    \
			if (((_pos) + 1) <= darray_end(_arr))                                                             \
				memmove(&_arr[(_pos)], &_arr[(_pos) + 1], sizeof(*_arr) * (darray_count(_arr) - (_pos) - 1)); \
			_meta_header_t* h = (_meta_header_t*)(_arr) - 1;                                                  \
			h->count--;                                                                                       \
		}                                                                                                     \
	} while (0)

#define darray_erase_if(_arr, _pos, _condition) \
	do {                                        \
		if (_condition)                         \
			darray_erase(_arr, _pos);           \
	} while (0)

#define darray_erase_range(_arr, _first, _last)                                                                     \
	do {                                                                                                            \
		if (_arr != NULL && !darray_is_empty(_arr)                                                                  \
					     && (_first) < (_last)                                                                      \
					     && (_first) >= 0 && (_last) > 0                                                            \
					     && (_first) < darray_count(_arr)                                                           \
					     && (_last) < darray_count(_arr)) {                                                         \
			ptrdiff_t diff = (&_arr[(_last)]) - (&_arr[(_first)]);                                                  \
			if ((_last) != (darray_end(_arr)))                                                                      \
				memmove(&_arr[(_first)], &_arr[(_last) + 1], sizeof(*_arr) * (darray_count(_arr) - ((_last)) - 1)); \
			_meta_header_t* h = (_meta_header_t*)(_arr) - 1;                                                        \
			h->count -= (diff) + 1;                                                                                 \
		}                                                                                                           \
	} while (0)

#define darray_erase_range_if(_arr, _first, _last, _condition) \
	do {                                                       \
		if (_condition)                                        \
			darray_erase_in_range(_arr, _first, _last);        \
	} while (0)

#define darray_clear(_arr)  (((_meta_header_t*)(_arr) - 1)->count = 0)

// --- inserting --- //

#define darray_insert(_arr, _pos, _val)                                                          \
	do {                                                                                         \
		if (_arr != NULL && !darray_is_empty(_arr)                                               \
				         && (_pos) < darray_count(_arr)                                          \
				         && (_pos) >= 0) {                                                       \
			_meta_header_t* h = (_meta_header_t*)(_arr) - 1;                                     \
			if ((darray_count(_arr) + 1) >= darray_capacity(_arr)) {                             \
				h->capacity *= 2;                                                                \
				h = realloc(h, sizeof(*_arr) * h->capacity + sizeof(_meta_header_t));            \
				DARRAY_ASSERT(h != NULL && "failed to reallocate memory.");                      \
				_arr = (void*)(h + 1);                                                           \
			}                                                                                    \
			if (((_pos) + 1) < (h->capacity))                                                    \
				memmove(&_arr[(_pos) + 1], &_arr[(_pos)], sizeof(*_arr) * (darray_count(_arr))); \
			_arr[(_pos)] = (_val);                                                               \
			h->count++;                                                                          \
		}                                                                                        \
	} while (0)

#define darray_insert_if(_arr, _pos, _val, _condition) \
	do {                                               \
		if (_condition)                                \
			darray_insert(_arr, _pos, _val);           \
	} while (0)

#define darray_insert_range(_arr, _first, _last, _val)                                              \
	do {                                                                                            \
		if (_arr != NULL && !darray_is_empty(_arr)                                                  \
					     && (_first) < (_last)                                                      \
					     && (_first) >= 0 && (_last) > 0                                            \
					     && (_first) < darray_count(_arr)                                           \
					     && (_last) < darray_count(_arr)) {                                         \
			_meta_header_t* h = (_meta_header_t*)(_arr) - 1;                                        \
			if ((darray_count(_arr) + ((_first) + (_last))) >= darray_capacity(_arr)) {             \
				h->capacity *= 2;                                                                   \
				h = realloc(h, sizeof(*_arr) * h->capacity + sizeof(_meta_header_t));               \
				DARRAY_ASSERT(h != NULL && "failed to reallocate memory.");                         \
				_arr = (void*)(h + 1);                                                              \
			}                                                                                       \
			if (((_first) + (_last)) < (h->capacity))                                               \
				memmove(&_arr[(_last) + 1], &_arr[(_first)], sizeof(*_arr) * (darray_count(_arr))); \
 			for (size_t i = (_first); i <= (_last); i++) {                                          \
 				_arr[i] = (_val);                                                                   \
 			}	                                                                                    \
			ptrdiff_t diff = (&_arr[(_last)]) - (&_arr[(_first)]);                                  \
			h->count += (diff) + 1;                                                                 \
		}                                                                                           \
	} while (0)

#define darray_insert_range_if(_arr, _first, _last, _val, _condition) \
	do {                                                              \
		if (_condition)                                               \
			darray_insert_range(_arr, _first, _last, _val);           \
	} while (0)

// --- helpers --- //

#define darray_count(_arr)     (((_meta_header_t*)(_arr) - 1)->count)
#define darray_capacity(_arr)  (((_meta_header_t*)(_arr) - 1)->capacity)

#define darray_is_empty(_arr)  (darray_count(_arr) == 0 && darray_capacity(_arr) == 0)

#define darray_begin(_arr)     0
#define darray_end(_arr)       ((darray_count(_arr) > 0 ? darray_count(_arr) : 0))

#define darray_first(_arr)     0
#define darray_last(_arr)      ((darray_count(_arr) > 0 ? darray_count(_arr) - 1 : 0))

#endif // DYNAMIC_ARRAY_H
