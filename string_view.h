#ifndef STRING_VIEW_H
#define STRING_VIEW_H

/*
	to use this library do this:
		#define STRING_VIEW_IMPLEMENTAION
	before you include this file in *one* C or C++ file to create the implementation.
		#include "string_view.h"
	like this:
		#define STRING_VIEW_IMPLEMENTAION
		#include "string_view.h"


	basic usage:
		creating a string view looks like this:
			string_view_t sv = sv_create("Your string hoes here");

		or if you want to create a string_view with only a part of a C string
			string_view_t sv = sv_create_sub("Your string hoes here", 11);

		to print a string_view you have to use
			SV_FMT
		as the format and
			SV_ARGS
		as the format arguments

		which looks like this:
			printf("This is my string_view = "SV_FMT"\n", SV_ARGS(my_vs));


	examples:
		- create and print string_view:
			string_view_t sv = sv_create("this is some text for the string view");
    		printf("string view string = '"SV_FMT"', length = %d\n", SV_ARGS(sv), sv.length);

    	- get last character:
		    printf("back char = '%c'\n", sv_back(sv));

		- get first character:
		    printf("front char = '%c'\n", sv_front(sv));

		- get character at position
			int char_pos = 6;
		    printf("chat at (%d) = '%c'\n", char_pos, sv_at(sv, char_pos));

		- shrink string_view from position by count
			string_view_t substr = sv_substr(sv, 25, 5);
		    printf("substr = '"SV_FMT"'\n", SV_ARGS(substr));

		- compare 2 string_views together
			printf("are string views the same = %s\n", sv_compare(sv, substr) ? "true" : "false");

		- check if string_view starts with a string
			const char* check_str = "this is";
		    printf("starts with '%s'?  %s\n", check_str, sv_starts_with(sv, check_str) ? "true" : "false");

		- check if string_view ends with a string
			const char* check_str2 = "view";
		    printf("ends with '%s'?  %s\n", check_str2, sv_ends_with(sv, check_str2) ? "true" : "false");

		- check if string_view contains a string
			const char* contains_str = "for";
		    printf("contains '%s'?  %s\n", contains_str, sv_contains(sv, contains_str) ? "true" : "false");

		- clip/shrink the string_view from the start by count
			string_view_t sv_prefix = sv_create(sv.data);
		    printf("sv_prefix before = '"SV_FMT"'\n", SV_ARGS(sv_prefix));
		    printf("sv_prefix after = '"SV_FMT"'\n", SV_ARGS(sv_clip_prefix(sv_prefix, 12)));

		- clip/shrink the string_view from the end by count
			string_view_t sv_suffix = sv_create(sv.data);
		    printf("sv_suffix before = '"SV_FMT"'\n", SV_ARGS(sv_suffix));
		    printf("sv_suffix after = '"SV_FMT"'\n", SV_ARGS(sv_clip_suffix(sv_suffix, 12)));
*/

#include <corecrt.h>  // for size_t
#include <stdbool.h>  // for bool, true and false

#define SV_FMT "%.*s"
#define SV_ARGS(sv) (sv).length, (sv).data

typedef struct
{
	const char* data;
	size_t length;
} string_view_t;

#ifndef STRING_VIEW_IMPLEMENTAION

string_view_t sv_create(const char* str);
string_view_t sv_create_sub(const char* str, size_t length);

bool sv_is_valid(string_view_t sv);

char sv_back(string_view_t sv);
char sv_front(string_view_t sv);
char sv_at(string_view_t sv, size_t pos);

bool sv_compare(string_view_t sv1, string_view_t sv2);
bool sv_compare_str(string_view_t sv, const char* str);
bool sv_starts_with(string_view_t sv, const char* str);
bool sv_ends_with(string_view_t sv, const char* str);
bool sv_contains(string_view_t sv, const char* str);

string_view_t sv_substr(string_view_t sv, size_t pos, size_t count);
string_view_t sv_clip_prefix(string_view_t sv, size_t count);
string_view_t sv_clip_suffix(string_view_t sv, size_t count);

#else // STRING_VIEW_IMPLEMENTAION

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

bool sv_is_valid(string_view_t sv)
{
	if (sv.data == NULL) return false;
	if (sv.length <= 0) return false;
	return true;
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

bool sv_compare(string_view_t sv1, string_view_t sv2)
{
	if (!sv_is_valid(sv1)) return false;
	if (!sv_is_valid(sv2)) return false;

	static char str1[256];
	static char str2[256];

	snprintf(str1, sizeof(str1), SV_FMT, SV_ARGS(sv1));
	snprintf(str2, sizeof(str2), SV_FMT, SV_ARGS(sv2));

	return strncmp(str1, str2, sizeof(str1)) == 0;
}

bool sv_compare_str(string_view_t sv, const char* str)
{
	if (!sv_is_valid(sv)) return false;
	if (strlen(str) == 0) return false;

	static char sv_str[256];
	snprintf(sv_str, sizeof(sv_str), SV_FMT, SV_ARGS(sv));

	return strncmp(sv_str, str, sizeof(sv_str)) == 0;
}

bool sv_starts_with(string_view_t sv, const char* str)
{
	if (!sv_is_valid(sv)) return false;

	int strl = strlen(str);
	if (strl == 0) return false;

	string_view_t start = sv_create_sub(sv.data, strl);
	return sv_compare_str(start, str);
}

bool sv_ends_with(string_view_t sv, const char* str)
{
	if (!sv_is_valid(sv)) return false;

	int strl = strlen(str);
	if (strl == 0) return false;

	if ((sv.length - strl) <= 0) return false;

	string_view_t end = sv_create(sv.data + (sv.length - strl));
	return sv_compare_str(end, str);
}

bool sv_contains(string_view_t sv, const char* str)
{
	if (!sv_is_valid(sv)) return false;

	int strl = strlen(str);
	if (strl == 0) return false;

	size_t i = 0;
	while (i < sv.length) {
		char sv_char = sv.data[i];
		if (sv_char == str[0]) {
			string_view_t substr = sv_create_sub(&sv.data[i], strl);

			if (sv_compare_str(substr, str))
				return true;
			else {
				i++;
				continue;
			}
		}
		i++;
	}

	return false;
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
#endif // STRING_VIEW_IMPLEMENTAION

#endif // STRING_VIEW_H
