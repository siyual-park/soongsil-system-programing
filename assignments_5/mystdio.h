#ifndef MYSTDIO_H
#define MYSTDIO_H

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define BUFSIZE 4
#define EOF -1

#define stdin  1
#define stdout 2
#define stderr 3

typedef struct FILE {
    int fd;
    int flag;

    char *buffer;
    size_t buffer_size;
    int buffer_offset;

    int offset;

    bool eof;
} FILE;

int get_flags(const char *mode) {
    static char *modes[] = { 
        "r", "rb", "r+", "rb+", "r+b", 
        "w", "wb", "w+", "wb+", "w+b", 
        "a", "ab", "a+", "ab+", "a+b" 
    };
    static int flags[] = { 
        O_RDONLY, O_RDONLY, O_RDWR, O_RDWR, O_RDWR, 
        O_WRONLY|O_CREAT|O_TRUNC, O_WRONLY|O_CREAT|O_TRUNC, O_RDWR|O_CREAT|O_TRUNC, O_RDWR|O_CREAT|O_TRUNC, O_RDWR|O_CREAT|O_TRUNC,
        O_WRONLY|O_CREAT|O_APPEND, O_WRONLY|O_CREAT|O_APPEND, O_RDWR|O_CREAT|O_APPEND, O_RDWR|O_CREAT|O_APPEND, O_RDWR|O_CREAT|O_APPEND
    };
    static int modes_size = 15;
    
    for (int i  = 0; i < modes_size; i++) {
        if (strcmp(modes[i], mode) == 0) {
            return flags[i];
        }
    }

    return -1;
}

FILE *fopen(const char *pathname, const char *mode) {
    int flag = get_flags(mode);
    assert(flag != -1);
    
    // O_APPEND 는 fwrite 부분에서 제어
    int fd = open(pathname, flag & (~O_APPEND));

    int offset = 0;
    offset = lseek(fd, 0, SEEK_SET);

    char *buffur = malloc(BUFSIZE);
    assert(buffur != NULL);
    memset(buffur, 0, sizeof(BUFSIZE));
    size_t size = read(fd, buffur, BUFSIZE);
    if (size > BUFSIZE) {
        size = 0;
    }

    FILE *file = malloc(sizeof(FILE));
    assert(file != NULL);
    memset(file, 0, sizeof(FILE));

    file->flag = flag;
    file->fd = fd;

    file->buffer = buffur;
    file->buffer_size = size;
    file->buffer_offset = 0;

    file->offset = offset;

    file->eof = false;

    return file;
}

int fflush(FILE *stream) {    
    if ((stream->flag & O_ACCMODE) == O_RDONLY || stream->buffer_size == 0) {
        return 0;
    }
    
    int current = lseek(stream->fd, 0, SEEK_CUR);
    if (current == -1) {
        return EOF;
    }
    if (lseek(stream->fd, stream->offset, SEEK_SET) == -1) {
        return EOF;
    }

    int result = 0;
    if (write(stream->fd, stream->buffer, stream->buffer_size) == -1) {
        result = EOF;
    }

    lseek(stream->fd, current, SEEK_SET);

    return result;
}

int feof(FILE *stream) {
    if (stream->eof) {
        return EOF;
    }

    return 0;
}

int not_update_eof_seek(FILE *stream, int offset, int whence) {
    fflush(stream);
    
    if (lseek(stream->fd, stream->offset + stream->buffer_offset, SEEK_SET) == EOF) {
        return EOF;
    }

    int file_offset = lseek(stream->fd, offset, whence);
    if (file_offset == -1) {
        return EOF;
    }

    size_t size = read(stream->fd, stream->buffer, BUFSIZE);
    if (size > BUFSIZE) {
        size = 0;
    }

    stream->offset = file_offset;
    stream->buffer_size = size;
    stream->buffer_offset = 0;

    if (lseek(stream->fd, file_offset, SEEK_SET) == -1) {
        return EOF;
    }

    if (size == 0) {
        return EOF;
    }

    return file_offset;
}

int fseek(FILE *stream, int offset, int whence) {
    int result = not_update_eof_seek(stream, offset, whence);
    if (result == EOF) {
        stream->eof = true;
    } else {
        stream->eof = false;
    }

    return result;
}

int fread(void *ptr, int size, int nmemb, FILE *stream) {
    if ((stream->flag & O_ACCMODE) == O_WRONLY) {
        return 0;
    }

    int renaming = size * nmemb;
    char *current = ptr;
    while (renaming > 0) {
        if (stream->buffer_offset == stream->buffer_size && fseek(stream, stream->offset + stream->buffer_size, SEEK_SET) == EOF) {
            break;
        }

        size_t renaming_buffer = stream->buffer_size - stream->buffer_offset;
        size_t read_size = renaming <= renaming_buffer ? renaming : renaming_buffer;

        if (read_size > 0) {
            memccpy(current, stream->buffer + stream->buffer_offset, 1, read_size);
            current += read_size;
            renaming -= read_size;
            stream->buffer_offset += read_size;
        }
    }

    return size * nmemb - renaming;
}

int fwrite(const void *ptr, int size, int nmemb, FILE *stream) {
    if ((stream->flag & O_ACCMODE) == O_RDONLY) {
        return 0;
    }

    int origin_offset = -1;
    if ((stream->flag & O_APPEND) == O_APPEND) {
        origin_offset = lseek(stream->fd, 0, SEEK_CUR);
        assert(origin_offset != -1);
        not_update_eof_seek(stream, 0, SEEK_END);
    }

    int renaming = size * nmemb;
    const char *current = ptr;
    while (renaming > 0) {
        if (stream->buffer_offset == BUFSIZE) {
            not_update_eof_seek(stream, stream->offset + stream->buffer_size, SEEK_SET);
        }

        size_t renaming_buffer = BUFSIZE - stream->buffer_offset;
        size_t write_size = renaming <= renaming_buffer ? renaming : renaming_buffer;

        memccpy(stream->buffer + stream->buffer_offset, current, 1, write_size);
        current += write_size;
        renaming -= write_size;
        stream->buffer_offset += write_size;
        if (stream->buffer_offset > stream->buffer_size) {
            stream->buffer_size = stream->buffer_offset;
        }
    }

    if ((stream->flag & O_APPEND) == O_APPEND) {
        not_update_eof_seek(stream, origin_offset, SEEK_SET);
    }

    return size * nmemb - renaming;
}

int fgetc(FILE *stream) {
    unsigned char ch = '\0';
    size_t read_size = fread(&ch, 1, 1, stream);
    
    if (read_size == 0) {
        return EOF;
    }
    return (int) ch;
}

int fputc(int c, FILE *stream) {
    unsigned char ch = c;
    size_t write_size = fwrite(&ch, 1, 1, stream);
    
    if (write_size == 0) {
        return EOF;
    }
    return (int) ch;
}

int fclose(FILE *stream) {
    if (fflush(stream) == EOF) {
        return EOF;
    }
    if (close(stream->fd) == -1) {
        return EOF;
    }
    free(stream->buffer);
    free(stream);

    return 0;
}

#endif //MYSTDIO_H
