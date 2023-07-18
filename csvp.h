/*
  Author: wwotz 

  Add #CSVP_IMPLEMENTATION to the start of the
  implementation file in order to add the implementation
  code to your project
*/

#ifndef CSVP_INCLUDE_H_
#define CSVP_INCLUDE_H_

/* turn off when debugging is to be disabled
   this will slightly improve performance in preparation
   for deployable applications */
#define CSVP_DEBUG

#ifdef CSVP_DEBUG
#include <assert.h>
#endif /* CSVP_DEBUG */

#ifndef CSVPDEF
#ifdef CSVPSTATIC
#define CSVPDEF static
#else /* !defined(CSVPSTATIC) */
#define CSVPDEF extern
#endif /* CSVPSTATIC */
#endif /* CSVPDEF */

CSVPDEF const char *
csvp_log_get_message(void);
CSVPDEF int
csvp_log_had_error(void);

#include <stdio.h>
#include <string.h>

#if !(defined(CSVP_MALLOC) && defined(CSVP_REALLOC) && defined(CSVP_FREE))
#include <stdlib.h>
#define CSVP_MALLOC(x) malloc(x)
#define CSVP_REALLOC(x, newsz) realloc(x, newsz)
#define CSVP_FREE(x) free(x)
#endif /* custom memory procedures not defined,
          or at least not all that is required */

#define CSVP_STRING_DEFAULT_CAPACITY 8

typedef struct csvp_string_t {
        char *data;
        size_t size;
        size_t capacity;
} csvp_string_t;

CSVPDEF int
csvp_string_init(csvp_string_t *str);
CSVPDEF int
csvp_string_write(csvp_string_t *str, const char *data, size_t data_len);
CSVPDEF void
csvp_string_clear(csvp_string_t *str);
CSVPDEF void
csvp_string_free(csvp_string_t *str);

typedef enum csvp_type_t {
        CSVP_INTEGER_TYPE,
        CSVP_FLOAT_TYPE,
        CSVP_STRING_TYPE,
} csvp_type_t;

typedef char csvp_char_t;
typedef short csvp_short_t;
typedef int csvp_int_t;
typedef long csvp_long_t;

// for now
typedef double csvp_float_t;
typedef double csvp_double_t;

typedef struct csvp_entry_t {
        csvp_type_t type;
        union {
                csvp_int_t v_int;
                csvp_float_t v_float;
                csvp_string_t v_string;
        };
} csvp_entry_t;

typedef struct csvp_record_t {
        size_t size;
        csvp_entry_t *entries;
} csvp_record_t;

/* Obtains a format based on a description of the record format,
   this can then be filled by subsequent calls to:
   'csvp_next_record' and 'csvp_prev_record', returns a CSVP_MALLOC'd
   structure on success, NULL if the format can not be processed.
   */
CSVPDEF csvp_record_t *
csvp_create_format(const char *desc);

/* Obtains the next record in the csv file, returns 0 on success,
   non-zero otherwise.
   @fd - A file open for reading of the csv that you want to process.
   It returns the record as a series of bytes.
*/
CSVPDEF int
csvp_next_record(FILE *fd, csvp_record_t *record);

/* Obtains the previous record in the csv file, returns 0 on success,
   non-zero otherwise.
   @fd - A file open for reading of the csv that you want to process.
   It returns the record as a series of bytes.
*/
CSVPDEF int
csvp_prev_record(FILE *fd, csvp_record_t *record);

#ifdef CSVP_IMPLEMENTATION

#ifdef CSVP_DEBUG
#define CSVP_LOG_MESSAGE_CAPACITY 256
#define CSVP_LOG_STACK_CAPACITY 20

static char csvp_log_stack[CSVP_LOG_STACK_CAPACITY][CSVP_LOG_MESSAGE_CAPACITY] = {0};
static int csvp_log_stack_ptr = 0;
static int csvp_log_stack_size = 0;

static int
csvp_log_empty(void)
{
        return csvp_log_stack_size == 0;
}

static int
csvp_log_full(void)
{
        return csvp_log_stack_size == CSVP_LOG_MESSAGE_CAPACITY;
}

static void
csvp_log_push_message(const char *msg, const char *file, size_t line)
{
        size_t c, i;
        assert(msg);
        assert(file);
        assert(line >= 0);
        if (!csvp_log_full())
                csvp_log_stack_size++;
        for (i = 0; (c = msg[i]) != '\0' && i < CSVP_LOG_MESSAGE_CAPACITY - 1; i++)
                csvp_log_stack[csvp_log_stack_ptr][i] = c;
        csvp_log_stack[csvp_log_stack_ptr][i] = '\0';
        csvp_log_stack_ptr = (csvp_log_stack_ptr + 1) % CSVP_LOG_STACK_CAPACITY;
}

static const char *
csvp_log_pop_message(void)
{
        if (csvp_log_empty())
                return NULL;
        csvp_log_stack_ptr--;
        csvp_log_stack_size--;
        if (csvp_log_stack_ptr < 0)
                csvp_log_stack_ptr += CSVP_LOG_STACK_CAPACITY;
        return (const char *) csvp_log_stack[csvp_log_stack_ptr];
}

#define CSVP_LOG_MESSAGE(msg) \
        csvp_log_push_message(msg, __FILE__, __LINE__)

CSVPDEF const char *
csvp_log_get_message(void)
{
        return csvp_log_pop_message();
}

CSVPDEF int
csvp_log_had_error(void)
{
        return !csvp_log_empty();
}

#else /* !defined(CSVP_DEBUG) */

static int
csvp_log_empty(void)
{
        return csvp_log_stack_size == 0;
}

static int
csvp_log_full(void)
{
        return csvp_log_stack_size == CSVP_LOG_MESSAGE_CAPACITY;
}

static void
csvp_log_push_message(const char *msg, const char *file, size_t line)
{
        return;
}

static const char *
csvp_log_pop_message(void)
{
        return NULL;
}

CSVPDEF const char *
csvp_log_get_message(void)
{
        return "Debugging is turned off!";
}

CSVPDEF int
csvp_log_had_error(void)
{
        /* assume always an error, so the callee can see that
           debugging has been turned off when they check the log */
        return 1;
}

#endif /* CSVP_DEBUG */

CSVPDEF int
csvp_string_init(csvp_string_t *str)
{
#ifdef CSVP_DEBUG
        assert(str);
#endif /* CSVP_DEBUG */
        str->capacity = CSVP_STRING_DEFAULT_CAPACITY;
        str->size = 0;
        str->data = CSVP_MALLOC(str->capacity * sizeof(*str->data));
        if (!str->data) {
#ifdef CSVP_DEBUG
                CSVP_LOG_MESSAGE("Failed to allocate memory for string!");
#endif /* CSVP_DEBUG */
                return -1;
        }
        return 0;
}

CSVPDEF int
csvp_string_write(csvp_string_t *str, const char *data, size_t data_len)
{
#ifdef CSVP_DEBUG
        assert(str);
        assert(str->capacity > 0);
#endif /* CSVP_DEBUG */
        if (!data || data_len <= 0)
                return 0;
        while (str->capacity <= data_len) {
                char *new_data = CSVP_REALLOC(str->data, str->capacity*2);
                if (!new_data) {
#ifdef CSVP_DEBUG
                        CSVP_LOG_MESSAGE("Failed to allocate space for string!");
#endif /* CSVP_DEBUG */
                        return -1;
                }
                str->data = new_data;
                str->capacity *= 2;
        }

        str->data = memcpy(str->data, data, data_len);
        str->data[data_len] = '\0';
        return data_len;
}

CSVPDEF void
csvp_string_clear(csvp_string_t *str)
{
#ifdef CSVP_DEBUG
        assert(str);
        assert(str->capacity > 0);
#endif /* CSVP_DEBUG */
        memset(str->data, 0, str->capacity);
        str->size = 0;
}

CSVPDEF void
csvp_string_free(csvp_string_t *str)
{
#ifdef CSVP_DEBUG
        assert(str);
        assert(str->capacity > 0);
#endif /* CSVP_DEBUG */
        csvp_string_clear(str);
        str->capacity = 0;
        CSVP_FREE(str->data);
}

static int
csvp_skip_whitespace(const char *buffer, size_t buffer_len)
{
        size_t i, c;
        for (i = 0; i < buffer_len && (c = buffer[i]) != '\0'
                     && c == ' '; i++)
                ;
        return i;
}

static int
csvp_check_match(const char *desc, const char *rest, size_t rest_size)
{
        size_t i;
        for (i = 0; i < rest_size && desc[i] != '\0'
                     && desc[i] == rest[i]; i++)
                ;
        return i == rest_size;
}

static int
csvp_check_integer(const char *buffer, size_t buffer_len)
{
        size_t i, c;
        i = csvp_skip_whitespace(buffer, buffer_len);
        c = buffer[i];
        if (c == ',' || c == '\0') {
                return 1;
        } else if (c >= '0' && c <= '9' || c == '-') {
                if (c == '-')
                        i++;
                while ((c = buffer[i]) >= '0' && c <= '9')
                        i++;
                i += csvp_skip_whitespace(buffer+i, buffer_len-i);
                if ((c = buffer[i]) == '\0' || c == ',')
                        return 1;
        }
        return 0;
}

static int
csvp_check_float(const char *buffer, size_t buffer_len)
{
        size_t i, c;
        i = csvp_skip_whitespace(buffer, buffer_len);
        c = buffer[i];
        if (c == ',' || c == '\0') {
                return 1;
        } else if ((c >= '0' && c <= '9') || c == '.'
                   || c == '-') {
                if (c == '.' || c == '-')
                        i++;
                while ((c = buffer[i]) >= '0' && c <= '9'
                       && i < buffer_len)
                        i++;
                if (c == '.') {
                        i++;
                        while ((c = buffer[i]) >= '0' && c <= '9'
                               && i < buffer_len)
                                i++;
                        if (c == ',' || c == '\0')
                                return 1;
                } else if (c == ',' || c == '\0') {
                        return 1;
                }
        }
        return 0;
}


static int
csvp_write_integer(csvp_entry_t *entry, const char *buffer, size_t buffer_len)
{
        size_t i, c;
        int n;
#ifdef CSVP_DEBUG
        assert(entry);
        assert(entry->type == CSVP_INTEGER_TYPE);
#endif /* CSVP_DEBUG */
        n = atoi(buffer);
        entry->v_int = n;
        for (i = 0; i < buffer_len && (c = buffer[i]) != '\0' && c != ','; i++)
                ;
        return (int) i;
}

static int
csvp_write_float(csvp_entry_t *entry, const char *buffer, size_t buffer_len)
{
        size_t i, c;
        double n;
#ifdef CSVP_DEBUG
        assert(entry);
        assert(entry->type == CSVP_FLOAT_TYPE);
#endif /* CSVP_DEBUG */
        n = atof(buffer);
        entry->v_float = n;
        for (i = 0; i < buffer_len && (c = buffer[i]) != '\0' && c != ','; i++)
                ;
        return (int) i;
}

static int
csvp_write_string(csvp_entry_t *entry, const char *buffer, size_t buffer_len)
{
        size_t i, c;
#ifdef CSVP_DEBUG
        assert(entry);
        assert(entry->type == CSVP_STRING_TYPE);
#endif /* CSVP_DEBUG */
        for (i = 0; i < buffer_len && (c = buffer[i]) != '\0' && c != ','; i++)
                ;
        csvp_string_write(&entry->v_string, buffer, i);
        return i;
}

/* used to hold an entry in the record being processed*/
#define RECORD_BUFSIZE 1024

CSVPDEF csvp_record_t *
csvp_create_format(const char *desc)
{
        #define MAX_ENTRIES 100
        csvp_type_t types[MAX_ENTRIES] = {0};
        csvp_record_t *record;
        record = CSVP_MALLOC(sizeof(*record));
        if (!record) {
#ifdef CSVP_DEBUG
                CSVP_LOG_MESSAGE("Failed to allocate memory for record!");
#endif /* CSVP_DEBUG */
                return NULL;
        }
        record->size = 0;
        for (size_t i = 0, c; (c = desc[i]) != '\0'
                     && record->size < MAX_ENTRIES; i++) {
                while ((c = desc[i]) == ',' || c == ' ')
                        i++;
                if (c == '\0')
                        break;
                
                switch (c) {
                case 's':
                        /* handles strings */
                        goto mismatch;
                case 'i':
                        if (csvp_check_match(desc+i+1, "nteger", 6)) {
                                types[record->size++] = CSVP_INTEGER_TYPE;
                        } else {
                                goto mismatch;
                        }
                        break;
                case 'f':
                        if (csvp_check_match(desc+i+1, "loat", 4)) {
                                types[record->size++] = CSVP_FLOAT_TYPE;
                        } else {
                                goto mismatch;
                        }
                        break;
                default:
                mismatch:
                        types[record->size++] = CSVP_STRING_TYPE;
                        break;
                }
                
                while ((c = desc[i]) != ',' && c != '\0')
                        i++;
                if (c == '\0')
                        break;
        }

        if (record->size <= 0) {
                CSVP_FREE(record);
                return NULL;
        }

        record->entries = CSVP_MALLOC(record->size * sizeof(*record->entries));
        if (!record->entries) {
#ifdef CSVP_DEBUG
                CSVP_LOG_MESSAGE("Failed to allocate memory for record entries!");
#endif /* CSVP_DEBUG */
                CSVP_FREE(record);
                return NULL;
        }

        for (int i = 0; i < record->size; i++) {
                record->entries[i].type = types[i];
                if (types[i] == CSVP_STRING_TYPE)
                        csvp_string_init(&record->entries[i].v_string);
        }
        return record;
}

static ssize_t
csvp_getline(char *buffer, size_t buffer_size, FILE *fd)
{
#ifdef CSVP_DEBUG
        assert(buffer);
        assert(buffer_size > 0);
        assert(fd);
#endif /* CSVP_DEBUG */
        size_t i, c;
        for (i = 0, c; i < buffer_size - 1
                     && (c = fgetc(fd)) != '\0' && c != '\n' && c != EOF; i++)
                buffer[i] = c;
        buffer[i] = '\0';
        return i;
}

CSVPDEF int
csvp_next_record(FILE *fd, csvp_record_t *record)
{
        size_t i, c, idx;
        ssize_t buffer_size;
        char buffer[RECORD_BUFSIZE];
#ifdef CSVP_DEBUG
        assert(fd);
        assert(record);
#endif /* CSVP_DEBUG */
        buffer_size = csvp_getline(buffer, RECORD_BUFSIZE, fd);
        for (i = 0, c, idx = 0; (c = buffer[i]) != '\0'
                     && c != '\n' && idx < record->size; i++) {
                while ((c = buffer[i]) == ' ')
                        i++;
                if (c == '\0' || c == '\n')
                        break;
                switch (record->entries[idx].type) {
                case CSVP_INTEGER_TYPE:
                        if (csvp_check_integer(buffer+i, buffer_size-i)) {
                                csvp_write_integer(record->entries+idx, buffer+i, buffer_size-i);
                                idx++;
                        } else {
                                idx = 0;
                        }
                        break;
                case CSVP_FLOAT_TYPE:
                        if (csvp_check_float(buffer+i, buffer_size-i)) {
                                csvp_write_float(record->entries+idx, buffer+i, buffer_size-i);
                                idx++;
                        } else {
                                idx = 0;
                        }
                        break;
                case CSVP_STRING_TYPE:
                        csvp_write_string(record->entries+idx, buffer+i, buffer_size-i);
                        idx++;
                        break;
                }

                while ((c = buffer[i]) != ',' && c != '\0')
                        i++;
                if (c == '\0')
                        break;
        }

        return buffer_size;
}

CSVPDEF int
csvp_prev_record(FILE *fd, csvp_record_t *record)
{
#ifdef CSVP_DEBUG
        assert(fd);
        assert(record);
#endif /* CSVP_DEBUG */
        return 0;
}

CSVPDEF void
csvp_record_free(csvp_record_t *record)
{
#ifdef CSVP_DEBUG
        assert(record);
        assert(record->entries);
        assert(record->size > 0);
#endif /* CSVP_DEBUG */
        for (int i = 0; i < record->size; i++) {
                csvp_entry_t *entry = record->entries+i;
                if (entry->type == CSVP_STRING_TYPE)
                        csvp_string_free(&entry->v_string);
        }
        CSVP_FREE(record->entries);
        CSVP_FREE(record);
}

#endif /* CSVP_IMPLEMENTATION */
#endif /* CSVP_INCLUDE_H_ */
