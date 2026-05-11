#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <stddef.h>

#define SV_FMT "%.*s"
#define SV_ARGS(sv) (int)(sv).length, (sv).data

typedef struct
{
	const char* data;
	size_t length;
} string_view_t;

#define string_view(str) sv_create((str))

#define sv_compare(sv, str) _Generic((str), \
	string_view_t: sv_compare_sv,           \
	const char*: sv_compare_str,            \
	char*: sv_compare_str                   \
) ((sv), (str))

#ifndef STRING_VIEW_IMPL

string_view_t sv_create(const char* str);
string_view_t sv_create_sub(const char* str, size_t length);

_Bool sv_is_valid(string_view_t sv);

char sv_back(string_view_t sv);
char sv_front(string_view_t sv);
char sv_at(string_view_t sv, size_t pos);

_Bool sv_compare_sv(string_view_t sv1, string_view_t sv2);
_Bool sv_compare_str(string_view_t sv, const char* str);
_Bool sv_starts_with(string_view_t sv, const char* str);
_Bool sv_ends_with(string_view_t sv, const char* str);
_Bool sv_contains(string_view_t sv, const char* str);

string_view_t sv_substr(string_view_t sv, size_t pos, size_t count);
string_view_t sv_clip_prefix(string_view_t sv, size_t count);
string_view_t sv_clip_suffix(string_view_t sv, size_t count);

#else // STRING_VIEW_IMPL

#include <stdio.h>    // for snprintf
#include <string.h>   // for strlen

string_view_t sv_create(const char* str)
{
	return (string_view_t){.data = str, .length = strlen(str)};
}

string_view_t sv_create_sub(const char* str, size_t length)
{
	return (string_view_t){.data = str, .length = length};
}

_Bool sv_is_valid(string_view_t sv)
{
	if (sv.data == NULL) return 0;
	if (sv.length <= 0) return 0;
	return 1;
}

char sv_back(string_view_t sv)
{
	if (!sv_is_valid(sv)) return ' ';
	return sv.data[sv.length - 1];
}

char sv_front(string_view_t sv)
{
	if (!sv_is_valid(sv)) return ' ';
	return sv.data[0];
}

char sv_at(string_view_t sv, size_t pos)
{
	if (!sv_is_valid(sv)) return ' ';
	if (pos > sv.length)return ' ';
	return sv.data[pos - 1];
}

_Bool sv_compare_sv(string_view_t sv1, string_view_t sv2)
{
	if (!sv_is_valid(sv1)) return 0;
	if (!sv_is_valid(sv2)) return 0;

	static char str1[256];
	static char str2[256];

	snprintf(str1, sizeof(str1), SV_FMT, SV_ARGS(sv1));
	snprintf(str2, sizeof(str2), SV_FMT, SV_ARGS(sv2));

	return strncmp(str1, str2, sizeof(str1)) == 0;
}

_Bool sv_compare_str(string_view_t sv, const char* str)
{
	if (!sv_is_valid(sv)) return 0;
	if (strlen(str) == 0) return 0;

	static char sv_str[256];
	snprintf(sv_str, sizeof(sv_str), SV_FMT, SV_ARGS(sv));

	return strncmp(sv_str, str, sizeof(sv_str)) == 0;
}

_Bool sv_starts_with(string_view_t sv, const char* str)
{
	if (!sv_is_valid(sv)) return 0;

	int strl = strlen(str);
	if (strl == 0) return 0;

	string_view_t start = sv_create_sub(sv.data, strl);
	return sv_compare_str(start, str);
}

_Bool sv_ends_with(string_view_t sv, const char* str)
{
	if (!sv_is_valid(sv)) return 0;

	int strl = strlen(str);
	if (strl == 0) return 0;

	if ((sv.length - strl) <= 0) return 0;

	string_view_t end = sv_create(sv.data + (sv.length - strl));
	return sv_compare_str(end, str);
}

_Bool sv_contains(string_view_t sv, const char* str)
{
	if (!sv_is_valid(sv)) return 0;

	int strl = strlen(str);
	if (strl == 0) return 0;

	size_t i = 0;
	while (i < sv.length) {
		char sv_char = sv.data[i];
		if (sv_char == str[0]) {
			string_view_t substr = sv_create_sub(&sv.data[i], strl);

			if (sv_compare_str(substr, str))
				return 1;
			else {
				i++;
				continue;
			}
		}
		i++;
	}

	return 0;
}

string_view_t sv_substr(string_view_t sv, size_t pos, size_t count)
{
	if (!sv_is_valid(sv)) return sv;
	if (pos >= sv.length) return sv;
	if (count >= sv.length || ((pos - 1) + count) >= sv.length) return sv;

	return (string_view_t){.data = sv.data + pos, .length = count};
}

string_view_t sv_clip_prefix(string_view_t sv, size_t count) {
	if (!sv_is_valid(sv)) return sv;
	if (count >= sv.length) return sv;

	return (string_view_t){.data = sv.data + count, .length = sv.length - count};
}

string_view_t sv_clip_suffix(string_view_t sv, size_t count) {
	if (!sv_is_valid(sv)) return sv;
	if ((sv.length - count) <= 0) return sv;

	return (string_view_t){.data = sv.data, .length = sv.length - count};
}
#endif // STRING_VIEW_IMPL

#endif // STRING_VIEW_H
