/* -*-c-*- */
/* Copyright (C) 2002  Mikhael Goikhman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * FBidi.c - interface to Bidi, we use fribidi implementation here.
 * See FBidi.h for some comments on this interface.
 */

#include "FBidi.h"

#if HAVE_BIDI

#include "FBidiJoin.h"
#include <fribidi/fribidi.h>
#include "safemalloc.h"
#include <stdio.h>

Bool FBidiIsApplicable(const char *charset)
{
	if (fribidi_parse_charset((char *)charset) == FRIBIDI_CHARSET_NOT_FOUND)
	{
		return False;
	}
	return True;
}

char *FBidiConvert(
	const char *logical_str, const char *charset, int str_len, Bool *is_rtl,
	int *out_len)
{
	char *visual_str;
	FriBidiCharSet fribidi_charset;
	FriBidiChar *logical_unicode_str;
	FriBidiChar *visual_unicode_str;
	FriBidiCharType pbase_dir = FRIBIDI_TYPE_ON;

	if (logical_str == NULL || charset == NULL)
	{
		return NULL;
	}
	if (str_len < 0)
	{
		str_len = strlen(logical_str);
	}
	if (is_rtl != NULL)
	{
		*is_rtl = False;
	}

	fribidi_charset = fribidi_parse_charset((char *)charset);
	if (fribidi_charset == FRIBIDI_CHARSET_NOT_FOUND)
	{
		return NULL;
	}

	/* it is possible that we allocate a bit more here, if utf-8 */
	logical_unicode_str =
		(FriBidiChar *)safemalloc((str_len + 1) * sizeof(FriBidiChar));

	/* convert to unicode first */
	str_len = fribidi_charset_to_unicode(
		fribidi_charset, (char *)logical_str, str_len,
		logical_unicode_str);

	visual_unicode_str =
		(FriBidiChar *)safemalloc((str_len + 1) * sizeof(FriBidiChar));

	/* apply bidi algorithm, convert logical string to visual string */
	fribidi_log2vis(
		logical_unicode_str, str_len, &pbase_dir,
		visual_unicode_str, NULL, NULL, NULL);

	/* character shape/join - will get pulled into fribidi with time */
	str_len = shape_n_join(visual_unicode_str, str_len);

	visual_str = (char *)safemalloc((4 * str_len + 1) * sizeof(char));

	/* convert from unicode finally */
	*out_len = fribidi_unicode_to_charset(
		fribidi_charset, visual_unicode_str, str_len, visual_str);

	if (is_rtl != NULL && pbase_dir == FRIBIDI_TYPE_RTL)
	{
		*is_rtl = True;
	}

	free(logical_unicode_str);
	free(visual_unicode_str);
	return visual_str;
}

#endif /* HAVE_BIDI */