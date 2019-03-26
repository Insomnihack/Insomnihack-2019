#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#define PACKAGE_VERSION "0.1"
#define ROT_OFFSET 13
#define STR_LENGTH(str) ((sizeof(str)) - 1)

#define ARGCOUNTCHECK(typestr)                                                                                                                       \
  if (args->arg_count != 1)                                                                                                                          \
  {                                                                                                                                                  \
    snprintf(message, MYSQL_ERRMSG_SIZE, "wrong argument count: %s requires one " typestr " argument, got %d arguments", funcname, args->arg_count); \
    return 1;                                                                                                                                        \
  }

#define ARGTYPECHECK(arg, type, typestr)                                                                                                                    \
  if (arg != type)                                                                                                                                          \
  {                                                                                                                                                         \
    snprintf(message, MYSQL_ERRMSG_SIZE, "wrong argument type: %s requires one " typestr " argument. Expected type %d, got type %d.", funcname, type, arg); \
    return 1;                                                                                                                                               \
  }

#define STRARGCHECK ARGTYPECHECK(args->arg_type[0], STRING_RESULT, "string")
#define INTARGCHECK ARGTYPECHECK(args->arg_type[0], INT_RESULT, "integer")
#define LIBVERSION ("lib_stuff_version " PACKAGE_VERSION)


size_t x_strlcpy(char *__restrict dest, const char *__restrict src, size_t dest_len)
{
    if (dest_len == 0) {
        return strlen(src);
    } else {
        char *const dest_str_end = dest + dest_len - 1;
        char *__restrict d = dest;
        const char *__restrict s = src;

        while (d != dest_str_end) {
            if ((*d++ = *s++) == '\0') {
                return (d - 1 - dest);
            }
        }
        *d = '\0';
        return (dest_len - 1) + strlen(s);
    }
}
