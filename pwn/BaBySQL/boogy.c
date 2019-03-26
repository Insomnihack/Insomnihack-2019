#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <assert.h>

#include "stuff.h"
// sources:
// https://github.com/mysqludf/lib_stuff_str
// https://github.com/mysqludf/lib_stuff_str/blob/master/lib_stuff_str.c
#include <mysql/mysql.h>


#define MAX_BUFF 1024
#define BREAKPOINT asm volatile("int3;")

typedef struct stuff
{
  FILE *f;
  char buff[MAX_BUFF];
  int (*pprint)();
} stuff_t;


void log_data(char *data, size_t len)
{
  stuff_t stuff;
  memset(stuff.buff, 0, sizeof(stuff.buff));

  stuff.f = fopen("logfile.log", "a+");
  memcpy(stuff.buff, data, len);

  chmod("/var/lib/mysql/logfile.log", 0644);
  if (stuff.f == NULL)
  {
    fprintf(stderr, "Can't open log file");
    exit(1);
  }
  // cheet code to have a libc pointer on the stack
  stuff.pprint = fprintf;
  stuff.pprint(stuff.f, "%s\n", stuff.buff);
  fclose(stuff.f);
}


/*
  called once for each SQL statement which invokes str_rot13();
  returns:	1 => failure; 0 => successful initialization
*/
my_bool str_rot13_init(UDF_INIT *initid,
                       UDF_ARGS *args,
                       char *message)
{
  /* make sure user has provided exactly one string argument */
  if (args->arg_count != 1)
  {
    snprintf(message, MYSQL_ERRMSG_SIZE, "\nERROR: str_rot13 need a string !\nDon't you know how to reverse ?");
    return 1;
  }
  if (args->arg_type[0] != STRING_RESULT)
  {
    args->arg_type[0] = STRING_RESULT;
  }

  unsigned long res_length;
  res_length = args->lengths[0];

  if (SIZE_MAX < res_length)
  {
    snprintf(message, MYSQL_ERRMSG_SIZE, "res_length (%lu) cannot be greater than SIZE_MAX (%zu)", res_length, (size_t)(SIZE_MAX));
    return 1;
  }

  initid->ptr = NULL;
  if (res_length > 255)
  {
    char *tmp = (char *)malloc((size_t)res_length); /* res_length <= SIZE_MAX. */
    if (tmp == NULL)
    {
      snprintf(message, MYSQL_ERRMSG_SIZE, "malloc() failed to allocate %zu bytes of memory", (size_t)res_length);
      return 1;
    }
    initid->ptr = tmp;
  }
  initid->maybe_null = 1;
  initid->max_length = res_length;

  return 0;
}

/*
  Here is the cool stuff to exploit
*/
char *str_rot13(UDF_INIT *initid, UDF_ARGS *args,
                char *result, unsigned long *res_length,
                char *null_value, char *error)
{
  int i, cod_ascii;
  char *s;

  if (args->args[0] == NULL)
  {
    result = NULL;
    *res_length = 0;
    *null_value = 1;
    return result;
  }

  // s will contain the user-supplied argument
  s = args->args[0];
  if (initid->ptr != NULL)
  {
    result = initid->ptr;
  }

  *res_length = args->lengths[0];
  for (i = 0; i < *res_length; i++)
  {
    // cod_ascii is an integer containing the ascii code of a single character
    cod_ascii = s[i];
    if (cod_ascii >= 97 && cod_ascii <= 122) // lower case character
    {
      cod_ascii += ROT_OFFSET;

      if (cod_ascii > 122)
        cod_ascii = 96 + (cod_ascii - 122);
    }
    else if (cod_ascii >= 65 && cod_ascii <= 90) // upper case character
    {
      cod_ascii += ROT_OFFSET;
      if (cod_ascii > 90)
        cod_ascii = 64 + (cod_ascii - 90);
    }
    result[i] = cod_ascii;
  }
  log_data(s, *res_length);

  return result;
}

/*
  called once for each SQL statement which invokes str_rot13();
  returns:	nothing
*/
void str_rot13_deinit(UDF_INIT *initid)
{
  if (initid->ptr != NULL)
    free(initid->ptr);
}

/******************************************************************************
** purpose:	called once for each invocation of str_xor();
**					checks arguments, sets restrictions
** receives:	pointer to UDF_INIT struct; pointer to UDF_ARGS struct which contains information about
**					the number, length, and type of args that were passed to str_xor(); pointer to a char
**					array of size MYSQL_ERRMSG_SIZE in which an error message can be stored if necessary
** returns:	1 => failure; 0 => success
******************************************************************************/
my_bool str_xor_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  static const char funcname[] = "str_xor";
  unsigned long res_length;

  if (args->arg_count != 2)
  {
    snprintf(message, MYSQL_ERRMSG_SIZE, "wrong argument count: str_xor requires exactly two string arguments, got %d arguments.", args->arg_count);
    return 1;
  }
  if (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT)
  {
    x_strlcpy(message, "wrong argument type: str_xor requires two string arguments", MYSQL_ERRMSG_SIZE);
    return 1;
  }

  res_length = args->lengths[0];
  if (args->lengths[1] > res_length)
    res_length = args->lengths[1];

  if (SIZE_MAX < res_length)
  {
    snprintf(message, MYSQL_ERRMSG_SIZE, "res_length (%lu) cannot be greater than SIZE_MAX (%zu)", res_length, (size_t)(SIZE_MAX));
    return 1;
  }

  initid->ptr = NULL;

  if (res_length > 255)
  {
    char *tmp = (char *)malloc((size_t)res_length); /* This is a safe cast because res_length <= SIZE_MAX. */
    if (tmp == NULL)
    {
      snprintf(message, MYSQL_ERRMSG_SIZE, "malloc() failed to allocate %zu bytes of memory", (size_t)res_length);
      return 1;
    }
    initid->ptr = tmp;
  }

  initid->maybe_null = 1;
  initid->max_length = res_length;
  return 0;
}

void str_xor_deinit(UDF_INIT *initid)
{
  if (initid->ptr != NULL)
    free(initid->ptr);
}

/******************************************************************************
** purpose:	exclusive OR (XOR) each byte of the two string arguments.
**					If one string argument is longer than the other, the shorter string
**					is considered to be padded with enough trailing NUL bytes that the
**					arguments would have the same length.
** receives:	pointer to UDF_INIT struct; pointer to UDF_ARGS struct which
**					contains the two string arguments and their lengths;
**					pointer to the result buffer; pointer to ulong that stores the result length;
**					pointer to mem which can be set to 1 if the result is NULL; pointer
**					to mem which can be set to 1 if the calculation resulted in an
**					error
** returns:	the bytewise XOR of the two strings
******************************************************************************/
char *str_xor(UDF_INIT *initid, UDF_ARGS *args, char *result,
              unsigned long *res_length, char *null_value, char *error)
{
  assert(args->arg_count == 2);
  assert(args->arg_type[0] == STRING_RESULT && args->arg_type[1] == STRING_RESULT);

  if (args->args[0] == NULL || args->args[1] == NULL)
  {
    result = NULL;
    *res_length = 0;
    *null_value = 1;
    return result;
  }

  if (initid->ptr != NULL)
  {
    result = initid->ptr;
  }

  {
    char *__restrict p = result,
                     *arg0 = args->args[0],
                     *arg1 = args->args[1];
    const unsigned long arg0_length = args->lengths[0],
                        arg1_length = args->lengths[1];
    char *const arg0_end = arg0 + arg0_length;
    char *const arg1_end = arg1 + arg1_length;

    if (arg0_length <= arg1_length)
    {
      for (; arg0 != arg0_end; ++arg0, ++arg1)
        *p++ = (*arg0) ^ (*arg1);
      //for (; arg1 != arg1_end; ++arg1)
      //	*p++ = *arg1 /* '\x00' ^ (*arg1) */;
      memcpy(p, arg1, (arg1_end - arg1));

      *res_length = arg1_length;
    }
    else
    {
      for (; arg1 != arg1_end; ++arg0, ++arg1)
        *p++ = (*arg0) ^ (*arg1);
      //for (; arg0 != arg0_end; ++arg0)
      //	*p++ = *arg0 /* (*arg0) ^ '\x00' */;
      memcpy(p, arg0, (arg0_end - arg0));

      *res_length = arg0_length;
    }
  }

  *null_value = 0;
  *error = 0;
  return result;
}

/******************************************************************************
** purpose:	called once for each SQL statement which invokes str_shuffle();
**					checks arguments, sets restrictions, allocates memory that
**					will be used during the main str_shuffle() function
** receives:	pointer to UDF_INIT struct which is to be shared with all
**					other functions (str_shuffle() and str_shuffle_deinit()) -
**					the components of this struct are described in the MySQL manual;
**					pointer to UDF_ARGS struct which contains information about
**					the number, size, and type of args the query will be providing
**					to each invocation of str_shuffle(); pointer to a char
**					array of size MYSQL_ERRMSG_SIZE in which an error message
**					can be stored if necessary
** returns:	1 => failure; 0 => successful initialization
******************************************************************************/

my_bool str_shuffle_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
  static const char funcname[] = "str_shuffle";
  unsigned long res_length;

  /* make sure user has provided exactly one string argument */
  ARGCOUNTCHECK("string");
  STRARGCHECK;

  res_length = args->lengths[0];

  if (SIZE_MAX < res_length)
  {
    snprintf(message, MYSQL_ERRMSG_SIZE, "res_length (%lu) cannot be greater than SIZE_MAX (%zu)", res_length, (size_t)(SIZE_MAX));
    return 1;
  }

  initid->ptr = NULL;

  if (res_length > 255)
  {
    char *tmp = (char *)malloc((size_t)res_length); /* This is a safe cast because res_length <= SIZE_MAX. */
    if (tmp == NULL)
    {
      snprintf(message, MYSQL_ERRMSG_SIZE, "malloc() failed to allocate %zu bytes of memory", (size_t)res_length);
      return 1;
    }
    initid->ptr = tmp;
  }

  initid->maybe_null = 1;
  initid->max_length = res_length;
  return 0;
}

/******************************************************************************
** purpose:	deallocate memory allocated by str_shuffle_init(); this func
**					is called once for each query which invokes str_shuffle(),
**					it is called after all of the calls to str_shuffle() are done
** receives:	pointer to UDF_INIT struct (the same which was used by
**					str_shuffle_init() and str_shuffle())
** returns:	nothing
******************************************************************************/
void str_shuffle_deinit(UDF_INIT *initid)
{
  if (initid->ptr != NULL)
    free(initid->ptr);
}

/******************************************************************************
** purpose:	randomly shuffle the characters of a string
** receives:	pointer to UDF_INIT struct which contains pre-allocated memory
**					in which work can be done; pointer to UDF_ARGS struct which
**					contains the functions arguments and data about them; pointer
**					to mem which can be set to 1 if the result is NULL; pointer
**					to mem which can be set to 1 if the calculation resulted in an
**					error
** returns:	one of the possible permutations of the original string
******************************************************************************/
char *str_shuffle(UDF_INIT *initid, UDF_ARGS *args,
                  char *result, unsigned long *res_length,
                  char *null_value, char *error)
{
  int i, j;
  char swp;

  if (args->args[0] == NULL)
  {
    result = NULL;
    *res_length = 0;
    *null_value = 1;
    return result;
  }

  if (initid->ptr != NULL)
  {
    result = initid->ptr;
  }

  // copy the argument string into result
  memcpy(result, args->args[0], args->lengths[0]);
  *res_length = args->lengths[0];

  for (i = 0; i < *res_length; i++)
  {
    // select a random position to swap result[i]
    j = i + rand() / (RAND_MAX / (*res_length - i) + 1);

    // swap the two characters
    swp = result[j];
    result[j] = result[i];
    result[i] = swp;
  }

  return result;
}


/******************************************************************************
** purpose:	called once for each SQL statement which invokes lib_stuff_str_info_init();
**					checks arguments, sets restrictions, allocates memory that
**					will be used during the main lib_stuff_str_info_init() function
** receives:	pointer to UDF_INIT struct which is to be shared with all
**					other functions (lib_stuff_str_info_init() and lib_stuff_str_info_init_deinit()) -
**					the components of this struct are described in the MySQL manual;
**					pointer to UDF_ARGS struct which contains information about
**					the number, size, and type of args the query will be providing
**					to each invocation of lib_stuff_str_info_init(); pointer to a char
**					array of size MYSQL_ERRMSG_SIZE in which an error message
**					can be stored if necessary
** returns:	1 => failure; 0 => successful initialization
******************************************************************************/
my_bool lib_stuff_str_info_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if (args->arg_count != 0)
	{
		x_strlcpy(message, "No arguments allowed (udf: lib_stuff_str_info)", MYSQL_ERRMSG_SIZE);
		return 1;
	}

	initid->maybe_null = 0;
	initid->max_length = (sizeof LIBVERSION) - 1;
	initid->const_item = 1;
	return 0;
}

/******************************************************************************
** purpose:	deallocate memory allocated by lib_stuff_str_info_init();
**					this function is called once for each query which invokes
**					lib_stuff_str_info(), it is called after all of the calls to
**					lib_stuff_str_info() are done
** receives:	pointer to UDF_INIT struct (the same which was used by
**					lib_stuff_str_info_init() and lib_stuff_str_info())
** returns:	nothing
******************************************************************************/
void lib_stuff_str_info_deinit(UDF_INIT *initid)
{
}

/******************************************************************************
** purpose:	obtain information about the currently installed version
**					of lib_stuff_str.
** receives:	pointer to UDF_INIT struct which contains pre-allocated memory
**					in which work can be done; pointer to UDF_ARGS struct which
**					contains the functions arguments and data about them; pointer
**					to mem which can be set to 1 if the result is NULL; pointer
**					to mem which can be set to 1 if the calculation resulted in an
**					error
** returns:	the library version number
******************************************************************************/
char *lib_stuff_str_info(UDF_INIT *initid, UDF_ARGS *args,
			char *result, unsigned long *res_length,
			char *null_value, char *error)
{
	strcpy(result, LIBVERSION);
	*res_length = (sizeof LIBVERSION) - 1;
	return result;
}
