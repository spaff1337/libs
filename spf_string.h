#ifndef SPF_STRING_H
#define SPF_STRING_H

#include <stddef.h>

#define SPFS_FMT "%.*s"
#define SPFS_ARGS(spfs) (int)(spfs).length, (__string_is_valid(&spfs) ? ((__string_is_sso(&spfs) ? (spfs).sso : (spfs).data)) : "")

#if defined(SPFS_NO_ASSERT)
#	define SPFS_ASSERT(...)
#else
#	include <assert.h>
#   define SPFS_ASSERT(_expr) assert(_expr)
#endif

#define SSO_MAX_LENGTH 16

typedef struct
{
	char* data;    // not used for short strings
	size_t length; // shared between long and short strings

	union {
		size_t capacity;          // overwritten by 'sso' if string is short
		char sso[SSO_MAX_LENGTH]; // overwritten by 'capacity' if string is long
	};
} spf_string;

#define spf_string(str) string_create(str)

#define string_append(spf_str, str) _Generic((str), \
	spf_string*: string_append_spfstr,              \
	const char*: string_append_str,                 \
	char*: string_append_str,                       \
	char: string_append_char,                       \
	int: string_append_char                         \
) ((spf_str), (str))

#define string_compare(str1, str2) _Generic((str2), \
	spf_string*: string_compare_spfstr,             \
	const char*: string_compare_cstr,               \
	char*: string_compare_cstr                      \
) ((str1), (str2))

#ifdef SPF_STRING_IMPL

#include <stdlib.h>
#include <string.h>

_Bool __string_is_sso(spf_string* spf_str)
{
	return spf_str->data == NULL && spf_str->length < SSO_MAX_LENGTH;
}

_Bool __string_is_valid(spf_string* spf_str)
{
	if (spf_str == NULL) return 0;
	if (!__string_is_sso(spf_str) && spf_str->data == NULL) return 0;
	return 1;
}

char* __string_data(spf_string* spf_str, _Bool null_terminate)
{
	if (spf_str->data != NULL) {
		if (null_terminate)
			spf_str->data[spf_str->length] = '\0';
		return spf_str->data;
	}
	else {
		if (null_terminate)
			spf_str->sso[spf_str->length] = '\0';
		return spf_str->sso;
	}
}

spf_string string_create(const char* str)
{
	size_t len = strlen(str);
	if (len <= 0) return (spf_string){0};

	spf_string spf_str;
	spf_str.length = len;

	if (len >= SSO_MAX_LENGTH) {
		spf_str.data = malloc(sizeof(char) * len + 1);
		SPFS_ASSERT(spf_str.data != NULL);

		strncpy(spf_str.data, str, sizeof(char) * len);
		spf_str.capacity = len + 1;
	}
	else {
		strncpy(spf_str.sso, str, sizeof(char) * len);
	}

	return spf_str;
}

void string_reserve(spf_string* spf_str, size_t capacity)
{
	if (capacity == 0 && capacity < SSO_MAX_LENGTH) return;

	if (capacity >= SSO_MAX_LENGTH) {
		spf_str->data = malloc(sizeof(char) * capacity + 1);
		SPFS_ASSERT(spf_str->data != NULL);

		spf_str->capacity = capacity + 1;
	}

	return;
}

void string_destroy(spf_string* spf_str)
{
	if (!__string_is_valid(spf_str)) return;
	if (!__string_is_sso(spf_str)) {
		free(spf_str->data);

		spf_str->data     = NULL;
		spf_str->length   = 0;
		spf_str->capacity = 0;
	}
}

void string_clear(spf_string* spf_str)
{
	if (!__string_is_valid(spf_str)) return;
	if (spf_str->length == 0) return;

	memset(__string_data(spf_str, 0), 0, sizeof(char) * spf_str->length);
	spf_str->length = 0;
}

void string_append_char(spf_string* spf_str, char c)
{
	if (!__string_is_valid(spf_str)) return;

	if (__string_is_sso(spf_str)) {
		// string too big start allocating on the heap
		if ((spf_str->length + 1) >= SSO_MAX_LENGTH) {
			spf_str->data = malloc(sizeof(char) * ((SSO_MAX_LENGTH * 2) + 1)); // +1 for null terminator
			SPFS_ASSERT(spf_str->data != NULL);

			strncpy(spf_str->data, spf_str->sso, sizeof(char) * SSO_MAX_LENGTH);

			spf_str->capacity = (SSO_MAX_LENGTH * 2) + 1;
			spf_str->data[spf_str->length++] = c;
		}
		else
			spf_str->sso[spf_str->length++] = c;
	}
	else {
		if ((spf_str->length + 1) >= spf_str->capacity) {
			spf_str->capacity *= 2;
			spf_str->data = realloc(spf_str->data, sizeof(char) * (spf_str->capacity + 1)); // +1 for null terminator
			SPFS_ASSERT(spf_str->data != NULL);

			spf_str->data[spf_str->length++] = c;
		}
		else
			spf_str->data[spf_str->length++] = c;
	}
}

void string_append_spfstr(spf_string* spf_str, spf_string* spf_str_append)
{
	if (!__string_is_valid(spf_str)) return;
	if (!__string_is_valid(spf_str_append)) return;
	if (spf_str_append->length == 0) return;

	if (__string_is_sso(spf_str)) {
		// string too big start allocating on the heap
		if ((spf_str->length + spf_str_append->length) >= SSO_MAX_LENGTH) {
			size_t new_capacity = ((spf_str->length + spf_str_append->length) * 2) + 1; // +1 for null terminator

			spf_str->data = malloc(sizeof(char) * new_capacity);
			SPFS_ASSERT(spf_str->data != NULL);

			// copy the SSO first
			strncpy(spf_str->data, spf_str->sso, sizeof(char) * SSO_MAX_LENGTH);

			spf_str->capacity = new_capacity;

			// move the spf_str_append data to spf_str
			memmove(spf_str->data + spf_str->length, __string_data(spf_str_append, 0), sizeof(char) * spf_str_append->length);

			spf_str->length += spf_str_append->length;
		}
		else {
			// move appended str to SSO if it didnt exceed the limit
			memmove(spf_str->sso + spf_str->length, __string_data(spf_str_append, 0), sizeof(char) * spf_str_append->length);
			spf_str->length += spf_str_append->length;
		}
	}
	else {
		if ((spf_str->length + spf_str_append->length) >= spf_str->capacity) {
			spf_str->capacity = ((spf_str->length + spf_str_append->length) * 2);
			spf_str->data = realloc(spf_str->data, sizeof(char) * (spf_str->capacity + 1)); // +1 for null terminator
			SPFS_ASSERT(spf_str->data != NULL);

			memmove(spf_str->data + spf_str->length, __string_data(spf_str_append, 0), sizeof(char) * spf_str_append->length);
			spf_str->length += spf_str_append->length;
		}
		else {

			memmove(spf_str->data + spf_str->length, __string_data(spf_str_append, 0), sizeof(char) * spf_str_append->length);
			spf_str->length += spf_str_append->length;
		}
	}
}

void string_append_str(spf_string* spf_str, const char* str)
{
	if (!__string_is_valid(spf_str)) return;

	size_t len = strlen(str);
	if (len == 0) return;

	if (__string_is_sso(spf_str)) {
		// string too big start allocating on the heap
		if ((spf_str->length + len) >= SSO_MAX_LENGTH) {
			size_t new_capacity = ((spf_str->length + len) * 2) + 1; // +1 for null terminator

			spf_str->data = malloc(sizeof(char) * new_capacity);
			SPFS_ASSERT(spf_str->data != NULL);

			// copy the SSO first
			strncpy(spf_str->data, spf_str->sso, sizeof(char) * SSO_MAX_LENGTH);

			spf_str->capacity = new_capacity;

			// move the str data to spf_str
			memmove(spf_str->data + spf_str->length, str, sizeof(char) * len);

			spf_str->length += len;
		}
		else {
			// move appended str to SSO if it didnt exceed the limit
			memmove(spf_str->sso + spf_str->length, str, sizeof(char) * len);
			spf_str->length += len;
		}
	}
	else {
		if ((spf_str->length + len) >= spf_str->capacity) {
			spf_str->capacity = ((spf_str->length + len) * 2);
			spf_str->data = realloc(spf_str->data, sizeof(char) * (spf_str->capacity + 1)); // +1 for null terminator
			SPFS_ASSERT(spf_str->data != NULL);

			memmove(spf_str->data + spf_str->length, str, sizeof(char) * len);
			spf_str->length += len;
		}
		else {
			memmove(spf_str->data + spf_str->length, str, sizeof(char) * len);
			spf_str->length += len;
		}
	}
}

const char* string_to_cstr(spf_string* spf_str)
{
	if (!__string_is_valid(spf_str)) return "";
	return __string_data(spf_str, 1);
}

_Bool string_compare_spfstr(spf_string* spf_str1, spf_string* spf_str2)
{
	if (!__string_is_valid(spf_str1)) return 0;
	if (!__string_is_valid(spf_str2)) return 0;
	if (spf_str1->length == 0 || spf_str2->length == 0) return 0;
	if (spf_str1->length != spf_str2->length) return 0;

	size_t i = 0;
	while (i <= spf_str1->length) {
		if (i >= spf_str1->length) // all characters matched
			return 1;

		if (__string_data(spf_str1, 0)[i] == __string_data(spf_str2, 0)[i]) {
			i++;
			continue;
		}

		return 0; // character mismatch
	}

	return 1;
}

_Bool string_compare_cstr(spf_string* spf_str, const char* str)
{
	if (!__string_is_valid(spf_str)) return 0;
	size_t len = strlen(str);

	if (spf_str->length == 0 || len == 0) return 0;
	if (spf_str->length != len) return 0;

	size_t i = 0;
	while (i <= spf_str->length) {
		if (i >= spf_str->length) // all characters matched
			return 1;

		if (__string_data(spf_str, 0)[i] == str[i]) {
			i++;
			continue;
		}

		return 0; // character mismatch
	}

	return 1;
}

#else // STRING_VIEW_IMPL

_Bool __string_is_sso(spf_string* spf_str);
_Bool __string_is_valid(spf_string* spf_str);

char* __string_data(spf_string* spf_str, _Bool null_terminate);
spf_string string_create(const char* str);

void string_reserve(spf_string* spf_str, size_t capacity);

void string_destroy(spf_string* spf_str);
void string_clear(spf_string* spf_str);

void string_append_char(spf_string* spf_str, char c);
void string_append_spfstr(spf_string* spf_str, spf_string* spf_str_append);
void string_append_str(spf_string* spf_str, const char* str);

const char* string_to_cstr(spf_string* spf_str);

_Bool string_compare_spfstr(spf_string* spf_str1, spf_string* spf_str2);
_Bool string_compare_cstr(spf_string* spf_str, const char* str);

#endif // SPF_STRING_IMPL

#endif // SPF_STRING_H
